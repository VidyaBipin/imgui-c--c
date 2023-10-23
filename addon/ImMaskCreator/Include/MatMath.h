#pragma once
#include <iterator>
#include <limits>
#include <cmath>
#include <imgui.h>
#include <immat.h>

namespace MatUtils
{
template<template<class T, class Alloc = std::allocator<T>> class Container>
bool CheckPointInsidePolygon(const ImVec2& ptPoint, Container<ImVec2> const& aPloyVertices)
{
    if (aPloyVertices.size() < 3)
        return false;

    int crossNum = 0;
    const auto& x = ptPoint.x;
    const auto& y = ptPoint.y;
    auto it0 = aPloyVertices.begin();
    auto it1 = it0; it1++;
    while (it0 != aPloyVertices.end())
    {
        const auto& x0 = it0->x < it1->x ? it0->x : it1->x;
        const auto& x1 = it0->x < it1->x ? it1->x : it0->x;
        const auto& y0 = it0->y < it1->y ? it0->y : it1->y;
        const auto& y1 = it0->y < it1->y ? it1->y : it0->y;
        if (y >= y0 && y < y1)
        {
            if (x <= x0)
                crossNum++;
            else if (x < x1)
            {
                const auto cx = (it1->x-it0->x)*(y-it0->y)/(it1->y-it0->y)+it0->x;
                if (x <= cx)
                    crossNum++;
            }
        }
        it0++; it1++;
        if (it1 == aPloyVertices.end())
            it1 = aPloyVertices.begin();
    }
    return (crossNum&0x1) > 0;
}

template<template<class T, class Alloc = std::allocator<T>> class Container>
ImVec2 CalcNearestPointOnPloygon(const ImVec2& ptPoint, Container<ImVec2> const& aPloyVertices, ImVec2* pv2VerticalSlope = nullptr, int* piLineIdx = nullptr, float* pfMinDist = nullptr)
{
    if (aPloyVertices.size() < 3)
        return ptPoint;

    ImVec2 result(ptPoint), resSlope(0, 0);
    int resLineIdx = 0, lineIdx = 0;
    float minDistSqr = std::numeric_limits<float>::max();
    auto it1 = aPloyVertices.begin();
    auto it2 = it1; it2++;
    const auto& xp = ptPoint.x;
    const auto& yp = ptPoint.y;
    while (it1 != aPloyVertices.end())
    {
        const auto& x1 = it1->x;
        const auto& y1 = it1->y;
        const auto& x2 = it2->x;
        const auto& y2 = it2->y;
        const auto a = y2-y1;
        const auto b = x1-x2;
        const auto c = x2*y1-x1*y2;

        const auto xc = -((a*yp-b*xp)*b+a*c)/(a*a+b*b);
        const auto yc = ((a*yp-b*xp)*a-b*c)/(a*a+b*b);
        bool isVcIn = false;
        if (x1 != x2)
            isVcIn = (xc < x1)^(xc < x2);
        else
            isVcIn = (yc < y1)^(yc < y2);

        float distSqr;
        ImVec2 candidate, candSlope(x1-x2, y2-y1);
        if (isVcIn)
        {
            const auto dx12 = x2-x1;
            const auto dy12 = y2-y1;
            const auto l3num = dy12*xp-dx12*yp+c;
            distSqr = l3num*l3num/(dx12*dx12+dy12*dy12);
            candidate.x = xc;
            candidate.y = yc;
        }
        else
        {
            const auto l1sqr = (xp-x1)*(xp-x1)+(yp-y1)*(yp-y1);
            const auto l2sqr = (xp-x2)*(xp-x2)+(yp-y2)*(yp-y2);
            if (l1sqr < l2sqr)
            {
                distSqr = l1sqr;
                candidate.x = x1;
                candidate.y = y1;
            }
            else
            {
                distSqr = l2sqr;
                candidate.x = x2;
                candidate.y = y2;
            }
        }

        if (distSqr < minDistSqr)
        {
            minDistSqr = distSqr;
            result = candidate;
            resSlope = candSlope;
            resLineIdx = lineIdx;
        }

        it1++; it2++; lineIdx++;
        if (it2 == aPloyVertices.end())
        {
            it2 = aPloyVertices.begin();
            lineIdx = 0;
        }
    }
    if (pv2VerticalSlope)
        *pv2VerticalSlope = resSlope;
    if (piLineIdx)
        *piLineIdx = resLineIdx;
    if (pfMinDist)
        *pfMinDist = std::sqrt(minDistSqr);
    return result;
}

struct MatOp1
{
    using Holder = std::shared_ptr<MatOp1>;
    virtual void operator()(const ImGui::ImMat& src, ImGui::ImMat& dst) = 0;
};

struct MatOp2
{
    using Holder = std::shared_ptr<MatOp2>;
    virtual void operator()(const ImGui::ImMat& src1, const ImGui::ImMat& src2, ImGui::ImMat& dst) = 0;
};

void Max(ImGui::ImMat& dst, const ImGui::ImMat& src);
void Copy(ImGui::ImMat& dst, const ImGui::ImMat& src);

enum DataTypeConvType
{
    DataTypeConvType_None = 0,
    DataTypeConvType_Saturate,
    DataTypeConvType_Truncate,
    DataTypeConvType_Round,
    DataTypeConvType_Floor,
    DataTypeConvType_Ceil,
};

void Convert(ImGui::ImMat& dst, const ImGui::ImMat& src, DataTypeConvType convType);
void ConvertColorDepth(ImGui::ImMat& dst, const ImGui::ImMat& src);

void GrayToRgba(ImGui::ImMat& dst, const ImGui::ImMat& src, double alphaVal);

// channelMap: for index in [0,3], copy value from 'src' to 'dst'; for [4,7], copy value from 'constVals' [0,3]
// void MapChannel(ImGui::ImMat& dst, const ImGui::ImMat& src, int channelMap[4], double constVals[4], DataTypeConvType convType);
}