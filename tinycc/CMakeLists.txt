set(TCC_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/")

# setup tcc
add_library(tcc STATIC
    ${TCC_SOURCE_PATH}libtcc.c
    ${TCC_SOURCE_PATH}tccgen.c
    ${TCC_SOURCE_PATH}tccelf.c
    ${TCC_SOURCE_PATH}tccpp.c
    ${TCC_PLATFORM_SRC}
)

target_include_directories(tcc PUBLIC
    ${TCC_SOURCE_PATH}
)

target_compile_definitions(tcc PRIVATE
    CONFIG_TCCDIR=""
    ONE_SOURCE
)

set_target_properties(tcc PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_options(tcc PRIVATE -w)

target_link_libraries(beauvoir PRIVATE tcc)

message(STATUS "Added TINYCC")