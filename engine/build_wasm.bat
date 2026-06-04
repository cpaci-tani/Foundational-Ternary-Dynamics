@echo off
REM ==========================================================================
REM  WASM build wrapper for the FTD engine -- DUAL build (wasm32 + wasm64).
REM
REM  Builds BOTH memory models so the dashboard can feature-detect Memory64
REM  and load the right module (see engine/web/js/bridge/wasm-bridge.js and
REM  engine/web/docs/PLAN_WASM64_UPGRADE.md):
REM      build_wasm/   -> ftd_core.{js,wasm}    (wasm32, 2 GB heap, L~117)
REM      build_wasm64/ -> ftd_core64.{js,wasm}  (wasm64/Memory64, 8 GB, L~187)
REM  The two ABIs cannot share libftd_core.a, hence separate build trees.
REM
REM  Usage (from project root or from engine/):
REM      engine\build_wasm.bat            REM configure + build BOTH + deploy
REM      engine\build_wasm.bat clean      REM wipe both build trees first
REM
REM  After a successful build, all four artifacts are copied to engine/web/wasm/.
REM  NOTE: keep this file ASCII-only -- cmd parses .bat in the OEM codepage and
REM  chokes on UTF-8 box-drawing / arrow characters.
REM ==========================================================================
setlocal

REM Resolve script-relative paths so the script works from any cwd.
set "ENGINE_DIR=%~dp0"
if "%ENGINE_DIR:~-1%"=="\" set "ENGINE_DIR=%ENGINE_DIR:~0,-1%"
set "BUILD32=%ENGINE_DIR%\build_wasm"
set "BUILD64=%ENGINE_DIR%\build_wasm64"
set "DEPLOY_DIR=%ENGINE_DIR%\web\wasm"

REM Locate emsdk. Prefer EMSDK environment variable; fall back to C:\emsdk.
if not defined EMSDK (
    if exist "C:\emsdk\upstream\emscripten\emcmake.bat" (
        set "EMSDK=C:\emsdk"
    ) else (
        echo [build_wasm] EMSDK not set and C:\emsdk not found.
        echo            Install Emscripten and either set EMSDK or place it at C:\emsdk.
        exit /b 1
    )
)
set "EMCMAKE=%EMSDK%\upstream\emscripten\emcmake.bat"
set "EMMAKE=%EMSDK%\upstream\emscripten\emmake.bat"

if /i "%~1"=="clean" (
    if exist "%BUILD32%" ( echo [build_wasm] Wiping %BUILD32% & rmdir /s /q "%BUILD32%" )
    if exist "%BUILD64%" ( echo [build_wasm] Wiping %BUILD64% & rmdir /s /q "%BUILD64%" )
)

REM --- WASM32 (ftd_core.{js,wasm}) ---
echo === Configure WASM32 build ===
call "%EMCMAKE%" cmake -S "%ENGINE_DIR%" -B "%BUILD32%" -DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=OFF
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm32 configure FAILED & exit /b 1 )
echo === Build WASM32 ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILD32%" --target ftd_wasm
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm32 build FAILED & exit /b 1 )

REM --- WASM64 / Memory64 (ftd_core64.{js,wasm}) ---
echo === Configure WASM64 build (Memory64) ===
call "%EMCMAKE%" cmake -S "%ENGINE_DIR%" -B "%BUILD64%" -DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=ON
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm64 configure FAILED & exit /b 1 )
echo === Build WASM64 ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILD64%" --target ftd_wasm
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm64 build FAILED & exit /b 1 )

REM --- Deploy both module pairs ---
echo === Deploy to engine/web/wasm/ ===
copy /y "%BUILD32%\wasm\ftd_core.js"     "%DEPLOY_DIR%\" >nul
copy /y "%BUILD32%\wasm\ftd_core.wasm"   "%DEPLOY_DIR%\" >nul
copy /y "%BUILD64%\wasm\ftd_core64.js"   "%DEPLOY_DIR%\" >nul
copy /y "%BUILD64%\wasm\ftd_core64.wasm" "%DEPLOY_DIR%\" >nul
echo [build_wasm] OK -- wasm32 + wasm64 modules deployed to %DEPLOY_DIR%

endlocal
