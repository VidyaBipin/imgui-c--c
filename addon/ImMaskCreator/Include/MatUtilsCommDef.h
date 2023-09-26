#pragma once
#include <cstdint>
#include <utility>

namespace MatUtils
{
template<typename T>
struct Vec2 : public std::pair<T,T>
{
    using elemtype = T;
    using basetype = std::pair<elemtype,elemtype>;
    Vec2() : x(basetype::first), y(basetype::second) {}
    Vec2(const elemtype& a, const elemtype& b) : basetype(a, b), x(basetype::first), y(basetype::second) {}
    Vec2(const basetype& vec2) : basetype(vec2), x(basetype::first), y(basetype::second) {}
    Vec2(const Vec2<T>& vec2) : basetype(vec2.x, vec2.y), x(basetype::first), y(basetype::second) {}
    template<typename U>
    Vec2(const Vec2<U>& vec2) : basetype(vec2.x, vec2.y), x(basetype::first), y(basetype::second) {}
    Vec2<T>& operator=(const Vec2<T>& a) { *static_cast<basetype*>(this) = static_cast<const basetype>(a); return *this; }
    Vec2<T> operator+(const Vec2<T>& a) const { return {x+a.x, y+a.y}; }
    Vec2<T> operator-(const Vec2<T>& a) const { return {x-a.x, y-a.y}; }
    Vec2<T> operator*(const Vec2<T>& a) const { return {x*a.x, y*a.y}; }
    Vec2<T> operator/(const Vec2<T>& a) const { return {x/a.x, y/a.y}; }
    Vec2<T>& operator+=(const Vec2<T>& a) { x += a.x; y += a.y; return *this; }

    elemtype& x;
    elemtype& y;
};

template<typename T>
using Point = Vec2<T>;
using Point2i = Point<int32_t>;
using Point2l = Point<int64_t>;
using Point2f = Point<float>;

template<typename T>
using Size = Vec2<T>;
using Size2i = Size<int32_t>;
using Size2l = Size<int64_t>;
using Size2f = Size<float>;

template <typename T>
struct Rect : public std::pair<Point<T>, Size<T>>
{
    using elemtype1 = Point<T>;
    using elemtype2 = Size<T>;
    using basetype = std::pair<elemtype1, elemtype2>;
    Rect() : leftTop(basetype::first), size(basetype::second) {}
    Rect(const elemtype1& a, const elemtype2& b) : basetype(a, b), leftTop(basetype::first), size(basetype::second) {}
    Rect(const basetype& rect) : basetype(rect), leftTop(basetype::first), size(basetype::second) {}
    Rect(const Rect<T>& rect) : basetype(rect.leftTop, rect.size), leftTop(basetype::first), size(basetype::second) {}
    template<typename U>
    Rect(const Rect<U>& rect) : basetype(rect.leftTop, rect.size), leftTop(basetype::first), size(basetype::second) {}
    Rect(const T& x, const T& y, const T& w, const T& h) : basetype(elemtype1(x, y), elemtype2(w, h)), leftTop(basetype::first), size(basetype::second) {}
    Rect<T>& operator=(const Rect<T>& a) { *static_cast<basetype*>(this) = static_cast<const basetype>(a); return *this; }
    Rect<T> operator/(const Vec2<T>& a) const { return {leftTop/a, size/a}; }

    elemtype1& leftTop; // left top coordinates
    elemtype2& size;    // size
    Point<T> rightBottom() const { return {leftTop.x+size.x, leftTop.y+size.y}; }
};
using Recti = Rect<int32_t>;
using Rectl = Rect<int64_t>;
using Rectf = Rect<float>;
}