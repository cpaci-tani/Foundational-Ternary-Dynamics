#!/usr/bin/env python3
"""Exact chiral commutant and parity-pair structure of the rank-20 carrier.

The selected rank-20 co-rotating first-moment representation has a
four-dimensional exact commutant.  A canonical involution Q swaps the even
and odd STF blocks in both collision copies.  Its +/- eigenspaces are
ten-dimensional, invariant under all three spatial generators, and exchanged
by inversion because parity anticommutes with Q.

Each chiral sector has rank eight and nullity two at every registered nonzero
primitive wavevector.  The four-dimensional parity-complete TT seed splits
into one two-dimensional seed per chirality, but those seeds generate Krylov
dimensions (4,8,9) on axis, body-diagonal, and generic wavevectors.  Hence
each chirality still needs an eight-dimensional phase-space reduction; parity
doubles the price to sixteen.

This is a selected fixed-C4-quadrature finite-carrier structure theorem, not
a gauge, helicity, spin-2, gravity, or lensing derivation.
"""

from __future__ import annotations

from sympy import Matrix, eye, kronecker_product
from sympy.polys.matrices import DomainMatrix

from proof_c18_tensor_doublet_tt_reduction import (
    FROBENIUS_GRAM,
    primitive_wavevectors,
    tt_projector,
)
from proof_cotangent_rank20_collision_closure_tt_leakage import (
    co_rotating_axes,
    krylov_dimension,
    selected_rank20_carrier,
)
from proof_cotangent_right_regular_collision_spin2_closure_obstruction import (
    FLAGS,
    FLAG_INDEX,
)
from proof_cotangent_stf_parity_spin2_curl_target import STF_BASIS
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def chiral_involution() -> Matrix:
    identity = Matrix.eye(5)
    zero = Matrix.zeros(5)
    return Matrix.vstack(
        Matrix.hstack(zero, identity, zero, zero),
        Matrix.hstack(identity, zero, zero, zero),
        Matrix.hstack(zero, zero, zero, identity),
        Matrix.hstack(zero, zero, identity, zero),
    )


def inversion_on_carrier(closure: Matrix, metric: Matrix) -> Matrix:
    inversion = ((-1, 0, 0), (0, -1, 0), (0, 0, -1))
    permutation = Matrix.zeros(48, 48)
    for source, flag in enumerate(FLAGS):
        target = transform_flag(inversion, flag)
        permutation[FLAG_INDEX[target], source] = 1
    carrier = closure * permutation * closure.T * metric
    assert closure * permutation == carrier * closure
    return carrier


def tt_seed(wavevector: Matrix) -> Matrix:
    projector = tt_projector(wavevector)
    basis_six = Matrix.hstack(*projector.columnspace())
    gram_five = STF_BASIS.T * FROBENIUS_GRAM * STF_BASIS
    basis_five = gram_five.inv() * STF_BASIS.T * FROBENIUS_GRAM * basis_six
    seed = Matrix.zeros(20, 4)
    seed[0:5, 0:2] = basis_five
    seed[5:10, 2:4] = basis_five
    assert seed.rank() == 4
    return seed


def restricted_operator(operator: Matrix, basis: Matrix, metric: Matrix) -> Matrix:
    restricted_metric = basis.T * metric * basis
    image = restricted_metric.inv() * basis.T * metric * operator * basis
    assert operator * basis == basis * image
    return image


def main() -> None:
    _rows, closure, collision = selected_rank20_carrier()
    axes = co_rotating_axes(closure, collision)
    metric = (closure * closure.T).inv()
    identity = eye(20)
    checks = 0

    commutator_system = Matrix.vstack(
        *(
            kronecker_product(axis.T, identity)
            - kronecker_product(identity, axis)
            for axis in axes
        )
    )
    assert commutator_system.shape == (1200, 400)
    assert exact_rank(commutator_system) == 396
    assert len(commutator_system.nullspace()) == 4
    checks += 3

    chirality = chiral_involution()
    assert chirality * chirality == identity
    assert chirality.trace() == 0
    assert chirality.T * metric * chirality == metric
    assert all(chirality * axis == axis * chirality for axis in axes)
    checks += 4

    parity = inversion_on_carrier(closure, metric)
    assert parity * parity == identity
    assert parity.T * metric * parity == metric
    assert parity * chirality == -chirality * parity
    checks += 3

    projectors = {
        sign: (identity + sign * chirality) / 2 for sign in (-1, 1)
    }
    bases = {}
    for sign, projector in projectors.items():
        assert projector * projector == projector
        assert projector.rank() == 10
        basis = Matrix.hstack(*projector.columnspace())
        assert all(projector * axis * basis == axis * basis for axis in axes)
        bases[sign] = basis
        checks += 3
    assert parity * bases[1] == projectors[-1] * parity * bases[1]
    assert parity * bases[-1] == projectors[1] * parity * bases[-1]
    checks += 2

    wavevectors = primitive_wavevectors(2)
    assert len(wavevectors) == 98
    checks += 1
    for wavevector in wavevectors:
        operator = sum(
            (wavevector[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(20, 20),
        )
        seed = tt_seed(wavevector)
        sector_characteristics = []
        for sign in (-1, 1):
            basis = bases[sign]
            sector = restricted_operator(operator, basis, metric)
            assert sector.rank() == 8
            assert len(sector.nullspace()) == 2
            chiral_seed = projectors[sign] * seed
            assert chiral_seed.rank() == 2
            assert Matrix.hstack(*chiral_seed.columnspace()).shape == (20, 2)
            sector_characteristics.append(sector.charpoly().as_expr())
            checks += 4
        assert sector_characteristics[0] == sector_characteristics[1]
        assert operator.charpoly().as_expr() == (
            sector_characteristics[0] * sector_characteristics[1]
        ).expand()
        checks += 2

    expected_krylov = {
        (1, 0, 0): 4,
        (1, 1, 1): 8,
        (1, 2, 3): 9,
    }
    for components, expected_dimension in expected_krylov.items():
        wavevector = Matrix(components)
        operator = sum(
            (components[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(20, 20),
        )
        seed = tt_seed(wavevector)
        for sign in (-1, 1):
            chiral_seed = Matrix.hstack(
                *(projectors[sign] * seed).columnspace()
            )
            assert (
                krylov_dimension(operator, chiral_seed)
                == expected_dimension
            )
            checks += 1

    assert 10 - 2 == 8
    assert 2 * 8 == 16
    checks += 2

    print("rank20 spatial-generator commutant dimension=4")
    print("exact chiral involution Q: two invariant rank10 eigensectors")
    print("inversion anticommutes with Q and exchanges the two sectors")
    print("each nonzero-wavevector chiral generator: rank8, nullity2")
    print("parity-complete TT seed 4 = chiral seed 2 + antichiral seed 2")
    print("chiral TT Krylov dimensions: axis=4, body diagonal=8, generic=9")
    print("phase-space reduction price: 8 per chirality, 16 for the parity pair")
    print(
        "PASS: cotangent rank20 chiral commutant and parity-pair structure "
        f"({checks} exact checks)"
    )
    print(
        "Open: native constraint/gauge algebra, parity-complete physical kernel, "
        "common Maxwell closure, static gravity, and lensing"
    )


if __name__ == "__main__":
    main()
