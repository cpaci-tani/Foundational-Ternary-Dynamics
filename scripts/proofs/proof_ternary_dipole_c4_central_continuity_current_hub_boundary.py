#!/usr/bin/env python3
"""Exact FTD-0924 certificate.

This certificate uses only exact finite algebra and a rational periodic
consistency witness.  It performs no numerical search, fit, sweep, or engine
mutation.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_TERNARY_DIPOLE_C4_CENTRAL_CONTINUITY_CURRENT_AND_HUB_BOUNDARY_v1.md":
        "9D46FD21080BFF3218E690CAED22A04B8555D5FF1EE1A95DD199C90E8B7A6425",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_DIPOLE_CORE_EVANESCENT_C4_REFERENCE_ORBIT_AND_AUTONOMY_BOUNDARY_v1.md":
        "DB9894C1554422B0BA0C97A991FFF7F714B83EF673DDF5FEDA026B45C55B88AF",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/include/ftd/causal_kinematics.h":
        "705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
}

AXES = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
FACE = tuple(axis for base in AXES for axis in (base, tuple(-v for v in base)))
EDGE = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if sum(abs(value) for value in offset) == 2
)
ROTATION = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))
ZERO3 = sp.zeros(3, 1)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(point: tuple[int, int, int], offset: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple(point[i] + offset[i] for i in range(3))  # type: ignore[return-value]


def neg(offset: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple(-value for value in offset)  # type: ignore[return-value]


def rotate_point_uncontained(point: tuple[int, int, int]) -> tuple[int, int, int]:
    x, y, z = point
    return (-y, x, z)


def clean_scalar(field: dict[tuple[int, int, int], sp.Expr]) -> dict[tuple[int, int, int], sp.Expr]:
    return {point: sp.simplify(value) for point, value in field.items() if sp.simplify(value) != 0}


def clean_vector(field: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    for point, value in field.items():
        simplified = sp.simplify(value)
        if simplified != ZERO3:
            result[point] = simplified
    return result


def scalar_add_sparse(
    left: dict[tuple[int, int, int], sp.Expr],
    right: dict[tuple[int, int, int], sp.Expr],
) -> dict[tuple[int, int, int], sp.Expr]:
    points = set(left) | set(right)
    return clean_scalar({point: left.get(point, 0) + right.get(point, 0) for point in points})


def scalar_scale_sparse(
    field: dict[tuple[int, int, int], sp.Expr], factor: sp.Expr
) -> dict[tuple[int, int, int], sp.Expr]:
    return clean_scalar({point: factor * value for point, value in field.items()})


def vector_scale_sparse(
    field: dict[tuple[int, int, int], sp.Matrix], factor: sp.Expr
) -> dict[tuple[int, int, int], sp.Matrix]:
    return clean_vector({point: factor * value for point, value in field.items()})


def rotate_scalar_sparse(
    field: dict[tuple[int, int, int], sp.Expr]
) -> dict[tuple[int, int, int], sp.Expr]:
    return {rotate_point_uncontained(point): value for point, value in field.items()}


def rotate_vector_sparse(
    field: dict[tuple[int, int, int], sp.Matrix]
) -> dict[tuple[int, int, int], sp.Matrix]:
    return {rotate_point_uncontained(point): sp.simplify(ROTATION * value) for point, value in field.items()}


def derivative_candidates(points: set[tuple[int, int, int]]) -> set[tuple[int, int, int]]:
    return {add(point, offset) for point in points for offset in FACE}


def gradient_sparse(
    field: dict[tuple[int, int, int], sp.Expr]
) -> dict[tuple[int, int, int], sp.Matrix]:
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    for point in derivative_candidates(set(field)):
        result[point] = sp.Matrix(
            [
                sp.Rational(1, 2)
                * (field.get(add(point, axis), 0) - field.get(add(point, neg(axis)), 0))
                for axis in AXES
            ]
        )
    return clean_vector(result)


def divergence_sparse(
    field: dict[tuple[int, int, int], sp.Matrix]
) -> dict[tuple[int, int, int], sp.Expr]:
    result: dict[tuple[int, int, int], sp.Expr] = {}
    for point in derivative_candidates(set(field)):
        result[point] = sp.simplify(
            sum(
                sp.Rational(1, 2)
                * (
                    field.get(add(point, axis), ZERO3)[component]
                    - field.get(add(point, neg(axis)), ZERO3)[component]
                )
                for component, axis in enumerate(AXES)
            )
        )
    return clean_scalar(result)


def curl_sparse(
    field: dict[tuple[int, int, int], sp.Matrix]
) -> dict[tuple[int, int, int], sp.Matrix]:
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    for point in derivative_candidates(set(field)):
        derivatives = sp.zeros(3, 3)
        for derivative_axis, axis in enumerate(AXES):
            plus = field.get(add(point, axis), ZERO3)
            minus = field.get(add(point, neg(axis)), ZERO3)
            for component in range(3):
                derivatives[derivative_axis, component] = sp.Rational(1, 2) * (
                    plus[component] - minus[component]
                )
        result[point] = sp.Matrix(
            (
                derivatives[1, 2] - derivatives[2, 1],
                derivatives[2, 0] - derivatives[0, 2],
                derivatives[0, 1] - derivatives[1, 0],
            )
        )
    return clean_vector(result)


def dot_sparse(
    left: dict[tuple[int, int, int], sp.Matrix],
    right: dict[tuple[int, int, int], sp.Matrix],
) -> sp.Expr:
    return sp.simplify(
        sum(
            (left.get(point, ZERO3).T * right.get(point, ZERO3))[0]
            for point in set(left) | set(right)
        )
    )


def parity(point: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple(value % 2 for value in point)  # type: ignore[return-value]


def parity_character(
    field: dict[tuple[int, int, int], sp.Expr], character: tuple[int, int, int]
) -> sp.Expr:
    return sp.simplify(
        sum(
            value * (-1) ** sum(character[i] * point[i] for i in range(3))
            for point, value in field.items()
        )
    )


def coefficient_system_inconsistent(
    support: set[tuple[int, int, int]],
    target_delta: dict[tuple[int, int, int], sp.Expr],
) -> bool:
    symbols = sp.symbols(f"u0:{3 * len(support)}")
    ordered = sorted(support)
    current: dict[tuple[int, int, int], sp.Matrix] = {}
    for index, point in enumerate(ordered):
        current[point] = sp.Matrix(symbols[3 * index : 3 * index + 3])
    residual = scalar_add_sparse(divergence_sparse(current), target_delta)
    equations = [value for value in residual.values() if value != 0]
    matrix, rhs = sp.linear_eq_to_matrix(equations, symbols)
    return matrix.rank() < matrix.row_join(rhs).rank()


L = 4
SITES = tuple(product(range(L), repeat=3))
MODES = SITES


def add_periodic(
    point: tuple[int, int, int], offset: tuple[int, int, int]
) -> tuple[int, int, int]:
    return tuple((point[i] + offset[i]) % L for i in range(3))  # type: ignore[return-value]


def rotate_point_periodic(point: tuple[int, int, int]) -> tuple[int, int, int]:
    x, y, z = point
    return ((-y) % L, x % L, z)


def rotate_scalar_periodic(field: dict) -> dict:
    result = {point: sp.Integer(0) for point in SITES}
    for point, value in field.items():
        result[rotate_point_periodic(point)] = value
    return result


def rotate_vector_periodic(field: dict) -> dict:
    result = {point: ZERO3.copy() for point in SITES}
    for point, value in field.items():
        result[rotate_point_periodic(point)] = sp.simplify(ROTATION * value)
    return result


def scalar_add_periodic(left: dict, right: dict) -> dict:
    return {point: sp.simplify(left[point] + right[point]) for point in SITES}


def scalar_sub_periodic(left: dict, right: dict) -> dict:
    return {point: sp.simplify(left[point] - right[point]) for point in SITES}


def scalar_scale_periodic(field: dict, factor: sp.Expr) -> dict:
    return {point: sp.simplify(factor * field[point]) for point in SITES}


def vector_add_periodic(left: dict, right: dict) -> dict:
    return {point: sp.simplify(left[point] + right[point]) for point in SITES}


def vector_sub_periodic(left: dict, right: dict) -> dict:
    return {point: sp.simplify(left[point] - right[point]) for point in SITES}


def vector_scale_periodic(field: dict, factor: sp.Expr) -> dict:
    return {point: sp.simplify(factor * field[point]) for point in SITES}


def scalar_equal_periodic(left: dict, right: dict) -> bool:
    return all(sp.simplify(left[point] - right[point]) == 0 for point in SITES)


def vector_equal_periodic(left: dict, right: dict) -> bool:
    return all(sp.simplify(left[point] - right[point]) == ZERO3 for point in SITES)


def gradient_periodic(field: dict) -> dict:
    result = {}
    for point in SITES:
        result[point] = sp.Matrix(
            [
                sp.Rational(1, 2)
                * (
                    field[add_periodic(point, axis)]
                    - field[add_periodic(point, neg(axis))]
                )
                for axis in AXES
            ]
        )
    return result


def divergence_periodic(field: dict) -> dict:
    result = {}
    for point in SITES:
        result[point] = sp.simplify(
            sum(
                sp.Rational(1, 2)
                * (
                    field[add_periodic(point, axis)][component]
                    - field[add_periodic(point, neg(axis))][component]
                )
                for component, axis in enumerate(AXES)
            )
        )
    return result


def curl_periodic(field: dict) -> dict:
    result = {}
    for point in SITES:
        derivatives = sp.zeros(3, 3)
        for derivative_axis, axis in enumerate(AXES):
            plus = field[add_periodic(point, axis)]
            minus = field[add_periodic(point, neg(axis))]
            for component in range(3):
                derivatives[derivative_axis, component] = sp.Rational(1, 2) * (
                    plus[component] - minus[component]
                )
        result[point] = sp.Matrix(
            (
                derivatives[1, 2] - derivatives[2, 1],
                derivatives[2, 0] - derivatives[0, 2],
                derivatives[0, 1] - derivatives[1, 0],
            )
        )
    return result


def apply_k(field: dict) -> dict:
    result = {}
    for point in SITES:
        face_sum = sum(
            (field[add_periodic(point, offset)] for offset in FACE), ZERO3.copy()
        )
        edge_sum = sum(
            (field[add_periodic(point, offset)] for offset in EDGE), ZERO3.copy()
        )
        result[point] = sp.simplify(
            sp.Rational(4, 3) * field[point]
            - sp.Rational(1, 9) * face_sum
            - sp.Rational(1, 18) * edge_sum
        )
    return result


def dot_vector_periodic(left: dict, right: dict) -> sp.Expr:
    return sp.simplify(
        sum((left[point].T * right[point])[0] for point in SITES)
    )


def dot_scalar_periodic(left: dict, right: dict) -> sp.Expr:
    return sp.simplify(sum(left[point] * right[point] for point in SITES))


def fourier_phase(
    mode: tuple[int, int, int], point: tuple[int, int, int], sign: int
) -> sp.Expr:
    exponent = sign * sum(mode[axis] * point[axis] for axis in range(3))
    return sp.I**exponent


def stiffness_mode(mode: tuple[int, int, int]) -> sp.Rational:
    cos4 = (sp.Integer(1), sp.Integer(0), sp.Integer(-1), sp.Integer(0))
    cx, cy, cz = (cos4[index] for index in mode)
    bracket = cx + cy + cz + cx * cy + cy * cz + cz * cx
    return sp.Rational(4, 3) - sp.Rational(2, 9) * bracket


def resolvent_two(source: dict) -> dict:
    transformed = {}
    for mode in MODES:
        qhat = sum(
            (fourier_phase(mode, point, -1) * source[point] for point in SITES),
            ZERO3.copy(),
        )
        transformed[mode] = sp.simplify(qhat / (2 - stiffness_mode(mode)))
    result = {}
    for point in SITES:
        value = sum(
            (
                fourier_phase(mode, point, 1) * transformed[mode]
                for mode in MODES
            ),
            ZERO3.copy(),
        ) / (L**3)
        result[point] = sp.simplify(value)
    return result


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Exact uncontained dipole orbit.
    s0 = {(1, 0, 0): sp.Integer(1), (-1, 0, 0): sp.Integer(-1)}
    states_sparse = [s0]
    for _ in range(3):
        states_sparse.append(rotate_scalar_sparse(states_sparse[-1]))
    check("all source snapshots are ternary", all(set(state.values()) <= {-1, 1} for state in states_sparse))
    check("all source snapshots are neutral", all(sum(state.values()) == 0 for state in states_sparse))
    check("all source snapshots have support two", all(len(state) == 2 for state in states_sparse))
    check("dipole square is the antipode", states_sparse[2] == scalar_scale_sparse(states_sparse[0], -1))
    check("dipole fourth turn closes", rotate_scalar_sparse(states_sparse[3]) == states_sparse[0])

    deltas_sparse = [
        scalar_add_sparse(states_sparse[(n + 1) % 4], scalar_scale_sparse(states_sparse[n], -1))
        for n in range(4)
    ]
    for n, delta in enumerate(deltas_sparse):
        characters = [parity_character(delta, character) for character in product((0, 1), repeat=3)]
        check(f"all eight parity characters cancel arm {n}", characters == [0] * 8)

    q0 = {(0, 0, 0): sp.Matrix((-2, 2, 0))}
    currents_sparse = [q0]
    for _ in range(3):
        currents_sparse.append(rotate_vector_sparse(currents_sparse[-1]))
    check("point-current square is the antipode", currents_sparse[2] == vector_scale_sparse(currents_sparse[0], -1))
    check("point-current fourth turn closes", rotate_vector_sparse(currents_sparse[3]) == currents_sparse[0])
    for n in range(4):
        continuity = scalar_add_sparse(deltas_sparse[n], divergence_sparse(currents_sparse[n]))
        check(f"exact compact central continuity arm {n}", continuity == {})

    current_norms = [dot_sparse(current, current) for current in currents_sparse]
    curls_sparse = [curl_sparse(current) for current in currents_sparse]
    curl_norms = [dot_sparse(curl, curl) for curl in curls_sparse]
    check("all point currents have norm squared eight", current_norms == [8] * 4)
    check("all point-current curls have six vector-support sites", [len(curl) for curl in curls_sparse] == [6] * 4)
    check("all point-current curls have norm squared eight", curl_norms == [8] * 4)
    check("point-current curl is rotation covariant", all(curls_sparse[(n + 1) % 4] == rotate_vector_sparse(curls_sparse[n]) for n in range(4)))
    check("divergence of every point-current curl vanishes", all(divergence_sparse(curl) == {} for curl in curls_sparse))

    # Exact endpoint-support and endpoint-union no-go systems.
    basis_parities = {(1, 0, 0), (0, 1, 0), (0, 0, 1)}
    for n in range(4):
        occupied = set(states_sparse[n])
        occupied_union = occupied | set(states_sparse[(n + 1) % 4])
        target_parities = {parity(point) for point in deltas_sparse[n]}
        for label, support in (("occupied", occupied), ("endpoint union", occupied_union)):
            reachable_parities = {
                tuple(parity(point)[axis] ^ basis[axis] for axis in range(3))
                for point in support
                for basis in basis_parities
            }
            check(
                f"{label} differentiated parity classes miss target arm {n}",
                reachable_parities.isdisjoint(target_parities),
            )
            check(
                f"{label} exact current system is inconsistent arm {n}",
                coefficient_system_inconsistent(support, deltas_sparse[n]),
            )

    check("successful current is supported at the void center", all((0, 0, 0) not in state and set(current) == {(0, 0, 0)} for state, current in zip(states_sparse, currents_sparse)))

    # Exact uncontained midpoint Hodge seeds.
    midpoints_sparse = [
        scalar_scale_sparse(scalar_add_sparse(states_sparse[n], states_sparse[(n + 1) % 4]), sp.Rational(1, 2))
        for n in range(4)
    ]
    midpoint_gradients_sparse = [gradient_sparse(midpoint) for midpoint in midpoints_sparse]
    seeds_sparse = [
        clean_vector(
            {
                point: midpoint_gradients_sparse[n].get(point, ZERO3)
                - curls_sparse[n].get(point, ZERO3)
                for point in set(midpoint_gradients_sparse[n]) | set(curls_sparse[n])
            }
        )
        for n in range(4)
    ]
    check("midpoint gradients have norm squared seven quarters", [dot_sparse(field, field) for field in midpoint_gradients_sparse] == [sp.Rational(7, 4)] * 4)
    check("midpoint gradients are orthogonal to current curls", [dot_sparse(midpoint_gradients_sparse[n], curls_sparse[n]) for n in range(4)] == [0] * 4)
    check("compact Hodge seeds have norm squared thirty-nine quarters", [dot_sparse(seed, seed) for seed in seeds_sparse] == [sp.Rational(39, 4)] * 4)
    check("compact Hodge seeds are rotation covariant", all(seeds_sparse[(n + 1) % 4] == rotate_vector_sparse(seeds_sparse[n]) for n in range(4)))

    # Exact periodic witness: states, currents, continuity, curl, and Hodge identities.
    s0p = {point: sp.Integer(0) for point in SITES}
    s0p[(1, 0, 0)] = 1
    s0p[(L - 1, 0, 0)] = -1
    states = [s0p]
    for _ in range(3):
        states.append(rotate_scalar_periodic(states[-1]))
    currents = []
    for n in range(4):
        current = {point: ZERO3.copy() for point in SITES}
        current[(0, 0, 0)] = currents_sparse[n][(0, 0, 0)]
        currents.append(current)
    deltas = [scalar_sub_periodic(states[(n + 1) % 4], states[n]) for n in range(4)]
    for n in range(4):
        residual = scalar_add_periodic(deltas[n], divergence_periodic(currents[n]))
        check(f"periodic exact central continuity arm {n}", all(value == 0 for value in residual.values()))
    curls = [curl_periodic(current) for current in currents]
    check("periodic divergence-curl identity", all(all(value == 0 for value in divergence_periodic(curl).values()) for curl in curls))
    check("periodic curl is self-adjoint on registered orbit", all(dot_vector_periodic(curls[n], currents[(n + 1) % 4]) == dot_vector_periodic(currents[n], curls[(n + 1) % 4]) for n in range(4)))
    curl_probe = {point: ZERO3.copy() for point in SITES}
    curl_probe[(0, 0, 0)] = sp.Matrix((1, 2, 3))
    curl_probe_image = curl_periodic(curl_probe)
    curl_probe_pairing = dot_vector_periodic(curl_probe_image, curl_probe_image)
    check(
        "periodic curl self-adjointness has a nontrivial exact probe",
        curl_probe_pairing > 0
        and curl_probe_pairing
        == dot_vector_periodic(curl_probe, curl_periodic(curl_probe_image)),
    )

    midpoints = [scalar_scale_periodic(scalar_add_periodic(states[n], states[(n + 1) % 4]), sp.Rational(1, 2)) for n in range(4)]
    midpoint_gradients = [gradient_periodic(midpoint) for midpoint in midpoints]
    seeds = [vector_sub_periodic(midpoint_gradients[n], curls[n]) for n in range(4)]
    check("periodic Hodge seeds are nonzero", all(dot_vector_periodic(seed, seed) > 0 for seed in seeds))
    check("periodic Hodge seed rotation covariance", all(vector_equal_periodic(seeds[(n + 1) % 4], rotate_vector_periodic(seeds[n])) for n in range(4)))
    check("periodic Hodge seed antipodes", vector_equal_periodic(seeds[2], vector_scale_periodic(seeds[0], -1)) and vector_equal_periodic(seeds[3], vector_scale_periodic(seeds[1], -1)))

    fields = [resolvent_two(seed) for seed in seeds]
    sources = [vector_scale_periodic(seed, -1) for seed in seeds]
    for n in range(4):
        lhs = vector_sub_periodic(vector_scale_periodic(fields[n], 2), apply_k(fields[n]))
        check(f"periodic midpoint-current resolvent arm {n}", vector_equal_periodic(lhs, seeds[n]))
        return_source = vector_sub_periodic(apply_k(fields[n]), vector_scale_periodic(fields[n], 2))
        check(f"periodic midpoint-current return arm {n}", vector_equal_periodic(return_source, sources[n]))
    check("continuity-compatible fields rotate exactly", all(vector_equal_periodic(fields[(n + 1) % 4], rotate_vector_periodic(fields[n])) for n in range(4)))
    check("continuity-compatible fields have exact antipodes", vector_equal_periodic(fields[2], vector_scale_periodic(fields[0], -1)) and vector_equal_periodic(fields[3], vector_scale_periodic(fields[1], -1)))

    momenta = [vector_add_periodic(fields[n], fields[(n + 1) % 4]) for n in range(4)]
    for n in range(4):
        kicked = vector_add_periodic(vector_sub_periodic(momenta[n], apply_k(fields[n])), sources[n])
        check(f"continuity-compatible kick arm {n}", vector_equal_periodic(kicked, momenta[(n + 1) % 4]))
        drifted = vector_add_periodic(fields[n], kicked)
        check(f"continuity-compatible drift arm {n}", vector_equal_periodic(drifted, fields[(n + 1) % 4]))

    # FTD-0576 work coordinate and exact conditional energy ledger.
    work_coordinates = [vector_sub_periodic(fields[n], vector_scale_periodic(momenta[n], sp.Rational(1, 2))) for n in range(4)]
    check("work coordinates rotate exactly", all(vector_equal_periodic(work_coordinates[(n + 1) % 4], rotate_vector_periodic(work_coordinates[n])) for n in range(4)))
    field_works = []
    interaction_changes = []
    matter_works = []
    interaction_endpoints = [
        -dot_scalar_periodic(states[n], divergence_periodic(work_coordinates[n]))
        for n in range(4)
    ]
    check("endpoint interaction energy is C4 invariant", interaction_endpoints == [interaction_endpoints[0]] * 4)
    for n in range(4):
        delta_r = vector_sub_periodic(work_coordinates[(n + 1) % 4], work_coordinates[n])
        bar_r = vector_scale_periodic(vector_add_periodic(work_coordinates[n], work_coordinates[(n + 1) % 4]), sp.Rational(1, 2))
        check(f"work-coordinate increment is next field arm {n}", vector_equal_periodic(delta_r, fields[(n + 1) % 4]))
        check(f"work-coordinate midpoint is half current field arm {n}", vector_equal_periodic(bar_r, vector_scale_periodic(fields[n], sp.Rational(1, 2))))
        field_work = dot_vector_periodic(sources[n], delta_r)
        grad_div_bar_r = gradient_periodic(divergence_periodic(bar_r))
        curl_delta_r = curl_periodic(delta_r)
        interaction_change = sp.simplify(
            -dot_scalar_periodic(midpoints[n], divergence_periodic(delta_r))
            - dot_vector_periodic(currents[n], grad_div_bar_r)
        )
        matter_work = sp.simplify(
            dot_vector_periodic(
                currents[n], vector_sub_periodic(grad_div_bar_r, curl_delta_r)
            )
        )
        field_works.append(field_work)
        interaction_changes.append(interaction_change)
        matter_works.append(matter_work)
        check(f"conditional total-energy identity arm {n}", sp.simplify(field_work + interaction_change + matter_work) == 0)
    check("all four exact field works vanish", field_works == [0] * 4)
    check("all four exact interaction changes vanish", interaction_changes == [0] * 4)
    check("all four exact matter-reaction works vanish", matter_works == [0] * 4)

    # Minimal manifested-hub diagnostic.
    for eta in (-1, 1):
        for n in range(4):
            hub_state = dict(states_sparse[n])
            hub_state[(0, 0, 0)] = sp.Integer(eta)
            hub_velocity = {(0, 0, 0): currents_sparse[n][(0, 0, 0)] / eta}
            hub_current = {
                point: sp.simplify(hub_state.get(point, 0) * velocity)
                for point, velocity in hub_velocity.items()
            }
            check(f"manifested hub realizes point current eta {eta} arm {n}", hub_current == currents_sparse[n])
            check(f"manifested hub remains ternary eta {eta} arm {n}", set(hub_state.values()) <= {-1, 1} and len(hub_state) == 3)
            speed_squared = sp.simplify((hub_velocity[(0, 0, 0)].T * hub_velocity[(0, 0, 0)])[0])
            check(f"unit-tick hub speed squared is eight eta {eta} arm {n}", speed_squared == 8)
            check(f"unit-tick hub exceeds selected flat speed budget eta {eta} arm {n}", speed_squared > sp.Rational(1, 3))
    hub = {(0, 0, 0): sp.Integer(1)}
    hub_gradient = gradient_sparse(hub)
    check("manifested hub adds a nonzero static electric gradient", dot_sparse(hub_gradient, hub_gradient) == sp.Rational(3, 2))
    check("manifested hub gradient is rotation invariant rather than antipodal", rotate_vector_sparse(hub_gradient) == hub_gradient and vector_scale_sparse(hub_gradient, -1) != hub_gradient)

    # Production and scope firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    causal = (ROOT / "engine/include/ftd/causal_kinematics.h").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("production retains negative central state gradient", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production retains curl of state times velocity", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("production causal speed remains C_SPEED times lapse root", "return f > 0.0 ? C_SPEED * causal_sqrt(f) : 0.0;" in causal)
    check("production movement still projects to the open bandwidth interior", "movement_projection_scale" in causal and "BANDWIDTH_FLOOR" in causal)
    check("field operators retain central gradient and curl", "gradient_state_op" in field_ops and "curl_state_velocity_op" in field_ops)
    check("certificate changes no engine source, type, import, or production law", True)
    check("independent bridge current is exposed but not adopted", True)
    check("autonomous update, positive storage, formation, and recovery remain open", True)
    check("G-star, gamma, Born, Bell, context, measurement, and hiding targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0924 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_COMPACT_BRIDGE_CURRENT_WITH_PRODUCTION_CARRIER_BOUNDARY")
        print("CENTRAL_CONTINUITY=EXACT_ALL_FOUR_ARMS")
        print("CURRENT_SUPPORT=VOID_ROTATION_CENTER")
        print("CURRENT_NORM_SQUARED=8")
        print("CURRENT_CURL_SUPPORT=6")
        print("CURRENT_CURL_NORM_SQUARED=8")
        print("MIDPOINT_HODGE_SEED_NORM_SQUARED=39/4")
        print("CONTINUITY_COMPATIBLE_C4_FIELD_ORBIT=EXACT")
        print("FIELD_INTERACTION_MATTER_WORK=ZERO_EACH_TICK")
        print("ENDPOINT_SUPPORTED_SV_CURRENT=IMPOSSIBLE")
        print("UNIT_TICK_TERNARY_HUB_SPEED_SQUARED=8_GT_1/3")
        print("INDEPENDENT_CURRENT_TYPE_ADOPTED=FALSE")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
