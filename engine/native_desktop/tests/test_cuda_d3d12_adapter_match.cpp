#include "native_desktop/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/test_telemetry.h"

#include <cstdio>
#include <cstring>

int main() {
    ftd::test::init("test_cuda_d3d12_adapter_match");

    LUID d3d12_luid{};
    bool is_hardware = false;
    const bool found = ftd::native_desktop::D3D12Presenter::select_hardware_adapter(
        &d3d12_luid, &is_hardware);
    ftd::test::check("D3D12 hardware adapter found", found && is_hardware);

    // L=9: an arbitrary small odd lattice, just big enough to construct a
    // live GPU backend (mirrors L_PARITY in test_gauge_gpu_parity.cpp).
    ftd::RenderBridge rb(9);
    rb.set_interactive_gpu_mode(true);
    if (rb.backend_kind() != ftd::Backend::Kind::Gpu) {
        std::printf("[adapter-match] SKIP: no GPU backend in this build\n");
        ftd::test::check("adapter match skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }

    ftd::gpu::GpuEngine* engine = rb.gpu_engine_ptr();
    ftd::test::check("gpu_engine_ptr() is non-null on GPU backend", engine != nullptr);
    if (!engine) return ftd::test::finalize();

    char cuda_luid[8] = {};
    const bool has_luid = engine->device_luid(cuda_luid);
    ftd::test::check("CUDA device reports a LUID", has_luid);
    if (!has_luid) return ftd::test::finalize();

    // LUID is a { DWORD LowPart; LONG HighPart; } pair; cudaDeviceProp::luid
    // stores the same 8 bytes in the same layout. A plain byte-for-byte
    // memcmp is endian-agnostic by construction — compare directly.
    const bool matches = std::memcmp(&d3d12_luid, cuda_luid, sizeof(cuda_luid)) == 0;
    ftd::test::check("D3D12's selected hardware adapter matches CUDA's device",
                      matches,
                      "CUDA and D3D12 picked different physical GPUs -- interop "
                      "would silently target the wrong device");

    return ftd::test::finalize();
}
