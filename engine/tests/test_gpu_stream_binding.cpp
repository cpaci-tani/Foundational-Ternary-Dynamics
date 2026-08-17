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

    return test::finalize();
}
