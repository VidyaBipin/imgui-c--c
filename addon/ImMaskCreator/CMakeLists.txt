cmake_minimum_required(VERSION 3.0.0)
project(ImMaskCreator VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)


option(SIMD_ARCH_X86                    "Build simd optimizations of x86_64" ON)

option(ImMaskCreator_STATIC             "Build ImMaskCreator as static library" OFF)
if(ImMaskCreator_STATIC)
    set(LIBRARY_TYPE STATIC)
else(ImMaskCreator_STATIC)
    set(LIBRARY_TYPE SHARED)
endif(ImMaskCreator_STATIC)

set(LIB_SRC_DIR ${PROJECT_SOURCE_DIR}/Src)
add_library(ImMaskCreator ${LIBRARY_TYPE}
    ${LIB_SRC_DIR}/ImMaskCreatorImpl.cpp
    ${LIB_SRC_DIR}/Contour2Mask.cpp
    ${LIB_SRC_DIR}/MatFilter.cpp
    # ${LIB_SRC_DIR}/MatIo.cpp
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
target_link_libraries(ImMaskCreator PUBLIC
    imgui
)

# if(IMGUI_BUILD_EXAMPLE)
# get_target_property(IMGUI_SRC_DIR imgui SOURCE_DIR)
# add_executable(ImMaskCreatorTest
#     ${PROJECT_SOURCE_DIR}/test/ImMaskCreatorTest.cpp
#     ${IMGUI_SRC_DIR}/../${IMGUI_APP_ENTRY_SRC}
# )
# target_link_libraries(ImMaskCreatorTest PRIVATE
#     ImMaskCreator
# )
# endif(IMGUI_BUILD_EXAMPLE)
