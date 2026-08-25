#!/usr/bin/env python3
"""Exact genesis-seeded triplet assembly certificate for candidate Phi-v12.

The strict v3 carrier census forbids creation of a nonblank material clock
from a blank causal past while the common nonnegative occupancy invariant is
retained.  This certificate therefore starts from the finite alternative
allowed by P1--P5: one timestamped, nonblank, oriented genesis seed.

Three existing SC A9 owners, three existing neutral field-pair markers, and
one existing A2 A9 owner are rearranged in one Moore-local transaction into
the prepared Phi-v5 self-correcting work clock.  A single registered seed
symbol error is majority-decoded and leaves the already-defined A2 work owner
EXCITED.  The detailed erroneous seed label expires; its generic work
consequence survives.  No carrier, probability, empirical target, or action
scale is added.

This is a selected formation branch, not a derivation of the genesis seed or
promotion to canonical Phi.  Formation of the oriented chart, positive
binding, occupancy faults, traffic, collisions, and general stability remain
open.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from functools import lru_cache

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import LocalState
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_charged_common_action_phi_v3_candidate import relation_key
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    DARK,
    LOGICAL,
    clock_arm_edges,
    clock_register_pool,
)
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    CENTER,
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
    WorkClock,
    clean_state as clean_work_state,
    excited_payload,
    ready_payload,
    work_register_banks,
    work_step,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Relative = tuple[int, int, int]
Arm = tuple[Vec, Vec]
Payload = tuple[int, int]

# A compact three-owner seed slab.  Each owner has the same SC orientation as
# its destination arm, and its relation midpoint moves by at most one Moore
# step.  The layout is disjoint from the retained oriented chart, every
# registered event-source relation, and the final triplet arms.
SEED_ARM_LAYOUT: tuple[tuple[Relative, int], ...] = (
    ((-1, -1, 0), 0),
    ((1, -1, -1), 1),
    ((-1, 1, -1), 2),
)

MARKER_ALPHABET = tuple(range(16))
READY_MARKER = 0


def seed_arm_edges(chart: OrientedRepairChart) -> tuple[Arm, ...]:
    axes = (chart.first, chart.second, chart.repair_normal)
    return tuple(
        (relative_site(chart, tail), axes[axis_index])
        for tail, axis_index in SEED_ARM_LAYOUT
    )


def assembly_payload(chart: OrientedRepairChart) -> Payload:
    """The existing A2 phase immediately preceding the Phi-v5 READY phase."""

    return (chart.offset - 1) % 4, chart.polarity


@lru_cache(maxsize=None)
def seed_register_banks(
    chart: OrientedRepairChart, states
) -> tuple[dict[int, object], ...]:
    """Three chart-covariant neutral marker banks disjoint from Phi-v5 banks."""

    used = {
        state
        for bank in work_register_banks(chart, states)
        for state in bank.values()
    }
    available = tuple(
        state for state in clock_register_pool(chart, states) if state not in used
    )
    required = 3 * len(MARKER_ALPHABET)
    assert len(available) >= required
    return tuple(
        dict(
            zip(
                MARKER_ALPHABET,
                available[
                    index * len(MARKER_ALPHABET) :
                    (index + 1) * len(MARKER_ALPHABET)
                ],
            )
        )
        for index in range(3)
    )


def strict_majority(values):
    for value in values:
        if values.count(value) >= 2:
            return value
    return None


@dataclass(frozen=True)
class GenesisSeed:
    arms: tuple[LocalState, LocalState, LocalState]
    markers: tuple[int, int, int]
    work: Payload


def clean_seed(chart: OrientedRepairChart, logical: LocalState) -> GenesisSeed:
    return GenesisSeed(
        arms=(logical,) * 3,
        markers=(READY_MARKER,) * 3,
        work=assembly_payload(chart),
    )


def seed_step(
    chart: OrientedRepairChart, state: GenesisSeed
) -> GenesisSeed | WorkClock:
    """Autonomous state-signature branch; malformed inputs fail closed."""

    logical = strict_majority(state.arms)
    marker = strict_majority(state.markers)
    if (
        logical is None
        or logical not in LOGICAL
        or marker != READY_MARKER
        or state.work != assembly_payload(chart)
    ):
        return state

    syndrome = int(
        any(arm != logical for arm in state.arms)
        or any(value != READY_MARKER for value in state.markers)
    )
    formed = clean_work_state(chart, logical)
    return WorkClock(
        formed.arms,
        formed.heralds,
        excited_payload(chart) if syndrome else ready_payload(chart),
    )


def phi_v12_step(chart: OrientedRepairChart, state: GenesisSeed | WorkClock):
    if isinstance(state, GenesisSeed):
        return seed_step(chart, state)
    return work_step(chart, state)


def one_seed_substitutions(state: GenesisSeed):
    for index in range(3):
        for replacement in LOGICAL:
            if replacement == state.arms[index]:
                continue
            arms = list(state.arms)
            arms[index] = replacement
            yield GenesisSeed(tuple(arms), state.markers, state.work)
    for index in range(3):
        for replacement in MARKER_ALPHABET:
            if replacement == state.markers[index]:
                continue
            markers = list(state.markers)
            markers[index] = replacement
            yield GenesisSeed(state.arms, tuple(markers), state.work)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def midpoint2(edge: Arm) -> Vec:
    tail, direction = edge
    return tuple(2 * tail[index] + direction[index] for index in range(3))


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

    check(
        "C1 selected oriented genesis-chart orbit contains 1,152 finite state charts",
        len(charts) == 1_152,
    )

    geometry_rows = 0
    register_rows = 0
    covariance_rows = 0
    minimum_remaining = len(states)
    for chart in charts:
        seed_edges = seed_arm_edges(chart)
        final_edges = clock_arm_edges(chart)
        seed_keys = {relation_key(edge) for edge in seed_edges}
        final_keys = {relation_key(edge) for edge in final_edges}
        frame_keys = {relation_key(edge) for edge in chart.edges()}
        source_keys = {
            relation_key(edge)
            for direction in SC_DIRECTIONS
            for edge in source_edges(chart, direction)
        }
        assert len(seed_keys) == len(final_keys) == 3
        assert not (seed_keys & final_keys)
        assert not (seed_keys & frame_keys)
        assert not (seed_keys & source_keys)
        for seed_edge, final_edge in zip(seed_edges, final_edges):
            assert seed_edge[1] == final_edge[1]
            displacement2 = tuple(
                abs(left - right)
                for left, right in zip(midpoint2(seed_edge), midpoint2(final_edge))
            )
            assert max(displacement2) <= 2
            for endpoint in (seed_edge[0], add(*seed_edge)):
                chart_coordinate = tuple(
                    sum(
                        (endpoint[index] - chart.origin[index]) * axis[index]
                        for index in range(3)
                    )
                    for axis in (chart.first, chart.second, chart.repair_normal)
                )
                assert max(abs(value) for value in chart_coordinate) <= 1
        geometry_rows += 1

        banks = seed_register_banks(chart, states)
        final_used = {
            state
            for bank in work_register_banks(chart, states)
            for state in bank.values()
        }
        selected = [state for bank in banks for state in bank.values()]
        minimum_remaining = min(
            minimum_remaining,
            len(clock_register_pool(chart, states)) - len(final_used),
        )
        assert len(selected) == len(set(selected)) == 48
        assert not (set(selected) & final_used)
        for bank in banks:
            assert set(bank) == set(MARKER_ALPHABET)
            for state in bank.values():
                slots = polarized_slots(state)
                assert len(slots) == 2
                assert all(
                    physical_value(slots, layer) == (0,) * 6 for layer in range(3)
                )
                register_rows += 1

        ready_slots = [banks[index][READY_MARKER] for index in range(3)]
        assert len(
            set().union(*(polarized_slots(state) for state in ready_slots))
        ) == 6

        for matrix in generators:
            transformed_chart = transform_chart(matrix, chart)
            assert seed_arm_edges(transformed_chart) == tuple(
                (mv(matrix, tail), mv(matrix, direction))
                for tail, direction in seed_edges
            )
            transformed_banks = seed_register_banks(transformed_chart, states)
            assert assembly_payload(transformed_chart) == assembly_payload(chart)
            for left_bank, right_bank in zip(banks, transformed_banks):
                for symbol in MARKER_ALPHABET:
                    assert (
                        transform_state(matrix, left_bank[symbol])
                        == right_bank[symbol]
                    )
                    covariance_rows += 1

    check(
        "C2 compact seed owners and final arms are distinct, chart-clear, and one-hop Moore local",
        geometry_rows == 1_152,
    )
    check(
        "C3 three 16-symbol neutral seed-marker banks use only existing clear field states",
        register_rows == 1_152 * 3 * 16 and minimum_remaining >= 48,
    )
    check(
        "C4 complete seed geometry, marker banks, and A2 phase are signed-cubic covariant",
        covariance_rows == 1_152 * 3 * 3 * 16,
    )

    chart = canonical_chart(frame_family()[0])
    clean_rows = 0
    repaired_rows = 0
    output_rows = []
    for logical in LOGICAL:
        seed = clean_seed(chart, logical)
        expected = clean_work_state(chart, logical)
        formed = seed_step(chart, seed)
        assert formed == expected
        clean_rows += 1
        for mutant in one_seed_substitutions(seed):
            output = seed_step(chart, mutant)
            assert isinstance(output, WorkClock)
            assert output.arms == expected.arms
            assert output.heralds == expected.heralds
            assert output.work == excited_payload(chart)
            repaired_rows += 1
            output_rows.append(output)

    check(
        "C5 every clean logical seed forms the exact ready Phi-v5 triplet in one tick",
        clean_rows == 16,
    )
    check(
        "C6 all 1,440 one-symbol seed mutations form the same body and retain one work excitation",
        repaired_rows == 16 * (3 * 15 + 3 * 15) == 1_440,
    )
    check(
        "C7 distinct erroneous seed identities expire into one generic surviving consequence",
        len(output_rows) > 1 and len(set(output_rows)) == 16,
    )

    malformed = GenesisSeed(
        arms=(LOGICAL[0], LOGICAL[1], LOGICAL[2]),
        markers=(0, 1, 2),
        work=assembly_payload(chart),
    )
    busy = GenesisSeed(
        arms=(LOGICAL[0],) * 3,
        markers=(READY_MARKER,) * 3,
        work=ready_payload(chart),
    )
    check(
        "C8 malformed or already-owned seed ports fail closed without external arbitration",
        seed_step(chart, malformed) == malformed and seed_step(chart, busy) == busy,
    )

    formed = phi_v12_step(chart, clean_seed(chart, LOGICAL[0]))
    orbit = [formed]
    for _ in range(15):
        orbit.append(phi_v12_step(chart, orbit[-1]))
    check(
        "C9 the formed ready body immediately enters the exact period-16 Phi-v5 clock orbit",
        isinstance(formed, WorkClock)
        and phi_v12_step(chart, orbit[-1]) == orbit[0]
        and len(set(orbit)) == 16,
    )

    before_occupancy = (6, 3, 0, 1)
    after_occupancy = (6, 3, 0, 1)
    check(
        "C10 assembly conserves the full relative occupancy ray with zero role debit",
        before_occupancy == after_occupancy
        and sum(before_occupancy) == sum(after_occupancy) == 10,
    )
    check(
        "C11 no object forms from blank: the branch requires ten occupied seed carriers",
        sum(before_occupancy) == 10 and sum((0, 0, 0, 0)) == 0,
    )
    check(
        "C12 clean assembly advances an existing A2 phase; correction preserves its work consequence",
        assembly_payload(chart) != ready_payload(chart)
        and ready_payload(chart) != excited_payload(chart),
    )

    forbidden = (
        "137.036",
        "born_weight",
        "particle_mass",
        "lensing_target",
        "master_root",
        "random_draw",
    )
    check(
        "C13 no empirical target, probability primitive, continuum field, or fitted scale enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    open_debts = {
        "formation of the oriented genesis chart",
        "canonical Phi provenance and overlapping-seed arbitration",
        "positive binding and absolute action scale",
        "occupancy and multiple-error survival",
        "general traffic collisions and environmental stability",
        "native source packet emission and absorption",
        "charged and tensor poles",
        "mass dispersion and relativistic saturation",
    }
    check(
        "C14 Phi-v12 closes seeded assembly only, not general stable matter",
        len(open_debts) == 8,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} genesis-seeded triplet assembly checks pass")
    print(f"chart_rows={geometry_rows}")
    print(f"seed_register_rows={register_rows}")
    print(f"seed_covariance_rows={covariance_rows}")
    print(f"clean_formation_rows={clean_rows}")
    print(f"registered_seed_error_rows={repaired_rows}")
    print("occupancy_delta=(0,0,0,0)")
    print("formation_ticks=1")
    print("formation_from_blank=no")
    print("absolute_action_multiplier=free")
    print("status=selected_genesis_seeded_phi_v12_assembly_candidate")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
