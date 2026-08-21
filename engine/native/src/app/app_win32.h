#pragma once
//
// app/app_win32.h — Win32/WIC helpers: UTF-8 command-line parsing, a WIC PNG
// readback writer, and parent-console attach for the WIN32-subsystem exe. Split
// out of app/main.cpp (behavior-neutral).
//
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ftd::native::app {

std::wstring widen(const std::string& s);
bool save_png(const std::wstring& path, const std::uint8_t* rgba, UINT w, UINT h,
              UINT row_pitch);
void attach_parent_console_if_any();
std::vector<std::string> utf8_args();

}  // namespace ftd::native::app