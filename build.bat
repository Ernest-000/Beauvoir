@echo off
setlocal enabledelayedexpansion

:: Configuration
set "BVR_GENERATOR=MinGW Makefiles"
set "BVR_CC=gcc"
set "BVR_CXX=c++"

set "BVR_BUILD_DIR=%CD%\build"
set "BVR_INCLUDE_DIR=%CD%\include"
set "BVR_NUKLEAR_DIR=.\extern\Nuklear\src\"

set "BVR_EXTERNAL_MODULES=SDL Zlib Lpng json-c"

:: FLAGS
set "BVR_CLEAR=false"
set "BVR_SKIP_BIN=false"
set "BVR_SKIP_GIT_SYNC=false"
for %%A in (%*) do (
    if "%%A"=="--clear" (set "BVR_CLEAR=true")
    if "%%A"=="--skip-binaries" (set "BVR_SKIP_BIN=true")
    if "%%A"=="--skip-sync" (set "BVR_SKIP_GIT_SYNC=true")
)

if %BVR_SKIP_GIT_SYNC%=="true" (
    git submodule deinit -f .
    git submodule update --init
)

:: Clear command
if %BVR_CLEAR%=="true" (
    echo Cleaning...

    rmdir /s /q "%CD%\bin"
    rmdir /s /q "%CD%\lib"
    rmdir /s /q "%CD%\licenses"
    rmdir /s /q "%CD%\cmake"

    mkdir "%CD%\bin"
    mkdir "%CD%\lib"
)

:: Loop over external modules
if %BVR_SKIP_BIN%=="true" (
    for %%M in (%BVR_EXTERNAL_MODULES%) do (
        set "MOD=%%M"
        set "MODULE_PATH=%CD%\extern\!MOD!"

        if exist "!MODULE_PATH!" (
            echo !MODULE_PATH! found!
            set "BVR_MODULE_FLAGS="

            del /f /q "%BVR_BUILD_DIR%\!MOD!\CMakeCache.txt" 2>nul

            cmake "!MODULE_PATH!\CmakeLists.txt" -G "!BVR_GENERATOR!" -B "%BVR_BUILD_DIR%\!MOD!" -D CMAKE_INSTALL_PREFIX="%CD%" !BVR_MODULE_FLAGS! -DCMAKE_C_COMPILER=!BVR_CC!
            cmake --build "%BVR_BUILD_DIR%\!MOD!" --target install
        ) else (
            echo !MODULE_PATH! not found!
        )
    )
)

:: Nuklear
set "NK_HEADER=%BVR_NUKLEAR_DIR%HEADER.md"
set "NK_FOOTER=%BVR_NUKLEAR_DIR%LICENSE %BVR_NUKLEAR_DIR%CHANGELOG %BVR_NUKLEAR_DIR%CREDITS"
set "NK_PRIV1=%BVR_NUKLEAR_DIR%nuklear_internal.h %BVR_NUKLEAR_DIR%nuklear_math.c %BVR_NUKLEAR_DIR%nuklear_util.c %BVR_NUKLEAR_DIR%nuklear_color.c %BVR_NUKLEAR_DIR%nuklear_utf8.c %BVR_NUKLEAR_DIR%nuklear_buffer.c %BVR_NUKLEAR_DIR%nuklear_string.c %BVR_NUKLEAR_DIR%nuklear_draw.c %BVR_NUKLEAR_DIR%nuklear_vertex.c"
set "NK_PRIV2=%BVR_NUKLEAR_DIR%nuklear_font.c %BVR_NUKLEAR_DIR%nuklear_input.c %BVR_NUKLEAR_DIR%nuklear_style.c %BVR_NUKLEAR_DIR%nuklear_context.c %BVR_NUKLEAR_DIR%nuklear_pool.c %BVR_NUKLEAR_DIR%nuklear_page_element.c %BVR_NUKLEAR_DIR%nuklear_table.c %BVR_NUKLEAR_DIR%nuklear_panel.c %BVR_NUKLEAR_DIR%nuklear_window.c %BVR_NUKLEAR_DIR%nuklear_popup.c %BVR_NUKLEAR_DIR%nuklear_contextual.c %BVR_NUKLEAR_DIR%nuklear_menu.c %BVR_NUKLEAR_DIR%nuklear_layout.c %BVR_NUKLEAR_DIR%nuklear_tree.c %BVR_NUKLEAR_DIR%nuklear_group.c %BVR_NUKLEAR_DIR%nuklear_list_view.c %BVR_NUKLEAR_DIR%nuklear_widget.c %BVR_NUKLEAR_DIR%nuklear_text.c %BVR_NUKLEAR_DIR%nuklear_image.c %BVR_NUKLEAR_DIR%nuklear_9slice.c %BVR_NUKLEAR_DIR%nuklear_button.c %BVR_NUKLEAR_DIR%nuklear_toggle.c %BVR_NUKLEAR_DIR%nuklear_selectable.c %BVR_NUKLEAR_DIR%nuklear_slider.c %BVR_NUKLEAR_DIR%nuklear_knob.c %BVR_NUKLEAR_DIR%nuklear_progress.c %BVR_NUKLEAR_DIR%nuklear_scrollbar.c %BVR_NUKLEAR_DIR%nuklear_text_editor.c %BVR_NUKLEAR_DIR%nuklear_edit.c %BVR_NUKLEAR_DIR%nuklear_property.c %BVR_NUKLEAR_DIR%nuklear_chart.c %BVR_NUKLEAR_DIR%nuklear_color_picker.c %BVR_NUKLEAR_DIR%nuklear_combo.c %BVR_NUKLEAR_DIR%nuklear_tooltip.c"
set "NK_EXTERN=%BVR_NUKLEAR_DIR%stb_rect_pack.h %BVR_NUKLEAR_DIR%stb_truetype.h"
set "NK_PUBLIC=%BVR_NUKLEAR_DIR%nuklear.h"

python3 %BVR_NUKLEAR_DIR%build.py --macro NK --intro "%NK_HEADER%" --pub "%NK_PUBLIC%" --priv1 "%NK_PRIV1%" --extern "%NK_EXTERN%" --priv2 "%NK_PRIV2%" --outro "%NK_FOOTER%" > %BVR_INCLUDE_DIR%\nuklear.h

:: Auto clear-up unused SDL folders
if exist "%CD%\licenses" (rmdir /s /q "%CD%\licenses")
if exist "%CD%\cmake" (rmdir /s /q "%CD%\cmake")

endlocal