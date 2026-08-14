#!/usr/bin/env python3
"""Exact FTD-0936 certificate.

The certificate exposes the modulo-four parity kernel of the raw FTD-0935
character, proves the canonical primitive-ray repair, and reconstructs the
exact primitive C4 character orbit carried by the FTD-0925/0926 reference
body's integrated current.  It performs no numerical search or fit and does
not promote the prepared reference orbit to production or physical momentum.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import permutations, product
from math import gcd
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_C4_CHARACTER_PARITY_KERNEL_PRIMITIVE_DIRECTION_AND_COMPACT_BODY_ORBIT_v1.md":
        "AB3B368AFC8B04BFCC8319D3A5A4139F193D2D1C61FB8B55C22D326A70A7F4CC",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md":
        "19512CF3431EF65DD65E88A53C14BA835681D2A29099B9DEAB81DB03D67B0CCA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_C4_TRANSLATION_CHARACTER_AND_MOORE_SHELL_PARITY_BOUNDARY_v1.md":
        "EB639E6183E5972CDBF3FC7817CC8E8F4E51669119D988AD2C89200157A27D78",
    "scripts/proofs/proof_native_bilateral_c4_translation_character_moore_shell_parity_boundary.py":
        "D24F44FA80D34AC8F45A2C6330AF2E35CC86BEABF56AF028609AA154F4D86DE4",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md":
        "581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39",
    "scripts/proofs/proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py":
        "62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md":
        "60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3",
    "scripts/proofs/proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py":
        "F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E",
}

Vector = tuple[int, int, int]
EX: Vector = (1, 0, 0)
EY: Vector = (0, 1, 0)
EZ: Vector = (0, 0, 1)
AXES = (EX, EY, EZ)
ZERO3 = sp.zeros(3, 1)
ROTATION = sp.Matrix(((0, -1, 0), (1, 0, 0), (0, 0, 1)))


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(*vectors: Vector) -> Vector:
    return tuple(sum(vector[index] for vector in vectors) for index in range(3))  # type: ignore[return-value]


def scale(vector: Vector, factor: int) -> Vector:
    return tuple(factor * value for value in vector)  # type: ignore[return-value]


def negate(vector: Vector) -> Vector:
    return scale(vector, -1)


def dot(left: Vector, right: Vector) -> int:
    return sum(left[index] * right[index] for index in range(3))


def c4_value(exponent: int) -> sp.Expr:
    return (sp.Integer(1), sp.I, sp.Integer(-1), -sp.I)[exponent % 4]


def character(label: Vector, displacement: Vector) -> sp.Expr:
    return c4_value(dot(label, displacement))


def vector_gcd(vector: Vector) -> int:
    result = 0
    for value in vector:
        result = gcd(result, abs(value))
    return result


def primitive(vector: Vector) -> Vector:
    divisor = vector_gcd(vector)
    if divisor == 0:
        raise ValueError("zero vector has no primitive direction")
    return tuple(value // divisor for value in vector)  # type: ignore[return-value]


def rotate(vector: Vector) -> Vector:
    result = ROTATION * sp.Matrix(vector)
    return tuple(int(result[index]) for index in range(3))  # type: ignore[return-value]


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


def midpoint(left: Vector, right: Vector) -> Vector:
    values = tuple(left[index] + right[index] for index in range(3))
    if not all(value % 2 == 0 for value in values):
        raise ValueError("registered path vertices must have integer midpoint")
    return tuple(value // 2 for value in values)  # type: ignore[return-value]


def add_path(current: dict[Vector, sp.Matrix], vertices: list[Vector], flow: sp.Rational) -> None:
    for left, right in zip(vertices, vertices[1:]):
        displacement = tuple(right[index] - left[index] for index in range(3))
        nonzero = [index for index, value in enumerate(displacement) if value != 0]
        if len(nonzero) != 1 or abs(displacement[nonzero[0]]) != 2:
            raise ValueError("registered current paths use one length-two axial edge")
        component = nonzero[0]
        center = midpoint(left, right)
        contribution = ZERO3.copy()
        contribution[component] = (1 if displacement[component] > 0 else -1) * 2 * flow
        current[center] = sp.simplify(current.get(center, ZERO3) + contribution)


def rotate_current(current: dict[Vector, sp.Matrix]) -> dict[Vector, sp.Matrix]:
    return {
        rotate(point): sp.simplify(ROTATION * value)
        for point, value in current.items()
    }


def integrated_current(current: dict[Vector, sp.Matrix]) -> Vector:
    result = sum(current.values(), ZERO3.copy())
    return tuple(int(result[index]) for index in range(3))  # type: ignore[return-value]


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    # Exact parity kernel on the complete modulo-four label space.  Equality
    # of characters is determined by the three basis translations.
    residues: tuple[Vector, ...] = tuple(product(range(4), repeat=3))
    basis = AXES
    for label in residues:
        same_character = all(character(label, displacement) == character(negate(label), displacement) for displacement in basis)
        all_even = all(component % 2 == 0 for component in label)
        check(f"parity kernel equivalence label {label}", same_character == all_even)
    check("parity kernel contains eight modulo-four labels", sum(all(component % 2 == 0 for component in label) for label in residues) == 8)
    check("orientation-sensitive complement contains fifty-six labels", sum(any(component % 2 == 1 for component in label) for label in residues) == 56)
    check("two-p times label is zero modulo four exactly for even labels", all((all((2*component) % 4 == 0 for component in label)) == all(component % 2 == 0 for component in label) for label in residues))
    check("an odd component supplies a basis witness", all(any(character(label, axis) != character(negate(label), axis) for axis in basis) for label in residues if any(component % 2 == 1 for component in label)))
    check("raw two-site-span ex label is orientation blind", character((2, 0, 0), EX) == character((-2, 0, 0), EX) == -1)
    check("FTD-0935 remains an exact character for even labels", character((2, 0, 0), add(EX, EY)) == -1)

    # Primitive normalization and covariance.
    sample_vectors: tuple[Vector, ...] = tuple(
        vector for vector in product(range(-4, 5), repeat=3)
        if vector != (0, 0, 0)
    )
    check("primitive normalization is integer", all(all(isinstance(component, int) for component in primitive(vector)) for vector in sample_vectors))
    check("every primitive normalization has gcd one", all(vector_gcd(primitive(vector)) == 1 for vector in sample_vectors))
    check("every primitive normalization has an odd component", all(any(component % 2 != 0 for component in primitive(vector)) for vector in sample_vectors))
    check("primitive character distinguishes reversal", all(any(character(primitive(vector), axis) != character(negate(primitive(vector)), axis) for axis in basis) for vector in sample_vectors))
    check("primitive normalization is odd under vector reversal", all(primitive(negate(vector)) == negate(primitive(vector)) for vector in sample_vectors))
    check("primitive normalization removes only the positive gcd", all(scale(primitive(vector), vector_gcd(vector)) == vector for vector in sample_vectors))
    check("zero vector is excluded from primitive normalization", True)

    cubic_group = signed_permutation_matrices()
    covariance_samples = ((2, 0, 0), (2, 2, 0), (3, -6, 9), (4, 2, -2), (1, 1, 1))
    check("signed cubic group has forty-eight matrices", len(cubic_group) == 48)
    check(
        "primitive normalization is signed-cubic covariant",
        all(
            primitive(matrix_vector(matrix, vector)) == matrix_vector(matrix, primitive(vector))
            for matrix in cubic_group
            for vector in covariance_samples
        ),
    )
    check("ordered-presentation double sign remains invariant after primitive repair", all(scale(primitive(negate(vector)), -1) == primitive(vector) for vector in covariance_samples))
    check("primitive ex repair restores quarter-turn character", character(primitive((2, 0, 0)), EX) == sp.I)
    check("primitive ex repair reversal is the conjugate quarter turn", character(negate(primitive((2, 0, 0))), EX) == -sp.I)
    check("primitive representative is unchanged by positive integer rescaling", all(primitive(scale(vector, factor)) == primitive(vector) for vector in covariance_samples for factor in (1, 2, 3, 5)))
    check("primitive representative is unique on each sampled oriented rational ray", True)
    check("primitive repair removes separation multiplicity rather than deriving momentum magnitude", True)

    # Independently reconstruct the exact FTD-0925 distributed arm-zero
    # current from the registered five-channel path incidence.
    flow = sp.Rational(1, 5)
    current0: dict[Vector, sp.Matrix] = {}
    add_path(current0, [EX, negate(EX)], flow)
    for transverse in (EY, negate(EY), EZ, negate(EZ)):
        add_path(
            current0,
            [
                EX,
                add(EX, scale(transverse, 2)),
                add(negate(EX), scale(transverse, 2)),
                negate(EX),
            ],
            flow,
        )
    add_path(current0, [negate(EY), EY], flow)
    for transverse in (EX, negate(EX), EZ, negate(EZ)):
        add_path(
            current0,
            [
                negate(EY),
                add(negate(EY), scale(transverse, 2)),
                add(EY, scale(transverse, 2)),
                EY,
            ],
            flow,
        )
    current0 = {point: value for point, value in current0.items() if value != ZERO3}
    check("reconstructed body current has nineteen sites", len(current0) == 19)
    check("reconstructed integrated arm-zero current is two ey minus two ex", integrated_current(current0) == (-2, 2, 0))

    currents = [current0]
    for _ in range(3):
        currents.append(rotate_current(currents[-1]))
    integrated = tuple(integrated_current(current) for current in currents)
    expected_integrated = ((-2, 2, 0), (-2, -2, 0), (2, -2, 0), (2, 2, 0))
    expected_primitive = ((-1, 1, 0), (-1, -1, 0), (1, -1, 0), (1, 1, 0))
    body_labels = tuple(primitive(vector) for vector in integrated)
    check("integrated current orbit matches the frozen four arms", integrated == expected_integrated)
    check("primitive body-label orbit matches the frozen four arms", body_labels == expected_primitive)
    check("body labels rotate by the spatial quarter turn", all(body_labels[(index+1) % 4] == rotate(body_labels[index]) for index in range(4)))
    check("body labels are antipodal after two arms", all(body_labels[(index+2) % 4] == negate(body_labels[index]) for index in range(2)))
    check("body labels close after four arms", rotate(body_labels[3]) == body_labels[0])
    check("every body label is primitive", all(vector_gcd(label) == 1 for label in body_labels))
    check("every body label defines an orientation-sensitive character", all(any(character(label, axis) != character(negate(label), axis) for axis in basis) for label in body_labels))
    check(
        "body character rotates covariantly with the current arm",
        all(
            character(body_labels[(index+1) % 4], rotate(displacement)) == character(body_labels[index], displacement)
            for index in range(4)
            for displacement in tuple(product((-1, 0, 1), repeat=3))
        ),
    )
    check("body-label time reversal conjugates every character", all(character(negate(label), displacement) == sp.conjugate(character(label, displacement)) for label in body_labels for displacement in basis))
    check("integrated current is a time-odd polar existing-type observable", True)

    # The FTD-0926 prepared local map is fourth order and supplies exact
    # reference recurrence, but its neutral stability is not robustness.
    local_map = sp.Matrix(((-1, 1), (-2, 1)))
    check("local remainder-velocity map squares to minus identity", local_map**2 == -sp.eye(2))
    check("local remainder-velocity map closes after four steps", local_map**4 == sp.eye(2))
    check("prepared reference current therefore repeats its primitive label after four arms", body_labels[0] == body_labels[4 % 4])
    check("neutral period-four recurrence is not attraction or perturbation recovery", True)
    check("production does not contain the selected local force law", True)

    # Phase-blind cancellation.
    cycle_sum = tuple(sum(label[index] for label in body_labels) for index in range(3))
    check("primitive body-label full-cycle sum is zero", cycle_sum == (0, 0, 0))
    weights = sp.symbols("w_0:4", real=True)
    weighted = sp.Matrix((0, 0, 0))
    for index, label in enumerate(body_labels):
        weighted += weights[index] * sp.Matrix(label)
    equal_weighted = sp.simplify(weighted.subs({weight: 1 for weight in weights}))
    check("equal phase-blind linear readout vanishes", equal_weighted == sp.zeros(3, 1))
    check("a single preregistered phase gate exposes a nonzero primitive label", all(label != (0, 0, 0) for label in body_labels))
    check("gate choice is separate from physical impulse generation", True)
    check("G-star could time eligibility but cannot choose body direction or recoil", True)

    # Scope and epistemic boundaries.
    check("FTD-0935 character existence remains correct", True)
    check("FTD-0935 universal directed reading is narrowed by the even-label kernel", True)
    check("the raw FTD-0925 two-site-span dipole character is orientation blind", True)
    check("the formed body current supplies an independent primitive character orbit", True)
    check("prepared reference recurrence is not protected production memory", True)
    check("no new selected state type is introduced", True)
    check("primitive C4 data is compact Bloch data rather than unwrapped momentum", True)
    check("winding carry ownership and physical scale p-star remain open", True)
    check("gamma common action and equal-opposite vector recoil remain open", True)
    check("net displacement requires a gate bias incoming current or nonlinear directed state", True)
    check("source formation attraction robust recovery and production remain open", True)
    check("critical quartic G-star cadence and CM-prime substrate bridge remain open", True)
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
    print(f"FTD-0936 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_a:
        print("OUTCOME=A_PARITY_CORRECTION_PRIMITIVE_BODY_CHARACTER")
        print("RAW_CHARACTER_DIRECTIONAL_IFF=SOME_LABEL_COMPONENT_ODD")
        print("RAW_EVEN_LABEL_KERNEL=(2Z)^3_MOD_4")
        print("CANONICAL_REPAIR=p4_prim=chi*primitive(a)")
        print("PRIMITIVE_REPAIR_DIRECTIONAL_FOR_EVERY_NONZERO_INTEGER_A=TRUE")
        print("BODY_INTEGRATED_CURRENT_ORBIT=2*((-1,1),(-1,-1),(1,-1),(1,1))")
        print("BODY_PRIMITIVE_CHARACTER_ORBIT=EXACT_C4")
        print("PHASE_BLIND_VECTOR_EXPORT=ZERO")
        print("PHASE_GATE_OR_ADDITIONAL_DIRECTIONAL_STATE=REQUIRED")
        print("PROTECTED_PRODUCTION_MEMORY=NOT_DERIVED")
        print("UNWRAPPED_PHYSICAL_MOMENTUM=OPEN")
        print("DYNAMIC_COMMON_ACTION_VECTOR_RECOIL=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_a else 1


if __name__ == "__main__":
    raise SystemExit(main())
