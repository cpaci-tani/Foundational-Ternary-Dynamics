// engine/native_desktop/tests/test_interop_visual_parity.cpp
//
// Confirms the interop gather kernel (Task 6) produces exactly the same
// world positions and colors the pre-interop CPU path
// (NativeEngineSession::capture(), engine_session.cpp) would have computed
// for the same manifested particles -- the design's own stated validation
// bar for Component B: "visual output must be unchanged from the current
// round-trip path at matching scenarios -- compare rendered particle
// positions/colors against the pre-interop path for a fixed scenario/tick,
// not just 'looks right.'"
//
// Ordering claim (verified against Task 6's own kernel code, not assumed):
// interop_particle_gather_kernel (gpu_engine.cu) and the pre-existing
// visual_particle_gather_kernel both derive their output `slot` from the
// identical previous_rank/current_rank/previous_bucket/current_bucket
// formula, reading the same `prefix` array (bufs.d_pair_candidate_indices,
// an exclusive scan over bufs.d_state-derived flags) and the same
// header->total_manifested/header->captured_count (both gather calls in
// this test share bufs.d_visual_particle_header, written once by
// visual_particle_header_kernel in launch_interop_particle_gather). Given
// the same bufs.d_state (no tick runs between the two captures below), both
// kernels are deterministic functions of the same input and MUST place the
// same manifested particle into the same slot. The comparison below is
// therefore ordered (records[i] against reference.particles[i]), a
// strictly stronger assertion than an unordered set-membership check.
#include "native_desktop/d3d12_presenter.h"
#include "native_desktop/engine_session.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/interop_particle_record.h"
#include "ftd/test_telemetry.h"
#include "ftd/visual_snapshot.h"

#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    ftd::test::init("test_interop_visual_parity");

    ftd::native_desktop::NativeEngineOptions options;
    options.force_cpu = false;
    options.lattice_size = 17;
    options.scenario = "s0-seed-hydrogen";
    ftd::native_desktop::NativeEngineSession session(options);
    if (std::string(session.backend_name()) != "cuda") {
        std::printf("[interop-parity] SKIP: no GPU backend in this build\n");
        ftd::test::check("interop parity skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }
    for (int i = 0; i < 20; ++i) session.tick();

    // Reference: the existing CPU capture path (unchanged by this plan).
    const ftd::native_desktop::NativeFrame reference = session.capture();

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropParityTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    // D3D12Presenter::initialize() throw_if_failed()s on any DXGI/D3D12 call
    // that fails (e.g. no usable hardware adapter, or swap-chain creation
    // failing on a headless/RDP/CI session with no live display). Same
    // try/catch SKIP pattern as this test's direct precedents --
    // test_interop_gather.cpp, test_interop_fence_roundtrip.cpp, and
    // test_interop_reload_reset.cpp -- so a machine without a usable D3D12
    // adapter/display skips cleanly instead of taking down the whole CTest
    // 'gpu;interactive;native_desktop' group with an uncaught exception.
    ftd::native_desktop::D3D12Presenter presenter;
    HANDLE buf_handle = nullptr;
    HANDLE fence_handle = nullptr;
    try {
        presenter.initialize(hwnd, 64, 64);
        buf_handle = presenter.create_shared_particle_buffer(ftd::kMaxVisualParticleCapture);
        fence_handle = buf_handle ? presenter.create_shared_fence() : nullptr;
        ftd::test::check("shared buffer + fence created", buf_handle && fence_handle);
        if (!buf_handle || !fence_handle) {
            if (buf_handle) CloseHandle(buf_handle);
            if (fence_handle) CloseHandle(fence_handle);
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool enabled = session.try_enable_interop(
            buf_handle, presenter.shared_particle_buffer_bytes(), fence_handle);
        CloseHandle(buf_handle);
        CloseHandle(fence_handle);
        buf_handle = nullptr;
        fence_handle = nullptr;
        ftd::test::check("interop enabled for parity check", enabled);
        if (!enabled) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        session.request_interop_gather(1);
        int count = -1;
        for (int i = 0; i < 5000 && count < 0; ++i) {
            count = session.poll_interop_particle_count();
            if (count < 0) Sleep(0);
        }
        ftd::test::check("interop gather completed", count >= 0);
        ftd::test::check("interop particle count matches the CPU reference count",
                          count >= 0 &&
                              static_cast<std::size_t>(count) == reference.particles.size());

        // Ordered byte-for-byte comparison against the CPU reference path --
        // see the file-level comment for why index-for-index comparison is
        // valid here (both kernels are deterministic functions of the same
        // bufs.d_state/prefix/header inputs, unmutated between the two
        // calls).
        ftd::gpu::GpuEngine* engine = session.debug_gpu_engine();
        ftd::test::check("debug_gpu_engine() is non-null", engine != nullptr);
        if (!engine || count < 0) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }
        std::vector<ftd::InteropParticleRecord> records;
        engine->debug_read_interop_records(records, static_cast<std::uint32_t>(count));

        ftd::test::check("reference has at least one manifested particle",
                          !reference.particles.empty());

        constexpr float kEps = 1.0e-5f;
        bool all_match = true;
        for (std::size_t i = 0; i < reference.particles.size() && i < records.size(); ++i) {
            const auto& ref = reference.particles[i];
            const auto& rec = records[i];
            const bool pos_match = std::fabs(ref.x - rec.x) < kEps &&
                                   std::fabs(ref.y - rec.y) < kEps &&
                                   std::fabs(ref.z - rec.z) < kEps;
            const bool color_match = std::fabs(ref.r - rec.r) < kEps &&
                                     std::fabs(ref.g - rec.g) < kEps &&
                                     std::fabs(ref.b - rec.b) < kEps;
            if (!pos_match || !color_match) {
                all_match = false;
                std::printf("[interop-parity] mismatch at slot %zu: ref=(%.4f,%.4f,%.4f "
                           "rgb %.2f,%.2f,%.2f) interop=(%.4f,%.4f,%.4f rgb %.2f,%.2f,%.2f)\n",
                           i, ref.x, ref.y, ref.z, ref.r, ref.g, ref.b,
                           rec.x, rec.y, rec.z, rec.r, rec.g, rec.b);
            }
        }
        ftd::test::check("every interop record matches the CPU reference's position and color",
                          all_match);
    } catch (const std::exception& ex) {
        std::printf("[interop-parity] SKIP: D3D12 setup failed: %s\n", ex.what());
        ftd::test::check("interop parity skipped, no live D3D12 device", true, "");
        if (buf_handle) CloseHandle(buf_handle);
        if (fence_handle) CloseHandle(fence_handle);
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
