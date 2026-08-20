#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ftd::native {

bool enable_per_monitor_v2_dpi();
bool apply_dpi_suggested_rect(HWND hwnd, LPARAM lparam);

}  // namespace ftd::native
