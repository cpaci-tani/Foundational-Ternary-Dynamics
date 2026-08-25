#!/usr/bin/env python3
"""Exact STF parity price and staggered spin-2 tensor-curl target.

The existing cotangent FCC dyad D is inversion even.  Its handedness-twisted
partner h*STF(D) is inversion odd.  This certificate proves that the same
finite flag alphabet spans both five-component tensor types, while the
existing phase-only tensor doublet contains only the even type and therefore
cannot possess an inversion-equivariant first-derivative self-coupling.

It then constructs the unique isotropic symmetric-curl target on an even/odd
STF pair.  On the transverse-traceless sector the exact symbol has two
helicity-two polarizations and, conditional on reusing the cotangent incidence
rate, the same speed 1/6 as the registered Maxwell sector.  This is a finite
type/target theorem, not a finite collision lift, gravity derivation, or
lensing result.
"""

from __future__ import annotations

from sympy import I, Matrix, Rational, diag, symbols

from proof_c18_tensor_doublet_tt_reduction import (
    FROBENIUS_GRAM,
    primitive_wavevectors,
    symmetric_coordinates,
    symmetric_matrix,
    tt_projector,
)
from proof_cotangent_fcc_dyad_tensor_doublet_and_source import (
    dyad,
    exact_rank,
    flatten_symmetric,
)
from proof_cotangent_stabilizer_packet_gauss_source import packet
from proof_global_c3_cotangent_layer_hodge_maxwell_target import internal_tick
from proof_hodge_flag_pair_collision_invariant_space import (
    PHASE_COORDINATES,
    one_particle_states,
)
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    signed_permutation_matrices,
)
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


TRACE_ROW = Matrix([[1, 1, 1, 0, 0, 0]])
STF_BASIS = Matrix.hstack(
    Matrix([1, 0, -1, 0, 0, 0]),
    Matrix([0, 1, -1, 0, 0, 0]),
    Matrix([0, 0, 0, 1, 0, 0]),
    Matrix([0, 0, 0, 0, 1, 0]),
    Matrix([0, 0, 0, 0, 0, 1]),
)
C_EFF = Rational(1, 6)


def stf_dyad(state, layer: int) -> Matrix:
    value = dyad(state, layer)
    return value - value.trace() * Matrix.eye(3) / 3


def stf_coordinates(state, layer: int) -> Matrix:
    return Matrix(flatten_symmetric(stf_dyad(state, layer)))


def even_odd_value(state, layer: int) -> Matrix:
    handedness = state[0][2]
    value = stf_coordinates(state, layer)
    return Matrix.vstack(value, handedness * value)


def phase_parity_quartet(state, layer: int) -> Matrix:
    handedness = state[0][2]
    _flag, phase = state
    u, v = PHASE_COORDINATES[phase]
    value = stf_coordinates(state, layer)
    return Matrix.vstack(
        u * value,
        v * value,
        u * handedness * value,
        v * handedness * value,
    )


def cross_matrix(k: Matrix) -> Matrix:
    kx, ky, kz = k
    return Matrix(
        [
            [0, -kz, ky],
            [kz, 0, -kx],
            [-ky, kx, 0],
        ]
    )


def symmetric_curl_tensor(k: Matrix, tensor: Matrix) -> Matrix:
    cross = cross_matrix(k)
    return (cross * tensor - tensor * cross) / 2


def symmetric_curl_matrix(k: Matrix) -> Matrix:
    columns = []
    for coordinate in range(6):
        basis = Matrix([int(index == coordinate) for index in range(6)])
        columns.append(
            symmetric_coordinates(
                symmetric_curl_tensor(k, symmetric_matrix(basis))
            )
        )
    return Matrix.hstack(*columns)


def restricted_matrix(operator: Matrix, basis: Matrix, gram: Matrix) -> Matrix:
    metric = basis.T * gram * basis
    result = metric.inv() * basis.T * gram * operator * basis
    assert operator * basis == basis * result
    return result


def verify_finite_parity_types() -> int:
    checks = 0
    states = one_particle_states()
    group = tuple(signed_permutation_matrices())
    assert len(states) == 192
    assert len(group) == 48
    checks += 2

    for layer in range(3):
        even_odd_rows = Matrix.hstack(
            *(even_odd_value(state, layer) for state in states)
        )
        phase_rows = Matrix.hstack(
            *(phase_parity_quartet(state, layer) for state in states)
        )
        assert exact_rank(even_odd_rows) == 10
        assert exact_rank(phase_rows) == 20
        checks += 2

        for state in states:
            current = phase_parity_quartet(state, layer)
            advanced = phase_parity_quartet(
                internal_tick(state), (layer - 1) % 3
            )
            qe, pe, qo, po = (
                current[0:6, 0],
                current[6:12, 0],
                current[12:18, 0],
                current[18:24, 0],
            )
            assert advanced[0:6, 0] == -pe
            assert advanced[6:12, 0] == qe
            assert advanced[12:18, 0] == -po
            assert advanced[18:24, 0] == qo
            assert stf_dyad(internal_tick(state), (layer - 1) % 3) == stf_dyad(
                state, layer
            )
            checks += 5

            source_even = stf_dyad(state, layer)
            source_odd = state[0][2] * source_even
            for signed_rotation in group:
                transformed_state = (
                    transform_flag(signed_rotation, state[0]),
                    state[1],
                )
                rotation = Matrix(signed_rotation)
                determinant = determinant_3(signed_rotation)
                transformed_even = stf_dyad(transformed_state, layer)
                transformed_odd = transformed_state[0][2] * transformed_even
                assert transformed_even == rotation * source_even * rotation.T
                assert transformed_odd == (
                    determinant * rotation * source_odd * rotation.T
                )
                checks += 2

    return checks


def verify_tensor_curl() -> int:
    checks = 0
    wavelength = symbols("k", positive=True)
    vectors = primitive_wavevectors(2)
    assert len(vectors) == 98
    checks += 1

    for k in vectors:
        k2 = (k.T * k)[0]
        curl = symmetric_curl_matrix(k)
        projector = tt_projector(k)

        assert TRACE_ROW * curl * STF_BASIS == Matrix.zeros(1, 5)
        assert curl.T * FROBENIUS_GRAM + FROBENIUS_GRAM * curl == Matrix.zeros(6)
        assert curl * projector == projector * curl
        checks += 3

        tt_basis = Matrix.hstack(*projector.columnspace())
        assert tt_basis.shape == (6, 2)
        tt_curl = restricted_matrix(curl, tt_basis, FROBENIUS_GRAM)
        assert tt_curl * tt_curl == -k2 * Matrix.eye(2)
        checks += 2

        modal_generator = Matrix.vstack(
            Matrix.hstack(Matrix.zeros(2), I * C_EFF * tt_curl),
            Matrix.hstack(-I * C_EFF * tt_curl, Matrix.zeros(2)),
        )
        lam = modal_generator.charpoly().gen
        expected = (lam * lam + C_EFF * C_EFF * k2) ** 2
        assert (modal_generator.charpoly().as_expr() - expected).expand() == 0

        tt_gram = tt_basis.T * FROBENIUS_GRAM * tt_basis
        energy_gram = diag(tt_gram, tt_gram)
        assert (
            modal_generator.conjugate().T * energy_gram
            + energy_gram * modal_generator
            == Matrix.zeros(4)
        )
        checks += 2

    # Axis spectrum exposes helicity 0, 1, and 2 before TT reduction.
    axis = Matrix([0, 0, wavelength])
    curl = symmetric_curl_matrix(axis)
    stf_curl = restricted_matrix(curl, STF_BASIS, FROBENIUS_GRAM)
    lam = stf_curl.charpoly().gen
    expected_stf = lam * (lam * lam + wavelength * wavelength / 4) * (
        lam * lam + wavelength * wavelength
    )
    assert (stf_curl.charpoly().as_expr() - expected_stf).expand() == 0

    stf_generator = Matrix.vstack(
        Matrix.hstack(Matrix.zeros(5), I * C_EFF * stf_curl),
        Matrix.hstack(-I * C_EFF * stf_curl, Matrix.zeros(5)),
    )
    lam = stf_generator.charpoly().gen
    expected_full = (
        lam**2
        * (lam * lam + C_EFF * C_EFF * wavelength * wavelength / 4) ** 2
        * (lam * lam + C_EFF * C_EFF * wavelength * wavelength) ** 2
    )
    assert (stf_generator.charpoly().as_expr() - expected_full).expand() == 0
    checks += 2

    # Exact O(3) covariance: the curl changes tensor parity under an improper
    # transformation.  Checking a spanning STF basis suffices by linearity.
    for signed_rotation in signed_permutation_matrices():
        rotation = Matrix(signed_rotation)
        determinant = determinant_3(signed_rotation)
        for k in (Matrix([1, 0, 0]), Matrix([1, 2, 3])):
            transformed_k = rotation * k
            for column in range(STF_BASIS.cols):
                tensor = symmetric_matrix(STF_BASIS[:, column])
                transformed_tensor = rotation * tensor * rotation.T
                left = symmetric_curl_tensor(transformed_k, transformed_tensor)
                right = determinant * rotation * symmetric_curl_tensor(
                    k, tensor
                ) * rotation.T
                assert left == right
                checks += 1

    return checks


def verify_shared_packet_source() -> int:
    checks = 0
    for direction in (
        (1, 0, 0),
        (-1, 0, 0),
        (0, 1, 0),
        (0, -1, 0),
        (0, 0, 1),
        (0, 0, -1),
    ):
        column = Matrix(direction)
        expected_even = 2 * (column * column.T - Matrix.eye(3) / 3)
        for phase in range(4):
            records = packet(direction, phase)
            even_source = Matrix.zeros(3)
            odd_source = Matrix.zeros(3)
            for state in records:
                source = stf_dyad(state, 0)
                even_source += source
                odd_source += state[0][2] * source
            assert even_source == expected_even
            assert odd_source == Matrix.zeros(3)
            assert even_source.trace() == 0
            checks += 3
    return checks


def main() -> None:
    checks = verify_finite_parity_types()
    checks += verify_tensor_curl()
    checks += verify_shared_packet_source()

    print("finite cotangent STF ranks: even+odd=10, phase-parity quartet=20")
    print("existing phase-only tensor pair is inversion-even and O(k)-blocked")
    print("symmetric curl preserves TT and has exact helicity-2 speed coefficient")
    print("conditional common cotangent incidence gives c_T=c_EM=1/6")
    print("Gauss packet sources even STF=2(dd^T-I/3), odd STF=0")
    print(
        "PASS: cotangent STF parity price and spin-2 curl target "
        f"({checks} exact checks)"
    )
    print(
        "Open: finite staggered lift, action-derived TT constraints, universal "
        "static response, nonlinear completion, and lensing"
    )


if __name__ == "__main__":
    main()
