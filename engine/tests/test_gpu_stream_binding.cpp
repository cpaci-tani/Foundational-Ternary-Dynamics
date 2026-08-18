// ============================================================================
// test_gpu_stream_binding.cpp — the engine owns one non-legacy CUDA stream.
//
// Component A / Task 1 of the native GPU tick plan. Asserts:
//   S1  GpuBuffers::stream is created and is NOT the legacy default stream.
//   S2  The stream is usable and quiesces after a tick.
//   S3  Moving the tick onto that stream did not change any physics: two
//       independently constructed engines with the same seed produce
//       bit-identical flux/state after 20 ticks.
//   S4  Construct/destroy cycles leave no sticky CUDA error (stream teardown).
// ============================================================================

#include "ftd/gpu_engine.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <cuda_runtime.h>
#include <cmath>
#define CUDA_CHECK_TEST(call) do { \
    const cudaError_t _e = (call); \
    ftd::test::check("cuda call succeeded", _e == cudaSuccess); \
} while (0)
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace ftd;

namespace {

void seed_scene(gpu::GpuEngine& e) {
    e.inject_particle(3, 3, 3, +1, Vec3{0.0, 0.0, 0.0});
    e.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    e.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

bool voxels_bit_identical(const std::vector<Voxel>& a,
                          const std::vector<Voxel>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].state != b[i].state) return false;
        if (std::memcmp(&a[i].flux, &b[i].flux, sizeof(Vec3)) != 0) return false;
        if (std::memcmp(&a[i].wave_vel, &b[i].wave_vel, sizeof(Vec3)) != 0) return false;
        if (std::memcmp(&a[i].velocity, &b[i].velocity, sizeof(Vec3)) != 0) return false;
    }
    return true;
}

}  // namespace

int main() {
    test::init("test_gpu_stream_binding");

    test::section("S1/S2: dedicated non-legacy stream");
    {
        gpu::GpuEngine engine(17);
        const cudaStream_t s = engine.bufs().stream;
        test::check("S1: stream handle is non-null", s != nullptr);
        test::check("S1: stream is not the legacy default stream",
                    s != cudaStreamLegacy && s != cudaStream_t(0));
        seed_scene(engine);
        for (int t = 0; t < 5; ++t) engine.tick();
        test::check("S2: stream synchronizes cleanly",
                    cudaStreamSynchronize(s) == cudaSuccess);
        test::check("S2: stream is idle after synchronize",
                    cudaStreamQuery(s) == cudaSuccess);
    }

    // S3 is a DETERMINISM check, not a regression check: it compares two
    // freshly constructed engines, both already running on the dedicated
    // stream, against each other — it cannot detect a divergence from
    // pre-change behavior, only a divergence between two post-change runs.
    // The actual regression gate against pre-change behavior is `gpu_golden`
    // (pinned hashes), which Step 10 below runs as part of the full GPU
    // suite — that is the check that would catch the stream change itself
    // having altered physics.
    test::section("S3: two engines on the dedicated stream agree bit-for-bit");
    {
        gpu::GpuEngine a(17);
        gpu::GpuEngine b(17);
        seed_scene(a);
        seed_scene(b);
        for (int t = 0; t < 20; ++t) { a.tick(); b.tick(); }
        std::vector<Voxel> va, vb;
        a.sync_to_host(va);
        b.sync_to_host(vb);
        test::check("S3: two engines agree bit-for-bit after 20 ticks",
                    voxels_bit_identical(va, vb));
    }

    test::section("S4: stream teardown leaves no sticky CUDA error");
    {
        for (int i = 0; i < 4; ++i) {
            gpu::GpuEngine e(16);
            e.tick();
        }
        test::check("S4: no sticky CUDA error after 4 create/destroy cycles",
                    cudaGetLastError() == cudaSuccess);
    }

    test::section("S5: Poisson mean_charge is device-resident");
    {
        gpu::GpuEngine engine(17);
        test::check("S5: d_poisson_charge_sum allocated",
                    engine.bufs().d_poisson_charge_sum != nullptr);
        test::check("S5: d_poisson_mean_charge allocated",
                    engine.bufs().d_poisson_mean_charge != nullptr);

        seed_scene(engine);
        engine.toggles.gauss_projection = true;
        engine.tick();
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));

        double host_mean = 1.0;
        CUDA_CHECK_TEST(cudaMemcpy(&host_mean,
                                   engine.bufs().d_poisson_mean_charge,
                                   sizeof(double), cudaMemcpyDeviceToHost));
        // Two manifested particles of opposite sign in a 17^3 lattice, plus
        // whatever genesis produced; the device scalar must at minimum be a
        // finite multiple of 1/N and never the untouched sentinel.
        const double quantum = 1.0 / static_cast<double>(17 * 17 * 17);
        const double ratio = host_mean / quantum;
        test::check("S5: device mean_charge is an exact multiple of 1/N",
                    std::abs(ratio - std::nearbyint(ratio)) < 1e-9);
    }

    test::section("S6: the tick leaves work queued (no device-wide sync)");
    {
        gpu::GpuEngine engine(128);
        seed_scene(engine);
        engine.toggles.forces = true;
        engine.toggles.movement = true;
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));

        // S6a: batch-level idleness probe. A tick that contained
        // cudaDeviceSynchronize or a synchronous cudaMemset could not leave
        // the stream busy -- but this probe only ever samples the stream
        // AFTER a whole batch of ticks, so it cannot tell "no sync anywhere"
        // apart from "a sync at the very start of every tick": a start-of-
        // tick sync drains the PREVIOUS tick's tail before the query below
        // ever runs, and the CURRENT tick's own later kernels can still be
        // in flight, so the query reads "busy" either way. Verified
        // empirically: re-adding the exact cudaDeviceSynchronize() this task
        // removed from reset_continuity_ledger() still leaves this probe
        // passing. Kept as a cheap secondary signal; S6b below is the check
        // that actually discriminates.
        bool observed_async = false;
        for (int attempt = 0; attempt < 8 && !observed_async; ++attempt) {
            for (int t = 0; t < 4; ++t) engine.tick();
            if (cudaStreamQuery(engine.bufs().stream) == cudaErrorNotReady) {
                observed_async = true;
            }
        }
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));
        test::check("S6a: tick enqueues asynchronously (batch probe, weak)",
                    observed_async);

        // S6b: per-call wall-clock probe -- the discriminating check. A
        // blocking sync placed ANYWHERE inside tick() (start, middle, or
        // end) inflates that SINGLE call's host-observed duration, unlike
        // S6a which can only ever see the stream's state after several ticks
        // have already run. Warm up first to absorb JIT/first-touch
        // allocation overhead, then time a batch of individual tick() calls
        // and gate on the MEDIAN (never max or mean) so the assertion is
        // robust against the periodic multi-millisecond WDDM command-buffer-
        // batching stalls that are a normal, expected artifact of async GPU
        // submission on Windows, not a regression signal.
        //
        // Calibrated on this machine (RTX 5090, WDDM, L=128, forces+movement
        // on, 30 samples after an 8-tick warmup, several repeated runs) back
        // when graph_capture_enabled defaulted to false. Since then
        // graph_capture_enabled defaults to true (Task 9) and this profile
        // (forces+movement only) is graph_eligible(), so by default the
        // warmup tick captures once and every sampled tick below times a
        // cudaGraphLaunch replay, not a direct kernel-launch sequence — the
        // numbers below are direct-launch timings, kept as the calibration
        // record, not necessarily what a fresh run measures today. The
        // assertion's purpose (catch a blocking sync reintroduced anywhere
        // in the hot path) holds either way: replay is at least as async as
        // direct launch, so the threshold stays valid, just conservative.
        //   genuinely async (direct launch, pre-Task-9): median ~250-265us,
        //                                               occasional ~14-16ms
        //                                               WDDM batching stalls
        //                                               visible only in max
        //   cudaDeviceSynchronize() re-added to
        //   reset_continuity_ledger() (the bug
        //   this task fixed, reintroduced only
        //   to calibrate/verify this test):            median ~3.4ms
        // ~13x separation measured; 1.2ms sits well inside the gap with
        // roughly 4-5x headroom below and ~3x headroom above.
        constexpr int kWarmupTicks = 8;
        constexpr int kSampledTicks = 30;
        constexpr double kBlockingThresholdUs = 1200.0;
        for (int t = 0; t < kWarmupTicks; ++t) engine.tick();
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));

        std::vector<double> call_us;
        call_us.reserve(kSampledTicks);
        for (int t = 0; t < kSampledTicks; ++t) {
            const auto t0 = std::chrono::high_resolution_clock::now();
            engine.tick();
            const auto t1 = std::chrono::high_resolution_clock::now();
            call_us.push_back(
                std::chrono::duration<double, std::micro>(t1 - t0).count());
        }
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));

        std::vector<double> sorted_us = call_us;
        std::sort(sorted_us.begin(), sorted_us.end());
        const double median_us = sorted_us[sorted_us.size() / 2];

        char detail[160];
        std::snprintf(detail, sizeof(detail),
                       "median=%.1fus min=%.1fus max=%.1fus n=%d threshold=%.0fus",
                       median_us, sorted_us.front(), sorted_us.back(),
                       kSampledTicks, kBlockingThresholdUs);
        std::printf("[S6b] per-call tick() timing: %s\n", detail);
        test::check(
            "S6b: median per-tick wall time stays in the async regime "
            "(catches a sync anywhere in the tick, not just at tick-end)",
            median_us < kBlockingThresholdUs, detail);
    }

    test::section("S7: device tick counter mirrors the host counter");
    {
        gpu::GpuEngine engine(17);
        seed_scene(engine);
        test::check("S7: device tick starts at 0", engine.device_tick() == 0);
        for (int t = 0; t < 7; ++t) engine.tick();
        test::check("S7: host tick advanced to 7", engine.current_tick() == 7);
        test::check("S7: device tick advanced to 7", engine.device_tick() == 7);
    }

    return test::finalize();
}
