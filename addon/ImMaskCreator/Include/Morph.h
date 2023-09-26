#pragma once
#include "immat.h"
#include "MatUtilsCommDef.h"

namespace MatUtils
{
enum MorphShape
{
    MORPH_RECT = 0,
    MORPH_CROSS,
    MORPH_ELLIPSE,
};
ImGui::ImMat GetStructuringElement(MorphShape eShape, const Size2i& szKsize, const Point2i& ptAnchor = Point2i(-1, -1));

ImGui::ImMat Erode(const ImGui::ImMat& mInput, const ImGui::ImMat& mKernel, const Point2i& ptAnchor, int iIterations = 1);
ImGui::ImMat Dilate(const ImGui::ImMat& mInput, const ImGui::ImMat& mKernel, const Point2i& ptAnchor, int iIterations = 1);
}