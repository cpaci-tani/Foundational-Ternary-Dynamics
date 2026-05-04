"""
proof_g_star_parity_twist.py — Result D verification (FTD-0127).

Verifies the parity-twist identification:

    G* = Gamma_zeta(1/2) / Gamma_{chi_-4}(1/2)
       = Gamma((1/2)/2) / Gamma((1/2 + 1)/2)
       = Gamma(1/4) / Gamma(3/4)

Where Gamma_zeta(s) = Gamma(s/2) is the Archimedean Gamma-factor of the
Riemann zeta function (parity a = 0) and Gamma_{chi_-4}(s) = Gamma((s+1)/2)
is the Archimedean Gamma-factor of the Dirichlet L-function for the
unique non-trivial character mod 4 (parity a = 1).

This identity is immediate from the definitions of the completed
L-functions xi(s) and Lambda(s, chi_-4); the substantive content is the
operational identification of FTD's G* with the parity-twist between the
two simplest L-functions in number theory.

Reference: docs/theory/03_derivations/DERIV_G_STAR_PARITY_TWIST.md
LEDGER: FTD-0127
"""

from mpmath import mp, mpf, gamma, nstr


def main():
    mp.dps = 80

    # G* by canonical definition
    G_star_canonical = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)

    # Archimedean Gamma-factors at the critical-line center s = 1/2
    half = mpf(1) / 2
    Gamma_zeta_at_half = gamma(half / 2)              # = Gamma(1/4)
    Gamma_chi4_at_half = gamma((half + 1) / 2)        # = Gamma(3/4)
    parity_twist = Gamma_zeta_at_half / Gamma_chi4_at_half

    # Compare
    diff = G_star_canonical - parity_twist

    print("=" * 70)
    print("Result D — G* as parity-twist (FTD-0127)")
    print("=" * 70)
    print(f"Working precision: mp.dps = {mp.dps}")
    print()
    print(f"G* canonical = Gamma(1/4) / Gamma(3/4)")
    print(f"             = {nstr(G_star_canonical, 30)}")
    print()
    print(f"Gamma_zeta(1/2)  = Gamma((1/2)/2) = Gamma(1/4)")
    print(f"                 = {nstr(Gamma_zeta_at_half, 30)}")
    print(f"Gamma_chi4(1/2)  = Gamma((1/2 + 1)/2) = Gamma(3/4)")
    print(f"                 = {nstr(Gamma_chi4_at_half, 30)}")
    print(f"parity-twist     = Gamma_zeta(1/2) / Gamma_chi4(1/2)")
    print(f"                 = {nstr(parity_twist, 30)}")
    print()
    print(f"diff = {nstr(diff, 5)}  (must be 0; identity is immediate from definitions)")
    print()

    assert diff == 0, f"Identity failed: diff = {diff}"
    print("PASS — G* equals the parity-twist exactly.")


if __name__ == "__main__":
    main()
