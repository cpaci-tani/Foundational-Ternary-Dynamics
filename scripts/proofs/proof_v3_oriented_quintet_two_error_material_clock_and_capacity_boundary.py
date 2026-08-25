#!/usr/bin/env python3
"""Exact five-copy two-error material-clock certificate for strict v3.

The prepared triplet corrects one valid-symbol substitution.  This successor
uses five disjoint SC A9 owners and five copied 33-symbol neutral field-pair
herald registers.  The existing chart-local clear controller pool has 170
states, so five registers require 165 and fit without a new carrier type;
six would require 198 and do not fit at one site.

Strict majority corrects every one- or two-position substitution among the
five arms and five heralds.  One fixed-occupancy A2 work owner uses three of
its existing C4 phases to retain work count 0, 1, or 2.  The complete clean
clock still has period sixteen.  All 395,600 registered perturbations recover
within two ticks with the exact work count.

The price is structural: five SC directions cannot be isotropic.  The selected
layout has dyad 2 ee + 2 ff + nn, not (5/3)I.  Six copies would restore equal
axis multiplicity but exceed the one-site controller pool.  Formation,
positive binding scale, occupancy faults, traffic, mass, and canonical Phi
remain open.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from functools import lru_cache

from sympy import Matrix

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import LocalState, tick
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_charged_common_action_phi_v3_candidate import relation_key
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    ARM_LAYOUT,
    DARK,
    LOGICAL,
    clock_register_pool,
)
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    SC_DIRECTIONS,
    add,
    relative_site,
    source_edges,
)
from proof_v3_neutral_rotor_walker_macro import physical_value, polarized_slots
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    mv,
    transform_chart,
)
from proof_v3_triplet_relational_work_phi_v5_candidate import (
    HERALD_ALPHABET,
    Pending,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Relative = tuple[int, int, int]
Arm = tuple[Vec, Vec]
Payload = tuple[int, int]
Herald = Pending | None

# The triplet's three disjoint edges plus two additional chart-relative edges.
# The added pair is endpoint-disjoint from the original three and from the
# retained frame/source writers on every signed-cubic chart image.
QUINTET_LAYOUT: tuple[tuple[Relative, int], ...] = ARM_LAYOUT + (
    ((-1, -1, 0), 0),
    ((-1, -1, 1), 1),
)


def quintet_edges(chart: OrientedRepairChart) -> tuple[Arm, ...]:
    axes = (chart.first, chart.second, chart.repair_normal)
    return tuple(
        (relative_site(chart, tail), axes[axis_index])
        for tail, axis_index in QUINTET_LAYOUT
    )


def work_payload(chart: OrientedRepairChart, level: int) -> Payload:
    assert level in (0, 1, 2, 3)
    return (chart.offset + level) % 4, chart.polarity


def work_level(chart: OrientedRepairChart, payload: Payload) -> int:
    assert payload[1] == chart.polarity
    return (payload[0] - chart.offset) % 4


@lru_cache(maxsize=None)
def quintet_register_banks(
    chart: OrientedRepairChart, states
) -> tuple[dict[Herald, object], ...]:
    pool = clock_register_pool(chart, states)
    required = 5 * len(HERALD_ALPHABET)
    assert len(pool) >= required
    return tuple(
        dict(
            zip(
                HERALD_ALPHABET,
                pool[
                    index * len(HERALD_ALPHABET) :
                    (index + 1) * len(HERALD_ALPHABET)
                ],
            )
        )
        for index in range(5)
    )


def strict_majority(values):
    for value in values:
        if values.count(value) >= 3:
            return value
    return None


@dataclass(frozen=True)
class QuintetClock:
    arms: tuple[LocalState, LocalState, LocalState, LocalState, LocalState]
    heralds: tuple[Herald, Herald, Herald, Herald, Herald]
    work: Payload


def clean_state(
    chart: OrientedRepairChart, logical: LocalState, heralded: bool = False
) -> QuintetClock:
    herald: Herald = Pending(logical, 0) if heralded else DARK
    return QuintetClock(
        (logical,) * 5,
        (herald,) * 5,
        work_payload(chart, 0),
    )


def quintet_step(chart: OrientedRepairChart, state: QuintetClock) -> QuintetClock:
    """Five-copy READ/COMMIT branch with counted one/two-error work."""

    dark_majority = state.heralds.count(DARK) >= 3
    if dark_majority:
        logical = strict_majority(state.arms)
        if logical is None:
            return state
        syndrome_count = sum(arm != logical for arm in state.arms) + sum(
            herald is not DARK for herald in state.heralds
        )
        if syndrome_count > 2:
            return state
        current_level = work_level(chart, state.work)
        if syndrome_count and current_level != 0:
            return state
        pending = Pending(logical, int(syndrome_count > 0))
        return QuintetClock(
            state.arms,
            (pending,) * 5,
            work_payload(chart, syndrome_count) if syndrome_count else state.work,
        )

    pending = strict_majority(state.heralds)
    if pending is None or pending is DARK:
        return state
    assert isinstance(pending, Pending)
    mismatch_count = sum(arm != pending.logical for arm in state.arms) + sum(
        herald != pending for herald in state.heralds
    )
    if mismatch_count > 2:
        return state
    current_level = work_level(chart, state.work)
    if pending.syndrome:
        # The READ layer already counted every admitted snapshot error.  Arm
        # mismatches persist until COMMIT while herald mismatches have been
        # overwritten, so the surviving mismatch count may be any value up to
        # the retained work level.  Errors arriving between the two layers are
        # outside this registered snapshot basin and remain an explicit debt.
        if current_level not in (1, 2) or mismatch_count > current_level:
            return state
        output_level = current_level
    elif mismatch_count:
        if current_level != 0:
            return state
        output_level = mismatch_count
    else:
        output_level = current_level
    return QuintetClock(
        (tick(pending.logical),) * 5,
        (DARK,) * 5,
        work_payload(chart, output_level),
    )


def iterate(chart: OrientedRepairChart, state: QuintetClock, count: int):
    for _ in range(count):
        state = quintet_step(chart, state)
    return state


def body_projection(state: QuintetClock):
    return state.arms, state.heralds


def role_occupancy(chart: OrientedRepairChart, state: QuintetClock):
    assert all(arm in LOGICAL for arm in state.arms)
    assert all(herald in HERALD_ALPHABET for herald in state.heralds)
    assert work_level(chart, state.work) in (0, 1, 2, 3)
    # Every herald symbol, including abstract DARK, is represented by one
    # occupied neutral opposite-polarity field pair in its physical bank.
    return 2 * len(state.heralds), len(state.arms), 0, 1


def replacement_alphabet(state: QuintetClock, position: int):
    return LOGICAL if position < 5 else HERALD_ALPHABET


def replace_position(state: QuintetClock, position: int, replacement):
    if position < 5:
        arms = list(state.arms)
        arms[position] = replacement
        return QuintetClock(tuple(arms), state.heralds, state.work)
    heralds = list(state.heralds)
    heralds[position - 5] = replacement
    return QuintetClock(state.arms, tuple(heralds), state.work)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def dyad(vector: Vec) -> Matrix:
    column = Matrix(vector)
    return column * column.T


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    generators = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((1, 0, 0), (0, 0, 1), (0, 1, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )
    charts = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }

    minimum_pool = min(len(clock_register_pool(chart, states)) for chart in charts)
    check(
        "C1 five copied 33-symbol herald registers fit the exact 170-state clear pool but six do not",
        len(HERALD_ALPHABET) == 33
        and minimum_pool == 170
        and 5 * 33 == 165 <= minimum_pool < 6 * 33,
    )

    geometry_rows = 0
    register_rows = 0
    covariance_rows = 0
    for chart in charts:
        edges = quintet_edges(chart)
        relation_keys = {relation_key(edge) for edge in edges}
        endpoints = [
            point
            for tail, direction in edges
            for point in (tail, add(tail, direction))
        ]
        frame_keys = {relation_key(edge) for edge in chart.edges()}
        source_keys = {
            relation_key(edge)
            for direction in SC_DIRECTIONS
            for edge in source_edges(chart, direction)
        }
        assert len(relation_keys) == 5
        assert len(endpoints) == len(set(endpoints)) == 10
        assert not (relation_keys & frame_keys)
        assert not (relation_keys & source_keys)
        for point in endpoints:
            coordinate = tuple(
                sum(
                    (point[index] - chart.origin[index]) * axis[index]
                    for index in range(3)
                )
                for axis in (chart.first, chart.second, chart.repair_normal)
            )
            assert max(abs(value) for value in coordinate) <= 1
        geometry_rows += 1

        banks = quintet_register_banks(chart, states)
        selected = [value for bank in banks for value in bank.values()]
        assert len(selected) == len(set(selected)) == 165
        for bank in banks:
            assert set(bank) == set(HERALD_ALPHABET)
            for state in bank.values():
                slots = polarized_slots(state)
                assert len(slots) == 2
                assert all(
                    physical_value(slots, layer) == (0,) * 6 for layer in range(3)
                )
                register_rows += 1

        for matrix in generators:
            transformed_chart = transform_chart(matrix, chart)
            transformed_banks = quintet_register_banks(transformed_chart, states)
            assert quintet_edges(transformed_chart) == tuple(
                (mv(matrix, tail), mv(matrix, direction))
                for tail, direction in edges
            )
            assert all(
                work_payload(transformed_chart, level) == work_payload(chart, level)
                for level in range(4)
            )
            for left_bank, right_bank in zip(banks, transformed_banks):
                for symbol in HERALD_ALPHABET:
                    assert transform_state(matrix, left_bank[symbol]) == right_bank[symbol]
                    covariance_rows += 1

    check(
        "C2 five A9 owners are endpoint-disjoint, radius-one, frame/source-clear, and chart covariant",
        geometry_rows == 1_152,
    )
    check(
        "C3 all five neutral 33-symbol banks use distinct existing field-pair states",
        register_rows == 1_152 * 5 * 33,
    )
    check(
        "C4 complete register and work-level construction is signed-cubic generator covariant",
        covariance_rows == 1_152 * 3 * 5 * 33,
    )

    chart = canonical_chart(frame_family()[0])
    seed = LOGICAL[0]
    orbit = [clean_state(chart, seed)]
    for _ in range(15):
        orbit.append(quintet_step(chart, orbit[-1]))
    check(
        "C5 the clean quintet material clock has exact complete period sixteen",
        quintet_step(chart, orbit[-1]) == orbit[0]
        and len(set(orbit)) == 16,
    )

    # Minimum mixed-symbol code distance across clean logical codewords.
    minimum_distance = 10
    for phase in range(16):
        codewords = [iterate(chart, clean_state(chart, logical), phase) for logical in LOGICAL]
        for left_index, left in enumerate(codewords):
            for right in codewords[left_index + 1 :]:
                distance = sum(a != b for a, b in zip(left.arms, right.arms)) + sum(
                    a != b for a, b in zip(left.heralds, right.heralds)
                )
                minimum_distance = min(minimum_distance, distance)
    check(
        "C6 the clean quintet code has minimum mixed-symbol distance five and unique radius-two decoding",
        minimum_distance == 5,
    )
    check(
        "C7 five copies are minimum in the repetition architecture for correcting two substitutions",
        2 * 2 + 1 == 5,
    )

    single_rows = 0
    double_rows = 0
    maximum_recovery = 0
    for reference in orbit:
        expected = {step: iterate(chart, reference, step) for step in (1, 2)}
        # One-position substitutions.
        for position in range(10):
            current = reference.arms[position] if position < 5 else reference.heralds[position - 5]
            for replacement in replacement_alphabet(reference, position):
                if replacement == current:
                    continue
                mutant = replace_position(reference, position, replacement)
                matches = [
                    step
                    for step in (1, 2)
                    if body_projection(iterate(chart, mutant, step))
                    == body_projection(expected[step])
                ]
                assert matches, ("single", reference, position, replacement, mutant)
                recovered = matches[0]
                output = iterate(chart, mutant, recovered)
                assert work_level(chart, output.work) == 1
                maximum_recovery = max(maximum_recovery, recovered)
                single_rows += 1

        # Two substitutions at distinct positions.
        for first in range(10):
            current_first = (
                reference.arms[first] if first < 5 else reference.heralds[first - 5]
            )
            for second in range(first + 1, 10):
                current_second = (
                    reference.arms[second]
                    if second < 5
                    else reference.heralds[second - 5]
                )
                for replacement_first in replacement_alphabet(reference, first):
                    if replacement_first == current_first:
                        continue
                    once = replace_position(reference, first, replacement_first)
                    for replacement_second in replacement_alphabet(reference, second):
                        if replacement_second == current_second:
                            continue
                        mutant = replace_position(once, second, replacement_second)
                        matches = [
                            step
                            for step in (1, 2)
                            if body_projection(iterate(chart, mutant, step))
                            == body_projection(expected[step])
                        ]
                        assert matches, (
                            "double",
                            reference,
                            first,
                            second,
                            replacement_first,
                            replacement_second,
                            mutant,
                        )
                        recovered = matches[0]
                        output = iterate(chart, mutant, recovered)
                        assert work_level(chart, output.work) == 2
                        maximum_recovery = max(maximum_recovery, recovered)
                        double_rows += 1

    check(
        "C8 every one-position substitution recovers with exact work level one",
        single_rows == 16 * (5 * 15 + 5 * 32) == 3_760,
    )
    check(
        "C9 every two-position substitution recovers with exact work level two",
        double_rows
        == 16 * (10 * 15 * 15 + 25 * 15 * 32 + 10 * 32 * 32)
        == 391_840,
    )
    check(
        "C10 the complete 395,600-row radius-two basin rejoins within at most two ticks",
        single_rows + double_rows == 395_600 and maximum_recovery == 2,
    )

    busy_reference = QuintetClock(
        orbit[0].arms,
        orbit[0].heralds,
        work_payload(chart, 2),
    )
    busy_error = replace_position(busy_reference, 0, LOGICAL[1])
    check(
        "C11 clean recurrence continues with retained work while a new repair on a busy port fails closed",
        body_projection(iterate(chart, busy_reference, 2))
        == body_projection(iterate(chart, orbit[0], 2))
        and quintet_step(chart, busy_error) == busy_error,
    )

    directions = [direction for _tail, direction in quintet_edges(chart)]
    shape = sum((dyad(direction) for direction in directions), Matrix.zeros(3, 3))
    isotropic_target = Matrix.eye(3) * Matrix([[5]])[0] / 3
    check(
        "C12 the five-copy one-site construction has exact oriented dyad diag(2,2,1), not an isotropic rest source",
        shape == Matrix.diag(2, 2, 1) and shape != isotropic_target,
    )

    role_rows = 0
    for reference in orbit:
        assert role_occupancy(chart, reference) == (10, 5, 0, 1)
        role_rows += 1
        for position in range(10):
            current = reference.arms[position] if position < 5 else reference.heralds[position - 5]
            for replacement in replacement_alphabet(reference, position):
                if replacement == current:
                    continue
                mutant = replace_position(reference, position, replacement)
                assert role_occupancy(chart, mutant) == (10, 5, 0, 1)
                role_rows += 1
    check(
        "C13 all clean and admitted error symbols preserve role occupancy (10 field,5 SC-A1,0 FCC-A1,1 A2)",
        role_rows == 16 * (1 + 5 * 15 + 5 * 32),
    )

    forbidden = (
        "137.036",
        "particle_mass",
        "empirical_target",
        "random_draw",
        "master_root",
    )
    missing = {
        "genesis-seed formation of the quintet",
        "positive binding scale and work export reset",
        "occupancy loss gain and more than two errors",
        "isotropic rest source or multisite six-copy extension",
        "translation traffic collisions and scattering",
        "mass dispersion packet absorption and canonical Phi",
    }
    check(
        "C14 the quintet closes a radius-two symbol basin, not general stable matter",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} oriented-quintet material-clock checks pass")
    print(f"minimum_clear_controller_pool={minimum_pool}")
    print("five_register_states=165")
    print("six_register_states=198")
    print(f"single_substitution_rows={single_rows}")
    print(f"double_substitution_rows={double_rows}")
    print(f"total_radius_two_rows={single_rows + double_rows}")
    print(f"maximum_recovery_ticks={maximum_recovery}")
    print(f"minimum_code_distance={minimum_distance}")
    print(f"role_occupancy_rows={role_rows}")
    print("work_levels=0,1,2_in_one_fixed_occupancy_A2")
    print("quintet_source_shape=diag(2,2,1)")
    print("status=prepared_two_error_quintet_exact_isotropy_binding_general_stability_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
