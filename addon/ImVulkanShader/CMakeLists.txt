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
option(VKSHADER_BUILD_TESTS           "Build Vulkan Shader Tests" OFF)
option(VKSHADER_SKIP_INSTALL          "Skip Vulkan install" ON)

find_package(PkgConfig)
find_package(OpenMP)

set(LINK_LIBS)

if(IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm")
    message(STATUS "Target arch: arm-ios")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp -mfpu=neon -march=armv7 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loop")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=softfp -mfpu=neon -march=armv7 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loop")
    set(VKSHADER_VULKAN_BENCHMARK OFF CACHE BOOL "Disable Vulkan Benchmark by Force" FORCE)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
    message(STATUS "Target arch: arm64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loops")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8 -ftree-vectorize -fpermissive -fomit-frame-pointer -funroll-loops")
    set(VKSHADER_VULKAN_BENCHMARK OFF CACHE BOOL "Disable Vulkan Benchmark by Force" FORCE)
elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|x86_64|AMD64)")
    if(CMAKE_OSX_ARCHITECTURES MATCHES "x86")
        message(STATUS "Target arch: x86")
    else()
        message(STATUS "Target arch: x86_64")
    endif()
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

if (APPLE AND OPENMP_FOUND)
    message(STATUS "VkShader(Apple) Use OpenMP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    include_directories(${OpenMP_CXX_INCLUDE_DIR})
    set(LINK_LIBS ${LINK_LIBS} ${OpenMP_libomp_LIBRARY})
elseif(OPENMP_FOUND)
    if(WIN32 OR MINGW)
        if (NOT VKSHADER_STATIC)
            message(STATUS "VkShader(Windows) Use OpenMP")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set(LINK_LIBS ${LINK_LIBS} gmp pthread)
        endif()
    else()
        message(STATUS "VkShader Use OpenMP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(LINK_LIBS ${LINK_LIBS} gmp pthread)
    endif()
endif()


if(NOT VULKAN_FOUND AND PKG_CONFIG_FOUND)
    pkg_search_module(VULKAN REQUIRED vulkan glslang)
endif()

if (VULKAN_FOUND)
    message(STATUS "[ImVulkanShader] vulkan found")
    set(VKSHADER_INC_DIRS
        ${Vulkan_INCLUDE_DIR}
    )
    if ( NOT "${VULKAN_LDFLAGS}" STREQUAL "")
        message(STATUS "[ImVulkanShader] vulkan link with ${VULKAN_LDFLAGS}")
        set(LINK_LIBS
            ${LINK_LIBS}
            ${VULKAN_LDFLAGS}
        )
    elseif (NOT "${Vulkan_LIBRARY}" STREQUAL "")
        message(STATUS "[ImVulkanShader] vulkan link with ${Vulkan_LIBRARY}")
        set(LINK_LIBS
            ${LINK_LIBS}
            ${Vulkan_LIBRARY}
        )
    endif()
endif(VULKAN_FOUND)

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
# general
compile_shader(others/ColorConvert_shader.h YUV2RGB_data)
compile_shader(others/ColorConvert_shader.h RGB2YUV_data)
compile_shader(others/ColorConvert_shader.h GRAY2RGB_data)
compile_shader(others/ColorConvert_shader.h Conv_data)
compile_shader(others/Concat_shader.h Shader_data)
compile_shader(others/Copy_make_border_shader.h Filter_data)
compile_shader(others/CopyTo_shader.h CopyTo_data)
compile_shader(others/Crop_shader.h CropShader_data)
compile_shader(others/Crop_shader.h CropToShader_data)
compile_shader(others/Flip_shader.h Shader_data)
compile_shader(others/Resize_shader.h Resize_data)
compile_shader(others/Substract_mean_normalize_shader.h Filter_data)
compile_shader(others/Transpose_shader.h Filter_data)
compile_shader(others/warpAffine_shader.h Filter_data)
compile_shader(others/warpPerspective_shader.h Filter_data)
# filters
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
compile_shader(filters/ColorBalance_shader.h Filter_data)
compile_shader(filters/ColorCurve_shader.h Filter_data)
compile_shader(filters/ColorInvert_shader.h Filter_data)
compile_shader(filters/Contrast_shader.h Filter_data)
compile_shader(filters/DeBand_shader.h DeBand_data)
compile_shader(filters/DeInterlace_shader.h DeInterlace_data)
compile_shader(filters/Exposure_shader.h Filter_data)
compile_shader(filters/Filter2D_shader.h Filter_data)
compile_shader(filters/Filter2DS_shader.h FilterColumn_data)
compile_shader(filters/Filter2DS_shader.h FilterRow_data)
compile_shader(filters/Filter2DS_shader.h FilterColumnMono_data)
compile_shader(filters/Filter2DS_shader.h FilterRowMono_data)
compile_shader(filters/Gamma_shader.h Filter_data)
compile_shader(filters/Guided_shader.h ToMatting_data)
compile_shader(filters/Guided_shader.h Guided_data)
compile_shader(filters/Guided_shader.h Matting_data)
compile_shader(filters/HQDN3D_shader.h HQDN3D_data)
compile_shader(filters/Hue_shader.h Filter_data)
compile_shader(filters/Saturation_shader.h Filter_data)
compile_shader(filters/Sobel_shader.h Filter_data)
compile_shader(filters/USM_shader.h USMFilter_data)
compile_shader(filters/Vibrance_shader.h Filter_data)
compile_shader(filters/WhiteBalance_shader.h Filter_data)
compile_shader(filters/lut3d/Lut3D_Shader.h LUT3D_data)
# scopes
compile_shader(scopes/CIE_shader.h CIE_data)
compile_shader(scopes/CIE_shader.h CIE_merge_data)
compile_shader(scopes/CIE_shader.h CIE_set_data)
compile_shader(scopes/Harris_shader.h PrewittFilter_data)
compile_shader(scopes/Harris_shader.h HarrisFilter_data)
compile_shader(scopes/Harris_shader.h NMSFilter_data)
compile_shader(scopes/Histogram_shader.h Histogram_data)
compile_shader(scopes/Histogram_shader.h Zero_data)
compile_shader(scopes/Histogram_shader.h ConvInt2Float_data)
compile_shader(scopes/Vector_shader.h Vector_data)
compile_shader(scopes/Vector_shader.h Zero_data)
compile_shader(scopes/Vector_shader.h Vector_merge_data)
compile_shader(scopes/Waveform_shader.h Waveform_data)
compile_shader(scopes/Waveform_shader.h Zero_data)
compile_shader(scopes/Waveform_shader.h ConvInt2Mat_data)
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
compile_shader(fusions/Pixelize_shader.h Pixelize_data)
compile_shader(fusions/Polar_shader.h Polar_data)
compile_shader(fusions/RandomSquares_shader.h RandomSquares_data)
compile_shader(fusions/RotateScale_shader.h RotateScale_data)
compile_shader(fusions/SquaresWire_shader.h SquaresWire_data)
compile_shader(fusions/Squeeze_shader.h Squeeze_data)
compile_shader(fusions/Wind_shader.h Wind_data)
compile_shader(fusions/LinearBlur_shader.h LinearBlur_data)
compile_shader(fusions/Door_shader.h Door_data)
compile_shader(fusions/Fade_shader.h Fade_data)
compile_shader(fusions/Slider_shader.h Slider_data)
compile_shader(fusions/Wipe_shader.h Wipe_data)
compile_shader(fusions/Move_shader.h Move_data)
compile_shader(fusions/Rolls_shader.h Rolls_data)
compile_shader(fusions/Edge_shader.h Edge_data)
compile_shader(fusions/DirectionalScaled_shader.h DirectionalScaled_data)
compile_shader(fusions/Rectangle_shader.h Rectangle_data)
compile_shader(fusions/RotateScaleVanish_shader.h RotateScaleVanish_data)
compile_shader(fusions/SimpleZoomOut_shader.h SimpleZoomOut_data)
compile_shader(fusions/StaticWipe_shader.h StaticWipe_data)
compile_shader(fusions/CircleBlur_shader.h CircleBlur_data)

add_custom_target(generate-comp DEPENDS ${SHADER_SPV_HEX_FILES})

endif(VKSHADER_VULKAN_PREBUILD)

find_package(Glslang COMPONENTS Glslang SPIRV)
if (Glslang_LIBRARY_RELEASE)
    message(STATUS "VkShader Found system glslang library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        ${Glslang_LIBRARY_RELEASE}
        ${Glslang_GenericCodeGen_LIBRARY_RELEASE}
        ${Glslang_OGLCompiler_LIBRARY_RELEASE}
        ${Glslang_OSDependent_LIBRARY_RELEASE}
        ${Glslang_SPIRV_LIBRARY_RELEASE}
    )
    if(NOT WIN32 AND NOT MINGW)
        set(GLSLANG_LIBRARY
            ${GLSLANG_LIBRARY}
            ${Glslang_MachineIndependent_LIBRARY_RELEASE}
        )
    endif()
endif()
find_package(SpirvTools)
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
    ImVulkanShader.h
)

# add filter
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    filters/Filter2D_vulkan.cpp
    filters/Filter2DS_vulkan.cpp
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
    filters/USM_vulkan.cpp
    filters/ChromaKey_vulkan.cpp
    filters/GaussianBlur.cpp
    filters/Box.cpp
    filters/Laplacian.cpp
    filters/CAS_vulkan.cpp
    filters/HQDN3D_vulkan.cpp
    filters/DeBand_vulkan.cpp
    filters/DeInterlace_vulkan.cpp
    filters/Guided_vulkan.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    filters/Filter2D_shader.h
    filters/Filter2D_vulkan.h
    filters/Filter2DS_shader.h
    filters/Filter2DS_vulkan.h
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
    filters/USM_shader.h
    filters/USM_vulkan.h
    filters/ChromaKey_shader.h
    filters/ChromaKey_vulkan.h
    filters/GaussianBlur.h
    filters/Box.h
    filters/Laplacian.h
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
    fusions/Swirl_vulkan.cpp
    fusions/Dreamy_vulkan.cpp
    fusions/GridFlip_vulkan.cpp
    fusions/ZoomInCircles_vulkan.cpp
    fusions/Mosaic_vulkan.cpp
    fusions/KaleidoScope_vulkan.cpp
    fusions/WindowBlinds_vulkan.cpp
    fusions/Hexagonalize_vulkan.cpp
    fusions/GlitchDisplace_vulkan.cpp
    fusions/DreamyZoom_vulkan.cpp
    fusions/DoomScreen_vulkan.cpp
    fusions/Ripple_vulkan.cpp
    fusions/Pinwheel_vulkan.cpp
    fusions/Doorway_vulkan.cpp
    fusions/Flyeye_vulkan.cpp
    fusions/Heart_vulkan.cpp
    fusions/MultiplyBlend_vulkan.cpp
    fusions/Pixelize_vulkan.cpp
    fusions/Polar_vulkan.cpp
    fusions/RandomSquares_vulkan.cpp
    fusions/RotateScale_vulkan.cpp
    fusions/SquaresWire_vulkan.cpp
    fusions/Squeeze_vulkan.cpp
    fusions/Wind_vulkan.cpp
    fusions/Door_vulkan.cpp
    fusions/Fade_vulkan.cpp
    fusions/Slider_vulkan.cpp
    fusions/Wipe_vulkan.cpp
    fusions/Move_vulkan.cpp
    fusions/Rolls_vulkan.cpp
    fusions/Edge_vulkan.cpp
    fusions/StaticWipe_vulkan.cpp
    fusions/DirectionalScaled_vulkan.cpp
    fusions/Rectangle_vulkan.cpp
    fusions/SimpleZoomOut_vulkan.cpp
    fusions/RotateScaleVanish_vulkan.cpp
)

set(VKSHADER_INCS
    ${VKSHADER_INCS}
    fusions/AlphaBlending_shader.h
    fusions/AlphaBlending_vulkan.h
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
    fusions/Swirl_shader.h
    fusions/Swirl_vulkan.h
    fusions/Dreamy_shader.h
    fusions/Dreamy_vulkan.h
    fusions/GridFlip_shader.h
    fusions/GridFlip_vulkan.h
    fusions/ZoomInCircles_shader.h
    fusions/ZoomInCircles_vulkan.h
    fusions/Mosaic_shader.h
    fusions/Mosaic_vulkan.h
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
    fusions/Doorway_shader.h
    fusions/Doorway_vulkan.h
    fusions/Flyeye_shader.h
    fusions/Flyeye_vulkan.h
    fusions/Heart_shader.h
    fusions/Heart_vulkan.h
    fusions/MultiplyBlend_shader.h
    fusions/MultiplyBlend_vulkan.h
    fusions/Pixelize_shader.h
    fusions/Pixelize_vulkan.h
    fusions/Polar_shader.h
    fusions/Polar_vulkan.h
    fusions/RandomSquares_shader.h
    fusions/RandomSquares_vulkan.h
    fusions/RotateScale_shader.h
    fusions/RotateScale_vulkan.h
    fusions/SquaresWire_shader.h
    fusions/SquaresWire_vulkan.h
    fusions/Squeeze_shader.h
    fusions/Squeeze_vulkan.h
    fusions/Wind_shader.h
    fusions/Wind_vulkan.h
    fusions/Door_shader.h
    fusions/Door_vulkan.h
    fusions/Fade_shader.h
    fusions/Fade_vulkan.h
    fusions/Slider_shader.h
    fusions/Slider_vulkan.h
    fusions/Wipe_shader.h
    fusions/Wipe_vulkan.h
    fusions/Move_shader.h
    fusions/Move_vulkan.h
    fusions/Rolls_shader.h
    fusions/Rolls_vulkan.h
    fusions/Edge_shader.h
    fusions/Edge_vulkan.h
    fusions/StaticWipe_shader.h
    fusions/StaticWipe_vulkan.h
    fusions/DirectionalScaled_shader.h
    fusions/DirectionalScaled_vulkan.h
    fusions/Rectangle_shader.h
    fusions/Rectangle_vulkan.h
    fusions/SimpleZoomOut_shader.h
    fusions/SimpleZoomOut_vulkan.h
    fusions/RotateScaleVanish_shader.h
    fusions/RotateScaleVanish_vulkan.h
)
set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/fusions
)

# add scopes
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    scopes/Harris_vulkan.cpp
    scopes/CIE_vulkan.cpp
    scopes/Histogram_vulkan.cpp
    scopes/Waveform_vulkan.cpp
    scopes/Vector_vulkan.cpp
)

set(VKSHADER_INCS
    ${VKSHADER_INCS}
    scopes/Harris_vulkan.h
    scopes/Harris_shader.h
    scopes/CIE_shader.h
    scopes/CIE_vulkan.h
    scopes/Histogram_shader.h
    scopes/Histogram_vulkan.h
    scopes/Waveform_shader.h
    scopes/Waveform_vulkan.h
    scopes/Vector_shader.h
    scopes/Vector_vulkan.h
)

set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/scopes
)

# add others
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    others/colorspace_table.cpp
    others/ColorConvert_vulkan.cpp
    others/Resize_vulkan.cpp
    others/CopyTo_vulkan.cpp
    others/Flip_vulkan.cpp
    others/Crop_vulkan.cpp
    others/Transpose_vulkan.cpp
    others/Concat_vulkan.cpp
    others/warpPerspective_vulkan.cpp
    others/warpAffine_vulkan.cpp
    others/Substract_mean_normalize.cpp
    others/Copy_make_border.cpp
)

set(VKSHADER_INCS
    ${VKSHADER_INCS}
    others/ColorConvert_shader.h
    others/ColorConvert_vulkan.h
    others/Resize_shader.h
    others/Resize_vulkan.h
    others/CopyTo_shader.h
    others/CopyTo_vulkan.h
    others/Flip_shader.h
    others/Flip_vulkan.h
    others/Crop_shader.h
    others/Crop_vulkan.h
    others/Transpose_shader.h
    others/Transpose_vulkan.h
    others/Concat_shader.h
    others/Concat_vulkan.h
    others/warpPerspective_shader.h
    others/warpPerspective_vulkan.h
    others/warpAffine_shader.h
    others/warpAffine_vulkan.h
    others/Substract_mean_normalize_shader.h
    others/Substract_mean_normalize.h
    others/Copy_make_border_shader.h
    others/Copy_make_border.h
)

set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/others
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
set(VKSHADER_VERSION_MINOR 9)
set(VKSHADER_VERSION_PATCH 1)
string(TIMESTAMP VKSHADER_VERSION_BUILD "%y%m%d")
set(VKSHADER_VERSION_STRING ${VKSHADER_VERSION_MAJOR}.${VKSHADER_VERSION_MINOR}.${VKSHADER_VERSION_PATCH})
add_definitions(-DVKSHADER_VERSION_MAJOR=${VKSHADER_VERSION_MAJOR})
add_definitions(-DVKSHADER_VERSION_MINOR=${VKSHADER_VERSION_MINOR})
add_definitions(-DVKSHADER_VERSION_PATCH=${VKSHADER_VERSION_PATCH})
add_definitions(-DVKSHADER_VERSION_BUILD=${VKSHADER_VERSION_BUILD})

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
if(UNIX AND NOT VKSHADER_SKIP_INSTALL)
install(TARGETS VkShader
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(VKSHADER_BUILD_TESTS)
    add_executable(
        ImVulkan_tests
        test/ImVulkan_tests.cpp
    )
    target_link_libraries(
        ImVulkan_tests
        VkShader
        ${LINK_LIBS}
    )
endif(VKSHADER_BUILD_TESTS)