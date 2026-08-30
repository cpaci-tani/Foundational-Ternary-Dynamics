@echo off
REM ==========================================================================
REM  WASM build wrapper for the FTD engine -- TRIPLE build
REM  (wasm32 + wasm64 + wasm32-threads).
REM
REM  Builds all three variants so the dashboard can feature-detect Memory64 and
REM  crossOriginIsolation and load the right module (see
REM  engine/web/js/bridge/wasm-bridge.js, wasm-bridge.worker.js and
REM  engine/web/docs/PLAN_WASM64_UPGRADE.md):
REM      build_wasm/    -> ftd_core.{js,wasm}     (wasm32, 2 GB heap, L~117)
REM      build_wasm64/  -> ftd_core64.{js,wasm}   (wasm64/Memory64, 8 GB, L~187)
REM      build_wasm_mt/ -> ftd_core_mt.{js,wasm}  (wasm32 + pthreads / SAB;
REM                                                off-thread Scale-0 worker)
REM  The MT variant is the SAME ftd_wasm target with -DFTD_WASM_THREADS=ON
REM  (renames output to ftd_core_mt, adds -pthread + PTHREAD_POOL_SIZE=8,
REM  exports createFTDModuleMT). It is the DEFAULT Scale-0 engine when the page
REM  is crossOriginIsolated (loaded by wasm-bridge.worker.js). Any embind binding
REM  change MUST rebuild this variant too, or the worker loads a stale core.
REM  The three ABIs cannot share libftd_core.a, hence separate build trees.
REM
REM  Usage (from project root or from engine/):
REM      engine\build_wasm.bat            REM configure + build ALL three + deploy
REM      engine\build_wasm.bat clean      REM wipe all three build trees first
REM
REM  After a successful build, all six artifacts are copied to engine/web/wasm/.
REM  NOTE: keep this file ASCII-only -- cmd parses .bat in the OEM codepage and
REM  chokes on UTF-8 box-drawing / arrow characters.
REM ==========================================================================
setlocal

REM Resolve script-relative paths so the script works from any cwd.
set "ENGINE_DIR=%~dp0"
if "%ENGINE_DIR:~-1%"=="\" set "ENGINE_DIR=%ENGINE_DIR:~0,-1%"
set "BUILD32=%ENGINE_DIR%\build_wasm"
set "BUILD64=%ENGINE_DIR%\build_wasm64"
set "BUILDMT=%ENGINE_DIR%\build_wasm_mt"
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
    if exist "%BUILDMT%" ( echo [build_wasm] Wiping %BUILDMT% & rmdir /s /q "%BUILDMT%" )
)

REM --- WASM32 (ftd_core.{js,wasm}) ---
echo === Configure WASM32 build ===
call "%EMCMAKE%" cmake -G "MinGW Makefiles" -S "%ENGINE_DIR%" -B "%BUILD32%" -DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=OFF
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm32 configure FAILED & exit /b 1 )
echo === Build WASM32 ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILD32%" --target ftd_wasm --parallel 24
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm32 build FAILED & exit /b 1 )

REM --- WASM64 / Memory64 (ftd_core64.{js,wasm}) ---
echo === Configure WASM64 build (Memory64) ===
call "%EMCMAKE%" cmake -G "MinGW Makefiles" -S "%ENGINE_DIR%" -B "%BUILD64%" -DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=ON
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm64 configure FAILED & exit /b 1 )
echo === Build WASM64 ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILD64%" --target ftd_wasm --parallel 24
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm64 build FAILED & exit /b 1 )

REM --- WASM32 + THREADS (ftd_core_mt.{js,wasm}) ---
REM Same ftd_wasm target with -DFTD_WASM_THREADS=ON (wasm32, NOT Memory64):
REM renames output to ftd_core_mt, adds -pthread (SharedArrayBuffer heap) +
REM -sPTHREAD_POOL_SIZE=8, exports createFTDModuleMT, ENVIRONMENT=node,web,worker.
REM This is the off-thread Scale-0 worker engine -- must track binding changes.
echo === Configure WASM32+threads build (ftd_core_mt) ===
call "%EMCMAKE%" cmake -G "MinGW Makefiles" -S "%ENGINE_DIR%" -B "%BUILDMT%" -DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=OFF -DFTD_WASM_THREADS=ON
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm_mt configure FAILED & exit /b 1 )
echo === Build WASM32+threads ftd_wasm target ===
call "%EMMAKE%" cmake --build "%BUILDMT%" --target ftd_wasm --parallel 24
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] wasm_mt build FAILED & exit /b 1 )

REM --- Deploy all three module pairs (directory-level atomic swap) ---
REM Stage as a SIBLING of the live directory. Staging inside engine/web/wasm
REM followed by seven individual moves still exposed mixed generations when a
REM later move failed. A same-volume directory rename makes the complete set
REM visible together; the previous directory is restored if the second rename
REM fails.
echo === Deploy to engine/web/wasm/ (staged) ===
set "STAGE_DIR=%ENGINE_DIR%\web\wasm.next"
set "BACKUP_DIR=%ENGINE_DIR%\web\wasm.previous"
if exist "%STAGE_DIR%" rmdir /s /q "%STAGE_DIR%"
if exist "%BACKUP_DIR%" rmdir /s /q "%BACKUP_DIR%"
mkdir "%STAGE_DIR%"
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] staging dir creation FAILED & exit /b 1 )

copy /y "%BUILD32%\wasm\ftd_core.js"      "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core.js FAILED & exit /b 1 )
copy /y "%BUILD32%\wasm\ftd_core.wasm"    "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core.wasm FAILED & exit /b 1 )
copy /y "%BUILD64%\wasm\ftd_core64.js"    "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core64.js FAILED & exit /b 1 )
copy /y "%BUILD64%\wasm\ftd_core64.wasm"  "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core64.wasm FAILED & exit /b 1 )
copy /y "%BUILDMT%\wasm\ftd_core_mt.js"   "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core_mt.js FAILED & exit /b 1 )
copy /y "%BUILDMT%\wasm\ftd_core_mt.wasm" "%STAGE_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] stage ftd_core_mt.wasm FAILED & exit /b 1 )

REM Smoke: every staged artifact must exist and be non-trivially sized.
for %%F in (ftd_core.js ftd_core.wasm ftd_core64.js ftd_core64.wasm ftd_core_mt.js ftd_core_mt.wasm) do (
    if not exist "%STAGE_DIR%\%%F" ( echo [build_wasm] staged %%F MISSING & exit /b 1 )
    for %%A in ("%STAGE_DIR%\%%F") do if %%~zA LSS 1024 ( echo [build_wasm] staged %%F suspiciously small: %%~zA bytes & exit /b 1 )
)

REM Stamp the deployed generation so committed-binary vs source drift is
REM auditable (deploy-pages publishes engine/web/** verbatim).
REM
REM DIRTY MARKER (2026-07-27, physics-audit M3/W9 fix): a clean sha with no
REM dirty check is indistinguishable from a dirty build that happens to
REM report a clean sha -- the failure is silent by construction. Appending
REM -dirty when the tree has uncommitted changes to the sources this build
REM actually compiled makes that distinction visible instead of assumed.
REM The tracked deployment outputs MUST be excluded: this check runs after the
REM newly built bundle has been staged, and otherwise every legitimate rebuild
REM dirties its own provenance stamp. Build trees are gitignored; the scoped
REM status below therefore covers engine inputs while excluding only the six
REM generated deploy, staging, and rollback directories under web/wasm*.
set "GIT_SHA=unknown"
for /f %%H in ('git -C "%ENGINE_DIR%" rev-parse HEAD 2^>nul') do set "GIT_SHA=%%H"
set "GIT_DIRTY="
git -C "%ENGINE_DIR%" status --porcelain --untracked-files=all -- . 2>nul | %SystemRoot%\System32\findstr.exe /v /c:"engine/web/wasm/" /c:"web/wasm/" /c:"engine/web/wasm.next/" /c:"web/wasm.next/" /c:"engine/web/wasm.previous/" /c:"web/wasm.previous/" >nul
if not ERRORLEVEL 1 set "GIT_DIRTY=1"
if defined GIT_DIRTY set "GIT_SHA=%GIT_SHA%-dirty"
set "EMCC_VERSION=unknown"
for /f "delims=" %%V in ('call "%EMSDK%\upstream\emscripten\emcc.bat" --version 2^>nul ^| findstr /b /c:"emcc "') do if "%EMCC_VERSION%"=="unknown" set "EMCC_VERSION=%%V"
set "CMAKE_VERSION=unknown"
for /f "delims=" %%V in ('cmake --version 2^>nul ^| findstr /b /c:"cmake version "') do if "%CMAKE_VERSION%"=="unknown" set "CMAKE_VERSION=%%V"

REM Deterministic, schema-versioned machine-readable identity. It deliberately
REM contains no wall clock, host path, or other per-build noise, so two clean
REM builds from one source/toolchain can compare this file byte-for-byte.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ENGINE_DIR%\tools\write_wasm_build_info.ps1" -StageDir "%STAGE_DIR%" -SourceSha "%GIT_SHA%" -EmccVersion "%EMCC_VERSION%" -CmakeVersion "%CMAKE_VERSION%"
if %ERRORLEVEL% NEQ 0 ( echo [build_wasm] build_info.json generation FAILED & exit /b 1 )

> "%STAGE_DIR%\build_info.txt" echo sha=%GIT_SHA%
>> "%STAGE_DIR%\build_info.txt" echo built=%DATE% %TIME%
>> "%STAGE_DIR%\build_info.txt" echo variants=wasm32,wasm64,wasm32-threads
>> "%STAGE_DIR%\build_info.txt" echo source_scope=engine/** excluding generated engine/web/wasm{,.next,.previous}/**
>> "%STAGE_DIR%\build_info.txt" echo emcc=%EMCC_VERSION%
>> "%STAGE_DIR%\build_info.txt" echo cmake=%CMAKE_VERSION%
>> "%STAGE_DIR%\build_info.txt" echo deterministic_manifest=build_info.json
>> "%STAGE_DIR%\build_info.txt" echo cmake_flags.wasm32=-DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=OFF -DFTD_WASM_THREADS=OFF
>> "%STAGE_DIR%\build_info.txt" echo cmake_flags.wasm64=-DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=ON -DFTD_WASM_THREADS=OFF
>> "%STAGE_DIR%\build_info.txt" echo cmake_flags.wasm32-threads=-DCMAKE_BUILD_TYPE=Release -DFTD_MEMORY64=OFF -DFTD_WASM_THREADS=ON
for %%F in (ftd_core.js ftd_core.wasm ftd_core64.js ftd_core64.wasm ftd_core_mt.js ftd_core_mt.wasm) do call :append_artifact_hash "%%F"
if ERRORLEVEL 1 ( echo [build_wasm] artifact hashing FAILED & exit /b 1 )
call :append_artifact_hash "build_info.json"
if ERRORLEVEL 1 ( echo [build_wasm] build manifest hashing FAILED & exit /b 1 )

REM All artifacts verified good -- swap the whole directory on the same volume.
set "HAD_DEPLOY="
if exist "%DEPLOY_DIR%" (
    move "%DEPLOY_DIR%" "%BACKUP_DIR%" >nul
    if ERRORLEVEL 1 ( echo [build_wasm] could not preserve live deploy directory & exit /b 1 )
    set "HAD_DEPLOY=1"
)
move "%STAGE_DIR%" "%DEPLOY_DIR%" >nul
if ERRORLEVEL 1 (
    echo [build_wasm] atomic deploy rename FAILED -- restoring previous directory
    if defined HAD_DEPLOY move "%BACKUP_DIR%" "%DEPLOY_DIR%" >nul
    exit /b 1
)
if defined HAD_DEPLOY rmdir /s /q "%BACKUP_DIR%"
echo [build_wasm] OK -- wasm32 + wasm64 + wasm32-threads modules deployed to %DEPLOY_DIR% (sha=%GIT_SHA%)

endlocal
exit /b 0

:append_artifact_hash
set "ARTIFACT_HASH="
for /f "skip=1 tokens=* delims=" %%H in ('certutil -hashfile "%STAGE_DIR%\%~1" SHA256 2^>nul') do if not defined ARTIFACT_HASH set "ARTIFACT_HASH=%%H"
set "ARTIFACT_HASH=%ARTIFACT_HASH: =%"
if not defined ARTIFACT_HASH (
    echo [build_wasm] SHA-256 failed for %~1
    exit /b 1
)
>> "%STAGE_DIR%\build_info.txt" echo sha256.%~1=%ARTIFACT_HASH%
exit /b 0
