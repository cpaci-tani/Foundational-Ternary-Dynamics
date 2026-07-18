@echo off
REM DEPRECATED (2026-07-17): the separate engine/build_cuda tree is retired.
REM CUDA is ON by default in the canonical engine/build tree, and VS 18's
REM default MSVC toolset (14.51+) crashes CUDA 13.0's cudafe++ -- builds must
REM run inside vcvarsall x64 -vcvars_ver=14.44. build_native.bat does both
REM (vswhere -> vcvars 14.44 -> cmake --preset native -> Ninja Multi-Config).
echo [build_cuda] DEPRECATED -- delegating to engine\build_native.bat ^(engine/build, CUDA ON, MSVC 14.44 pin^).
call "%~dp0build_native.bat" %*
