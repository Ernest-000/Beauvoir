cmake_minimum_required(VERSION 3.16.3)

project(Beauvoir C)

if(NOT DEFINED BVR_TARGET_SHARED)
    set(BVR_TARGET_SHARED OFF)
endif()

# Setting up Beauvoir paths
if(NOT DEFINED BVR_CURRENT_DIR)
    set(BVR_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(BVR_SOURCE_DIR ${BVR_CURRENT_DIR}/src)
set(BVR_INCLUDE_DIR ${BVR_CURRENT_DIR}/include)
set(BVR_LIB_DIR ${BVR_CURRENT_DIR}/lib)
set(BVR_BUILD_DIR ${BVR_CURRENT_DIR}/build)
set(BVR_BINARY_DIR ${BVR_CURRENT_DIR}/bin)

set(BVR_DEMO_DIR ${BVR_CURRENT_DIR}/dev)

# Define main file
# Development only
if(NOT ${BVR_TARGET_SHARED})
    if (NOT DEFINED BVR_MAIN_FILE)
        #set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/landscape.c")
        set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/model_viewer.c")
    endif()
endif()

## Check for modules
set(BVR_THIRD_PARTY_MODULES_NAMES "SDL" "PortAudio" "Zlib" "LPng")
foreach(BVR_THIRD_PARTY_MODULES_ITEM ${BVR_THIRD_PARTY_MODULES_NAMES})
    set(BVR_THIRD_PARTY_MODULES_ITEM_PATH "${BVR_CURRENT_DIR}/extern/${BVR_THIRD_PARTY_MODULES_ITEM}")

    if(EXISTS ${BVR_THIRD_PARTY_MODULES_ITEM_PATH})
        message("found ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
    else()
        message(FATAL_ERROR "cannot find ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
    endif()
endforeach()

## add OpenGL package
set(BVR_THIRD_PARTY_PACKAGES "OpenGL")
foreach(BVR_THIRD_PARTY_PACKAGES_ITEM ${BVR_THIRD_PARTY_PACKAGES})
    find_package(${BVR_THIRD_PARTY_PACKAGES_ITEM} REQUIRED)
endforeach()

## Define variables for libraries based on your compiler
#
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(BVR_SDL_LIB ${BVR_LIB_DIR}/SDL3.lib)
    set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/portaudio.lib)
    set(BVR_Z_LIB ${BVR_LIB_DIR}/zsd.lib)
    set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16d.lib)
    set(BVR_JSON_LIB ${BVR_LIB_DIR}/libjson-c.lib)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")   
    set(BVR_SDL_LIB ${BVR_LIB_DIR}/SDL3.lib)
    set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/portaudio.lib)
    set(BVR_Z_LIB ${BVR_LIB_DIR}/zsd.lib)
    set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16d.lib)
    set(BVR_JSON_LIB ${BVR_LIB_DIR}/libjson-c.lib)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(BVR_SDL_LIB ${BVR_LIB_DIR}/libSDL3.dll.a)
    set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/libportaudio.dll.a)
    set(BVR_Z_LIB ${BVR_LIB_DIR}/libz.dll.a)
    set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16.dll.a)
    set(BVR_JSON_LIB ${BVR_LIB_DIR}/libjson-c.dll.a)

else()
    set(BVR_SDL_LIB ${BVR_LIB_DIR}/libSDL3.dll.a)
    set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/libportaudio.dll.a)
    set(BVR_Z_LIB ${BVR_LIB_DIR}/libz.dll.a)
    set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16.dll.a)
    set(BVR_JSON_LIB ${BVR_LIB_DIR}/libjson-c.dll.a)

endif()

if(NOT EXISTS ${BVR_SDL_LIB})
    message(FATAL_ERROR "Cannot find SDL3 '${BVR_SDL_LIB}'")
endif()

if(NOT EXISTS ${BVR_PORT_AUDIO_LIB})
    message(FATAL_ERROR "Cannot find PortAudio '${BVR_PORT_AUDIO_LIB}'")
endif()

if(NOT EXISTS ${BVR_Z_LIB})
    message(FATAL_ERROR "Cannot find Zlib '${BVR_Z_LIB}'")
endif()

if(NOT EXISTS ${BVR_PNG_LIB})
    message(FATAL_ERROR "Cannot find lPng '${BVR_PNG_LIB}'")
endif()

if(NOT EXISTS ${BVR_JSON_LIB})
    message(FATAL_ERROR "Cannot find json-c '${BVR_JSON_LIB}'")
endif()
include_directories(${BVR_INCLUDE_DIR})

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

## Create library or executable depends on the compilation mode
if(${BVR_TARGET_SHARED})
    add_compile_options(-g -w)
    add_library(Beauvoir STATIC)
    message("Successfully create a new library")
else()
    add_compile_options(-g -w)
    add_executable(Beauvoir ${BVR_MAIN_FILE})
    message("Successfully create a new executable")
endif()

## Link libs
target_link_libraries(Beauvoir PUBLIC
    ${OPENGL_LIBRARIES}
    ${BVR_SDL_LIB}
    ${BVR_PORT_AUDIO_LIB}
    ${BVR_Z_LIB}
    ${BVR_PNG_LIB}
    ${BVR_JSON_LIB}
)

target_include_directories(Beauvoir PUBLIC ${BVR_SOURCE_DIR})

## Include framework c files
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

if(MSVC)
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BVR_BUILD_DIR} )
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BVR_BUILD_DIR} )
    set_target_properties(Beauvoir PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BVR_BUILD_DIR} )
endif(MSVC)