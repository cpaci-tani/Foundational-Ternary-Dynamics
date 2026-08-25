#!/usr/bin/env python3
"""Exact A9/cotangent no-spare-scalar permission obstruction.

The autonomous A9 clock owns exactly one token split between a link and its
reserve.  On its physical one-token orbit, link and reserve capacities are
complements, not two independent permissions.  This certificate classifies
all C4- and charge-conjugation-invariant binary readouts of that orbit and
proves that no two nonconstant readouts factorize.

It also proves that the 192-state cotangent flag/phase carrier is transitive
under O_h x C4, so it contains no nonconstant invariant scalar permission;
handedness cannot be reused because it is a pseudoscalar.  A Cartesian product
of two independently owned A9 copies does have exact equal factorized
marginals, establishing the minimum existing-alphabet type repair while
leaving its native dual ownership and dynamics open.
"""

from __future__ import annotations

from itertools import product

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    signed_permutation_matrices,
)
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag
from proof_ternary_square_phase_polarity_autonomous_clock import (
    capacity,
    conjugate_state,
    iterate,
    occupation,
    polarity,
    rotate_state,
    valid_owned_states,
)


def a9_symmetry_orbit(state):
    return {
        rotate_state(conjugate_state(state) if conjugated else state, turns)
        for conjugated in (0, 1)
        for turns in range(4)
    }


def classify_a9_invariant_permissions() -> tuple[int, tuple, tuple]:
    checks = 0
    states = valid_owned_states()
    unseen = set(states)
    orbits = []
    while unseen:
        start = min(
            unseen,
            key=lambda item: (item.left, item.right, item.link, item.reserve),
        )
        orbit = a9_symmetry_orbit(start)
        assert orbit <= set(states)
        orbits.append(tuple(orbit))
        unseen -= orbit
        checks += 1

    assert sorted(len(orbit) for orbit in orbits) == [8, 8]
    assert {
        occupation(orbit[0].link) for orbit in orbits
    } == {0, 1}
    checks += 2

    reserve_owned = next(orbit for orbit in orbits if occupation(orbit[0].link) == 0)
    link_owned = next(orbit for orbit in orbits if occupation(orbit[0].link) == 1)

    for state in states:
        link_occupation = occupation(state.link)
        reserve_occupation = occupation(state.reserve)
        manifestation = int(state.left != 0 or state.right != 0)
        assert link_occupation + reserve_occupation == 1
        assert manifestation == link_occupation
        assert capacity(state.link) == reserve_occupation
        assert capacity(state.reserve) == link_occupation
        checks += 4

    # Every symmetry-invariant binary readout is one of the four assignments
    # to the two ownership orbits.  The two nonconstant functions are exactly
    # complementary ownership/capacity readouts.
    invariant_readouts = tuple(product((0, 1), repeat=2))
    assert len(invariant_readouts) == 4
    nonconstant = tuple(bits for bits in invariant_readouts if bits[0] != bits[1])
    assert set(nonconstant) == {(0, 1), (1, 0)}
    checks += 2

    # Every autonomous clock orbit contains each ownership state four times.
    # Same-read joint counts are idempotent; complement-read joint counts are
    # zero.  Neither equals the product of two nontrivial marginals.
    for start in states:
        clock_orbit = tuple(iterate(start, step) for step in range(8))
        assert len(set(clock_orbit)) == 8
        assert sum(occupation(state.link) for state in clock_orbit) == 4
        checks += 2

        ownership_index = {
            state: int(occupation(state.link) == 1) for state in clock_orbit
        }
        for left_bits in nonconstant:
            for right_bits in nonconstant:
                left_values = tuple(
                    left_bits[ownership_index[state]] for state in clock_orbit
                )
                right_values = tuple(
                    right_bits[ownership_index[state]] for state in clock_orbit
                )
                left_count = sum(left_values)
                right_count = sum(right_values)
                joint_count = sum(
                    left * right
                    for left, right in zip(left_values, right_values)
                )
                assert left_count == right_count == 4
                assert joint_count in (0, 4)
                assert joint_count * 8 != left_count * right_count
                checks += 4

    return checks, reserve_owned, link_owned


def verify_no_self_admission_on_one_clock_cycle() -> int:
    checks = 0
    representatives = {}
    for state in valid_owned_states():
        representatives.setdefault(polarity(state.link if occupation(state.link) else state.reserve), state)
    assert set(representatives) == {-1, 1}
    checks += 1

    for start in representatives.values():
        cycle = tuple(iterate(start, step) for step in range(8))
        assert iterate(start, 8) == start
        assert len(set(cycle)) == 8
        checks += 2

        admissible_masks = []
        for mask in product((0, 1), repeat=8):
            image = tuple(
                (index + 1) % 8 if mask[index] else index
                for index in range(8)
            )
            if len(set(image)) == 8:
                admissible_masks.append(mask)
            checks += 1
        assert admissible_masks == [(0,) * 8, (1,) * 8]
        checks += 1
    return checks


def verify_cotangent_has_no_invariant_scalar_bit() -> int:
    checks = 0
    states = one_particle_states()
    group = tuple(signed_permutation_matrices())
    assert len(states) == 192
    assert len(group) == 48
    checks += 2

    start_flag, start_phase = states[0]
    orbit = {
        (transform_flag(matrix, start_flag), (start_phase + turn) % 4)
        for matrix in group
        for turn in range(4)
    }
    assert orbit == set(states)
    checks += 1

    # Transitivity means every O_h x C4-invariant scalar binary readout is
    # constant.  Handedness is explicitly not such a readout: every improper
    # cubic transformation flips it.
    improper = tuple(matrix for matrix in group if determinant_3(matrix) == -1)
    assert len(improper) == 24
    checks += 1
    for state in states:
        handedness = state[0][2]
        for matrix in improper:
            transformed = transform_flag(matrix, state[0])
            assert transformed[2] == -handedness
            checks += 1
    return checks


def verify_second_a9_copy_type_witness() -> int:
    checks = 0
    states = valid_owned_states()
    product_states = tuple(product(states, repeat=2))
    total = len(product_states)
    assert total == 256
    checks += 1

    time_count = sum(occupation(primal.link) for primal, _dual in product_states)
    space_count = sum(occupation(dual.link) for _primal, dual in product_states)
    joint_count = sum(
        occupation(primal.link) * occupation(dual.link)
        for primal, dual in product_states
    )
    assert time_count == space_count == total // 2
    assert joint_count == total // 4
    assert joint_count * total == time_count * space_count
    checks += 4

    # Each copy separately retains one token and all A9 phase/polarity data.
    for primal, dual in product_states:
        assert occupation(primal.link) + occupation(primal.reserve) == 1
        assert occupation(dual.link) + occupation(dual.reserve) == 1
        checks += 2
    return checks


def main() -> None:
    checks, _reserve_orbit, _link_orbit = classify_a9_invariant_permissions()
    checks += verify_no_self_admission_on_one_clock_cycle()
    checks += verify_cotangent_has_no_invariant_scalar_bit()
    checks += verify_second_a9_copy_type_witness()

    print("A9 one-token carrier: two symmetry orbits = reserve-owned/link-owned")
    print("link and reserve capacities are exact complements; joint-open count=0")
    print("partial self-gating of one period-8 clock is reversible only for all/none")
    print("cotangent O_h x C4 carrier is transitive: no spare invariant scalar bit")
    print("two independently owned A9 copies: equal marginals=1/2, joint=1/4 exactly")
    print(
        "PASS: A9/cotangent no-spare scalar permission and dual-copy type price "
        f"({checks} exact checks)"
    )
    print(
        "Open: dual-complex ownership, conservative local generator, sourced "
        "marginals, Maxwell/tensor lift, inhomogeneous response, and lensing"
    )


if __name__ == "__main__":
    main()
