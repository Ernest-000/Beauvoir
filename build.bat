@echo off
setlocal enabledelayedexpansion

set BVR_GENERATOR=NMake Makefiles
set BVR_CC=gcc
set BVR_CXX=g++

set BVR_ROOT_DIR=%~dp0
set BVR_ROOT_DIR=%BVR_ROOT_DIR:~0,-1%
set BVR_BUILD_DIR=%TEMP%\beauvoir-build
set BVR_INCLUDE_DIR=%BVR_ROOT_DIR%\include
set BVR_NUKLEAR_DIR=%BVR_ROOT_DIR%\extern\Nuklear\src

set BVR_EXTERNAL_MODULES=SDL Zlib Lpng json-c

set BVR_CLEAR=false
set BVR_SKIP_BIN=false
set BVR_SKIP_SYNC=false

for %%A in (%*) do (
    if "%%A"=="--clear"          set BVR_CLEAR=true
    if "%%A"=="--skip-binaries"  set BVR_SKIP_BIN=true
    if "%%A"=="--skip-git-sync"  set BVR_SKIP_SYNC=true
)

if "%BVR_SKIP_SYNC%"=="false" (
    git submodule deinit -f .
    git submodule update --init
    git submodule update --remote --merge
)

if "%BVR_CLEAR%"=="true" (
    echo Cleaning...

    if exist "%BVR_ROOT_DIR%\bin"      rmdir /s /q "%BVR_ROOT_DIR%\bin"
    if exist "%BVR_ROOT_DIR%\lib"      rmdir /s /q "%BVR_ROOT_DIR%\lib"
    if exist "%BVR_ROOT_DIR%\licenses" rmdir /s /q "%BVR_ROOT_DIR%\licenses"
    if exist "%BVR_ROOT_DIR%\cmake"    rmdir /s /q "%BVR_ROOT_DIR%\cmake"
    if exist "%BVR_BUILD_DIR%"         rmdir /s /q "%BVR_BUILD_DIR%"

    mkdir "%BVR_ROOT_DIR%\bin"
    mkdir "%BVR_ROOT_DIR%\lib"
)

if "%BVR_SKIP_BIN%"=="false" (
    for %%M in (%BVR_EXTERNAL_MODULES%) do (
        set MOD=%%M
        set MODULE_PATH=%BVR_ROOT_DIR%\extern\%%M
        set MODULE_BUILD_DIR=%BVR_BUILD_DIR%\%%M

        if exist "!MODULE_PATH!" (
            echo !MODULE_PATH! found!

            set BVR_MODULE_FLAGS=-DBUILD_SHARED_LIBS=OFF -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON

            if "%%M"=="SDL" (
                set BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DSDL_SHARED=OFF -DSDL_STATIC=ON
            )
            if "%%M"=="Lpng" (
                set BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_NO_VERSIONEDLINKS=ON
            )
            if "%%M"=="Zlib" (
                set BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DZLIB_BUILD_EXAMPLES=OFF
            )
            if "%%M"=="json-c" (
                set BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DBUILD_SHARED_LIBS=ON
            )

            set BVR_TMP_INSTALL=!MODULE_BUILD_DIR!\install

            cmake --fresh ^
                -S "!MODULE_PATH!" ^
                -G "%BVR_GENERATOR%" ^
                -B "!MODULE_BUILD_DIR!" ^
                -DCMAKE_INSTALL_PREFIX="!BVR_TMP_INSTALL!" ^
                -DCMAKE_C_COMPILER="%BVR_CC%" ^
                !BVR_MODULE_FLAGS!

            cmake --build "!MODULE_BUILD_DIR!" --target install

            for %%F in ("!BVR_TMP_INSTALL!\lib\*.a" "!BVR_TMP_INSTALL!\lib\*.lib") do (
                if exist "%%F" copy /Y "%%F" "%BVR_ROOT_DIR%\lib\"
            )

            if exist "!BVR_TMP_INSTALL!\include\" (
                xcopy /E /I /Y "!BVR_TMP_INSTALL!\include\*" "%BVR_ROOT_DIR%\include\"
            )
        ) else (
            echo !MODULE_PATH! not found!
        )
    )
)

set NK_HEADER=%BVR_NUKLEAR_DIR%\HEADER.md
set NK_FOOTER=%BVR_NUKLEAR_DIR%\LICENSE %BVR_NUKLEAR_DIR%\CHANGELOG %BVR_NUKLEAR_DIR%\CREDITS

set NK_PRIV1=^
%BVR_NUKLEAR_DIR%\nuklear_internal.h ^
%BVR_NUKLEAR_DIR%\nuklear_math.c ^
%BVR_NUKLEAR_DIR%\nuklear_util.c ^
%BVR_NUKLEAR_DIR%\nuklear_color.c ^
%BVR_NUKLEAR_DIR%\nuklear_utf8.c ^
%BVR_NUKLEAR_DIR%\nuklear_buffer.c ^
%BVR_NUKLEAR_DIR%\nuklear_string.c ^
%BVR_NUKLEAR_DIR%\nuklear_draw.c ^
%BVR_NUKLEAR_DIR%\nuklear_vertex.c

set NK_PRIV2=^
%BVR_NUKLEAR_DIR%\nuklear_font.c ^
%BVR_NUKLEAR_DIR%\nuklear_input.c ^
%BVR_NUKLEAR_DIR%\nuklear_style.c ^
%BVR_NUKLEAR_DIR%\nuklear_context.c ^
%BVR_NUKLEAR_DIR%\nuklear_pool.c ^
%BVR_NUKLEAR_DIR%\nuklear_page_element.c ^
%BVR_NUKLEAR_DIR%\nuklear_table.c ^
%BVR_NUKLEAR_DIR%\nuklear_panel.c ^
%BVR_NUKLEAR_DIR%\nuklear_window.c ^
%BVR_NUKLEAR_DIR%\nuklear_popup.c ^
%BVR_NUKLEAR_DIR%\nuklear_contextual.c ^
%BVR_NUKLEAR_DIR%\nuklear_menu.c ^
%BVR_NUKLEAR_DIR%\nuklear_layout.c ^
%BVR_NUKLEAR_DIR%\nuklear_tree.c ^
%BVR_NUKLEAR_DIR%\nuklear_group.c ^
%BVR_NUKLEAR_DIR%\nuklear_list_view.c ^
%BVR_NUKLEAR_DIR%\nuklear_widget.c ^
%BVR_NUKLEAR_DIR%\nuklear_text.c ^
%BVR_NUKLEAR_DIR%\nuklear_image.c ^
%BVR_NUKLEAR_DIR%\nuklear_9slice.c ^
%BVR_NUKLEAR_DIR%\nuklear_button.c ^
%BVR_NUKLEAR_DIR%\nuklear_toggle.c ^
%BVR_NUKLEAR_DIR%\nuklear_selectable.c ^
%BVR_NUKLEAR_DIR%\nuklear_slider.c ^
%BVR_NUKLEAR_DIR%\nuklear_knob.c ^
%BVR_NUKLEAR_DIR%\nuklear_progress.c ^
%BVR_NUKLEAR_DIR%\nuklear_scrollbar.c ^
%BVR_NUKLEAR_DIR%\nuklear_text_editor.c ^
%BVR_NUKLEAR_DIR%\nuklear_edit.c ^
%BVR_NUKLEAR_DIR%\nuklear_property.c ^
%BVR_NUKLEAR_DIR%\nuklear_chart.c ^
%BVR_NUKLEAR_DIR%\nuklear_color_picker.c ^
%BVR_NUKLEAR_DIR%\nuklear_combo.c ^
%BVR_NUKLEAR_DIR%\nuklear_tooltip.c

set NK_EXTERN=^
%BVR_NUKLEAR_DIR%\stb_rect_pack.h ^
%BVR_NUKLEAR_DIR%\stb_truetype.h

set NK_PUBLIC=%BVR_NUKLEAR_DIR%\nuklear.h

python "%BVR_NUKLEAR_DIR%\build.py" --macro NK --intro "%NK_HEADER%" --pub "%NK_PUBLIC%" --priv1 %NK_PRIV1% --extern %NK_EXTERN% --priv2 %NK_PRIV2% --outro %NK_FOOTER% > "%BVR_INCLUDE_DIR%\nuklear.h"

if exist "%BVR_ROOT_DIR%\licenses" rmdir /s /q "%BVR_ROOT_DIR%\licenses"
if exist "%BVR_ROOT_DIR%\cmake"    rmdir /s /q "%BVR_ROOT_DIR%\cmake"

endlocal