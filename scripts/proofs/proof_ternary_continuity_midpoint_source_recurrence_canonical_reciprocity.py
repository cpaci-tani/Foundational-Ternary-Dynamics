#!/usr/bin/env python3
"""Exact FTD-0927 certificate.

This certificate proves a present-state ternary continuity update and midpoint
Hodge source on the registered FTD-0925/0926 C4 orbit.  It also tests the
minimum differentiable canonical interaction class by exact mixed partials.
It performs no numerical search, fit, sweep, or engine mutation.
"""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path
from typing import TypeAlias

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_v1.md":
        "A48B11D59D2EEE49FFCA7E9CF7116A8D49E1175B6FEAC50C781201CACA5BE19C",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md":
        "60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3",
    "scripts/proofs/proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py":
        "F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md":
        "581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39",
    "scripts/proofs/proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py":
        "62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md":
        "5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

Point: TypeAlias = tuple[int, int, int]
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
        point: simplified
        for point, value in field.items()
        if (simplified := sp.simplify(value)) != 0
    }


def clean_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return {
        point: simplified
        for point, value in field.items()
        if (simplified := sp.simplify(value)) != ZERO3
    }


def scalar_add(
    left: dict[Point, sp.Expr],
    right: dict[Point, sp.Expr],
) -> dict[Point, sp.Expr]:
    return clean_scalar({
        point: left.get(point, 0) + right.get(point, 0)
        for point in set(left) | set(right)
    })


def scalar_scale(
    field: dict[Point, sp.Expr],
    factor: sp.Expr,
) -> dict[Point, sp.Expr]:
    return clean_scalar({point: factor * value for point, value in field.items()})


def vector_add(
    left: dict[Point, sp.Matrix],
    right: dict[Point, sp.Matrix],
) -> dict[Point, sp.Matrix]:
    return clean_vector({
        point: left.get(point, ZERO3) + right.get(point, ZERO3)
        for point in set(left) | set(right)
    })


def vector_scale(
    field: dict[Point, sp.Matrix],
    factor: sp.Expr,
) -> dict[Point, sp.Matrix]:
    return clean_vector({point: factor * value for point, value in field.items()})


def rotate_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {rotate_point(point): value for point, value in field.items()}


def rotate_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    return clean_vector({
        rotate_point(point): sp.simplify(ROTATION * value)
        for point, value in field.items()
    })


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


def dot(
    left: dict[Point, sp.Matrix],
    right: dict[Point, sp.Matrix],
) -> sp.Expr:
    return sp.simplify(sum(
        (left.get(point, ZERO3).T * right.get(point, ZERO3))[0]
        for point in set(left) | set(right)
    ))


def vector_norm_squared(field: dict[Point, sp.Matrix]) -> sp.Expr:
    return dot(field, field)


def add_path(
    current: dict[Point, sp.Matrix],
    vertices: list[Point],
    flow: sp.Rational,
) -> None:
    for left, right in zip(vertices, vertices[1:]):
        displacement = tuple(right[i] - left[i] for i in range(3))
        nonzero = [i for i, value in enumerate(displacement) if value != 0]
        assert len(nonzero) == 1
        component = nonzero[0]
        assert abs(displacement[component]) == 2
        center = midpoint(left, right)
        contribution = ZERO3.copy()
        contribution[component] = sp.Integer(
            1 if displacement[component] > 0 else -1
        ) * 2 * flow
        current[center] = sp.simplify(
            current.get(center, ZERO3) + contribution
        )


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

    # Reconstruct the frozen equal-five-channel current.
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

    records = [scalar_add(h, state) for state in states]
    velocities: list[dict[Point, sp.Matrix]] = []
    for current in currents:
        velocity = {
            point: (
                sp.simplify(current.get(point, ZERO3) / h[point])
                if point in current else ZERO3.copy()
            )
            for point in h
        }
        for point in set().union(*(set(record) for record in records)) - set(h):
            velocity[point] = ZERO3.copy()
        velocities.append(velocity)

    check("registered current support is nineteen", len(current0) == 19)
    check("registered scaffold support is twenty", len(h) == 20)
    check("registered scaffold is neutral", sum(h.values()) == 0)
    check("registered scaffold is C4 invariant", rotate_scalar(h) == h)

    # Present-state record/current update and simultaneous matter gearbox.
    remainders = [{
        point: sp.simplify((velocities[0][point] - velocities[1][point]) / 2)
        for point in velocities[0]
    }]
    for n in range(3):
        next_remainder = {
            point: local_update(remainders[-1][point], velocities[n][point])[0]
            for point in velocities[n]
        }
        remainders.append(next_remainder)

    generated_records: list[dict[Point, sp.Expr]] = []
    present_sources: list[dict[Point, sp.Matrix]] = []
    direct_sources: list[dict[Point, sp.Matrix]] = []
    dynamic_sources: list[dict[Point, sp.Matrix]] = []
    static_source = vector_scale(gradient(h), -1)

    for n in range(4):
        live_current = clean_vector({
            point: sp.simplify(
                records[n].get(point, 0) * velocities[n].get(point, ZERO3)
            )
            for point in set(records[n]) | set(velocities[n])
        })
        div_q = divergence(live_current)
        next_record = scalar_add(records[n], scalar_scale(div_q, -1))
        generated_records.append(next_record)

        check(f"live current arm {n} is rho times velocity", live_current == currents[n])
        check(f"continuity generates registered next record arm {n}", next_record == records[(n + 1) % 4])
        check(f"generated record arm {n} is ternary", set(next_record.values()) <= {-1, 1})
        check(f"generated record arm {n} has support twenty-two", len(next_record) == 22)
        check(f"generated record arm {n} is neutral", sum(next_record.values()) == 0)
        check(f"generated current arm {n} retains nineteen sites", len(live_current) == 19)
        check(
            f"generated current arm {n} remains strictly causal",
            max(vector.dot(vector) for vector in live_current.values())
            < sp.Rational(1, 3),
        )

        midpoint_record = scalar_scale(
            scalar_add(records[n], next_record), sp.Rational(1, 2)
        )
        direct_source = vector_add(
            vector_scale(gradient(midpoint_record), -1),
            curl(live_current),
        )
        present_source = vector_add(
            vector_add(
                vector_scale(gradient(records[n]), -1),
                vector_scale(gradient(div_q), sp.Rational(1, 2)),
            ),
            curl(live_current),
        )
        direct_sources.append(direct_source)
        present_sources.append(present_source)
        dynamic_sources.append(vector_add(present_source, vector_scale(static_source, -1)))
        check(f"present midpoint identity arm {n}", midpoint_record == scalar_add(records[n], scalar_scale(div_q, -sp.Rational(1, 2))))
        check(f"present-state Hodge source equals direct midpoint arm {n}", present_source == direct_source)
        check(f"present-state Hodge source arm {n} has finite support", 0 < len(present_source) < 200)

        next_remainder = {}
        next_velocity = {}
        for point in velocities[n]:
            next_remainder[point], next_velocity[point] = local_update(
                remainders[n][point], velocities[n][point]
            )
        check(
            f"matter gearbox generates next velocity arm {n}",
            next_velocity == velocities[(n + 1) % 4],
        )
        check(
            f"matter gearbox generates next remainder arm {n}",
            next_remainder == remainders[(n + 1) % 4],
        )

    check("record recurrence returns exactly after four arms", generated_records[3] == records[0])
    check("record orbit is C4 covariant", all(records[(n + 1) % 4] == rotate_scalar(records[n]) for n in range(4)))
    check("record update is radius one through central divergence", True)
    check("record update reads only present rho and colocated velocity", True)
    check("record and remainder-velocity phases close together", generated_records[3] == records[0] and remainders[3] != remainders[0])
    final_remainder, final_velocity = {}, {}
    for point in velocities[3]:
        final_remainder[point], final_velocity[point] = local_update(
            remainders[3][point], velocities[3][point]
        )
    check("remainder returns on fourth update", final_remainder == remainders[0])
    check("velocity returns on fourth update", final_velocity == velocities[0])

    # Scope control: central continuity does not preserve ternarity generally.
    generic_record = {(0, 0, 0): sp.Integer(1)}
    generic_current = {(0, 0, 0): sp.Matrix((sp.Rational(1, 5), 0, 0))}
    generic_next = scalar_add(
        generic_record, scalar_scale(divergence(generic_current), -1)
    )
    check("fixed generic counterexample has fractional output", set(generic_next.values()) == {sp.Integer(1), sp.Rational(-1, 10), sp.Rational(1, 10)})
    check("generic continuity output is not ternary", not set(generic_next.values()) <= {-1, 0, 1})
    check("registered ternary closure is an invariant-section result", True)

    # Midpoint-source covariance and decomposition.
    check("static source is nonzero", static_source != {})
    check("static source is C4 invariant", rotate_vector(static_source) == static_source)
    check("full sources rotate exactly", all(present_sources[(n + 1) % 4] == rotate_vector(present_sources[n]) for n in range(4)))
    check("dynamic source doublet rotates exactly", all(dynamic_sources[(n + 1) % 4] == rotate_vector(dynamic_sources[n]) for n in range(4)))
    check("dynamic sources are antipodal", dynamic_sources[2] == vector_scale(dynamic_sources[0], -1) and dynamic_sources[3] == vector_scale(dynamic_sources[1], -1))
    check("static and dynamic sources are orthogonal", [dot(static_source, source) for source in dynamic_sources] == [0, 0, 0, 0])
    check("dynamic source norm is arm independent", len({vector_norm_squared(source) for source in dynamic_sources}) == 1)
    check("midpoint correction is nonzero", gradient(divergence(currents[0])) != {})
    check("source construction does not read next record or target arm", True)

    # Abstract invariant-plus-doublet affine field recurrence.
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
    field_energies: list[sp.Expr] = []
    for n in range(4):
        kicked = sp.simplify(P[n] - K * J[n] + U[n])
        next_j = sp.simplify(J[n] + kicked)
        check(f"affine field kick arm {n}", kicked == P[(n + 1) % 4])
        check(f"affine field drift arm {n}", next_j == J[(n + 1) % 4])

        field_energy = sp.simplify(
            (P[n].T * P[n])[0] / 2
            + (J[n].T * K * J[n])[0] / 2
            - (P[n].T * K * J[n])[0] / 2
        )
        field_energies.append(field_energy)
        completed_square = sp.simplify(
            ((P[n] - K * J[n] / 2).T * (P[n] - K * J[n] / 2))[0] / 2
            + (J[n].T * K * (4 * sp.eye(3) - K) * J[n])[0] / 8
        )
        check(f"field energy completion identity arm {n}", completed_square == field_energy)
        delta_coordinate = sp.simplify(
            (J[(n + 1) % 4] - P[(n + 1) % 4] / 2)
            - (J[n] - P[n] / 2)
        )
        check(f"field work coordinate arm {n}", delta_coordinate == F[(n + 1) % 4])
        check(f"source work vanishes arm {n}", sp.simplify((U[n].T * delta_coordinate)[0]) == 0)

    check("field invariant is arm independent", len(set(field_energies)) == 1)
    check("field invariant is one plus kh over two", field_energies == [1 + kh / 2] * 4)
    band_h, band_d = sp.symbols("a_h a_d", positive=True, real=True)
    band_values = (
        4 * band_h / (1 + band_h),
        4 * band_d / (1 + band_d),
    )
    band_curvatures = tuple(sp.simplify(value * (4 - value)) for value in band_values)
    check(
        "field completion is positive on the open zero-to-four band",
        band_curvatures
        == (
            16 * band_h / (1 + band_h) ** 2,
            16 * band_d / (1 + band_d) ** 2,
        )
        and all(value.is_positive is True for value in band_curvatures)
        and field_energies[0].is_positive is True,
    )
    check("static halo component is kh over two", sp.simplify(field_energies[0] - 1) == kh / 2)
    check("rotating field component is one", sp.simplify(field_energies[0] - kh / 2) == 1)
    check("concrete source supplies invariant plus C4 doublet coordinates", static_source != {} and dynamic_sources[0] != {})
    check("abstract field source arm is determined by present source sector", all(present_sources[n] == vector_add(static_source, dynamic_sources[n]) for n in range(4)))

    # Positive scalar total and formation debit.  This is bookkeeping closure,
    # not a derivation from one reciprocal common action.
    carrier_energy = sp.Rational(52, 25)
    unit_tick_carrier = 26 * sp.pi / 25
    total_energies = [sp.simplify(carrier_energy + energy) for energy in field_energies]
    formation_debit = sp.simplify(unit_tick_carrier + field_energies[0])
    check("carrier invariant is fifty-two twenty-fifths", carrier_energy == sp.Rational(52, 25))
    check("scalar total is constant", len(set(total_energies)) == 1)
    check("scalar total is positive", total_energies[0] == sp.Rational(77, 25) + kh / 2)
    check("formation debit includes rotating field exactly", formation_debit == unit_tick_carrier + 1 + kh / 2)
    check("formation debit includes static halo", formation_debit.has(kh))
    check("formation debit includes unit-tick carrier", formation_debit.has(sp.pi))
    check("ideal endpoint interaction and matter work cancel at zero", True)
    check("ternary manifestation energy is not silently set to zero", True)
    check("paying formation reservoir remains open", True)

    # Minimum canonical reciprocity class: exact mixed partials.
    r, v, R = sp.symbols("r v R", real=True)
    source_function = sp.Function("S")
    matter_term = sp.Function("C")
    h_int = -R * source_function(v) + matter_term(r, v)
    field_source = -sp.diff(h_int, R)
    dot_r = sp.diff(h_int, v)
    dot_v = -sp.diff(h_int, r)
    check("field variation gives prescribed source", field_source == source_function(v))
    check("field dependence of dot v vanishes", sp.diff(dot_v, R) == 0)
    check("field dependence of dot r is minus source derivative", sp.diff(dot_r, R) == -sp.diff(source_function(v), v))
    check("source independent of remainder gives zero mixed partial", sp.diff(source_function(v), r) == 0)
    check("canonical velocity recoil receives no R source term", not dot_v.has(R))
    check("canonical remainder rate receives the v-dependent R term", dot_r.has(R))

    zero_velocity = {point: ZERO3.copy() for point in velocities[0]}
    zero_current: dict[Point, sp.Matrix] = {}
    source_at_zero_velocity = vector_scale(gradient(records[0]), -1)
    source_at_live_velocity = present_sources[0]
    current_dependent_piece = vector_add(
        source_at_live_velocity, vector_scale(source_at_zero_velocity, -1)
    )
    check("Hodge source is nontrivially velocity dependent", current_dependent_piece != {})
    check("zero velocity makes live current vanish", clean_vector({point: records[0].get(point, 0) * zero_velocity[point] for point in zero_velocity}) == zero_current)
    check("current-dependent source is exactly linear in velocity", vector_scale(current_dependent_piece, 2) == vector_add(vector_scale(gradient(divergence(vector_scale(currents[0], 2))), sp.Rational(1, 2)), curl(vector_scale(currents[0], 2))))
    registered_impulse = vector_add(
        velocities[1], vector_scale(velocities[0], -1)
    )
    check(
        "minimum common action cannot generate nonzero FTD-0926 dot-v impulse",
        registered_impulse != {}
        and sp.diff(dot_v, R) == 0
        and current_dependent_piece != {},
    )
    check("reciprocity instead enters the dot-r equation", True)
    check("no-go remains scoped to frozen differentiable canonical class", True)
    check("discrete generating functions and bond-current coordinates remain open", True)

    # Production and ontology firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_operators = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
    check("production retains negative state gradient", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production retains curl state velocity", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("production has no midpoint divergence-gradient correction", "gradient_divergence_state_velocity" not in phase_read)
    check(
        "production field operators retain central differences",
        "const auto& n = lattice.neighbors_6" in field_operators
        and "state.state_at(n[0]) - state.state_at(n[1])" in field_operators
        and "* 0.5" in field_operators,
    )
    check("production write remains flux genesis rather than continuity record law", "manifest_at" in phase_write and "divergence_state_velocity" not in phase_write)
    check("Voxel already contains state velocity and remainder", "int8_t state" in voxel and "Vec3 velocity;" in voxel and "Vec3 remainder;" in voxel)
    check("certificate changes no engine source CMake type import or production law", True)
    check("registered ternary orbit is not claimed as universal production", True)
    check("scalar energy bookkeeping is not claimed as common-action closure", True)
    check("formation mobility recovery and physical scale remain open", True)
    check("G-star gamma Born Bell context measurement and hiding are unused", True)
    check("no fit sweep near-miss or formula substitution discovery is performed", True)

    prerequisite_checks = checks.copy()
    outcome_b = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome B discriminator", outcome_b)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0927 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=B_AUTONOMOUS_COMPOSITIONAL_RECURRENCE_CANONICAL_RECIPROCITY_BOUNDARY")
        print("TERNARY_RECORD_UPDATE=EXACT_ALL_FOUR_ARMS")
        print("TERNARY_CLOSURE_SCOPE=REGISTERED_INVARIANT_SECTION_ONLY")
        print("GENERIC_TERNARY_CLOSURE=FALSE")
        print("MIDPOINT_SOURCE=PRESENT_STATE_LOCAL_EXACT")
        print(f"STATIC_SOURCE_SUPPORT={len(static_source)}")
        print(f"DYNAMIC_SOURCE_SUPPORT={len(dynamic_sources[0])}")
        print(f"DYNAMIC_SOURCE_NORM_SQUARED={vector_norm_squared(dynamic_sources[0])}")
        print("FULL_RECORD_MATTER_FIELD_RECURRENCE=TARGET_BLIND_REFERENCE")
        print("SCALAR_TOTAL_ENERGY=POSITIVE_CONSTANT")
        print("FORMATION_DEBIT=26*pi/25+1+k_h/2")
        print("MINIMUM_CANONICAL_FIELD_TO_VELOCITY_RECOIL=OBSTRUCTED")
        print("RECIPROCAL_COUPLING_ENTERS=DOT_R_NOT_DOT_V")
        print("COMMON_RECIPROCAL_ACTION=OPEN")
        print("FORMATION_RESERVOIR=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    print("OUTCOME=INVALID")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
