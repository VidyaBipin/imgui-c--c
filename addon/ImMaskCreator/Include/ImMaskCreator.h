#pragma once
#include <memory>
#include <string>
#include <imgui.h>
#include <immat.h>
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
    using Holder = std::shared_ptr<MaskCreator>;
    static Holder CreateInstance();

    virtual bool DrawContent(const ImVec2& v2Pos, const ImVec2& v2Size) = 0;
    virtual ImGui::ImMat GetMask(int iLineType, bool bFilled = true) = 0;
    virtual const ContourPoint* GetHoveredPoint() const = 0;
    virtual ImVec4 GetContourContainBox() const = 0;

    virtual bool SaveAsJson(const std::string& filePath) const = 0;
    static Holder LoadFromJson(const std::string& filePath);

    virtual std::string GetError() const = 0;
};
}