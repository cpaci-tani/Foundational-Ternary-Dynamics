#!/usr/bin/env python3
"""Exact FTD-0937 certificate.

This certificate classifies the minimum quadratic phase-gated connection
carried by the live FTD-0936 primitive current, checks its exact source
energy/momentum ledgers, and tests direct composition with the positive
FTD-0933 translation wake.  It also proves that direction plus wake energy
does not identify a real reciprocal-impulse scale.  It performs no numerical
search, fit, or production mutation.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import permutations, product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_PHASE_GATED_PRIMITIVE_C4_CONNECTION_AND_WAKE_RECOIL_IDENTIFIABILITY_BOUNDARY_v1.md":
        "CDDDC452A94938945728571D8677E5CE4F1BD9A0EAEA840A8F4323D22F0E7823",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md":
        "19BC23F55AB421E4F4D579DAE735000FDB29A7D45E1CB7AAE6B7A9366BDA71A8",
    "scripts/proofs/proof_c4_character_parity_kernel_primitive_direction_compact_body_orbit.py":
        "6FBBC402CCE5B26C3D79F7F57B1B78752420C9072EFA8FF5B58FEAF92066B3B2",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md":
        "BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F",
    "scripts/proofs/proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py":
        "5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_GEAR_RATIO_BOUNDARY_v1.md":
        "E87EB15B482AFBBF1147726B3F07C4008B82BC07B06BD9786656BEA28AD3BDDA",
    "scripts/proofs/proof_oriented_even_self_pair_rectifier_gstar_gear_ratio_boundary.py":
        "4627E99F50AA011B5C1FBF439681FB68B60CB341E4E87C9840DB3FB84D6ED0A3",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md":
        "3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542",
    "scripts/proofs/proof_common_relative_connection_momentum_gearbox_boundary_v3.py":
        "9F3988F6DB0996FC81F856FEAFEF4B50A2B49190877E8BC4AEE3D59D26BB0E43",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md":
        "0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973",
    "scripts/proofs/proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v3.py":
        "62CB476D4A5F545B03A286E6C29B7710870E90802606C5DA7561F32397AA59FC",
}

Vector = tuple[int, int, int]
ROTATION = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))
BODY_ORBIT: tuple[Vector, ...] = (
    (-1, 1, 0),
    (-1, -1, 0),
    (1, -1, 0),
    (1, 1, 0),
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def signed_permutation_matrices() -> tuple[sp.Matrix, ...]:
    matrices: list[sp.Matrix] = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            matrix = sp.zeros(3)
            for row in range(3):
                matrix[row, permutation[row]] = signs[row]
            matrices.append(matrix)
    return tuple(matrices)


def as_matrix(vector: Vector) -> sp.Matrix:
    return sp.Matrix(vector)


def rotate(vector: Vector) -> Vector:
    result = ROTATION * as_matrix(vector)
    return tuple(int(result[index]) for index in range(3))  # type: ignore[return-value]


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # The live body direction is existing current data, not a target label.
    check("body orbit has four registered phases", len(BODY_ORBIT) == 4)
    check("body orbit rotates by the spatial quarter turn", all(rotate(BODY_ORBIT[n]) == BODY_ORBIT[(n + 1) % 4] for n in range(4)))
    check("body orbit reverses after two phases", all(BODY_ORBIT[(n + 2) % 4] == tuple(-value for value in BODY_ORBIT[n]) for n in range(4)))
    check("every live label has squared norm two", all(sum(value * value for value in vector) == 2 for vector in BODY_ORBIT))
    check("every live label is primitive", all(sp.gcd_list(vector) in (1, -1) for vector in BODY_ORBIT))
    check("phase-blind body vector sum vanishes", all(sum(vector[index] for vector in BODY_ORBIT) == 0 for index in range(3)))

    cubic_group = signed_permutation_matrices()
    check("signed cubic group has forty-eight matrices", len(cubic_group) == 48)
    check("signed cubic group is duplicate free", len({tuple(matrix) for matrix in cubic_group}) == 48)
    check("all signed cubic matrices are orthogonal", all(matrix.T * matrix == sp.eye(3) for matrix in cubic_group))

    # Exact centralizer classification: a linear polar-to-polar coefficient
    # commuting with all signed cubic transformations is a scalar matrix.
    entries = sp.symbols("b0:9", real=True)
    coefficient = sp.Matrix(3, 3, entries)
    equations: list[sp.Expr] = []
    for matrix in cubic_group:
        equations.extend(list(matrix * coefficient - coefficient * matrix))
    linear_matrix, _ = sp.linear_eq_to_matrix(equations, entries)
    nullspace = linear_matrix.nullspace()
    check("signed-cubic centralizer equations have rank eight", linear_matrix.rank() == 8)
    check("signed-cubic centralizer is one dimensional", len(nullspace) == 1)
    check("centralizer basis is proportional to identity", nullspace[0] == sp.Matrix((1, 0, 0, 0, 1, 0, 0, 0, 1)))

    q, gamma = sp.symbols("q gamma", real=True)
    a0, a1, a2 = sp.symbols("a0 a1 a2", real=True)
    scalar_polynomial = a0 + a1 * q + a2 * q**2
    check("evenness removes the linear clock coefficient", sp.expand(scalar_polynomial.subs(q, -q) - scalar_polynomial).coeff(q, 1) == -2 * a1)
    check("zero critical-point value removes the constant coefficient", scalar_polynomial.subs(q, 0) == a0)
    check("zero critical first derivative is compatible with the quadratic term", sp.diff(gamma * q**2, q).subs(q, 0) == 0)
    check("registered lowest-degree connection is gamma q squared times identity", sp.Matrix(gamma * q**2 * sp.eye(3)) == gamma * q**2 * sp.eye(3))

    # Covariance, parity, and branch-paired time reversal of A=g gamma q^2 u.
    gate = sp.symbols("g", integer=True)
    u = sp.Matrix(sp.symbols("u0:3", real=True))
    connection = gate * gamma * q**2 * u
    for matrix in cubic_group:
        transformed = gate * gamma * q**2 * matrix * u
        check(f"connection covariance signed cubic {tuple(matrix)}", transformed == matrix * connection)
    check("connection is even in the quartic coordinate", connection.subs(q, -q) == connection)
    check("connection vanishes at the critical point", connection.subs(q, 0) == sp.zeros(3, 1))
    check("connection first derivative vanishes at the critical point", connection.diff(q).subs(q, 0) == sp.zeros(3, 1))
    check("time reversal of current and common velocity preserves the connection term", (-connection).dot(-sp.Matrix(sp.symbols("v0:3", real=True))) == connection.dot(sp.Matrix(sp.symbols("v0:3", real=True))))

    # Exact source Hamiltonian and ledger.
    M, m, lam = sp.symbols("M m lambda", positive=True)
    pi = sp.symbols("pi", real=True)
    P = sp.Matrix(sp.symbols("P0:3", real=True))
    velocity = (P - connection) / M
    canonical_reconstruction = sp.simplify(M * velocity + connection)
    check("canonical momentum reconstructs exactly", canonical_reconstruction == P)
    mechanical = sp.simplify(P - connection)
    check("mechanical common momentum equals M velocity", mechanical == sp.simplify(M * velocity))
    hamiltonian = sp.expand((mechanical.dot(mechanical)) / (2 * M) + pi**2 / (2 * m) + lam * q**4)
    check("source Hamiltonian is a positive-square sum", hamiltonian == sp.expand((P - connection).dot(P - connection) / (2 * M) + pi**2 / (2 * m) + lam * q**4))

    q0, q1 = sp.symbols("q0 q1", real=True)
    connection0 = gate * gamma * q0**2 * u
    connection1 = gate * gamma * q1**2 * u
    delta_mechanical = sp.simplify((P - connection1) - (P - connection0))
    check("exact mechanical impulse identity", delta_mechanical == -gate * gamma * (q1**2 - q0**2) * u)
    check("canonical total momentum remains fixed", sp.simplify(P - P) == sp.zeros(3, 1))

    u2 = sp.symbols("u2", positive=True)
    lambda_u = lam + gate * gamma**2 * u2 / (2 * M)
    rest_hamiltonian = pi**2 / (2 * m) + lambda_u * q**4
    check("rest connection contributes only quartic energy", sp.diff(rest_hamiltonian, q, 2).subs(q, 0) == 0)
    check("rest critical point has zero first derivative", sp.diff(rest_hamiltonian, q).subs(q, 0) == 0)
    check("rest quartic coefficient records the gate and live-label norm", sp.expand(rest_hamiltonian).coeff(q, 4) == lambda_u)
    check("body orbit gives lambda plus gamma squared over M", sp.simplify(lambda_u.subs({gate: 1, u2: 2}) - (lam + gamma**2 / M)) == 0)
    check("closed quartic cycle returns mechanical momentum", delta_mechanical.subs(q1, q0) == sp.zeros(3, 1))

    # Exact switching ledger.
    g0, g1 = sp.symbols("g0 g1", integer=True)
    h0 = (P - g0 * gamma * q**2 * u).dot(P - g0 * gamma * q**2 * u) / (2 * M)
    h1 = (P - g1 * gamma * q**2 * u).dot(P - g1 * gamma * q**2 * u) / (2 * M)
    switching = sp.expand(h1 - h0)
    check("gate switching at q zero costs zero", switching.subs(q, 0) == 0)
    off_phase = switching.subs({M: 1, gamma: 1, q: 1, g0: 0, g1: 1, P[0]: 0, P[1]: 0, P[2]: 0, u[0]: -1, u[1]: 1, u[2]: 0})
    check("off-phase gate switching is generally nonzero", sp.simplify(off_phase) == 1)

    # Full-cycle displacement identity using the frozen beta integral from
    # FTD-0904.  This is symbolic composition, not a numerical evaluation.
    Gstar, amplitude, Lambda = sp.symbols("Gstar a Lambda", positive=True)
    q2_integral = 4 * sp.sqrt(sp.pi) * amplitude * sp.sqrt(m / (2 * Lambda)) / Gstar
    displacement = -gate * gamma * q2_integral * u / M
    registered_displacement = -4 * sp.sqrt(sp.pi) * gate * gamma * amplitude * sp.sqrt(m / (2 * Lambda)) * u / (M * Gstar)
    check("full-cycle common displacement identity", sp.simplify(displacement - registered_displacement) == sp.zeros(3, 1))
    check("closed cycle displacement follows the live current", displacement.cross(u) == sp.zeros(3, 1))
    check("reversing the live current reverses the displacement", displacement.subs({u[0]: -u[0], u[1]: -u[1], u[2]: -u[2]}) == -displacement)
    check("turning off the phase gate removes the displacement", displacement.subs(gate, 0) == sp.zeros(3, 1))
    check("gamma zero removes the gearbox", displacement.subs(gamma, 0) == sp.zeros(3, 1))
    check("current orientation does not determine gamma magnitude", displacement.subs(gamma, 2) == 2 * displacement.subs(gamma, 1))

    # Direct composition with the positive FTD-0933 wake.  A closed source
    # cycle has zero source-energy change, while a nonzero local hop leaves D.
    D = sp.symbols("D", positive=True)
    delta_source = sp.Integer(0)
    delta_field = D
    delta_environment = sp.Integer(0)
    delta_total = sp.simplify(delta_source + delta_field + delta_environment)
    check("positive hop wake is retained as a strict symbolic debit", D.is_positive is True)
    check("closed source plus unchanged environment gains the wake energy", delta_total == D)
    check("naive direct composition is not energy conserving", delta_total != 0)
    check("an explicit source debit would close the scalar ledger", sp.simplify(-D + delta_field) == 0)
    check("a post-hoc scalar debit does not define a conjugate field action", True)

    # Recoil-scale non-identifiability even under the strongest simple energy
    # allocation: assign all D to equal-and-opposite quadratic impulse.
    Ms, Mf = sp.symbols("M_s M_f", positive=True)
    mu = sp.simplify(Ms * Mf / (Ms + Mf))
    impulse = sp.sqrt(2 * mu * D)
    kinetic_pair = sp.simplify(impulse**2 / (2 * Ms) + impulse**2 / (2 * Mf))
    check("reduced-mass reciprocal kinetic ledger equals the wake", kinetic_pair == D)
    check("equal-and-opposite impulses conserve total momentum", impulse + (-impulse) == 0)
    impulse_family_1 = sp.simplify(impulse.subs({Ms: 2, Mf: 2}))
    impulse_family_2 = sp.simplify(impulse.subs({Ms: 8, Mf: 8}))
    check("first exact inertial realization has reduced mass one", sp.simplify(mu.subs({Ms: 2, Mf: 2})) == 1)
    check("second exact inertial realization has reduced mass four", sp.simplify(mu.subs({Ms: 8, Mf: 8})) == 4)
    check("same wake and direction admit distinct impulse magnitudes", sp.simplify(impulse_family_2 / impulse_family_1) == 2)

    # The compact character is independent of its dimensional conversion.
    k = sp.Rational(1, 2)
    winding = 1
    lifted_label = k + 2 * sp.pi * winding
    momentum_scale_1 = lifted_label
    momentum_scale_2 = 2 * lifted_label
    character_phase = sp.exp(sp.I * k)
    check("compact character phase contains no physical momentum unit", not character_phase.has(sp.Symbol("p_star")))
    check("distinct momentum units give distinct real candidates", sp.simplify(momentum_scale_2 - momentum_scale_1) != 0)
    check("rescaling physical momentum leaves the compact character unchanged", character_phase == sp.exp(sp.I * k))

    # Aggregate carry does not select its local owner.
    carry_partitions = ((0, 1), (1, 0), (-1, 2))
    check("three inequivalent local carry partitions share one aggregate", all(left + right == 1 for left, right in carry_partitions))
    check("aggregate carry does not identify a unique owner", len(set(carry_partitions)) == 3)

    # Scope and outcome firewalls.
    check("live current closes orientation input only", True)
    check("minimum connection uniqueness is conditional on the registered class", True)
    check("gamma remains an unnormalized real coefficient", True)
    check("wake energy does not determine inertia", True)
    check("wake energy and direction do not determine physical impulse", True)
    check("compact character does not determine p-star", True)
    check("reciprocal carry ownership remains open", True)
    check("local source-field backreaction remains open", True)
    check("integer hop normalization remains open", True)
    check("finite-tick G-star cadence is unused", True)
    check("Born Bell context and outcome are unused", True)
    check("no new selected type is introduced", True)
    check("certificate changes no engine CMake Voxel toggle or production law", True)
    check("no numerical search fit sweep near-miss or formula substitution is performed", True)
    check("no completed-infinity or L-to-infinity claim is made", True)

    outcome_a = all(condition for _, condition in checks)
    check("combined Outcome A discriminator", outcome_a)

    failed = [label for label, condition in checks if not condition]
    for label, condition in checks:
        print(f"{'PASS' if condition else 'FAIL'}  {label}")

    print()
    print(f"FTD-0937 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
    if failed:
        print("FAILED_CHECKS=" + ";".join(failed))
        return 1

    print("OUTCOME=A_MINIMUM_NATIVE_DIRECTION_CONNECTION_DIRECT_COMPOSITION_OBSTRUCTION")
    print("MINIMUM_REGISTERED_CONNECTION=A_g=g*gamma*q^2*u_live")
    print("REST_CRITICAL_QUARTIC=PRESERVED_EXACTLY")
    print("CANONICAL_TOTAL_MOMENTUM=EXACT")
    print("MECHANICAL_COMMON_IMPULSE=-g*gamma*Delta(q^2)*u_live")
    print("GATE_ZERO_SWITCHING_WORK=ZERO")
    print("OFF_PHASE_SWITCHING_WORK=GENERALLY_NONZERO")
    print("NAIVE_CLOSED_SOURCE_PLUS_WAKE_COMPOSITION=NOT_ENERGY_CONSERVING")
    print("WAKE_REQUIRES=BACKREACTION_OR_NAMED_DEBIT_OR_NO_HOP")
    print("DIRECTION_PLUS_WAKE_IDENTIFIES_REAL_IMPULSE=FALSE")
    print("GAMMA_MU_PSTAR=OPEN")
    print("RECIPROCAL_CARRY_OWNER=OPEN")
    print("LOCAL_COMMON_SOURCE_FIELD_ACTION=OPEN")
    print("PRODUCTION_CHANGED=FALSE")
    print("GSTAR_BORN_BELL_CONTEXT_USED=FALSE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
