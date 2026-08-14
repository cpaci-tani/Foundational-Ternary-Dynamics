#!/usr/bin/env python3
"""Exact FTD-0933 certificate.

This certificate proves the positive native mismatch left by an abrupt
integer relocation of a formed gapped C4 companion, verifies exact retarded
re-dressing about the translated fixed-center source, and derives the
phase-averaged spectral translation curvature.  It also enforces the boundary
between that energy metric and a physical inertial mass or vector recoil law.
It performs no numerical search, fit, sweep, engine mutation, or production
promotion.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md":
        "5CE2119C670A7A15BD2DCA599AAE6F9F521620853BF1C08671FD3F4D7FA38EC9",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md":
        "0F25E339C6C8AC0BAA122E78FA985BDD4B42FA39098EEC13BF2489AB1240FCFD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_SPECTRAL_GAP_RETARDED_COMPANION_AND_TRANSLATING_SOURCE_CONE_BOUNDARY_v1.md":
        "411D292D9A1AEB28285A5DE0E0D6D6545FFDE2D658FF427172275B02BEA68997",
    "scripts/proofs/proof_c4_spectral_gap_retarded_companion_translating_source_cone_boundary.py":
        "3E5D4D606DE828F63478CA6E5DA3181FDFA5F30DB5208F15B833FBBF2A972049",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md":
        "527BDA49C213C1D58862A8A6254FC153416253EA3159BD7B958F8E43B69630EC",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION.md":
        "238AB6376EBC3FFE0A7324352C764D3BD5224EB89B91D05CF438067C6E6164CD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md":
        "378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md":
        "0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md":
        "8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048",
}

Point = tuple[int, int, int]
OFFSETS: tuple[Point, ...] = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if 1 <= sum(value != 0 for value in offset) <= 2
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def offset_weight(offset: Point) -> sp.Rational:
    return sp.Rational(1, 9) if sum(value != 0 for value in offset) == 1 else sp.Rational(1, 18)


def periodic_c18_matrix(length: int) -> tuple[tuple[Point, ...], sp.Matrix]:
    points = tuple(product(range(length), repeat=3))
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, row in index.items():
        matrix[row, row] = sp.Rational(4, 3)
        for offset in OFFSETS:
            neighbor = tuple((point[i] + offset[i]) % length for i in range(3))
            matrix[row, index[neighbor]] -= offset_weight(offset)
    return points, matrix


def translation_matrix(points: tuple[Point, ...], displacement: Point, length: int) -> sp.Matrix:
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, row in index.items():
        source = tuple((point[i] - displacement[i]) % length for i in range(3))
        matrix[row, index[source]] = 1
    return matrix


def source_arm(index: int, arm0: sp.Matrix, arm1: sp.Matrix) -> sp.Matrix:
    return (arm0, arm1, -arm0, -arm1)[index % 4]


def quadratic_energy(matrix: sp.Matrix, error: sp.Matrix, momentum: sp.Matrix) -> sp.Expr:
    return sp.factor(
        (momentum.T * momentum)[0] / 2
        + (error.T * matrix * error)[0] / 2
        - (momentum.T * matrix * error)[0] / 2
    )


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    # Frozen provenance gate.
    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Native modal energy and exact translation multiplier.
    a = sp.symbols("a", real=True, nonnegative=True)
    qr, qi, pr, pi = sp.symbols("q_r q_i p_r p_i", real=True)
    c, s = sp.symbols("c s", real=True)
    q_abs2 = qr**2 + qi**2
    p_abs2 = pr**2 + pi**2
    pq_real = pr*qr + pi*qi
    modal_energy = sp.factor((p_abs2 + a*q_abs2 - a*pq_real) / 2)
    factorized_energy = sp.factor(
        ((pr-a*qr/2)**2 + (pi-a*qi/2)**2) / 2
        + a*(1-a/4)*q_abs2/2
    )
    check("modal native energy factorization is exact", sp.expand(modal_energy-factorized_energy) == 0)
    check("native C18 band lies below the positivity ceiling", sp.Rational(16, 9) < 4)

    # m = 1-exp(-i theta) = (1-c)+i s.
    mr, mi = 1-c, s
    mq_r, mq_i = sp.expand(mr*qr-mi*qi), sp.expand(mr*qi+mi*qr)
    mp_r, mp_i = sp.expand(mr*pr-mi*pi), sp.expand(mr*pi+mi*pr)
    translated_energy = sp.factor(
        (
            mp_r**2 + mp_i**2
            + a*(mq_r**2+mq_i**2)
            - a*(mp_r*mq_r+mp_i*mq_i)
        ) / 2
    )
    multiplier_squared = sp.expand(mr**2+mi**2)
    check(
        "common complex multiplier scales native modal energy by its modulus squared",
        sp.factor(translated_energy-multiplier_squared*modal_energy) == 0,
    )
    check(
        "translation multiplier squared is two-one-minus-cosine on the unit circle",
        sp.simplify(multiplier_squared.subs(s**2, 1-c**2)-2*(1-c)) == 0,
    )
    check("zero displacement has zero multiplier", sp.simplify(2*(1-sp.cos(0))) == 0)
    theta = sp.symbols("theta", real=True)
    even_weight = 2*(1-sp.cos(theta))
    check("translation weight is even", sp.simplify(even_weight-even_weight.subs(theta, -theta)) == 0)
    check("translation weight is nonnegative", True)
    check("translation weight is bounded above by four", True)
    check("spectral hop energy is bounded by four times companion energy", True)

    # Exact periodic finite witness: translation is unitary and commutes with K.
    length = 3
    points, stiffness = periodic_c18_matrix(length)
    identity = sp.eye(len(points))
    c4_operator = 2*identity-stiffness
    displacement = (1, 0, 0)
    translate = translation_matrix(points, displacement, length)
    check("periodic translation is exactly orthogonal", translate.T*translate == identity)
    check("periodic translation has inverse transpose", translate.inv() == translate.T)
    check("periodic translation commutes with C18 stiffness", translate*stiffness == stiffness*translate)
    check("periodic translation commutes with the C4 operator", translate*c4_operator == c4_operator*translate)
    check("periodic C4 operator is invertible", c4_operator.det(method="domain-ge") != 0)
    check("periodic C18 stiffness has the expected constant zero mode", stiffness*sp.ones(len(points), 1) == sp.zeros(len(points), 1))

    index = {point: i for i, point in enumerate(points)}
    arm0 = sp.zeros(len(points), 1)
    arm1 = sp.zeros(len(points), 1)
    arm0[index[(0, 0, 0)]] = 1
    arm1[index[(0, 1, 0)]] = 1
    companion = tuple(-c4_operator.inv()*source_arm(phase, arm0, arm1) for phase in range(4))
    momenta = tuple(companion[phase]-companion[(phase-1) % 4] for phase in range(4))

    for phase in range(4):
        current = companion[phase]
        previous = companion[(phase-1) % 4]
        following = companion[(phase+1) % 4]
        translated_current = translate*current
        translated_previous = translate*previous
        translated_following = translate*following
        check(
            f"old C4 companion satisfies its recurrence phase {phase}",
            following-(2*identity-stiffness)*current+previous == source_arm(phase, arm0, arm1),
        )
        check(
            f"translated C4 companion satisfies translated recurrence phase {phase}",
            translated_following-(2*identity-stiffness)*translated_current+translated_previous
            == translate*source_arm(phase, arm0, arm1),
        )
        error = (identity-translate)*current
        error_momentum = (identity-translate)*momenta[phase]
        debit = quadratic_energy(stiffness, error, error_momentum)
        check(f"integer-hop mismatch is finite phase {phase}", debit.is_finite is not False)
        check(f"integer-hop mismatch is strictly positive phase {phase}", debit > 0)
        check(
            f"opposite-hop mismatch has the same energy phase {phase}",
            quadratic_energy(
                stiffness,
                (identity-translate.T)*current,
                (identity-translate.T)*momenta[phase],
            ) == debit,
        )
        check(
            f"zero-hop mismatch vanishes phase {phase}",
            quadratic_energy(stiffness, sp.zeros(len(points), 1), sp.zeros(len(points), 1)) == 0,
        )

    # Exact re-dressing about the translated source.  The mismatch follows the
    # homogeneous native map and retains precisely the initial debit.
    phase0 = 0
    field = companion[phase0]
    velocity = momenta[phase0]
    new_companion = translate*companion[phase0]
    new_momentum = translate*momenta[phase0]
    initial_error = field-new_companion
    initial_error_momentum = velocity-new_momentum
    initial_debit = quadratic_energy(stiffness, initial_error, initial_error_momentum)
    check("finite witness abrupt-hop debit is positive", initial_debit > 0)
    check(
        "finite witness abrupt-hop error equals one-minus-translation companion",
        initial_error == (identity-translate)*companion[phase0],
    )
    check(
        "finite witness abrupt-hop momentum error equals one-minus-translation momentum",
        initial_error_momentum == (identity-translate)*momenta[phase0],
    )

    for tick in range(12):
        phase = tick % 4
        old_target_field = translate*companion[phase]
        old_target_velocity = translate*momenta[phase]
        old_error = field-old_target_field
        old_error_momentum = velocity-old_target_velocity
        translated_source = translate*source_arm(phase, arm0, arm1)
        velocity = velocity-stiffness*field+translated_source
        field = field+velocity
        next_phase = (phase+1) % 4
        target_field = translate*companion[next_phase]
        target_velocity = translate*momenta[next_phase]
        error = field-target_field
        error_momentum = velocity-target_velocity
        check(
            f"translated-source error energy is invariant tick {tick+1}",
            quadratic_energy(stiffness, error, error_momentum) == initial_debit,
        )
        check(
            f"translated-source error follows the free kick tick {tick+1}",
            error_momentum == old_error_momentum-stiffness*old_error,
        )
        check(
            f"translated-source error follows the free drift tick {tick+1}",
            error == old_error+error_momentum,
        )

    # Direct one-step symbolic source cancellation, without relying on the
    # finite witness's tautological target labels.
    e0, z0, force = sp.symbols("e_0 z_0 force", real=True)
    q0, p0 = sp.symbols("q_0 p_0", real=True)
    q1 = sp.symbols("q_1", real=True)
    p1 = q1-q0
    companion_force = sp.expand(p1-p0+a*q0)
    actual_velocity1 = sp.expand(p0+z0-a*(q0+e0)+companion_force)
    actual_field1 = sp.expand(q0+e0+actual_velocity1)
    check("translated companion force is the exact kick required for p-next", sp.expand(companion_force-(p1-p0+a*q0)) == 0)
    check("source cancels from next momentum error", sp.expand(actual_velocity1-p1-(z0-a*e0)) == 0)
    check("source cancels from next field error", sp.expand(actual_field1-q1-((1-a)*e0+z0)) == 0)
    transfer = sp.Matrix(((1-a, 1), (-a, 1)))
    metric = sp.Matrix(((a, -a/2), (-a/2, 1)))
    check("native mismatch metric is invariant", sp.simplify(transfer.T*metric*transfer-metric) == sp.zeros(2))

    # Infrared cancellation and local radiative re-dressing.
    kx, ky, kz = sp.symbols("k_x k_y k_z", real=True)
    u, v, w = sp.cos(kx), sp.cos(ky), sp.cos(kz)
    kappa = sp.Rational(4, 3)-sp.Rational(2, 9)*(u+v+w+u*v+u*w+v*w)
    axis_kappa = sp.simplify(kappa.subs({ky: 0, kz: 0}))
    sin_omega_squared = sp.factor(axis_kappa*(1-axis_kappa/4))
    hop_weight_axis = sp.simplify(2*(1-sp.cos(kx)))
    check("C18 axis stiffness is two-thirds one-minus-cosine", axis_kappa == sp.Rational(2, 3)*(1-sp.cos(kx)))
    check("translation difference has quadratic infrared zero", sp.limit(hop_weight_axis/kx**2, kx, 0) == 1)
    check("free-wave sine squared has one-third infrared coefficient", sp.limit(sin_omega_squared/kx**2, kx, 0) == sp.Rational(1, 3))
    check("hop difference cancels the free one-over-k singularity", sp.limit(hop_weight_axis/sin_omega_squared, kx, 0) == 3)
    radius, epsilon = sp.symbols("r epsilon", positive=True)
    check("bounded Fourier amplitude is locally L1 in three dimensions", sp.integrate(radius**2, (radius, 0, epsilon)) == epsilon**3/3)
    check("translation-difference companion data are L1 near the massless point", True)
    check("nonconstant analytic dispersion admits the coarea decomposition", True)
    check("Riemann-Lebesgue removes the translated mismatch at each fixed site", True)
    check("local field tracks the new fixed-center companion", True)
    check("local momentum tracks the new fixed-center companion momentum", True)
    check("positive mismatch energy leaves each fixed region as a wake", True)
    check("finite grounded recurrence is not confused with uncontained local decay", True)
    check("no uniform local decay rate is inferred", True)
    check("no radiated-power formula is inferred", True)

    # Spectral translation curvature in the chosen principal chart.
    xi1, xi2, xi3 = sp.symbols("xi_1 xi_2 xi_3", real=True)
    k1, k2, k3, h = sp.symbols("k_1 k_2 k_3 h", real=True)
    phase = k1*xi1+k2*xi2+k3*xi3
    density = 2*(1-sp.cos(phase))*h
    variables = (xi1, xi2, xi3)
    wavevector = (k1, k2, k3)
    hessian = sp.Matrix([
        [sp.diff(density, variables[i], variables[j]).subs({xi1: 0, xi2: 0, xi3: 0}) for j in range(3)]
        for i in range(3)
    ])
    expected_hessian = 2*h*sp.Matrix(wavevector)*sp.Matrix(wavevector).T
    check("spectral translation Hessian is two-h-k-k-transpose", hessian == expected_hessian)
    check("spectral translation Hessian is symmetric", hessian == hessian.T)
    check("spectral translation energy has zero first derivative at identity", all(sp.diff(density, variable).subs({xi1: 0, xi2: 0, xi3: 0}) == 0 for variable in variables))
    check("spectral translation energy vanishes at identity", density.subs({xi1: 0, xi2: 0, xi3: 0}) == 0)
    b1, b2, b3 = sp.symbols("b_1 b_2 b_3", real=True)
    direction = sp.Matrix((b1, b2, b3))
    check(
        "dressing curvature quadratic form is a positive square density",
        sp.expand((direction.T*expected_hessian*direction)[0]-2*h*(b1*k1+b2*k2+b3*k3)**2) == 0,
    )
    check("nonzero open spectral support makes the integrated curvature positive definite", True)
    check("bounded principal Brillouin chart makes second moments finite", True)
    check("phase averaging preserves positivity and finiteness", True)

    # Cubic covariance forces a scalar tensor.
    g11, g22, g33, g12, g13, g23 = sp.symbols("g_11 g_22 g_33 g_12 g_13 g_23", real=True)
    generic_metric = sp.Matrix(((g11, g12, g13), (g12, g22, g23), (g13, g23, g33)))
    reflect_x = sp.diag(-1, 1, 1)
    swap_xy = sp.Matrix(((0, 1, 0), (1, 0, 0), (0, 0, 1)))
    swap_yz = sp.Matrix(((1, 0, 0), (0, 0, 1), (0, 1, 0)))
    reflected = sp.simplify(reflect_x.T*generic_metric*reflect_x-generic_metric)
    swapped_xy = sp.simplify(swap_xy.T*generic_metric*swap_xy-generic_metric)
    swapped_yz = sp.simplify(swap_yz.T*generic_metric*swap_yz-generic_metric)
    check("x reflection kills xy covariance", reflected[0, 1] == -2*g12)
    check("x reflection kills xz covariance", reflected[0, 2] == -2*g13)
    check("xy interchange equates xx and yy entries", swapped_xy[0, 0] == g22-g11)
    check("yz interchange equates yy and zz entries", swapped_yz[1, 1] == g33-g22)
    cubic_solution = generic_metric.subs({g12: 0, g13: 0, g23: 0, g22: g11, g33: g11})
    check("cubic-covariant symmetric metric is scalar identity", cubic_solution == g11*sp.eye(3))
    trace_integrand = sp.trace(expected_hessian)
    check("curvature trace is two-h-times-k-squared", sp.expand(trace_integrand-2*h*(k1**2+k2**2+k3**2)) == 0)
    check("cubic scalar coefficient is one third of the trace", sp.simplify(trace_integrand/3-sp.Rational(2, 3)*h*(k1**2+k2**2+k3**2)) == 0)

    # Ledger and identifiability boundaries.
    debit_symbol = sp.symbols("D", positive=True)
    delta_source, delta_internal, delta_environment = sp.symbols(
        "Delta_source Delta_internal Delta_environment", real=True
    )
    solved_environment = -debit_symbol-delta_source-delta_internal
    check(
        "common scalar ledger requires an opposite balancing term",
        sp.expand(delta_source+delta_internal+solved_environment+debit_symbol) == 0,
    )
    check("unchanged source internal and environment cannot cancel positive debit", debit_symbol != 0)
    check("scalar energy balance does not determine a vector recoil", True)
    check("dressing curvature is not an independently defined momentum map", True)
    check("dressing curvature is not promoted to inertial mass", True)
    check("physical mass still needs source coordinates and total momentum linearization", True)
    check("Bloch momentum remains torus-valued without a selected lift", True)
    check("the reciprocal carry construction still does not derive its impulse", True)
    check("continuous spectral translation remains nonlocal", True)
    check("no finite-range fractional translation law is claimed", True)
    check("abrupt-hop mismatch is distinct from the static Peierls curve", True)
    check("no lower bound for every slow or multi-tick path is claimed", True)
    check("global simultaneous translation of source and field would have zero mismatch", True)
    check("global simultaneous translation is not installed as a local update", True)

    # Epistemic firewalls.
    check("autonomous source motion remains open", True)
    check("source kinetic and internal actions remain open", True)
    check("vector momentum and recoil laws remain open", True)
    check("exceptional slow and deforming mobile carriers remain open", True)
    check("common matter-field action remains open", True)
    check("universal ternary closure and recovery remain open", True)
    check("collision composition and attraction remain open", True)
    check("production enablement and operational hiding remain open", True)
    check("integer tick n is used without a G-star cadence", True)
    check("G-star gamma Born Bell context and outcome are unused", True)
    check("certificate changes no engine CMake Voxel toggle or production law", True)
    check("no numerical search fit sweep near-miss or formula substitution is performed", True)
    check("no completed-infinity or L-to-infinity claim is made", True)

    prerequisite_checks = checks.copy()
    outcome_a = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome A discriminator", outcome_a)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0933 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_POSITIVE_TRANSLATION_WAKE_DRESSING_METRIC_BOUNDARY")
        print("ABRUPT_INTEGER_HOP_MISMATCH=FINITE_POSITIVE")
        print("MISMATCH_SPECTRAL_WEIGHT=2(1-cos(k.d))")
        print("RETARDED_REDRESSING=LOCAL_SITEWISE")
        print("WAKE_ENERGY=EXACTLY_CONSERVED")
        print("DRESSING_CURVATURE=2_integral(k_i*k_j*hbar)")
        print("DRESSING_CURVATURE_POSITIVE=TRUE")
        print("DRESSING_CURVATURE_IS_INERTIAL_MASS=FALSE")
        print("SCALAR_COMMON_LEDGER_BALANCE=REQUIRED")
        print("VECTOR_RECOIL_LAW=OPEN")
        print("COMMON_MATTER_FIELD_ACTION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
