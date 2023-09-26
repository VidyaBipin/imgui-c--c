#pragma once
#include <vector>
#include "immat.h"
#include "MatUtilsCommDef.h"

namespace MatUtils
{
ImGui::ImMat Contour2Mask(
        const std::vector<Point2f>& av2ContourVertices, const Size2i& v2MaskSize, const Point2f& v2ContourOffset,
        ImDataType dtMaskDataType, double dMaskValue, double dNonMaskValue, int iLineType, bool bFilled = true);
};