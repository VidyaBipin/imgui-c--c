#pragma once
#include "MatUtilsCommDef.h"
#include "imgui.h"

namespace MatUtils
{
template<typename T>
ImVec2 ToImVec2(const Point<T>& pt)
{ return ImVec2(pt.x, pt.y); }

template<typename T>
Point<T> FromImVec2(const ImVec2& vec)
{ return Point<T>(vec.x, vec.y); }
}