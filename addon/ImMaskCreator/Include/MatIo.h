#pragma once
#include <string>
#include "immat.h"

namespace MatUtils
{
    bool SaveAsPng(const ImGui::ImMat& m, const std::string& savePath);
}