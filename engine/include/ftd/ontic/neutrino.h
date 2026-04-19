#pragma once
/**
 * ontic/neutrino.h — Layer 7b: Absolute Neutrino Masses (Seesaw).
 *
 * Contents:
 *   Layer 7b: Absolute Neutrino Masses (M_D_NEUTRINO, M_R_NEUTRINO,
 *             M_NU_1/2/3, SUM_M_NU, M_BETA)
 *
 * Depends on: none (all values are pre-computed doubles).
 * (PMNS mixing angles SIN2_THETA12/23/13 + DM2_RATIO live in
 *  ontic/master_quadratic.h alongside the framework integers.)
 */

namespace ftd {
namespace ontic {

// ============================================================================
// Layer 7b: Absolute Neutrino Masses (Seesaw Mechanism)
// ============================================================================
// The Type-I seesaw mechanism with FTD-derived parameters:
//
//   m_D = v_Higgs * alpha               [SELECTION: neutrino Yukawa = alpha]
//   M_R = (N_c/N_base) * v / alpha^4    [SELECTION: framework integers]
//
// Combined result:
//   m3 = m_D^2 / M_R = v * (N_base/N_c) * alpha^6
//      = m_P * sqrt(2pi) * (4/3) * alpha^14
//
// Exponent 14 = 2*b_3 = 2*7 (QCD beta function doubled)
// Factor  4/3 = N_base/N_c (spinor/color ratio)
//
// The mass-squared ratio Dm2_31/Dm2_21 = 100/3 [THEOREM] fixes all three
// masses once m3 is known. The hierarchical seesaw gives m1 ~ 0.
//
// Epistemic status: [SELECTION] — the seesaw mechanism is adopted from
// standard physics, not derived from FTD axioms. The m_D = v*alpha
// identification is argued but not proven inevitable.

// Dirac neutrino mass: m_D = v * alpha ~ 1.796 GeV
// (Note: m_D/m_tau ~ 1.01, near the tau mass — natural for 3rd gen.)
// Pre-computed: V_HIGGS * ALPHA = 246.09 * 0.007297 = 1.796 GeV
inline constexpr double M_D_NEUTRINO = 1.796;  // GeV

// Right-handed Majorana mass: M_R = (N_c/N_base) * v / alpha^4
// = 0.75 * 246.09 / (0.007297)^4 = 6.509e10 GeV (intermediate scale)
inline constexpr double M_R_NEUTRINO = 6.509e10;  // GeV

// Heaviest neutrino mass (from seesaw): m3 = v * (N_base/N_c) * alpha^6
// = 0.04955 eV = 49.55 meV
inline constexpr double M_NU_3 = 4.955e-2;  // eV

// Middle neutrino mass: m2 = m3 * sqrt(N_c) / (b_3 + N_c)
// = m3 * sqrt(3)/10 = 8.58 meV
inline constexpr double M_NU_2 = 8.58e-3;  // eV

// Lightest neutrino mass: m1 = m3 * (m_e/m_tau)^2 = m3 / 3477^2
// = 4.1 neV (effectively zero)
inline constexpr double M_NU_1 = 4.1e-9;  // eV

// Sum of neutrino masses: Sigma = m1 + m2 + m3 ~ 58.1 meV
// Must satisfy: Sigma < 120 meV (Planck+BAO cosmological bound, 2024)
inline constexpr double SUM_M_NU = 5.813e-2;  // eV

// Effective electron-neutrino mass (for beta decay):
// m_beta = sqrt(|U_e1|^2 m1^2 + |U_e2|^2 m2^2 + |U_e3|^2 m3^2)
// ~ 8.3 meV (below KATRIN bound of 450 meV)
inline constexpr double M_BETA = 8.3e-3;  // eV

}  // namespace ontic
}  // namespace ftd
