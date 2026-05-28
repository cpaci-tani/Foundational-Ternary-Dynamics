@echo off
REM Full engine build: core, all tests, runner, CUDA. Used for end-to-end
REM verification after the FTD Test Bench merge.
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH exit /b 1
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set "CUDA_EXT=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\extras\visual_studio_integration\MSBuildExtensions"
set "VS_CUSTOM=%VS_PATH%\MSBuild\Microsoft\VC\v180\BuildCustomizations"
if not exist "%VS_CUSTOM%\CUDA 13.0.targets" (
    copy "%CUDA_EXT%\*" "%VS_CUSTOM%\" >nul 2>&1
)

echo === Configure full engine build ===
cmake -S "%~dp0." -B "%~dp0build_strong" -DFTD_ENABLE_CUDA=ON -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64
if %ERRORLEVEL% NEQ 0 (
    echo CMake configure FAILED
    exit /b 1
)

echo.
echo === Build ALL_BUILD (everything) ===
cmake --build "%~dp0build_strong" --config Release -- /m:16 /verbosity:minimal
set BUILD_RC=%ERRORLEVEL%
if %BUILD_RC% NEQ 0 (
    echo.
    echo === BUILD FAILED with code %BUILD_RC% ===
    exit /b %BUILD_RC%
)

echo.
echo === Build complete ===
exit /b 0
