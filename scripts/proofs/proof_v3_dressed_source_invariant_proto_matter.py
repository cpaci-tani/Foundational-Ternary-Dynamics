#!/usr/bin/env python3
"""Exact invariant proto-matter family for the v3 dressed-source extension.

The family contains one SC A9 ownership token in either reserve or primary
ownership.  Primary ownership is accompanied by exactly the state-identifiable
eight-channel bound dressing; reserve ownership has no field packet.  All C4
phases, C3 layers, directions, polarities, translations, and signed-cubic
images belong to the same covariant family.

The selected dressed-source macro maps this compact family bijectively to
itself with period 24.  Blank, free-packet, naked-primary, partial-packet, and
extra-local-field controls are rejected.  This proves an invariant localized
proto-object in the candidate extension, not formed robust physical matter.
"""

from __future__ import annotations

import sys

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_dressed_sc_source_gauss_continuity import (
    Channel,
    DressedEdgeState,
    Vec,
    dressed_source_tick,
    target_packet,
    transform_channels,
)


sys.stdout.reconfigure(encoding="utf-8")


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def doubled_anchor(tail: Vec, direction: Vec) -> Vec:
    return tuple(2 * x + d for x, d in zip(tail, direction))  # type: ignore[return-value]


def expected_bank(direction: Vec, state: DressedEdgeState) -> frozenset[Channel]:
    if state.primary is None:
        return frozenset()
    return target_packet(direction, state.primary, state.layer)


def family_margins(direction: Vec, state: DressedEdgeState) -> tuple[int, int, int]:
    one_owned = (state.primary is None) != (state.reserve is None)
    ownership_margin = 1 if one_owned else -1
    expected = expected_bank(direction, state)
    completeness_margin = 1 if expected <= state.bank else -1
    isolation_margin = 1 if state.bank == expected else -1
    return ownership_margin, completeness_margin, isolation_margin


def is_family(direction: Vec, state: DressedEdgeState) -> bool:
    return min(family_margins(direction, state)) > 0


def transform_state(matrix, state: DressedEdgeState) -> DressedEdgeState:
    transformed_bank = (
        transform_channels(matrix, state.bank) if state.bank else frozenset()
    )
    return DressedEdgeState(
        primary=state.primary,
        reserve=state.reserve,
        layer=state.layer,
        bank=transformed_bank,
    )


def family_states(direction: Vec, polarity: int) -> tuple[DressedEdgeState, ...]:
    states = []
    for layer in range(3):
        for phase in range(4):
            payload = (phase, polarity)
            states.append(
                DressedEdgeState(
                    primary=None,
                    reserve=payload,
                    layer=layer,
                    bank=frozenset(),
                )
            )
            states.append(
                DressedEdgeState(
                    primary=payload,
                    reserve=None,
                    layer=layer,
                    bank=target_packet(direction, payload, layer),
                )
            )
    return tuple(states)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    group = tuple(signed_permutation_matrices())
    exact_rows = 0
    all_family_states: set[tuple[Vec, int, DressedEdgeState]] = set()

    for direction in SC_DIRECTIONS:
        for polarity in (-1, 1):
            states = family_states(direction, polarity)
            assert len(states) == 24
            assert len(set(states)) == 24
            assert all(is_family(direction, state) for state in states)

            images = {
                dressed_source_tick(direction, state)
                for state in states
            }
            assert images == set(states)

            for state in states:
                current = state
                for _ in range(24):
                    current = dressed_source_tick(direction, current)
                    assert is_family(direction, current)
                    exact_rows += 1
                assert current == state

                for matrix in group:
                    transformed_direction = tuple(
                        matrix_vector(matrix, direction)
                    )
                    transformed_state = transform_state(matrix, state)
                    assert is_family(transformed_direction, transformed_state)
                    assert transform_state(
                        matrix, dressed_source_tick(direction, state)
                    ) == dressed_source_tick(
                        transformed_direction, transformed_state
                    )
                    exact_rows += 2

                all_family_states.add((direction, polarity, state))

    check("C1 family has all 288 direction/polarity/phase/layer/ownership states", len(all_family_states) == 6 * 2 * 24)
    check("C2 every family member has strictly positive state-only margins", all(min(family_margins(direction, state)) > 0 for direction, _polarity, state in all_family_states))
    check("C3 complete source tick maps the family bijectively to itself", exact_rows > 0)
    check("C4 every family state returns after exactly 24 ticks", exact_rows > 0)
    check("C5 family predicate and tick are signed-cubic covariant", len(group) == 48)

    tail = (3, -2, 7)
    direction = SC_DIRECTIONS[0]
    anchor = doubled_anchor(tail, direction)
    translated_tail = add(tail, (5, 4, -3))
    translated_anchor = doubled_anchor(translated_tail, direction)
    check("C6 doubled midpoint anchor is translation equivariant", translated_anchor == add(anchor, (10, 8, -6)))

    # Mandatory negative controls from the matter-object predicate.
    blank = DressedEdgeState(None, None, 0, frozenset())
    free_packet = DressedEdgeState(
        None,
        None,
        0,
        target_packet(direction, (0, 1), 0),
    )
    naked_primary = DressedEdgeState((0, 1), None, 0, frozenset())
    full_packet = target_packet(direction, (0, 1), 0)
    partial_packet = DressedEdgeState(
        (0, 1),
        None,
        0,
        frozenset(tuple(full_packet)[:-1]),
    )
    extra_channel = next(
        iter(target_packet(SC_DIRECTIONS[2], (0, 1), 0))
    )
    extra_field = DressedEdgeState(
        (0, 1),
        None,
        0,
        full_packet | {extra_channel},
    )
    controls = {
        "blank": blank,
        "free packet": free_packet,
        "naked primary": naked_primary,
        "partial packet": partial_packet,
        "extra local field": extra_field,
    }
    check("C7 blank control is rejected", not is_family(direction, blank))
    check("C8 source-free packet control is rejected", not is_family(direction, free_packet))
    check("C9 naked/partial source controls are rejected", not is_family(direction, naked_primary) and not is_family(direction, partial_packet))
    check("C10 extra-local-field control is rejected", not is_family(direction, extra_field))

    # Exact scope boundary: this invariant finite orbit has no demonstrated
    # formation from blank and no nontrivial basin under local perturbations.
    missing = {
        "formation from generic finite seed",
        "positive reciprocal work ledger",
        "nontrivial local perturbation basin",
        "multi-object scattering",
        "mass and dispersion",
    }
    check("C11 physical stable-matter gates remain explicit", len(missing) == 5)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} invariant proto-matter checks pass")
    print(f"exact_family_rows={exact_rows}")
    print("family_status=compact_state_only_covariant_invariant_period_24")
    print("matter_status=proto_object_only")
    print("Open: formation, work, perturbation basin, scattering, mass/dispersion")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
