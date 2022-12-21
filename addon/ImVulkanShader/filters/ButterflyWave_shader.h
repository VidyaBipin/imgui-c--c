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
    int w2; \n\
    int h2; \n\
    int cstep2; \n\
    int in_format2; \n\
    int in_type2; \n\
\n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
\n\
    float progress; \n\
    float amplitude; \n\
    float waves; \n\
    float colorSeparation; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
const float PI = 3.14159265358979323846264; \n\
float compute(vec2 point, vec2 center) \n\
{ \n\
    vec2 o = point * sin(p.progress * p.amplitude) - center; \n\
    // horizontal vector \n\
    vec2 h = vec2(1., 0.); \n\
    // butterfly polar function (don't ask me why this one :)) \n\
    float theta = acos(dot(o, h)) * p.waves; \n\
    return (exp(cos(theta)) - 2. * cos(4. * theta) + pow(sin((2. * theta - PI) / 24.), 5.)) / 10.; \n\
} \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.out_w || uv.y >= p.out_h) \n\
        return; \n\
    vec2 point = vec2(float(uv.x) / float(p.out_w - 1), float(uv.y) / float(p.out_h - 1)); \n\
    float inv = 1. - p.progress; \n\
    float disp = compute(point, vec2(0.5, 0.5)); \n\
    sfpvec4 result = sfpvec4(0.f); \n\
    vec2 to_point = point + inv * disp; \n\
    sfpvec4 rgba_to = load_rgba_src2(int(to_point.x * (p.w2 - 1)), int(to_point.y * (p.h2 - 1)), p.w2, p.cstep2, p.in_format2, p.in_type2); \n\
    vec2 from_p1 = point + p.progress * disp * (1.0 - p.colorSeparation); \n\
    vec2 from_p2 = point + p.progress * disp; \n\
    vec2 from_p3 = point + p.progress * disp * (1.0 + p.colorSeparation); \n\
    sfpvec4 rgba_from_p1 = load_rgba(int(from_p1.x * (p.w - 1)), int(from_p1.y * (p.h - 1)), p.w, p.cstep, p.in_format, p.in_type); \n\
    sfpvec4 rgba_from_p2 = load_rgba(int(from_p2.x * (p.w - 1)), int(from_p2.y * (p.h - 1)), p.w, p.cstep, p.in_format, p.in_type); \n\
    sfpvec4 rgba_from_p3 = load_rgba(int(from_p3.x * (p.w - 1)), int(from_p3.y * (p.h - 1)), p.w, p.cstep, p.in_format, p.in_type); \n\
    sfpvec4 rgba_from = sfpvec4(rgba_from_p1.r, rgba_from_p2.g, rgba_from_p3.b, sfp(1.0f)); \n\
    result = rgba_to * sfp(p.progress) + rgba_from * sfp(inv); \n\
    store_rgba(result, uv.x, uv.y, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char ButterflyWave_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
R"(
layout (binding =  8) readonly buffer src2_int8       { uint8_t   src2_data_int8[]; };
layout (binding =  9) readonly buffer src2_int16      { uint16_t  src2_data_int16[]; };
layout (binding = 10) readonly buffer src2_float16    { float16_t src2_data_float16[]; };
layout (binding = 11) readonly buffer src2_float32    { float     src2_data_float32[]; };
)"
SHADER_LOAD_RGBA
SHADER_LOAD_RGBA_NAME(src2)
SHADER_STORE_RGBA
SHADER_MAIN
;
