#pragma once
#include "MatUtilsCommDef.h"
#include "imgui.h"

namespace MatUtils
{
template<typename T>
ImVec2 ToImVec2(const Vec2<T>& pt)
{ return ImVec2(pt.x, pt.y); }

template<typename T>
Vec2<T> FromImVec2(const ImVec2& vec)
{ return Vec2<T>(vec.x, vec.y); }
}