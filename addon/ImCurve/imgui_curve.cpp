#include "imgui_curve.h"

// CurveEdit from https://github.com/CedricGuillemet/ImGuizmo
template <typename T>
static T __tween_bounceout(const T& p) { return (p) < 4 / 11.0 ? (121 * (p) * (p)) / 16.0 : (p) < 8 / 11.0 ? (363 / 40.0 * (p) * (p)) - (99 / 10.0 * (p)) + 17 / 5.0 : (p) < 9 / 10.0 ? (4356 / 361.0 * (p) * (p)) - (35442 / 1805.0 * (p)) + 16061 / 1805.0 : (54 / 5.0 * (p) * (p)) - (513 / 25.0 * (p)) + 268 / 25.0; }
const char * curveTypeName[] = {
        "Hold", "Step", "Linear", "Smooth",
        "QuadIn", "QuadOut", "QuadInOut", 
        "CubicIn", "CubicOut", "CubicInOut", 
        "SineIn", "SineOut", "SineInOut",
        "ExpIn", "ExpOut", "ExpInOut",
        "CircIn", "CircOut", "CircInOut",
        "ElasticIn", "ElasticOut", "ElasticInOut",
        "BackIn", "BackOut", "BackInOut",
        "BounceIn", "BounceOut", "BounceInOut"};

int ImGui::ImCurveEdit::GetCurveTypeName(char**& list)
{
    list = (char **)curveTypeName;
    return IM_ARRAYSIZE(curveTypeName);
}

float ImGui::ImCurveEdit::smoothstep(float edge0, float edge1, float t, CurveType type)
{
    const double s = 1.70158f;
    const double s2 = 1.70158f * 1.525f;
    t = ImClamp((t - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    auto t2 = t - 1;
    switch (type)
    {
        case Hold:
            return (0);
        case Step:
            return (t > .5);
        case Linear:
            return t;
        case Smooth:
            return t * t * (3 - 2 * t);
        case QuadIn:
            return t * t;
        case QuadOut:
            return -(t * (t - 2));
        case QuadInOut:
            return (t < 0.5) ? (2 * t * t) : ((-2 * t * t) + (4 * t) - 1);
        case CubicIn:
            return t * t * t;
        case CubicOut:
            return (t - 1) * (t - 1) * (t - 1) + 1;
        case CubicInOut:
            return (t < 0.5) ? (4 * t * t * t) : (.5 * ((2 * t) - 2) * ((2 * t) - 2) * ((2 * t) - 2) + 1);
        case SineIn:
            return sin((t - 1) * M_PI_2) + 1;
        case SineOut:
            return sin(t * M_PI_2);
        case SineInOut:
            return 0.5 * (1 - cos(t * M_PI));
        case ExpIn:
            return (t == 0.0) ? t : pow(2, 10 * (t - 1));
        case ExpOut:
            return (t == 1.0) ? t : 1 - pow(2, -10 * t);
        case ExpInOut:
            if (t == 0.0 || t == 1.0) return t;
            if (t < 0.5) { return 0.5 * pow(2, (20 * t) - 10); }
            else { return -0.5 * pow(2, (-20 * t) + 10) + 1; }
        case CircIn:
            return 1 - sqrt(1 - (t * t));
        case CircOut:
            return sqrt((2 - t) * t);
        case CircInOut:
            if (t < 0.5) { return 0.5 * (1 - sqrt(1 - 4 * (t * t)));}
            else { return 0.5 * (sqrt(-((2 * t) - 3) * ((2 * t) - 1)) + 1); }
        case ElasticIn:
            return sin(13 * M_PI_2 * t) * pow(2, 10 * (t - 1));
        case ElasticOut:
            return sin(-13 * M_PI_2 * (t + 1)) * pow(2, -10 * t) + 1;
        case ElasticInOut:
            if (t < 0.5) { return 0.5 * sin(13 * M_PI_2 * (2 * t)) * pow(2, 10 * ((2 * t) - 1)); }
            else { return 0.5 * (sin(-13 * M_PI_2 * ((2 * t - 1) + 1)) * pow(2, -10 * (2 * t - 1)) + 2); }
        case BackIn:
            return t * t * ((s + 1) * t - s);
        case BackOut:
            return 1.f * (t2 * t2 * ((s + 1) * t2 + s) + 1);
        case BackInOut:
            if (t < 0.5) { auto p2 = t * 2; return 0.5 * p2 * p2 * (p2 * s2 + p2 - s2);}
            else { auto p = t * 2 - 2; return 0.5 * (2 + p * p * (p * s2 + p + s2)); }
        case BounceIn:
            return 1 - __tween_bounceout(1 - t);
        case BounceOut:
            return __tween_bounceout(t);
        case BounceInOut:
            if (t < 0.5) { return 0.5 * (1 - __tween_bounceout(1 - t * 2)); }
            else { return 0.5 * __tween_bounceout((t * 2 - 1)) + 0.5; }
        default:
            return t;
    }
}

float ImGui::ImCurveEdit::distance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

float ImGui::ImCurveEdit::distance(float x, float y, float x1, float y1, float x2, float y2)
{
    float A = x - x1;
    float B = y - y1;
    float C = x2 - x1;
    float D = y2 - y1;
    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = -1.f;
    if (len_sq > FLT_EPSILON)
        param = dot / len_sq;
    float xx, yy;
    if (param < 0.f) {
        xx = x1;
        yy = y1;
    }
    else if (param > 1.f) {
        xx = x2;
        yy = y2;
    }
    else {
       xx = x1 + param * C;
       yy = y1 + param * D;
    }
    float dx = x - xx;
    float dy = y - yy;
    return sqrtf(dx * dx + dy * dy);
}

int ImGui::ImCurveEdit::DrawPoint(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited)
{
    int ret = 0;
    ImGuiIO& io = ImGui::GetIO();
    static const ImVec2 localOffsets[4] = { ImVec2(1,0), ImVec2(0,1), ImVec2(-1,0), ImVec2(0,-1) };
    ImVec2 offsets[4];
    for (int i = 0; i < 4; i++)
    {
       offsets[i] = pos * size + localOffsets[i] * 4.5f + offset;
    }
    const ImVec2 center = pos * size + offset;
    const ImRect anchor(center - ImVec2(5, 5), center + ImVec2(5, 5));
    draw_list->AddConvexPolyFilled(offsets, 4, 0xFF000000);
    if (anchor.Contains(io.MousePos))
    {
        ret = 1;
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            ret = 2;
    }
    if (edited)
        draw_list->AddPolyline(offsets, 4, 0xFFFFFFFF, true, 3.0f);
    else if (ret)
        draw_list->AddPolyline(offsets, 4, 0xFF80B0FF, true, 2.0f);
    else
        draw_list->AddPolyline(offsets, 4, 0xFF0080FF, true, 2.0f);
    return ret;
}

bool ImGui::ImCurveEdit::Edit(ImDrawList* draw_list, Delegate* delegate, const ImVec2& size, unsigned int id, bool editable, 
                            float& cursor_pos, float firstTime, float lastTime, 
                            unsigned int flags, const ImRect* clippingRect, bool * changed)
{
    bool hold = false;
    bool curve_changed = false;
    bool draw_timeline = flags & CURVE_EDIT_FLAG_DRAW_TIMELINE;
    const float timeline_height = 30.f;
    if (!delegate) return hold;
    ImGuiIO& io = ImGui::GetIO();
    bool bEnableDelete = ImGui::IsKeyDown(ImGuiKey_LeftShift) && (io.KeyMods == ImGuiMod_Shift);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::BeginChildFrame(id, size);
    delegate->focused = ImGui::IsWindowFocused();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    if (!draw_list) draw_list = ImGui::GetWindowDrawList();
    if (clippingRect)
        draw_list->PushClipRect(clippingRect->Min, clippingRect->Max, true);

    ImVec2 edit_size = size - ImVec2(0, 4) - (draw_timeline ? ImVec2(0, timeline_height) : ImVec2(0, 0));
    const ImVec2 offset = window_pos + ImVec2(0.f, edit_size.y + 2) + (draw_timeline ? ImVec2(0, timeline_height) : ImVec2(0, 0));
    const ImVec2 ssize(edit_size.x, -edit_size.y);
    const ImRect container(offset + ImVec2(0.f, ssize.y), offset + ImVec2(ssize.x, 0.f));
    ImVec2& vmin = delegate->GetMin();
    ImVec2& vmax = delegate->GetMax();
    ImVec2 _vmin = ImVec2(firstTime, vmin.y);
    ImVec2 _vmax = ImVec2(lastTime, vmax.y);
    ImVec2 _range = _vmax - _vmin;
    const ImVec2 viewSize(edit_size.x, -edit_size.y);
    const size_t curveCount = delegate->GetCurveCount();
    const ImVec2 sizeOfPixel = ImVec2(1.f, 1.f) / viewSize;
    bool point_selected = delegate->overSelectedPoint && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool curve_selected = (flags & CURVE_EDIT_FLAG_MOVE_CURVE) && (delegate->movingCurve != -1) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    auto pointToRange = [&](ImVec2 pt) { return (pt - _vmin) / _range; };
    auto rangeToPoint = [&](ImVec2 pt) { return (pt * _range) + _vmin; };

    // draw timeline and mark
    if (draw_timeline)
    {
        float duration = _vmax.x - _vmin.x;
        float msPixelWidth = size.x / (duration + FLT_EPSILON);
        ImVec2 headerSize(size.x, (float)timeline_height);
        ImGui::InvisibleButton("CurveTimelineBar", headerSize);
        draw_list->AddRectFilled(window_pos, window_pos + headerSize, IM_COL32( 33,  33,  38, 255), 0);
        ImRect movRect(window_pos, window_pos + headerSize);
        if (editable && !delegate->MovingCurrentTime && movRect.Contains(io.MousePos) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !point_selected && !curve_selected)
        {
            delegate->MovingCurrentTime = true;
        }
        if (delegate->MovingCurrentTime && duration > 0)
        {
            auto oldPos = cursor_pos;
            auto newPos = (int64_t)((io.MousePos.x - movRect.Min.x) / msPixelWidth) + _vmin.x;
            if (newPos < _vmin.x)
                newPos = _vmin.x;
            if (newPos >= _vmax.x)
                newPos = _vmax.x;
            cursor_pos = newPos;
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            delegate->MovingCurrentTime = false;
        }
        
        int64_t modTimeCount = 10;
        int timeStep = 1;
        while ((modTimeCount * msPixelWidth) < 100)
        {
            modTimeCount *= 10;
            timeStep *= 10;
        };
        int halfModTime = modTimeCount / 2;
        auto drawLine = [&](int64_t i, int regionHeight)
        {
            bool baseIndex = ((i % modTimeCount) == 0) || (i == 0 || i == duration);
            bool halfIndex = (i % halfModTime) == 0;
            int px = (int)window_pos.x + int(i * msPixelWidth);
            int timeStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
            int timeEnd = baseIndex ? regionHeight : timeline_height;
            if (px <= (size.x + window_pos.x) && px >= window_pos.x)
            {
                draw_list->AddLine(ImVec2((float)px, window_pos.y + (float)timeStart), ImVec2((float)px, window_pos.y + (float)timeEnd - 1), halfIndex ? IM_COL32(255, 255, 255, 255) : IM_COL32(128, 128, 128, 255), halfIndex ? 2 : 1);
            }
            if (baseIndex && px >= window_pos.x)
            {
                std::string time_str = ImGuiHelper::MillisecToString(i + _vmin.x, 2);
                ImGui::SetWindowFontScale(0.8);
                draw_list->AddText(ImVec2((float)px + 3.f, window_pos.y), IM_COL32(224, 224, 224, 255), time_str.c_str());
                ImGui::SetWindowFontScale(1.0);
            }
        };
        for (auto i = 0; i < duration; i+= timeStep)
        {
            drawLine(i, timeline_height);
        }
        drawLine(0, timeline_height);
        drawLine(duration, timeline_height);
        // cursor Arrow
        draw_list->PushClipRect(movRect.Min - ImVec2(32, 0), movRect.Max + ImVec2(32, 0));
        const float arrowWidth = draw_list->_Data->FontSize;
        float arrowOffset = window_pos.x + (cursor_pos - vmin.x) * msPixelWidth - arrowWidth * 0.5f;
        ImGui::RenderArrow(draw_list, ImVec2(arrowOffset, window_pos.y), IM_COL32(255, 255, 255, 192), ImGuiDir_Down);
        ImGui::SetWindowFontScale(0.8);
        auto time_str = ImGuiHelper::MillisecToString(cursor_pos, 2);
        ImVec2 str_size = ImGui::CalcTextSize(time_str.c_str(), nullptr, true);
        float strOffset = window_pos.x + (cursor_pos - _vmin.x) * msPixelWidth - str_size.x * 0.5f;
        ImVec2 str_pos = ImVec2(strOffset, window_pos.y + 10);
        draw_list->AddRectFilled(str_pos + ImVec2(-3, 0), str_pos + str_size + ImVec2(3, 3), IM_COL32(128, 128, 128, 144), 2.0, ImDrawFlags_RoundCornersAll);
        draw_list->AddText(str_pos, IM_COL32(255, 255, 255, 255), time_str.c_str());
        ImGui::SetWindowFontScale(1.0);
        draw_list->PopClipRect();
        // draw cursor line
        if (cursor_pos >= _vmin.x && cursor_pos <= _vmax.x)
        {
            auto pt = pointToRange(ImVec2(cursor_pos, 0)) * viewSize + offset;
            draw_list->AddLine(pt - ImVec2(0.5, 0), pt - ImVec2(0.5, edit_size.y), IM_COL32(255, 255, 255, 128), 2);
        }
    }

    // handle zoom and VScroll
    if (flags & CURVE_EDIT_FLAG_SCROLL_V)
    {
        if (editable && container.Contains(io.MousePos))
        {
            if (fabsf(io.MouseWheel) > FLT_EPSILON)
            {
                const float r = (io.MousePos.y - offset.y) / ssize.y;
                float ratioY = ImLerp(vmin.y, vmax.y, r);
                auto scaleValue = [&](float v) {
                    v -= ratioY;
                    v *= (1.f - io.MouseWheel * 0.05f);
                    v += ratioY;
                    return v;
                };
                vmin.y = scaleValue(vmin.y);
                vmax.y = scaleValue(vmax.y);
                delegate->SetMin(vmin);
                delegate->SetMax(vmax);
                curve_changed = true;
            }
            if (!delegate->scrollingV && ImGui::IsMouseDown(2))
            {
                delegate->scrollingV = true;
            }
        }
    }

    if ((flags & CURVE_EDIT_FLAG_SCROLL_V) && delegate->scrollingV && editable)
    {
        float deltaH = io.MouseDelta.y * _range.y * sizeOfPixel.y;
        vmin.y -= deltaH;
        vmax.y -= deltaH;
        if (!ImGui::IsMouseDown(2))
            delegate->scrollingV = false;
        curve_changed = true;
    }

    // draw graticule line
    const float graticule_height = edit_size.y / 10.f;
    for (int i = 0; i <= 10; i++)
    {
        draw_list->AddLine(offset + ImVec2(0, - graticule_height * i), offset + ImVec2(edit_size.x, - graticule_height * i), delegate->GetGraticuleColor(), 1.0f);
    }

    bool overCurveOrPoint = false;
    int localOverCurve = -1;
    int localOverPoint = -1;
    // make sure highlighted curve is rendered last
    std::vector<int> curvesIndex(curveCount);
    for (size_t c = 0; c < curveCount; c++)
        curvesIndex[c] = int(c);
    int highLightedCurveIndex = -1;
    if (delegate->overCurve != -1 && curveCount)
    {
        ImSwap(curvesIndex[delegate->overCurve], curvesIndex[curveCount - 1]);
        highLightedCurveIndex = delegate->overCurve;
    }

    for (size_t cur = 0; cur < curveCount; cur++)
    {
        int c = curvesIndex[cur];
        if (!delegate->IsVisible(c))
            continue;
        const size_t ptCount = delegate->GetCurvePointCount(c);
        if (ptCount < 1)
            continue;
        const KeyPoint* pts = delegate->GetPoints(c);
        ImU32 curveColor = delegate->GetCurveColor(c);
        float curve_width = 1.3f;
        if ((c == highLightedCurveIndex && delegate->selectedPoints.empty() && !delegate->selectingQuad) || delegate->movingCurve == c)
            curve_width = 2.6f;
        for (size_t p = 0; p < ptCount - 1; p++)
        {
            const ImVec2 p1 = pointToRange(pts[p].point);
            const ImVec2 p2 = pointToRange(pts[p + 1].point);
            if (pts[p].point.x > lastTime || pts[p + 1].point.x < firstTime)
                continue;
            CurveType curveType = delegate->GetCurvePointType(c, p + 1);
            size_t subStepCount = distance(pts[p].point.x, pts[p].point.y, pts[p + 1].point.x, pts[p + 1].point.y);
            if (subStepCount <= 1) subStepCount = 100;
            subStepCount = ImClamp(subStepCount, (size_t)10, (size_t)100);
            float step = 1.f / float(subStepCount - 1);
            for (size_t substep = 0; substep < subStepCount - 1; substep++)
            {
                float t = float(substep) * step;
                const ImVec2 sp1 = ImLerp(p1, p2, t);
                const ImVec2 sp2 = ImLerp(p1, p2, t + step);

                const float rt1 = smoothstep(p1.x, p2.x, sp1.x, curveType);
                const float rt2 = smoothstep(p1.x, p2.x, sp2.x, curveType);

                const ImVec2 pos1 = ImVec2(sp1.x, ImLerp(p1.y, p2.y, rt1)) * viewSize + offset;
                const ImVec2 pos2 = ImVec2(sp2.x, ImLerp(p1.y, p2.y, rt2)) * viewSize + offset;

                if (distance(io.MousePos.x, io.MousePos.y, pos1.x, pos1.y, pos2.x, pos2.y) < 8.f)
                {
                    localOverCurve = int(c);
                    delegate->overCurve = int(c);
                }
                draw_list->AddLine(pos1, pos2, curveColor, curve_width);
            } // substep
        }// point loop

        for (size_t p = 0; p < ptCount; p++)
        {
            editPoint point(c, p);
            const int drawState = DrawPoint(draw_list, pointToRange(pts[p].point), viewSize, offset, (delegate->selectedPoints.find(point) != delegate->selectedPoints.end() && delegate->movingCurve == -1/* && !scrollingV*/));
            if (editable && drawState && delegate->movingCurve == -1 && !delegate->selectingQuad)
            {
                overCurveOrPoint = true;
                delegate->overSelectedPoint = true;
                delegate->overCurve = -1;
                if (drawState == 2)
                {
                    if (!io.KeyShift && delegate->selectedPoints.find(point) == delegate->selectedPoints.end())
                        delegate->selectedPoints.clear();
                    delegate->selectedPoints.insert(point);
                }
                localOverPoint = (int)p;
            }
        }
    }  // curves loop

    if (localOverCurve == -1)
        delegate->overCurve = -1;

    // move selection
    if (editable && delegate->overSelectedPoint && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if ((fabsf(io.MouseDelta.x) > 0.f || fabsf(io.MouseDelta.y) > 0.f) && !delegate->selectedPoints.empty())
        {
            if (!delegate->pointsMoved)
            {
                delegate->BeginEdit(0);
                delegate->mousePosOrigin = io.MousePos;
                delegate->originalPoints.resize(delegate->selectedPoints.size());
                int index = 0;
                for (auto& sel : delegate->selectedPoints)
                {
                    const KeyPoint* pts = delegate->GetPoints(sel.curveIndex);
                    delegate->originalPoints[index++] = pts[sel.pointIndex];
                }
            }
            delegate->pointsMoved = true;
            hold = true;
            auto prevSelection = delegate->selectedPoints;
            int originalIndex = 0;
            for (auto& sel : prevSelection)
            {
                const size_t ptCount = delegate->GetCurvePointCount(sel.curveIndex);
                auto value_range = fabs(delegate->GetCurveMax(sel.curveIndex) - delegate->GetCurveMin(sel.curveIndex)); 
                ImVec2 p = rangeToPoint(pointToRange(delegate->originalPoints[originalIndex].point) + (io.MousePos - delegate->mousePosOrigin) * sizeOfPixel);
                delegate->AlignValue(p);
                if (flags & CURVE_EDIT_FLAG_VALUE_LIMITED)
                {
                    if (p.x < vmin.x) p.x = vmin.x;
                    if (p.y < vmin.y) p.y = vmin.y;
                    if (p.x > vmax.x) p.x = vmax.x;
                    if (p.y > vmax.y) p.y = vmax.y;
                }
                if (flags & CURVE_EDIT_FLAG_DOCK_BEGIN_END)
                {
                    if (sel.pointIndex == 0)
                    {
                        p.x = vmin.x;
                    }
                    else if (sel.pointIndex == ptCount - 1)
                    {
                        p.x = vmax.x;
                    }
                }
                const CurveType t = delegate->originalPoints[originalIndex].type;
                p.y = (p.y * value_range) + delegate->GetCurveMin(sel.curveIndex);
                const int newIndex = delegate->EditPoint(sel.curveIndex, sel.pointIndex, p, t);
                if (localOverPoint == -1 && ImGui::BeginTooltip())
                {
                    ImGui::Text("%.2f", p.y);
                    ImGui::EndTooltip();
                }
                if (newIndex != sel.pointIndex)
                {
                    delegate->selectedPoints.erase(sel);
                    delegate->selectedPoints.insert({ sel.curveIndex, newIndex });
                }
                originalIndex++;
            }
            curve_changed = true;
        }
    }

    if (delegate->overSelectedPoint && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        delegate->overSelectedPoint = false;
        if (delegate->pointsMoved)
        {
            delegate->pointsMoved = false;
            delegate->EndEdit();
        }
    }

    // add point with double left click 
    if (editable && delegate->overCurve != -1 && io.MouseDoubleClicked[0])
    {
        ImVec2 np = rangeToPoint((io.MousePos - offset) / viewSize);
        const CurveType t = delegate->GetCurveType(delegate->overCurve);
        auto value_range = delegate->GetCurveMax(delegate->overCurve) - delegate->GetCurveMin(delegate->overCurve); 
        auto point_value = delegate->GetValue(delegate->overCurve, np.x);
        point_value = (point_value - delegate->GetCurveMin(delegate->overCurve)) / (value_range + FLT_EPSILON);
        delegate->AlignValue(np);
        delegate->BeginEdit(delegate->overCurve);
        delegate->AddPoint(delegate->overCurve, ImVec2(np.x, point_value), t);
        delegate->EndEdit();
        curve_changed = true;
    }

    // draw value in tooltip
    if (editable && localOverCurve != -1 && localOverPoint != -1 && ImGui::BeginTooltip())
    {
        auto value_range = fabs(delegate->GetCurveMax(localOverCurve) - delegate->GetCurveMin(localOverCurve)); 
        const KeyPoint* pts = delegate->GetPoints(localOverCurve);
        const ImVec2 p = pointToRange(pts[localOverPoint].point);
        ImGui::Text("%.2f", p.y * value_range + delegate->GetCurveMin(localOverCurve));
        if (localOverPoint != 0 && localOverPoint != delegate->GetCurvePointCount(localOverCurve) - 1)
            ImGui::TextUnformatted("Delete key with L-shift and L-click");
        ImGui::EndTooltip();
    }

    // delete point with right click
    if (editable && localOverCurve !=-1 && localOverPoint != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && editable && bEnableDelete)
    {
        bool deletable = true;
        if (flags & CURVE_EDIT_FLAG_KEEP_BEGIN_END)
        {
            const size_t ptCount = delegate->GetCurvePointCount(localOverCurve);
            if (localOverPoint == 0 || localOverPoint == ptCount - 1)
                deletable = false;
        }
        if (deletable)
        {
            delegate->BeginEdit(localOverCurve);
            delegate->DeletePoint(localOverCurve, localOverPoint);
            delegate->EndEdit();
            auto selected_point = std::find_if(delegate->selectedPoints.begin(), delegate->selectedPoints.end(), [&](const editPoint& point) {
                return point.curveIndex == localOverCurve && point.pointIndex == localOverPoint;
            });
            if (selected_point != delegate->selectedPoints.end())
                delegate->selectedPoints.erase(selected_point);
            curve_changed = true;
        }
    }

    // move curve
    if (editable && (flags & CURVE_EDIT_FLAG_MOVE_CURVE))
    {
        if (delegate->movingCurve != -1)
        {
            auto value_range = fabs(delegate->GetCurveMax(delegate->movingCurve) - delegate->GetCurveMin(delegate->movingCurve)); 
            const size_t ptCount = delegate->GetCurvePointCount(delegate->movingCurve);
            const KeyPoint* pts = delegate->GetPoints(delegate->movingCurve);
            if (!delegate->pointsMoved)
            {
                delegate->mousePosOrigin = io.MousePos;
                delegate->pointsMoved = true;
                delegate->originalPoints.resize(ptCount);
                for (size_t index = 0; index < ptCount; index++)
                {
                    delegate->originalPoints[index] = pts[index];
                }
            }
            if (ptCount >= 1)
            {
                for (size_t p = 0; p < ptCount; p++)
                {
                    ImVec2 pt = rangeToPoint(pointToRange(delegate->originalPoints[p].point) + (io.MousePos - delegate->mousePosOrigin) * sizeOfPixel);
                    if (flags & CURVE_EDIT_FLAG_VALUE_LIMITED)
                    {
                        if (pt.x < vmin.x) pt.x = vmin.x;
                        if (pt.y < vmin.y) pt.y = vmin.y;
                        if (pt.x > vmax.x) pt.x = vmax.x;
                        if (pt.y > vmax.y) pt.y = vmax.y;
                    }
                    if (flags & CURVE_EDIT_FLAG_DOCK_BEGIN_END)
                    {
                        if (p == 0)
                        {
                            pt.x = vmin.x;
                        }
                        else if (p == ptCount - 1)
                        {
                            pt.x = vmax.x;
                        }
                    }
                    pt.y = pt.y * value_range + delegate->GetCurveMin(delegate->movingCurve);
                    delegate->EditPoint(delegate->movingCurve, int(p), pt, delegate->originalPoints[p].type);
                }
                hold = true;
                curve_changed = true;
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                delegate->movingCurve = -1;
                delegate->pointsMoved = false;
                delegate->EndEdit();
            }
        }
        if (delegate->movingCurve == -1 && delegate->overCurve != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !delegate->selectingQuad)
        {
            delegate->movingCurve = delegate->overCurve;
            delegate->BeginEdit(delegate->overCurve);
        }
    }

    // quad selection
    if (editable && delegate->selectingQuad)
    {
        const ImVec2 bmin = ImMin(delegate->quadSelection, io.MousePos);
        const ImVec2 bmax = ImMax(delegate->quadSelection, io.MousePos);
        draw_list->AddRectFilled(bmin, bmax, IM_COL32(255, 255, 0, 64), 1.f);
        draw_list->AddRect(bmin, bmax, IM_COL32(255,255,0,255), 1.f);
        const ImRect selectionQuad(bmin, bmax);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (!io.KeyShift)
                delegate->selectedPoints.clear();
            // select everythnig is quad
            for (size_t c = 0; c < curveCount; c++)
            {
                if (!delegate->IsVisible(c))
                    continue;

                const size_t ptCount = delegate->GetCurvePointCount(c);
                if (ptCount < 1)
                    continue;

                const KeyPoint* pts = delegate->GetPoints(c);
                for (size_t p = 0; p < ptCount; p++)
                {
                    const ImVec2 center = pointToRange(pts[p].point) * viewSize + offset;
                    if (selectionQuad.Contains(center))
                        delegate->selectedPoints.insert({ int(c), int(p) });
                }
            }
            // done
            delegate->selectingQuad = false;
        }
    }
    if (editable && !overCurveOrPoint && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !delegate->selectingQuad && delegate->movingCurve == -1 && !delegate->overSelectedPoint && container.Contains(io.MousePos))
    {
        delegate->selectingQuad = true;
        delegate->quadSelection = io.MousePos;
    }

    if (clippingRect)
        draw_list->PopClipRect();

    ImGui::EndChildFrame();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    if (changed)
        *changed = curve_changed;
    return hold;
}

bool ImGui::ImCurveEdit::Edit(ImDrawList* draw_list, Delegate* delegate, const ImVec2& size, unsigned int id, bool editable, float& cursor_pos, unsigned int flags, const ImRect* clippingRect, bool * changed)
{
    float firstTime = 0;
    float lastTime = delegate ? delegate->GetMax().x : 0;
    return ImGui::ImCurveEdit::Edit(draw_list, delegate, size, id, editable, cursor_pos, firstTime, lastTime, flags, clippingRect, changed);
}

ImGui::KeyPointEditor& ImGui::KeyPointEditor::operator=(const ImGui::KeyPointEditor& keypoint)
{
    mKeys.clear();
    for (auto curve : keypoint.mKeys)
    {
        auto curve_index = AddCurve(curve.name, curve.type, curve.color, curve.visible, curve.m_min, curve.m_max, curve.m_default);
        for (auto p : curve.points)
        {
            AddPoint(curve_index, p.point, p.type);
        }
    }
    mMin = keypoint.mMin;
    mMax = keypoint.mMax;
    BackgroundColor = keypoint.BackgroundColor;
    GraticuleColor = keypoint.GraticuleColor;
    return *this;
}

void ImGui::KeyPointEditor::Load(const imgui_json::value& keypoint)
{
    if (!keypoint.is_object())
        return;
    // keypoint global
    if (keypoint.contains("Min"))
    {
        auto& val = keypoint["Min"];
        if (val.is_vec2()) SetMin(val.get<imgui_json::vec2>());
    }
    if (keypoint.contains("Max"))
    {
        auto& val = keypoint["Max"];
        if (val.is_vec2()) SetMax(val.get<imgui_json::vec2>());
    }

    // Clear Curves
    Clear();

    // keypoint curve
    const imgui_json::array* curveArray = nullptr;
    if (imgui_json::GetPtrTo(keypoint, "Curves", curveArray))
    {
        for (auto& curve : *curveArray)
        {
            if (!curve.is_object()) continue;
            std::string name = "";
            int type = -1;
            ImU32 color = 0;
            bool visible = false;
            float _min = 0.f;
            float _max = 0.f;
            float _default = 0.f;
            int64_t _id = -1;
            int64_t _sub_id = -1;
            if (curve.contains("Name"))
            {
                auto& val = curve["Name"];
                if (val.is_string()) name = val.get<imgui_json::string>();
            }
            if (curve.contains("Type"))
            {
                auto& val = curve["Type"];
                if (val.is_number()) type = val.get<imgui_json::number>();
            }
            if (curve.contains("Color"))
            {
                auto& val = curve["Color"];
                if (val.is_number()) color = val.get<imgui_json::number>();
            }
            if (curve.contains("Visible"))
            {
                auto& val = curve["Visible"];
                if (val.is_boolean()) visible = val.get<imgui_json::boolean>();
            }
            if (curve.contains("Min"))
            {
                auto& val = curve["Min"];
                if (val.is_number()) _min = val.get<imgui_json::number>();
            }
            if (curve.contains("Max"))
            {
                auto& val = curve["Max"];
                if (val.is_number()) _max = val.get<imgui_json::number>();
            }
            if (curve.contains("Default"))
            {
                auto& val = curve["Default"];
                if (val.is_number()) _default = val.get<imgui_json::number>();
            }
            if (curve.contains("ID"))
            {
                auto& val = curve["ID"];
                if (val.is_number()) _id = val.get<imgui_json::number>();
            }
            if (curve.contains("SubID"))
            {
                auto& val = curve["SubID"];
                if (val.is_number()) _sub_id = val.get<imgui_json::number>();
            }
            if (!name.empty())
            {
                auto curve_index = AddCurve(name, (ImGui::ImCurveEdit::CurveType)type, color, visible, _min, _max, _default, _id, _sub_id);
                const imgui_json::array* pointArray = nullptr;
                if (imgui_json::GetPtrTo(curve, "KeyPoints", pointArray))
                {
                    for (auto& point : *pointArray)
                    {
                        if (!point.is_object()) continue;
                        ImGui::ImCurveEdit::KeyPoint p;
                        if (point.contains("Point"))
                        {
                            auto& val = point["Point"];
                            if (val.is_vec2()) p.point = val.get<imgui_json::vec2>();
                        }
                        if (point.contains("Type"))
                        {
                            auto& val = point["Type"];
                            if (val.is_number()) p.type = (ImGui::ImCurveEdit::CurveType)val.get<imgui_json::number>();
                        }
                        if (p.type != ImGui::ImCurveEdit::CurveType::UnKnown)
                        {
                            AddPoint(curve_index, p.point, p.type);
                        }
                    }
                }
            }
        }
    }
}

void ImGui::KeyPointEditor::Save(imgui_json::value& keypoint)
{
    keypoint["Min"] = imgui_json::vec2(GetMin());
    keypoint["Max"] = imgui_json::vec2(GetMax());
    imgui_json::value curves;
    for (int i = 0; i < GetCurveCount(); i++)
    {
        imgui_json::value curve;
        curve["Name"] = GetCurveName(i);
        curve["Type"] = imgui_json::number(GetCurveType(i));
        curve["Color"] = imgui_json::number(GetCurveColor(i));
        curve["Visible"] = imgui_json::boolean(IsVisible(i));
        curve["Min"] = imgui_json::number(GetCurveMin(i));
        curve["Max"] = imgui_json::number(GetCurveMax(i));
        curve["Default"] = imgui_json::number(GetCurveDefault(i));
        curve["ID"] = imgui_json::number(GetCurveID(i));
        curve["SubID"] = imgui_json::number(GetCurveSubID(i));
        // save curve key point
        imgui_json::value points;
        for (int p = 0; p < GetCurvePointCount(i); p++)
        {
            imgui_json::value point;
            auto pt = mKeys[i].points[p];
            point["Point"] = imgui_json::vec2(pt.point);
            point["Type"] = imgui_json::number(pt.type);
            points.push_back(point);
        }
        curve["KeyPoints"] = points;
        curves.push_back(curve);
    }
    keypoint["Curves"] = curves;
}

ImGui::ImCurveEdit::CurveType ImGui::KeyPointEditor::GetCurvePointType(size_t curveIndex, size_t point) const 
{
    if (curveIndex < mKeys.size())
    {
        if (point < mKeys[curveIndex].points.size())
        {
            return mKeys[curveIndex].points[point].type;
        }
    }
    return ImCurveEdit::CurveType::Hold;
}

ImGui::ImCurveEdit::KeyPoint ImGui::KeyPointEditor::GetPoint(size_t curveIndex, size_t pointIndex)
{
    if (curveIndex < mKeys.size())
    {
        if (pointIndex < mKeys[curveIndex].points.size())
        {
            auto value_range = fabs(GetCurveMax(curveIndex) - GetCurveMin(curveIndex)); 
            ImCurveEdit::KeyPoint point = mKeys[curveIndex].points[pointIndex];
            point.point.y = point.point.y * value_range + GetCurveMin(curveIndex);
            return point;
        }
    }
    return {};
}

int ImGui::KeyPointEditor::EditPoint(size_t curveIndex, size_t pointIndex, ImVec2 value, ImCurveEdit::CurveType type)
{
    if (curveIndex < mKeys.size())
    {
        if (pointIndex < mKeys[curveIndex].points.size())
        {
            auto value_range = fabs(GetCurveMax(curveIndex) - GetCurveMin(curveIndex)); 
            auto point_value = (value.y - GetCurveMin(curveIndex)) / (value_range + FLT_EPSILON);
            mKeys[curveIndex].points[pointIndex] = {ImVec2(value.x, point_value), type};
            SortValues(curveIndex);
            for (size_t i = 0; i < GetCurvePointCount(curveIndex); i++)
            {
                if (mKeys[curveIndex].points[i].point.x == value.x)
                    return (int)i;
            }
        }
    }
    return -1;
}

void ImGui::KeyPointEditor::AlignValue(ImVec2& value)
{
    if (mAlign.x > 0)
    {
        int64_t value_x = value.x;
        int64_t index = (int64_t)floor((double)value_x / mAlign.x);
        value_x = index * mAlign.x;
        value.x = value_x;
    }
    if (mAlign.y > 0)
    {
        int64_t value_y = value.y;
        int64_t index = (int64_t)floor((double)value_y / mAlign.y);
        value_y = index * mAlign.y;
        value.y = value_y;
    }
}

void ImGui::KeyPointEditor::AddPoint(size_t curveIndex, ImVec2 value, ImCurveEdit::CurveType type)
{
    if (curveIndex < mKeys.size())
    {
        auto iter = std::find_if(mKeys[curveIndex].points.begin(), mKeys[curveIndex].points.end(), [value](const ImGui::ImCurveEdit::KeyPoint item)
        {
            return value.x == item.point.x;
        });
        if (iter == mKeys[curveIndex].points.end())
        {
            mKeys[curveIndex].points.push_back({ImVec2(value.x, value.y), type});
            SortValues(curveIndex);
        }
    }
}

void ImGui::KeyPointEditor::ClearPoint(size_t curveIndex)
{
    if (curveIndex < mKeys.size())
    {
        mKeys[curveIndex].points.clear();
    }
}

void ImGui::KeyPointEditor::DeletePoint(size_t curveIndex, size_t pointIndex)
{
    if (curveIndex < mKeys.size())
    {
        if (pointIndex < mKeys[curveIndex].points.size())
        {
            auto iter = mKeys[curveIndex].points.begin() + pointIndex;
            mKeys[curveIndex].points.erase(iter);
        }
    }
}

int ImGui::KeyPointEditor::AddCurve(std::string name, ImCurveEdit::CurveType type, ImU32 color, bool visible, float _min, float _max, float _default, int64_t _id, int64_t _sub_id)
{
    auto new_key = ImCurveEdit::keys(name, type, color, visible, _min, _max, _default);
    new_key.m_id = _id;
    new_key.m_sub_id = _sub_id;
    mKeys.push_back(new_key);
    return mKeys.size() - 1;
}

void ImGui::KeyPointEditor::DeleteCurve(size_t curveIndex)
{
    if (curveIndex < mKeys.size())
    {
        auto iter = mKeys.begin() + curveIndex;
        mKeys.erase(iter);
    }
}

void ImGui::KeyPointEditor::DeleteCurve(std::string name)
{
    int index = GetCurveIndex(name);
    if (index != -1)
    {
        DeleteCurve(index);
    }
}

int ImGui::KeyPointEditor::GetCurveIndex(std::string name)
{
    int index = -1;
    auto iter = std::find_if(mKeys.begin(), mKeys.end(), [name](const ImCurveEdit::keys& key)
    {
        return key.name == name;
    });
    if (iter != mKeys.end())
    {
        index = iter - mKeys.begin();
    }
    return index;
}

int ImGui::KeyPointEditor::GetCurveIndex(int64_t id)
{
    int index = -1;
    auto iter = std::find_if(mKeys.begin(), mKeys.end(), [id](const ImCurveEdit::keys& key)
    {
        return key.m_id != -1 && key.m_id == id;
    });
    if (iter != mKeys.end())
    {
        index = iter - mKeys.begin();
    }
    return index;
}

const ImGui::ImCurveEdit::keys* ImGui::KeyPointEditor::GetCurveKey(std::string name)
{
    int index = -1;
    auto iter = std::find_if(mKeys.begin(), mKeys.end(), [name](const ImCurveEdit::keys& key)
    {
        return key.name == name;
    });
    if (iter != mKeys.end())
    {
        index = iter - mKeys.begin();
    }
    if (index == -1)
        return nullptr;
    return &mKeys[index];
}

const ImGui::ImCurveEdit::keys* ImGui::KeyPointEditor::GetCurveKey(size_t curveIndex)
{
    if (curveIndex < mKeys.size())
        return &mKeys[curveIndex];
    return nullptr;
}

float ImGui::KeyPointEditor::GetPointValue(size_t curveIndex, float t)
{
    auto value_range = GetCurveMax(curveIndex) - GetCurveMin(curveIndex); 
    auto value = GetValue(curveIndex, t);
    value = (value - GetCurveMin(curveIndex)) / (value_range + FLT_EPSILON);
    return value;
}

float ImGui::KeyPointEditor::GetValue(size_t curveIndex, float t)
{
    if (curveIndex < mKeys.size())
    {
        auto range = GetMax() - GetMin() + ImVec2(1.f, 0.f); 
        auto value_range = fabs(GetCurveMax(curveIndex) - GetCurveMin(curveIndex)); 
        auto pointToRange = [&](ImVec2 pt) { return (pt - GetMin()) / range; };
        const size_t ptCount = GetCurvePointCount(curveIndex);
        if (ptCount <= 0)
            return 0;
        const ImCurveEdit::KeyPoint* pts = GetPoints(curveIndex);
        if (ptCount <= 1)
            return pointToRange(pts[0].point).x;
        int found_index = -1;
        for (int i = 0; i < ptCount - 1; i++)
        {
            if (t >= pts[i].point.x && t <= pts[i + 1].point.x)
            {
                found_index = i;
                break;
            }
        }
        if (found_index != -1)
        {
            const ImVec2 p1 = pointToRange(pts[found_index].point);
            const ImVec2 p2 = pointToRange(pts[found_index + 1].point);
            float x = (t - pts[found_index].point.x) / (pts[found_index + 1].point.x - pts[found_index].point.x);
            const ImVec2 sp = ImLerp(p1, p2, x);
            ImCurveEdit::CurveType type = (t - pts[found_index].point.x) < (pts[found_index + 1].point.x - t) ? pts[found_index].type : pts[found_index + 1].type;
            const float rt = ImCurveEdit::smoothstep(p1.x, p2.x, sp.x, type);
            const float v = ImLerp(p1.y, p2.y, rt);
            return v * value_range + GetCurveMin(curveIndex);
        }
    }
    return 0;
}

void ImGui::KeyPointEditor::SetCurvePointDefault(size_t curveIndex, size_t pointIndex)
{
    if (curveIndex < mKeys.size())
    {
        if (pointIndex < mKeys[curveIndex].points.size())
        {
            auto value_range = fabs(GetCurveMax(curveIndex) - GetCurveMin(curveIndex)); 
            auto value_default = GetCurveDefault(curveIndex);
            value_default = (value_default - GetCurveMin(curveIndex)) / (value_range + FLT_EPSILON);
            mKeys[curveIndex].points[pointIndex].point.y = value_default;
            mKeys[curveIndex].points[pointIndex].type = GetCurveType(curveIndex);
        }
    }
}

void ImGui::KeyPointEditor::MoveTo(float x)
{
    float offset = x - mMin.x;
    float length = fabs(mMax.x - mMin.x);
    mMin.x = x;
    mMax.x = mMin.x + length;
    for (size_t i = 0; i < mKeys.size(); i++)
    {
        for (auto iter = mKeys[i].points.begin(); iter != mKeys[i].points.end(); iter++)
        {
            iter->point.x += offset;
        }
    }
}

void ImGui::KeyPointEditor::SetMin(ImVec2 vmin, bool dock)
{
    if (dock)
    {
        for (size_t i = 0; i < mKeys.size(); i++)
        {
            // first get current begin value
            auto value = GetPointValue(i, mMin.x);
            if (vmin.x > mMin.x) value = GetPointValue(i, vmin.x);
            // second delete out of range points, keep begin end
            if (mKeys[i].points.size() > 2)
            {
                for (auto iter = mKeys[i].points.begin() + 1; iter != mKeys[i].points.end() - 1;)
                {
                    if (iter->point.x < vmin.x || iter->point.y < vmin.y)
                    {
                        iter = mKeys[i].points.erase(iter);
                    }
                    else
                        ++iter;
                }
            }
            // finanl reset begin point
            if (mKeys[i].points.size() > 0)
            {
                auto start_iter = mKeys[i].points.begin();
                if (start_iter != mKeys[i].points.end()) 
                {
                    start_iter->point.x = vmin.x;
                    start_iter->point.y = value;
                }
            }
        }
    }
    mMin = vmin;
}

void ImGui::KeyPointEditor::SetMax(ImVec2 vmax, bool dock)
{
    if (dock)
    {
        for (size_t i = 0; i < mKeys.size(); i++)
        {
            // first get current begin value
            auto value = GetPointValue(i, mMax.x);
            if (vmax.x < mMax.x) value = GetPointValue(i, vmax.x);
            // second delete out of range points, keep begin end
            if (mKeys[i].points.size() > 2)
            {
                for (auto iter = mKeys[i].points.begin() + 1; iter != mKeys[i].points.end() - 1;)
                {
                    if (iter->point.x > vmax.x)
                    {
                        iter = mKeys[i].points.erase(iter);
                    }
                    else
                        ++iter;
                }
            }
            // finanl reset begin end point
            if (mKeys[i].points.size() > 0)
            {
                auto end_iter = mKeys[i].points.begin() + mKeys[i].points.size() - 1;
                if (end_iter != mKeys[i].points.end()) 
                {
                    end_iter->point.x = vmax.x;
                    end_iter->point.y = value;
                }
            }
        }
    }
    mMax = vmax;
}

ImVec2 ImGui::KeyPointEditor::GetPrevPoint(float pos)
{
    ImVec2 point(0, 0);
    const size_t curveCount = GetCurveCount();
    float min_pos = FLT_MAX;
    for (size_t cur = 0; cur < curveCount; cur++)
    {
        const size_t ptCount = GetCurvePointCount(cur);
        const ImCurveEdit::KeyPoint* pts = GetPoints(cur);
        for (size_t p = ptCount - 1; p >= 0; p--)
        {
            const ImVec2 p1 = pts[p].point;
            if (p1.x > pos)
                continue;
            if (p1.x == pos && p != 0)
                continue;
            if (p1.x < min_pos)
            {
                min_pos = p1.x;
                point.x = min_pos;
                break;
            }
        }
    }
    return point;
}

ImVec2 ImGui::KeyPointEditor::GetNextPoint(float pos)
{
    ImVec2 point(0, 0);
    const size_t curveCount = GetCurveCount();
    float max_pos = FLT_MIN;
    for (size_t cur = 0; cur < curveCount; cur++)
    {
        const size_t ptCount = GetCurvePointCount(cur);
        const ImCurveEdit::KeyPoint* pts = GetPoints(cur);
        for (size_t p = 0; p < ptCount; p++)
        {
            const ImVec2 p1 = pts[p].point;
            if (p1.x < pos)
                continue;
            if (p1.x == pos && p != ptCount - 1)
                continue;
            if (p1.x > max_pos)
            {
                max_pos = p1.x;
                point.x = max_pos;
                break;
            }
        }
    }
    return point;
}

void ImGui::KeyPointEditor::SortValues(size_t curveIndex)
{
    if (curveIndex < mKeys.size())
    {
        auto b = std::begin(mKeys[curveIndex].points);
        auto e = std::begin(mKeys[curveIndex].points) + GetCurvePointCount(curveIndex);
        std::sort(b, e, [](ImCurveEdit::KeyPoint a, ImCurveEdit::KeyPoint b) { return a.point.x < b.point.x; });
    }
}

bool ImGui::ImCurveEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, std::string name, float _min, float _max, float _default, float space)
{
    if (!key || name.empty() || button_lable.empty() || key->m_id == -1)
        return false;
    ImGui::SameLine(space);
    std::string button_id_str = button_lable + "@" + std::to_string(key->m_id);
    std::string key_name = name + "@" + std::to_string(key->m_id);
    if (ImGui::DiamondButton(button_id_str.c_str(), false)) 
    {
        key->name = key_name;
        key->m_min = _min;
        key->m_max = _max;
        key->m_default = _default;
        return true;
    }
    ImGui::ShowTooltipOnHover("Add Curve");
    return false;
}

bool ImGui::ImCurveCheckEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, bool &check, std::string name, float _min, float _max, float _default, float space)
{
    if (!key || name.empty() || button_lable.empty() || key->m_id == -1)
        return false;
    ImGui::SameLine(space);
    std::string button_id_str = button_lable + "@" + std::to_string(key->m_id);
    std::string key_name = name + "@" + std::to_string(key->m_id);
    if (check)
    {
        if (ImGui::DiamondButton(button_id_str.c_str(), true)) 
        {
            check = false;
            return true;
        }
        ImGui::ShowTooltipOnHover("Remove Curve");
    }
    else
    {
        if (ImGui::DiamondButton(button_id_str.c_str(), false)) 
        {
            key->name = key_name;
            key->m_min = _min;
            key->m_max = _max;
            key->m_default = _default;
            check = true;
            return true;
        }
        ImGui::ShowTooltipOnHover("Add Curve");
    }
    return false;
}

bool ImGui::ImCurveCheckEditKeyWithID(std::string button_lable, ImGui::ImCurveEdit::keys * key, bool check, std::string name, float _min, float _max, float _default, int64_t subid, float space)
{
    if (!key || name.empty() || button_lable.empty() || key->m_id == -1)
        return false;
    ImGui::SameLine(space);
    std::string button_id_str = button_lable + "@" + std::to_string(key->m_id);
    std::string key_name = name + "@" + std::to_string(key->m_id);
    if (check)
    {
        if (ImGui::DiamondButton(button_id_str.c_str(), true)) 
        {
            key->name = key_name;
            key->m_sub_id = subid;
            key->checked = false;
            return true;
        }
        ImGui::ShowTooltipOnHover("Remove Curve");
    }
    else
    {
        if (ImGui::DiamondButton(button_id_str.c_str(), false)) 
        {
            key->name = key_name;
            key->m_min = _min;
            key->m_max = _max;
            key->m_default = _default;
            key->m_sub_id = subid;
            key->checked = true;
            return true;
        }
        ImGui::ShowTooltipOnHover("Add Curve");
    }
    return false;
}
