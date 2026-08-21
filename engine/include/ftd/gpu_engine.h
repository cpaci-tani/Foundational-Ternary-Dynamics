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
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/strong_stress_energy.h"
#include <vector>
#include <atomic>
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

    void upload_matched_gauss(const eft::MatchedGaussDynamics& src);
    void download_matched_gauss(eft::MatchedGaussDynamics& dst);
    bool matched_gauss_last_step_valid() const { return matched_gauss_last_valid_; }
    void download_strong_stress(std::vector<StrongStressCell>& out);
    void download_strong_step_diagnostics(StrongEnergyStepDiagnostics& out);

    // --- Accessors ---
    const GpuBuffers& bufs() const { return bufs_; }
    int lattice_size() const { return size_; }
    int current_tick() const { return tick_; }
    // Device mirror of current_tick(). Blocking 4-byte D2H — diagnostics and
    // tests only, never the tick path.
    int device_tick() const;

    // The CUDA device's D3D12-comparable LUID (cudaDeviceProp::luid), used by
    // the native desktop app to confirm CUDA and D3D12 selected the same
    // physical adapter before attempting shared-memory interop.
    //
    // Precondition: must be called from the same OS thread that constructed
    // this GpuEngine (or otherwise established CUDA context ownership for
    // it). cudaGetDevice() reads the CALLING THREAD's current-device state —
    // thread-local CUDA runtime state, not a property of this GpuEngine
    // instance — so a call from a different thread (e.g. a UI thread when
    // the engine's context lives on a dedicated sim thread) can silently
    // report a different device's LUID with no error.
    //
    // Returns false for three distinct reasons, left undifferentiated to the
    // caller (this is a capability probe answering "can I attempt interop?",
    // not a diagnostic):
    //   1. cudaGetDevice() failed — a genuine CUDA runtime error.
    //   2. cudaGetDeviceProperties() failed for the reported device — a
    //      genuine CUDA runtime error.
    //   3. The device's luidDeviceNodeMask reads 0, checked as a heuristic
    //      signal that the LUID is unpopulated (non-WDDM). CUDA's own docs
    //      only say luid/luidDeviceNodeMask's value is "undefined on TCC and
    //      non-Windows platforms" — not that it is guaranteed zero there.
    //      Zero is the no-LUID signal observed in practice on this project's
    //      WIN32-only, WDDM-mode-consumer-GPU native app, not a
    //      documented CUDA invalidity guarantee.
    bool device_luid(char out_luid[8]) const;

    // Imports a D3D12_HEAP_FLAG_SHARED resource (via its NT handle, as
    // returned by ID3D12Device::CreateSharedHandle) as CUDA external memory,
    // and maps it as a flat device buffer of `byte_count` bytes. The handle
    // is NOT closed by this call -- ownership semantics for
    // cudaExternalMemoryHandleDesc::handle.win32.handle say the OS handle
    // must stay valid until AFTER cudaImportExternalMemory returns, but CUDA
    // does not take ownership of it. This function does not require the
    // caller to close the handle at any particular time either: native_
    // desktop's main.cpp deliberately keeps interop_buf_handle open for the
    // whole process lifetime (Interop Task 12, commit 93d03a3c) so every
    // later reload can re-import the same underlying D3D12 buffer into the
    // freshly constructed GpuEngine boot() produces, closing it only once,
    // near process exit, well after any number of import calls.
    //
    // Returns false on either of two distinct failure sources, left
    // undifferentiated to the caller (same probe-style contract as
    // device_luid() above): cudaImportExternalMemory() rejecting the handle
    // itself, or cudaExternalMemoryGetMappedBuffer() failing to map the
    // imported object as a flat buffer.
    //
    // Safe to call more than once (e.g. to re-import after a D3D12-side
    // resize) -- matches D3D12Presenter::create_shared_particle_buffer()'s
    // own "safe to call more than once" contract: any previously-imported
    // external memory object is torn down before the new import, so a
    // second call never leaks the first import's driver-level reference.
    // Every (re-)import attempt also resets the interop gather-readiness
    // state (see interop_gather_ready()/interop_particle_count() below), so
    // polling those immediately after a fresh import but before the next
    // interop_gather_particles() call correctly reports "not ready" / 0
    // instead of the previous buffer's stale state.
    //
    // On success, byte_count / sizeof(InteropParticleRecord) is recorded as
    // this buffer's element capacity: interop_gather_particles() below can
    // never write more particles than that, no matter what max_particles it
    // is asked for. byte_count must be the D3D12-side resource's EXACT size
    // (as D3D12Presenter::create_shared_particle_buffer() constructs it,
    // max_particles * sizeof(InteropParticleRecord)) for that bound to be
    // meaningful -- passing a byte_count larger than the resource actually
    // is defeats the clamp and reintroduces the out-of-bounds write it
    // exists to prevent.
    //
    // Precondition: like device_luid(), must be called from the same OS
    // thread that owns this GpuEngine's CUDA context -- the underlying CUDA
    // calls operate against the calling thread's current CUDA context, not
    // a property of this GpuEngine instance.
    bool import_d3d12_particle_buffer(void* nt_handle, std::uint64_t byte_count);

    // Imports a D3D12_FENCE_FLAG_SHARED fence (via its NT handle) as a CUDA
    // external semaphore. Same handle-lifetime contract as
    // import_d3d12_particle_buffer(): this function does not take ownership
    // of the handle and does not require the caller to close it at any
    // particular time -- the native app's main.cpp deliberately keeps
    // interop_fence_handle open for the whole process lifetime (Interop
    // Task 12, commit 93d03a3c) to support re-import across reloads,
    // closing it only once near process exit.
    //
    // Precondition: like import_d3d12_particle_buffer(), must be called from
    // the same OS thread that owns this GpuEngine's CUDA context --
    // cudaImportExternalSemaphore operates against the calling thread's
    // current CUDA context, not a property of this GpuEngine instance.
    bool import_d3d12_fence(void* nt_handle);
    // Signals the imported fence to `value` on the engine stream. When
    // invoked internally by interop_gather_particles() (the expected use),
    // this is ordered after that call's gather kernel launch -- D3D12's
    // wait_shared_fence(value) will unblock once this retires on the GPU
    // timeline (no CPU synchronization involved on either side). This method
    // is public and may also be called directly; a direct external call
    // carries no such ordering guarantee against any particular gather -- it
    // is simply issued on the engine stream at the point of the call.
    //
    // Precondition: like import_d3d12_fence(), must be called from the same
    // OS thread that owns this GpuEngine's CUDA context --
    // cudaSignalExternalSemaphoresAsync operates against the calling
    // thread's current CUDA context, not a property of this GpuEngine
    // instance.
    bool interop_signal_fence(std::uint64_t value);

    // Runs the interop particle gather (writes directly into the imported
    // D3D12 buffer set up by import_d3d12_particle_buffer) and records
    // bufs_.interop_gather_ready. Call after tick(). No-op (returns false)
    // if import_d3d12_particle_buffer() hasn't succeeded yet.
    //
    // max_particles is clamped to the imported buffer's actual element
    // capacity (set by import_d3d12_particle_buffer() from the byte_count it
    // was given) in addition to the unrelated kMaxVisualParticleCapture
    // constant that bounds the separate CPU-decode capture path -- passing 0
    // or a value larger than the imported buffer can hold never writes past
    // the end of the mapped external-memory view; it silently gathers fewer
    // particles instead.
    //
    // fence_value is passed straight to interop_signal_fence() after the
    // gather kernel launches (a no-op if no fence has been imported via
    // import_d3d12_fence() -- interop_signal_fence() itself no-ops when
    // bufs_.interop_fence is null). Callers with no imported fence (e.g.
    // tests that only exercise the gather path) may pass any value; it is
    // unused in that case.
    bool interop_gather_particles(std::uint32_t max_particles,
                                  std::uint64_t fence_value);
    // True once the CPU-visible event recorded by interop_gather_particles()
    // has retired -- i.e. the header's captured_count is safe to read on the
    // CPU. This does NOT by itself prove the buffer is safe for D3D12 to
    // read from: the event is recorded on bufs_.stream BEFORE
    // interop_gather_particles() issues interop_signal_fence()'s
    // cudaSignalExternalSemaphoresAsync() call, and CUDA only guarantees
    // same-stream ops retire in issue order -- observing this earlier
    // event's completion does not prove the later-issued semaphore signal
    // has also retired. The real GPU-timeline safety guarantee for D3D12
    // reads is carried entirely by D3D12Presenter::wait_shared_fence() (the
    // D3D12-side Wait() against the cross-API fence), independent of this
    // CPU-side poll; this function is a "has the CUDA-side gather finished"
    // convenience only, not a D3D12-read-safety signal. Also false before
    // the first interop_gather_particles() call ever succeeds, and false
    // again immediately after any import_d3d12_particle_buffer() call until
    // the next gather completes.
    bool interop_gather_ready() const;
    // Returns 0 (never reads possibly-uninitialized or in-flight host
    // memory) unless interop_gather_ready() is true for the CURRENT gather;
    // internally re-checks interop_gather_ready() itself rather than trusting
    // the caller to have checked it first.
    std::uint32_t interop_particle_count() const;

    // TEST-ONLY. Synchronously downloads `count` records from the imported
    // interop buffer. Production code never calls this -- avoiding exactly
    // this download is the point of the interop path (Task 6). Used only by
    // test_interop_visual_parity to verify the interop kernel's output
    // against the pre-interop CPU reference path byte-for-byte.
    void debug_read_interop_records(std::vector<InteropParticleRecord>& out,
                                    std::uint32_t count) const;

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
    // user actions, so recapture is off the hot path. Default ON: this is the
    // canonical interactive path for the native app. Set false to force
    // direct kernel launches (the graph parity test does exactly that on one
    // of its two engines).
    bool graph_capture_enabled = true;

    std::size_t graph_replays() const { return graph_replays_; }
    std::size_t graph_captures() const { return graph_captures_; }
    std::size_t graph_capture_failures() const { return graph_capture_failures_; }
    std::size_t graph_cache_size() const { return graph_cache_.size(); }

    // Deterministic seeding helper used by the graph parity test so both the
    // direct and the graphed engine start from the same RNG stream. The GPU
    // RNG seed that actually reaches kernels is toggles.langevin_seed, read
    // directly at each launch_pair_production/launch_weak_transmutation call
    // site (see gpu_engine.cu) — set_rng_seed() alone only writes the private
    // rng_seed_ member, which nothing else in the engine reads. This mirrors
    // RenderBridge::seed_rng() (render_bridge.cpp), which sets
    // toggles.langevin_seed directly and calls the backend's set_rng_seed()
    // as a secondary no-op-on-GPU step.
    void seed_rng_for_test() { toggles.langevin_seed = 42; set_rng_seed(42); }

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
    // The 64-bit FNV-1a-style hash is used directly as the graph_cache_ key
    // with no equality check against the source toggle/parameter state on
    // lookup (the usual hash-map shortcut of trusting the hash). This is an
    // accepted, deliberate risk, not an oversight: the input space is a
    // small, bounded combinatorial set of toggles/scalars, at most
    // MAX_GRAPH_CACHE (16) entries are resident at once, and a 64-bit hash's
    // collision probability over that space is negligible — a
    // simplicity/performance tradeoff against adding a full state-equality
    // fallback on every cache hit.
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
    bool matched_gauss_ready_ = false;
    bool matched_gauss_last_valid_ = false;

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

    // True once interop_gather_particles() has launched at least one gather
    // against the CURRENTLY-imported D3D12 buffer. Unlike
    // visual_snapshot_pending_ (a one-shot request/consume flag cleared by
    // poll_visual_snapshot()), this stays true across repeated
    // interop_gather_ready()/interop_particle_count() polls once a gather has
    // run -- the interop caller (a render loop) is expected to poll
    // repeatedly between gathers, not consume once. Gates
    // interop_gather_ready(): cudaEventQuery() on an event that has never had
    // cudaEventRecord() called on it reports cudaSuccess (nothing to wait
    // for), so without this flag readiness would read true before the first
    // gather ever launched. Reset to false by import_d3d12_particle_buffer()
    // on every (re-)import attempt, so it can never describe a gather that
    // ran against a since-replaced buffer.
    //
    // std::atomic: written by interop_gather_particles() (called from the
    // sim thread via NativeEngineSession::request_interop_gather()) and read
    // by interop_gather_ready() (called from the same thread today, but this
    // flag is the one piece of GpuEngine state a future caller could
    // plausibly poll from a different thread than the one driving tick()/
    // interop_gather_particles() -- a plain bool here is a formal data race
    // the moment that happens). Relaxed ordering is sufficient: the real
    // happens-before relationship between a gather's writes and a caller
    // observing them ready is already carried by the CUDA event
    // (bufs_.interop_gather_ready, queried in interop_gather_ready() below)
    // and, cross-API, by the D3D12 shared fence -- this flag only gates
    // "has any gather ever launched", not the gather's own completion.
    std::atomic<bool> interop_gather_launched_{false};

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
