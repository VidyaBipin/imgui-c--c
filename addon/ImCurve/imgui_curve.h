#ifndef IMGUI_CURVE_H
#define IMGUI_CURVE_H

#include <functional>
#include <vector>
#include <algorithm>
#include <set>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_json.h>
#include <imgui_helper.h>
#include <imgui_extra_widget.h>

// CurveEdit from https://github.com/CedricGuillemet/ImGuizmo
namespace ImGui
{
struct IMGUI_API ImCurveEdit
{
#define CURVE_EDIT_FLAG_NONE            (0)
#define CURVE_EDIT_FLAG_VALUE_LIMITED   (1)
#define CURVE_EDIT_FLAG_SCROLL_V        (1<<1)
#define CURVE_EDIT_FLAG_MOVE_CURVE      (1<<2)
#define CURVE_EDIT_FLAG_KEEP_BEGIN_END  (1<<3)
#define CURVE_EDIT_FLAG_DOCK_BEGIN_END  (1<<4)
#define CURVE_EDIT_FLAG_DRAW_TIMELINE   (1<<5)

    enum CurveType
    {
        UnKnown = -1,
        Hold,
        Step,
        Linear,
        Smooth,
        QuadIn,
        QuadOut,
        QuadInOut,
        CubicIn,
        CubicOut,
        CubicInOut,
        SineIn,
        SineOut,
        SineInOut,
        ExpIn,
        ExpOut,
        ExpInOut,
        CircIn,
        CircOut,
        CircInOut,
        ElasticIn,
        ElasticOut,
        ElasticInOut,
        BackIn,
        BackOut,
        BackInOut,
        BounceIn,
        BounceOut,
        BounceInOut
    };

    struct KeyPoint
    {
        ImVec2 point {0, 0};
        ImCurveEdit::CurveType type {UnKnown};
    };

    struct keys
    {
        keys() {};
        keys(std::string _name, ImGui::ImCurveEdit::CurveType _type, ImU32 _color, bool _visible, float _min, float _max, float _default)
            : type(_type), name(_name), color(_color), visible(_visible), m_min(_min), m_max(_max), m_default(_default) {};
        std::vector<ImGui::ImCurveEdit::KeyPoint> points;
        ImGui::ImCurveEdit::CurveType type {Smooth};
        std::string name;
        ImU32 color;
        float m_min {0.f};
        float m_max {0.f};
        float m_default {0.f};
        bool visible {true};
        int64_t m_id {-1};
    };

    struct editPoint
    {
        int curveIndex;
        int pointIndex;
        editPoint(int c, int p)
        {
            curveIndex = c;
            pointIndex = p;
        }
        bool operator <(const editPoint& other) const
        {
            if (curveIndex < other.curveIndex)
                return true;
            if (curveIndex > other.curveIndex)
                return false;
            if (pointIndex < other.pointIndex)
                return true;
            return false;
        }
    };
    struct Delegate
    {
        bool focused = false;
        bool selectingQuad {false};
        ImVec2 quadSelection {0, 0};
        int overCurve {-1};
        int movingCurve {-1};
        bool scrollingV {false};
        bool overSelectedPoint {false};
        std::set<ImCurveEdit::editPoint> selectedPoints;
        bool MovingCurrentTime {false};
        bool pointsMoved {false};
        ImVec2 mousePosOrigin;
        std::vector<ImCurveEdit::KeyPoint> originalPoints;
        
        virtual void Clear() = 0;
        virtual size_t GetCurveCount() = 0;
        virtual bool IsVisible(size_t /*curveIndex*/) { return true; }
        virtual CurveType GetCurveType(size_t /*curveIndex*/) const { return Linear; }
        virtual CurveType GetCurvePointType(size_t /*curveIndex*/, size_t /*pointIndex*/) const { return Linear; }
        virtual ImVec2& GetMin() = 0;
        virtual ImVec2& GetMax() = 0;
        virtual void SetMin(ImVec2 vmin, bool dock = false) = 0;
        virtual void SetMax(ImVec2 vmax, bool dock = false) = 0;
        virtual void MoveTo(float x) = 0;
        virtual void SetRangeX(float _min, float _max, bool dock = false) = 0;
        virtual size_t GetCurvePointCount(size_t curveIndex) = 0;
        virtual ImU32 GetCurveColor(size_t curveIndex) = 0;
        virtual std::string GetCurveName(size_t curveIndex) = 0;
        virtual KeyPoint* GetPoints(size_t curveIndex) = 0;
        virtual KeyPoint GetPoint(size_t curveIndex, size_t pointIndex) = 0;
        virtual void AlignValue(ImVec2& value) = 0;
        virtual int EditPoint(size_t curveIndex, size_t pointIndex, ImVec2 value, CurveType type) = 0;
        virtual void AddPoint(size_t curveIndex, ImVec2 value, CurveType type) = 0;
        virtual float GetValue(size_t curveIndex, float t) = 0;
        virtual float GetPointValue(size_t curveIndex, float t) = 0;
        virtual void ClearPoint(size_t curveIndex) = 0;
        virtual void DeletePoint(size_t curveIndex, size_t pointIndex) = 0;
        virtual int AddCurve(std::string name, CurveType type, ImU32 color, bool visible, float _min, float _max, float _default) = 0;
        virtual void DeleteCurve(size_t curveIndex) = 0;
        virtual void DeleteCurve(std::string name) = 0;
        virtual int GetCurveIndex(std::string name) = 0;
        virtual void SetCurveColor(size_t curveIndex, ImU32 color) = 0;
        virtual void SetCurveName(size_t curveIndex, std::string name) = 0;
        virtual void SetCurveVisible(size_t curveIndex, bool visible) = 0;
        virtual float GetCurveMin(size_t curveIndex) = 0;
        virtual float GetCurveMax(size_t curveIndex) = 0;
        virtual float GetCurveDefault(size_t curveIndex) = 0;
        virtual void SetCurveMin(size_t curveIndex, float _min) = 0;
        virtual void SetCurveMax(size_t curveIndex, float _max) = 0;
        virtual void SetCurveDefault(size_t curveIndex, float _default) = 0;
        virtual void SetCurvePointDefault(size_t curveIndex, size_t pointIndex) = 0;

        virtual ImU32 GetBackgroundColor() { return 0xFF101010; }
        virtual ImU32 GetGraticuleColor() { return 0xFF202020; }
        virtual void SetBackgroundColor(ImU32 color) = 0;
        virtual void SetGraticuleColor(ImU32 color) = 0;
        // TODO::Dicky handle undo/redo thru this functions
        virtual void BeginEdit(int /*index*/) {}
        virtual void EndEdit() {}

        virtual ~Delegate() = default;
    };
private:
    static int DrawPoint(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited);
public:
    static int GetCurveTypeName(char**& list);
    static float smoothstep(float edge0, float edge1, float t, CurveType type);
    static float distance(float x1, float y1, float x2, float y2);
    static float distance(float x, float y, float x1, float y1, float x2, float y2);
    static bool Edit(ImDrawList* draw_list, Delegate& delegate, const ImVec2& size, unsigned int id, float& cursor_pos, unsigned int flags = CURVE_EDIT_FLAG_NONE, const ImRect* clippingRect = NULL, bool * changed = nullptr);
    static bool Edit(ImDrawList* draw_list, Delegate& delegate, const ImVec2& size, unsigned int id, 
                    float& cursor_pos, float firstTime, float lastTime, float visibleTime, float msPixelWidthTarget,
                    unsigned int flags = CURVE_EDIT_FLAG_NONE, const ImRect* clippingRect = NULL, bool * changed = nullptr);
};

struct IMGUI_API KeyPointEditor : public ImCurveEdit::Delegate
{
    KeyPointEditor() {}
    KeyPointEditor(ImU32 bg_color, ImU32 gr_color) 
        : BackgroundColor(bg_color), GraticuleColor(gr_color)
    {}
    ~KeyPointEditor() { mKeys.clear(); }

    void Clear() { mKeys.clear(); }

    KeyPointEditor& operator=(const KeyPointEditor& keypoint);

    ImU32 GetBackgroundColor() { return BackgroundColor; }
    ImU32 GetGraticuleColor() { return GraticuleColor; }
    void SetBackgroundColor(ImU32 color) { BackgroundColor = color; }
    void SetGraticuleColor(ImU32 color) { GraticuleColor = color; }
    size_t GetCurveCount() { return mKeys.size(); }
    std::string GetCurveName(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].name; return ""; }
    size_t GetCurvePointCount(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].points.size(); return 0; }
    ImU32 GetCurveColor(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].color; return 0; }
    ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { if (curveIndex < mKeys.size()) return mKeys[curveIndex].type; return ImCurveEdit::Linear; }
    void SetCurveColor(size_t curveIndex, ImU32 color) { if (curveIndex < mKeys.size()) mKeys[curveIndex].color = color; }
    void SetCurveName(size_t curveIndex, std::string name) { if (curveIndex < mKeys.size()) mKeys[curveIndex].name = name; }
    void SetCurveVisible(size_t curveIndex, bool visible) { if (curveIndex < mKeys.size()) mKeys[curveIndex].visible = visible; }
    ImCurveEdit::CurveType GetCurvePointType(size_t curveIndex, size_t point) const 
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
    ImCurveEdit::KeyPoint* GetPoints(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].points.data(); return nullptr; }
    ImCurveEdit::KeyPoint GetPoint(size_t curveIndex, size_t pointIndex)
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
    
    int EditPoint(size_t curveIndex, size_t pointIndex, ImVec2 value, ImCurveEdit::CurveType type)
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
    void AlignValue(ImVec2& value)
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
    void AddPoint(size_t curveIndex, ImVec2 value, ImCurveEdit::CurveType type)
    {
        if (curveIndex < mKeys.size())
        {
            mKeys[curveIndex].points.push_back({ImVec2(value.x, value.y), type});
            SortValues(curveIndex);
        }
    }
    void ClearPoint(size_t curveIndex)
    {
        if (curveIndex < mKeys.size())
        {
            mKeys[curveIndex].points.clear();
        }
    }
    void DeletePoint(size_t curveIndex, size_t pointIndex)
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
    int AddCurve(std::string name, ImCurveEdit::CurveType type, ImU32 color, bool visible, float _min, float _max, float _default)
    {
        auto new_key = ImCurveEdit::keys(name, type, color, visible, _min, _max, _default);
        mKeys.push_back(new_key);
        return mKeys.size() - 1;
    }

    void DeleteCurve(size_t curveIndex)
    {
        if (curveIndex < mKeys.size())
        {
            auto iter = mKeys.begin() + curveIndex;
            mKeys.erase(iter);
        }
    }

    void DeleteCurve(std::string name)
    {
        int index = GetCurveIndex(name);
        if (index != -1)
        {
            DeleteCurve(index);
        }
    }

    int GetCurveIndex(std::string name)
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
    float GetPointValue(size_t curveIndex, float t)
    {
        auto value_range = GetCurveMax(curveIndex) - GetCurveMin(curveIndex); 
        auto value = GetValue(curveIndex, t);
        value = (value - GetCurveMin(curveIndex)) / (value_range + FLT_EPSILON);
        return value;
    }
    float GetValue(size_t curveIndex, float t)
    {
        if (curveIndex <  mKeys.size())
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

    ImVec2& GetMax() { return mMax; }
    ImVec2& GetMin() { return mMin; }
    float GetCurveMin(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_min; return 0.f; }
    float GetCurveMax(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_max; return 0.f; }
    void SetCurveMin(size_t curveIndex, float _min) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_min = _min;  }
    void SetCurveMax(size_t curveIndex, float _max) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_max = _max;  }
    float GetCurveDefault(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_default; return 0.f; }
    void SetCurveDefault(size_t curveIndex, float _default) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_default = _default; }
    void SetCurveAlign(ImVec2 align) { mAlign = align; }
    void SetCurvePointDefault(size_t curveIndex, size_t pointIndex)
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
    void MoveTo(float x)
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
    void SetRangeX(float _min, float _max, bool dock)
    {
        SetMin(ImVec2(_min, 0.f), dock);
        SetMax(ImVec2(_max, 1.f), dock);
    }
    void SetMin(ImVec2 vmin, bool dock = false)
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
    void SetMax(ImVec2 vmax, bool dock = false)
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
    bool IsVisible(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].visible; return false; }

    void Load(const imgui_json::value& value);
    void Save(imgui_json::value& value);

private:
    std::vector<ImCurveEdit::keys> mKeys;
    ImVec2 mMin {-1.f, -1.f};
    ImVec2 mMax {-1.f, -1.f};
    ImVec2 mAlign {0.f, 0.f};
    ImU32 BackgroundColor {IM_COL32(24, 24, 24, 255)};
    ImU32 GraticuleColor {IM_COL32(48, 48, 48, 128)};

private:
    void SortValues(size_t curveIndex)
    {
        if (curveIndex < mKeys.size())
        {
            auto b = std::begin(mKeys[curveIndex].points);
            auto e = std::begin(mKeys[curveIndex].points) + GetCurvePointCount(curveIndex);
            std::sort(b, e, [](ImCurveEdit::KeyPoint a, ImCurveEdit::KeyPoint b) { return a.point.x < b.point.x; });
        }
    }
};

IMGUI_API bool ImCurveEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, std::string name, float _min, float _max, float _default, float space = 0);
IMGUI_API bool ImCurveCheckEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, bool &check, std::string name, float _min, float _max, float _default, float space = 0);

} // namespace ImGui
#if IMGUI_BUILD_EXAMPLE
namespace ImGui
{
    IMGUI_API void ShowCurveDemo();
} //namespace ImGui
#endif
#endif /* IMGUI_CURVE_H */
