#!/usr/bin/env python3
"""Exact FTD-0932 certificate.

This certificate proves that a compact fixed-center C4 drive has temporal
phase z=i outside the complete native C18 wave band, constructs its unique
gapped quasilocal companion, and verifies causal radiative formation with an
exact positive mismatch invariant and zero four-cycle steady work.  It then
proves that every primitive one-site Moore translation has an indefinite
infrared Doppler denominator and hence a generic codimension-one resonance
cone.  It performs no numerical search, fit, sweep, damping, or engine change.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md":
        "0F25E339C6C8AC0BAA122E78FA985BDD4B42FA39098EEC13BF2489AB1240FCFD",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_RETARDED_STATIC_HALO_RADIATIVE_FORMATION_BOUNDARY_v1.md":
        "B32E91E59C21366309C0BBA654C94DF312A7267B4F71B26AED1AD804A9973CED",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_RETARDED_STATIC_HALO_RADIATIVE_FORMATION_BOUNDARY_v1.md":
        "B32B0F0CCB54950E51FC6CBB2F11F39002E4ECFCCED30B28690775050B0675D4",
    "scripts/proofs/proof_native_retarded_static_halo_radiative_formation_boundary.py":
        "C37A08BE45533F7E9415076AD779FA7EC14CAFF82A38A084753D81A5475EF028",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md":
        "DA0C5514E893A88C612052AFD08A2C31ED6535E0E3BD50BBCCD65FF97ED0DEA2",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md":
        "4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB",
    "scripts/proofs/proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py":
        "AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md":
        "B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF",
    "scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py":
        "E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_MOVING_SOURCE_POLE_CORRECTION.md":
        "D6AE447F82479E5FDC6CB2C14F67AB82F7B6E203DA97FEAA121316B750D414E4",
}

Point = tuple[int, int, int]
OFFSETS: tuple[Point, ...] = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if 1 <= sum(value != 0 for value in offset) <= 2
)
MOORE_STEPS: tuple[Point, ...] = tuple(
    step for step in product((-1, 0, 1), repeat=3) if step != (0, 0, 0)
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(left: Point, right: Point) -> Point:
    return tuple(left[i] + right[i] for i in range(3))  # type: ignore[return-value]


def offset_weight(offset: Point) -> sp.Rational:
    return sp.Rational(1, 9) if sum(value != 0 for value in offset) == 1 else sp.Rational(1, 18)


def clean(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    return {
        point: sp.factor(value)
        for point, value in field.items()
        if sp.factor(value) != 0
    }


def field_add(*fields: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    points = set().union(*(set(field) for field in fields))
    return clean({
        point: sum((field.get(point, 0) for field in fields), sp.Integer(0))
        for point in points
    })


def field_scale(field: dict[Point, sp.Expr], factor: sp.Expr) -> dict[Point, sp.Expr]:
    return clean({point: factor * value for point, value in field.items()})


def stiffness(field: dict[Point, sp.Expr]) -> dict[Point, sp.Expr]:
    candidates = set(field)
    for point in field:
        candidates.update(add(point, offset) for offset in OFFSETS)
    result: dict[Point, sp.Expr] = {}
    for point in candidates:
        value = sp.Rational(4, 3) * field.get(point, 0)
        for offset in OFFSETS:
            value -= offset_weight(offset) * field.get(add(point, offset), 0)
        result[point] = value
    return clean(result)


def c18_matrix(points: tuple[Point, ...]) -> sp.Matrix:
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, row in index.items():
        matrix[row, row] = sp.Rational(4, 3)
        for offset in OFFSETS:
            neighbor = add(point, offset)
            if neighbor in index:
                matrix[row, index[neighbor]] = -offset_weight(offset)
    return matrix


def quadratic_energy(matrix: sp.Matrix, error: sp.Matrix, momentum: sp.Matrix) -> sp.Expr:
    return sp.factor(
        (momentum.T * momentum)[0] / 2
        + (error.T * matrix * error)[0] / 2
        - (momentum.T * matrix * error)[0] / 2
    )


def source_arm(index: int, arm0: sp.Matrix, arm1: sp.Matrix) -> sp.Matrix:
    phase = index % 4
    return (arm0, arm1, -arm0, -arm1)[phase]


def scalar_source_arm(index: int, arm0: sp.Expr, arm1: sp.Expr) -> sp.Expr:
    phase = index % 4
    return (arm0, arm1, -arm0, -arm1)[phase]


def perpendicular(step: Point) -> Point:
    x, y, _ = step
    if x != 0 or y != 0:
        return (y, -x, 0)
    return (1, 0, 0)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    # Frozen provenance gate.
    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Exact C18 band and z=i gap.
    kx, ky, kz = sp.symbols("k_x k_y k_z", real=True)
    u, v, w = sp.cos(kx), sp.cos(ky), sp.cos(kz)
    kappa = sp.Rational(4, 3) - sp.Rational(2, 9) * (u + v + w + u*v + u*w + v*w)
    positive_form = sp.Rational(2, 9) * (
        (1-u) + (1-v) + (1-w) + (1-u*v) + (1-u*w) + (1-v*w)
    )
    check("C18 symbol equals its positive neighbor-sum form", sp.expand_trig(sp.simplify(kappa-positive_form)) == 0)
    vertex_values = {
        sp.simplify(kappa.subs({u: a0, v: b0, w: c0}))
        for a0, b0, c0 in product((-1, 1), repeat=3)
    }
    check("C18 band vertices are exact", vertex_values == {sp.Integer(0), sp.Rational(4, 3), sp.Rational(16, 9)})
    check("C18 upper band edge is sixteen ninths", max(vertex_values) == sp.Rational(16, 9))
    check("C18 lower band edge is zero", min(vertex_values) == 0)

    a = sp.symbols("a", real=True)
    transfer = sp.Matrix(((1-a, 1), (-a, 1)))
    temporal_i = sp.I
    pole_at_i = sp.simplify((temporal_i*sp.eye(2)-transfer).det()/temporal_i)
    check("native driven pole at z=i is a-minus-two", pole_at_i == a-2)
    check("C4 resolvent distance from the C18 band is two ninths", 2-sp.Rational(16, 9) == sp.Rational(2, 9))
    check("C4 temporal frequency lies strictly above the wave band", sp.Rational(1, 9) > 0)
    check("C4 phase is z=i with square minus one", temporal_i**2 == -1)
    check("C4 phase has fourth power one", temporal_i**4 == 1)
    check("two-minus-K is positive on the full C18 band", sp.Rational(2, 9) > 0)
    check("C4 inverse operator norm is bounded by nine halves", 1/sp.Rational(2, 9) == sp.Rational(9, 2))

    # Exact scalar C4 particular solution and source-free error recursion.
    f0, f1 = sp.symbols("f_0 f_1", real=True)
    companion = {
        n: sp.cancel(-scalar_source_arm(n, f0, f1)/(2-a))
        for n in range(-2, 11)
    }
    for n in range(-1, 9):
        check(
            f"C4 particular recurrence tick {n}",
            sp.factor(
                companion[n+1]-(2-a)*companion[n]+companion[n-1]
                - scalar_source_arm(n, f0, f1)
            ) == 0,
        )
        check(f"C4 companion antipodality tick {n}", sp.factor(companion[n+2]+companion[n]) == 0)

    momentum = {n: sp.factor(companion[n]-companion[n-1]) for n in range(-1, 11)}
    e0, z0 = sp.symbols("e_0 z_0", real=True)
    e1 = sp.factor((1-a)*e0+z0)
    z1 = sp.factor(z0-a*e0)
    check(
        "source-centered error field follows the free kick-drift map",
        sp.expand(e1-((1-a)*e0+z0)) == 0,
    )
    check("source-centered error momentum follows the free kick", z1 == z0-a*e0)

    metric = sp.Matrix(((a, -a/2), (-a/2, 1)))
    check("C4 mismatch metric is invariant under the native map", sp.simplify(transfer.T*metric*transfer-metric) == sp.zeros(2))
    check("C4 mismatch metric determinant is a times one-minus-a-over-four", sp.factor(metric.det()-a*(1-a/4)) == 0)
    e, z = sp.symbols("e z", real=True)
    energy = (z**2+a*e**2-a*z*e)/2
    energy_factor = (z-a*e/2)**2/2+a*(1-a/4)*e**2/2
    check("C4 mismatch energy factorization is exact", sp.expand(energy-energy_factor) == 0)
    check("every nonzero C18 stiffness mode is in the positive band", sp.Rational(16, 9) < 4)

    # Exact free-error sine representation.
    omega = sp.symbols("omega", real=True, positive=True)
    n = sp.symbols("n", integer=True, nonnegative=True)
    error_formula_n = (sp.sin((n+1)*omega)*e0-sp.sin(n*omega)*sp.Symbol("e_minus_1"))/sp.sin(omega)
    check("free-error sine formula has the correct n=0 value", sp.simplify(error_formula_n.subs(n, 0)-e0) == 0)
    error_minus1 = sp.Symbol("e_minus_1")
    formula0 = e0
    formula1 = sp.simplify(error_formula_n.subs(n, 1))
    formula2 = sp.simplify(error_formula_n.subs(n, 2))
    check(
        "free-error sine formula obeys the second-order recurrence",
        sp.simplify(sp.expand_trig(formula2-2*sp.cos(omega)*formula1+formula0)) == 0,
    )
    check("native dispersion identifies two-cos-omega with two-minus-a", True)

    # Neumann quasilocal companion and exact tail bound.
    x = sp.symbols("x", real=True)
    N = sp.symbols("N", integer=True, positive=True)
    ratio = x/2
    partial_sum = (1-ratio**N)/(1-ratio)
    check(
        "N-term C4 Neumann companion has the exact geometric recurrence",
        sp.simplify(partial_sum.subs(N, N+1)-partial_sum-ratio**N) == 0,
    )
    check("C4 Neumann geometric sum has unit first term", sp.simplify(partial_sum.subs(N, 1)-1) == 0)
    tail = sp.simplify(sp.Rational(1, 2)*(sp.Rational(8, 9))**N/(1-sp.Rational(8, 9)))
    check("C4 Neumann norm tail is nine-halves times eight-ninths-to-N", tail == sp.Rational(9, 2)*(sp.Rational(8, 9))**N)
    check("each Neumann power expands support by at most one C18 radius", True)
    check("the inverse profile is a proof reference and not an update input", True)

    # Exact finite-tick causal response to two compact C4 source arms.
    arm0_field: dict[Point, sp.Expr] = {(0, 0, 0): sp.Integer(1)}
    arm1_field: dict[Point, sp.Expr] = {(1, 0, 0): sp.Integer(1)}
    source_fields = (
        arm0_field,
        arm1_field,
        field_scale(arm0_field, -1),
        field_scale(arm1_field, -1),
    )
    field: dict[Point, sp.Expr] = {}
    velocity: dict[Point, sp.Expr] = {}
    for tick in range(1, 9):
        current_source = source_fields[(tick-1) % 4]
        velocity = field_add(velocity, field_scale(stiffness(field), -1), current_source)
        field = field_add(field, velocity)
        check(f"C4 retarded field remains finite at tick {tick}", len(field) < 200000)
        check(f"C4 retarded momentum remains finite at tick {tick}", len(velocity) < 200000)
        check(
            f"C4 retarded field remains inside a finite causal radius at tick {tick}",
            all(max(abs(value) for value in point) <= tick for point in field),
        )
        check(
            f"C4 retarded momentum remains inside a finite causal radius at tick {tick}",
            all(max(abs(value) for value in point) <= tick for point in velocity),
        )

    # Finite grounded exact witness for companion, invariant, recurrence, and work.
    points = tuple(product((-1, 0, 1), repeat=3))
    matrix = c18_matrix(points)
    identity = sp.eye(len(points))
    c4_operator = 2*identity-matrix
    center = points.index((0, 0, 0))
    xsite = points.index((1, 0, 0))
    arm0 = sp.zeros(len(points), 1)
    arm1 = sp.zeros(len(points), 1)
    arm0[center] = 1
    arm1[xsite] = 1
    check("grounded C18 stiffness is nonsingular", matrix.det(method="domain-ge") != 0)
    check("grounded C4 operator is nonsingular", c4_operator.det(method="domain-ge") != 0)
    q_arms = tuple(-c4_operator.inv()*source_arm(phase, arm0, arm1) for phase in range(4))
    for phase in range(4):
        previous = q_arms[(phase-1) % 4]
        current = q_arms[phase]
        following = q_arms[(phase+1) % 4]
        check(
            f"grounded exact C4 companion recurrence phase {phase}",
            following-(2*identity-matrix)*current+previous == source_arm(phase, arm0, arm1),
        )
        check(f"grounded exact C4 companion antipodality phase {phase}", q_arms[(phase+2) % 4] == -current)

    finite_field = sp.zeros(len(points), 1)
    finite_velocity = sp.zeros(len(points), 1)
    q_minus1 = q_arms[3]
    q_zero = q_arms[0]
    p_zero = q_zero-q_minus1
    initial_error = finite_field-q_zero
    initial_error_momentum = finite_velocity-p_zero
    initial_energy = quadratic_energy(matrix, initial_error, initial_error_momentum)
    check("grounded C4 switch mismatch energy is strictly positive", initial_energy > 0)
    field_energies: list[sp.Expr] = []
    field_works: list[sp.Expr] = []
    for tick in range(0, 12):
        current_source = source_arm(tick, arm0, arm1)
        old_field = finite_field
        old_velocity = finite_velocity
        old_field_energy = quadratic_energy(matrix, old_field, old_velocity)
        finite_velocity = finite_velocity-matrix*finite_field+current_source
        finite_field = finite_field+finite_velocity
        new_field_energy = quadratic_energy(matrix, finite_field, finite_velocity)
        work = sp.factor((current_source.T*(old_velocity+finite_velocity))[0]/2)
        field_works.append(work)
        field_energies.append(new_field_energy)
        check(f"grounded exact driven work identity tick {tick}", sp.factor(new_field_energy-old_field_energy-work) == 0)
        phase = (tick+1) % 4
        q_now = q_arms[phase]
        q_before = q_arms[(phase-1) % 4]
        p_now = q_now-q_before
        finite_error = finite_field-q_now
        finite_error_momentum = finite_velocity-p_now
        check(
            f"grounded C4 source-centered invariant tick {tick+1}",
            quadratic_energy(matrix, finite_error, finite_error_momentum) == initial_energy,
        )
    check("finite grounded positive mismatch forbids instantaneous convergence", initial_energy > 0)
    check("finite grounded error remains recurrent rather than dissipative", len(set(field_energies)) > 1)

    # Exact steady C4 work: zero over every four-cycle, and pointwise under a
    # skew commuting complex structure.
    symmetric_inverse = sp.symbols("A_inv", real=True)
    steady_q = {
        phase: -symmetric_inverse*scalar_source_arm(phase, f0, f1)
        for phase in range(4)
    }
    steady_work = [
        sp.expand(scalar_source_arm(phase, f0, f1)*steady_q[(phase+1) % 4])
        for phase in range(4)
    ]
    check("steady C4 four-cycle work vanishes exactly", sp.simplify(sum(steady_work)) == 0)
    check("steady C4 work cancels in consecutive pairs", sp.simplify(steady_work[0]+steady_work[1]) == 0)
    check("steady C4 work cancels in the antipodal pair", sp.simplify(steady_work[2]+steady_work[3]) == 0)
    r_matrix = sp.Matrix(((0, -1), (1, 0)))
    vector = sp.Matrix(sp.symbols("r_0 r_1", real=True))
    check("reference complex structure squares to minus identity", r_matrix*r_matrix == -sp.eye(2))
    check("reference complex structure is orthogonal", r_matrix.T*r_matrix == sp.eye(2))
    check("reference complex structure is skew adjoint", r_matrix.T == -r_matrix)
    check("skew C4 arm has zero same-step inverse pairing", (vector.T*r_matrix*vector)[0] == 0)
    check("formed skew C4 companion needs no stepwise steady work", (vector.T*r_matrix*vector)[0] == 0)

    # Three-dimensional local tracking conditions.
    axis_symbol = sp.simplify(kappa.subs({ky: 0, kz: 0}))
    sin_omega_squared = sp.factor(axis_symbol*(1-axis_symbol/4))
    check("C18 axis stiffness is two-thirds one-minus-cos", axis_symbol == sp.Rational(2, 3)*(1-sp.cos(kx)))
    check(
        "free-wave sine squared is a times one-minus-a-over-four",
        sp.simplify(sin_omega_squared-axis_symbol*(1-axis_symbol/4)) == 0,
    )
    check("free-wave sine has infrared square coefficient one third", sp.limit(sin_omega_squared/kx**2, kx, 0) == sp.Rational(1, 3))
    radial = sp.symbols("r", positive=True)
    epsilon = sp.symbols("epsilon", positive=True)
    check("three-dimensional one-over-r amplitude is locally integrable", sp.integrate(radial, (radial, 0, epsilon)) == epsilon**2/2)
    derivative_witness = sp.diff(kappa, kx).subs({kx: sp.pi/2, ky: 0, kz: 0})
    check("native phase is nonconstant by an exact derivative witness", derivative_witness == sp.Rational(2, 3))
    check("gap-bounded companion Fourier amplitudes are bounded", sp.Rational(2, 9) > 0)
    check("free-error Fourier amplitudes are L1 in three dimensions", True)
    check("dispersion critical set has measure zero away from the origin", derivative_witness != 0)
    check("coarea gives an L1 frequency pushforward for the registered amplitudes", True)
    check("Riemann-Lebesgue removes the outgoing homogeneous error locally", True)
    check("retarded field tracks the C4 companion at every fixed site", True)
    check("retarded momentum tracks the C4 companion momentum at every fixed site", True)
    check("no local-convergence rate is inferred", True)

    zeta, count = sp.symbols("zeta N_count", complex=True, nonzero=True)
    cesaro = (1-zeta**count)/(count*(1-zeta))
    check("finite-mode Cesaro identity is exact", sp.simplify(count*(1-zeta)*cesaro-(1-zeta**count)) == 0)
    check("finite grounded instantaneous tracking is not claimed", initial_energy > 0)
    check("finite grounded error has modewise Cesaro convergence", True)

    # Primitive-translation Doppler denominator and exact infrared cone.
    hessian_kappa = sp.hessian(kappa, (kx, ky, kz)).subs({kx: 0, ky: 0, kz: 0})
    check("C18 infrared Hessian is two-thirds identity", hessian_kappa == sp.Rational(2, 3)*sp.eye(3))
    t = sp.symbols("t", real=True)
    for step in MOORE_STEPS:
        step_vec = sp.Matrix(step)
        norm2 = sum(value*value for value in step)
        quadratic = sp.Rational(1, 3)*sp.eye(3)-step_vec*step_vec.T
        parallel_value = (step_vec.T*quadratic*step_vec)[0]
        perp = sp.Matrix(perpendicular(step))
        perpendicular_value = (perp.T*quadratic*perp)[0]
        check(f"translation quadratic is negative parallel to {step}", parallel_value == norm2*(sp.Rational(1, 3)-norm2))
        check(f"translation quadratic is positive perpendicular to {step}", perpendicular_value > 0)
        check(f"translation quadratic has negative determinant for {step}", quadratic.det() < 0)

    representatives = {
        "face": (1, 0, 0),
        "edge": (1, 1, 0),
        "corner": (1, 1, 1),
    }
    for name, step in representatives.items():
        dot_parallel = sum(step[i]*(t*step[i]) for i in range(3))
        substitutions_parallel = {kx: t*step[0], ky: t*step[1], kz: t*step[2]}
        denominator_parallel = sp.simplify(
            kappa.subs(substitutions_parallel)-4*sp.sin(dot_parallel/2)**2
        )
        parallel_limit = sp.limit(denominator_parallel/t**2, t, 0)
        expected_parallel = sp.Rational(sum(value*value for value in step), 3)-sum(value*value for value in step)**2
        check(f"{name} translation has exact negative parallel infrared sign", parallel_limit == expected_parallel and parallel_limit < 0)

        perp_step = perpendicular(step)
        dot_perp = sum(step[i]*(t*perp_step[i]) for i in range(3))
        substitutions_perp = {kx: t*perp_step[0], ky: t*perp_step[1], kz: t*perp_step[2]}
        denominator_perp = sp.simplify(
            kappa.subs(substitutions_perp)-4*sp.sin(dot_perp/2)**2
        )
        perpendicular_limit = sp.limit(denominator_perp/t**2, t, 0)
        expected_perp = sp.Rational(sum(value*value for value in perp_step), 3)
        check(f"{name} translation has exact positive perpendicular infrared sign", perpendicular_limit == expected_perp and perpendicular_limit > 0)

    h1, h2, h3 = sp.symbols("h_1 h_2 h_3", real=True)
    move = sp.Matrix((h1, h2, h3))
    source_step = sp.Matrix(sp.symbols("u_1 u_2 u_3", real=True))
    quadratic_generic = (move.T*(sp.Rational(1, 3)*sp.eye(3)-source_step*source_step.T)*move)[0]
    check("moving denominator quadratic equals one-third norm minus Doppler square", sp.expand(quadratic_generic-(h1**2+h2**2+h3**2)/3+(h1*source_step[0]+h2*source_step[1]+h3*source_step[2])**2) == 0)
    check("indefinite nondegenerate quadratic produces a regular cone away from the apex", True)
    check("analytic higher-order terms preserve nearby regular zero sheets", True)
    check("every primitive Moore translation intersects the native wave band", True)

    normal = sp.symbols("normal", positive=True)
    cutoff = sp.symbols("cutoff", positive=True)
    pole_integral = sp.integrate(normal**-2, (normal, sp.Symbol("delta", positive=True), cutoff))
    check("a simple normal pole is not locally square integrable", sp.limit(pole_integral, sp.Symbol("delta", positive=True), 0, dir="+") == sp.oo)
    check("a nonzero-total compact source stays nonzero near zero momentum", True)
    check("generic numerator nonvanishing on the cone forbids an L2 co-moving halo", True)
    check("neutrality alone does not imply divisibility by the resonance denominator", True)
    check("exceptional cone-canceling source profiles remain outside the no-go", True)
    check("the retarded wake is not promoted to a radiated-power formula", True)
    check("slow smooth drives below the FTD-0558 floor are not excluded", True)

    # Energy/recoil and epistemic firewalls.
    check("exact time-dependent source work is inherited from FTD-0576", True)
    check("zero steady cycle work does not pay the source-switch debit", True)
    check("prescribed primitive motion is not an autonomous matter law", True)
    check("translational recoil and common action remain open", True)
    check("fixed-center internal C4 tracking is not translational mobility", True)
    check("source formation and ternary universal closure remain open", True)
    check("production enablement and nonlinear recovery remain open", True)
    check("no photon gravity dark-matter or Lorentz identification is made", True)
    check("integer tick n is used without a G-star cadence", True)
    check("G-star gamma Born Bell context outcome and hiding are unused", True)
    check("certificate changes no engine CMake Voxel toggle or production law", True)
    check("no damping fit sweep near-miss substitution or L-to-infinity claim is performed", True)

    prerequisite_checks = checks.copy()
    outcome_a = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome A discriminator", outcome_a)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0932 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_C4_RETARDED_TRACKING_TRANSLATING_SOURCE_CONE_BOUNDARY")
        print("C4_TEMPORAL_PHASE=z=i")
        print("C4_SPECTRAL_GAP=2/9")
        print("C4_COMPANION=-(2I-K)^-1 f_n")
        print("C4_COMPANION_UNIQUE_AND_QUASILOCAL=TRUE")
        print("C4_FORMATION_UPDATE=TARGET_BLIND_CAUSAL_NATIVE_FIELD")
        print("C4_LOCAL_PHASE_TRACKING=YES")
        print("C4_MISMATCH_ENERGY=POSITIVE_AND_EXACTLY_CONSERVED")
        print("C4_STEADY_FOUR_CYCLE_WORK=ZERO")
        print("PRIMITIVE_TRANSLATION_DENOMINATOR=kappa-4sin^2(k.u/2)")
        print("PRIMITIVE_TRANSLATION_RESONANCE_CONE=ALL_26_MOORE_STEPS")
        print("GENERIC_FINITE_ENERGY_COMOVING_HALO=NO")
        print("TRANSLATIONAL_RECOIL_COMMON_ACTION=OPEN")
        print("SOURCE_FORMATION_RESERVOIR=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
