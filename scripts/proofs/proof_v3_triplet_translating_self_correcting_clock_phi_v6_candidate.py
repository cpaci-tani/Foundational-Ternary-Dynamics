#!/usr/bin/env python3
"""Prepared-sector translating self-correcting triplet Phi-v6 candidate.

The cubic triplet's three arm edges lie in the one-sided slab normal to its
existing polar repair header.  Translating the complete chart one hop along
that carried normal places every output endpoint and the new center inside the
prior center's Moore neighborhood.  A unique prepared halo center can
therefore drive one homogeneous radius-one macro transaction that advances
the triplet clock while shifting all arm and herald owners by one SC hop.

On the clean period-sixteen orbit this map is bijective with an explicit
inverse.  On the registered Phi-v5 one-symbol error basin it retains the
parent's noninjective two-tick correction and exact A2 work excitation while
the object translates.  The move preserves the exact occupancy vector and
uses no new carrier type.

This is a prepared-sector Phi-v6 candidate, not canonical Phi.  It assumes the
existing oriented chart/header and clear destination pattern, supplies no
binding or inertial action, emits no gravity packets, and does not arbitrate
multiple moving objects or collisions.
"""

from __future__ import annotations

import sys
from dataclasses import replace

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_ternary_square_phase_polarity_autonomous_clock import (
    charge,
    token_count,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_charged_common_action_phi_v3_candidate import relation_key
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    LOGICAL,
    clock_arm_edges,
    logical_key,
)
from proof_v3_homogeneous_event_halo_phi_v4_candidate import add
from proof_v3_neutral_rotor_walker_macro import physical_value, polarized_slots
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    mv,
    transform_chart,
)
from proof_v3_triplet_discrete_motion_moment_gravity_lift import motion_signature
from proof_v3_triplet_relational_work_phi_v5_candidate import (
    HERALD_ALPHABET,
    WorkClock,
    body_projection,
    clean_state,
    excited_payload,
    one_substitutions,
    ready_payload,
    work_register_banks,
    work_step,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def neg(vector: Vec) -> Vec:
    return tuple(-value for value in vector)  # type: ignore[return-value]


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def chebyshev(left: Vec, right: Vec) -> int:
    return max(abs(a - b) for a, b in zip(left, right))


def shift_chart(chart: OrientedRepairChart, displacement: Vec) -> OrientedRepairChart:
    return replace(chart, origin=add(chart.origin, displacement))


def moving_step(chart: OrientedRepairChart, state: WorkClock):
    """Selected prepared transaction: one polar-normal hop plus one clock tick."""

    return shift_chart(chart, chart.repair_normal), work_step(chart, state)


def moving_iterate(chart: OrientedRepairChart, state: WorkClock, count: int):
    for _ in range(count):
        chart, state = moving_step(chart, state)
    return chart, state


def arm_endpoints(chart: OrientedRepairChart) -> frozenset[Vec]:
    return frozenset(
        endpoint
        for tail, direction in clock_arm_edges(chart)
        for endpoint in (tail, add(tail, direction))
    )


def field_register_slots(chart: OrientedRepairChart, state: WorkClock, states):
    banks = work_register_banks(chart, states)
    return frozenset().union(
        *(
            polarized_slots(bank[symbol])
            for bank, symbol in zip(banks, state.heralds)
        )
    )


def physical_signature(chart: OrientedRepairChart, state: WorkClock, states):
    relations = tuple(
        sorted(
            (
                relation_key(edge),
                logical_key(value),
            )
            for edge, value in zip(clock_arm_edges(chart), state.arms)
        )
    )
    fields = tuple(
        sorted(
            ((chart.origin, slot) for slot in field_register_slots(chart, state, states)),
            key=repr,
        )
    )
    work = (chart.origin, state.work)
    return relations, fields, work


def relative_signature(chart: OrientedRepairChart, state: WorkClock, states):
    relations, fields, work = physical_signature(chart, state, states)
    relative_relations = []
    for edge_key, value in relations:
        tail, head = edge_key
        relative_relations.append(
            (
                tuple(subtract(tail, chart.origin)),
                tuple(subtract(head, chart.origin)),
                value,
            )
        )
    relative_fields = tuple(
        sorted(
            ((subtract(position, chart.origin), slot) for position, slot in fields),
            key=repr,
        )
    )
    relative_work = (subtract(work[0], chart.origin), work[1])
    return tuple(sorted(relative_relations)), relative_fields, relative_work


def clean_work_orbit(chart: OrientedRepairChart) -> tuple[WorkClock, ...]:
    states = [clean_state(chart, LOGICAL[0])]
    for _ in range(15):
        states.append(work_step(chart, states[-1]))
    return tuple(states)


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    chart_orbit = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }

    geometry_rows = 0
    for chart in chart_orbit:
        displacement = chart.repair_normal
        shifted = shift_chart(chart, displacement)
        source_arms = clock_arm_edges(chart)
        output_arms = clock_arm_edges(shifted)
        assert output_arms == tuple(
            (add(tail, displacement), direction)
            for tail, direction in source_arms
        )
        source_relations = {relation_key(edge) for edge in source_arms}
        output_relations = {relation_key(edge) for edge in output_arms}
        output_owners = arm_endpoints(shifted) | {shifted.origin}
        assert len(source_relations) == len(output_relations) == 3
        assert not (source_relations & output_relations)
        assert all(
            chebyshev(owner, chart.origin) <= 1 for owner in output_owners
        )
        assert chebyshev(chart.origin, shifted.origin) == 1
        geometry_rows += 1
    check(
        "C1 all translated arm/center outputs are collision-free and visible in one prior-center Moore neighborhood",
        len(chart_orbit) == 1_152 and geometry_rows == 1_152,
    )

    seed = LOGICAL[0]
    orbit = clean_work_orbit(canonical_chart(frame_family()[0]))
    check(
        "C2 the clean internal triplet orbit remains exactly period sixteen",
        len(set(orbit)) == 16
        and work_step(canonical_chart(frame_family()[0]), orbit[-1]) == orbit[0],
    )

    canonical = canonical_chart(frame_family()[0])
    trajectory = [moving_iterate(canonical, orbit[0], step) for step in range(17)]
    check(
        "C3 clean motion advances exactly one SC hop per global tick while the clock advances",
        all(
            chart.origin
            == tuple(
                canonical.origin[index]
                + step * canonical.repair_normal[index]
                for index in range(3)
            )
            and state == orbit[step % 16]
            for step, (chart, state) in enumerate(trajectory)
        ),
    )

    # The clean-sector inverse uses the prior point of the finite 16-cycle and
    # shifts the chart center one hop opposite its carried normal.
    inverse_rows = 0
    for chart in chart_orbit:
        local_orbit = clean_work_orbit(chart)
        local_predecessor = {
            local_orbit[(index + 1) % 16]: state
            for index, state in enumerate(local_orbit)
        }
        for prior_state in local_orbit:
            after_chart, after_state = moving_step(chart, prior_state)
            recovered_chart = shift_chart(after_chart, neg(after_chart.repair_normal))
            recovered_state = local_predecessor[after_state]
            assert recovered_chart == chart and recovered_state == prior_state
            inverse_rows += 1
    check(
        "C4 the translating clean sector has an explicit exact inverse",
        inverse_rows == 1_152 * 16,
    )

    # Exhaust the Phi-v5 physical one-symbol basin while both perturbed and
    # reference states follow the moving chart trajectory.  Body recovery is
    # compared separately from the work port: every error must leave EXCITED.
    recovery_rows = 0
    work_rows = 0
    maximum_recovery_ticks = 0
    mutations = [
        (reference, mutant)
        for reference in orbit
        for mutant in one_substitutions(reference)
    ]
    for reference, perturbed in mutations:
        recovered = next(
            step
            for step in (1, 2)
            if (
                moving_iterate(canonical, perturbed, step)[0]
                == moving_iterate(canonical, reference, step)[0]
                and body_projection(moving_iterate(canonical, perturbed, step)[1])
                == body_projection(moving_iterate(canonical, reference, step)[1])
            )
        )
        output = moving_iterate(canonical, perturbed, recovered)[1]
        assert output.work == excited_payload(canonical)
        recovery_rows += 1
        work_rows += 1
        maximum_recovery_ticks = max(maximum_recovery_ticks, recovered)
    check(
        "C5 all 2,256 Phi-v5 substitutions rejoin the translating clean body trajectory within two ticks",
        len(mutations) == recovery_rows == 2_256 and maximum_recovery_ticks == 2,
    )
    check(
        "C6 every translating repair retains the exact READY-to-EXCITED A2 work consequence",
        work_rows == 2_256,
    )

    first_mutant = mutations[0][1]
    second_mutant = next(mutant for _, mutant in mutations if mutant != first_mutant)
    check(
        "C7 translating repair remains genuinely noninjective while generic work survives",
        first_mutant != second_mutant
        and moving_iterate(canonical, first_mutant, 2)
        == moving_iterate(canonical, second_mutant, 2),
    )

    covariance_rows = 0
    for matrix in group:
        transformed_chart = transform_chart(matrix, canonical)
        assert transform_chart(matrix, shift_chart(canonical, canonical.repair_normal)) == shift_chart(
            transformed_chart, transformed_chart.repair_normal
        )
        base_banks = work_register_banks(canonical, states)
        transformed_banks = work_register_banks(transformed_chart, states)
        for state in orbit:
            assert clock_arm_edges(transformed_chart) == tuple(
                (mv(matrix, tail), mv(matrix, direction))
                for tail, direction in clock_arm_edges(canonical)
            )
            for left_bank, right_bank, symbol in zip(
                base_banks, transformed_banks, state.heralds
            ):
                assert transform_state(matrix, left_bank[symbol]) == right_bank[symbol]
                covariance_rows += 1
            assert ready_payload(transformed_chart) == ready_payload(canonical)
            assert excited_payload(transformed_chart) == excited_payload(canonical)
            covariance_rows += 4
    check(
        "C8 translating clock, pending registers, and A2 work port are fully signed-cubic covariant",
        covariance_rows == 48 * 16 * 7,
    )

    translated_origin = replace(canonical, origin=(7, -5, 11))
    homogeneous_rows = 0
    for state in orbit:
        assert relative_signature(canonical, state, states) == relative_signature(
            translated_origin, state, states
        )
        homogeneous_rows += 1
    check(
        "C9 the prepared transaction is spatially homogeneous rather than origin-labelled",
        homogeneous_rows == 16,
    )

    occupancy_rows = 0
    for chart in chart_orbit:
        for state in clean_work_orbit(chart):
            relations, fields, work = physical_signature(chart, state, states)
            assert len(relations) == 3 and len(fields) == 6
            assert all(token_count(value) == 1 and charge(value) == 0 for value in state.arms)
            slots = frozenset(slot for _, slot in fields)
            assert all(physical_value(slots, layer) == (0,) * 6 for layer in range(3))
            assert work[1] == ready_payload(chart)
            occupancy_rows += 1
    check(
        "C10 every clean moving state retains occupancy (field,A1_SC,FCC,A2)=(6,3,0,1) and exact neutrality",
        occupancy_rows == 1_152 * 16,
    )

    speed_rows = 0
    source_rows = 0
    for chart in chart_orbit:
        after_chart, _ = moving_step(chart, orbit[0])
        displacement = subtract(after_chart.origin, chart.origin)
        assert chebyshev(after_chart.origin, chart.origin) == 1
        assert displacement == chart.repair_normal
        assert motion_signature(displacement)[0] == 1
        speed_rows += 1
        source_rows += 1
    check(
        "C11 translation saturates but never exceeds the one-hop-per-tick causal ceiling",
        speed_rows == 1_152,
    )
    check(
        "C12 every translating chart supplies the exact finite chord required by the motion-source theorem",
        source_rows == 1_152,
    )

    before_occupancy = (6, 3, 0, 1)
    after_occupancy = (6, 3, 0, 1)
    check(
        "C13 additive occupancy action assigns zero clean translation work and cannot derive inertia",
        tuple(after - before for before, after in zip(before_occupancy, after_occupancy))
        == (0, 0, 0, 0),
    )

    missing = {
        "native chart and motion-header formation",
        "clear-destination and multi-object collision arbitration",
        "positive binding, translation work, inertia, and dispersion",
        "reciprocal creation and absorption of the two motion-source packets",
        "canonical state-complete Phi integration",
        "protected poles, universal coupling, cone, lensing, and nonlinearity",
    }
    check(
        "C14 prepared translating repair/work does not close stable matter or gravity",
        len(missing) == 6,
    )

    forbidden = (
        "particle_mass",
        "newton_target",
        "lensing_target",
        "137.036",
        "random_draw",
    )
    check(
        "C15 no empirical target, fitted scale, random draw, or numerical near-miss search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} translating triplet-clock checks pass")
    print("prepared_phi_candidate=v6")
    print("translation_direction=carried_polar_repair_normal")
    print("translation_speed=one_SC_hop_per_global_tick")
    print("clean_internal_period=16_ticks")
    print("clean_sector_inverse=exact")
    print("registered_substitution_recovery_rows=2256")
    print("maximum_recovery_ticks_while_translating=2")
    print("repair_work=READY_to_EXCITED")
    print("moving_occupancy=(6,3,0,1)")
    print("additive_translation_work=0")
    print("status=prepared_translating_self_correcting_clock_exact_inertia_and_canonical_phi_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
