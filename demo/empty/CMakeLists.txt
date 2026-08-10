cmake_minimum_required(VERSION 3.16.3)

project(app LANGUAGES C)

##
# set options
##
set(BVR_TARGET_SHARED ON)
set(BVR_DEBUG ON)

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

add_executable(app ${SOURCES})

##
# Enable or not the debug flag
##
if(BVR_DEBUG)
    target_compile_definitions(app PRIVATE BVR_DEBUG=${BVR_DEBUG})
    target_compile_options(beauvoir PRIVATE -g)
    target_compile_options(app PRIVATE -g)
endif()

target_link_libraries(app PRIVATE beauvoir)
target_include_directories(app PRIVATE "${BVR_PATH}/include/")

##
# Define your project DLL path
##
set_target_properties(app PROPERTIES
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