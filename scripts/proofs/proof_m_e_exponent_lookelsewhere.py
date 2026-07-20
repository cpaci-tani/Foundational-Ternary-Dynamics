"""proof_m_e_exponent_lookelsewhere.py -- look-elsewhere audit of the
ordering-selection step in the m_e exponent n=11 promotion attempt
(FTD-0015, MC-T3.2).

Context (see LEDGER.md FTD-0015, corrected 2026-07-01, and
docs/theory/05_particles/DERIV_COLOR_BINDING_STRUCTURE_AND_ME_STATUS.md):

    m_e = M_P * sqrt(2*pi) * (16/3) * alpha^n,  n = 11 (0.19% match)

proof_m_e_exponent_n11.py (2026-05-02) established:
    [THEOREM] multiset {3, 3, 4, 6} is the UNIQUE solution to
              "4 parts from {N_c=3, N_base=4, N_f=6}, all present, sum=16"
    [SELECTION x2] "gravity last" (S1) + "spinor before color" (S2)
              narrow the 12 orderings of that multiset to exactly one:
              (4, 3, 3, 6), whose cumulative position after 2 steps is 11.

That script never asks what the OTHER 11 orderings give, or whether S1+S2
are doing independent work versus being reverse-engineered to hit the
already-known target. This script answers exactly that, the same way
FTD-0097 look-elsewhere-scanned the master quadratic before trusting its
uniqueness. No new postulate is introduced; this only audits the existing
promotion attempt.

Epistemic status: [PRE-REGISTERED-STYLE AUDIT]. Purely deterministic
combinatorics (12 permutations, exhaustively enumerable) plus one
numerical cross-check against CODATA -- there is no stochastic outcome to
p-hack, so the LOCK-STD v1 pre-commit ceremony is not the applicable
safeguard here; the safeguard is showing the FULL distribution rather
than the favorable slice, which is what Test 2 below does.

Usage:
    python scripts/proofs/proof_m_e_exponent_lookelsewhere.py
"""

from __future__ import annotations

import math
import sys
from itertools import permutations
from typing import Dict, List, Tuple

# Canonical values (scripts/constants.py)
N_BASE = 4
N_C = 3
N_F = 6
LADDER_START = 4

ALPHA_INV = 137.035999177        # CODATA 2022, scripts/constants.py Experimental.alpha_inv
ALPHA = 1.0 / ALPHA_INV
M_PLANCK_MEV = 1.220890e19 * 1000.0   # scripts/constants.py M_PLANCK is in GeV
PREFACTOR = math.sqrt(2 * math.pi) * (N_BASE**2 / N_C)   # sqrt(2pi) * 16/3
M_ELECTRON_MEV = 0.51099895      # CODATA 2022, scripts/constants.py Experimental.m_electron


def cumulative_positions(steps: Tuple[int, ...]) -> Tuple[int, ...]:
    pos = [LADDER_START]
    for step in steps:
        pos.append(pos[-1] + step)
    return tuple(pos)


def test_full_distribution() -> Dict[int, List[Tuple[int, ...]]]:
    """Every one of the 12 orderings, not just the 4 that hit n=11."""
    print("Test 1: full distribution of the electron position (pos[2]) over")
    print("        all 12 orderings of the forced multiset {3, 3, 4, 6}")
    print()
    multiset = (N_C, N_C, N_BASE, N_F)
    orderings = sorted(set(permutations(multiset)))
    buckets: Dict[int, List[Tuple[int, ...]]] = {}
    for o in orderings:
        pos = cumulative_positions(o)
        buckets.setdefault(pos[2], []).append(o)
    for n in sorted(buckets):
        orders = buckets[n]
        print(f"  n = {n:2d}  <- {len(orders)}/12 orderings: {orders}")
    print()
    counts = {n: len(v) for n, v in buckets.items()}
    print(f"  Frequency table: {counts}")
    is_symmetric = sorted(counts.keys()) == [10, 11, 13, 14] and counts[10] == counts[14] and counts[11] == counts[13]
    print(f"  Distribution symmetric about 12, with n=11 and n=13 EXACTLY TIED: {is_symmetric}")
    return buckets


def test_empirical_viability(buckets: Dict[int, List[Tuple[int, ...]]]) -> bool:
    """For every distinct n the multiset can reach, how does the predicted
    m_e compare to the CODATA value? This is the ONLY thing in this file
    that breaks the n=11 / n=13 tie."""
    print()
    print("Test 2: empirical viability of every reachable n")
    print(f"  alpha = 1/{ALPHA_INV}, M_P = {M_PLANCK_MEV:.6e} MeV, "
          f"prefactor sqrt(2pi)*(16/3) = {PREFACTOR:.6f}")
    print(f"  Measured m_e (CODATA 2022) = {M_ELECTRON_MEV} MeV")
    print()
    all_pass = True
    for n in sorted(buckets):
        predicted = M_PLANCK_MEV * PREFACTOR * ALPHA**n
        rel_err = (predicted - M_ELECTRON_MEV) / M_ELECTRON_MEV
        tag = "MATCH" if abs(rel_err) < 0.01 else "excluded"
        print(f"  n={n:2d}  predicted m_e = {predicted:.6e} MeV   "
              f"rel. error = {rel_err:+.3%}   [{tag}]")
        if n == 11 and abs(rel_err) >= 0.01:
            all_pass = False
        if n != 11 and abs(rel_err) < 0.01:
            all_pass = False
    print()
    print("  Only n=11 is within 1% of the measured value; n=10, 13, 14 are")
    print("  wrong by multiple orders of magnitude (each unit of n rescales")
    print(f"  the prediction by a factor of alpha = 1/{ALPHA_INV:.1f}).")
    return all_pass


def test_rule_pair_census(buckets: Dict[int, List[Tuple[int, ...]]]) -> bool:
    """Exhaustively enumerate every rule pair in the SAME STYLE as
    (S1, S2) -- one position-extreme rule ("X is first/last") plus one
    relative-order rule ("X before Y") -- and ask how many of them
    uniquely select a single ordering, and which n that ordering gives.
    This replaces a hand-picked 'mirror rule' with a full census, so the
    claim that S1+S2 is not special is checked, not asserted."""
    print()
    print("Test 3: census of every S1-style + S2-style rule pair")
    print("        (position-extreme rule x relative-order rule)")
    print()
    multiset = (N_C, N_C, N_BASE, N_F)
    orderings = sorted(set(permutations(multiset)))

    # Type-A: position-extreme rules on the two elements with unique value
    # (4 and 6 -- "a 3" is ambiguous since there are two of them).
    type_a = {
        "4 first": lambda o: o[0] == N_BASE,
        "4 last":  lambda o: o[-1] == N_BASE,
        "6 first": lambda o: o[0] == N_F,
        "6 last":  lambda o: o[-1] == N_F,
    }
    # Type-B: relative-order rules on each unordered pair, both directions.
    def before(x: int, y: int):
        def rule(o: Tuple[int, ...]) -> bool:
            return o.index(x) < o.index(y)
        return rule
    type_b = {
        "4 before 3": before(N_BASE, N_C),
        "3 before 4": before(N_C, N_BASE),
        "6 before 3": before(N_F, N_C),
        "3 before 6": before(N_C, N_F),
        "4 before 6": before(N_BASE, N_F),
        "6 before 4": before(N_F, N_BASE),
    }

    unique_selectors: Dict[str, int] = {}
    for a_name, a_rule in type_a.items():
        for b_name, b_rule in type_b.items():
            survivors = [o for o in orderings if a_rule(o) and b_rule(o)]
            if len(survivors) == 1:
                n = cumulative_positions(survivors[0])[2]
                unique_selectors[f"{a_name} & {b_name}"] = n

    print(f"  {len(type_a)} position rules x {len(type_b)} order rules = "
          f"{len(type_a) * len(type_b)} combinations checked")
    print(f"  Combinations that uniquely select ONE ordering: "
          f"{len(unique_selectors)}")
    print()
    by_n: Dict[int, int] = {}
    for combo, n in sorted(unique_selectors.items()):
        by_n[n] = by_n.get(n, 0) + 1
        print(f"    {combo:20s} -> n = {n}")
    print()
    print(f"  Of the unique-selecting rule pairs, outcome n is distributed: {by_n}")
    n11_share = by_n.get(11, 0) / len(unique_selectors) if unique_selectors else 0.0
    print(f"  n=11 accounts for {by_n.get(11, 0)}/{len(unique_selectors)} "
          f"({n11_share:.0%}) of the rule pairs that manage to select uniquely "
          f"at all -- i.e. even restricted to this rule STYLE, most")
    print(f"  uniquely-selecting pairs do NOT land on 11. S1+S2 is one member")
    print(f"  of a family that mostly points elsewhere; nothing marks it as")
    print(f"  the privileged member ahead of knowing the empirical target.")
    return True


def main() -> int:
    print("=" * 72)
    print("proof_m_e_exponent_lookelsewhere.py -- ordering-selection audit")
    print("=" * 72)
    buckets = test_full_distribution()
    empirical_ok = test_empirical_viability(buckets)
    test_rule_pair_census(buckets)
    print()
    print("=" * 72)
    print("VERDICT")
    print("=" * 72)
    print("  The O_h multiset-forcing theorem (FTD-0084) is real and remains")
    print("  [THEOREM]. It narrows the exponent to one of {10, 11, 13, 14},")
    print("  with 11 and 13 EXACTLY TIED as the modal outcomes under a")
    print("  uniform prior over orderings (4/12 each) -- n=11 is not even")
    print("  the unique mode, let alone forced.")
    print()
    print("  Among the 24 simple position+order rule pairs in the same style")
    print("  as (S1, S2), only 4 manage to uniquely select a single ordering")
    print("  at all -- and S1+S2 ('6 last & 4 before 3') is just one of")
    print("  those four. The other three uniquely-selecting pairs give")
    print("  n=13 and n=14 (twice), both empirically excluded. S1+S2 was")
    print("  not independently forced ahead of knowing the target; it is")
    print("  the one member of a small, symmetric family that happens to")
    print("  match, selected after the target was already known.")
    print()
    print("  This CLOSES the ordering-uniqueness promotion path (route (a)")
    print("  of MC-T3.2) as NEGATIVE. FTD-0015's [SELECTION] tag on the")
    print("  exponent is confirmed, not just preserved by default. Route")
    print("  (b) (pole-mass calculation) remains separately blocked by")
    print("  FTD-0075 (ultralocal propagator) and is untouched by this file.")
    return 0 if empirical_ok else 1


if __name__ == "__main__":
    sys.exit(main())
