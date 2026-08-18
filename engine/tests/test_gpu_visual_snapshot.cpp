// CUDA native visual-capture contract.
//
// The old particle-frame path downloaded N^3 ternary states, chose a host
// sample, then uploaded selected indices for a second synchronous gather.  The
// visual lane instead performs count -> exclusive scan -> deterministic gather
// on the device, copies one fixed bounded staging slot to pinned memory, and
// publishes it through a begin/poll lifecycle.

#include "ftd/gpu_buffers.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/visual_snapshot.h"

#include <cuda_runtime.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& label, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
}

struct Seed {
    int x;
    int y;
    int z;
    std::int8_t state;
    std::int8_t spin;
    std::int8_t color;
};

constexpr std::array<Seed, 9> kSeeds{{
    {0, 0, 0, +1, +1, 0},
    {0, 0, 1, -1, -1, 1},
    {0, 0, 2, +1, +1, 2},
    {0, 0, 3, -1, -1, 3},
    {0, 0, 4, +1, +1, 0},
    {0, 0, 5, -1, -1, 1},
    {0, 0, 6, +1, +1, 2},
    {0, 0, 7, -1, -1, 3},
    {0, 0, 8, +1, +1, 0},
}};

void seed_particles(ftd::RenderBridge& bridge) {
    for (const Seed& seed : kSeeds) {
        bridge.inject_particle(seed.x, seed.y, seed.z, seed.state,
                               {0.1 * seed.state, 0.0, 0.0},
                               seed.spin, seed.color);
    }
}

bool same_record(const ftd::VisualParticleRecord& a,
                 const ftd::VisualParticleRecord& b) {
    return a.index == b.index && a.state == b.state && a.spin == b.spin
        && a.color == b.color && a.remainder_x == b.remainder_x
        && a.remainder_y == b.remainder_y && a.remainder_z == b.remainder_z;
}

bool poll_until_ready(ftd::RenderBridge& bridge, ftd::VisualSnapshot& out) {
    for (int attempt = 0; attempt < 5000; ++attempt) {
        if (bridge.poll_visual_snapshot(out)) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}

bool records_match(const ftd::VisualSnapshot& actual,
                   const ftd::VisualSnapshot& expected) {
    if (actual.particles.records.size() != expected.particles.records.size()) {
        return false;
    }
    for (std::size_t i = 0; i < actual.particles.records.size(); ++i) {
        if (!same_record(actual.particles.records[i], expected.particles.records[i])) {
            return false;
        }
    }
    return true;
}

void CUDART_CB hold_visual_source_stream(void*) {
    // Used only to create a deterministic unfinished CUDA event below.  The
    // native source-replacement barrier must see NotReady and defer rather
    // than destroy the bridge; no simulation state is touched.
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
}

bool cuda_ok(const std::string& label, cudaError_t status) {
    check(label, status == cudaSuccess);
    return status == cudaSuccess;
}

}  // namespace

int main() {
    std::cout << "CUDA visual snapshot contract\n";

    constexpr int L = 9;
    ftd::VisualSnapshotRequest request;
    request.kind = ftd::VisualCaptureKind::Particles;
    request.epoch = 701;
    request.max_particles = 4;

    // The compatibility backend provides the same versioned result and exact
    // legacy accumulator ordering, immediately pollable without a CUDA event.
    ftd::RenderBridge cpu(L);
    cpu.force_cpu();
    cpu.toggles.disable_all();
    seed_particles(cpu);
    check("CPU visual snapshot begins", cpu.begin_visual_snapshot(request));
    check("CPU visual snapshot is immediately ready", cpu.visual_snapshot_ready());
    check("CPU visual snapshot is always safe to replace",
          cpu.visual_snapshot_safe_to_replace() && !cpu.visual_snapshot_in_flight());
    ftd::VisualSnapshot expected;
    check("CPU visual snapshot polls", cpu.poll_visual_snapshot(expected));
    check("CPU visual snapshot consumes once", !cpu.poll_visual_snapshot(expected));
    check("CPU total manifested count", expected.particles.total_manifested == 9u);
    check("CPU bounded capture count", expected.particles.records.size() == 4u);
    // M=9, C=4; the legacy accumulator selects manifested ranks 3,5,7,9.
    const std::array<int, 4> expected_indices{{2, 4, 6, 8}};
    bool ordered = expected.particles.records.size() == expected_indices.size();
    for (std::size_t i = 0; ordered && i < expected_indices.size(); ++i) {
        ordered = expected.particles.records[i].index == expected_indices[i]
               && expected.particles.records[i].state == kSeeds[expected_indices[i]].state
               && expected.particles.records[i].spin == kSeeds[expected_indices[i]].spin
               && expected.particles.records[i].color == kSeeds[expected_indices[i]].color;
    }
    check("CPU legacy deterministic sample ranks", ordered);
    check("CPU provenance uses epoch/tick compatibility convention",
          expected.meta.epoch == request.epoch && expected.meta.tick == 0
          && expected.meta.state_version == 0 && expected.meta.lattice_size == L);

    ftd::RenderBridge gpu(L);
    gpu.set_interactive_gpu_mode(true);
    gpu.toggles.disable_all();
    seed_particles(gpu);
    auto* engine = gpu.gpu_engine_ptr();
    check("GPU backend is active", engine != nullptr);
    if (!engine) return 1;

    const auto* persistent_records = engine->bufs().d_visual_particle_records;
    const auto* persistent_header = engine->bufs().d_visual_particle_header;
    ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
    ftd::gpu::g_gpu_full_voxel_download_calls = 0;
    ftd::gpu::g_gpu_visual_snapshot_download_bytes = 0;
    ftd::gpu::g_gpu_visual_snapshot_launches = 0;

    check("GPU visual snapshot begins", gpu.begin_visual_snapshot(request));
    check("GPU rejects overlapping visual capture", !gpu.begin_visual_snapshot(request));
    check("GPU reports capture in flight before consume", gpu.visual_snapshot_in_flight());
    ftd::VisualSnapshot actual;
    check("GPU visual snapshot becomes pollable", poll_until_ready(gpu, actual));
    check("GPU visual capture retires after poll",
          !gpu.visual_snapshot_in_flight() && gpu.visual_snapshot_safe_to_replace()
          && !gpu.poll_visual_snapshot(actual));
    check("GPU/CPU bounded records have exact deterministic parity",
          records_match(actual, expected));
    check("GPU total manifested parity",
          actual.particles.total_manifested == expected.particles.total_manifested);
    check("GPU provenance stamps submission source",
          actual.meta.epoch == request.epoch && actual.meta.tick == 0
          && actual.meta.lattice_size == L && actual.meta.state_version > 0);
    check("GPU visual capture avoids canonical voxel mirror",
          ftd::gpu::g_gpu_full_voxel_download_calls == 0
          && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
    constexpr std::size_t expected_copy_bytes =
        sizeof(ftd::VisualParticleStagingHeader)
        + static_cast<std::size_t>(ftd::kMaxVisualParticleCapture)
        * sizeof(ftd::VisualParticleRecord);
    check("GPU visual capture copies one fixed bounded pinned frame",
          ftd::gpu::g_gpu_visual_snapshot_launches == 1u
          && ftd::gpu::g_gpu_visual_snapshot_download_bytes == expected_copy_bytes);
    check("GPU visual staging pointers remain persistent",
          persistent_records == engine->bufs().d_visual_particle_records
          && persistent_header == engine->bufs().d_visual_particle_header);

    // A second capture after retirement proves the slot is reusable without a
    // per-request allocation and retains the same deterministic ordering.
    ftd::VisualSnapshot second;
    check("GPU visual staging slot is reusable", gpu.begin_visual_snapshot(request));
    check("second GPU visual snapshot polls", poll_until_ready(gpu, second));
    check("second GPU visual snapshot keeps deterministic records",
          records_match(second, expected));
    check("second capture reuses same persistent staging pointers",
          persistent_records == engine->bufs().d_visual_particle_records
          && persistent_header == engine->bufs().d_visual_particle_header
          && ftd::gpu::g_gpu_visual_snapshot_launches == 2u
          && ftd::gpu::g_gpu_visual_snapshot_download_bytes == 2u * expected_copy_bytes);

    // Make the default stream explicitly wait behind an asynchronous host
    // callback in a separate nonblocking stream.  This gives the visual D2H
    // fence a deterministic NotReady interval, proving the destructive-source
    // barrier is nonblocking and refuses replacement until the event retires.
    cudaStream_t gate_stream = nullptr;
    cudaEvent_t gate_event = nullptr;
    const bool gate_ready = cuda_ok("creates visual barrier stream",
                                    cudaStreamCreateWithFlags(
                                        &gate_stream, cudaStreamNonBlocking))
        && cuda_ok("creates visual barrier event",
                   cudaEventCreateWithFlags(&gate_event, cudaEventDisableTiming))
        && cuda_ok("queues visual barrier callback",
                   cudaLaunchHostFunc(gate_stream, hold_visual_source_stream, nullptr))
        && cuda_ok("records visual barrier event", cudaEventRecord(gate_event, gate_stream))
        && cuda_ok("makes default stream wait for barrier",
                   cudaStreamWaitEvent(nullptr, gate_event, 0));
    if (gate_ready) {
        ftd::VisualSnapshot delayed;
        check("delayed GPU visual snapshot begins", gpu.begin_visual_snapshot(request));
        check("unfinished visual capture blocks source replacement",
              gpu.visual_snapshot_in_flight() && !gpu.visual_snapshot_safe_to_replace());
        check("delayed visual capture retires through poll",
              poll_until_ready(gpu, delayed));
        check("retired visual capture permits source replacement",
              gpu.visual_snapshot_safe_to_replace() && !gpu.visual_snapshot_in_flight());
        check("delayed capture preserves deterministic records",
              records_match(delayed, expected));
    }
    if (gate_event) cuda_ok("destroys visual barrier event", cudaEventDestroy(gate_event));
    if (gate_stream) cuda_ok("destroys visual barrier stream", cudaStreamDestroy(gate_stream));

    // Exercise normal repeated source replacement: every capture is first
    // retired through poll, satisfying the nonblocking destructive barrier.
    bool repeated_retirement = true;
    for (int pass = 0; pass < 3; ++pass) {
        ftd::RenderBridge replacement(L);
        replacement.set_interactive_gpu_mode(true);
        replacement.toggles.disable_all();
        seed_particles(replacement);
        repeated_retirement = replacement.begin_visual_snapshot(request)
            && poll_until_ready(replacement, second)
            && replacement.visual_snapshot_safe_to_replace()
            && !replacement.visual_snapshot_in_flight();
        if (!repeated_retirement) break;
    }
    check("repeated capture then safe source replacement", repeated_retirement);

    std::cout << (failures == 0 ? "ALL PASS\n"
                                : "FAILURES: " + std::to_string(failures) + "\n");
    return failures == 0 ? 0 : 1;
}
