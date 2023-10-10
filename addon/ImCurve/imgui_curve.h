#ifndef IMGUI_CURVE_H
#define IMGUI_CURVE_H

#include <functional>
#include <vector>
#include <algorithm>
#include <set>
#include <imgui.h>
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
        bool checked {false};
        int64_t m_id {-1};
        int64_t m_sub_id {-1};
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
        virtual int64_t GetCurveID(size_t curveIndex) = 0;
        virtual int64_t GetCurveSubID(size_t curveIndex) = 0;
        virtual KeyPoint* GetPoints(size_t curveIndex) = 0;
        virtual KeyPoint GetPoint(size_t curveIndex, size_t pointIndex) = 0;
        virtual ImVec2 GetPrevPoint(float pos) = 0;
        virtual ImVec2 GetNextPoint(float pos) = 0;
        virtual void AlignValue(ImVec2& value) = 0;
        virtual int EditPoint(size_t curveIndex, size_t pointIndex, ImVec2 value, CurveType type) = 0;
        virtual void AddPoint(size_t curveIndex, ImVec2 value, CurveType type) = 0;
        virtual float GetValue(size_t curveIndex, float t) = 0;
        virtual float GetPointValue(size_t curveIndex, float t) = 0;
        virtual void ClearPoint(size_t curveIndex) = 0;
        virtual void DeletePoint(size_t curveIndex, size_t pointIndex) = 0;
        virtual int AddCurve(std::string name, CurveType type, ImU32 color, bool visible, float _min, float _max, float _default, int64_t _id = -1, int64_t _sub_id = -1) = 0;
        virtual void DeleteCurve(size_t curveIndex) = 0;
        virtual void DeleteCurve(std::string name) = 0;
        virtual int GetCurveIndex(std::string name) = 0;
        virtual int GetCurveIndex(int64_t id) = 0;
        virtual int GetCurveKeyCount() = 0;
        virtual const ImCurveEdit::keys* GetCurveKey(std::string name) = 0;
        virtual const ImCurveEdit::keys* GetCurveKey(size_t curveIndex) = 0;
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
    static bool Edit(ImDrawList* draw_list, Delegate* delegate, const ImVec2& size, unsigned int id, bool editable, float& cursor_pos, unsigned int flags = CURVE_EDIT_FLAG_NONE, const ImRect* clippingRect = NULL, bool * changed = nullptr);
    static bool Edit(ImDrawList* draw_list, Delegate* delegate, const ImVec2& size, unsigned int id, bool editable,
                    float& cursor_pos, float firstTime, float lastTime,
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
    ImVec2 GetPrevPoint(float pos);
    ImVec2 GetNextPoint(float pos);
    ImCurveEdit::CurveType GetCurvePointType(size_t curveIndex, size_t point) const;
    ImCurveEdit::KeyPoint GetPoint(size_t curveIndex, size_t pointIndex);
    int EditPoint(size_t curveIndex, size_t pointIndex, ImVec2 value, ImCurveEdit::CurveType type);
    void AlignValue(ImVec2& value);
    void AddPoint(size_t curveIndex, ImVec2 value, ImCurveEdit::CurveType type);
    void ClearPoint(size_t curveIndex);
    void DeletePoint(size_t curveIndex, size_t pointIndex);
    int AddCurve(std::string name, ImCurveEdit::CurveType type, ImU32 color, bool visible, float _min, float _max, float _default, int64_t _id = -1, int64_t _sub_id = -1);
    void DeleteCurve(size_t curveIndex);
    void DeleteCurve(std::string name);
    int GetCurveIndex(std::string name);
    int GetCurveIndex(int64_t id);
    int GetCurveKeyCount() { return mKeys.size(); }
    const ImCurveEdit::keys* GetCurveKey(std::string name);
    const ImCurveEdit::keys* GetCurveKey(size_t curveIndex);
    float GetPointValue(size_t curveIndex, float t);
    float GetValue(size_t curveIndex, float t);
    void SetCurvePointDefault(size_t curveIndex, size_t pointIndex);
    void MoveTo(float x);
    void SetMin(ImVec2 vmin, bool dock = false);
    void SetMax(ImVec2 vmax, bool dock = false);

    ImU32 GetBackgroundColor() { return BackgroundColor; }
    ImU32 GetGraticuleColor() { return GraticuleColor; }
    void SetBackgroundColor(ImU32 color) { BackgroundColor = color; }
    void SetGraticuleColor(ImU32 color) { GraticuleColor = color; }
    size_t GetCurveCount() { return mKeys.size(); }
    std::string GetCurveName(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].name; return ""; }
    int64_t GetCurveID(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_id; return -1; }
    int64_t GetCurveSubID(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_sub_id; return -1; }
    size_t GetCurvePointCount(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].points.size(); return 0; }
    ImU32 GetCurveColor(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].color; return 0; }
    ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { if (curveIndex < mKeys.size()) return mKeys[curveIndex].type; return ImCurveEdit::Linear; }
    void SetCurveColor(size_t curveIndex, ImU32 color) { if (curveIndex < mKeys.size()) mKeys[curveIndex].color = color; }
    void SetCurveName(size_t curveIndex, std::string name) { if (curveIndex < mKeys.size()) mKeys[curveIndex].name = name; }
    void SetCurveVisible(size_t curveIndex, bool visible) { if (curveIndex < mKeys.size()) mKeys[curveIndex].visible = visible; }
    ImCurveEdit::KeyPoint* GetPoints(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].points.data(); return nullptr; }
    ImVec2& GetMax() { return mMax; }
    ImVec2& GetMin() { return mMin; }
    float GetCurveMin(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_min; return 0.f; }
    float GetCurveMax(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_max; return 0.f; }
    void SetCurveMin(size_t curveIndex, float _min) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_min = _min;  }
    void SetCurveMax(size_t curveIndex, float _max) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_max = _max;  }
    float GetCurveDefault(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].m_default; return 0.f; }
    void SetCurveDefault(size_t curveIndex, float _default) { if (curveIndex < mKeys.size()) mKeys[curveIndex].m_default = _default; }
    void SetCurveAlignX(ImVec2 align) { mAlignX = align; }
    void SetCurveAlignY(ImVec2 align) { mAlignY = align; }
    void SetRangeX(float _min, float _max, bool dock) { SetMin(ImVec2(_min, 0.f), dock); SetMax(ImVec2(_max, 1.f), dock); }
    bool IsVisible(size_t curveIndex) { if (curveIndex < mKeys.size()) return mKeys[curveIndex].visible; return false; }

    void Load(const imgui_json::value& value);
    void Save(imgui_json::value& value);

private:
    std::vector<ImCurveEdit::keys> mKeys;
    ImVec2 mMin {-1.f, -1.f};
    ImVec2 mMax {-1.f, -1.f};
    ImVec2 mAlignX {0.f, 0.f};
    ImVec2 mAlignY {0.f, 0.f};
    ImU32 BackgroundColor {IM_COL32(24, 24, 24, 255)};
    ImU32 GraticuleColor {IM_COL32(48, 48, 48, 128)};

private:
    void SortValues(size_t curveIndex);
};

IMGUI_API bool ImCurveEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, std::string name, float _min, float _max, float _default, float space = 0);
IMGUI_API bool ImCurveCheckEditKey(std::string button_lable, ImGui::ImCurveEdit::keys * key, bool &check, std::string name, float _min, float _max, float _default, float space = 0);
IMGUI_API bool ImCurveCheckEditKeyWithID(std::string button_lable, ImGui::ImCurveEdit::keys * key, bool check, std::string name, float _min, float _max, float _default, int64_t subid = -1, float space = 0);

} // namespace ImGui
#if IMGUI_BUILD_EXAMPLE
namespace ImGui
{
    IMGUI_API void ShowCurveDemo();
} //namespace ImGui
#endif
#endif /* IMGUI_CURVE_H */
