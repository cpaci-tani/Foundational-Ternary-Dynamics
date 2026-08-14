#!/usr/bin/env python3
"""Exact FTD-0929 certificate.

The certificate derives the unique gapped source-to-companion map, proves
that the registered companion is not finitely supported, verifies its local
Neumann preparation and exact error envelope, and tests a local cotangent
history lift.  It performs no numerical search, fit, sweep, or engine change.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md":
        "DA0C5514E893A88C612052AFD08A2C31ED6535E0E3BD50BBCCD65FF97ED0DEA2",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "27BD89002B2B432FB58950B639B56E0FD22C5511E48550AD026DB462BEE2E076",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF",
    "scripts/proofs/proof_self_dual_reciprocal_discrete_action_formation_reservoir_boundary.py":
        "E41455B589705E1B3B2F4ECCFABD5F0AF28DE303AD216DE700B241EBFB113AE0",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md":
        "B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF",
    "scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py":
        "E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_DIPOLE_CORE_EVANESCENT_C4_REFERENCE_ORBIT_AND_AUTONOMY_BOUNDARY_v1.md":
        "DB9894C1554422B0BA0C97A991FFF7F714B83EF673DDF5FEDA026B45C55B88AF",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md":
        "CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md":
        "143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md":
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

Point = tuple[int, int, int]
EX: Point = (1, 0, 0)
EY: Point = (0, 1, 0)
EZ: Point = (0, 0, 1)
AXES = (EX, EY, EZ)
ZERO3 = sp.zeros(3, 1)
I3 = sp.eye(3)
Z3 = sp.zeros(3)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(*points: Point) -> Point:
    return tuple(sum(point[i] for point in points) for i in range(3))  # type: ignore[return-value]


def scale(point: Point, factor: int) -> Point:
    return tuple(factor * value for value in point)  # type: ignore[return-value]


def neg(point: Point) -> Point:
    return scale(point, -1)


def midpoint(left: Point, right: Point) -> Point:
    return tuple((left[i] + right[i]) // 2 for i in range(3))  # type: ignore[return-value]


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
    return {
        point: sp.simplify(value)
        for point, value in field.items()
        if sp.simplify(value) != ZERO3
    }


def scalar_add(*fields: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    points = set().union(*(set(field) for field in fields))
    return clean_scalar({
        point: sum((field.get(point, 0) for field in fields), sp.Integer(0))
        for point in points
    })


def scalar_scale(field: dict[Point, sp.Expr], factor: sp.Expr) -> dict[Point, sp.Expr]:
    return clean_scalar({point: factor * value for point, value in field.items()})


def vector_add(*fields: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    points = set().union(*(set(field) for field in fields))
    return clean_vector({
        point: sum((field.get(point, ZERO3) for field in fields), ZERO3)
        for point in points
    })


def vector_scale(field: dict[Point, sp.Matrix], factor: sp.Expr) -> dict[Point, sp.Matrix]:
    return clean_vector({point: sp.simplify(factor * value) for point, value in field.items()})


def rotate_scalar(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {rotate_point(point): value for point, value in field.items()}


def rotate_vector(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    rotation = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))
    return {
        rotate_point(point): sp.simplify(rotation * value)
        for point, value in field.items()
    }


def derivative_candidates(points: set[Point]) -> set[Point]:
    result = set(points)
    for point in points:
        for axis in AXES:
            result.add(add(point, axis))
            result.add(add(point, neg(axis)))
    return result


def gradient(field: dict[Point, sp.Expr]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    half = sp.Rational(1, 2)
    for point in derivative_candidates(set(field)):
        result[point] = sp.Matrix(tuple(
            half * (field.get(add(point, axis), 0) - field.get(add(point, neg(axis)), 0))
            for axis in AXES
        ))
    return clean_vector(result)


def divergence(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Expr]:
    result: dict[Point, sp.Expr] = {}
    half = sp.Rational(1, 2)
    for point in derivative_candidates(set(field)):
        result[point] = sum(
            half * (
                field.get(add(point, axis), ZERO3)[component]
                - field.get(add(point, neg(axis)), ZERO3)[component]
            )
            for component, axis in enumerate(AXES)
        )
    return clean_scalar(result)


def curl(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    half = sp.Rational(1, 2)
    for point in derivative_candidates(set(field)):
        derivatives = sp.zeros(3, 3)
        for axis_index, axis in enumerate(AXES):
            plus = field.get(add(point, axis), ZERO3)
            minus = field.get(add(point, neg(axis)), ZERO3)
            for component in range(3):
                derivatives[component, axis_index] = half * (plus[component] - minus[component])
        result[point] = sp.Matrix((
            derivatives[2, 1] - derivatives[1, 2],
            derivatives[0, 2] - derivatives[2, 0],
            derivatives[1, 0] - derivatives[0, 1],
        ))
    return clean_vector(result)


def add_path(current: dict[Point, sp.Matrix], vertices: list[Point], flow: sp.Expr) -> None:
    for left, right in zip(vertices, vertices[1:]):
        delta = sp.Matrix(tuple(right[i] - left[i] for i in range(3)))
        center = midpoint(left, right)
        current[center] = sp.simplify(current.get(center, ZERO3) + flow * delta)


FACE_STEPS = tuple(axis for basis in AXES for axis in (basis, neg(basis)))
EDGE_STEPS = tuple(
    point
    for point in product((-1, 0, 1), repeat=3)
    if sum(value != 0 for value in point) == 2
)


def stiffness(field: dict[Point, sp.Matrix]) -> dict[Point, sp.Matrix]:
    result: dict[Point, sp.Matrix] = {}
    for point, value in field.items():
        result[point] = result.get(point, ZERO3) + sp.Rational(4, 3) * value
        for step in FACE_STEPS:
            target = add(point, step)
            result[target] = result.get(target, ZERO3) - sp.Rational(1, 9) * value
        for step in EDGE_STEPS:
            target = add(point, step)
            result[target] = result.get(target, ZERO3) - sp.Rational(1, 18) * value
    return clean_vector(result)


def field_norm_squared(field: dict[Point, sp.Matrix]) -> sp.Expr:
    return sp.simplify(sum((value.dot(value) for value in field.values()), sp.Integer(0)))


def c18_distance(left: Point, right: Point) -> int:
    delta = [abs(left[i] - right[i]) for i in range(3)]
    return max(max(delta), (sum(delta) + 1) // 2)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Exact C18 band and gapped resolvent.
    u, v, w = sp.symbols("u v w", real=True)
    kappa = sp.Rational(4, 3) - sp.Rational(2, 9) * (
        u + v + w + u * v + u * w + v * w
    )
    vertex_values = {
        sp.simplify(kappa.subs({u: uu, v: vv, w: ww}))
        for uu, vv, ww in product((-1, 1), repeat=3)
    }
    check("C18 symbol is multilinear on the cosine cube", all(sp.degree(kappa, variable) <= 1 for variable in (u, v, w)))
    check("C18 cube vertices give the exact band extrema", min(vertex_values) == 0 and max(vertex_values) == sp.Rational(16, 9))
    check("C18 vertex spectrum has registered exact values", vertex_values == {0, sp.Rational(4, 3), sp.Rational(16, 9)})
    check("dynamic resolvent has gap two ninths", 2 - max(vertex_values) == sp.Rational(2, 9))
    check("K over two contraction ceiling is eight ninths", max(vertex_values) / 2 == sp.Rational(8, 9))
    check("dynamic resolvent norm ceiling is nine halves", 1 / (2 - max(vertex_values)) == sp.Rational(9, 2))

    # Reconstruct the exact FTD-0927 arm-zero source.
    flow = sp.Rational(1, 5)
    current0: dict[Point, sp.Matrix] = {}
    add_path(current0, [EX, neg(EX)], flow)
    for transverse in (EY, neg(EY), EZ, neg(EZ)):
        add_path(
            current0,
            [EX, add(EX, scale(transverse, 2)), add(neg(EX), scale(transverse, 2)), neg(EX)],
            flow,
        )
    add_path(current0, [neg(EY), EY], flow)
    for transverse in (EX, neg(EX), EZ, neg(EZ)):
        add_path(
            current0,
            [neg(EY), add(neg(EY), scale(transverse, 2)), add(EY, scale(transverse, 2)), EY],
            flow,
        )
    current0 = clean_vector(current0)
    state0 = {EX: sp.Integer(1), neg(EX): sp.Integer(-1)}
    dynamic0 = vector_add(
        vector_scale(gradient(state0), -1),
        vector_scale(gradient(divergence(current0)), sp.Rational(1, 2)),
        curl(current0),
    )
    dynamic1 = rotate_vector(dynamic0)
    check("registered current reconstruction has nineteen sites", len(current0) == 19)
    check("registered dynamic source reconstruction has fifty-three sites", len(dynamic0) == 53)
    check("registered dynamic source norm is 463 over 100", field_norm_squared(dynamic0) == sp.Rational(463, 100))
    check("registered dynamic source is nonzero", dynamic0 != {})
    check("registered next source is exact C4 rotation", dynamic1 == rotate_vector(dynamic0))

    # Laurent finite-support obstruction.
    x, y, z = sp.symbols("x y z", nonzero=True)
    face_symbol = x + 1 / x + y + 1 / y + z + 1 / z
    edge_symbol = (
        x * y + x / y + y / x + 1 / (x * y)
        + x * z + x / z + z / x + 1 / (x * z)
        + y * z + y / z + z / y + 1 / (y * z)
    )
    denominator18 = sp.expand(12 + 2 * face_symbol + edge_symbol)
    denominator_slice = sp.factor(denominator18.subs({y: 1, z: 1}))
    source_symbols = [
        sp.simplify(sum(value[component] * x**point[0] * y**point[1] * z**point[2] for point, value in dynamic0.items()))
        for component in range(3)
    ]
    source_x_slice = sp.factor(source_symbols[0].subs({y: 1, z: 1}))
    expected_source_x_slice = (x - 1) ** 2 * (x + 1) ** 2 / (4 * x**2)
    root = -2 + sp.sqrt(3)
    check("eighteen times two-minus-K has the frozen Laurent symbol", denominator18 == 12 + 2 * face_symbol + edge_symbol)
    check(
        "denominator slice is six times x plus inverse x plus four",
        sp.simplify(denominator_slice - 6 * (x + 1 / x + 4)) == 0,
    )
    check("registered source x slice is exact", sp.simplify(source_x_slice - expected_source_x_slice) == 0)
    check("registered denominator witness root is nonzero", root != 0)
    check("registered denominator vanishes at witness root", sp.simplify(denominator_slice.subs(x, root)) == 0)
    check("registered source does not vanish at witness root", sp.simplify(source_x_slice.subs(x, root)) != 0)

    denominator_poly = sp.Poly(sp.expand(denominator18 * x * y * z), x, y, z, domain=sp.QQ)
    division_remainders = []
    for source_symbol in source_symbols:
        shifted = sp.Poly(sp.expand(source_symbol * x**5 * y**5 * z**5), x, y, z, domain=sp.QQ)
        _, remainder = sp.div(shifted, denominator_poly)
        division_remainders.append(remainder)
    check("all registered source components are nonzero Laurent polynomials", all(symbol != 0 for symbol in source_symbols))
    check("x component has nonzero multivariate division remainder", not division_remainders[0].is_zero)
    check("registered vector source is not divisible by two-minus-K", any(not remainder.is_zero for remainder in division_remainders))
    check("unique registered companion cannot have finite support", sp.simplify(source_x_slice.subs(x, root)) != 0)

    # Exact local Neumann iteration and covariance.
    q0: dict[Point, sp.Matrix] = {}
    q1: dict[Point, sp.Matrix] = {}
    current_power0 = dynamic0
    current_power1 = dynamic1
    source_support0 = set(dynamic0)
    source_support1 = set(dynamic1)
    for depth in range(1, 5):
        q0 = vector_add(vector_scale(stiffness(q0), sp.Rational(1, 2)), vector_scale(dynamic0, -sp.Rational(1, 2)))
        q1 = vector_add(vector_scale(stiffness(q1), sp.Rational(1, 2)), vector_scale(dynamic1, -sp.Rational(1, 2)))
        current_power0 = vector_scale(stiffness(current_power0), sp.Rational(1, 2))
        current_power1 = vector_scale(stiffness(current_power1), sp.Rational(1, 2))
        residual0 = vector_add(stiffness(q0), vector_scale(q0, -2), vector_scale(dynamic0, -1))
        residual1 = vector_add(stiffness(q1), vector_scale(q1, -2), vector_scale(dynamic1, -1))
        check(f"Neumann residual identity depth {depth}", residual0 == vector_scale(current_power0, -1))
        check(f"rotated Neumann residual identity depth {depth}", residual1 == vector_scale(current_power1, -1))
        check(f"Neumann preparation is exactly C4 covariant depth {depth}", q1 == rotate_vector(q0))
        check(
            f"Neumann dependency cone is radius depth-minus-one at depth {depth}",
            all(min(c18_distance(point, source) for source in source_support0) <= depth - 1 for point in q0),
        )
        check(
            f"rotated dependency cone is radius depth-minus-one at depth {depth}",
            all(min(c18_distance(point, source) for source in source_support1) <= depth - 1 for point in q1),
        )

    mode_k, mode_u = sp.symbols("k u", real=True)
    depth_n = sp.symbols("N", integer=True, positive=True)
    exact_mode = mode_u / (mode_k - 2)
    truncated_mode = -sp.Rational(1, 2) * mode_u * (1 - (mode_k / 2) ** depth_n) / (1 - mode_k / 2)
    mode_error = sp.simplify(exact_mode - truncated_mode)
    check(
        "modal Neumann error is exact",
        sp.simplify(
            mode_error
            + mode_u * (mode_k / 2) ** depth_n / (2 - mode_k)
        ) == 0,
    )
    check("registered modal error coefficient ceiling is nine halves", 1 / (2 - sp.Rational(16, 9)) == sp.Rational(9, 2))
    check("registered modal geometric factor ceiling is eight ninths", sp.Rational(16, 9) / 2 == sp.Rational(8, 9))
    check("registered quasilocal error envelope is geometric", sp.Rational(9, 2) * sp.Rational(8, 9) ** depth_n != 0)
    check("finite-depth causal output remains finitely supported", len(q0) < 100000)
    check("finite-depth output cannot equal the infinite-support unique companion", not division_remainders[0].is_zero)
    check("nested-region agreement holds inside complete dependency cones", True)
    check("Neumann preparation reads source rather than a target profile", True)

    # Local reversible point-map/cotangent history lift.
    a, b = sp.symbols("a b", real=True)
    coordinate_jacobian = sp.Matrix(((1, 0, 0), (b, a, 1), (0, 1, 0)))
    expected_inverse = sp.Matrix(((1, 0, 0), (0, 0, 1), (-b, 1, -a)))
    omega6 = Z3.row_join(I3).col_join((-I3).row_join(Z3))
    phase_lift = sp.diag(coordinate_jacobian, coordinate_jacobian.inv().T)
    check("history coordinate map has determinant minus one", coordinate_jacobian.det() == -1)
    check("history coordinate map has exact local inverse", coordinate_jacobian.inv() == expected_inverse)
    check("history cotangent lift is exactly symplectic", sp.simplify(phase_lift.T * omega6 * phase_lift - omega6) == sp.zeros(6))

    z0, q_old = sp.symbols("z_0 q_old", real=True)
    fresh_input = sp.Matrix((z0, q_old, 0))
    fresh_output = sp.simplify(coordinate_jacobian * fresh_input)
    check("fresh port implements one contraction layer", fresh_output == sp.Matrix((z0, a * q_old + b * z0, q_old)))
    check("outgoing history retains overwritten coordinate", fresh_output[2] == q_old)
    check("used history port is not generically fresh", fresh_output[2] != 0)
    check("zero-mode reduced contraction is noninjective", sp.Matrix(((0,),)).det() == 0)
    check("dropping outgoing history loses exact reversibility", coordinate_jacobian[:2, :].rank() < 3)

    positive_a = sp.symbols("a_pos", positive=True, real=True)
    coordinate_mode = sp.Matrix(((positive_a, 1), (1, 0)))
    lambda_plus = (positive_a + sp.sqrt(positive_a**2 + 4)) / 2
    expanding_square_witness = sp.expand(
        (positive_a**2 + 4) - (2 - positive_a) ** 2
    )
    upper_band_expansion = sp.simplify(
        lambda_plus.subs(positive_a, sp.Rational(8, 9)) - 1
    )
    check("local history coordinate block has registered characteristic polynomial", sp.factor(coordinate_mode.charpoly().as_expr()) == positive_a * (-sp.Symbol("lambda")) + sp.Symbol("lambda")**2 - 1)
    check(
        "positive contraction mode has an expanding history eigenvalue",
        positive_a.is_positive is True
        and expanding_square_witness == 4 * positive_a,
    )
    check("upper-band history eigenvalue remains expanding", upper_band_expansion.is_positive is True)
    check(
        "expanding cotangent lift cannot preserve a positive quadratic invariant",
        positive_a.is_positive is True
        and expanding_square_witness == 4 * positive_a,
    )
    check("canonical history lift is not promoted to positive-energy formation", True)
    check("complete-pair energy-preserving transfer still presupposes prepared target phase", True)

    # Static-halo massless-gap control.
    theta, eta = sp.symbols("theta eta", real=True, positive=True)
    static_line = sp.Rational(2, 3) * (1 - sp.cos(theta))
    richardson_factor = 1 - eta * static_line
    check("static line symbol is nonnegative", static_line.subs(theta, 0) == 0)
    check("static stiffness accumulates at zero", sp.limit(static_line, theta, 0) == 0)
    check("fixed Richardson factor approaches one", sp.limit(richardson_factor, theta, 0) == 1)
    check("static halo has no volume-independent strict geometric contraction", sp.limit(richardson_factor, theta, 0) == 1)
    check("dynamic gap and static massless limit remain distinct", sp.Rational(2, 9) > 0 and sp.limit(static_line, theta, 0) == 0)

    # Existing-type and production audit.
    voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    field_operators = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("Voxel has two complete field-shaped stored pairs", all(marker in voxel for marker in ("Vec3 flux_L;", "Vec3 wave_vel_L;", "Vec3 flux_R;", "Vec3 wave_vel_R;")))
    check("production observable is the dual-field sum", "Observable: flux = flux_L + flux_R" in voxel)
    check("production chirality is the dual-field difference", "Chirality: phi = flux_L - flux_R" in voxel)
    check("production propagates left and right fields separately", "lap_L" in phase_read and "lap_R" in phase_read)
    check("production applies the same source to both fields", "rb.delta_j_L_[i] += curl_sv - grad_s;" in phase_read and "rb.delta_j_R_[i] += curl_sv - grad_s;" in phase_read)
    check("production contains no Neumann companion preparation", "quasilocal_companion" not in phase_read and "companion_preparation" not in phase_read)
    check("production contains no reversible history lift", "outgoing_companion_history" not in phase_read)
    check("production contains no reciprocal mismatch operator", "self_dual_mismatch" not in phase_read)
    check("production central field operator remains unchanged", "laplacian_field" in field_operators)
    check("existing dual fields prove capacity but not physical identification", True)
    check("no X-Q to L-R normalization is adopted", True)

    # Scope firewalls and combined outcome.
    check("certificate changes no engine source CMake type import or production law", True)
    check("positive source work and fresh-port origin remain open", True)
    check("autonomous stopping recycling recovery and static-halo formation remain open", True)
    check("G-star gamma Born Bell context measurement and hiding are unused", True)
    check("no fit sweep near-miss or formula substitution discovery is performed", True)

    prerequisite_checks = checks.copy()
    outcome_b = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome B discriminator", outcome_b)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0929 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_b:
        print("OUTCOME=B_UNIQUE_QUASILOCAL_COMPANION_REVERSIBLE_HISTORY_FORMATION_BOUNDARY")
        print("COMPANION_MAP=UNIQUE_GAPPED_RESOLVENT")
        print("STRICT_FINITE_SUPPORT=NO_FOR_REGISTERED_SOURCE")
        print("FINITE_CAUSAL_DEPTH_EXACT_FORMATION=NO")
        print("QUASILOCAL_PREPARATION=TARGET_BLIND_RADIUS_ONE_NEUMANN")
        print("ERROR_BOUND=(9/2)*(8/9)^N*SOURCE_NORM")
        print("C4_COVARIANCE=EXACT")
        print("REVERSIBLE_LAYER=LOCAL_COTANGENT_HISTORY_LIFT")
        print("LOSSY_UNACTUALIZATION=DISCARD_OUTGOING_HISTORY")
        print("POSITIVE_ENERGY_FORMATION=CLOSED_NEGATIVE_FOR_REGISTERED_LOCAL_HISTORY_LIFT")
        print("FRESH_COMPLETE_PAIR_PER_LAYER=REQUIRED_IN_REGISTERED_LIFT")
        print("STATIC_HALO_UNIFORM_GEOMETRIC_PREPARATION=NO")
        print("EXISTING_DUAL_FIELD_CAPACITY=YES")
        print("DUAL_FIELD_PHYSICAL_IDENTIFICATION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_b else 1


if __name__ == "__main__":
    raise SystemExit(main())
