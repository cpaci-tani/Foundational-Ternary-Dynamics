// app/app_win32.cpp — Win32/WIC helpers (see app/app_win32.h).

#include "app/app_win32.h"

#include <shellapi.h>    // CommandLineToArgvW
#include <wincodec.h>    // WIC (PNG encode) + COM decls
#include <wrl/client.h>  // ComPtr
#include <fcntl.h>
#include <io.h>

#include <cstdio>
#include <iostream>

namespace ftd::native::app {

using Microsoft::WRL::ComPtr;
// ── PNG writer (WIC, RGBA8 rows) — mirrors test_ui_rml_smoke.cpp's save_png ──
std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

bool save_png(const std::wstring& path, const std::uint8_t* rgba, UINT w, UINT h,
              UINT row_pitch) {
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory))))
        return false;
    ComPtr<IWICStream> stream;
    if (FAILED(factory->CreateStream(&stream))) return false;
    if (FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) return false;
    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder))) return false;
    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) return false;
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    if (FAILED(encoder->CreateNewFrame(&frame, &props))) return false;
    if (FAILED(frame->Initialize(props.Get()))) return false;
    if (FAILED(frame->SetSize(w, h))) return false;
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(frame->SetPixelFormat(&fmt))) return false;
    ComPtr<IWICBitmap> source;
    if (FAILED(factory->CreateBitmapFromMemory(w, h, GUID_WICPixelFormat32bppRGBA, row_pitch,
                                               row_pitch * h, const_cast<BYTE*>(rgba), &source)))
        return false;
    if (FAILED(frame->WriteSource(source.Get(), nullptr))) return false;
    if (FAILED(frame->Commit())) return false;
    if (FAILED(encoder->Commit())) return false;
    return true;
}

// Bind stdout/stderr to the launching console so --capture-frames logging is
// visible for a WIN32-subsystem exe (copied from the retired native_desktop prototype).
bool bind_crt_to_std_handle(DWORD std_id, int crt_fd) {
    HANDLE handle = GetStdHandle(std_id);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) return false;
    const DWORD type = GetFileType(handle);
    if (type != FILE_TYPE_DISK && type != FILE_TYPE_PIPE && type != FILE_TYPE_CHAR) return false;
    const int fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_TEXT);
    if (fd < 0) return false;
    _dup2(fd, crt_fd);
    return true;
}
void attach_parent_console_if_any() {
    const bool out_bound = bind_crt_to_std_handle(STD_OUTPUT_HANDLE, 1);
    bind_crt_to_std_handle(STD_ERROR_HANDLE, 2);
    if (out_bound) { std::cout.clear(); std::cerr.clear(); return; }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stdout);
    s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

std::string wide_to_utf8(const wchar_t* wide) {
    if (!wide || wide[0] == L'\0') return {};
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (bytes <= 1) return {};
    std::string out(static_cast<size_t>(bytes - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), bytes, nullptr, nullptr);
    return out;
}
std::vector<std::string> utf8_args() {
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;
    if (!wargv) return {"native_app"};
    for (int i = 0; i < argc; ++i) args.push_back(wide_to_utf8(wargv[i]));
    LocalFree(wargv);
    if (args.empty()) args.emplace_back("native_app");
    return args;
}

}  // namespace ftd::native::app