@echo off
setlocal

title Mathematical Visualization Lab
cd /d "%~dp0apps\math-studio"

where npm >nul 2>nul
if errorlevel 1 (
  echo.
  echo Node.js and npm are required to start the studio.
  echo Install Node.js, then run this file again.
  echo.
  pause
  exit /b 1
)

if not exist "node_modules\" (
  echo Installing Math Studio dependencies...
  call npm install
  if errorlevel 1 goto :failed
)

echo Starting Mathematical Visualization Lab...
echo.
call npm run dev -- --open
exit /b %errorlevel%

:failed
echo.
echo The Math Studio dependencies could not be installed.
pause
exit /b 1
