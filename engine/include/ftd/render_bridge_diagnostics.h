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
 * ½·|·|² local density and the explicit cubic VOXEL_VOLUME measure.
 * coulomb_pe carries the canonical ½ in the Σ q·φ form.
 */

#include <cstdint>
#include "voxel.h"  // Vec3 lives here (no separate vec3.h yet)
#include "volumetric_measure.h"

namespace ftd {

struct Diagnostics {
    int tick = 0;
    double total_flux = 0.0;
    // Sum of |born_infeld_core| over all sites — NOT the accounted energy
    // budget (see EnergyAudit.dynamic_energy / total_energy).
    double total_energy = 0.0;
    double avg_drag = 0.0;
    double max_bandwidth = 0.0;
    double max_causal_budget = 0.0;
    long long causal_projection_events = 0;
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
    bool requested = false;        // the toggles asking for gravity are ON
    bool active = false;           // latency machinery on AND a non-trivial field
                                   // requested && !active distinguishes "term
                                   // off" from "term on but produced nothing" --
                                   // the latter is how a backend that does not
                                   // implement the term (field_energy_gravity
                                   // has no CUDA read site) used to read as
                                   // simply inactive. P4, 2026-07-26.
};

struct EnergyAudit {
    // Field-amplitude norm, not the gradient-plus-cross Hamiltonian of the
    // production wave tick. See FTD-0293 and FTD-0452.
    double field_energy = 0.0;     // sum [½|J|² · V_cell] over all sites
    double wave_energy = 0.0;      // sum [½|wave_vel|² · V_cell] over all sites
    double particle_ke = 0.0;      // sum (gamma_0-1)·E_REST
    double total_energy = 0.0;     // accounted total: field + wave + particle energy
    double gauss_violation = 0.0;  // sum |div(J) - state|^2
    double max_gauss_error = 0.0;  // max |div(J) - state|
    double self_field_injection = 0.0;  // Energy injected by self-field floor this tick
    double coulomb_pe = 0.0;       // ½·sum α·s·φ_C (electrostatic PE; pair-PE convention)
    double E_field_energy = 0.0;   // sum ½·|E|^2 (electric field energy)
    double B_field_energy = 0.0;   // sum (c²/2)·|B|² (magnetic field energy)
    int charge_total = 0;          // sum of states (should be conserved)
    int manifested_count = 0;      // particle count
    Vec3 total_poynting;           // Σ S(v) = Σ c²(E(v) × B(v))

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

    // FTD-0402 append-only mass-role / flat energy-momentum diagnostics.
    // Interaction energies remain incomplete until NCEMC; dynamic_energy is
    // the rest-offset-free channel used by conservation charts.
    double particle_rest_energy = 0.0;
    double particle_energy = 0.0;
    Vec3 particle_momentum;
    double dynamic_energy = 0.0;

    // FTD-0404 append-only density/integral metadata. At the production
    // unit edge V_cell=1, so the density sums equal their integrated channels
    // exactly; keeping both names prevents that unit choice becoming ontology.
    double cell_volume = VOXEL_VOLUME;
    double field_energy_density_sum = 0.0;
    double wave_energy_density_sum = 0.0;

    // FTD-0406 append-only selected strong Hamiltonian / gravity contract.
    double strong_potential_energy = 0.0;   // sum_{i<j} U_ij, U_ij(1)=0
    double strong_gravitational_mass = 0.0; // strong_potential_energy/C_SPEED^2
    double strong_projection_residual = 0.0;
    double strong_projection_lambda = 1.0;
    int strong_projection_events = 0;
    int strong_projection_failures = 0;
    int strong_topology_failures = 0;
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
    std::uint64_t updates = 0;       // number of completed ledger updates
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

// One-site native inspector snapshot.  Keeping the derived stencil values in
// the same compact request prevents inspector hover/click events from
// materializing the entire CUDA lattice merely to read one Voxel and its six
// face-neighbor fluxes.
struct VoxelInspection {
    Voxel voxel;
    double divergence = 0.0;
    Vec3 curl;
    EMFieldDiag em;
};

// ============================================================================
// Scale-context readout admissibility gate (C_scale)
// Canonical spec: docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md.
//
// These POD types are the *result* of a read-only diagnostics layer that
// decides whether an engine cloud is eligible for public physical readout
// (e.g. the Koopman α estimator). The layer is BLIND to α by contract — see
// the invariant note in scale_context.h. Behaviour (config + measurement +
// tracker) lives in scale_context.{h,cpp}; only the POD results live here so
// they stay trivially copyable / Embind-bindable like the other diagnostics.
// ============================================================================

// Geometric classification of a cloud relative to the lattice scale a and box
// scale L (per SPEC_SCALE_CONTEXT_READOUT §1–§2). Order matters only for the
// enum value; the classifier (scale_context.cpp) applies a fixed priority.
enum class ScaleRegime : int {
    Indeterminate     = 0,  // default (zero-init) — gate not evaluated
    Evaporating       = 1,  // R→0 / support collapsing to vacuum
    UVLocked          = 2,  // κ = R_eff/a too small (cloud ~ a single voxel)
    BoundedAdmissible = 3,  // golden window 1 ≪ R_eff/a ≪ L satisfied
    ShellDominated    = 4,  // β = δ_shell/R_eff too large (surface beats volume)
    Percolating       = 5,  // ζ = R_eff/L too large (cloud ~ box / phase transition)
};

// Public-readout eligibility verdict. DiagnosticOnly is the observe-only
// default (gate not armed); the three Rejected* values record WHY a cloud is
// inadmissible (SPEC_SCALE_CONTEXT_READOUT §2 scale, §3 self-confinement,
// §2.2 stationarity).
enum class ReadoutStatus : int {
    DiagnosticOnly          = 0,  // gate not armed — annotation only
    Admissible              = 1,  // passed scale + self-confinement + stationarity
    RejectedScaleContext    = 2,  // failed golden-window / volume / shell tests
    RejectedSelfConfinement = 3,  // no stable flux-balance fixed point
    RejectedNonStationary   = 4,  // still relaxing (|dR/dt| or |dJ²/dt| too large)
};

// One snapshot of the scale-context measurement. All quantities are derived
// purely from lattice geometry and the flux field |J|² (the "ρ" of the spec's
// R_eff² = Σ ρ|v−v_c|² / Σ ρ) plus the observation-only genesis/evaporation
// counters. No coupling, no α, no Koopman eigenvalue ever enters here.
struct ScaleContextDiagnostics {
    int    tick = 0;
    int    L    = 0;
    double a    = 1.0;              // unit lattice spacing (voxel scale)

    // ---- support / occupancy ----
    int    support_count   = 0;    // voxels with |J|² ≥ energy_threshold (∪ optional state≠0)
    double active_fraction = 0.0;  // support_count / L³  (= f_active)
    double cloud_energy    = 0.0;  // ½·Σ_support |J|²  (canonical ½ convention)

    // ---- center (PBC circular mean) ----
    double center_x = 0.0, center_y = 0.0, center_z = 0.0;
    bool   center_well_defined  = false; // false ⇒ delocalized / box-filling
    double center_concentration = 0.0;   // min over axes of resultant length R̄∈[0,1]

    // ---- geometry / dimensionless ratios ----
    double R_eff       = 0.0;      // PBC energy-weighted second moment about circular center
    double kappa       = 0.0;      // R_eff / a   (lattice decoupling)
    double zeta        = 0.0;      // R_eff / L   (finite-volume decoupling)
    double beta        = 0.0;      // δ_shell / R_eff (surface-to-volume)
    double delta_shell = 0.0;      // r90 − r50 (radial energy-quantile spread)
    double peak_density = 0.0;     // max |J| over support
    double r50 = 0.0, r90 = 0.0;   // radial energy quantiles (audit)

    // ---- self-confinement (flux balance at the R_eff shell) ----
    double phi_outward = 0.0;      // Σ max(0,  J·r̂) in the boundary shell
    double phi_return  = 0.0;      // Σ max(0, −J·r̂) in the boundary shell
    double phi_balance = 0.0;      // phi_outward − phi_return
    double phi_balance_norm = 0.0; // |Φout−Φret| / (Φout+Φret+ε)
    double dPhi_dR = 0.0;          // local slope of (Φout−Φret) across adjacent shells
    bool   confinement_fixed_point = false; // balance small AND dPhi_dR < 0

    // ---- boundary susceptibility (this tick; pure telemetry) ----
    long long genesis_events     = 0;
    long long evaporation_events = 0;
    double    B_t = 0.0;           // genesis − evaporation events this tick

    // ---- rolling (filled by ScaleContextTracker; 0 from a single-shot measure) ----
    double dR_dt     = 0.0;
    double dJ2_dt    = 0.0;        // relative rate (slope / mean)
    double J2_total  = 0.0;        // Σ_support |J|² (NO ½ — matches the Koopman j2 column)
    double tau_cloud = 0.0;        // estimated relaxation time (advisory, noisy)
    double Theta     = 0.0;        // tau_cloud / tau_bath
    bool   stationary = false;     // |dR_dt|,|dJ2_dt|,|⟨B(t)⟩| all below tolerance

    // ---- verdict ----
    ScaleRegime   regime = ScaleRegime::Indeterminate;
    ReadoutStatus status = ReadoutStatus::DiagnosticOnly;
};

}  // namespace ftd
