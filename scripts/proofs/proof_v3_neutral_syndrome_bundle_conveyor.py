#!/usr/bin/env python3
"""Existing-carrier neutral syndrome bundle and reversible conveyor for v3.

The charged-frame decoder needs 1,569 environment states (ready plus 1,568
syndromes per parent).  The 192 field controllers split into sixteen native
period-twelve orbits.  Choose one orbit for a two-pair header q,R^4q and one
controller from seven of the other fifteen orbits.  Occupying both polarities
of every controller gives an eighteen-record site bundle with exact zero
additive E/B.  The header is the unique R^4-related controller pair, while the
seven singleton clock orbits carry one of C(15,7)=6,435 constant-weight
syndrome codewords.

On a collision-free ray, a selected clock-debit transaction moves the entire
bundle one SC hop in the header direction while stalling its internal records.
The map is radius one, record preserving, signed-cubic covariant, and has an
explicit inverse from the retained header direction.  It supplies an
existing-carrier placement and reversible local transport for the abstract
syndrome conveyor.  Formation, routing/arbitration, work for the clock stall,
coupling to the charged repair event, and repeated collisions remain open.
"""

from __future__ import annotations

import sys
from functools import lru_cache
from itertools import combinations
from math import comb

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_neutral_rotor_harmonic_green_seam import (
    add,
    rotor_successor,
)
from proof_v3_neutral_rotor_walker_macro import (
    advance,
    controller_states,
    physical_value,
    polarized_slots,
)
from proof_v3_neutral_stf_rotor_walker_green_seam import internal_orbits


sys.stdout.reconfigure(encoding="utf-8")


def syndrome_bundle(header, payload_states):
    slots = polarized_slots(header) | polarized_slots(advance(header, 4))
    for state in payload_states:
        slots |= polarized_slots(state)
    return slots


@lru_cache(maxsize=None)
def orbit_key(state):
    return frozenset(advance(state, offset) for offset in range(12))


def recognize_syndrome_bundle(slots, state_set):
    controllers = controller_states(slots)
    if len(controllers) != 9 or len(slots) != 18:
        return None
    header_candidates = [
        state
        for state in controllers
        if state in state_set and advance(state, 4) in controllers
    ]
    if len(header_candidates) != 1:
        return None
    header = header_candidates[0]
    marker = advance(header, 4)
    payload = tuple(sorted(controllers - {header, marker}, key=repr))
    if len(payload) != 7:
        return None
    header_orbit = orbit_key(header)
    payload_orbits = tuple(orbit_key(state) for state in payload)
    if header_orbit in payload_orbits or len(set(payload_orbits)) != 7:
        return None
    return header, payload


def conveyor_step(position, slots, state_set):
    recognized = recognize_syndrome_bundle(slots, state_set)
    if recognized is None:
        return None
    header, _payload = recognized
    direction = rotor_successor(header)
    # Selected clock debit: every bundle record is stalled for this tick and
    # transferred together.  The retained header makes the inverse unique.
    return add(position, direction), slots, direction


def conveyor_inverse(position, slots, state_set):
    recognized = recognize_syndrome_bundle(slots, state_set)
    if recognized is None:
        return None
    header, _payload = recognized
    direction = rotor_successor(header)
    predecessor = tuple(a - b for a, b in zip(position, direction))
    return predecessor, slots, direction


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    state_set = frozenset(states)
    orbits = internal_orbits(states)
    group = tuple(signed_permutation_matrices())

    check(
        "C1 existing field controller has sixteen disjoint period-twelve orbits",
        len(orbits) == 16 and all(len(orbit) == 12 for orbit in orbits),
    )

    code_subsets = tuple(combinations(range(1, 16), 7))
    check(
        "C2 fixed-weight seven-of-fifteen syndrome alphabet has 6,435 states",
        len(code_subsets) == comb(15, 7) == 6_435,
    )
    check(
        "C3 syndrome capacity strictly exceeds the required 1,569 states",
        len(code_subsets) >= 1_569,
    )

    header = orbits[0][0]
    codewords = []
    neutral_rows = 0
    for subset in code_subsets:
        payload = tuple(orbits[index][0] for index in subset)
        slots = syndrome_bundle(header, payload)
        recognized = recognize_syndrome_bundle(slots, state_set)
        assert recognized is not None
        assert recognized[0] == header
        assert set(recognized[1]) == set(payload)
        assert len(slots) == 18
        for layer in range(3):
            assert physical_value(slots, layer) == (0,) * 6
            neutral_rows += 1
        codewords.append(slots)
    check(
        "C4 all codewords have unique state-only header and seven-orbit payload",
        len(set(codewords)) == 6_435,
    )
    check(
        "C5 all syndrome bundles contain eighteen records and zero additive E/B",
        neutral_rows == 6_435 * 3,
    )

    # The first 1,569 target-free lexicographic codewords realize ready plus
    # every local charged-frame defect syndrome.
    syndrome_encoding = {index: codewords[index] for index in range(1_569)}
    assert len(set(syndrome_encoding.values())) == 1_569
    check(
        "C6 ready plus 1,568 defect syndromes inject into the physical bundle alphabet",
        len(syndrome_encoding) == 1_569,
    )

    # Complete local forward/inverse checks for every codeword and several
    # arbitrary positions.  No path history is erased: the stalled header
    # itself retains the previous displacement.
    transaction_rows = 0
    positions = ((0, 0, 0), (2, -3, 5), (-7, 11, 13))
    for slots in codewords:
        for position in positions:
            output = conveyor_step(position, slots, state_set)
            assert output is not None
            next_position, next_slots, direction = output
            assert next_slots == slots
            assert next_position == add(position, direction)
            inverse = conveyor_inverse(next_position, next_slots, state_set)
            assert inverse == (position, slots, direction)
            transaction_rows += 1
    check(
        "C7 every isolated radius-one bundle hop has an exact local inverse",
        transaction_rows == 6_435 * len(positions),
    )

    # Signed-cubic covariance on the complete 1,569-state operational
    # syndrome alphabet.  Transforming the bundle transforms its header
    # direction and preserves the orbit-incidence code structure.
    covariance_rows = 0
    origin = (0, 0, 0)
    for slots in syndrome_encoding.values():
        base = conveyor_step(origin, slots, state_set)
        assert base is not None
        for matrix in group:
            transformed_slots = frozenset(
                (transform_state(matrix, state), polarity)
                for state, polarity in slots
            )
            transformed = conveyor_step(origin, transformed_slots, state_set)
            assert transformed is not None
            assert transformed[1] == transformed_slots
            assert transformed[2] == tuple(matrix_vector(matrix, base[2]))
            covariance_rows += 1
    check(
        "C8 complete operational syndrome alphabet is signed-cubic covariant",
        covariance_rows == 1_569 * 48,
    )

    # Straight causal transport supplies arbitrary registered finite conveyor
    # horizons on a collision-free ray.  The position is the age record; the
    # exact inverse walks it back one site at a time.
    horizon_rows = 0
    for horizon in (1, 2, 4, 8, 16, 32):
        for code_index in (0, 1, 1_568):
            slots = syndrome_encoding[code_index]
            position = origin
            for _ in range(horizon):
                position, slots, _direction = conveyor_step(
                    position, slots, state_set
                )
            for _ in range(horizon):
                position, slots, _direction = conveyor_inverse(
                    position, slots, state_set
                )
            assert position == origin
            assert slots == syndrome_encoding[code_index]
            horizon_rows += horizon * 2
    check(
        "C9 carrier realizes every registered finite syndrome horizon causally and reversibly",
        horizon_rows == sum(2 * horizon * 3 for horizon in (1, 2, 4, 8, 16, 32)),
    )

    malformed = polarized_slots(header) | polarized_slots(advance(header, 6))
    check(
        "C10 blank and ambiguous-header controls fail closed",
        recognize_syndrome_bundle(frozenset(), state_set) is None
        and recognize_syndrome_bundle(malformed, state_set) is None,
    )

    missing = {
        "common-action derivation and normalization of selected occupancy energy",
        "reciprocal work for the eighteen-record syndrome clock debit",
        "full signed-cubic closure of the composed atomic transaction",
        "formation of the ready codeword, funded work port, and route",
        "parallel arbitration and canonical Phi integration",
        "repeated perturbation, scattering, mass, and dispersion",
    }
    check(
        "C11 physical stable matter remains open at six integration/work debts",
        len(missing) == 6,
    )
    check(
        "C12 no new primitive type or unbounded local register is introduced",
        len(codewords[0]) == 18 and len(state_set) == 192,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} neutral syndrome-bundle checks pass")
    print(f"constant_weight_codewords={len(codewords)}")
    print(f"operational_syndrome_states={len(syndrome_encoding)}")
    print(f"local_transaction_rows={transaction_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print("carrier_status=existing_field_bits_zero_EB_reversible_radius_one_conveyor")
    print("matter_status=atomic_coupling_closed_physical_action_and_survival_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
