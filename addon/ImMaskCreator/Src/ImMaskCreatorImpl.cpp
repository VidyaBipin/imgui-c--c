#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <chrono>
#include <imgui_internal.h>
#include <unordered_map>
#include "ImMaskCreator.h"
#include "Contour2Mask.h"
#include "MatMath.h"
#include "Morph.h"
#include "MatUtilsImguiHelper.h"
#include "SysUtils.h"

using namespace std;
namespace json = imgui_json;

namespace ImGui
{
class MaskCreatorImpl : public MaskCreator
{
public:
    MaskCreatorImpl(const MatUtils::Size2i& size, const string& name) : m_size(size), m_name(name), m_tMorphCtrl(this)
    {
        m_v2PointSizeHalf = m_v2PointSize/2;
        m_itMorphCtrlVt = m_itHoveredVertex = m_atContourPoints.end();
        m_mMorphKernel = MatUtils::GetStructuringElement(MatUtils::MORPH_ELLIPSE, {5, 5});
    }

    string GetName() const override
    {
        return m_name;
    }

    void SetName(const string& name) override
    {
        m_name = name;
    }

    bool DrawContent(const ImVec2& v2Pos, const ImVec2& v2Size, bool bUpdateUiScale) override
    {
        ImRect bb(v2Pos, v2Pos+v2Size);
        if (!ItemAdd(bb, 0))
        {
            ostringstream oss; oss << "FAILED at 'ItemAdd' with item rect (("
                    << bb.Min.x << ", " << bb.Min.y << "), (" << bb.Max.x << ", " << bb.Max.y << "))!";
            m_sErrMsg = oss.str();
            return false;
        }
        m_rWorkArea = bb;
        if (bUpdateUiScale)
        {
            m_v2UiScale.x = v2Size.x/(float)m_size.x;
            m_v2UiScale.y = v2Size.y/(float)m_size.y;
        }

        // reactions
        const auto mousePosAbs = GetMousePos();
        const auto& origin = m_rWorkArea.Min;
        const auto temp = mousePosAbs-origin;
        const ImVec2 mousePos(temp.x/m_v2UiScale.x, temp.y/m_v2UiScale.y);
        const bool bMouseInWorkArea = m_rWorkArea.Contains(mousePosAbs);
        // check hovering state
        if (!IsMouseDown(ImGuiMouseButton_Left))
        {
            auto iter = m_atContourPoints.begin();
            while (iter != m_atContourPoints.end())
            {
                auto& v = *iter;
                if (v.CheckGrabberHovering(mousePos))
                    break;
                iter++;
            }
            if (iter != m_atContourPoints.end())
            {
                if (HasHoveredVertex() && m_itHoveredVertex != iter)
                    m_itHoveredVertex->QuitHover();
                if (HasHoveredMorphCtrl())
                    m_tMorphCtrl.QuitHover();
                m_itHoveredVertex = iter;
            }
            if ((!HasHoveredVertex() || HasHoveredContour()) && IsMorphCtrlShown())
            {
                if (m_tMorphCtrl.CheckHovering(mousePos))
                {
                    if (HasHoveredContour())
                    {
                        m_itHoveredVertex->QuitHover();
                        m_itHoveredVertex = m_atContourPoints.end();
                    }
                }
            }
            if (!HasHoveredSomething())
            {
                iter = m_atContourPoints.begin();
                while (iter != m_atContourPoints.end())
                {
                    auto& v = *iter;
                    if (v.CheckContourHovering(mousePos))
                        break;
                    iter++;
                }
                if (iter != m_atContourPoints.end())
                    m_itHoveredVertex = iter;
            }
        }
        if (IsMouseClicked(ImGuiMouseButton_Left) && bMouseInWorkArea)
        {
            if (!HasHoveredVertex() && !m_bContourCompleted && !IsKeyDown(m_eRemoveVertexKey))
            {
                // add new vertex to the end of vertex list
                m_atContourPoints.push_back({this, mousePos});
                auto iter = m_atContourPoints.end();
                iter--;
                m_itHoveredVertex = iter;
                iter->UpdateGrabberContainBox();
                UpdateContourVertices(m_itHoveredVertex);
            }
            else if (HasHoveredVertex())
            {
                if (IsKeyDown(m_eRemoveVertexKey))
                {
                    if (m_itHoveredVertex->m_iHoverType == 0)
                    {
                        // remove vertex
                        bool bUpdateMorphCtrlIter = m_itHoveredVertex == m_itMorphCtrlVt;
                        auto iterNextVt = m_atContourPoints.erase(m_itHoveredVertex);
                        m_itHoveredVertex = m_atContourPoints.end();
                        if (m_atContourPoints.size() <= 2)
                        {
                            m_bContourCompleted = false;
                            m_itMorphCtrlVt = m_atContourPoints.end();
                        }
                        if (m_bContourCompleted && iterNextVt == m_atContourPoints.end())
                            iterNextVt = m_atContourPoints.begin();
                        if (iterNextVt != m_atContourPoints.end())
                        {
                            if (bUpdateMorphCtrlIter)
                                m_itMorphCtrlVt = iterNextVt;
                            UpdateContourVertices(iterNextVt);
                        }
                    }
                    else
                    {
                        // disable bezier curve
                        auto& v = *m_itHoveredVertex;
                        v.m_bEnableBezier = false;
                        v.m_bFirstDrag = true;
                        v.m_grabber0Offset = {0.f, 0.f};
                        v.m_grabber1Offset = {0.f, 0.f};
                        v.UpdateGrabberContainBox();
                        UpdateContourVertices(m_itHoveredVertex);
                    }
                }
                else if (!m_bContourCompleted && m_itHoveredVertex == m_atContourPoints.begin() && m_itHoveredVertex->m_iHoverType == 0 && m_atContourPoints.size() >= 3)
                {
                    // complete the contour
                    m_bContourCompleted = true;
                    auto iterVt = m_atContourPoints.begin();
                    iterVt++;
                    m_itMorphCtrlVt = iterVt;
                    UpdateContourVertices(m_atContourPoints.begin());
                }
                else if (m_itHoveredVertex->m_iHoverType == 4)
                {
                    // insert new vertex on the contour
                    bool bNeedUpdateMorphCtrlIter = m_itHoveredVertex == m_itMorphCtrlVt;
                    m_itHoveredVertex = m_atContourPoints.insert(m_itHoveredVertex, {this, m_itHoveredVertex->m_v2HoverPointOnContour});
                    if (bNeedUpdateMorphCtrlIter) m_itMorphCtrlVt = m_itHoveredVertex;
                    m_itHoveredVertex->UpdateGrabberContainBox();
                    UpdateContourVertices(m_itHoveredVertex);
                }
            }
        }
        bool bMorphChanged = false;
        if (IsMouseDragging(ImGuiMouseButton_Left))
        {
            if (HasHoveredVertex())
            {
                ImGui::SetNextFrameWantCaptureMouse(true);
                if (m_itHoveredVertex->m_bJustAdded || !m_itHoveredVertex->m_bEnableBezier && IsKeyDown(m_eEnableBezierKey))
                {
                    // enable bezier curve
                    m_itHoveredVertex->m_bEnableBezier = true;
                    m_itHoveredVertex->m_iHoverType = 2;
                }
                bool bContourChanged = false;
                const auto coordOff = mousePos-m_itHoveredVertex->m_pos;
                // cout << "---> iHoverType = " << m_itHoveredVertex->m_iHoverType << endl;
                if (m_itHoveredVertex->m_bFirstDrag && m_itHoveredVertex->m_bEnableBezier && m_itHoveredVertex->m_grabber1Offset != coordOff)
                {
                    // 1st-time dragging bezier grabber, change both the grabbers
                    m_itHoveredVertex->m_grabber0Offset = -coordOff;
                    m_itHoveredVertex->m_grabber1Offset = coordOff;
                    bContourChanged = true;
                }
                else if (m_itHoveredVertex->m_iHoverType == 0 && bMouseInWorkArea && m_itHoveredVertex->m_pos != mousePos)
                {
                    // moving the vertex
                    m_itHoveredVertex->m_pos = mousePos;
                    bContourChanged = true;
                }
                else if (m_itHoveredVertex->m_iHoverType == 1 && m_itHoveredVertex->m_grabber0Offset != coordOff)
                {
                    // moving bezier grabber0
                    m_itHoveredVertex->m_grabber0Offset = coordOff;
                    auto c0sqr = coordOff.x*coordOff.x+coordOff.y*coordOff.y;
                    auto& coordOff1 = m_itHoveredVertex->m_grabber1Offset;
                    auto c1sqr = coordOff1.x*coordOff1.x+coordOff1.y*coordOff1.y;
                    auto ratio = sqrt(c1sqr/c0sqr);
                    coordOff1.x = -coordOff.x*ratio;
                    coordOff1.y = -coordOff.y*ratio;
                    bContourChanged = true;
                }
                else if (m_itHoveredVertex->m_iHoverType == 2 && m_itHoveredVertex->m_grabber1Offset != coordOff)
                {
                    // moving bezier grabber1
                    m_itHoveredVertex->m_grabber1Offset = coordOff;
                    auto c1sqr = coordOff.x*coordOff.x+coordOff.y*coordOff.y;
                    auto& coordOff0 = m_itHoveredVertex->m_grabber0Offset;
                    auto c0sqr = coordOff0.x*coordOff0.x+coordOff0.y*coordOff0.y;
                    auto ratio = sqrt(c0sqr/c1sqr);
                    coordOff0.x = -coordOff.x*ratio;
                    coordOff0.y = -coordOff.y*ratio;
                    bContourChanged = true;
                }
                if (bContourChanged)
                {
                    m_itHoveredVertex->UpdateGrabberContainBox();
                    UpdateContourVertices(m_itHoveredVertex);
                }
            }
            else if (HasHoveredMorphCtrl())
            {
                // moving morph controller's root position
                if (m_tMorphCtrl.m_iHoverType == 0)
                {
                    ImVec2 v2VertSlope;
                    int iLineIdx = 0;
                    auto ptRootPos = MatUtils::CalcNearestPointOnPloygon(mousePos, m_av2AllContourVertices, &v2VertSlope, &iLineIdx);
                    const auto fCtrlSlope = v2VertSlope.y == 0 ? numeric_limits<float>::infinity() : v2VertSlope.x/v2VertSlope.y;
                    m_itMorphCtrlVt = m_tMorphCtrl.SetPosAndSlope(ptRootPos, fCtrlSlope, iLineIdx);
                }
                else if (m_tMorphCtrl.m_iHoverType == 1)
                {
                    const auto dx = mousePos.x-m_tMorphCtrl.m_ptRootPos.x;
                    const auto dy = mousePos.y-m_tMorphCtrl.m_ptRootPos.y;
                    float l = sqrt(dx*dx+dy*dy);
                    if (l < MorphController::MIN_CTRL_LENGTH) l = MorphController::MIN_CTRL_LENGTH;
                    if (m_tMorphCtrl.m_fMorphCtrlLength != l)
                    {
                        m_tMorphCtrl.m_fMorphCtrlLength = l;
                        auto iMorphIters = (int)floor(l-MorphController::MIN_CTRL_LENGTH);
                        // auto iMorphIters = (int)std::ceil(tmp*2/(m_mMorphKernel.w-1));
                        if (iMorphIters != m_tMorphCtrl.m_iMorphIterations)
                        {
                            m_tMorphCtrl.m_iMorphIterations = iMorphIters;
                            bMorphChanged = true;
                        }
                        const auto& ptOrgRootPos = m_tMorphCtrl.m_ptRootPos;
                        const auto& ptOrgGrabberPos = m_tMorphCtrl.m_ptGrabberPos;
                        const auto& fVertSlope = isinf(m_tMorphCtrl.m_fCtrlSlope) ? 0 : m_tMorphCtrl.m_fCtrlSlope == 0 ? numeric_limits<float>::infinity() : -1/m_tMorphCtrl.m_fCtrlSlope;
                        bool bGrabberSide = isinf(fVertSlope) ? ptOrgGrabberPos.x > ptOrgRootPos.x : ptOrgGrabberPos.y > fVertSlope*(ptOrgGrabberPos.x-ptOrgRootPos.x)+ptOrgRootPos.y;
                        bool bMousePosSide = isinf(fVertSlope) ? mousePos.x > ptOrgRootPos.x : mousePos.y > fVertSlope*(mousePos.x-ptOrgRootPos.x)+ptOrgRootPos.y;
                        if (bGrabberSide != bMousePosSide)
                        {
                            m_tMorphCtrl.m_bInsidePoly = !m_tMorphCtrl.m_bInsidePoly;
                            bMorphChanged = true;
                        }
                        m_tMorphCtrl.CalcGrabberPos();
                    }
                }
                else if (m_tMorphCtrl.m_iHoverType == 2)
                {
                    const auto dx = mousePos.x-m_tMorphCtrl.m_ptRootPos.x;
                    const auto dy = mousePos.y-m_tMorphCtrl.m_ptRootPos.y;
                    float l = sqrt(dx*dx+dy*dy);
                    const auto fMinFeatherGrabberLength = m_tMorphCtrl.m_fMorphCtrlLength+m_fGrabberRadius*2+m_fHoverDetectExRadius;
                    float fFeatherGrabberLength = l-fMinFeatherGrabberLength;
                    if (fFeatherGrabberLength < 0) fFeatherGrabberLength = 0;
                    if (m_tMorphCtrl.m_fFeatherCtrlLength != fFeatherGrabberLength)
                    {
                        m_tMorphCtrl.m_fFeatherCtrlLength = fFeatherGrabberLength;
                        m_tMorphCtrl.m_iFeatherIterations = (int)floor(fFeatherGrabberLength);
                        bMorphChanged = true;
                        m_tMorphCtrl.CalcGrabberPos();
                    }
                }
            }
        }
        else if (!IsMouseDown(ImGuiMouseButton_Left))
        {
            if (HasHoveredVertex())
            {
                if (!m_itHoveredVertex->CheckHovering(mousePos))
                {
                    // quit hovering state
                    m_itHoveredVertex->QuitHover();
                    m_itHoveredVertex = m_atContourPoints.end();
                }
            }
            else if (HasHoveredMorphCtrl())
            {
                if (!m_tMorphCtrl.CheckHovering(mousePos))
                {
                    m_tMorphCtrl.m_bIsHovered = false;
                }
            }
        }
        if (IsMouseReleased(ImGuiMouseButton_Left) && HasHoveredVertex())
        {
            ImGui::SetNextFrameWantCaptureMouse(false);
            if (m_itHoveredVertex->m_bEnableBezier)
                m_itHoveredVertex->m_bFirstDrag = false;
            m_itHoveredVertex->m_bJustAdded = false;
        }

        // draw mask trajectory
        ImDrawList* pDrawList = GetWindowDrawList();
        auto pointIter = m_atContourPoints.begin();
        auto prevIter = m_atContourPoints.end();
        // draw contour
        while (pointIter != m_atContourPoints.end())
        {
            if (prevIter != m_atContourPoints.end())
                DrawContour(pDrawList, *prevIter, *pointIter);
            prevIter = pointIter++;
        }
        if (m_bContourCompleted)
            DrawContour(pDrawList, *prevIter, m_atContourPoints.front());
        // draw points
        for (const auto& p : m_atContourPoints)
            DrawPoint(pDrawList, p);
        // draw morph controller
        if (IsMorphCtrlShown())
            DrawMorphController(pDrawList);
        // draw hovering point on contour
        if (HasHoveredVertex() && m_itHoveredVertex->m_iHoverType == 4)
        {
            const auto pointPos = origin+m_itHoveredVertex->m_v2HoverPointOnContour*m_v2UiScale;
            const auto& offsetSize1 = m_v2PointSizeHalf;
            pDrawList->AddRectFilled(pointPos-offsetSize1, pointPos+offsetSize1, m_u32PointBorderHoverColor);
            const ImVec2 offsetSize2(m_v2PointSizeHalf.x-m_fPointBorderThickness, m_v2PointSizeHalf.y-m_fPointBorderThickness);
            pDrawList->AddRectFilled(pointPos-offsetSize2, pointPos+offsetSize2, m_u32ContourHoverPointColor);
        }
        return m_bContourCompleted && (m_bContourChanged || bMorphChanged);
    }

    bool ChangeMaskSize(const MatUtils::Size2i& size) override
    {
        if (m_size != size)
        {
            m_size = size;
            m_bContourChanged = true;
        }
        return true;
    }

    MatUtils::Size2i GetMaskSize() const override
    {
        return m_size;
    }

    ImGui::ImMat GetMask(int iLineType, bool bFilled, ImDataType eDataType, double dMaskValue, double dNonMaskValue) override
    {
        ImGui::ImMat res;
        if (!m_bContourCompleted)
            return res;

        res = m_mMask;
        const auto iMorphIters = m_tMorphCtrl.m_bInsidePoly ? -m_tMorphCtrl.m_iMorphIterations : m_tMorphCtrl.m_iMorphIterations;
        const auto iFeatherIters = m_tMorphCtrl.m_iFeatherIterations;
        if (m_bContourChanged || m_mMask.empty() ||
            m_iLastMaskLineType != iLineType || m_bLastMaskFilled != bFilled ||
            m_eLastMaskDataType != eDataType || m_dLastMaskValue != dMaskValue || m_dLastNonMaskValue != dNonMaskValue)
        {
            if ((iMorphIters == 0 && iFeatherIters == 0) || !bFilled)
            {
                int iTotalVertexCount = m_av2AllContourVertices.size();
                vector<MatUtils::Point2f> av2TotalVertices(iTotalVertexCount);
                auto itVtSrc = m_av2AllContourVertices.begin();
                auto itVtDst = av2TotalVertices.begin();
                while (itVtSrc != m_av2AllContourVertices.end())
                    *itVtDst++ = MatUtils::FromImVec2<float>(*itVtSrc++);
                m_mMask = MatUtils::Contour2Mask(av2TotalVertices, m_size, {0, 0}, eDataType, dMaskValue, dNonMaskValue, iLineType, bFilled);
                res = m_mMask;
            }
            else
            {
                m_mMask.release();
            }
        }

        if (iMorphIters != 0 || iFeatherIters != 0)
        {
            if (m_bContourChanged || m_iLastMorphIters != iMorphIters || m_iLastFeatherIters != iFeatherIters ||
                m_iLastMaskLineType != iLineType || m_bLastMaskFilled != bFilled ||
                m_eLastMaskDataType != eDataType || m_dLastMaskValue != dMaskValue || m_dLastNonMaskValue != dNonMaskValue)
            {
                vector<MatUtils::Point2f> aMorphContourVertices;
                ImGui::ImMat mColor;
                if (!bFilled)
                {
                    m_mMorphMask = m_mMask.clone();
                    if (iMorphIters != 0)
                    {
                        aMorphContourVertices = iMorphIters < 0 ? ErodeContour(-iMorphIters) : DilateContour(iMorphIters);
                        mColor = MatUtils::MakeColor(res.type, dMaskValue);
                        MatUtils::DrawPolygon(m_mMorphMask, aMorphContourVertices, mColor, iLineType);
                    }
                    if (iFeatherIters > 0)
                    {
                        mColor = MatUtils::MakeColor(res.type, dMaskValue/2);
                        const auto iFeatherIter1 = iMorphIters-iFeatherIters;
                        if (iFeatherIter1 != 0)
                        {
                            aMorphContourVertices = iFeatherIter1 < 0 ? ErodeContour(-iFeatherIter1) : DilateContour(iFeatherIter1);
                            MatUtils::DrawPolygon(m_mMorphMask, aMorphContourVertices, mColor, iLineType);
                        }
                        const auto iFeatherIter2 = iMorphIters+iFeatherIters;
                        if (iFeatherIter2 != 0)
                        {
                            aMorphContourVertices = iFeatherIter2 < 0 ? ErodeContour(-iFeatherIter2) : DilateContour(iFeatherIter2);
                            MatUtils::DrawPolygon(m_mMorphMask, aMorphContourVertices, mColor, iLineType);
                        }
                    }
                }
                else
                {
                    if (iFeatherIters > 0)
                    {
                        const auto iOutterMorphIters = iMorphIters+iFeatherIters;
                        aMorphContourVertices = iOutterMorphIters < 0 ? ErodeContour(-iOutterMorphIters) : DilateContour(iOutterMorphIters);
                        const auto iFeatherSize = 2*iFeatherIters+1;
                        // aMorphContourVertices = ConvertPointsType(m_av2AllContourVertices);
                        m_mMorphMask = MatUtils::Contour2Mask(aMorphContourVertices, m_size, {0, 0}, eDataType, dMaskValue, dNonMaskValue, iLineType, bFilled, iFeatherSize);
                    }
                    else
                    {
                        aMorphContourVertices = iMorphIters < 0 ? ErodeContour(-iMorphIters) : DilateContour(iMorphIters);
                        m_mMorphMask = MatUtils::Contour2Mask(aMorphContourVertices, m_size, {0, 0}, eDataType, dMaskValue, dNonMaskValue, iLineType, bFilled);
                    }
                }
                m_iLastMorphIters = iMorphIters;
                m_iLastFeatherIters = iFeatherIters;
            }
            res = m_mMorphMask;
        }

        m_bContourChanged = false;
        m_iLastMaskLineType = iLineType;
        m_bLastMaskFilled = bFilled;
        m_eLastMaskDataType = eDataType;
        m_dLastMaskValue = dMaskValue;
        m_dLastNonMaskValue = dNonMaskValue;
        return res;
    }

    const ContourPoint* GetHoveredPoint() const override
    {
        if (HasHoveredVertex())
            return static_cast<ContourPoint*>(&(*m_itHoveredVertex));
        return nullptr;
    }

    ImVec4 GetContourContainBox() const override
    {
        ImRect rBox(m_rContianBox);
        rBox.Min *= m_v2UiScale;
        rBox.Max *= m_v2UiScale;
        rBox.Min += m_rWorkArea.Min;
        rBox.Max += m_rWorkArea.Min;
        return ImVec4(rBox.Min.x, rBox.Min.y, rBox.Max.x, rBox.Max.y);
    }

    ImVec2 GetUiScale() const override
    {
        return m_v2UiScale;
    }

    bool SaveAsJson(imgui_json::value& j) const override
    {
        j = imgui_json::value();
        json::array subj;
        for (const auto& cp : m_atContourPoints)
            subj.push_back(cp.ToJson());
        j["contour_points"] = subj;
        j["morph_controller"] = m_tMorphCtrl.ToJson();
        if (IsMorphCtrlShown())
        {
            int cpidx = 0;
            auto it = m_atContourPoints.begin();
            while (it != m_itMorphCtrlVt && it != m_atContourPoints.end())
            {
                cpidx++; it++;
            }
            if (it != m_atContourPoints.end())
                j["morph_ctrl_cpidx"] = json::number(cpidx);
            else
                j["morph_ctrl_cpidx"] = json::number(-1);
        }
        else
        {
            j["morph_ctrl_cpidx"] = json::number(-1);
        }
        j["name"] = json::string(m_name);
        j["size"] = MatUtils::ToImVec2(m_size);
        j["point_size"] = m_v2PointSize;
        j["point_color"] = json::number(m_u32PointColor);
        j["point_border_color"] = json::number(m_u32PointBorderColor);
        j["point_border_hover_color"] = json::number(m_u32PointBorderHoverColor);
        j["point_border_thickness"] = m_fPointBorderThickness;
        j["contour_hover_point_color"] = json::number(m_u32ContourHoverPointColor);
        j["grabber_radius"] = m_fGrabberRadius;
        j["grabber_border_thickness"] = m_fGrabberBorderThickness;
        j["grabber_color"] = json::number(m_u32GrabberColor);
        j["grabber_border_color"] = json::number(m_u32GrabberBorderColor);
        j["grabber_border_hover_color"] = json::number(m_u32GrabberBorderHoverColor);
        j["grabberline_thickness"] = m_fGrabberLineThickness;
        j["grabberline_color"] = json::number(m_u32GrabberLineColor);
        j["contour_thickness"] = m_fContourThickness;
        j["contour_color"] = json::number(m_u32ContourColor);
        j["contour_hover_detect_ex_radius"] = m_fContourHoverDetectExRadius;
        j["hover_detect_ex_radius"] = m_fHoverDetectExRadius;
        j["contour_complete"] = m_bContourCompleted;
        j["mask_filled"] = m_bLastMaskFilled;
        j["mask_line_type"] = json::number(m_iLastMaskLineType);
        return true;
    }

    bool SaveAsJson(const std::string& filePath) const override
    {
        json::value j;
        if (!SaveAsJson(j))
            return false;
        j.save(filePath);
        return true;
    }

    void LoadFromJson(const json::value& j)
    {
        if (j.contains("name")) m_name = j["name"].get<json::string>();
        if (j.contains("size")) m_size = MatUtils::FromImVec2<int32_t>(j["size"].get<json::vec2>());
        m_v2PointSize = j["point_size"].get<json::vec2>();
        m_u32PointColor = j["point_color"].get<json::number>();
        m_u32PointBorderColor = j["point_border_color"].get<json::number>();
        m_u32PointBorderHoverColor = j["point_border_hover_color"].get<json::number>();
        m_fPointBorderThickness = j["point_border_thickness"].get<json::number>();
        m_u32ContourHoverPointColor = j["contour_hover_point_color"].get<json::number>();
        m_fGrabberRadius = j["grabber_radius"].get<json::number>();
        m_fGrabberBorderThickness = j["grabber_border_thickness"].get<json::number>();
        m_u32GrabberColor = j["grabber_color"].get<json::number>();
        m_u32GrabberBorderColor = j["grabber_border_color"].get<json::number>();
        m_u32GrabberBorderHoverColor = j["grabber_border_hover_color"].get<json::number>();
        m_fGrabberLineThickness = j["grabberline_thickness"].get<json::number>();
        m_u32GrabberLineColor = j["grabberline_color"].get<json::number>();
        m_fContourThickness = j["contour_thickness"].get<json::number>();
        m_u32ContourColor = j["contour_color"].get<json::number>();
        m_fContourHoverDetectExRadius = j["contour_hover_detect_ex_radius"].get<json::number>();
        m_fHoverDetectExRadius = j["hover_detect_ex_radius"].get<json::number>();
        m_bContourCompleted = j["contour_complete"].get<json::boolean>();
        m_bLastMaskFilled = j["mask_filled"].get<json::boolean>();
        m_iLastMaskLineType = j["mask_line_type"].get<json::number>();
        const auto& subj = j["contour_points"].get<json::array>();
        for (const auto& elem : subj)
        {
            ContourPointImpl cp(this, {0, 0});
            cp.FromJson(elem);
            m_atContourPoints.push_back(std::move(cp));
        }
        m_tMorphCtrl.FromJson(j["morph_controller"]);

        m_v2PointSizeHalf = m_v2PointSize/2;
        m_itMorphCtrlVt = m_itHoveredVertex = m_atContourPoints.end();
        auto itVt = m_atContourPoints.begin();
        while (itVt != m_atContourPoints.end())
        {
            itVt->UpdateGrabberContainBox();
            UpdateContourVertices(itVt);
            itVt++; if (itVt == m_atContourPoints.end()) break;
            itVt->UpdateGrabberContainBox();
            itVt++;
        }
        int cpidx = j["morph_ctrl_cpidx"].get<json::number>();
        if (cpidx >= 0)
        {
            auto it = m_atContourPoints.begin();
            for (int i = 0; i < cpidx; i++)
                it++;
            m_itMorphCtrlVt = it;
            m_tMorphCtrl.SetPosAndSlope(it, m_tMorphCtrl.m_fMorphCtrlLength);
        }
    }

    string GetError() const override
    {
        return m_sErrMsg;
    }

private:
    struct ContourPointImpl : public ContourPoint
    {
        ContourPointImpl(MaskCreatorImpl* owner, const ImVec2& pos)
            : m_owner(owner)
            , m_pos(pos)
            , m_rGrabberContBox(pos, pos)
            , m_rContourContBox(-1.f, -1.f, -1.f, -1.f)
        {}

        MaskCreatorImpl* m_owner;
        ImVec2 m_pos;
        bool m_bJustAdded{true};  // when point is 1st time added, grabbing op will trigger bezier grabber. after mouse button released, following dragging op will treated as moving
        bool m_bEnableBezier{false};
        bool m_bFirstDrag{true};  // 1st time dragging bezier grabber, both 1&2 grabbers move together
        bool m_bHovered{true};
        int m_iHoverType{0};  // 0: on contour point, 1: on 1st bezier grabber, 2: on 2nd bezier grabber, 4: on contour
        ImVec2 m_grabber0Offset{0.f, 0.f};
        ImVec2 m_grabber1Offset{0.f, 0.f};
        list<ImVec2> m_aContourVertices;
        ImVec2 m_v2HoverPointOnContour;
        int m_iHoverPointOnContourIdx;
        ImRect m_rGrabberContBox;
        ImRect m_rContourContBox;

        json::value ToJson() const
        {
            json::value j;
            j["pos"] = m_pos;
            j["enable_bezier"] = m_bEnableBezier;
            j["first_drag"] = m_bFirstDrag;
            j["grabber0_offset"] = m_grabber0Offset;
            j["grabber1_offset"] = m_grabber1Offset;
            return std::move(j);
        }

        void FromJson(const json::value& j)
        {
            m_pos = j["pos"].get<json::vec2>();
            m_bEnableBezier = j["enable_bezier"].get<json::boolean>();
            m_bFirstDrag = j["first_drag"].get<json::boolean>();
            m_grabber0Offset = j["grabber0_offset"].get<json::vec2>();
            m_grabber1Offset = j["grabber1_offset"].get<json::vec2>();
            m_bJustAdded = false;
            m_bHovered = false;
            m_iHoverType = -1;
            UpdateGrabberContainBox();
        }

        MatUtils::Point2i GetPos() const override
        {
            return MatUtils::FromImVec2<int32_t>(m_pos);
        }

        MatUtils::Point2i GetBezierGrabberOffset(int idx) const override
        {
            if (idx < 0 || idx > 1)
                return {-1, -1};
            if (idx == 0)
                return MatUtils::FromImVec2<int32_t>(m_grabber0Offset);
            else
                return MatUtils::FromImVec2<int32_t>(m_grabber1Offset);
        }

        int GetHoverType() const override
        {
            return m_iHoverType;
        }

        MatUtils::Recti GetContainBox() const
        {
            return MatUtils::Recti(
                    m_rGrabberContBox.Min.x < m_rContourContBox.Min.x ? m_rGrabberContBox.Min.x : m_rContourContBox.Min.x,
                    m_rGrabberContBox.Min.y < m_rContourContBox.Min.y ? m_rGrabberContBox.Min.y : m_rContourContBox.Min.y,
                    m_rGrabberContBox.Max.x > m_rContourContBox.Max.x ? m_rGrabberContBox.Max.x : m_rContourContBox.Max.x,
                    m_rGrabberContBox.Max.y > m_rContourContBox.Max.y ? m_rGrabberContBox.Max.y : m_rContourContBox.Max.y);
        }

        void UpdateGrabberContainBox()
        {
            const auto& pointSizeHalf = m_owner->m_v2PointSizeHalf;
            const auto& grabberRadiusOutter = m_owner->m_fGrabberRadius;
            ImRect result(m_pos-pointSizeHalf, m_pos+pointSizeHalf);
            if (m_bEnableBezier)
            {
                const auto grabber0Center = m_pos+m_grabber0Offset;
                auto c = grabber0Center.x-grabberRadiusOutter;
                if (c < result.Min.x)
                    result.Min.x = c;
                c = grabber0Center.x+grabberRadiusOutter;
                if (c > result.Max.x)
                    result.Max.x = c;
                c = grabber0Center.y-grabberRadiusOutter;
                if (c < result.Min.y)
                    result.Min.y = c;
                c = grabber0Center.y+grabberRadiusOutter;
                if (c > result.Max.y)
                    result.Max.y = c;
                const auto grabber1Center = m_pos+m_grabber1Offset;
                c = grabber1Center.x-grabberRadiusOutter;
                if (c < result.Min.x)
                    result.Min.x = c;
                c = grabber1Center.x+grabberRadiusOutter;
                if (c > result.Max.x)
                    result.Max.x = c;
                c = grabber1Center.y-grabberRadiusOutter;
                if (c < result.Min.y)
                    result.Min.y = c;
                c = grabber1Center.y+grabberRadiusOutter;
                if (c > result.Max.y)
                    result.Max.y = c;
            }
            m_rGrabberContBox = result;
        }

        bool CheckHovering(const ImVec2& mousePos)
        {
            if (CheckGrabberHovering(mousePos))
                return true;
            return CheckContourHovering(mousePos);
        }

        bool CheckGrabberHovering(const ImVec2& mousePos)
        {
            const ImVec2 szDetectExRadius(m_owner->m_fHoverDetectExRadius, m_owner->m_fHoverDetectExRadius);
            const auto& grabberRadius = m_owner->m_fGrabberRadius;
            if (m_bEnableBezier)
            {
                const ImVec2 radiusSize(grabberRadius, grabberRadius);
                const auto grabber0Center = m_pos+m_grabber0Offset;
                const ImRect grabber0Rect(grabber0Center-radiusSize-szDetectExRadius, grabber0Center+radiusSize+szDetectExRadius);
                if (grabber0Rect.Contains(mousePos))
                {
                    // cout << "--> Grabber-0 hovered" << endl;
                    m_bHovered = true;
                    m_iHoverType = 1;
                    return true;
                }
                const auto grabber1Center = m_pos+m_grabber1Offset;
                const ImRect grabber1Rect(grabber1Center-radiusSize-szDetectExRadius, grabber1Center+radiusSize+szDetectExRadius);
                if (grabber1Rect.Contains(mousePos))
                {
                    // cout << "--> Grabber-1 hovered" << endl;
                    m_bHovered = true;
                    m_iHoverType = 2;
                    return true;
                }
            }
            const auto& pointSizeHalf = m_owner->m_v2PointSizeHalf;
            const ImRect pointRect(m_pos-pointSizeHalf-szDetectExRadius, m_pos+pointSizeHalf+szDetectExRadius);
            if (pointRect.Contains(mousePos))
            {
                m_bHovered = true;
                m_iHoverType = 0;
                return true;
            }
            return false;
        }

        bool CheckContourHovering(const ImVec2& mousePos)
        {
            const auto& contourThickness = m_owner->m_fContourThickness;
            const auto& hoverDetectExtension = m_owner->m_fContourHoverDetectExRadius;
            if (!m_aContourVertices.empty() && m_rContourContBox.Contains(mousePos))
            {
                auto itVt0 = m_aContourVertices.begin();
                auto itVt1 = itVt0; itVt1++;
                int idx = 0;
                while (itVt1 != m_aContourVertices.end())
                {
                    const auto& v0 = *itVt0++;
                    const auto& v1 = *itVt1++;
                    float minX, minY, maxX, maxY;
                    if (v0.x < v1.x) { minX = v0.x; maxX = v1.x; }
                    else { minX = v1.x; maxX = v0.x; }
                    if (v0.y < v1.y) { minY = v0.y; maxY = v1.y; }
                    else { minY = v1.y; maxY = v0.y; }
                    if (minX != maxX && mousePos.x >= minX && mousePos.x <= maxX)
                    {
                        float crossY = (mousePos.x-v0.x)*(v1.y-v0.y)/(v1.x-v0.x)+v0.y;
                        if (abs(crossY-mousePos.y) < contourThickness+hoverDetectExtension)
                        {
                            m_bHovered = true;
                            m_iHoverType = 4;
                            m_v2HoverPointOnContour = {mousePos.x, crossY};
                            // cout << "--> mousePos(" << mousePos.x << ", " << mousePos.y << "), crossY-hoverPoint(" << mousePos.x << ", " << crossY << ")" << endl;
                            m_iHoverPointOnContourIdx = idx;
                            return true;
                        }
                        // else
                        // {
                        //     cout << "[" << i << "] mousePos(" << mousePos.x << ", " << mousePos.y << "), crossY=" << crossY << ", v0(" << v0.x << ", " << v0.y << "), v1(" << v1.x << ", " << v1.y << ")"
                        //             << ", abs(crossY-mousePos.y)=" << abs(crossY-mousePos.y) << endl;
                        // }
                    }
                    if (minY != maxY && mousePos.y >= minY && mousePos.y <= maxY)
                    {
                        float crossX = (mousePos.y-v0.y)*(v1.x-v0.x)/(v1.y-v0.y)+v0.x;
                        if (abs(crossX-mousePos.x) < contourThickness+hoverDetectExtension)
                        {
                            m_bHovered = true;
                            m_iHoverType = 4;
                            m_v2HoverPointOnContour = {crossX, mousePos.y};
                            // cout << "--> mousePos(" << mousePos.x << ", " << mousePos.y << "), crossX-hoverPoint(" << crossX << ", " << mousePos.y << ")" << endl;
                            m_iHoverPointOnContourIdx = idx;
                            return true;
                        }
                        // else
                        // {
                        //     cout << "[" << i << "] mousePos(" << mousePos.x << ", " << mousePos.y << "), crossX=" << crossX << ", v0(" << v0.x << ", " << v0.y << "), v1(" << v1.x << ", " << v1.y << ")"
                        //             << ", abs(crossX-mousePos.x)=" << abs(crossX-mousePos.x) << endl;
                        // }
                    }
                    idx++;
                }
            }
            return false;
        }

        void QuitHover()
        {
            m_bHovered = false;
            m_iHoverType = -1;
        }

        void DrawPoint(ImDrawList* pDrawList, const ImVec2& origin) const
        {
            const auto& v2UiScale = m_owner->m_v2UiScale;
            const auto& pointSizeHalf = m_owner->m_v2PointSizeHalf;
            const auto& pointColor = m_owner->m_u32PointColor;
            const auto& pointBorderThickness = m_owner->m_fPointBorderThickness;
            const auto& pointBorderColor = m_owner->m_u32PointBorderColor;
            const auto& pointBorderHoverColor = m_owner->m_u32PointBorderHoverColor;
            const auto grabberRadiusInner = m_owner->m_fGrabberRadius-m_owner->m_fGrabberBorderThickness;
            const auto& grabberColorInner = m_owner->m_u32GrabberColor;
            const auto& grabberRadiusOutter = m_owner->m_fGrabberRadius;
            const auto& grabberColorOutter = m_owner->m_u32GrabberBorderColor;
            const auto& grabberColorHoverOutter = m_owner->m_u32GrabberBorderHoverColor;
            const auto& lineThickness = m_owner->m_fGrabberLineThickness;
            const auto& lineColor = m_owner->m_u32GrabberLineColor;
            const auto pointPos = origin+m_pos*v2UiScale;
            if (m_bEnableBezier)
            {
                const auto grabber0Center = pointPos+m_grabber0Offset*v2UiScale;
                const auto grabber1Center = pointPos+m_grabber1Offset*v2UiScale;
                pDrawList->AddLine(pointPos, grabber0Center, lineColor, lineThickness);
                pDrawList->AddLine(pointPos, grabber1Center, lineColor, lineThickness);
                auto borderColor = m_bHovered && m_iHoverType==1 ? grabberColorHoverOutter : grabberColorOutter;
                pDrawList->AddCircleFilled(grabber0Center, grabberRadiusOutter, borderColor);
                pDrawList->AddCircleFilled(grabber0Center, grabberRadiusInner, grabberColorInner);
                borderColor = m_bHovered && m_iHoverType==2 ? grabberColorHoverOutter : grabberColorOutter;
                pDrawList->AddCircleFilled(grabber1Center, grabberRadiusOutter, borderColor);
                pDrawList->AddCircleFilled(grabber1Center, grabberRadiusInner, grabberColorInner);
            }
            const auto& offsetSize1 = pointSizeHalf;
            auto borderColor = m_bHovered && m_iHoverType==0 ? pointBorderHoverColor : pointBorderColor;
            pDrawList->AddRectFilled(pointPos-offsetSize1, pointPos+offsetSize1, borderColor);
            const ImVec2 offsetSize2(pointSizeHalf.x-pointBorderThickness, pointSizeHalf.y-pointBorderThickness);
            pDrawList->AddRectFilled(pointPos-offsetSize2, pointPos+offsetSize2, pointColor);
        }

        void CalcContourVertices(const ContourPointImpl& prevVt)
        {
            if (!prevVt.m_bEnableBezier && !m_bEnableBezier)
            {
                m_aContourVertices.clear();
                m_aContourVertices.push_back(prevVt.m_pos);
                m_aContourVertices.push_back(m_pos);
                UpdateContourContianBox();
                return;
            }

            ImVec2 bzVts[] = {prevVt.m_pos, prevVt.m_grabber1Offset, m_grabber0Offset, m_pos};
            m_aContourVertices = CalcBezierCurve(bzVts);
            UpdateContourContianBox();
        }

        void UpdateContourContianBox()
        {
            const auto contourHoverRadius = m_owner->m_fContourThickness+m_owner->m_fContourHoverDetectExRadius;
            if (m_aContourVertices.empty())
            {
                m_rContourContBox = {-1, -1, -1, -1};
                return;
            }
            auto iter = m_aContourVertices.begin();
            auto& firstVertex = *iter++;
            ImRect rContourContBox = { firstVertex.x-contourHoverRadius, firstVertex.y-contourHoverRadius, firstVertex.x+contourHoverRadius, firstVertex.y+contourHoverRadius };
            while (iter != m_aContourVertices.end())
            {
                auto& v = *iter++;
                if (v.x-contourHoverRadius < rContourContBox.Min.x)
                    rContourContBox.Min.x = v.x-contourHoverRadius;
                if (v.y-contourHoverRadius < rContourContBox.Min.y)
                    rContourContBox.Min.y = v.y-contourHoverRadius;
                if (v.x+contourHoverRadius > rContourContBox.Max.x)
                    rContourContBox.Max.x = v.x+contourHoverRadius;
                if (v.y+contourHoverRadius > rContourContBox.Max.y)
                    rContourContBox.Max.y = v.y+contourHoverRadius;
            }
            m_rContourContBox = rContourContBox;
        }

        void DrawContour(ImDrawList* pDrawList, const ImVec2& origin, const ContourPointImpl& prevVt) const
        {
            const auto& v2UiScale = m_owner->m_v2UiScale;
            const auto& contourColor = m_owner->m_u32ContourColor;
            const auto& contourThickness = m_owner->m_fContourThickness;
            if (!m_aContourVertices.empty())
            {
                // const int smoothness = m_aContourVertices.size()-1;
                auto itVt0 = m_aContourVertices.begin();
                auto itVt1 = itVt0; itVt1++;
                while (itVt1 != m_aContourVertices.end())
                {
                    const auto v0 = origin+(*itVt0++)*v2UiScale;
                    const auto v1 = origin+(*itVt1++)*v2UiScale;
                    pDrawList->AddLine(v0, v1, contourColor, contourThickness);
                }
            }
            else
            {
                const auto v0 = origin+prevVt.m_pos*v2UiScale;
                const auto v1 = origin+       m_pos*v2UiScale;
                pDrawList->AddLine(v0, v1, contourColor, contourThickness);
            }
        }
    };

    struct MorphController
    {
        MaskCreatorImpl* m_owner;
        ImVec2 m_ptRootPos;
        ImVec2 m_ptGrabberPos;
        bool m_bInsidePoly{false};
        float m_fCtrlSlope;
        float m_fMorphCtrlLength;
        float m_fDistant{0.f};
        int m_iMorphIterations{0};
        ImVec2 m_ptFeatherGrabberPos;
        float m_fFeatherCtrlLength{0};
        int m_iFeatherIterations{0};
        bool m_bIsHovered{false};
        int m_iHoverType{-1};

        static const int MIN_CTRL_LENGTH{30};

        MorphController(MaskCreatorImpl* owner)
            : m_owner(owner), m_ptRootPos({-1, -1}), m_fCtrlSlope(0)
            , m_fMorphCtrlLength(MIN_CTRL_LENGTH)
        {}

        json::value ToJson() const
        {
            json::value j;
            j["ctrl_lendth"] = json::number(m_fMorphCtrlLength);
            j["iterations"] = json::number(m_iMorphIterations);
            j["inside_polygon"] = m_bInsidePoly;
            return std::move(j);
        }

        void FromJson(const json::value& j)
        {
            m_fMorphCtrlLength = j["ctrl_lendth"].get<json::number>();
            m_iMorphIterations = j["iterations"].get<json::number>();
            m_bInsidePoly = j["inside_polygon"].get<json::boolean>();
        }

        void Reset()
        {
            m_ptRootPos = {-1, -1};
            m_fCtrlSlope = 0;
            m_fDistant = 0;
            m_bIsHovered = false;
            m_iHoverType = -1;
        }

        list<ContourPointImpl>::iterator SetPosAndSlope(ImVec2& ptRootPos, float fCtrlSlope, int iLineIdx)
        {
            auto itCv = m_owner->m_av2AllContourVertices.begin();
            auto itCp = m_owner->m_atContourPoints.begin(); itCp++;
            auto itCpSub1 = itCp->m_aContourVertices.begin();
            auto itCpSub2 = itCpSub1; itCpSub2++;
            int idx = 0;
            while (itCv != m_owner->m_av2AllContourVertices.end() && idx < iLineIdx)
            {
                idx++; itCv++;
                itCpSub1++; itCpSub2++;
                if (itCpSub2 == itCp->m_aContourVertices.end())
                {
                    itCp++; if (itCp == m_owner->m_atContourPoints.end()) itCp = m_owner->m_atContourPoints.begin();
                    itCpSub1 = itCp->m_aContourVertices.begin();
                    itCpSub2 = itCpSub1; itCpSub2++;
                }
            }
            assert(*itCv == *itCpSub1);

            auto v2Dist = CalcRootPosDist(*itCp, itCpSub1, ptRootPos);
            const auto fMinDist = m_owner->m_v2PointSizeHalf.x+m_owner->m_fHoverDetectExRadius;
            if (v2Dist.x+v2Dist.y < fMinDist*2)
            {
                auto itSearchCp = itCp;
                itSearchCp++; if (itSearchCp == m_owner->m_atContourPoints.end()) itSearchCp = m_owner->m_atContourPoints.begin();
                ImVec2 ptSearchRootPos, v2SearchDist;
                float fSearchCtrlSlope;
                bool bFoundCp = false;
                while (itSearchCp != itCp)
                {
                    v2SearchDist = CalcRootPosDist(*itSearchCp, fMinDist, ptSearchRootPos, fSearchCtrlSlope);
                    if (v2SearchDist.x+v2SearchDist.y >= fMinDist*2)
                    {
                        bFoundCp = true;
                        break;
                    }
                    itSearchCp++; if (itSearchCp == m_owner->m_atContourPoints.end()) itSearchCp = m_owner->m_atContourPoints.begin();
                }
                if (bFoundCp)
                {
                    ptRootPos = ptSearchRootPos;
                    fCtrlSlope = fSearchCtrlSlope;
                    itCp = itSearchCp;
                    v2Dist = v2SearchDist;
                }
            }
            else if (v2Dist.y < fMinDist)
            {
                auto fTargetDist = v2Dist.x+v2Dist.y-fMinDist;
                v2Dist = CalcRootPosDist(*itCp, fTargetDist, ptRootPos, fCtrlSlope);
            }
            else if (v2Dist.x < fMinDist)
            {
                v2Dist = CalcRootPosDist(*itCp, fMinDist, ptRootPos, fCtrlSlope);
            }

            m_ptRootPos = ptRootPos;
            m_fCtrlSlope = fCtrlSlope;
            m_fDistant =  v2Dist.x;
            CalcGrabberPos();
            return itCp;
        }

        list<ContourPointImpl>::iterator SetPosAndSlope(list<ContourPointImpl>::iterator itCp, float fTargetDist)
        {
            ImVec2 ptRootPos, v2Dist;
            float fCtrlSlope;
            bool bFoundCp = false;
            const auto fMinDist = m_owner->m_v2PointSizeHalf.x+m_owner->m_fHoverDetectExRadius;
            auto fTargetDist2 = fTargetDist < fMinDist ? fMinDist : fTargetDist;
            v2Dist = CalcRootPosDist(*itCp, fTargetDist2, ptRootPos, fCtrlSlope);
            if (v2Dist.x+v2Dist.y < fMinDist*2)
            {
                auto itSearchCp = itCp;
                itSearchCp++; if (itSearchCp == m_owner->m_atContourPoints.end()) itSearchCp = m_owner->m_atContourPoints.begin();
                ImVec2 ptSearchRootPos, v2SearchDist;
                float fSearchCtrlSlope;
                while (itSearchCp != itCp)
                {
                    v2SearchDist = CalcRootPosDist(*itSearchCp, fMinDist, ptSearchRootPos, fSearchCtrlSlope);
                    if (v2SearchDist.x+v2SearchDist.y >= fMinDist*2)
                    {
                        bFoundCp = true;
                        break;
                    }
                    itSearchCp++; if (itSearchCp == m_owner->m_atContourPoints.end()) itSearchCp = m_owner->m_atContourPoints.begin();
                }
                if (bFoundCp)
                {
                    ptRootPos = ptSearchRootPos;
                    fCtrlSlope = fSearchCtrlSlope;
                    itCp = itSearchCp;
                    v2Dist = v2SearchDist;
                }
            }
            else
            {
                bFoundCp = true;
                if (fTargetDist < fMinDist)
                {
                    fTargetDist2 = (v2Dist.x+v2Dist.y)/2;
                    v2Dist = CalcRootPosDist(*itCp, fTargetDist2, ptRootPos, fCtrlSlope);
                }
                else if (v2Dist.y < fMinDist)
                {
                    fTargetDist2 = v2Dist.x+v2Dist.y-fMinDist;
                    v2Dist = CalcRootPosDist(*itCp, fTargetDist2, ptRootPos, fCtrlSlope);
                }
            }

            m_ptRootPos = ptRootPos;
            m_fCtrlSlope = fCtrlSlope;
            m_fDistant = v2Dist.x;
            CalcGrabberPos();
            return itCp;
        }

        ImVec2 CalcRootPosDist(const ContourPointImpl& cp, list<ImVec2>::iterator itCpSubTarget, const ImVec2& ptRootPos) const 
        {
            float fDist1 = 0, fDist2 = 0;
            auto itCpSub2 = cp.m_aContourVertices.begin();
            auto itCpSub1 = cp.m_aContourVertices.end();
            while (itCpSub2 != itCpSubTarget)
            {
                if (itCpSub1 != cp.m_aContourVertices.end())
                {
                    const auto dx = itCpSub2->x-itCpSub1->x;
                    const auto dy = itCpSub2->y-itCpSub1->y;
                    fDist1 += sqrt(dx*dx+dy*dy);
                }
                itCpSub1 = itCpSub2++;
            }
            {
                const auto dx = ptRootPos.x-itCpSub2->x;
                const auto dy = ptRootPos.y-itCpSub2->y;
                fDist1 += sqrt(dx*dx+dy*dy);
            }
            {
                itCpSub2++;
                const auto dx = ptRootPos.x-itCpSub2->x;
                const auto dy = ptRootPos.y-itCpSub2->y;
                fDist2 = sqrt(dx*dx+dy*dy);
                itCpSub1 = itCpSub2++;
            }
            while (itCpSub2 != cp.m_aContourVertices.end())
            {
                const auto dx = itCpSub2->x-itCpSub1->x;
                const auto dy = itCpSub2->y-itCpSub1->y;
                fDist2 += sqrt(dx*dx+dy*dy);
                itCpSub1 = itCpSub2++;
            }
            return ImVec2(fDist1, fDist2);
        }

        ImVec2 CalcRootPosDist(const ContourPointImpl& cp, float fTargetDist1, ImVec2& ptRootPos, float& fVertSlope) const
        {
            float fDist1 = 0, fDist2 = 0;
            auto itCpSub2 = cp.m_aContourVertices.begin();
            auto itCpSub1 = cp.m_aContourVertices.end();
            while (itCpSub2 != cp.m_aContourVertices.end())
            {
                if (itCpSub1 != cp.m_aContourVertices.end())
                {
                    const auto dx = itCpSub2->x-itCpSub1->x;
                    const auto dy = itCpSub2->y-itCpSub1->y;
                    const auto l = sqrt(dx*dx+dy*dy);
                    if (fDist1+l <= fTargetDist1)
                        fDist1 += l;
                    else
                    {
                        fDist2 = fDist1+l-fTargetDist1;
                        fDist1 = fTargetDist1;
                        const auto fSlope = itCpSub1->x == itCpSub2->x ? numeric_limits<float>::infinity() : (itCpSub1->y-itCpSub2->y)/(itCpSub1->x-itCpSub2->x);
                        if (isinf(fSlope))
                        {
                            ptRootPos.x = itCpSub1->x;
                            ptRootPos.y = itCpSub2->y+(itCpSub2->y > itCpSub1->y ? -fDist2 : fDist2);
                            fVertSlope = 0;
                        }
                        else
                        {
                            const auto u = 1/sqrt(fSlope*fSlope+1);
                            const auto d = itCpSub1->x < itCpSub2->x ? -fDist2 : fDist2;
                            ptRootPos.x = itCpSub2->x+d*u;
                            ptRootPos.y = itCpSub2->y+d*u*fSlope;
                            fVertSlope = fSlope == 0 ? numeric_limits<float>::infinity() : -1/fSlope;
                        }
                        break;
                    }
                }
                itCpSub1 = itCpSub2++;
            }
            if (itCpSub2 != cp.m_aContourVertices.end())
                itCpSub1 = itCpSub2++;
            while (itCpSub2 != cp.m_aContourVertices.end())
            {
                const auto dx = itCpSub2->x-itCpSub1->x;
                const auto dy = itCpSub2->y-itCpSub1->y;
                fDist2 += sqrt(dx*dx+dy*dy);
                itCpSub1 = itCpSub2++;
            }
            return ImVec2(fDist1, fDist2);
        }

        void CalcGrabberPos()
        {
            float x, y;
            const auto ratio = 1/sqrt(m_fCtrlSlope*m_fCtrlSlope+1);
            if (isinf(m_fCtrlSlope))
            {
                x = m_ptRootPos.x;
                y = m_ptRootPos.y+1;
            }
            else
            {
                x = m_ptRootPos.x+ratio;
                y = m_ptRootPos.y+ratio*m_fCtrlSlope;
            }
            bool bIsInside = MatUtils::CheckPointInsidePolygon({x, y}, m_owner->m_av2AllContourVertices);
            float fLength = m_fMorphCtrlLength;
            if (m_bInsidePoly^bIsInside)
                fLength = -fLength;
            if (isinf(m_fCtrlSlope))
            {
                x = m_ptRootPos.x;
                y = m_ptRootPos.y+fLength;
            }
            else
            {
                const auto u = fLength*ratio;
                x = m_ptRootPos.x+u;
                y = m_ptRootPos.y+u*m_fCtrlSlope;
            }
            m_ptGrabberPos = {x, y};
            const auto fFeatherGrabberDistance = m_fFeatherCtrlLength+m_owner->m_fGrabberRadius*2+m_owner->m_fHoverDetectExRadius;
            fLength = m_bInsidePoly^bIsInside ? fLength-fFeatherGrabberDistance : fLength+fFeatherGrabberDistance;
            if (isinf(m_fCtrlSlope))
            {
                x = m_ptRootPos.x;
                y = m_ptRootPos.y+fLength;
            }
            else
            {
                const auto u = fLength*ratio;
                x = m_ptRootPos.x+u;
                y = m_ptRootPos.y+u*m_fCtrlSlope;
            }
            m_ptFeatherGrabberPos = {x, y};
        }

        bool CheckHovering(const ImVec2& mousePos)
        {
            const ImVec2 szDetectExRadius(m_owner->m_fHoverDetectExRadius, m_owner->m_fHoverDetectExRadius);
            const auto& pointSizeHalf = m_owner->m_v2PointSizeHalf;
            const auto& grabberRadius = m_owner->m_fGrabberRadius;
            const ImRect rootPosRect(m_ptRootPos-pointSizeHalf-szDetectExRadius, m_ptRootPos+pointSizeHalf+szDetectExRadius);
            if (rootPosRect.Contains(mousePos))
            {
                m_bIsHovered = true;
                m_iHoverType = 0;
                return true;
            }
            const ImVec2 radiusSize(grabberRadius, grabberRadius);
            ImRect grabberRect(m_ptGrabberPos-radiusSize-szDetectExRadius, m_ptGrabberPos+radiusSize+szDetectExRadius);
            if (grabberRect.Contains(mousePos))
            {
                m_bIsHovered = true;
                m_iHoverType = 1;
                return true;
            }
            grabberRect = {m_ptFeatherGrabberPos-radiusSize-szDetectExRadius, m_ptFeatherGrabberPos+radiusSize+szDetectExRadius};
            if (grabberRect.Contains(mousePos))
            {
                m_bIsHovered = true;
                m_iHoverType = 2;
                return true;
            }
            m_bIsHovered = false;
            m_iHoverType = -1;
            return false;
        }

        void QuitHover()
        {
            m_bIsHovered = false;
        }

        void Draw(ImDrawList* pDrawList, const ImVec2& origin) const
        {
            const auto& v2UiScale = m_owner->m_v2UiScale;
            const ImVec2 rootPos = origin+m_ptRootPos*v2UiScale;
            ImVec2 grabberPos = origin+m_ptFeatherGrabberPos*v2UiScale;
            pDrawList->AddLine(rootPos, grabberPos, m_owner->m_u32FeatherGrabberColor, m_owner->m_fContourThickness);
            grabberPos = origin+m_ptGrabberPos*v2UiScale;
            pDrawList->AddLine(rootPos, grabberPos, m_owner->m_u32ContourColor, m_owner->m_fContourThickness);
            const auto& offsetSize1 = m_owner->m_v2PointSizeHalf;
            auto borderColor = m_iHoverType == 0 ? m_owner->m_u32PointBorderHoverColor : m_owner->m_u32PointBorderColor;
            pDrawList->AddRectFilled(rootPos-offsetSize1, rootPos+offsetSize1, borderColor);
            const ImVec2 offsetSize2(m_owner->m_v2PointSizeHalf.x-m_owner->m_fPointBorderThickness, m_owner->m_v2PointSizeHalf.y-m_owner->m_fPointBorderThickness);
            pDrawList->AddRectFilled(rootPos-offsetSize2, rootPos+offsetSize2, m_owner->m_u32ContourColor);
            borderColor = m_iHoverType == 1 ? m_owner->m_u32GrabberBorderHoverColor : m_owner->m_u32GrabberBorderColor;
            pDrawList->AddCircleFilled(grabberPos, m_owner->m_fGrabberRadius, borderColor);
            pDrawList->AddCircleFilled(grabberPos, m_owner->m_fGrabberRadius-m_owner->m_fGrabberBorderThickness, m_owner->m_u32GrabberColor);
            grabberPos = origin+m_ptFeatherGrabberPos*v2UiScale;
            borderColor = m_iHoverType == 2 ? m_owner->m_u32GrabberBorderHoverColor : m_owner->m_u32GrabberBorderColor;
            pDrawList->AddCircleFilled(grabberPos, m_owner->m_fGrabberRadius, borderColor);
            pDrawList->AddCircleFilled(grabberPos, m_owner->m_fGrabberRadius-m_owner->m_fGrabberBorderThickness, m_owner->m_u32FeatherGrabberColor);
        }
    };

    struct BezierTable
    {
        using Holder = shared_ptr<BezierTable>;

        int m_steps;
        float* m_C;
        chrono::time_point<chrono::system_clock> m_tpLastRefTime;

        BezierTable(int steps) : m_steps(steps)
        {
            m_C = new float[(steps+1)*4];
            for (int step = 0; step <= steps; ++step)
            {
                float t = (float)step/(float)steps;
                m_C[step*4+0] = (1-t)*(1-t)*(1-t);
                m_C[step*4+1] = 3*(1-t)*(1-t)*t;
                m_C[step*4+2] = 3*(1-t)*t*t;
                m_C[step*4+3] = t*t*t;
            }
            m_tpLastRefTime = chrono::system_clock::now();
        }

        ~BezierTable()
        {
            if (m_steps)
                delete [] m_C;
        }

        const float* GetBezierConstant()
        {
            m_tpLastRefTime = chrono::system_clock::now();
            return m_C;
        }

        template <class Rep, class Period>
        bool NotUsedInTime(const chrono::time_point<chrono::system_clock>& tpNow, const chrono::duration<Rep, Period>& dur) const
        {return tpNow-m_tpLastRefTime > dur; }

        const chrono::time_point<chrono::system_clock>& LastRefTime() const
        { return m_tpLastRefTime; }
    };

    static unordered_map<int, BezierTable::Holder> s_mapBezierTables;
    static int s_iKeepBezierTableCountMin;
    static chrono::seconds s_iKeepBezierTableTimeOut;

    static BezierTable::Holder GetBezierTable(int steps)
    {
        BezierTable::Holder hRes;
        auto iter = s_mapBezierTables.find(steps);
        if (iter != s_mapBezierTables.end())
            hRes = iter->second;
        else
        {
            hRes = BezierTable::Holder(new BezierTable(steps));
            s_mapBezierTables[steps] = hRes;
        }

        if (s_mapBezierTables.size() > s_iKeepBezierTableCountMin)
        {
            list<BezierTable::Holder> refTimePriList;
            for (auto& elem : s_mapBezierTables)
                refTimePriList.push_back(elem.second);
            refTimePriList.sort([] (auto& a, auto& b) {
                return a->LastRefTime() < b->LastRefTime();
            });

            auto tpNow = chrono::system_clock::now();
            auto iter = refTimePriList.begin();
            while (iter != refTimePriList.end() && s_mapBezierTables.size() > s_iKeepBezierTableCountMin)
            {
                auto hBt = *iter++;
                if (hBt->NotUsedInTime(tpNow, s_iKeepBezierTableTimeOut))
                    s_mapBezierTables.erase(hBt->m_steps);
            }
        }
        return hRes;
    }

    static list<ImVec2> CalcBezierCurve(ImVec2 v[4])
    {
        auto offsetVt = v[3]-v[0];
        const float distance = sqrt((double)offsetVt.x*offsetVt.x+(double)offsetVt.y*offsetVt.y);
        int log2value = (int)floor(log2(distance));
        if (log2value <= 0)
            return {v[0], v[3]};

        int steps = 1 << (log2value-1);
        auto hBt = GetBezierTable(steps);
        auto C = hBt->GetBezierConstant();
        static const float TRANS_FACTOR = 1e2/sqrt(2);
        static const float INV_TRANS_FACTOR = 1e-2/sqrt(2);
        bool bNeedInvTrans = false;
        if (abs(offsetVt.x) < 1e-2 || abs(offsetVt.y) < 1e-2)
        {
            ImVec2 tmp;
            tmp.x = v[1].x*TRANS_FACTOR-v[1].y*TRANS_FACTOR;
            tmp.y = v[1].x*TRANS_FACTOR+v[1].y*TRANS_FACTOR;
            v[1] = tmp;
            tmp.x = (offsetVt.x+v[2].x)*TRANS_FACTOR-(offsetVt.y+v[2].y)*TRANS_FACTOR;
            tmp.y = (offsetVt.x+v[2].x)*TRANS_FACTOR+(offsetVt.y+v[2].y)*TRANS_FACTOR;
            v[2] = tmp;
            tmp.x = offsetVt.x*TRANS_FACTOR-offsetVt.y*TRANS_FACTOR;
            tmp.y = offsetVt.x*TRANS_FACTOR+offsetVt.y*TRANS_FACTOR;
            offsetVt = tmp;
            v[2].x -= tmp.x;
            v[2].y -= tmp.y;
            bNeedInvTrans = true;
        }
        ImVec2 P[] = {
            { 0.f, 0.f },
            { v[1].x/offsetVt.x, v[1].y/offsetVt.y },
            { 1+v[2].x/offsetVt.x, 1+v[2].y/offsetVt.y },
            { 1.f, 1.f } };
        vector<ImVec2> v2BezierTable(steps+1);
        for (int step = 0; step <= steps; ++step)
        {
            ImVec2 point = {
                C[step*4+0]*P[0].x+C[step*4+1]*P[1].x+C[step*4+2]*P[2].x+C[step*4+3]*P[3].x,
                C[step*4+0]*P[0].y+C[step*4+1]*P[1].y+C[step*4+2]*P[2].y+C[step*4+3]*P[3].y
            };
            v2BezierTable[step] = point;
        }

        const int smoothness = v2BezierTable.size();
        list<ImVec2> res;
        if (bNeedInvTrans)
        {
            for (int i = 0; i < smoothness; i++)
            {
                const auto& p = v2BezierTable[i];
                const ImVec2 tmp(p.x*offsetVt.x, p.y*offsetVt.y);
                const float dx = tmp.x*INV_TRANS_FACTOR+tmp.y*INV_TRANS_FACTOR;
                const float dy = -tmp.x*INV_TRANS_FACTOR+tmp.y*INV_TRANS_FACTOR;
                res.push_back(ImVec2(dx+v[0].x, dy+v[0].y));
            }
        }
        else
        {
            for (int i = 0; i < smoothness; i++)
            {
                const auto& p = v2BezierTable[i];
                res.push_back(ImVec2(p.x*offsetVt.x+v[0].x, p.y*offsetVt.y+v[0].y));
            }
        }
        return res;
    }

    static list<ImVec2> CalcBezierConnection(const ImVec2 v[4])
    {
        const double fSlope1 = v[1].x != v[0].x ? ((double)v[1].y-v[0].y)/((double)v[1].x-v[0].x) : numeric_limits<double>::infinity();
        const double fSlope2 = v[2].x != v[3].x ? ((double)v[2].y-v[3].y)/((double)v[2].x-v[3].x) : numeric_limits<double>::infinity();
        const auto vec1 = v[0]-v[1];
        const auto vec2 = v[3]-v[2];
        const auto dotVec12 = vec1.x*vec2.x+vec1.y*vec2.y;
        const auto dotNum = dotVec12*dotVec12;
        const auto dotDen = (vec1.x*vec1.x+vec1.y*vec1.y)*(vec2.x*vec2.x+vec2.y*vec2.y);

        const auto vOffset = v[2]-v[1];
        const double scale = dotVec12 > 0 ? 0.5*(1+dotNum/(dotDen*2)) : 0.5;
        const double fMorphLen = sqrt((double)vOffset.x*vOffset.x+(double)vOffset.y*vOffset.y)*scale;
        float dx1, dy1;
        if (isinf(fSlope1))
        {
            dx1 = 0;
            dy1 = v[1].y > v[0].y ? fMorphLen : -fMorphLen;
        }
        else
        {
            const double ratio = 1/sqrt(1+fSlope1*fSlope1);
            dx1 = v[1].x > v[0].x ? fMorphLen*ratio : -fMorphLen*ratio;
            dy1 = dx1*fSlope1;
        }
        float dx2, dy2;
        if (isinf(fSlope2))
        {
            dx2 = 0;
            dy2 = v[2].y > v[3].y ? fMorphLen : -fMorphLen;
        }
        else
        {
            const double ratio = 1/sqrt(1+fSlope2*fSlope2);
            dx2 = v[2].x > v[3].x ? fMorphLen*ratio : -fMorphLen*ratio;
            dy2 = dx2*fSlope2;
        }
        ImVec2 v2[] = {v[1], {dx1, dy1}, {dx2, dy2}, v[2]};
        return CalcBezierCurve(v2);
    }

private:
    void DrawPoint(ImDrawList* pDrawList, const ContourPointImpl& v) const
    {
        const auto& origin = m_rWorkArea.Min;
        v.DrawPoint(pDrawList, origin);
    }

    void DrawContour(ImDrawList* pDrawList, const ContourPointImpl& v0, const ContourPointImpl& v1) const
    {
        const auto& origin = m_rWorkArea.Min;
        v1.DrawContour(pDrawList, origin, v0);
    }

    void DrawMorphController(ImDrawList* pDrawList) const
    {
        const auto& origin = m_rWorkArea.Min;
        m_tMorphCtrl.Draw(pDrawList, origin);
    }

    void UpdateContainBox()
    {
        if (m_atContourPoints.size() < 2)
        {
            m_rContianBox = {-1, -1, -1, -1};
            return;
        }
        auto it = m_atContourPoints.begin();
        it++;
        m_rContianBox = it->m_rContourContBox;
        it++;
        while (it != m_atContourPoints.end())
        {
            const auto& r = it->m_rContourContBox;
            if (r.Min.x < m_rContianBox.Min.x)
                m_rContianBox.Min.x = r.Min.x;
            if (r.Max.x > m_rContianBox.Max.x)
                m_rContianBox.Max.x = r.Max.x;
            if (r.Min.y < m_rContianBox.Min.y)
                m_rContianBox.Min.y = r.Min.y;
            if (r.Max.y > m_rContianBox.Max.y)
                m_rContianBox.Max.y = r.Max.y;
            it++;
        }
        if (m_bContourCompleted)
        {
            const auto& r = m_atContourPoints.front().m_rContourContBox;
            if (r.Min.x < m_rContianBox.Min.x)
                m_rContianBox.Min.x = r.Min.x;
            if (r.Max.x > m_rContianBox.Max.x)
                m_rContianBox.Max.x = r.Max.x;
            if (r.Min.y < m_rContianBox.Min.y)
                m_rContianBox.Min.y = r.Min.y;
            if (r.Max.y > m_rContianBox.Max.y)
                m_rContianBox.Max.y = r.Max.y;
        }
        m_ptCenter.x = (m_rContianBox.Min.x+m_rContianBox.Max.x)/2;
        m_ptCenter.y = (m_rContianBox.Min.y+m_rContianBox.Max.y)/2;
    }

    void UpdateContourVertices(list<ContourPointImpl>::iterator iterCurrVt)
    {
        if (m_bContourCompleted && iterCurrVt == m_atContourPoints.begin())
        {
            iterCurrVt->CalcContourVertices(m_atContourPoints.back());
        }
        else if (iterCurrVt != m_atContourPoints.begin())
        {
            auto iterPrevVt = iterCurrVt;
            iterPrevVt--;
            iterCurrVt->CalcContourVertices(*iterPrevVt);
        }

        auto iterNextVt = iterCurrVt;
        iterNextVt++;
        if (m_bContourCompleted && iterNextVt == m_atContourPoints.end())
        {
            iterNextVt = m_atContourPoints.begin();
        }
        if (iterNextVt != m_atContourPoints.end())
        {
            iterNextVt->CalcContourVertices(*iterCurrVt);
        }

        if (m_bContourCompleted)
        {
            // refresh all-contour-vertex list
            m_av2AllContourVertices.clear();
            if (m_atContourPoints.size() > 1)
            {
                bool b1stVertex = true;
                auto it1 = m_atContourPoints.begin();
                do
                {
                    it1++; if (it1 == m_atContourPoints.end()) it1 = m_atContourPoints.begin();
                    const auto& v = *it1;
                    if (!v.m_aContourVertices.empty())
                    {
                        auto it2 = v.m_aContourVertices.begin();
                        auto it3 = it2; it3++;
                        while (it3 != v.m_aContourVertices.end())
                        {
                            m_av2AllContourVertices.push_back(*it2++);
                            it3++;
                        }
                    }
                } while (it1 != m_atContourPoints.begin());
            }

            CalcMorphCtrlPos();
        }

        UpdateContainBox();
        m_bContourChanged = true;
    }

    void CalcMorphCtrlPos()
    {
        auto iterVt = m_itMorphCtrlVt;
        if (iterVt == m_atContourPoints.end() || iterVt->m_aContourVertices.size() < 2)
        {
            m_tMorphCtrl.Reset();
            m_itMorphCtrlVt = m_atContourPoints.end();
            return;
        }
        // int idx1 = iterVt->m_aContourVertices.size()/2;
        // int idx0 = idx1-1;
        // const auto& pt0 = iterVt->m_aContourVertices[idx0];
        // const auto& pt1 = iterVt->m_aContourVertices[idx1];
        // const ImVec2 ptRootPos((pt0.x+pt1.x)/2, (pt0.y+pt1.y)/2);
        // const float slope = pt0.y == pt1.y ? numeric_limits<float>::infinity() : -(pt1.x-pt0.x)/(pt1.y-pt0.y);
        m_itMorphCtrlVt = m_tMorphCtrl.SetPosAndSlope(iterVt, m_tMorphCtrl.m_fDistant);
        // m_itMorphCtrlVt = iterVt;
    }

    bool HasHoveredVertex() const
    {
        return m_itHoveredVertex != m_atContourPoints.end();
    }

    bool HasHoveredContour() const
    {
        return m_itHoveredVertex != m_atContourPoints.end() && m_itHoveredVertex->m_iHoverType == 4;
    }

    bool HasHoveredMorphCtrl() const
    {
        return m_itMorphCtrlVt != m_atContourPoints.end() && m_tMorphCtrl.m_bIsHovered;
    }

    bool HasHoveredSomething() const
    {
        return HasHoveredVertex() || HasHoveredMorphCtrl();
    }

    bool IsMorphCtrlShown() const
    {
        return m_itMorphCtrlVt != m_atContourPoints.end();
    }

    MatUtils::Point2f CalcEdgeVerticalPoint(const ImVec2& v2Root, double fVertSlope, float fLen, bool bInside)
    {
        double dx, dy;
        const double ratio = 1/sqrt(1+fVertSlope*fVertSlope);
        const double fTestUnit = fLen > 0 ? 1e-1 : -1e-1;
        if (isinf(fVertSlope))
        {
            dx = 0;
            dy = fTestUnit;
        }
        else
        {
            dx = fTestUnit*ratio;
            dy = dx*fVertSlope;
        }
        const bool bIsTestUnitInside = MatUtils::CheckPointInsidePolygon(ImVec2(v2Root.x+dx, v2Root.y+dy), m_av2AllContourVertices);
        if (bIsTestUnitInside^bInside)
            fLen = -fLen;
        if (isinf(fVertSlope))
        {
            dx = 0;
            dy = fLen;
        }
        else
        {
            dx = fLen*ratio;
            dy = dx*fVertSlope;
        }
        return MatUtils::Point2f(v2Root.x+dx, v2Root.y+dy);
    }

    ImVec2 CalcEdgeVerticalPoint(const ImVec2& v2Start, const ImVec2& v2End, float fLen, bool bLeft)
    {
        const double fVertSlope = v2Start.y != v2End.y ? ((double)v2Start.x-(double)v2End.x)/((double)v2End.y-(double)v2Start.y) : numeric_limits<double>::infinity();
        const double ratio = 1/sqrt(1+fVertSlope*fVertSlope);
        float dx, dy;
        if (isinf(fVertSlope))
        {
            dx = 0;
            dy = bLeft ^ (v2End.x > v2Start.x) ? fLen : -fLen;
        }
        else if (fVertSlope == 0)
        {
            dx = bLeft ^ (v2End.y < v2Start.y) ? fLen : -fLen;
            dy = 0;
        }
        else
        {
            if ((v2End.x > v2Start.x) ^ (bLeft ^ (fVertSlope > 0)))
                fLen = -fLen;
            dx = fLen*ratio;
            dy = dx*fVertSlope;
        }
        return ImVec2(v2End.x+dx, v2End.y+dy);
    }

    void CalcTwoCrossPoints(const ImVec2& v0, const ImVec2& v1, const ImVec2& v2, float fParaDist, ImVec2& cp1, ImVec2& cp2)
    {
        // for l1 (v0->v1), calculate two parallel lines that has distance 'iDilateSize', as l1p1 (on the left side) and l1p2 (on the right side)
        const double A1 = (double)v1.y-v0.y;
        const double B1 = (double)v0.x-v1.x;
        const double C1 = (double)v1.x*v0.y-(double)v0.x*v1.y;
        const double D1 = fParaDist*sqrt(A1*A1+B1*B1);
        double C1p1, C1p2;  // l1p1: A1*x+B1*y+C1p1=0, l1p2: A1*x+B1*y+C1p2=0
        if (v1.x == v0.x)
        {
            C1p1 = C1-D1;
            C1p2 = C1+D1;
        }
        else
        {
            C1p1 = C1-D1;
            C1p2 = C1+D1;
        }
        // for l2 (v1->v2), calculate two parallel lines that has distance 'iDilateSize', as l2p1 (on the left side) and l2p2 (on the right side)
        const double A2 = v2.y-v1.y;
        const double B2 = v1.x-v2.x;
        const double C2 = v2.x*v1.y-v1.x*v2.y;
        const double D2 = fParaDist*sqrt(A2*A2+B2*B2);
        double C2p1, C2p2;  // l2p1: A2*x+B2*y+C2p1=0, l2p2: A2*x+B2*y+C2p2=0
        if (v2.x == v1.x)
        {
            C2p1 = C2-D2;
            C2p2 = C2+D2;
        }
        else
        {
            C2p1 = C2-D2;
            C2p2 = C2+D2;
        }
        // calculate two cross points: l1p1 X l2p1, and l1p2 X l2p2. One of these two points is outside of contour
        const double den = A1*B2-A2*B1;
        cp1.x = (B1*C2p1-B2*C1p1)/den;
        cp1.y = (A2*C1p1-A1*C2p1)/den;
        cp2.x = (B1*C2p2-B2*C1p2)/den;
        cp2.y = (A2*C1p2-A1*C2p2)/den;
    }

    bool CheckPointValidOnDilateContour(const ImVec2& v, float fDilateSize)
    {
        float fDistToOrgContour;
        MatUtils::CalcNearestPointOnPloygon(v, m_av2AllContourVertices, nullptr, nullptr, &fDistToOrgContour);
        bool bIsInside = MatUtils::CheckPointInsidePolygon(v, m_av2AllContourVertices);
        return fDistToOrgContour >= fDilateSize-1e-2 && !bIsInside;
    }

    vector<MatUtils::Point2f> DilateContour(int iDilateSize)
    {
        if (iDilateSize <= 0)
            return ConvertPointsType(m_av2AllContourVertices);

        list<ImVec2> aVts;
        auto itCv = m_av2AllContourVertices.begin();
        auto itCvNext = itCv; itCvNext++;
        const float fTestLen = 1e-1;
        bool bIs1stVt = true, bPreV2Inside;
        while (itCv != m_av2AllContourVertices.end())
        {
            const auto& v1 = *itCv;
            const auto& v2 = *itCvNext;

            bool bV1VertLeftInside = bPreV2Inside;
            if (bIs1stVt)
            {
                const auto v1VertLeft = CalcEdgeVerticalPoint(v2, v1, fTestLen, false);
                bV1VertLeftInside = MatUtils::CheckPointInsidePolygon(v1VertLeft, m_av2AllContourVertices);
                bIs1stVt = false;
            }
#if 0
            const auto v2VertLeft = CalcEdgeVerticalPoint(v1, v2, fTestLen, true);
            const bool bV2VertLeftInside = MatUtils::CheckPointInsidePolygon(v2VertLeft, m_av2AllContourVertices);
            if (bV1VertLeftInside^bV2VertLeftInside)
                cout << "Edge two ends have different inside-bility! v1(" << v1.x << "," << v1.y << ") inside(" << bV1VertLeftInside
                    << "), v2(" << v2.x << "," << v2.y << ") inside(" << bV2VertLeftInside << ")." << endl;
            bPreV2Inside = bV2VertLeftInside;
#else
            bPreV2Inside = bV1VertLeftInside;
#endif

            const auto v1Morph = CalcEdgeVerticalPoint(v2, v1, iDilateSize, bV1VertLeftInside);
#if 0
            const auto v2Morph = CalcEdgeVerticalPoint(v1, v2, iDilateSize, !bV2VertLeftInside);
#else
            const auto v2Morph = CalcEdgeVerticalPoint(v1, v2, iDilateSize, !bV1VertLeftInside);
#endif
            if (aVts.empty())
            {
                aVts.push_back(v1Morph);
                aVts.push_back(v2Morph);
            }
            else
            {
                auto itBack = aVts.end(); itBack--;
                const auto& vTail0 = *itBack--;
                const auto& vTail1 = *itBack;
                if (vTail0 != v1Morph)
                {
                    const ImVec2 v[] = {vTail1, vTail0, v1Morph, v2Morph};
                    auto aBzVts = CalcBezierConnection(v);
                    auto itIns = aBzVts.begin(); itIns++;
                    aVts.insert(aVts.end(), itIns, aBzVts.end());
                }
                aVts.push_back(v2Morph);
            }

            itCv++;
            itCvNext++;
            if (itCvNext == m_av2AllContourVertices.end()) itCvNext = m_av2AllContourVertices.begin();
        }
        if (!aVts.empty())
        {
            const auto& vTail0 = aVts.back();
            const auto& vHead0 = aVts.front();
            if (vTail0 != vHead0)
            {
                auto itVt = aVts.end(); itVt--; itVt--;
                const auto& vTail1 = *itVt;
                itVt = aVts.begin(); itVt++;
                const auto& vHead1 = *itVt;
                const ImVec2 v[] = {vTail1, vTail0, vHead0, vHead1};
                auto aBzVts = CalcBezierConnection(v);
                auto itIns0 = aBzVts.begin(); itIns0++;
                auto itIns1 = aBzVts.end(); itIns1--;
                aVts.insert(aVts.end(), itIns0, itIns1);
            }
            else
            {
                aVts.pop_back();
            }

            // remove invalid points caused by contour cross
            list<ImVec2> aValidVts;
            auto itChk0 = aVts.begin();
            auto itChk1 = itChk0; itChk1++;
            bool bPrevChk1Valid = CheckPointValidOnDilateContour(*itChk0, iDilateSize);
            while (itChk0 != aVts.end())
            {
                auto vChk0 = *itChk0;
                const auto& vChk1 = *itChk1;
                const double dSlopeChk = vChk0.x != vChk1.x ? ((double)vChk1.y-vChk0.y)/((double)vChk1.x-vChk0.x) : numeric_limits<double>::infinity();

                bool bChk0Valid, bChk1Valid;
                bChk0Valid = bPrevChk1Valid;
                bChk1Valid = CheckPointValidOnDilateContour(vChk1, iDilateSize);

                ImVec2 v2CrossPoint;
                auto itSch0 = aVts.begin();
                auto itSch1 = itSch0; itSch1++;
                while (itSch0 != aVts.end())
                {
                    if (itSch0 != itChk0)
                    {
                        bool bFoundCross = false;
                        const auto& vSch0 = *itSch0;
                        const auto& vSch1 = *itSch1;
                        const double dSlopeSch = vSch0.x != vSch1.x ? ((double)vSch1.y-vSch0.y)/((double)vSch1.x-vSch0.x) : numeric_limits<double>::infinity();
#if 0
                        if (itSch1 == itChk0)
                        {
                            if (isinf(dSlopeChk) && isinf(dSlopeSch))
                            {
                                if ((vSch0.y>vSch1.y) ^ (vChk0.y>vChk1.y))
                                {
                                    v2CrossPoint.x = vChk0.x;
                                    v2CrossPoint.y = (vSch0.y>vSch1.y) ^ (vChk1.y>vSch0.y) ? vChk1.y : vSch0.y;
                                    bFoundCross = true;
                                }
                            }
                            else if (dSlopeChk == dSlopeSch)
                            {
                                if ((vSch0.x>vSch1.x) ^ (vChk0.x>vChk1.x))
                                {
                                    if ((vSch0.x>vSch1.x) ^ (vChk1.x>vSch0.x))
                                    {
                                        v2CrossPoint.x = vChk1.x;
                                        v2CrossPoint.y = vChk1.y;
                                    }
                                    else
                                    {
                                        v2CrossPoint.x = vSch0.x;
                                        v2CrossPoint.y = vSch0.y;
                                    }
                                    bFoundCross = true;
                                }
                            }
                            if (bFoundCross)
                            {
                                aVts.erase(itSch1);  // remove 'vSch1' (also is 'vChk0')
                                itChk0 = itSch0;
                                itSch1 = itChk1;
                            }
                        }
                        else if (itSch0 == itChk1)
                        {
                            const double dSlopeSch = vSch0.x != vSch1.x ? ((double)vSch1.y-vSch0.y)/((double)vSch1.x-vSch0.x) : numeric_limits<double>::infinity();
                            if (isinf(dSlopeChk) && isinf(dSlopeSch))
                            {
                                if ((vSch0.y>vSch1.y) ^ (vChk0.y>vChk1.y))
                                {
                                    v2CrossPoint.x = vChk0.x;
                                    v2CrossPoint.y = (vSch0.y>vSch1.y) ^ (vChk0.y>vSch1.y) ? vSch1.y : vChk0.y;
                                    bFoundCross = true;
                                }
                            }
                            else if (dSlopeChk == dSlopeSch)
                            {
                                if ((vSch0.x>vSch1.x) ^ (vChk0.x>vChk1.x))
                                {
                                    if ((vSch0.x>vSch1.x) ^ (vChk0.x>vSch1.x))
                                    {
                                        v2CrossPoint.x = vSch1.x;
                                        v2CrossPoint.y = vSch1.y;
                                    }
                                    else
                                    {
                                        v2CrossPoint.x = vChk0.x;
                                        v2CrossPoint.y = vChk0.y;
                                    }
                                    bFoundCross = true;
                                }
                            }
                            if (bFoundCross)
                            {
                                aVts.erase(itSch0);  // remove 'vSch0' (also is 'vChk1')
                                itSch0 = itChk0;
                                itChk1 = itSch1;
                            }
                        }
                        else
#endif
                        {
                            if (isinf(dSlopeChk) && isinf(dSlopeSch))
                            {
                                // TODO: check two lines have overlapped part
                            }
                            else if (dSlopeChk == dSlopeSch)
                            {
                                // TODO: check two lines have overlapped part
                            }
                            else
                            {
                                // check if two lines have cross point
                                const ImVec2 aPoints[] = { vChk0, vChk1, vSch0, vSch1 };
                                if (MatUtils::CheckTwoLinesCross(aPoints, &v2CrossPoint))
                                {
                                    if (v2CrossPoint != vChk1)
                                    {
                                        if (v2CrossPoint != vChk0)
                                        {
                                            if (bChk0Valid)
                                                aValidVts.push_back(vChk0);
                                            vChk0 = v2CrossPoint;
                                        }
                                        bChk0Valid = CheckPointValidOnDilateContour(vChk0, iDilateSize);
                                    }
                                }
                            }
                        }
                    }

                    itSch0++; itSch1++;
                    if (itSch1 == aVts.end()) itSch1 = aVts.begin();
                }

                if (bChk0Valid)
                    aValidVts.push_back(vChk0);
                bPrevChk1Valid = bChk1Valid;
                itChk0++; itChk1++;
                if (itChk1 == aVts.end()) itChk1 = aVts.begin();
            }
            aVts.swap(aValidVts);
        }

        const int iLoopCnt = aVts.size();
        vector<MatUtils::Point2f> res(iLoopCnt);
        auto itVt = aVts.begin();
        for (int i = 0; i < iLoopCnt; i++)
            res[i] = MatUtils::FromImVec2<float>(*itVt++);
        return res;
    }

    vector<MatUtils::Point2f> ErodeContour(int iDilateSize)
    {
        if (iDilateSize <= 0)
            return ConvertPointsType(m_av2AllContourVertices);

        list<ImVec2> aVts;
        auto itCv = m_av2AllContourVertices.begin();
        auto itCvNext = itCv; itCvNext++;
        const float fTestLen = 1e-1;
        bool bIs1stVt = true, bPreV2Inside;
        while (itCv != m_av2AllContourVertices.end())
        {
            const auto& v1 = *itCv;
            const auto& v2 = *itCvNext;

            bool bV1VertLeftInside = bPreV2Inside;
            if (bIs1stVt)
            {
                const auto v1VertLeft = CalcEdgeVerticalPoint(v2, v1, fTestLen, false);
                bV1VertLeftInside = MatUtils::CheckPointInsidePolygon(v1VertLeft, m_av2AllContourVertices);
                bIs1stVt = false;
            }
            bPreV2Inside = bV1VertLeftInside;

            const auto v1Morph = CalcEdgeVerticalPoint(v2, v1, iDilateSize, !bV1VertLeftInside);
            const auto v2Morph = CalcEdgeVerticalPoint(v1, v2, iDilateSize, bV1VertLeftInside);

            if (aVts.empty())
            {
                aVts.push_back(v1Morph);
                aVts.push_back(v2Morph);
            }
            else
            {
                auto itBack = aVts.end(); itBack--;
                const auto& vTail0 = *itBack--;
                const auto& vTail1 = *itBack;
                if (vTail0 != v1Morph)
                {
                    const ImVec2 v[] = {vTail1, vTail0, v1Morph, v2Morph};
                    auto aBzVts = CalcBezierConnection(v);
                    auto itIns = aBzVts.begin(); itIns++;
                    aVts.insert(aVts.end(), itIns, aBzVts.end());
                }
                aVts.push_back(v2Morph);
            }

            itCv++;
            itCvNext++;
            if (itCvNext == m_av2AllContourVertices.end()) itCvNext = m_av2AllContourVertices.begin();
        }
        if (!aVts.empty())
        {
            const auto& vTail0 = aVts.back();
            const auto& vHead0 = aVts.front();
            if (vTail0 != vHead0)
            {
                auto itVt = aVts.end(); itVt--; itVt--;
                const auto& vTail1 = *itVt;
                itVt = aVts.begin(); itVt++;
                const auto& vHead1 = *itVt;
                const ImVec2 v[] = {vTail1, vTail0, vHead0, vHead1};
                auto aBzVts = CalcBezierConnection(v);
                auto itIns0 = aBzVts.begin(); itIns0++;
                auto itIns1 = aBzVts.end(); itIns1--;
                aVts.insert(aVts.end(), itIns0, itIns1);
            }
            else
            {
                aVts.pop_back();
            }

            // remove invalid points caused by contour cross
            list<ImVec2> aValidVts;
            auto itChk0 = aVts.begin();
            auto itChk1 = itChk0; itChk1++;
            bool bPrevChk1Valid = CheckPointValidOnErodeContour(*itChk0, iDilateSize);
            while (itChk0 != aVts.end())
            {
                auto vChk0 = *itChk0;
                const auto& vChk1 = *itChk1;
                const double dSlopeChk = vChk0.x != vChk1.x ? ((double)vChk1.y-vChk0.y)/((double)vChk1.x-vChk0.x) : numeric_limits<double>::infinity();

                bool bChk0Valid, bChk1Valid;
                bChk0Valid = bPrevChk1Valid;
                bChk1Valid = CheckPointValidOnErodeContour(vChk1, iDilateSize);

                ImVec2 v2CrossPoint;
                auto itSch0 = aVts.begin();
                auto itSch1 = itSch0; itSch1++;
                while (itSch0 != aVts.end())
                {
                    if (itSch0 != itChk0)
                    {
                        bool bFoundCross = false;
                        const auto& vSch0 = *itSch0;
                        const auto& vSch1 = *itSch1;
                        const double dSlopeSch = vSch0.x != vSch1.x ? ((double)vSch1.y-vSch0.y)/((double)vSch1.x-vSch0.x) : numeric_limits<double>::infinity();
                        if (isinf(dSlopeChk) && isinf(dSlopeSch))
                        {
                            // TODO: check two lines have overlapped part
                        }
                        else if (dSlopeChk == dSlopeSch)
                        {
                            // TODO: check two lines have overlapped part
                        }
                        else
                        {
                            // check if two lines have cross point
                            const ImVec2 aPoints[] = { vChk0, vChk1, vSch0, vSch1 };
                            if (MatUtils::CheckTwoLinesCross(aPoints, &v2CrossPoint))
                            {
                                if (v2CrossPoint != vChk1)
                                {
                                    if (v2CrossPoint != vChk0)
                                    {
                                        if (bChk0Valid)
                                            aValidVts.push_back(vChk0);
                                        vChk0 = v2CrossPoint;
                                    }
                                    bChk0Valid = CheckPointValidOnErodeContour(vChk0, iDilateSize);
                                }
                            }
                        }
                    }

                    itSch0++; itSch1++;
                    if (itSch1 == aVts.end()) itSch1 = aVts.begin();
                }

                if (bChk0Valid)
                    aValidVts.push_back(vChk0);
                bPrevChk1Valid = bChk1Valid;
                itChk0++; itChk1++;
                if (itChk1 == aVts.end()) itChk1 = aVts.begin();
            }
            aVts.swap(aValidVts);
        }

        const int iLoopCnt = aVts.size();
        vector<MatUtils::Point2f> res(iLoopCnt);
        auto itVt = aVts.begin();
        for (int i = 0; i < iLoopCnt; i++)
            res[i] = MatUtils::FromImVec2<float>(*itVt++);
        return res;
    }

    bool CheckPointValidOnErodeContour(const ImVec2& v, float fDilateSize)
    {
        float fDistToOrgContour;
        MatUtils::CalcNearestPointOnPloygon(v, m_av2AllContourVertices, nullptr, nullptr, &fDistToOrgContour);
        bool bIsInside = MatUtils::CheckPointInsidePolygon(v, m_av2AllContourVertices);
        return fDistToOrgContour >= fDilateSize-1e-2 && bIsInside;
    }

    vector<MatUtils::Point2f> ConvertPointsType(const list<ImVec2>& vts)
    {
        const int iLoopCnt = vts.size();
        vector<MatUtils::Point2f> res(iLoopCnt);
        auto itVt = vts.begin();
        for (int i = 0; i < iLoopCnt; i++)
            res[i] = MatUtils::FromImVec2<float>(*itVt++);
        return res;
    }

private:
    string m_name;
    MatUtils::Size2i m_size;
    ImVec2 m_v2UiScale{1.0f, 1.0f};
    ImRect m_rWorkArea{{-1, -1}, {-1, -1}};
    list<ContourPointImpl> m_atContourPoints;
    list<ImVec2> m_av2AllContourVertices;
    ImVec2 m_v2PointSize{7.f, 7.f}, m_v2PointSizeHalf;
    ImU32 m_u32PointColor{IM_COL32(40, 170, 40, 255)};
    ImU32 m_u32ContourHoverPointColor{IM_COL32(80, 80, 80, 255)};
    float m_fPointBorderThickness{1.5f};
    ImU32 m_u32PointBorderColor{IM_COL32(150, 150, 150, 255)};
    ImU32 m_u32PointBorderHoverColor{IM_COL32(240, 240, 240, 255)};
    float m_fGrabberRadius{4.5f}, m_fGrabberBorderThickness{1.5f};
    ImU32 m_u32GrabberColor{IM_COL32(80, 80, 150, 255)};
    ImU32 m_u32GrabberBorderColor{IM_COL32(150, 150, 150, 255)};
    ImU32 m_u32GrabberBorderHoverColor{IM_COL32(240, 240, 240, 255)};
    float m_fGrabberLineThickness{2.f};
    ImU32 m_u32GrabberLineColor{IM_COL32(80, 80, 150, 255)};
    float m_fContourThickness{3.f};
    ImU32 m_u32ContourColor{IM_COL32(40, 40, 170, 255)};
    ImU32 m_u32FeatherGrabberColor{IM_COL32(200, 150, 80, 255)};
    ImRect m_rContianBox{{-1, -1}, {-1, -1}};
    ImVec2 m_ptCenter;
    float m_fContourHoverDetectExRadius{4.f}, m_fHoverDetectExRadius{5.f};
    list<ContourPointImpl>::iterator m_itHoveredVertex;
    MorphController m_tMorphCtrl;
    list<ContourPointImpl>::iterator m_itMorphCtrlVt;
    ImGuiKey m_eRemoveVertexKey{ImGuiKey_LeftAlt};
    ImGuiKey m_eEnableBezierKey{ImGuiKey_LeftCtrl};
    bool m_bContourCompleted{false};
    bool m_bContourChanged{false};
    bool m_bLastMaskFilled{false};
    int m_iLastMaskLineType{0};
    ImDataType m_eLastMaskDataType{IM_DT_UNDEFINED};
    double m_dLastMaskValue{0}, m_dLastNonMaskValue{0};
    ImGui::ImMat m_mMask, m_mMorphMask;
    ImGui::ImMat m_mMorphKernel;
    int m_iLastMorphIters{0}, m_iLastFeatherIters{0};
    string m_sErrMsg;
};

unordered_map<int, MaskCreatorImpl::BezierTable::Holder> MaskCreatorImpl::s_mapBezierTables;
int MaskCreatorImpl::s_iKeepBezierTableCountMin = 10;
chrono::seconds MaskCreatorImpl::s_iKeepBezierTableTimeOut = chrono::seconds(30);

static const auto MASK_CREATOR_DELETER = [] (MaskCreator* p) {
    MaskCreatorImpl* ptr = dynamic_cast<MaskCreatorImpl*>(p);
    delete ptr;
};

MaskCreator::Holder MaskCreator::CreateInstance(const MatUtils::Size2i& size, const string& name)
{
    return MaskCreator::Holder(new MaskCreatorImpl(size, name), MASK_CREATOR_DELETER);
}

void MaskCreator::GetVersion(int& major, int& minor, int& patch, int& build)
{
    major = ImMaskCreator_VERSION_MAJOR;
    minor = ImMaskCreator_VERSION_MINOR;
    patch = ImMaskCreator_VERSION_PATCH;
    build = ImMaskCreator_VERSION_BUILD;
}

MaskCreator::Holder MaskCreator::LoadFromJson(const imgui_json::value& j)
{
    MaskCreator::Holder hInst = CreateInstance({0, 0});
    MaskCreatorImpl* pInst = dynamic_cast<MaskCreatorImpl*>(hInst.get());
    pInst->LoadFromJson(j);
    return hInst;
}

MaskCreator::Holder MaskCreator::LoadFromJson(const string& filePath)
{
    auto res = json::value::load(filePath);
    if (!res.second)
        return nullptr;
    MaskCreator::Holder hInst = CreateInstance({0, 0});
    MaskCreatorImpl* pInst = dynamic_cast<MaskCreatorImpl*>(hInst.get());
    pInst->LoadFromJson(res.first);
    return hInst;
}
}