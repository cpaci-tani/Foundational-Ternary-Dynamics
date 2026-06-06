#pragma once
/**
 * Render-Bridge Tick Engine
 *
 * Implements the G*-tick dynamics with Read/Write sub-phases.
 * Each tick:
 *   1. Read phase (sqrt(G*) sub-tick): compute Delta_J from 6 face neighbors
 *   2. Write phase (sqrt(G*) sub-tick): commit updated J, round to grid
 *   3. Update latency L from local density
 *   4. Enforce bandwidth: if v^2 + L^2 >= 1, scale velocity down
 *   5. Advance proper time: tau += G* * sqrt(1 - v^2 - L^2)
 */

#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <atomic>
#include <string>
#include <cstdint>
#include "ftd/injector.h"
#include "ftd/backend.h"
#include "ftd/bridge_rng.h"
#include "ftd/engine_state.h"
#include "lattice.h"
#include "voxel.h"
#include "hilbert.h"
#include "term_toggles.h"
#include "constants.h"
#include "ftd/gauge_field.h"
#include "field_operators.h"
#include "ftd/eft/dual_cell_continuity.h"

// Phase 1 of refactor sweep (2026-04-27): the 5 POD diagnostic structs
// (Diagnostics, AggregateProfile, EnergyAudit, EnergyLedger, EMFieldDiag)
// previously defined here are now in render_bridge_diagnostics.h. The
// header is re-included below so existing consumers see no API change;
// the goal is to cut TU rebuild fan-out for diagnostic-only field
// additions from ~30 TUs to ~5. See docs/adr/0008-r1-r5-phase-extraction.md.
#include "render_bridge_diagnostics.h"

#ifdef FTD_ENABLE_CUDA
namespace ftd { namespace gpu { class GpuEngine; } }
#endif

namespace ftd {

class RenderBridge {
    // ARCH-2: Backend implementations need access to GPU sync state and
    // private buffers during the migration. Friendship is the simplest path
    // while the state still lives on RenderBridge; a future cleanup will
    // move host/device sync state into GpuBackend itself.
    friend class CpuBackend;
#ifdef FTD_ENABLE_CUDA
    friend class GpuBackend;
#endif

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
    // Phase 4b (2026-04-27): phase_forces decomposition. See
    // engine/src/render_bridge_phases/phase_forces.cpp.
    friend void phase_forces_solve_potentials(RenderBridge&);
    friend void phase_forces_build_color_cache(RenderBridge&);
    friend void phase_forces_main_loop(RenderBridge&);
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
        if (backend_) {
            backend_->sync_to_host();
            backend_->mark_host_dirty();  // non-const overload — caller may mutate
        }
        ternary_dirty_from_voxels_ = true;
        fields_dirty_from_voxels_ = true;
        return voxels_;
    }
    const std::vector<Voxel>& voxels() const {
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
    const EngineState& engine_state() const {
        sync_ternary_from_voxels_if_needed();
        sync_fields_from_voxels_if_needed();
        return engine_state_;
    }
    int current_tick() const { return tick_; }
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

    // Physics term toggles (pedagogy system)
    TermToggles toggles;

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

    // Scale 0 Gauge Field link variable getters
    const std::vector<SU2Link>& su2_links_x() const { return su2_links_x_; }
    const std::vector<SU2Link>& su2_links_y() const { return su2_links_y_; }
    const std::vector<SU2Link>& su2_links_z() const { return su2_links_z_; }
    const std::vector<SU3Link>& su3_links_x() const { return su3_links_x_; }
    const std::vector<SU3Link>& su3_links_y() const { return su3_links_y_; }
    const std::vector<SU3Link>& su3_links_z() const { return su3_links_z_; }
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

    // Per-tick conservation bookkeeping. `update_energy_ledger()` is
    // called automatically at the end of tick() on BOTH paths:
    //   - CPU: sums are computed directly from voxel state.
    //   - GPU: gpu_sync_to_host() runs first (downloads the device
    //          voxels), then the same host-side sum executes.
    //
    // Cost on GPU: one PCIe download per tick (~3 MB at L=64,
    // sub-ms on modern hardware). If ever a bottleneck, a device-side
    // reduction kernel returning (E_field, E_wave, E_kin) as three
    // scalars would eliminate the download — stub comment is in
    // cuda/gpu_engine.cu near energy_audit().
    //
    // Tests assert on `bridge.energy_ledger().residual` — expected
    // = −DAMPING when damping ON, 0 otherwise — and refuse regressions.
    // See EnergyLedger docstring above for the conservation formulae.
    const EnergyLedger& energy_ledger() const { return energy_ledger_; }
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
      return 1.0 - std::exp(-(rho - K_GENESIS) / K_B);
    }
    void create_entangled_pair(int x, int y, int z, const Vec3& flux_val);

    double compute_entropy() const;

    // Aggregate diagnostics (Phase 6)
    AggregateProfile aggregate_profile(int center_idx, double threshold = 0.01) const;

    // EM field decomposition at a single site
    EMFieldDiag em_field_at(int idx) const;

    // Poynting vector S = E × B at a single site
    // E = -wave_vel, B = ∇×J → S = (-wave_vel) × (∇×J)
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
    void accumulate_proper_time();              // Rule 8: dτ/dt = √(f²-v²)/√f with f = 1-L²

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

    // Scale 0 Gauge Field link variable arrays (edge variables)
    std::vector<SU2Link> su2_links_x_;
    std::vector<SU2Link> su2_links_y_;
    std::vector<SU2Link> su2_links_z_;
    std::vector<SU3Link> su3_links_x_;
    std::vector<SU3Link> su3_links_y_;
    std::vector<SU3Link> su3_links_z_;
    EnergyLedger energy_ledger_;  // per-tick conservation drift, populated by update_energy_ledger()
    mutable bool cpu_warnings_emitted_ = false;  // F2 callstack audit: GPU-only-toggle warning emitted flag
    std::string last_validation_warn_;  // ARCH-3: dedup repeated validate() warnings to one per unique string
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
    struct ColoredSiteCache { int cx, cy, cz; int8_t state, color; };
    std::vector<ColoredSiteCache> colored_sites_cache_;

    double self_field_injection_ = 0.0;  // Energy injected by self-field floor this tick
    int tick_ = 0;
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
    // Genesis probability: p = 1 - exp(-(|J| - K_GENESIS) / K_B)
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
    void sync_ternary_from_voxels() const;
    void sync_ternary_from_voxels_if_needed() const;
    void sync_fields_from_voxels() const;
    void sync_fields_from_voxels_if_needed() const;
    void mark_fields_dirty_from_voxels() const;
    int8_t set_state_unlocked(int idx, int8_t state);
};

}  // namespace ftd
