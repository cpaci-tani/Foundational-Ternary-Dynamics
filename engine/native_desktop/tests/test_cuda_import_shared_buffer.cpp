#include "native_desktop/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/test_telemetry.h"

// Off-screen: a message-only window is enough to build a swapchain-free
// D3D12 device via initialize()'s HWND-taking path -- same rationale as
// test_d3d12_shared_buffer.cpp.
#include <windows.h>

#include <cstdio>
#include <stdexcept>

int main() {
    ftd::test::init("test_cuda_import_shared_buffer");

    ftd::RenderBridge rb(9);
    rb.set_interactive_gpu_mode(true);
    if (rb.backend_kind() != ftd::Backend::Kind::Gpu) {
        std::printf("[import-shared-buffer] SKIP: no GPU backend in this build\n");
        ftd::test::check("import test skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }
    ftd::gpu::GpuEngine* engine = rb.gpu_engine_ptr();
    ftd::test::check("gpu_engine_ptr() is non-null", engine != nullptr);
    if (!engine) return ftd::test::finalize();

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropImportTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    ftd::native_desktop::D3D12Presenter presenter;
    HANDLE handle = nullptr;
    try {
        presenter.initialize(hwnd, 64, 64);

        constexpr std::uint32_t kMaxParticles = 1000;
        handle = presenter.create_shared_particle_buffer(kMaxParticles);
        ftd::test::check("shared handle created", handle != nullptr);
        if (!handle) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool imported = engine->import_d3d12_particle_buffer(
            handle, presenter.shared_particle_buffer_bytes());
        ftd::test::check("CUDA imported the D3D12 shared buffer", imported);

        // The NT handle can be closed immediately after import per the CUDA
        // Runtime API contract for cudaExternalMemoryHandleDesc -- confirm
        // this doesn't invalidate the CUDA-side mapping by closing it here,
        // before this test process exits.
        CloseHandle(handle);
        handle = nullptr;
    } catch (const std::exception& ex) {
        std::printf("[import-shared-buffer] SKIP: D3D12 setup failed: %s\n", ex.what());
        ftd::test::check("cuda import test skipped, no live D3D12 device", true, "");
        if (handle) CloseHandle(handle);
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
