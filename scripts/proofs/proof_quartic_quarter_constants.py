"""
proof_quartic_quarter_constants.py

Verification of a quartic (Borwein-type) iteration claimed to compute
the quarter constants Gamma(3/4), Gamma(1/4), G*, and pi directly.

Proposed 2026-05-21.  The quartic iteration is from J. Guillera,
"Self-replication and Borwein-like algorithms," Ramanujan J. 47(2)
(2018) 447-455, arXiv:1702.05378 -- attribution confirmed by
literature search 2026-05-21.  Companion: Cooper-Guillera-Straub-
Zudilin, "Crouching AGM, Hidden Modularity," arXiv:1604.01106.
Corpus map: docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md.
This script verifies the mathematics numerically.

The iteration (parameter w):
  d_0 = 2^(-1/4),  c_0 = 2,  a_0 = 0
  d_{n+1} = (1 - (1-d_n^4)^(1/4)) / (1 + (1-d_n^4)^(1/4))
  c_{n+1} = 4 c_n (1 + d_{n+1})^(2w-2)
  a_{n+1} = a_n (1 + d_{n+1})^(2w+2)
            + (1/2) c_{n+1} (d_{n+1}/(1 + d_{n+1})) (1 - d_{n+1}^4)

Claimed limit:
  a_n(w) -> 1 / ( Gamma(3/4)^(2w-2) * pi^(3/2 - w/2) )

Claimed readouts:
  B = a_inf(3)    -> 1 / Gamma(3/4)^4
  A = a_inf(1/3)  -> (Gamma(3/4)/pi)^(4/3)
  G* = sqrt(2) * B^(1/4) * A^(-3/4)
  pi = A^(-3/4) * B^(-1/4)

This script runs the iteration at high precision and compares a_N(w)
(for w = 1, 3, 1/3) and the reconstructed G*, pi to independent mpmath
references.  It is a TEST: it exits non-zero if anything fails.

Note: once A and B are substituted by their limit values, the boxed
"G* = sqrt2 B^(1/4) A^(-3/4)" reduces algebraically to the known
relation  G* = sqrt(2) * pi / Gamma(3/4)^2  -- so the novelty here is
purely COMPUTATIONAL (the quartic iteration computes A, B without a
Gamma library or an imported pi), not a new identity.
"""
import sys
import mpmath as mpm

mpm.mp.dps = 500


def quartic_branch(w, N):
    """Run the proposed quartic iteration N steps; return a_N(w)."""
    w = mpm.mpf(w)
    d = mpm.power(2, mpm.mpf(-1) / 4)            # d_0 = 2^(-1/4)
    c = mpm.mpf(2)                               # c_0 = 2
    a = mpm.mpf(0)                               # a_0 = 0
    for _ in range(N):
        root = mpm.power(1 - d**4, mpm.mpf(1) / 4)
        d = (1 - root) / (1 + root)
        c = 4 * c * (1 + d) ** (2 * w - 2)
        a = a * (1 + d) ** (2 * w + 2) \
            + mpm.mpf(1) / 2 * c * (d / (1 + d)) * (1 - d**4)
    return a


def digits(a, b):
    """Decimal digits of agreement between a and b (b != 0)."""
    if a == b:
        return float(mpm.mp.dps)
    return float(-mpm.log10(abs(a - b) / abs(b)))


def main():
    G34 = mpm.gamma(mpm.mpf(3) / 4)
    G14 = mpm.gamma(mpm.mpf(1) / 4)
    PI = +mpm.pi
    GSTAR = G14 / G34

    def ref(w):
        w = mpm.mpf(w)
        return 1 / (G34 ** (2 * w - 2) * PI ** (mpm.mpf(3) / 2 - w / 2))

    print("=" * 72)
    print("  Quartic quarter-constant iteration  --  verification")
    print("=" * 72)
    print(f"  mp.dps = {mpm.mp.dps}")
    print(f"  reference G* = {mpm.nstr(GSTAR, 50)}")
    print("-" * 72)
    print("  branch-limit check -- digits of agreement")
    print("  a_N(w)  vs  1/(Gamma(3/4)^(2w-2) * pi^(3/2-w/2)):")
    print(f"  {'w':>7} {'N=1':>8} {'N=2':>8} {'N=3':>8} {'N=4':>8} {'N=5':>9}")
    ok = True
    for w in [mpm.mpf(1), mpm.mpf(3), mpm.mpf(1) / 3]:
        r = ref(w)
        row = [digits(quartic_branch(w, N), r) for N in range(1, 6)]
        label = mpm.nstr(w, 5)
        print(f"  {label:>7} " + " ".join(f"{x:8.1f}" for x in row))
        if row[-1] < 0.8 * mpm.mp.dps:
            ok = False
    print("-" * 72)
    print("  reconstruction:  G*_N = sqrt2 * B_N^(1/4) * A_N^(-3/4)")
    print("                   pi_N = A_N^(-3/4) * B_N^(-1/4)")
    print(f"  {'N':>3} {'G* digits':>14} {'pi digits':>14}")
    recon_ok = False
    for N in range(1, 6):
        A = quartic_branch(mpm.mpf(1) / 3, N)
        B = quartic_branch(mpm.mpf(3), N)
        Gn = mpm.sqrt(2) * mpm.power(B, mpm.mpf(1) / 4) \
            * mpm.power(A, mpm.mpf(-3) / 4)
        Pn = mpm.power(A, mpm.mpf(-3) / 4) * mpm.power(B, mpm.mpf(-1) / 4)
        dG, dP = digits(Gn, GSTAR), digits(Pn, PI)
        print(f"  {N:>3} {dG:14.1f} {dP:14.1f}")
        if N == 5 and min(dG, dP) > 0.8 * mpm.mp.dps:
            recon_ok = True
    print("=" * 72)
    if ok and recon_ok:
        print("  RESULT: the quartic branches converge to the claimed limits;")
        print("  the G* and pi readouts are verified. Convergence order is")
        print("  quartic (digits roughly quadruple per step).")
        return 0
    print("  RESULT: a check FAILED -- the iteration does not match the")
    print("  claimed limit structure as transcribed. See the table above.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
