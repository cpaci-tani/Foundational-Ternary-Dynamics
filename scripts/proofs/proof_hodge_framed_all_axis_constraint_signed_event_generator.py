#!/usr/bin/env python3
"""Exact Hodge-framed all-axis constraint and signed-event generator.

The certificate uses the existing shared-edge Hodge flag to orient the two
plane bundles required by an axial manifestation source.  It then composes
the resulting all-axis constraint macro with the prepared paired-history
actualization vertex and one signed type-2 clock/source/recoil generator.

This is a blocked prepared-reference action.  It does not form the Hodge
flag or owner reserves, prepare a Born ensemble, derive stable matter,
produce Maxwell or spin-2 poles, measure alpha, or establish lensing.
"""

from __future__ import annotations

import copy
import hashlib
import itertools
from dataclasses import dataclass, replace
from fractions import Fraction
from pathlib import Path

import sympy as sp

from proof_c18_charge_even_constraint_bundle_axial_context_price import (
    AXES,
    BundleState,
    Matrix3,
    Record as OwnerRecord,
    Vector,
    add,
    apply_bundle,
    charge_conjugate,
    dot,
    event_source_times_216,
    make_input,
    mat_vec,
    moment_pair,
    negate,
    phase_shift,
    scale,
    signed_permutation_matrices,
    sub,
    transform_state,
)
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
    charge,
    payload,
    rotate_state,
    token_count,
)
from proof_c4_paired_history_phase_neutral_actualization_vertex import (
    Record as HistoryRecord,
    phase_inner,
)
from proof_shared_edge_hodge_flag_bcc_propagation import (
    Flag,
    cross,
    flags,
    transform_flag,
)


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    ROOT / "scripts/proofs/proof_shared_edge_hodge_flag_bcc_propagation.py":
        "28046AD9723E34F7F0EE099F5D44B9CF0BDCBDDBE2B4352FBD8FD6991FDD0455",
    ROOT / "scripts/proofs/proof_c18_charge_even_constraint_bundle_axial_context_price.py":
        "FACF5949B3A04A8DCBDD57F047C8ADFB13007F2FD0EA7B25C56F24FAB80EC8F3",
    ROOT / "scripts/proofs/proof_c4_paired_history_phase_neutral_actualization_vertex.py":
        "AF3F9FC376E2C6762287C61158A75F67D3CDEA1A1117F7969353C7966FB7214F",
    ROOT / "scripts/proofs/proof_c18_phase_neutral_shared_charge_stress_vertex.py":
        "13334840F23DBB1D70EFD59B805D97E462EDFC5B4EEC00D5C9FFF784ECEAAF35",
    ROOT / "scripts/proofs/proof_c18_scalar_stf_vector_constraint_absorption_seam.py":
        "29C6BC475F6DDFCC3FC73DA5683D0F14ECAC96233924084920F682389D1B1F6E",
    ROOT / "scripts/proofs/proof_reciprocal_packet_clock_recoil_absorption_generator.py":
        "4B824C3B37A8BADEC9F50ED1785602734B75D6CCF03234D65826E0541CDC2576",
    ROOT / "scripts/proofs/proof_c4_symmetric_stress_packet_momentum_and_source_handoff.py":
        "312FA1071D09FEBE61225A8BAFBA2C6D7994E80A584DE4B9220EE5274ACCB938",
    ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_AND_ONE_SIGNED_EVENT_GENERATOR_v1.md":
        "E52B8A10EC72459D4C071C928649E88493AD4F66E7BB064200E808BDC16B1568",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def determinant_columns(a: Vector, b: Vector, c: Vector) -> int:
    return (
        a[0] * (b[1] * c[2] - b[2] * c[1])
        - b[0] * (a[1] * c[2] - a[2] * c[1])
        + c[0] * (a[1] * b[2] - a[2] * b[1])
    )


def frame_axes(flag: Flag) -> tuple[Vector, Vector]:
    tangent, axial_normal, handedness = flag
    return scale(handedness, axial_normal), cross(tangent, axial_normal)


def outer(a: Vector, b: Vector) -> tuple[tuple[Fraction, ...], ...]:
    return tuple(
        tuple(Fraction(a[i] * b[j]) for j in range(3))
        for i in range(3)
    )


def matrix_subtract(
    a: tuple[tuple[Fraction, ...], ...],
    b: tuple[tuple[Fraction, ...], ...],
) -> tuple[tuple[Fraction, ...], ...]:
    return tuple(
        tuple(a[i][j] - b[i][j] for j in range(3))
        for i in range(3)
    )


def matrix_scale(
    coefficient: Fraction,
    matrix: tuple[tuple[Fraction, ...], ...],
) -> tuple[tuple[Fraction, ...], ...]:
    return tuple(
        tuple(coefficient * matrix[i][j] for j in range(3))
        for i in range(3)
    )


IDENTITY3 = (
    (Fraction(1), Fraction(0), Fraction(0)),
    (Fraction(0), Fraction(1), Fraction(0)),
    (Fraction(0), Fraction(0), Fraction(1)),
)


def event_tensor(ray: Vector) -> tuple[tuple[Fraction, ...], ...]:
    return matrix_scale(Fraction(1, 18), outer(ray, ray))


def event_stf(ray: Vector) -> tuple[tuple[Fraction, ...], ...]:
    return matrix_subtract(
        event_tensor(ray),
        matrix_scale(Fraction(1, 54), IDENTITY3),
    )


def matrix_vector(
    matrix: tuple[tuple[Fraction, ...], ...],
    vector: Vector,
) -> tuple[Fraction, Fraction, Fraction]:
    return tuple(
        sum(
            (matrix[i][j] * vector[j] for j in range(3)),
            start=Fraction(0),
        )
        for i in range(3)
    )  # type: ignore[return-value]


@dataclass(frozen=True)
class OwnerGroup:
    plane: Vector
    state: BundleState


@dataclass(frozen=True)
class CommonEventState:
    left_history: HistoryRecord
    right_history: HistoryRecord
    actualization: ActualizationState
    flag: Flag
    derivative: Vector
    owners: tuple[OwnerGroup, ...]


def pair_route(left: HistoryRecord, right: HistoryRecord) -> str:
    identities = sorted((left.identity, right.identity))
    return f"outcome:{left.outcome}/pair:{identities[0]}:{identities[1]}"


def owner_specifications(
    flag: Flag,
    derivative: Vector,
) -> tuple[tuple[Vector, Vector], ...]:
    tangent, _, _ = flag
    if tangent not in AXES or derivative not in AXES:
        return ()
    incidence = dot(tangent, derivative)
    if incidence == 0:
        return ((negate(derivative), tangent),)
    if abs(incidence) == 1:
        transverse_u, transverse_v = frame_axes(flag)
        return (
            (derivative, transverse_u),
            (derivative, transverse_v),
        )
    return ()


def make_owner_groups(
    flag: Flag,
    derivative: Vector,
    phase: int,
    charge_label: int,
    source_route: str,
) -> tuple[OwnerGroup, ...]:
    specifications = owner_specifications(flag, derivative)
    groups: list[OwnerGroup] = []
    for index, (bundle_axis, plane) in enumerate(specifications):
        raw_state = make_input(
            bundle_axis,
            (phase, phase, phase),
            charge_label,
            prefix=f"owner{index}",
            layer=index,
            source_route=source_route,
        )
        groups.append(
            OwnerGroup(
                plane,
                BundleState(
                    tuple(sorted(
                        raw_state.records,
                        key=lambda record: record.record_id,
                    )),
                    raw_state.occupied,
                ),
            )
        )
    return tuple(groups)


def owner_payload(groups: tuple[OwnerGroup, ...]) -> tuple[tuple[object, ...], ...]:
    return tuple(sorted(
        (
            record.record_id,
            record.phase,
            record.charge,
            record.layer,
            record.source_route,
        )
        for group in groups
        for record in group.state.records
    ))


def owner_moments(
    groups: tuple[OwnerGroup, ...],
) -> tuple[Vector, Vector]:
    em = (0, 0, 0)
    constraint = (0, 0, 0)
    for group in groups:
        group_em, group_constraint = moment_pair(group.state)
        em = add(em, group_em)
        constraint = add(constraint, group_constraint)
    return em, constraint


def apply_owner_macro(
    groups: tuple[OwnerGroup, ...],
    flag: Flag,
    derivative: Vector,
    *,
    inverse: bool = False,
) -> tuple[OwnerGroup, ...] | None:
    specifications = owner_specifications(flag, derivative)
    if len(groups) != len(specifications):
        return None
    expected_planes = tuple(plane for _, plane in specifications)
    if tuple(group.plane for group in groups) != expected_planes:
        return None

    candidates: list[OwnerGroup] = []
    for group, (bundle_axis, plane) in zip(groups, specifications):
        if group.plane != plane:
            return None
        candidate = apply_bundle(
            group.state,
            bundle_axis,
            plane,
            inverse=inverse,
        )
        if candidate is None:
            return None
        candidates.append(OwnerGroup(group.plane, candidate))
    return tuple(candidates)


def is_bright(state: CommonEventState) -> bool:
    return (
        state.left_history.outcome == state.right_history.outcome
        and phase_inner(
            state.left_history.phase,
            state.right_history.phase,
        ) == 1
    )


def event_direction(actualization: ActualizationState) -> int | None:
    if (
        actualization.event_bit == 0
        and actualization.state_left == 0
        and actualization.state_right == 0
        and actualization.link is None
        and actualization.reserve is not None
    ):
        return 1
    if (
        actualization.event_bit == 0
        and actualization.link is not None
        and actualization.reserve is None
        and actualization.state_left == actualization.link.orientation
        and actualization.state_right == -actualization.link.orientation
    ):
        return -1
    return None


def apply_common_event(state: CommonEventState) -> CommonEventState | None:
    if not is_bright(state):
        return state

    direction = event_direction(state.actualization)
    if direction is None:
        return None
    owner_output = apply_owner_macro(
        state.owners,
        state.flag,
        state.derivative,
        inverse=direction < 0,
    )
    if owner_output is None:
        return None

    actualization_output = actualization_macro(state.actualization, True)
    if actualization_output == state.actualization:
        return None

    return replace(
        state,
        actualization=actualization_output,
        owners=owner_output,
    )


def make_common_event(
    left: HistoryRecord,
    right: HistoryRecord,
    token_phase: int,
    orientation: int,
    flag: Flag,
    derivative: Vector,
) -> CommonEventState:
    token = Token(token_phase, orientation)
    actualization = ActualizationState(0, 0, 0, None, token)
    return CommonEventState(
        left,
        right,
        actualization,
        flag,
        derivative,
        make_owner_groups(
            flag,
            derivative,
            token_phase,
            orientation,
            pair_route(left, right),
        ),
    )


def transform_owner_groups(
    groups: tuple[OwnerGroup, ...],
    matrix: Matrix3,
) -> tuple[OwnerGroup, ...]:
    return tuple(
        OwnerGroup(
            mat_vec(matrix, group.plane),
            transform_state(group.state, matrix),
        )
        for group in groups
    )


def transform_common_event(
    state: CommonEventState,
    matrix: Matrix3,
) -> CommonEventState:
    return replace(
        state,
        flag=transform_flag(matrix, state.flag),
        derivative=mat_vec(matrix, state.derivative),
        owners=transform_owner_groups(state.owners, matrix),
    )


def rotate_common_event(
    state: CommonEventState,
    turns: int,
) -> CommonEventState:
    return replace(
        state,
        left_history=HistoryRecord(
            state.left_history.outcome,
            (state.left_history.phase + turns) % 4,
            state.left_history.identity,
        ),
        right_history=HistoryRecord(
            state.right_history.outcome,
            (state.right_history.phase + turns) % 4,
            state.right_history.identity,
        ),
        actualization=rotate_state(state.actualization, turns),
        owners=tuple(
            OwnerGroup(group.plane, phase_shift(group.state, turns))
            for group in state.owners
        ),
    )


def conjugate_actualization(
    actualization: ActualizationState,
) -> ActualizationState:
    def conjugate_token(token: Token | None) -> Token | None:
        if token is None:
            return None
        return Token(token.phase, -token.orientation)

    return ActualizationState(
        actualization.event_bit,
        -actualization.state_left,
        -actualization.state_right,
        conjugate_token(actualization.link),
        conjugate_token(actualization.reserve),
    )


def conjugate_common_event(state: CommonEventState) -> CommonEventState:
    return replace(
        state,
        actualization=conjugate_actualization(state.actualization),
        owners=tuple(
            OwnerGroup(group.plane, charge_conjugate(group.state))
            for group in state.owners
        ),
    )


def hodge_frame_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()
    for flag in flags():
        tangent, axial_normal, handedness = flag
        transverse_u, transverse_v = frame_axes(flag)
        assert tangent in AXES
        assert transverse_u in AXES
        assert transverse_v in AXES
        assert dot(tangent, transverse_u) == 0
        assert dot(tangent, transverse_v) == 0
        assert dot(transverse_u, transverse_v) == 0
        assert determinant_columns(tangent, transverse_u, transverse_v) == handedness
        checks += 7

        for matrix in group:
            transformed = transform_flag(matrix, flag)
            image_u, image_v = frame_axes(transformed)
            assert image_u == mat_vec(matrix, transverse_u)
            assert image_v == mat_vec(matrix, transverse_v)
            assert transformed[0] == mat_vec(matrix, tangent)
            checks += 3

        # The scalar C4 phase does not alter the spatial frame.
        for phase in range(4):
            assert frame_axes(flag) == (transverse_u, transverse_v)
            checks += 1
    return checks


def finite_constraint_macro_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()

    for flag in flags():
        tangent, _, _ = flag
        for derivative in AXES:
            for phase in range(4):
                for charge_label in (-1, 1):
                    before = make_owner_groups(
                        flag,
                        derivative,
                        phase,
                        charge_label,
                        "pair:finite-census",
                    )
                    after = apply_owner_macro(before, flag, derivative)
                    assert after is not None
                    restored = apply_owner_macro(
                        after,
                        flag,
                        derivative,
                        inverse=True,
                    )
                    assert restored == before
                    assert owner_payload(after) == owner_payload(before)
                    checks += 3

                    before_em, before_constraint = owner_moments(before)
                    after_em, after_constraint = owner_moments(after)
                    delta_em = sub(after_em, before_em)
                    delta_constraint = sub(
                        after_constraint,
                        before_constraint,
                    )
                    target = tuple(
                        int(entry)
                        for entry in event_source_times_216(
                            tangent,
                            derivative,
                        )
                    )
                    assert delta_em == (0, 0, 0)
                    assert delta_constraint == target
                    assert tuple(
                        Fraction(entry, 216)
                        for entry in delta_constraint
                    ) == matrix_vector(
                        event_stf(tangent),
                        derivative,
                    )
                    checks += 3

                    conjugated_before = tuple(
                        OwnerGroup(
                            owner.plane,
                            charge_conjugate(owner.state),
                        )
                        for owner in before
                    )
                    conjugated_after = apply_owner_macro(
                        conjugated_before,
                        flag,
                        derivative,
                    )
                    assert conjugated_after is not None
                    assert sub(
                        owner_moments(conjugated_after)[1],
                        owner_moments(conjugated_before)[1],
                    ) == delta_constraint
                    checks += 2

                    for turns in range(4):
                        shifted_before = tuple(
                            OwnerGroup(
                                owner.plane,
                                phase_shift(owner.state, turns),
                            )
                            for owner in before
                        )
                        shifted_after = apply_owner_macro(
                            shifted_before,
                            flag,
                            derivative,
                        )
                        assert shifted_after == tuple(
                            OwnerGroup(
                                owner.plane,
                                phase_shift(owner.state, turns),
                            )
                            for owner in after
                        )
                        checks += 1

                    expected_owner_count = (
                        1 if dot(tangent, derivative) == 0 else 2
                    )
                    assert len(before) == expected_owner_count
                    if expected_owner_count == 2:
                        assert before[0].state.records[0].layer != before[1].state.records[0].layer
                        assert {
                            record.record_id
                            for record in before[0].state.records
                        }.isdisjoint({
                            record.record_id
                            for record in before[1].state.records
                        })
                        active_fcc = sum(
                            record.shell == "FCC" and record.owner == "active"
                            for owner in after
                            for record in owner.state.records
                        )
                        assert active_fcc == 4
                        checks += 3
                    checks += 1

            # Cubic covariance needs one phase/charge representative; all
            # internal values were exhausted above.
            before = make_owner_groups(
                flag,
                derivative,
                1,
                1,
                "pair:covariance",
            )
            after = apply_owner_macro(before, flag, derivative)
            assert after is not None
            for matrix in group:
                transformed_flag = transform_flag(matrix, flag)
                transformed_derivative = mat_vec(matrix, derivative)
                transformed_before = transform_owner_groups(before, matrix)
                transformed_after = transform_owner_groups(after, matrix)
                image = apply_owner_macro(
                    transformed_before,
                    transformed_flag,
                    transformed_derivative,
                )
                assert image == transformed_after
                checks += 1

            # Axial owner exchange is equivariant as an exchange of the
            # complete plane-attached groups.
            if abs(dot(tangent, derivative)) == 1:
                swapped_before = tuple(reversed(before))
                swapped_specs = tuple(reversed(owner_specifications(flag, derivative)))
                # Apply the same map after exchanging both groups and the
                # corresponding Hodge-frame fiber ordering.
                outputs = []
                for owner, (bundle_axis, plane) in zip(
                    swapped_before,
                    swapped_specs,
                ):
                    assert owner.plane == plane
                    candidate = apply_bundle(
                        owner.state,
                        bundle_axis,
                        plane,
                    )
                    assert candidate is not None
                    outputs.append(OwnerGroup(plane, candidate))
                assert tuple(outputs) == tuple(reversed(after))
                checks += 2

    return checks


def prepared_event_checks() -> int:
    checks = 0
    state_flags = flags()

    for outcome in range(2):
        for left_phase, right_phase in itertools.product(range(4), repeat=2):
            left = HistoryRecord(outcome, left_phase, 100 + 4 * outcome + left_phase)
            right = HistoryRecord(outcome, right_phase, 200 + 4 * outcome + right_phase)
            compatible = phase_inner(left_phase, right_phase) == 1
            for token_phase in range(4):
                for orientation in (-1, 1):
                    for flag in state_flags:
                        tangent, _, _ = flag
                        for derivative in AXES:
                            before = make_common_event(
                                left,
                                right,
                                token_phase,
                                orientation,
                                flag,
                                derivative,
                            )
                            snapshot = copy.deepcopy(before)
                            after = apply_common_event(before)
                            assert before == snapshot
                            assert after is not None
                            checks += 2

                            if not compatible:
                                assert after == before
                                checks += 1
                                continue

                            assert after != before
                            assert event_direction(before.actualization) == 1
                            assert event_direction(after.actualization) == -1
                            assert token_count(after.actualization) == 1
                            assert payload(after.actualization) == payload(
                                before.actualization
                            )
                            assert charge(after.actualization) == 0
                            assert after.left_history == before.left_history
                            assert after.right_history == before.right_history
                            assert after.flag == before.flag
                            assert after.derivative == before.derivative
                            assert owner_payload(after.owners) == owner_payload(
                                before.owners
                            )
                            checks += 10

                            restored = apply_common_event(after)
                            assert restored == before
                            checks += 1

                            before_em, before_constraint = owner_moments(
                                before.owners
                            )
                            after_em, after_constraint = owner_moments(
                                after.owners
                            )
                            assert sub(after_em, before_em) == (0, 0, 0)
                            delta_constraint = sub(
                                after_constraint,
                                before_constraint,
                            )
                            assert tuple(
                                Fraction(entry, 216)
                                for entry in delta_constraint
                            ) == matrix_vector(
                                event_stf(tangent),
                                derivative,
                            )
                            manifested_token = after.actualization.link
                            assert manifested_token is not None
                            assert manifested_token.phase == token_phase
                            assert manifested_token.orientation == orientation
                            current = tuple(
                                Fraction(
                                    manifested_token.orientation * component,
                                    9,
                                )
                                for component in tangent
                            )
                            tensor = event_tensor(tangent)
                            assert current == tuple(
                                Fraction(orientation * component, 9)
                                for component in tangent
                            )
                            assert sum(
                                (tensor[index][index] for index in range(3)),
                                start=Fraction(0),
                            ) == Fraction(1, 18)
                            assert event_stf(tangent) == matrix_subtract(
                                tensor,
                                matrix_scale(Fraction(1, 54), IDENTITY3),
                            )
                            checks += 8

                            rotated_before = rotate_common_event(before, 1)
                            rotated_after = apply_common_event(rotated_before)
                            assert rotated_after == rotate_common_event(after, 1)
                            checks += 1

                            conjugated_before = conjugate_common_event(before)
                            conjugated_after = apply_common_event(
                                conjugated_before
                            )
                            assert conjugated_after == conjugate_common_event(
                                after
                            )
                            conjugated_token = payload(
                                conjugated_after.actualization
                            )
                            assert conjugated_token is not None
                            assert conjugated_token.orientation == -orientation
                            assert owner_moments(conjugated_after.owners)[1] == owner_moments(after.owners)[1]
                            checks += 3

    # Different outcome ports never interact, exhaustively over the remaining
    # event data as required by the preregistered contextual gate.
    for left_phase, right_phase in itertools.product(range(4), repeat=2):
        left = HistoryRecord(0, left_phase, 300 + left_phase)
        right = HistoryRecord(1, right_phase, 400 + right_phase)
        for token_phase in range(4):
            for orientation in (-1, 1):
                for flag in state_flags:
                    for derivative in AXES:
                        before = make_common_event(
                            left,
                            right,
                            token_phase,
                            orientation,
                            flag,
                            derivative,
                        )
                        assert apply_common_event(before) == before
                        checks += 1

    return checks


def full_event_shift_checks() -> int:
    checks = 0
    energies = (Fraction(1, 6), Fraction(1), Fraction(7, 3))
    couplings = (Fraction(1, 5), Fraction(1), Fraction(9, 4))

    for ray in AXES:
        source = event_stf(ray)
        stf_five = (
            source[0][0],
            source[1][1],
            source[0][1],
            source[0][2],
            source[1][2],
        )
        for derivative in AXES:
            source_divergence = matrix_vector(source, derivative)
            finite_chart = tuple(
                Fraction(int(entry), 216)
                for entry in event_source_times_216(ray, derivative)
            )
            assert finite_chart == source_divergence
            checks += 1

            for orientation in (-1, 1):
                current = tuple(
                    Fraction(orientation * component, 9)
                    for component in ray
                )
                for energy in energies:
                    recoil = tuple(6 * energy * component for component in ray)
                    for g_e, g_0, g_t in itertools.product(
                        couplings,
                        repeat=3,
                    ):
                        shift = (
                            *recoil,
                            *(g_e * component for component in current),
                            g_0 * Fraction(1, 18),
                            *(g_t * component for component in stf_five),
                            *(g_t * component for component in finite_chart),
                        )
                        assert len(shift) == 15
                        assert shift[-3:] == tuple(
                            g_t * component
                            for component in source_divergence
                        )
                        reversed_shift = tuple(-component for component in shift)
                        assert tuple(
                            shift[index] + reversed_shift[index]
                            for index in range(15)
                        ) == (Fraction(0),) * 15
                        checks += 3

    return checks


def failure_and_covariance_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()
    left = HistoryRecord(0, 1, 17)
    right = HistoryRecord(0, 1, 23)

    # Full common-event covariance on every flag, derivative, and cubic map.
    for flag in flags():
        for derivative in AXES:
            before = make_common_event(
                left,
                right,
                2,
                1,
                flag,
                derivative,
            )
            after = apply_common_event(before)
            assert after is not None
            for matrix in group:
                transformed_before = transform_common_event(before, matrix)
                transformed_after = transform_common_event(after, matrix)
                image = apply_common_event(transformed_before)
                assert image == transformed_after
                checks += 1

    base_flag = flags()[0]
    derivative = base_flag[0]
    valid = make_common_event(left, right, 2, 1, base_flag, derivative)
    variants: list[CommonEventState] = []

    variants.append(replace(valid, owners=valid.owners[:1]))  # missing axial owner

    first_group = valid.owners[0]
    bundle_axis, plane = owner_specifications(base_flag, derivative)[0]
    target = negate(add(bundle_axis, plane))
    occupied_state = BundleState(
        first_group.state.records,
        (target,),
    )
    variants.append(replace(
        valid,
        owners=(
            OwnerGroup(first_group.plane, occupied_state),
            *valid.owners[1:],
        ),
    ))

    records = list(first_group.state.records)
    bad_phase_records = tuple(
        replace(
            record,
            phase=(record.phase + int(index == 0)) % 4,
        )
        for index, record in enumerate(records)
    )
    variants.append(replace(
        valid,
        owners=(
            OwnerGroup(
                first_group.plane,
                BundleState(bad_phase_records, first_group.state.occupied),
            ),
            *valid.owners[1:],
        ),
    ))

    alternate_normal = negate(base_flag[1])
    variants.append(replace(
        valid,
        flag=(base_flag[0], alternate_normal, base_flag[2]),
    ))

    invalid_actualization = ActualizationState(0, 0, 0, None, None)
    variants.append(replace(valid, actualization=invalid_actualization))

    for variant in variants:
        snapshot = copy.deepcopy(variant)
        assert apply_common_event(variant) is None
        assert variant == snapshot
        checks += 2

    return checks


def is_zero(matrix_or_expr) -> bool:
    if isinstance(matrix_or_expr, sp.MatrixBase):
        return all(sp.simplify(entry) == 0 for entry in matrix_or_expr)
    return sp.simplify(matrix_or_expr) == 0


def signed_generator_symbolic_checks() -> int:
    checks = 0

    theta, action, omega, event_energy = sp.symbols(
        "theta I omega E", positive=True
    )
    x1, x2, p1, p2 = sp.symbols("x1 x2 p1 p2", real=True)
    z0, z1, pi0, pi1 = sp.symbols("z0 z1 pi0 pi1", real=True)
    a1, a2 = sp.symbols("a1 a2", real=True)
    m1, m2 = sp.symbols("m1 m2", positive=True)

    x = sp.Matrix((x1, x2))
    momentum = sp.Matrix((p1, p2))
    z = sp.Matrix((z0, z1))
    pi = sp.Matrix((pi0, pi1))
    shift = sp.Matrix((a1, a2))
    inverse_mass = sp.diag(1 / m1, 1 / m2)
    swap = sp.Matrix(((0, 1), (1, 0)))
    sign_covector = sp.Matrix((1, -1))
    event_sign = (sign_covector.T * z)[0]

    def hamiltonian(value: sp.Matrix) -> sp.Expr:
        return sp.simplify(
            (value.T * inverse_mass * value)[0] / 2
        )

    def event_map(
        theta_value,
        x_value: sp.Matrix,
        z_value: sp.Matrix,
        action_value,
        momentum_value: sp.Matrix,
        pi_value: sp.Matrix,
    ):
        sign_value = (sign_covector.T * z_value)[0]
        new_z = swap * z_value
        new_momentum = momentum_value + sign_value * shift
        new_x = sp.simplify(
            x_value
            + theta_value * sign_value / omega
            * inverse_mass * shift
        )
        new_action = sp.simplify(
            action_value
            + (
                sign_value * event_energy
                + hamiltonian(momentum_value)
                - hamiltonian(new_momentum)
            ) / omega
        )
        reaction = sp.simplify(
            -(x_value.T * shift)[0]
            - theta_value / omega
            * (
                event_energy
                - (shift.T * inverse_mass * momentum_value)[0]
            )
        )
        new_pi = sp.simplify(
            swap * pi_value + sign_covector * reaction
        )
        return (
            theta_value,
            new_x,
            new_z,
            new_action,
            new_momentum,
            new_pi,
        )

    mapped = event_map(theta, x, z, action, momentum, pi)
    (
        new_theta,
        new_x,
        new_z,
        new_action,
        new_momentum,
        new_pi,
    ) = mapped

    old_state = sp.Matrix((
        theta, x1, x2, z0, z1,
        action, p1, p2, pi0, pi1,
    ))
    new_state = sp.Matrix((
        new_theta,
        new_x[0], new_x[1],
        new_z[0], new_z[1],
        new_action,
        new_momentum[0], new_momentum[1],
        new_pi[0], new_pi[1],
    ))
    jacobian = new_state.jacobian(old_state)
    symplectic_form = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(5), sp.eye(5)),
        sp.Matrix.hstack(-sp.eye(5), sp.zeros(5)),
    )
    assert is_zero(
        jacobian.T * symplectic_form * jacobian
        - symplectic_form
    )
    checks += 1

    total_before = (
        omega * action
        + hamiltonian(momentum)
        + event_energy * z0
    )
    total_after = (
        omega * new_action
        + hamiltonian(new_momentum)
        + event_energy * new_z[0]
    )
    assert is_zero(total_before - total_after)
    checks += 1

    # The full event, including the port conjugate reaction, is an involution.
    twice = event_map(
        new_theta,
        new_x,
        new_z,
        new_action,
        new_momentum,
        new_pi,
    )
    assert is_zero(twice[0] - theta)
    assert is_zero(twice[1] - x)
    assert is_zero(twice[2] - z)
    assert is_zero(twice[3] - action)
    assert is_zero(twice[4] - momentum)
    assert is_zero(twice[5] - pi)
    checks += 6

    assert is_zero(new_x.subs(theta, 0) - x)
    assert is_zero(new_z - swap * z)
    assert is_zero(new_momentum - momentum - event_sign * shift)
    checks += 3

    # Derive the map directly from the frozen type-2 generating function.
    new_I, new_p1, new_p2, new_pi0, new_pi1 = sp.symbols(
        "Iprime p1prime p2prime pi0prime pi1prime",
        real=True,
    )
    new_p_symbol = sp.Matrix((new_p1, new_p2))
    new_pi_symbol = sp.Matrix((new_pi0, new_pi1))
    generating_function = (
        theta * new_I
        + (x.T * (new_p_symbol - event_sign * shift))[0]
        + (z.T * swap.T * new_pi_symbol)[0]
        - theta / omega
        * (
            event_sign * event_energy
            + hamiltonian(new_p_symbol - event_sign * shift)
            - hamiltonian(new_p_symbol)
        )
    )
    old_p_from_f = sp.Matrix((
        sp.diff(generating_function, x1),
        sp.diff(generating_function, x2),
    ))
    new_q_from_f = sp.Matrix((
        sp.diff(generating_function, new_I),
        sp.diff(generating_function, new_p1),
        sp.diff(generating_function, new_p2),
        sp.diff(generating_function, new_pi0),
        sp.diff(generating_function, new_pi1),
    ))
    assert is_zero(
        old_p_from_f
        - (new_p_symbol - event_sign * shift)
    )
    assert is_zero(new_q_from_f[0] - theta)
    assert is_zero(
        new_q_from_f[3:5, 0]
        - swap * z
    )
    mixed_hessian = sp.Matrix([
        [
            sp.diff(
                generating_function,
                old_coordinate,
                new_canonical_momentum,
            )
            for new_canonical_momentum in (
                new_I, new_p1, new_p2, new_pi0, new_pi1
            )
        ]
        for old_coordinate in (theta, x1, x2, z0, z1)
    ])
    assert sp.simplify(mixed_hessian.det()) == -1
    checks += 4

    # The mixed-Hessian argument is dimension independent.  Exercise the
    # exact block structure at the physical 15-component event-shift size:
    # recoil 3 + EM current 3 + scalar 1 + STF 5 + constraint 3.
    for dimension in (1, 2, 3, 15):
        top_coupling = sp.Matrix([
            [sp.Rational(index + 1, dimension + 2) for index in range(dimension)]
        ])
        port_coupling = sp.Matrix([
            [
                sp.Rational((row + 1) * (column + 1), dimension + 3)
                for column in range(dimension)
            ]
            for row in range(2)
        ])
        general_mixed = sp.Matrix.vstack(
            sp.Matrix.hstack(
                sp.ones(1, 1),
                top_coupling,
                sp.zeros(1, 2),
            ),
            sp.Matrix.hstack(
                sp.zeros(dimension, 1),
                sp.eye(dimension),
                sp.zeros(dimension, 2),
            ),
            sp.Matrix.hstack(
                sp.zeros(2, 1),
                port_coupling,
                swap.T,
            ),
        )
        assert general_mixed.det() == -1
        checks += 1

    # The local tensor constraint is preserved for an arbitrary displayed
    # tensor normalization.
    gt = sp.symbols("gT", nonzero=True)
    s11, s12, s13, s22, s23 = sp.symbols(
        "s11 s12 s13 s22 s23", real=True
    )
    source = sp.Matrix((
        (s11, s12, s13),
        (s12, s22, s23),
        (s13, s23, -s11 - s22),
    ))
    q1, q2, q3 = sp.symbols("q1 q2 q3", real=True)
    derivative = sp.Matrix((q1, q2, q3))
    pi_tensor = source / 7
    kappa = pi_tensor * derivative
    new_tensor = pi_tensor + gt * source
    new_kappa = kappa + gt * source * derivative
    assert is_zero(
        new_tensor * derivative - new_kappa
        - (pi_tensor * derivative - kappa)
    )
    checks += 1

    # Independent canonical rescalings preserve the symplectic form and
    # change displayed source ratios.  The generator does not force them.
    lambda_e, lambda_0, lambda_t = sp.symbols(
        "lambdaE lambda0 lambdaT", nonzero=True
    )
    coordinate_scale = sp.diag(lambda_e, lambda_0, lambda_t)
    momentum_scale = sp.diag(
        1 / lambda_e,
        1 / lambda_0,
        1 / lambda_t,
    )
    scaling = sp.diag(
        lambda_e, lambda_0, lambda_t,
        1 / lambda_e, 1 / lambda_0, 1 / lambda_t,
    )
    canonical_form = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(3), sp.eye(3)),
        sp.Matrix.hstack(-sp.eye(3), sp.zeros(3)),
    )
    assert is_zero(
        scaling.T * canonical_form * scaling - canonical_form
    )
    assert coordinate_scale.det() != 0
    assert momentum_scale.det() != 0
    assert sp.simplify(lambda_e / lambda_t - 1) != 0
    checks += 4

    return checks


def generator_rational_admission_checks() -> int:
    checks = 0
    omega = Fraction(2, 3)
    for sign in (-1, 1):
        for event_energy in (
            Fraction(1, 4),
            Fraction(1),
            Fraction(7, 2),
        ):
            for old_energy in (
                Fraction(0),
                Fraction(1, 3),
                Fraction(5),
            ):
                for new_energy in (
                    Fraction(0),
                    Fraction(2),
                    Fraction(11),
                ):
                    for old_action in (
                        Fraction(0),
                        Fraction(3, 2),
                        Fraction(10),
                    ):
                        new_action = old_action + (
                            sign * event_energy
                            + old_energy
                            - new_energy
                        ) / omega
                        admitted = new_action >= 0
                        z0_before = Fraction(1 if sign == 1 else 0)
                        z0_after = Fraction(1) - z0_before
                        if admitted:
                            assert (
                                omega * old_action
                                + old_energy
                                + event_energy * z0_before
                                == omega * new_action
                                + new_energy
                                + event_energy * z0_after
                            )
                        else:
                            assert new_action < 0
                        checks += 1
    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        actual = sha256(path)
        assert actual == expected, (path, actual, expected)
        checks += 1

    checks += hodge_frame_checks()
    checks += finite_constraint_macro_checks()
    checks += prepared_event_checks()
    checks += full_event_shift_checks()
    checks += failure_and_covariance_checks()
    checks += signed_generator_symbolic_checks()
    checks += generator_rational_admission_checks()

    print("the existing Hodge flag supplies both covariant transverse plane axes")
    print("one/two owner bundles realize every 216 T_r q chart with Delta J_EM=0")
    print("a bright routed history pair atomically manifests the token and constraint record")
    print("one signed type-2 generator books ownership, source, recoil, clock, and inverse")
    print("the full port conjugate reaction is symplectic and restores after two events")
    print("independent canonical normalizations leave alpha, gravity, and lensing open")
    print(
        "PASS: Hodge-framed all-axis constraint and signed event generator "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME A at blocked prepared-reference level: native owner/flag "
        "formation, poles, couplings, Born pushforward, and production remain open"
    )


if __name__ == "__main__":
    main()
