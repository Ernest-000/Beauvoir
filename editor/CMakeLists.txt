cmake_minimum_required(VERSION 3.16.3)

project(bergson LANGUAGES C)

##
# set options
##
set(BVR_TARGET_SHARED ON)
set(BVR_USE_GTK ON)
set(BVR_DEBUG ON)

##
# Set Beauvoir's path
##
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../build/" ${CMAKE_PREFIX_PATH})
set(BVR_PATH "${CMAKE_SOURCE_DIR}/..")

##
# Set editor paths
##
set(BGS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BGS_SOURCE_DIR ${BGS_ROOT_DIR}/src)
set(BGS_XML_SOURCE_DIR ${BGS_ROOT_DIR}/xml)
set(BGS_INCLUDE_DIR ${BGS_ROOT_DIR}/include)
set(BGS_LIB_DIR ${BGS_ROOT_DIR}/lib)
set(BGS_BUILD_DIR ${BGS_ROOT_DIR}/build)
set(BGS_BIN_DIR ${BGS_ROOT_DIR}/bin)

message(STATUS "BGS_SOURCE_DIR = ${BGS_SOURCE_DIR}")
message(STATUS "BGS_INCLUDE_DIR = ${BGS_INCLUDE_DIR}")
message(STATUS "BGS_LIB_DIR = ${BGS_LIB_DIR}")
message(STATUS "BGS_BUILD_DIR = ${BGS_BUILD_DIR}")
message(STATUS "BGS_BIN_DIR = ${BGS_BIN_DIR}")

##
# PKG
##
find_package(PkgConfig REQUIRED)

##
# GTK4
##
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

add_compile_options("${compiler_opt}" ${GTK4_CFLAGS_OTHER})

##
# Use Beauvoir without building it
##
# find_package(beauvoir REQUIRED)
find_package (Threads REQUIRED)

##
# Use Beauvoir and recompile it each time
##
add_subdirectory("${CMAKE_SOURCE_DIR}/../" "${CMAKE_SOURCE_DIR}/../build/")


##
# Set Sources
##
file(GLOB_RECURSE BGS_SOURCES "${BGS_SOURCE_DIR}/*.c")
file(GLOB_RECURSE BGS_XML_SOURCES "${BGS_XML_SOURCE_DIR}/*.xml")

add_executable(bergson ${BGS_SOURCES})

##
# Enable or not the debug flag
##
if(BVR_DEBUG)
    target_compile_definitions(bergson PRIVATE BVR_DEBUG=${BVR_DEBUG})
    target_compile_options(beauvoir PRIVATE -g)
    target_compile_options(bergson PRIVATE -g)
endif()

##
# check for valid xmls
##


##
# target lib
##
target_link_libraries(bergson PRIVATE 
    ${GTK4_LIBRARIES} 
    ${BVR_THIRD_PARTY_MODULES}
    Threads::Threads
    beauvoir
)

target_include_directories(bergson PRIVATE 
    "${BVR_PATH}/include/"
    ${BGS_INCLUDE_DIR}
)

##
# Define your project DLL path
##
set_target_properties(bergson PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BGS_BIN_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${BGS_BUILD_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BGS_BUILD_DIR}"
)

##
# Define beauvoir DLL path
##
set_target_properties(beauvoir PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BGS_BIN_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${BGS_BUILD_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BGS_BUILD_DIR}"
)