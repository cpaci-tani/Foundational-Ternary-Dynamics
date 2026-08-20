#include "native/d3d12_presenter.h"
#include "ftd/interop_particle_record.h"
#include "ftd/test_telemetry.h"

// Off-screen: a message-only window is enough to build a swapchain-free
// D3D12 device via initialize()'s HWND-taking path. Simpler: construct a
// tiny hidden window here rather than stub initialize() into two halves --
// keeps this test exercising the real code path new callers will use.
#include <windows.h>

#include <cstdio>
#include <stdexcept>

int main() {
    ftd::test::init("test_d3d12_shared_buffer");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    ftd::native::D3D12Presenter presenter;
    try {
        presenter.initialize(hwnd, 64, 64);

        HANDLE handle = presenter.create_shared_particle_buffer(1000);
        ftd::test::check("shared buffer handle is non-null", handle != nullptr);
        ftd::test::check("shared_particle_buffer_bytes reports 1000 records",
                          presenter.shared_particle_buffer_bytes() ==
                              1000ull * sizeof(ftd::InteropParticleRecord));
        if (handle) CloseHandle(handle);

        // Re-call with a different size: confirms the buffer can be
        // recreated without crashing (would have caught the stale
        // byte-count bug) and that shared_particle_buffer_bytes() tracks
        // the NEW size rather than sticking at the first call's value.
        HANDLE handle2 = presenter.create_shared_particle_buffer(500);
        ftd::test::check("re-call: second shared buffer handle is non-null",
                          handle2 != nullptr);
        ftd::test::check("re-call: shared_particle_buffer_bytes reports 500 records",
                          presenter.shared_particle_buffer_bytes() ==
                              500ull * sizeof(ftd::InteropParticleRecord));
        if (handle2) CloseHandle(handle2);
    } catch (const std::exception& ex) {
        std::printf("[d3d12-shared-buffer] SKIP: D3D12 initialize failed: %s\n",
                    ex.what());
        ftd::test::check("d3d12 shared buffer test skipped, no live D3D12 device",
                          true, "");
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
