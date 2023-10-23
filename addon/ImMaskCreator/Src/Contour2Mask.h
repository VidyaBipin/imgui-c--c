#pragma once
#include <vector>
#include "immat.h"
#include "imgui.h"
#include "MatUtilsCommDef.h"

namespace MatUtils
{
ImGui::ImMat MakeColor(ImDataType eDtype, double dColorVal);

ImGui::ImMat Contour2Mask(
        const std::vector<Point2f>& av2ContourVertices, const Size2i& v2MaskSize, const Point2f& v2ContourOffset,
        ImDataType dtMaskDataType, double dMaskValue, double dNonMaskValue, int iLineType, bool bFilled = true);

void DrawPolygon(ImGui::ImMat& img, const std::vector<Point2f>& aContourVertices, const ImGui::ImMat& color, int iLineType);

bool CheckTwoLinesCross(const Point2f v[4], Point2f* pCross);
bool CheckTwoLinesCross(const ImVec2 v[4], ImVec2* pCross);
bool CheckPointOnLine(const ImVec2& po, const ImVec2 v[2]);
};