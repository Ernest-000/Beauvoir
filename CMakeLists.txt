cmake_minimum_required(VERSION 3.16.3)

project(Beauvoir C)

if(NOT DEFINED BVR_TARGET_SHARED)
    set(BVR_TARGET_SHARED OFF)
endif()

if(NOT DEFINED BVR_DEBUG)
    set(BVR_DEBUG OFF)
endif()

# Setting up Beauvoir paths
if(NOT DEFINED BVR_CURRENT_DIR)
    set(BVR_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(BVR_SOURCE_DIR ${BVR_CURRENT_DIR}/src/)
set(BVR_INCLUDE_DIR ${BVR_CURRENT_DIR}/include/)
set(BVR_LIB_DIR ${BVR_CURRENT_DIR}/lib/)
set(BVR_BUILD_DIR ${BVR_CURRENT_DIR}/build/)
set(BVR_BINARY_DIR ${BVR_CURRENT_DIR}/bin/)

set(BVR_DEMO_DIR ${BVR_CURRENT_DIR}/dev/)

# Define main file
# Development only
if(NOT ${BVR_TARGET_SHARED})
    if (NOT DEFINED BVR_MAIN_FILE)
        set(BVR_DEBUG OFF)
        # set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/landscape.c")
        # set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/model_viewer.c")
        set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/memory.c")
    endif()
endif()

## Check for modules
set(BVR_THIRD_PARTY_MODULES "")
set(BVR_THIRD_PARTY_MODULES_NAMES "SDL" "PortAudio" "Zlib" "LPng" "json-c")

if(("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC"))
    set(BVR_THIRD_PARTY_MODULES_PATH "SDL3.lib" "portaudio.lib" "zsd.lib" "libpng16d.lib" "libjson-c.lib")
else()
    set(BVR_THIRD_PARTY_MODULES_PATH "libSDL3.dll.a" "libportaudio.dll.a" "libz.dll.a" "libpng16.dll.a" "libjson-c.dll.a")
endif()

# Check for submodules
# foreach(BVR_THIRD_PARTY_MODULES_ITEM ${BVR_THIRD_PARTY_MODULES_NAMES})
#     set(BVR_THIRD_PARTY_MODULES_ITEM_PATH "${BVR_CURRENT_DIR}/extern/${BVR_THIRD_PARTY_MODULES_ITEM}")
# 
#     if(EXISTS ${BVR_THIRD_PARTY_MODULES_ITEM_PATH})
#         message("found ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
#     else()
#         message(FATAL_ERROR "cannot find ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
#     endif()
# endforeach()

foreach(BVR_THIRD_PARTY_MODULES_ITEM ${BVR_THIRD_PARTY_MODULES_PATH})
    if(EXISTS ${BVR_LIB_DIR}/${BVR_THIRD_PARTY_MODULES_ITEM})
        list(APPEND BVR_THIRD_PARTY_MODULES "${BVR_LIB_DIR}/${BVR_THIRD_PARTY_MODULES_ITEM}")
        message("found ${BVR_THIRD_PARTY_MODULES_ITEM}!")
    else()
        message(FATAL_ERROR "${BVR_THIRD_PARTY_MODULES_ITEM} does not exists")
    endif()
endforeach()


## add OpenGL package
set(BVR_THIRD_PARTY_PACKAGES "OpenGL")
foreach(BVR_THIRD_PARTY_PACKAGES_ITEM ${BVR_THIRD_PARTY_PACKAGES})
    find_package(${BVR_THIRD_PARTY_PACKAGES_ITEM} REQUIRED)
endforeach()

include_directories(${BVR_INCLUDE_DIR})

if(${BVR_DEBUG})
    message("setup for debugging")
else()
    # add_compile_options(-g)
    add_compile_options(-g  -w)
    message("setup for release")
endif()

## Create library or executable depends on the compilation mode
if(${BVR_TARGET_SHARED})
    add_library(Beauvoir SHARED)
    message("Successfully create a new library")
else()
    add_executable(Beauvoir ${BVR_MAIN_FILE})
    message("Successfully create a new executable")
endif()

## Link libs
target_link_libraries(Beauvoir PUBLIC
    ${OPENGL_LIBRARIES}
    ${BVR_THIRD_PARTY_MODULES}
)

target_include_directories(Beauvoir PUBLIC ${BVR_SOURCE_DIR})

## Include engine c files
file(GLOB_RECURSE SOURCES "${BVR_SOURCE_DIR}/*.c")
target_sources(Beauvoir PRIVATE ${SOURCES})

## Include editor c files
file(GLOB_RECURSE SOURCES "${BVR_SOURCE_DIR}/editor/*.c")
target_sources(Beauvoir PRIVATE ${SOURCES})

set_target_properties(Beauvoir PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BVR_BIN_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${BVR_BUILD_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BVR_BUILD_DIR}"
)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BVR_BIN_DIR})
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BVR_BUILD_DIR})
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BVR_BUILD_DIR})
endif()