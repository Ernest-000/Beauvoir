@echo off
setlocal EnableDelayedExpansion

:: Avaible parameters :
::   --clear: clear all caches and building files
::   --force: force building
::   --skip-binaries: skip cmake building pass
::   --skip-git: skip git sync pass
::   --skip-nk: skip nuklear header generation

:: CMake building system
set "BVR_GENERATOR=Unix Makefiles"
set "BVR_CC=gcc"
set "BVR_CXX=g++"
set "BVR_USE_CCACHE=false"

:: path variables
set "BVR_ROOT_DIR=%~dp0"
if "%BVR_ROOT_DIR:~-1%"=="\" set "BVR_ROOT_DIR=%BVR_ROOT_DIR:~0,-1%"
set "BVR_BUILD_DIR=%TEMP%\beauvoir-build"
set "BVR_INCLUDE_DIR=%BVR_ROOT_DIR%\include"
set "BVR_NUKLEAR_DIR=%BVR_ROOT_DIR%\extern\Nuklear\src"

:: Third-party lib list
set "BVR_EXTERNAL_MODULES=Zlib json-c"

:: flags
set "BVR_CLEAR=false"
set "BVR_FORCE=false"
set "BVR_SKIP_BIN=false"
set "BVR_SKIP_SYNC=false"
set "BVR_SKIP_NK=false"

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="--clear"         set "BVR_CLEAR=true"
if /I "%~1"=="--force"         set "BVR_FORCE=true"
if /I "%~1"=="--skip-binaries" set "BVR_SKIP_BIN=true"
if /I "%~1"=="--skip-git"      set "BVR_SKIP_SYNC=true"
if /I "%~1"=="--skip-nk"       set "BVR_SKIP_NK=true"
shift
goto parse_args
:args_done

:: =========================== MAIN ===========================

if "%BVR_SKIP_SYNC%"=="false" (
    call :bvr_git_sync
    if errorlevel 1 echo Submodules sync had errors
)

if "%BVR_CLEAR%"=="true" (
    call :bvr_clear
    if errorlevel 1 echo clearing stage had errors
)

if "%BVR_SKIP_BIN%"=="false" (
    if not exist "%BVR_BUILD_DIR%" mkdir "%BVR_BUILD_DIR%"

    set "BVR_MODULE_FAILED=false"

    :: loop through all modules
    for %%M in (%BVR_EXTERNAL_MODULES%) do (
        call :bvr_build_module "%%M" > "%BVR_BUILD_DIR%\%%M.log" 2>&1
        if errorlevel 1 (
            echo Error: build failed for %%M 1>&2
            set "BVR_MODULE_FAILED=true"
        )
    )

    :: print logs when failed
    for %%M in (%BVR_EXTERNAL_MODULES%) do (
        echo ----- %%M:%BVR_BUILD_DIR%\%%M.log -----
        type "%BVR_BUILD_DIR%\%%M.log"
    )

    if "!BVR_MODULE_FAILED!"=="true" (
        echo One or more external modules failed to build. 1>&2
        exit /b 1
    )
)

if "%BVR_SKIP_NK%"=="false" (
    call :bvr_merge_nuklear
    if errorlevel 1 echo Nuklear header gen had errors
)

call :bvr_post_build

endlocal
exit /b 0

:: ======================= FUNCTIONS =======================

:: clear all cmake and user's cache
:bvr_clear
echo Cleaning...

if exist "%BVR_ROOT_DIR%\bin"       rmdir /s /q "%BVR_ROOT_DIR%\bin"
if exist "%BVR_ROOT_DIR%\lib"       rmdir /s /q "%BVR_ROOT_DIR%\lib"
if exist "%BVR_ROOT_DIR%\licenses"  rmdir /s /q "%BVR_ROOT_DIR%\licenses"
if exist "%BVR_ROOT_DIR%\cmake"     rmdir /s /q "%BVR_ROOT_DIR%\cmake"
if exist "%BVR_BUILD_DIR%"          rmdir /s /q "%BVR_BUILD_DIR%"

mkdir "%BVR_ROOT_DIR%\bin"
mkdir "%BVR_ROOT_DIR%\lib"

exit /b 0

:: clear post build folders
:bvr_post_build
if exist "%BVR_BUILD_DIR%"         rmdir /s /q "%BVR_BUILD_DIR%"
if exist "%BVR_ROOT_DIR%\licenses" rmdir /s /q "%BVR_ROOT_DIR%\licenses"
if exist "%BVR_ROOT_DIR%\cmake"    rmdir /s /q "%BVR_ROOT_DIR%\cmake"
exit /b 0

:: sync and update git submodules
:bvr_git_sync
setlocal EnableDelayedExpansion

:: maximum retries
set "submodules_max_retries=3"

echo Syncing git submodules...

:: check for .gitmodules
if not exist "%BVR_ROOT_DIR%\.gitmodules" (
    echo No .gitmodules file found, skipping submodule sync.
    endlocal
    exit /b 0
)

:: keep remote URLs
git submodule sync --recursive

:: safe deinit
git submodule deinit --all -f

:: tries multiple times to update init all modules
set "attempt=1"
set "success=false"

:sync_retry_loop
if !attempt! gtr %submodules_max_retries% goto sync_retry_done
echo Initializing submodules (attempt !attempt!/%submodules_max_retries%)...
git submodule update --init --recursive
if !errorlevel! equ 0 (
    set "success=true"
    goto sync_retry_done
)
echo Submodule init/update failed, retrying... 1>&2
set /a attempt+=1
timeout /t 2 /nobreak >nul
goto sync_retry_loop
:sync_retry_done

:: print error if needed
if "!success!"=="false" (
    echo Error: could not initialize submodules after %submodules_max_retries% attempts. 1>&2
    endlocal
    exit /b 1
)

set "failed_modules="

:: try to get latest release
for /f "usebackq tokens=1,2" %%K in (`git config -f "%BVR_ROOT_DIR%\.gitmodules" --get-regexp "^submodule\..*\.path$"`) do (
    call :bvr_sync_one_submodule "%%K" "%%L"
    if errorlevel 1 set "failed_modules=!failed_modules! %%K"
)

:: print failed modules
if not "!failed_modules!"=="" (
    echo Warning: the following submodules failed to update: !failed_modules! 1>&2
)

endlocal
exit /b 0

:: helper: checkout latest release tag for a single submodule (mirrors the
:: bash subshell "(cd "${module_dir}" ... )" block)
:bvr_sync_one_submodule
setlocal EnableDelayedExpansion
set "key=%~1"
set "path=%~2"
set "name=!key:submodule.=!"
set "name=!name:.path=!"
set "module_dir=%BVR_ROOT_DIR%\!path!"

echo Updating submodule: !name! ^(!path!^)

pushd "!module_dir!" 2>nul
if errorlevel 1 (
    endlocal
    exit /b 1
)

git fetch --tags --force >nul 2>&1

set "latest_tag="
for /f "delims=" %%T in ('git tag --list --sort=-v:refname') do (
    if not defined latest_tag set "latest_tag=%%T"
)

:: find latest with the tag
if defined latest_tag (
    echo Checking out latest release tag: !latest_tag!
    git checkout --detach "!latest_tag!"
) else (
    echo No tags found, falling back to remote branch head
    set "default_branch="
    for /f "delims=" %%B in ('git symbolic-ref --short HEAD 2^>nul') do set "default_branch=%%B"
    if not defined default_branch set "default_branch=main"
    git checkout "!default_branch!"
    git pull --ff-only
)

set "rc=!errorlevel!"
popd
endlocal & exit /b %rc%

:: build (or skip if up to date) one external module, then stage its
:: libs/headers into lib/ and include/
:bvr_build_module
setlocal EnableDelayedExpansion

:: max parallel jobs (passed to cmake --parallel)
set "BVR_JOBS=%NUMBER_OF_PROCESSORS%"
if "%BVR_JOBS%"=="" set "BVR_JOBS=4"

:: mod variables
set "MOD=%~1"
set "MOD_PATH=%BVR_ROOT_DIR%\extern\%MOD%"
set "MOD_BUILD_DIR=%BVR_BUILD_DIR%\%MOD%"
set "MOD_TMP=%MOD_BUILD_DIR%\install"
set "MOD_HASH_FILE=%MOD_BUILD_DIR%\.bvr-mod-hash"

:: check for missing module
if not exist "%MOD_PATH%" (
    echo Missing module: %MOD_PATH%
    endlocal
    exit /b 1
)

if not exist "%MOD_BUILD_DIR%" mkdir "%MOD_BUILD_DIR%"

:: get version hash
set "SRC_HASH="
for /f "delims=" %%H in ('git -C "%MOD_PATH%" rev-parse HEAD 2^>nul') do set "SRC_HASH=%%H"
if not defined SRC_HASH set "SRC_HASH=nogit"

:: skip building if everything is up to date
set "BVR_UP_TO_DATE=false"
if exist "%MOD_HASH_FILE%" if exist "%MOD_TMP%\lib" if "%BVR_FORCE%"=="false" (
    set /p STORED_HASH=<"%MOD_HASH_FILE%"
    if "!STORED_HASH!"=="!SRC_HASH!" set "BVR_UP_TO_DATE=true"
)

if "!BVR_UP_TO_DATE!"=="true" (
    echo Module %MOD% unchanged ^(!SRC_HASH:~0,8!^), skipping rebuild.
) else (
    echo Building module %MOD%

    :: generic flags
    set "BVR_MODULE_FLAGS=-DBUILD_SHARED_LIBS=OFF -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON"

    :: module specific flags
    if /I "%MOD%"=="Zlib"   set "BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DZLIB_BUILD_EXAMPLES=OFF"
    if /I "%MOD%"=="json-c" set "BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DBUILD_SHARED_LIBS=ON"

    :: enable ccache
    if "%BVR_USE_CCACHE%"=="true" set "BVR_MODULE_FLAGS=!BVR_MODULE_FLAGS! -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"

    :: use --fresh for the the first build of the module
    set "FRESH_FLAG=--fresh"
    if exist "%MOD_BUILD_DIR%\CMakeCache.txt" set "FRESH_FLAG="

    cmake !FRESH_FLAG! ^
        -S "%MOD_PATH%" ^
        -G "%BVR_GENERATOR%" ^
        -B "%MOD_BUILD_DIR%" ^
        -DCMAKE_INSTALL_PREFIX="%MOD_TMP%" ^
        -DCMAKE_C_COMPILER="%BVR_CC%" ^
        -DCMAKE_BUILD_TYPE=Release ^
        !BVR_MODULE_FLAGS!
    if errorlevel 1 (
        endlocal
        exit /b 1
    )

    cmake --build "%MOD_BUILD_DIR%" --target install --parallel !BVR_JOBS!
    if errorlevel 1 (
        endlocal
        exit /b 1
    )

    >"%MOD_HASH_FILE%" echo %SRC_HASH%
)

:: check for lib files
if exist "%MOD_TMP%\lib" (
    for %%E in (dll lib a) do (
        for %%F in ("%MOD_TMP%\lib\*.%%E") do (
            if exist "%%F" copy /y "%%F" "%BVR_ROOT_DIR%\lib\" >nul
        )
    )
)

:: copy include fils
if exist "%MOD_TMP%\include" (
    xcopy /s /e /y /i "%MOD_TMP%\include\*" "%BVR_INCLUDE_DIR%\" >nul
)

endlocal
exit /b 0

:: generate the single-header nuklear.h from the split sources
:bvr_merge_nuklear
setlocal EnableDelayedExpansion

set "NK_HEADER=%BVR_NUKLEAR_DIR%\HEADER.md"
set "NK_FOOTER=%BVR_NUKLEAR_DIR%\LICENSE %BVR_NUKLEAR_DIR%\CHANGELOG %BVR_NUKLEAR_DIR%\CREDITS"
set "NK_PRIV1=%BVR_NUKLEAR_DIR%\nuklear_internal.h %BVR_NUKLEAR_DIR%\nuklear_math.c %BVR_NUKLEAR_DIR%\nuklear_util.c %BVR_NUKLEAR_DIR%\nuklear_color.c %BVR_NUKLEAR_DIR%\nuklear_utf8.c %BVR_NUKLEAR_DIR%\nuklear_buffer.c %BVR_NUKLEAR_DIR%\nuklear_string.c %BVR_NUKLEAR_DIR%\nuklear_draw.c %BVR_NUKLEAR_DIR%\nuklear_vertex.c"
set "NK_PRIV2=%BVR_NUKLEAR_DIR%\nuklear_font.c %BVR_NUKLEAR_DIR%\nuklear_input.c %BVR_NUKLEAR_DIR%\nuklear_style.c %BVR_NUKLEAR_DIR%\nuklear_context.c %BVR_NUKLEAR_DIR%\nuklear_pool.c %BVR_NUKLEAR_DIR%\nuklear_page_element.c %BVR_NUKLEAR_DIR%\nuklear_table.c %BVR_NUKLEAR_DIR%\nuklear_panel.c %BVR_NUKLEAR_DIR%\nuklear_window.c %BVR_NUKLEAR_DIR%\nuklear_popup.c %BVR_NUKLEAR_DIR%\nuklear_contextual.c %BVR_NUKLEAR_DIR%\nuklear_menu.c %BVR_NUKLEAR_DIR%\nuklear_layout.c %BVR_NUKLEAR_DIR%\nuklear_tree.c %BVR_NUKLEAR_DIR%\nuklear_group.c %BVR_NUKLEAR_DIR%\nuklear_list_view.c %BVR_NUKLEAR_DIR%\nuklear_widget.c %BVR_NUKLEAR_DIR%\nuklear_text.c %BVR_NUKLEAR_DIR%\nuklear_image.c %BVR_NUKLEAR_DIR%\nuklear_9slice.c %BVR_NUKLEAR_DIR%\nuklear_button.c %BVR_NUKLEAR_DIR%\nuklear_toggle.c %BVR_NUKLEAR_DIR%\nuklear_selectable.c %BVR_NUKLEAR_DIR%\nuklear_slider.c %BVR_NUKLEAR_DIR%\nuklear_knob.c %BVR_NUKLEAR_DIR%\nuklear_progress.c %BVR_NUKLEAR_DIR%\nuklear_scrollbar.c %BVR_NUKLEAR_DIR%\nuklear_text_editor.c %BVR_NUKLEAR_DIR%\nuklear_edit.c %BVR_NUKLEAR_DIR%\nuklear_property.c %BVR_NUKLEAR_DIR%\nuklear_chart.c %BVR_NUKLEAR_DIR%\nuklear_color_picker.c %BVR_NUKLEAR_DIR%\nuklear_combo.c %BVR_NUKLEAR_DIR%\nuklear_tooltip.c"
set "NK_EXTERN=%BVR_NUKLEAR_DIR%\stb_rect_pack.h %BVR_NUKLEAR_DIR%\stb_truetype.h"
set "NK_PUBLIC=%BVR_NUKLEAR_DIR%\nuklear.h"

set "NK_OUT=%BVR_INCLUDE_DIR%\nuklear.h"
set "NK_STAMP=%BVR_BUILD_DIR%\.nuklear"

if not exist "%BVR_BUILD_DIR%" mkdir "%BVR_BUILD_DIR%"

:: check for versioning
set "NK_HASH="
for /f "delims=" %%H in ('git -C "%BVR_ROOT_DIR%\extern\Nuklear" rev-parse HEAD 2^>nul') do set "NK_HASH=%%H"
if not defined NK_HASH set "NK_HASH=nogit"

:: skip when up to date
:: NOTE: mirrors the original's check against the output file (the bash
:: source used "${OUT}" instead of "${NK_OUT}" here - fixed below to the
:: correct variable, see note at end of the response)
set "NK_UP_TO_DATE=false"
if exist "%NK_STAMP%" if exist "%NK_OUT%" (
    set /p NK_STORED_HASH=<"%NK_STAMP%"
    if "!NK_STORED_HASH!"=="!NK_HASH!" set "NK_UP_TO_DATE=true"
)

if "!NK_UP_TO_DATE!"=="true" (
    echo nuklear.h up to date ^(!NK_HASH:~0,8!^), skipping regeneration.
    endlocal
    exit /b 0
)

echo Generating nuklear.h...

set "NK_TMP=%BVR_INCLUDE_DIR%\.nuklear.h.%RANDOM%.tmp"

python "%BVR_NUKLEAR_DIR%\build.py" ^
    --macro NK ^
    --intro  "%NK_HEADER%" ^
    --pub    "%NK_PUBLIC%" ^
    --priv1  "%NK_PRIV1%" ^
    --extern "%NK_EXTERN%" ^
    --priv2  "%NK_PRIV2%" ^
    --outro  "%NK_FOOTER%" ^
    > "%NK_TMP%"

if errorlevel 1 (
    del /f /q "%NK_TMP%" 2>nul
    echo Error: nuklear.h generation failed. 1>&2
    endlocal
    exit /b 1
) else (
    move /y "%NK_TMP%" "%NK_OUT%" >nul
    >"%NK_STAMP%" echo %NK_HASH%
)

endlocal
exit /b 0