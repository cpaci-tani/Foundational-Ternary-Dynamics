@echo off
setlocal
set "EXE=%~dp0build\native_desktop\Release\ftd_native_desktop.exe"
if not exist "%EXE%" set "EXE=%~dp0build\Release\ftd_native_desktop.exe"
if not exist "%EXE%" (
  echo Building ftd_native_desktop...
  call "%~dp0build_native.bat" build --target ftd_native_desktop --parallel 32
  if errorlevel 1 exit /b 1
)
if not exist "%EXE%" set "EXE=%~dp0build\native_desktop\Release\ftd_native_desktop.exe"
if "%~1"=="" (
  echo Starting FTD native desktop  --cpu --lattice 32 --scenario s0-seed-hydrogen
  "%EXE%" --cpu --lattice 32 --scenario s0-seed-hydrogen
) else (
  "%EXE%" %*
)
endlocal
