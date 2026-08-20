#pragma once
/**
 * FTD Render-Bridge Constants
 *
 * Engine-facing interface to the ontic constants registry. It mixes exact
 * identities with tagged selections and calibrations; not all values derive
 * from {D=3, ϖ}.
 *
 * This header re-exports the ontic constants into the ftd:: namespace
 * so existing engine code (render_bridge.cpp, lagrangian.h, etc.) can
 * use them without qualification. The derivation logic lives in ontic.h.
 *
 * See docs/theory/SPEC_FTD_LAGRANGIAN.md v2.0.
 */

#include "ontic.h"
#include "ftd/constants_shared.h"
#include <algorithm>
#include <cmath>


namespace ftd {

// ============================================================================
// Engine version — SINGLE SOURCE OF TRUTH (revision 6.1).
// Was prose-only in SPEC_ENGINE.md ("2.18.0") while CMake said
// project(... VERSION 1.0) and nothing was exposed at runtime. This constant
// is now the canonical value; CMake mirrors it, SPEC cites it, ftd_sim
// --version and the WASM getEngineVersion() binding return it.
// Bump here on every release; keep CMakeLists project(VERSION ...) in sync.
// ============================================================================
inline constexpr int         ENGINE_VERSION_MAJOR = 2;
inline constexpr int         ENGINE_VERSION_MINOR = 18;
inline constexpr int         ENGINE_VERSION_PATCH = 0;
inline constexpr const char* ENGINE_VERSION       = "2.18.0";

// ============================================================================
// Re-export ontic constants into ftd:: namespace for engine use
// ============================================================================

// Layer -1: Self-referential seed
using ontic::EULER_E;

// Layer 0: Transcendental seeds
using ontic::EULER_GAMMA;
using ontic::GAMMA_QUARTER;

// Layer 0b: Modular selection
using ontic::NOME_LEMNISCATIC;
using ontic::THETA_LEMNISCATIC;

// Layer 1: Elliptic geometry
using ontic::GAUSS_CONSTANT_M;
using ontic::PI;
using ontic::VARPI;

// Layer 2: Lemniscatic identities and legacy power aliases
using ontic::G_STAR;
using ontic::GSTAR_ACTION;
using ontic::GSTAR_FLUX;
using ontic::GSTAR_TIME;
using ontic::PF;
using ontic::SQRT_GSTAR;

// Layer 2b: Generalized-quadratic discriminant
using ontic::K_CRIT;
using ontic::X_BORN;

// Layer 3: Master quadratic
using ontic::COEFFICIENT;
using ontic::X_MINUS;
using ontic::X_PLUS;
using ontic::X_PLUS_PRECISION;

// Layer 3c: Charge-space duality
using ontic::E2_COLOR;

// Layer 3b: Dual-substrate decomposition
using ontic::DELTA_APPROX;
using ontic::DELTA_SQUARED;
using ontic::E_LEFT_APPROX;
using ontic::E_PRODUCT;
using ontic::E_RIGHT_APPROX;
using ontic::E_SUM;
using ontic::MATTER_FRACTION;
using ontic::OMEGA_LAMBDA_CONJ;
using ontic::VACUUM_FRACTION;

// Layer 4: Framework integers
using ontic::B_3;
using ontic::D_CONSTRAINT;
using ontic::D_SPATIAL;
using ontic::LADDER_ELECTRON;
using ontic::LADDER_GRAVITY;
using ontic::LADDER_HIGGS;
using ontic::LADDER_NEUTRINO;
using ontic::LADDER_PERTURBATIVE;
using ontic::N_BASE;
using ontic::N_C;
using ontic::N_EFF;
using ontic::N_F;
using ontic::N_GEN;

// Layer 4b: Neutrino mixing
using ontic::DM2_RATIO;
using ontic::NORMAL_HIERARCHY;
using ontic::SIN2_THETA12;
using ontic::SIN2_THETA13;
using ontic::SIN2_THETA23;

// Layer 5: Coupling constants
using ontic::ALPHA;             // = 1/X_PLUS_PRECISION (CODATA match; 2026-04-17)
using ontic::ALPHA_G_APPROX;
using ontic::ALPHA_PRECISION;   // alias to ALPHA (same value)
using ontic::ALPHA_TREE;        // = 1/X_PLUS (tree-level, reference only)
using ontic::ALPHA_WEAK;
using ontic::G_C;
using ontic::G_N;
using ontic::G_PE;
using ontic::G_DERIVED;
using ontic::ALPHA_G_ELECTRON;
using ontic::M_PLANCK_MEV;
using ontic::SIN2_WEINBERG;

// ──────────────────────────────────────────────────────────────────────
// ALPHA_EFT: two-vertex coupling used at every force-computation site.
//
// HONEST FRAMING:
//   G_C was DEFINED as √α in ontic.h Layer 5 (the state-flux coupling
//   in the Born-Infeld Lagrangian). So ALPHA_EFT = G_C² = α is an
//   algebraic identity by construction — this is a CONSISTENCY CHECK,
//   not a derivation. The static_assert below catches drift between
//   the two definitions, it does NOT prove new physics.
//
//   α is NOT derived here or anywhere in FTD. The split (stated canonically
//   at ontic/gauge_couplings.h:32-35) is:
//     • the master quadratic x² − 16G*²x + 16G*³ = 0 and its root
//       X_PLUS = 137.036… are pure algebra — [THEOREM];
//     • the *identification* x₊ ≡ 1/α is [STRONGLY MOTIVATED CONJECTURE]
//       (LEDGER FTD-0013). No derivation chain reaches it.
//   The 4-term corrected root X_PLUS_PRECISION agrees with CODATA 2022 to
//   ~3e-4 ppt, but that is a post-hoc four-coefficient fit quoted ~500×
//   finer than CODATA 2022's own 153 ppt uncertainty — a [CONJECTURE], not a
//   precision derivation (see gauge_couplings.h:40-43, "Do NOT re-tag this
//   line [THEOREM]"). The engine's ALPHA has used that precision value since
//   2026-04-17 (TRACKER §1.5); α is an INPUT to the engine, never an output.
//
// WHAT THIS MEANS FOR THE ENGINE:
//   Force code may use either ALPHA or ALPHA_EFT — they are equal by
//   construction. Prefer ALPHA_EFT in code paths where the two-vertex
//   picture is pedagogically helpful (e.g., the EFT emergent force mode
//   where flux field + state probe each contribute one vertex).
inline constexpr double ALPHA_EFT = G_C * G_C;
static_assert(ALPHA_EFT > 0.00729 && ALPHA_EFT < 0.00731,
              "Consistency: G_C^2 must equal alpha (G_C was defined as sqrt(alpha))");
static_assert(ALPHA_EFT > 0.99999999 * ontic::ALPHA &&
              ALPHA_EFT < 1.00000001 * ontic::ALPHA,
              "Consistency: G_C hardcoded value must match sqrt(ALPHA) to 1e-8");

// Layer 5b: QCD sector
using ontic::ALPHA_S_MZ;
using ontic::alpha_s_running;
using ontic::B0_NF5;
using ontic::B0_NF6;
using ontic::LAMBDA_QCD;
using ontic::M_Z;

// Layer 6: Mass scale
using ontic::K_B;
using ontic::M_INERTIAL;
using ontic::E_REST;
using ontic::M_GRAVITATIONAL;
using ontic::M_REST;     // compatibility alias only; production consumers must name a role
using ontic::K_MANIFEST; // genesis/evaporation kinetics scale; unified-mass Phase 0
                         // NOT equal to K_B: since FTD-0388 K_MANIFEST := W_SC =
                         // 0.50546..., which is -1.084% from K_B = 0.511. The old
                         // "(= K_B)" asserted the exact role conflation FTD-0130/0388
                         // split apart (mass anchor vs kinetics scale).
using ontic::K_GENESIS;

// Layer 6c: Mass ratios
using ontic::M_PROTON;
using ontic::MU_RATIO;
using ontic::PROTON_RATIO;
using ontic::R_BOHR;
using ontic::TAU_RATIO;

// Layer 6b: Electroweak scale (Higgs)
using ontic::LAMBDA_HIGGS;
using ontic::M_HIGGS;
using ontic::V_HIGGS;

// Lattice Representation of VEV and W/Z Mass
// V_HIGGS is 246.09 GeV. K_B is 0.511 MeV.
// Therefore V_HIGGS_LATTICE = V_HIGGS * 1000.0 / K_B (scaled by the electron mass amplitude)
inline constexpr double HIGGS_VEV_LATTICE = V_HIGGS * 1000.0 / K_B;

// cos(theta_W) at tree level. Derived from SIN2_WEINBERG = N_C/N_EFF = 3/13:
//   cos²(theta_W) = 1 - SIN2_WEINBERG = 10/13
//   cos(theta_W)  = sqrt(10/13) ≈ 0.8770580193
// Previously hardcoded as 0.881 (the experimental M_W/M_Z ≈ 80.4/91.2 ratio),
// which drifted ~0.45% from the FTD-derived value. JS counterpart already
// expresses this as Math.sqrt(1 - SIN2_WEINBERG) (engine/web/js/constants.js:90).
// Not constexpr because std::sqrt is non-constexpr until C++26.
inline const double WZ_MIXING_ANGLE_COS = std::sqrt(1.0 - SIN2_WEINBERG);

// Layer 7: Precision formula
using ontic::C1;
using ontic::C2;
using ontic::C3;
using ontic::C4;
using ontic::EPSILON;
using ontic::EPSILON_ABS;

// Layer 8: Reference frame context quadratic (noetic domain)
using ontic::C_MANDELBROT;
using ontic::COS2_THETA_C;
using ontic::K_C_SQUARED;
using ontic::K_NOETIC;
using ontic::SIN2_THETA_C;
using ontic::Y_REAL;

// Layer 8b: Golden ratio fixed point (self-referential reference frame context)
using ontic::BETA_INTROSPECTION;
using ontic::LAMBDA_LOOP;
using ontic::N_CONSCIOUSNESS_MIN;
using ontic::PHI;
using ontic::PHI_INV;

// Simulation parameters
using ontic::C_SPEED;
using ontic::C_WAVE;
using ontic::DAMPING;
using ontic::DRAG_PER_AXIS;

// ============================================================================
// Engine-specific constants (not part of ontic chain)
// ============================================================================

// Drag values for specific particle types
inline constexpr double DRAG_ELECTRON = 0.25; // 1D drag
inline constexpr double DRAG_TOP = 0.75;      // 3D drag

// Default effective radius for Scale 1 particles (from Phase 6 engine data)
inline constexpr double R_EFF_DEFAULT = 2.48;

// ============================================================================
// SM Sector Constants (Phase 2 — phenomenological insertions)
// ============================================================================

// Strong coupling at lattice (Planck) scale [IMPOSED]
// At the Planck scale, QCD is strongly coupled: α_s ~ O(1).
// F_strong(r) = ALPHA_S * exp(-M_YUKAWA*r) / r² * (1 + M_YUKAWA*r)
// NAMING HAZARD: this ALPHA_S is NOT the QCD coupling at M_Z. It is an
// [IMPOSED] Planck-scale strong-force prefactor = 1.0, while ALPHA_S_MZ
// (Layer 5b above) is the running coupling 7/59 = 0.1186 — the two differ by
// 8.43x. scripts/constants.py:350 uses the name ALPHA_S for the *M_Z* object,
// so the same identifier means different things across languages; the JS
// mirror already disambiguates (STRONG_ALPHA_S / ALPHA_S_MZ). Renaming this
// symbol requires a consumer sweep and is deliberately NOT done here.
inline constexpr double ALPHA_S = 1.0;

// Yukawa range parameter (inverse meson mass in lattice units) [IMPOSED]
// 1/M_YUKAWA sets the effective range of the strong force (~1-2 voxels).
inline constexpr double M_YUKAWA = 1.0;

// Energy scale per lattice unit for QCD running (GeV) [IMPOSED]
// Maps lattice separation r to energy scale: Q(r) = Q_LATTICE / r
// At Q_LATTICE = 2.0: r=1 → α_s≈0.37, r=√2 → α_s≈0.44, r≥5 → α_s=1.0
inline constexpr double Q_LATTICE = 2.0;

// Running strong coupling as a function of lattice separation.
// Maps r (voxels) → Q (GeV) via Q = Q_LATTICE / r, then evaluates running.
// Clamped to [0, ALPHA_S] to avoid Landau pole artifacts.
inline double alpha_s_lattice(double r_voxels) {
  if (r_voxels <= 0.0)
    return ALPHA_S;
  double Q = Q_LATTICE / r_voxels;
  double as = alpha_s_running(Q);
  return std::min(as, ALPHA_S);
}

// Strong force string tension [DERIVED from ALPHA_S and K_B]
// V(r) = SIGMA_STRING * r (linear confinement potential)
// F = -dV/dr = -SIGMA_STRING (constant force at long range)
// Motivated by lattice QCD: sigma ~ alpha_s * K_B^2
inline constexpr double SIGMA_STRING = ALPHA_S * K_B * K_B;

#ifdef __CUDACC__
#define FTD_COLOR_HD __host__ __device__ __forceinline__
#else
#define FTD_COLOR_HD inline
#endif

// Scale-0 colour pairwise magnitude. Default r>=8 is harmonic (F∝r).
// TermToggles::confinement switches that shell to ParticleEngine's
// constant string F = SIGMA_STRING * cf. [SELECTION], not FTD-0025.
FTD_COLOR_HD double color_regime_force_mag(double r, double as, double cf,
                                           bool linear_confinement) {
    if (r < COLOR_COULOMB_RADIUS)
        return as * cf / (r * r);
    if (r < COLOR_TRANSITION_RADIUS)
        return as * cf / (COLOR_TRANSITION_DENOM * r);
    if (linear_confinement)
        return SIGMA_STRING * cf;
    return as * cf * r / COLOR_LINEAR_DENOM;
}


// Asymptotic freedom crossover radius [IMPOSED]
// Below this radius, coulombic 1/r^2 term dominates (asymptotic freedom).
// Above this radius, linear confinement dominates.
// Set to 1/M_YUKAWA for consistency with former Yukawa range.
inline constexpr double R_CONFINEMENT = 1.0 / M_YUKAWA;

// Weak transmutation threshold [IMPOSED]
// Polarity flips when field stress |∇·J| + |∇×J| + |∇ρ| exceeds this.
inline constexpr double WEAK_THRESHOLD = K_GENESIS;

// Triad binding energy: K_B × φ [DERIVED from stability analysis]
// PHI now in ontic.h Layer 8b as [THEOREM] (emerges from Softplus fixed point)
inline constexpr double BINDING_ENERGY = K_B * PHI;

// FTD-0271: de Broglie internal-clock frequency ω₀ [rad/tick]. The KG mass
// term is −ω₀²·J. ω₀∝K_B is [IMPOSED] (native flux is massless, A0); the
// proportionality K_B→ω₀[rad/tick] is [SELECTION] (no ℏ in the substrate —
// the lattice fixes the de Broglie *shape* λ∝1/v, never the absolute scale).
// This is a reference value only; the engine reads toggles.omega0 at runtime.
// Stability bound for the leapfrog integrator: ω₀·dt < 2.
inline constexpr double OMEGA0_COMPTON = K_B;  // imposed calibration, not a unified mass role

// Exchange repulsion strength scale (Fermi pressure)
// From DERIV_SPIN_STATISTICS_BRIDGE: the discriminant trichotomy produces
// an antisymmetric exchange term between identical fermions.
// The 1/r^4 exchange force uses α² as its natural scale.
inline constexpr double ALPHA_EXCHANGE = ALPHA * ALPHA; // ≈ 5.3e-5

// Larmor radiation: flux damping proportional to acceleration²
// From classical Larmor formula P = (2α/3)·a², normalized by K_B.
// K_LARMOR * a² gives the fraction of max damping to apply.
//
// The coefficient must exceed the coupling injection rate g_c ≈ 0.085
// per site visited, otherwise accelerating particles GAIN net field
// energy (coupling pumps faster than Larmor damps). Scaling by N_EFF
// (the effective DoF count) ensures Larmor dominates at moderate a.
inline constexpr double K_LARMOR = 4.0 * N_EFF / (3.0 * K_B); // ≈ 33.9

// Minimum damping floor: 1% of full damping even at a=0.
// Ensures thermodynamic dissipation is never completely off.
inline constexpr double LARMOR_FLOOR = 0.01;

// Triad binding detection (CPU port of GPU triad_detection_kernel)
// Three same-sign particles within TRIAD_RADIUS with near-equilateral geometry.
inline constexpr double TRIAD_RADIUS = 3.0;          // max pairwise distance
inline constexpr double TRIAD_RATIO_THRESHOLD = 0.8; // min(r)/max(r) threshold

// Exchange (Pauli) force range (CPU port of GPU exchange_force_kernel)
// Same-spin repulsion falls off as exp(-r²/EXCHANGE_RANGE²).
inline constexpr double EXCHANGE_RANGE = 3.0; // voxels
inline constexpr double EXCHANGE_RANGE_SQ = EXCHANGE_RANGE * EXCHANGE_RANGE;

FTD_COLOR_HD double ftd_exp(double x) {
#ifdef __CUDACC__
    return exp(x);
#else
    return std::exp(x);
#endif
}

// Shortest-path delta on a periodic N^3 lattice. Returns d in (-N/2, N/2].
FTD_COLOR_HD int lattice_periodic_delta(int a, int b, int N) {
    int d = a - b;
    if (d >  N / 2) d -= N;
    if (d < -N / 2) d += N;
    return d;
}

// Continuous remainder wrap matching CPU phase_forces (half = integer L/2).
FTD_COLOR_HD double lattice_periodic_delta_real(double delta, double half, double L) {
    if (delta > half) delta -= L;
    if (delta < -half) delta += L;
    return delta;
}

// Magnitude of the FTD-0406 remainder colour law (no colour factor).
FTD_COLOR_HD double strong_radial_profile_from_as(double r, double as) {
    if (r < 1.0) r = 1.0;
    if (r < COLOR_COULOMB_RADIUS) return as / (r * r);
    if (r < COLOR_TRANSITION_RADIUS) return as / (COLOR_TRANSITION_DENOM * r);
    return as * r / COLOR_LINEAR_DENOM;
}

// GPU yukawa_force_kernel / CPU phase_forces: attractive, all manifested pairs.
// r is clamped for the 1/r² and exponential; callers still clamp the unit
// vector's r separately so a coincident pair does not divide by zero.
FTD_COLOR_HD double yukawa_pair_force_mag(double r) {
    if (r < 1.0) r = 1.0;
    return ALPHA_S * ftd_exp(-M_YUKAWA * r) / (r * r) * (1.0 + M_YUKAWA * r);
}

// GPU exchange_force_kernel / CPU phase_forces: same-spin repulsion.
// Exponential uses the unclamped r² (GPU historical); 1/r² uses clamped r.
FTD_COLOR_HD double exchange_pair_force_mag(double r, double r2) {
    if (r < 1.0) r = 1.0;
    return ALPHA_EXCHANGE * ftd_exp(-r2 / EXCHANGE_RANGE_SQ) / (r * r);
}

// ============================================================================
// Engine tuning constants (extracted from render_bridge.cpp)
// ============================================================================

// SOR solver parameters (used by gauss_project and solve_coulomb_poisson).
//
// Per-iteration SOR cost dominates the tick at L>=64 (~70% of total), so the
// iteration count has direct linear FPS impact. 6 is the cheapest point that
// still places the solver on its accuracy floor for the interactive default.
//
// HONEST NOTE — the central-difference gauss_violation does NOT converge to
// zero with iterations; it SATURATES to an iteration-independent structural
// FLOOR (~5e-3 RMS on a non-Gauss dipole config; sum-of-squares ~0.342 at
// L=24). The floor is a STENCIL MISMATCH: gauss_project solves phi with an
// 18-point Laplacian (sor_sweep_18pt) while the Gauss residual is MEASURED
// with a 6-point central-difference divergence (EnergyAudit.gauss_violation
// via divergence_flux). The 18-pt-solved phi cannot zero the 6-pt divergence,
// so the residual saturates and is BIT-IDENTICAL from ~100 iterations through
// 1000 (it is within ~4e-8 of the floor already by 50). At the interactive
// default of 6 iters the residual sits only ~7.6% ABOVE that saturated floor,
// and going to 1000 iters closes that ~7.6% and then PINS — it never trends
// toward zero. Raising SOR_ITERATIONS therefore buys at most a few percent on
// this metric and then NOTHING. See engine/include/ftd/eft/matched_poisson.h
// lines 7-19 for the analysis, and engine/tests/test_conservation_profile.cpp
// (CP-3) for the runnable measurement. (An earlier comment here showed a
// vacuum-only L=64 convergence table implying more iterations monotonically
// help; that was misleading — it did not reflect the per-config floor.)
//
// Energy conservation under LIVE dynamics is limited by the NON-VARIATIONAL
// projection operator (J -= grad(phi) is a hard constraint not derived from
// the action; it injects energy each tick — see test_conservation_profile.cpp
// CP-2), NOT by SOR convergence. Machine-precision conservation needs a
// variational/energy-aware projection (a separate physics task), not a tighter
// or longer solve. 6 iterations is retained as the interactive default
// because additional iterations improve neither the gauss_violation floor nor
// the live-dynamics energy drift.
inline constexpr int SOR_ITERATIONS = 6;
inline constexpr double SOR_OMEGA = 1.75;

// (EVAP_THRESHOLD removed 2026-07-16, BH-F5 completion: the deterministic
// evaporation cutoff was retired on CPU 2026-04-23 (15882e98) but fossilized
// in the GPU evaporation_kernel; both backends now run the stochastic
// Boltzmann rule scaled by K_EVAP_RATE below.)

// Numerical underflow guards
inline constexpr double EPSILON_FLUX_SQ = 1e-30; // guard for |J|² divisions
inline constexpr double EPSILON_MAG = 1e-15; // guard for magnitude divisions

// RF-8 (2026-04-25): bandwidth budget floor used by accumulate_proper_time()
// to keep the effective speed-of-light denominator finite when L² approaches
// 1 (near-horizon limit) and to clamp the kinetic budget below 1.0. Was
// previously a bare `1e-6` literal at three call sites in render_bridge.cpp.
inline constexpr double BANDWIDTH_FLOOR = 1e-6;

// Wavepacket injection: Gaussian truncated at this many sigma
inline constexpr double GAUSSIAN_CUTOFF_SIGMA = 3.0;

// Tier-1 gradient scale: central difference 1/2.
inline constexpr double GRAD_TIER1_SCALE = 0.5;

// Tier-2 gravity gradient scale: 1/(2×2) for r=2 stencil
inline constexpr double GRAD_TIER2_SCALE = 0.25;

// 18-pt isotropic Laplacian weights (Patra-Karttunen 2006).
// Cancel O(k⁴) anisotropy on the 26-neighbor Moore stencil.
// Sum rule: 6·FACE + 12·EDGE − 4·center = 0.
inline constexpr double LAPLACIAN_FACE_WEIGHT = 1.0 / 3.0;
inline constexpr double LAPLACIAN_EDGE_WEIGHT = 1.0 / 6.0;

// Genesis/evaporation tuning (formerly bare literals in render_bridge.cpp)
// K_GENESIS_KINETIC_DRAIN: fraction of wave_vel consumed at manifestation
// (selected genesis drain; not an exact latent-heat identity per FTD-0567).
inline constexpr double K_GENESIS_KINETIC_DRAIN = 0.5;
// K_EVAP_RATE: per-tick evaporation probability scaling. The Boltzmann
// decay probability p = exp(-local_energy/K_MANIFEST²) is multiplied by
// this AND by the proper-time rate dτ/dt (ftd/proper_time_rate.h; the
// 2026-07-19 proper-time-hazard amendment — decay statistics are clocks,
// so the hazard integrates the same dτ the τ-accumulator defines; at
// L=0, v=0 the factor is exactly 1).
inline constexpr double K_EVAP_RATE = 0.1;
// K_GENESIS_FLUX_EPSILON: floor on |J| during the genesis flux drain
// to prevent division-by-near-zero. Distinct from EPSILON_MAG (which
// is the universal magnitude underflow guard at 1e-15) because genesis
// uses a more permissive 1e-9 threshold to avoid numerical pathology
// in the flux pre-drain integral.
inline constexpr double K_GENESIS_FLUX_EPSILON = 1e-9;

// Color force regime boundaries — now defined in ftd/constants_shared.h (host+device shared header, renamed from constants_gpu.cuh in revision 2.5).
// Using declarations bring them into the ftd:: namespace so existing callers are unchanged.
using ::COLOR_COULOMB_RADIUS;
using ::COLOR_TRANSITION_RADIUS;
using ::COLOR_TRANSITION_DENOM;
using ::COLOR_LINEAR_DENOM;

// Latency / horizon clamps used by the GR sector
inline constexpr double LATENCY_HORIZON_CLAMP = 0.998;   // f = 1 - L² floor

// ============================================================================
// Non-Abelian gauge-link relaxation (revision 0.9 option a — tick wiring)
// ============================================================================
// Per-tick step size and coupling for the SU(2)/SU(3) Wilson-action staple
// relaxation (relax_su2/su3_links_cpu + kernels_gauge.cu), gated on
// toggles.su2_gauge / toggles.su3_gauge. [IMPOSED]: the staple/plaquette
// relaxation form is imported from standard lattice gauge theory and these
// rates are calibrations, not derived quantities — values match the
// test_gauge_links characterization exercise (dt=0.1, beta=1.0) under which
// unitarity/finiteness/determinism were pinned. Same constants feed both
// backends so CPU/GPU parity is meaningful.
inline constexpr double GAUGE_RELAX_DT   = 0.1;
inline constexpr double GAUGE_RELAX_BETA = 1.0;

// ============================================================================
// Scale 2 Phase 3 Constants — Inter-atomic forces
// ============================================================================

// H-bond parameters [DERIVED from α perturbation theory]
// H-bonds are ~10× weaker than covalent, ~comparable to vdW for small atoms.
// LJ 10-12 form: V = eps * [5*(sig/r)^12 - 6*(sig/r)^10] * cos^n(theta)
inline constexpr double H_BOND_EPSILON = K_B * ALPHA * ALPHA * ALPHA; // ~1.98e-7
inline constexpr double H_BOND_COS_POWER = 2.0; // angular dependence exponent

// Angle strain (VSEPR) [DERIVED from α × K_B]
// V = K_ANGLE * (theta - theta_eq)^2 / 2
inline constexpr double K_ANGLE = ALPHA * K_B; // ~3.72e-3

// Torsional (Dihedral) Strain [DERIVED from α² × K_B]
// V = V_TORSION / 2 * [1 + cos(n*phi - gamma)]
inline constexpr double V_TORSION = ALPHA * ALPHA * K_B; // ~2.71e-5

// Improper Torsion (Planarity) [DERIVED from α × K_B]
// V = K_IMPROPER * (omega)^2 / 2
inline constexpr double K_IMPROPER = ALPHA * K_B * 2.0;

// Thermostat coupling timescale [IMPOSED]
// Berendsen: lambda = sqrt(1 + dt/tau * (T_target/T_current - 1))
inline constexpr double THERMOSTAT_TAU_DEFAULT = 10.0; // in dt units

} // namespace ftd
