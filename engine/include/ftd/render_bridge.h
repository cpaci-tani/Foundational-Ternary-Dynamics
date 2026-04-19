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
#include <random>
#include <memory>
#include <cmath>
#include "lattice.h"
#include "voxel.h"
#include "hilbert.h"
#include "term_toggles.h"
#include "constants.h"
#include "field_operators.h"

#ifdef FTD_ENABLE_CUDA
namespace ftd { namespace gpu { class GpuEngine; } }
#endif

namespace ftd {

struct Diagnostics {
    int tick = 0;
    double total_flux = 0.0;
    double total_energy = 0.0;
    double avg_drag = 0.0;
    double max_bandwidth = 0.0;
    int manifested_count = 0;
    int positive_count = 0;
    int negative_count = 0;
    double total_entropy = 0.0;
    // Spin-statistics diagnostics
    int spin_up_count = 0;
    int spin_down_count = 0;
    int color_count[4] = {0, 0, 0, 0};  // [0]=colorless, [1]=R, [2]=G, [3]=B
    // Angular momentum diagnostics
    Vec3 total_angular_momentum;  // L = sum_i r_i x (m_i * v_i)
};

// Phase 6: Aggregate profile for spatially extended flux structures
struct AggregateProfile {
    Vec3 center_of_mass;        // flux-weighted center
    double total_energy = 0.0;  // sum |J|^2 within region
    double effective_radius = 0.0; // sqrt(sum r^2|J|^2 / sum |J|^2)
    double peak_density = 0.0;  // max |J| in aggregate
    double radial_profile[20] = {}; // avg |J| at r = 1..20
    int site_count = 0;         // sites with |J| > threshold
};

struct EnergyAudit {
    double field_energy = 0.0;     // sum |J|^2 over all sites
    double wave_energy = 0.0;      // sum |wave_vel|^2 over all sites
    double particle_ke = 0.0;      // sum 0.5*|v|^2 for manifested particles
    double total_energy = 0.0;     // field + wave + particle_ke
    double gauss_violation = 0.0;  // sum |div(J) - state|^2
    double max_gauss_error = 0.0;  // max |div(J) - state|
    double self_field_injection = 0.0;  // Energy injected by self-field floor this tick
    double coulomb_pe = 0.0;       // sum alpha * s * phi_C (electrostatic PE)
    double E_field_energy = 0.0;   // sum |E|^2 / 2 (electric field energy)
    double B_field_energy = 0.0;   // sum |B|^2 / 2 (magnetic field energy)
    int charge_total = 0;          // sum of states (should be conserved)
    int manifested_count = 0;      // particle count
    Vec3 total_poynting;           // Σ S(v) = Σ E(v) × B(v) (Poynting vector)

    // Dual-substrate diagnostics (only populated when dual_substrate=true)
    double E_L_total = 0.0;        // sum |J_L|^2 (left substrate energy)
    double E_R_total = 0.0;        // sum |J_R|^2 (right substrate energy)
    double wv_L_total = 0.0;       // sum |wave_vel_L|^2 (left wave energy)
    double wv_R_total = 0.0;       // sum |wave_vel_R|^2 (right wave energy)
    double chirality_total = 0.0;  // sum chi (chirality density)
};

/**
 * EnergyLedger — per-tick conservation bookkeeping.
 *
 * Tracks total energy tick-over-tick so tests can assert:
 *   - With damping OFF:  |ΔE / E| < epsilon           (strict conservation)
 *   - With damping ON:   |ΔE / E + γ| < epsilon       (expected dissipation rate)
 *
 * Populated by RenderBridge::update_energy_ledger() at the end of each
 * tick. Read via RenderBridge::energy_ledger(). Kept separate from
 * EnergyAudit (which is a one-shot snapshot) to avoid muddling the
 * "current state" and "flow between ticks" concepts.
 *
 * Epistemic purpose: addresses the long-standing gap that long engine
 * runs drift by an unknown amount with no assertion. Tests can now ratchet
 * on drift_per_tick and refuse to land regressions.
 */
struct EnergyLedger {
    int    tick_prev = -1;           // tick number of the previous snapshot
    double E_prev    = 0.0;          // total energy at previous tick
    double E_curr    = 0.0;          // total energy at current tick
    double dE_dt     = 0.0;          // (E_curr − E_prev) / dt
    double drift_frac = 0.0;         // (E_curr − E_prev) / max(|E_prev|, ε)
    double expected_rate = 0.0;      // −DAMPING when damping on, 0 otherwise
    double residual  = 0.0;          // drift_frac − expected_rate (conservation violation)

    // Running accumulators over the whole sim (useful for test harnesses):
    double cumulative_injection = 0.0;  // self-field + manifestation input
    double cumulative_dissipation = 0.0; // damping loss
    double max_residual_seen = 0.0;     // worst-case |residual| across run
};

// EM field decomposition at a single site
// E = -∂J/∂t ≈ -wave_vel (leapfrog momentum variable)
// B = ∇×J (curl of flux field)
struct EMFieldDiag {
    Vec3 E;                        // Electric field
    Vec3 B;                        // Magnetic field
    double E_mag = 0.0;
    double B_mag = 0.0;
};

class RenderBridge {
    // Extracted physics modules (2026-04-18 refactor R1-R6) need access to
    // internal state (rng_, uniform_, next_particle_id_, phi_, buffers).
    friend void weak_transmutation_cpu(RenderBridge&);
    friend void accumulate_proper_time(RenderBridge&);
    friend void pair_production_cpu(RenderBridge&);
    friend void triad_binding_cpu(RenderBridge&);
    friend void update_energy_ledger_cpu(RenderBridge&);
    friend void inject_flux_cpu(RenderBridge&, int, int, int, const Vec3&);
    friend void inject_flux_add_cpu(RenderBridge&, int, int, int, const Vec3&);
    friend void inject_wave_vel_add_cpu(RenderBridge&, int, int, int, const Vec3&);
    friend void inject_particle_cpu(RenderBridge&, int, int, int, int8_t,
                                    const Vec3&, int8_t, int8_t);
    friend void inject_wavepacket_cpu(RenderBridge&, int, int, int, int8_t, double, double);
    friend void create_entangled_pair_cpu(RenderBridge&, int, int, int, const Vec3&);

public:
    RenderBridge(int lattice_size);
    ~RenderBridge();

    // Access (GPU-aware: syncs device→host when needed)
    Lattice& lattice() { return lattice_; }
    const Lattice& lattice() const { return lattice_; }
    std::vector<Voxel>& voxels() {
#ifdef FTD_ENABLE_CUDA
        gpu_sync_to_host();
        // Wave 5.2: callers of the non-const overload may mutate the vector
        // (e.g. tests doing `voxels()[idx].locked = true` between ticks).
        // Flag it so the next tick() pushes the host state back to the GPU.
        host_mutated_ = true;
#endif
        return voxels_;
    }
    const std::vector<Voxel>& voxels() const {
#ifdef FTD_ENABLE_CUDA
        const_cast<RenderBridge*>(this)->gpu_sync_to_host();
#endif
        return voxels_;
    }
    Voxel& voxel_at(int x, int y, int z) {
#ifdef FTD_ENABLE_CUDA
        gpu_sync_to_host();
        host_mutated_ = true;  // Wave 5.2: same reason as voxels() non-const
#endif
        return voxels_[lattice_.index(x, y, z)];
    }
    int current_tick() const { return tick_; }
    double physical_time() const { return physical_time_; }
    double dt() const { return dt_; }
    void set_dt(double dt) { dt_ = (dt >= 1.0) ? dt : 1.0; }

    // Re-seed the internal RNG (for ensemble runs with independent realizations)
    void seed_rng(unsigned int seed) { rng_.seed(seed); }

    // Force CPU-only execution (disables GPU backend even if CUDA is available).
    // Used by parity tests that need a true CPU reference.
    void force_cpu() {
#ifdef FTD_ENABLE_CUDA
        use_gpu_ = false;
#endif
    }

    // Ensure host voxels_ is up-to-date with GPU device memory.
    void sync_from_gpu() {
#ifdef FTD_ENABLE_CUDA
        gpu_sync_to_host();
#endif
    }

    // Physics term toggles (pedagogy system)
    TermToggles toggles;

    // EL residual verification: exposes delta_j_ computed by phase_read()
    const std::vector<Vec3>& delta_j() const { return delta_j_; }

    // Recompute delta_j from current state (for EL residual verification).
    // Calls phase_read() without advancing the tick — state is NOT modified.
    void prepare_delta_j() { phase_read(); }

    // Coulomb potential (for particle EL residual verification)
    const std::vector<double>& phi_coulomb() const { return phi_coulomb_; }

    // Force diagnostics (separate buffer for cache-friendly Voxel layout)
    const std::vector<ForceDiag>& force_diag() const { return force_diag_; }
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
    inline Vec3 laplacian_flux(int idx) const  { return ::ftd::laplacian_flux_op(voxels_, lattice_, idx); }
    inline double divergence_flux(int idx) const { return ::ftd::divergence_flux_op(voxels_, lattice_, idx); }
    inline Vec3 curl_flux(int idx) const       { return ::ftd::curl_flux_op(voxels_, lattice_, idx); }
    inline Vec3 gradient_state(int idx) const  { return ::ftd::gradient_state_op(voxels_, lattice_, idx); }
    inline Vec3 gradient_density(int idx) const { return ::ftd::gradient_density_op(voxels_, lattice_, idx); }
    inline Vec3 gradient_divergence(int idx) const { return ::ftd::gradient_divergence_op(voxels_, lattice_, idx); }
    inline Vec3 gradient_scalar(int idx, const std::vector<double>& field) const {
      return ::ftd::gradient_scalar_op(lattice_, idx, field);
    }
    inline Vec3 curl_state_velocity(int idx) const { return ::ftd::curl_state_velocity_op(voxels_, lattice_, idx); }

    // Hilbert space construction from current flux field
    // H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v)
    HilbertState hilbert_state() const { return HilbertState::from_flux(voxels_); }

    // SM sector operators (Phase 2). R6: inlined for hot-path performance.
    inline double compute_stress(int idx) const       { return ::ftd::stress_field<&Voxel::flux  >(voxels_, lattice_, idx); }
    inline double compute_stress_left(int idx) const  { return ::ftd::stress_field<&Voxel::flux_L>(voxels_, lattice_, idx); }
    inline double born_probability(int idx) const {
      double rho = voxels_[idx].density();
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
    std::vector<Voxel> voxels_;
    std::vector<ForceDiag> force_diag_;  // Per-voxel force breakdown (populated by phase_forces)
    std::vector<Vec3> delta_j_;  // Temporary buffer for Read phase
    std::vector<Vec3> delta_j_L_;  // Dual-substrate: Read phase buffer for J_L
    std::vector<Vec3> delta_j_R_;  // Dual-substrate: Read phase buffer for J_R
    std::vector<double> phi_;    // Poisson potential for Gauss projection
    std::vector<double> phi_coulomb_;  // Coulomb potential (warm-started between ticks)
    std::vector<double> phi_latency_;  // Latency (gravitational) potential (warm-started)
    EnergyLedger energy_ledger_;  // per-tick conservation drift, populated by update_energy_ledger()
    mutable bool cpu_warnings_emitted_ = false;  // F2 callstack audit: GPU-only-toggle warning emitted flag
    std::vector<uint8_t> moved_; // Per-tick flag: prevent double-processing in phase_movement
    std::vector<uint8_t> near_particle_; // Phase D: selective damping mask (1 = near particle)
    std::vector<double> near_accel_;     // Phase D: max accel_mag of nearby particles (for Larmor)

    // PERF: per-tick scratch buffers, promoted from local-vars in tick phases
    // to bridge members so they don't malloc/free every tick. Capacity is
    // fixed once in the ctor (or grows once on first use); cleared in place.
    std::vector<unsigned int> thread_seeds_;        // phase_write: per-thread RNG seeds
    std::vector<std::mt19937> thread_rngs_;         // phase_write: per-thread RNGs (replaces per-voxel construction)
    std::vector<double>       sor_source_;          // shared scratch for all 3 SOR Poisson solvers (sized N)
    // Phase forces: ColoredSite list reused tick-to-tick (only filled when color_forces ON)
    struct ColoredSiteCache { int cx, cy, cz; int8_t state, color; };
    std::vector<ColoredSiteCache> colored_sites_cache_;

    double self_field_injection_ = 0.0;  // Energy injected by self-field floor this tick
    int tick_ = 0;
    double dt_ = 1.0;             // Time step multiplier (≥1.0). Scales damping, forces, movement.
    double physical_time_ = 0.0;  // Accumulated physical time (sum of dt_ per tick)
    int next_pair_id_ = 0;  // Counter for entangled pair IDs
    int next_particle_id_ = 0;  // Monotonic counter for particle IDs (thread-safe via local atomic in phase_write)

    // RNG for stochastic genesis (Born rule manifestation)
    // Genesis probability: p = 1 - exp(-(|J| - K_GENESIS) / K_B)
    // See CLAUDE.md §4.1 — manifestation is probabilistic, not deterministic.
    std::mt19937 rng_{42};
    std::uniform_real_distribution<double> uniform_{0.0, 1.0};

#ifdef FTD_ENABLE_CUDA
    // GPU backend: when available, tick() delegates to GpuEngine for speedup.
    // All injection/access methods sync between host voxels_ and device memory.
    std::unique_ptr<gpu::GpuEngine> gpu_;
    bool use_gpu_ = false;
    bool gpu_dirty_ = false;   // true = GPU has newer data than host voxels_
    mutable bool host_mutated_ = false;  // Wave 5.2: tracks non-const voxels() handouts

    void gpu_sync_to_host();   // Download GPU state to voxels_
    void gpu_push_to_device(); // Upload voxels_ to GPU
    // Wave 5.2: invoked at the start of tick() to push out-of-band host writes
    // (e.g. voxels()[idx].locked = true) back to the device before the next tick.
    void gpu_flush_host_mutations();
#endif
};

}  // namespace ftd
