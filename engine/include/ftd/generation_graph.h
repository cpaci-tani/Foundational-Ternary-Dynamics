#pragma once
/**
 * generation_graph.h — Γ_F(d) triangle graph + 3x3 Hermitian eigensolver.
 *
 * STATUS — [CANDIDATE RECONSTRUCTION], NOT a theorem.
 * ─────────────────────────────────────────────────────────────────────────
 * The Γ_F(d) construction below is a CANDIDATE structural reading of the
 * three-generation flavour graph. The CKM-like overlap matrix it produces
 * is a diagnostic, not a derivation. There is no proof that this is the
 * correct or unique reconstruction; the LEDGER lists it as
 * `[CANDIDATE RECONSTRUCTION]` and the CKM-like overlap as
 * `[RECONSTRUCTION / diagnostic]`. Equating it with the physical CKM
 * matrix is `[CONJECTURE]` at best (GTCA F1/F10 applies — a structural
 * resemblance is not a derivation).
 *
 * DEFINITION
 * ─────────────────────────────────────────────────────────────────────────
 * Let `q* = (G* − √(G*²−4)) / 2`, the smaller root of `x² − G*·x + 1 = 0`
 * (so `q* · (1/q*) = 1` and `q* + 1/q* = G*`). Then Γ_F(d) is the 3×3
 * Hermitian "K_3" matrix on three vertices `(0, 1, 2)` with
 *
 * Γ_F(d) is a **weighted graph Laplacian** of the K_3 triangle on vertices
 * (0, 1, 2), with edge weights and a single Wilson-loop phase on the closing
 * edge (0, 2):
 *
 *     edge (0,1) weight   w_12  =  q*^{d+1}
 *     edge (1,2) weight   w_23  =  1
 *     edge (0,2) weight   w_13  =  q*^d
 *     Wilson-loop phase   φ(d)  =  π + π/d        (carried by (0,2) only)
 *
 *           ⎡ w_12 + w_13      −w_12               −w_13 · e^{ iφ} ⎤
 *     L  =  ⎢ −w_12             w_12 + w_23        −w_23           ⎥
 *           ⎣ −w_13 · e^{−iφ}  −w_23                w_13 + w_23    ⎦
 *
 * Diagonals are vertex degrees (sum of incident edge weights); off-diagonals
 * are minus the edge weights; the Wilson-loop holonomy `e^{i 3φ}` distinguishes
 * Γ_F(d) for different `d`. This is the canonical PLAN_03 form — a standard
 * weighted graph Laplacian, *not* a uniform-phase adjacency matrix.
 *
 * The eigenvectors of Γ_U := Γ_F(3) and Γ_D := Γ_F(2), interpreted as
 * "up-type" and "down-type" generation bases, give a CKM-like overlap
 * matrix `V_{ij} = ⟨U_i | D_j⟩` whose magnitudes are reported by the
 * test for inspection.
 *
 * SCOPE
 * ─────────────────────────────────────────────────────────────────────────
 * Pure header-only mathematics. No engine state, no `RenderBridge` touch,
 * no physics-toggle wiring. The 3×3 Hermitian eigensolver uses the
 * Smith-1961 stable cubic formula for eigenvalues and cross-product null
 * vectors for eigenvectors — robust at small matrix size, no external
 * linear-algebra dependency.
 *
 * REUSE OF EXISTING CONSTANTS
 * ─────────────────────────────────────────────────────────────────────────
 * G* is sourced from `engine/include/ftd/ontic/lemniscate.h`
 * (`ftd::ontic::G_STAR`), the canonical engine-side ontic chain. No local
 * duplicate.
 */

#include "ftd/color_center.h"  // reuse ComplexMatrix3 + matrix helpers
#include "ftd/ontic/lemniscate.h"  // ftd::ontic::G_STAR

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <stdexcept>

namespace ftd {
namespace generation {

using ftd::color::ComplexMatrix3;

namespace detail {
inline constexpr double kPi = 3.14159265358979323846;
}

// ---------------------------------------------------------------------------
// q* = (G* − √(G*² − 4)) / 2 — smaller root of x² − G*·x + 1 = 0.
// ---------------------------------------------------------------------------
inline double q_star() {
    const double g = ftd::ontic::G_STAR;
    const double disc = g * g - 4.0;
    if (disc < 0.0) {
        throw std::runtime_error("q_star: G*^2 < 4 (no real root)");
    }
    return 0.5 * (g - std::sqrt(disc));
}

// ---------------------------------------------------------------------------
// Γ_F(d) — the K_3 generation triangle, as a 3×3 Hermitian graph Laplacian.
// Canonical PLAN_03 form (see header comment above).
// ---------------------------------------------------------------------------

// Edge weights + Wilson-loop phase for the K_3 triangle at depth d.
struct GenerationWeights {
    double w12;   // edge (0,1) — q*^{d+1}
    double w23;   // edge (1,2) — 1
    double w13;   // edge (0,2) — q*^d   (carries the Wilson-loop phase)
    double phi;   // loop phase   π + π/d
};

inline GenerationWeights generation_weights(int d) {
    if (d < 1) throw std::invalid_argument("generation_weights: d must be >= 1");
    const double q = q_star();
    return GenerationWeights{
        std::pow(q, static_cast<double>(d + 1)),       // w12
        1.0,                                            // w23
        std::pow(q, static_cast<double>(d)),           // w13
        detail::kPi + detail::kPi / static_cast<double>(d),  // phi
    };
}

inline ComplexMatrix3 gamma_F_from_weights(const GenerationWeights& w) {
    const std::complex<double> e_phi  = std::polar(1.0, w.phi);
    const std::complex<double> e_mphi = std::conj(e_phi);
    ComplexMatrix3 L{};
    // Diagonals: vertex degree = sum of incident edge weights.
    L[0][0] = w.w12 + w.w13;
    L[1][1] = w.w12 + w.w23;
    L[2][2] = w.w13 + w.w23;
    // Off-diagonals: minus the edge weights; Wilson-loop phase on (0,2)/(2,0).
    L[0][1] = -w.w12;            L[1][0] = -w.w12;
    L[1][2] = -w.w23;            L[2][1] = -w.w23;
    L[0][2] = -w.w13 * e_phi;    L[2][0] = -w.w13 * e_mphi;
    return L;
}

inline ComplexMatrix3 gamma_F(int d) {
    return gamma_F_from_weights(generation_weights(d));
}

// ---------------------------------------------------------------------------
// 3×3 Hermitian eigendecomposition.
// ---------------------------------------------------------------------------
// Eigenvalues via the Smith-1961 stable trigonometric cubic formula;
// eigenvectors via row-cross-product null vectors of (H − λ·I).
// Eigenvalues returned sorted ascending; eigenvectors are unit-norm and
// paired with their eigenvalue (vectors[k] is the eigenvector of values[k]).

inline std::array<double, 3>
hermitian_eigenvalues_3x3(const ComplexMatrix3& H) {
    const double p1 = std::real(H[0][0] + H[1][1] + H[2][2]) / 3.0;
    ComplexMatrix3 Hp = H;
    Hp[0][0] -= p1; Hp[1][1] -= p1; Hp[2][2] -= p1;
    double sq = 0.0;
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            sq += std::norm(Hp[i][j]);
    const double p2 = std::sqrt(sq / 6.0);
    if (p2 < 1e-15) {
        return {p1, p1, p1};
    }
    ComplexMatrix3 B{};
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            B[i][j] = Hp[i][j] / p2;
    const std::complex<double> det =
          B[0][0] * (B[1][1] * B[2][2] - B[1][2] * B[2][1])
        - B[0][1] * (B[1][0] * B[2][2] - B[1][2] * B[2][0])
        + B[0][2] * (B[1][0] * B[2][1] - B[1][1] * B[2][0]);
    double r = 0.5 * std::real(det);
    if (r < -1.0) r = -1.0;
    if (r >  1.0) r =  1.0;
    const double phi = std::acos(r) / 3.0;
    std::array<double, 3> e;
    e[0] = p1 + 2.0 * p2 * std::cos(phi + 2.0 * detail::kPi / 3.0);  // smallest
    e[1] = p1 + 2.0 * p2 * std::cos(phi + 4.0 * detail::kPi / 3.0);  // middle
    e[2] = p1 + 2.0 * p2 * std::cos(phi);                            // largest
    return e;
}

inline std::array<std::complex<double>, 3>
cross_product_3(const std::array<std::complex<double>, 3>& a,
                const std::array<std::complex<double>, 3>& b) {
    return {{
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    }};
}

inline double vec3_norm(const std::array<std::complex<double>, 3>& v) {
    double s = 0.0;
    for (std::size_t i = 0; i < 3; ++i) s += std::norm(v[i]);
    return std::sqrt(s);
}

inline std::array<std::complex<double>, 3>
null_vector_3x3(const ComplexMatrix3& M) {
    const std::array<std::complex<double>, 3> r0 = {M[0][0], M[0][1], M[0][2]};
    const std::array<std::complex<double>, 3> r1 = {M[1][0], M[1][1], M[1][2]};
    const std::array<std::complex<double>, 3> r2 = {M[2][0], M[2][1], M[2][2]};
    const auto v01 = cross_product_3(r0, r1);
    const auto v02 = cross_product_3(r0, r2);
    const auto v12 = cross_product_3(r1, r2);
    const double n01 = vec3_norm(v01);
    const double n02 = vec3_norm(v02);
    const double n12 = vec3_norm(v12);
    auto best = v01;
    double best_n = n01;
    if (n02 > best_n) { best = v02; best_n = n02; }
    if (n12 > best_n) { best = v12; best_n = n12; }
    if (best_n < 1e-15) {
        return {std::complex<double>(1.0, 0.0), 0.0, 0.0};
    }
    for (auto& z : best) z /= best_n;
    return best;
}

struct HermEigen3 {
    std::array<double, 3> values;   // sorted ascending
    std::array<std::array<std::complex<double>, 3>, 3> vectors;  // vectors[k] ↔ values[k]
};

inline HermEigen3 hermitian_eigendecomposition_3x3(const ComplexMatrix3& H) {
    HermEigen3 r;
    r.values = hermitian_eigenvalues_3x3(H);
    for (std::size_t k = 0; k < 3; ++k) {
        ComplexMatrix3 M = H;
        for (std::size_t i = 0; i < 3; ++i) M[i][i] -= r.values[k];
        r.vectors[k] = null_vector_3x3(M);
    }
    return r;
}

// ---------------------------------------------------------------------------
// Hermitian inner product ⟨a | b⟩ = Σ_k a_k^* · b_k.
// ---------------------------------------------------------------------------
inline std::complex<double>
inner_product_3(const std::array<std::complex<double>, 3>& a,
                const std::array<std::complex<double>, 3>& b) {
    std::complex<double> s{0.0, 0.0};
    for (std::size_t i = 0; i < 3; ++i) s += std::conj(a[i]) * b[i];
    return s;
}

// ---------------------------------------------------------------------------
// Overlap matrix V_{ij} = ⟨U_i | D_j⟩ between two eigendecompositions.
// Returns the 3x3 magnitudes |V_{ij}|.
// ---------------------------------------------------------------------------
inline std::array<std::array<double, 3>, 3>
overlap_magnitudes(const HermEigen3& U, const HermEigen3& D) {
    std::array<std::array<double, 3>, 3> out{};
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            out[i][j] = std::abs(inner_product_3(U.vectors[i], D.vectors[j]));
    return out;
}

}  // namespace generation
}  // namespace ftd
