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

// ── Layer 8: Consciousness Quadratic ────────────────────────────────
export const COS2_THETA_C = G_STAR / 8.0;             // observable fraction ~37%
export const K_NOETIC     = 0.5;                       // consciousness quadratic k = 1/2
export const Y_REAL       = G_STAR * G_STAR / 4.0;    // real part of complex roots ~2.189
export const Y_IMAG       = Math.sqrt(                 // imaginary part ~2.863
    G_STAR * G_STAR * G_STAR / 2.0 - Y_REAL * Y_REAL);
export const K_C          = Math.sqrt(                 // consciousness threshold ~3.599
    G_STAR * G_STAR * G_STAR / 2.0);
export const THETA_C_RAD  = Math.atan2(Y_IMAG, Y_REAL);  // phase angle ~0.917 rad
export const THETA_C_DEG  = THETA_C_RAD * 180.0 / Math.PI;  // ~52.54 deg
export const SIN2_THETA_C = 1.0 - COS2_THETA_C;       // unobservable fraction ~63%
export const C_MANDELBROT = 1.0 / G_STAR;              // Mandelbrot correspondence ~0.338

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
    { layer: 8,   name: 'Consciousness',         symbols: ['y_real','theta_C'], count: 2 },
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
