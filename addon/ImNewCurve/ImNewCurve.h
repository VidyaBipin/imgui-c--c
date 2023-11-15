#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <imgui.h>
#include <imgui_json.h>

namespace ImGui
{
namespace ImNewCurve
{

#define CURVE_EDIT_FLAG_NONE            (0)
#define CURVE_EDIT_FLAG_VALUE_LIMITED   (1)
#define CURVE_EDIT_FLAG_SCROLL_V        (1<<1)
#define CURVE_EDIT_FLAG_MOVE_CURVE      (1<<2)
#define CURVE_EDIT_FLAG_KEEP_BEGIN_END  (1<<3)
#define CURVE_EDIT_FLAG_DOCK_BEGIN_END  (1<<4)
// #define CURVE_EDIT_FLAG_DRAW_TIMELINE   (1<<5)

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

    const std::vector<std::string>& GetCurveTypeNames();

    enum ValueDimension
    {
        DIM_X = 0,
        DIM_Y,
        DIM_Z,
        DIM_T,
    };

    struct KeyPoint
    {
        using Holder = std::shared_ptr<KeyPoint>;
        using ValType = ImVec4;
        static Holder CreateInstance();
        static Holder CreateInstance(const ValType& val, CurveType type);

        float &x, &y, &z, &t;
        ValType val {0, 0, 0, 0};
        CurveType type {UnKnown};

        KeyPoint() : x(val.x), y(val.y), z(val.z), t(val.w) {}
        KeyPoint(const KeyPoint& a) : val(a.val), type(a.type), x(val.x), y(val.y), z(val.z), t(val.w) {}
        KeyPoint(const ValType& _val, CurveType _type) : val(_val), type(_type), x(val.x), y(val.y), z(val.z), t(val.w) {}
        KeyPoint& operator=(const KeyPoint& a) { val = a.val; type = a.type; return *this; }

        ImVec2 GetVec2PointValByDim(ValueDimension eDim) const
        {
            if (eDim == DIM_X)
                return ImVec2(t, x);
            else if (eDim == DIM_Y)
                return ImVec2(t, y);
            else
                return ImVec2(t, z);
        }

        ImVec2 GetVec2PointValByDim(ValueDimension eDim1, ValueDimension eDim2) const
        {
            float x_, y_;
            if (eDim1 == DIM_X)
                x_ = x;
            else if (eDim1 == DIM_Y)
                x_ = y;
            else if (eDim1 == DIM_Z)
                x_ = z;
            else
                x_ = t;
            if (eDim2 == DIM_X)
                y_ = x;
            else if (eDim2 == DIM_Y)
                y_ = y;
            else if (eDim2 == DIM_Z)
                y_ = z;
            else
                y_ = t;
            return ImVec2(x_, y_);
        }

        imgui_json::value SaveAsJson() const;
        void LoadFromJson(const imgui_json::value& j);

        static inline float GetDimVal(const ValType& v, ValueDimension eDim)
        {
            if (eDim == DIM_X)
                return v.x;
            else if (eDim == DIM_Y)
                return v.y;
            else if (eDim == DIM_Z)
                return v.z;
            else
                return v.w;
        }

        static inline void SetDimVal(ValType& v, float f, ValueDimension eDim)
        {
            if (eDim == DIM_X)
                v.x = f;
            else if (eDim == DIM_Y)
                v.y = f;
            else if (eDim == DIM_Z)
                v.z = f;
            else
                v.w = f;
        }
    };

    static inline KeyPoint::ValType Min(const KeyPoint::ValType& a, const KeyPoint::ValType& b)
    { return KeyPoint::ValType(a.x<b.x?a.x:b.x, a.y<b.y?a.y:b.y, a.z<b.z?a.z:b.z, a.w<b.w?a.w:b.w); }

    static inline KeyPoint::ValType Max(const KeyPoint::ValType& a, const KeyPoint::ValType& b)
    { return KeyPoint::ValType(a.x>b.x?a.x:b.x, a.y>b.y?a.y:b.y, a.z>b.z?a.z:b.z, a.w>b.w?a.w:b.w); }

    class Curve
    {
    public:
        using Holder = std::shared_ptr<Curve>;
        static Holder CreateInstance(const std::string& name, CurveType eCurveType, const std::pair<uint32_t, uint32_t>& tTimeBase,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal);
        static Holder CreateInstance(const std::string& name, CurveType eCurveType,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal);
        static Holder CreateFromJson(const imgui_json::value& j);

        Curve(const std::string& name, CurveType eCurveType, const std::pair<uint32_t, uint32_t>& tTimeBase,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal);
        Curve(const std::string& name, CurveType eCurveType,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal)
            : Curve(name, eCurveType, {0, 0}, minVal, maxVal, defaultVal) {}

        const std::string& GetName() const { return m_strName; }
        CurveType GetCurveType() const { return m_eCurveType; }
        const KeyPoint::ValType& GetMinVal() const { return m_tMinVal; }
        const KeyPoint::ValType& GetMaxVal() const { return m_tMaxVal; }
        const KeyPoint::ValType& GetDefaultVal() const { return m_tDefaultVal; }
        const std::pair<uint32_t, uint32_t>& GetTimeBase() const { return m_tTimeBase; }
        int GetKeyPointIndex(const KeyPoint::Holder& hKp) const;
        KeyPoint::ValType CalcPointVal(float t, bool bDenormalize, bool bAlignTime = true) const;
        float Tick2Time(float tick) const { if (m_bTimeBaseValid) return tick*1000*m_tTimeBase.first/m_tTimeBase.second; return tick; }
        float Time2Tick(float time) const { if (m_bTimeBaseValid) return time*m_tTimeBase.second/(m_tTimeBase.first*1000); return time; }
        float Time2TickAligned(float time) const { if (m_bTimeBaseValid) return roundf(time*m_tTimeBase.second/(m_tTimeBase.first*1000)); return time; }

        void SetMinVal(const KeyPoint::ValType& minVal);
        void SetMaxVal(const KeyPoint::ValType& maxVal);
        virtual int AddPoint(KeyPoint::Holder hKp, bool bNeedNormalize, bool bOverwriteIfExists = true);
        virtual int AddPointByDim(ValueDimension eDim, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize, bool bOverwriteIfExists = true);
        virtual int EditPoint(size_t idx, const KeyPoint::ValType& tKpVal, CurveType eCurveType, bool bNormalize);
        virtual int EditPointByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize);
        virtual int ChangePointVal(size_t idx, const KeyPoint::ValType& tKpVal, bool bNormalize);
        virtual int ChangePointValByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, bool bNormalize);
        virtual int ChangeCurveType(size_t idx, CurveType eCurveType);
        virtual float MoveVerticallyByDim(ValueDimension eDim, const ImVec2& v2SyncPoint, bool bNormalize);
        virtual bool SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds);

        std::string PrintKeyPointsByDim(ValueDimension eDim) const;
        virtual imgui_json::value SaveAsJson() const;
        virtual void LoadFromJson(const imgui_json::value& j);

    protected:
        Curve() {}
        void SortKeyPoints();

    protected:
        std::string m_strName;
        std::vector<KeyPoint::Holder> m_aKeyPoints;
        CurveType m_eCurveType {Smooth};
        KeyPoint::ValType m_tMinVal;
        KeyPoint::ValType m_tMaxVal;
        KeyPoint::ValType m_tValRange;
        KeyPoint::ValType m_tDefaultVal;
        std::pair<uint32_t, uint32_t> m_tTimeBase;
        bool m_bTimeBaseValid;
    };

    // forward declaration
    class Editor;

    class CurveUiObj : public Curve
    {
    public:
        using Holder = std::shared_ptr<CurveUiObj>;
        static Holder CreateInstance(Editor* owner, const std::string& name, CurveType type,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
                ImU32 color, bool visible, int64_t id, int64_t subId);
        static Holder CreateInstance(Editor* owner, const std::string& name, CurveType type, const std::pair<uint32_t, uint32_t>& tTimeBase,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
                ImU32 color, bool visible, int64_t id, int64_t subId);
        static Holder CreateFromJson(Editor* owner, const imgui_json::value& j);

        CurveUiObj(Editor* owner, const std::string& name, CurveType type, const std::pair<uint32_t, uint32_t>& tTimeBase,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
                ImU32 curveColor, bool visible, int64_t id, int64_t subId)
            : Curve(name, type, tTimeBase, minVal, maxVal, defaultVal), m_owner(owner), m_u32CurveColor(curveColor), m_bVisible(visible), m_id(id), m_subId(subId)
        {
            ImColor tmp(curveColor);
            tmp.Value.x += 0.1f; if (tmp.Value.x > 1) tmp.Value.x = 1;
            tmp.Value.y += 0.1f; if (tmp.Value.y > 1) tmp.Value.y = 1;
            tmp.Value.z += 0.1f; if (tmp.Value.z > 1) tmp.Value.z = 1;
            m_u32CurveHoveringColor = (ImU32)tmp;
        }
        CurveUiObj(Editor* owner, const std::string& name, CurveType type,
                const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
                ImU32 curveColor, bool visible, int64_t id, int64_t subId)
            : CurveUiObj(owner, name, type, {0, 0}, minVal, maxVal, defaultVal, curveColor, visible, id, subId) {}

        bool DrawDim(ValueDimension eDim, ImDrawList* pDrawList, const ImVec2& v2CursorPos, bool bIsHovering);
        bool CheckMouseHoverCurve(ValueDimension eDim, const ImVec2& pos);
        int CheckMouseHoverPoint(ValueDimension eDim, const ImVec2& pos) const;
        int AddPoint(KeyPoint::Holder hKp, bool bNeedNormalize, bool bOverwriteIfExists = true) override;
        int AddPointByDim(ValueDimension eDim, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize, bool bOverwriteIfExists = true) override;
        int EditPoint(size_t idx, const KeyPoint::ValType& tKpVal, CurveType eCurveType, bool bNormalize) override;
        int EditPointByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize) override;
        int ChangeCurveType(size_t idx, CurveType eCurveType) override;
        float MoveVerticallyByDim(ValueDimension eDim, const ImVec2& v2SyncPoint, bool bNormalize) override;
        bool SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds) override;
        void SetVisible(bool bVisible) { m_bVisible = bVisible; }
        bool IsVisible() const { return m_bVisible; }

        void UpdateContourPoints(int idx);
        void UpdateContourPointsByDim(ValueDimension eDim, int idx);
        void RemoveInvalidContourPoints();

        imgui_json::value SaveAsJson() const override;
        void LoadFromJson(const imgui_json::value& j) override;

    private:
        CurveUiObj(Editor* owner) : m_owner(owner) {}

    private:
        Editor* m_owner;
        ImU32 m_u32CurveColor;
        ImU32 m_u32CurveHoveringColor;
        float m_fCurveWidth {1.5f};
        float m_fCurveHoveringWidth {2.5f};
        ImU32 m_u32KeyPointColor {IM_COL32(40, 40, 40, 255)};
        ImU32 m_u32KeyPointHoveringColor {IM_COL32(240, 240, 240, 255)};
        ImU32 m_u32KeyPointEdgeColor {IM_COL32(255, 128, 0, 255)};
        float m_fKeyPointRadius {3.5f};
        bool m_bVisible {true};
        int64_t m_id {-1};
        int64_t m_subId {-1};

        using ContourPointsTable = std::unordered_map<KeyPoint::Holder, std::vector<ImVec2>>;
        std::unordered_map<ValueDimension, ContourPointsTable> m_aContourPoints;
    };

    class Editor
    {
    public:
        using Holder = std::shared_ptr<Editor>;
        static Holder CreateInstance(const ImVec2& v2TimeRange);

        Editor(const ImVec2& v2TimeRange) : m_v2TimeRange(v2TimeRange) {}
        void Clear();
        ImU32 GetBackgroundColor() const { return m_u32BackgroundColor; }
        ImU32 GetGraticuleColor() const { return m_u32GraticuleColor; }
        void SetBackgroundColor(ImU32 color) { m_u32BackgroundColor = color; }
        void SetGraticuleColor(ImU32 color) { m_u32GraticuleColor = color; }
        const ImVec2& GetTimeRange() const { return m_v2TimeRange; }
        bool SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds);
        void SetShowValueToolTip(bool bShow) { m_bShowValueToolTip = bShow; }
        bool DrawDim(ValueDimension eDim, const char* pcLabel, const ImVec2& v2ViewSize, uint32_t flags, ImDrawList* pDrawList = nullptr);

        // Curve related apis
        size_t GetCurveCount() const { return m_aCurveUiObjs.size(); }
        CurveUiObj::Holder AddCurve(
                const std::string& name, CurveType eType, const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
                ImU32 color, bool visible, int64_t id = -1, int64_t subId = -1);
        CurveUiObj::Holder AddCurveByDim(
                const std::string& name, CurveType eType, ValueDimension eDim, float minVal, float maxVal, float defaultVal,
                ImU32 color, bool visible, int64_t id = -1, int64_t subId = -1);
        CurveUiObj::Holder GetCurveByIndex(size_t idx) const;
        CurveUiObj::Holder GetCurveByName(const std::string& name) const;
        bool IsCurveVisible(size_t idx) const { if (idx >= m_aCurveUiObjs.size()) return false; return m_aCurveUiObjs[idx]->IsVisible(); }

        inline ImVec2 CvtPoint2Pos(const KeyPoint::ValType& tKpVal, ValueDimension eDim) const
        {
            const float x = (tKpVal.w-m_v2TimeRange.x)*m_v2Point2PosScale.x;
            const float y = KeyPoint::GetDimVal(tKpVal, eDim)*m_v2Point2PosScale.y;
            return ImVec2(x, y);
        }

        inline ImVec2 CvtPoint2Pos(const ImVec2& v2PtVal) const
        {
            const float x = (v2PtVal.x-m_v2TimeRange.x)*m_v2Point2PosScale.x;
            const float y = v2PtVal.y*m_v2Point2PosScale.y;
            return ImVec2(x, y);
        }

        inline ImVec2 CvtPos2Point(const ImVec2& v2Pos) const
        {
            const float x = v2Pos.x/m_v2Point2PosScale.x+m_v2TimeRange.x;
            const float y = v2Pos.y/m_v2Point2PosScale.y;
            return ImVec2(x, y);
        }

        imgui_json::value SaveAsJson() const;
        void LoadFromJson(const imgui_json::value& j);

    private:
        std::vector<CurveUiObj::Holder> m_aCurveUiObjs;
        ImU32 m_u32BackgroundColor {IM_COL32(24, 24, 24, 255)};
        ImU32 m_u32GraticuleColor {IM_COL32(48, 48, 48, 128)};
        ImVec2 m_v2TimeRange;
        float m_fCheckHoverDistanceThresh {8.f};
        int m_iCurveHoveringIdx {-1};
        int m_iPointHoveringIdx {-1};
        bool m_bIsDragging {false};
        ImVec2 m_v2UiScale {1.f, 1.f};
        ImVec2 m_v2CurveAreaSize;
        ImVec2 m_v2Point2PosScale;
        ImVec2 m_v2PanOffset;
        bool m_bShowValueToolTip {false};

        friend CurveUiObj;
    };

#if IMGUI_BUILD_EXAMPLE
    IMGUI_API void ShowDemo();
#endif

}  // ~ namespace ImCurve
}  // ~ namespace ImGui
