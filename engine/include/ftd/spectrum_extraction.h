#pragma once
/**
 * Two-state spectrum extraction from a one-dimensional correlator C(τ).
 *
 * Required infrastructure for the Cluster A BCC band-spectrum campaign
 * (PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md). The pre-registered prediction
 * is that the master-quadratic roots (x₊ ≈ 137.04, x₋ ≈ 3.024) appear
 * as decay rates of a thermalized two-point function on σ_BCC, with
 * calibration-invariant ratio x₊/x₋ ≈ 45.31.
 *
 * Two extractors are provided so disagreement between them is itself a
 * falsification signal:
 *
 *   1. Matrix-Prony (single-operator). Solves the autoregressive
 *      recurrence  C(τ+2) − a·C(τ+1) − b·C(τ) = 0  by least squares
 *      across τ. The two-state decay rates λ₁, λ₂ are roots of
 *      x² − a x − b = 0; amplitudes are recovered by linear least-
 *      squares against the model C(τ) = A₁ e^(−λ₁ τ) + A₂ e^(−λ₂ τ).
 *
 *   2. Generalized eigenvalue (GEVP, two-operator). Builds the 2×2
 *      Hermitian matrices C(τ₀) and C(τ₀+1) of operator-pair
 *      correlators, then solves det(C(τ₀+1) − μ·C(τ₀)) = 0. The decay
 *      rates are λ_n = −log(μ_n).
 *
 * Both return a TwoStateSpectrum struct with eigenvalues sorted so that
 * x_plus is the LARGER eigenvalue (matching the master-quadratic
 * convention x₊ > x₋). If the extractor fails (negative discriminant,
 * non-positive eigenvalues, etc.), `valid` is set to false.
 *
 * Header-only; no dynamic allocation in inner loops; small O(N_lags)
 * least-squares is implemented inline for portability.
 */

#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace ftd {

struct TwoStateSpectrum {
    double x_plus  = 0.0;   // larger decay rate
    double x_minus = 0.0;   // smaller decay rate
    double amp_plus  = 0.0;
    double amp_minus = 0.0;
    bool   valid    = false;
    const char* failure_reason = "";
};

namespace detail {

// Minimal 2x2 generalized eigenvalue solver for symmetric A, B with B SPD.
// Solves A v = μ B v in closed form.
// Returns (μ_plus, μ_minus) sorted descending. Sets valid=false if B is singular
// or the discriminant is negative.
inline std::pair<std::array<double, 2>, bool> gevp_2x2(
    const std::array<double, 4>& A,   // row-major: A00, A01, A10, A11
    const std::array<double, 4>& B)
{
    // (A − μ B) v = 0  →  det(A − μ B) = 0
    //   det = (A00 − μ B00)(A11 − μ B11) − (A01 − μ B01)(A10 − μ B10)
    // Symmetric assumption: A01=A10, B01=B10.
    const double a00 = A[0], a01 = A[1], a11 = A[3];
    const double b00 = B[0], b01 = B[1], b11 = B[3];
    // Quadratic in μ:  α μ² + β μ + γ = 0
    //   α = b00 b11 − b01²
    //   β = −(a00 b11 + a11 b00 − 2 a01 b01)
    //   γ = a00 a11 − a01²
    const double alpha = b00 * b11 - b01 * b01;
    const double beta  = -(a00 * b11 + a11 * b00 - 2.0 * a01 * b01);
    const double gamma_ = a00 * a11 - a01 * a01;
    if (std::abs(alpha) < 1e-30) return {{0.0, 0.0}, false};
    const double disc = beta * beta - 4.0 * alpha * gamma_;
    if (disc < 0.0) return {{0.0, 0.0}, false};
    const double sd = std::sqrt(disc);
    double mu1 = (-beta + sd) / (2.0 * alpha);
    double mu2 = (-beta - sd) / (2.0 * alpha);
    if (mu1 < mu2) std::swap(mu1, mu2);
    return {{mu1, mu2}, true};
}

// 2x2 linear solve A x = b for amplitudes. Returns valid=false if singular.
inline bool solve_2x2(double a00, double a01, double a10, double a11,
                      double b0, double b1, double& x0, double& x1) {
    const double det = a00 * a11 - a01 * a10;
    if (std::abs(det) < 1e-30) return false;
    x0 = ( a11 * b0 - a01 * b1) / det;
    x1 = (-a10 * b0 + a00 * b1) / det;
    return true;
}

}  // namespace detail

// Matrix-Prony two-state extractor. Input: C(τ) for τ = 0..N-1, with N ≥ 5.
// Solves the AR(2) recurrence by least-squares over τ ∈ [tau0, N-3].
// Eigenvalues x_n = −log(λ_n); amplitudes recovered by 2x2 least-squares
// against (C(τ0), C(τ0+1)).
inline TwoStateSpectrum extract_two_state_prony(const std::vector<double>& C,
                                                  int tau0 = 1)
{
    TwoStateSpectrum out;
    const int N = static_cast<int>(C.size());
    if (N < tau0 + 4) {
        out.failure_reason = "Prony: insufficient samples (need tau0+4)";
        return out;
    }

    // Build normal equations for [a, b] from C(τ+2) = a C(τ+1) + b C(τ),
    // τ ∈ [tau0, N−3].
    double s11 = 0, s12 = 0, s22 = 0, r1 = 0, r2 = 0;
    for (int t = tau0; t <= N - 3; ++t) {
        const double c0 = C[t];
        const double c1 = C[t+1];
        const double c2 = C[t+2];
        s11 += c1 * c1;
        s12 += c1 * c0;
        s22 += c0 * c0;
        r1  += c2 * c1;
        r2  += c2 * c0;
    }
    double a = 0, b = 0;
    if (!detail::solve_2x2(s11, s12, s12, s22, r1, r2, a, b)) {
        out.failure_reason = "Prony: normal equations singular";
        return out;
    }

    // Roots of x² − a x − b = 0 are the per-step decay multipliers λ_n.
    const double disc = a * a + 4.0 * b;
    if (disc < 0.0) {
        out.failure_reason = "Prony: complex roots (not pure exponentials)";
        return out;
    }
    const double sd = std::sqrt(disc);
    double lam1 = 0.5 * (a + sd);
    double lam2 = 0.5 * (a - sd);
    if (lam1 < lam2) std::swap(lam1, lam2);
    if (lam1 <= 0.0 || lam2 <= 0.0) {
        out.failure_reason = "Prony: non-positive multiplier (unstable mode)";
        return out;
    }

    // Per-step decay multiplier λ → continuous eigenvalue x = −log(λ).
    // Convention: x_plus is the LARGER eigenvalue (faster decay).
    double xL = -std::log(lam1);   // larger |x| if lam1 < 1
    double xS = -std::log(lam2);
    if (xL < xS) std::swap(xL, xS);

    // Solve for amplitudes from (C(τ0), C(τ0+1)) = (A1 e^{−xL τ0} + A2 e^{−xS τ0}, ...)
    const double e1_t0 = std::exp(-xL * tau0);
    const double e2_t0 = std::exp(-xS * tau0);
    const double e1_t1 = std::exp(-xL * (tau0 + 1));
    const double e2_t1 = std::exp(-xS * (tau0 + 1));
    double A1 = 0, A2 = 0;
    if (!detail::solve_2x2(e1_t0, e2_t0, e1_t1, e2_t1, C[tau0], C[tau0+1], A1, A2)) {
        out.failure_reason = "Prony: amplitude solve singular";
        return out;
    }

    out.x_plus    = xL;
    out.x_minus   = xS;
    out.amp_plus  = A1;
    out.amp_minus = A2;
    out.valid     = true;
    return out;
}

// Two-operator GEVP extractor. Input: three correlator series for the
// 2-operator system (C00, C01, C11), each indexed by τ. Solves
//   C(τ0+1) v = μ C(τ0) v  (2×2 generalized eigenvalue),
// with x_n = −log(μ_n).
inline TwoStateSpectrum extract_two_state_gevp(const std::vector<double>& C00,
                                                  const std::vector<double>& C01,
                                                  const std::vector<double>& C11,
                                                  int tau0 = 1)
{
    TwoStateSpectrum out;
    const int N = static_cast<int>(C00.size());
    if (N < tau0 + 2 || (int)C01.size() < tau0 + 2 || (int)C11.size() < tau0 + 2) {
        out.failure_reason = "GEVP: insufficient samples";
        return out;
    }
    const std::array<double, 4> Bmat = {C00[tau0],   C01[tau0],   C01[tau0],   C11[tau0]};
    const std::array<double, 4> Amat = {C00[tau0+1], C01[tau0+1], C01[tau0+1], C11[tau0+1]};
    auto [mus, ok] = detail::gevp_2x2(Amat, Bmat);
    if (!ok) {
        out.failure_reason = "GEVP: 2x2 solve singular or complex";
        return out;
    }
    if (mus[0] <= 0.0 || mus[1] <= 0.0) {
        out.failure_reason = "GEVP: non-positive eigenvalue";
        return out;
    }
    out.x_plus  = -std::log(mus[1]);   // smaller mu → larger x
    out.x_minus = -std::log(mus[0]);   // larger mu  → smaller x
    if (out.x_plus < out.x_minus) std::swap(out.x_plus, out.x_minus);
    out.amp_plus  = 0.0;   // amplitudes not extracted in this 2x2 form
    out.amp_minus = 0.0;
    out.valid = true;
    return out;
}

}  // namespace ftd
