@echo off
REM =========================================================================
REM _build_runner.bat — build the FTD Test Bench (ftd_test_runner)
REM =========================================================================
REM
REM Configures and builds the Qt6 test runner in engine/build_runner.
REM Detects Visual Studio via vswhere, sources vcvars64, and points CMake
REM at the expected Qt 6.10.2 msvc2022_64 install at CMAKE_PREFIX_PATH.
REM
REM Usage:  engine\_build_runner.bat
REM
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH (
    echo ERROR: Visual Studio not found.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

REM Copy CUDA MSBuild extensions if missing (parity with build_cuda.bat).
set "CUDA_EXT=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\extras\visual_studio_integration\MSBuildExtensions"
set "VS_CUSTOM=%VS_PATH%\MSBuild\Microsoft\VC\v180\BuildCustomizations"
if not exist "%VS_CUSTOM%\CUDA 13.0.targets" (
    echo Installing CUDA MSBuild extensions...
    copy "%CUDA_EXT%\*" "%VS_CUSTOM%\" >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo WARNING: Could not copy CUDA extensions. Continuing anyway.
    )
)

echo === Configuring test runner build ===
cmake -S "%~dp0." -B "%~dp0build_runner" ^
    -DFTD_ENABLE_CUDA=ON ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64
if %ERRORLEVEL% NEQ 0 (
    echo CMake configure failed
    exit /b 1
)

echo === Building ftd_test_runner ===
cmake --build "%~dp0build_runner" --config Release --target ftd_test_runner -- /m:16
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    exit /b 1
)

echo === Build complete ===
echo Executable: %~dp0build_runner\tools\test_runner\Release\ftd_test_runner.exe
