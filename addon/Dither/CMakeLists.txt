cmake_minimum_required(VERSION 3.5.1)
PROJECT(Dither)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_MACOSX_RPATH 0)
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif()

option(BUILD_DITHER_STATIC          "Set to ON to build Static Library"  ON)
option(BUILD_DITHER_TEST            "Set to ON to build LLama test"  OFF)

## core library
SET(DITHER_SOURCE_FILES
    src/dither_grid.cpp
    src/dither_errordiff.cpp
    src/dither_ordered.cpp
    src/dither_dotdiff.cpp
    src/dither_varerrdiff.cpp
    src/dither_threshold.cpp
    src/dither_dbs.cpp
    src/dither_kallebach.cpp
    src/dither_riemersma.cpp
    src/dither_pattern.cpp
    src/dither_dotlippens.cpp
    src/hash.cpp
    src/random.cpp
    src/queue.cpp
)

SET(DITHER_HEADER_FILES
    src/dither_errordiff_data.h
    src/dither_ordered_data.h
    src/dither_dotdiff_data.h
    src/dither_varerrdiff_data.h
    src/dither_kallebach_data.h
    src/dither_riemersma_data.h
    src/dither_pattern_data.h
    src/dither_dotlippens_data.h
    src/hash.h
    src/random.h
    src/queue.h
    inc/libdither.h
)

set(DITHER_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

include_directories(${DITHER_INC_DIRS})

if (BUILD_DITHER_STATIC)
set(LIBRARY STATIC)
else(BUILD_DITHER_STATIC)
set(LIBRARY SHARED)
endif(BUILD_DITHER_STATIC)

add_library(dither
    ${LIBRARY}
    ${DITHER_SOURCE_FILES}
    ${DITHER_HEADER_FILES}
)

add_compile_definitions(LIB_VERSION="CodeWin")

if (NOT BUILD_DITHER_STATIC)
set_target_properties(dither PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if (BUILD_DITHER_TEST)
add_executable(
    dither_test
    test/test.cpp
    test/bmp.cpp
    test/bmp.h
)

target_link_libraries(
    dither_test
    dither
)
endif(BUILD_DITHER_TEST)

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
set(DITHER_LIBRARYS dither ${LINK_LIBS} PARENT_SCOPE )
set(DITHER_INCLUDES ${DITHER_HEADER_FILES} PARENT_SCOPE )
set(DITHER_INCLUDE_DIRS ${DITHER_INC_DIRS} PARENT_SCOPE )
endif(hasParent)
