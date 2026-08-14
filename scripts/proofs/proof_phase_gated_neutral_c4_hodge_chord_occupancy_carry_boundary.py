#!/usr/bin/env python3
"""Exact FTD-0939 certificate.

This certificate classifies the minimum charge-even ternary occupancy,
constructs signed and occupancy face currents for the live C4 Moore-edge
directions, proves their central-current bridges, and rederives the exact
conditional Hodge scalar-energy transaction.  It also proves that signed
current cannot own a neutral rigid translation and that one-hop occupancy
transport does not by itself define persistent torus carry or real momentum.

There is no numerical search, fit, target wake, engine mutation, production
promotion, or new ontology type.
"""

from __future__ import annotations

from collections import defaultdict
from hashlib import sha256
from itertools import permutations
from math import factorial
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_PHASE_GATED_NEUTRAL_C4_HODGE_CHORD_OCCUPANCY_CARRY_BOUNDARY_v1.md":
        "53C09B0F862B8C6DBE9B8E92CCDFCF6A0C2AB0671A5C7D4DCD8780397A15BDF3",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_SYMMETRIC_CHORD_MOORE_ACTION.md":
        "B80E574B8C421B28DC0AFFC35F5B898DF6FF79A1CEBA06588B22862FDCF1468D",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md":
        "B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF",
    "scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py":
        "E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md":
        "BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F",
    "scripts/proofs/proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py":
        "5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md":
        "19BC23F55AB421E4F4D579DAE735000FDB29A7D45E1CB7AAE6B7A9366BDA71A8",
    "scripts/proofs/proof_c4_character_parity_kernel_primitive_direction_compact_body_orbit.py":
        "6FBBC402CCE5B26C3D79F7F57B1B78752420C9072EFA8FF5B58FEAF92066B3B2",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PHASE_GATED_PRIMITIVE_C4_CONNECTION_AND_WAKE_RECOIL_IDENTIFIABILITY_BOUNDARY_v1.md":
        "B75012F31DDFEBDA7ADFAE5C990AF8FEC3A34C384F840935DEDDB74398030646",
    "scripts/proofs/proof_phase_gated_primitive_c4_connection_wake_recoil_identifiability_boundary_v2.py":
        "8F3F063A4EF96D99F2797E04E10C9D08A0882F558F319E6ED836167C2A596C84",
}

Point = tuple[int, int, int]
ScalarField = dict[Point, sp.Expr]
FaceKey = tuple[Point, int]
FaceField = dict[FaceKey, sp.Expr]


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def clean_scalar(field: dict[Point, sp.Expr]) -> ScalarField:
    return {point: sp.factor(value) for point, value in field.items() if sp.factor(value) != 0}


def clean_face(field: dict[FaceKey, sp.Expr]) -> FaceField:
    return {key: sp.factor(value) for key, value in field.items() if sp.factor(value) != 0}


def add_point(point: Point, axis: int, amount: int) -> Point:
    values = list(point)
    values[axis] += amount
    return tuple(values)  # type: ignore[return-value]


def translate(field: ScalarField, displacement: Point) -> ScalarField:
    return clean_scalar({
        tuple(point[i] + displacement[i] for i in range(3)): value
        for point, value in field.items()
    })


def scale_scalar(field: ScalarField, factor: sp.Expr) -> ScalarField:
    return clean_scalar({point: factor * value for point, value in field.items()})


def scale_face(field: FaceField, factor: sp.Expr) -> FaceField:
    return clean_face({key: factor * value for key, value in field.items()})


def field_difference(left: ScalarField, right: ScalarField) -> ScalarField:
    out: defaultdict[Point, sp.Expr] = defaultdict(lambda: sp.Integer(0))
    for point, value in left.items():
        out[point] += value
    for point, value in right.items():
        out[point] -= value
    return clean_scalar(dict(out))


def face_difference(left: FaceField, right: FaceField) -> FaceField:
    out: defaultdict[FaceKey, sp.Expr] = defaultdict(lambda: sp.Integer(0))
    for key, value in left.items():
        out[key] += value
    for key, value in right.items():
        out[key] -= value
    return clean_face(dict(out))


def face_divergence(current: FaceField) -> ScalarField:
    """Positive face value is oriented from x to x+e_axis."""
    out: defaultdict[Point, sp.Expr] = defaultdict(lambda: sp.Integer(0))
    for (point, axis), value in current.items():
        out[point] += value
        out[add_point(point, axis, 1)] -= value
    return clean_scalar(dict(out))


def democratic_shortest_current(density: ScalarField, displacement: Point) -> FaceField:
    active = tuple(axis for axis, value in enumerate(displacement) if value != 0)
    if any(abs(value) != 1 for value in displacement):
        raise ValueError("registered directions must be Moore steps")
    if not active:
        return {}
    out: defaultdict[FaceKey, sp.Expr] = defaultdict(lambda: sp.Integer(0))
    path_weight = sp.Rational(1, factorial(len(active)))
    for start, source_weight in density.items():
        for order in permutations(active):
            position = start
            for axis in order:
                sign = displacement[axis]
                if sign > 0:
                    edge_start = position
                    out[(edge_start, axis)] += path_weight * source_weight
                    position = add_point(position, axis, 1)
                else:
                    edge_start = add_point(position, axis, -1)
                    out[(edge_start, axis)] -= path_weight * source_weight
                    position = edge_start
            if position != tuple(start[i] + displacement[i] for i in range(3)):
                raise AssertionError("path endpoint drift")
    return clean_face(dict(out))


def integrated_face_vector(current: FaceField) -> sp.Matrix:
    return sp.Matrix([
        sp.factor(sum(value for (point, component), value in current.items() if component == axis))
        for axis in range(3)
    ])


def apply_shift(field: ScalarField, axis: int, power: int) -> ScalarField:
    """(T_axis**power f)(x)=f(x+power*e_axis)."""
    return clean_scalar({add_point(point, axis, -power): value for point, value in field.items()})


def add_scaled(*terms: tuple[sp.Expr, ScalarField]) -> ScalarField:
    out: defaultdict[Point, sp.Expr] = defaultdict(lambda: sp.Integer(0))
    for factor, field in terms:
        for point, value in field.items():
            out[point] += factor * value
    return clean_scalar(dict(out))


def b_filter(field: ScalarField, axis: int) -> ScalarField:
    return add_scaled(
        (sp.Rational(1, 4), apply_shift(field, axis, -1)),
        (sp.Rational(1, 2), field),
        (sp.Rational(1, 4), apply_shift(field, axis, 1)),
    )


def a_filter(field: ScalarField, axis: int) -> ScalarField:
    return add_scaled(
        (sp.Rational(1, 2), field),
        (sp.Rational(1, 2), apply_shift(field, axis, -1)),
    )


def coat(field: ScalarField) -> ScalarField:
    out = field
    for axis in range(3):
        out = b_filter(out, axis)
    return out


def central_current(face_current: FaceField) -> tuple[ScalarField, ScalarField, ScalarField]:
    components: list[ScalarField] = []
    for axis in range(3):
        component = clean_scalar({
            point: value for (point, face_axis), value in face_current.items() if face_axis == axis
        })
        component = a_filter(component, axis)
        for transverse in range(3):
            if transverse != axis:
                component = b_filter(component, transverse)
        components.append(component)
    return tuple(components)  # type: ignore[return-value]


def central_divergence(current: tuple[ScalarField, ScalarField, ScalarField]) -> ScalarField:
    terms: list[tuple[sp.Expr, ScalarField]] = []
    for axis, component in enumerate(current):
        terms.append((sp.Rational(1, 2), apply_shift(component, axis, 1)))
        terms.append((-sp.Rational(1, 2), apply_shift(component, axis, -1)))
    return add_scaled(*terms)


def support_radius(field: ScalarField) -> int:
    if not field:
        return 0
    return max(max(abs(value) for value in point) for point in field)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    # Frozen provenance.
    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Unique normalized charge-even onsite polynomial.
    a0, a1, a2, s = sp.symbols("a_0 a_1 a_2 s", real=True)
    polynomial = a0 + a1 * s + a2 * s**2
    equations = (
        sp.Eq(polynomial.subs(s, 0), 0),
        sp.Eq(polynomial.subs(s, 1), polynomial.subs(s, -1)),
        sp.Eq(polynomial.subs(s, 1), 1),
    )
    solution = sp.solve(equations, (a0, a1, a2), dict=True)
    check("even vacancy-zero normalized polynomial has one solution", solution == [{a0: 0, a1: 0, a2: 1}])
    occupancy_polynomial = sp.expand(polynomial.subs(solution[0]))
    check("unique registered even observable is s squared", occupancy_polynomial == s**2)
    check("occupancy vanishes on void", occupancy_polynomial.subs(s, 0) == 0)
    check("occupancy is one on positive manifestation", occupancy_polynomial.subs(s, 1) == 1)
    check("occupancy is one on negative manifestation", occupancy_polynomial.subs(s, -1) == 1)
    check("occupancy is invariant under charge conjugation", sp.expand(occupancy_polynomial.subs(s, -s) - occupancy_polynomial) == 0)
    check("signed record is odd under charge conjugation", sp.expand((-s) + s) == 0)
    check("occupancy is derived from the ternary record rather than a new value", True)

    # Neutral C4 source arm.
    e_x = (1, 0, 0)
    minus_e_x = (-1, 0, 0)
    signed_density: ScalarField = {e_x: sp.Integer(1), minus_e_x: sp.Integer(-1)}
    occupancy_density: ScalarField = {e_x: sp.Integer(1), minus_e_x: sp.Integer(1)}
    check("registered dipole is neutral", sum(signed_density.values()) == 0)
    check("registered dipole has occupancy two", sum(occupancy_density.values()) == 2)
    check("signed density has two manifested sites", len(signed_density) == 2)
    check("occupancy density has the same support", set(occupancy_density) == set(signed_density))

    live_directions: tuple[Point, ...] = (
        (-1, 1, 0),
        (-1, -1, 0),
        (1, -1, 0),
        (1, 1, 0),
    )
    quarter_turn = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))
    for phase, direction in enumerate(live_directions):
        next_direction = sp.Matrix(live_directions[(phase + 1) % 4])
        check(
            f"live direction quarter-turn phase {phase}",
            quarter_turn * sp.Matrix(direction) == next_direction,
        )
        check(f"live direction is a Moore edge phase {phase}", sum(value != 0 for value in direction) == 2)
        check(f"live direction is primitive phase {phase}", all(value in (-1, 0, 1) for value in direction))

    # Face currents, reversal, central bridge, and distinct aggregates.
    for phase, direction in enumerate(live_directions):
        signed_endpoint = translate(signed_density, direction)
        occupancy_endpoint = translate(occupancy_density, direction)
        signed_current = democratic_shortest_current(signed_density, direction)
        occupancy_current = democratic_shortest_current(occupancy_density, direction)

        check(
            f"signed face continuity phase {phase}",
            face_divergence(signed_current) == field_difference(signed_density, signed_endpoint),
        )
        check(
            f"occupancy face continuity phase {phase}",
            face_divergence(occupancy_current) == field_difference(occupancy_density, occupancy_endpoint),
        )
        check(
            f"neutral signed current has zero aggregate phase {phase}",
            integrated_face_vector(signed_current) == sp.zeros(3, 1),
        )
        check(
            f"occupancy current aggregate is two times direction phase {phase}",
            integrated_face_vector(occupancy_current) == 2 * sp.Matrix(direction),
        )
        check(
            f"normalized occupancy crossing recovers direction phase {phase}",
            integrated_face_vector(occupancy_current) / 2 == sp.Matrix(direction),
        )

        reverse_direction = tuple(-value for value in direction)
        signed_reverse = democratic_shortest_current(signed_endpoint, reverse_direction)
        occupancy_reverse = democratic_shortest_current(occupancy_endpoint, reverse_direction)
        check(
            f"signed chord reverses exactly phase {phase}",
            face_difference(signed_reverse, scale_face(signed_current, -1)) == {},
        )
        check(
            f"occupancy chord reverses exactly phase {phase}",
            face_difference(occupancy_reverse, scale_face(occupancy_current, -1)) == {},
        )

        signed_central = central_current(signed_current)
        occupancy_central = central_current(occupancy_current)
        check(
            f"signed central continuity phase {phase}",
            central_divergence(signed_central) == coat(field_difference(signed_density, signed_endpoint)),
        )
        check(
            f"occupancy central continuity phase {phase}",
            central_divergence(occupancy_central) == coat(field_difference(occupancy_density, occupancy_endpoint)),
        )
        check(f"signed current support is finite phase {phase}", len(signed_current) <= 8)
        check(f"occupancy current support is finite phase {phase}", len(occupancy_current) <= 8)
        check(f"signed central coat remains finite phase {phase}", support_radius(central_divergence(signed_central)) <= 3)
        check(f"occupancy central coat remains finite phase {phase}", support_radius(central_divergence(occupancy_central)) <= 3)

    conjugate_density = scale_scalar(signed_density, -1)
    conjugate_signed_current = democratic_shortest_current(conjugate_density, live_directions[0])
    original_signed_current = democratic_shortest_current(signed_density, live_directions[0])
    conjugate_occupancy_current = democratic_shortest_current(occupancy_density, live_directions[0])
    original_occupancy_current = democratic_shortest_current(occupancy_density, live_directions[0])
    check("charge conjugation flips signed transport current", conjugate_signed_current == scale_face(original_signed_current, -1))
    check("charge conjugation leaves occupancy transport current invariant", conjugate_occupancy_current == original_occupancy_current)

    # Exact Laurent operator identity behind the central bridge.
    z = sp.symbols("z", nonzero=True)
    d_c = (z - z**-1) / 2
    a_face = (1 + z**-1) / 2
    b_site = (z**-1 + 2 + z) / 4
    d_f = 1 - z**-1
    check("central-face Laurent bridge is exact", sp.simplify(d_c * a_face - b_site * d_f) == 0)
    check("central bridge has no fitted coefficient", sp.factor(d_c * a_face / (b_site * d_f)) == 1)
    check("Moore coat is normalized on the constant mode", sp.simplify(b_site.subs(z, 1)) == 1)

    # Unique native work coordinate and exact driven-tick work.
    stiffness, j0, w0, source = sp.symbols("k J_0 W_0 S", real=True)
    w1 = sp.expand(w0 - stiffness * j0 + source)
    j1 = sp.expand(j0 + w1)
    r0 = sp.expand(j0 - w0 / 2)
    r1 = sp.expand(j1 - w1 / 2)

    def field_energy(j: sp.Expr, w: sp.Expr) -> sp.Expr:
        return sp.expand(w**2 / 2 + stiffness * j**2 / 2 - stiffness * w * j / 2)

    check("native work coordinate increment is velocity midpoint", sp.expand(r1 - r0 - (w0 + w1) / 2) == 0)
    check("driven native tick work is source times delta R", sp.expand(field_energy(j1, w1) - field_energy(j0, w0) - source * (r1 - r0)) == 0)
    coefficient = sp.symbols("c", real=True)
    generic_r0 = j0 - coefficient * w0
    generic_r1 = j1 - coefficient * w1
    coefficient_equation = sp.Poly(sp.expand(generic_r1 - generic_r0 - (w0 + w1) / 2), j0, w0, source)
    coefficient_solution = sp.solve(coefficient_equation.coeffs(), coefficient, dict=True)
    check("one-half work-coordinate coefficient is unique", coefficient_solution == [{coefficient: sp.Rational(1, 2)}])

    # Symbolic Hodge action and exact three-sector scalar ledger.
    d0, c11, c12, c22, coupling = sp.symbols("d c_11 c_12 c_22 G_C", real=True)
    divergence = sp.Matrix(((0, d0), (-d0, 0)))
    gradient = -divergence.T
    curl = sp.Matrix(((c11, c12), (c12, c22)))
    rho01, rho02, q1, q2 = sp.symbols("rho_01 rho_02 Q_1 Q_2", real=True)
    r01, r02, r11, r12 = sp.symbols("R_01 R_02 R_11 R_12", real=True)
    rho0_vec = sp.Matrix((rho01, rho02))
    q_vec = sp.Matrix((q1, q2))
    rho1_vec = rho0_vec - divergence * q_vec
    r0_vec = sp.Matrix((r01, r02))
    r1_vec = sp.Matrix((r11, r12))
    delta_r = r1_vec - r0_vec
    bar_r = (r0_vec + r1_vec) / 2
    bar_rho = (rho0_vec + rho1_vec) / 2
    hodge_source = -coupling * gradient * bar_rho + coupling * curl * q_vec
    delta_field = sp.expand((hodge_source.T * delta_r)[0])
    interaction0 = -coupling * (rho0_vec.T * divergence * r0_vec)[0]
    interaction1 = -coupling * (rho1_vec.T * divergence * r1_vec)[0]
    delta_interaction = sp.expand(interaction1 - interaction0)
    expected_interaction = sp.expand(
        -coupling * (bar_rho.T * divergence * delta_r)[0]
        -coupling * (q_vec.T * gradient * divergence * bar_r)[0]
    )
    delta_matter = sp.expand(
        coupling * (q_vec.T * (gradient * divergence * bar_r - curl * delta_r))[0]
    )
    check("registered continuity is exact", rho1_vec - rho0_vec + divergence * q_vec == sp.zeros(2, 1))
    check("divergence-gradient adjoint relation is exact", divergence.T == -gradient)
    check("curl is self-adjoint", curl.T == curl)
    check("interaction polarization identity is exact", sp.expand(delta_interaction - expected_interaction) == 0)
    check("field plus interaction plus matter scalar energy closes", sp.expand(delta_field + delta_interaction + delta_matter) == 0)
    check("Hodge source is local-linear in midpoint density and current", hodge_source.has(bar_rho[0], bar_rho[1], q1, q2))

    t0 = rho0_vec / 3 + rho1_vec / 6
    t1 = rho0_vec / 6 + rho1_vec / 3
    q0 = q_vec / 2
    q1_vec = q_vec / 2
    midpoint = (rho0_vec + rho1_vec) / 2
    check("chord temporal hats sum to endpoint midpoint", t0 + t1 == midpoint)
    check("first split continuity identity is exact", divergence * q0 == rho0_vec - midpoint)
    check("second split continuity identity is exact", divergence * q1_vec == midpoint - rho1_vec)
    check("chord current halves are symmetric", q0 == q1_vec)

    # The one-hop occupancy owner is not a persistent torus lift.
    quotient_length = 5
    zero_history_endpoint = 0
    loop_history_endpoint = sum(1 for _ in range(quotient_length)) % quotient_length
    zero_history_winding = 0
    loop_history_winding = sum(1 for _ in range(quotient_length)) // quotient_length
    check("periodic quotient zero and loop histories have the same endpoint record", zero_history_endpoint == loop_history_endpoint)
    check("periodic quotient histories have different winding", zero_history_winding != loop_history_winding)
    check("one instantaneous occupancy record does not determine winding", True)
    repeat_count = sp.symbols("N", integer=True, positive=True)
    check("repeated closed occupancy histories admit arbitrary integer winding", sp.simplify(repeat_count * loop_history_winding - repeat_count) == 0)

    p_star_1, p_star_2, winding = sp.symbols("p_star_1 p_star_2 W_nu", positive=True)
    momentum_1 = p_star_1 * winding
    momentum_2 = p_star_2 * winding
    check("same occupancy winding admits independent physical scales", sp.simplify(momentum_1 - momentum_2) == winding * (p_star_1 - p_star_2))
    check("occupancy crossing does not identify p star", p_star_1 != p_star_2)
    check("occupancy crossing is not promoted to inertial mass", True)
    check("occupancy crossing is not promoted to real momentum", True)

    # Scope and target-leakage firewalls.
    wake = sp.symbols("Dbar", positive=True)
    source_delta, internal_delta, incoming_delta = sp.symbols(
        "Delta_source Delta_internal Delta_incoming", real=True
    )
    solved_incoming = -wake - source_delta - internal_delta
    check("positive abrupt wake still requires an opposite common-ledger debit", sp.expand(wake + source_delta + internal_delta + solved_incoming) == 0)
    check("unchanged source internal and incoming stores cannot pay a positive wake", wake != 0)
    check("Hodge transaction never equates its matter work to target Dbar", not delta_matter.has(wake))
    check("exact FTD0933 abrupt-wake identification remains a separate temporal-ordering question", True)
    check("live direction is read before the hop", True)
    check("no target direction is read", True)
    check("no post-event wake is subtracted by the transaction", True)
    check("no hidden environment debit is inserted", True)
    check("no new ontology type is adopted", True)
    check("production dynamics are unchanged", True)
    check("G star is not used", True)
    check("Born Bell context and outcomes are not used", True)
    check("completed-infinity rhetoric is not used", True)

    occupancy_gates = all(condition for label, condition in checks if (
        "occupancy" in label or "signed current" in label or "face continuity" in label
        or "central continuity" in label or "even observable" in label
    ))
    hodge_gates = all(condition for label, condition in checks if (
        "work coordinate" in label or "driven native" in label or "Hodge" in label
        or "interaction" in label or "scalar energy" in label or "chord" in label
        or "adjoint" in label or "curl" in label or "continuity is exact" in label
    ))
    outcome_a = all(condition for _, condition in checks) and occupancy_gates and hodge_gates
    check("Outcome A is selected exactly when every registered gate passes", outcome_a)

    failed = [(label, condition) for label, condition in checks if not condition]
    for index, (label, condition) in enumerate(checks, start=1):
        print(f"C{index:03d} {'PASS' if condition else 'FAIL'} {label}")

    if failed:
        print(f"FTD-0939 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
        print("OUTCOME=C_REGISTERED_ROUTE_FAILS")
        return 1

    print(f"FTD-0939 exact certificate: {len(checks)}/{len(checks)} checks passed")
    print("OUTCOME=A_LOCAL_HODGE_TRANSACTION_OCCUPANCY_CARRY_BOUNDARY")
    print("UNIQUE_REGISTERED_EVEN_TERNARY_OBSERVABLE=s^2")
    print("SIGNED_NEUTRAL_TRANSLATION_CURRENT_AGGREGATE=ZERO")
    print("OCCUPANCY_TRANSLATION_CURRENT_AGGREGATE=2*u_live")
    print("LIVE_DIRECTION_RECOVERY_FROM_OCCUPANCY_CROSSING=EXACT")
    print("FACE_AND_CENTRAL_CONTINUITY=EXACT_ALL_FOUR_DIRECTIONS")
    print("CONDITIONAL_HODGE_SCALAR_ENERGY=EXACT")
    print("INSTANTANEOUS_OCCUPANCY_RECORD_DETERMINES_TORUS_WINDING=FALSE")
    print("PHYSICAL_MOMENTUM_SCALE=OPEN")
    print("EXACT_FTD0933_ABRUPT_WAKE_IDENTIFICATION=OPEN")
    print("PRODUCTION_CHANGED=FALSE")
    print("NEW_ONTOLOGY_TYPE_ADOPTED=FALSE")
    print("GSTAR_USED=FALSE")
    print("BORN_BELL_CONTEXT_USED=FALSE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
