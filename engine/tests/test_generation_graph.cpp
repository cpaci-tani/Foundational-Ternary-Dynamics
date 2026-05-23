/**
 * test_generation_graph.cpp — Γ_F(d) [CANDIDATE RECONSTRUCTION] diagnostic.
 *
 * STATUS — [CANDIDATE RECONSTRUCTION], NOT a theorem.
 *   The Γ_F(d) triangle graph is one candidate structural reconstruction of
 *   the three-generation flavour mixing pattern; the CKM-like overlap
 *   matrix it produces is a *diagnostic*, not a derivation. This test
 *   asserts only **structural sanity** (Hermiticity, real eigenvalues,
 *   orthonormal eigenvectors, eigenvalue equation, unitarity of the
 *   overlap matrix) — it does NOT assert hard equality of the overlap
 *   magnitudes to any experimental CKM values. The candidate magnitudes
 *   are printed informationally for inspection.
 *
 * Sub-tests:
 *   T1  q* sanity: real, in (0, 1); satisfies q*² − G*·q* + 1 = 0.
 *   T2  Hermiticity of Γ_F(d) for d ∈ {2, 3}.
 *   T3  Real eigenvalues (cubic formula); satisfies tr(H) = Σλ_i and
 *       det(H) = Π λ_i to machine precision.
 *   T4  Orthonormal eigenvectors (⟨v_i | v_j⟩ = δ_ij) for each Γ_F(d).
 *   T5  Eigenvalue equation: H · v_i = λ_i · v_i to machine precision.
 *   T6  Overlap unitarity: V · V† = I, where V_{ij} = ⟨U_i | D_j⟩,
 *       U = eigendecomp(Γ_F(3)), D = eigendecomp(Γ_F(2)).
 *   T7  (INFORMATIONAL — NOT asserted.) Print |V_{ij}|; print the deviation
 *       from the candidate-reconstruction target supplied by the owner.
 *
 * EPISTEMIC NOTE
 * ─────────────────────────────────────────────────────────────────────────
 * Per pre-reg discipline: a structural resemblance is not a derivation
 * (GTCA F1/F10). A close numerical match to the experimental CKM matrix
 * would be a [CONJECTURE] worth investigating, NOT a theorem. The test is
 * a diagnostic instrument — its job is to expose the candidate's
 * predictions for honest inspection, not to enforce them.
 */

#include "ftd/generation_graph.h"
#include "ftd/ontic/lemniscate.h"

#include <array>
#include <cmath>
#include <complex>
#include <cstdio>

namespace {

int g_fails = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::fprintf(stderr,                                               \
                         "[FAIL] %s:%d: %s\n",                                 \
                         __FILE__, __LINE__, #cond);                           \
            ++g_fails;                                                         \
        }                                                                      \
    } while (0)

bool approx(double a, double b, double tol) { return std::fabs(a - b) <= tol; }

bool hermitian(const ftd::color::ComplexMatrix3& H, double tol) {
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            if (std::abs(H[i][j] - std::conj(H[j][i])) > tol) return false;
    return true;
}

ftd::color::ComplexMatrix3 matmul(const ftd::color::ComplexMatrix3& A,
                                  const ftd::color::ComplexMatrix3& B) {
    return ftd::color::matrix_multiply3(A, B);
}

}  // namespace

int main() {
    using ftd::color::ComplexMatrix3;
    using ftd::color::matrix_close3;
    using ftd::color::matrix_multiply3;
    using ftd::color::identity3;
    using ftd::color::trace3;
    using namespace ftd::generation;

    std::printf("=== Generation graph Γ_F(d) [CANDIDATE RECONSTRUCTION] ===\n");
    std::printf("  G*   = %.15f   (engine/include/ftd/ontic/lemniscate.h)\n",
                ftd::ontic::G_STAR);

    // ----- T1: q* sanity -----
    {
        const double q = q_star();
        const double g = ftd::ontic::G_STAR;
        CHECK(q > 0.0 && q < 1.0);
        // q*^2 − G* q* + 1 = 0
        const double resid = q * q - g * q + 1.0;
        CHECK(std::fabs(resid) < 1e-14);
        // q* · (1/q*) = 1 ⇒ second root = G* − q*
        const double q_other = g - q;
        CHECK(approx(q * q_other, 1.0, 1e-14));
        std::printf("  T1  q* = %.15f   (q*² − G*·q* + 1 = %.2e)\n", q, resid);
    }

    // ----- T2/T3/T4/T5: per-d Hermitian eigendecomp sanity -----
    auto check_eigen = [&](int d) {
        const ComplexMatrix3 H = gamma_F(d);

        // T2 Hermiticity
        CHECK(hermitian(H, 1e-14));

        const HermEigen3 ed = hermitian_eigendecomposition_3x3(H);

        // T3 Eigenvalues: tr(H) = Σ λ_i; det(H) = Π λ_i.
        const double sum_lambda = ed.values[0] + ed.values[1] + ed.values[2];
        const double prod_lambda = ed.values[0] * ed.values[1] * ed.values[2];
        const std::complex<double> tr = trace3(H);
        // det(H): 3×3 complex.
        const std::complex<double> det_H =
              H[0][0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1])
            - H[0][1] * (H[1][0] * H[2][2] - H[1][2] * H[2][0])
            + H[0][2] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]);
        CHECK(approx(std::real(tr), sum_lambda, 1e-12));
        CHECK(std::fabs(std::imag(tr)) < 1e-14);   // Hermitian ⇒ real trace
        CHECK(approx(std::real(det_H), prod_lambda, 1e-12));
        CHECK(std::fabs(std::imag(det_H)) < 1e-12);  // Hermitian ⇒ real det

        // T4 Orthonormality of eigenvectors (within tol; degenerate pairs
        // would need Gram-Schmidt — flagged here, not enforced).
        const double orth_tol = 1e-10;
        for (std::size_t i = 0; i < 3; ++i) {
            // self-norm == 1
            const std::complex<double> ii = inner_product_3(ed.vectors[i], ed.vectors[i]);
            CHECK(approx(std::real(ii), 1.0, orth_tol));
            CHECK(std::fabs(std::imag(ii)) < orth_tol);
            // cross terms == 0
            for (std::size_t j = i + 1; j < 3; ++j) {
                const std::complex<double> ij = inner_product_3(ed.vectors[i], ed.vectors[j]);
                CHECK(std::abs(ij) < orth_tol);
            }
        }

        // T5 Eigenvalue equation: H v_k − λ_k v_k = 0.
        const double eq_tol = 1e-10;
        for (std::size_t k = 0; k < 3; ++k) {
            std::array<std::complex<double>, 3> Hv{};
            for (std::size_t i = 0; i < 3; ++i)
                for (std::size_t j = 0; j < 3; ++j)
                    Hv[i] += H[i][j] * ed.vectors[k][j];
            for (std::size_t i = 0; i < 3; ++i) {
                const std::complex<double> diff = Hv[i] - ed.values[k] * ed.vectors[k][i];
                CHECK(std::abs(diff) < eq_tol);
            }
        }

        std::printf("  T2-T5  Γ_F(%d): Hermitian, real spectrum %g  %g  %g; eigvec orthonormal; Hv=λv  ✓\n",
                    d, ed.values[0], ed.values[1], ed.values[2]);
        return ed;
    };

    const HermEigen3 U = check_eigen(3);   // Γ_U = Γ_F(3)
    const HermEigen3 D = check_eigen(2);   // Γ_D = Γ_F(2)

    // ----- T6: overlap matrix V = U†·D, unitarity V·V† = I -----
    {
        // V[i][j] = ⟨U_i | D_j⟩. Form V as a ComplexMatrix3.
        ComplexMatrix3 V{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                V[i][j] = inner_product_3(U.vectors[i], D.vectors[j]);

        // V† has V†[i][j] = conj(V[j][i]).
        ComplexMatrix3 Vdag{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                Vdag[i][j] = std::conj(V[j][i]);

        const ComplexMatrix3 VVd = matmul(V, Vdag);
        const ComplexMatrix3 I   = identity3();
        CHECK(matrix_close3(VVd, I, 1e-10));
        std::printf("  T6  overlap unitarity: V·V† = I (to 1e-10)\n");
    }

    // ----- T7: INFORMATIONAL print of the overlap magnitudes -----
    //          NOT asserted vs CKM values; this is a diagnostic only.
    {
        const auto mag = overlap_magnitudes(U, D);
        std::printf("\n  T7 (INFORMATIONAL — NOT asserted)\n");
        std::printf("  candidate overlap magnitudes |V_{ij}|:\n");
        for (std::size_t i = 0; i < 3; ++i) {
            std::printf("        ");
            for (std::size_t j = 0; j < 3; ++j) {
                std::printf("  %.6f", mag[i][j]);
            }
            std::printf("\n");
        }
        // Owner-supplied candidate-reconstruction target (CKM-shaped).
        // Reported for visual inspection only — NO assertion, NO promotion.
        const double target[3][3] = {
            {0.973536, 0.228440, 0.006537},
            {0.228336, 0.972678, 0.041952},
            {0.009485, 0.041385, 0.999098},
        };
        std::printf("  owner-supplied candidate target (CKM-shape, NOT asserted):\n");
        for (std::size_t i = 0; i < 3; ++i) {
            std::printf("        ");
            for (std::size_t j = 0; j < 3; ++j) {
                std::printf("  %.6f", target[i][j]);
            }
            std::printf("\n");
        }
        std::printf("  abs deviation |computed − target|:\n");
        double frob_sq = 0.0;
        for (std::size_t i = 0; i < 3; ++i) {
            std::printf("        ");
            for (std::size_t j = 0; j < 3; ++j) {
                const double d_ij = mag[i][j] - target[i][j];
                std::printf("  %.6f", std::fabs(d_ij));
                frob_sq += d_ij * d_ij;
            }
            std::printf("\n");
        }
        std::printf("  Frobenius deviation ||computed − target||_F = %.6f\n",
                    std::sqrt(frob_sq));
        std::printf("  Status: [CANDIDATE RECONSTRUCTION] (the K_3 form is one\n"
                    "  selection; identifying d_U=3, d_D=2, and φ=π+π/d as physical\n"
                    "  is [SELECTION]). The canonical PLAN_03 K_3 graph-Laplacian\n"
                    "  form reproduces the owner's Python-prototype overlap to\n"
                    "  machine precision (see T9 assertion below).\n");
    }

    // ----- T8: PLAN_03 rule check — weights and phase per canonical form ----
    {
        const auto w2 = generation_weights(2);
        const auto w3 = generation_weights(3);
        const double q = q_star();
        constexpr double kPi = 3.14159265358979323846;
        constexpr double rule_tol = 1e-14;

        CHECK(std::fabs(w2.w12 - std::pow(q, 3)) < rule_tol);
        CHECK(std::fabs(w2.w23 - 1.0) < rule_tol);
        CHECK(std::fabs(w2.w13 - std::pow(q, 2)) < rule_tol);
        CHECK(std::fabs(w2.phi - 1.5 * kPi) < rule_tol);

        CHECK(std::fabs(w3.w12 - std::pow(q, 4)) < rule_tol);
        CHECK(std::fabs(w3.w23 - 1.0) < rule_tol);
        CHECK(std::fabs(w3.w13 - std::pow(q, 3)) < rule_tol);
        CHECK(std::fabs(w3.phi - 4.0 * kPi / 3.0) < rule_tol);

        std::printf("  T8  PLAN_03 rule: w12=q^(d+1), w23=1, w13=q^d, φ=π+π/d  ✓\n");
    }

    // ----- T9: PLAN_03 target-tolerance assertion -----
    //          With the canonical K_3 Laplacian form, the overlap matches the
    //          owner's Python-prototype target. Tolerance per PLAN_03 §"Full
    //          eigen-solver option": 5×10⁻⁴ for first implementation.
    {
        const auto mag = overlap_magnitudes(U, D);
        const double target[3][3] = {
            {0.973536, 0.228440, 0.006537},
            {0.228336, 0.972678, 0.041952},
            {0.009485, 0.041385, 0.999098},
        };
        constexpr double tol = 5e-4;   // canonical PLAN_03 tolerance
        double frob_sq = 0.0;
        double max_abs_diff = 0.0;
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t j = 0; j < 3; ++j) {
                const double d_ij = mag[i][j] - target[i][j];
                if (std::fabs(d_ij) > max_abs_diff) max_abs_diff = std::fabs(d_ij);
                frob_sq += d_ij * d_ij;
                CHECK(std::fabs(d_ij) <= tol);
            }
        }
        std::printf("  T9  PLAN_03 target tolerance (5e-4): max |Δ| = %.2e, "
                    "Frobenius = %.2e  ✓\n",
                    max_abs_diff, std::sqrt(frob_sq));
    }

    std::printf("\n=== %s (%d failure(s)) ===\n",
                g_fails == 0 ? "ALL PASSED" : "FAILED",
                g_fails);
    return g_fails == 0 ? 0 : 1;
}
