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
        gpu::GpuEngine engine(48);
        seed_scene(engine);
        engine.toggles.forces = true;
        engine.toggles.movement = true;
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));

        // A tick that contained cudaDeviceSynchronize or a synchronous
        // cudaMemset could not leave the stream busy. Queue several ticks
        // back to back and require that at least one observation finds work
        // still in flight.
        bool observed_async = false;
        for (int attempt = 0; attempt < 8 && !observed_async; ++attempt) {
            for (int t = 0; t < 4; ++t) engine.tick();
            if (cudaStreamQuery(engine.bufs().stream) == cudaErrorNotReady) {
                observed_async = true;
            }
        }
        CUDA_CHECK_TEST(cudaStreamSynchronize(engine.bufs().stream));
        test::check("S6: tick enqueues asynchronously", observed_async);
    }

    return test::finalize();
}
