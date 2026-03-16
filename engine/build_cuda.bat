@echo off
REM FTD CUDA build — portable VS detection via vswhere
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH (
    echo ERROR: Visual Studio not found. Install VS or set vcvars64 manually.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
cmake -S engine -B engine\build_cuda -DFTD_ENABLE_CUDA=ON -G Ninja
if errorlevel 1 exit /b 1
cmake --build engine\build_cuda --config Release
if errorlevel 1 exit /b 1
echo Build succeeded.
