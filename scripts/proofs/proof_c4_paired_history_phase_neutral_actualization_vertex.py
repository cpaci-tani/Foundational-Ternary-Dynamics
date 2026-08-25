#!/usr/bin/env python3
"""Exact paired-history invariant to phase-neutral actualization/source vertex.

The real C4 quadrature carrier has a unique normalized symmetric invariant
bilinear form.  For phase records it evaluates to +1 on equal phase, 0 on
quadrature-separated phases, and -1 on opposite phase.  The existing bright,
cross-rail, and dark predicates are exactly these three values, not an
independent compatibility table.

After reversible opposite-phase cancellation, the positive contractions in
the residual ordered-pair bank number exactly |Z_o|^2.  That derived positive
predicate controls the existing reversible manifestation ownership transfer.
The manifested token then produces the registered phase-neutral charge-odd
current and charge-even stress/capacity source on every C18 line.

This closes an exact conditional chain from a prepared history pair to a
physical common source.  It does not derive history-bank preparation, routing,
the address orbit, detector context formation, reciprocal field work, a Born
frequency law for autonomous trials, or any coupling coefficient.
"""

from __future__ import annotations

from itertools import product

from sympy import Matrix, Rational, solve, symbols

from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    LINE_DYADS,
    subtract,
    zero_chart,
)
from proof_c18_phase_neutral_shared_charge_stress_vertex import phase_neutral_sources
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
    charge,
    payload,
    token_count,
)
from proof_reversible_c4_cancellation_click_circuit import (
    Record,
    canonical_cancel,
    click_compatible,
    phase_relation,
    records_from_counts,
)


PHASE_VECTORS = (
    Matrix([1, 0]),
    Matrix([0, 1]),
    Matrix([-1, 0]),
    Matrix([0, -1]),
)


def phase_inner(left_phase: int, right_phase: int) -> int:
    return int((PHASE_VECTORS[left_phase].T * PHASE_VECTORS[right_phase])[0])


def invariant_relation(left: Record, right: Record) -> str | None:
    if left.outcome != right.outcome:
        return None
    value = phase_inner(left.phase, right.phase)
    return {1: "bright", 0: None, -1: "dark"}[value]


def main() -> None:
    checks = 0

    # The normalized symmetric C4-invariant form on the quadrature doublet is
    # unique.  Pair exchange removes the independent antisymmetric wedge.
    a, b, c = symbols("a b c", real=True)
    candidate = Matrix([[a, b], [b, c]])
    quarter_turn = Matrix([[0, -1], [1, 0]])
    equations = list(quarter_turn.T * candidate * quarter_turn - candidate)
    solution = solve(equations + [a - 1], (a, b, c), dict=True)
    assert solution == [{a: 1, b: 0, c: 1}]
    invariant_metric = candidate.subs(solution[0])
    assert invariant_metric == Matrix.eye(2)
    checks += 2

    # The complete local relation table is the invariant contraction table.
    records = tuple(
        Record(outcome, phase, 4 * outcome + phase)
        for outcome, phase in product(range(2), range(4))
    )
    for left in records:
        for right in records:
            assert invariant_relation(left, right) == phase_relation(left, right)
            assert click_compatible(left, right) == (
                left.outcome == right.outcome
                and phase_inner(left.phase, right.phase) == 1
            )
            rotated_left = Record(
                left.outcome, (left.phase + 1) % 4, left.identity
            )
            rotated_right = Record(
                right.outcome, (right.phase + 1) % 4, right.identity
            )
            assert phase_inner(rotated_left.phase, rotated_right.phase) == phase_inner(
                left.phase, right.phase
            )
            checks += 3

    assert tuple(phase_inner(0, phase) for phase in range(4)) == (1, 0, -1, 0)
    checks += 1

    # Reversible dark cancellation leaves only positive or zero contractions;
    # the positive ordered-pair count is the exact coherent norm.
    for raw_counts in product(range(5), repeat=4):
        original = records_from_counts((raw_counts,))
        residual, dark = canonical_cancel(original)
        assert all(
            phase_inner(pair.left.phase, pair.right.phase) == -1 for pair in dark
        )
        positive = 0
        for left in residual:
            for right in residual:
                if left.outcome != right.outcome:
                    continue
                contraction = phase_inner(left.phase, right.phase)
                assert contraction in (0, 1)
                positive += int(contraction == 1)
                checks += 1
        n_0, n_1, n_2, n_3 = raw_counts
        expected = (n_0 - n_2) ** 2 + (n_1 - n_3) ** 2
        assert positive == expected
        checks += 2

    # The positive invariant predicate controls one complete reversible token
    # ownership transfer.  On every bright pair, the same state change gives
    # the already certified phase-neutral current/stress/capacity source.
    for left in records:
        for right in records:
            compatible = invariant_relation(left, right) == "bright"
            for token_phase, orientation in product(range(4), (-1, 1)):
                token = Token(token_phase, orientation)
                reserve = ActualizationState(0, 0, 0, None, token)
                manifested = actualization_macro(reserve, compatible)
                assert actualization_macro(manifested, compatible) == reserve
                assert token_count(manifested) == token_count(reserve) == 1
                assert payload(manifested) == payload(reserve) == token
                assert charge(manifested) == charge(reserve) == 0
                assert (manifested.link is not None) == compatible
                checks += 5

                if not compatible:
                    assert manifested == reserve
                    checks += 1
                    continue

                for line_index, (direction, dyad) in enumerate(
                    zip(LINE_DIRECTIONS, LINE_DYADS)
                ):
                    delta = subtract(
                        zero_chart(manifested, line_index),
                        zero_chart(reserve, line_index),
                    )
                    current, tensor, vector_cross, tensor_cross = phase_neutral_sources(
                        delta, token_phase
                    )
                    assert current == Rational(orientation, 9) * direction
                    assert tensor == dyad / 18 == -delta.capacity
                    assert vector_cross == Matrix.zeros(3, 1)
                    assert tensor_cross == Matrix.zeros(3, 3)
                    checks += 4

    print("unique normalized symmetric C4 quadrature contraction = Euclidean dot")
    print("equal/cross-rail/opposite phases map exactly to bright/none/dark")
    print("reversible dark cancellation leaves |Z|^2 positive ordered contractions")
    print("the positive invariant controls one reversible manifestation transfer")
    print("that same transfer creates phase-neutral charge current and even stress/capacity")
    print(
        "PASS: paired-history phase-neutral actualization vertex "
        f"({checks} exact checks)"
    )
    print(
        "Boundary: prepared histories/addressing/context and reciprocal work remain open; "
        "this is not an autonomous general Born or coupling derivation"
    )


if __name__ == "__main__":
    main()
