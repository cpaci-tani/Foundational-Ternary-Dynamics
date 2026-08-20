#pragma once
/**
 * SoA (Structure-of-Arrays) device buffers for GPU-accelerated FTD engine.
 *
 * The CPU engine uses AoS (Voxel is currently 344 bytes on x64). For GPU memory
 * coalescence, we decompose into separate arrays per field.
 * Upload/download functions convert between AoS (host) and SoA (device).
 */

#include "voxel.h"
#include "ftd/visual_snapshot.h"
#include "ftd/interop_particle_record.h"
#include <cstddef>   // std::size_t
#include <cstdint>   // uint8_t etc. — Linux/clang require explicit include
#include <vector>
#include <cufft.h>
#include <cuda_runtime_api.h>

namespace ftd {
namespace gpu {

// ─── C5 (CUDA ticket): host→device upload instrumentation + test knob ───────
// g_gpu_upload_bytes accumulates the bytes actually memcpy'd host→device by
// upload_voxels_range() — i.e. by BOTH the full and the delta upload paths —
// so campaigns/tests can record transfer volume before/after. Reset it to 0
// before a measured operation and read it afterwards.
//
// g_gpu_force_full_upload forces upload_voxels_delta() to fall back to a full
// upload. It exists ONLY so test_gpu_delta_upload can capture the pre-C5
// (full-upload) reference and prove the delta path is byte-identical. Default
// false; pure host-side state that never changes any device byte, so it is
// golden-neutral.
extern std::size_t g_gpu_upload_bytes;
extern bool        g_gpu_force_full_upload;

// Interactive-readback instrumentation.  The first pair measures only the
// canonical SoA -> AoS voxel mirror (not compact renderer/observer reads).
// Compact diagnostic kernels increment the final counter by the fixed scalar
// payload copied to the host.  Tests reset these counters around a request to
// prove that a diagnostics poll did not accidentally materialize the lattice.
extern std::size_t g_gpu_full_voxel_download_bytes;
extern std::size_t g_gpu_full_voxel_download_calls;
extern std::size_t g_gpu_compact_diagnostic_download_bytes;
// Async telemetry snapshots have their own persistent staging allocation. The
// counter covers only their fixed scalar D2H payload, never a voxel mirror.
extern std::size_t g_gpu_telemetry_snapshot_download_bytes;
extern std::size_t g_gpu_telemetry_snapshot_launches;
// Native visual captures use a distinct persistent bounded staging slot.  The
// counter covers the fixed particle-frame D2H copy, never a full voxel mirror.
extern std::size_t g_gpu_visual_snapshot_download_bytes;
extern std::size_t g_gpu_visual_snapshot_launches;
// Identity high-water reconciliation is a compact two-scalar D2H operation.
// Separate counters let performance regressions prove clean host reads do not
// repeat that synchronization after the high-water marks are current.
extern std::size_t g_gpu_identity_counter_download_bytes;
extern std::size_t g_gpu_identity_counter_download_calls;

struct GpuBuffers {
    GpuBuffers() = default;
    ~GpuBuffers() { free(); }
    GpuBuffers(const GpuBuffers&) = delete;
    GpuBuffers& operator=(const GpuBuffers&) = delete;

    int N = 0;    // total sites (L^3)
    int L = 0;    // lattice side length

    // --- Engine execution stream (Component A) ---
    // Created by allocate() with DEFAULT (blocking) flags, destroyed by
    // free(). Blocking is deliberate: every CUDA call this engine has NOT
    // migrated (compact diagnostics, injection kernels, the AoS downloads,
    // the visual capture path) still runs on the legacy default stream, and
    // a blocking stream implicitly synchronizes with it, so those paths stay
    // correctly ordered with zero further work. It also makes CUDA reject
    // legacy-stream work issued while this stream is capturing
    // (cudaErrorStreamCaptureImplicit), turning an un-migrated tick launcher
    // into a loud capture failure instead of a silently wrong graph.
    // NEVER capture on the legacy stream: cudaStreamBeginCapture rejects it.
    // This ordering guarantee holds only because today's only caller issues
    // all GPU work from one host thread in program order — it is host
    // issue-order, not a device-side interlock. A future multi-threaded
    // caller (e.g. a snapshot poller on its own thread) issuing legacy-stream
    // work concurrently with a tick in flight on this stream would race at
    // the driver's enqueue boundary with no error signal outside an active
    // capture window.
    cudaStream_t stream = nullptr;

    // --- Per-voxel state ---
    int8_t*   d_state       = nullptr;  // ternary state {-1,0,+1}

    // --- Flux field (Vec3) ---
    double*   d_flux_x      = nullptr;
    double*   d_flux_y      = nullptr;
    double*   d_flux_z      = nullptr;

    // --- Wave velocity (Vec3) ---
    double*   d_wave_vel_x  = nullptr;
    double*   d_wave_vel_y  = nullptr;
    double*   d_wave_vel_z  = nullptr;

    // --- Particle velocity (Vec3) ---
    double*   d_velocity_x  = nullptr;
    double*   d_velocity_y  = nullptr;
    double*   d_velocity_z  = nullptr;

    // --- Sub-lattice remainder (Vec3) ---
    double*   d_remainder_x = nullptr;
    double*   d_remainder_y = nullptr;
    double*   d_remainder_z = nullptr;

    // --- Scalar fields ---
    uint8_t*  d_locked      = nullptr;  // I2 fix: match kernel/memcpy usage (was bool*)
    int32_t*  d_particle_id = nullptr;
    // Canonical lifetime-monotonic identity allocators.  These live on the
    // device so genesis/reaction kernels and host-triggered injections draw
    // from one namespace.  Host-staged uploads only ever raise the counters.
    int32_t*  d_next_particle_id = nullptr;
    int32_t*  d_next_pair_id     = nullptr;
    int32_t*  d_identity_allocation_base = nullptr;
    int32_t*  d_identity_error = nullptr;  // nonzero => exhausted int32 namespace
    int8_t*   d_spin        = nullptr;
    int8_t*   d_color       = nullptr;
    int8_t*   d_flavor      = nullptr;
    double*   d_accel_mag   = nullptr;

    // --- Solver fields ---
    double*   d_phi         = nullptr;  // Gauss potential (warm-started)
    double*   d_phi_coulomb = nullptr;  // Coulomb potential (warm-started)
    double*   d_phi_latency = nullptr;  // Latency Poisson potential (warm-started)
    double*   d_latency     = nullptr;  // voxel.latency = sqrt(clamp(|phi_latency|, 0, 0.998))
    double*   d_tau         = nullptr;  // voxel.tau: accumulated proper time
    double*   d_phase       = nullptr;  // voxel.phase: de Broglie clock phase

    // --- Read-phase temporary (delta_j) ---
    double*   d_delta_j_x   = nullptr;
    double*   d_delta_j_y   = nullptr;
    double*   d_delta_j_z   = nullptr;

    // --- Dual-substrate fields (active when dual_substrate toggle = true) ---
    double*   d_flux_L_x     = nullptr;
    double*   d_flux_L_y     = nullptr;
    double*   d_flux_L_z     = nullptr;
    double*   d_flux_R_x     = nullptr;
    double*   d_flux_R_y     = nullptr;
    double*   d_flux_R_z     = nullptr;
    double*   d_wave_vel_L_x = nullptr;
    double*   d_wave_vel_L_y = nullptr;
    double*   d_wave_vel_L_z = nullptr;
    double*   d_wave_vel_R_x = nullptr;
    double*   d_wave_vel_R_y = nullptr;
    double*   d_wave_vel_R_z = nullptr;
    double*   d_delta_j_L_x  = nullptr;
    double*   d_delta_j_L_y  = nullptr;
    double*   d_delta_j_L_z  = nullptr;
    double*   d_delta_j_R_x  = nullptr;
    double*   d_delta_j_R_y  = nullptr;
    double*   d_delta_j_R_z  = nullptr;

    // --- Strong Substrate Field (Stella Octangula) ---
    // Note: strong_field_stencil_kernel writes wvs_* / fs_* in-place (leapfrog
    // fuses the delta_j accumulator into the velocity update), so no separate
    // d_delta_j_strong_* buffers are needed. Same for weak below.
    double*   d_flux_strong_x     = nullptr;
    double*   d_flux_strong_y     = nullptr;
    double*   d_flux_strong_z     = nullptr;
    double*   d_wave_vel_strong_x = nullptr;
    double*   d_wave_vel_strong_y = nullptr;
    double*   d_wave_vel_strong_z = nullptr;

    // --- Weak Substrate Field (Cuboctahedron) ---
    double*   d_flux_weak_x     = nullptr;
    double*   d_flux_weak_y     = nullptr;
    double*   d_flux_weak_z     = nullptr;
    double*   d_wave_vel_weak_x = nullptr;
    double*   d_wave_vel_weak_y = nullptr;
    double*   d_wave_vel_weak_z = nullptr;

    // --- Selective damping mask ---
    uint8_t*  d_near_particle = nullptr;
    double*   d_near_accel    = nullptr;  // max accel_mag of nearby particles (for Larmor)

    // --- Per-site force diagnostics (mirror of CPU RenderBridge::force_diag_) ---
    // Five components × 3 axes, indexed by lattice site. Populated by the
    // force kernels (phase_forces, color_force) so GpuBackend::sync_to_host()
    // can scatter them back into RenderBridge::force_diag_. Allocated
    // unconditionally — 15 doubles × N is about 30 MiB at L=64 and
    // 1.88 GiB at L=256 — and is included in native resize preflight.
    double*   d_fd_coulomb_x  = nullptr;
    double*   d_fd_coulomb_y  = nullptr;
    double*   d_fd_coulomb_z  = nullptr;
    double*   d_fd_strong_x   = nullptr;
    double*   d_fd_strong_y   = nullptr;
    double*   d_fd_strong_z   = nullptr;
    double*   d_fd_magnetic_x = nullptr;
    double*   d_fd_magnetic_y = nullptr;
    double*   d_fd_magnetic_z = nullptr;
    double*   d_fd_gravity_x  = nullptr;
    double*   d_fd_gravity_y  = nullptr;
    double*   d_fd_gravity_z  = nullptr;
    double*   d_fd_exchange_x = nullptr;
    double*   d_fd_exchange_y = nullptr;
    double*   d_fd_exchange_z = nullptr;

    // Per-tick count of movement-entry repairs for externally mutated
    // out-of-budget velocities (FTD-0402). Normal force evolution leaves zero.
    unsigned long long* d_causal_projection_events = nullptr;

    // --- Poisson mean-charge scratch (Component A) ---
    // The Gauss and Coulomb RHS both need mean_charge = (Σ state) / N. That
    // used to be a per-solve cudaMalloc + cudaMemset + blocking D2H memcpy +
    // cudaFree, i.e. 1-3 host round trips per tick. Both scalars now live
    // here for the engine's lifetime and the value never crosses PCIe: the
    // RHS kernels read the device pointer. Integer reduction is unchanged, so
    // the computed value is bit-identical to the previous host scalar.
    long long* d_poisson_charge_sum  = nullptr;
    double*    d_poisson_mean_charge = nullptr;

    // --- FFT workspace ---
    // Both precisions are active: float (C2C) is the default 2× faster path;
    // double (Z2Z) is used by high-accuracy callsites in kernels_poisson.cu.
    cufftDoubleComplex* d_fft_buf   = nullptr;  // N complex doubles (high-accuracy path)
    cufftComplex*       d_fft_buf_f = nullptr;  // N complex floats (default, 2× faster C2C)
    double*             d_green     = nullptr;   // precomputed 1/G(k) (double precision, computed once)
    float*              d_visual_flux_magnitude = nullptr; // lazy compact renderer staging
    float*              d_visual_flux_plane = nullptr; // lazy L^2 slice staging

    // Fixed-size scratch for device-side diagnostic reductions.  Every
    // diagnostics result is reduced to <=64 doubles plus one signed charge
    // scalar before crossing PCIe, so memory and readback cost do not scale
    // with L.  Shared by the serialized native WebSocket request path.
    static constexpr int COMPACT_DIAGNOSTIC_SCALARS = 64;
    double*    d_compact_diagnostics = nullptr;
    long long* d_compact_charge_sum  = nullptr;

    // Snapshot-specific scratch remains independent from the legacy compact
    // getter scratch above. This is essential because a telemetry publisher
    // may have an async D2H copy in flight while an inspector issues a small
    // synchronous point query on the default stream.
    static constexpr int COMPACT_TELEMETRY_SCALARS = 80;
    double* d_telemetry_snapshot = nullptr;
    double* h_telemetry_snapshot = nullptr;  // pinned host readback target
    cudaEvent_t telemetry_snapshot_ready = nullptr;

    // One low-priority visual capture slot, deliberately independent of the
    // scalar telemetry buffers/event.  Particle capture performs a device
    // count/scan/gather into this fixed <=100k-record staging allocation, then
    // copies the header plus bounded record array to pinned host memory.
    // No request-path cudaMalloc/cudaFree or canonical AoS mirror is needed.
    VisualParticleStagingHeader* d_visual_particle_header = nullptr;
    VisualParticleRecord* d_visual_particle_records = nullptr;
    VisualParticleStagingHeader* h_visual_particle_header = nullptr;
    VisualParticleRecord* h_visual_particle_records = nullptr;
    cudaEvent_t visual_snapshot_ready = nullptr;
    // Set only when teardown observes a faulted visual event.  In that
    // terminal case free() intentionally avoids every dependent CUDA release
    // (and its implicit stream wait); recovery replaces the context.
    bool visual_capture_quarantined = false;

    // Native-desktop D3D12 interop (Component B). Imported from a shared
    // D3D12_HEAP_FLAG_SHARED resource via cudaImportExternalMemory --
    // d_interop_particle_buffer is a *mapped view* into that resource, not a
    // cudaMalloc allocation, so free() must NOT cudaFree() it -- only
    // destroy the mapping and the external memory object.
    void* d_interop_particle_buffer = nullptr;
    cudaExternalMemory_t interop_external_memory = nullptr;
    InteropParticleHeader* d_interop_header = nullptr;         // owned, cudaMalloc'd
    InteropParticleHeader* h_interop_header = nullptr;         // owned, pinned
    cudaEvent_t interop_gather_ready = nullptr;                // owned
    // Element capacity of the currently-imported D3D12 buffer, i.e.
    // byte_count / sizeof(InteropParticleRecord) as computed by
    // GpuEngine::import_d3d12_particle_buffer() from the SAME byte_count the
    // D3D12 side sized the resource to (D3D12Presenter::
    // create_shared_particle_buffer() allocates exactly
    // max_particles * sizeof(InteropParticleRecord) bytes). 0 whenever no
    // buffer is imported (initial state, or between a torn-down import and a
    // new one). launch_interop_particle_gather() MUST clamp its write cap to
    // this value in addition to kMaxVisualParticleCapture -- this is the only
    // record of how many InteropParticleRecord slots the mapped external
    // memory view actually has room for; the D3D12-side buffer is created
    // with an exact, non-padded size, so writing past this many records is an
    // out-of-bounds GPU write into memory shared with D3D12.
    std::uint32_t interop_particle_capacity = 0;
    // Cross-API GPU-timeline fence (Component B, Task 7). Imported from a
    // D3D12_FENCE_FLAG_SHARED fence via cudaImportExternalSemaphore --
    // GpuEngine::interop_signal_fence() signals it (on bufs_.stream) after
    // the gather kernel so D3D12's command queue can Wait() on it before
    // issuing the draw that reads d_interop_particle_buffer. Null whenever
    // no fence is imported (initial state, or between a torn-down import
    // and a new one).
    cudaExternalSemaphore_t interop_fence = nullptr;  // imported D3D12 shared fence

    // --- Particle list (compact indices of manifested particles) ---
    // Scales with lattice: enough for ~1.5% occupation at any size
    static constexpr int MAX_PARTICLES = 8192;
    int*      d_plist_idx     = nullptr;  // lattice indices [MAX_PARTICLES]
    int*      d_num_particles = nullptr;  // count (single int on device)
    // Sticky-until-acknowledged capacity guard (Component A).
    // finalize_particle_list_kernel (kernels_forces.cu) sets this to 1 when
    // the manifested count exceeds MAX_PARTICLES; it never clears it back
    // to 0 itself. The pairwise/triad kernels clamp their loop bound to
    // MAX_PARTICLES so they can never read past d_plist_idx; the host
    // surfaces the condition as the same std::runtime_error it used to
    // throw inline, but reads the flag only at synchronization boundaries
    // that already copy scalars D2H (causal_projection_events /
    // ensure_host_synced), never in the tick. throw_if_particle_overflow()
    // (gpu_buffers.cu) resets the flag to 0 immediately after reading it, so
    // the condition is sticky only until the next check observes it — a
    // transient overflow that later resolves (e.g. via annihilation
    // bringing the count back under MAX_PARTICLES) does not permanently
    // poison every future call.
    int*      d_particle_overflow = nullptr;

    // --- Deterministic particle-list compaction scratch (2026-08-17 fix) ---
    // build_particle_list_kernel used to assign each manifested particle's
    // plist_idx[] slot via atomicAdd(d_num_particles, 1). GPU
    // thread-scheduling order for that race is NOT guaranteed identical
    // between separate kernel launches, even from bit-identical prior
    // device state, so the ORDER of particles within plist_idx[] varied run
    // to run — and color_force_kernel/yukawa_force_kernel/
    // exchange_force_kernel accumulate double-precision force sums by
    // walking plist_idx[] in that order, so a different order flipped low
    // force bits that then compounded across ticks into full state
    // divergence. Replaced with the same cub::DeviceSelect::Flagged
    // compaction pattern already used for pair-production/lifecycle
    // candidates (d_pair_candidate_* above), which preserves ascending
    // lattice-index order deterministically. Because the true manifested
    // count can exceed MAX_PARTICLES — exactly the condition
    // d_particle_overflow exists to detect — CUB's compacted output cannot
    // be written directly into the capacity-bounded d_plist_idx (that would
    // silently overrun it), so it lands here, in an UNCAPPED N-sized
    // scratch pair, and a finalize kernel clamps/copies into d_plist_idx.
    uint8_t*  d_particle_flags             = nullptr;  // [N] is-manifested
    int32_t*  d_particle_candidate_indices = nullptr;  // [N] uncapped compacted indices
    int32_t*  d_particle_candidate_count   = nullptr;  // one scalar, uncapped count

    // --- Device mirror of GpuEngine::tick_ (Component A) ---
    // Every RNG-consuming kernel salts SplitMix64 with (seed, voxel, tick).
    // Passing `tick` by value bakes it into a captured graph's node params,
    // so a replay would repeat the draw. The kernels read this pointer
    // instead, and an in-tick kernel increments it exactly where the host
    // does `tick_++`, keeping the two counters equal by construction.
    int* d_tick = nullptr;

    // --- Pair production tracking ---
    int32_t*  d_pair_id       = nullptr;  // pair ID (-1 = unpaired) [N]
    // Pair candidates are detected in parallel, then stably compacted in
    // ascending X-major index order before one device thread commits the
    // canonical greedy transactions.  The persistent CUB workspace avoids a
    // per-tick cudaMalloc/cudaFree stall in interactive scenarios.
    uint8_t*  d_pair_candidate_flags   = nullptr;  // [N]
    // Movement reuses the first ceil(N/256) bytes as per-block crossing flags.
    // Pair/lifecycle compaction and movement are serialized on the default
    // stream, and the 4N-byte allocation is ample for that byte view.
    int32_t*  d_pair_candidate_indices = nullptr;  // [N]
    int32_t*  d_pair_candidate_count   = nullptr;  // one scalar
    uint8_t*  d_movement_moved         = nullptr;  // [N], per-tick CPU-equivalent arrival guard
    int32_t*  d_movement_order         = nullptr;  // [N], shuffle permutation; cluster_inertia DFS stack
    int32_t*  d_movement_rank          = nullptr;  // [N], inverse permutation; cluster_inertia members
    void*     d_pair_select_temp       = nullptr;
    std::size_t pair_select_temp_bytes = 0;

    // --- Native EFT continuity event ledger ---
    // Reset immediately before GPU movement. Kernels write integrated
    // one-tick currents/reactions directly, avoiding host snapshot inference.
    int*      d_ledger_rho_before = nullptr;
    int*      d_ledger_reaction   = nullptr;
    double*   d_ledger_current_x  = nullptr;
    double*   d_ledger_current_y  = nullptr;
    double*   d_ledger_current_z  = nullptr;

    // FTD-0428 oriented-face Maxwell/Gauss (lazy; NativeCuda isolated sector).
    double* d_matched_ex = nullptr;
    double* d_matched_ey = nullptr;
    double* d_matched_ez = nullptr;
    double* d_matched_bx = nullptr;
    double* d_matched_by = nullptr;
    double* d_matched_bz = nullptr;
    double* d_matched_cx = nullptr;
    double* d_matched_cy = nullptr;
    double* d_matched_cz = nullptr;
    int*    d_matched_valid = nullptr;

    // FTD-0406 string T00/stress + projection scratch (lazy).
    double* d_strong_t00 = nullptr;
    double* d_strong_sxx = nullptr;
    double* d_strong_syy = nullptr;
    double* d_strong_szz = nullptr;
    double* d_strong_sxy = nullptr;
    double* d_strong_sxz = nullptr;
    double* d_strong_syz = nullptr;
    int*    d_strong_idx = nullptr;
    int*    d_strong_id = nullptr;
    int*    d_strong_begin_id = nullptr;
    int8_t* d_strong_color = nullptr;
    double* d_strong_px = nullptr;
    double* d_strong_py = nullptr;
    double* d_strong_pz = nullptr;
    double* d_strong_mx = nullptr;
    double* d_strong_my = nullptr;
    double* d_strong_mz = nullptr;
    int*    d_strong_count = nullptr;
    struct StrongStepDevice {
        double h_before = 0.0;
        double h_after = 0.0;
        double residual = 0.0;
        double lambda = 1.0;
        double mx_before = 0.0;
        double my_before = 0.0;
        double mz_before = 0.0;
        double mx_after = 0.0;
        double my_after = 0.0;
        double mz_after = 0.0;
        int projection_events = 0;
        int projection_failures = 0;
        int topology_failures = 0;
        int projected_particles = 0;
        int active = 0;
    };
    StrongStepDevice* d_strong_step = nullptr;

    // Lifecycle
    void allocate(int lattice_size);
    void free();
    void ensure_matched_gauss();
    void ensure_strong_stress();

    // AoS ↔ SoA transfers
    void upload(const std::vector<Voxel>& host_voxels,
                const std::vector<double>& host_phi,
                const std::vector<double>& host_phi_coulomb);

    void download(std::vector<Voxel>& host_voxels,
                  std::vector<double>& host_phi,
                  std::vector<double>& host_phi_coulomb) const;

    // Upload all voxel fields for the whole lattice (equivalent to
    // upload_voxels_range(host_voxels, 0, N)). Used by inject_particle /
    // inject_wavepacket / the full-upload fallback.
    void upload_voxels(const std::vector<Voxel>& host_voxels);

    // Upload the AoS voxel fields for the contiguous index range [lo, lo+count)
    // into the SoA device arrays. The field set copied here is the SINGLE
    // SOURCE OF TRUTH for a voxel upload; upload_voxels() is the lo=0,count=N
    // case and the C5 delta path (upload_voxels_delta) calls this once per
    // contiguous dirty run. Adds count·sizeof(voxel-fields) to
    // g_gpu_upload_bytes.
    void upload_voxels_range(const std::vector<Voxel>& host_voxels,
                             int lo, int count);

    // C5: partial host→device upload. `shadow` is the host mirror of the
    // CURRENT device SoA state (the caller guarantees device == shadow at every
    // index). Diffs `host_voxels` against `shadow`, uploads only the changed
    // voxels (coalesced into contiguous runs), and is byte-identical to
    // upload_voxels(host_voxels) because every unchanged index already holds
    // the correct bytes. Falls back to a full upload on cold start
    // (shadow.size()!=N), when a large fraction changed, or when
    // g_gpu_force_full_upload is set.
    void upload_voxels_delta(const std::vector<Voxel>& host_voxels,
                             const std::vector<Voxel>& shadow);

    // Raise (never lower) the device counters after a host-staged scenario or
    // test upload. `next_*` are the first IDs not present in the host image.
    void raise_identity_counters(int32_t next_particle_id,
                                 int32_t next_pair_id);
    void download_identity_counters(int32_t& next_particle_id,
                                    int32_t& next_pair_id) const;
    void throw_if_identity_error() const;
    // Throws std::runtime_error when the sticky capacity guard fired. Reads
    // one int D2H; call only at existing synchronization boundaries. Resets
    // the device flag to 0 after reading it (sticky-until-acknowledged, not
    // sticky-forever) — a later call on the same instance only throws again
    // if the condition recurs after this one observed it.
    void throw_if_particle_overflow() const;

    // Download only voxels (for diagnostics)
    void download_voxels(std::vector<Voxel>& host_voxels) const;

    // Compact interactive visualization readbacks. These do not materialize
    // the full host Voxel mirror.
    void download_states(std::vector<std::int8_t>& out) const;
    void download_flux_magnitude(std::vector<float>& out);
    void download_flux_magnitude_plane(int axis, int index,
                                       std::vector<float>& out);

    // Download phi_latency from device (Wave 5: GPU latency Poisson)
    void download_phi_latency(std::vector<double>& out) const;

    // Download per-site force diagnostics. Each output vector is resized to N
    // and filled in voxel-major order so callers can scatter into
    // RenderBridge::force_diag_[i] directly (one ForceDiag per site).
    void download_force_diag(std::vector<double>& fc_x, std::vector<double>& fc_y, std::vector<double>& fc_z,
                             std::vector<double>& fs_x, std::vector<double>& fs_y, std::vector<double>& fs_z,
                             std::vector<double>& fm_x, std::vector<double>& fm_y, std::vector<double>& fm_z,
                             std::vector<double>& fg_x, std::vector<double>& fg_y, std::vector<double>& fg_z,
                             std::vector<double>& fe_x, std::vector<double>& fe_y, std::vector<double>& fe_z) const;

    // Zero all force-diag arrays (called once per tick before force kernels).
    void reset_force_diag();
    unsigned long long download_causal_projection_events() const;

    // Native EFT continuity event ledger helpers
    void reset_continuity_ledger();
    void download_continuity_ledger(std::vector<int>& rho_before,
                                    std::vector<int>& rho_after,
                                    std::vector<int>& reaction,
                                    std::vector<double>& current_x,
                                    std::vector<double>& current_y,
                                    std::vector<double>& current_z) const;

    // Precompute Green's function for FFT Poisson solver
    void precompute_green_function();
};

}  // namespace gpu
}  // namespace ftd
