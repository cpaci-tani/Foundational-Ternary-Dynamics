#pragma once
/**
 * @file engine/include/ftd/render_bridge_diagnostics.h
 * @purpose POD diagnostic structs returned by RenderBridge inspection methods.
 * @consumers engine/src/diagnostics_compute.cpp, energy_ledger_compute.cpp,
 *            engine/wasm/bindings_render_bridge.cpp, engine/tests/test_*.cpp,
 *            and anywhere a test or audit reads engine state without needing
 *            the full RenderBridge class definition.
 * @related engine/include/ftd/render_bridge.h (re-includes this header)
 *
 * Phase 1 of the refactor sweep extracted these 5 structs from
 * render_bridge.h to cut TU rebuild fan-out: pure POD diagnostic field
 * additions previously forced ~30 TUs to recompile because every consumer
 * of RenderBridge transitively depended on the struct definitions.
 *
 * The structs MUST stay POD (no member functions, no inheritance, no
 * non-trivial constructors) so they remain trivially copyable across
 * thread boundaries and trivially bound to WASM via Embind.
 *
 * Convention reminder (CONTRACTS.md §6 Energy Convention Contract):
 * field_energy / wave_energy / E_L_total / E_R_total / wv_L_total /
 * wv_R_total / E_field_energy / B_field_energy all carry the canonical
 * ½·|·|² factor. coulomb_pe carries the canonical ½ in the Σ q·φ form.
 */

#include <cstdint>
#include "voxel.h"  // Vec3 lives here (no separate vec3.h yet)

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

// Phase 2 (gravity panel): aggregate of the REAL C++ latency field
// (voxel.latency from the Poisson solver), distinct from the |J|² web proxy.
struct GravityMetricAgg {
    double latency_max = 0.0;      // max voxel.latency (real gravity potential L)
    double latency_mean = 0.0;     // mean over voxels with L>0
    double f_min = 1.0;            // min lapse f = 1 - L_max² (deepest dilation)
    double gamma_max = 1.0;        // max gamma_ftd()
    double dilation_max_pct = 0.0; // (1 - sqrt(f_min))·100
    int voxel_count = 0;           // voxels with latency > 0
    bool active = false;           // latency machinery on AND a non-trivial field
};

struct EnergyAudit {
    double field_energy = 0.0;     // ½·sum |J|^2 over all sites (canonical ½ convention)
    double wave_energy = 0.0;      // ½·sum |wave_vel|^2 over all sites
    double particle_ke = 0.0;      // sum ½·|v|^2 for manifested particles
    double total_energy = 0.0;     // field + wave + particle_ke
    double gauss_violation = 0.0;  // sum |div(J) - state|^2
    double max_gauss_error = 0.0;  // max |div(J) - state|
    double self_field_injection = 0.0;  // Energy injected by self-field floor this tick
    double coulomb_pe = 0.0;       // ½·sum α·s·φ_C (electrostatic PE; pair-PE convention)
    double E_field_energy = 0.0;   // sum ½·|E|^2 (electric field energy)
    double B_field_energy = 0.0;   // sum ½·|B|^2 (magnetic field energy)
    int charge_total = 0;          // sum of states (should be conserved)
    int manifested_count = 0;      // particle count
    Vec3 total_poynting;           // Σ S(v) = Σ E(v) × B(v) (Poynting vector)

    // Dual-substrate diagnostics (only populated when dual_substrate=true)
    double E_L_total = 0.0;        // ½·sum |J_L|^2 (left substrate energy)
    double E_R_total = 0.0;        // ½·sum |J_R|^2 (right substrate energy)
    double wv_L_total = 0.0;       // ½·sum |wave_vel_L|^2 (left wave energy)
    double wv_R_total = 0.0;       // ½·sum |wave_vel_R|^2 (right wave energy)
    double chirality_total = 0.0;  // sum chi (chirality density)

    // Strong Field
    double strong_energy = 0.0;    // sum |J_strong|^2 (strong field energy)

    // Weak Field
    double weak_energy = 0.0;      // sum |J_weak|^2 (weak field energy)
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

}  // namespace ftd
