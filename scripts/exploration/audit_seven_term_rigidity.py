"""
audit_seven_term_rigidity.py

Rigidity audit for the 7-term precision series conjecture:

    1/alpha = x+ + sum_{n=1..7} s_n c_n |eps|^n

where eps = e^pi - pi - 20, x+ is the master-quadratic root, and the (s_n, c_n)
are the claimed sign/coefficient pairs from
docs/theory/09_mathematical/CONJ_SEVEN_TERM_PRECISION_SERIES.md.

Conjecture upgrades to [THEOREM] iff the denominators {47, 64, 141, 11, 21, 8}
are *uniquely* the lowest-height rational fit in the base-integer set
{N_c=3, N_base=4, b_3=7, N_eff=13, D=47, BCC=8}. Refuted as a fit if alternative
low-height rationals from the same base set match CODATA at comparable precision.

This script performs three checks:

  (A) Reproduce the claimed 24-digit CODATA match.
  (B) For each coefficient c_n, solve for the *required* value given all others
      fixed, then search low-height rationals in the base-integer set for
      competitors.
  (C) Report whether the claimed integer presentation of each c_n is the
      unique / lowest-height / only-one-within-tolerance candidate.

Run:
    python audit_seven_term_rigidity.py
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product
from mpmath import mp, mpf, mpc, exp, pi, sqrt
from typing import Iterable


# -------- Precision --------
mp.dps = 60  # decimal digits; comfortably above the claimed 24-digit match


# -------- Base integers (lattice-structural) --------
N_c = 3
N_base = 4
b3 = 7
N_eff = 13
D = N_c * N_base**2 - 1     # 47
BCC = 8                     # 2^3

BASE_INTS = (N_c, N_base, b3, N_eff, D, BCC)
BASE_NAMES = ("N_c", "N_base", "b3", "N_eff", "D", "BCC")

# -------- Claimed series --------
# (sign, Fraction(p, q), integer-form description)
CLAIMED = [
    (-1, Fraction(9, 47),    "N_c^2 / D"),
    (+1, Fraction(5, 64),    "(N_eff - 2*N_base) / N_base^3"),
    (-1, Fraction(4, 141),   "N_base / (N_c * D)"),
    (-1, Fraction(141, 11),  "(N_c * D) / (b3 + N_base)"),
    (-1, Fraction(1472, 21), "(2*N_eff - N_c) * N_base^3 / (N_c * b3)"),
    (-1, Fraction(416, 21),  "2 * N_eff * N_base^2 / (N_c * b3)"),
    (+1, Fraction(299, 8),   "N_eff * (2*N_eff - N_c) / BCC"),
]


# -------- Master quadratic x+ and epsilon --------
def compute_x_plus_and_eps():
    """Return (x+, eps) as mpf values."""
    # G* = 2*varpi / sqrt(pi). varpi = Gamma(1/4)^2 / (2*sqrt(2*pi)).
    # Equivalent closed form: G* = sqrt(2) * Gamma(1/4)^2 / (2*pi).
    from mpmath import gamma
    Gstar = sqrt(mpf(2)) * gamma(mpf(1)/4)**2 / (2 * pi)
    # x+ = 8*G*^2 + 4*G*^(3/2) * sqrt(4*G* - 1)
    x_plus = 8*Gstar**2 + 4*Gstar**(mpf(3)/2) * sqrt(4*Gstar - 1)
    eps = exp(pi) - pi - 20
    return x_plus, eps, Gstar


# -------- Base-integer rational enumeration --------
# Build the set of "natural" rational numerators/denominators expressible
# as small polynomial combinations of BASE_INTS.
#
# We enumerate products/sums up to the height observed in the claimed
# denominators (max denominator = 141 = 3 * 47). To stay generous, we
# allow all expressions of the form
#   prod(base^a_i) * (+/-) prod(base^b_i)  with sum of exponents <= 3,
#   plus pairwise sums and differences p +/- q where p, q are single
#   products, plus factor-of-2 multiples.
# This yields a few thousand candidate integers which we then form into
# rationals.

def enumerate_candidate_integers(max_height: int = 2000) -> set[int]:
    """Enumerate integers expressible as low-complexity combinations of BASE_INTS."""
    seeds: set[int] = set()
    # Single base integers and their small powers/products
    for a in BASE_INTS:
        seeds.add(a)
        seeds.add(a*a)
    for a, b in product(BASE_INTS, repeat=2):
        seeds.add(a * b)
        seeds.add(abs(a - b))
        seeds.add(a + b)
        if a != b:
            seeds.add(a * a * b)
            seeds.add(a * b * b)
    for a, b, c in product(BASE_INTS, repeat=3):
        seeds.add(a * b * c)
    # Pairwise sums/differences of products
    products_all = set()
    for a, b in product(BASE_INTS, repeat=2):
        products_all.add(a * b)
    for a, b in product(BASE_INTS, repeat=2):
        products_all.add(a + b)
    for p, q in product(products_all, repeat=2):
        seeds.add(abs(p - q))
        seeds.add(p + q)
    # Factor-of-2 and factor-of-small multiples
    extended = set()
    for s in seeds:
        if 0 < s <= max_height:
            extended.add(s)
        for k in (2, 3, 4, 5, 6, 7, 8, 9):
            if 0 < s * k <= max_height:
                extended.add(s * k)
    # Always include small sanity values
    extended.update(range(1, 30))
    return {s for s in extended if 0 < s <= max_height}


def enumerate_candidate_rationals(num_bound: int, den_bound: int) -> set[Fraction]:
    """Build Fractions p/q with p in candidate-int set up to num_bound, same for q."""
    nums = {s for s in enumerate_candidate_integers(num_bound) if s <= num_bound}
    dens = {s for s in enumerate_candidate_integers(den_bound) if s <= den_bound}
    rats = set()
    for p in nums:
        for q in dens:
            if q == 0:
                continue
            rats.add(Fraction(p, q))
    return rats


# -------- Check (A): precision reproduction --------
def check_precision_reproduction():
    x_plus, eps, Gstar = compute_x_plus_and_eps()
    abs_eps = abs(eps)
    total = mpf(x_plus)
    for n, (s, c, _) in enumerate(CLAIMED, start=1):
        total += s * mpf(c.numerator) / mpf(c.denominator) * abs_eps**n

    # Compare with CODATA 2022 (value from BIPM / CODATA website)
    codata_alpha_inv = mpf("137.035999177")  # +/- 21e-9
    gap = abs(total - codata_alpha_inv)
    print("=" * 70)
    print("(A) Precision reproduction check")
    print("=" * 70)
    print(f"G*                 = {Gstar}")
    print(f"x+                 = {x_plus}")
    print(f"eps                = {eps}")
    print(f"|eps|              = {abs_eps}")
    print(f"7-term prediction  = {total}")
    print(f"CODATA 2022 alpha^-1 = {codata_alpha_inv}")
    print(f"|prediction - CODATA| = {gap}")
    print(f"CODATA uncertainty = 2.1e-8 (the 21e-9 on the 9th decimal)")
    ppb = gap * mpf(10)**9
    if ppb > 0:
        print(f"Gap in ppb of alpha^-1: {ppb / codata_alpha_inv * mpf(10)**9}")
    return total, codata_alpha_inv, gap, x_plus, eps


# -------- Check (B): required-c_n competitor search --------
def required_c_n(n_target: int, x_plus, eps) -> mpf:
    """Given all other c_k fixed at claimed values, return the signed c_{n_target}
    needed to match CODATA exactly.

    1/alpha - x+ - sum_{k != n_target} s_k c_k |eps|^k = s_{n_target} c_{n_target} |eps|^{n_target}
    => c_{n_target} = (LHS) / (s_{n_target} * |eps|^{n_target})
    """
    codata = mpf("137.035999177000")
    abs_eps = abs(eps)
    lhs = codata - x_plus
    for k, (s, c, _) in enumerate(CLAIMED, start=1):
        if k == n_target:
            continue
        lhs -= s * mpf(c.numerator) / mpf(c.denominator) * abs_eps**k
    s_target = CLAIMED[n_target - 1][0]
    c_needed = lhs / (s_target * abs_eps**n_target)
    return c_needed


def search_competitors(c_needed: mpf, claimed: Fraction,
                       tol_digits: int = 6,
                       num_bound: int = 2000, den_bound: int = 2000,
                       max_report: int = 20) -> list[tuple[Fraction, mpf]]:
    """Find rationals in the base-integer set within 10^-tol_digits of c_needed.

    tol_digits=6 means we count as a "competitor" anything matching the required
    value to 6 decimals (after scaling by |eps|^n, this maps roughly to the
    CODATA-precision regime for that term).
    """
    candidates = enumerate_candidate_rationals(num_bound, den_bound)
    tol = mpf(10) ** (-tol_digits)
    hits: list[tuple[Fraction, mpf]] = []
    for r in candidates:
        diff = abs(mpf(r.numerator) / mpf(r.denominator) - c_needed)
        if diff < tol:
            hits.append((r, diff))
    # Sort by height (max of |p|, |q|), then by closeness
    hits.sort(key=lambda pair: (max(abs(pair[0].numerator), pair[0].denominator),
                                 float(pair[1])))
    return hits[:max_report]


def check_rigidity_all(x_plus, eps):
    print()
    print("=" * 70)
    print("(B) Rigidity search: competitors for each c_n")
    print("=" * 70)
    print(f"Per-n tolerance scales with |eps|^n so that each c_n is tested at the")
    print(f"precision that actually matters for the overall 24-digit CODATA fit.")
    print(f"Specifically: tol_n = 10^(-24) / |eps|^n (the natural cascade tolerance).")
    print(f"Also reported: tol at CODATA experimental precision (2.1e-8).")
    print(f"Rational search: numerator/denominator drawn from low-complexity")
    print(f"combinations of {BASE_NAMES} up to height 2000.")
    print()

    abs_eps = abs(eps)
    summary = []
    for n in range(1, 8):
        s_n, claimed_c, desc = CLAIMED[n - 1]
        c_needed = required_c_n(n, x_plus, eps)
        # Natural cascade tolerance: c_n error of this much contributes ~10^-24 to total
        tol_natural = float(mpf(10)**(-24) / abs_eps**n)
        # CODATA experimental tolerance
        tol_codata = float(mpf("2.1e-8") / abs_eps**n)
        tol_digits_used = max(3, int(-mp.log10(mpf(tol_natural)) - 0.5))
        competitors = search_competitors(
            c_needed, claimed_c,
            tol_digits=tol_digits_used, num_bound=2000, den_bound=2000, max_report=15
        )
        print(f"--- c_{n} (claimed: {claimed_c} = {desc}) ---")
        print(f"  required value     : {c_needed}")
        print(f"  claimed value      : {mpf(claimed_c.numerator)/mpf(claimed_c.denominator)}")
        print(f"  cascade tol (1e-24): {tol_natural:.2e}")
        print(f"  CODATA tol (2.1e-8): {tol_codata:.2e}")
        print(f"  # competitors within cascade tol: {len(competitors)}")
        for frac, diff in competitors:
            mark = "  <-- CLAIMED" if frac == claimed_c else ""
            height = max(abs(frac.numerator), frac.denominator)
            print(f"    {frac!s:>12}  (height={height:5d}, |diff|={float(diff):.3e}){mark}")

        unique_height = all(
            max(abs(f.numerator), f.denominator) >= max(abs(claimed_c.numerator), claimed_c.denominator)
            for f, _ in competitors
        )
        n_strictly_lower_height = sum(
            1 for f, _ in competitors
            if (f != claimed_c) and (max(abs(f.numerator), f.denominator)
                                     < max(abs(claimed_c.numerator), claimed_c.denominator))
        )
        # Also count competitors at CODATA experimental precision — i.e., how many
        # rationals in the base-integer set are observationally indistinguishable.
        codata_digits = max(1, int(-mp.log10(mpf(tol_codata)) - 0.5))
        if codata_digits > 0:
            codata_competitors = search_competitors(
                c_needed, claimed_c,
                tol_digits=codata_digits, num_bound=2000, den_bound=2000, max_report=200
            )
        else:
            codata_competitors = [(claimed_c, mpf(0))]  # tolerance already looser than any height bound
        print(f"  # competitors within CODATA tol: {len(codata_competitors)}")
        if len(codata_competitors) > 1 and len(codata_competitors) <= 10:
            for frac, diff in codata_competitors[:10]:
                mark = "  <-- CLAIMED" if frac == claimed_c else ""
                height = max(abs(frac.numerator), frac.denominator)
                print(f"    CODATA-peer: {frac!s:>12}  (height={height:5d}){mark}")
        elif len(codata_competitors) > 10:
            print(f"    (showing first 5 of {len(codata_competitors)})")
            for frac, diff in codata_competitors[:5]:
                mark = "  <-- CLAIMED" if frac == claimed_c else ""
                height = max(abs(frac.numerator), frac.denominator)
                print(f"    CODATA-peer: {frac!s:>12}  (height={height:5d}){mark}")
        print()

        summary.append({
            "n": n,
            "claimed": claimed_c,
            "height": max(abs(claimed_c.numerator), claimed_c.denominator),
            "cascade_competitors": len(competitors),
            "codata_competitors": len(codata_competitors),
            "lower_height_competitors": n_strictly_lower_height,
            "claimed_is_min_height": unique_height,
        })

    return summary


def check_verdict(summary):
    print("=" * 70)
    print("(C) Verdict")
    print("=" * 70)
    any_lower_height = False
    any_cascade_multiple = False
    any_codata_multiple = False
    for s in summary:
        tag = []
        if s["lower_height_competitors"] > 0:
            tag.append(f"{s['lower_height_competitors']} LOWER-HEIGHT alt(s)")
            any_lower_height = True
        if s["cascade_competitors"] > 1:
            tag.append(f"{s['cascade_competitors']-1} cascade-tol peer(s)")
            any_cascade_multiple = True
        if s["codata_competitors"] > 1:
            tag.append(f"{s['codata_competitors']-1} CODATA-tol peer(s)")
            any_codata_multiple = True
        tag_str = "; ".join(tag) if tag else "unique at cascade AND CODATA tol"
        print(f"  c_{s['n']} claimed={s['claimed']!s:>12} height={s['height']:5d}  {tag_str}")
    print()
    print()
    if any_lower_height:
        print("VERDICT: REFUTED as [THEOREM].")
        print("At least one coefficient has a LOWER-height alternative in the base-integer set.")
        print("The claimed presentation is not minimum-complexity.")
    elif any_codata_multiple:
        print("VERDICT: AMBIGUOUS AT EXPERIMENTAL PRECISION.")
        print("Within CODATA 2022 experimental tolerance (2.1e-8 on alpha^-1), multiple")
        print("rationals in the base-integer set are indistinguishable from the claimed c_n.")
        print("The 24-digit 'match' is a structural property of the series once coefficients")
        print("are chosen; it cannot be *verified* experimentally beyond digit ~11.")
        print("Recommendation: keep [CONJECTURE]; note that uniqueness at cascade precision")
        print("is an algebraic curiosity unless experimental precision catches up.")
    elif any_cascade_multiple:
        print("VERDICT: AMBIGUOUS AT CASCADE PRECISION.")
        print("No lower-height alternatives, but multiple rationals match within the")
        print("self-consistency cascade tolerance. Keep [CONJECTURE].")
    else:
        print("VERDICT: UPGRADE CANDIDATE.")
        print("The claimed integer presentation is the unique lowest-height rational in the")
        print("base-integer set matching the required value at cascade precision. Note this")
        print("does NOT establish that the base-integer set itself is forced.")
    print()
    return any_lower_height, any_cascade_multiple, any_codata_multiple


def main():
    print("7-term precision series rigidity audit")
    print(f"(mpmath precision: {mp.dps} digits)")
    print()
    total, codata, gap, x_plus, eps = check_precision_reproduction()
    summary = check_rigidity_all(x_plus, eps)
    refuted, cascade_amb, codata_amb = check_verdict(summary)
    return 0 if not refuted else 1


if __name__ == "__main__":
    raise SystemExit(main())
