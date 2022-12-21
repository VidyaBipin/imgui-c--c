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
    float shadow_height; \n\
    float bounces; \n\
    \n\
    float red_shadow; \n\
    float green_shadows; \n\
    float blue_shadows; \n\
    float alpha_shadows; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
const sfp PI = sfp(3.14159265358f); \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.out_w || uv.y >= p.out_h) \n\
        return; \n\
    sfpvec4 shadow_colour = sfpvec4(sfp(p.red_shadow), sfp(p.green_shadows), sfp(p.blue_shadows), sfp(p.alpha_shadows)); \n\
    sfp time = sfp(p.progress); \n\
    sfp stime = sin(time * PI / sfp(2.f)); \n\
    sfp phase = time * PI * sfp(p.bounces); \n\
    sfp y = (abs(cos(phase))) * (sfp(1.f) - stime); \n\
    sfp d = sfp(uv.y) / sfp(p.out_h) - y; \n\
    sfpvec4 rgba_src1 = load_rgba(uv.x, uv.y + int((sfp(1.f) - y) * sfp(p.h - 1)), p.w, p.cstep, p.in_format, p.in_type); \n\
    sfpvec4 rgba_src2 = load_rgba_src2(uv.x, uv.y, p.w2, p.cstep2, p.in_format2, p.in_type2); \n\
    sfpvec4 result = mix( \n\
            mix( \n\
            rgba_src2, \n\
            shadow_colour, \n\
            step(d, sfp(p.shadow_height)) * (sfp(1.f) - mix( \n\
                ((d / sfp(p.shadow_height)) * shadow_colour.a) + (sfp(1.f) - shadow_colour.a), \n\
                sfp(1.f), \n\
                smoothstep(sfp(0.95f), sfp(1.f), sfp(p.progress)) \n\
            )) \n\
        ), \n\
        rgba_src1, \n\
        step(d, sfp(0.f)) \n\
    ); \n\
    store_rgba(result, uv.x, uv.y, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char Bounce_data[] = 
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
