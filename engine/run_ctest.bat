@echo off
REM Run FTD CTests — portable VS detection via vswhere
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
if not defined VS_PATH (
    echo ERROR: Visual Studio not found. Install VS or set vcvars64 manually.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
REM Use build_cuda if it exists, else build
if exist "%~dp0build_cuda" (
    cd /d "%~dp0build_cuda"
) else (
    cd /d "%~dp0build"
)
echo ================================================================
echo   FTD Engine Test Suite
echo   Build dir: %CD%
echo ================================================================
ctest -C Release --progress --output-on-failure --output-junit test-results.xml %*
