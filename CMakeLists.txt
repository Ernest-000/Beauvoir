cmake_minimum_required(VERSION 3.16)
project(beauvoir LANGUAGES C)

##
# Options
##
option(BVR_TARGET_SHARED "Build shared library" OFF)
option(BVR_TARGET_STATIC "Build static library" OFF)

##
# Paths
##
set(BVR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BVR_SOURCE_DIR ${BVR_ROOT_DIR}/src)
set(BVR_INCLUDE_DIR ${BVR_ROOT_DIR}/include)
set(BVR_LIB_DIR ${BVR_ROOT_DIR}/lib)
set(BVR_BUILD_DIR ${BVR_ROOT_DIR}/build)
set(BVR_BIN_DIR ${BVR_ROOT_DIR}/bin)
set(BVR_DEMO_DIR ${BVR_ROOT_DIR}/dev)

##
# Main file (executable only)
##
if(NOT BVR_TARGET_SHARED AND NOT BVR_TARGET_STATIC)
    set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/memory.c")
endif()

##
# Third-party libraries
##
if(CMAKE_C_COMPILER_ID MATCHES "Clang|MSVC")
    set(BVR_THIRD_PARTY_LIBS
        SDL3.lib portaudio.lib zsd.lib libpng16d.lib libjson-c.lib)
else()
    set(BVR_THIRD_PARTY_LIBS
        libSDL3.dll.a libportaudio.dll.a libz.dll.a libpng16.dll.a libjson-c.dll.a)
endif()

set(BVR_THIRD_PARTY_MODULES "")
foreach(lib ${BVR_THIRD_PARTY_LIBS})
    if(EXISTS "${BVR_LIB_DIR}/${lib}")
        list(APPEND BVR_THIRD_PARTY_MODULES "${BVR_LIB_DIR}/${lib}")
        message("Found ${lib}")
    else()
        message(FATAL_ERROR "Missing third-party library: ${lib}")
    endif()
endforeach()

##
# Packages
##
find_package(OpenGL REQUIRED)

##
# Target creation
##
if(BVR_TARGET_SHARED)
    add_library(beauvoir SHARED)
elseif(BVR_TARGET_STATIC)
    add_library(beauvoir STATIC)
else()
    add_executable(beauvoir ${BVR_MAIN_FILE})
endif()

##
# Sources
##
file(GLOB_RECURSE BVR_ENGINE_SOURCES "${BVR_SOURCE_DIR}/*.c")
file(GLOB_RECURSE BVR_EDITOR_SOURCES "${BVR_SOURCE_DIR}/editor/*.c")

target_sources(beauvoir PRIVATE
    ${BVR_ENGINE_SOURCES}
    ${BVR_EDITOR_SOURCES}
)

##
# Includes (BUILD/INSTALL interface)
##
target_include_directories(beauvoir
    PUBLIC
        $<BUILD_INTERFACE:${BVR_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${BVR_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

##
# Link libraries
##
target_link_libraries(beauvoir PUBLIC
    OpenGL::GL
    ${BVR_THIRD_PARTY_MODULES}
)

##
# Compiler flags (multi-config)
##
target_compile_definitions(beauvoir PRIVATE
    $<$<CONFIG:Debug>:BVR_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(beauvoir PRIVATE
    $<$<CONFIG:Debug>:
        $<$<C_COMPILER_ID:GNU,Clang>:-g -O0 -Wall -Wextra>
        $<$<C_COMPILER_ID:MSVC>:/Zi /Od /W4>
    >
    $<$<CONFIG:Release>:
        $<$<C_COMPILER_ID:GNU,Clang>:-O3>
        $<$<C_COMPILER_ID:MSVC>:/O2>
    >
)

##
# C standard
##
set_target_properties(beauvoir PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

##
# Output directories
##
set_target_properties(beauvoir PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BVR_BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${BVR_BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BVR_BUILD_DIR}
)

##
# Installation & export
##
install(TARGETS beauvoir
    EXPORT beauvoirTargets
    RUNTIME DESTINATION ${BVR_BIN_DIR}
    LIBRARY DESTINATION ${BVR_BUILD_DIR}
    ARCHIVE DESTINATION ${BVR_BUILD_DIR}
)

install(EXPORT beauvoirTargets
    FILE beauvoirConfig.cmake
    NAMESPACE beauvoir::
    DESTINATION lib/cmake/beauvoir
)

install(DIRECTORY ${BVR_INCLUDE_DIR}/ DESTINATION include)
install(DIRECTORY ${BVR_LIB_DIR}/ DESTINATION lib)
install(DIRECTORY ${BVR_BIN_DIR}/ DESTINATION bin)