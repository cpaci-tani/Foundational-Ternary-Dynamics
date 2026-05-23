/**
 * test_branch_holonomy_gap.cpp — verifies the Z_2 torus branch-twist gap
 *
 *     λ_min  =  4 sin²( π / (2N) )                  (eq. 3)
 *
 * for the signed Laplacian on a periodic ring of N sites with a single Z_2
 * branch twist (one or any odd number of edge sign flips). The supporting
 * primitive lives in `engine/include/ftd/branch_holonomy.h`. The test is
 * pure unit / theory: no engine state, no RenderBridge, no physics
 * toggles — only the signed Laplacian, an inline Jacobi eigensolver, and
 * comparison against the closed-form spectra.
 *
 * SCOPE OF VERIFICATION
 *   T1. Trivial holonomy (all σ = +1): spectrum matches the periodic
 *       closed form 4 sin²(πk/N); λ_min = 0 (kernel of constants).
 *   T2. Z_2 twist (single flip): spectrum matches the antiperiodic-like
 *       closed form 4 sin²(π(2m+1)/(2N)); λ_min equals eq. 3.
 *   T3. Gauge equivalence: flipping a different single edge gives the
 *       identical spectrum (the spectrum depends only on the Z_2 cycle
 *       holonomy, eq. 2).
 *   T4. Parity: any even number of flips ⇒ trivial spectrum; any odd
 *       number ⇒ twisted spectrum.
 *   T5. apply() consistency: apply(U) agrees with build_matrix() * U to
 *       round-off.
 *
 * EPISTEMIC NOTE
 *   Equation 3 is [THEOREM] (standard antiperiodic-BC fact). This test is
 *   the constructive verification at N ∈ {4, 8, 16, 32}. It promotes no
 *   LEDGER claim and touches no engine physics.
 */

#include "ftd/branch_holonomy.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Symmetric Jacobi eigensolver — sorted eigenvalues only (no eigenvectors).
// ---------------------------------------------------------------------------
// Cyclic Jacobi: each outer sweep visits every off-diagonal pair (i,j),
// i < j, in row-major order and applies the Givens rotation that zeroes
// M[i][j]. Convergence is quadratic once the off-diagonal Frobenius norm
// is small; for the small (N ≤ 32) symmetric matrices in this test
// ~5–10 sweeps suffice to drive the off-diagonal sum below 1e-14.
std::vector<double>
jacobi_eigenvalues(std::vector<std::vector<double>> M,
                   int max_sweeps = 100,
                   double tol = 1e-14) {
    const int n = static_cast<int>(M.size());

    auto off_sq = [&]() {
        double s = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const double v = M[static_cast<std::size_t>(i)]
                                  [static_cast<std::size_t>(j)];
                s += v * v;
            }
        }
        return s;
    };

    for (int sweep = 0; sweep < max_sweeps; ++sweep) {
        if (off_sq() < tol) break;
        // Cyclic sweep over all off-diagonal pairs.
        for (int p = 0; p < n; ++p) {
            for (int q = p + 1; q < n; ++q) {
                const double apq = M[static_cast<std::size_t>(p)]
                                     [static_cast<std::size_t>(q)];
                if (std::fabs(apq) < 1e-300) continue;
                const double app = M[static_cast<std::size_t>(p)]
                                     [static_cast<std::size_t>(p)];
                const double aqq = M[static_cast<std::size_t>(q)]
                                     [static_cast<std::size_t>(q)];
                const double theta = 0.5 * std::atan2(2.0 * apq, app - aqq);
                const double c = std::cos(theta);
                const double s = std::sin(theta);

                // Rotate the 2x2 block (p,p),(q,q),(p,q),(q,p).
                M[static_cast<std::size_t>(p)][static_cast<std::size_t>(p)] =
                    c * c * app + 2.0 * c * s * apq + s * s * aqq;
                M[static_cast<std::size_t>(q)][static_cast<std::size_t>(q)] =
                    s * s * app - 2.0 * c * s * apq + c * c * aqq;
                M[static_cast<std::size_t>(p)][static_cast<std::size_t>(q)] = 0.0;
                M[static_cast<std::size_t>(q)][static_cast<std::size_t>(p)] = 0.0;

                // Apply the rotation to all other rows / columns.
                for (int k = 0; k < n; ++k) {
                    if (k == p || k == q) continue;
                    const double akp = M[static_cast<std::size_t>(k)]
                                         [static_cast<std::size_t>(p)];
                    const double akq = M[static_cast<std::size_t>(k)]
                                         [static_cast<std::size_t>(q)];
                    const double new_kp =  c * akp + s * akq;
                    const double new_kq = -s * akp + c * akq;
                    M[static_cast<std::size_t>(k)][static_cast<std::size_t>(p)] = new_kp;
                    M[static_cast<std::size_t>(p)][static_cast<std::size_t>(k)] = new_kp;
                    M[static_cast<std::size_t>(k)][static_cast<std::size_t>(q)] = new_kq;
                    M[static_cast<std::size_t>(q)][static_cast<std::size_t>(k)] = new_kq;
                }
            }
        }
    }

    std::vector<double> eigs(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        eigs[static_cast<std::size_t>(i)] =
            M[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)];
    }
    std::sort(eigs.begin(), eigs.end());
    return eigs;
}

bool approx(double a, double b, double tol) {
    return std::fabs(a - b) <= tol;
}

bool vectors_close(const std::vector<double>& a,
                   const std::vector<double>& b,
                   double tol) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!approx(a[i], b[i], tol)) return false;
    }
    return true;
}

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

}  // namespace

int main() {
    using namespace ftd::branch;

    const int N_values[] = {4, 8, 16, 32};
    constexpr double kSpecTol = 1e-10;   // spectrum match
    constexpr double kGapTol  = 1e-12;   // gap value match (tighter)
    constexpr double kApplyTol = 1e-12;

    std::printf("=== Branch holonomy gap test ===\n");

    // ----- T1: trivial holonomy → matches periodic closed form -----
    for (int N : N_values) {
        std::vector<int> signs(static_cast<std::size_t>(N), +1);
        SignedRing1D op(N, signs);
        CHECK(op.holonomy() == +1);
        auto eigs = jacobi_eigenvalues(op.build_matrix());
        auto closed = trivial_ring_spectrum_closed_form(N);
        std::sort(closed.begin(), closed.end());
        CHECK(vectors_close(eigs, closed, kSpecTol));
        CHECK(std::fabs(eigs.front()) < kGapTol);
        std::printf("  T1  N=%2d  trivial (H=+1): spectrum match, lambda_min = %.3e\n",
                    N, eigs.front());
    }

    // ----- T2: Z_2 twist (single flip) → matches twisted closed form ----
    //          λ_min = 4 sin²(π/(2N))    [THEOREM, eq. 3]
    for (int N : N_values) {
        std::vector<int> signs(static_cast<std::size_t>(N), +1);
        signs[0] = -1;  // flip edge 0
        SignedRing1D op(N, signs);
        CHECK(op.holonomy() == -1);
        auto eigs = jacobi_eigenvalues(op.build_matrix());
        auto closed = twisted_ring_spectrum_closed_form(N);
        std::sort(closed.begin(), closed.end());
        CHECK(vectors_close(eigs, closed, kSpecTol));
        const double gap = torus_branch_twist_gap_1d(N);
        CHECK(approx(eigs.front(), gap, kGapTol));
        std::printf("  T2  N=%2d  twisted (H=-1): spectrum match; lambda_min = %.12g  (theorem: %.12g)\n",
                    N, eigs.front(), gap);
    }

    // ----- T3: gauge equivalence — flipping any single edge gives same spectrum ----
    {
        const int N = 8;
        std::vector<int> sa(static_cast<std::size_t>(N), +1); sa[0] = -1;
        std::vector<int> sb(static_cast<std::size_t>(N), +1); sb[3] = -1;
        std::vector<int> sc(static_cast<std::size_t>(N), +1); sc[7] = -1;
        const auto ea = jacobi_eigenvalues(SignedRing1D(N, sa).build_matrix());
        const auto eb = jacobi_eigenvalues(SignedRing1D(N, sb).build_matrix());
        const auto ec = jacobi_eigenvalues(SignedRing1D(N, sc).build_matrix());
        CHECK(vectors_close(ea, eb, kSpecTol));
        CHECK(vectors_close(ea, ec, kSpecTol));
        std::printf("  T3  gauge equivalence (N=8): flipping edge 0, 3, 7 all give the same spectrum\n");
    }

    // ----- T4: parity — odd #flips → twisted spectrum; even → trivial ----
    {
        const int N = 8;
        std::vector<int> s_odd(static_cast<std::size_t>(N), +1);
        s_odd[1] = -1; s_odd[3] = -1; s_odd[5] = -1;  // 3 flips (odd)
        SignedRing1D op_odd(N, s_odd);
        CHECK(op_odd.holonomy() == -1);
        auto eigs_odd = jacobi_eigenvalues(op_odd.build_matrix());
        auto closed_twisted = twisted_ring_spectrum_closed_form(N);
        std::sort(closed_twisted.begin(), closed_twisted.end());
        CHECK(vectors_close(eigs_odd, closed_twisted, kSpecTol));

        std::vector<int> s_even(static_cast<std::size_t>(N), +1);
        s_even[1] = -1; s_even[4] = -1;  // 2 flips (even)
        SignedRing1D op_even(N, s_even);
        CHECK(op_even.holonomy() == +1);
        auto eigs_even = jacobi_eigenvalues(op_even.build_matrix());
        auto closed_trivial = trivial_ring_spectrum_closed_form(N);
        std::sort(closed_trivial.begin(), closed_trivial.end());
        CHECK(vectors_close(eigs_even, closed_trivial, kSpecTol));

        std::printf("  T4  parity (N=8): 3 flips → twisted spectrum; 2 flips → trivial spectrum\n");
    }

    // ----- T5: apply() == build_matrix() * U ----
    {
        const int N = 8;
        std::vector<int> signs(static_cast<std::size_t>(N), +1); signs[2] = -1;
        SignedRing1D op(N, signs);
        std::vector<double> U(static_cast<std::size_t>(N));
        for (int i = 0; i < N; ++i) {
            U[static_cast<std::size_t>(i)] = std::sin(0.7 * i + 0.3) + 0.25 * std::cos(i);
        }
        std::vector<double> y_apply;
        op.apply(U, y_apply);
        const auto M = op.build_matrix();
        std::vector<double> y_mat(static_cast<std::size_t>(N), 0.0);
        for (int i = 0; i < N; ++i) {
            double acc = 0.0;
            for (int j = 0; j < N; ++j) {
                acc += M[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
                       * U[static_cast<std::size_t>(j)];
            }
            y_mat[static_cast<std::size_t>(i)] = acc;
        }
        CHECK(vectors_close(y_apply, y_mat, kApplyTol));
        std::printf("  T5  apply() consistency: matrix-vector product agrees with apply()\n");
    }

    // ----- Argument validation: invalid inputs throw -----
    {
        bool thrown = false;
        try { SignedRing1D op(1, {1}); } catch (const std::invalid_argument&) { thrown = true; }
        CHECK(thrown);
        thrown = false;
        try { SignedRing1D op(4, {1, 1, 1}); } catch (const std::invalid_argument&) { thrown = true; }
        CHECK(thrown);
        thrown = false;
        try { SignedRing1D op(4, {1, 1, 1, 2}); } catch (const std::invalid_argument&) { thrown = true; }
        CHECK(thrown);
        std::printf("  T6  argument validation: N<2 / wrong size / bad sign all throw\n");
    }

    // ----- T7: PLAN_01 canonical 3D API — k-twist additivity + asymptote ----
    {
        constexpr double kPi = 3.14159265358979323846;

        // Enum / momentum-shift sanity.
        CHECK(holonomy(BranchTwist::Periodic) == +1);
        CHECK(holonomy(BranchTwist::AntiPeriodic) == -1);
        CHECK(std::fabs(branch_momentum_shift(BranchTwist::Periodic)) < 1e-15);
        CHECK(std::fabs(branch_momentum_shift(BranchTwist::AntiPeriodic) - kPi) < 1e-15);

        for (int N : {8, 10, 12, 16, 18, 20, 32, 64}) {
            BranchTwist3 none{};
            BranchTwist3 x{BranchTwist::AntiPeriodic,
                           BranchTwist::Periodic,
                           BranchTwist::Periodic};
            BranchTwist3 xy{BranchTwist::AntiPeriodic,
                            BranchTwist::AntiPeriodic,
                            BranchTwist::Periodic};
            BranchTwist3 xyz{BranchTwist::AntiPeriodic,
                             BranchTwist::AntiPeriodic,
                             BranchTwist::AntiPeriodic};

            CHECK(twist_count(none) == 0);
            CHECK(twist_count(x)    == 1);
            CHECK(twist_count(xy)   == 2);
            CHECK(twist_count(xyz)  == 3);

            const double gap_none = exact_torus_branch_gap(N, none);
            const double gap_x    = exact_torus_branch_gap(N, x);
            const double gap_xy   = exact_torus_branch_gap(N, xy);
            const double gap_xyz  = exact_torus_branch_gap(N, xyz);

            CHECK(std::fabs(gap_none) < 1e-15);
            CHECK(std::fabs(gap_xy  - 2.0 * gap_x) < 1e-13);
            CHECK(std::fabs(gap_xyz - 3.0 * gap_x) < 1e-13);

            // 1-axis gap should equal the existing 1D theorem helper.
            const double expected_1axis = torus_branch_twist_gap_1d(N);
            CHECK(std::fabs(gap_x - expected_1axis) < 1e-13);

            // Lowest 3D mode (m_x=m_y=m_z=0) under x-twist =
            //   4 sin²(π/(2N))  (k_x = π/N, k_y = k_z = 0).
            const double mode000 = torus_laplacian_eigenvalue_3d(N, 0, 0, 0, x);
            CHECK(std::fabs(mode000 - gap_x) < 1e-13);

            // Large-N asymptote: gap_x → π²/N². Check ratio ≈ 1 for N ≥ 32.
            if (N >= 32) {
                const double asym = kPi * kPi
                                    / (static_cast<double>(N)
                                       * static_cast<double>(N));
                const double ratio = gap_x / asym;
                CHECK(std::fabs(ratio - 1.0) < 0.01);
            }
        }
        std::printf("  T7  PLAN_01 3D API: enum + momentum-shift; k-twist additivity (k ∈ {0,1,2,3}); π²/N² asymptote  ✓\n");
    }

    std::printf("=== %s (%d failure(s)) ===\n",
                g_fails == 0 ? "ALL PASSED" : "FAILED",
                g_fails);
    return g_fails == 0 ? 0 : 1;
}
