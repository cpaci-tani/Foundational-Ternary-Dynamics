@echo off
REM =========================================================================
REM _deploy_runner.bat — bundle Qt6 DLLs + MSVC runtime next to the runner
REM =========================================================================
REM
REM Uses Qt's windeployqt to inspect ftd_test_runner.exe and copy every Qt
REM dependency it references (Qt6Core.dll, Qt6Widgets.dll, platform plugins,
REM style plugins, imageformats, SQL drivers, etc.) into the same directory.
REM
REM Then also copies the Visual C++ runtime (MSVCP140, VCRUNTIME140,
REM VCRUNTIME140_1) from the VS redist directory, so the bundle works on
REM machines that don't already have the VC++ 2015-2022 Redistributable
REM installed. Without these three DLLs, the runner would fail to launch
REM with "VCRUNTIME140.dll was not found" on a fresh Windows install.
REM
REM After this runs the runner directory is self-contained — you can zip
REM and ship it to any Windows 10+ x64 machine.
REM
REM Prereqs:
REM   - Qt 6.10.2 msvc2022_64 installed at C:\Qt\6.10.2\msvc2022_64
REM   - Visual Studio 2026 (v18) Community installed
REM   - Runner already built at engine\build_strong\tools\test_runner\Release\

set "QT_DIR=C:\Qt\6.10.2\msvc2022_64"
set "VS_REDIST=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT"
set "EXE_DIR=%~dp0build_strong\tools\test_runner\Release"
set "EXE=%EXE_DIR%\ftd_test_runner.exe"

if not exist "%QT_DIR%\bin\windeployqt.exe" (
    echo ERROR: windeployqt.exe not found at %QT_DIR%\bin\
    exit /b 1
)
if not exist "%EXE%" (
    echo ERROR: runner exe not found at %EXE%
    echo Run engine\_build_runner.bat or engine\_build_all.bat first.
    exit /b 1
)

REM Put Qt's bin on PATH so windeployqt can find its sibling tools
set "PATH=%QT_DIR%\bin;%PATH%"

echo === Running windeployqt against %EXE% ===
"%QT_DIR%\bin\windeployqt.exe" ^
    --release ^
    --no-translations ^
    --verbose 1 ^
    "%EXE%"

if %ERRORLEVEL% NEQ 0 (
    echo windeployqt FAILED with %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo.
echo === Copying MSVC runtime DLLs ===
if exist "%VS_REDIST%\vcruntime140.dll" (
    copy /Y "%VS_REDIST%\vcruntime140.dll"   "%EXE_DIR%\" >nul
    copy /Y "%VS_REDIST%\vcruntime140_1.dll" "%EXE_DIR%\" >nul
    copy /Y "%VS_REDIST%\msvcp140.dll"       "%EXE_DIR%\" >nul
    echo   vcruntime140.dll
    echo   vcruntime140_1.dll
    echo   msvcp140.dll
) else (
    echo WARNING: VS redist directory not found at %VS_REDIST%
    echo          The runner will require VC++ 2015-2022 Redistributable
    echo          to be preinstalled on target machines.
)

echo.
echo === Deploy complete ===
echo Runner directory is now self-contained:
echo   %EXE_DIR%
exit /b 0
