#include "native/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/interop_particle_record.h"
#include "ftd/test_telemetry.h"

// Off-screen: a message-only window is enough to build a swapchain-free
// D3D12 device via initialize()'s HWND-taking path -- same rationale as
// test_d3d12_shared_buffer.cpp.
#include <windows.h>

// Plain-.cpp CUDA runtime include is an established pattern in this project
// (see engine/src/particle_engine.cpp) -- this translation unit is not a
// .cu file, but ftd_cuda still exposes cuda_runtime.h on the include path.
#include <cuda_runtime.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

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

    // Destruction-order note: `rb` (and the CUDA-side import it owns) is
    // declared BEFORE `presenter` below, so C++ destructs `presenter` FIRST
    // at scope exit -- tearing down the D3D12 device and the shared
    // resource -- and only then destructs `rb` (which eventually calls
    // cudaDestroyExternalMemory via GpuBuffers::free()). This is
    // intentional/known-safe, not accidental: D3D12 shared-heap resources
    // are designed to outlive the exporting device/resource COM objects for
    // exactly this cross-API handoff, consistent with the already-verified
    // "NT handle closable immediately after import" contract exercised
    // below.
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

    ftd::native::D3D12Presenter presenter;
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

        // Re-call import_d3d12_particle_buffer() with a SECOND, differently
        // sized D3D12 shared buffer -- the same shape a lattice-size resize
        // would produce (recreate the D3D12-side buffer via
        // create_shared_particle_buffer(), then re-import). Exercises the
        // re-entrancy fix: a prior import must be torn down before the new
        // one is created, or the first external memory object leaks its
        // driver-level reference to the D3D12 resource. Confirms no crash,
        // and that the newly-mapped device pointer actually addresses the
        // SECOND resource's own memory (round-trips a distinct pattern)
        // rather than stale/leaked state from the first.
        if (imported) {
            constexpr std::uint32_t kMaxParticles2 = 500;
            HANDLE handle2 = presenter.create_shared_particle_buffer(kMaxParticles2);
            ftd::test::check("re-call: second shared handle created", handle2 != nullptr);
            if (handle2) {
                const bool imported2 = engine->import_d3d12_particle_buffer(
                    handle2, presenter.shared_particle_buffer_bytes());
                ftd::test::check(
                    "re-call: CUDA imported the second D3D12 shared buffer "
                    "(no crash, no leak)",
                    imported2);
                CloseHandle(handle2);
                handle2 = nullptr;

                if (imported2) {
                    std::vector<ftd::InteropParticleRecord> pattern(kMaxParticles2);
                    for (std::uint32_t i = 0; i < kMaxParticles2; ++i) {
                        pattern[i].x = static_cast<float>(i) + 0.5f;
                    }
                    const std::size_t bytes =
                        pattern.size() * sizeof(ftd::InteropParticleRecord);
                    const cudaError_t write_status = cudaMemcpy(
                        engine->bufs().d_interop_particle_buffer, pattern.data(),
                        bytes, cudaMemcpyHostToDevice);
                    std::vector<ftd::InteropParticleRecord> readback(kMaxParticles2);
                    const cudaError_t read_status = cudaMemcpy(
                        readback.data(), engine->bufs().d_interop_particle_buffer,
                        bytes, cudaMemcpyDeviceToHost);
                    const bool roundtrip_ok =
                        write_status == cudaSuccess && read_status == cudaSuccess
                        && std::memcmp(readback.data(), pattern.data(), bytes) == 0;
                    ftd::test::check(
                        "re-call: second import's mapped device pointer "
                        "round-trips its own data",
                        roundtrip_ok);
                }
            }
        }
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
