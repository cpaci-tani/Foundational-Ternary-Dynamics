// Thin Win32 entry point for the native desktop application.
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>

#include "app/app_win32.h"
#include "app/run_app.h"

#include <exception>
#include <iostream>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    ftd::native::app::attach_parent_console_if_any();
    const HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    int rc = 1;
    try {
        rc = ftd::native::app::run_app(ftd::native::app::utf8_args());
    } catch (const std::exception& ex) {
        std::cerr << "native_app: " << ex.what() << "\\n";
        MessageBoxA(nullptr, ex.what(), "FTD Native App", MB_ICONERROR);
        rc = 1;
    }
    if (SUCCEEDED(co)) CoUninitialize();
    std::cout.flush();
    std::cerr.flush();
    return rc;
}
