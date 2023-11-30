#include <iostream>
#include <algorithm>
#include <sstream>
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
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal)
{
    return Curve::Holder(new Curve(name, eCurveType, tTimeBase, minVal, maxVal, defaultVal));
}

Curve::Holder Curve::CreateInstance(
        const std::string& name, CurveType eCurveType,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal)
{
    return CreateInstance(name, eCurveType, {0, 0}, minVal, maxVal, defaultVal);
}

Curve::Holder Curve::CreateFromJson(const imgui_json::value& j)
{
    auto hCurve = Curve::Holder(new Curve());
    hCurve->LoadFromJson(j);
    return hCurve;
}

Curve::Curve(const std::string& name, CurveType eCurveType, const std::pair<uint32_t, uint32_t>& tTimeBase,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal)
    : m_strName(name), m_eCurveType(eCurveType), m_tTimeBase(tTimeBase), m_tDefaultVal(defaultVal)
{
    m_tMinVal = Min(minVal, maxVal);
    m_tMaxVal = Max(minVal, maxVal);
    m_tValRange = m_tMaxVal-m_tMinVal;
    m_tValRange.w = 1.f;
    m_bTimeBaseValid = tTimeBase.first > 0 && tTimeBase.second > 0;
    if (m_bTimeBaseValid)
    {
        m_tMinVal.w = Time2TickAligned(m_tMinVal.w);
        m_tMaxVal.w = Time2TickAligned(m_tMaxVal.w);
    }
}

Curve::Holder Curve::Clone() const
{
    auto hNewInstance = CreateInstance(m_strName, m_eCurveType, m_tTimeBase, m_tMinVal, m_tMaxVal, m_tDefaultVal);
    auto& aClonedKeyPoints = hNewInstance->m_aKeyPoints;
    for (const auto& hKp : m_aKeyPoints)
        aClonedKeyPoints.push_back(KeyPoint::Holder(new KeyPoint(*hKp)));
    return hNewInstance;
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

int Curve::AddPoint(KeyPoint::Holder hKp, bool bNormalize, bool bOverwriteIfExists)
{
    hKp->t = Time2TickAligned(hKp->t);
    auto iter = std::find_if(m_aKeyPoints.begin(), m_aKeyPoints.end(), [hKp] (const auto& elem) {
        return hKp->t == elem->t;
    });
    if (iter != m_aKeyPoints.end() && !bOverwriteIfExists)
        return -1;

    if (bNormalize)
    {
        const auto t = hKp->t;
        hKp->val = (hKp->val-m_tMinVal)/(m_tValRange+FLT_EPSILON);
        hKp->t = t;
    }

    if (iter == m_aKeyPoints.end())
    {
        m_aKeyPoints.push_back(hKp);
        SortKeyPoints();
    }
    else
        (*iter)->val = hKp->val;
    return GetKeyPointIndex(hKp->t);
}

int Curve::AddPointByDim(ValueDimension eDim, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize, bool bOverwriteIfExists)
{
    KeyPoint::ValType tKpVal(m_tDefaultVal);
    KeyPoint::SetDimVal(tKpVal, v2DimVal.y, eDim);
    tKpVal.w = v2DimVal.x;
    auto hKp = KeyPoint::CreateInstance(tKpVal, eCurveType);
    return AddPoint(hKp, bNormalize, bOverwriteIfExists);
}

int Curve::EditPoint(size_t idx, const KeyPoint::ValType& tKpVal, CurveType eCurveType, bool bNormalize)
{
    const auto szKpCnt = m_aKeyPoints.size();
    if (idx >= szKpCnt)
        return -1;

    KeyPoint::ValType tKpVal_;
    if (bNormalize)
        tKpVal_ = (tKpVal-m_tMinVal)/(m_tValRange+FLT_EPSILON);
    else
        tKpVal_ = tKpVal;
    tKpVal_.w = Time2TickAligned(tKpVal.w);
    auto hKp = GetKeyPoint(idx);
    if (tKpVal_ == hKp->val && eCurveType == hKp->type)
        return -2;

    float t;
    if (idx == 0)
        t = m_tMinVal.w;
    else if (idx == szKpCnt-1)
        t = m_tMaxVal.w;
    else
    {
        t = tKpVal_.w;
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
        else
        {
            if (t < m_tMinVal.w) t = m_tMinVal.w;
            else if (t > m_tMaxVal.w) t = m_tMaxVal.w;
        }
    }

    hKp->val = tKpVal_;
    hKp->t = t;
    hKp->type = eCurveType;
    SortKeyPoints();
    return GetKeyPointIndex(hKp);
}

int Curve::EditPointByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize)
{
    if (idx >= m_aKeyPoints.size())
        return -1;
    KeyPoint::ValType tKpVal = GetKeyPoint_(idx)->val;
    KeyPoint::SetDimVal(tKpVal, v2DimVal.y, eDim);
    tKpVal.w = v2DimVal.x;
    return EditPoint(idx, tKpVal, eCurveType, bNormalize);
}

int Curve::ChangePointVal(size_t idx, const KeyPoint::ValType& tKpVal, bool bNormalize)
{
    if (idx >= m_aKeyPoints.size())
        return -1;
    return EditPoint(idx, tKpVal, GetKeyPoint(idx)->type, bNormalize);
}

int Curve::ChangePointValByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, bool bNormalize)
{
    if (idx >= m_aKeyPoints.size())
        return -1;
    return EditPointByDim(eDim, idx, v2DimVal, GetKeyPoint(idx)->type, bNormalize);
}

int Curve::ChangeCurveType(size_t idx, CurveType eCurveType)
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
    while (idx-- > 0)
        iter++;
    auto hKp = *iter;
    m_aKeyPoints.erase(iter);
    return hKp;
}

KeyPoint::Holder Curve::RemovePoint(float t)
{
    auto idx = GetKeyPointIndex(t);
    if (idx < 0)
        return nullptr;
    return RemovePoint((size_t)idx);
}

float Curve::MoveVerticallyByDim(ValueDimension eDim, const ImVec2& v2SyncPoint, bool bNormalize)
{
    if (v2SyncPoint.x < m_tMinVal.w || v2SyncPoint.x > m_tMaxVal.w)
        return 0.f;
    auto v2SyncPoint_(v2SyncPoint);
    if (bNormalize)
    {
        const auto fMinVal = KeyPoint::GetDimVal(m_tMinVal, eDim);
        const auto fValRange = KeyPoint::GetDimVal(m_tValRange, eDim);
        v2SyncPoint_.y = (v2SyncPoint.y-fMinVal)/(fValRange+FLT_EPSILON);
    }
    const auto fOrgPtVal = KeyPoint::GetDimVal(CalcPointVal(v2SyncPoint_.x, false, false), eDim);
    const auto fOffset = v2SyncPoint_.y-fOrgPtVal;
    if (fabs(fOffset) < FLT_EPSILON)
        return 0.f;
    for (auto& hKp : m_aKeyPoints)
    {
        const auto fOrgVal = KeyPoint::GetDimVal(hKp->val, eDim);
        KeyPoint::SetDimVal(hKp->val, fOrgVal+fOffset, eDim);
    }
    return fOffset;
}

KeyPoint::ValType Curve::CalcPointVal(float t, bool bDenormalize, bool bAlignTime) const
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
    if (bDenormalize)
        res = res*m_tValRange+m_tMinVal;
    res.w = Tick2Time(t);
    return res;
}

bool Curve::SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds)
{
    if (v2TimeRange.x >= v2TimeRange.y)
        return false;
    if (m_bTimeBaseValid)
    {
        m_tMinVal.w = Time2TickAligned(m_tMinVal.w);
        m_tMaxVal.w = Time2TickAligned(m_tMaxVal.w);
    }
    else
    {
        m_tMinVal.w = v2TimeRange.x;
        m_tMaxVal.w = v2TimeRange.y;
    }
    m_tValRange.w = m_tMaxVal.w-m_tMinVal.w;
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt < 1 || !bDockEnds)
        return true;
    if (szKpCnt == 1)
    {
        m_aKeyPoints.front()->t = m_tMinVal.w;
        return true;
    }
    if (szKpCnt == 2)
    {
        m_aKeyPoints.front()->t = m_tMinVal.w;
        m_aKeyPoints.back()->t = m_tMaxVal.w;
        return true;
    }
    const auto eBeginCurveType = m_aKeyPoints.front()->type;
    const auto eEndCurveType = m_aKeyPoints.back()->type;
    auto tNewBeginVal = CalcPointVal(Tick2Time(m_tMinVal.w), false);
    tNewBeginVal.w = m_tMinVal.w;
    auto tNewEndVal = CalcPointVal(Tick2Time(m_tMaxVal.w), false);
    tNewEndVal.w = m_tMaxVal.w;
    std::list<KeyPoint::Holder> aNewPoints;
    aNewPoints.push_back(KeyPoint::CreateInstance(tNewBeginVal, eBeginCurveType));
    auto iter = m_aKeyPoints.begin(); iter++;
    auto itEnd = m_aKeyPoints.end(); itEnd--;
    const auto fMinTime = m_tMinVal.w;
    const auto fMaxTime = m_tMaxVal.w;
    while (iter != itEnd)
    {
        auto hKp = *iter++;
        if (hKp->t >= fMaxTime)
            break;
        if (hKp->t > fMinTime)
            aNewPoints.push_back(hKp);
    }
    aNewPoints.push_back(KeyPoint::CreateInstance(tNewEndVal, eEndCurveType));
    m_aKeyPoints = std::move(aNewPoints);
    return true;
}

bool Curve::ScaleKeyPoints(const KeyPoint::ValType& tScale)
{
    for (auto& hKp : m_aKeyPoints)
        hKp->val *= tScale;
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
    m_tValRange.w = 1.f;
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

// 'CurveUiObj' implementation
CurveUiObj::Holder CurveUiObj::CreateInstance(Editor* owner, const std::string& name, CurveType type, const std::pair<uint32_t, uint32_t>& tTimeBase,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
        ImU32 color, bool visible, int64_t id, int64_t subId)
{
    return CurveUiObj::Holder(new CurveUiObj(owner, name, type, tTimeBase, minVal, maxVal, defaultVal, color, visible, id, subId));
}

CurveUiObj::Holder CurveUiObj::CreateInstance(Editor* owner, const std::string& name, CurveType type,
        const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
        ImU32 color, bool visible, int64_t id, int64_t subId)
{
    return CreateInstance(owner, name, type, {0, 0}, minVal, maxVal, defaultVal, color, visible, id, subId);
}

CurveUiObj::Holder CurveUiObj::CreateFromJson(Editor* owner, const imgui_json::value& j)
{
    auto hCurveUiObj = CurveUiObj::Holder(new CurveUiObj(owner));
    hCurveUiObj->LoadFromJson(j);
    return hCurveUiObj;
}

bool CurveUiObj::CheckMouseHoverCurve(ValueDimension eDim, const ImVec2& pos)
{
    if (m_aKeyPoints.size() < 2)
        return false;
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
        UpdateContourPointsByDim(eDim, -1);
    auto& aDimCpTable = m_aContourPoints.at(eDim);
    auto iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        const auto& hKp = *iter;
        if (aDimCpTable.find(hKp) == aDimCpTable.end())
            UpdateDimContourPoints(eDim, iter);
        iter++;
    }

    bool bIsHovering = false;
    ImVec2 p1, p2;
    bool bStartPoint = true;
    const auto fCheckHoverDistanceThresh = m_owner->m_fCheckHoverDistanceThresh;
    iter = m_aKeyPoints.begin();
    while (true)
    {
        const auto& hKp = *iter++;
        if (iter == m_aKeyPoints.end())
            break;
        p1 = p2;
        const auto& aKpCurvePoints = aDimCpTable.at(hKp);
        for (const auto& v2CurvePt : aKpCurvePoints)
        {
            p2 = v2CurvePt;
            if (bStartPoint)
                bStartPoint = false;
            else if (CalcDistance(pos, p1, p2) < fCheckHoverDistanceThresh)
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

int CurveUiObj::CheckMouseHoverPoint(ValueDimension eDim, const ImVec2& pos) const
{
    const int iKpCnt = m_aKeyPoints.size();
    if (iKpCnt < 1)
        return false;
    auto& aDimCpTable = m_aContourPoints.at(eDim);
    const auto fCheckHoverDistanceThresh = m_owner->m_fCheckHoverDistanceThresh;
    int idx = 0;
    auto iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        const auto& hKp = *iter++;
        const auto& p = aDimCpTable.at(hKp)[0];
        if (CalcDistance(pos, p) < fCheckHoverDistanceThresh)
            break;
        idx++;
    }
    if (idx >= iKpCnt)
        return -1;
    return idx;
}

int CurveUiObj::AddPoint(KeyPoint::Holder hKp, bool bNormalize, bool bOverwriteIfExists)
{
    int idx = Curve::AddPoint(hKp, bNormalize, bOverwriteIfExists);
    if (idx < 0)
        return idx;
    UpdateContourPoints(idx);
    return idx;
}

int CurveUiObj::AddPointByDim(ValueDimension eDim, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize, bool bOverwriteIfExists)
{
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
        UpdateContourPointsByDim(eDim, -1);
    return Curve::AddPointByDim(eDim, v2DimVal, eCurveType, bNormalize, bOverwriteIfExists);
}

int CurveUiObj::EditPoint(size_t idx, const KeyPoint::ValType& tKpVal, CurveType eCurveType, bool bNormalize)
{
    int iNewIdx = Curve::EditPoint(idx, tKpVal, eCurveType, bNormalize);
    if (iNewIdx < 0)
        return iNewIdx;
    UpdateContourPoints(iNewIdx);
    if (iNewIdx != idx)
        UpdateContourPoints(idx);
    return iNewIdx;
}

int CurveUiObj::EditPointByDim(ValueDimension eDim, size_t idx, const ImVec2& v2DimVal, CurveType eCurveType, bool bNormalize)
{
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
        UpdateContourPointsByDim(eDim, -1);
    return Curve::EditPointByDim(eDim, idx, v2DimVal, eCurveType, bNormalize);
}

int CurveUiObj::ChangeCurveType(size_t idx, CurveType eCurveType)
{
    if (Curve::ChangeCurveType(idx, eCurveType) < 0)
        return -1;
    UpdateContourPoints(idx);
    return idx;
}

KeyPoint::Holder CurveUiObj::RemovePoint(size_t idx)
{
    auto hKp = Curve::RemovePoint(idx);
    if (!hKp)
        return nullptr;
    for (auto& elem : m_aContourPoints)
    {
        auto& aCpTable = elem.second;
        aCpTable.erase(hKp);
    }
    UpdateContourPoints(idx);
    return hKp;
}

KeyPoint::Holder CurveUiObj::RemovePoint(float t)
{
    auto idx = GetKeyPointIndex(t);
    if (idx < 0)
        return nullptr;
    return RemovePoint((size_t)idx);
}

float CurveUiObj::MoveVerticallyByDim(ValueDimension eDim, const ImVec2& v2SyncPoint, bool bNormalize)
{
    const auto fOffset = Curve::MoveVerticallyByDim(eDim, v2SyncPoint, bNormalize);
    if (fabs(fOffset) < FLT_EPSILON)
        return fOffset;
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
        UpdateContourPointsByDim(eDim, -1);
    else
    {
        const auto v2PosOffset = m_owner->CvtPoint2Pos(ImVec2(m_owner->m_v2TimeRange.x, fOffset));
        const auto fVPosOffset = v2PosOffset.y;
        auto& aDimCpTable = m_aContourPoints.at(eDim);
        for (auto& elem : aDimCpTable)
        {
            auto& aContourPoints = elem.second;
            for (auto& v2Pos : aContourPoints)
                v2Pos.y += fVPosOffset;
        }
    }
    return fOffset;
}

bool CurveUiObj::SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds)
{
    if (!Curve::SetTimeRange(v2TimeRange, bDockEnds))
        return false;
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt > 1)
    {
        for (const auto& elem : m_aContourPoints)
        {
            const auto eDim = elem.first;
            UpdateContourPointsByDim(eDim, 0);
            if (szKpCnt > 2)
                UpdateContourPointsByDim(eDim, szKpCnt-1);
        }
    }
    if (bDockEnds)
        RemoveInvalidContourPoints();
    return true;
}

bool CurveUiObj::DrawDim(ValueDimension eDim, ImDrawList* pDrawList, const ImVec2& v2CursorPos, bool bIsHovering)
{
    if (m_aKeyPoints.size() < 1)
        return false;
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
        UpdateContourPointsByDim(eDim, -1);
    auto& aDimCpTable = m_aContourPoints.at(eDim);
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt > 1)
    {
        auto iter = m_aKeyPoints.begin();
        while (iter != m_aKeyPoints.end())
        {
            const auto& hKp = *iter;
            if (aDimCpTable.find(hKp) == aDimCpTable.end())
                UpdateDimContourPoints(eDim, iter);
            iter++;
        }
    }

    const ImVec2 v2DrawOffset = v2CursorPos+ImVec2(0, m_owner->m_v2CurveAreaSize.y*m_owner->m_v2UiScale.y)+m_owner->m_v2PanOffset;
    // draw curve contour
    ImVec2 p1, p2;
    bool bStartPoint = true;
    const auto u32CurveColor = bIsHovering ? m_u32CurveHoveringColor : m_u32CurveColor;
    const auto fCurveWidth = bIsHovering ? m_fCurveHoveringWidth : m_fCurveWidth;
    auto iter = m_aKeyPoints.begin();
    while (true)
    {
        const auto& hKp = *iter++;
        if (iter == m_aKeyPoints.end())
            break;
        p1 = p2;
        const auto& aKpCurvePoints = aDimCpTable.at(hKp);
        p2 = aKpCurvePoints[0];
        if (bStartPoint)
            bStartPoint = false;
        else
            pDrawList->AddLine(p1+v2DrawOffset, p2+v2DrawOffset, u32CurveColor, fCurveWidth);

        const auto szKpCurvePointsCnt = aKpCurvePoints.size();
        for (auto j = 1; j < szKpCurvePointsCnt; j++)
        {
            p1 = p2;
            p2 = aKpCurvePoints[j];
            pDrawList->AddLine(p1+v2DrawOffset, p2+v2DrawOffset, u32CurveColor, fCurveWidth);
        }
    }
    // draw key points
    int iHoveringKpIdx = bIsHovering ? m_owner->m_iPointHoveringIdx : -1;
    const auto fKeyPointRadius = m_fKeyPointRadius;
    size_t idx = 0;
    iter = m_aKeyPoints.begin();
    while (iter != m_aKeyPoints.end())
    {
        const auto u32KeyPointColor = idx==iHoveringKpIdx ? m_u32KeyPointHoveringColor : m_u32KeyPointColor;
        const auto& hKp = *iter++; idx++;
        const auto v2KpCenter = m_owner->CvtPoint2Pos(hKp->GetVec2PointValByDim(eDim));
        ImVec2 aPolyPoints[] = {
                ImVec2(v2KpCenter.x+fKeyPointRadius, v2KpCenter.y), ImVec2(v2KpCenter.x, v2KpCenter.y+fKeyPointRadius),
                ImVec2(v2KpCenter.x-fKeyPointRadius, v2KpCenter.y), ImVec2(v2KpCenter.x, v2KpCenter.y-fKeyPointRadius) };
        for (int j = 0; j < 4; j++)
            aPolyPoints[j] += v2DrawOffset;
        pDrawList->AddConvexPolyFilled(aPolyPoints, 4, u32KeyPointColor);
        const auto fPointEdgeWidth = fCurveWidth-0.5;
        aPolyPoints[0].x += fPointEdgeWidth; aPolyPoints[1].y += fPointEdgeWidth;
        aPolyPoints[2].x -= fPointEdgeWidth; aPolyPoints[3].y -= fPointEdgeWidth;
        const auto fPointEdgeColor = m_u32KeyPointEdgeColor;
        pDrawList->AddPolyline(aPolyPoints, 4, fPointEdgeColor, ImDrawFlags_Closed, fCurveWidth);
    }
    return false;
}

void CurveUiObj::UpdateContourPoints(int idx)
{
    for (const auto& elem : m_aContourPoints)
    {
        const auto eDim = elem.first;
        UpdateContourPointsByDim(eDim, idx);
    }
}

void CurveUiObj::UpdateContourPointsByDim(ValueDimension eDim, int idx)
{
    if (m_aContourPoints.find(eDim) == m_aContourPoints.end())
    {
        m_aContourPoints[eDim] = {};
        idx = -1;
    }
    auto& aDimCpTable = m_aContourPoints.at(eDim);
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt < 2)
    {
        aDimCpTable.clear();
        return;
    }
    if (idx >= (int)szKpCnt)
        return;
    ImVec2 p1, p2;
    size_t szLoopStart = idx > 0 ? idx-1 : 0;
    size_t szLoopEnd = idx >= 0 ? idx+1 : szKpCnt;
    auto itKp = GetKpIter(szLoopStart);
    auto itEnd = GetKpIter(szLoopEnd);
    KeyPoint::ValType tKpVal = (*itKp)->val;
    tKpVal.w = Tick2Time(tKpVal.w);
    p2 = m_owner->CvtPoint2Pos(tKpVal, eDim);
    while (itKp == itEnd)
    {
        const auto& hKp0 = *itKp++;
        if (itKp == m_aKeyPoints.end())
        {
            aDimCpTable[hKp0] = { p2 };
            break;
        }
        p1 = p2;
        const auto& hKp1 = *itKp;
        tKpVal = hKp1->val;
        tKpVal.w = Tick2Time(tKpVal.w);
        p2 = m_owner->CvtPoint2Pos(tKpVal, eDim);
        size_t steps = (size_t)(CalcDistance(p1, p2)/2);
        steps = ImClamp<size_t>(steps, 2, 100);
        auto eCurveType = hKp1->type;
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
        aDimCpTable[hKp0] = std::move(aContourPos);
    }
}

void CurveUiObj::UpdateDimContourPoints(ValueDimension eDim, std::list<KeyPoint::Holder>::iterator itKp)
{
    auto& aDimCpTable = m_aContourPoints.at(eDim);
    const auto szKpCnt = m_aKeyPoints.size();
    if (szKpCnt < 2 || itKp == m_aKeyPoints.end())
    {
        aDimCpTable.clear();
        return;
    }
    if (itKp != m_aKeyPoints.begin())
        itKp--;
    auto hKp0 = *itKp++;
    KeyPoint::ValType tKpVal = hKp0->val;
    tKpVal.w = Tick2Time(tKpVal.w);
    ImVec2 p1 = m_owner->CvtPoint2Pos(tKpVal, eDim);
    auto hKp1 = *itKp++;
    tKpVal = hKp1->val;
    tKpVal.w = Tick2Time(tKpVal.w);
    ImVec2 p2 = m_owner->CvtPoint2Pos(tKpVal, eDim);
    size_t steps = (size_t)(CalcDistance(p1, p2)/2);
    steps = ImClamp<size_t>(steps, 2, 100);
    auto eCurveType = hKp1->type;
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
    aDimCpTable[hKp0] = std::move(aContourPos);
    if (itKp == m_aKeyPoints.end())
    {
        aDimCpTable[hKp1] = { p2 };
        return;
    }
    p1 = p2;
    hKp0 = hKp1;
    hKp1 = *itKp++;
    tKpVal = hKp1->val;
    tKpVal.w = Tick2Time(tKpVal.w);
    p2 = m_owner->CvtPoint2Pos(tKpVal, eDim);
    steps = (size_t)(CalcDistance(p1, p2)/2);
    steps = ImClamp<size_t>(steps, 2, 100);
    eCurveType = hKp1->type;
    aContourPos.resize(steps);
    aContourPos[0] = p1;
    for (auto j = 1; j < steps; j++)
    {
        const float s = (float)j/steps;
        const auto t = ImLerp(p1.x, p2.x, s);
        const auto rt = SmoothStep(p1.x, p2.x, t, eCurveType);
        aContourPos[j].x = t;
        aContourPos[j].y = p1.y==p2.y ? p1.y : ImLerp(p1.y, p2.y, rt);
    }
    aDimCpTable[hKp0] = std::move(aContourPos);
}

void CurveUiObj::RemoveInvalidContourPoints()
{
    for (auto& elem1 : m_aContourPoints)
    {
        auto& aDimCpTable = elem1.second;
        // remove invalid key-point entries
        auto iter = aDimCpTable.begin();
        while (iter != aDimCpTable.end())
        {
            const auto& elem2 = *iter;
            if (std::find(m_aKeyPoints.begin(), m_aKeyPoints.end(), elem2.first) == m_aKeyPoints.end())
                iter = aDimCpTable.erase(iter);
            else
                iter++;
        }
    }
}

imgui_json::value CurveUiObj::SaveAsJson() const
{
    imgui_json::value j = Curve::SaveAsJson();
    j["curve_color"] = imgui_json::number(m_u32CurveColor);
    j["curve_hovering_color"] = imgui_json::number(m_u32CurveHoveringColor);
    j["curve_width"] = imgui_json::number(m_fCurveWidth);
    j["curve_hovering_width"] = imgui_json::number(m_fCurveHoveringWidth);
    j["key_point_color"] = imgui_json::number(m_u32KeyPointColor);
    j["key_point_hovering_color"] = imgui_json::number(m_u32KeyPointHoveringColor);
    j["key_point_edge_color"] = imgui_json::number(m_u32KeyPointEdgeColor);
    j["key_point_radius"] = imgui_json::number(m_fKeyPointRadius);
    j["visible"] = imgui_json::boolean(m_bVisible);
    j["id"] = imgui_json::number(m_id);
    j["sub_id"] = imgui_json::number(m_subId);
    return std::move(j);
}

void CurveUiObj::LoadFromJson(const imgui_json::value& j)
{
    Curve::LoadFromJson(j);
    m_u32CurveColor = (ImU32)j["curve_color"].get<imgui_json::number>();
    m_u32CurveHoveringColor = (ImU32)j["curve_hovering_color"].get<imgui_json::number>();
    m_fCurveWidth = (float)j["curve_width"].get<imgui_json::number>();
    m_fCurveHoveringWidth = (float)j["curve_hovering_width"].get<imgui_json::number>();
    m_u32KeyPointColor = (ImU32)j["key_point_color"].get<imgui_json::number>();
    m_u32KeyPointHoveringColor = (ImU32)j["key_point_hovering_color"].get<imgui_json::number>();
    m_u32KeyPointEdgeColor = (ImU32)j["key_point_edge_color"].get<imgui_json::number>();
    m_fKeyPointRadius = (float)j["key_point_radius"].get<imgui_json::number>();
    m_bVisible = (bool)j["visible"].get<imgui_json::boolean>();
    m_id = (int64_t)j["id"].get<imgui_json::number>();
    m_subId = (int64_t)j["sub_id"].get<imgui_json::number>();
}

bool Editor::SetTimeRange(const ImVec2& v2TimeRange, bool bDockEnds)
{
    if (v2TimeRange.x >= v2TimeRange.y)
        return false;

    m_v2TimeRange = v2TimeRange;
    for (auto& hCurve : m_aCurveUiObjs)
        hCurve->SetTimeRange(v2TimeRange, bDockEnds);
    return true;
}

void Editor::Clear()
{
    m_aCurveUiObjs.clear();
}

bool Editor::DrawDim(ValueDimension eDim, const char* pcLabel, const ImVec2& v2ViewSize, uint32_t flags, ImDrawList* pDrawList)
{
    bool bCurveChanged = false;
    ImGuiIO& io = GetIO();
    const bool bDeleteMode = IsKeyDown(ImGuiKey_LeftShift) && (io.KeyMods == ImGuiMod_Shift);
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    PushStyleColor(ImGuiCol_Border, 0);
    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    BeginChildFrame(GetID(pcLabel), v2ViewSize);
    const ImVec2 v2CursorPos = GetCursorScreenPos();
    if (!pDrawList) pDrawList = GetWindowDrawList();

    const ImVec2 v2ViewPadding(0, 2);
    const ImVec2 v2EditorSize = v2ViewSize-v2ViewPadding*2;
    const ImVec2 v2DrawCursorPos = v2CursorPos+v2ViewPadding;
    const ImRect rContainer(v2CursorPos, v2CursorPos+v2EditorSize);

    // update curve point->pos scale
    if (m_v2CurveAreaSize != v2EditorSize)
    {
        m_v2Point2PosScale.x = v2EditorSize.x*m_v2UiScale.x/(m_v2TimeRange.y-m_v2TimeRange.x);
        m_v2Point2PosScale.y = -v2EditorSize.y*m_v2UiScale.y;
        m_v2CurveAreaSize = v2EditorSize;
        for (auto& hCurveUiObj : m_aCurveUiObjs)
            hCurveUiObj->UpdateContourPoints(-1);
    }

    // handle zoom and VScroll
    if (flags & CURVE_EDIT_FLAG_SCROLL_V)
    {
        if (rContainer.Contains(io.MousePos))
        {
            if (fabsf(io.MouseWheel) > FLT_EPSILON)
            {
                const float r = 1+ImClamp(io.MouseWheel, -0.5f, 0.5f);
                m_v2UiScale.y *= r;
            }
        }
    }

    const float fGraticuleHeight = v2EditorSize.y/10.f;
    for (int i = 0; i <= 10; i++)
    {
        const float fYOffset = fGraticuleHeight*i;
        pDrawList->AddLine(v2DrawCursorPos+ImVec2(0, fYOffset), v2DrawCursorPos+ImVec2(v2EditorSize.x, fYOffset), m_u32GraticuleColor, 1.0f);
    }

    const auto szCurveCnt = m_aCurveUiObjs.size();
    const auto v2AdjustedMousePos = io.MousePos-v2DrawCursorPos-m_v2PanOffset-ImVec2(0, v2EditorSize.y);
    int iCurveHoveringIdx = -1, iPointHoveringIdx = -1;
    if (!m_bIsDragging)
    {
        // check hovering state
        if (rContainer.Contains(io.MousePos))
        {
            // if there is already a hovering curve, check its hovering state first
            if (m_iCurveHoveringIdx >= 0 && m_iCurveHoveringIdx < (int)m_aCurveUiObjs.size() &&
                m_aCurveUiObjs[m_iCurveHoveringIdx]->CheckMouseHoverCurve(eDim, v2AdjustedMousePos))
                iCurveHoveringIdx = m_iCurveHoveringIdx;
            if (iCurveHoveringIdx < 0)
            {
                // check other curves' hovering state
                const auto iPrevCurveHoveringIdx = m_iCurveHoveringIdx;
                for (auto i = 0; i < szCurveCnt; i++)
                {
                    if ((int)i == iPrevCurveHoveringIdx)
                        continue;
                    if (m_aCurveUiObjs[i]->CheckMouseHoverCurve(eDim, v2AdjustedMousePos))
                    {
                        iCurveHoveringIdx = i;
                        break;
                    }
                }
            }
            // if there is a hovering curve, check whether there is a key point under hovering
            if (iCurveHoveringIdx >= 0)
                iPointHoveringIdx = m_aCurveUiObjs[iCurveHoveringIdx]->CheckMouseHoverPoint(eDim, v2AdjustedMousePos);
        }
        m_iCurveHoveringIdx = iCurveHoveringIdx;
        m_iPointHoveringIdx = iPointHoveringIdx;
    }
    else
    {
        iCurveHoveringIdx = m_iCurveHoveringIdx;
        iPointHoveringIdx = m_iPointHoveringIdx;
    }

    // draw curves
    // 1st, draw curves except the hovering one
    for (auto i = 0; i < szCurveCnt; i++)
    {
        if ((int)i == iCurveHoveringIdx)
            continue;
        m_aCurveUiObjs[i]->DrawDim(eDim, pDrawList, v2DrawCursorPos, false);
    }
    // 2nd, draw the hovering curve if there is one, to make sure the hovering curve is drawn ontop of the others
    if (iCurveHoveringIdx >= 0)
        m_aCurveUiObjs[iCurveHoveringIdx]->DrawDim(eDim, pDrawList, v2DrawCursorPos, true);

    const auto v2PtVal = CvtPos2Point(v2AdjustedMousePos);
    if (IsMouseDown(ImGuiMouseButton_Left))
    {
        if (iPointHoveringIdx >= 0 || iCurveHoveringIdx >= 0)
            m_bIsDragging = true;
    }
    if (IsMouseReleased(ImGuiMouseButton_Left))
    {
        if (m_bIsDragging) m_bIsDragging = false;
    }
    if (m_bIsDragging)
    {
        // handle move key point
        if (iPointHoveringIdx >= 0)
        {
            auto& hCurveUiObj = m_aCurveUiObjs[iCurveHoveringIdx];
            // std::cout << "-> ChangePointValByDim: curveIdx=" << iCurveHoveringIdx << ", pointIdx=" << iPointHoveringIdx << std::endl;
            auto ret = hCurveUiObj->ChangePointValByDim(eDim, iPointHoveringIdx, v2PtVal, false);
            // std::cout << "------>> KeyPoints: " << hCurveUiObj->PrintKeyPointsByDim(eDim) << std::endl;
            if (ret == -1)
                std::cerr << "FAILED to ChangePointValByDim(), curveIdx=" << iCurveHoveringIdx << ", pointIdx=" << iPointHoveringIdx
                        << ", newPointVal=(" << v2PtVal.x << ", " << v2PtVal.y << "), ret=" << ret << std::endl;
            else if (ret >= 0)
            {
                m_bIsDragging = true;
                m_iPointHoveringIdx = iPointHoveringIdx = ret;
            }
        }
        // handle move curve
        else if (iCurveHoveringIdx >= 0)
        {
            auto& hCurveUiObj = m_aCurveUiObjs[iCurveHoveringIdx];
            hCurveUiObj->MoveVerticallyByDim(eDim, v2PtVal, false);
            m_bIsDragging = true;
        }
    }

    if (m_bShowValueToolTip && iCurveHoveringIdx >= 0)
    {
        auto& hCurveUiObj = m_aCurveUiObjs[iCurveHoveringIdx];
        const auto tKpVal = hCurveUiObj->CalcPointVal(v2PtVal.x, true);
        const auto strCurveName = hCurveUiObj->GetName();
        SetTooltip("%s: %.03f, %.03f", strCurveName.c_str(), tKpVal.w, KeyPoint::GetDimVal(tKpVal, eDim));
    }

    EndChildFrame();
    PopStyleColor(2);
    PopStyleVar();
    return bCurveChanged;
}

CurveUiObj::Holder Editor::AddCurve(
        const std::string& name, CurveType eType, const KeyPoint::ValType& minVal, const KeyPoint::ValType& maxVal, const KeyPoint::ValType& defaultVal,
        ImU32 color, bool visible, int64_t id, int64_t subId)
{
    KeyPoint::ValType minVal_(minVal); minVal_.w = m_v2TimeRange.x;
    KeyPoint::ValType maxVal_(maxVal); maxVal_.w = m_v2TimeRange.y;
    auto hCurveUiObj = CurveUiObj::CreateInstance(this, name, eType, minVal_, maxVal_, defaultVal, color, visible, id, subId);
    m_aCurveUiObjs.push_back(hCurveUiObj);
    return hCurveUiObj;
}

CurveUiObj::Holder Editor::AddCurveByDim(
        const std::string& name, CurveType eType, ValueDimension eDim, float minVal, float maxVal, float _defaultVal,
        ImU32 color, bool visible, int64_t id, int64_t subId)
{
    KeyPoint::ValType minVal_, maxVal_(1, 1, 1, 1), defaultVal;
    KeyPoint::SetDimVal(minVal_, minVal, eDim);
    KeyPoint::SetDimVal(maxVal_, maxVal, eDim);
    KeyPoint::SetDimVal(defaultVal, _defaultVal, eDim);
    return AddCurve(name, eType, minVal_, maxVal_, defaultVal, color, visible, id, subId);
}

CurveUiObj::Holder Editor::GetCurveByIndex(size_t idx) const
{
    if (idx >= m_aCurveUiObjs.size())
        return nullptr;
    return m_aCurveUiObjs[idx];
}

CurveUiObj::Holder Editor::GetCurveByName(const std::string& name) const
{
    auto iter = std::find_if(m_aCurveUiObjs.begin(), m_aCurveUiObjs.end(), [name] (const auto& elem) {
        return elem->GetName() == name;
    });
    if (iter == m_aCurveUiObjs.end())
        return nullptr;
    return *iter;
}

imgui_json::value Editor::SaveAsJson() const
{
    imgui_json::value j;
    j["background_color"] = imgui_json::number(m_u32BackgroundColor);
    j["graticule_color"] = imgui_json::number(m_u32GraticuleColor);
    j["time_range"] = imgui_json::vec2(m_v2TimeRange);
    j["check_hover_distance_thresh"] = imgui_json::number(m_fCheckHoverDistanceThresh);
    j["ui_scale"] = imgui_json::vec2(m_v2UiScale);
    j["pan_offset"] = imgui_json::vec2(m_v2PanOffset);
    j["show_value_tool_tip"] = imgui_json::boolean(m_bShowValueToolTip);
    imgui_json::array jaCurves;
    for (const auto& hCurve : m_aCurveUiObjs)
        jaCurves.push_back(hCurve->SaveAsJson());
    j["curves"] = jaCurves;
    return std::move(j);
}

void Editor::LoadFromJson(const imgui_json::value& j)
{
    m_u32BackgroundColor = (ImU32)j["background_color"].get<imgui_json::number>();
    m_u32GraticuleColor = (ImU32)j["graticule_color"].get<imgui_json::number>();
    m_v2TimeRange = j["time_range"].get<imgui_json::vec2>();
    m_fCheckHoverDistanceThresh = (float)j["check_hover_distance_thresh"].get<imgui_json::number>();
    m_v2UiScale = j["ui_scale"].get<imgui_json::vec2>();
    m_v2PanOffset = j["pan_offset"].get<imgui_json::vec2>();
    m_bShowValueToolTip = j["show_value_tool_tip"].get<imgui_json::boolean>();
    imgui_json::array jaCurves = j["curves"].get<imgui_json::array>();
    m_aCurveUiObjs.clear();
    m_aCurveUiObjs.reserve(jaCurves.size());
    for (const auto& jelem : jaCurves)
    {
        auto hCurve = CurveUiObj::CreateFromJson(this, jelem);
        m_aCurveUiObjs.push_back(hCurve);
    }
}

Editor::Holder Editor::CreateInstance(const ImVec2& v2TimeRange)
{
    return Editor::Holder(new Editor(v2TimeRange));
}

}  // ~ namespace ImCurve
}  // ~ namespace ImGui
