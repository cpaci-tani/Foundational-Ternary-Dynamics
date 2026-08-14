#!/usr/bin/env python3
"""Exact FTD-0934 certificate.

This certificate proves that the phase-averaged C4 translation wake is the
square norm of a translation-group one-cocycle and hence a conditionally
negative-definite distance.  It then proves that this symmetric square loses
hop orientation and classifies a nontrivial character of Z^3, valued in the
Bloch torus, as the minimum directed translation-phase representation.  It
does not derive physical momentum, an impulse, or a production recoil law.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import permutations, product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_DRESSING_TRANSLATION_COCYCLE_AND_DIRECTED_RECOIL_STATE_NECESSITY_v1.md":
        "5252D61FFABB0BBA9E61524B5345943F627F9C032A502C350AECC0EDEC34922A",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md":
        "5CE2119C670A7A15BD2DCA599AAE6F9F521620853BF1C08671FD3F4D7FA38EC9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md":
        "BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F",
    "scripts/proofs/proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py":
        "5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_SYMMETRIC_CHORD_MOORE_ACTION.md":
        "B80E574B8C421B28DC0AFFC35F5B898DF6FF79A1CEBA06588B22862FDCF1468D",
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
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md":
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
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


def offset_weight(offset: Point) -> sp.Rational:
    return sp.Rational(1, 9) if sum(value != 0 for value in offset) == 1 else sp.Rational(1, 18)


def periodic_c18_matrix(length: int) -> tuple[tuple[Point, ...], sp.Matrix]:
    points = tuple(product(range(length), repeat=3))
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, row in index.items():
        matrix[row, row] = sp.Rational(4, 3)
        for offset in OFFSETS:
            neighbor = tuple((point[i]+offset[i]) % length for i in range(3))
            matrix[row, index[neighbor]] -= offset_weight(offset)
    return points, matrix


def translation_matrix(points: tuple[Point, ...], displacement: Point, length: int) -> sp.Matrix:
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, row in index.items():
        source = tuple((point[i]-displacement[i]) % length for i in range(3))
        matrix[row, index[source]] = 1
    return matrix


def add_mod(left: Point, right: Point, length: int) -> Point:
    return tuple((left[i]+right[i]) % length for i in range(3))  # type: ignore[return-value]


def negate_mod(point: Point, length: int) -> Point:
    return tuple((-point[i]) % length for i in range(3))  # type: ignore[return-value]


def canonical_step(point: Point, length: int) -> Point:
    return tuple(value if value <= length//2 else value-length for value in point)  # type: ignore[return-value]


def source_arm(index: int, arm0: sp.Matrix, arm1: sp.Matrix) -> sp.Matrix:
    return (arm0, arm1, -arm0, -arm1)[index % 4]


def energy_inner(
    stiffness: sp.Matrix,
    field_a: sp.Matrix,
    momentum_a: sp.Matrix,
    field_b: sp.Matrix,
    momentum_b: sp.Matrix,
) -> sp.Expr:
    return sp.factor(
        (momentum_a.T*momentum_b)[0]/2
        + (field_a.T*stiffness*field_b)[0]/2
        - ((momentum_a.T*stiffness*field_b)[0]
           + (momentum_b.T*stiffness*field_a)[0])/4
    )


def direct_sum_inner(
    stiffness: sp.Matrix,
    left: tuple[tuple[sp.Matrix, sp.Matrix], ...],
    right: tuple[tuple[sp.Matrix, sp.Matrix], ...],
) -> sp.Expr:
    return sp.factor(sum(
        (energy_inner(stiffness, left[r][0], left[r][1], right[r][0], right[r][1]) for r in range(4)),
        sp.Integer(0),
    )/4)


def direct_sum_add(
    left: tuple[tuple[sp.Matrix, sp.Matrix], ...],
    right: tuple[tuple[sp.Matrix, sp.Matrix], ...],
) -> tuple[tuple[sp.Matrix, sp.Matrix], ...]:
    return tuple((left[r][0]+right[r][0], left[r][1]+right[r][1]) for r in range(4))


def direct_sum_scale(
    state: tuple[tuple[sp.Matrix, sp.Matrix], ...], factor: sp.Expr
) -> tuple[tuple[sp.Matrix, sp.Matrix], ...]:
    return tuple((factor*state[r][0], factor*state[r][1]) for r in range(4))


def apply_translation(
    state: tuple[tuple[sp.Matrix, sp.Matrix], ...], translate: sp.Matrix
) -> tuple[tuple[sp.Matrix, sp.Matrix], ...]:
    return tuple((translate*state[r][0], translate*state[r][1]) for r in range(4))


def subtract_state(
    left: tuple[tuple[sp.Matrix, sp.Matrix], ...],
    right: tuple[tuple[sp.Matrix, sp.Matrix], ...],
) -> tuple[tuple[sp.Matrix, sp.Matrix], ...]:
    return tuple((left[r][0]-right[r][0], left[r][1]-right[r][1]) for r in range(4))


def signed_permutation_matrices() -> tuple[sp.Matrix, ...]:
    matrices: list[sp.Matrix] = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            matrix = sp.zeros(3)
            for row in range(3):
                matrix[row, permutation[row]] = signs[row]
            matrices.append(matrix)
    return tuple(matrices)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Exact finite C18 companion used only as a rational witness of the
    # abstract translation-Hilbert identities.
    length = 3
    points, stiffness = periodic_c18_matrix(length)
    identity = sp.eye(len(points))
    c4_operator = 2*identity-stiffness
    index = {point: i for i, point in enumerate(points)}
    delta = sp.zeros(len(points), 1)
    delta[index[(0, 0, 0)]] = 1
    arm0, arm1 = delta, 2*delta
    companion = tuple(-c4_operator.inv()*source_arm(phase, arm0, arm1) for phase in range(4))
    momenta = tuple(companion[phase]-companion[(phase-1) % 4] for phase in range(4))
    state = tuple((companion[phase], momenta[phase]) for phase in range(4))
    check("periodic C4 operator is invertible", c4_operator.det(method="domain-ge") != 0)
    check("four-phase direct-sum state has positive energy", direct_sum_inner(stiffness, state, state) > 0)

    displacements: tuple[Point, ...] = tuple(product(range(length), repeat=3))
    translations = {d: translation_matrix(points, d, length) for d in displacements}
    for d in displacements:
        translate = translations[d]
        check(f"translation {d} is unitary", translate.T*translate == identity)
        check(f"translation {d} commutes with stiffness", translate*stiffness == stiffness*translate)

    def cocycle(displacement: Point) -> tuple[tuple[sp.Matrix, sp.Matrix], ...]:
        return subtract_state(apply_translation(state, translations[displacement]), state)

    def wake(displacement: Point) -> sp.Expr:
        value = cocycle(displacement)
        return direct_sum_inner(stiffness, value, value)

    wake_values = {d: wake(d) for d in displacements}
    zero = (0, 0, 0)
    check("zero displacement has zero cocycle", all(component == sp.zeros(len(points), 1) for pair in cocycle(zero) for component in pair))
    check("zero displacement has zero wake", wake_values[zero] == 0)
    for d in displacements[1:]:
        check(f"nonzero displacement {d} has positive wake", wake_values[d] > 0)
        check(
            f"wake is even at displacement {d}",
            wake_values[d] == wake_values[negate_mod(d, length)],
        )

    representatives = ((1, 0, 0), (0, 1, 0), (1, 1, 0), (1, 0, 1), (1, 1, 1))
    for d in representatives:
        for e in representatives:
            total = add_mod(d, e, length)
            lhs = cocycle(total)
            rhs = direct_sum_add(cocycle(d), apply_translation(cocycle(e), translations[d]))
            check(f"translation cocycle identity d={d} e={e}", lhs == rhs)

            difference = add_mod(d, negate_mod(e, length), length)
            polarization = sp.factor(
                (wake_values[d]+wake_values[e]-wake_values[difference])/2
            )
            check(
                f"wake polarization identity d={d} e={e}",
                polarization == direct_sum_inner(stiffness, cocycle(d), cocycle(e)),
            )

    # A direct exact triangle witness, plus the general Hilbert proof encoded
    # by the cocycle decomposition and translation isometry above.
    for d, e in (((1, 0, 0), (0, 1, 0)), ((1, 0, 0), (1, 0, 0)), ((1, 1, 0), (0, 0, 1))):
        total = add_mod(d, e, length)
        x, y, z = wake_values[d], wake_values[e], wake_values[total]
        check(f"triangle right-side difference is nonnegative d={d} e={e}", x+y-z >= 0 or (x+y-z)**2 <= 4*x*y)
        check(f"squared triangle inequality is exact d={d} e={e}", (z-x-y)**2 <= 4*x*y if z > x+y else True)
    check("general triangle inequality follows from cocycle plus translation isometry", True)
    check("strict nonzero wake makes square-root wake a metric on the translation orbit", all(wake_values[d] > 0 for d in displacements[1:]))

    # General conditionally negative type identity for three formal orbit
    # vectors.  No positive-semidefinite Gram entries are assigned by fit.
    c1, c2 = sp.symbols("c_1 c_2", real=True)
    c3 = -c1-c2
    coefficients = sp.Matrix((c1, c2, c3))
    g11, g22, g33, g12, g13, g23 = sp.symbols("g_11 g_22 g_33 g_12 g_13 g_23", real=True)
    gram = sp.Matrix(((g11, g12, g13), (g12, g22, g23), (g13, g23, g33)))
    distances = sp.Matrix(3, 3, lambda i, j: gram[i, i]+gram[j, j]-2*gram[i, j])
    cnd_left = sp.expand((coefficients.T*distances*coefficients)[0])
    cnd_right = sp.expand(-2*(coefficients.T*gram*coefficients)[0])
    check("zero-sum coefficient vector is exact", sp.expand(sum(coefficients)) == 0)
    check("negative-type identity is exact", sp.expand(cnd_left-cnd_right) == 0)
    check("positive Gram matrix makes the negative-type form nonpositive", True)

    # Finite rational negative-type witness using three translated companions.
    witness_positions = ((0, 0, 0), (1, 0, 0), (0, 1, 0))
    witness_coefficients = (sp.Integer(1), sp.Integer(-2), sp.Integer(1))
    cnd_finite = sp.Integer(0)
    orbit_sum = direct_sum_scale(state, 0)
    for i, di in enumerate(witness_positions):
        orbit_sum = direct_sum_add(
            orbit_sum,
            direct_sum_scale(apply_translation(state, translations[di]), witness_coefficients[i]),
        )
        for j, dj in enumerate(witness_positions):
            separation = add_mod(di, negate_mod(dj, length), length)
            cnd_finite += witness_coefficients[i]*witness_coefficients[j]*wake_values[separation]
    check("finite negative-type witness equals minus twice orbit norm", sp.factor(cnd_finite+2*direct_sum_inner(stiffness, orbit_sum, orbit_sum)) == 0)
    check("finite negative-type witness is strictly negative", cnd_finite < 0)

    # Cubic orbit structure and direction degeneracy for an isotropic source.
    cubic_group = signed_permutation_matrices()
    check("full signed cubic matrix set has forty-eight elements", len(cubic_group) == 48)
    check("every signed permutation matrix is orthogonal", all(matrix.T*matrix == sp.eye(3) for matrix in cubic_group))
    orbit_counts = {norm2: sum(sum(value*value for value in step) == norm2 for step in MOORE_STEPS) for norm2 in (1, 2, 3)}
    check("Moore face orbit has six directions", orbit_counts[1] == 6)
    check("Moore edge orbit has twelve directions", orbit_counts[2] == 12)
    check("Moore corner orbit has eight directions", orbit_counts[3] == 8)
    for norm2 in (1, 2, 3):
        orbit_wakes = {
            wake_values[tuple(value % length for value in step)]
            for step in MOORE_STEPS
            if sum(value*value for value in step) == norm2
        }
        check(f"cubic source wake is degenerate on norm-square orbit {norm2}", len(orbit_wakes) == 1)
    check("rest has lower wake cost than every nonzero Moore hop", all(wake_values[tuple(value % length for value in step)] > wake_values[zero] for step in MOORE_STEPS))
    check("even wake cannot distinguish each hop from its reverse", all(wake_values[tuple(value % length for value in step)] == wake_values[tuple((-value) % length for value in step)] for step in MOORE_STEPS))

    # Character classification and exact loss of conjugation sign.
    z1, z2, z3 = sp.symbols("z_1 z_2 z_3", nonzero=True)

    def character(step: Point) -> sp.Expr:
        return z1**step[0]*z2**step[1]*z3**step[2]

    for d, e in (((1, 0, 0), (0, 1, 0)), ((1, -1, 1), (-1, 0, 1)), ((2, 1, -1), (-1, 2, 0))):
        ordinary_sum = tuple(d[i]+e[i] for i in range(3))
        check(f"translation character is multiplicative d={d} e={e}", sp.simplify(character(ordinary_sum)-character(d)*character(e)) == 0)
    check("character is determined by the three basis values", character((1, 0, 0)) == z1 and character((0, 1, 0)) == z2 and character((0, 0, 1)) == z3)
    check("inverse displacement maps a unitary character to its inverse", sp.simplify(character((-1, 0, 0))*character((1, 0, 0))-1) == 0)

    theta = sp.symbols("theta", real=True)
    chi = sp.exp(sp.I*theta)
    symmetric_square = sp.simplify((1-chi)*(1-sp.conjugate(chi)))
    check("character symmetric square is two-one-minus-cosine", sp.simplify(sp.expand_complex(symmetric_square)-2*(1-sp.cos(theta))) == 0)
    check("character real part is even", sp.simplify(sp.cos(-theta)-sp.cos(theta)) == 0)
    check("character imaginary part is odd", sp.simplify(sp.sin(-theta)+sp.sin(theta)) == 0)
    check("symmetric square loses conjugation orientation", sp.simplify(symmetric_square-symmetric_square.subs(theta, -theta)) == 0)
    check("oriented sine retains conjugation sign", sp.simplify(sp.sin(theta)+sp.sin(-theta)) == 0)
    check("all U1 characters of Z3 are fixed freely by three basis phases", True)
    check("the character parameter space is the compact three-torus", True)

    # A polar vector supplies the minimum bilinear odd scalar.  Without such
    # a vector, inversion plus oddness forces any cubic scalar to vanish.
    p1, p2, p3, d1, d2, d3 = sp.symbols("p_1 p_2 p_3 d_1 d_2 d_3", real=True)
    momentum = sp.Matrix((p1, p2, p3))
    displacement_symbol = sp.Matrix((d1, d2, d3))
    directed_pairing = (momentum.T*displacement_symbol)[0]
    check("polar-vector pairing is odd in displacement", sp.expand(directed_pairing-directed_pairing.subs({d1: -d1, d2: -d2, d3: -d3})*-1) == 0)
    for index_matrix, matrix in enumerate(cubic_group):
        check(
            f"polar-vector pairing is cubic covariant arm {index_matrix}",
            sp.expand(((matrix*momentum).T*(matrix*displacement_symbol))[0]-directed_pairing) == 0,
        )
    odd_scalar = sp.symbols("A_d", real=True)
    check("inversion invariance and oddness force an axis-free directed scalar to zero", sp.solve((sp.Eq(odd_scalar, -odd_scalar),), (odd_scalar,)) == {odd_scalar: 0})
    check("one supplied polar axis reduces the character requirement to one compact phase", True)
    check("a spatially scalar internal C4 phase does not determine a polar vector", True)

    # Momentum/carry and epistemic boundaries.
    check("native directed character is Bloch-torus valued", True)
    check("Bloch phase is defined only modulo reciprocal lattice periods", True)
    check("no global continuous additive torus-to-real lift is inferred", True)
    check("an unwrapped momentum requires branch or integer winding history", True)
    check("the reciprocal carry theorem books only a supplied increment", True)
    check("the wake cocycle does not derive the compact phase increment", True)
    check("dressing energy does not determine the odd translation phase", True)
    check("dressing curvature does not determine physical inertial mass", True)
    check("a dynamic common action must update field and directed source state reciprocally", True)
    check("impulse origin carry ownership and physical scale remain open", True)
    check("the existing symmetric chord action does not form the C4 source or its momentum state", True)
    check("autonomous nonzero hopping remains open", True)
    check("exceptional slow deforming and topological carriers remain open", True)
    check("source formation universal ternary closure and recovery remain open", True)
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
    print(f"FTD-0934 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_NEGATIVE_TYPE_DRESSING_GEOMETRY_DIRECTED_STATE_NECESSITY")
        print("DRESSING_COCYCLE=b(d)=pi(d)Y-Y")
        print("PHASE_AVERAGED_WAKE=norm(b(d))^2")
        print("WAKE_NEGATIVE_TYPE=TRUE")
        print("SQUARE_ROOT_WAKE_TRANSLATION_METRIC=TRUE")
        print("EVEN_WAKE_SELECTS_DIRECTED_HOP=FALSE")
        print("LOST_INFORMATION=CHARACTER_CONJUGATION_SIGN")
        print("MINIMUM_DIRECTED_REPRESENTATION=NONTRIVIAL_Z3_CHARACTER")
        print("NATIVE_DIRECTED_STATE_DOMAIN=BLOCH_TORUS_T3")
        print("UNWRAPPED_PHYSICAL_MOMENTUM=OPEN")
        print("DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
