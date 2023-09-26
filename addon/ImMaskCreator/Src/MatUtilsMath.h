#pragma once
#include <algorithm>
#include <cmath>
#include "MatUtilsCommDef.h"

namespace MatUtils
{

template<typename T> inline T SaturateCast(int8_t v) { return T(v); }
template<typename T> inline T SaturateCast(uint8_t v) { return T(v); }
template<typename T> inline T SaturateCast(int16_t v) { return T(v); }
template<typename T> inline T SaturateCast(uint16_t v) { return T(v); }
template<typename T> inline T SaturateCast(int32_t v) { return T(v); }
template<typename T> inline T SaturateCast(uint32_t v) { return T(v); }
template<typename T> inline T SaturateCast(int64_t v) { return T(v); }
template<typename T> inline T SaturateCast(uint64_t v) { return T(v); }
template<typename T> inline T SaturateCast(float v) { return T(v); }
template<typename T> inline T SaturateCast(double v) { return T(v); }

template<> inline int8_t SaturateCast<int8_t>(uint8_t v) { return (int8_t)std::min(v, (uint8_t)INT8_MAX); }
template<> inline int8_t SaturateCast<int8_t>(int16_t v) { return (int8_t)std::max(std::min(v, (int16_t)INT8_MAX), (int16_t)INT8_MIN); }
template<> inline int8_t SaturateCast<int8_t>(uint16_t v) { return (int8_t)std::min(v, (uint16_t)INT8_MAX); }
template<> inline int8_t SaturateCast<int8_t>(int32_t v) { return (int8_t)std::max(std::min(v, (int32_t)INT8_MAX), (int32_t)INT8_MIN); }
template<> inline int8_t SaturateCast<int8_t>(uint32_t v) { return (int8_t)std::min(v, (uint32_t)INT8_MAX); }
template<> inline int8_t SaturateCast<int8_t>(int64_t v) { return (int8_t)std::max(std::min(v, (int64_t)INT8_MAX), (int64_t)INT8_MIN); }
template<> inline int8_t SaturateCast<int8_t>(uint64_t v) { return (int8_t)std::min(v, (uint64_t)INT8_MAX); }
template<> inline int8_t SaturateCast<int8_t>(float v) { int64_t iv = std::round(v); return SaturateCast<int8_t>(iv); }
template<> inline int8_t SaturateCast<int8_t>(double v) { int64_t iv = std::round(v); return SaturateCast<int8_t>(iv); }

template<> inline uint8_t SaturateCast<uint8_t>(int8_t v) { return (uint8_t)std::max(v, (int8_t)0); }
template<> inline uint8_t SaturateCast<uint8_t>(int16_t v) { return (uint8_t)std::max(std::min(v, (int16_t)UINT8_MAX), (int16_t)0); }
template<> inline uint8_t SaturateCast<uint8_t>(uint16_t v) { return (uint8_t)std::min(v, (uint16_t)UINT8_MAX); }
template<> inline uint8_t SaturateCast<uint8_t>(int32_t v) { return (uint8_t)std::max(std::min(v, (int32_t)UINT8_MAX), (int32_t)0); }
template<> inline uint8_t SaturateCast<uint8_t>(uint32_t v) { return (uint8_t)std::min(v, (uint32_t)UINT8_MAX); }
template<> inline uint8_t SaturateCast<uint8_t>(int64_t v) { return (uint8_t)std::max(std::min(v, (int64_t)UINT8_MAX), (int64_t)0); }
template<> inline uint8_t SaturateCast<uint8_t>(uint64_t v) { return (uint8_t)std::min(v, (uint64_t)UINT8_MAX); }
template<> inline uint8_t SaturateCast<uint8_t>(float v) { int64_t iv = std::round(v); return SaturateCast<uint8_t>(iv); }
template<> inline uint8_t SaturateCast<uint8_t>(double v) { int64_t iv = std::round(v); return SaturateCast<uint8_t>(iv); }

template<> inline int16_t SaturateCast<int16_t>(uint16_t v) { return (int16_t)std::min(v, (uint16_t)INT16_MAX); }
template<> inline int16_t SaturateCast<int16_t>(int32_t v) { return (int16_t)std::max(std::min(v, (int32_t)INT16_MAX), (int32_t)INT16_MIN); }
template<> inline int16_t SaturateCast<int16_t>(uint32_t v) { return (int16_t)std::min(v, (uint32_t)INT16_MAX); }
template<> inline int16_t SaturateCast<int16_t>(int64_t v) { return (int16_t)std::max(std::min(v, (int64_t)INT16_MAX), (int64_t)INT16_MIN); }
template<> inline int16_t SaturateCast<int16_t>(uint64_t v) { return (int16_t)std::min(v, (uint64_t)INT16_MAX); }
template<> inline int16_t SaturateCast<int16_t>(float v) { int64_t iv = std::round(v); return SaturateCast<int16_t>(iv); }
template<> inline int16_t SaturateCast<int16_t>(double v) { int64_t iv = std::round(v); return SaturateCast<int16_t>(iv); }

template<> inline uint16_t SaturateCast<uint16_t>(int8_t v) { return (uint16_t)std::max(v, (int8_t)0); }
template<> inline uint16_t SaturateCast<uint16_t>(int16_t v) { return (uint16_t)std::max(v, (int16_t)0); }
template<> inline uint16_t SaturateCast<uint16_t>(int32_t v) { return (uint16_t)std::max(std::min(v, (int32_t)UINT16_MAX), (int32_t)0); }
template<> inline uint16_t SaturateCast<uint16_t>(uint32_t v) { return (uint16_t)std::min(v, (uint32_t)UINT16_MAX); }
template<> inline uint16_t SaturateCast<uint16_t>(int64_t v) { return (uint16_t)std::max(std::min(v, (int64_t)UINT16_MAX), (int64_t)0); }
template<> inline uint16_t SaturateCast<uint16_t>(uint64_t v) { return (uint16_t)std::min(v, (uint64_t)UINT16_MAX); }
template<> inline uint16_t SaturateCast<uint16_t>(float v) { int64_t iv = std::round(v); return SaturateCast<uint16_t>(iv); }
template<> inline uint16_t SaturateCast<uint16_t>(double v) { int64_t iv = std::round(v); return SaturateCast<uint16_t>(iv); }

template<> inline int32_t SaturateCast<int32_t>(uint32_t v) { return (int32_t)std::min(v, (uint32_t)INT32_MAX); }
template<> inline int32_t SaturateCast<int32_t>(int64_t v) { return (int32_t)std::max(std::min(v, (int64_t)INT32_MAX), (int64_t)INT32_MIN); }
template<> inline int32_t SaturateCast<int32_t>(uint64_t v) { return (int32_t)std::min(v, (uint64_t)INT32_MAX); }
template<> inline int32_t SaturateCast<int32_t>(float v) { int64_t iv = std::round(v); return SaturateCast<int32_t>(iv); }
template<> inline int32_t SaturateCast<int32_t>(double v) { int64_t iv = std::round(v); return SaturateCast<int32_t>(iv); }

template<> inline uint32_t SaturateCast<uint32_t>(int8_t v) { return (uint32_t)std::max(v, (int8_t)0); }
template<> inline uint32_t SaturateCast<uint32_t>(int16_t v) { return (uint32_t)std::max(v, (int16_t)0); }
template<> inline uint32_t SaturateCast<uint32_t>(int32_t v) { return (uint32_t)std::max(v, (int32_t)0); }
template<> inline uint32_t SaturateCast<uint32_t>(int64_t v) { return (uint32_t)std::max(v, (int64_t)0); }
template<> inline uint32_t SaturateCast<uint32_t>(uint64_t v) { return (uint32_t)std::min(v, (uint64_t)UINT32_MAX); }
template<> inline uint32_t SaturateCast<uint32_t>(float v) { int64_t iv = std::round(v); return SaturateCast<uint32_t>(iv); }
template<> inline uint32_t SaturateCast<uint32_t>(double v) { int64_t iv = std::round(v); return SaturateCast<uint32_t>(iv); }

template<> inline int64_t SaturateCast<int64_t>(uint64_t v) { return (int64_t)std::min(v, (uint64_t)INT64_MAX); }
template<> inline int64_t SaturateCast<int64_t>(float v) { return (int64_t)std::round(v); }
template<> inline int64_t SaturateCast<int64_t>(double v) { return (int64_t)std::round(v); }

template<> inline uint64_t SaturateCast<uint64_t>(int8_t v) { return (uint64_t)std::max(v, (int8_t)0); }
template<> inline uint64_t SaturateCast<uint64_t>(int16_t v) { return (uint64_t)std::max(v, (int16_t)0); }
template<> inline uint64_t SaturateCast<uint64_t>(int32_t v) { return (uint64_t)std::max(v, (int32_t)0); }
template<> inline uint64_t SaturateCast<uint64_t>(int64_t v) { return (uint64_t)std::max(v, (int64_t)0); }
template<> inline uint64_t SaturateCast<uint64_t>(float v) { int64_t iv = std::round(v); return SaturateCast<uint64_t>(iv); }
template<> inline uint64_t SaturateCast<uint64_t>(double v) { int64_t iv = std::round(v); return SaturateCast<uint64_t>(iv); }
}