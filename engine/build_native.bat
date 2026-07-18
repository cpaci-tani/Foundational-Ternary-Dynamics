@echo off
REM ==========================================================================
REM  Native (Windows) build wrapper for the FTD engine -- the canonical entry
REM  point for the engine/build tree (CPU + CUDA, Ninja Multi-Config).
REM
REM  WHY THIS EXISTS -- MSVC TOOLSET PIN (2026-07-17):
REM    VS 18's default MSVC toolset (14.51.36231+) ships headers that crash
REM    CUDA 13.0's cudafe++ with an ACCESS_VIOLATION on every .cu file. The
REM    side-by-side 14.44 toolset (14.44.35207) is CUDA-supported, so every
REM    configure/build of engine/build MUST run inside
REM        vcvarsall.bat x64 -vcvars_ver=14.44
REM    Generator-level pins do NOT work under the "Visual Studio 18 2026"
REM    MSBuild generator (CMAKE_VS_GLOBALS VCToolsVersion -> TRK0005 CL.exe
REM    not found; -T version=14.44.* -> invalid version spec). The working
REM    recipe is Ninja Multi-Config configured inside the pinned vcvars
REM    environment -- exactly what this wrapper + engine/CMakePresets.json do.
REM
REM  Usage (from project root or from engine/):
REM      engine\build_native.bat                  REM configure-if-needed + build Release
REM      engine\build_native.bat build [args]     REM same; args forwarded to cmake --build
REM      engine\build_native.bat configure [args] REM (re)configure via cmake --preset native
REM      engine\build_native.bat golden           REM serial golden battery (7 tests:
REM                                               REM   ctest -C Release -R "golden|gauge_links")
REM      engine\build_native.bat test [args]      REM ctest -C Release (default -j 32)
REM      engine\build_native.bat clean            REM wipe engine/build, then full rebuild
REM      engine\build_native.bat shell <cmd...>   REM run any command inside the pinned env
REM
REM  Pin override: set FTD_VCVARS_VER before calling (default 14.44).
REM  NOTE: quote any forwarded arg containing '=' (cmd splits on it), e.g.
REM      engine\build_native.bat configure "-DFTD_ENABLE_PCH=OFF"
REM  Keep this file ASCII-only -- cmd parses .bat in the OEM codepage and
REM  chokes on UTF-8 box-drawing / arrow characters.
REM ==========================================================================
setlocal

REM Resolve script-relative paths so the script works from any cwd.
set "ENGINE_DIR=%~dp0"
if "%ENGINE_DIR:~-1%"=="\" set "ENGINE_DIR=%ENGINE_DIR:~0,-1%"
set "BUILD_DIR=%ENGINE_DIR%\build"
if not defined FTD_VCVARS_VER set "FTD_VCVARS_VER=14.44"

REM --- Locate Visual Studio via vswhere (portable, per project convention) ---
set "VS_INSTALLER_DIR=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
if not exist "%VS_INSTALLER_DIR%\vswhere.exe" (
    echo [build_native] ERROR: vswhere.exe not found at "%VS_INSTALLER_DIR%".
    exit /b 1
)
for /f "delims=" %%i in ('"%VS_INSTALLER_DIR%\vswhere.exe" -latest -products * -property installationPath 2^>nul') do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo [build_native] ERROR: Visual Studio not found via vswhere.
    exit /b 1
)
REM vcvarsall's internal scripts invoke bare 'vswhere' -- put the Installer dir
REM on PATH so they resolve it (otherwise a cosmetic 'vswhere.exe is not
REM recognized' error prints before the banner).
set "PATH=%VS_INSTALLER_DIR%;%PATH%"

REM --- Verify the pinned toolset is installed side-by-side ---
dir /b "%VS_PATH%\VC\Tools\MSVC\%FTD_VCVARS_VER%*" >nul 2>&1
if errorlevel 1 (
    echo [build_native] ERROR: MSVC toolset %FTD_VCVARS_VER%.x not found under:
    echo     %VS_PATH%\VC\Tools\MSVC
    echo Install it: VS Installer ^> Individual components ^> MSVC v145 build tools ^(14.44^).
    echo Installed toolsets:
    dir /b "%VS_PATH%\VC\Tools\MSVC"
    exit /b 1
)

REM --- Enter the pinned environment ---
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=%FTD_VCVARS_VER%
if errorlevel 1 (
    echo [build_native] ERROR: vcvarsall x64 -vcvars_ver=%FTD_VCVARS_VER% failed.
    exit /b 1
)
echo %VCToolsVersion% | findstr /b /c:"%FTD_VCVARS_VER%" >nul
if errorlevel 1 (
    echo [build_native] ERROR: expected VCToolsVersion %FTD_VCVARS_VER%.x, got "%VCToolsVersion%".
    echo     CUDA 13.0 cudafe++ crashes under MSVC 14.51+ -- refusing to build.
    exit /b 1
)
echo [build_native] VCToolsVersion=%VCToolsVersion% ^(pin %FTD_VCVARS_VER% OK^)

REM --- Guard: an existing cache must match the pinned recipe's generator ---
if exist "%BUILD_DIR%\CMakeCache.txt" (
    findstr /c:"CMAKE_GENERATOR:INTERNAL=Ninja Multi-Config" "%BUILD_DIR%\CMakeCache.txt" >nul
    if errorlevel 1 (
        echo [build_native] ERROR: engine\build was configured with a different generator.
        echo     The pinned recipe is Ninja Multi-Config. Delete engine\build and re-run.
        exit /b 1
    )
)

REM --- Parse subcommand + collect remaining args (quotes preserved) ---
set "CMD=%~1"
if not defined CMD set "CMD=build"
shift
set "ARGS="
:collect
if "%~1"=="" goto collected
set ARGS=%ARGS% %1
shift
goto collect
:collected

REM cmake/ctest --preset resolve engine/CMakePresets.json relative to cwd.
cd /d "%ENGINE_DIR%"

if /i "%CMD%"=="clean" (
    if exist "%BUILD_DIR%" ( echo [build_native] Wiping %BUILD_DIR% & rmdir /s /q "%BUILD_DIR%" )
    set "CMD=build"
)
if /i "%CMD%"=="configure" goto do_configure
if /i "%CMD%"=="build" goto do_build
if /i "%CMD%"=="golden" goto do_golden
if /i "%CMD%"=="test" goto do_test
if /i "%CMD%"=="shell" goto do_shell
echo [build_native] Unknown command "%CMD%" ^(expected: build / configure / golden / test / clean / shell^)
exit /b 1

:do_configure
echo === Configure engine/build ^(preset native: Ninja Multi-Config, MSVC %VCToolsVersion%^) ===
cmake --preset native %ARGS%
exit /b %ERRORLEVEL%

:do_build
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo === Configure engine/build ^(preset native: Ninja Multi-Config, MSVC %VCToolsVersion%^) ===
    cmake --preset native
    if errorlevel 1 ( echo [build_native] configure FAILED & exit /b 1 )
)
echo === Build Release ^(preset native-release, -j 32^) ===
cmake --build --preset native-release %ARGS%
if errorlevel 1 ( echo [build_native] build FAILED & exit /b 1 )
echo [build_native] OK -- engine\build Release up to date ^(MSVC %VCToolsVersion%^)
exit /b 0

:do_golden
echo === Golden merge-gate battery ^(serial: -R "golden|gauge_links"^) ===
ctest --preset golden
exit /b %ERRORLEVEL%

:do_test
if "%ARGS%"=="" set "ARGS=-j 32 --output-on-failure"
cd /d "%BUILD_DIR%"
ctest -C Release %ARGS%
exit /b %ERRORLEVEL%

:do_shell
if "%ARGS%"=="" (
    echo [build_native] shell: supply a command to run inside the pinned env, e.g.
    echo     engine\build_native.bat shell cmake --build engine/build --config Release --parallel 32
    exit /b 1
)
cd /d "%ENGINE_DIR%\.."
%ARGS%
exit /b %ERRORLEVEL%
