@echo off
setlocal

title Sift - Rebuild this release folder
set "EXE_DIR=%~dp0"
for %%I in ("%EXE_DIR%..\..") do set "PROJECT_DIR=%%~fI"
set "BUILD_SCRIPT=%PROJECT_DIR%\build-release.ps1"

echo.
echo  Sift rebuild
echo  ================
echo  Project: %PROJECT_DIR%
echo  Output:  %EXE_DIR%
echo.
echo  Close Sift.exe first if it is running from this folder.
echo.

if not exist "%BUILD_SCRIPT%" (
  echo  ERROR: Cannot find build-release.ps1 at:
  echo         %BUILD_SCRIPT%
  echo  This rebuild script must stay beside Sift.exe inside the
  echo  published folder under apps\Sift\dist\.
  echo.
  pause
  exit /b 1
)

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%BUILD_SCRIPT%"
set "BUILD_EXIT=%ERRORLEVEL%"

echo.
if not "%BUILD_EXIT%"=="0" (
  echo  REBUILD FAILED with exit code %BUILD_EXIT%.
  echo  If Sift is running from this folder, close it and try again.
) else (
  echo  REBUILD COMPLETE.
  echo  Executable: %EXE_DIR%Sift.exe
  echo  Keep every file in this folder together.
)
echo.
pause
exit /b %BUILD_EXIT%
