@echo off
REM DEPRECATED (2026-07-17): the separate engine/build_cuda tree is retired,
REM and unpinned vcvars now selects MSVC 14.51+, which crashes CUDA 13.0's
REM cudafe++. The incremental build this script provided is exactly the
REM default behavior of build_native.bat (vcvars 14.44 -> cmake --build
REM --preset native-release on engine/build; no reconfigure when cached).
echo [build_cuda_fast] DEPRECATED -- delegating to engine\build_native.bat ^(incremental Release build, MSVC 14.44 pin^).
call "%~dp0build_native.bat" build %*
