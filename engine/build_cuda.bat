@echo off
REM Build FTD with CUDA enabled — uses VS Developer environment + Ninja
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH (
    echo ERROR: Visual Studio not found.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >/dev/null 2>&1

REM Copy CUDA MSBuild extensions if missing
set "CUDA_EXT=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\extras\visual_studio_integration\MSBuildExtensions"
set "VS_CUSTOM=%VS_PATH%\MSBuild\Microsoft\VC\v180\BuildCustomizations"
if not exist "%VS_CUSTOM%\CUDA 13.0.targets" (
    echo Installing CUDA MSBuild extensions...
    copy "%CUDA_EXT%\*" "%VS_CUSTOM%\" >/dev/null 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo WARNING: Could not copy CUDA extensions. Trying Ninja without MSBuild...
    )
)

echo === Configuring CUDA build with Visual Studio generator ===
cmake -S "%~dp0." -B "%~dp0build_cuda" -DFTD_ENABLE_CUDA=ON -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo CMake configure failed
    exit /b 1
)

echo === Building (serial; NVCC 13 + CMake 4 + MSBuild don't parallelise cleanly) ===
cmake --build "%~dp0build_cuda" --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    exit /b 1
)

echo === Build complete ===
