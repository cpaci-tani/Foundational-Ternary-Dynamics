@echo off
REM M2 fix: Use vswhere to find VS and %~dp0 for project root (no hardcoded paths)

REM Find Visual Studio installation via vswhere
for /f "tokens=*" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH (
    echo ERROR: Visual Studio not found. Install VS or set VS_PATH manually.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Navigate to project root (parent of engine/)
cd /d "%~dp0.."

echo Building CUDA (incremental)...
cmake --build engine\build_cuda --config Release
if %errorlevel% neq 0 (
    echo Build FAILED
    exit /b 1
)

echo Build SUCCESS
