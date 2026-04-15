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
#include "lattice.h"
#include "voxel.h"
#include "hilbert.h"
#include "term_toggles.h"

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

    // Inject a localized flux source (for testing)
    void inject_flux(int x, int y, int z, const Vec3& flux_val);

    // Inject a manifested particle (spin/color default to 0 for backward compatibility)
    void inject_particle(int x, int y, int z, int8_t state, const Vec3& flux_val,
                         int8_t spin = 0, int8_t color = 0);

    // Inject a flux-aggregate wavepacket (Phase 6: spatially extended particle)
    // Sets state ±1 at center as coupling seed, distributes Gaussian flux envelope.
    // sigma: Gaussian width in lattice units (default 3.0, from Stage 1 r_eff=3.33)
    // amplitude: total flux energy normalization (default K_B)
    void inject_wavepacket(int cx, int cy, int cz, int8_t state,
                           double sigma = 3.0, double amplitude = K_B);

    // Discrete operators (public for Lagrangian diagnostics)
    Vec3 laplacian_flux(int idx) const;
    double divergence_flux(int idx) const;
    Vec3 curl_flux(int idx) const;
    Vec3 gradient_state(int idx) const;
    Vec3 gradient_density(int idx) const;
    Vec3 gradient_divergence(int idx) const;
    Vec3 gradient_scalar(int idx, const std::vector<double>& field) const;
    Vec3 curl_state_velocity(int idx) const;  // ∇×(s·v) — Biot-Savart analog

    // Hilbert space construction from current flux field
    // H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v)
    HilbertState hilbert_state() const { return HilbertState::from_flux(voxels_); }

    // SM sector operators (Phase 2)
    double compute_stress(int idx) const;
    double compute_stress_left(int idx) const;  // Weak stress from J_L only (parity violation)
    double born_probability(int idx) const;
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

    // Dual-substrate helpers
    void sync_observable();                 // Set flux = flux_L + flux_R for all voxels

    // RF-09: pointer-to-member template for the 18-pt isotropic Laplacian.
    // Instantiated explicitly for Voxel::flux, Voxel::flux_L, Voxel::flux_R.
    template <Vec3 Voxel::*F>
    Vec3 laplacian_impl(int idx) const;

    // RF-16: pointer-to-member template for the (div+curl+grad_density) stress.
    // Instantiated explicitly for Voxel::flux and Voxel::flux_L.
    template <Vec3 Voxel::*F>
    double stress_impl(int idx) const;

    Lattice lattice_;
    std::vector<Voxel> voxels_;
    std::vector<ForceDiag> force_diag_;  // Per-voxel force breakdown (populated by phase_forces)
    std::vector<Vec3> delta_j_;  // Temporary buffer for Read phase
    std::vector<Vec3> delta_j_L_;  // Dual-substrate: Read phase buffer for J_L
    std::vector<Vec3> delta_j_R_;  // Dual-substrate: Read phase buffer for J_R
    std::vector<double> phi_;    // Poisson potential for Gauss projection
    std::vector<double> phi_coulomb_;  // Coulomb potential (warm-started between ticks)
    std::vector<double> phi_latency_;  // Latency (gravitational) potential (warm-started)
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
