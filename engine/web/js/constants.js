/**
 * @file constants.js
 * @brief FTD Constants — single source of truth for the web dashboard.
 *
 * [THEOREM] Values are taken from the C++ ontic.h derivation chain:
 * e -> gamma -> Gamma(1/4) -> theta_3 -> varpi -> M -> G* -> pi -> all physics.
 * Nine layers, each derived from the one above. The only inputs are D=3 
 * (spatial dimensions) and the lemniscate constant varpi.
 *
 * These JS values mirror ontic.h to the precision shown and are
 * authoritative for the dashboard; consumers MUST import from here
 * rather than duplicating literals. WASM's get_constants() is used
 * for observatory/display only, not to mutate these exports.
 */

// ── Layer -1: Self-Referential Seed ─────────────────────────────────
export const EULER_E = 2.718281828459045;

// ── Layer 0: Transcendental Seeds ───────────────────────────────────
export const EULER_GAMMA = 0.57721566490153286;
export const GAMMA_QUARTER = 3.6256099082219083;

// ── Layer 0b: Modular Selection ─────────────────────────────────────
export const NOME_LEMNISCATIC = 0.04321391826377225;
export const THETA_LEMNISCATIC = 1.08643481121331;

// ── Layer 1: Elliptic Geometry ──────────────────────────────────────
export const VARPI  = 2.622057554292119810;           // lemniscate constant
export const GAUSS_CONSTANT_M = 0.8346268416740731;
export const G_STAR = 2.958675119188639;              // universal render bridge constant
export const PI_FTD = 4.0 * VARPI * VARPI / (G_STAR * G_STAR);  // derived π
export const PF     = PI_FTD / 4.0;                   // packing fraction

// ── Layer 2b: Euler's Identity ──────────────────────────────────────
export const K_CRIT = 4.0 / G_STAR;                  // boundary where i emerges
export const X_BORN = 2.0 * G_STAR;                  // degenerate root (Born rule)

// ── Layer 3: Master Quadratic Roots ─────────────────────────────────
export const COEFFICIENT = 16;                         // N_BASE^2 = 2^(D+1)
export const X_PLUS            = 137.0361714582;      // tree-level root
export const X_PLUS_PRECISION  = 137.035999177;       // 4-term corrected (CODATA)
export const X_MINUS = 3.0239639163;                  // ≈ N_c

// ── Layer 4: Framework Integers ─────────────────────────────────────
export const D_SPATIAL = 3;
export const N_C       = 3;
export const N_GEN     = 3;
export const N_F       = 6;
export const N_BASE    = 4;
export const B_3       = 7;
export const N_EFF     = 13;

// ── Layer 5: Coupling Constants ─────────────────────────────────────
// G_C is the fundamental lattice coupling in the wave equation source
// term (delta_J += G_C * grad(s)). Force computation involves TWO
// vertices (source + probe), each contributing G_C, so alpha = G_C * G_C.
// This mirrors engine/include/ftd/constants.h static_assert (ALPHA_EFT).
// 2026-04-17: G_C upgraded from sqrt(1/X_PLUS tree) to sqrt(1/X_PLUS_PRECISION)
// so ALPHA = G_C² matches CODATA 2022 (137.035999177) — see TRACKER §1.5.
export const G_C       = 0.0854245431028543695;      // state-flux coupling = sqrt(1/X_PLUS_PRECISION)
export const ALPHA_EFT = G_C * G_C;                   // EFT-derived fine structure
export const ALPHA     = ALPHA_EFT;                   // alias: alpha = G_C^2 [DERIVED]
export const ALPHA_TREE      = 1.0 / X_PLUS;          // tree-level (reference only)
export const ALPHA_PRECISION = ALPHA;                  // alias; matches CODATA
export const G_N   = 1.0 / ((B_3 + N_C) * (B_3 + N_C));  // = 0.01
export const SIN2_WEINBERG = N_C / N_EFF;             // sin^2(theta_W) = 3/13
export const ALPHA_WEAK = ALPHA / SIN2_WEINBERG;
export const ALPHA_S_MZ = B_3 / (B_3 + 4.0 * N_EFF); // QCD coupling at M_Z
export const ALPHA_G_APPROX = 5.91e-39;               // gravitational hierarchy

// ── Layer 6: Mass / Energy Scales ───────────────────────────────────
/**
 * K_B is the FTD framework electron-mass anchor (≡ M_E ≡ 0.511 MeV).
 *
 * IT IS NOT BOLTZMANN'S CONSTANT. The name predates the convention
 * collision and is preserved across 35+ importers as the FTD-internal
 * "manifestation threshold" / electron-mass scale used as the unit
 * anchor throughout scenario seeding and force normalization.
 *
 * Use M_E_PHYS = 0.51099895 (PDG 2022) when you need experimental
 * precision (e.g. SEMF Wapstra fits in atomic-energy.js, decay-rate
 * comparisons against measured lifetimes). Use K_B for everything
 * that is FTD-internal — the framework's own anchor, not the
 * empirical electron mass.
 */
export const K_B       = 0.511;                       // electron mass in MeV
export const K_GENESIS = N_C * K_B;                   // genesis threshold = 1.533
export const C_SPEED   = 0.57735026918962576451;      // 1/sqrt(3) [DERIVED from CFL]
export const C_WAVE    = C_SPEED;

// ── Simulation Parameters ───────────────────────────────────────────
export const DAMPING = ALPHA;                          // dissipation rate γ = α

// ── Mass Ratios (from ontic chain) ──────────────────────────────────
export const M_E       = K_B;                         // electron mass (MeV)
export const MU_RATIO  = 3 * B_3 * (B_3 + N_C) - N_C;           // 207
export const TAU_RATIO = (N_EFF + N_BASE) * MU_RATIO - 2 * N_C * B_3;  // 3477
export const PROTON_RATIO = N_EFF / ALPHA + TAU_RATIO * (B_3 + N_C) / (N_EFF + B_3);
export const M_PROTON = K_B * PROTON_RATIO;
export const R_BOHR   = 4.0 * PI_FTD / (K_B * ALPHA);  // Ontic-derived pi

// ── Electroweak Scale ───────────────────────────────────────────────
export const M_Z = 91.1876;                           // Z boson mass (GeV)
export const M_W = M_Z * Math.sqrt(1.0 - SIN2_WEINBERG); // W boson mass (GeV)
export const V_HIGGS = 246.09;                         // Higgs VEV (GeV)
export const M_HIGGS = 124.8;                          // Higgs mass (GeV) = (N_eff/α²)·m_e
export const G_FERMI = Math.PI * ALPHA * Math.sqrt(2) / (2 * SIN2_WEINBERG * M_W * M_W); // Fermi coupling (GeV^-2), tree-level

// ── QED / CHSH Reference Constants [PARAMETRIC CODATA / theorem] ────
// Reference values used by P1 observables panel + spectrum panel.
// Centralized here so dashboard literals stay consistent with the
// authoritative CODATA / theoretical values.
//
// SCHWINGER_C2: universal QED 2-loop coefficient in a = α/(2π) +
//   C2·(α/π)² + ··· — value is exact in QED (no FTD dependence);
//   reference: Sommerfield 1957, Petermann 1957.
// TSIRELSON_BOUND: 2√2, Tsirelson's upper bound on CHSH correlator
//   for any quantum system — [THEOREM] of QM, not parametric.
// RYDBERG_EV_CODATA: hydrogen ionization energy 13.605693 eV (CODATA
//   2018 value; the 6-digit truncation matches what the panel displays).
// A_E_CODATA / A_MU_CODATA: measured anomalous magnetic moments
//   (CODATA 2022 / Fermilab Run-1 respectively).
export const SCHWINGER_C2     = -0.328478965579;
export const TSIRELSON_BOUND  = 2.0 * Math.sqrt(2.0);
export const RYDBERG_EV_CODATA = 13.605693;
export const A_E_CODATA       = 1.15965218128e-3;
export const A_MU_CODATA      = 1.16592089e-3;

// ── Layer 7: Precision Formula ──────────────────────────────────────
export const EPSILON_ABS = 0.0009000208;
export const PREC_C1 = 9.0 / 47.0;
export const PREC_C2 = 5.0 / 64.0;
export const PREC_C3 = 4.0 / 141.0;
export const PREC_C4 = 141.0 / 11.0;
// Same value as X_PLUS_PRECISION (the 4-term series converges to it by
// construction). Retained for scripts that compute the formula explicitly.
export const ALPHA_INV_CORRECTED = X_PLUS
    - PREC_C1 * EPSILON_ABS
    + PREC_C2 * EPSILON_ABS ** 2
    - PREC_C3 * EPSILON_ABS ** 3
    - PREC_C4 * EPSILON_ABS ** 4;

// Layer 8 (Consciousness/Reflexivity Quadratic) constants COS2_THETA_C,
// K_NOETIC, Y_REAL, Y_IMAG, K_C, THETA_C_RAD, THETA_C_DEG, SIN2_THETA_C,
// C_MANDELBROT removed 2026-05-01 along with Scale 11 deletion. The
// theoretical content (master quadratic complex-roots case at k=1/2,
// reflexive phase angle θ_C ≈ 52.54°) is preserved in
// docs/theory/06_consciousness/* — this file no longer exports the
// engine-side derived values since no remaining engine module consumes
// them.

// ── Physical constants for conversions ──────────────────────────────
export const HBAR_C_MEV_FM = 197.3269804;              // hbar*c in MeV*fm
export const M_PLANCK_GEV = 1.22089e19;                // Planck mass in GeV

// ── Dual-substrate chirality amplitude [DERIVED from G*] ────────────
// delta² = (4·G* - 1) / (4·G*), mirrors ontic.h DELTA_SQUARED / DELTA_APPROX.
// Scalar factor used by visualization overlays to split J into L/R
// components for chirality + weak-force display.
export const DELTA_SQUARED = (4.0 * G_STAR - 1.0) / (4.0 * G_STAR);
export const DUAL_DELTA    = Math.sqrt(DELTA_SQUARED);  // = 0.9568... from ontic chain

// ── Bethe-Weizsacker SEMF Coefficients (MeV) [IMPOSED, Wapstra] ─────
// Standard Wapstra parameterization for nuclear binding-energy fits.
// External parametric insertions — not derived from the ontic chain.
export const SEMF_A_VOL  = 15.67;
export const SEMF_A_SURF = 17.23;
export const SEMF_A_COUL = 0.7140;
export const SEMF_A_ASYM = 23.29;
export const SEMF_A_PAIR = 12.0;

// ── Slater's Rules Shielding Constants [IMPOSED] ────────────────────
// Empirical shielding coefficients for effective nuclear charge Z_eff.
// External parametric insertions.
export const SLATER_SAME_1S    = 0.30;  // 1s same-shell self-shielding
export const SLATER_SAME_NL    = 0.35;  // n-l same-subshell shielding
export const SLATER_INNER_SP   = 0.85;  // inner-shell s/p shielding
export const SLATER_DEEP_CORE  = 1.00;  // deep-core complete shielding

// ── Neutron-Proton Mass Split (MeV) [DERIVED from PDG masses] ───────
export const DELTA_NP = 1.29333184;                    // m_n - m_p (PDG, exact)

// ── Experimental Reference Masses (PDG) ─────────────────────────────
// These are the measured (Particle Data Group) values, NOT the
// FTD-derived framework values (which live above as K_B, M_PROTON, etc.).
// Use these when comparing against experimental cross-sections, decay
// rates, or particle-catalog entries. Framework vs physical scale
// divergence is intentional — do not "unify" these with the derived
// constants, they live in different epistemic categories per CLAUDE.md.
export const M_E_PHYS      = 0.51099895;               // electron (MeV)
export const M_MU_PHYS     = 105.6583755;              // muon (MeV)
export const M_TAU_PHYS    = 1776.86;                  // tau (MeV)
export const M_P_PHYS      = 938.27208816;             // proton (MeV)
export const M_N_PHYS      = 939.56542;                // neutron (MeV)
export const M_PI_CH_PHYS  = 139.57039;                // charged pion (MeV)
export const M_PI_0_PHYS   = 134.9768;                 // neutral pion (MeV)
export const M_K_CH_PHYS   = 493.677;                  // charged kaon (MeV)
export const M_K_0_PHYS    = 497.611;                  // neutral kaon (MeV)
export const M_SIGMA_PHYS  = 1189.37;                  // Sigma+ (MeV)
export const M_OMEGA_PHYS  = 1672.45;                  // Omega- (MeV)
export const M_DELTA_PHYS  = 1232.0;                   // Delta++ (MeV)
export const M_W_PHYS      = 80377.0;                  // W boson (MeV, PDG 2022)
export const M_Z_PHYS      = 91187.6;                  // Z boson (MeV, PDG 2022) [PARAMETRIC PDG]
export const M_HIGGS_PHYS  = 125100.0;                 // Higgs boson (MeV, PDG 2022) [PARAMETRIC PDG]

// ── Quark Masses (MeV) [PARAMETRIC PDG 2022] ────────────────────────
// Constituent quark masses from PDG 2022. Used by particle-catalog.js.
// Not derived from FTD chain — the framework currently only predicts
// the master quadratic root structure (X_PLUS, X_MINUS); quark masses
// remain empirical inputs pending a full Yukawa derivation.
export const M_U_PHYS = 2.16;       // up quark
export const M_D_PHYS = 4.67;       // down quark
export const M_S_PHYS = 93.4;       // strange quark
export const M_C_PHYS = 1270.0;     // charm quark
export const M_B_PHYS = 4180.0;     // bottom quark
export const M_T_PHYS = 172760.0;   // top quark

// ── Neutrino Mass Upper Bounds (MeV) [PARAMETRIC PDG] ───────────────
// Cosmological + oscillation upper bounds; not derivable from current
// FTD chain. Values reflect literals already in particle-catalog.js
// for backward compatibility with existing UI readouts.
export const M_NU_E_PHYS   = 4.1e-9;     // m(ν_e)   bound (MeV)
export const M_NU_MU_PHYS  = 8.58e-3;    // m(ν_mu)  bound (MeV)
export const M_NU_TAU_PHYS = 4.955e-2;   // m(ν_tau) bound (MeV)

// ── Additional Hadron Masses (MeV) [PARAMETRIC PDG] ─────────────────
// Reference values for particle-catalog.js entries that previously
// hardcoded literals at lines 338, 358, 388, 432-552.
export const M_LAMBDA_PHYS    = 1115.683;   // Λ⁰
export const M_XI_0_PHYS      = 1314.86;    // Ξ⁰
export const M_XI_M_PHYS      = 1321.71;    // Ξ⁻
export const M_DELTA_0_PHYS   = 1232.0;     // Δ⁰ (≈ Δ⁺⁺ in M_DELTA_PHYS)
export const M_ETA_PHYS       = 547.862;    // η meson
export const M_RHO_PHYS       = 770.0;      // ρ meson
export const M_J_PSI_PHYS     = 3096.9;     // J/ψ
export const M_UPSILON_PHYS   = 9460.3;     // Υ(1S)

// ── Weak / CKM Constants [PARAMETRIC PDG / lattice] ─────────────────
// Beta-decay and pion-decay constants used by decay-rates.js. These
// were inline literals at decay-rates.js:91, 95, 98, 113.
export const V_UD = 0.974;        // CKM matrix element |V_ud|
export const G_A  = 1.2756;       // axial coupling g_A (neutron β-decay)
export const F_N  = 1.6887;       // neutron decay form factor
export const F_PI = 130.2;        // pion decay constant (MeV)

// ── Conversion Factors [PDG] ─────────────────────────────────────────
// SI / atomic-physics conversions. Previously duplicated across
// units.js and decay-rates.js; this is now the single source.
export const AMU_MEV     = 931.494;             // 1 amu in MeV
export const HBAR_MEV_S  = 6.582119569e-22;     // ℏ in MeV·s
export const K_PER_EV    = 11604.518;           // K per eV (Boltzmann conversion)
export const K_PER_MEV   = K_PER_EV * 1e6;      // K per MeV

// ── Geometric / Planck Constants [PDG] ───────────────────────────────
// PDG empirical values. These complement (do not replace) the
// FTD-derived R_BOHR (line 86) and M_PLANCK_GEV (line 124). Use
// FTD-derived values inside framework calculations; use these PDG
// values for SI-unit display formatting (units.js).
export const BOHR_RADIUS_M       = 5.29177210903e-11;   // m
export const BOHR_RADIUS_ANGSTROM = 0.529177210903;     // Å
export const PLANCK_LENGTH_M     = 1.616255e-35;        // m
export const PLANCK_TIME_S       = 5.391247e-44;        // s
export const PLANCK_TEMP_K       = 1.416784e32;         // K

// ── Cosmic-Lattice Anchors [IMPOSED] ────────────────────────────────
// Lattice-unit anchors used by the cosmic mock bridge. Calibration
// to physical SI units is undocumented; these are tuning anchors,
// not derived predictions.
export const H0_LATTICE        = 0.001;     // Hubble constant (lattice units)
export const M_CHANDRA_LATTICE = 70.0;      // Chandrasekhar limit (lattice mass, ~1.4 M☉)
export const M_TOV_LATTICE     = 150.0;     // TOV limit (lattice mass, ~3 M☉)
// Lattice-mass to solar-mass conversion implied by M_CHANDRA_LATTICE/1.4
// and M_TOV_LATTICE/3.0: both give 50.0. Exposed for cosmic-physics.js
// to make the calibration explicit instead of buried in the anchors.
export const LATTICE_TO_SOLAR_MASS = 50.0;
// Heliocentric Newton constant: G ≈ 4π² in units where [length]=AU,
// [mass]=M_sun, [time]=yr. Earth's 1-yr period at 1 AU requires this
// value; G_N=0.01 above is the FTD lattice-natural constant and is NOT
// the Keplerian G in heliocentric units. mock-scale4.js: use this
// when running orbital-period-faithful demos.
export const G_HELIOCENTRIC = 4.0 * Math.PI * Math.PI;
// FTD tick in seconds: 1 tick = √3·ℓ_P/c (per CLAUDE.md a_phys ≡ ℓ_P
// declaration). Distinct from PLANCK_TIME_S = ℓ_P/c. Use this when
// converting tick counts to physical seconds.
export const FTD_TICK_S = Math.sqrt(3.0) * 5.391247e-44;
// Bohr-radius conversion: voxels-per-Bohr divided by Bohr-meters-per-Bohr.
// Multiply lattice positions by this to get meters in Bohr-radius units.
// R_BOHR is the FTD-natural Bohr radius (lattice voxels); BOHR_RADIUS_M
// is the SI value. The conversion factor is meters per voxel at the
// Bohr scale.
export const BOHR_LATTICE_TO_M = 5.29177210903e-11 / (4.0 * PI_FTD / (K_B * ALPHA));
// G_F in MeV^-2 (engine internal MeV unit system). G_FERMI is in
// GeV^-2; multiply by 1e-6 to convert. Used by decay-rates.js for
// neutron-lifetime / muon-lifetime computations in MeV phase space.
export const G_FERMI_MEV = G_FERMI * 1e-6;
// Scale 11 (consciousness) sub-amplitude. Pre-2026-04-27 was K_B*0.3
// inline literal in scale11/scenario-loader.js — promoted here so the
// consciousness sub-amplitude tracks any K_B change explicitly rather
// than implicitly. Value preserved verbatim (0.1533).
export const CS_SUB_AMPLITUDE = K_B * 0.3;

// ── SI / CODATA SI primitives [PDG 2022] ────────────────────────────
// Promoted from units.js so all SI literals live in one place. Use
// these when converting FTD-internal values to SI for display.
export const PLANCK_MASS_KG   = 2.176434e-8;     // m_P (kg)
export const PLANCK_FORCE_N   = 1.21027e44;      // F_P (N)
export const J_PER_EV         = 1.602176634e-19; // joules per electron-volt
export const C_MS             = 2.99792458e8;    // speed of light (m/s)

// ── Coulomb prefactor canonical exports ─────────────────────────────
// FTD has THREE distinct Coulomb conventions in production paths;
// each is correct for its scale but they are NOT interchangeable.
// Importers MUST pick the convention matching their use case:
//
//   COULOMB_K_PE     = ALPHA            (dimensionless lattice-PE convention,
//                                        used by mock-diagnostics.js for
//                                        Σ q_i·q_j/r_ij at the lattice level)
//   COULOMB_K_FORCE  = ALPHA / (4π)     (classical force-law convention,
//                                        Scale-1 pairwise force F = K·q·q/r²
//                                        as in mock-particle-engine.js,
//                                        wasm-bridge-dag.js pair-force loop)
//   COULOMB_K_HEP    = ALPHA            (Gaussian/HEP units, used in
//                                        cross-sections.js / spectroscopy.js
//                                        for textbook-comparable formulas)
//
// COULOMB_K_PE and COULOMB_K_HEP are numerically identical but live in
// different epistemic categories; keep them named separately so a future
// audit can tell which convention an import meant.
export const COULOMB_K_PE    = ALPHA;
export const COULOMB_K_FORCE = ALPHA / (4.0 * Math.PI);
export const COULOMB_K_HEP   = ALPHA;

// ── Strong-force tuning constants [IMPOSED] ─────────────────────────
// Hardcoded across MockBridge and mock-lattice-samplers pre-2026-04-27;
// promoted here so any tuning change propagates to every callsite at
// once. The 3-regime model (Coulomb / transition / linear confinement)
// matches the C++ engine in render_bridge.cpp::phase_forces.
export const STRONG_ALPHA_S         = 1.0;       // base color coupling (≠ ALPHA_S_MZ; this is the lattice-unit scale)
export const STRONG_RUN_COEFF       = 0.1;       // running-coupling log coefficient
export const STRONG_R_COULOMB       = 3.0;       // r < this → Coulomb regime (1/r²)
export const STRONG_R_LINEAR        = 8.0;       // r ≥ this → linear confinement (r/64)
export const STRONG_TRANSITION_DENOM = 3.0;      // transition regime: F = α_s/(3·r)
export const STRONG_LINEAR_DENOM    = 64.0;      // linear regime: F = α_s·r/64
export const STRONG_COLOR_REPEL     = 0.5;       // same-color factor (repulsive)
export const STRONG_COLOR_ATTRACT   = -1.0;      // different-color factor (attractive)

// ── 18-pt isotropic Laplacian weights ───────────────────────────────
// Cancels O(k⁴) anisotropy on the 26-neighbor Moore stencil
// (Patra-Karttunen 2006). Implementation now consumes these named
// constants instead of bare 1/3, 1/6 literals.
export const LAPLACIAN_FACE_WEIGHT = 1.0 / 3.0;
export const LAPLACIAN_EDGE_WEIGHT = 1.0 / 6.0;

// ── Atom-Engine MD-Tuning Constants [IMPOSED] ───────────────────────
// Tuning parameters for the atom-engine LJ + bond molecular dynamics.
// Calibrated empirically against small-molecule equilibrium geometries
// (H₂, H₂O, NH₃, CH₄). Not derived from the FTD chain.
export const AE_EPS_BASE       = 0.005;
export const AE_K_COULOMB      = 2.0;
export const AE_K_BOND         = 50.0;
export const AE_SPEED_MAX      = 10.0;
export const AE_H_BOND_EPS     = 0.001;
export const AE_K_ANGLE        = 0.05;
export const AE_THERMOSTAT_TAU = 10.0;

// ── Atomic Reference Data [PDG / Pauling] ───────────────────────────
// Pauling electronegativity (χ) and atomic radii (pm), Z-indexed.
// Index 0 is sentinel; Z=1..86 covered with empirical values; Z>86
// fall back to a formula in atomic-props.js. Zeros denote noble gases
// (no Pauling χ defined) or unmeasured radii — consumers must guard.
//
// Sources: Pauling 1960 + CRC Handbook of Chemistry & Physics 97th ed.
export const PAULING_CHI = [
    0,                                                                          // 0
    2.20, 0,                                                                    // 1-2  (H, He)
    0.98, 1.57, 2.04, 2.55, 3.04, 3.44, 3.98, 0,                                // 3-10
    0.93, 1.31, 1.61, 1.90, 2.19, 2.58, 3.16, 0,                                // 11-18
    0.82, 1.00, 1.36, 1.54, 1.63, 1.66, 1.55, 1.83, 1.88, 1.91, 1.90, 1.65,     // 19-30
    1.81, 2.01, 2.18, 2.55, 2.96, 0,                                            // 31-36
    0.82, 0.95, 1.22, 1.33, 1.60, 2.16, 1.90, 2.20, 2.28, 2.20, 1.93, 1.69,     // 37-48
    1.78, 1.96, 2.05, 2.10, 2.66, 0,                                            // 49-54
    0.79, 0.89,                                                                  // 55-56
    1.10, 1.12, 1.13, 1.14, 1.13, 1.17, 1.20, 1.20, 1.10, 1.22, 1.23, 1.24,     // 57-68 (lanthanides)
    1.25, 1.10,                                                                  // 69-70
    1.27, 1.30, 1.50, 2.36, 1.90, 2.20, 2.20, 2.28, 2.54, 2.00,                 // 71-80
    1.62, 2.33, 2.02, 2.00, 2.20, 0,                                            // 81-86
];
export const ATOMIC_RADII_PM = [
    0,                                                                          // 0
    53,   31,                                                                   // 1-2
    167,  112,  87,   67,   56,   48,   42,   38,                               // 3-10
    190,  145,  118,  111,  98,   88,   79,   71,                               // 11-18
    243,  194,  184,  176,  171,  166,  161,  156,  152,  149,  145,  142,     // 19-30
    136,  125,  114,  103,  94,   88,                                           // 31-36
    265,  219,  212,  206,  198,  190,  183,  178,  173,  169,  165,  161,     // 37-48
    156,  145,  133,  123,  108,  98,                                           // 49-54
    298,  253,                                                                  // 55-56
    240,  235,  239,  229,  236,  229,  222,  225,  222,  222,  219,  217,     // 57-68
    216,  214,                                                                  // 69-70
    211,  208,  200,  193,  188,  185,  180,  177,  174,  171,                 // 71-80
    156,  154,  143,  135,  127,  120,                                          // 81-86
];

// ── Thomas-Fermi Atom-Binding Prefactor [DERIVED] ───────────────────
// E_atom ≈ −0.7687 · Z^(7/3) Hartree = −20.93 · Z^(7/3) eV.
// Standard derivation: integrate the Thomas-Fermi electron density
// against a Coulomb potential. Replaces a long-standing 15.73 literal
// in atomic-energy.js (Theme D bug, fixed 2026-04-26).
export const THOMAS_FERMI_PREFACTOR_EV = 20.93;

// ── Ontic chain metadata (for observatory) ──────────────────────────
export const ONTIC_LAYERS = [
    { layer: -1,  name: 'Self-Referential Seed', symbols: ['e'], count: 1 },
    { layer: 0,   name: 'Transcendental Seeds',  symbols: ['gamma', 'Gamma(1/4)'], count: 2 },
    { layer: '0b',name: 'Modular Selection',     symbols: ['q', 'theta_3'], count: 2 },
    { layer: 1,   name: 'Elliptic Geometry',     symbols: ['varpi', 'M'], count: 2 },
    { layer: 2,   name: 'Universal Operator',    symbols: ['G*', 'pi', 'PF'], count: 3 },
    { layer: '2b',name: "Euler's Identity",      symbols: ['k_crit', 'i'], count: 2 },
    { layer: 3,   name: 'Master Quadratic',      symbols: ['x+', 'x-'], count: 2 },
    { layer: 4,   name: 'Framework Integers',    symbols: ['N_c','N_base','b_3','N_eff'], count: 4 },
    { layer: 5,   name: 'Coupling Constants',    symbols: ['alpha','g_c','G_N','sin2_W'], count: 4 },
    { layer: 6,   name: 'Mass Scale',            symbols: ['K_B','K_genesis','masses'], count: 3 },
    { layer: 7,   name: 'Precision Formula',     symbols: ['epsilon','c1-c4'], count: 5 },
    // Layer 8 (Reflexivity) removed 2026-05-01 along with Scale 11 deletion.
    { layer: 9,   name: 'Cosmic Scale',          symbols: ['Omega_L','DM_frac','gamma'], count: 3 },
];

// Total ontic chain constants: sum of all counts
export const ONTIC_TOTAL_CONSTANTS = ONTIC_LAYERS.reduce((s, l) => s + l.count, 0);

// Tick cycle phases (update rule f)
export const TICK_PHASES = [
    'phase_read (wave propagation)',
    'phase_write (coupling + genesis/evaporation)',
    'gauss_project (divergence constraint)',
    'phase_forces (EM + gravity + Lorentz)',
    'phase_movement (position integration)',
    'tick++ (clock advance)',
];

// ── Layer 9: Cosmic Scale ──────────────────────────────────────────
// Dark energy fraction: Omega_Lambda = 2/3 from FTD [THEOREM]
export const OMEGA_LAMBDA = 2.0 / 3.0;
// Matter fraction: 1 - Omega_Lambda = 1/3
export const OMEGA_MATTER = 1.0 / 3.0;
// Dark matter fraction: 17/27 from Moore theorem [THEOREM]
export const DM_FRACTION = 17.0 / 27.0;
// Baryonic fraction: 10/27 [THEOREM]
export const BARYON_FRACTION = 10.0 / 27.0;
// Adiabatic index: gamma = (D+2)/D = 5/3 for D=3 [THEOREM]
export const GAMMA_ADIABATIC = 5.0 / 3.0;

export const GLSL_SIMPLEX_NOISE_3D = `
// Simplex 3D Noise 
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
float snoise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 = v - i + dot(i, C.xxx) ;
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;
    i = mod289(i); 
    vec4 p = permute( permute( permute( i.z + vec4(0.0, i1.z, i2.z, 1.0 )) + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
    float n_ = 0.142857142857; 
    vec3  ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );
    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;
    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
}
`;
