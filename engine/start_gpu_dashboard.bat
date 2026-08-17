@echo off
rem Compatibility entry point. The desktop shell uses the production-speed
rem WSL2 CUDA backend, embeds the dashboard, and owns both process lifetimes.
call "%~dp0start_desktop.bat" %*
