#pragma once
/**
 * GPU-Accelerated FTD Engine
 *
 * Drop-in alternative to RenderBridge that executes the tick cycle
 * on NVIDIA GPU via CUDA. All field data lives on the device; host
 * transfers happen only for diagnostics or injection.
 *
 * Requires: CUDA Toolkit 12.8+, cuFFT, cuRAND
 * Target: GPU (SM 120, 32 GB VRAM)
 */

#include "voxel.h"
#include "render_bridge.h"  // for Diagnostics, EnergyAudit, TermToggles
#include "gpu_buffers.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/visual_snapshot.h"
#include "ftd/eft/dual_cell_continuity.h"
#include <vector>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <cufft.h>
#include <cuda_runtime_api.h>

namespace ftd {
struct LagrangianDiag;
namespace gpu {

class GpuEngine {
public:
    explicit GpuEngine(int lattice_size);
    ~GpuEngine();

    // Non-copyable
    GpuEngine(const GpuEngine&) = delete;
    GpuEngine& operator=(const GpuEngine&) = delete;

    // --- Core simulation ---
    void tick();
    void run(int num_ticks);
    // End-of-tick d_tau update. Public for parity tests and embedders that
    // drive the GPU sub-phases manually; tick() invokes it automatically for
    // latency_field when the host-only de Broglie phase is not requested.
    void accumulate_proper_time(bool update_phase = false, double omega0 = 0.0);

    // --- Diagnostics (downloads from GPU) ---
    Diagnostics diagnostics();
    EnergyAudit energy_audit();
    GravityMetricAgg gravity_metric_agg();
    void lagrangian_diagnostics(LagrangianDiag& out);
    void inspect_voxel(int index, VoxelInspection& out);
    void inspect_force(int index, ForceDiag& out);

    // --- Coherent native telemetry snapshots ---
    // begin_telemetry_snapshot() appends a fused scalar reduction and pinned
    // D2H copy to the engine stream, then returns without waiting. Exactly one
    // snapshot may be pending. poll_telemetry_snapshot() returns false until
    // its CUDA event has completed; a successful poll consumes the result.
    // The snapshot carries the tick/state-version captured at begin time.
    bool begin_telemetry_snapshot(const TelemetrySnapshotRequest& request);
    bool telemetry_snapshot_ready() const;
    bool poll_telemetry_snapshot(TelemetrySnapshot& out);
    void wait_telemetry_snapshot(TelemetrySnapshot& out);
    TelemetrySnapshot telemetry_snapshot(const TelemetrySnapshotRequest& request);

    // --- Coherent native visual captures ---
    // Exactly one bounded visual capture may be pending.  The first capture
    // kind (Particles) counts, scans, and gathers on the device, then starts
    // a fixed pinned D2H copy.  Polling only observes its event; it never
    // synchronizes the canonical voxel mirror or allocates CUDA memory.
    bool begin_visual_snapshot(const VisualSnapshotRequest& request);
    bool visual_snapshot_ready() const;
    bool poll_visual_snapshot(VisualSnapshot& out);
    /// Nonblocking destructive-source barrier.  `true` means the capture is
    /// absent or its D2H event has completed and may safely be discarded.
    /// A false result means the owner must keep polling rather than destroy
    /// the source GpuEngine underneath the capture kernel.
    bool visual_snapshot_safe_to_replace() const;
    bool visual_snapshot_in_flight() const { return visual_snapshot_pending_; }

    // --- Particle injection (uploads to GPU) ---
    void inject_flux(int x, int y, int z, const Vec3& flux_val);
    void inject_flux_add(int x, int y, int z, const Vec3& flux_val);
    void inject_wave_vel_add(int x, int y, int z, const Vec3& wave_vel);
    void inject_particle(int x, int y, int z, int8_t state,
                         const Vec3& flux_val,
                         int8_t spin = 0, int8_t color = 0, int8_t flavor = 0);
    void inject_wavepacket(int cx, int cy, int cz, int8_t state,
                           double sigma = 3.0, double amplitude = K_B);
    void create_entangled_pair(int x, int y, int z, const Vec3& flux_val);

    // --- Sync to host for inspection ---
    void sync_to_host(std::vector<Voxel>& out);
    // Lifetime identity high-water marks.  These include IDs whose particles
    // have since evaporated/annihilated and therefore cannot be recovered by
    // scanning the current voxel image.
    void identity_counters(int32_t& next_particle_id,
                           int32_t& next_pair_id) const;
    void raise_identity_counters(int32_t next_particle_id,
                                 int32_t next_pair_id);
    void copy_visual_states(std::vector<std::int8_t>& out) const;
    void copy_visual_flux_magnitude(std::vector<float>& out);
    void copy_visual_flux_magnitude_plane(int axis, int index,
                                          std::vector<float>& out);
    void copy_visual_field_sample(VisualFieldKind kind, int stride,
                                  VisualFieldSample& out) const;
    void copy_visual_particle_attributes(const std::vector<int>& indices,
                                         std::vector<float>& out) const;
    eft::DualCellContinuity continuity_step() const;

    // --- Bulk upload from host (for test setup with custom initial conditions) ---
    void upload_from_host(const std::vector<Voxel>& voxels);

    // --- Accessors ---
    const GpuBuffers& bufs() const { return bufs_; }
    int lattice_size() const { return size_; }
    int current_tick() const { return tick_; }
    // Device mirror of current_tick(). Blocking 4-byte D2H — diagnostics and
    // tests only, never the tick path.
    int device_tick() const;
    int total_sites() const { return N_; }
    double dt() const { return dt_; }
    void set_dt(double dt);
    void set_rng_seed(unsigned int seed);

    // Access Coulomb potential (downloads from GPU if stale)
    const std::vector<double>& phi() { ensure_host_synced(); return host_phi_; }
    const std::vector<double>& phi_coulomb() { ensure_host_synced(); return host_phi_coulomb_; }
    // Access latency potential (downloads from GPU on demand)
    const std::vector<double>& phi_latency() { ensure_host_synced(); return host_phi_latency_; }

    // Download per-site force diagnostics (component breakdown of phase_forces
    // + color_force kernels). Sized to N. Vectors stay valid until the next
    // call. Used by GpuBackend::sync_to_host() to repopulate
    // RenderBridge::force_diag_ in lockstep with the voxel mirror.
    struct ForceDiagHost {
        std::vector<double> coulomb_x, coulomb_y, coulomb_z;
        std::vector<double> strong_x,  strong_y,  strong_z;
        std::vector<double> magnetic_x, magnetic_y, magnetic_z;
        std::vector<double> gravity_x,  gravity_y,  gravity_z;
        std::vector<double> exchange_x, exchange_y, exchange_z;
    };
    const ForceDiagHost& force_diag() { ensure_host_synced(); return host_force_diag_; }
    unsigned long long causal_projection_events() const {
        bufs_.throw_if_identity_error();
        bufs_.throw_if_particle_overflow();
        return bufs_.download_causal_projection_events();
    }

    // Physics toggles (same as CPU engine)
    TermToggles toggles;

    // --- CUDA graph capture (Component A) ---
    // When true, tick() captures its kernel sequence once per distinct
    // combination of topology- and parameter-affecting host state, then
    // replays the instantiated cudaGraphExec_t on every later tick with the
    // same key. Replay must be BIT-IDENTICAL to direct launch — that is what
    // test_gpu_graph_capture asserts. Toggle changes are rare, deliberate
    // user actions, so recapture is off the hot path.
    bool graph_capture_enabled = false;

    std::size_t graph_replays() const { return graph_replays_; }
    std::size_t graph_captures() const { return graph_captures_; }
    std::size_t graph_capture_failures() const { return graph_capture_failures_; }
    std::size_t graph_cache_size() const { return graph_cache_.size(); }

    // Deterministic seeding helper used by the graph parity test so both the
    // direct and the graphed engine start from the same RNG stream.
    void seed_rng_for_test() { set_rng_seed(42); }

    double genesis_threshold_override = -1.0;
    double manifest_scale_override = -1.0;
    bool manifest_use_temperature = false;

    // --- Non-Abelian gauge link sector (revision 0.9 option a) ---
    // Device link buffers are lazily allocated by upload_gauge_links() on the
    // first su2_gauge/su3_gauge-enabled tick: 528 B/site live + the same again
    // for the Jacobi scratch set — zero cost unless the sector is active
    // (mirrors the CPU-side lazy allocation, revision 4.1b). GpuBackend::tick()
    // uploads the RenderBridge host arrays ONCE on activation (host-side link
    // mutations after activation are not tracked — no engine path writes them)
    // and downloads after each gauge-enabled tick via sync_to_host(), so the
    // RenderBridge su2/su3_links_*() accessors stay truthful. Separately
    // allocated from GpuBuffers so the gauge sector never touches the
    // golden/parity buffer lifecycle (same rationale as the spectroscopy
    // probe facility below).
    void upload_gauge_links(const std::vector<SU2Link>& su2_x,
                            const std::vector<SU2Link>& su2_y,
                            const std::vector<SU2Link>& su2_z,
                            const std::vector<SU3Link>& su3_x,
                            const std::vector<SU3Link>& su3_y,
                            const std::vector<SU3Link>& su3_z);
    void download_gauge_links(std::vector<SU2Link>& su2_x,
                              std::vector<SU2Link>& su2_y,
                              std::vector<SU2Link>& su2_z,
                              std::vector<SU3Link>& su3_x,
                              std::vector<SU3Link>& su3_y,
                              std::vector<SU3Link>& su3_z) const;
    bool gauge_links_on_device() const { return gauge_links_device_; }

    // --- Spectroscopy probe facility (FTD-0281 rung-b, 2026-06-20) ---
    // Device-side shell-autocorrelation so large-L spectroscopy does NOT pay the
    // per-tick full-lattice download. spectro_set_probes() uploads the scattered
    // probe-index set + captures J(0); spectro_autocorr() gathers J(t) at those
    // probes on device, downloads only the compact array, and sums J(0)·J(t) in
    // fixed probe order (deterministic). All flux lives on device the whole run.
    // Single-substrate observable flux only (dual_substrate not used for FTD-0281).
    void spectro_set_probes(const std::vector<int>& probe_indices);
    double spectro_autocorr();   // returns C(t) = Σ_probe J(0)·J(t) for current device state
    void spectro_free();

private:
    // The kernel sequence of one tick, with NO host-side device reads and no
    // data-dependent host branching. This is what gets captured.
    void record_tick_body();
    // Hash of every host-derived value that reaches a kernel argument, EXCEPT
    // the tick counter (which is device-resident, see GpuBuffers::d_tick).
    std::uint64_t graph_key() const;
    // False for tick shapes that cannot be represented by a static graph.
    bool graph_eligible() const;
    void destroy_graph_cache();

    std::unordered_map<std::uint64_t, cudaGraphExec_t> graph_cache_;
    std::size_t graph_replays_ = 0;
    std::size_t graph_captures_ = 0;
    std::size_t graph_capture_failures_ = 0;
    // Bound the cache so a toggle sweep cannot leak graph execs. On overflow
    // the whole cache is dropped and rebuilt — deterministic and simple.
    static constexpr std::size_t MAX_GRAPH_CACHE = 16;

    // GPU tick sub-phases
    void gpu_phase_read();
    void gpu_phase_write();

    void gpu_gauss_project();
    void gpu_solve_coulomb();
    void gpu_solve_latency_poisson();   // Wave 5: GPU latency Poisson. Renamed from gpu_solve_latency (F7 callstack audit 2026-04-17) for parity with CPU solve_latency_poisson.
    void gpu_phase_forces();
    void gpu_phase_movement();

    // Extended physics sub-phases
    void gpu_weak_transmutation();
    void gpu_build_particle_list();
    void gpu_particle_forces();
    void gpu_triad_detection();
    void gpu_pair_production();
    void gpu_gauge_relax();      // revision 0.9 option a: SU(2)/SU(3) staple sweep
    void free_gauge_links();     // dtor helper for the lazily-allocated buffers

    int size_;              // lattice side length
    int N_;                 // total sites (size^3)
    int tick_ = 0;
    double dt_ = 1.0;
    // Increments after every GPU-visible state mutation. Unlike tick_, this
    // also advances for direct injection and host uploads, allowing a native
    // scheduler to reject stale observations between ticks.
    std::uint64_t state_version_ = 0;

    GpuBuffers bufs_;

    // cuFFT plans (created once, reused every tick).
    // Both precisions are active; see gpu_buffers.h for usage notes.
    cufftHandle fft_plan_forward_  = 0;   // Z2Z double (high-accuracy path)
    cufftHandle fft_plan_inverse_  = 0;   // Z2Z double (high-accuracy path)
    cufftHandle fft_plan_forward_f_ = 0;  // C2C float (default, 2× faster)
    cufftHandle fft_plan_inverse_f_ = 0;  // C2C float (default, 2× faster)

    unsigned int rng_seed_ = 0;
    bool rng_seed_initialized_ = false;

    // Host-side shadow for injection and diagnostics
    // Lazily allocated on first use
    std::vector<Voxel> host_voxels_;
    std::vector<double> host_phi_;
    std::vector<double> host_phi_coulomb_;
    std::vector<double> host_phi_latency_;  // Wave 5: GPU latency Poisson shadow
    ForceDiagHost host_force_diag_;          // Per-site force component mirror
    bool host_dirty_ = true;  // true = device has newer data than host

    // Non-Abelian gauge link device buffers (revision 0.9 option a) — live +
    // Jacobi scratch per direction; lazily allocated by upload_gauge_links(),
    // swapped after each relax launch, freed in the dtor.
    SU2Link* d_su2_[3]     = {nullptr, nullptr, nullptr};
    SU2Link* d_su2_scr_[3] = {nullptr, nullptr, nullptr};
    SU3Link* d_su3_[3]     = {nullptr, nullptr, nullptr};
    SU3Link* d_su3_scr_[3] = {nullptr, nullptr, nullptr};
    bool gauge_links_device_ = false;

    // Spectroscopy probe device buffers (FTD-0281 rung-b). Separately allocated
    // from GpuBuffers so the probe facility is self-contained and never touches
    // the golden/parity buffer lifecycle.
    int*    d_probe_idx_ = nullptr;   // n_probe scattered lattice indices
    double* d_probe_jx_  = nullptr;   // n_probe gathered flux.x (scratch)
    double* d_probe_jy_  = nullptr;
    double* d_probe_jz_  = nullptr;
    int     n_probe_ = 0;
    std::vector<double> probe_j0x_, probe_j0y_, probe_j0z_;  // host J(0) reference
    std::vector<double> probe_jx_, probe_jy_, probe_jz_;     // host gather scratch

    bool weak_field_active_ = false;  // true when flavor/weak-field state needs stepping
    bool continuity_ledger_valid_ = false;

    // One pinned host staging buffer/event lives in GpuBuffers. These fields
    // capture the immutable provenance for the in-flight request; subsequent
    // simulation work can be enqueued after the D2H copy without altering it.
    bool telemetry_snapshot_pending_ = false;
    TelemetrySnapshotRequest telemetry_snapshot_request_{};
    std::uint64_t telemetry_snapshot_state_version_ = 0;
    int telemetry_snapshot_tick_ = 0;
    bool telemetry_snapshot_gravity_requested_ = false;

    // Immutable provenance of the one in-flight visual capture.  The actual
    // bounded records/header and completion event live in GpuBuffers so their
    // allocation/destruction follows the engine's CUDA resource lifecycle.
    bool visual_snapshot_pending_ = false;
    VisualSnapshotRequest visual_snapshot_request_{};
    std::uint64_t visual_snapshot_state_version_ = 0;
    int visual_snapshot_tick_ = 0;

    // Helper: ensure host shadow is up-to-date
    void ensure_host_synced();
    // Helper: push host changes to device
    void push_to_device();
    // Helper: refresh weak_field_active_ after host-side setup changes
    void refresh_weak_field_active_from_host();
    void mark_device_state_changed() { ++state_version_; }
};

}  // namespace gpu
}  // namespace ftd
