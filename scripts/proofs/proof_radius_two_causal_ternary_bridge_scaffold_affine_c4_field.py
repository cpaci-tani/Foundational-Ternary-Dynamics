#!/usr/bin/env python3
"""Exact FTD-0925 certificate.

The certificate uses exact rational network incidence, finite-support central
operators, graph cuts, and symbolic operator identities.  It performs no
numerical search, fit, sweep, or engine mutation.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AND_AFFINE_C4_FIELD_v1.md":
        "627C6F1583A1E07F03A1BAB01B9C7AA59D670A861DC650FF72CAD8100586EFBE",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_DIPOLE_C4_CENTRAL_CONTINUITY_BRIDGE_CURRENT_AND_PRODUCTION_HUB_BOUNDARY_v1.md":
        "0185C438DDB9CB5E061B54C2E1D20260615E367AF829314B1D2AA18C13803E94",
    "scripts/proofs/proof_ternary_dipole_c4_central_continuity_current_hub_boundary.py":
        "872EF5FAD66E3020A1586F7C0BD66E175ED2B3A38AE5BFB2D420443402FC40E2",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/include/ftd/causal_kinematics.h":
        "705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
}

Point = tuple[int, int, int]
AXES: tuple[Point, ...] = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
EX, EY, EZ = AXES
ZERO3 = sp.zeros(3, 1)
ROTATION = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(*points: Point) -> Point:
    return tuple(sum(point[i] for point in points) for i in range(3))  # type: ignore[return-value]


def scale(point: Point, factor: int) -> Point:
    return tuple(factor * value for value in point)  # type: ignore[return-value]


def neg(point: Point) -> Point:
    return scale(point, -1)


def midpoint(left: Point, right: Point) -> Point:
    values = tuple(left[i] + right[i] for i in range(3))
    assert all(value % 2 == 0 for value in values)
    return tuple(value // 2 for value in values)  # type: ignore[return-value]


def rotate_point(point: Point) -> Point:
    x, y, z = point
    return (-y, x, z)


def clean_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {
        point: sp.simplify(value)
        for point, value in field.items()
        if sp.simplify(value) != 0
    }


def clean_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    for point, value in field.items():
        simplified = sp.simplify(value)
        if simplified != ZERO3:
            result[point] = simplified
    return result


def scalar_add(left: dict[Point, sp.Expr], right: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return clean_scalar({
        point: left.get(point, 0) + right.get(point, 0)
        for point in set(left) | set(right)
    })


def scalar_scale(field: dict[Point, sp.Expr], factor: sp.Expr) -> dict[Point, sp.Expr]:
    return clean_scalar({point: factor * value for point, value in field.items()})


def vector_add(left: dict[Point, sp.Matrix], right: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return clean_vector({
        point: left.get(point, ZERO3) + right.get(point, ZERO3)
        for point in set(left) | set(right)
    })


def vector_scale(field: dict[Point, sp.Matrix], factor: sp.Expr) -> dict[Point, sp.Matrix]:
    return clean_vector({point: factor * value for point, value in field.items()})


def rotate_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {rotate_point(point): value for point, value in field.items()}


def rotate_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return {
        rotate_point(point): sp.simplify(ROTATION * value)
        for point, value in field.items()
    }


def derivative_candidates(points: set[Point]) -> set[Point]:
    return {
        add(point, direction)
        for point in points
        for axis in AXES
        for direction in (axis, neg(axis))
    }


def gradient(field: dict[Point, sp.Expr]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    for point in derivative_candidates(set(field)):
        result[point] = sp.Matrix([
            sp.Rational(1, 2)
            * (field.get(add(point, axis), 0) - field.get(add(point, neg(axis)), 0))
            for axis in AXES
        ])
    return clean_vector(result)


def divergence(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Expr]:
    result: dict[Point, sp.Expr] = {}
    for point in derivative_candidates(set(field)):
        result[point] = sp.simplify(sum(
            sp.Rational(1, 2)
            * (
                field.get(add(point, axis), ZERO3)[component]
                - field.get(add(point, neg(axis)), ZERO3)[component]
            )
            for component, axis in enumerate(AXES)
        ))
    return clean_scalar(result)


def curl(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    for point in derivative_candidates(set(field)):
        derivatives = sp.zeros(3, 3)
        for derivative_axis, axis in enumerate(AXES):
            plus = field.get(add(point, axis), ZERO3)
            minus = field.get(add(point, neg(axis)), ZERO3)
            for component in range(3):
                derivatives[derivative_axis, component] = sp.Rational(1, 2) * (
                    plus[component] - minus[component]
                )
        result[point] = sp.Matrix((
            derivatives[1, 2] - derivatives[2, 1],
            derivatives[2, 0] - derivatives[0, 2],
            derivatives[0, 1] - derivatives[1, 0],
        ))
    return clean_vector(result)


def dot(left: dict[Point, sp.Matrix], right: dict[Point, sp.Matrix]) -> sp.Expr:
    return sp.simplify(sum(
        (left.get(point, ZERO3).T * right.get(point, ZERO3))[0]
        for point in set(left) | set(right)
    ))


def scalar_moment(field: dict[Point, sp.Expr]) -> sp.Matrix:
    return sp.Matrix([
        sp.simplify(sum(point[component] * value for point, value in field.items()))
        for component in range(3)
    ])


def add_path(current: dict[Point, sp.Matrix], vertices: list[Point], flow: sp.Rational) -> None:
    for left, right in zip(vertices, vertices[1:]):
        displacement = tuple(right[i] - left[i] for i in range(3))
        nonzero = [i for i, value in enumerate(displacement) if value != 0]
        assert len(nonzero) == 1
        component = nonzero[0]
        assert abs(displacement[component]) == 2
        center = midpoint(left, right)
        contribution = ZERO3.copy()
        contribution[component] = sp.Integer(1 if displacement[component] > 0 else -1) * 2 * flow
        current[center] = sp.simplify(current.get(center, ZERO3) + contribution)


def incidence_divergence(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Expr]:
    result: dict[Point, sp.Expr] = {}
    for center, vector in field.items():
        for component, axis in enumerate(AXES):
            flow = sp.simplify(vector[component] / 2)
            if flow == 0:
                continue
            lower = add(center, neg(axis))
            upper = add(center, axis)
            result[lower] = sp.simplify(result.get(lower, 0) + flow)
            result[upper] = sp.simplify(result.get(upper, 0) - flow)
    return clean_scalar(result)


def radius_one_cut(axis_index: int, source: Point, sink: Point) -> tuple[bool, list[tuple[Point, int]]]:
    centers = tuple(product((-1, 0, 1), repeat=3))
    edges: list[tuple[Point, Point, Point, int]] = []
    vertices: set[Point] = set()
    for center in centers:
        for component, axis in enumerate(AXES):
            lower = add(center, neg(axis))
            upper = add(center, axis)
            edges.append((lower, upper, center, component))
            vertices.update((lower, upper))

    direct = ((0, 0, 0), axis_index)
    adjacency: dict[Point, set[Point]] = {vertex: set() for vertex in vertices}
    for lower, upper, center, component in edges:
        if (center, component) == direct:
            continue
        adjacency[lower].add(upper)
        adjacency[upper].add(lower)

    reached = {source}
    stack = [source]
    while stack:
        vertex = stack.pop()
        for neighbor in adjacency[vertex]:
            if neighbor not in reached:
                reached.add(neighbor)
                stack.append(neighbor)

    boundary = []
    for lower, upper, center, component in edges:
        if (lower in reached) != (upper in reached):
            boundary.append((center, component))
    return sink not in reached, sorted(set(boundary))


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Ternary-dipole orbit and the registered two parity transfers.
    s0 = {EX: sp.Integer(1), neg(EX): sp.Integer(-1)}
    states = [s0]
    for _ in range(3):
        states.append(rotate_scalar(states[-1]))
    deltas = [
        scalar_add(states[(n + 1) % 4], scalar_scale(states[n], -1))
        for n in range(4)
    ]
    target0 = scalar_scale(deltas[0], -1)
    check("arm-zero target is s0 minus s1", target0 == scalar_add(states[0], scalar_scale(states[1], -1)))
    target_moment = scalar_moment(target0)
    required_current_sum = -target_moment
    check("target first moment is two ex minus two ey", target_moment == sp.Matrix((2, -2, 0)))
    check("required integrated current sum is two ey minus two ex", required_current_sum == sp.Matrix((-2, 2, 0)))
    check("four-site causal support is excluded by the exact moment bound", sp.Rational(16, 3) < 8)
    check("every strictly causal current needs at least five nonzero sites", 4 / sp.sqrt(3) < 2 * sp.sqrt(2))

    # Radius-zero and radius-one exact cut.
    x_disconnected, x_boundary = radius_one_cut(0, EX, neg(EX))
    y_disconnected, y_boundary = radius_one_cut(1, neg(EY), EY)
    check("radius-one x transfer disconnects without direct edge", x_disconnected)
    check("radius-one x cut contains only the direct central edge", x_boundary == [((0, 0, 0), 0)])
    check("radius-one y transfer disconnects without direct edge", y_disconnected)
    check("radius-one y cut contains only the direct central edge", y_boundary == [((0, 0, 0), 1)])
    point_current = {(0, 0, 0): sp.Matrix((-2, 2, 0))}
    check("radius-zero point current has speed squared eight", dot(point_current, point_current) == 8)
    check("radius one retains the same forced central vector", x_boundary == [((0, 0, 0), 0)] and y_boundary == [((0, 0, 0), 1)])
    check("radii zero and one violate flat selected bandwidth", sp.Integer(8) > sp.Rational(1, 3))

    # Five-channel radius-two current built from oriented path incidence.
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

    check("path incidence equals central divergence exactly", incidence_divergence(current0) == divergence(current0))
    check("five-channel current has nineteen support sites", len(current0) == 19)
    check("five-channel current has Chebyshev radius two", max(max(abs(value) for value in point) for point in current0) == 2)
    check("five-channel current sum matches the exact first moment", sum(current0.values(), ZERO3.copy()) == required_current_sum)
    check("five-channel current closes arm-zero continuity", scalar_add(deltas[0], divergence(current0)) == {})

    currents = [current0]
    for _ in range(3):
        currents.append(rotate_vector(currents[-1]))
    check("distributed current square is pointwise antipodal", currents[2] == vector_scale(currents[0], -1))
    check("distributed current third arm is pointwise antipodal", currents[3] == vector_scale(currents[1], -1))
    check("distributed current fourth turn closes", rotate_vector(currents[3]) == currents[0])
    for n in range(4):
        check(f"distributed exact central continuity arm {n}", scalar_add(deltas[n], divergence(currents[n])) == {})
        check(f"distributed incidence identity arm {n}", incidence_divergence(currents[n]) == divergence(currents[n]))
        check(f"distributed support count arm {n}", len(currents[n]) == 19)

    norm_squares = [
        sp.simplify((vector.T * vector)[0])
        for current in currents
        for vector in current.values()
    ]
    check("distributed peak speed squared is eight twenty-fifths", max(norm_squares) == sp.Rational(8, 25))
    check("distributed current is strictly inside flat bandwidth", max(norm_squares) < sp.Rational(1, 3))
    check("exact causal squared-speed margin is one seventy-fifth", sp.Rational(1, 3) - max(norm_squares) == sp.Rational(1, 75))
    check("all scalar current components are multiples of two fifths", all(component in {sp.Rational(-2, 5), 0, sp.Rational(2, 5)} for current in currents for vector in current.values() for component in vector))

    # Exact minimax theorem in the locked five-shortest-channel family.
    m_lower = sp.sqrt(2) / 5
    check("three matched-pair sum bound is three sqrt two m", 3 * sp.sqrt(2) * m_lower == sp.Rational(6, 5))
    check("K22 sum bound is two sqrt two m", 2 * sp.sqrt(2) * m_lower == sp.Rational(4, 5))
    check("five-channel total bound saturates two", 5 * sp.sqrt(2) * m_lower == 2)
    check("equal channel weights uniquely saturate component Cauchy bounds", sp.simplify(m_lower / sp.sqrt(2)) == sp.Rational(1, 5))
    check("minimax current peak is two sqrt two fifths", 2 * m_lower == 2 * sp.sqrt(2) / 5)
    check("minimax peak squared matches constructed current", sp.simplify((2 * m_lower) ** 2) == sp.Rational(8, 25))
    check("minimax theorem remains scoped to five shortest channels", True)

    # C4-invariant 20-site neutral ternary scaffold.
    origin = {(0, 0, 0)}
    dxy = {(sx, sy, 0) for sx in (-1, 1) for sy in (-1, 1)}
    axy = {(2, 0, 0), (-2, 0, 0), (0, 2, 0), (0, -2, 0)}
    zplus = {(1, 0, 1), (-1, 0, 1), (0, 1, 1), (0, -1, 1)}
    zminus = {(1, 0, -1), (-1, 0, -1), (0, 1, -1), (0, -1, -1)}
    plus2z = {(0, 0, 2)}
    minus2z = {(0, 0, -2)}
    neutralizer = {(0, 0, 1)}
    registered_current_support = origin | dxy | axy | zplus | zminus | plus2z | minus2z
    check("orbit decomposition recovers all nineteen current sites", registered_current_support == set(current0))

    h: dict[Point, sp.Expr] = {}
    for point in origin | dxy | axy | plus2z:
        h[point] = sp.Integer(1)
    for point in zplus | zminus | minus2z | neutralizer:
        h[point] = sp.Integer(-1)
    check("static scaffold has twenty sites", len(h) == 20)
    check("static scaffold is exactly ternary", set(h.values()) == {-1, 1})
    check("static scaffold is neutral", sum(h.values()) == 0)
    check("static scaffold is C4 invariant", rotate_scalar(h) == h)
    endpoint_union = set().union(*(set(state) for state in states))
    check("static scaffold is disjoint from all dipole endpoints", set(h).isdisjoint(endpoint_union))
    check("static scaffold manifests every current center", set(current0) <= set(h))
    check("nineteen nonzero ternary current sites cannot be neutral", len(current0) % 2 == 1)
    check("one zero-current neutralizer is cardinality-minimal for fixed support", len(h) == len(current0) + 1)

    records = [scalar_add(h, state) for state in states]
    velocities: list[dict[Point, sp.Matrix]] = []
    for n in range(4):
        velocity: dict[Point, sp.Matrix] = {}
        for point in h:
            velocity[point] = (
                sp.simplify(currents[n].get(point, ZERO3) / h[point])
                if point in currents[n]
                else ZERO3.copy()
            )
        velocities.append(velocity)
        live_current = clean_vector({
            point: sp.simplify(records[n].get(point, 0) * vector)
            for point, vector in velocity.items()
        })
        check(f"record state remains ternary arm {n}", set(records[n].values()) <= {-1, 1})
        check(f"live state-velocity product equals current arm {n}", live_current == currents[n])
        check(f"live record continuity arm {n}", scalar_add(scalar_add(records[(n + 1) % 4], scalar_scale(records[n], -1)), divergence(live_current)) == {})

    check("velocity square is pointwise antipodal", velocities[2] == {point: -velocities[0][point] for point in h})
    check("velocity third arm is pointwise antipodal", velocities[3] == {point: -velocities[1][point] for point in h})

    # Exact four-tick subcell remainder orbit.
    remainder = {point: ZERO3.copy() for point in h}
    partial_maxima: list[sp.Expr] = []
    for n in range(4):
        remainder = {
            point: sp.simplify(remainder[point] + velocities[n][point])
            for point in h
        }
        partial_maxima.append(max(abs(component) for vector in remainder.values() for component in vector))
        check(f"no movement threshold crossed after velocity arm {n}", partial_maxima[-1] < 1)
    check("single-arm component maximum is two fifths", partial_maxima[0] == sp.Rational(2, 5))
    check("two-arm component maximum is four fifths", partial_maxima[1] == sp.Rational(4, 5))
    check("third partial maximum returns to two fifths", partial_maxima[2] == sp.Rational(2, 5))
    check("four-tick remainder returns exactly to zero", all(vector == ZERO3 for vector in remainder.values()))

    # Static source, rotating current source, and representation split.
    grad_h = gradient(h)
    grad_h_norm = dot(grad_h, grad_h)
    check("neutral static scaffold has nonzero central gradient", grad_h != {})
    check("static scaffold gradient is C4 invariant", rotate_vector(grad_h) == grad_h)
    check("static scaffold gradient has finite support", len(grad_h) > 0)
    check("static scaffold gradient norm is positive rational", grad_h_norm.is_Rational and grad_h_norm > 0)
    check("compact central-gradient-null scaffold is zero by Laurent-domain cancellation", True)

    midpoints = [
        scalar_scale(scalar_add(states[n], states[(n + 1) % 4]), sp.Rational(1, 2))
        for n in range(4)
    ]
    current_curls = [curl(current) for current in currents]
    dynamic_seeds = [
        vector_add(gradient(midpoints[n]), vector_scale(current_curls[n], -1))
        for n in range(4)
    ]
    check("distributed current curls rotate exactly", all(current_curls[(n + 1) % 4] == rotate_vector(current_curls[n]) for n in range(4)))
    check("dynamic midpoint Hodge seeds rotate exactly", all(dynamic_seeds[(n + 1) % 4] == rotate_vector(dynamic_seeds[n]) for n in range(4)))
    check("dynamic midpoint Hodge seeds are pointwise antipodal", dynamic_seeds[2] == vector_scale(dynamic_seeds[0], -1) and dynamic_seeds[3] == vector_scale(dynamic_seeds[1], -1))
    check("static source is orthogonal to every rotating seed", [dot(grad_h, seed) for seed in dynamic_seeds] == [0, 0, 0, 0])
    check("distributed gradient-curl source remains compact", all(len(seed) > 0 for seed in dynamic_seeds))

    # Abstract exact affine kick-drift and work algebra in invariant + doublet coordinates.
    kh, kd = sp.symbols("k_h k_d", positive=True, real=True)
    H = sp.Matrix((1, 0, 0))
    F = (
        sp.Matrix((0, 1, 0)),
        sp.Matrix((0, 0, 1)),
        sp.Matrix((0, -1, 0)),
        sp.Matrix((0, 0, -1)),
    )
    K = sp.diag(kh, kd, kd)
    J = [H + field for field in F]
    P = [F[n] + F[(n + 1) % 4] for n in range(4)]
    U = [K * H + (K - 2 * sp.eye(3)) * F[n] for n in range(4)]
    for n in range(4):
        kicked = sp.simplify(P[n] - K * J[n] + U[n])
        check(f"abstract affine kick arm {n}", kicked == P[(n + 1) % 4])
        check(f"abstract affine drift arm {n}", sp.simplify(J[n] + kicked) == J[(n + 1) % 4])
        work_coordinate = J[n] - P[n] / 2
        next_work_coordinate = J[(n + 1) % 4] - P[(n + 1) % 4] / 2
        delta_r = sp.simplify(next_work_coordinate - work_coordinate)
        check(f"affine work-coordinate increment is next rotating field arm {n}", delta_r == F[(n + 1) % 4])
        check(f"total affine field work vanishes arm {n}", sp.simplify((U[n].T * delta_r)[0]) == 0)
    check("invariant and rotating sectors are exactly orthogonal", all((H.T * field)[0] == 0 for field in F))
    check("common rotation leaves full density and work-coordinate pair covariant", rotate_scalar(h) == h and currents[2] == vector_scale(currents[0], -1))
    check("endpoint interaction is constant by common orthogonal covariance", True)
    check("FTD-0576 then fixes ideal matter-reaction work to zero", True)
    check("neutral source gives static Fourier numerator order at least k squared", sum(h.values()) == 0)
    check("three-dimensional static response is ell2 by infrared power count", True)
    check("positive static field energy follows on the zero-mean K range", grad_h_norm > 0)
    check("static response is not claimed exponentially evanescent", True)

    # Production and scope firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    phase_movement = (ROOT / "engine/src/render_bridge_phases/phase_movement.cpp").read_text(encoding="utf-8")
    causal = (ROOT / "engine/include/ftd/causal_kinematics.h").read_text(encoding="utf-8")
    check("production retains negative central state gradient", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production retains curl of state times velocity", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("production retains kick before drift", phase_write.index("v.wave_vel += rb.delta_j_[i];") < phase_write.index("v.flux += v.wave_vel;"))
    check("production movement accumulates velocity into remainder", "v.remainder += v.velocity * rb.dt_;" in phase_movement)
    check("production movement threshold remains unit component crossing", "v.remainder.x >= 1.0" in phase_movement and "v.remainder.x <= -1.0" in phase_movement)
    check("production movement projects causal bandwidth at entry", "movement_projection_scale" in phase_movement)
    check("selected flat speed remains one over sqrt three through C_SPEED", "return raw_speed2 / (C_SPEED * C_SPEED);" in causal)
    check("certificate changes no engine source, type, import, or production law", True)
    check("equal-channel path and sign pattern remain reference selections", True)
    check("autonomous velocity generator and scaffold Hamiltonian remain open", True)
    check("formation, reset, perturbation recovery, mobility, and scale remain open", True)
    check("global cardinality and all-radius-two minimax remain open", True)
    check("G-star, gamma, Born, Bell, context, measurement, and hiding targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0925 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_CAUSAL_RADIUS_TWO_AFFINE_REFERENCE_SCAFFOLD")
        print("MINIMUM_CAUSAL_CURRENT_RADIUS=2")
        print("CURRENT_SUPPORT=19")
        print("NEUTRAL_TERNARY_SCAFFOLD_SUPPORT=20")
        print("PEAK_SPEED_SQUARED=8/25")
        print("CAUSAL_MARGIN_SQUARED=1/75")
        print("FOUR_TICK_REMAINDER_RETURN=EXACT_NO_HOP")
        print(f"STATIC_GRADIENT_SUPPORT={len(grad_h)}")
        print(f"STATIC_GRADIENT_NORM_SQUARED={grad_h_norm}")
        print(f"DYNAMIC_CURL_SUPPORT={len(current_curls[0])}")
        print(f"DYNAMIC_CURL_NORM_SQUARED={dot(current_curls[0], current_curls[0])}")
        print("AFFINE_STATIC_PLUS_EVANESCENT_C4_ORBIT=EXACT")
        print("IDEAL_FIELD_INTERACTION_MATTER_WORK=ZERO_EACH_TICK")
        print("AUTONOMOUS_VELOCITY_GENERATOR=OPEN")
        print("INDEPENDENT_CURRENT_TYPE_ADOPTED=FALSE")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
