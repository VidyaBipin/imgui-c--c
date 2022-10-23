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
    \n\
    int curve_w; \n\
    int curve_c; \n\
} p;\
"

#define SHADER_RGB2YUV \
" \n\
sfpmat3 matrix_mat_r2y = { \n\
    {sfp(0.262700f), sfp(-0.139630f), sfp( 0.500000f)}, \n\
    {sfp(0.678000f), sfp(-0.360370f), sfp(-0.459786f)}, \n\
    {sfp(0.059300f), sfp( 0.500000f), sfp(-0.040214f)} \n\
}; \n\
sfpvec3 rgb_to_yuv(sfpvec3 rgb) \n\
{\n\
    sfpvec3 yuv; \n\
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5)}; \n\
    yuv = yuv_offset + matrix_mat_r2y * rgb; \n\
    return clamp(yuv, sfp(0.f), sfp(1.f)); \n\
} \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    sfpvec4 color = load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    ivec4 color_index = ivec4(floor(color * sfp(p.curve_w))); \n\
    sfpvec4 result = sfpvec4(0); \n\
    sfpvec3 yuv_org = rgb_to_yuv(color.rgb); \n\
    result.r = sfp(curve[color_index.r + p.curve_w * 1]); \n\
    result.g = sfp(curve[color_index.g + p.curve_w * 2]); \n\
    result.b = sfp(curve[color_index.b + p.curve_w * 3]); \n\
    sfpvec3 yuv = rgb_to_yuv(result.rgb); \n\
    sfp deta_y = yuv_org.x - yuv.x; \n\
    int y_index = int(floor(yuv.x * sfp(p.curve_w))); \n\
    deta_y += sfp(curve[y_index]) - yuv.x; \n\
    result += deta_y; \n\
    store_rgba(sfpvec4(result.rgb, color.a), gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char Filter_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
R"(
layout (binding = 8) readonly buffer mat_curve { float curve[]; };
)"
SHADER_LOAD_RGBA
SHADER_STORE_RGBA
SHADER_RGB2YUV
SHADER_MAIN
;
