// ontic_running_coupling.cpp
//
// Out-of-line definition of alpha_s_running(), extracted from
// engine/include/ftd/ontic.h per extraction-audit ticket O2.
//
// A pure-constants header should contain no function bodies; moving the
// one-loop QCD running coupling here keeps ontic/gauge_couplings.h free
// of code while preserving the exact behaviour previously inlined in
// the monolithic ontic.h.

#include "ftd/ontic/gauge_couplings.h"

#include <cmath>

namespace ftd {
namespace ontic {

// α_s(Q) = 4π / (b₀ · ln(Q²/Λ²))  [one-loop running]
// Valid for 5 active flavors: m_b < Q < m_t.
// Returns 1.0 in the non-perturbative regime (Q ≤ Λ_QCD).
double alpha_s_running(double Q_GeV) {
    if (Q_GeV <= LAMBDA_QCD) return 1.0;  // non-perturbative
    double log_ratio = std::log(Q_GeV * Q_GeV / (LAMBDA_QCD * LAMBDA_QCD));
    if (log_ratio <= 0.0) return 1.0;
    return 4.0 * PI / (B0_NF5 * log_ratio);
}

}  // namespace ontic
}  // namespace ftd
