#pragma once
#include <imvk_mat_shader.h>
#define SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
    \n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec4 rgba = load_rgba(gx, gy, p.w, p.h, p.cstep, p.in_format, p.in_type); \n\
    sfpvec4 histogram = abs(rgba); \n\
    histogram.r += sfp(0.001); \n\
    histogram.g += sfp(0.001); \n\
    histogram.b += sfp(0.001); \n\
    histogram.a = sfp(0.299) * histogram.r + sfp(0.587) * histogram.g + sfp(0.114) * histogram.b; \n\
    uint rid = clamp(uint(floor(histogram.r * sfp(p.out_w - 1))), 0, p.out_w - 1) + 0 * p.out_cstep; \n\
    uint gid = clamp(uint(floor(histogram.g * sfp(p.out_w - 1))), 0, p.out_w - 1) + 1 * p.out_cstep; \n\
    uint bid = clamp(uint(floor(histogram.b * sfp(p.out_w - 1))), 0, p.out_w - 1) + 2 * p.out_cstep; \n\
    uint yid = clamp(uint(floor(histogram.a * sfp(p.out_w - 1))), 0, p.out_w - 1) + 3 * p.out_cstep; \n\
    memoryBarrierBuffer(); \n\
    atomicAdd(histogram_int32_data[rid], 1); \n\
    atomicAdd(histogram_int32_data[gid], 1); \n\
    atomicAdd(histogram_int32_data[bid], 1); \n\
    atomicAdd(histogram_int32_data[yid], 1); \n\
    memoryBarrierBuffer(); \n\
} \
"

static const char Histogram_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_SRC_DATA
R"(
layout (binding = 4) restrict buffer histogram_int32  { int histogram_int32_data[]; };
)"
SHADER_LOAD_RGBA
SHADER_MAIN
;


#define PARAM_ZERO \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
} p;\
"

#define ZERO_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    ivec4 in_offset = (gy * p.w + gx) * p.cstep + ivec4(0, 1, 2, 3); \n\
    data_int32_data[in_offset.r] = 0; \n\
    data_int32_data[in_offset.g] = 0; \n\
    data_int32_data[in_offset.b] = 0; \n\
    data_int32_data[in_offset.a] = 0; \n\
} \
"

static const char Zero_data[] =
SHADER_HEADER
PARAM_ZERO
R"(
layout (binding = 0) restrict buffer data_int32  { int data_int32_data[]; };
)"
ZERO_MAIN
;

#define PARAM_CONV \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    float scale; \n\
    int log_view; \n\
} p;\
"

#define CONV_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    ivec4 data_offset = (gy * p.w + gx) + ivec4(0, 1, 2, 3) * p.cstep; \n\
    // R Conv \n\
    int v_r = histogram_int32_data[data_offset.r]; \n\
    histogram_float32_data[data_offset.r] = (p.log_view == 1 ? log2(float(v_r + 1)) : float(v_r)) * p.scale; \n\
    // G Conv \n\
    int v_g = histogram_int32_data[data_offset.g]; \n\
    histogram_float32_data[data_offset.g] = (p.log_view == 1 ? log2(float(v_g + 1)) : float(v_g)) * p.scale; \n\
    // B Conv \n\
    int v_b = histogram_int32_data[data_offset.b]; \n\
    histogram_float32_data[data_offset.b] = (p.log_view == 1 ? log2(float(v_b + 1)) : float(v_b)) * p.scale; \n\
    // Y Conv \n\
    int v_y = histogram_int32_data[data_offset.a]; \n\
    histogram_float32_data[data_offset.a] = (p.log_view == 1 ? log2(float(v_y + 1)) : float(v_y)) * p.scale; \n\
} \
"

static const char ConvInt2Float_data[] = 
SHADER_HEADER
PARAM_CONV
R"(
layout (binding = 0) readonly buffer histogram_int32  { int histogram_int32_data[]; };
layout (binding = 1) writeonly buffer histogram_float32  { float histogram_float32_data[]; };
)"
CONV_MAIN
;