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

// Genesis threshold: energy needed to CREATE a new particle.
// Must fill all N_c color channels: K_GENESIS = N_c · K_B
inline constexpr double K_GENESIS = K_B * N_C;

// ============================================================================
// Layer 6c: Mass Ratios (from framework integers)
// ============================================================================
// All mass ratios derive from {N_c, N_base, b_3, N_eff} — no free parameters.
//
//   MU_RATIO   = 3·b₃·(b₃ + N_c) - N_c       = 3·7·10 - 3 = 207
//   TAU_RATIO  = (N_eff + N_base)·MU - 2·N_c·b₃ = 17·207 - 42 = 3477
//   PROTON_RATIO = N_eff·x₊ + TAU·(b₃+N_c)/(N_eff+b₃)

inline constexpr int    MU_RATIO  = 3 * B_3 * (B_3 + N_C) - N_C;        // 207
inline constexpr int    TAU_RATIO = (N_EFF + N_BASE) * MU_RATIO
                                    - 2 * N_C * B_3;                      // 3477

// Note: PROTON_RATIO uses x₊ = 1/α directly (avoids constexpr division issues)
inline constexpr double PROTON_RATIO =
    static_cast<double>(N_EFF) * X_PLUS
    + static_cast<double>(TAU_RATIO) * (B_3 + N_C)
      / static_cast<double>(N_EFF + B_3);

// Derived mass and length scales
// NOTE: M_PROTON = K_B * PROTON_RATIO ≈ 0.511 * 3520 ≈ 1799 MeV.
// This is a framework-derived composite scale, NOT the physical proton mass
// (938.3 MeV, ratio m_p/m_e ≈ 1836). PROTON_RATIO (~3520) encodes the
// ontic integer combination N_eff·x₊ + TAU·(b₃+N_c)/(N_eff+b₃) and is
// used as an internal mass scale for the atom engine.
inline constexpr double M_PROTON = K_B * PROTON_RATIO;   // framework mass scale (MeV)
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

}  // namespace ontic
}  // namespace ftd
