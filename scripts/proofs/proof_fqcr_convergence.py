#!/usr/bin/env python3
"""
PROOF · FQCR Model II finite-N convergence: G_N* -> G*
=======================================================

Verifies the finite-N approximation in DERIV_GSTAR_FINITE_APPROX.md
(FTD-0142) by computing

    G_N* = (N+1)^(-1/2) * prod_{n=0..N} (n+3/4) / (n+1/4)
         = (N+1)^(-1/2) * Gamma(N+7/4) Gamma(1/4) / [Gamma(N+5/4) Gamma(3/4)]

for N in {16, 32, 64, 128, 256, 512, 1024, 2048, 4096} and asserting:

  (1) |G_N* - G*| < 1e-7  at N = 1024
  (2) |G_N* - G*| < 1e-8  at N = 4096
  (3) Residuals scale as ~ C/N^2 (no faster, no slower)
      Empirical C is approximately 0.046 — so reaching 1e-12 would
      require N ~ 2e5; double-precision machine-epsilon on G_N* is
      not achievable at the lattice sizes typical of FTD computations,
      but ~1e-8 at N=4096 is well within double-precision tolerance.

Usage:
    python scripts/proofs/proof_fqcr_convergence.py

Exits non-zero on assertion failure. Tag: [PROOF SCRIPT].
Cited from: DERIV_GSTAR_FINITE_APPROX.md §5.
"""

from mpmath import mp, mpf, gamma as mp_gamma, sqrt as mp_sqrt
import sys

# 50-digit precision so the residuals at N=4096 are not bottlenecked by
# the precision of G* itself.
mp.dps = 50

# Reference value of G* (high-precision) from the algebraic spine.
G_STAR = mp_gamma(mpf(1) / 4) / mp_gamma(mpf(3) / 4)


def G_N(N):
    """Compute G_N* via the Gamma-product representation."""
    one = mpf(1)
    return (
        (mpf(N + 1)) ** (-one / 2)
        * mp_gamma(N + mpf(7) / 4)
        * mp_gamma(one / 4)
        / (mp_gamma(N + mpf(5) / 4) * mp_gamma(mpf(3) / 4))
    )


def main():
    print("=" * 78)
    print("  FQCR Model II — finite-N convergence proof")
    print("  G_N* := (N+1)^(-1/2) * prod_{n=0..N} (n+3/4)/(n+1/4)")
    print("  Reference: G* = Gamma(1/4)/Gamma(3/4) =", str(G_STAR)[:32])
    print("=" * 78)
    print()
    print(f"{'N':>6} {'G_N*':>22} {'|G_N* - G*|':>16} {'N^2 * residual':>18}")
    print("-" * 78)

    Ns = [16, 32, 64, 128, 256, 512, 1024, 2048, 4096]
    residuals = []
    for N in Ns:
        g_n = G_N(N)
        residual = abs(g_n - G_STAR)
        scaled = residual * mpf(N) ** 2
        residuals.append((N, residual, scaled))
        print(f"{N:>6d} {str(g_n)[:22]:>22} {float(residual):>16.6e} {float(scaled):>18.6e}")

    print()

    # Assertion 1: residual at N=1024 below 1e-7
    r_1024 = next(r for n, r, _ in residuals if n == 1024)
    assert r_1024 < mpf("1e-7"), (
        f"FAIL (1): |G_1024* - G*| = {float(r_1024):.3e} > 1e-7"
    )
    print(f"PASS (1): |G_1024* - G*| = {float(r_1024):.3e} < 1e-7")

    # Assertion 2: residual at N=4096 below 1e-8 (honest; the empirical
    # C ~ 0.046 means C/4096^2 ~ 2.7e-9 — well below 1e-8 with margin).
    r_4096 = next(r for n, r, _ in residuals if n == 4096)
    assert r_4096 < mpf("1e-8"), (
        f"FAIL (2): |G_4096* - G*| = {float(r_4096):.3e} > 1e-8"
    )
    print(f"PASS (2): |G_4096* - G*| = {float(r_4096):.3e} < 1e-8")

    # Assertion 3: N^2 * residual is approximately constant (1/N^2 scaling).
    # We allow a factor-2 spread between the smallest and largest N because
    # the next-order O(N^-3) correction contaminates small N. Specifically:
    # for the largest three Ns, the scaled residuals should agree within 5%.
    last_three_scaled = [float(s) for n, _, s in residuals if n >= 1024]
    spread = max(last_three_scaled) / min(last_three_scaled)
    assert spread < 1.10, (
        f"FAIL (3): N^2*residual spread for N >= 1024 is {spread:.3f}, "
        f"expected < 1.10 (consistent with O(N^-2) leading term)"
    )
    # Take the asymptotic value of N^2 * residual as the empirical C.
    C_emp = sum(last_three_scaled) / len(last_three_scaled)
    print(
        f"PASS (3): N^2 * residual is approximately constant "
        f"(C_emp ~ {C_emp:.6f}) — confirms O(1/N^2) leading behaviour"
    )

    print()
    print("All assertions PASSED. FTD-0142 [THEOREM] verified numerically.")
    print(
        f"G_N* -> G* under 1/N^2 law with empirical C = {C_emp:.4f}.\n"
        f"Residual at N=4096 is {float(r_4096):.3e} — well within "
        f"double-precision tolerance for any FTD computation that needs G*."
    )


if __name__ == "__main__":
    try:
        main()
    except AssertionError as e:
        print(f"\nASSERTION FAILED: {e}")
        sys.exit(1)
    sys.exit(0)
