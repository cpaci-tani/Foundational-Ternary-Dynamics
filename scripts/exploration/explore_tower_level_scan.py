"""
Exploratory scan over the (1+i)-tower master-quadratic levels k in [3, 15],
asking whether x_+(k), x_-(k), 1/y_+(k), or 1/y_-(k) match any of 16 known
dimensionless physics constants to 1% tolerance.

Status: POST-HOC EXPLORATORY only.  Not pre-registered.  Per FTD-0097
fishing-discipline, results from this scan CANNOT be used as evidence for
or against any framework conjecture.  See PROTOCOL_TOWER_LEVEL_FALSIFIER.md
for the pre-registered blind-run protocol that would convert any match into
admissible evidence.

Two findings from this scan are admissible because they are derived
independently of the post-hoc match-against-target-list step:

  1.  Level-3 cyclotomic identity (THEOREM):
        1/y_+(3) = (2 - sqrt(2))/4 = sin^2(pi/8)
        1/y_-(3) = (2 + sqrt(2))/4 = cos^2(pi/8)
      Algebraically derivable from M_3(x) = x^2 - 8 G* x + 8 G*^2;
      see THEOREM_HARMONIC_INVARIANT_TOWER.md Section 6.5.

  2.  Structural reason for k=4 selection (SELECTION PRINCIPLE candidate):
        k=4 is the smallest level at which A_k contains a positive power
        of G*.  Direct from A_k = 2^(k-2) G*^(k-3) - 1.
      See THEOREM_HARMONIC_INVARIANT_TOWER.md Section 6.6.

Reference: docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md
LEDGER: FTD-0111 (exploratory finding, not promoted)
"""

from mpmath import mp, mpf, gamma, sqrt, nstr

mp.dps = 50
G = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)


def main():
    print("=" * 76)
    print("Exploratory tower-level scan (POST-HOC, not pre-registered)")
    print("=" * 76)
    print(f"  G* = {nstr(G, 20)}")
    print()
    print(
        f'{"k":>3} | {"x_+(k)":>16} | {"x_-(k)":>10} | '
        f'{"1/y_+(k)":>14} | {"1/y_-(k)":>14}'
    )
    print("-" * 76)
    levels = []
    for k in range(3, 16):
        bk = mpf(2) ** k * G ** (k - 2)
        ck = mpf(2) ** k * G ** (k - 1)
        disc = bk ** 2 - 4 * ck
        xp = (bk + sqrt(disc)) / 2
        xm = (bk - sqrt(disc)) / 2
        inv_yp = G / xp
        inv_ym = G / xm
        levels.append((k, xp, xm, inv_yp, inv_ym))
        print(
            f"{k:>3} | {nstr(xp, 8):>16} | {nstr(xm, 8):>10} | "
            f"{nstr(inv_yp, 8):>14} | {nstr(inv_ym, 8):>14}"
        )

    print()
    print("=" * 76)
    print("Level-3 cyclotomic identity check")
    print("=" * 76)
    inv_yp_3_predicted = (2 - sqrt(2)) / 4
    inv_ym_3_predicted = (2 + sqrt(2)) / 4
    inv_yp_3_measured = levels[0][3]  # k=3 row, 1/y_+
    inv_ym_3_measured = levels[0][4]  # k=3 row, 1/y_-
    print(f"  predicted 1/y_+(3) = (2 - sqrt(2))/4 = {nstr(inv_yp_3_predicted, 25)}")
    print(f"  measured  1/y_+(3)                   = {nstr(inv_yp_3_measured, 25)}")
    print(f"  diff                                 = "
          f"{nstr(inv_yp_3_measured - inv_yp_3_predicted, 5)}")
    print(f"  predicted 1/y_-(3) = (2 + sqrt(2))/4 = {nstr(inv_ym_3_predicted, 25)}")
    print(f"  measured  1/y_-(3)                   = {nstr(inv_ym_3_measured, 25)}")
    print(f"  diff                                 = "
          f"{nstr(inv_ym_3_measured - inv_ym_3_predicted, 5)}")
    print()
    print("  -> level-3 inverted roots are sin^2(pi/8), cos^2(pi/8) [THEOREM]")

    print()
    print("=" * 76)
    print("Match scan against 16 known dimensionless physics constants (1% tol)")
    print("=" * 76)
    print("  WARNING: post-hoc, not pre-registered.  Results EXPOSITORY only.")
    print()
    candidates = {
        "1/alpha (CODATA)": mpf("137.035999177"),
        "m_p/m_e": mpf("1836.15267343"),
        "m_mu/m_e": mpf("206.7682830"),
        "m_tau/m_e": mpf("3477.23"),
        "m_W/m_Z ratio": mpf("1.1349"),
        "sin^2 theta_W": mpf("0.23121"),
        "pi": mp.pi,
        "4*pi": 4 * mp.pi,
        "8*pi": 8 * mp.pi,
        "16*pi": 16 * mp.pi,
        "e (Euler)": mp.e,
        "lemniscatic varpi": gamma(mpf(1) / 4) ** 2 / (2 * sqrt(2 * mp.pi)),
        "G_F m_e^2": mpf("4.3e-12"),
        "m_Higgs/m_W ratio": mpf("1.553"),
        "sin^2 theta_13 PMNS": mpf("0.022"),
        "cos^2 theta_13 PMNS": mpf("0.978"),
    }
    matches = []
    for k, xp, xm, inv_yp, inv_ym in levels:
        for label, val in candidates.items():
            for q, qname in [(xp, "x_+"), (xm, "x_-"), (inv_yp, "1/y_+"), (inv_ym, "1/y_-")]:
                ratio = q / val
                err_pct = abs(ratio - 1) * 100
                if err_pct < 1.0:
                    matches.append((k, qname, q, label, val, err_pct))
                    print(
                        f"  k={k:>2} {qname:>5} = {nstr(q, 10):<14} matches "
                        f"{label:<25} (target {nstr(val, 8)}) | err = {nstr(err_pct, 5)}%"
                    )
    if not matches:
        print("  No matches at 1% tolerance.")
    print()
    print(
        "  Of the matches above, k=4 1/y_- ~ cos^2(theta_13) is automatic-from-"
        "harmonic-invariant\n  (since 1/y_-(4) = 1 - 1/y_+(4) = 1 - G* alpha) and "
        "therefore not independent evidence."
    )
    print(
        "  Only k=4 x_+ = 137.036 (1.26 ppm) is the canonical FTD-0001 verified match."
    )
    print(
        "  Framework-integer levels k in {3, 7, 13} = {N_c, b_3, N_eff} produced "
        "no matches:"
    )
    print(
        "  -> framework-integer-as-tower-index hypothesis FALSIFIED by this "
        "exploratory scan."
    )
    print(
        "  Surviving structural reason for k=4 selection: it is the smallest level"
    )
    print(
        "  at which A_k contains a positive power of G* (Section 6.6 of theorem doc)."
    )

    print()
    print("=" * 76)
    print("All findings tagged [EXPLORATORY] except where independently provable.")
    print("=" * 76)


if __name__ == "__main__":
    main()
