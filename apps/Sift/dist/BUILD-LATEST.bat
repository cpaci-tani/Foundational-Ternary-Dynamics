@echo off
setlocal

title Sift - Build latest WinUI release
set "DIST_DIR=%~dp0"
for %%I in ("%DIST_DIR%..") do set "PROJECT_DIR=%%~fI"

echo.
echo  Sift WinUI 3 release builder
echo  ==============================
echo  Project: %PROJECT_DIR%
echo  Output:  %DIST_DIR%Sift\
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_DIR%\build-release.ps1"
set "BUILD_EXIT=%ERRORLEVEL%"

echo.
if not "%BUILD_EXIT%"=="0" (
  echo  BUILD FAILED with exit code %BUILD_EXIT%.
  echo  If Sift is running from the output folder, close it and try again.
) else (
  echo  BUILD COMPLETE.
  echo  Latest executable: %DIST_DIR%Sift\Sift.exe
  echo  Keep every file in that folder together.
)
echo.
pause
exit /b %BUILD_EXIT%
