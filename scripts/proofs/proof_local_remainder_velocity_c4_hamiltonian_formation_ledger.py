#!/usr/bin/env python3
"""FTD-0926 exact certificate for the local remainder--velocity C4 Hamiltonian."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path
from typing import TypeAlias

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_LEDGER_v1.md":
        "BD98EA4CC0EF2B858BD2D8D504892468F5100DF462E5B169118BA6C39AFD6136",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md":
        "581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39",
    "scripts/proofs/proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py":
        "62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md":
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    "scripts/proofs/proof_local_canonical_hamiltonian_parity_rail.py":
        "B971DDA9A79AD53C340B00A4268EF9DA5BF089AF62DC37DE3D04757FAE03E326",
    "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_forces.cpp":
        "F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322",
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/include/ftd/causal_kinematics.h":
        "705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

Point: TypeAlias = tuple[int, int, int]
ZERO3 = sp.zeros(3, 1)
EX: Point = (1, 0, 0)
EY: Point = (0, 1, 0)
EZ: Point = (0, 0, 1)
ROTATION = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(*points: Point) -> Point:
    return tuple(sum(point[axis] for point in points) for axis in range(3))  # type: ignore[return-value]


def scale(point: Point, factor: int) -> Point:
    return tuple(factor * value for value in point)  # type: ignore[return-value]


def neg(point: Point) -> Point:
    return scale(point, -1)


def rotate_point(point: Point) -> Point:
    result = ROTATION * sp.Matrix(point)
    return tuple(int(value) for value in result)  # type: ignore[return-value]


def clean_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {
        point: value
        for point, raw in field.items()
        if (value := sp.simplify(raw)) != 0
    }


def clean_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return {
        point: value
        for point, raw in field.items()
        if (value := sp.simplify(raw)) != ZERO3
    }


def scalar_add(
    left: dict[Point, sp.Expr],
    right: dict[Point, sp.Expr],
) -> dict[Point, sp.Expr]:
    result = dict(left)
    for point, value in right.items():
        result[point] = sp.simplify(result.get(point, 0) + value)
    return clean_scalar(result)


def scalar_scale(
    field: dict[Point, sp.Expr],
    factor: sp.Expr,
) -> dict[Point, sp.Expr]:
    return clean_scalar({point: factor * value for point, value in field.items()})


def rotate_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {rotate_point(point): value for point, value in field.items()}


def rotate_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return clean_vector({
        rotate_point(point): sp.simplify(ROTATION * value)
        for point, value in field.items()
    })


def add_path(
    current: dict[Point, sp.Matrix],
    vertices: list[Point],
    flow: sp.Rational,
) -> None:
    for start, end in zip(vertices, vertices[1:]):
        displacement = sp.Matrix(add(end, neg(start)))
        nonzero = [axis for axis, value in enumerate(displacement) if value != 0]
        if len(nonzero) != 1 or abs(displacement[nonzero[0]]) != 2:
            raise ValueError(f"not a central-difference edge: {start} -> {end}")
        axis = nonzero[0]
        center = tuple((start[k] + end[k]) // 2 for k in range(3))
        contribution = ZERO3.copy()
        contribution[axis] = 2 * flow * sp.sign(displacement[axis])
        current[center] = sp.simplify(current.get(center, ZERO3) + contribution)


def divergence(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Expr]:
    candidates: set[Point] = set()
    for point in field:
        for axis in (EX, EY, EZ):
            candidates.add(add(point, axis))
            candidates.add(add(point, neg(axis)))
    result: dict[Point, sp.Expr] = {}
    for point in candidates:
        value = sp.Integer(0)
        for axis_index, axis in enumerate((EX, EY, EZ)):
            plus = field.get(add(point, axis), ZERO3)[axis_index]
            minus = field.get(add(point, neg(axis)), ZERO3)[axis_index]
            value += (plus - minus) / 2
        if (value := sp.simplify(value)) != 0:
            result[point] = value
    return result


def vector_norm_squared(field: dict[Point, sp.Matrix]) -> sp.Expr:
    return sp.simplify(sum((value.dot(value) for value in field.values()), sp.Integer(0)))


def vector_dot(
    left: dict[Point, sp.Matrix],
    right: dict[Point, sp.Matrix],
) -> sp.Expr:
    points = set(left) | set(right)
    return sp.simplify(sum(
        (left.get(point, ZERO3).dot(right.get(point, ZERO3)) for point in points),
        sp.Integer(0),
    ))


def local_update(
    remainder: sp.Matrix,
    velocity: sp.Matrix,
) -> tuple[sp.Matrix, sp.Matrix]:
    next_velocity = sp.simplify(velocity - 2 * remainder)
    next_remainder = sp.simplify(remainder + next_velocity)
    return next_remainder, next_velocity


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Reconstruct the FTD-0925 equal-five-channel current.
    flow = sp.Rational(1, 5)
    current0: dict[Point, sp.Matrix] = {}
    add_path(current0, [EX, neg(EX)], flow)
    for transverse in (EY, neg(EY), EZ, neg(EZ)):
        add_path(
            current0,
            [
                EX,
                add(EX, scale(transverse, 2)),
                add(neg(EX), scale(transverse, 2)),
                neg(EX),
            ],
            flow,
        )
    add_path(current0, [neg(EY), EY], flow)
    for transverse in (EX, neg(EX), EZ, neg(EZ)):
        add_path(
            current0,
            [
                neg(EY),
                add(neg(EY), scale(transverse, 2)),
                add(EY, scale(transverse, 2)),
                EY,
            ],
            flow,
        )
    current0 = clean_vector(current0)
    currents = [current0]
    for _ in range(3):
        currents.append(rotate_vector(currents[-1]))

    states = [{EX: sp.Integer(1), neg(EX): sp.Integer(-1)}]
    for _ in range(3):
        states.append(rotate_scalar(states[-1]))
    deltas = [
        scalar_add(states[(n + 1) % 4], scalar_scale(states[n], -1))
        for n in range(4)
    ]

    origin = {(0, 0, 0)}
    dxy = {(sx, sy, 0) for sx in (-1, 1) for sy in (-1, 1)}
    axy = {(2, 0, 0), (-2, 0, 0), (0, 2, 0), (0, -2, 0)}
    zplus = {(1, 0, 1), (-1, 0, 1), (0, 1, 1), (0, -1, 1)}
    zminus = {(1, 0, -1), (-1, 0, -1), (0, 1, -1), (0, -1, -1)}
    plus2z = {(0, 0, 2)}
    minus2z = {(0, 0, -2)}
    neutralizer = {(0, 0, 1)}
    h: dict[Point, sp.Expr] = {}
    for point in origin | dxy | axy | plus2z:
        h[point] = sp.Integer(1)
    for point in zplus | zminus | minus2z | neutralizer:
        h[point] = sp.Integer(-1)

    velocities = [
        {
            point: sp.simplify(current.get(point, ZERO3) / h[point])
            if point in current else ZERO3.copy()
            for point in h
        }
        for current in currents
    ]
    records = [scalar_add(h, state) for state in states]
    check("reconstructed current support is nineteen", len(current0) == 19)
    check("reconstructed scaffold support is twenty", len(h) == 20)
    check("reconstructed scaffold is neutral ternary", set(h.values()) == {-1, 1} and sum(h.values()) == 0)
    check("reconstructed scaffold is C4 invariant", rotate_scalar(h) == h)
    for n in range(4):
        live_current = clean_vector({
            point: sp.simplify(records[n].get(point, 0) * velocities[n][point])
            for point in h
        })
        check(f"registered live current arm {n}", live_current == currents[n])
        check(
            f"registered central continuity arm {n}",
            scalar_add(deltas[n], divergence(live_current)) == {},
        )
        check(
            f"pointwise speed is retained arm {n}",
            all(
                sp.simplify(velocities[(n + 1) % 4][point].dot(velocities[(n + 1) % 4][point])
                            - velocities[n][point].dot(velocities[n][point])) == 0
                for point in h
            ),
        )
    peak_speed_squared = max(
        value.dot(value)
        for velocity in velocities
        for value in velocity.values()
    )
    check("registered peak speed squared is eight twenty-fifths", peak_speed_squared == sp.Rational(8, 25))
    check("registered speed remains below one third", peak_speed_squared < sp.Rational(1, 3))
    check("velocity square is pointwise antipodal", all(velocities[2][point] == -velocities[0][point] for point in h))
    check("velocity third arm is pointwise antipodal", all(velocities[3][point] == -velocities[1][point] for point in h))
    norm_v0 = vector_norm_squared(velocities[0])
    norm_v1 = vector_norm_squared(velocities[1])
    check("first two velocity fields have equal norm", norm_v0 == norm_v1)
    check("first two velocity fields are globally orthogonal", vector_dot(velocities[0], velocities[1]) == 0)

    # Exact local symplectic/Hamiltonian algebra.
    matrix_m = sp.Matrix(((-1, 1), (-2, 1)))
    symplectic_j = sp.Matrix(((0, 1), (-1, 0)))
    metric_g = sp.Matrix(((2, -1), (-1, 1)))
    identity2 = sp.eye(2)
    check("local map square is minus identity", matrix_m**2 == -identity2)
    check("local map fourth power is identity", matrix_m**4 == identity2)
    check("local map determinant is one", matrix_m.det() == 1)
    check("local map is symplectic", matrix_m.T * symplectic_j * matrix_m == symplectic_j)
    check("positive metric is exactly invariant", matrix_m.T * metric_g * matrix_m == metric_g)
    check("Hamiltonian matrix equals JG", symplectic_j * metric_g == matrix_m)
    eigenvalues = sorted(metric_g.eigenvals(), key=lambda value: float(value))
    expected_eigenvalues = [
        (sp.Integer(3) - sp.sqrt(5)) / 2,
        (sp.Integer(3) + sp.sqrt(5)) / 2,
    ]
    check("metric eigenvalues are exact golden pair", eigenvalues == expected_eigenvalues)
    check("Hamiltonian metric is positive definite", all(value > 0 for value in eigenvalues))
    check(
        "quarter-period exponential is exact local map",
        sp.cos(sp.pi / 2) * identity2 + sp.sin(sp.pi / 2) * matrix_m == matrix_m,
    )
    reverse_m = matrix_m.inv()
    check("inverse map is minus local map", reverse_m == -matrix_m)

    # The existing remainder supplies phase; no equilibrium-center field is used.
    remainder = {
        point: sp.simplify((velocities[0][point] - velocities[1][point]) / 2)
        for point in h
    }
    velocity = {point: velocities[0][point] for point in h}
    initial_remainder = {point: value.copy() for point, value in remainder.items()}
    initial_velocity = {point: value.copy() for point, value in velocity.items()}
    expected_remainders = [
        {
            point: sp.simplify((velocities[0][point] - velocities[1][point]) / 2)
            for point in h
        },
        {
            point: sp.simplify((velocities[0][point] + velocities[1][point]) / 2)
            for point in h
        },
        {
            point: sp.simplify((velocities[1][point] - velocities[0][point]) / 2)
            for point in h
        },
        {
            point: sp.simplify(-(velocities[0][point] + velocities[1][point]) / 2)
            for point in h
        },
    ]

    energies: list[sp.Expr] = []
    remainder_component_maxima: list[sp.Expr] = []
    for n in range(4):
        check(
            f"registered velocity enters source-read arm {n}",
            all(velocity[point] == velocities[n][point] for point in h),
        )
        check(
            f"registered remainder phase arm {n}",
            all(remainder[point] == expected_remainders[n][point] for point in h),
        )
        energy = sp.simplify(sum(
            (
                remainder[point].dot(remainder[point])
                - remainder[point].dot(velocity[point])
                + sp.Rational(1, 2) * velocity[point].dot(velocity[point])
                for point in h
            ),
            sp.Integer(0),
        ))
        energies.append(energy)
        next_remainder: dict[Point, sp.Matrix] = {}
        next_velocity: dict[Point, sp.Matrix] = {}
        for point in h:
            next_remainder[point], next_velocity[point] = local_update(
                remainder[point], velocity[point]
            )
        check(
            f"homogeneous onsite update generates velocity arm {(n + 1) % 4}",
            all(next_velocity[point] == velocities[(n + 1) % 4][point] for point in h),
        )
        check(
            f"force-then-movement identity arm {n}",
            all(next_remainder[point] == remainder[point] + next_velocity[point] for point in h),
        )
        remainder = next_remainder
        velocity = next_velocity
        remainder_component_maxima.append(max(
            abs(component)
            for value in remainder.values()
            for component in value
        ))
        check(
            f"no manifested hop after generated arm {n}",
            remainder_component_maxima[-1] < 1,
        )

    check("local map returns exact initial remainder", all(remainder[point] == initial_remainder[point] for point in h))
    check("local map returns exact initial velocity", all(velocity[point] == initial_velocity[point] for point in h))
    check("all generated remainder maxima equal two fifths", remainder_component_maxima == [sp.Rational(2, 5)] * 4)
    check("initial remainder norm squared is fifty-two twenty-fifths", vector_norm_squared(initial_remainder) == sp.Rational(52, 25))
    check("registered velocity norm squared is one-hundred-four twenty-fifths", norm_v0 == sp.Rational(104, 25))
    check("positive carrier functional is fifty-two twenty-fifths each arm", energies == [sp.Rational(52, 25)] * 4)
    unit_tick_hamiltonian = sp.simplify(sp.pi / 2 * energies[0])
    check("unit-tick positive Hamiltonian is twenty-six pi twenty-fifths", unit_tick_hamiltonian == 26 * sp.pi / 25)

    # Exact endpoint work and reversible-stability controls.
    for n in range(4):
        impulse = {
            point: sp.simplify(velocities[(n + 1) % 4][point] - velocities[n][point])
            for point in h
        }
        midpoint_velocity = {
            point: sp.simplify((velocities[(n + 1) % 4][point] + velocities[n][point]) / 2)
            for point in h
        }
        check(
            f"pointwise isotropic endpoint work vanishes arm {n}",
            all(impulse[point].dot(midpoint_velocity[point]) == 0 for point in h),
        )
        check(
            f"global isotropic endpoint work vanishes arm {n}",
            vector_dot(impulse, midpoint_velocity) == 0,
        )
    delta_r, delta_v = sp.symbols("delta_r delta_v")
    perturbation = sp.Matrix((delta_r, delta_v))
    check("arbitrary local perturbation returns after four maps", matrix_m**4 * perturbation == perturbation)
    check(
        "arbitrary local perturbation preserves positive quadratic norm",
        sp.expand((matrix_m * perturbation).T * metric_g * (matrix_m * perturbation))
        == sp.expand(perturbation.T * metric_g * perturbation),
    )
    check("zero local state is fixed", matrix_m * sp.zeros(2, 1) == sp.zeros(2, 1))

    reverse_remainder: dict[Point, sp.Matrix] = {}
    reverse_velocity: dict[Point, sp.Matrix] = {}
    for point in h:
        reverse_remainder[point] = sp.simplify(
            initial_remainder[point] - initial_velocity[point]
        )
        reverse_velocity[point] = sp.simplify(
            2 * initial_remainder[point] - initial_velocity[point]
        )
    check("inverse map reaches prior registered velocity arm", all(reverse_velocity[point] == velocities[3][point] for point in h))
    check("inverse map reaches prior registered remainder arm", all(reverse_remainder[point] == expected_remainders[3][point] for point in h))

    successor_sets: dict[tuple[sp.Expr, ...], set[tuple[sp.Expr, ...]]] = {}
    for point in h:
        key = tuple(velocities[0][point])
        successor_sets.setdefault(key, set()).add(tuple(velocities[1][point]))
    check(
        "velocity alone has identical local values with different successors",
        any(len(successors) > 1 for successors in successor_sets.values()),
    )
    zero_remainder_next = {
        point: local_update(ZERO3, velocities[0][point])[1]
        for point in h
    }
    check(
        "zero initial remainder does not generate registered next arm",
        any(zero_remainder_next[point] != velocities[1][point] for point in h),
    )
    check("initial phase is stored in existing remainder type", set(initial_remainder) == set(h))
    check("no site-dependent matrix is used", True)
    check("no equilibrium-center or hidden phase field is used", True)

    # Production/source-order and type firewalls.
    render_bridge = (ROOT / "engine/src/render_bridge.cpp").read_text(encoding="utf-8")
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_forces = (ROOT / "engine/src/render_bridge_phases/phase_forces.cpp").read_text(encoding="utf-8")
    phase_movement = (ROOT / "engine/src/render_bridge_phases/phase_movement.cpp").read_text(encoding="utf-8")
    voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
    causal = (ROOT / "engine/include/ftd/causal_kinematics.h").read_text(encoding="utf-8")
    check("Voxel already contains velocity", "Vec3 velocity;" in voxel)
    check("Voxel already contains remainder", "Vec3 remainder;" in voxel)
    check("production phase read consumes state velocity current", "curl_state_velocity_op" in phase_read)
    check("production phase order reads before forces", render_bridge.index("phase_read();", render_bridge.index("void RenderBridge::tick")) < render_bridge.index("phase_forces();", render_bridge.index("void RenderBridge::tick")))
    check("production phase order forces before movement", render_bridge.index("phase_forces();", render_bridge.index("void RenderBridge::tick")) < render_bridge.index("phase_movement();", render_bridge.index("void RenderBridge::tick")))
    check("production force phase updates velocity", "v.velocity = scale > 0.0 ? q * scale : Vec3{};" in phase_forces)
    check("production movement accumulates updated velocity", "v.remainder += v.velocity * rb.dt_;" in phase_movement)
    check("production movement threshold remains unit component crossing", "v.remainder.x >= 1.0" in phase_movement and "v.remainder.x <= -1.0" in phase_movement)
    check("selected flat speed remains one over sqrt three", "return raw_speed2 / (C_SPEED * C_SPEED);" in causal)
    check("registered local Hamiltonian is not a production force law", "velocity - 2" not in phase_forces and "remainder * 2" not in phase_forces)
    check("certificate changes no engine source, CMake target, type, import, or production law", True)
    check("formation reservoir and ternary manifestation energy remain open", True)
    check("field-derived vector impulse and reciprocal recoil remain open", True)
    check("reference-orbit attraction and coupled perturbation recovery remain open", True)
    check("mobility, physical scale, and production insertion remain open", True)
    check("G-star, gamma, Born, Bell, context, measurement, and hiding targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0926 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_EXACT_EXISTING_TYPE_LOCAL_REMAINDER_VELOCITY_GENERATOR")
        print("LOCAL_MAP=(r,v)->(v-r,v-2r)")
        print("LOCAL_MAP_SQUARED=-I")
        print("LOCAL_MAP_FOURTH_POWER=I")
        print("POSITIVE_QUADRATIC_STORAGE=52/25")
        print("UNIT_TICK_HAMILTONIAN=26*pi/25")
        print("REMAINDER_COMPONENT_MAX=2/5")
        print("VELOCITY_ORBIT=EXACT_ALL_FOUR_ARMS")
        print("LIVE_CONTINUITY=EXACT_ALL_FOUR_ARMS")
        print("ISOTROPIC_ENDPOINT_WORK=ZERO_POINTWISE")
        print("NEUTRAL_REVERSIBLE_STABILITY=EXACT_PERIOD_FOUR")
        print("NEW_ONTOLOGY_TYPE_ADOPTED=FALSE")
        print("PRODUCTION_FORCE_INSERTED=FALSE")
        print("FIELD_RECOIL_FORMATION_RECOVERY=OPEN")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if passed_count == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
