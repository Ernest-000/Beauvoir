cmake_minimum_required(VERSION 3.16.3)

project(Beauvoir_script LANGUAGES C)

set(BVR_TARGET_SHARED ON)
set(BVR_DEBUG OFF)

set(BVRS_SOURCE_PATH "${CMAKE_SOURCE_DIR}/src")
set(TCC_SOURCE_PATH "${CMAKE_SOURCE_DIR}/tinycc")

##
# SETUP TCC
##
add_library(libtcc STATIC
    ${TCC_SOURCE_PATH}/libtcc.c
    ${TCC_SOURCE_PATH}/tccgen.c
    ${TCC_SOURCE_PATH}/tccpp.c
    ${TCC_PLATFORM_SRC}
)

target_include_directories(libtcc PUBLIC
    ${TCC_SOURCE_PATH}
)

target_compile_definitions(libtcc PRIVATE
    CONFIG_TCCDIR=""
    ONE_SOURCE
)

target_compile_options(libtcc PRIVATE -w)

##
# Set Sources
##
file(GLOB_RECURSE SOURCES "${BVRS_SOURCE_PATH}/*.c")
# file(GLOB_RECURSE TINYC_SOURCES "${TINYC_SOURCE_PATH}/*.c")

add_executable(Beauvoir_script ${TINYC_SOURCES} ${SOURCES})

target_include_directories(Beauvoir_script PUBLIC
    ${BVRS_SOURCE_PATH}
    ${TCC_SOURCE_PATH}
)

target_link_libraries(Beauvoir_script PUBLIC libtcc)

# target_link_libraries(demo PRIVATE beauvoir)
# target_include_directories(demo PRIVATE "${BVR_PATH}/include/")

##
# Define your project DLL path
##
set_target_properties(Beauvoir_script PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)

##
# Define beauvoir DLL path
##
# set_target_properties(beauvoir PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
#     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
# )