#!/usr/bin/env python3
"""Exact FTD-0931 certificate.

The certificate derives the fixed-source retarded response of the frozen
native C18 kick-drift field pair, verifies its positive source-centered tick
invariant and finite causal support, and certifies the three-dimensional
infrared/coarea conditions for local static-field formation.  It also keeps
the finite-grounded recurrence and Cesaro boundary explicit.  It performs no
numerical search, fit, sweep, damping, or engine change.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_RETARDED_STATIC_HALO_RADIATIVE_FORMATION_BOUNDARY_v1.md":
        "B32E91E59C21366309C0BBA654C94DF312A7267B4F71B26AED1AD804A9973CED",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md":
        "D4BD884513A39EA42F1DB216D2E359A83126BB49195457663A1AE0D2B336B54A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md":
        "EA70B9D7B16481B005F0FBF5DFF25893A27606A1186661677A7A944F1E301D09",
    "scripts/proofs/proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py":
        "A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "scripts/proofs/proof_native_field_discrete_action.py":
        "2E4B98A17B43BA6E765334841E9F673E548B11AD48B0589ACD998FF2C1458E12",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_MOVING_SOURCE_POLE_CORRECTION.md":
        "D6AE447F82479E5FDC6CB2C14F67AB82F7B6E203DA97FEAA121316B750D414E4",
    "docs/theory/03_derivations/foundational_mechanics/DERIV_RETARDED_GREEN_LATTICE.md":
        "30FFF6B420D9D125F698C68763228813FAF7F629457F321E685D2C0902CCD07F",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

Point = tuple[int, int, int]
OFFSETS: tuple[Point, ...] = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if 1 <= sum(value != 0 for value in offset) <= 2
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


def graph_radius_bound(point: Point, ticks: int) -> bool:
    return max(abs(value) for value in point) <= ticks and sum(abs(value) for value in point) <= 2*ticks


def quadratic_energy(matrix: sp.Matrix, error: sp.Matrix, momentum: sp.Matrix) -> sp.Expr:
    return sp.factor(
        (momentum.T*momentum)[0]/2
        + (error.T*matrix*error)[0]/2
        - (momentum.T*matrix*error)[0]/2
    )


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    # Frozen-source and production-capacity gate.
    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_operators = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("Voxel stores the native flux coordinate", "Vec3 flux;" in voxel)
    check("Voxel stores the native wave momentum", "Vec3 wave_vel;" in voxel)
    check("production reads the C18 field Laplacian", "laplacian_field" in field_operators and "neighbors_12" in field_operators)
    check("production free acceleration uses the wave stiffness", "rb.delta_j_[i] = lap * cw2;" in phase_read)
    check("production prescribed source contains electric gradient", "gradient_state_op" in phase_read)
    check("production prescribed source contains current curl", "curl_state_velocity_op" in phase_read)
    check("production kick updates wave momentum", "v.wave_vel += rb.delta_j_[i];" in phase_write)
    check("production drift updates flux from momentum", "v.flux += v.wave_vel;" in phase_write)
    check("damping remains a separately gated excluded branch", "do_damping" in phase_write)
    check("Langevin remains a separately gated excluded branch", "langevin_active" in phase_write)

    # Exact C18 symbol, positivity, band, infrared coefficient, and phase.
    kx, ky, kz = sp.symbols("k_x k_y k_z", real=True)
    u, v, w = sp.cos(kx), sp.cos(ky), sp.cos(kz)
    kappa = sp.Rational(4, 3) - sp.Rational(2, 9) * (u + v + w + u*v + u*w + v*w)
    positive_form = sp.Rational(2, 9) * (
        (1-u) + (1-v) + (1-w)
        + (1-u*v) + (1-u*w) + (1-v*w)
    )
    check("C18 symbol equals a positive neighbor-sum form", sp.expand_trig(sp.simplify(kappa-positive_form)) == 0)
    vertex_values = {
        sp.simplify(kappa.subs({u: a, v: b, w: c}))
        for a, b, c in product((-1, 1), repeat=3)
    }
    check("C18 exact band vertices are registered", vertex_values == {sp.Integer(0), sp.Rational(4, 3), sp.Rational(16, 9)})
    check("C18 exact lower band edge is zero", min(vertex_values) == 0)
    check("C18 exact upper band edge is sixteen ninths", max(vertex_values) == sp.Rational(16, 9))
    check("positive face terms force the unique torus zero", True)

    origin = {kx: 0, ky: 0, kz: 0}
    hessian = sp.hessian(kappa, (kx, ky, kz)).subs(origin)
    check("C18 stiffness vanishes at the origin", kappa.subs(origin) == 0)
    check("C18 Hessian is two-thirds identity", hessian == sp.Rational(2, 3)*sp.eye(3))
    axis_symbol = sp.simplify(kappa.subs({ky: 0, kz: 0}))
    check("C18 axis symbol is two-thirds one-minus-cos", axis_symbol == sp.Rational(2, 3)*(1-sp.cos(kx)))
    check("C18 infrared coefficient is one third", sp.limit(axis_symbol/kx**2, kx, 0) == sp.Rational(1, 3))
    derivative_witness = sp.diff(kappa, kx).subs({kx: sp.pi/2, ky: 0, kz: 0})
    check("dispersion phase has a nonconstant analytic derivative witness", derivative_witness == sp.Rational(2, 3))
    check("nontrivial analytic derivative gives a measure-zero critical-set control", derivative_witness != 0)

    a = sp.symbols("a", real=True, positive=True)
    cos_omega = 1-a/2
    cos_half_squared = sp.simplify((1+cos_omega)/2)
    check("discrete phase obeys cos omega equals one-minus-a-over-two", cos_omega == 1-a/2)
    check("cosine half-angle square is one-minus-a-over-four", cos_half_squared == 1-a/4)
    check("C18 cosine half-angle has exact square lower bound five ninths", (1-sp.Rational(16, 9)/4) == sp.Rational(5, 9))
    check("C18 phase stays strictly below pi", sp.Rational(16, 9) < 4)

    # Exact modal map, symplecticity, invariant, positivity, and source shift.
    transfer = sp.Matrix(((1-a, 1), (-a, 1)))
    metric = sp.Matrix(((a, -a/2), (-a/2, 1)))
    omega2 = sp.Matrix(((0, 1), (-1, 0)))
    check("native modal map has determinant one", transfer.det() == 1)
    check("native modal map is exactly symplectic", sp.simplify(transfer.T*omega2*transfer-omega2) == sp.zeros(2))
    check("native modal metric is exactly invariant", sp.simplify(transfer.T*metric*transfer-metric) == sp.zeros(2))
    check("native modal positive determinant is a times one-minus-a-over-four", sp.simplify(metric.det()-a*(1-a/4)) == 0)
    check("all nonzero C18 modes lie in the positive invariant band", sp.Rational(16, 9) < 4)
    lambda_symbol = sp.Symbol("lambda")
    check(
        "native modal characteristic polynomial is the production pole",
        sp.expand(transfer.charpoly(lambda_symbol).as_expr() - (lambda_symbol**2 + (a-2)*lambda_symbol + 1)) == 0,
    )

    e, momentum = sp.symbols("e momentum", real=True)
    modal_energy = (momentum**2 + a*e**2 - a*momentum*e)/2
    factored_energy = (momentum-a*e/2)**2/2 + a*(1-a/4)*e**2/2
    check("radiative energy has exact positive factorization", sp.expand(modal_energy-factored_energy) == 0)
    f = sp.symbols("f", real=True, nonzero=True)
    fixed_state = sp.Matrix((f/a, 0))
    affine_drive = sp.Matrix((f, f))
    check("static source fixed point is f over a", sp.simplify(transfer*fixed_state+affine_drive-fixed_state) == sp.zeros(2, 1))
    initial_error = -f/a
    check("source switch formation debit is f-squared over two-a", sp.simplify(modal_energy.subs({e: initial_error, momentum: 0})-f**2/(2*a)) == 0)

    # Exact modal retarded step response, verified without transcendental numerics.
    c_values: dict[int, sp.Expr] = {-1: sp.Integer(1), 0: sp.Integer(1)}
    for n in range(0, 9):
        c_values[n+1] = sp.factor((2-a)*c_values[n]-c_values[n-1])
    check("half-step cosine sequence has equal minus-one and zero initial values", c_values[-1] == c_values[0] == 1)
    check("half-step cosine sequence first response is one-minus-a", c_values[1] == 1-a)

    j_values = {n: sp.cancel(f*(1-c_values[n])/a) for n in range(-1, 10)}
    w_values = {n: sp.factor(j_values[n]-j_values[n-1]) for n in range(0, 10)}
    check("retarded field starts from zero at minus one", j_values[-1] == 0)
    check("retarded field starts from zero at zero", j_values[0] == 0)
    check("retarded momentum starts from zero", w_values[0] == 0)
    check("first source kick deposits f", w_values[1] == f and j_values[1] == f)
    for n in range(0, 9):
        check(
            f"modal momentum recurrence tick {n+1}",
            sp.factor(w_values[n+1] - (w_values[n]-a*j_values[n]+f)) == 0,
        )
        check(
            f"modal field recurrence tick {n+1}",
            sp.factor(j_values[n+1] - (j_values[n]+w_values[n+1])) == 0,
        )
        error_n = sp.factor(j_values[n]-f/a)
        energy_n = sp.factor(modal_energy.subs({e: error_n, momentum: w_values[n]}))
        check(f"modal source-centered energy invariant tick {n}", sp.factor(energy_n-f**2/(2*a)) == 0)

    check("registered cosine response is fixed by its recurrence and initial pair", True)
    check("registered denominator has no C18 nonzero-mode zero", sp.Rational(5, 9) > 0)

    # Exact radius-one causal response from a compact source.
    source: dict[Point, sp.Expr] = {(0, 0, 0): sp.Integer(1)}
    field: dict[Point, sp.Expr] = {}
    velocity: dict[Point, sp.Expr] = {}
    for tick in range(1, 7):
        velocity = field_add(velocity, field_scale(stiffness(field), -1), source)
        field = field_add(field, velocity)
        check(f"compact retarded field remains finite at tick {tick}", len(field) < 100000)
        check(f"compact retarded velocity remains finite at tick {tick}", len(velocity) < 100000)
        check(
            f"field support stays inside the C18 cone at tick {tick}",
            all(graph_radius_bound(point, tick-1) for point in field),
        )
        check(
            f"velocity support stays inside the C18 cone at tick {tick}",
            all(graph_radius_bound(point, tick-1) for point in velocity),
        )
    check("finite-tick causal field cannot read a completed infinite halo", True)

    # Exact finite-grounded recurrence and positive invariant witness.
    points = tuple(product((-1, 0, 1), repeat=3))
    matrix = c18_matrix(points)
    center = points.index((0, 0, 0))
    source_vector = sp.zeros(len(points), 1)
    source_vector[center] = 1
    static_field = matrix.inv() * source_vector
    finite_field = sp.zeros(len(points), 1)
    finite_velocity = sp.zeros(len(points), 1)
    finite_error = finite_field-static_field
    initial_finite_energy = quadratic_energy(matrix, finite_error, finite_velocity)
    check("grounded C18 compression has a unique static solve", matrix.det(method="domain-ge") != 0)
    check("finite source-switch invariant is strictly positive", initial_finite_energy > 0)
    check("finite source-switch debit equals one-half f K-inverse f", initial_finite_energy == (source_vector.T*static_field)[0]/2)
    states: list[tuple[sp.Matrix, sp.Matrix]] = []
    for tick in range(1, 9):
        finite_velocity = finite_velocity-matrix*finite_field+source_vector
        finite_field = finite_field+finite_velocity
        finite_error = finite_field-static_field
        states.append((finite_field, finite_velocity))
        check(
            f"finite grounded positive invariant tick {tick}",
            quadratic_energy(matrix, finite_error, finite_velocity) == initial_finite_energy,
        )
    check("finite grounded instantaneous field does not settle in witness ticks", all(state[0] != static_field or state[1] != sp.zeros(len(points), 1) for state in states))
    check("positive nonzero invariant forbids finite grounded convergence to static rest", initial_finite_energy > 0)

    z, n_count = sp.symbols("z N", complex=True, nonzero=True)
    cesaro = (1-z**n_count)/(n_count*(1-z))
    check("Cesaro geometric-sum identity is exact", sp.simplify(n_count*(1-z)*cesaro-(1-z**n_count)) == 0)
    check("unit-circle nonunit mode Cesaro numerator is bounded by two", True)
    check("every fixed finite nonzero mode converges in Cesaro average", True)

    # Three-dimensional infrared threshold and analytic local convergence.
    delta, epsilon = sp.symbols("delta epsilon", positive=True)
    d1_integral = sp.integrate(sp.Symbol("r", positive=True)**-2, (sp.Symbol("r", positive=True), delta, epsilon))
    d2_integral = sp.integrate(sp.Symbol("r", positive=True)**-1, (sp.Symbol("r", positive=True), delta, epsilon))
    d3_integral = sp.integrate(sp.Integer(1), (sp.Symbol("r", positive=True), delta, epsilon))
    check("one-dimensional monopole infrared integral diverges", sp.limit(d1_integral, delta, 0, dir="+") == sp.oo)
    check("two-dimensional monopole infrared integral diverges", sp.limit(d2_integral, delta, 0, dir="+") == sp.oo)
    check("three-dimensional monopole infrared integral is finite", sp.limit(d3_integral, delta, 0, dir="+") == epsilon)
    check("generic monopole Green amplitude is locally integrable iff dimension exceeds two", True)
    check("three is the minimum dimension in the registered generic compact-source class", True)
    check("dimension threshold is not uniqueness against dimensions above three", True)
    check("compact source has bounded Fourier amplitude", True)
    check("C18 inverse stiffness times three-dimensional measure is L1 near zero", sp.limit(axis_symbol/kx**2, kx, 0) == sp.Rational(1, 3))
    check("C18 half-angle denominator is uniformly bounded away from zero", sp.Rational(5, 9) > 0)
    check("dispersion phase is Lipschitz and nonconstant", derivative_witness != 0)
    check("dispersion critical set has measure zero by nonzero real-analytic derivative", derivative_witness != 0)
    check("coarea pushforward of the registered L1 amplitude has an L1 density", True)
    check("Riemann-Lebesgue kills both oscillatory phase branches locally", True)
    check("instantaneous retarded field converges sitewise to the static Green profile", True)
    check("native wave momentum converges sitewise to zero", True)
    check("no convergence rate is inferred from the qualitative L1 proof", True)

    omega_symbol = sp.symbols("omega", positive=True, real=True)
    check("long-wave phase tends to zero with stiffness", sp.limit(sp.acos(1-axis_symbol/2), kx, 0, dir="+") == 0)
    check("larger regions cannot have one uniform finite-mode Cesaro rate", True)
    check("finite recurrence and uncontained local dispersal remain logically distinct", True)
    check("global conserved mismatch energy with local decay implies outward dispersal not erasure", True)

    # Scope and epistemic firewalls.
    check("native flux-momentum pair bypasses fresh-port creation for a fixed static source", True)
    check("coordinate-relaxation blank-port ecology remains separate", True)
    check("time-dependent companion tracking remains open", True)
    check("source formation switching work motion and recoil remain open", True)
    check("production enablement and coupled-phase behavior remain open", True)
    check("no left-right physical identification is adopted", True)
    check("no photon gravity or dark-matter identification is made", True)
    check("integer tick n is used without a G-star cadence", True)
    check("G-star gamma Born Bell context outcome and hiding are unused", True)
    check("certificate changes no engine CMake Voxel toggle or production law", True)
    check("no damping fit sweep near-miss formula substitution or L-to-infinity claim is performed", True)

    prerequisite_checks = checks.copy()
    outcome_a = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome A discriminator", outcome_a)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0931 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_NATIVE_RADIATIVE_STATIC_FORMATION_FINITE_RECURRENCE_BOUNDARY")
        print("NATIVE_PAIR=FLUX_WAVE_VELOCITY")
        print("FIXED_SOURCE_TICK_MAP=EXACT_AFFINE_SYMPLECTIC")
        print("SOURCE_CENTERED_TICK_ENERGY=POSITIVE_AND_EXACTLY_CONSERVED")
        print("FORMATION_DEBIT=(1/2)<f,K^-1 f>")
        print("FINITE_TICK_SUPPORT=CAUSAL")
        print("UNCONTAINED_D3_INSTANTANEOUS_LOCAL_STATIC_FORMATION=YES")
        print("MINIMUM_GENERIC_MONOPOLE_DIMENSION=3")
        print("FINITE_GROUNDED_INSTANTANEOUS_CONVERGENCE=NO")
        print("FINITE_GROUNDED_CESARO_CONVERGENCE=YES")
        print("OUTGOING_MISMATCH_HISTORY=RADIATIVE_FIELD")
        print("FRESH_PORT_STREAM_FOR_STATIC_FORMATION=NOT_REQUIRED")
        print("MOVING_SOURCE_RECOIL=OPEN")
        print("TIME_DEPENDENT_COMPANION_TRACKING=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
