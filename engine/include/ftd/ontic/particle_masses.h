#pragma once
/**
 * ontic/particle_masses.h — Layers 6, 6b, 6c of the ontic chain.
 *
 * Contents:
 *   Layer 6:  Mass Scale (K_B, K_GENESIS)
 *   Layer 6b: Electroweak Scale / Higgs sector (V_HIGGS, M_HIGGS, LAMBDA_HIGGS)
 *   Layer 6c: Mass Ratios (MU_RATIO, TAU_RATIO, PROTON_RATIO, M_PROTON, R_BOHR)
 *
 * Depends on: ontic/lemniscate.h (PI),
 *             ontic/master_quadratic.h (N_C, N_BASE, B_3, N_EFF, X_PLUS),
 *             ontic/gauge_couplings.h (ALPHA).
 */

#include "ftd/ontic/lemniscate.h"
#include "ftd/ontic/master_quadratic.h"
#include "ftd/ontic/gauge_couplings.h"

namespace ftd {
namespace ontic {

// ============================================================================
// Layer 6: Mass Scale
// ============================================================================
// Manifestation threshold = electron mass (in simulation energy units):
//   m_e = m_P · √(2π) · (N_base²/N_c) · α¹¹
//       = m_P · √(2π) · (16/3) · α¹¹
//       ≈ 0.5096 MeV  (0.27% from experimental 0.5110 MeV)
//
// In simulation units where m_P = 1, this is ~4.18e-23.
// We use K_B = 0.511 MeV as the practical simulation value.
inline constexpr double K_B = 0.511;

// ── FTD-0130 role disentanglement (unified-mass Phase 0, 2026-06-06) ──────────
// K_B conflated four roles. Split here into named constants so the genuinely-
// independent genesis kinetics can vary without perturbing the rest-mass scale.
// The FTD action forces the rest/inertial/gravitational roles to be ONE number
// (SPEC_FTD_LAGRANGIAN §4.1↔§4.2), so those stay fused as M_REST; only
// K_MANIFEST is independent — and since FTD-0388 it actually differs (below).
inline constexpr double M_REST     = K_B;   // rest = inertial = gravitational mass quantum (= m_e); UNTOUCHED by FTD-0388

// K_MANIFEST := W_SC, the substrate's unit-charge Gauss self-energy
// [SELECTION — ADOPTED, FTD-0388, owner ruling 2026-07-17]. The identification
// is the adopted selection; the value it selects is then forced by lattice
// geometry (Γ-class SC Watson constant; measured engine realization ≤ 0.00084%
// at L=17/33/65, prereg selfenergy-pinning v1/v1.1, commits 66a830ac/d8b27995/
// 19d14df0/67fab0d4). Replaces the MeV-mirroring 0.511 convention for the
// KINETICS role only (genesis + evaporation Boltzmann scale); the mass anchor
// stays M_REST = K_B. Falsifiers live: genesis hard gate at |J| = K_GENESIS
// = 3·W_SC = 1.516386059; evaporation exponent scale K_MANIFEST² = 0.255492.
inline constexpr double K_MANIFEST = 0.5054620197173260;  // := W_SC (was = K_B pre-FTD-0388)

// Genesis threshold: energy needed to CREATE a new particle.
// Must fill all N_c color channels: K_GENESIS = N_c · K_MANIFEST (kinetics, not mass)
inline constexpr double K_GENESIS = K_MANIFEST * N_C;   // = 3·W_SC = 1.5163860591519780 (FTD-0388)

// ============================================================================
// Layer 6c: Mass Ratios (from framework integers)
// ============================================================================
// All mass ratios derive from {N_c, N_base, b_3, N_eff} — no free parameters.
//
//   MU_RATIO   = 3·b₃·(b₃ + N_c) - N_c       = 3·7·10 - 3 = 207
//   TAU_RATIO  = (N_eff + N_base)·MU - 2·N_c·b₃ = 17·207 - 42 = 3477
//   PROTON_RATIO = N_eff/α + N_base·N_eff + N_c   (canonical, FTD-0016)

inline constexpr int    MU_RATIO  = 3 * B_3 * (B_3 + N_C) - N_C;        // 207
inline constexpr int    TAU_RATIO = (N_EFF + N_BASE) * MU_RATIO
                                    - 2 * N_C * B_3;                      // 3477

// Canonical proton/electron mass ratio per FTD-0016:
//   m_p/m_e = N_eff/α + N_base·N_eff + N_c ≈ 1836.47 (174 ppm to PDG 1836.15).
// Matches engine/web/js/constants.js:132 and the Python proof
// (scripts/constants.py). Written with x₊ = 1/α so N_eff/α = N_eff·x₊,
// keeping the expression constexpr-division-free. Supersedes the
// 2026-pre-F9 erroneous form N_eff·x₊ + TAU·(b₃+N_c)/(N_eff+b₃) (= 3519.97,
// ~1.91× too large; produced negative dM in decay-rates.js).
inline constexpr double PROTON_RATIO =
    static_cast<double>(N_EFF) * X_PLUS          // = N_eff / α
    + static_cast<double>(N_BASE) * static_cast<double>(N_EFF)
    + static_cast<double>(N_C);

// Derived mass and length scales
// M_PROTON = K_B * PROTON_RATIO ≈ 0.511 * 1836.47 ≈ 938.4 MeV — the PHYSICAL
// proton mass scale (m_p/m_e ≈ 1836.47, ~174 ppm to PDG), now matching the
// JS (constants.js) and Python (constants.py) definitions. PROTON_RATIO
// encodes the canonical ontic combination N_eff/α + N_base·N_eff + N_c
// (FTD-0016) and feeds the atom engine.
inline constexpr double M_PROTON = K_B * PROTON_RATIO;   // physical proton mass scale (MeV)
inline constexpr double R_BOHR   = 4.0 * PI / (K_B * ALPHA);  // FTD Bohr radius

// ============================================================================
// Layer 6b: Electroweak Scale (Higgs sector)
// ============================================================================
// Higgs VEV: v = M_P · √(2π) · α⁸ [THEOREM]
// In simulation units (M_P = 1): V_HIGGS_SIM = √(2π) · α⁸
// Physical: 246.09 GeV (0.05% from experimental 246.22 GeV)
inline constexpr double V_HIGGS = 246.09;  // GeV (physical units for reference)

// Higgs mass: m_H = (N_eff / α²) · m_e [SELECTION]
// = 13 / (1/137.036)² × 0.511 MeV = 124.8 GeV (0.24% from 125.1 GeV)
inline constexpr double M_HIGGS = 124.8;   // GeV

// Higgs self-coupling: λ_H = m_H² / (2·v²) [DERIVED]
inline constexpr double LAMBDA_HIGGS = (124.8 * 124.8) / (2.0 * 246.09 * 246.09);

// ============================================================================
// FTD-0131 derived gravity (Scale-1 ParticleEngine only)
// ============================================================================
// Gravitational charge q_g = m/m_P with unit coupling; two-body α_G(e,e) =
// (m_e/m_P)² ≈ 1.75e-45. G_PE = 1/(4π·m_P²) in the engine's 4π convention.
// Scale 0/4/5 substrate demos retain lattice-toy G_N from gauge_couplings.h.
inline constexpr double M_PLANCK_GEV     = 1.22089e19;
inline constexpr double M_PLANCK_MEV     = M_PLANCK_GEV * 1.0e3;
inline constexpr double G_DERIVED        = 1.0 / (4.0 * PI * M_PLANCK_MEV * M_PLANCK_MEV);
inline constexpr double ALPHA_G_ELECTRON = (K_B / M_PLANCK_MEV) * (K_B / M_PLANCK_MEV);
inline constexpr double G_PE             = G_DERIVED;

}  // namespace ontic
}  // namespace ftd
