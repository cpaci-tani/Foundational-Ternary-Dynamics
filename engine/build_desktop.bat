@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "PROJECT=%SCRIPT_DIR%desktop\FtdDesktop.csproj"
set "OUTPUT=%SCRIPT_DIR%build_desktop"

echo ================================================================
echo   FTD Desktop - Windows native interface
echo ================================================================
echo.
echo Publishing the WPF/WebView2 shell for win-x64...

dotnet publish "%PROJECT%" -c Release -r win-x64 --self-contained false -o "%OUTPUT%" --nologo
if errorlevel 1 exit /b %errorlevel%
if not exist "%OUTPUT%\FtdDesktop.exe" (
    echo ERROR: publish completed without producing FtdDesktop.exe.
    exit /b 1
)

echo.
echo Build complete:
echo   %OUTPUT%\FtdDesktop.exe
echo.
echo Each launch incrementally builds engine/build_wsl/ws_server with 32-way
echo parallelism, starts a safe CUDA bootstrap lattice, memory-preflights the
echo requested size, and verifies native CUDA plus WebGL. Pass
echo --skip-engine-build to start_desktop.bat when the binary is already fresh.

endlocal
