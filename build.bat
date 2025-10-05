@echo off
setlocal enabledelayedexpansion

:: Configuration
set "BVR_GENERATOR=MinGW Makefiles"
set "BVR_CC=gcc"
set "BVR_CXX=g++"
set "BVR_BUILD_DIR=%CD%\build"
set "BVR_EXTERNAL_MODULES=SDL PortAudio Zlib Lpng json-c"

:: Clear command
if "%1"=="-clear" (
    echo Cleaning...

    rmdir /s /q "%CD%\bin"
    rmdir /s /q "%CD%\lib"
    rmdir /s /q "%CD%\licenses"
    rmdir /s /q "%CD%\cmake"

    mkdir "%CD%\bin"
    mkdir "%CD%\lib"
)

:: Loop over external modules
for %%M in (%BVR_EXTERNAL_MODULES%) do (
    set "MOD=%%M"
    set "MODULE_PATH=%CD%\extern\!MOD!"

    if exist "!MODULE_PATH!" (
        echo !MODULE_PATH! found!
        set "BVR_MODULE_FLAGS="

        if "!MOD!"=="PortAudio" (
            set "BVR_MODULE_FLAGS=-DPA_BUILD_SHARED_LIBS=ON -DPA_USE_WASAPI=OFF -DPA_USE_WDMKS=OFF -DPA_USE_WDMKS_DEVICE_INFO=OFF"
        )

        del /f /q "%BVR_BUILD_DIR%\!MOD!\CMakeCache.txt" 2>nul

        cmake "!MODULE_PATH!\CmakeLists.txt" -G "!BVR_GENERATOR!" -B "%BVR_BUILD_DIR%\!MOD!" -D CMAKE_INSTALL_PREFIX="%CD%" !BVR_MODULE_FLAGS! -DCMAKE_C_COMPILER=!BVR_CC! -DCMAKE_CXX_COMPILER=!BVR_CXX!
        cmake --build "%BVR_BUILD_DIR%\!MOD!" --target install
    ) else (
        echo !MODULE_PATH! not found!
    )
)

:: Auto clear-up unused SDL folders
rmdir /s /q "%CD%\licenses"
rmdir /s /q "%CD%\cmake"

endlocal