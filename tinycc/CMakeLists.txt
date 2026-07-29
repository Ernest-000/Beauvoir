set(TCC_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/")
set(TCC_DEFAULT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config.h.default")

# try to find user's architecture
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" HOST_ARCH)
if(HOST_ARCH MATCHES "^(x86_64|amd64)$")
    set(TCC_TARGET_ARCH_DEFINE "TCC_TARGET_X86_64")
    set(TCC_PLATFORM_SRC
        ${TCC_SOURCE_PATH}x86_64-gen.c
        ${TCC_SOURCE_PATH}x86_64-asm.h
        ${TCC_SOURCE_PATH}i386-asm.c
    )
elseif(HOST_ARCH MATCHES "^(i386|i686|x86)$")
    set(TCC_TARGET_ARCH_DEFINE "TCC_TARGET_I386")
    set(TCC_PLATFORM_SRC
        ${TCC_SOURCE_PATH}i386-gen.c
        ${TCC_SOURCE_PATH}i386-asm.c
    )
elseif(HOST_ARCH MATCHES "^(aarch64|arm64)$")
    set(TCC_TARGET_ARCH_DEFINE "TCC_TARGET_ARM64")
    set(TCC_PLATFORM_SRC
        ${TCC_SOURCE_PATH}arm64-gen.c
        ${TCC_SOURCE_PATH}arm64-asm.c
    )
elseif(HOST_ARCH MATCHES "^(arm|armv7|armv7l|armv6)$")
    set(TCC_TARGET_ARCH_DEFINE "TCC_TARGET_ARM")
    set(TCC_PLATFORM_SRC
        ${TCC_SOURCE_PATH}arm-gen.c
        ${TCC_SOURCE_PATH}arm-asm.c
        ${TCC_SOURCE_PATH}arm-link.c
    )
else()
    message(FATAL_ERROR "TinyCC doesn't support ${HOST_ARCH}")
endif()

# try to find user's os
if(WIN32)
    set(TCC_HOST_OS "Windows")
    set(TCC_TARGET_PE ON)          # #cmakedefine TCC_TARGET_PE
elseif(APPLE)
    set(TCC_HOST_OS "Darwin")
    set(TCC_TARGET_PE OFF)
else()
    set(TCC_HOST_OS "Linux")
    set(TCC_TARGET_PE OFF)
endif()

set(TCC_HOST_CPU "${CMAKE_HOST_SYSTEM_PROCESSOR}")
set(CONFIG_TCC_STATIC ON)          # #cmakedefine CONFIG_TCC_STATIC
set(TCC_CONFIG_TCCDIR "")
set(TCC_LIBTCC1 "")
set(TCC_CONFIG_LDDIR "")

message(STATUS "Running TinyCC with ${TCC_HOST_OS} | ${HOST_ARCH} | ${TCC_TARGET_ARCH_DEFINE}")

# create tcc config.h
configure_file(
    ${TCC_DEFAULT_CONFIG}
    ${TCC_SOURCE_PATH}/config.h
    @ONLY
)

# create tcc lib
add_library(tcc STATIC
    ${TCC_SOURCE_PATH}libtcc.c
    ${TCC_PLATFORM_SRC}
)

target_include_directories(tcc PUBLIC
    ${TCC_SOURCE_PATH}
)

target_compile_definitions(tcc PRIVATE
    ONE_SOURCE
    TCC_IS_NATIVE
)

set_target_properties(tcc PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_options(tcc PRIVATE -w)
target_link_libraries(beauvoir PRIVATE tcc)