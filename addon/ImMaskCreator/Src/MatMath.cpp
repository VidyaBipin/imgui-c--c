#include <cassert>
#include "MatMath.h"
#include "SysUtils.h"

#include "SimdOpt.h"
#if SIMD_ARCH_X86

#define INTRIN_MODE AVX2
#undef USE_AVX2
#define USE_AVX2
#include "MatMath.Simd.h"
#undef USE_AVX2
#undef INTRIN_MODE

#define INTRIN_MODE AVX
#include "MatMath.Simd.h"
#undef INTRIN_MODE

#define INTRIN_MODE SSE4_1
#undef USE_SSE4_1
#define USE_SSE4_1
#include "MatMath.Simd.h"
#undef USE_SSE4_1
#undef INTRIN_MODE

#define INTRIN_MODE SSSE3
#undef USE_SSSE3
#define USE_SSSE3
#include "MatMath.Simd.h"
#undef USE_SSSE3
#undef INTRIN_MODE

#define INTRIN_MODE SSE3
#undef USE_SSE3
#define USE_SSE3
#include "MatMath.Simd.h"
#undef USE_SSE3
#undef INTRIN_MODE

#define INTRIN_MODE SSE
#include "MatMath.Simd.h"
#undef INTRIN_MODE

#endif // ~SIMD_ARCH_X86

#define INTRIN_MODE NONE
#include "MatMath.Simd.h"
#undef INTRIN_MODE

using namespace SysUtils;

template<typename T> struct MinOp
{
    typedef T type1;
    typedef T type2;
    typedef T rtype;
    T operator ()(const T a, const T b) const { return std::min(a, b); }
};

template<typename T> struct MaxOp
{
    typedef T type1;
    typedef T type2;
    typedef T rtype;
    T operator ()(const T a, const T b) const { return std::max(a, b); }
};

namespace MatUtils
{
void Max(ImGui::ImMat& dst, const ImGui::ImMat& src)
{
    assert(dst.type == src.type && dst.w == src.w && dst.h == src.h && dst.c == src.c);

    int iLineSize = src.w*src.c*src.elemsize;
    const auto type = src.type;
    MatOp2::Holder hOp2;

    #define GET_SIMD_MAX_OP(type) \
        if (type == IM_DT_INT8) { \
            using MaxSimdOp = MaxSimd<MaxOp<uint8_t>, Op2VecRow<VMax<v_uint8>>>; \
            hOp2 = MatOp2::Holder(new MaxSimdOp(), [] (MatOp2* p) { \
                MaxSimdOp* ptr = dynamic_cast<MaxSimdOp*>(p); \
                delete ptr; }); \
        } \
        if (type == IM_DT_INT16) { \
            using MaxSimdOp = MaxSimd<MaxOp<uint16_t>, Op2VecRow<VMax<v_uint16>>>; \
            hOp2 = MatOp2::Holder(new MaxSimdOp(), [] (MatOp2* p) { \
                MaxSimdOp* ptr = dynamic_cast<MaxSimdOp*>(p); \
                delete ptr; }); \
        } \
        if (type == IM_DT_INT32) { \
            using MaxSimdOp = MaxSimd<MaxOp<int32_t>, Op2VecRow<VMax<v_int32>>>; \
            hOp2 = MatOp2::Holder(new MaxSimdOp(), [] (MatOp2* p) { \
                MaxSimdOp* ptr = dynamic_cast<MaxSimdOp*>(p); \
                delete ptr; }); \
        } \
        if (type == IM_DT_FLOAT32) { \
            using MaxSimdOp = MaxSimd<MaxOp<float>, Op2VecRow<VMax<v_float32>>>; \
            hOp2 = MatOp2::Holder(new MaxSimdOp(), [] (MatOp2* p) { \
                MaxSimdOp* ptr = dynamic_cast<MaxSimdOp*>(p); \
                delete ptr; }); \
        } \
        if (type == IM_DT_FLOAT64) { \
            using MaxSimdOp = MaxSimd<MaxOp<double>, Op2VecRow<VMax<v_float64>>>; \
            hOp2 = MatOp2::Holder(new MaxSimdOp(), [] (MatOp2* p) { \
                MaxSimdOp* ptr = dynamic_cast<MaxSimdOp*>(p); \
                delete ptr; }); \
        }

#if SIMD_ARCH_X86
    if (CpuChecker::HasFeature(CpuFeature::AVX2))
    {
        using namespace SimdOpt::AVX2;
        GET_SIMD_MAX_OP(type);
    }
    else if (CpuChecker::HasFeature(CpuFeature::AVX))
    {
        using namespace SimdOpt::AVX;
        GET_SIMD_MAX_OP(type);
    }
    else if (CpuChecker::HasFeature(CpuFeature::SSE4_1))
    {
        using namespace SimdOpt::SSE4_1;
        GET_SIMD_MAX_OP(type);
    }
    else if (CpuChecker::HasFeature(CpuFeature::SSSE3))
    {
        using namespace SimdOpt::SSSE3;
        GET_SIMD_MAX_OP(type);
    }
    else if (CpuChecker::HasFeature(CpuFeature::SSE3))
    {
        using namespace SimdOpt::SSE3;
        GET_SIMD_MAX_OP(type);
    }
    else if (CpuChecker::HasFeature(CpuFeature::SSE))
    {
        using namespace SimdOpt::SSE;
        GET_SIMD_MAX_OP(type);
    }
    else
#endif // ~SIMD_ARCH_X86
    {
        using namespace SimdOpt::NONE;
        GET_SIMD_MAX_OP(type);
    }

    if (hOp2)
        (*hOp2)(src, dst, dst);
    else
        throw std::runtime_error("No simd MAX operator is available!");
}
}