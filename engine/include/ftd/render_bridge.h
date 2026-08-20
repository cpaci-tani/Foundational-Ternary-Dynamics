#pragma once
/**
 * Render-Bridge Tick Engine
 *
 * Implements the Scale-0 lattice tick dynamics with staged read/write loops
 * and toggle-gated extension phases. The full call graph is documented in
 * engine/CALLSTACKS.md; the short CPU path is:
 *   1. phase_read(): wave/coupling deltas
 *   2. phase_write(): flux commit, damping/noise, genesis/evaporation
 *   3. optional pair production
 *   4. Gauss projection
 *   5. optional latency Poisson
 *   6. forces, then movement/collisions
 *   7. optional boundary, weak, triad, and proper-time phases
 *   8. tick/time/dirty-flag/energy-ledger bookkeeping
 */

#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <atomic>
#include <string>
#include <cstdint>
#include <thread>
#include "ftd/injector.h"
#include "ftd/backend.h"
#include "ftd/bridge_rng.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/engine_state.h"
#include "lattice.h"
#include "voxel.h"
#include "hilbert.h"
#include "term_toggles.h"
#include "constants.h"
#include "ftd/gauge_field.h"
#include "ftd/strong_stress_energy.h"
#include "ftd/visual_field_sample.h"
#include "ftd/visual_snapshot.h"
#include "field_operators.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/eft/matched_gauss_transport.h"

// Phase 1 of refactor sweep (2026-04-27): the 5 POD diagnostic structs
// (Diagnostics, AggregateProfile, EnergyAudit, EnergyLedger, EMFieldDiag)
// previously defined here are now in render_bridge_diagnostics.h. The
// header is re-included below so existing consumers see no API change;
// the goal is to cut TU rebuild fan-out for diagnostic-only field
// additions from ~30 TUs to ~5. See docs/adr/0008-r1-r5-phase-extraction.md.
#include "render_bridge_diagnostics.h"
#include "ftd/telemetry_snapshot.h"

#ifdef FTD_ENABLE_CUDA
namespace ftd { namespace gpu { class GpuEngine; } }
#endif

namespace ftd {

struct LagrangianDiag;

// Observation-only per-knot telemetry recorder (defined in knot_telemetry.h).
// Forward-declared + held by unique_ptr (PIMPL) here because knot_telemetry.h
// itself includes render_bridge.h (cluster_tracker/observables/genealogy do
// too) — pulling its full definition into this header before `class
// RenderBridge` is closed would be a circular dependency. The accessor /
// recorder bodies live in render_bridge.cpp where KnotTracker is complete,
// mirroring the existing forward-declared gpu::GpuEngine PIMPL. Read-only
// ⇒ golden-neutral.
class KnotTracker;

class RenderBridge {
    // ARCH-2: Backend implementations need access to GPU sync state and
    // private buffers during the migration. Friendship is the simplest path
    // while the state still lives on RenderBridge; a future cleanup will
    // move host/device sync state into GpuBackend itself.
    friend class CpuBackend;
#ifdef FTD_ENABLE_CUDA
    friend class GpuBackend;
#endif
    // Revision 3.1: the default-backend factory constructs (and, under CUDA,
    // assigns) the private gpu_ engine — the backend-selection policy now
    // lives entirely in backend.h/backend.cpp.
    friend std::unique_ptr<Backend> make_default_backend(RenderBridge&, int);

    // Extracted physics modules (2026-04-18 refactor R1-R6) need access to
    // internal state (rng_, uniform_, next_particle_id_, phi_, buffers).
    friend void weak_transmutation_cpu(RenderBridge&);
    friend void relax_su2_links_cpu(RenderBridge&, double, double);
    friend void relax_su3_links_cpu(RenderBridge&, double, double);
    friend void accumulate_proper_time(RenderBridge&);
    friend void pair_production_cpu(RenderBridge&);
    friend void triad_binding_cpu(RenderBridge&);
    friend void update_energy_ledger_cpu(RenderBridge&);
    // Phase 4a (2026-04-27): phase_write decomposition. See
    // engine/src/render_bridge_phases/phase_write.cpp.
    friend void compute_near_particle_mask(RenderBridge&);
    friend void snapshot_flux_pre_write(RenderBridge&);
    friend void phase_write_main_loop(RenderBridge&);
    friend void phase_write_assign_pending_ids(RenderBridge&);
    friend void apply_absorbing_boundary(RenderBridge&);
    friend void apply_reflective_flux_boundary(RenderBridge&);
    friend void apply_dispersal_flux_boundary(RenderBridge&);
    // Phase 4b (2026-04-27): phase_forces decomposition. See
    // engine/src/render_bridge_phases/phase_forces.cpp.
    friend void phase_forces_solve_potentials(RenderBridge&);
    friend void phase_forces_build_color_cache(RenderBridge&);
    friend void phase_forces_main_loop(RenderBridge&);
    friend void phase_forces_integrate_clusters(RenderBridge&);  // unified-mass Phase 2: rigid-body cluster inertia
    // FTD-0406 selected strong Hamiltonian / local stress-energy contract.
    friend double compute_strong_potential_energy(const RenderBridge&);
    friend void compute_strong_stress_cells(const RenderBridge&, std::vector<StrongStressCell>&);
    friend void begin_strong_energy_step(RenderBridge&);
    friend void complete_strong_energy_step(RenderBridge&);
    // Test access (test_cluster_inertia.cpp): the cluster-inertia falsifier
    // injects a FIXED total force into force_diag_ and reads V_COM back, both
    // of which are private. A friend keeps the test honest (no public mutator
    // for force_diag_) and does not alter struct layout (golden-safe).
    friend void test_cluster_inertia_inject_force(RenderBridge&, int idx, const Vec3& f);
    // Phase 4c (2026-04-27): phase_read + phase_movement decomposition. See
    // engine/src/render_bridge_phases/phase_read.cpp and phase_movement.cpp.
    friend void phase_read_main_loop(RenderBridge&);
    friend void phase_movement_main_loop(RenderBridge&);
    // ARCH-2-I (2026-04-25): the 6 inject_*_cpu friends were dropped — the
    // injection helpers now use the public API (backend(), voxels(),
    // lattice(), injector(), gpu_engine_ptr()) only.

public:
    RenderBridge(int lattice_size);
    ~RenderBridge();

    // Access (GPU-aware: syncs device→host when needed via Backend dispatch).
    // ARCH-2-G/H: replaced #ifdef ladder with backend_->sync_to_host() +
    // backend_->mark_host_dirty(). CpuBackend methods are no-ops; GpuBackend
    // performs the sync. Calls are unconditional — the no-op cost on CPU is
    // a virtual-call indirection (negligible).
    Lattice& lattice() { return lattice_; }
    const Lattice& lattice() const { return lattice_; }
    std::vector<Voxel>& voxels() {
        assert_sim_thread();
        if (backend_) {
            backend_->sync_to_host();
            backend_->mark_host_dirty();  // non-const overload — caller may mutate
        }
        ternary_dirty_from_voxels_ = true;
        fields_dirty_from_voxels_ = true;
        return voxels_;
    }
    const std::vector<Voxel>& voxels() const {
        assert_sim_thread();
        if (backend_) backend_->sync_to_host();
        return voxels_;
    }
    Voxel& voxel_at(int x, int y, int z) {
        if (backend_) {
            backend_->sync_to_host();
            backend_->mark_host_dirty();
        }
        ternary_dirty_from_voxels_ = true;
        fields_dirty_from_voxels_ = true;
        return voxels_[lattice_.index(x, y, z)];
    }
    int8_t state_at(int idx) const;
    int8_t state_at(int x, int y, int z) const {
        return state_at(lattice_.index(x, y, z));
    }
    bool is_manifested(int idx) const;
    void set_state(int idx, int8_t state);
    void set_state(int x, int y, int z, int8_t state) {
        set_state(lattice_.index(x, y, z), state);
    }
    long long charge_sum() const;
    const std::vector<int>& active_indices() const;
    const std::vector<int>& ordered_active_indices() const;
    const TernaryField& ternary_field() const;
    const FieldSoA& fields() const;
    Vec3 flux_at(int idx) const;
    Vec3 wave_vel_at(int idx) const;
    double density_at(int idx) const;
    // Phase 2 gravity panel: reduced stats of the real C++ latency field
    // (voxel.latency, distinct from the |J|² web proxy). The dense latency
    // volume itself is exported (layout-transposed) directly in the WASM binding.
    GravityMetricAgg gravity_metric_agg() const; // reduced L/f/γ/dilation stats
    const EngineState& engine_state() const {
        sync_ternary_from_voxels_if_needed();
        sync_fields_from_voxels_if_needed();
        return engine_state_;
    }
    int current_tick() const { return tick_; }
    // Observation-only genesis/evaporation event counters (FTD-0267).
    // Reset at the top of phase_write_main_loop() and atomically incremented
    // at the genesis-manifest and evaporation decision points. Pure telemetry:
    // they touch no physics state, RNG draw, or control flow, so the golden
    // hash is preserved by construction (gated by test_render_bridge_golden).
    long long genesis_events_this_tick() const { return genesis_events_this_tick_; }
    long long evaporation_events_this_tick() const { return evaporation_events_this_tick_; }
    long long causal_projection_events_this_tick() const {
        return causal_projection_events_this_tick_;
    }
    // Observation-only per-knot telemetry (gated by toggles.knot_tracking).
    // Recorded at tick-end from settled state; reads voxels()/lattice()/
    // current_tick() only ⇒ golden-hash neutral (gated by
    // test_knot_tracking_golden). Bodies are out-of-line in render_bridge.cpp
    // because KnotTracker is forward-declared here (PIMPL, see above).
    const KnotTracker& knot_tracker() const;
    void reset_knot_tracker();
    double physical_time() const { return physical_time_; }
    double dt() const { return dt_; }
    void set_dt(double dt);

    // SOR iteration count for Poisson solvers. Default = SOR_ITERATIONS (6)
    // for interactive frame rates. Scientific benchmarks should set 20-30
    // for accurate Coulomb force law and tight Gauss constraint.
    void set_sor_iterations(int n) { sor_iterations_ = (n >= 1) ? n : 1; }
    int sor_iterations() const { return sor_iterations_; }

    // Re-seed the internal RNG (for ensemble runs with independent realizations).
    //
    // ARCH-4 (2026-04-25): now propagates to:
    //   1. The bridge's mt19937 (rng_)
    //   2. The thread_rngs_ via langevin_seed_initialized_=false reset
    //   3. The GPU cuRAND generator (when GPU backend is active)
    //   4. toggles.langevin_seed (so the next phase_write Langevin step
    //      uses the same seed)
    // Previously seed_rng() only touched (1), so the per-thread RNGs
    // and GPU cuRAND would silently keep the OLD seed — making seed-based
    // determinism unreliable across paths.
    //
    // Body in src/render_bridge.cpp (GpuEngine is forward-declared here).
    void seed_rng(unsigned int seed);

    // FTD native-charge gate: optional read-only event instrumentation.
    // Enabling is accepted only on the CPU backend; the observer consumes no
    // RNG values and does not alter the tick map.
    bool enable_history_journal(bool enabled = true);
    bool history_journal_enabled() const;
    void clear_history_events();
    std::vector<eft::HistoryEvent> history_events() const;
    std::uint64_t rng_state_hash() const;

    // FTD-0428 default-off selected engine extension. Initialization performs
    // the sole global solve (minimum-energy longitudinal dressing). Each tick
    // thereafter is local and projection-free on the matched face/edge complex.
    eft::MatchedMinimumEnergyResult initialize_matched_gauss_dynamics(
        double tolerance = 1e-12, int max_iterations = 0);
    bool matched_gauss_initialized() const;
    const eft::MatchedGaussDynamics& matched_gauss_state() const;
    bool inject_matched_transverse_edge_potential(
        int x, int y, int z, int axis, double amplitude);
    double matched_gauss_voxel_sync_residual() const;

    // Injector accessor (ARCH-1 Phase C): owns next_particle_id_ / next_pair_id_.
    // Available to inject_*_cpu free functions (and tests) so they don't need
    // friend declarations to assign IDs.
    Injector& injector() { return injector_; }
    const Injector& injector() const { return injector_; }

    // Backend accessor (ARCH-2 Phase A): which execution backend is active.
    // Currently parallel to use_gpu_ — once all 14 #ifdef blocks migrate,
    // use_gpu_ will be removed and this becomes the single source of truth.
    Backend& backend() { return *backend_; }
    const Backend& backend() const { return *backend_; }
    Backend::Kind backend_kind() const { return backend_ ? backend_->kind() : Backend::Kind::Cpu; }

    // Interactive native clients can defer the canonical full device-to-host
    // mirror until an API actually asks for host-only diagnostics. Default is
    // false so tests, campaigns, and embedders keep the established semantics.
    void set_interactive_gpu_mode(bool enabled) { interactive_gpu_mode_ = enabled; }
    bool interactive_gpu_mode() const { return interactive_gpu_mode_; }

    // ARCH-2-I: raw access to the GPU engine pointer for code paths that
    // must call GPU-specific methods (inject_*_cpu free functions forward
    // to gpu_->inject_*). Returns nullptr when no GPU is active. Prefer
    // the Backend interface for new code; this accessor is the controlled
    // escape hatch that lets injection helpers drop their friend
    // declarations without exposing the rest of RenderBridge's internals.
    //
    // ARCH-2-M: the active-backend check now goes through the Backend kind,
    // not the deleted use_gpu_ flag. force_cpu() swaps backend_ to a
    // CpuBackend, and this accessor immediately returns nullptr.
#ifdef FTD_ENABLE_CUDA
    gpu::GpuEngine* gpu_engine_ptr() {
        return (backend_ && backend_->kind() == Backend::Kind::Gpu && gpu_)
                   ? gpu_.get() : nullptr;
    }
#else
    void*           gpu_engine_ptr() { return nullptr; }
#endif

    // Force CPU-only execution (disables GPU backend even if CUDA is available).
    // Used by parity tests that need a true CPU reference.
    //
    // Env override (2026-04-23): set FTD_FORCE_GPU=1 to make this a no-op so
    // test runs stay on the GPU backend even when the test source forces CPU.
    // Parity tests become GPU-vs-GPU tautologies in that mode; that is the
    // expected trade-off when a full-GPU pass is specifically requested.
    void force_cpu() {
#ifdef FTD_ENABLE_CUDA
        if (const char* p = std::getenv("FTD_FORCE_GPU"); p && *p && *p != '0') {
            return;
        }
        // A backend switch is a synchronization boundary.  Besides preserving
        // the latest device voxel image, GpuBackend::sync_to_host() raises the
        // host Injector counters to the device lifetime high-water marks.  A
        // scan of live voxels is not sufficient here because every GPU-born
        // particle may already have evaporated or annihilated.  Flush first:
        // callers may have staged explicit particle/pair IDs through the
        // mutable voxels() API, and upload_from_host() is what raises the
        // device counters above those host-only live maxima before the final
        // device -> Injector reconciliation.
        if (backend_ && backend_->kind() == Backend::Kind::Gpu) {
            backend_->flush_host_mutations();
            backend_->sync_to_host();
        }
        // ARCH-2-M: backend_ is now the single source of truth for backend
        // selection. Previous code also set `use_gpu_ = false`; the flag
        // has been deleted.
        backend_ = std::make_unique<CpuBackend>(*this);
#endif
    }

    // Ensure host voxels_ is up-to-date with GPU device memory.
    // ARCH-2-J: now a backend dispatch; CpuBackend is a no-op.
    void sync_from_gpu() {
        if (backend_) backend_->sync_to_host();
    }

    // Compact visualization snapshots. The GPU backend implements these as
    // selective device readbacks, avoiding the 469-byte/site canonical mirror.
    void copy_visual_states(std::vector<std::int8_t>& out);
    void copy_visual_flux_magnitude(std::vector<float>& out);
    void copy_visual_flux_magnitude_plane(int axis, int index,
                                          std::vector<float>& out);
    void copy_visual_field_sample(VisualFieldKind kind, int stride,
                                  VisualFieldSample& out);
    void copy_visual_particle_attributes(const std::vector<int>& indices,
                                         std::vector<float>& out);

    // Native visual-lane capture.  This is a versioned begin/poll contract,
    // not another synchronous renderer getter: CUDA captures into persistent
    // bounded staging and leaves the canonical AoS host mirror dirty.
    bool begin_visual_snapshot(const VisualSnapshotRequest& request);
    bool visual_snapshot_ready() const;
    bool poll_visual_snapshot(VisualSnapshot& out);
    bool visual_snapshot_safe_to_replace() const;
    bool visual_snapshot_in_flight() const;

    // Physics term toggles (pedagogy system)
    TermToggles toggles;

    // ── Research overrides for the genesis constants (single-substrate path) ──
    // Lets a campaign vary the genesis threshold and the manifestation ramp
    // scale at runtime, to test whether K_GENESIS = N_c·K_B and the
    // K_MANIFEST = K_B ramp are the right choices (vs K_GENESIS = K_B, or a ramp
    // tied to the actual temperature). Default sentinels (<=0 / false) reproduce
    // the compile-time constants EXACTLY ⇒ all existing runs are byte-identical
    // and golden-safe. Only phase_write's single-substrate genesis reads these.
    double genesis_threshold_override = -1.0;  // <=0 ⇒ use compile-time K_GENESIS
    double manifest_scale_override    = -1.0;  // <=0 ⇒ use compile-time K_MANIFEST
    bool   manifest_use_temperature   = false; // true ⇒ ramp scale = toggles.langevin_T

    // EL residual verification: exposes delta_j_ computed by phase_read()
    const std::vector<Vec3>& delta_j() const { return delta_j_; }
    const std::vector<Vec3>& dJ() const { return dJ_; }

    // Recompute delta_j from current state (for EL residual verification).
    // Calls phase_read() without advancing the tick — state is NOT modified.
    void prepare_delta_j() { phase_read(); }

    // Coulomb potential (for particle EL residual verification)
    const std::vector<double>& phi_coulomb() const { return phi_coulomb_; }

    // Force diagnostics (separate buffer for cache-friendly Voxel layout)
    const std::vector<ForceDiag>& force_diag() const { return force_diag_; }

    // Scale 0 Gauge Field link variable getters. Buffers are lazily
    // allocated (revision 4.1b): 528 B/site — larger than the voxel array —
    // so nothing is paid until the gauge sector is actually used. Accessors
    // allocate-on-demand (identity links), preserving the pre-4.1b contract
    // that these always return total_sites()-sized identity-initialized arrays.
    const std::vector<SU2Link>& su2_links_x() const { ensure_gauge_links(); return su2_links_x_; }
    const std::vector<SU2Link>& su2_links_y() const { ensure_gauge_links(); return su2_links_y_; }
    const std::vector<SU2Link>& su2_links_z() const { ensure_gauge_links(); return su2_links_z_; }
    const std::vector<SU3Link>& su3_links_x() const { ensure_gauge_links(); return su3_links_x_; }
    const std::vector<SU3Link>& su3_links_y() const { ensure_gauge_links(); return su3_links_y_; }
    const std::vector<SU3Link>& su3_links_z() const { ensure_gauge_links(); return su3_links_z_; }
    // True once the link buffers exist (first accessor call, first relax
    // call, or first gauge-gated tick). Exposed so tests can pin laziness.
    bool gauge_links_allocated() const { return !su2_links_x_.empty(); }
    // Allocate + identity-initialize the 6 link buffers if not yet present.
    // const because read-only accessors must be able to materialize the
    // identity configuration (members are mutable for exactly this).
    void ensure_gauge_links() const {
        if (!su2_links_x_.empty()) return;
        const std::size_t n = lattice_.total_sites();
        su2_links_x_.resize(n); su2_links_y_.resize(n); su2_links_z_.resize(n);
        su3_links_x_.resize(n); su3_links_y_.resize(n); su3_links_z_.resize(n);
    }
    const ForceDiag& force_diag_at(int x, int y, int z) const {
        return force_diag_[lattice_.index(x, y, z)];
    }
    const ForceDiag& force_diag_at(int idx) const { return force_diag_[idx]; }

    // Run one G*-tick
    void tick();

    // Run N ticks
    void run(int num_ticks);

    // Compute diagnostics for current state
    Diagnostics diagnostics() const;

    // Rigorous energy breakdown + Gauss constraint audit
    EnergyAudit energy_audit() const;
    // Native telemetry publisher contract. GPU begins a fence-backed compact
    // reduction without blocking; CPU captures an immediately pollable
    // fallback. Group provenance is carried on TelemetrySnapshot itself.
    bool begin_telemetry_snapshot(const TelemetrySnapshotRequest& request);
    bool telemetry_snapshot_ready() const;
    bool poll_telemetry_snapshot(TelemetrySnapshot& out);
    // Used by compute_lagrangian_diagnostics() to select the fixed-size CUDA
    // reduction without making the free diagnostic API backend-aware.
    bool copy_compact_lagrangian(LagrangianDiag& out) const;
    VoxelInspection inspect_voxel(int x, int y, int z) const;
    ForceDiag inspect_force(int x, int y, int z) const;

    // FTD-0406 selected local strong T00 / Irving-Kirkwood stress allocation.
    // Recomputed from the current state on every call so direct public voxel
    // mutation cannot leave a stale gravitational source or diagnostic.
    const std::vector<StrongStressCell>& strong_stress_cells() const;
    const StrongEnergyStepDiagnostics& strong_energy_step_diagnostics() const {
        return strong_energy_step_diag_;
    }

    // Per-tick conservation bookkeeping. `update_energy_ledger()` is
    // called automatically at the end of tick() on BOTH paths:
    //   - CPU: sums are computed directly from voxel state.
    //   - GPU: gpu_sync_to_host() runs first (downloads the device
    //          voxels), then the same host-side sum executes.
    //
    // Non-interactive GPU mode materializes the canonical host snapshot for
    // this bookkeeping. Interactive mode defers it and uses compact device
    // diagnostics, avoiding the ~87 MiB voxel payload at L=64.
    //
    // Tests assert on `bridge.energy_ledger().residual` — expected
    // = −DAMPING when damping ON, 0 otherwise — and refuse regressions.
    // See EnergyLedger docstring above for the conservation formulae.
    const EnergyLedger& energy_ledger() const {
        assert_sim_thread();
        return energy_ledger_;
    }

    // Debug-only owner-thread pin for native-UI observers (SPEC_UI_V2 §11).
    // Release (NDEBUG) bodies are no-ops, so goldens stay bit-identical.
    void bind_sim_thread();
    void assert_sim_thread() const;
    void update_energy_ledger();

    // Native EFT continuity ledger for the most recent GPU tick.
    // Contains rho_before, rho_after, oriented face currents, and local
    // reaction source terms satisfying Delta rho + div I = S_reaction.
    eft::DualCellContinuity continuity_step() const;

    // Inject a localized flux source (assigns: v.flux = flux_val).
    // Used by single-shot scenarios that write each voxel exactly once.
    void inject_flux(int x, int y, int z, const Vec3& flux_val);

    // Additive flux injection: v.flux += flux_val. Used by the ported JS
    // scenarios where the injection loops deliberately hit the same voxel
    // from overlapping Gaussian kernels and rely on accumulation. Wraps
    // coordinates around the lattice (matches JS _fluxIdx semantics).
    void inject_flux_add(int x, int y, int z, const Vec3& flux_val);

    // Additive wave-velocity injection: v.wave_vel += wv_val. Equivalent to
    // JS _injectWaveVel; required to port the s0-field-* and light-* scenarios
    // that seed traveling waves by populating wave_vel directly.
    // Wraps coordinates around the lattice (matches JS).
    void inject_wave_vel_add(int x, int y, int z, const Vec3& wv_val);

    // Inject a manifested particle (spin/color default to 0 for backward compatibility)
    void inject_particle(int x, int y, int z, int8_t state, const Vec3& flux_val,
                         int8_t spin = 0, int8_t color = 0);

    // Inject a flux-aggregate wavepacket (Phase 6: spatially extended particle)
    // Sets state ±1 at center as coupling seed, distributes Gaussian flux envelope.
    // sigma: Gaussian width in lattice units (default 3.0, from Stage 1 r_eff=3.33)
    // amplitude: total flux energy normalization (default K_B)
    void inject_wavepacket(int cx, int cy, int cz, int8_t state,
                           double sigma = 3.0, double amplitude = K_B);

    // Zero all flux and wave-velocity fields. Mirrors MockBridge.clearField().
    void clearField();

    // Fill the flux field with spatially uniform random perturbations
    // of amplitude ~K_B*0.3, matching MockBridge.seedRandomFlux().
    void seedRandomFlux();

    // Discrete operators (public for Lagrangian diagnostics).
    // R6 (2026-04-18): inlined in the header — hot path, called per-voxel per-tick.
    // Bodies live in field_operators.h as free helpers.
    //
    // ARCH-1 Phase D NOTE (2026-04-25): these 8 inline methods are thin
    // delegators over the free functions in field_operators.h. New code
    // should prefer the free-function form directly:
    //     ::ftd::laplacian_flux_op(bridge.voxels(), bridge.lattice(), idx)
    // The delegators are retained for the 29 existing call sites; a future
    // mass-migration ticket (CHECKLIST_ENGINE.md) will retire them.
    inline Vec3 laplacian_flux(int idx) const  { return ::ftd::laplacian_flux_op(voxels(), lattice_, idx); }
    inline double divergence_flux(int idx) const { return ::ftd::divergence_flux_op(voxels(), lattice_, idx); }
    inline Vec3 curl_flux(int idx) const       { return ::ftd::curl_flux_op(voxels(), lattice_, idx); }
    inline Vec3 gradient_state(int idx) const  {
      sync_ternary_from_voxels_if_needed();
      return ::ftd::gradient_state_op(engine_state_.ternary, lattice_, idx);
    }
    inline Vec3 gradient_density(int idx) const { return ::ftd::gradient_density_op(voxels(), lattice_, idx); }
    inline Vec3 gradient_divergence(int idx) const { return ::ftd::gradient_divergence_op(voxels(), lattice_, idx); }
    inline Vec3 gradient_scalar(int idx, const std::vector<double>& field) const {
      return ::ftd::gradient_scalar_op(lattice_, idx, field);
    }
    inline Vec3 curl_state_velocity(int idx) const {
      sync_ternary_from_voxels_if_needed();
      return ::ftd::curl_state_velocity_op(engine_state_.ternary, voxels(), lattice_, idx);
    }

    // Hilbert space construction from current flux field
    // H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v)
    HilbertState hilbert_state() const { return HilbertState::from_flux(voxels()); }

    // SM sector operators (Phase 2). R6: inlined for hot-path performance.
    inline double compute_stress(int idx) const       { return ::ftd::stress_field<&Voxel::flux  >(voxels(), lattice_, idx); }
    inline double compute_stress_left(int idx) const  { return ::ftd::stress_field<&Voxel::flux_L>(voxels(), lattice_, idx); }
    inline double born_probability(int idx) const {
      double rho = voxels()[idx].density();
      if (rho < K_GENESIS) return 0.0;
      return 1.0 - std::exp(-(rho - K_GENESIS) / K_MANIFEST);
    }
    void create_entangled_pair(int x, int y, int z, const Vec3& flux_val);

    double compute_entropy() const;

    // Aggregate diagnostics (Phase 6)
    AggregateProfile aggregate_profile(int center_idx, double threshold = 0.01) const;

    // EM field decomposition at a single site
    EMFieldDiag em_field_at(int idx) const;

    // Hamiltonian-consistent Poynting vector S = c²(E × B) at a single site.
    // E = -wave_vel, B = ∇×J.
    Vec3 poynting_vector(int idx) const;

    // Latency (gravitational potential) field accessor.
    // When the GPU backend is active, this syncs d_phi_latency → host first
    // (Wave 5 GPU-first sweep). CPU path just returns the cached vector.
    const std::vector<double>& phi_latency() const;

private:
    // Sub-phases of a single G*-tick
    void phase_read();      // Compute delta_J from neighbors
    void phase_write();     // Commit flux, round positions
    void phase_forces();    // Compute forces on manifested particles from flux
    void phase_movement();  // Move particles between voxels
    void gauss_project();       // Gauss constraint projection (∇·J = ρ)
    void solve_coulomb_poisson(); // SOR Poisson solver for Coulomb potential
    void solve_latency_poisson(); // SOR Poisson solver for gravitational latency field

    // Tick sub-phases extracted in the 2026-04-17 callstack audit (F5)
    // so they're callable from both the CPU path (inline in tick()) and
    // the GPU path (post-sync fall-through, e.g. proper-time).
    void weak_transmutation_cpu();              // Rule 6: stress-driven polarity flip
    void accumulate_proper_time();              // Rule 8: dτ/dt = √max(1-u²/C_SPEED²-L²,0)

    // CPU ports of GPU-only physics (F2, callstack audit 2026-04-17).
    // Default-OFF toggles that previously ran silently on CPU.
    void pair_production_cpu();                 // Rule 2b: correlated ±1 pairs from high-|J| void
    void triad_binding_cpu();                   // Rule 7:  lock 3 same-sign compact triads

    // Dual-substrate helpers
    void sync_observable();                 // Set flux = flux_L + flux_R for all voxels

    // R6 (2026-04-18): laplacian_impl and stress_impl templates moved to
    // field_operators.h as inline free helpers (`laplacian_field<F>` /
    // `stress_field<F>`). Bodies that used them elsewhere reference those
    // directly. No explicit instantiation needed anymore.

    Lattice lattice_;
    mutable EngineState engine_state_;
    std::vector<Voxel> voxels_;
    std::vector<ForceDiag> force_diag_;  // Per-voxel force breakdown (populated by phase_forces)
    std::vector<Vec3> delta_j_;  // Temporary buffer for Read phase
    std::vector<Vec3> delta_j_L_;  // Dual-substrate: Read phase buffer for J_L
    std::vector<Vec3> delta_j_R_;  // Dual-substrate: Read phase buffer for J_R
    std::vector<Vec3> dJ_;         // Conjugate velocity buffer (Scale 0 Symplectic wave update)
    std::vector<double> phi_;    // Poisson potential for Gauss projection
    std::vector<double> phi_coulomb_;  // Coulomb potential (warm-started between ticks)
    std::vector<double> phi_latency_;  // Latency (gravitational) potential (warm-started)

    // Scale 0 Gauge Field link variable arrays (edge variables).
    // Lazily allocated via ensure_gauge_links() (revision 4.1b) — empty until
    // the sector is used; mutable so const accessors can materialize the
    // identity configuration on demand.
    mutable std::vector<SU2Link> su2_links_x_;
    mutable std::vector<SU2Link> su2_links_y_;
    mutable std::vector<SU2Link> su2_links_z_;
    mutable std::vector<SU3Link> su3_links_x_;
    mutable std::vector<SU3Link> su3_links_y_;
    mutable std::vector<SU3Link> su3_links_z_;
    // Jacobi double-buffer scratch for relax_su2/su3_links_cpu (race fix,
    // revision 0.9 option a): each sweep reads the previous state and writes
    // here, then the vectors swap. Persistent members so the sweep does not
    // allocate per tick after first use; sized lazily inside the relax calls.
    std::vector<SU2Link> su2_links_scratch_x_;
    std::vector<SU2Link> su2_links_scratch_y_;
    std::vector<SU2Link> su2_links_scratch_z_;
    std::vector<SU3Link> su3_links_scratch_x_;
    std::vector<SU3Link> su3_links_scratch_y_;
    std::vector<SU3Link> su3_links_scratch_z_;
    EnergyLedger energy_ledger_;  // per-tick conservation drift, populated by update_energy_ledger()
    mutable bool cpu_warnings_emitted_ = false;  // F2 callstack audit: GPU-only-toggle warning emitted flag
    std::string last_validation_warn_;  // ARCH-3: dedup repeated validate() warnings to one per unique string
    bool interactive_gpu_mode_ = false;
    std::vector<uint8_t> moved_; // Per-tick flag: prevent double-processing in phase_movement
    // ARCH-7b: pre-write flux snapshot. Populated at the start of phase_write
    // when genesis is on so that curl reads (used for spin assignment) see a
    // consistent neighbor field across all threads, eliminating the race
    // between thread-T1's curl_flux(i) read and thread-T2's voxel.flux write.
    std::vector<Vec3> flux_pre_write_;
    std::vector<uint8_t> near_particle_; // Phase D: selective damping mask (1 = near particle)
    std::vector<double> near_accel_;     // Phase D: max accel_mag of nearby particles (for Larmor)

    // PERF: per-tick scratch buffers, promoted from local-vars in tick phases
    // to bridge members so they don't malloc/free every tick. Capacity is
    // fixed once in the ctor (or grows once on first use); cleared in place.
    std::vector<unsigned int> thread_seeds_;        // phase_write: per-thread RNG seeds (PIMPL'd; see BridgeRng)
    std::vector<double>       sor_source_;          // shared scratch for all 3 SOR Poisson solvers (sized N)
    // Phase forces: ColoredSite list reused tick-to-tick (only filled when color_forces ON)
    struct ColoredSiteCache {
        int idx, cx, cy, cz;
        double px, py, pz;
        int8_t state, color;
    };
    std::vector<ColoredSiteCache> colored_sites_cache_;
    // PERF: per-tick scratch buffers for cluster integration and movement order
    std::vector<char> cluster_visited_;
    std::vector<int>  cluster_stack_;
    std::vector<int>  cluster_members_;
    std::vector<int>  movement_indices_;

    // FTD-0406 default-off CPU strong-energy projection and local stress
    // scratch. These are selected implementation state, not new substrate
    // degrees of freedom; the local cells are recomputed from each snapshot.
    mutable std::vector<StrongStressCell> strong_stress_cells_;
    StrongEnergyStepDiagnostics strong_energy_step_diag_;
    std::vector<int> strong_step_particle_ids_;
    double strong_step_h_before_ = 0.0;
    Vec3 strong_step_momentum_before_;
    bool strong_step_active_ = false;

    double self_field_injection_ = 0.0;  // Energy injected by self-field floor this tick
    int tick_ = 0;
    // FTD-0267 observation-only telemetry (see accessor docstring above).
    long long genesis_events_this_tick_ = 0;
    long long evaporation_events_this_tick_ = 0;
    long long causal_projection_events_this_tick_ = 0;
    // Observation-only per-knot telemetry (gated by toggles.knot_tracking).
    // PIMPL: KnotTracker is forward-declared in this header (circular include
    // with knot_telemetry.h); constructed in the ctor, dtor emitted in
    // render_bridge.cpp where the type is complete. Recorded at tick-end.
    std::unique_ptr<KnotTracker> knot_tracker_;
    std::unique_ptr<eft::HistoryEventJournal> history_event_journal_;
    std::unique_ptr<eft::MatchedGaussDynamics> matched_gauss_dynamics_;
    int sor_iterations_ = SOR_ITERATIONS;  // Configurable SOR iterations (default 6)
    double dt_ = 1.0;             // Time step multiplier (≥1.0). Scales damping, forces, movement.
    double physical_time_ = 0.0;  // Accumulated physical time (sum of dt_ per tick)
    // Particle/pair IDs are owned by Injector (ARCH-1 Phase C). The
    // backwards-compatible references below remain for legacy access patterns
    // but should be migrated to bridge.injector().next_particle_id() etc.
    Injector injector_;

    // ARCH-2 Phase A: execution backend. Constructed alongside (and parallel
    // to) the legacy use_gpu_ flag during the migration. force_cpu() swaps
    // backend_ to a CpuBackend; the GpuBackend (if CUDA enabled) is the
    // default.
    std::unique_ptr<Backend> backend_;

    // RNG for stochastic genesis (Born rule manifestation)
    // Genesis probability: p = 1 - exp(-(|J| - K_GENESIS) / K_MANIFEST)
    // See CLAUDE.md §4.1 — manifestation is probabilistic, not deterministic.
    // RF-9: PIMPL'd to keep <random> out of the public render_bridge.h
    // include surface. All sampling routes through BridgeRng accessors.
    std::unique_ptr<BridgeRng> rng_state_;
    bool langevin_seed_initialized_ = false;
    unsigned int active_langevin_seed_ = 0;

#ifdef FTD_ENABLE_CUDA
    // GPU backend: native CUDA builds require this path by default. Explicit
    // CPU-only builds and parity tests are the only intended CPU escapes.
    // All injection/access methods sync between host voxels_ and device memory.
    std::unique_ptr<gpu::GpuEngine> gpu_;
    // ARCH-2-M (2026-04-25): the legacy `bool use_gpu_` flag was deleted.
    // Backend selection is owned by `backend_` (declared above the buffers
    // section); query via `backend_->kind() == Backend::Kind::Gpu`.
    bool gpu_dirty_ = false;   // true = GPU has newer data than host voxels_
    mutable bool host_mutated_ = false;  // Wave 5.2: tracks non-const voxels() handouts

    // ARCH-2-J (2026-04-25): private delegators (gpu_sync_to_host,
    // gpu_push_to_device, gpu_flush_host_mutations) DELETED. Their callers
    // route through backend_->sync_to_host() / push_to_device() /
    // flush_host_mutations() directly. Implementation lives in
    // src/backend.cpp::GpuBackend.
#endif

    mutable bool ternary_dirty_from_voxels_ = false;
    mutable bool fields_dirty_from_voxels_ = false;
    std::thread::id sim_thread_{};
    bool sim_thread_bound_ = false;
    void sync_ternary_from_voxels() const;
    void sync_ternary_from_voxels_if_needed() const;
    void sync_fields_from_voxels() const;
    void sync_fields_from_voxels_if_needed() const;
    void mark_fields_dirty_from_voxels() const;
    int8_t set_state_unlocked(int idx, int8_t state);
    void record_history_event(const eft::HistoryEvent& event);
    std::vector<int> matched_state_snapshot() const;
    void sync_matched_gauss_to_voxels();
};

}  // namespace ftd
