@echo off
echo ================================================================
echo   FTD GPU Dashboard Launcher
echo ================================================================
echo.
echo Starting GPU engine server (ws_server.exe on port 9100)...
echo Starting web server (python on port 8080)...
echo.
echo Open http://localhost:8080 in your browser.
echo Badge should show: Native Engine / GPU
echo.
echo Press Ctrl+C to stop both servers.
echo ================================================================

:: Start web server in background
start "FTD Web Server" /min cmd /c "cd /d %~dp0web && python -m http.server 8080"

:: Small delay to let web server start
timeout /t 2 /nobreak > nul

:: Start GPU engine server in foreground (so you see the output)
cd /d %~dp0
build\Release\ws_server.exe 64 9100
