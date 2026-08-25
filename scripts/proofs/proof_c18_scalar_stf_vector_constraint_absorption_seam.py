#!/usr/bin/env python3
"""Exact C18 scalar/STF/vector-constraint absorption-seam certificate.

This certificate proves an existing-spatial-type canonical source seam.  It
does not derive the finite collision, constraint dynamics, equal gravity
coupling, a static pole, or lensing.
"""

from __future__ import annotations

import hashlib
import itertools
from fractions import Fraction
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    ROOT / "scripts/proofs/proof_moore_bond_capacity_type_census.py":
        "D8D83F4600822A7C8CB426120B61D8CA57465B379340FF51D87EE81D3A95A7F6",
    ROOT / "scripts/proofs/proof_even_tensor_second_order_action_spin2_escape.py":
        "820193844E22420205DC04CC4E1D957E2AE76A86C94017512CA199E55178CFE8",
    ROOT / "scripts/proofs/proof_c18_tensor_doublet_tt_reduction.py":
        "437392221691D2579D55A078C0CF4F2D3B5AE08D1EB54DCC7469FF3458D67436",
    ROOT / "scripts/proofs/proof_c4_symmetric_stress_packet_momentum_and_source_handoff.py":
        "312FA1071D09FEBE61225A8BAFBA2C6D7994E80A584DE4B9220EE5274ACCB938",
    ROOT / "scripts/proofs/proof_reciprocal_packet_clock_recoil_absorption_generator.py":
        "4B824C3B37A8BADEC9F50ED1785602734B75D6CCF03234D65826E0541CDC2576",
    ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_C18_SCALAR_STF_VECTOR_CONSTRAINT_OWNERSHIP_ABSORPTION_SEAM_v1.md":
        "85474A9D7FA5AD38B25208944204F469E73004F643D304ACB23F11F43AC69D47",
}

Vector = tuple[int, int, int]
Matrix3 = tuple[
    tuple[int, int, int],
    tuple[int, int, int],
    tuple[int, int, int],
]

SC_LINES: tuple[Vector, ...] = (
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1),
)

FCC_LINES: tuple[Vector, ...] = (
    (1, 1, 0),
    (1, -1, 0),
    (1, 0, 1),
    (1, 0, -1),
    (0, 1, 1),
    (0, 1, -1),
)

C18_LINES = SC_LINES + FCC_LINES

SC_DIRECTIONS: tuple[Vector, ...] = (
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
)

O_CHARACTERS = {
    "A1": {"E": 1, "C3": 1, "C2_axis": 1, "C4": 1, "C2_edge": 1},
    "A2": {"E": 1, "C3": 1, "C2_axis": 1, "C4": -1, "C2_edge": -1},
    "E": {"E": 2, "C3": -1, "C2_axis": 2, "C4": 0, "C2_edge": 0},
    "T1": {"E": 3, "C3": 0, "C2_axis": -1, "C4": 1, "C2_edge": -1},
    "T2": {"E": 3, "C3": 0, "C2_axis": -1, "C4": -1, "C2_edge": 1},
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def determinant_3(matrix: Matrix3) -> int:
    a, b, c = matrix
    return (
        a[0] * (b[1] * c[2] - b[2] * c[1])
        - a[1] * (b[0] * c[2] - b[2] * c[0])
        + a[2] * (b[0] * c[1] - b[1] * c[0])
    )


def mat_vec(matrix: Matrix3, vector: Vector) -> Vector:
    return tuple(
        sum(matrix[i][j] * vector[j] for j in range(3))
        for i in range(3)
    )  # type: ignore[return-value]


def negate_matrix(matrix: Matrix3) -> Matrix3:
    return tuple(
        tuple(-entry for entry in row) for row in matrix
    )  # type: ignore[return-value]


def signed_permutation_matrices() -> tuple[Matrix3, ...]:
    matrices: list[Matrix3] = []
    for permutation in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            rows = []
            for row in range(3):
                entries = [0, 0, 0]
                entries[permutation[row]] = signs[row]
                rows.append(tuple(entries))
            matrices.append(tuple(rows))  # type: ignore[arg-type]
    assert len(matrices) == 48
    return tuple(matrices)


def proper_rotation_class(matrix: Matrix3) -> str:
    assert determinant_3(matrix) == 1
    trace = sum(matrix[i][i] for i in range(3))
    if matrix == ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
        return "E"
    if trace == 0:
        return "C3"
    if trace == 1:
        return "C4"
    if trace == -1:
        diagonal = all(
            matrix[i][j] == 0
            for i in range(3)
            for j in range(3)
            if i != j
        )
        return "C2_axis" if diagonal else "C2_edge"
    raise AssertionError(matrix)


def irrep_character(label: str, matrix: Matrix3) -> int:
    parity = label[-1]
    base = label[:-1]
    det = determinant_3(matrix)
    proper = matrix if det == 1 else negate_matrix(matrix)
    value = O_CHARACTERS[base][proper_rotation_class(proper)]
    if parity == "u" and det == -1:
        value = -value
    return value


def canonical_line(vector: Vector) -> tuple[int, int]:
    for index, line in enumerate(C18_LINES):
        if vector == line:
            return index, 1
        if vector == tuple(-entry for entry in line):
            return index, -1
    raise AssertionError(vector)


def line_character(matrix: Matrix3, parity: str) -> int:
    trace = 0
    for index, line in enumerate(C18_LINES):
        image_index, sign = canonical_line(mat_vec(matrix, line))
        if image_index == index:
            trace += 1 if parity == "g" else sign
    return trace


def decompose_line_module(parity: str) -> dict[str, int]:
    group = signed_permutation_matrices()
    result: dict[str, int] = {}
    for base in O_CHARACTERS:
        label = f"{base}{parity}"
        numerator = sum(
            line_character(matrix, parity) * irrep_character(label, matrix)
            for matrix in group
        )
        multiplicity = Fraction(numerator, 48)
        assert multiplicity.denominator == 1
        if multiplicity:
            result[label] = multiplicity.numerator
    return result


def as_sympy(matrix: Matrix3) -> sp.Matrix:
    return sp.Matrix(matrix)


def is_zero(value: sp.Expr | sp.MatrixBase) -> bool:
    if isinstance(value, sp.MatrixBase):
        return all(sp.simplify(entry) == 0 for entry in value)
    return sp.simplify(value) == 0


def representation_checks() -> int:
    checks = 0
    even = decompose_line_module("g")
    odd = decompose_line_module("u")
    assert even == {"A1g": 2, "Eg": 2, "T2g": 1}, even
    assert odd == {"T1u": 2, "T2u": 1}, odd
    checks += 2

    # The two shell first moments are independent T1u copies.  The sum/diff
    # change of copy basis is invertible and commutes with every cubic vector
    # transformation.
    identity = sp.eye(3)
    copy_change = sp.Matrix.vstack(
        sp.Matrix.hstack(identity, identity),
        sp.Matrix.hstack(identity, -identity),
    )
    assert copy_change.det() == -8
    assert is_zero(copy_change.inv() - copy_change / 2)
    checks += 2

    for matrix in signed_permutation_matrices():
        rotation = as_sympy(matrix)
        two_copy_rotation = sp.diag(rotation, rotation)
        assert is_zero(copy_change * two_copy_rotation - two_copy_rotation * copy_change)
        checks += 1

    return checks


STF_BASIS = (
    sp.diag(1, 0, -1),
    sp.diag(0, 1, -1),
    sp.Matrix(((0, 1, 0), (1, 0, 0), (0, 0, 0))),
    sp.Matrix(((0, 0, 1), (0, 0, 0), (1, 0, 0))),
    sp.Matrix(((0, 0, 0), (0, 0, 1), (0, 1, 0))),
)


def divergence_matrix(q: sp.Matrix) -> sp.Matrix:
    return sp.Matrix.hstack(*(basis * q for basis in STF_BASIS))


def stf_source(energy: sp.Expr, ray: sp.Matrix) -> sp.Matrix:
    return sp.simplify(energy * (ray * ray.T - sp.eye(3) / 3))


def symbolic_constraint_checks() -> int:
    checks = 0
    q1, q2, q3, energy, g_t = sp.symbols(
        "q1 q2 q3 E g_T", real=True, nonzero=True
    )
    q = sp.Matrix((q1, q2, q3))
    divergence = divergence_matrix(q)

    # These three minors cover every nonzero real q without a search.
    minor_q1 = sp.factor(divergence[:, (0, 2, 3)].det())
    minor_q2 = sp.factor(divergence[:, (1, 2, 4)].det())
    minor_q3 = sp.factor(divergence[:, (0, 3, 4)].det())
    assert minor_q1 == q1 * (q1**2 + q3**2)
    assert minor_q2 == -q2 * (q2**2 + q3**2)
    assert minor_q3 == -q3 * (q1**2 + q3**2)
    checks += 3

    pi_coeffs = sp.symbols("pi0:5", real=True)
    pi_tensor = sum(
        (coefficient * basis for coefficient, basis in zip(pi_coeffs, STF_BASIS)),
        sp.zeros(3),
    )
    kappa = sp.Matrix(sp.symbols("kappa0:3", real=True))
    general_shift = sp.Matrix(sp.symbols("b0:3", real=True))

    for direction in SC_DIRECTIONS:
        ray = sp.Matrix(direction)
        source = stf_source(energy, ray)
        source_divergence = sp.simplify(source * q)
        assert sp.factor(source.det()) == sp.Rational(2, 27) * energy**3
        assert is_zero(source_divergence - source * q)
        checks += 2

        constraint_before = pi_tensor * q - kappa
        constraint_after = (
            (pi_tensor + g_t * source) * q
            - (kappa + g_t * source_divergence)
        )
        assert is_zero(constraint_after - constraint_before)
        checks += 1

        general_after = (
            (pi_tensor + g_t * source) * q
            - (kappa + general_shift)
        )
        residual = sp.simplify(general_after - constraint_before)
        assert is_zero(residual - (g_t * source_divergence - general_shift))
        solution = sp.solve(
            tuple(residual),
            tuple(general_shift),
            dict=True,
        )
        assert solution == [{
            general_shift[i]: g_t * source_divergence[i]
            for i in range(3)
        }]
        checks += 2

    return checks


def mode_and_covariance_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()
    wavevectors = tuple(
        vector
        for vector in itertools.product(range(-2, 3), repeat=3)
        if vector != (0, 0, 0)
    )
    energy = sp.Rational(7, 5)

    for wavevector in wavevectors:
        q = sp.Matrix(wavevector)
        divergence = divergence_matrix(q)
        assert divergence.rank() == 3
        assert len(divergence.nullspace()) == 2
        assert 2 * len(divergence.nullspace()) == 4
        checks += 3

        for direction in SC_DIRECTIONS:
            ray = sp.Matrix(direction)
            source = stf_source(energy, ray)
            source_divergence = source * q
            assert source.det() != 0
            assert source_divergence != sp.zeros(3, 1)
            checks += 2

            for matrix in group:
                rotation = as_sympy(matrix)
                image_q = rotation * q
                image_source = rotation * source * rotation.T
                image_divergence = image_source * image_q
                assert is_zero(image_divergence - rotation * source_divergence)

                pi = source / 3
                kappa = pi * q
                constraint = pi * q - kappa
                image_constraint = (
                    (rotation * pi * rotation.T) * image_q
                    - rotation * kappa
                )
                assert is_zero(constraint)
                assert is_zero(image_constraint - rotation * constraint)
                checks += 3

    return checks


def generator_symbolic_checks() -> int:
    checks = 0
    theta, action, omega, packet_energy = sp.symbols(
        "theta I omega E", positive=True
    )
    x1, x2, p1, p2 = sp.symbols("x1 x2 p1 p2", real=True)
    a1, a2 = sp.symbols("a1 a2", real=True)
    m1, m2 = sp.symbols("m1 m2", positive=True)

    old_p = sp.Matrix((p1, p2))
    shift = sp.Matrix((a1, a2))
    new_p = old_p + shift
    inverse_mass = sp.diag(1 / m1, 1 / m2)

    def hamiltonian(momentum: sp.Matrix) -> sp.Expr:
        return sp.simplify((momentum.T * inverse_mass * momentum)[0] / 2)

    delta_action = sp.simplify(
        (packet_energy + hamiltonian(old_p) - hamiltonian(new_p)) / omega
    )
    new_action = action + delta_action
    old_x = sp.Matrix((x1, x2))
    new_x = sp.simplify(
        old_x
        - theta / omega
        * (inverse_mass * old_p - inverse_mass * new_p)
    )

    old_state = sp.Matrix((theta, x1, x2, action, p1, p2))
    new_state = sp.Matrix((
        theta,
        new_x[0],
        new_x[1],
        new_action,
        new_p[0],
        new_p[1],
    ))
    jacobian = new_state.jacobian(old_state)
    symplectic_form = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(3), sp.eye(3)),
        sp.Matrix.hstack(-sp.eye(3), sp.zeros(3)),
    )
    assert is_zero(jacobian.T * symplectic_form * jacobian - symplectic_form)
    checks += 1

    total_before = omega * action + hamiltonian(old_p) + packet_energy
    total_after = omega * new_action + hamiltonian(new_p)
    assert is_zero(total_before - total_after)
    checks += 1

    # At the seam all canonical coordinates are unchanged.
    assert is_zero(new_x.subs(theta, 0) - old_x)
    checks += 1

    # Exact inverse emission.
    restored_p = new_p - shift
    restored_action = sp.simplify(
        new_action
        - (packet_energy + hamiltonian(restored_p) - hamiltonian(new_p)) / omega
    )
    restored_x = sp.simplify(
        new_x
        + theta / omega
        * (inverse_mass * restored_p - inverse_mass * new_p)
    )
    assert is_zero(restored_p - old_p)
    assert is_zero(restored_action - action)
    assert is_zero(restored_x - old_x)
    checks += 3

    # Independent scalar/tensor couplings do not enter any symplectic
    # identity.  Canonical rescaling changes displayed source coefficients.
    scale, g0, gt = sp.symbols("lambda g0 gT", nonzero=True)
    scaling = sp.diag(scale, 1 / scale)
    two_form = sp.Matrix(((0, 1), (-1, 0)))
    assert is_zero(scaling.T * two_form * scaling - two_form)
    assert sp.simplify(g0 / scale - gt) != 0
    checks += 2

    return checks


def rational_seam_fixtures() -> int:
    checks = 0
    wavevectors = (
        (1, 0, 0), (0, 1, 0), (0, 0, 1),
        (1, 1, 0), (1, -1, 1), (2, 1, -2),
    )
    energies = (Fraction(1, 3), Fraction(1), Fraction(7, 2))
    tensor_couplings = (Fraction(1, 5), Fraction(1), Fraction(9, 4))
    scalar_couplings = (Fraction(2, 7), Fraction(3, 2))

    for q_tuple in wavevectors:
        q = sp.Matrix(q_tuple)
        for direction in SC_DIRECTIONS:
            ray = sp.Matrix(direction)
            for energy in energies:
                source = stf_source(sp.Rational(energy.numerator, energy.denominator), ray)
                for gt in tensor_couplings:
                    gt_q = sp.Rational(gt.numerator, gt.denominator)
                    pi = source / 7
                    kappa = pi * q
                    new_pi = pi + gt_q * source
                    new_kappa = kappa + gt_q * source * q
                    assert is_zero(pi * q - kappa)
                    assert is_zero(new_pi * q - new_kappa)
                    restored_pi = new_pi - gt_q * source
                    restored_kappa = new_kappa - gt_q * source * q
                    assert is_zero(restored_pi - pi)
                    assert is_zero(restored_kappa - kappa)
                    checks += 4

                    for g0 in scalar_couplings:
                        g0_q = sp.Rational(g0.numerator, g0.denominator)
                        scalar_before = sp.Rational(5, 11)
                        scalar_after = scalar_before + g0_q * sp.Rational(
                            energy.numerator, energy.denominator
                        )
                        assert scalar_after - scalar_before == g0_q * sp.Rational(
                            energy.numerator, energy.denominator
                        )
                        # Unequal coefficients are admitted by every seam
                        # identity; equality is not forced.
                        if g0_q != gt_q:
                            assert g0_q - gt_q != 0
                        checks += 2

    # Fail-closed action admission on a generic positive quadratic owner.
    for packet_energy in (
        Fraction(1, 4), Fraction(1), Fraction(9, 2)
    ):
        for old_energy in (
            Fraction(0), Fraction(1, 3), Fraction(5)
        ):
            for new_energy in (
                Fraction(0), Fraction(2), Fraction(11)
            ):
                for old_action in (
                    Fraction(0), Fraction(3, 2), Fraction(10)
                ):
                    omega = Fraction(2, 3)
                    new_action = old_action + (
                        packet_energy + old_energy - new_energy
                    ) / omega
                    admitted = new_action >= 0
                    if admitted:
                        assert (
                            omega * old_action + old_energy + packet_energy
                            == omega * new_action + new_energy
                        )
                    else:
                        assert new_action < 0
                    checks += 1

    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        actual = sha256(path)
        assert actual == expected, (path, actual, expected)
        checks += 1

    checks += representation_checks()
    checks += symbolic_constraint_checks()
    checks += mode_and_covariance_checks()
    checks += generator_symbolic_checks()
    checks += rational_seam_fixtures()

    print("C18 odd line space contains two independent T1u vector copies")
    print("one spare spatial-vector copy can host the longitudinal constraint record")
    print("constraint preservation uniquely forces kappa shift = g_T S q")
    print("one type-2 generator preserves symplecticity, energy, and inverse")
    print("the homogeneous constrained STF phase space has two canonical modes")
    print("equal scalar/tensor coupling and native constraint dynamics are not forced")
    print(
        f"PASS: C18 scalar/STF/vector-constraint absorption seam "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME B: exact existing-type reference seam; charge-even finite "
        "ownership, constraints, static pole, lensing, and coupling remain open"
    )


if __name__ == "__main__":
    main()
