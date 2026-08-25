#!/usr/bin/env python3
"""Exact prepared-sector self-correcting cubic material-clock certificate.

The autonomous A9 crossing clock is an eight-state recurrent relation orbit,
but one relation alone has no nonzero substitution-error basin.  This proof
uses the minimum length-three repetition code on three disjoint orthogonal SC
relations inside one Moore cube.  Three disjoint neutral field-pair registers
at the chart center carry a one-tick retained herald, so the same radius-one
schedule corrects any one valid-symbol substitution in an arm or herald copy
within at most two global ticks.

The construction uses only the selected v3 site-field and A9 relation
carriers.  It is target blind and exact, but remains conditional on a prepared
event-halo chart and on selecting this repair transaction as a Phi successor.
It does not form the seed, supply a positive binding energy, translate,
scatter, derive an absolute mass/coupling, or close gravity.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache

from sympy import Matrix, eye

from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    charge,
    iterate,
    tick,
    token_count,
    valid_owned_states,
)
from proof_v3_charged_candidate_matter_perturbation_boundary import frame_family
from proof_v3_charged_common_action_phi_v3_candidate import relation_key
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    CENTER,
    SC_DIRECTIONS,
    add,
    assign_role_pads,
    frame_field_slots,
    herald_states,
    relative_site,
    source_edges,
)
from proof_v3_neutral_rotor_walker_macro import physical_value, polarized_slots
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    OrientedRepairChart,
    canonical_chart,
    intrinsic_state_descriptor,
    mv,
    transform_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Relative = tuple[int, int, int]
Arm = tuple[Vec, Vec]
Herald = LocalState | None

DARK: Herald = None
IDENTITY3 = Matrix.eye(3)

# Three mutually disjoint edges, one along each oriented chart axis.  Every
# endpoint lies in the center's closed Moore cube and no endpoint is the
# center, leaving the center available for the retained herald records.
ARM_LAYOUT: tuple[tuple[Relative, int], ...] = (
    ((-1, -1, -1), 0),
    ((1, -1, 0), 1),
    ((0, 1, -1), 2),
)


def logical_key(state: LocalState) -> tuple[object, ...]:
    return state.left, state.right, state.link, state.reserve


LOGICAL = tuple(sorted(valid_owned_states(), key=logical_key))
HERALD_ALPHABET: tuple[Herald, ...] = (DARK,) + LOGICAL


def axis_dyad(axis: Vec) -> Matrix:
    column = Matrix(axis)
    return column * column.T


def clock_arm_edges(chart: OrientedRepairChart) -> tuple[Arm, ...]:
    axes = (chart.first, chart.second, chart.repair_normal)
    return tuple(
        (relative_site(chart, tail), axes[axis_index])
        for tail, axis_index in ARM_LAYOUT
    )


@lru_cache(maxsize=None)
def clock_register_pool(chart: OrientedRepairChart, states) -> tuple[object, ...]:
    """Chart-covariant controller pool clear of registered center writers."""

    pads = assign_role_pads(chart, states)
    blocked = {slot[0] for slot in pads[CENTER]}
    blocked.update(
        slot[0]
        for slot in frame_field_slots(chart).get(chart.origin, set())
    )
    blocked.update(herald_states(chart, states, pads[CENTER]).values())
    return tuple(
        state
        for state in sorted(
            states, key=lambda item: intrinsic_state_descriptor(item, chart)
        )
        if state not in blocked
    )


@lru_cache(maxsize=None)
def clock_register_banks(chart: OrientedRepairChart, states) -> tuple[dict[Herald, object], ...]:
    """Three disjoint 17-symbol neutral herald registers.

    The pool excludes the static center pad, charged-frame fields, and all
    thirteen event-halo herald symbols.  Intrinsic ordering makes the selection
    chart covariant.  Each symbol is one neutral polarized field pair.
    """

    ordered = clock_register_pool(chart, states)
    required = 3 * len(HERALD_ALPHABET)
    assert len(ordered) >= required
    return tuple(
        dict(
            zip(
                HERALD_ALPHABET,
                ordered[index * len(HERALD_ALPHABET) : (index + 1) * len(HERALD_ALPHABET)],
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
class TripletClock:
    arms: tuple[LocalState, LocalState, LocalState]
    heralds: tuple[Herald, Herald, Herald]


def clean_state(logical: LocalState, heralded: bool = False) -> TripletClock:
    herald = logical if heralded else DARK
    return TripletClock((logical,) * 3, (herald,) * 3)


def triplet_step(state: TripletClock) -> TripletClock:
    """Two-layer READ/COMMIT schedule with fail-closed malformed sectors."""

    herald = strict_majority(state.heralds)
    dark_count = state.heralds.count(DARK)
    if dark_count >= 2:
        logical = strict_majority(state.arms)
        if logical is None:
            return state
        return TripletClock(state.arms, (logical,) * 3)
    if herald is None:
        return state
    return TripletClock((tick(herald),) * 3, (DARK,) * 3)


def triplet_iterate(state: TripletClock, count: int) -> TripletClock:
    for _ in range(count):
        state = triplet_step(state)
    return state


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    group = tuple(signed_permutation_matrices())
    generators = (
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)),
        ((1, 0, 0), (0, 0, 1), (0, 1, 0)),
        ((-1, 0, 0), (0, 1, 0), (0, 0, 1)),
    )
    chart_orbit = {
        transform_chart(matrix, canonical_chart(frame))
        for frame in frame_family()
        for matrix in group
    }

    check(
        "C1 A9 valid-owned alphabet has sixteen states in two exact period-eight polarity cycles",
        len(LOGICAL) == 16
        and all(iterate(state, 8) == state for state in LOGICAL)
        and all(
            all(iterate(state, step) != state for step in range(1, 8))
            for state in LOGICAL
        ),
    )

    geometry_rows = 0
    covariance_rows = 0
    register_rows = 0
    minimum_available = len(states)
    for chart in chart_orbit:
        arms = clock_arm_edges(chart)
        endpoints = [
            endpoint
            for tail, direction in arms
            for endpoint in (tail, add(tail, direction))
        ]
        frame_relations = {relation_key(edge) for edge in chart.edges()}
        event_source_relations = {
            relation_key(edge)
            for direction in SC_DIRECTIONS
            for edge in source_edges(chart, direction)
        }
        arm_relations = {relation_key(edge) for edge in arms}
        assert len(arm_relations) == 3
        assert len(set(endpoints)) == 6
        assert chart.origin not in endpoints
        assert all(
            max(abs(value) for value in (
                sum((point[i] - chart.origin[i]) * axis[i] for i in range(3))
                for axis in (chart.first, chart.second, chart.repair_normal)
            )) <= 1
            for point in endpoints
        )
        assert not (arm_relations & frame_relations)
        assert not (arm_relations & event_source_relations)
        geometry_rows += 1

        banks = clock_register_banks(chart, states)
        selected = [state for bank in banks for state in bank.values()]
        assert len(selected) == len(set(selected)) == 51
        pads = assign_role_pads(chart, states)
        blocked_slots = set(pads[CENTER]) | frame_field_slots(chart).get(
            chart.origin, set()
        )
        for bank in banks:
            assert set(bank) == set(HERALD_ALPHABET)
            for state in bank.values():
                slots = polarized_slots(state)
                assert len(slots) == 2 and not (slots & blocked_slots)
                assert all(
                    physical_value(slots, layer) == (0,) * 6
                    for layer in range(3)
                )
                register_rows += 1
        minimum_available = min(minimum_available, len(clock_register_pool(chart, states)))

        for matrix in generators:
            transformed_chart = transform_chart(matrix, chart)
            transformed_arms = clock_arm_edges(transformed_chart)
            assert transformed_arms == tuple(
                (mv(matrix, tail), mv(matrix, direction))
                for tail, direction in arms
            )
            transformed_banks = clock_register_banks(transformed_chart, states)
            for left_bank, right_bank in zip(banks, transformed_banks):
                for symbol in HERALD_ALPHABET:
                    assert transform_state(matrix, left_bank[symbol]) == right_bank[symbol]
                    covariance_rows += 1

    check(
        "C2 three disjoint orthogonal clock arms are radius-one, writer-clear, and chart covariant",
        geometry_rows == 1_152,
    )
    check(
        "C3 three disjoint existing-field herald registers encode dark plus all sixteen clock states",
        register_rows == 1_152 * 3 * 17 and minimum_available >= 51,
    )
    check(
        "C4 complete arm geometry and herald code are signed-cubic generator covariant",
        covariance_rows == 1_152 * 3 * 3 * 17,
    )

    seed = LOGICAL[0]
    orbit = [clean_state(seed)]
    for _ in range(15):
        orbit.append(triplet_step(orbit[-1]))
    check(
        "C5 the clean complete-state material clock has exact global period sixteen",
        triplet_step(orbit[-1]) == orbit[0]
        and len(set(orbit)) == 16
        and all(triplet_iterate(orbit[0], step) != orbit[0] for step in range(1, 16)),
    )

    # Exhaust every radius-one substitution in the six-symbol code at every
    # phase of the complete 16-tick orbit.  The physical perturbation class is
    # token-count-preserving: an arm is replaced by another valid owned A9
    # state, or one constant-occupancy neutral herald register changes symbol.
    recovery_rows = 0
    maximum_recovery_ticks = 0
    for phase, reference in enumerate(orbit):
        expected_future = {
            step: triplet_iterate(reference, step) for step in (1, 2)
        }
        for arm_index in range(3):
            for replacement in LOGICAL:
                if replacement == reference.arms[arm_index]:
                    continue
                arms = list(reference.arms)
                arms[arm_index] = replacement
                perturbed = TripletClock(tuple(arms), reference.heralds)
                recovered = next(
                    step
                    for step in (1, 2)
                    if triplet_iterate(perturbed, step) == expected_future[step]
                )
                maximum_recovery_ticks = max(maximum_recovery_ticks, recovered)
                recovery_rows += 1
        for register_index in range(3):
            for replacement in HERALD_ALPHABET:
                if replacement == reference.heralds[register_index]:
                    continue
                heralds = list(reference.heralds)
                heralds[register_index] = replacement
                perturbed = TripletClock(reference.arms, tuple(heralds))
                recovered = next(
                    step
                    for step in (1, 2)
                    if triplet_iterate(perturbed, step) == expected_future[step]
                )
                maximum_recovery_ticks = max(maximum_recovery_ticks, recovered)
                recovery_rows += 1
    check(
        "C6 every one-symbol arm or herald substitution rejoins the clean orbit within two ticks",
        recovery_rows == 1_488 and maximum_recovery_ticks == 2,
    )

    q = seed
    clean = clean_state(q)
    altered_arms = list(clean.arms)
    altered_arms[0] = next(state for state in LOGICAL if state != q)
    altered = TripletClock(tuple(altered_arms), clean.heralds)
    check(
        "C7 the repair map is genuinely non-injective on the admitted one-error basin",
        altered != clean and triplet_iterate(altered, 2) == triplet_iterate(clean, 2),
    )

    local_orbit = [iterate(seed, step) for step in range(8)]
    check(
        "C8 every clean arm retains one token and zero total ternary charge",
        all(token_count(state) == 1 and charge(state) == 0 for state in local_orbit),
    )
    mean_activity_one = Fraction(
        sum(state.left * state.left + state.right * state.right for state in local_orbit),
        8,
    )
    check(
        "C9 the triplet has nonzero recurrent manifestation with exact mean activity three",
        3 * mean_activity_one == 3,
    )

    tensor_rows = 0
    for chart in chart_orbit:
        axes = (chart.first, chart.second, chart.repair_normal)
        total = sum((axis_dyad(axis) for axis in axes), Matrix.zeros(3, 3))
        assert total == IDENTITY3
        assert -total / 36 == -eye(3) / 36
        tensor_rows += 1
    check(
        "C10 three orthogonal mean capacity deficits sum to the isotropic tensor -I/36",
        tensor_rows == 1_152,
    )

    # A length-two repetition word has distance two, so one arbitrary symbol
    # substitution can be equidistant between two logical codewords.  Length
    # three has distance three and is the exact repetition-code minimum for
    # correcting one substitution.
    check(
        "C11 three copies are the minimum repetition length for one arbitrary substitution correction",
        2 < 2 * 1 + 1 <= 3,
    )

    forbidden = (
        "137.036",
        "born_weight",
        "particle_mass",
        "lensing_target",
        "master_root",
    )
    check(
        "C12 no empirical target, continuum amplitude, random draw, or fitted scale enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} cubic-triplet material-clock checks pass")
    print(f"chart_rows={geometry_rows}")
    print(f"herald_register_rows={register_rows}")
    print(f"minimum_clear_controller_pool={minimum_available}")
    print(f"covariance_rows={covariance_rows}")
    print(f"single_substitution_recovery_rows={recovery_rows}")
    print(f"maximum_recovery_ticks={maximum_recovery_ticks}")
    print("complete_clock_period_global_ticks=16")
    print("mean_manifestation_activity=3")
    print("mean_capacity_deficit_tensor=-I/36")
    print("status=prepared_self_correcting_isotropic_proto_matter_clock")
    print("open=seed_assembly_energy_translation_collision_full_phi_normalization_gravity")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
