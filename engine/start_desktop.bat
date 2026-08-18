@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "APP=%SCRIPT_DIR%build_desktop\FtdDesktop.exe"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"

if not exist "%APP%" (
    call "%SCRIPT_DIR%build_desktop.bat"
    if errorlevel 1 exit /b 1
)

start "FTD Desktop" "%APP%" --repo "%REPO_ROOT%" %*

endlocal
