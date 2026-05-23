/**
 * test_z3_color_center.cpp — Z_3 center-closure [THEOREM] verification.
 *
 * Tests the primitives in `engine/include/ftd/color_center.h`:
 *
 *   T1  Center-closed configurations recognised as closed:
 *         meson (q + q̄)            : 1 + 2 ≡ 0
 *         baryon (q + q + q)        : 1+1+1 ≡ 0
 *         anti-baryon (q̄ + q̄ + q̄): 2+2+2 ≡ 0
 *         tetraquark (q+q + q̄+q̄)  : 1+1+2+2 ≡ 0
 *         pentaquark (4q + q̄)      : 4 + 2 ≡ 0
 *
 *   T2  Non-closed (confined) configurations correctly rejected:
 *         single quark (q)          : 1 ≢ 0
 *         diquark (q + q)           : 1+1 ≡ 2 ≢ 0
 *         single antiquark (q̄)     : 2 ≢ 0
 *         qq̄q                      : 1+2+1 ≡ 1 ≢ 0
 *
 *   T3  Z_3 arithmetic (canonicalisation, anti-charge involution):
 *         (1+2) ≡ 0,  (2+2) ≡ 1,  −1 = 2,  −2 = 1,  −(−x) = x.
 *
 *   T4  Cyclic-sum identity for the generator:
 *         1 + ω + ω² = 0          (Z_3 character orthogonality).
 *         ω³ = 1.
 *
 *   T5  Center projector form (eq. 2):
 *         P_0 = (1/3)(I + Z + Z²)   numerically equals  diag(1, 0, 0).
 *
 *   T6  Idempotence and rank of P_0:
 *         P_0² = P_0    [THEOREM, finite-center closure].
 *         tr(P_0) = 1   (one-dimensional image — the c = 0 subspace).
 *
 * EPISTEMIC NOTE
 * ─────────────────────────────────────────────────────────────────────────
 * The closure characterisation (eq. 1) and the projector identity (eq. 2)
 * are `[THEOREM]` from finite-group theory and linear algebra. This test
 * is the constructive verification; it promotes no LEDGER claim and
 * touches no engine physics. The open-flux penalty (a CANDIDATE PRINCIPLE
 * in the FTD-side formulation) is explicitly OUT OF SCOPE here.
 */

#include "ftd/color_center.h"

#include <complex>
#include <cstdio>
#include <vector>

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

}  // namespace

int main() {
    using namespace ftd::color;

    std::printf("=== Z_3 color-center closure test ===\n");

    // ----- T1: closed configurations recognised as closed -----
    {
        const Z3Charge q  = Z3Charge::color();      // 1
        const Z3Charge qb = Z3Charge::anticolor();  // 2

        CHECK(is_center_closed({q, qb}));               // meson
        CHECK(is_center_closed({q, q, q}));             // baryon
        CHECK(is_center_closed({qb, qb, qb}));          // anti-baryon
        CHECK(is_center_closed({q, q, qb, qb}));        // tetraquark
        CHECK(is_center_closed({q, q, q, q, qb}));      // pentaquark

        // Vacuum / pure-singlet limit.
        CHECK(is_center_closed({Z3Charge::neutral()}));

        std::printf("  T1  closed: meson, baryon, anti-baryon, tetraquark, pentaquark  ALL ≡ 0 mod 3\n");
    }

    // ----- T2: non-closed (confined) configurations correctly rejected -----
    {
        const Z3Charge q  = Z3Charge::color();
        const Z3Charge qb = Z3Charge::anticolor();

        CHECK(!is_center_closed({q}));               // 1
        CHECK(!is_center_closed({q, q}));            // 2
        CHECK(!is_center_closed({qb}));              // 2
        CHECK(!is_center_closed({q, qb, q}));        // 1 + 2 + 1 = 4 ≡ 1
        CHECK(!is_center_closed({qb, qb}));          // 2 + 2 = 4 ≡ 1
        CHECK(!is_center_closed({q, q, q, q}));      // 4 ≡ 1

        std::printf("  T2  not closed: q, qq, q̄, qq̄q, q̄q̄, qqqq  ALL ≢ 0 mod 3\n");
    }

    // ----- T3: Z_3 arithmetic + anti-charge involution -----
    {
        const Z3Charge zero = Z3Charge::neutral();
        const Z3Charge q    = Z3Charge::color();
        const Z3Charge qb   = Z3Charge::anticolor();

        CHECK((q + qb) == zero);     // 1 + 2 ≡ 0
        CHECK((qb + qb).value() == 1);   // 2 + 2 ≡ 1
        CHECK((-q) == qb);            // −1 ≡ 2
        CHECK((-qb) == q);            // −2 ≡ 1
        CHECK(-(-q) == q);            // double anti
        CHECK(Z3Charge(7).value() == 1);  // 7 mod 3
        CHECK(Z3Charge(-1).value() == 2); // (−1) mod 3 = 2
        CHECK(Z3Charge(-4).value() == 2); // (−4) mod 3 = 2

        std::printf("  T3  arithmetic + anti-involution: all expected\n");
    }

    // ----- T4: cyclic-sum identity 1 + ω + ω² = 0 and ω³ = 1 -----
    {
        const std::complex<double> w  = z3_omega();
        const std::complex<double> w2 = w * w;
        const std::complex<double> w3 = w2 * w;
        constexpr double tol = 1e-14;

        const std::complex<double> sum = 1.0 + w + w2;
        CHECK(std::abs(sum) < tol);
        CHECK(std::abs(w3 - 1.0) < tol);
        CHECK(std::abs(w2 - std::conj(w)) < tol);  // ω² = ω̄

        std::printf("  T4  ω + ω² + 1 = %.2e   ω³ − 1 = %.2e\n",
                    std::abs(sum), std::abs(w3 - 1.0));
    }

    // ----- T5: P_0 = (1/3)(I + Z + Z²) equals diag(1, 0, 0) -----
    {
        const ComplexMatrix3 P0 = center_projector_p0();
        ComplexMatrix3 target{};
        target[0][0] = 1.0;   // diag(1, 0, 0)

        CHECK(matrix_close3(P0, target, 1e-13));

        std::printf("  T5  P_0 = (1/3)(I + Z + Z²) = diag(1, 0, 0)\n");
    }

    // ----- T6: idempotence P_0² = P_0, and tr(P_0) = 1 -----
    {
        const ComplexMatrix3 P0  = center_projector_p0();
        const ComplexMatrix3 P0sq = matrix_multiply3(P0, P0);

        CHECK(matrix_close3(P0sq, P0, 1e-13));

        const std::complex<double> tr = trace3(P0);
        CHECK(std::abs(tr - 1.0) < 1e-13);

        std::printf("  T6  P_0² = P_0  (idempotent);  tr(P_0) = %.15g + %.0ei\n",
                    tr.real(), tr.imag());
    }

    std::printf("=== %s (%d failure(s)) ===\n",
                g_fails == 0 ? "ALL PASSED" : "FAILED",
                g_fails);
    return g_fails == 0 ? 0 : 1;
}
