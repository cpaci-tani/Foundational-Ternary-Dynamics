"""Exact group-action verifier for the scoped n=11 order-type no-go.

This file uses only the unordered multiset (3, 3, 4, 6), the position
permutation action, and exact integer combinatorics.  It contains no particle
masses, measured constants, fit targets, or residuals.

Run only after tag ``preregister-n11-order-type-no-go-v1`` exists.
"""

from __future__ import annotations

from collections import Counter
from itertools import permutations
import math
import sys


MultisetOrdering = tuple[int, int, int, int]
BASE: MultisetOrdering = (3, 3, 4, 6)
LADDER_START = 4


def position_permutations() -> tuple[tuple[int, int, int, int], ...]:
    return tuple(permutations(range(4)))


def act(ordering: MultisetOrdering, permutation: tuple[int, ...]) -> MultisetOrdering:
    """Left action by permuting positions: output[p[i]] = input[i]."""

    out = [0, 0, 0, 0]
    for source, target in enumerate(permutation):
        out[target] = ordering[source]
    return tuple(out)  # type: ignore[return-value]


def distinct_orderings() -> tuple[MultisetOrdering, ...]:
    return tuple(sorted(set(permutations(BASE))))


def cumulative_position(ordering: MultisetOrdering) -> int:
    return LADDER_START + ordering[0] + ordering[1]


def elementary_symmetric(ordering: MultisetOrdering) -> tuple[int, int, int, int]:
    e1 = sum(ordering)
    e2 = sum(ordering[i] * ordering[j] for i in range(4) for j in range(i + 1, 4))
    e3 = sum(
        ordering[i] * ordering[j] * ordering[k]
        for i in range(4)
        for j in range(i + 1, 4)
        for k in range(j + 1, 4)
    )
    e4 = math.prod(ordering)
    return e1, e2, e3, e4


def invariant_signature(ordering: MultisetOrdering) -> tuple[object, ...]:
    """Representative exact multiset invariants, all recomputed."""

    return (
        tuple(sorted(ordering)),
        tuple(sorted(Counter(ordering).items())),
        sum(ordering),
        math.prod(ordering),
        sum(x * x for x in ordering),
        elementary_symmetric(ordering),
    )


def orbit_components(
    points: tuple[MultisetOrdering, ...],
    group: tuple[tuple[int, int, int, int], ...],
) -> tuple[frozenset[MultisetOrdering], ...]:
    """Recompute connected orbit components under all group actions."""

    unseen = set(points)
    components: list[frozenset[MultisetOrdering]] = []
    while unseen:
        seed = min(unseen)
        component = {act(seed, g) for g in group}
        assert component <= set(points)
        components.append(frozenset(component))
        unseen -= component
    return tuple(components)


def main() -> int:
    group = position_permutations()
    orderings = distinct_orderings()

    assert len(group) == math.factorial(4) == 24
    assert len(orderings) == math.factorial(4) // math.factorial(2) == 12

    base_orbit = {act(BASE, g) for g in group}
    assert base_orbit == set(orderings)
    components = orbit_components(orderings, group)
    assert components == (frozenset(orderings),)

    positions = Counter(cumulative_position(o) for o in orderings)
    assert positions == Counter({10: 2, 11: 4, 13: 4, 14: 2})

    signatures = {invariant_signature(o) for o in orderings}
    assert len(signatures) == 1

    # General invariant-function theorem on a transitive orbit: an invariant
    # coloring is constant on every orbit component.  There is one component,
    # so the only invariant Boolean selectors are all-false and all-true;
    # neither selects exactly one ordering.
    invariant_boolean_selectors = []
    for component_values in ((False,), (True,)):
        selector = {point: component_values[0] for point in components[0]}
        assert all(selector[act(point, g)] == selector[point] for point in orderings for g in group)
        invariant_boolean_selectors.append(selector)
    assert {sum(selector.values()) for selector in invariant_boolean_selectors} == {0, 12}

    # Non-vacuity control: an order-bearing readout varies across the orbit.
    first_entry_values = {o[0] for o in orderings}
    assert first_entry_values == {3, 4, 6}

    print("FTD-0397 n=11 order-type no-go exact verifier")
    print(f"DISTINCT_ORDERINGS: {len(orderings)}")
    for ordering in orderings:
        print(f"  {ordering} -> cumulative_position={cumulative_position(ordering)}")
    print(f"POSITION_COUNTS: {dict(sorted(positions.items()))}")
    print(f"GROUP_SIZE: {len(group)}")
    print(f"ORBIT_COUNT: {len(components)}")
    print(f"ORBIT_SIZE: {len(components[0])}")
    print("MULTISET_INVARIANT_SIGNATURES: 1")
    print("INVARIANT_BOOLEAN_SELECTOR_CARDINALITIES: [0, 12]")
    print("ORDER_BEARING_CONTROL_FIRST_ENTRY_VALUES: [3, 4, 6]")
    print("OUTCOME: PROVEN-SCOPED")
    print("INTERPRETATION: selecting cumulative position 11 requires order-bearing data or independently derived symmetry-breaking dynamics")
    return 0


if __name__ == "__main__":
    sys.exit(main())
