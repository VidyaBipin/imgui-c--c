#include <iostream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <list>
#include <limits>
#include "ImNewCurve.h"
#include "imgui_internal.h"

namespace ImGui
{
namespace ImNewCurve
{

const std::vector<std::string> _CURVE_TYPE_NAMES = {
        "Hold", "Step", "Linear", "Smooth",
        "QuadIn", "QuadOut", "QuadInOut", 
        "CubicIn", "CubicOut", "CubicInOut", 
        "SineIn", "SineOut", "SineInOut",
        "ExpIn", "ExpOut", "ExpInOut",
        "CircIn", "CircOut", "CircInOut",
        "ElasticIn", "ElasticOut", "ElasticInOut",
        "BackIn", "BackOut", "BackInOut",
        "BounceIn", "BounceOut", "BounceInOut"};

const std::vector<std::string>& GetCurveTypeNames()
{
    return _CURVE_TYPE_NAMES;
}

template <typename T>
static T __tween_bounceout(const T& p) { return (p) < 4 / 11.0 ? (121 * (p) * (p)) / 16.0 : (p) < 8 / 11.0 ? (363 / 40.0 * (p) * (p)) - (99 / 10.0 * (p)) + 17 / 5.0 : (p) < 9 / 10.0 ? (4356 / 361.0 * (p) * (p)) - (35442 / 1805.0 * (p)) + 16061 / 1805.0 : (54 / 5.0 * (p) * (p)) - (513 / 25.0 * (p)) + 268 / 25.0; }

static float SmoothStep(float edge0, float edge1, float t, CurveType type)
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

static inline float CalcDistance(const KeyPoint::ValType& a, const KeyPoint::ValType& b)
{ const auto d = a-b; return sqrtf(d.x*d.x+d.y*d.y+d.z*d.z+d.w*d.w); }

static inline float CalcDistance(const ImVec2& a, const ImVec2& b)
{ const auto d = a-b; return sqrtf(d.x*d.x+d.y*d.y); }

float CalcDistance(const ImVec2& p, const ImVec2& p1, const ImVec2& p2)
{
    const auto A = p.x-p1.x;
    const auto B = p.y-p1.y;
    const auto C = p2.x-p1.x;
    const auto D = p2.y-p1.y;
    const auto dot = A*C+B*D;
    const auto lenSq = C*C+D*D;
    float param = -1.f;
    if (lenSq > FLT_EPSILON)
        param = dot/lenSq;
    float xx, yy;
    if (param < 0.f)
    {
        xx = p1.x;
        yy = p1.y;
    }
    else if (param > 1.f)
    {
        xx = p2.x;
        yy = p2.y;
    }
    else
    {
        xx = p1.x+param*C;
        yy = p1.y+param*D;
    }
    const float dx = p.x-xx;
    const float dy = p.y-yy;
    return sqrtf(dx*dx+dy*dy);
}

// 'KeyPoint' implementation
imgui_json::value KeyPoint::SaveAsJson() const
{
    imgui_json::value j;
    j["val"] = imgui_json::vec4(val);
    j["type"] = imgui_json::number((int)type);
    return std::move(j);
}

void KeyPoint::LoadFromJson(const imgui_json::value& j)
{
    val = j["val"].get<imgui_json::vec4>();
    type = (CurveType)((int)(j["type"].get<imgui_json::number>()));
}

KeyPoint::Holder KeyPoint::CreateInstance()
{
    return Holder(new KeyPoint());
}

KeyPoint::Holder KeyPoint::CreateInstance(const ValType& val, CurveType type)
{
    return Holder(new KeyPoint(val, type));
}

// 'Curve' implementation
Curve::Holder Curve::CreateInstance(
        const std::string& name, CurveType eCurveType, const std::pair<uint32_t, uint32_t>& tTimeBase,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal, bool bAddInitKp)
{
    return Curve::Holder(new Curve(name, eCurveType, tTimeBase, minVal, maxVal, defaultVal, bAddInitKp));
}

Curve::Holder Curve::CreateInstance(
        const std::string& name, CurveType eCurveType,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal, bool bAddInitKp)
{
    return CreateInstance(name, eCurveType, {0, 0}, minVal, maxVal, defaultVal, bAddInitKp);
}

Curve::Holder Curve::CreateFromJson(const imgui_json::value& j)
{
    auto hCurve = Curve::Holder(new Curve());
    hCurve->LoadFromJson(j);
    return hCurve;
}

Curve::Curve(const std::string& name, CurveType eCurveType, const std::pair<uint32_t, uint32_t>& tTimeBase,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal, bool bAddInitKp)
    : m_strName(name), m_eCurveType(eCurveType), m_tTimeBase(tTimeBase), m_tDefaultVal(defaultVal)
{
    m_tMinVal = Min(minVal, maxVal);
    m_tMaxVal = Max(minVal, maxVal);
    m_tValRange = m_tMaxVal-m_tMinVal;
    m_bTimeBaseValid = tTimeBase.first > 0 && tTimeBase.second > 0;
    if (m_bTimeBaseValid)
    {
        m_tMinVal.w = Time2TickAligned(m_tMinVal.w);
        m_tMaxVal.w = Time2TickAligned(m_tMaxVal.w);
    }
    if (m_tMinVal.w > m_tMaxVal.w)
        throw std::runtime_error("INVALID arguments to create a 'Curve'! 'minVal.w' (time range begin) is larger than 'maxVal.w' (time range end).");
    if (bAddInitKp)
    {
        auto tInitKpVal = defaultVal;
        tInitKpVal.w = m_tMinVal.w;
        m_aKeyPoints.push_back(KeyPoint::CreateInstance(tInitKpVal, eCurveType));
    }
}

Curve::Holder Curve::Clone() const
{
    auto hNewInstance = CreateInstance(m_strName, m_eCurveType, m_tTimeBase, m_tMinVal, m_tMaxVal, m_tDefaultVal);
    auto& aClonedKeyPoints = hNewInstance->m_aKeyPoints;
    for (const auto& hKp : m_aKeyPoints)
        aClonedKeyPoints.push_back(KeyPoint::Holder(new KeyPoint(*hKp)));
    hNewInstance->m_bLimitTimeInRange = m_bLimitTimeInRange;
    hNewInstance->m_bLimitKeyPointValueInRange = m_bLimitKeyPointValueInRange;
    hNewInstance->m_bLimitOutputValueInRange = m_bLimitOutputValueInRange;
    return hNewInstance;
}

std::vector<float> Curve::GetTicks() const
{
    std::vector<float> aTicks;
    aTicks.reserve(m_aKeyPoints.size());
    for (const auto& hKp : m_aKeyPoints)
        aTicks.push_back(hKp->t);
    return std::move(aTicks);
}

const KeyPoint::Holder Curve::GetKeyPoint(size_t idx) const
{
    if (idx >= m_aKeyPoints.size())
        return nullptr;
    auto iter = m_aKeyPoints.begin();
    while (idx-- > 0)
        iter++;
    return *iter;
}

KeyPoint::Holder Curve::GetKeyPoint_(size_t idx) const
{
    if (idx >= m_aKeyPoints.size())
        return nullptr;
    auto iter = m_aKeyPoints.begin();
    while (idx-- > 0)
        iter++;
    return *iter;
}

std::list<KeyPoint::Holder>::iterator Curve::GetKpIter(size_t idx)
{
    if (idx >= m_aKeyPoints.size())
        return m_aKeyPoints.end();
    auto iter = m_aKeyPoints.begin();
    while (idx-- > 0)
        iter++;
    return iter;
}

int Curve::GetKeyPointIndex(const KeyPoint::Holder& hKp) const
{
    int idx = -1, i = 0;
    auto iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        if (*iter++ == hKp)
        {
            idx = i;
            break;
        }
        i++;
    }
    return idx;
}

int Curve::GetKeyPointIndex(float t) const
{
    int idx = -1, i = 0;
    auto iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        if ((*iter++)->t == t)
        {
            idx = i;
            break;
        }
        i++;
    }
    return idx;
}

void Curve::SetLimitTimeInRange(bool bEnable, bool bClipKeyPoints)
{
    if (m_bLimitTimeInRange == bEnable && !bClipKeyPoints)
        return;
    m_bLimitTimeInRange = bEnable;
    if (m_aKeyPoints.empty())
        return;
    if (bEnable && bClipKeyPoints)
    {
        if (m_aKeyPoints.front()->t < m_tMinVal.w)
        {
            const auto tTimeRangeStartVal = CalcPointVal(m_tMinVal.w);
            while (!m_aKeyPoints.empty() && m_aKeyPoints.front()->t < m_tMinVal.w)
                m_aKeyPoints.pop_front();
            auto hHeadKp = KeyPoint::CreateInstance(tTimeRangeStartVal, m_eCurveType);
            if (m_aKeyPoints.empty())
                m_aKeyPoints.push_back(hHeadKp);
            else if (m_aKeyPoints.front()->t > m_tMinVal.w)
                m_aKeyPoints.push_front(hHeadKp);
        }
        if (m_aKeyPoints.back()->t > m_tMaxVal.w)
        {
            const auto tTimeRangeEndVal = CalcPointVal(m_tMaxVal.w);
            while (!m_aKeyPoints.empty() && m_aKeyPoints.back()->t > m_tMaxVal.w)
                m_aKeyPoints.pop_back();
            auto hTailKp = KeyPoint::CreateInstance(tTimeRangeEndVal, m_eCurveType);
            if (m_aKeyPoints.empty())
                m_aKeyPoints.push_back(hTailKp);
            else if (m_aKeyPoints.back()->t < m_tMaxVal.w)
                m_aKeyPoints.push_front(hTailKp);
        }
    }
}

void Curve::SetLimitKeyPointValueInRange(bool bEnable, bool bClipKeyPoints)
{
    if (m_bLimitKeyPointValueInRange == bEnable && !bClipKeyPoints)
        return;
    m_bLimitKeyPointValueInRange = bEnable;
    if (m_aKeyPoints.empty())
        return;
    if (bEnable && bClipKeyPoints)
    {
        for (auto& hKp : m_aKeyPoints)
        {
            auto& tVal = hKp->val;
            if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
            if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
            if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
        }
    }
}

void Curve::SetMinVal(const KeyPoint::ValType& minVal)
{
    const ImVec2 v2TimeRange(m_tMinVal.w, m_tMaxVal.w);
    m_tMinVal = Min(minVal, m_tMaxVal);
    m_tMaxVal = Max(minVal, m_tMaxVal);
    m_tMinVal.w = v2TimeRange.x;
    m_tMaxVal.w = v2TimeRange.y;
    m_tValRange = m_tMaxVal-m_tMinVal;
}

void Curve::SetMaxVal(const KeyPoint::ValType& maxVal)
{
    const ImVec2 v2TimeRange(m_tMinVal.w, m_tMaxVal.w);
    m_tMaxVal = Max(m_tMinVal, maxVal);
    m_tMinVal = Min(m_tMinVal, maxVal);
    m_tMinVal.w = v2TimeRange.x;
    m_tMaxVal.w = v2TimeRange.y;
    m_tValRange = m_tMaxVal-m_tMinVal;
}

int Curve::AddPoint(KeyPoint::Holder hKp, bool bOverwriteIfExists)
{
    if (m_bLimitTimeInRange)
    {
        auto& t = hKp->t;
        if (t < m_tMinVal.w) t = m_tMinVal.w; else if (t > m_tMaxVal.w) t = m_tMaxVal.w;
    }
    hKp->t = Time2TickAligned(hKp->t);
    auto iter = std::find_if(m_aKeyPoints.begin(), m_aKeyPoints.end(), [hKp] (const auto& elem) {
        return hKp->t == elem->t;
    });
    if (iter != m_aKeyPoints.end() && !bOverwriteIfExists)
        return -1;

    if (hKp->type == UnKnown)
        hKp->type = m_eCurveType;
    if (m_bLimitKeyPointValueInRange)
    {
        auto& tVal = hKp->val;
        if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
        if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
        if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
    }
    if (iter == m_aKeyPoints.end())
    {
        m_aKeyPoints.push_back(hKp);
        SortKeyPoints();
    }
    else
    {
        (*iter)->val = hKp->val;
        (*iter)->type = hKp->type;
    }
    const auto idx = GetKeyPointIndex(hKp->t);
    if (idx >= 0)
    {
        for (const auto& pCb : m_aCallbacksArray)
            pCb->OnKeyPointAdded((size_t)idx, hKp);
    }
    return idx;
}

int Curve::AddPointByDim(ValueDimension eDim, const ImVec2& v2DimVal, CurveType eCurveType, bool bOverwriteIfExists)
{
    auto tKpVal = CalcPointVal(v2DimVal.x);
    KeyPoint::SetDimVal(tKpVal, v2DimVal.y, eDim);
    tKpVal.w = v2DimVal.x;
    auto hKp = KeyPoint::CreateInstance(tKpVal, eCurveType);
    return AddPoint(hKp, bOverwriteIfExists);
}

int Curve::EditPoint(size_t idx, const KeyPoint::ValType& tKpVal, CurveType eCurveType)
{
    const auto szKpCnt = m_aKeyPoints.size();
    if (idx >= szKpCnt)
        return -1;

    KeyPoint::ValType tKpVal_(tKpVal);
    if (m_bLimitTimeInRange)
    {
        auto& t = tKpVal_.w;
        if (t < m_tMinVal.w) t = m_tMinVal.w; else if (t > m_tMaxVal.w) t = m_tMaxVal.w;
    }
    if (m_bLimitKeyPointValueInRange)
    {
        auto& tVal = tKpVal_;
        if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
        if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
        if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
    }
    tKpVal_.w = Time2TickAligned(tKpVal.w);
    auto hKp = GetKeyPoint(idx);
    if (eCurveType == UnKnown)
        eCurveType = hKp->type;
    if (tKpVal_ == hKp->val && eCurveType == hKp->type)
        return -2;

    float t = tKpVal_.w;
    if (m_bTimeBaseValid)
    {
        if (t <= m_tMinVal.w) t = m_tMinVal.w+1;
        else if (t >= m_tMaxVal.w) t = m_tMaxVal.w-1;
        uint32_t step = 0;
        float fTestTick;
        bool bOutOfRange;
        do {
            bOutOfRange = true;
            fTestTick = t-step;
            if (fTestTick > m_tMinVal.w && fTestTick < m_tMaxVal.w)
            {
                bOutOfRange = false;
                bool bFoundSame = false;
                auto iter = m_aKeyPoints.begin();
                while (iter != m_aKeyPoints.end())
                {
                    const auto& hKpCheck = *iter++;
                    if (hKp != hKpCheck && hKpCheck->t == fTestTick)
                    {
                        bFoundSame = true;
                        break;
                    }
                }
                if (!bFoundSame)
                {
                    t = fTestTick;
                    break;
                }
            }
            fTestTick = t+step;
            if (fTestTick > m_tMinVal.w && fTestTick < m_tMaxVal.w)
            {
                bOutOfRange = false;
                bool bFoundSame = false;
                auto iter = m_aKeyPoints.begin();
                while (iter != m_aKeyPoints.end())
                {
                    const auto& hKpCheck = *iter++;
                    if (hKp != hKpCheck && hKpCheck->t == fTestTick)
                    {
                        bFoundSame = true;
                        break;
                    }
                }
                if (!bFoundSame)
                {
                    t = fTestTick;
                    break;
                }
            }
            step++;
        } while (!bOutOfRange);
        if (bOutOfRange)
            return -1;
    }

    hKp->val = tKpVal_;
    hKp->t = t;
    hKp->type = eCurveType;
    SortKeyPoints();
    const auto iNewIdx = GetKeyPointIndex(hKp);
    if (iNewIdx >= 0)
    {
        for (const auto& pCb : m_aCallbacksArray)
            pCb->OnKeyPointChanged((size_t)iNewIdx, hKp);
        if (iNewIdx != idx)
            for (const auto& pCb : m_aCallbacksArray)
                pCb->OnKeyPointChanged(idx, hKp);
    }
    return iNewIdx;
}

int Curve::EditPointByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, CurveType eCurveType)
{
    if (idx >= m_aKeyPoints.size())
        return -1;
    KeyPoint::ValType tKpVal = GetKeyPoint_(idx)->val;
    KeyPoint::SetDimVal(tKpVal, v2DimVal.y, eDim);
    tKpVal.w = v2DimVal.x;
    return EditPoint(idx, tKpVal, eCurveType);
}

int Curve::ChangeKeyPointCurveType(size_t idx, CurveType eCurveType)
{
    if (idx >= m_aKeyPoints.size())
        return -1;
    auto hKp = GetKeyPoint_(idx);
    hKp->type = eCurveType;
    return idx;
}

KeyPoint::Holder Curve::RemovePoint(size_t idx)
{
    if (idx >= m_aKeyPoints.size())
        return nullptr;
    auto iter = m_aKeyPoints.begin();
    int c = (int)idx;
    while (c-- > 0)
        iter++;
    auto hKp = *iter;
    m_aKeyPoints.erase(iter);
    for (const auto& pCb : m_aCallbacksArray)
        pCb->OnKeyPointRemoved(idx, hKp);
    return hKp;
}

KeyPoint::Holder Curve::RemovePoint(float t)
{
    auto idx = GetKeyPointIndex(t);
    if (idx < 0)
        return nullptr;
    return RemovePoint((size_t)idx);
}

void Curve::ClearAll()
{
    m_aKeyPoints.clear();
}

float Curve::MoveVerticallyByDim(ValueDimension eDim, const ImVec2& v2SyncPoint)
{
    if (v2SyncPoint.x < m_tMinVal.w || v2SyncPoint.x > m_tMaxVal.w)
        return 0.f;
    const auto fOrgPtVal = KeyPoint::GetDimVal(CalcPointVal(v2SyncPoint.x, false), eDim);
    const auto fOffset = v2SyncPoint.y-fOrgPtVal;
    if (fabs(fOffset) < FLT_EPSILON)
        return 0.f;
    for (auto& hKp : m_aKeyPoints)
    {
        const auto fOrgVal = KeyPoint::GetDimVal(hKp->val, eDim);
        KeyPoint::SetDimVal(hKp->val, fOrgVal+fOffset, eDim);
    }
    return fOffset;
}

KeyPoint::ValType Curve::CalcPointVal(float t, bool bAlignTime) const
{
    t = bAlignTime ? Time2TickAligned(t) : Time2Tick(t);
    KeyPoint::ValType res(m_tDefaultVal);
    res.w = t;
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt < 1)
        return res;
    const auto& hKpHead = m_aKeyPoints.front();
    const auto& hKpTail = m_aKeyPoints.back();
    if (szKpCnt == 1 || t <= hKpHead->t)
    {
        res = hKpHead->val;
        res.w = t;
        return res;
    }
    if (t >= hKpTail->t)
    {
        res = hKpTail->val;
        res.w = t;
        return res;
    }
    auto itKp0 = m_aKeyPoints.begin();
    auto itKp1 = itKp0; itKp1++;
    while (itKp1 != m_aKeyPoints.end())
    {
        if (t >= (*itKp0)->t && t < (*itKp1)->t)
            break;
        itKp0 = itKp1; itKp1++;
    }
    const auto& p1 = *itKp0;
    const auto& p2 = *itKp1;
    if (t == p1->t)
    {
        res = p1->val;
    }
    else
    {
        const CurveType type = (t-p1->t) < (p2->t-t) ? p1->type : p2->type;
        const float rt = SmoothStep(p1->t, p2->t, t, type);
        res = ImLerp(p1->val, p2->val, rt);
    }
    res.w = Tick2Time(t);
    if (m_bLimitOutputValueInRange)
    {
        auto& tVal = res;
        if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
        if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
        if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
    }
    return res;
}

bool Curve::SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds)
{
    if (v2TimeRange.x >= v2TimeRange.y)
        return false;
    if (m_bTimeBaseValid)
    {
        m_tMinVal.w = Time2TickAligned(v2TimeRange.x);
        m_tMaxVal.w = Time2TickAligned(v2TimeRange.y);
    }
    else
    {
        m_tMinVal.w = v2TimeRange.x;
        m_tMaxVal.w = v2TimeRange.y;
    }
    m_tValRange.w = m_tMaxVal.w-m_tMinVal.w;
    return true;
}

bool Curve::ScaleKeyPoints(const KeyPoint::ValType& tScale, const KeyPoint::ValType& tOrigin)
{
    const bool bUseOrigin = tOrigin != KeyPoint::ValType(0, 0, 0, 0);
    if (bUseOrigin)
    {
        for (auto& hKp : m_aKeyPoints)
        {
            const auto tOff = hKp->val-tOrigin;
            hKp->val = tOrigin+tOff*tScale;
            if (m_bLimitKeyPointValueInRange)
            {
                auto& tVal = hKp->val;
                if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
                if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
                if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
            }
        }
    }
    else
    {
        for (auto& hKp : m_aKeyPoints)
        {
            hKp->val *= tScale;
            if (m_bLimitKeyPointValueInRange)
            {
                auto& tVal = hKp->val;
                if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
                if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
                if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
            }
        }
    }
    return true;
}

bool Curve::PanKeyPoints(const KeyPoint::ValType& tOffset)
{
    if (m_aKeyPoints.empty())
        return true;
    KeyPoint::ValType tOffset_(tOffset);
    if (m_bLimitTimeInRange)
    {
        if (tOffset_.w < 0)
        {
            const auto& tHeadVal = m_aKeyPoints.front()->val;
            if (tOffset_.w < m_tMinVal.w-tHeadVal.w)
                tOffset_.w = m_tMinVal.w-tHeadVal.w;
        }
        else if (tOffset_.w > 0)
        {
            const auto& tTailVal = m_aKeyPoints.back()->val;
            if (tOffset_.w > m_tMaxVal.w-tTailVal.w)
                tOffset_.w = m_tMaxVal.w-tTailVal.w;
        }
    }
    if (tOffset_ == KeyPoint::ValType(0, 0, 0, 0))
        return true;
    for (auto& hKp : m_aKeyPoints)
    {
        hKp->val += tOffset_;
        if (m_bLimitKeyPointValueInRange)
        {
            auto& tVal = hKp->val;
            if (tVal.x < m_tMinVal.x) tVal.x = m_tMinVal.x; else if (tVal.x > m_tMaxVal.x) tVal.x = m_tMaxVal.x;
            if (tVal.y < m_tMinVal.y) tVal.y = m_tMinVal.y; else if (tVal.y > m_tMaxVal.y) tVal.y = m_tMaxVal.y;
            if (tVal.z < m_tMinVal.z) tVal.z = m_tMinVal.z; else if (tVal.z > m_tMaxVal.z) tVal.z = m_tMaxVal.z;
        }
    }
    return true;
}

std::string Curve::PrintKeyPointsByDim(ValueDimension eDim) const
{
    std::ostringstream oss;
    oss << '{';
    auto iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        const auto& hKp = *iter++;
        const auto v2PtVal = hKp->GetVec2PointValByDim(eDim);
        oss << " (" << v2PtVal.x << ", " << v2PtVal.y << ')';
        if (iter != m_aKeyPoints.end()) oss << ','; else oss << ' ';
    }
    oss << '}';
    return oss.str();
}

void Curve::SortKeyPoints()
{
    if (m_aKeyPoints.size() < 2)
        return;
    m_aKeyPoints.sort([] (const auto& a, const auto& b) {
        return a->t < b->t;
    });
}

imgui_json::value Curve::SaveAsJson() const
{
    imgui_json::value j;
    j["name"] = imgui_json::string(m_strName);
    j["curve_type"] = imgui_json::number((int)m_eCurveType);
    j["min_val"] = imgui_json::vec4(m_tMinVal);
    j["max_val"] = imgui_json::vec4(m_tMaxVal);
    j["default_val"] = imgui_json::vec4(m_tDefaultVal);
    j["time_base"] = imgui_json::vec2(m_tTimeBase.first, m_tTimeBase.second);
    imgui_json::array jaKps;
    for (const auto& hKp : m_aKeyPoints)
        jaKps.push_back(hKp->SaveAsJson());
    j["key_points"] = jaKps;
    return std::move(j);
}

void Curve::LoadFromJson(const imgui_json::value& j)
{
    m_strName = j["name"].get<imgui_json::string>();
    m_eCurveType = (CurveType)((int)(j["curve_type"].get<imgui_json::number>()));
    const KeyPoint::ValType tMinVal = j["min_val"].get<imgui_json::vec4>();
    const KeyPoint::ValType tMaxVal = j["max_val"].get<imgui_json::vec4>();
    m_tMinVal = Min(tMinVal, tMaxVal);
    m_tMaxVal = Max(tMinVal, tMaxVal);
    m_tValRange = m_tMaxVal-m_tMinVal;
    m_tDefaultVal = j["default_val"].get<imgui_json::vec4>();
    ImVec2 v2Tmp = j["time_base"].get<imgui_json::vec2>();
    m_tTimeBase.first = (uint32_t)v2Tmp.x;
    m_tTimeBase.second = (uint32_t)v2Tmp.y;
    m_bTimeBaseValid = m_tTimeBase.first > 0 && m_tTimeBase.second > 0;
    if (m_bTimeBaseValid)
    {
        m_tMinVal.w = Time2TickAligned(m_tMinVal.w);
        m_tMaxVal.w = Time2TickAligned(m_tMaxVal.w);
    }
    m_aKeyPoints.clear();
    imgui_json::array jKps = j["key_points"].get<imgui_json::array>();
    for (const auto& jelem : jKps)
    {
        auto hKp = KeyPoint::CreateInstance();
        hKp->LoadFromJson(jelem);
        m_aKeyPoints.push_back(hKp);
    }
}

void Curve::RigisterCallbacks(Callbacks* pCb)
{
    auto iter = std::find(m_aCallbacksArray.begin(), m_aCallbacksArray.end(), pCb);
    if (iter == m_aCallbacksArray.end())
        m_aCallbacksArray.push_back(pCb);
}

void Curve::UnrigsterCallbacks(Callbacks* pCb)
{
    auto iter = std::find(m_aCallbacksArray.begin(), m_aCallbacksArray.end(), pCb);
    if (iter != m_aCallbacksArray.end())
        m_aCallbacksArray.erase(iter);
}

bool DrawCurveArraySimpleView(float fViewWidth, const std::vector<Curve::Holder>& aCurves, float& fCurrTick, const ImVec2& _v2TickRange, ImGuiKey eRemoveKey)
{
    std::list<float> aAllTicks;
    // merge ticks from all curves
    for (const auto& hCurve : aCurves)
    {
        const auto aCurveTicks = hCurve->GetTicks();
        if (aAllTicks.empty())
        {
            for (const auto& t : aCurveTicks)
                aAllTicks.push_back(t);
        }
        else
        {
            for (const auto& t : aCurveTicks)
            {
                auto itIns = find_if(aAllTicks.begin(), aAllTicks.end(), [t] (const auto& elem) {
                    return elem >= t;
                });
                if (itIns == aAllTicks.end() || *itIns > t)
                    aAllTicks.insert(itIns, t);
            }
        }
    }
    // prepare tick range
    ImVec2 v2TickRange(_v2TickRange);
    if (v2TickRange.y <= v2TickRange.x && !aAllTicks.empty())
    {
        v2TickRange.x = aAllTicks.front();
        v2TickRange.y = aAllTicks.back();
    }
    if (v2TickRange.y < v2TickRange.x+10)
        v2TickRange.y = v2TickRange.x+10;

    const auto v2MousePosAbs = GetMousePos();
    const auto v2ViewPos = GetCursorScreenPos();
    const auto v2MousePos = v2MousePosAbs-v2ViewPos;
    const auto v2AvailSize = GetContentRegionAvail();
    const auto v2CursorPos = GetCursorScreenPos();
    const ImVec2 v2PaddingUnit(5.f, 2.f);
    ImDrawList* pDrawList = GetWindowDrawList();
    ImVec2 v2ViewSize(fViewWidth <= 0 ? v2AvailSize.x : fViewWidth, v2AvailSize.y);
    // draw timeline slider
    const auto fTickLength = v2TickRange.y-v2TickRange.x;
    const float fTimelineSliderHeight = 5.f;
    const float fKeyFrameIndicatorRadius = 5.f;
    const ImVec2 v2TimeLineWdgSize(v2ViewSize.x, fTimelineSliderHeight);
    ImVec2 v2SliderRectLt(v2ViewPos.x+fKeyFrameIndicatorRadius+v2PaddingUnit.x, v2CursorPos.y+v2PaddingUnit.y);
    ImVec2 v2SliderRectRb(v2SliderRectLt.x+v2ViewSize.x-(fKeyFrameIndicatorRadius+v2PaddingUnit.x)*2, v2SliderRectLt.y+fTimelineSliderHeight);
    const auto fSliderWdgWidth = v2SliderRectRb.x-v2SliderRectLt.x;
    const ImU32 u32TimelineSliderBorderColor{IM_COL32(80, 80, 80, 255)};
    pDrawList->AddRect(v2SliderRectLt, v2SliderRectRb, u32TimelineSliderBorderColor, fTimelineSliderHeight/2, 0, 1);
    // draw key point indicators
    const float fKeyFrameIndicatorY = v2CursorPos.y+v2PaddingUnit.y+v2TimeLineWdgSize.y/2;
    const ImU32 u32KeyFrameIndicatorColor{IM_COL32(200, 150, 20, 255)};
    const ImU32 u32KeyFrameIndicatorBorderColor = u32TimelineSliderBorderColor;
    const float fHoverBrightnessIncreament = 0.4;
    ImColor tHoverColor(u32KeyFrameIndicatorColor);
    tHoverColor.Value.x += fHoverBrightnessIncreament; tHoverColor.Value.y += fHoverBrightnessIncreament; tHoverColor.Value.z += fHoverBrightnessIncreament;
    const ImU32 u32KeyFrameIndicatorHoverColor = (ImU32)tHoverColor;
    tHoverColor = ImColor(u32KeyFrameIndicatorBorderColor);
    tHoverColor.Value.x += fHoverBrightnessIncreament; tHoverColor.Value.y += fHoverBrightnessIncreament; tHoverColor.Value.z += fHoverBrightnessIncreament;
    const ImU32 u32KeyFrameIndicatorBorderHoverColor = (ImU32)tHoverColor;
    bool bHasPointAtCurrTick = false;
    bool bHasHoveredTick = false;
    float fHoveredTick;
    float fHoverDistance = std::numeric_limits<float>::max();
    auto fHoverDistThresh = fKeyFrameIndicatorRadius+5.f;
    fHoverDistThresh = fHoverDistThresh*fHoverDistThresh;
    const auto szTickCnt = aAllTicks.size();
    for (const auto t : aAllTicks)
    {
        const float fKeyFrameIndicatorX = v2SliderRectLt.x+((double)(t-v2TickRange.x)/fTickLength)*fSliderWdgWidth;
        const ImVec2 v2TickPos(fKeyFrameIndicatorX, fKeyFrameIndicatorY);
        const auto v2Offset = v2TickPos-v2MousePosAbs;
        const float distSqr = v2Offset.x*v2Offset.x+v2Offset.y*v2Offset.y;
        if (distSqr <= fHoverDistThresh && distSqr < fHoverDistance)
        {
            bHasHoveredTick = true;
            fHoveredTick = t;
            fHoverDistance = distSqr;
        }
        if (t != fCurrTick)
        {
            pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius+1, u32KeyFrameIndicatorBorderColor);
            pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius, u32KeyFrameIndicatorColor);
        }
        else
            bHasPointAtCurrTick = true;
    }
    if (bHasPointAtCurrTick)
    {
        const float fKeyFrameIndicatorX = v2SliderRectLt.x+((fCurrTick-v2TickRange.x)/fTickLength)*fSliderWdgWidth;
        const ImVec2 v2TickPos(fKeyFrameIndicatorX, fKeyFrameIndicatorY);
        pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius+1, u32KeyFrameIndicatorBorderHoverColor);
        pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius, u32KeyFrameIndicatorHoverColor);
    }
    if (bHasHoveredTick)
    {
        const float fKeyFrameIndicatorX = v2SliderRectLt.x+((fHoveredTick-v2TickRange.x)/fTickLength)*fSliderWdgWidth;
        const ImVec2 v2TickPos(fKeyFrameIndicatorX, fKeyFrameIndicatorY);
        pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius+2, u32KeyFrameIndicatorBorderHoverColor);
        pDrawList->AddCircleFilled(v2TickPos, fKeyFrameIndicatorRadius, u32KeyFrameIndicatorHoverColor);
    }

    bool bCurveChanged = false;
    const bool bRemoveKeyDown = IsKeyDown(eRemoveKey);
    // handle click event on a key-frame indicator
    if (IsMouseClicked(ImGuiMouseButton_Left) && bHasHoveredTick)
    {
        if (bRemoveKeyDown)
        {
            if (fHoveredTick != v2TickRange.x)
            {  // remove a key point
                for (const auto& hCurve : aCurves)
                    hCurve->RemovePoint(fHoveredTick);
                bCurveChanged = true;
            }
        }
        else
        {
            fCurrTick = fHoveredTick;
        }
    }

    // draw tick indicator
    const ImVec2 v2TickIndicatorPos(v2SliderRectLt.x+((fCurrTick-v2TickRange.x)/fTickLength)*fSliderWdgWidth, v2SliderRectRb.y+fKeyFrameIndicatorRadius+2);
    const ImVec2 v2TickIndicatorSize(10, 6);
    const ImU32 u32TickIndicatorColor{IM_COL32(80, 180, 80, 255)};
    if (fCurrTick >= v2TickRange.x && fCurrTick <= v2TickRange.y)
    {
        pDrawList->AddTriangleFilled(
                v2TickIndicatorPos,
                v2TickIndicatorPos+ImVec2(-v2TickIndicatorSize.x/2, v2TickIndicatorSize.y),
                v2TickIndicatorPos+ImVec2(v2TickIndicatorSize.x/2, v2TickIndicatorSize.y),
                u32TickIndicatorColor);
    }

    v2ViewSize.y = v2TickIndicatorPos.y+v2TickIndicatorSize.y-v2ViewPos.y+1;
    ImRect bb(v2ViewPos, v2ViewPos+v2ViewSize);
    ItemAdd(bb, 0);
    SetCursorScreenPos(ImVec2(v2ViewPos.x, v2ViewPos.y+v2ViewSize.y));

    const bool bMouseInView = bb.Contains(v2MousePosAbs);
    if (bRemoveKeyDown && bMouseInView && fCurrTick != v2TickRange.x)
        SetMouseCursor(ImGuiMouseCursor_Minus);
    return bCurveChanged;
}

Editor::CurveUiObj::CurveUiObj(Editor* pOwner, Curve::Holder hCurve, ValueDimension eDim, ImU32 u32CurveColor)
    : m_pOwner(pOwner), m_hCurve(hCurve), m_eDim(eDim), m_u32CurveColor(u32CurveColor)
{
    ImColor tHoverColor(u32CurveColor);
    tHoverColor.Value += ImVec4(0.1, 0.1, 0.1, 0);
    m_u32CurveHoverColor = (ImU32)tHoverColor;
    hCurve->RigisterCallbacks(this);
}

Editor::CurveUiObj::~CurveUiObj()
{
    m_hCurve->UnrigsterCallbacks(this);
}

void Editor::CurveUiObj::DrawCurve(ImDrawList* pDrawList, const ImVec2& v2OriginPos, bool bIsHovering) const
{
    const auto szKpCnt = m_hCurve->GetKeyPointCount();
    if (szKpCnt == 0)
        return;
    const auto v2DrawOffset = v2OriginPos+m_pOwner->m_v2PanOffset;
    // draw curve contour
    const auto u32CurveColor = bIsHovering ? m_u32CurveHoverColor : m_u32CurveColor;
    const auto fCurveWidth = bIsHovering ? m_pOwner->m_fCurveHoverWidth : m_pOwner->m_fCurveWidth;
    auto itKp = m_hCurve->GetKpIter(0);
    const auto itEnd = m_hCurve->GetKpIter(szKpCnt);
    bool bStartPoint = true;
    ImVec2 p1, p2;
    do
    {
        const auto& hKp = *itKp++;
        p1 = p2;
        const auto& aCps = m_aCpTable.at(hKp);
        p2 = aCps[0];
        if (bStartPoint)
        {
            if (p2.x > 0)
                pDrawList->AddLine(ImVec2(0, p2.y)+v2DrawOffset, p2+v2DrawOffset, u32CurveColor, fCurveWidth);
            bStartPoint = false;
        }
        else
            pDrawList->AddLine(p1+v2DrawOffset, p2+v2DrawOffset, u32CurveColor, fCurveWidth);

        const auto szCpCnt = aCps.size();
        for (auto j = 1; j < szCpCnt; j++)
        {
            p1 = p2;
            p2 = aCps[j];
            pDrawList->AddLine(p1+v2DrawOffset, p2+v2DrawOffset, u32CurveColor, fCurveWidth);
        }
    } while (itKp != itEnd);
    const auto fMaxPosX = v2OriginPos.x+m_pOwner->m_v2CurveAxisAreaSize.x;
    if (p2.x+v2DrawOffset.x < fMaxPosX)
        pDrawList->AddLine(p2+v2DrawOffset, ImVec2(fMaxPosX, p2.y+v2DrawOffset.y), u32CurveColor, fCurveWidth);
    // draw key points
    int iHoveredKpIdx = bIsHovering ? m_pOwner->m_iHoveredKeyPointIdx : -1;
    const auto fKeyPointRadius = m_pOwner->m_fKeyPointRadius;
    size_t idx = 0;
    itKp = m_hCurve->GetKpIter(0);
    while (itKp != itEnd)
    {
        const auto u32KpColor = idx==iHoveredKpIdx ? m_pOwner->m_u32KeyPointHoverColor : m_pOwner->m_u32KeyPointColor;
        const auto& hKp = *itKp++; idx++;
        const auto v2KpPos = m_aCpTable.at(hKp)[0];
        ImVec2 aPolyPoints[] = {
                ImVec2(v2KpPos.x+fKeyPointRadius, v2KpPos.y), ImVec2(v2KpPos.x, v2KpPos.y+fKeyPointRadius),
                ImVec2(v2KpPos.x-fKeyPointRadius, v2KpPos.y), ImVec2(v2KpPos.x, v2KpPos.y-fKeyPointRadius) };
        for (int j = 0; j < 4; j++)
            aPolyPoints[j] += v2DrawOffset;
        pDrawList->AddConvexPolyFilled(aPolyPoints, 4, u32KpColor);
        const auto fPointEdgeWidth = fCurveWidth-0.5f;
        aPolyPoints[0].x += fPointEdgeWidth; aPolyPoints[1].y += fPointEdgeWidth;
        aPolyPoints[2].x -= fPointEdgeWidth; aPolyPoints[3].y -= fPointEdgeWidth;
        const auto fPointEdgeColor = m_pOwner->m_u32KeyPointEdgeColor;
        pDrawList->AddPolyline(aPolyPoints, 4, fPointEdgeColor, ImDrawFlags_Closed, fCurveWidth);
    }
}

int Editor::CurveUiObj::MoveKeyPoint(int iKpIdx, const ImVec2& v2MousePos)
{
    const auto v2NormedKpDimVal = m_pOwner->CvtPos2Point(v2MousePos);
    const auto v2KpDimVal = v2NormedKpDimVal*m_v2DimValRange+m_v2DimMinVal;
    const auto iRet =m_hCurve->EditPointByDim(m_eDim, iKpIdx, v2KpDimVal);
    if (iRet >= 0)
    {
        UpdateContourPoints(iRet);
        iKpIdx = iRet;
    }
    else if (iRet != -2)
        std::cerr << "[Editor::CurveUiObj::MoveKeyPoint] FAILED to invoke 'Curve::EditPointByDim()' with arguments: eDim="
                << m_eDim << ", idx=" << iKpIdx << ", v2DimVal=(" << v2KpDimVal.x << ", " << v2KpDimVal.y << ")." << std::endl;
    return iKpIdx;
}

void Editor::CurveUiObj::MoveCurveVertically(const ImVec2& v2MousePos)
{
    const auto v2NormedKpDimVal = m_pOwner->CvtPos2Point(v2MousePos);
    const auto v2KpDimVal = v2NormedKpDimVal*m_v2DimValRange+m_v2DimMinVal;
    const auto fKpMoveOffsetV = m_hCurve->MoveVerticallyByDim(m_eDim, v2KpDimVal);
    if (fKpMoveOffsetV != 0)
    {
        const auto fUiMoveOffsetV = -fKpMoveOffsetV/m_v2DimValRange.y*m_pOwner->m_v2CurveAxisAreaSize.y;
        for (auto& elem : m_aCpTable)
        {
            auto& aCps = elem.second;
            for (auto& cp : aCps)
                cp.y += fUiMoveOffsetV;
        }
    }
}

void Editor::CurveUiObj::ScaleContour(const ImVec2& v2Scale)
{
    for (auto& elem2 : m_aCpTable)
    {
        auto& aCps = elem2.second;
        for (auto& v2Cp : aCps)
            v2Cp *= v2Scale;
    }
}

void Editor::CurveUiObj::UpdateCurveAttributes()
{
    const auto& tValRange = m_hCurve->GetValueRange();
    const auto v2DimValRange = ImVec2(tValRange.w, KeyPoint::GetDimVal(tValRange, m_eDim));
    if (m_v2DimValRange != v2DimValRange)
    {
        m_v2DimValRange = v2DimValRange;
        m_bNeedRefreshContour = true;
    }
    const auto& tMinVal = m_hCurve->GetMinVal();
    const auto v2DimMinVal = ImVec2(tMinVal.w, KeyPoint::GetDimVal(tMinVal, m_eDim));
    if (m_v2DimMinVal != v2DimMinVal)
    {
        m_v2DimMinVal = v2DimMinVal;
        m_bNeedRefreshContour = true;
    }
}

// void Editor::CurveUiObj::UpdateAllContourPoints(const ImVec2& v2TimeRange)
// {
//     if (v2TimeRange.y <= v2TimeRange.x)
//         return;
//     const ImVec2 v2DimValRange(v2TimeRange.y-v2TimeRange.x, m_v2DimValRange.y);
//     const ImVec2 v2DimMinVal(v2TimeRange.x, m_v2DimMinVal.y);
//     const auto szKpCnt = m_hCurve->GetKeyPointCount();
//     if (szKpCnt < 1)
//     {
//         m_aCpTable.clear();
//         return;
//     }
//     size_t szKpIdx = 0;
//     auto itKp = m_hCurve->GetKpIter(0);
//     auto itEnd = m_hCurve->GetKpIter(szKpCnt);
//     while (itKp != itEnd && (*itKp)->t < v2TimeRange.x)
//     { itKp++; szKpIdx++; }
//     ImVec2 p1, p2, v2NormedKpDimVal;
//     if ((*itKp)->t > v2TimeRange.x)
//         v2NormedKpDimVal = ImVec2(0, KeyPoint::GetDimVal(m_hCurve->CalcPointVal(v2TimeRange.x), m_eDim)-v2DimMinVal.y)/v2DimValRange;
//     else
//         v2NormedKpDimVal = ImVec2(0, KeyPoint::GetDimVal((*itKp)->val, m_eDim)-v2DimMinVal.y)/v2DimValRange;
//     p2 = m_pOwner->CvtPoint2Pos(v2NormedKpDimVal); p2.y = -p2.y;
//     while (itKp != itEnd && (*itKp)->t < v2TimeRange.y)
//     {
//         p1 = p2;
//     }
// }

void Editor::CurveUiObj::UpdateContourPoints(int iKpIdx)
{
    const auto szKpCnt = m_hCurve->GetKeyPointCount();
    if (szKpCnt < 1)
    {
        m_aCpTable.clear();
        return;
    }
    if (iKpIdx >= (int)szKpCnt)
        return;
    size_t szStartIdx, szStopIdx;
    if (iKpIdx < 0)
    { szStartIdx = 0; szStopIdx = szKpCnt-1; m_bNeedRefreshContour = false; }
    else if (iKpIdx == 0)
    { szStartIdx = 0; szStopIdx = szKpCnt > 1 ? 1 : 0; }
    else if (iKpIdx == szKpCnt-1)
    { szStartIdx = szKpCnt > 1 ? iKpIdx-1 : 0; szStopIdx = iKpIdx; }
    else
    { szStartIdx = iKpIdx-1; szStopIdx = iKpIdx+1; }
    auto itKp = m_hCurve->GetKpIter(szStartIdx);
    auto itStop = m_hCurve->GetKpIter(szStopIdx);
    auto v2NormedKpDimVal = ((*itKp)->GetVec2PointValByDim(m_eDim)-m_v2DimMinVal)/m_v2DimValRange;
    ImVec2 p1, p2;
    p2 = m_pOwner->CvtPoint2Pos(v2NormedKpDimVal); p2.y = -p2.y;
    while (itKp != itStop)
    {
        p1 = p2;
        const auto& hKp1 = *itKp++;
        const auto& hKp2 = *itKp;
        v2NormedKpDimVal = (hKp2->GetVec2PointValByDim(m_eDim)-m_v2DimMinVal)/m_v2DimValRange;
        p2 = m_pOwner->CvtPoint2Pos(v2NormedKpDimVal); p2.y = -p2.y;
        size_t steps = (size_t)(CalcDistance(p1, p2)/2);
        steps = ImClamp<size_t>(steps, 2, 100);
        auto eCurveType = hKp2->type;
        std::vector<ImVec2> aContourPos(steps);
        aContourPos[0] = p1;
        for (auto j = 1; j < steps; j++)
        {
            const float s = (float)j/steps;
            const auto t = ImLerp(p1.x, p2.x, s);
            const auto rt = SmoothStep(p1.x, p2.x, t, eCurveType);
            aContourPos[j].x = t;
            aContourPos[j].y = p1.y==p2.y ? p1.y : ImLerp(p1.y, p2.y, rt);
        }
        m_aCpTable[hKp1] = std::move(aContourPos);
    }
    if (iKpIdx < 0 || iKpIdx == szKpCnt-1)
    {
        const auto& hKp2 = *itKp;
        v2NormedKpDimVal = (hKp2->GetVec2PointValByDim(m_eDim)-m_v2DimMinVal)/m_v2DimValRange;
        p2 = m_pOwner->CvtPoint2Pos(v2NormedKpDimVal); p2.y = -p2.y;
        m_aCpTable[hKp2] = { p2 };
    }
}

bool Editor::CurveUiObj::CheckMouseHoverCurve(const ImVec2& _v2MousePos) const
{
    const ImVec2 v2MousePos(_v2MousePos.x, -_v2MousePos.y);
    const auto szKpCnt = m_hCurve->GetKeyPointCount();
    if (szKpCnt < 1)
        return false;
    if (szKpCnt == 1)
        return CheckMouseHoverPoint(v2MousePos);

    const auto fCheckHoverDistanceThresh = m_pOwner->m_fCheckHoverDistanceThresh;
    bool bIsHovering = false;
    bool bStartPoint = true;
    auto itKp = m_hCurve->GetKpIter(0);
    const auto itEnd = m_hCurve->GetKpIter(szKpCnt);
    ImVec2 p1, p2;
    while (true)
    {
        const auto& hKp = *itKp++;
        if (itKp == itEnd)
            break;
        p1 = p2;
        const auto& aKpCurvePoints = m_aCpTable.at(hKp);
        for (const auto& v2Cp : aKpCurvePoints)
        {
            p2 = v2Cp;
            if (bStartPoint)
                bStartPoint = false;
            else if (CalcDistance(v2MousePos, p1, p2) < fCheckHoverDistanceThresh)
            {
                bIsHovering = true;
                break;
            }
            p1 = p2;
        }
        if (bIsHovering)
            break;
    }
    return bIsHovering;
}

int Editor::CurveUiObj::CheckMouseHoverPoint(const ImVec2& _v2MousePos) const
{
    const ImVec2 v2MousePos(_v2MousePos.x, -_v2MousePos.y);
    const auto szKpCnt = m_hCurve->GetKeyPointCount();
    if (szKpCnt < 1)
        return -1;
    const auto fCheckHoverDistanceThresh = m_pOwner->m_fCheckHoverDistanceThresh;
    size_t idx = 0;
    auto itKp = m_hCurve->GetKpIter(0);
    const auto itEnd = m_hCurve->GetKpIter(szKpCnt);
    while (itKp != itEnd)
    {
        const auto& hKp = *itKp++;
        const auto& p = m_aCpTable.at(hKp)[0];
        if (CalcDistance(v2MousePos, p) < fCheckHoverDistanceThresh)
            break;
        idx++;
    }
    if (idx >= szKpCnt)
        return -1;
    return (int)idx;
}

ImVec2 Editor::CurveUiObj::CalcPointValue(const ImVec2& v2MousePos) const
{
    const auto v2NormedKpDimVal = m_pOwner->CvtPos2Point(v2MousePos);
    const auto fTick = v2NormedKpDimVal.x*m_v2DimValRange.x+m_v2DimMinVal.x;
    const auto tKpVal = m_hCurve->CalcPointVal(fTick);
    return ImVec2(tKpVal.w, KeyPoint::GetDimVal(tKpVal, m_eDim));
}

void Editor::CurveUiObj::OnKeyPointAdded(size_t szKpIdx, KeyPoint::Holder hKp)
{
    UpdateContourPoints(szKpIdx);
}

void Editor::CurveUiObj::OnKeyPointRemoved(size_t szKpIdx, KeyPoint::Holder hKp)
{
    UpdateContourPoints(szKpIdx);
}

void Editor::CurveUiObj::OnKeyPointChanged(size_t szKpIdx, KeyPoint::Holder hKp)
{
    UpdateContourPoints(szKpIdx);
}

void Editor::CurveUiObj::OnContourNeedUpdate(size_t szKpIdx)
{
    UpdateContourPoints(szKpIdx);
}

Editor::Editor()
{}

bool Editor::DrawContent(const char* pcLabel, const ImVec2& v2ViewSize, float fViewScaleX, float fViewOffsetX, uint32_t flags, bool* pCurveUpdated, ImDrawList* pDrawList)
{
    bool bMouseCaptured = false;
    if (pCurveUpdated) *pCurveUpdated = false;
    ImGuiIO& io = GetIO();
    const bool bDeleteMode = IsKeyDown(ImGuiKey_LeftShift) && io.KeyShift;
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    PushStyleColor(ImGuiCol_Border, 0);
    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.5f));
    BeginChildFrame(GetID(pcLabel), v2ViewSize);
    const ImVec2 v2CursorPos = GetCursorScreenPos();
    if (!pDrawList) pDrawList = GetWindowDrawList();

    const auto v2CurveAreaPos = v2CursorPos+m_v2ViewPadding;
    const auto v2CurveAreaSize = v2ViewSize-m_v2ViewPadding*2;
    const ImRect rCurveArea(v2CursorPos, v2CursorPos+m_v2CurveAreaSize);
    const auto v2CurveOriginPos = v2CurveAreaPos+ImVec2(0, v2CurveAreaSize.y*(1-m_fCurveAreaBottomMargin));
    const auto v2MousePos = GetMousePos();

    if (m_v2CurveAreaSize != v2CurveAreaSize || fViewScaleX != m_v2UiScale.x)
    {
        auto v2Scale = v2CurveAreaSize/m_v2CurveAreaSize;
        v2Scale.x *= fViewScaleX/m_v2UiScale.x;
        for (auto& hCurveUiObj : m_aCurveUiObjs)
            hCurveUiObj->ScaleContour(v2Scale);
        m_v2CurveAreaSize = v2CurveAreaSize;
        m_v2UiScale.x = fViewScaleX;
        m_v2CurveAxisAreaSize.x = m_v2CurveAreaSize.x;
        m_v2CurveAxisAreaSize.y = v2CurveAreaSize.y*(1-m_fCurveAreaBottomMargin-m_fCurveAreaBottomMargin);
    }
    m_v2PanOffset.x = -fViewOffsetX;

    // draw graticule lines
    if (m_szGraticuleLineCnt > 0)
    {
        const float fScaledGraticuleHeight = m_szGraticuleLineCnt > 1 ? m_v2CurveAxisAreaSize.y*m_v2UiScale.y/(m_szGraticuleLineCnt-1) : 0;
        for (auto i = 0; i < m_szGraticuleLineCnt; i++)
        {
            const float fYOffset = -fScaledGraticuleHeight*i;
            const auto p1 = v2CurveOriginPos+ImVec2(0, fYOffset+m_v2PanOffset.y);
            const auto p2 = v2CurveOriginPos+ImVec2(v2CurveAreaSize.x, fYOffset+m_v2PanOffset.y);
            if (i == 0)
                pDrawList->AddLine(p1, p2, m_u32MinValLineColor, m_fMinMaxLineThickness);
            else if (i == m_szGraticuleLineCnt-1)
                pDrawList->AddLine(p1, p2, m_u32MaxValLineColor, m_fMinMaxLineThickness);
            else
                pDrawList->AddLine(p1, p2, m_u32GraticuleColor, m_fGraticuleLineThickness);
        }
    }

    // handle zoom and scroll vertically
    if ((flags&IMNEWCURVE_EDITOR_FLAG_ZOOM_V) > 0)
    {
        if (rCurveArea.Contains(v2MousePos))
        {
            const auto fMouseWheel = io.MouseWheel*0.1f;
            if (fabs(fMouseWheel) > FLT_EPSILON)
            {
                if (io.KeyCtrl)
                {
                    const float r = 1+ImClamp(fMouseWheel, -0.5f, 0.5f);
                    const auto fNewScaleY = ImClamp(m_v2UiScale.y*r, 1.f, m_fMaxScale);
                    if (fNewScaleY != m_v2UiScale.y)
                    {
                        m_v2UiScale.y = fNewScaleY;
                        for (auto& hCurveUiObj : m_aCurveUiObjs)
                            hCurveUiObj->SetRefreshContour();
                        const auto fMaxOffset = v2CurveAreaSize.y*(m_v2UiScale.y > 1 ? m_v2UiScale.y-1 : 0);
                        if (m_v2PanOffset.y > fMaxOffset) m_v2PanOffset.y = fMaxOffset;
                    }
                }
                else
                {
                    const auto fMaxOffset = v2CurveAreaSize.y*(m_v2UiScale.y > 1 ? m_v2UiScale.y-1 : 0);
                    auto fPanOffsetY = fMouseWheel*v2CurveAreaSize.y+m_v2PanOffset.y;
                    if (fPanOffsetY > fMaxOffset) fPanOffsetY = fMaxOffset;
                    else if (fPanOffsetY < 0) fPanOffsetY = 0;
                    if (fPanOffsetY != m_v2PanOffset.y)
                        m_v2PanOffset.y = fPanOffsetY;
                }
            }
        }
    }

    for (auto& hCurveUiObj : m_aCurveUiObjs)
    {
        hCurveUiObj->UpdateCurveAttributes();
        if (hCurveUiObj->NeedRrefreshContour())
            hCurveUiObj->UpdateContourPoints(-1);
    }

    const auto szCurveCnt = m_aCurveUiObjs.size();
    const ImVec2 v2MousePosToCurveOrigin(v2MousePos.x-v2CurveOriginPos.x-m_v2PanOffset.x, v2CurveOriginPos.y-v2MousePos.y+m_v2PanOffset.y);
    int iCurveHoveringIdx = -1, iPointHoveringIdx = -1;
    if (!m_bIsDragging)
    {
        // check hovering state
        if (rCurveArea.Contains(v2MousePos))
        {
            // if there is already a hovering curve, check its hovering state first
            if (m_iHoveredCurveUiObjIdx >= 0 &&
                m_aCurveUiObjs[m_iHoveredCurveUiObjIdx]->CheckMouseHoverCurve(v2MousePosToCurveOrigin))
                iCurveHoveringIdx = m_iHoveredCurveUiObjIdx;
            if (iCurveHoveringIdx < 0)
            {
                // check other curves' hovering state
                for (auto i = 0; i < szCurveCnt; i++)
                {
                    if ((int)i == m_iHoveredCurveUiObjIdx)
                        continue;
                    if (m_aCurveUiObjs[i]->CheckMouseHoverCurve(v2MousePosToCurveOrigin))
                    {
                        iCurveHoveringIdx = i;
                        break;
                    }
                }
            }
            // if there is a hovering curve, check whether there is a key point under hovering
            if (iCurveHoveringIdx >= 0)
                iPointHoveringIdx = m_aCurveUiObjs[iCurveHoveringIdx]->CheckMouseHoverPoint(v2MousePosToCurveOrigin);
        }
        m_iHoveredCurveUiObjIdx = iCurveHoveringIdx;
        m_iHoveredKeyPointIdx = iPointHoveringIdx;
    }
    else
    {
        iCurveHoveringIdx = m_iHoveredCurveUiObjIdx;
        iPointHoveringIdx = m_iHoveredKeyPointIdx;
    }

    // draw curves
    // 1st, draw curves except the hovering one
    for (auto i = 0; i < szCurveCnt; i++)
    {
        const auto& hCurveUiObj = m_aCurveUiObjs[i];
        if ((int)i == iCurveHoveringIdx || !hCurveUiObj->IsVisible())
            continue;
        hCurveUiObj->DrawCurve(pDrawList, v2CurveOriginPos, false);
    }
    // 2nd, draw the hovering curve if there is one, to make sure the hovering curve is drawn ontop of the others
    if (iCurveHoveringIdx >= 0)
        m_aCurveUiObjs[iCurveHoveringIdx]->DrawCurve(pDrawList, v2CurveOriginPos, true);

    // handle mouse operation to manipulate curves
    bool bCurveUpdated = false;
    if (IsMouseDown(ImGuiMouseButton_Left))
    {
        if (iPointHoveringIdx >= 0 || iCurveHoveringIdx >= 0)
        {
            m_bIsDragging = true;
            bMouseCaptured = true;
        }
    }
    else if (IsMouseReleased(ImGuiMouseButton_Left))
    {
        m_bIsDragging = false;
    }
    if (m_bIsDragging)
    {
        // handle move key point
        if (iPointHoveringIdx >= 0)
        {
            m_iHoveredKeyPointIdx = m_aCurveUiObjs[iCurveHoveringIdx]->MoveKeyPoint(iPointHoveringIdx, v2MousePosToCurveOrigin);
            bCurveUpdated = true;
        }
        // handle move curve
        else if (iCurveHoveringIdx >= 0 && (flags&IMNEWCURVE_EDITOR_FLAG_MOVE_CURVE_V) > 0)
        {
            m_aCurveUiObjs[iCurveHoveringIdx]->MoveCurveVertically(v2MousePosToCurveOrigin);
            bCurveUpdated = true;
        }
    }

    if (m_bShowValueToolTip && iCurveHoveringIdx >= 0)
    {
        const auto& hCurveUiObj = m_aCurveUiObjs[iCurveHoveringIdx];
        const auto v2PointVal = hCurveUiObj->CalcPointValue(v2MousePosToCurveOrigin);
        std::ostringstream oss; oss << hCurveUiObj->GetCurveName();
        switch (hCurveUiObj->GetDim())
        {
            case DIM_X: oss << "[X]"; break;
            case DIM_Y: oss << "[Y]"; break;
            case DIM_Z: oss << "[Z]"; break;
            default: oss << "[T]"; break;
        }
        const auto strToolTipName = oss.str();
        SetTooltip("%s: %.03f, %.03f", strToolTipName.c_str(), v2PointVal.x, v2PointVal.y);
    }

    EndChildFrame();
    PopStyleColor(2);
    PopStyleVar();
    if (pCurveUpdated) *pCurveUpdated = bCurveUpdated;
    return bMouseCaptured;
}

bool Editor::AddCurve(Curve::Holder hCurve, ValueDimension eDim, ImU32 u32CurveColor)
{
    m_aCurveUiObjs.push_back(CurveUiObj::Holder(new CurveUiObj(this, hCurve, eDim, u32CurveColor)));
    return true;
}

// imgui_json::value Editor::SaveAsJson() const
// {
//     imgui_json::value j;
//     j["background_color"] = imgui_json::number(m_u32BackgroundColor);
//     j["graticule_color"] = imgui_json::number(m_u32GraticuleColor);
//     j["time_range"] = imgui_json::vec2(m_v2TimeRange);
//     j["check_hover_distance_thresh"] = imgui_json::number(m_fCheckHoverDistanceThresh);
//     j["ui_scale"] = imgui_json::vec2(m_v2UiScale);
//     j["pan_offset"] = imgui_json::vec2(m_v2PanOffset);
//     j["show_value_tool_tip"] = imgui_json::boolean(m_bShowValueToolTip);
//     imgui_json::array jaCurves;
//     for (const auto& hCurve : m_aCurveUiObjs)
//         jaCurves.push_back(hCurve->SaveAsJson());
//     j["curves"] = jaCurves;
//     return std::move(j);
// }

// void Editor::LoadFromJson(const imgui_json::value& j)
// {
//     m_u32BackgroundColor = (ImU32)j["background_color"].get<imgui_json::number>();
//     m_u32GraticuleColor = (ImU32)j["graticule_color"].get<imgui_json::number>();
//     m_v2TimeRange = j["time_range"].get<imgui_json::vec2>();
//     m_fCheckHoverDistanceThresh = (float)j["check_hover_distance_thresh"].get<imgui_json::number>();
//     m_v2UiScale = j["ui_scale"].get<imgui_json::vec2>();
//     m_v2PanOffset = j["pan_offset"].get<imgui_json::vec2>();
//     m_bShowValueToolTip = j["show_value_tool_tip"].get<imgui_json::boolean>();
//     imgui_json::array jaCurves = j["curves"].get<imgui_json::array>();
//     m_aCurveUiObjs.clear();
//     m_aCurveUiObjs.reserve(jaCurves.size());
//     for (const auto& jelem : jaCurves)
//     {
//         auto hCurve = CurveUiObj::CreateFromJson(this, jelem);
//         m_aCurveUiObjs.push_back(hCurve);
//     }
// }

Editor::Holder Editor::CreateInstance()
{
    return Editor::Holder(new Editor());
}

}  // ~ namespace ImCurve
}  // ~ namespace ImGui
