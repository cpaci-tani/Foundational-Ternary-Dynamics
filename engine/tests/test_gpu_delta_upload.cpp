// ============================================================================
// test_gpu_delta_upload.cpp — C5 (CUDA ticket): host→device delta upload.
//
// GpuEngine::upload_from_host() (the Route-B path taken by
// GpuBackend::flush_host_mutations when a caller edits the host voxel shadow
// via RenderBridge::voxels()) used to re-push the ENTIRE voxel image every
// time — ~85 MB at L=64 even for a single-voxel edit. C5 replaces that with a
// diff-against-device-shadow delta upload (GpuBuffers::upload_voxels_delta):
// only the changed voxels are transferred.
//
// This test pins the two properties that make C5 safe:
//   1. BYTE-IDENTITY — the delta upload lands the exact same device state as a
//      full upload (and as the intended host state), verified by reading the
//      DEVICE back (bufs().download_voxels), not the host shadow.
//   2. TRANSFER REDUCTION — a handful-of-voxels edit moves <<1 MB and orders of
//      magnitude less than a full upload (recorded via g_gpu_upload_bytes).
//
// The pre-C5 (full-upload) reference is reproduced in-process by setting
// gpu::g_gpu_force_full_upload — so the "before/after" comparison is exact, not
// a remembered constant. Canonical platform: WSL2 (engine/build_wsl); self-
// skips on a CPU-only build (no GPU backend).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/gpu_buffers.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>

using namespace ftd;

namespace {

std::vector<Voxel> debug_delta_snapshot;
std::vector<Voxel> debug_full_snapshot;

inline bool bits_eq(double a, double b) {
    std::uint64_t ua, ub;
    std::memcpy(&ua, &a, sizeof(ua));
    std::memcpy(&ub, &b, sizeof(ub));
    return ua == ub;
}
inline bool vec_eq(const Vec3& a, const Vec3& b) {
    return bits_eq(a.x, b.x) && bits_eq(a.y, b.y) && bits_eq(a.z, b.z);
}

// Bit-exact equality over exactly the fields upload_voxels_range() transfers.
// Returns true if equal; on mismatch sets *first to the first differing index.
bool voxels_equal_uploaded(const std::vector<Voxel>& a,
                           const std::vector<Voxel>& b, int* first) {
    if (a.size() != b.size()) { if (first) *first = -2; return false; }
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        const Voxel& x = a[i];
        const Voxel& y = b[i];
        const bool same =
            x.state == y.state && x.color == y.color && x.flavor == y.flavor &&
            x.spin == y.spin && x.locked == y.locked &&
            x.particle_id == y.particle_id && x.pair_id == y.pair_id &&
            bits_eq(x.accel_mag, y.accel_mag) && bits_eq(x.latency, y.latency) &&
            bits_eq(x.tau, y.tau) && bits_eq(x.phase, y.phase) &&
            vec_eq(x.flux, y.flux) && vec_eq(x.wave_vel, y.wave_vel) &&
            vec_eq(x.velocity, y.velocity) && vec_eq(x.remainder, y.remainder) &&
            vec_eq(x.flux_L, y.flux_L) && vec_eq(x.flux_R, y.flux_R) &&
            vec_eq(x.wave_vel_L, y.wave_vel_L) && vec_eq(x.wave_vel_R, y.wave_vel_R) &&
            vec_eq(x.flux_strong, y.flux_strong) &&
            vec_eq(x.wave_vel_strong, y.wave_vel_strong) &&
            vec_eq(x.flux_weak, y.flux_weak) &&
            vec_eq(x.wave_vel_weak, y.wave_vel_weak);
        if (!same) { if (first) *first = i; return false; }
    }
    if (first) *first = -1;
    return true;
}

// ------------------------------------------------------------------
// Direct GpuEngine test: delta upload == full upload == intended state,
// verified against the DEVICE, with the transfer volume recorded.
// ------------------------------------------------------------------
void test_delta_byte_identity() {
    test::section("C5: delta upload is byte-identical to a full upload (device ground truth)");

    constexpr int L = 16;
    constexpr int N = L * L * L;
    gpu::GpuEngine eng(L);

    // A varied initial state so the diff has structure to preserve/miss.
    std::vector<Voxel> init(N);
    for (int i = 0; i < N; ++i) {
        Voxel& v = init[i];
        v.flux        = Vec3(0.001 * i, -0.002 * (i % 7), 0.5 * std::sin(0.01 * i));
        v.wave_vel    = Vec3(0.003 * (i % 5), 0.0, 0.001 * i);
        v.state       = (i % 13 == 0) ? int8_t(1) : (i % 17 == 0 ? int8_t(-1) : int8_t(0));
        if (v.state != 0) { v.velocity = Vec3(0.01, 0.0, 0.0); v.spin = 1; v.color = int8_t((i % 3) + 1); }
        v.latency     = 0.001 * (i % 9);
        v.flux_weak   = Vec3(0.002 * (i % 4), 0.0, 0.0);
        v.flux_strong = Vec3(0.0, 0.001 * (i % 6), 0.0);
    }
    eng.upload_from_host(init);   // cold start → full upload; device == shadow == init

    // Mutate a handful of voxels touching many field types. Includes an
    // ADJACENT pair (100,101) to exercise run-coalescing plus two isolated
    // sites — three contiguous runs total.
    std::vector<Voxel> mutated = init;
    const int idxs[] = { 100, 101, 2000, 3500 };
    for (int idx : idxs) {
        Voxel& v = mutated[idx];
        v.flux.x      += 1.25;
        v.wave_vel.z  -= 0.5;
        v.velocity.y  += 0.03;
        v.state        = 1;
        v.spin         = -1;
        v.color        = 2;
        v.flavor       = 2;
        v.flux_weak.y += 0.7;
        v.flux_strong.z += 0.9;
        v.latency      = 0.42;
        v.tau         += 3.0;
        v.phase       -= 0.7;
        v.locked       = true;
        v.particle_id  = 777;
        v.pair_id      = 5;
    }

    // ---- DELTA path (C5) ----
    gpu::g_gpu_force_full_upload = false;
    gpu::g_gpu_upload_bytes = 0;
    eng.upload_from_host(mutated);
    const std::size_t delta_bytes = gpu::g_gpu_upload_bytes;
    std::vector<Voxel> dev_delta;
    eng.bufs().download_voxels(dev_delta);   // DEVICE ground truth

    // ---- reset device to `init`, then FULL path reference ----
    eng.upload_from_host(init);              // revert device (delta init<-mutated)
    gpu::g_gpu_force_full_upload = true;
    gpu::g_gpu_upload_bytes = 0;
    eng.upload_from_host(mutated);
    const std::size_t full_bytes = gpu::g_gpu_upload_bytes;
    std::vector<Voxel> dev_full;
    eng.bufs().download_voxels(dev_full);
    gpu::g_gpu_force_full_upload = false;    // restore default for any later test

    std::printf("[c5] delta_bytes=%zu  full_bytes=%zu  ratio=%.1fx  (%d voxels changed of %d)\n",
                delta_bytes, full_bytes,
                delta_bytes ? double(full_bytes) / double(delta_bytes) : 0.0,
                int(sizeof(idxs) / sizeof(idxs[0])), N);

    int mm = -1;
    bool eq_df = voxels_equal_uploaded(dev_delta, dev_full, &mm);
    std::string d1 = eq_df ? "" : ("first mismatch at voxel " + std::to_string(mm));
    test::check("delta device state == full device state (byte-identical)", eq_df, d1.c_str());

    bool eq_dm = voxels_equal_uploaded(dev_delta, mutated, &mm);
    std::string d2 = eq_dm ? "" : ("first mismatch at voxel " + std::to_string(mm));
    test::check("delta device state == intended mutated host state", eq_dm, d2.c_str());

    test::check("delta moved strictly less than a full upload (>=50x smaller)",
                delta_bytes > 0 && delta_bytes * 50 < full_bytes,
                "delta path should transfer only the changed voxels");
    test::check("handful-of-voxels delta well under 1 MB",
                delta_bytes < 1024u * 1024u,
                "C5 target: a small edit uploads <<1 MB");
}

// ------------------------------------------------------------------
// Idempotence: re-uploading an UNCHANGED host array must be a no-op
// (zero transfer) and must leave the device unchanged.
// ------------------------------------------------------------------
void test_delta_noop_on_no_change() {
    test::section("C5: re-uploading an unchanged array transfers zero bytes");

    constexpr int L = 12;
    constexpr int N = L * L * L;
    gpu::GpuEngine eng(L);

    std::vector<Voxel> st(N);
    for (int i = 0; i < N; ++i) st[i].flux = Vec3(0.01 * i, 0.0, 0.0);
    eng.upload_from_host(st);          // full (cold start)

    std::vector<Voxel> before;
    eng.bufs().download_voxels(before);

    gpu::g_gpu_force_full_upload = false;
    gpu::g_gpu_upload_bytes = 0;
    eng.upload_from_host(st);          // identical array → delta finds nothing
    const std::size_t noop_bytes = gpu::g_gpu_upload_bytes;

    std::vector<Voxel> after;
    eng.bufs().download_voxels(after);

    test::check("no-change re-upload transfers 0 bytes", noop_bytes == 0,
                "delta of an unchanged array must be a pure no-op");
    int mm = -1;
    test::check("device unchanged after no-op re-upload",
                voxels_equal_uploaded(before, after, &mm), "");
}

// ------------------------------------------------------------------
// Integration test through the REAL flush path: a host mutation via
// RenderBridge::voxels() followed by tick() must produce identical
// physics whether the flush used the delta path or a forced full
// upload. Uses shipping defaults (deterministic — the GPU golden is
// bit-stable), so the two hashes must match exactly.
// ------------------------------------------------------------------
std::uint64_t run_mutate_tick(bool force_full) {
    constexpr int L = 17;
    RenderBridge rb(L);   // GPU backend if this build has one

    // If this is a CPU-only build, the delta path is irrelevant — signal skip.
    if (rb.gpu_engine_ptr() == nullptr) return 0;

    rb.seed_rng(42);
    rb.inject_particle(3, 3, 3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
    for (int t = 0; t < 5; ++t) rb.tick();   // settle

    // Host-side single-voxel edit via the accessor → sets host_mutated_ →
    // next tick flushes through upload_from_host (delta, or forced full).
    gpu::g_gpu_force_full_upload = force_full;
    rb.voxels()[rb.lattice().index(9, 8, 8)].flux += Vec3{0.3, -0.1, 0.05};
    rb.tick();
    gpu::g_gpu_force_full_upload = false;

    (force_full ? debug_full_snapshot : debug_delta_snapshot) = rb.voxels();
    // The compact CUDA energy audit is an unordered parallel reduction, so
    // its last-bit sum is not a cross-instance byte hash. C5 is a trajectory
    // gate: compare the deterministic state fold here and the complete
    // uploaded voxel field set below.
    return test::compute_state_only_hash(rb);
}

void test_delta_integration_through_flush() {
    test::section("C5: RenderBridge flush path — delta vs full produce identical physics");

    const std::uint64_t h_delta = run_mutate_tick(/*force_full=*/false);
    const std::uint64_t h_full  = run_mutate_tick(/*force_full=*/true);

    if (h_delta == 0 && h_full == 0) {
        test::check("integration skipped on CPU-only build (no GPU backend)", true, "");
        return;
    }
    std::printf("[c5] flush-path hash: delta=0x%016llx full=0x%016llx\n",
                static_cast<unsigned long long>(h_delta),
                static_cast<unsigned long long>(h_full));
    const bool voxel_equal = voxels_equal_uploaded(
        debug_delta_snapshot, debug_full_snapshot, nullptr);
    test::check("delta flush path == full flush path (identical post-tick state)",
                h_delta == h_full && voxel_equal,
                "a single-voxel host edit must tick identically whether flushed "
                "via the delta path or a full upload");
}

}  // namespace

int main() {
    test::init("test_gpu_delta_upload");
    test_delta_byte_identity();
    test_delta_noop_on_no_change();
    test_delta_integration_through_flush();
    return test::finalize();
}
