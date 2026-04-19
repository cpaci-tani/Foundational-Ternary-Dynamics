// qcd_one_loop_perturbative.cpp
//
// [IMPOSED] — Imported one-loop QCD running coupling from perturbative QFT.
// NOT a lattice-measured β-function.
//
// This file implements `ftd::ontic::alpha_s_running(Q_GeV)`, which evaluates the
// standard perturbative-QCD formula
//
//     α_s(Q) = 4π / (b₀ · ln(Q²/Λ²))                [one-loop, 5 active flavors]
//
// using the FTD-derived values b₀ = B0_NF5 = 23/3 and Λ = LAMBDA_QCD. The
// *functional form* is imported from perturbative QFT (Gross–Wilczek–Politzer
// asymptotic freedom); only the coefficient b₀ is derived within FTD.
//
// Epistemic tag: [IMPOSED] / [PARAMETRIC]
// -----------------------------------------------------------------------------
// This formula is an external-physics insertion. The lattice has never been
// measured to reproduce it directly. The EFT Recovery Program (Phase 2) aims
// to *measure* the β-function via real-space blocking; that measurement is in
// `scripts/benchmarks/measure_beta_function.py` and is where the genuine,
// lattice-native coupling evolution will live. Until that measurement
// completes, `alpha_s_running()` remains an imported perturbative formula.
//
// Rename history:
//   2026-04-19: Renamed from engine/src/ontic_running_coupling.cpp to
//               engine/src/eft/qcd_one_loop_perturbative.cpp to reserve the
//               name `running_coupling.cpp` for the Phase-2 measured flow
//               and to reflect that the contents are parametric insertion,
//               not lattice derivation. See
//               docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §0.
//
// -----------------------------------------------------------------------------

#include "ftd/ontic/gauge_couplings.h"

#include <cmath>

namespace ftd {
namespace ontic {

// α_s(Q) = 4π / (b₀ · ln(Q²/Λ²))  [one-loop running, 5 active flavors]
// Valid for m_b < Q < m_t. Returns 1.0 in the non-perturbative regime
// (Q ≤ Λ_QCD) as a finite placeholder — NOT a physical value in that regime.
double alpha_s_running(double Q_GeV) {
    if (Q_GeV <= LAMBDA_QCD) return 1.0;  // non-perturbative
    double log_ratio = std::log(Q_GeV * Q_GeV / (LAMBDA_QCD * LAMBDA_QCD));
    if (log_ratio <= 0.0) return 1.0;
    return 4.0 * PI / (B0_NF5 * log_ratio);
}

}  // namespace ontic
}  // namespace ftd
