@echo off
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cmake --build "%~dp0build_strong" --config Release --target ftd_wilson_loops -- /m:16
exit /b %ERRORLEVEL%
