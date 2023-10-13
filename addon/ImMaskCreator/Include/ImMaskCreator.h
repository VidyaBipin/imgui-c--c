#pragma once
#include <memory>
#include <string>
#include <imgui.h>
#include <immat.h>
#include <imgui_json.h>
#include "MatUtilsCommDef.h"

namespace ImGui
{
struct ContourPoint
{
    virtual MatUtils::Point2i GetPos() const = 0;
    virtual MatUtils::Point2i GetBezierGrabberOffset(int idx) const = 0;
    virtual int GetHoverType() const = 0;
};

struct MaskCreator
{
    enum LineType
    {
        CONNECT8 = 0,
        CONNECT4,
        AA,
        CAPSULE_SDF,
    };

    using Holder = std::shared_ptr<MaskCreator>;
    IMGUI_API static Holder CreateInstance(const MatUtils::Size2i& size, const std::string& name = "");
    IMGUI_API static void GetVersion(int& major, int& minor, int& patch, int& build);

    virtual std::string GetName() const = 0;
    virtual void SetName(const std::string& name) = 0;
    virtual bool DrawContent(const ImVec2& v2Pos, const ImVec2& v2Size, bool bUpdateUiScale = true) = 0;
    virtual bool ChangeMaskSize(const MatUtils::Size2i& size) = 0;
    virtual MatUtils::Size2i GetMaskSize() const = 0;
    virtual ImGui::ImMat GetMask(int iLineType, bool bFilled = true, ImDataType eDataType = IM_DT_INT8, double dMaskValue = 255, double dNonMaskValue = 0) = 0;
    virtual const ContourPoint* GetHoveredPoint() const = 0;
    virtual ImVec4 GetContourContainBox() const = 0;
    virtual ImVec2 GetUiScale() const = 0;

    virtual bool SaveAsJson(imgui_json::value& j) const = 0;
    virtual bool SaveAsJson(const std::string& filePath) const = 0;
    IMGUI_API static Holder LoadFromJson(const imgui_json::value& j);
    IMGUI_API static Holder LoadFromJson(const std::string& filePath);

    virtual std::string GetError() const = 0;
};
}