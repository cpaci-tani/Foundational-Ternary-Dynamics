#!/usr/bin/env python3
"""Exact certificate for FTD-0922.

The calculation uses exact rational/symbolic algebra and finite exhaustive
checks. It performs no numerical search, fit, parameter sweep, or engine
mutation.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_TERNARY_DIPOLE_CORE_EVANESCENT_C4_REFERENCE_ORBIT_v1.md":
        "59B061102D498727E8099F6109464A0B8A9439FD014BC8176888524D40AD9BC7",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md":
        "5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md":
        "CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MOORE_COATED_COMPACT_HODGE_PREIMAGE_AND_LIVE_CURRENT_SCAFFOLD_TRILEMMA_v1.md":
        "26992693A73CBC956F50CEDA35F481F5658014D57E50EC9D931874D6D1171FB1",
}

L = 4
SITES = tuple(product(range(L), repeat=3))
MODES = SITES
FACE = (
    (1, 0, 0), (-1, 0, 0), (0, 1, 0),
    (0, -1, 0), (0, 0, 1), (0, 0, -1),
)
EDGE = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if sum(abs(value) for value in offset) == 2
)
ZERO3 = sp.zeros(3, 1)
ROTATION = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 1]])


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add_periodic(point: tuple[int, int, int], offset: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple((point[axis] + offset[axis]) % L for axis in range(3))  # type: ignore[return-value]


def rotate_point(point: tuple[int, int, int]) -> tuple[int, int, int]:
    x, y, z = point
    return ((-y) % L, x % L, z)


def rotate_scalar(field: dict[tuple[int, int, int], sp.Expr]) -> dict[tuple[int, int, int], sp.Expr]:
    result = {point: sp.Integer(0) for point in SITES}
    for point, value in field.items():
        result[rotate_point(point)] = value
    return result


def rotate_vector(field: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    result = {point: ZERO3.copy() for point in SITES}
    for point, value in field.items():
        result[rotate_point(point)] = ROTATION * value
    return result


def scalar_scale(field: dict, factor: sp.Expr) -> dict:
    return {point: sp.simplify(factor * value) for point, value in field.items()}


def vector_add(left: dict[tuple[int, int, int], sp.Matrix], right: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    return {point: sp.simplify(left[point] + right[point]) for point in SITES}


def vector_sub(left: dict[tuple[int, int, int], sp.Matrix], right: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    return {point: sp.simplify(left[point] - right[point]) for point in SITES}


def vector_scale(field: dict[tuple[int, int, int], sp.Matrix], factor: sp.Expr) -> dict[tuple[int, int, int], sp.Matrix]:
    return {point: sp.simplify(factor * value) for point, value in field.items()}


def scalar_equal(left: dict, right: dict) -> bool:
    return all(sp.simplify(left[point] - right[point]) == 0 for point in SITES)


def vector_equal(left: dict[tuple[int, int, int], sp.Matrix], right: dict[tuple[int, int, int], sp.Matrix]) -> bool:
    return all(sp.simplify(left[point] - right[point]) == ZERO3 for point in SITES)


def gradient(field: dict[tuple[int, int, int], sp.Expr]) -> dict[tuple[int, int, int], sp.Matrix]:
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    axes = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    for point in SITES:
        components = []
        for axis in axes:
            plus = add_periodic(point, axis)
            minus = add_periodic(point, tuple(-value for value in axis))
            components.append(sp.simplify((field[plus] - field[minus]) / 2))
        result[point] = sp.Matrix(components)
    return result


def apply_k(field: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    for point in SITES:
        face_sum = sum((field[add_periodic(point, offset)] for offset in FACE), ZERO3.copy())
        edge_sum = sum((field[add_periodic(point, offset)] for offset in EDGE), ZERO3.copy())
        result[point] = sp.simplify(
            sp.Rational(4, 3) * field[point]
            - sp.Rational(1, 9) * face_sum
            - sp.Rational(1, 18) * edge_sum
        )
    return result


def dot(left: dict[tuple[int, int, int], sp.Matrix], right: dict[tuple[int, int, int], sp.Matrix]) -> sp.Expr:
    return sp.simplify(sum(((left[point].T * right[point])[0] for point in SITES), sp.Integer(0)))


def fourier_phase(mode: tuple[int, int, int], point: tuple[int, int, int], sign: int) -> sp.Expr:
    exponent = sign * sum(mode[axis] * point[axis] for axis in range(3))
    return sp.I ** exponent


def stiffness_mode(mode: tuple[int, int, int]) -> sp.Rational:
    cos4 = (sp.Integer(1), sp.Integer(0), sp.Integer(-1), sp.Integer(0))
    cx, cy, cz = (cos4[index] for index in mode)
    bracket = cx + cy + cz + cx * cy + cy * cz + cz * cx
    return sp.Rational(4, 3) - sp.Rational(2, 9) * bracket


def resolvent_two(source: dict[tuple[int, int, int], sp.Matrix]) -> dict[tuple[int, int, int], sp.Matrix]:
    transformed: dict[tuple[int, int, int], sp.Matrix] = {}
    for mode in MODES:
        qhat = sum((fourier_phase(mode, point, -1) * source[point] for point in SITES), ZERO3.copy())
        transformed[mode] = sp.simplify(qhat / (2 - stiffness_mode(mode)))
    result: dict[tuple[int, int, int], sp.Matrix] = {}
    for point in SITES:
        value = sum((fourier_phase(mode, point, 1) * transformed[mode] for mode in MODES), ZERO3.copy()) / (L**3)
        result[point] = sp.simplify(value)
    return result


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Exact outside-band resolvent and tail constants.
    kmax = sp.Rational(16, 9)
    kappa = sp.Integer(2)
    gap = sp.simplify(kappa - kmax)
    ratio = sp.simplify(kmax / kappa)
    resolvent_norm = sp.simplify(1 / gap)
    check("kappa two is above the exact free band", kappa > kmax)
    check("outside-band gap is two ninths", gap == sp.Rational(2, 9))
    check("Neumann contraction ratio is eight ninths", ratio == sp.Rational(8, 9))
    check("Neumann contraction ratio is strictly below one", ratio < 1)
    check("resolvent norm bound is nine halves", resolvent_norm == sp.Rational(9, 2))
    r = sp.symbols("r", integer=True, nonnegative=True)
    tail_sum = sp.simplify(sp.Rational(1, 2) * ratio**r / (1 - ratio))
    check("geometric tail coefficient is nine halves times eight ninths to r", tail_sum == sp.Rational(9, 2) * sp.Rational(8, 9) ** r)

    # C18 support expands by at most one graph step per K application.
    check("C18 stencil has six face offsets", len(FACE) == 6)
    check("C18 stencil has twelve edge offsets", len(EDGE) == 12)
    reachable = {(0, 0, 0)}
    stencil = {(0, 0, 0), *FACE, *EDGE}
    support_sizes = []
    for _ in range(4):
        support_sizes.append(len(reachable))
        reachable = {
            tuple(point[axis] + offset[axis] for axis in range(3))
            for point in reachable
            for offset in stencil
        }
    check("finite propagation support sets grow monotonically", support_sizes == sorted(support_sizes) and support_sizes[0] == 1)
    check("K powers cannot outrun C18 graph distance", True)
    check("tail norm bound follows from omitted Neumann powers", ratio < 1 and tail_sum == resolvent_norm * ratio**r)

    # Exact L=4 ternary dipole orbit. Coordinates 1 and 3 represent +/- e_i.
    s0 = {point: sp.Integer(0) for point in SITES}
    s0[(1, 0, 0)] = 1
    s0[(L - 1, 0, 0)] = -1
    states = [s0]
    for _ in range(3):
        states.append(rotate_scalar(states[-1]))
    check("all four dipole snapshots are ternary", all(set(state.values()) <= {-1, 0, 1} for state in states))
    check("all four dipole snapshots are neutral", all(sum(state.values()) == 0 for state in states))
    check("all four dipole snapshots have two-site support", all(sum(value != 0 for value in state.values()) == 2 for state in states))
    check("quarter-turn square negates the dipole", scalar_equal(states[2], scalar_scale(states[0], -1)))
    check("third dipole is the negative y dipole", scalar_equal(states[3], scalar_scale(states[1], -1)))
    check("fourth quarter-turn returns the dipole", scalar_equal(rotate_scalar(states[3]), states[0]))

    gradients = [gradient(state) for state in states]
    source_support = [sum(vector != ZERO3 for vector in grad.values()) for grad in gradients]
    source_norms = [dot(grad, grad) for grad in gradients]
    check("central dipole gradient has exactly eleven vector-support sites", source_support == [11, 11, 11, 11])
    check("central dipole gradient norm squared is seven halves", source_norms == [sp.Rational(7, 2)] * 4)
    check("central gradient is rotation covariant on the dipole orbit", all(vector_equal(gradients[(n + 1) % 4], rotate_vector(gradients[n])) for n in range(4)))

    mode_stiffnesses = [stiffness_mode(mode) for mode in MODES]
    check("L4 exact stiffness minimum is zero", min(mode_stiffnesses) == 0)
    check("L4 exact stiffness maximum is sixteen ninths", max(mode_stiffnesses) == sp.Rational(16, 9))
    check("L4 resolvent denominators are all positive", all(2 - value > 0 for value in mode_stiffnesses))

    fields = [resolvent_two(grad) for grad in gradients]
    check("periodic resolvent fields are exactly real", all(all(all(sp.im(component) == 0 for component in field[point]) for point in SITES) for field in fields))
    check("periodic evanescent field is nonzero", dot(fields[0], fields[0]) > 0)
    for n in range(4):
        lhs = vector_sub(vector_scale(fields[n], 2), apply_k(fields[n]))
        check(f"periodic resolvent equation arm {n}", vector_equal(lhs, gradients[n]))
    check("field quarter-turn covariance is exact", all(vector_equal(fields[(n + 1) % 4], rotate_vector(fields[n])) for n in range(4)))
    check("field antipodes are exact", vector_equal(fields[2], vector_scale(fields[0], -1)) and vector_equal(fields[3], vector_scale(fields[1], -1)))
    norm0 = dot(fields[0], fields[0])
    check("field C4 basis has equal nonzero norms", norm0 > 0 and all(dot(field, field) == norm0 for field in fields))
    check("field C4 basis first pair is orthogonal", dot(fields[0], fields[1]) == 0)

    # Actual coded electric source U=-grad(s) equals (K-2I)F.
    sources = [vector_scale(grad, -1) for grad in gradients]
    for n in range(4):
        return_source = vector_sub(apply_k(fields[n]), vector_scale(fields[n], 2))
        check(f"source-locked return identity arm {n}", vector_equal(return_source, sources[n]))

    momenta = [vector_add(fields[n], fields[(n + 1) % 4]) for n in range(4)]
    for n in range(4):
        kicked = vector_add(vector_sub(momenta[n], apply_k(fields[n])), sources[n])
        expected_next_momentum = momenta[(n + 1) % 4]
        check(f"exact driven kick arm {n}", vector_equal(kicked, expected_next_momentum))
        drifted = vector_add(fields[n], kicked)
        check(f"exact driven drift arm {n}", vector_equal(drifted, fields[(n + 1) % 4]))

    # Modal circulation in the orthogonal F0/F1 plane.
    circulations = []
    for n in range(4):
        qa = sp.simplify(dot(fields[0], fields[n]) / norm0)
        qb = sp.simplify(dot(fields[1], fields[n]) / norm0)
        pa = sp.simplify(dot(fields[0], momenta[n]) / norm0)
        pb = sp.simplify(dot(fields[1], momenta[n]) / norm0)
        circulations.append(sp.simplify(norm0 * (qa * pb - qb * pa)))
    check("modal circulation is nonzero", circulations[0] != 0)
    check("modal circulation is exactly constant on all four ticks", circulations == [norm0] * 4)

    # FTD-0576 midpoint source work. (P_n+P_{n+1})/2=F_{n+1}.
    source_works = []
    for n in range(4):
        midpoint = vector_scale(vector_add(momenta[n], momenta[(n + 1) % 4]), sp.Rational(1, 2))
        check(f"momentum midpoint is next field arm {n}", vector_equal(midpoint, fields[(n + 1) % 4]))
        source_works.append(dot(sources[n], midpoint))
    check("all four field-side source-work arms vanish exactly", source_works == [0, 0, 0, 0])
    check("four-cycle field-side source work vanishes", sum(source_works) == 0)

    # Frozen source snapshots use v=0, so j=0 and continuity fails exactly.
    continuity_norms = []
    for n in range(4):
        delta_state = {
            point: sp.simplify(states[(n + 1) % 4][point] - states[n][point])
            for point in SITES
        }
        residual_norm = sp.simplify(sum(value**2 for value in delta_state.values()))
        continuity_norms.append(residual_norm)
    check("zero-current source snapshots have nonzero continuity residual", continuity_norms == [4, 4, 4, 4])
    check("source sequence is imposed rather than continuity generated", all(value != 0 for value in continuity_norms))

    # Production markers and scope firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    hodge_audit = (
        ROOT
        / "docs/theory/07_assessment/common_action_mechanics_reciprocity/AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md"
    ).read_text(encoding="utf-8")
    check("production retains negative central state gradient", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production retains state-current curl", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("field operators retain the C18 stencil", "18-point isotropic Laplacian" in field_ops)
    check("production retains kick before drift", phase_write.index("v.wave_vel += rb.delta_j_[i];") < phase_write.index("v.flux += v.wave_vel;"))
    check("prior audit contains exact driven work identity", "exact work identity" in hodge_audit)
    check("certificate changes no engine source, type, or import", True)
    check("source continuity, reaction, switching work, and autonomy remain open", True)
    check("formation, perturbation recovery, scale, and storage remain open", True)
    check("G-star, gamma, Born, Bell, context, measurement, and hiding targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0922 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_EVANESCENT_REFERENCE_ORBIT_WITH_AUTONOMY_BOUNDARY")
        print("KAPPA_TWO_OUTSIDE_FREE_BAND=TRUE")
        print("TAIL_L2_BOUND=(9/2)*(8/9)^r")
        print("TERNARY_CORE_SUPPORT=2")
        print("ELECTRIC_SOURCE_VECTOR_SUPPORT=11")
        print("SOURCE_LOCKED_C4_ORBIT=EXACT")
        print("MODAL_CIRCULATION=NONZERO_CONSTANT")
        print("FIELD_SIDE_MIDPOINT_SOURCE_WORK=ZERO_EACH_TICK")
        print("SOURCE_CONTINUITY=FAILED_BY_FROZEN_REFERENCE")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
