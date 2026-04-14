@echo off
REM =========================================================================
REM _verify_final.bat — final verification for the FTD Test Bench plan
REM =========================================================================
REM
REM Builds the telemetry selftest + Qt runner and runs the selftest in
REM both human-readable and NDJSON modes to confirm the core pipeline
REM is intact after all phases of the plan have landed.
REM

for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH exit /b 1
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set "CUDA_EXT=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\extras\visual_studio_integration\MSBuildExtensions"
set "VS_CUSTOM=%VS_PATH%\MSBuild\Microsoft\VC\v180\BuildCustomizations"
if not exist "%VS_CUSTOM%\CUDA 13.0.targets" (
    copy "%CUDA_EXT%\*" "%VS_CUSTOM%\" >nul 2>&1
)

echo === Configure final verification build ===
cmake -S "%~dp0." -B "%~dp0build_final" -DFTD_ENABLE_CUDA=ON -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64
if %ERRORLEVEL% NEQ 0 exit /b 1

echo.
echo === Build ftd_test_runner + test_telemetry_selftest ===
cmake --build "%~dp0build_final" --config Release --target ftd_test_runner --target test_telemetry_selftest -- /m:16
if %ERRORLEVEL% NEQ 0 exit /b 1

echo.
echo === Run selftest (human-readable mode) ===
"%~dp0build_final\Release\test_telemetry_selftest.exe"
if %ERRORLEVEL% NEQ 0 (
    echo SELFTEST FAILED IN HUMAN MODE
    exit /b 1
)

echo.
echo === Run selftest (NDJSON mode) ===
set FTD_TEST_TELEMETRY=1
"%~dp0build_final\Release\test_telemetry_selftest.exe" > "%~dp0build_final\selftest_ndjson.txt"
if %ERRORLEVEL% NEQ 0 (
    echo SELFTEST FAILED IN NDJSON MODE
    exit /b 1
)
set FTD_TEST_TELEMETRY=

echo.
echo === ctest --show-only=json-v1 ===
ctest --test-dir "%~dp0build_final" --show-only=json-v1 -C Release > "%~dp0build_final\ctest_show.json" 2>&1

echo.
echo === Final verification complete ===
echo Runner exe:         %~dp0build_final\tools\test_runner\Release\ftd_test_runner.exe
echo Selftest exe:       %~dp0build_final\Release\test_telemetry_selftest.exe
echo NDJSON capture:     %~dp0build_final\selftest_ndjson.txt
echo CTest enumeration:  %~dp0build_final\ctest_show.json
exit /b 0
