cmake_minimum_required(VERSION 3.16.3)

project(demo LANGUAGES C)

set(BVR_TARGET_SHARED ON)
set(BVR_DEBUG OFF)

##
# Set Beauvoir's path
##
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../../build/" ${CMAKE_PREFIX_PATH})
set(BVR_PATH "${CMAKE_SOURCE_DIR}/../..")

##
# Use Beauvoir without building it
##
# find_package(beauvoir REQUIRED)

##
# Use Beauvoir and recompile it each time
##
add_subdirectory("${CMAKE_SOURCE_DIR}/../../" "${CMAKE_SOURCE_DIR}/../../build/")

##
# Set Sources
##
set(SOURCES "main.c")

add_executable(demo ${SOURCES})

target_link_libraries(demo PRIVATE beauvoir)
target_include_directories(demo PRIVATE "${BVR_PATH}/include/")

##
# Define your project DLL path
##
set_target_properties(demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)

##
# Define beauvoir DLL path
##
set_target_properties(beauvoir PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)