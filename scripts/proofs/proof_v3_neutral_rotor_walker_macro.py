#!/usr/bin/env python3
"""Exact existing-field-carrier neutral rotor/walker macro for FTD-v3.

At an unmarked router site, one opposite-polarity record pair with controller
q stores the zero-field rotor.  A marked site additionally contains the
opposite-polarity pair controlled by R^4 q, where R is the native period-12
internal tick.  Because R^8 q is neither q nor R^4 q, this unordered four-bit
site state has a unique rotor/marker interpretation.

On a visit, q advances to R q, the marker is removed, and its destination is
the SC successor served by R q.  At a clean destination with rotor p, the
marker is written as R^4 p.  The transaction is radius one, signed-cubic
covariant, record-number conserving, and exactly E/B neutral.  Sequential
absorbing-box histories reproduce the previously certified deterministic
rotor paths.

This closes a carrier-complete *neutral sampler* macro, not a charged endpoint
transaction.  External injection, sink expiry, collisions between multiple
walkers, inverse/path retention, reciprocal work, and integration with A9
dressed Gauss strings remain selected/open prices.
"""

from __future__ import annotations

import sys
from collections import defaultdict

from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    SC_DIRECTIONS,
    Vec,
    add,
    box,
    rotor_successor,
    simulate_rotor_box,
)


sys.stdout.reconfigure(encoding="utf-8")


def advance(state, count: int):
    current = state
    for _ in range(count % 12):
        current = internal_tick(current)
    return current


def polarized_slots(state) -> frozenset[tuple[object, int]]:
    return frozenset(((state, 1), (state, -1)))


def unmarked_site(rotor) -> frozenset[tuple[object, int]]:
    return polarized_slots(rotor)


def marked_site(rotor) -> frozenset[tuple[object, int]]:
    return polarized_slots(rotor) | polarized_slots(advance(rotor, 4))


def controller_states(slots) -> frozenset:
    positive = {state for state, polarity in slots if polarity == 1}
    negative = {state for state, polarity in slots if polarity == -1}
    assert positive == negative
    return frozenset(positive)


def recognize_unmarked(slots, state_set):
    controllers = controller_states(slots)
    if len(controllers) != 1:
        return None
    rotor = next(iter(controllers))
    return rotor if rotor in state_set and slots == unmarked_site(rotor) else None


def recognize_marked(slots, state_set):
    controllers = controller_states(slots)
    if len(controllers) != 2:
        return None
    candidates = [
        rotor
        for rotor in controllers
        if rotor in state_set
        and advance(rotor, 4) in controllers
        and marked_site(rotor) == slots
    ]
    return candidates[0] if len(candidates) == 1 else None


def physical_value(slots, layer: int) -> tuple[int, ...]:
    total = [0] * 6
    for state, polarity in slots:
        value = layer_value(state, layer)
        for component in range(6):
            total[component] += polarity * value[component]
    return tuple(total)


def local_step(departure_slots, destination_slots, state_set):
    departure_rotor = recognize_marked(departure_slots, state_set)
    destination_rotor = recognize_unmarked(destination_slots, state_set)
    if departure_rotor is None or destination_rotor is None:
        return None
    advanced_rotor = internal_tick(departure_rotor)
    direction = rotor_successor(advanced_rotor)
    return (
        unmarked_site(advanced_rotor),
        marked_site(destination_rotor),
        direction,
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = one_particle_states()
    state_set = frozenset(states)
    group = tuple(signed_permutation_matrices())

    role_rows = 0
    for rotor in states:
        assert advance(rotor, 12) == rotor
        assert advance(rotor, 4) != rotor
        assert advance(rotor, 8) != rotor
        assert marked_site(rotor) == marked_site(rotor)
        assert recognize_unmarked(unmarked_site(rotor), state_set) == rotor
        assert recognize_marked(marked_site(rotor), state_set) == rotor
        assert len(unmarked_site(rotor)) == 2
        assert len(marked_site(rotor)) == 4
        role_rows += 8
    check("C1 R4 marker offset gives a unique state-only rotor/marker role", role_rows == 192 * 8)

    neutral_rows = 0
    for rotor in states:
        for layer in range(3):
            assert physical_value(unmarked_site(rotor), layer) == (0,) * 6
            assert physical_value(marked_site(rotor), layer) == (0,) * 6
            neutral_rows += 2
    check("C2 unmarked and marked site patterns are exactly E/B neutral", neutral_rows == 192 * 3 * 2)

    transaction_rows = 0
    for departure_rotor in states:
        for destination_rotor in states:
            output = local_step(
                marked_site(departure_rotor),
                unmarked_site(destination_rotor),
                state_set,
            )
            assert output is not None
            departure_after, destination_after, direction = output
            assert recognize_unmarked(departure_after, state_set) == internal_tick(
                departure_rotor
            )
            assert recognize_marked(destination_after, state_set) == destination_rotor
            assert direction in SC_DIRECTIONS
            assert len(marked_site(departure_rotor)) + len(
                unmarked_site(destination_rotor)
            ) == len(departure_after) + len(destination_after) == 6
            for layer in range(3):
                assert physical_value(departure_after, layer) == (0,) * 6
                assert physical_value(destination_after, layer) == (0,) * 6
            transaction_rows += 8
    check("C3 every clean local move retains six records and zero field", transaction_rows == 192 * 192 * 8)
    check("C4 one local transaction advances the departure rotor and moves one marker", transaction_rows > 0)
    check("C5 served displacement is exactly one SC hop", transaction_rows > 0)

    covariance_rows = 0
    for departure_rotor in states:
        destination_rotor = advance(departure_rotor, 3)
        base = local_step(
            marked_site(departure_rotor),
            unmarked_site(destination_rotor),
            state_set,
        )
        assert base is not None
        for matrix in group:
            transformed_departure = transform_state(matrix, departure_rotor)
            transformed_destination = transform_state(matrix, destination_rotor)
            transformed = local_step(
                marked_site(transformed_departure),
                unmarked_site(transformed_destination),
                state_set,
            )
            assert transformed is not None
            assert transformed[0] == frozenset(
                (transform_state(matrix, state), polarity)
                for state, polarity in base[0]
            )
            assert transformed[1] == frozenset(
                (transform_state(matrix, state), polarity)
                for state, polarity in base[1]
            )
            assert transformed[2] == tuple(
                matrix_vector(matrix, base[2])
            )
            covariance_rows += 3
    check("C6 complete neutral-walker transaction is signed-cubic covariant", covariance_rows == 192 * 48 * 3)

    # Exact carrier simulation on absorbing boxes.  It must reproduce the
    # controller-only rotor path from the parent theorem step for step.
    reproduction_rows = 0
    total_steps = 0
    for radius, injections, state_index in (
        (1, 7, 0),
        (1, 37, 37),
        (2, 7, 101),
        (2, 37, 0),
    ):
        vertices = frozenset(box(radius))
        source = (0, 0, 0)
        rotors = {vertex: states[state_index] for vertex in vertices}
        visits: defaultdict[Vec, int] = defaultdict(int)
        traversals: defaultdict[tuple[Vec, Vec], int] = defaultdict(int)

        for _ in range(injections):
            location = source
            marker_present = True
            while location in vertices:
                assert marker_present
                visits[location] += 1
                departure_rotor = rotors[location]
                output = local_step(
                    marked_site(departure_rotor),
                    unmarked_site(
                        rotors[add(location, rotor_successor(internal_tick(departure_rotor)))]
                    )
                    if add(location, rotor_successor(internal_tick(departure_rotor))) in vertices
                    else unmarked_site(states[state_index]),
                    state_set,
                )
                assert output is not None
                departure_after, _destination_after, direction = output
                rotors[location] = recognize_unmarked(departure_after, state_set)
                traversals[(location, direction)] += 1
                location = add(location, direction)
                total_steps += 1
                if location not in vertices:
                    marker_present = False
            assert not marker_present

        parent = simulate_rotor_box(radius, injections, states[state_index])
        assert visits == parent[2]
        assert traversals == parent[3]
        reproduction_rows += len(vertices) + len(traversals)

    check("C7 carrier-complete macro reproduces the certified rotor histories", reproduction_rows > 0 and total_steps > 0)

    blank = frozenset()
    malformed = polarized_slots(states[0]) | polarized_slots(advance(states[0], 6))
    check("C8 blank and ambiguous R6 marker controls fail closed", recognize_marked(blank, state_set) is None and recognize_marked(malformed, state_set) is None)

    missing = {
        "integration into the complete Phi schedule",
        "charged A9 endpoint and dressed-string composition",
        "autonomous source renewal",
        "owned sink expiry and reciprocal work",
        "multiple-walker collision arbitration",
        "inverse or retained path record",
        "field writeback and action normalization",
    }
    check("C9 neutral sampler does not close charged physical dynamics", len(missing) == 7)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 neutral rotor/walker macro checks pass")
    print(f"role_rows={role_rows}")
    print(f"local_transaction_rows={transaction_rows}")
    print(f"signed_cubic_rows={covariance_rows}")
    print(f"reproduction_rows={reproduction_rows}")
    print("neutral_sampler_status=carrier_complete_radius_one_existing_field_bits")
    print("charged_status=A9_string_work_and_full_Phi_integration_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
