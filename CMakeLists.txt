cmake_minimum_required(VERSION 3.16.3)

project(Beauvoir_script LANGUAGES C)

set(BVR_TARGET_SHARED ON)
set(BVR_DEBUG OFF)  

set(BVRS_SOURCE_PATH "${CMAKE_SOURCE_DIR}/src/")
set(BVRS_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include/")
set(BVRS_LIB_PATH "${CMAKE_SOURCE_DIR}/lib/")
set(TCC_SOURCE_PATH "${CMAKE_SOURCE_DIR}/tinycc/")

link_directories(Beauvoir_script PUBLIC ${BVRS_LIB_PATH})

##
# SETUP TCC
##
add_library(tcc STATIC
    ${TCC_SOURCE_PATH}/libtcc.c
    ${TCC_SOURCE_PATH}/tccgen.c
    ${TCC_SOURCE_PATH}/tccelf.c
    ${TCC_SOURCE_PATH}/tccpp.c
    ${TCC_PLATFORM_SRC}
)

target_include_directories(tcc PUBLIC
    ${TCC_SOURCE_PATH}
)

target_compile_definitions(tcc PRIVATE
    CONFIG_TCCDIR=""
    ONE_SOURCE
)

target_compile_options(tcc PRIVATE -w)

##
# Find beauvoir
##
# set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../Beauvoir/build/" ${CMAKE_PREFIX_PATH})
find_package(beauvoir REQUIRED)

##
# Set Sources
##
file(GLOB_RECURSE SOURCES "${BVRS_SOURCE_PATH}/*.c")
# file(GLOB_RECURSE TINYC_SOURCES "${TINYC_SOURCE_PATH}/*.c")

add_executable(Beauvoir_script ${SOURCES})

target_include_directories(Beauvoir_script PUBLIC
    ${BVRS_SOURCE_PATH}
    ${BVRS_INCLUDE_PATH}
    ${TCC_SOURCE_PATH}
)   

target_link_libraries(Beauvoir_script PUBLIC tcc)
target_link_libraries(Beauvoir_script PRIVATE beauvoir)

# target_link_libraries(demo PRIVATE beauvoir)
# target_include_directories(demo PRIVATE "${BVR_PATH}/include/")

##
# Define your project DLL path
##
set_target_properties(tcc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

set_target_properties(Beauvoir_script PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)