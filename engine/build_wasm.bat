@echo off
REM ==========================================================================
REM  WASM build wrapper for the FTD engine.
REM
REM  Replaces the manual emcmake / emmake invocation documented in CLAUDE.md
REM  ("WASM" line under "C++ Engine"). Mirrors the pattern of the other
REM  `_build_*.bat` helpers so a fresh contributor doesn't have to read prose
REM  to rebuild the WASM module after a C++ edit.
REM
REM  Usage (from project root or from engine/):
REM      engine\build_wasm.bat            REM full configure + build + deploy
REM      engine\build_wasm.bat clean      REM wipe build_wasm/ first
REM
REM  After a successful build, ftd_core.{js,wasm} are copied to engine/web/
REM  wasm/ so the dashboard picks them up immediately.
REM ==========================================================================
setlocal

REM Resolve script-relative paths so the script works from any cwd.
set "ENGINE_DIR=%~dp0"
set "PROJECT_DIR=%ENGINE_DIR%.."
set "BUILD_DIR=%ENGINE_DIR%build_wasm"
set "DEPLOY_DIR=%ENGINE_DIR%web\wasm"

REM Locate emsdk. Prefer EMSDK environment variable; fall back to /c/emsdk.
if not defined EMSDK (
    if exist "C:\emsdk\upstream\emscripten\emcmake.bat" (
        set "EMSDK=C:\emsdk"
    ) else (
        echo [build_wasm] EMSDK not set and C:\emsdk not found.
        echo            Install Emscripten and either set EMSDK or
        echo            place it at C:\emsdk.
        exit /b 1
    )
)
set "EMCMAKE=%EMSDK%\upstream\emscripten\emcmake.bat"
set "EMMAKE=%EMSDK%\upstream\emscripten\emmake.bat"

if /i "%~1"=="clean" (
    if exist "%BUILD_DIR%" (
        echo [build_wasm] Wiping %BUILD_DIR%
        rmdir /s /q "%BUILD_DIR%"
    )
)

echo === Configure WASM build ===
call "%EMCMAKE%" cmake -S "%ENGINE_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo [build_wasm] emcmake configure FAILED
    exit /b 1
)

echo === Build ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILD_DIR%" --target ftd_wasm
if %ERRORLEVEL% NEQ 0 (
    echo [build_wasm] emmake build FAILED
    exit /b 1
)

echo === Deploy to engine/web/wasm/ ===
copy /y "%BUILD_DIR%\wasm\ftd_core.js"   "%DEPLOY_DIR%\" >nul
copy /y "%BUILD_DIR%\wasm\ftd_core.wasm" "%DEPLOY_DIR%\" >nul
echo [build_wasm] OK — ftd_core.{js,wasm} deployed to %DEPLOY_DIR%

endlocal
