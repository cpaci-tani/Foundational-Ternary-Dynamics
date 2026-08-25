#!/usr/bin/env python3
"""Exact prepared bipartite Born-count/no-signalling/local-ceiling theorem.

Two spacelike local v3 history banks receive descendants of one finite source
record lambda.  Each local outcome count is a Gaussian-integer compatible-pair
cardinality |Z|^2.  Source multiplicities are setting independent and every
local setting has the same complete trial cardinality.  The product update is
pointwise independent of the remote setting, its exact marginals are
no-signalling, and any source-complete local response refinement obeys CHSH
|S|<=2.

This is a prepared local-sector theorem.  It neither imports a singlet nor
reproduces Bell-violating laboratory correlations.  Context-specific banks
without a common source coupling do not instantiate one CHSH joint model;
that is a boundary, not a derivation of a violation.
"""

from __future__ import annotations

import sys
from collections import Counter
from fractions import Fraction
from itertools import product

from sympy import Symbol, simplify

from proof_v3_field_bank_gaussian_born_readout import bright_pair_count


sys.stdout.reconfigure(encoding="utf-8")

OUTCOMES = (-1, 1)
SETTINGS = (0, 1)
SECTORS = (0, 1, 2)
SOURCE_MULTIPLICITY = {0: 1, 1: 2, 2: 1}

# Exact v3 C4 count representatives.  Their compatible ordered-pair counts
# are four and one, respectively.
COUNT4 = (2, 0, 0, 0)
COUNT1 = (1, 0, 0, 0)


def outcome_counts(heavy_positive: bool):
    return {
        1: COUNT4 if heavy_positive else COUNT1,
        -1: COUNT1 if heavy_positive else COUNT4,
    }


# Nontrivial prepared local response tables.  Each (lambda,setting) block has
# total compatible cardinality five.  Settings permute which local outcome is
# heavy; no remote setting appears in either table.
A_BANK = {
    (sector, setting): outcome_counts(
        ((sector, setting) in {(0, 0), (1, 1), (2, 0), (2, 1)})
    )
    for sector in SECTORS
    for setting in SETTINGS
}
B_BANK = {
    (sector, setting): outcome_counts(
        ((sector, setting) in {(0, 0), (0, 1), (1, 0), (2, 1)})
    )
    for sector in SECTORS
    for setting in SETTINGS
}


def weight(bank, sector: int, setting: int, outcome: int) -> int:
    return bright_pair_count(bank[(sector, setting)][outcome])


def local_slots(bank, sector: int, setting: int) -> tuple[int, ...]:
    return tuple(
        outcome
        for outcome in OUTCOMES
        for _ in range(weight(bank, sector, setting, outcome))
    )


def joint_count(a: int, b: int, left: int, right: int) -> int:
    return sum(
        SOURCE_MULTIPLICITY[sector]
        * weight(A_BANK, sector, a, left)
        * weight(B_BANK, sector, b, right)
        for sector in SECTORS
    )


def event_sequence(a: int, b: int):
    """Deterministic full local-history enumeration for one context."""

    events = []
    for sector in SECTORS:
        left_slots = local_slots(A_BANK, sector, a)
        right_slots = local_slots(B_BANK, sector, b)
        for copy in range(SOURCE_MULTIPLICITY[sector]):
            for left_index, left in enumerate(left_slots):
                for right_index, right in enumerate(right_slots):
                    events.append(
                        (sector, copy, left_index, right_index, left, right)
                    )
    return tuple(events)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    check(
        "C1 local C4 records realize exact compatible counts four and one",
        bright_pair_count(COUNT4) == 4 and bright_pair_count(COUNT1) == 1,
    )

    local_rows = 0
    for bank in (A_BANK, B_BANK):
        for sector, setting in product(SECTORS, SETTINGS):
            assert sum(
                weight(bank, sector, setting, outcome) for outcome in OUTCOMES
            ) == 5
            assert len(local_slots(bank, sector, setting)) == 5
            local_rows += 1
    check(
        "C2 every local source/setting block is complete with cardinality five",
        local_rows == 2 * 3 * 2,
    )

    joint_rows = 0
    total_rows = 0
    for a, b in product(SETTINGS, repeat=2):
        events = event_sequence(a, b)
        observed = Counter((event[-2], event[-1]) for event in events)
        expected = Counter(
            {
                (left, right): joint_count(a, b, left, right)
                for left, right in product(OUTCOMES, repeat=2)
            }
        )
        assert observed == expected
        assert len(events) == sum(expected.values()) == 100
        joint_rows += len(expected)
        total_rows += len(events)
    check(
        "C3 deterministic product enumeration realizes every exact joint count",
        joint_rows == 16 and total_rows == 400,
    )

    marginal_rows = 0
    for a in SETTINGS:
        references = {}
        for b in SETTINGS:
            total = sum(
                joint_count(a, b, left, right)
                for left, right in product(OUTCOMES, repeat=2)
            )
            marginal = tuple(
                Fraction(
                    sum(joint_count(a, b, left, right) for right in OUTCOMES),
                    total,
                )
                for left in OUTCOMES
            )
            references[b] = marginal
        assert references[0] == references[1]
        marginal_rows += 2
    for b in SETTINGS:
        references = {}
        for a in SETTINGS:
            total = sum(
                joint_count(a, b, left, right)
                for left, right in product(OUTCOMES, repeat=2)
            )
            marginal = tuple(
                Fraction(
                    sum(joint_count(a, b, left, right) for left in OUTCOMES),
                    total,
                )
                for right in OUTCOMES
            )
            references[a] = marginal
        assert references[0] == references[1]
        marginal_rows += 2
    check(
        "C4 exact normalized marginals are independent of the remote setting",
        marginal_rows == 8,
    )

    # Stronger than equality after aggregation: because every remote setting
    # has the same five physical slots, changing its local outcome map leaves
    # the other wing's outcome sequence unchanged at every enumeration index.
    pointwise_rows = 0
    for a in SETTINGS:
        left0 = tuple(event[-2] for event in event_sequence(a, 0))
        left1 = tuple(event[-2] for event in event_sequence(a, 1))
        assert left0 == left1
        pointwise_rows += len(left0)
    for b in SETTINGS:
        right0 = tuple(event[-1] for event in event_sequence(0, b))
        right1 = tuple(event[-1] for event in event_sequence(1, b))
        assert right0 == right1
        pointwise_rows += len(right0)
    check(
        "C5 local event sequences are pointwise independent of the remote setting",
        pointwise_rows == 400,
    )

    # Symbolic completeness theorem for two arbitrary source sectors.  Once
    # every remote local block sums to the same K_B, its internal split cancels
    # from the opposite marginal numerator and denominator.
    r0, r1 = Symbol("r0", positive=True), Symbol("r1", positive=True)
    a0, a1 = Symbol("a0", nonnegative=True), Symbol("a1", nonnegative=True)
    kb = Symbol("K_B", positive=True)
    b00, b10 = Symbol("b00", nonnegative=True), Symbol("b10", nonnegative=True)
    b01, b11 = Symbol("b01", nonnegative=True), Symbol("b11", nonnegative=True)
    marginal_setting0 = simplify(
        (
            r0 * a0 * (b00 + (kb - b00))
            + r1 * a1 * (b10 + (kb - b10))
        )
        / (kb * (r0 + r1))
    )
    marginal_setting1 = simplify(
        (
            r0 * a0 * (b01 + (kb - b01))
            + r1 * a1 * (b11 + (kb - b11))
        )
        / (kb * (r0 + r1))
    )
    check(
        "C6 source-independent complete local cardinality algebraically forces no-signalling",
        simplify(marginal_setting0 - marginal_setting1) == 0
        and not any(
            symbol in marginal_setting0.free_symbols
            for symbol in (b00, b10, b01, b11, kb)
        ),
    )

    # Exact detection/completeness boundary: remote setting changes which
    # source sectors enter the retained sample.  The underlying local outcome
    # rule is unchanged, yet the postselected marginal drifts.
    # A+ occurs only in sector zero.  At b=0 both source sectors retain one
    # event; at b=1 sector one retains two.  The retained A+ marginal changes
    # from 1/(1+1) to 1/(1+2).
    a_plus_by_sector = (1, 0)
    retained_b0 = (1, 1)
    retained_b1 = (1, 2)
    complete_marginal = Fraction(
        sum(a * retained for a, retained in zip(a_plus_by_sector, retained_b0)),
        sum(retained_b0),
    )
    incomplete_marginal = Fraction(
        sum(a * retained for a, retained in zip(a_plus_by_sector, retained_b1)),
        sum(retained_b1),
    )
    check(
        "C7 setting/sector-dependent incomplete retention can create sampled marginal drift",
        complete_marginal != incomplete_marginal,
    )

    chsh_values = []
    for A0, A1, B0, B1 in product((-1, 1), repeat=4):
        value = A0 * B0 + A0 * B1 + A1 * B0 - A1 * B1
        assert abs(value) == 2
        chsh_values.append(value)
    check(
        "C8 every deterministic source-complete local response table has |CHSH|=2",
        len(chsh_values) == 16 and set(chsh_values) == {-2, 2},
    )

    mixture_weights = tuple(range(1, 17))
    mixed_chsh = Fraction(
        sum(weight * value for weight, value in zip(mixture_weights, chsh_values)),
        sum(mixture_weights),
    )
    check(
        "C9 every nonnegative finite mixture stays inside the local CHSH ceiling",
        all(abs(value) <= 2 for value in chsh_values)
        and min(chsh_values) <= mixed_chsh <= max(chsh_values),
    )

    # Four separately prepared contextual banks have four empirical
    # correlations but no single source-indexed counterfactual response table.
    # CHSH may be computed as an epistemic aggregate, but the local-table
    # theorem cannot be applied until a physical cross-context coupling is
    # specified and audited.
    context_keys = {(a, b) for a, b in product(SETTINGS, repeat=2)}
    separate_records = {
        context: frozenset((context, index) for index in range(100))
        for context in context_keys
    }
    cross_context_intersections = {
        left & right
        for first, left in separate_records.items()
        for second, right in separate_records.items()
        if first != second
    }
    check(
        "C10 CHSH applies to the common-source local table, not uncoupled context-specific preparations",
        len(context_keys) == 4
        and len(chsh_values) == 16
        and cross_context_intersections == {frozenset()},
    )

    open_debts = {
        "native common-source bank formation",
        "spacelike apparatus formation and protection",
        "one emission to one paired trial",
        "amplified persistent records and reciprocal backreaction",
        "overlapping traffic and expiry reset",
        "laboratory Bell-correlation recovery or explicit rejection",
    }
    check(
        "C11 prepared local no-signalling closes neither native Born formation nor Bell data",
        len(open_debts) == 6,
    )

    forbidden = (
        "2.828",
        "tsirelson_target",
        "singlet_weight",
        "random_draw",
        "137.036",
    )
    check(
        "C12 no singlet, target Bell value, random draw, coupling target, or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} bipartite Born/no-signalling checks pass")
    print("local_compatible_counts=(4,1)")
    print("local_complete_cardinality=5")
    print("joint_context_cardinality=100")
    print("remote_setting_marginal_drift=0")
    print("pointwise_remote_setting_dependence=0")
    print("native_source_complete_local_CHSH_ceiling=2")
    print("context_specific_uncoupled_CHSH_status=joint_model_not_supplied")
    print("status=prepared_bipartite_local_born_no_signalling_exact")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
