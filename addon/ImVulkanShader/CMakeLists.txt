#
# Cmake for building ImVulkanShader
#
cmake_minimum_required(VERSION 3.5.1)
project(ImVulkanShader)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_MACOSX_RPATH 0)
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif()
if(POLICY CMP0025)
    # reference from https://cmake.org/cmake/help/latest/policy/CMP0025.html
    cmake_policy(SET CMP0025 NEW)
endif()
if (POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(${PROJECT_SOURCE_DIR}/cmake/generater_shader_comp.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/precompile_shader_spv.cmake)

#if we need run on MacOS before 12.0, then uncomment following code, but not guarantee it's working
#if(APPLE)
#set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0") 
#endif(APPLE)

option(VKSHADER_VULKAN_BENCHMARK      "Enable Vulkan Shader Benchmark" ON)
option(VKSHADER_VULKAN_PREBUILD       "Enable Vulkan Shader prebuild check" OFF)
option(VKSHADER_VULKAN_FP16           "Enable Vulkan Shader support 16bits float" ON)
option(VKSHADER_STATIC                "Build Vulkan Shader as static library" OFF)

find_package(PkgConfig)
find_package(Vulkan)

set(LINK_LIBS)

if(IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm")
    message(STATUS "Target arch: arm-ios")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp -mfpu=neon -march=armv7 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loop")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=softfp -mfpu=neon -march=armv7 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loop")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
    message(STATUS "Target arch: arm64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loops")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loops")
elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|x86_64|AMD64)")
    message(STATUS "Target arch: x86")
    if(MSVC OR MSVC_IDE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2 /arch:AVX /arch:FMA /arch:SSE /arch:SSE2 /arch:SSSE3 /arch:SSE4.1 /arch:SSE4.2")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2 /arch:AVX /arch:FMA /arch:SSE /arch:SSE2 /arch:SSSE3 /arch:SSE4.1 /arch:SSE4.2")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mavx")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2 -mavx")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -msse4.1 -mssse3 -msse2 -msse")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -msse4.1 -mssse3 -msse2 -msse")
    endif()
endif()

if (APPLE)
    message(STATUS "VkShader(Apple) Use OpenMP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xpreprocessor -fopenmp")
    set(LINK_LIBS ${LINK_LIBS} -L/usr/local/lib -lomp pthread)
else()
    find_package(OpenMP)
    if(NOT TARGET OpenMP::OpenMP_CXX)
    set_property(TARGET OpenMP::OpenMP_CXX
                PROPERTY INTERFACE_LINK_LIBRARIES ${OpenMP_CXX_FLAGS} Threads::Threads)
    endif()
    if(WIN32 OR MINGW)
        message(STATUS "VkShader(Windows) Use OpenMP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(LINK_LIBS ${LINK_LIBS} gmp pthread)
    else()
        message(STATUS "VkShader Use OpenMP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(LINK_LIBS ${LINK_LIBS} gmp pthread)
    endif()
endif()

if(PKG_CONFIG_FOUND)
    if(NOT VULKAN_FOUND)
        pkg_search_module(VULKAN REQUIRED vulkan)
        if (VULKAN_FOUND)
            message("[ImVulkanShader] vulkan found")
            set (Vulkan_LIBRARY vulkan)
        endif(VULKAN_FOUND)
    endif(NOT VULKAN_FOUND)
endif(PKG_CONFIG_FOUND)

set(VKSHADER_INC_DIRS
    ${Vulkan_INCLUDE_DIR}
)

set(LINK_LIBS
    ${LINK_LIBS}
    ${Vulkan_LIBRARY}
)


find_program(GLSLANGVALIDATOR_EXECUTABLE NAMES glslangValidator PATHS $ENV{VULKAN_SDK}/bin NO_CMAKE_FIND_ROOT_PATH)
message(STATUS "VkShader Found glslangValidator: ${GLSLANGVALIDATOR_EXECUTABLE}")
macro(compile_shader header data)
generater_shader_comp(SHADER_COMP ${CMAKE_CURRENT_SOURCE_DIR}/${header} ${data})
precompile_shader_spv(SHADER_SPV_HEX ${SHADER_COMP})
list(APPEND SHADER_SPV_HEX_FILES ${SHADER_SPV_HEX})
endmacro()

if(VKSHADER_VULKAN_PREBUILD)
set(SHADER_COMP_FILES)
set(SHADER_SPV_HEX_FILES)
#internal shaders
compile_shader(imvk_Cast_shader.h cast_fp16_to_fp32_pack4)
compile_shader(imvk_Cast_shader.h cast_fp16_to_fp32_pack8)
compile_shader(imvk_Cast_shader.h cast_fp16_to_fp32)
compile_shader(imvk_Cast_shader.h cast_fp32_to_fp16_pack4)
compile_shader(imvk_Cast_shader.h cast_fp32_to_fp16_pack8)
compile_shader(imvk_Cast_shader.h cast_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack1to4_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack1to4_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack1to4)
compile_shader(imvk_Packing_shader.h packing_pack1to8_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack1to8_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack1to8)
compile_shader(imvk_Packing_shader.h packing_pack4_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack4_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack4)
compile_shader(imvk_Packing_shader.h packing_pack4to1_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack4to1_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack4to1)
compile_shader(imvk_Packing_shader.h packing_pack4to8_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack4to8_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack4to8)
compile_shader(imvk_Packing_shader.h packing_pack8_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack8_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack8)
compile_shader(imvk_Packing_shader.h packing_pack8to1_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack8to1_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack8to1)
compile_shader(imvk_Packing_shader.h packing_pack8to4_fp16_to_fp32)
compile_shader(imvk_Packing_shader.h packing_pack8to4_fp32_to_fp16)
compile_shader(imvk_Packing_shader.h packing_pack8to4)
compile_shader(imvk_Packing_shader.h packing)
compile_shader(imvk_copy_make_border_shader.h Filter_data)
compile_shader(imvk_substract_mean_normalize_shader.h Filter_data)
# filters
compile_shader(filters/ALM_shader.h ALM_data)
compile_shader(filters/Bilateral_shader.h Filter_data)
compile_shader(filters/Brightness_shader.h Filter_data)
compile_shader(filters/Canny_shader.h DSobelFilter_data)
compile_shader(filters/Canny_shader.h NMSFilter_data)
compile_shader(filters/Canny_shader.h CannyFilter_data)
compile_shader(filters/CAS_shader.h CAS_data)
compile_shader(filters/ChromaKey_shader.h Filter_data)
compile_shader(filters/ChromaKey_shader.h Blur_data)
compile_shader(filters/ChromaKey_shader.h Sharpen_data)
compile_shader(filters/ChromaKey_shader.h Despill_data)
compile_shader(filters/CIE_shader.h CIE_data)
compile_shader(filters/CIE_shader.h CIE_merge_data)
compile_shader(filters/CIE_shader.h CIE_set_data)
compile_shader(filters/ColorBalance_shader.h Filter_data)
compile_shader(filters/ColorConvert_shader.h YUV2RGB_data)
compile_shader(filters/ColorConvert_shader.h RGB2YUV_data)
compile_shader(filters/ColorConvert_shader.h GRAY2RGB_data)
compile_shader(filters/ColorConvert_shader.h Conv_data)
compile_shader(filters/ColorCurve_shader.h Filter_data)
compile_shader(filters/ColorInvert_shader.h Filter_data)
compile_shader(filters/Concat_shader.h Shader_data)
compile_shader(filters/Contrast_shader.h Filter_data)
compile_shader(filters/CopyTo_shader.h CopyTo_data)
compile_shader(filters/Crop_shader.h CropShader_data)
compile_shader(filters/Crop_shader.h CropToShader_data)
compile_shader(filters/DeBand_shader.h DeBand_data)
compile_shader(filters/DeInterlace_shader.h DeInterlace_data)
compile_shader(filters/Exposure_shader.h Filter_data)
compile_shader(filters/Filter2D_shader.h Filter_data)
compile_shader(filters/Filter2DS_shader.h FilterColumn_data)
compile_shader(filters/Filter2DS_shader.h FilterRow_data)
compile_shader(filters/Filter2DS_shader.h FilterColumnMono_data)
compile_shader(filters/Filter2DS_shader.h FilterRowMono_data)
compile_shader(filters/Flip_shader.h Shader_data)
compile_shader(filters/Gamma_shader.h Filter_data)
compile_shader(filters/Guided_shader.h ToMatting_data)
compile_shader(filters/Guided_shader.h Guided_data)
compile_shader(filters/Guided_shader.h Matting_data)
compile_shader(filters/Harris_shader.h PrewittFilter_data)
compile_shader(filters/Harris_shader.h HarrisFilter_data)
compile_shader(filters/Harris_shader.h NMSFilter_data)
compile_shader(filters/Histogram_shader.h Histogram_data)
compile_shader(filters/Histogram_shader.h Zero_data)
compile_shader(filters/Histogram_shader.h ConvInt2Float_data)
compile_shader(filters/HQDN3D_shader.h HQDN3D_data)
compile_shader(filters/Hue_shader.h Filter_data)
compile_shader(filters/Resize_shader.h Resize_data)
compile_shader(filters/Saturation_shader.h Filter_data)
compile_shader(filters/Sobel_shader.h Filter_data)
compile_shader(filters/Transpose_shader.h Filter_data)
compile_shader(filters/USM_shader.h USMFilter_data)
compile_shader(filters/Vector_shader.h Vector_data)
compile_shader(filters/Vector_shader.h Zero_data)
compile_shader(filters/Vector_shader.h Vector_merge_data)
compile_shader(filters/Vibrance_shader.h Filter_data)
compile_shader(filters/warpAffine_shader.h Filter_data)
compile_shader(filters/warpPerspective_shader.h Filter_data)
compile_shader(filters/Waveform_shader.h Waveform_data)
compile_shader(filters/Waveform_shader.h Zero_data)
compile_shader(filters/Waveform_shader.h ConvInt2Mat_data)
compile_shader(filters/WhiteBalance_shader.h Filter_data)
compile_shader(filters/lut3d/Lut3D_Shader.h LUT3D_data)
# fusions
compile_shader(fusions/AlphaBlending_shader.h AlphaBlending_data)
compile_shader(fusions/AlphaBlending_shader.h AlphaBlending_alpha_data)
compile_shader(fusions/BookFlip_shader.h BookFlip_data)
compile_shader(fusions/Bounce_shader.h Bounce_data)
compile_shader(fusions/BowTie_shader.h BowTie_data)
compile_shader(fusions/ButterflyWave_shader.h ButterflyWave_data)
compile_shader(fusions/Swap_shader.h Swap_data)
compile_shader(fusions/Radial_shader.h Radial_data)
compile_shader(fusions/WindowSlice_shader.h WindowSlice_data)
compile_shader(fusions/PolkaDots_shader.h PolkaDots_data)
compile_shader(fusions/SimpleZoom_shader.h SimpleZoom_data)
compile_shader(fusions/WaterDrop_shader.h WaterDrop_data)
compile_shader(fusions/PageCurl_shader.h PageCurl_data)
compile_shader(fusions/GlitchMemories_shader.h GlitchMemories_data)
compile_shader(fusions/StereoViewer_shader.h StereoViewer_data)
compile_shader(fusions/LuminanceMelt_shader.h LuminanceMelt_data)
compile_shader(fusions/Perlin_shader.h Perlin_data)
compile_shader(fusions/DirectionalWarp_shader.h DirectionalWarp_data)
compile_shader(fusions/Morph_shader.h Morph_data)
compile_shader(fusions/ColourDistance_shader.h ColourDistance_data)
compile_shader(fusions/CircleCrop_shader.h CircleCrop_data)
compile_shader(fusions/Swirl_shader.h Swirl_data)
compile_shader(fusions/CrossZoom_shader.h CrossZoom_data)
compile_shader(fusions/Dreamy_shader.h Dreamy_data)
compile_shader(fusions/GridFlip_shader.h GridFlip_data)
compile_shader(fusions/ZoomInCircles_shader.h ZoomInCircles_data)
compile_shader(fusions/Mosaic_shader.h Mosaic_data)
compile_shader(fusions/BurnOut_shader.h BurnOut_data)
compile_shader(fusions/Crosshatch_shader.h Crosshatch_data)
compile_shader(fusions/CannabisLeaf_shader.h CannabisLeaf_data)
compile_shader(fusions/CrazyParametric_shader.h CrazyParametric_data)
compile_shader(fusions/KaleidoScope_shader.h KaleidoScope_data)
compile_shader(fusions/WindowBlinds_shader.h WindowBlinds_data)
compile_shader(fusions/Hexagonalize_shader.h Hexagonalize_data)
compile_shader(fusions/GlitchDisplace_shader.h GlitchDisplace_data)
compile_shader(fusions/DreamyZoom_shader.h DreamyZoom_data)
compile_shader(fusions/DoomScreen_shader.h DoomScreen_data)
compile_shader(fusions/Ripple_shader.h Ripple_data)
compile_shader(fusions/Pinwheel_shader.h Pinwheel_data)
compile_shader(fusions/Burn_shader.h Burn_data)
compile_shader(fusions/ColorPhase_shader.h ColorPhase_data)
compile_shader(fusions/CrossWarp_shader.h CrossWarp_data)
compile_shader(fusions/Cube_shader.h Cube_data)
compile_shader(fusions/Doorway_shader.h Doorway_data)
compile_shader(fusions/Flyeye_shader.h Flyeye_data)
compile_shader(fusions/Heart_shader.h Heart_data)
compile_shader(fusions/MultiplyBlend_shader.h MultiplyBlend_data)

add_custom_target(generate-comp DEPENDS ${SHADER_SPV_HEX_FILES})

endif(VKSHADER_VULKAN_PREBUILD)

compile_shader(fusions/CircleBlur_shader.h CircleBlur_data)

find_package(Glslang COMPONENTS Glslang SPIRV)
find_package(SpirvTools)
if(TARGET Glslang::Glslang)
    message(STATUS "VkShader Found system glslang library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        Glslang::Glslang
    )
endif()
if(TARGET Glslang::SPIRV)
    message(STATUS "VkShader Found system SPIRV library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        Glslang::SPIRV
    )
endif()
if (TARGET SpirvTools::SpirvTools)
    message(STATUS "VkShader Found system SPRIV-Tools library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        SpirvTools::SpirvTools
        SpirvTools::Opt
    )
endif()

if(VKSHADER_VULKAN_BENCHMARK)
    add_definitions(-DVULKAN_SHADER_BENCHMARK)
    message(STATUS "VkShader Support benchmark")
endif(VKSHADER_VULKAN_BENCHMARK)

if(VKSHADER_VULKAN_FP16)
    add_definitions(-DVULKAN_SHADER_FP16)
    message(STATUS "VkShader Support 16bits float")
endif(VKSHADER_VULKAN_FP16)

set(VKSHADER_SRCS
    imvk_option.cpp
    imvk_allocator.cpp
    imvk_gpu.cpp
    imvk_command.cpp
    imvk_pipeline.cpp
    imvk_pipelinecache.cpp
    imvk_Cast_vulkan.cpp
    imvk_Packing_vulkan.cpp
    imvk_substract_mean_normalize.cpp
    imvk_copy_make_border.cpp
    ImVulkanShader.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    imvk_platform.h
    imvk_vulkan_header_fix.h
    imvk_option.h
    imvk_allocator.h
    imvk_gpu.h
    imvk_command.h
    imvk_mat.h
    imvk_mat_shader.h
    imvk_image_mat.h
    imvk_pipeline.h
    imvk_pipelinecache.h
    imvk_Cast_shader.h
    imvk_Cast_vulkan.h
    imvk_Packing_shader.h
    imvk_Packing_vulkan.h
    imvk_substract_mean_normalize_shader.h
    imvk_substract_mean_normalize.h
    imvk_copy_make_border_shader.h
    imvk_copy_make_border.h
    ImVulkanShader.h
)
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    filters/colorspace_table.cpp
    filters/ColorConvert_vulkan.cpp
    filters/Resize_vulkan.cpp
    filters/CopyTo_vulkan.cpp
    filters/Filter2D_vulkan.cpp
    filters/Filter2DS_vulkan.cpp
    filters/Flip_vulkan.cpp
    filters/Crop_vulkan.cpp
    filters/Transpose_vulkan.cpp
    filters/Brightness_vulkan.cpp
    filters/Contrast_vulkan.cpp
    filters/Exposure_vulkan.cpp
    filters/Gamma_vulkan.cpp
    filters/Saturation_vulkan.cpp
    filters/WhiteBalance_vulkan.cpp
    filters/Hue_vulkan.cpp
    filters/Vibrance_vulkan.cpp
    filters/ColorInvert_vulkan.cpp
    filters/ColorBalance_vulkan.cpp
    filters/ColorCurve_vulkan.cpp
    filters/Bilateral_vulkan.cpp
    filters/Sobel_vulkan.cpp
    filters/Canny_vulkan.cpp
    filters/Harris_vulkan.cpp
    filters/USM_vulkan.cpp
    filters/ChromaKey_vulkan.cpp
    filters/GaussianBlur.cpp
    filters/Box.cpp
    filters/Laplacian.cpp
    filters/Concat_vulkan.cpp
    filters/ALM_vulkan.cpp
    filters/CAS_vulkan.cpp
    filters/HQDN3D_vulkan.cpp
    filters/DeBand_vulkan.cpp
    filters/DeInterlace_vulkan.cpp
    filters/Guided_vulkan.cpp
    filters/CIE_vulkan.cpp
    filters/Histogram_vulkan.cpp
    filters/Waveform_vulkan.cpp
    filters/Vector_vulkan.cpp
    filters/warpPerspective_vulkan.cpp
    filters/warpAffine_vulkan.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    filters/ColorConvert_shader.h
    filters/ColorConvert_vulkan.h
    filters/Resize_shader.h
    filters/Resize_vulkan.h
    filters/CopyTo_shader.h
    filters/CopyTo_vulkan.h
    filters/Filter2D_shader.h
    filters/Filter2D_vulkan.h
    filters/Filter2DS_shader.h
    filters/Filter2DS_vulkan.h
    filters/Flip_shader.h
    filters/Flip_vulkan.h
    filters/Crop_shader.h
    filters/Crop_vulkan.h
    filters/Transpose_shader.h
    filters/Transpose_vulkan.h
    filters/Brightness_shader.h
    filters/Brightness_vulkan.h
    filters/Contrast_shader.h
    filters/Contrast_vulkan.h
    filters/Exposure_shader.h
    filters/Exposure_vulkan.h
    filters/Gamma_shader.h
    filters/Gamma_vulkan.h
    filters/Saturation_shader.h
    filters/Saturation_vulkan.h
    filters/WhiteBalance_shader.h
    filters/WhiteBalance_vulkan.h
    filters/Hue_shader.h
    filters/Hue_vulkan.h
    filters/Vibrance_shader.h
    filters/Vibrance_vulkan.h
    filters/ColorInvert_shader.h
    filters/ColorInvert_vulkan.h
    filters/ColorBalance_shader.h
    filters/ColorBalance_vulkan.h
    filters/ColorCurve_shader.h
    filters/ColorCurve_vulkan.h
    filters/Bilateral_shader.h
    filters/Bilateral_vulkan.h
    filters/Sobel_shader.h
    filters/Sobel_vulkan.h
    filters/Canny_shader.h
    filters/Canny_vulkan.h
    filters/Harris_vulkan.h
    filters/Harris_shader.h
    filters/USM_shader.h
    filters/USM_vulkan.h
    filters/ChromaKey_shader.h
    filters/ChromaKey_vulkan.h
    filters/GaussianBlur.h
    filters/Box.h
    filters/Laplacian.h
    filters/Concat_shader.h
    filters/Concat_vulkan.h
    filters/ALM_shader.h
    filters/ALM_vulkan.h
    filters/CAS_shader.h
    filters/CAS_vulkan.h
    filters/HQDN3D_shader.h
    filters/HQDN3D_vulkan.h
    filters/DeBand_shader.h
    filters/DeBand_vulkan.h
    filters/DeInterlace_shader.h
    filters/DeInterlace_vulkan.h
    filters/Guided_shader.h
    filters/Guided_vulkan.h
    filters/CIE_shader.h
    filters/CIE_vulkan.h
    filters/Histogram_shader.h
    filters/Histogram_vulkan.h
    filters/Waveform_shader.h
    filters/Waveform_vulkan.h
    filters/Vector_shader.h
    filters/Vector_vulkan.h
    filters/warpPerspective_shader.h
    filters/warpPerspective_vulkan.h
    filters/warpAffine_shader.h
    filters/warpAffine_vulkan.h
)
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    filters/lut3d/Lut3D.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    filters/lut3d/Lut3D.h
    filters/lut3d/Lut3D_Shader.h
    filters/lut3d/SDR709_HDR2020_HLG.h
    filters/lut3d/SDR709_HDR2020_PQ.h
    filters/lut3d/HDR2020_HLG_SDR709.h
    filters/lut3d/HDR2020_PQ_SDR709.h
    filters/lut3d/HDR2020HLG_HDR2020PQ.h
    filters/lut3d/HDR2020PQ_HDR2020HLG.h
)
set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/filters
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lut3d
)

# add fusion
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    fusions/AlphaBlending_vulkan.cpp
    fusions/BookFlip_vulkan.cpp
    fusions/Bounce_vulkan.cpp
    fusions/BowTie_vulkan.cpp
    fusions/ButterflyWave_vulkan.cpp
    fusions/Swap_vulkan.cpp
    fusions/Radial_vulkan.cpp
    fusions/WindowSlice_vulkan.cpp
    fusions/PolkaDots_vulkan.cpp
    fusions/SimpleZoom_vulkan.cpp
    fusions/WaterDrop_vulkan.cpp
    fusions/PageCurl_vulkan.cpp
    fusions/GlitchMemories_vulkan.cpp
    fusions/StereoViewer_vulkan.cpp
    fusions/LuminanceMelt_vulkan.cpp
    fusions/Perlin_vulkan.cpp
    fusions/DirectionalWarp_vulkan.cpp
    fusions/Morph_vulkan.cpp
    fusions/ColourDistance_vulkan.cpp
    fusions/CircleCrop_vulkan.cpp
    fusions/Swirl_vulkan.cpp
    fusions/CrossZoom_vulkan.cpp
    fusions/Dreamy_vulkan.cpp
    fusions/GridFlip_vulkan.cpp
    fusions/ZoomInCircles_vulkan.cpp
    fusions/Mosaic_vulkan.cpp
    fusions/BurnOut_vulkan.cpp
    fusions/Crosshatch_vulkan.cpp
    fusions/CannabisLeaf_vulkan.cpp
    fusions/CrazyParametric_vulkan.cpp
    fusions/KaleidoScope_vulkan.cpp
    fusions/WindowBlinds_vulkan.cpp
    fusions/Hexagonalize_vulkan.cpp
    fusions/GlitchDisplace_vulkan.cpp
    fusions/DreamyZoom_vulkan.cpp
    fusions/DoomScreen_vulkan.cpp
    fusions/Ripple_vulkan.cpp
    fusions/Pinwheel_vulkan.cpp
    fusions/Burn_vulkan.cpp
    fusions/CircleBlur_vulkan.cpp
    fusions/ColorPhase_vulkan.cpp
    fusions/CrossWarp_vulkan.cpp
    fusions/Cube_vulkan.cpp
    fusions/Doorway_vulkan.cpp
    fusions/Flyeye_vulkan.cpp
    fusions/Heart_vulkan.cpp
    fusions/MultiplyBlend_vulkan.cpp
)

set(VKSHADER_INCS
    ${VKSHADER_INCS}
    fusions/AlphaBlending_shader.h
    fusions/AlphaBlending_vulkan.h
    fusions/BookFlip_shader.h
    fusions/BookFlip_vulkan.h
    fusions/Bounce_shader.h
    fusions/Bounce_vulkan.h
    fusions/BowTie_shader.h
    fusions/BowTie_vulkan.h
    fusions/ButterflyWave_shader.h
    fusions/ButterflyWave_vulkan.h
    fusions/Swap_shader.h
    fusions/Swap_vulkan.h
    fusions/Radial_shader.h
    fusions/Radial_vulkan.h
    fusions/WindowSlice_shader.h
    fusions/WindowSlice_vulkan.h
    fusions/PolkaDots_shader.h
    fusions/PolkaDots_vulkan.h
    fusions/SimpleZoom_shader.h
    fusions/SimpleZoom_vulkan.h
    fusions/WaterDrop_shader.h
    fusions/WaterDrop_vulkan.h
    fusions/PageCurl_shader.h
    fusions/PageCurl_vulkan.h
    fusions/GlitchMemories_shader.h
    fusions/GlitchMemories_vulkan.h
    fusions/StereoViewer_shader.h
    fusions/StereoViewer_vulkan.h
    fusions/LuminanceMelt_shader.h
    fusions/LuminanceMelt_vulkan.h
    fusions/Perlin_shader.h
    fusions/Perlin_vulkan.h
    fusions/DirectionalWarp_shader.h
    fusions/DirectionalWarp_vulkan.h
    fusions/Morph_shader.h
    fusions/Morph_vulkan.h
    fusions/ColourDistance_shader.h
    fusions/ColourDistance_vulkan.h
    fusions/CircleCrop_shader.h
    fusions/CircleCrop_vulkan.h
    fusions/Swirl_shader.h
    fusions/Swirl_vulkan.h
    fusions/CrossZoom_shader.h
    fusions/CrossZoom_vulkan.h
    fusions/Dreamy_shader.h
    fusions/Dreamy_vulkan.h
    fusions/GridFlip_shader.h
    fusions/GridFlip_vulkan.h
    fusions/ZoomInCircles_shader.h
    fusions/ZoomInCircles_vulkan.h
    fusions/Mosaic_shader.h
    fusions/Mosaic_vulkan.h
    fusions/BurnOut_shader.h
    fusions/BurnOut_vulkan.h
    fusions/Crosshatch_shader.h
    fusions/Crosshatch_vulkan.h
    fusions/CannabisLeaf_shader.h
    fusions/CannabisLeaf_vulkan.h
    fusions/CrazyParametric_shader.h
    fusions/CrazyParametric_vulkan.h
    fusions/KaleidoScope_shader.h
    fusions/KaleidoScope_vulkan.h
    fusions/WindowBlinds_shader.h
    fusions/WindowBlinds_vulkan.h
    fusions/Hexagonalize_shader.h
    fusions/Hexagonalize_vulkan.h
    fusions/GlitchDisplace_shader.h
    fusions/GlitchDisplace_vulkan.h
    fusions/DreamyZoom_shader.h
    fusions/DreamyZoom_vulkan.h
    fusions/DoomScreen_shader.h
    fusions/DoomScreen_vulkan.h
    fusions/Ripple_shader.h
    fusions/Ripple_vulkan.h
    fusions/Pinwheel_shader.h
    fusions/Pinwheel_vulkan.h
    fusions/Burn_shader.h
    fusions/Burn_vulkan.h
    fusions/CircleBlur_shader.h
    fusions/CircleBlur_vulkan.h
    fusions/ColorPhase_shader.h
    fusions/ColorPhase_vulkan.h
    fusions/CrossWarp_shader.h
    fusions/CrossWarp_vulkan.h
    fusions/Cube_shader.h
    fusions/Cube_vulkan.h
    fusions/Doorway_shader.h
    fusions/Doorway_vulkan.h
    fusions/Flyeye_shader.h
    fusions/Flyeye_vulkan.h
    fusions/Heart_shader.h
    fusions/Heart_vulkan.h
    fusions/MultiplyBlend_shader.h
    fusions/MultiplyBlend_vulkan.h
)
set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/fusions
)

include_directories(${VKSHADER_INC_DIRS})

if(VKSHADER_STATIC)
    set(LIBRARY STATIC)
    add_definitions(-DVKSHADER_STATIC_LIBRARY)
else(VKSHADER_STATIC)
    set(LIBRARY SHARED)
    add_definitions(-DVKSHADER_SHARED_LIBRARY)
endif(VKSHADER_STATIC)

add_library(
    VkShader
    ${LIBRARY}
    ${VKSHADER_SRCS}
    ${VKSHADER_INCS}
)

if(VKSHADER_VULKAN_PREBUILD)
add_dependencies(VkShader generate-comp)
endif(VKSHADER_VULKAN_PREBUILD)

set(VKSHADER_VERSION_MAJOR 1)
set(VKSHADER_VERSION_MINOR 7)
set(VKSHADER_VERSION_PATCH 0)
set(VKSHADER_VERSION_STRING ${VKSHADER_VERSION_MAJOR}.${VKSHADER_VERSION_MINOR}.${VKSHADER_VERSION_PATCH})

if(NOT VKSHADER_STATIC)
set_property(TARGET VkShader PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(VkShader PROPERTIES VERSION ${VKSHADER_VERSION_STRING} SOVERSION ${VKSHADER_VERSION_MAJOR})
endif()
target_link_libraries(VkShader ${LINK_LIBS} ${GLSLANG_LIBRARY})

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(VKSHADER_LIBRARYS VkShader ${LINK_LIBS} PARENT_SCOPE )
    set(VKSHADER_INCLUDES ${VKSHADER_INCS} PARENT_SCOPE )
    set(VKSHADER_INCLUDE_DIRS ${VKSHADER_INC_DIRS} PARENT_SCOPE )
endif()