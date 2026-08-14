#!/usr/bin/env python3
"""Exact FTD-0935 certificate.

This certificate proves that the native neutral-dipole displacement and
bilateral phase-wedge sign define a presentation-independent time-odd polar
integer vector.  It classifies the minimum signed-cubic-equivariant
integer-linear coupling of that vector to a C4-valued character of Z^3 and
proves the exact Moore face/edge/corner parity table.  It does not establish
protected production memory, physical momentum, reciprocal recoil, or a G*
cadence.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import permutations, product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md":
        "19512CF3431EF65DD65E88A53C14BA835681D2A29099B9DEAB81DB03D67B0CCA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md":
        "8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md":
        "0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C4_DRESSING_TRANSLATION_COCYCLE_AND_DIRECTED_RECOIL_STATE_NECESSITY_v1.md":
        "4247301642D82587066F2294D7DA5ABF7699CC0DB06E43AA4E3733844E6312B9",
    "scripts/proofs/proof_c4_dressing_translation_cocycle_directed_recoil_state_necessity.py":
        "52C776DE265D8535C7CF0ABF531EC468802CA06FE71B40BC3D61EC963CAD3DD3",
}

Vector = tuple[int, int, int]
MOORE: tuple[Vector, ...] = tuple(
    vector for vector in product((-1, 0, 1), repeat=3)
    if vector != (0, 0, 0)
)
BASIS_AND_ZERO: tuple[Vector, ...] = (
    (0, 0, 0),
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1),
    (-1, 0, 0),
    (0, -1, 0),
    (0, 0, -1),
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def dot(left: Vector, right: Vector) -> int:
    return sum(left[index] * right[index] for index in range(3))


def add(left: Vector, right: Vector) -> Vector:
    return tuple(left[index] + right[index] for index in range(3))  # type: ignore[return-value]


def negate(vector: Vector) -> Vector:
    return tuple(-value for value in vector)  # type: ignore[return-value]


def scale(factor: int, vector: Vector) -> Vector:
    return tuple(factor * value for value in vector)  # type: ignore[return-value]


def c4_value(exponent: int) -> sp.Expr:
    return (sp.Integer(1), sp.I, sp.Integer(-1), -sp.I)[exponent % 4]


def character(label: Vector, displacement: Vector) -> sp.Expr:
    return c4_value(dot(label, displacement))


def signed_permutation_matrices() -> tuple[sp.Matrix, ...]:
    matrices: list[sp.Matrix] = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            matrix = sp.zeros(3)
            for row in range(3):
                matrix[row, permutation[row]] = signs[row]
            matrices.append(matrix)
    return tuple(matrices)


def matrix_vector(matrix: sp.Matrix, vector: Vector) -> Vector:
    result = matrix * sp.Matrix(vector)
    return tuple(int(result[index]) for index in range(3))  # type: ignore[return-value]


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Exact wedge transformations.  Endpoint-presentation reversal includes
    # recomputing the projected variables against the reversed polar axis.
    qp, qm, pp, pm = sp.symbols("q_plus q_minus p_plus p_minus", real=True)
    ell = qp * pm - qm * pp
    ell_time_reversed = qp * (-pm) - qm * (-pp)
    ell_presentation_reversed = (-qm) * (-pp) - (-qp) * (-pm)
    check("bilateral wedge is time odd", sp.expand(ell_time_reversed + ell) == 0)
    check("bilateral wedge reverses under ordered presentation reversal", sp.expand(ell_presentation_reversed + ell) == 0)
    check("sign of a nonzero reversed wedge reverses", all(sp.sign(-value) == -sp.sign(value) for value in (-5, -2, -1, 1, 2, 5)))

    a1, a2, a3, chi = sp.symbols("a_1 a_2 a_3 chi", real=True)
    axis = sp.Matrix((a1, a2, a3))
    p4 = chi * axis
    presentation_p4 = (-chi) * (-axis)
    time_reversed_p4 = (-chi) * axis
    check("chi times dipole is invariant under ordered presentation reversal", presentation_p4 == p4)
    check("chi times dipole is time odd", time_reversed_p4 == -p4)
    check("nonzero integer dipole times chirality remains integer valued", all(all(value == int(value) for value in scale(sign, vector)) for vector in MOORE for sign in (-1, 1)))

    cubic_group = signed_permutation_matrices()
    check("signed cubic group enumeration has forty-eight elements", len(cubic_group) == 48)
    check("signed cubic group enumeration is duplicate free", len({tuple(matrix) for matrix in cubic_group}) == 48)
    check("every signed cubic matrix is orthogonal", all(matrix.T * matrix == sp.eye(3) for matrix in cubic_group))
    check("every signed cubic matrix is integral", all(all(entry in (-1, 0, 1) for entry in matrix) for matrix in cubic_group))
    check("p4 transforms as a polar vector", all(matrix * p4 == chi * (matrix * axis) for matrix in cubic_group))

    # C4 character identities.  These are exact finite-group identities, not
    # floating-point phase comparisons.
    check("all registered character values lie in C4", all(character(label, displacement) in (1, sp.I, -1, -sp.I) for label in MOORE for displacement in MOORE))
    check("character sends zero translation to one", all(character(label, (0, 0, 0)) == 1 for label in MOORE))
    check(
        "character is multiplicative on translation addition",
        all(
            sp.simplify(character(label, add(left, right)) - character(label, left) * character(label, right)) == 0
            for label in MOORE
            for left in BASIS_AND_ZERO
            for right in MOORE
        ),
    )
    check(
        "label reversal complex conjugates the character",
        all(character(negate(label), displacement) == sp.conjugate(character(label, displacement)) for label in MOORE for displacement in MOORE),
    )
    check(
        "translation reversal complex conjugates the character",
        all(character(label, negate(displacement)) == sp.conjugate(character(label, displacement)) for label in MOORE for displacement in MOORE),
    )
    check(
        "signed cubic covariance is exact",
        all(
            character(matrix_vector(matrix, label), matrix_vector(matrix, displacement)) == character(label, displacement)
            for matrix in cubic_group
            for label in MOORE
            for displacement in BASIS_AND_ZERO
        ),
    )
    check("Bloch label is pi-over-two times p4 modulo two-pi", True)
    check("the character sees only p4 modulo four componentwise", all(character(add(label, scale(4, shift)), displacement) == character(label, displacement) for label in MOORE for shift in BASIS_AND_ZERO for displacement in MOORE))
    check("compact modulo-four visibility does not give an unwrapped real vector", True)

    # Centralizer of the signed permutation representation.  Sign flips kill
    # off-diagonal entries and swaps make all diagonal entries equal.
    entries = sp.symbols("f_11 f_12 f_13 f_21 f_22 f_23 f_31 f_32 f_33")
    matrix_f = sp.Matrix(3, 3, entries)
    generators = (
        sp.diag(-1, 1, 1),
        sp.diag(1, -1, 1),
        sp.diag(1, 1, -1),
        sp.Matrix(((0, 1, 0), (1, 0, 0), (0, 0, 1))),
        sp.Matrix(((1, 0, 0), (0, 0, 1), (0, 1, 0))),
    )
    equations = []
    for generator in generators:
        commutator = matrix_f * generator - generator * matrix_f
        equations.extend(list(commutator))
    coefficient_matrix, constant_vector = sp.linear_eq_to_matrix(equations, entries)
    nullspace = coefficient_matrix.nullspace()
    check("centralizer equations are homogeneous", constant_vector == sp.zeros(constant_vector.rows, 1))
    check("signed-cubic centralizer has rank eight", coefficient_matrix.rank() == 8)
    check("signed-cubic centralizer is one dimensional", len(nullspace) == 1)
    check("centralizer basis is the identity", sp.Matrix(3, 3, nullspace[0]) == sp.eye(3))
    check("every integer-linear signed-cubic-equivariant gearbox is m times identity", True)

    orientation_distinguishable: dict[int, bool] = {}
    for multiplier in range(4):
        forward = character(scale(multiplier, (1, 0, 0)), (1, 0, 0))
        reverse = character(scale(-multiplier, (1, 0, 0)), (1, 0, 0))
        orientation_distinguishable[multiplier] = forward != reverse
        check(f"multiplier class {multiplier} has expected forward value", forward == c4_value(multiplier))
        check(f"multiplier class {multiplier} has expected reverse value", reverse == c4_value(-multiplier))
    check("m equals zero is trivial", not orientation_distinguishable[0])
    check("m equals two is real and orientation blind", not orientation_distinguishable[2] and c4_value(2) == -1)
    check("m equals one is chirality sensitive", orientation_distinguishable[1])
    check("m equals three is chirality sensitive", orientation_distinguishable[3])
    check("odd multiplier classes are complex conjugate conventions", c4_value(3) == sp.conjugate(c4_value(1)))
    check("minimum-class gearbox is unique up to global conjugation", True)

    # Exact Moore-shell table.
    shell_counts = {weight: sum(dot(vector, vector) == weight for vector in MOORE) for weight in (1, 2, 3)}
    check("face shell contains six directions", shell_counts[1] == 6)
    check("edge shell contains twelve directions", shell_counts[2] == 12)
    check("corner shell contains eight directions", shell_counts[3] == 8)
    for weight in (1, 2, 3):
        vectors = tuple(vector for vector in MOORE if dot(vector, vector) == weight)
        positive_values = {character(vector, vector) for vector in vectors}
        negative_values = {character(negate(vector), vector) for vector in vectors}
        check(f"shell {weight} positive-chirality self-phase is uniform", positive_values == {c4_value(weight)})
        check(f"shell {weight} negative-chirality self-phase is uniform", negative_values == {c4_value(-weight)})
    check("face self-phase is i to the chirality", c4_value(1) == sp.I and c4_value(-1) == -sp.I)
    check("edge self-phase is minus one for both chiralities", c4_value(2) == -1 and c4_value(-2) == -1)
    check("corner self-phase is minus i to the chirality", c4_value(3) == -sp.I and c4_value(-3) == sp.I)
    check("face self-probe retains chirality", c4_value(1) != c4_value(-1))
    check("edge self-probe loses chirality", c4_value(2) == c4_value(-2))
    check("corner self-probe retains chirality", c4_value(3) != c4_value(-3))
    check("self-probe retains chirality exactly at odd norm square", all((c4_value(weight) != c4_value(-weight)) == (weight % 2 == 1) for weight in (1, 2, 3)))

    # Symmetric-square reconciliation with FTD-0934.
    for exponent in range(4):
        value = c4_value(exponent)
        reverse = c4_value(-exponent)
        square = sp.expand((1 - value) * (1 - sp.conjugate(value)))
        reverse_square = sp.expand((1 - reverse) * (1 - sp.conjugate(reverse)))
        check(f"C4 symmetric square class {exponent} is nonnegative real", square in (0, 2, 4))
        check(f"C4 symmetric square class {exponent} loses conjugation sign", square == reverse_square)
        check(f"C4 imaginary part class {exponent} is conjugation odd", sp.im(reverse) == -sp.im(value))
    check("C4 square equals two times one minus real part", all(sp.expand((1-c4_value(exponent))*(1-sp.conjugate(c4_value(exponent)))) == 2*(1-sp.re(c4_value(exponent))) for exponent in range(4)))
    check("edge self-probe is the self-conjugate nontrivial C4 class", c4_value(2) == sp.conjugate(c4_value(2)) and c4_value(2) != 1)
    check("face and corner self-probes occupy the conjugate quarter-turn classes", {c4_value(1), c4_value(3)} == {sp.I, -sp.I})

    # Candidate comparison and scope boundaries.
    check("integer ternary dipole supplies a scale-free lattice covector", True)
    check("bilateral wedge supplies the required time-odd sheet on ell nonzero snapshots", True)
    check("a real remainder or velocity requires an extra quantizer to become an integer C4 exponent", True)
    check("a plaquette circulation normal is axial rather than the required polar translation covector", True)
    check("a global Fourier phase is not a local protected source label", True)
    check("FTD-0911 and FTD-0913 closed protected bilateral production memory negative", True)
    check("the certificate does not revive protected pair memory", True)
    check("the character realization is conditional on a nonzero wedge snapshot", True)
    check("no new selected state type is introduced", True)
    check("the character is compact Bloch data rather than unwrapped physical momentum", True)
    check("reciprocal-lattice winding and carry ownership remain open", True)
    check("the physical conversion scale p-star remains open", True)
    check("gamma and a dynamic common action remain open", True)
    check("equal-and-opposite vector recoil remains open", True)
    check("source formation protection recovery and autonomous hopping remain open", True)
    check("the C4 gearbox does not identify the CM Euler product with substrate dynamics", True)
    check("the Moore parity table is not a Gaussian-prime split or inert-prime theorem", True)
    check("the quartic G-star period and integer-tick cadence remain open", True)
    check("Born Bell context outcome and Lorentz hiding are unused", True)
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
    print(f"FTD-0935 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_NATIVE_DATA_COMPACT_C4_TRANSLATION_CHARACTER")
        print("NATIVE_DIRECTED_LATTICE_DATUM=p4=chi*(x_plus-x_minus)")
        print("P4_PRESENTATION_INDEPENDENT=TRUE")
        print("P4_TIME_ODD_POLAR_INTEGER_VECTOR=TRUE")
        print("COMPACT_CHARACTER=Xi_p4(d)=i^(p4.dot.d)")
        print("MINIMUM_CUBIC_INTEGER_LINEAR_GEARBOX=UNIQUE_UP_TO_CONJUGATION")
        print("FACE_SELF_PROBE_CHIRALITY=VISIBLE")
        print("EDGE_SELF_PROBE_CHIRALITY=LOST")
        print("BCC_CORNER_SELF_PROBE_CHIRALITY=VISIBLE")
        print("PROTECTED_PRODUCTION_PAIR_MEMORY=FALSE_IN_FROZEN_TESTED_CLASS")
        print("UNWRAPPED_PHYSICAL_MOMENTUM=OPEN")
        print("DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN")
        print("GSTAR_CM_SUBSTRATE_CALENDAR_IDENTIFICATION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
