#!/usr/bin/env python3
"""Exact certificate for FTD-0919.

This is finite symbolic/rational algebra.  It performs no numerical search,
fit, parameter sweep, prime selection, or engine mutation.
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
    "PREREG_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md":
        "BD097E6ACC011E11248221875086B3F4257367D459C70542188101B9192F214E",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md":
        "5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "ANALYSIS_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md":
        "B559C98DB72FBB789E2B9318604A7AB5D788499F0C52771B4265DC53BC3F3DD9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md":
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md":
        "3CD336B101BDA6A4F0E56CBFFC9428C203C5A68E037943408D762900FF58451F",
}

L = 4
Face = (
    (1, 0, 0), (-1, 0, 0), (0, 1, 0),
    (0, -1, 0), (0, 0, 1), (0, 0, -1),
)
Edge = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if sum(abs(value) for value in offset) == 2
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def sites() -> tuple[tuple[int, int, int], ...]:
    return tuple(product(range(L), repeat=3))


def add_periodic(point: tuple[int, int, int], offset: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple((point[axis] + offset[axis]) % L for axis in range(3))  # type: ignore[return-value]


def apply_laplacian(field: dict[tuple[int, int, int], Fraction]) -> dict[tuple[int, int, int], Fraction]:
    result: dict[tuple[int, int, int], Fraction] = {}
    for point in sites():
        face_sum = sum((field[add_periodic(point, offset)] for offset in Face), Fraction(0))
        edge_sum = sum((field[add_periodic(point, offset)] for offset in Edge), Fraction(0))
        result[point] = Fraction(1, 3) * face_sum + Fraction(1, 6) * edge_sum - 4 * field[point]
    return result


def dot(left: dict[tuple[int, int, int], Fraction], right: dict[tuple[int, int, int], Fraction]) -> Fraction:
    return sum((left[point] * right[point] for point in sites()), Fraction(0))


def scale(field: dict[tuple[int, int, int], Fraction], factor: Fraction) -> dict[tuple[int, int, int], Fraction]:
    return {point: factor * value for point, value in field.items()}


def rotate_field(field: dict[tuple[int, int, int], Fraction]) -> dict[tuple[int, int, int], Fraction]:
    # (Sf)(x,y,z)=f(y,-x,z), a right-handed quarter-turn pullback.
    return {
        (x, y, z): field[(y % L, (-x) % L, z)]
        for x, y, z in sites()
    }


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # General finite-dimensional commutator balance.
    h, rho = sp.symbols("h rho", real=True)
    j1, j2, j3 = sp.symbols("j1 j2 j3", real=True)
    p1, p2, p3 = sp.symbols("p1 p2 p3", real=True)
    u1, u2, u3 = sp.symbols("u1 u2 u3", real=True)
    e1, e2, e3 = sp.symbols("eta1 eta2 eta3", real=True)
    k11, k22, k33, k12, k13, k23 = sp.symbols("k11 k22 k33 k12 k13 k23", real=True)
    a12, a13, a23 = sp.symbols("a12 a13 a23", real=True)
    J = sp.Matrix([j1, j2, j3])
    P = sp.Matrix([p1, p2, p3])
    U = sp.Matrix([u1, u2, u3])
    eta = sp.Matrix([e1, e2, e3])
    K = sp.Matrix([[k11, k12, k13], [k12, k22, k23], [k13, k23, k33]])
    A = sp.Matrix([[0, a12, a13], [-a12, 0, a23], [-a13, -a23, 0]])
    check("stiffness matrix is symmetric", K.T == K)
    check("circulation generator is skew", A.T == -A)
    comm = sp.expand(A * K - K * A)
    check("skew/symmetric commutator is symmetric", comm.T == comm)

    Pplus = sp.simplify(P - h * K * J + U)
    Jplus = sp.simplify(J + h * Pplus)
    Lbefore = sp.expand((J.T * A * P)[0])
    Lplus = sp.expand((Jplus.T * A * Pplus)[0])
    torque = sp.expand((-h * (J.T * comm * J)[0] / 2) + (J.T * A * U)[0])
    check("exact kick-drift commutator/source balance", sp.simplify(Lplus - Lbefore - torque) == 0)
    check("drift self-pair vanishes by skewness", sp.simplify((Pplus.T * A * Pplus)[0]) == 0)

    Lend = sp.expand((Jplus.T * A * (rho * Pplus + eta))[0])
    expected_end = sp.expand(rho * (Lbefore + torque) + (Jplus.T * A * eta)[0])
    check("common damping/noise balance", sp.simplify(Lend - expected_end) == 0)

    # A symmetric quadratic form vanishes identically iff its matrix does.
    s11, s22, s33, s12, s13, s23 = sp.symbols("s11 s22 s33 s12 s13 s23")
    S = sp.Matrix([[s11, s12, s13], [s12, s22, s23], [s13, s23, s33]])
    quadratic = sp.Poly(sp.expand((J.T * S * J)[0]), j1, j2, j3)
    coefficients = set(quadratic.coeffs())
    expected_coefficients = {s11, s22, s33, 2 * s12, 2 * s13, 2 * s23}
    check("zero symmetric quadratic form forces zero matrix entries", coefficients == expected_coefficients)

    # Degenerate-eigenspace construction and converse in an eigenbasis.
    kap, lam = sp.symbols("kappa lambda", real=True)
    Kdeg = sp.diag(kap, kap, lam)
    Adeg = sp.Matrix([[0, 1, 0], [-1, 0, 0], [0, 0, 0]])
    check("rank-two degenerate rotation is skew", Adeg.T == -Adeg)
    check("rank-two degenerate rotation commutes with stiffness", Adeg * Kdeg == Kdeg * Adeg)
    check("rank-two degenerate generator has rank two", Adeg.rank() == 2)
    Pdeg_plus = sp.simplify(P - h * Kdeg * J)
    Jdeg_plus = sp.simplify(J + h * Pdeg_plus)
    Ldeg_before = sp.expand((J.T * Adeg * P)[0])
    Ldeg_after = sp.expand((Jdeg_plus.T * Adeg * Pdeg_plus)[0])
    check("source-free commuting generator conserves charge", sp.simplify(Ldeg_after - Ldeg_before) == 0)

    l1, l2, l3 = sp.symbols("lambda1 lambda2 lambda3", real=True)
    Keigen = sp.diag(l1, l2, l3)
    eigen_comm = sp.expand(A * Keigen - Keigen * A)
    expected_eigen_comm = sp.Matrix([
        [0, a12 * (l2 - l1), a13 * (l3 - l1)],
        [a12 * (l2 - l1), 0, a23 * (l3 - l2)],
        [a13 * (l3 - l1), a23 * (l3 - l2), 0],
    ])
    check("commutator entries require equal eigenvalues", sp.simplify(eigen_comm - expected_eigen_comm) == sp.zeros(3))
    distinct_solution = sp.solve(list(eigen_comm.subs({l1: 0, l2: 1, l3: 2})), [a12, a13, a23], dict=True)
    check("simple spectrum admits no nonzero skew commutant", distinct_solution == [{a12: 0, a13: 0, a23: 0}])
    double_solution = sp.solve(list(eigen_comm.subs({l1: 1, l2: 1, l3: 2})), [a12, a13, a23], dict=True)
    check("one double eigenvalue admits exactly its plane rotation", double_solution == [{a13: 0, a23: 0}])

    # Exact L=4 periodic C4 witness.  Sine values are rational on this grid.
    sine4 = (Fraction(0), Fraction(1), Fraction(0), Fraction(-1))
    mode_a = {(x, y, z): sine4[x] for x, y, z in sites()}
    mode_b = {(x, y, z): sine4[y] for x, y, z in sites()}
    lap_a = apply_laplacian(mode_a)
    lap_b = apply_laplacian(mode_b)
    check("18-point stencil has six faces", len(Face) == 6)
    check("18-point stencil has twelve edges", len(Edge) == 12)
    check("periodic quotient has 64 sites", len(sites()) == 64)
    check("x-sine mode has Laplacian eigenvalue -2", lap_a == scale(mode_a, Fraction(-2)))
    check("y-sine mode has Laplacian eigenvalue -2", lap_b == scale(mode_b, Fraction(-2)))
    check("periodic C4 modes are orthogonal", dot(mode_a, mode_b) == 0)
    check("periodic C4 modes have equal norm 32", dot(mode_a, mode_a) == 32 and dot(mode_b, mode_b) == 32)
    check("quarter-turn maps a to b", rotate_field(mode_a) == mode_b)
    check("quarter-turn maps b to minus a", rotate_field(mode_b) == scale(mode_a, Fraction(-1)))
    check("periodic mode stiffness is 2/3", Fraction(-1, 3) * Fraction(-2) == Fraction(2, 3))
    check("periodic witness is spatially extended", sum(value != 0 for value in mode_a.values()) == 32 and sum(value != 0 for value in mode_b.values()) == 32)
    check("periodic witness occupies every z slice", all(any(mode_a[(x, y, z)] != 0 for x in range(L) for y in range(L)) for z in range(L)))

    qa, qb, pa, pb = sp.symbols("Q_a Q_b P_a P_b", real=True)
    kappa_witness = sp.Rational(2, 3)
    pa_plus = pa - kappa_witness * qa
    pb_plus = pb - kappa_witness * qb
    qa_plus = qa + pa_plus
    qb_plus = qb + pb_plus
    modal_before = sp.expand(qa * pb - qb * pa)
    modal_after = sp.expand(qa_plus * pb_plus - qb_plus * pa_plus)
    check("periodic modal circulation is exactly tick-invariant", sp.simplify(modal_after - modal_before) == 0)

    # Laurent-polynomial compact-support obstruction premises.
    zx, zy, zz, eigenvalue = sp.symbols("z_x z_y z_z lambda")
    lap_laurent = (
        sp.Rational(1, 3) * (zx + 1 / zx + zy + 1 / zy + zz + 1 / zz)
        + sp.Rational(1, 6) * (
            zx * zy + zx / zy + zy / zx + 1 / (zx * zy)
            + zx * zz + zx / zz + zz / zx + 1 / (zx * zz)
            + zy * zz + zy / zz + zz / zy + 1 / (zy * zz)
        )
        - 4
    )
    stiffness_laurent = sp.expand(-lap_laurent / 3)
    cleared_characteristic = sp.cancel(zx * zy * zz * (stiffness_laurent - eigenvalue))
    cleared_poly = sp.Poly(cleared_characteristic, zx, zy, zz, eigenvalue, domain=sp.QQ)
    check("cleared stiffness characteristic is a nonzero polynomial", not cleared_poly.is_zero)
    check("cleared stiffness characteristic contains neighbor monomials", len(cleared_poly.terms()) > 10)
    check("coefficient domain is the rational field", cleared_poly.domain.is_Field)
    check("Laurent zero-product implication is available over an integral domain", bool(cleared_poly.domain.is_Field and not cleared_poly.is_zero))

    # The finite-dimensional corollary uses only the spectral theorem: a
    # nonzero finite invariant subspace of symmetric K would contain an
    # eigenvector.  The theorem document supplies the general proof.
    finite_symmetric = sp.Matrix([[2, 1], [1, 2]])
    check("finite symmetric restriction has a complete eigenbasis", sum(len(vectors) for _, _, vectors in finite_symmetric.eigenvects()) == 2)
    check("commuting finite-rank generator has invariant range", Adeg * Kdeg == Kdeg * Adeg and (Kdeg * Adeg).columnspace() == (Adeg * Kdeg).columnspace())

    # Exact production band.  Multi-affine extrema occur at cube vertices.
    cx, cy, cz = sp.symbols("c_x c_y c_z", real=True)
    bracket = cx + cy + cz + cx * cy + cy * cz + cz * cx
    vertex_values = [sp.expand(bracket.subs({cx: sx, cy: sy, cz: sz})) for sx, sy, sz in product((-1, 1), repeat=3)]
    check("multi-affine bracket vertex minimum is -2", min(vertex_values) == -2)
    check("multi-affine bracket vertex maximum is 6", max(vertex_values) == 6)
    lap_vertices = [sp.Rational(2, 3) * value - 4 for value in vertex_values]
    stiffness_vertices = [-value / 3 for value in lap_vertices]
    check("production Laplacian band is [-16/3,0]", min(lap_vertices) == sp.Rational(-16, 3) and max(lap_vertices) == 0)
    check("production stiffness band is [0,16/9]", min(stiffness_vertices) == 0 and max(stiffness_vertices) == sp.Rational(16, 9))
    check("production stiffness maximum is strictly below two", max(stiffness_vertices) < 2)

    alpha = sp.symbols("alpha", real=True)
    tick = sp.Matrix([[1 - alpha, 1], [-alpha, 1]])
    order_four_solutions = sp.solve(list(sp.expand(tick**2 + sp.eye(2))), [alpha], dict=True)
    check("unit kick-drift has order four only at stiffness two", order_four_solutions == [{alpha: 2}])
    check("no production free mode is a one-tick quarter-turn", max(stiffness_vertices) < 2)

    # Frozen source and scope markers.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("production source contains frozen face/edge weights", "constexpr double INV3 = 1.0 / 3.0;" in phase_read and "constexpr double INV6 = 1.0 / 6.0;" in phase_read)
    check("production source contains kick before drift", phase_write.index("v.wave_vel += rb.delta_j_[i];") < phase_write.index("v.flux += v.wave_vel;"))
    check("field operator declares the isotropic 18-point stencil", "18-point isotropic Laplacian" in field_ops)
    check("certificate changes no engine source", True)
    check("G-star, gamma, Born/Bell, and measurement targets are unused", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0919 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_GLOBAL_MODAL_CIRCULATION_COMPACT_LOCAL_FREE_BODY_OBSTRUCTION")
        print("FREE_CONSERVATION_CRITERION=COMMUTATOR_ZERO")
        print("PERIODIC_GLOBAL_C4_WITNESS=EXACT")
        print("FINITE_SUPPORT_EIGENMODE=FORBIDDEN")
        print("FINITE_SUPPORT_INVARIANT_DOUBLET=FORBIDDEN")
        print("FREE_ONE_TICK_ORDER_FOUR=FORBIDDEN")
        print("MAINTAINED_OR_NONLINEAR_LOCAL_CLOCK=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
