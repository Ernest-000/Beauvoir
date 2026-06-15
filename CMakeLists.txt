cmake_minimum_required(VERSION 3.16)
project(beauvoir LANGUAGES C)

option(BVR_TARGET_SHARED "Build shared library" ON)
option(BVR_TARGET_STATIC "Build static library" OFF)

message(STATUS "BVR_TARGET_SHARED = ${BVR_TARGET_SHARED}")
message(STATUS "BVR_TARGET_STATIC = ${BVR_TARGET_STATIC}")

set(BVR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BVR_SOURCE_DIR ${BVR_ROOT_DIR}/src)
set(BVR_INCLUDE_DIR ${BVR_ROOT_DIR}/include)
set(BVR_LIB_DIR ${BVR_ROOT_DIR}/lib)
set(BVR_BUILD_DIR ${BVR_ROOT_DIR}/build)
set(BVR_BIN_DIR ${BVR_ROOT_DIR}/bin)
set(BVR_DEMO_DIR ${BVR_ROOT_DIR}/intern)

if(NOT BVR_TARGET_SHARED AND NOT BVR_TARGET_STATIC)
    set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/main.c")
endif()

if(WIN32)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|MSVC")
        set(BVR_THIRD_PARTY_NAMES zsd libjson-c)
    else()
        set(BVR_THIRD_PARTY_NAMES z json-c)
    endif()
elseif(UNIX)
    set(BVR_THIRD_PARTY_NAMES z json-c)
endif()

set(BVR_THIRD_PARTY_MODULES "")
foreach(name ${BVR_THIRD_PARTY_NAMES})
    find_library(LIB_${name} NAMES ${name} PATHS ${BVR_LIB_DIR} NO_DEFAULT_PATH REQUIRED)
    if(LIB_${name})
        list(APPEND BVR_THIRD_PARTY_MODULES ${LIB_${name}})
        message(STATUS "Found ${name}: ${LIB_${name}}")
    else()
        message(FATAL_ERROR "Missing third-party library: ${name}")
    endif()
endforeach()

if(BVR_TARGET_SHARED)
    message(STATUS "Creating a new shared lib")
    add_library(beauvoir SHARED)
elseif(BVR_TARGET_STATIC)
    message(STATUS "Creating a new static lib")
    add_library(beauvoir STATIC)
else()
    message(STATUS "Creating a new executable")
    add_executable(beauvoir ${BVR_MAIN_FILE})
endif()

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE BVR_ENGINE_SOURCES "${BVR_SOURCE_DIR}/*.c")

target_sources(beauvoir PRIVATE ${BVR_ENGINE_SOURCES})

target_include_directories(beauvoir
    PUBLIC
        $<BUILD_INTERFACE:${BVR_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${BVR_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(beauvoir PUBLIC
    OpenGL::GL
    ${BVR_THIRD_PARTY_MODULES}
)

target_link_options(beauvoir PRIVATE -lm)

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

set_target_properties(beauvoir PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${BVR_BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${BVR_BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BVR_BUILD_DIR}
)

install(TARGETS beauvoir
    EXPORT beauvoirTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(EXPORT beauvoirTargets
    FILE beauvoirConfig.cmake
    NAMESPACE beauvoir::
    DESTINATION lib/cmake/beauvoir
)

install(DIRECTORY ${BVR_INCLUDE_DIR}/ DESTINATION include)
install(DIRECTORY ${BVR_LIB_DIR}/     DESTINATION lib)
install(DIRECTORY ${BVR_BIN_DIR}/     DESTINATION bin)