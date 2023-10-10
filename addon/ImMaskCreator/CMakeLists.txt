cmake_minimum_required(VERSION 3.5.0)
project(ImMaskCreator VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

option(SIMD_ARCH_X86                    "Build simd optimizations of x86_64" ON)

if(NOT DEFINED ImMaskCreator_STATIC)
option(ImMaskCreator_STATIC             "Build ImMaskCreator as static library" OFF)
endif()
if(ImMaskCreator_STATIC)
    message("Build ImMaskCreator as static library")
    set(LIBRARY_TYPE STATIC)
else(ImMaskCreator_STATIC)
    message("Build ImMaskCreator as shared library")
    set(LIBRARY_TYPE SHARED)
endif(ImMaskCreator_STATIC)

set(ImMaskCreator_VERSION_MAJOR 1)
set(ImMaskCreator_VERSION_MINOR 0)
set(ImMaskCreator_VERSION_PATCH 1)
string(TIMESTAMP ImMaskCreator_VERSION_BUILD "%y%m%d")
set(ImMaskCreator_VERSION_STRING ${ImMaskCreator_VERSION_MAJOR}.${ImMaskCreator_VERSION_MINOR}.${ImMaskCreator_VERSION_PATCH})
add_definitions(-DImMaskCreator_VERSION_MAJOR=${ImMaskCreator_VERSION_MAJOR})
add_definitions(-DImMaskCreator_VERSION_MINOR=${ImMaskCreator_VERSION_MINOR})
add_definitions(-DImMaskCreator_VERSION_PATCH=${ImMaskCreator_VERSION_PATCH})
add_definitions(-DImMaskCreator_VERSION_BUILD=${ImMaskCreator_VERSION_BUILD})

set(LIB_SRC_DIR ${PROJECT_SOURCE_DIR}/Src)
add_library(ImMaskCreator ${LIBRARY_TYPE}
    ${LIB_SRC_DIR}/ImMaskCreatorImpl.cpp
    ${LIB_SRC_DIR}/Contour2Mask.cpp
    ${LIB_SRC_DIR}/MatFilter.cpp
    ${LIB_SRC_DIR}/MatIo.cpp
    ${LIB_SRC_DIR}/MatMath.cpp
    ${LIB_SRC_DIR}/Morph.cpp
    ${LIB_SRC_DIR}/SimdOpt.cpp
    ${LIB_SRC_DIR}/SysUtils.cpp
)
target_include_directories(ImMaskCreator PRIVATE
    ${LIB_SRC_DIR}
    ${IMGUI_INC_DIRS}
)
if(SIMD_ARCH_X86)
target_compile_definitions(ImMaskCreator PRIVATE SIMD_ARCH_X86)
target_compile_options(ImMaskCreator PRIVATE -mavx2 -mavx -msse4.2 -msse4.1 -mssse3 -msse3 -msse2 -msse -mmmx -mfma -mf16c)
endif(SIMD_ARCH_X86)
target_include_directories(ImMaskCreator PUBLIC
    ${PROJECT_SOURCE_DIR}/Include
    ${IMGUI_INCLUDE_DIRS}
)

if(NOT ImMaskCreator_STATIC)
set_property(TARGET ImMaskCreator PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(ImMaskCreator PROPERTIES VERSION ${ImMaskCreator_VERSION_STRING} SOVERSION ${ImMaskCreator_VERSION_MAJOR})
endif()

target_link_libraries(ImMaskCreator PUBLIC
    imgui
)
