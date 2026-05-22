"""
Proof — Harmonic invariant of the master-quadratic (1+i)-tower (Theorem 8).

Numerical verification at 50-digit precision of:

  Theorem 1 (harmonic invariant):
      For the family M_k(x) = x^2 - 2^k G*^(k-2) x + 2^k G*^(k-1), k >= 3,
      with G* = Gamma(1/4)/Gamma(3/4), the normalized roots y_+- := x_+-/G*
      satisfy 1/y_+ + 1/y_- = 1 at every level k.

  Theorem 2 (tower discriminant factorization):
      disc(M_k) = 2^(k+2) G*^(k-1) A_k, where A_k := 2^(k-2) G*^(k-3) - 1.

  Closed-form corollary (level k=4):
      alpha_tree = 1/(2 G*) - sqrt(4 G* - 1) / (4 G*^(3/2))
      reproduces 1/alpha = 137.0361715 (1.26 ppm vs CODATA 137.0359991).

Reference: docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md
LEDGER row: FTD-0111
"""

from mpmath import mp, mpf, gamma, sqrt, nstr

mp.dps = 50

G = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)  # G_STAR per scripts/constants.py

CODATA_ALPHA_INV = mpf("137.035999177")  # CODATA 2022


def header(title):
    print()
    print("=" * 72)
    print(title)
    print("=" * 72)


def assert_equal(label, lhs, rhs, tol_dps=40):
    diff = abs(lhs - rhs)
    tol = mpf(10) ** (-tol_dps)
    status = "PASS" if diff < tol else "FAIL"
    print(f"  [{status}] {label}: |lhs - rhs| = {nstr(diff, 5)} (tol 1e-{tol_dps})")
    if status == "FAIL":
        raise AssertionError(label)


def main():
    header(
        "Proof — Harmonic invariant of the master-quadratic (1+i)-tower (Theorem 8)"
    )
    print(f"  Precision: mp.dps = {mp.dps}")
    print(f"  G* = Gamma(1/4)/Gamma(3/4) = {nstr(G, 30)}")
    print(f"  CODATA 1/alpha = {CODATA_ALPHA_INV}")

    header("Theorem 1 — harmonic invariant 1/y_+ + 1/y_- = 1 at each level")
    for k in range(3, 8):
        bk = mpf(2) ** k * G ** (k - 2)
        ck = mpf(2) ** k * G ** (k - 1)
        # Vieta: c_k should equal G* * b_k (the tower normalization)
        assert_equal(f"  k={k}: tower normalization c_k = G* . b_k", ck, G * bk)
        disc = bk ** 2 - 4 * ck
        xp = (bk + sqrt(disc)) / 2
        xm = (bk - sqrt(disc)) / 2
        yp = xp / G
        ym = xm / G
        inv = 1 / yp + 1 / ym
        assert_equal(f"  k={k}: harmonic invariant 1/y_+ + 1/y_- = 1", inv, mpf(1))

    header("Theorem 2 — tower discriminant factorization disc(M_k) = 2^(k+2) G*^(k-1) A_k")
    print("  Anomaly-tower / level-k correction A_k := 2^(k-2) G*^(k-3) - 1:")
    for k in range(3, 8):
        Ak = mpf(2) ** (k - 2) * G ** (k - 3) - 1
        bk = mpf(2) ** k * G ** (k - 2)
        ck = mpf(2) ** k * G ** (k - 1)
        disc_direct = bk ** 2 - 4 * ck
        disc_factored = mpf(2) ** (k + 2) * G ** (k - 1) * Ak
        print(f"    k={k}: A_{k} = {nstr(Ak, 8):>14}")
        assert_equal(f"  k={k}: disc factorization", disc_direct, disc_factored)

    header("Closed-form corollary (level 4) — alpha_tree from G* alone")
    alpha_geom = 1 / (2 * G)
    alpha_corr = sqrt(4 * G - 1) / (4 * G ** (mpf(3) / 2))
    alpha_tree = alpha_geom - alpha_corr
    inv_alpha_tree = 1 / alpha_tree

    print(f"  geometric term  1/(2 G*)              = {nstr(alpha_geom, 25)}")
    print(f"  correction term sqrt(4G*-1)/(4G*^3/2) = {nstr(alpha_corr, 25)}")
    print(f"  alpha_tree                             = {nstr(alpha_tree, 25)}")
    print(f"  1/alpha_tree                           = {nstr(inv_alpha_tree, 15)}")

    # Sanity: should solve dual master quadratic 16 G*^3 a^2 - 16 G*^2 a + 1 = 0
    residual = 16 * G ** 3 * alpha_tree ** 2 - 16 * G ** 2 * alpha_tree + 1
    assert_equal(
        "  alpha_tree solves dual master quadratic 16G*^3 a^2 - 16G*^2 a + 1 = 0",
        residual,
        mpf(0),
        tol_dps=40,
    )

    # Sanity: should equal level-4 small-root reciprocal
    bk = mpf(2) ** 4 * G ** 2
    ck = mpf(2) ** 4 * G ** 3
    disc = bk ** 2 - 4 * ck
    xp_level4 = (bk + sqrt(disc)) / 2
    assert_equal("  alpha_tree = 1/x_+(k=4)", alpha_tree, 1 / xp_level4)

    # Empirical match
    ppm_residual = (inv_alpha_tree - CODATA_ALPHA_INV) / CODATA_ALPHA_INV * mpf(10) ** 6
    print()
    print(f"  CODATA 1/alpha                         = {CODATA_ALPHA_INV}")
    print(f"  tree residual (1/alpha_tree - CODATA)  = "
          f"{nstr(inv_alpha_tree - CODATA_ALPHA_INV, 6)}")
    print(f"  tree residual in ppm                   = {nstr(ppm_residual, 6)}")
    print()
    print("  [DERIVED] Tree-level closed form reproduces CODATA 1/alpha to 1.26 ppm.")
    print("  [CONJECTURE] Physical identification alpha = 1/x_+ is unchanged in tag.")

    header("All assertions PASS")


if __name__ == "__main__":
    main()
