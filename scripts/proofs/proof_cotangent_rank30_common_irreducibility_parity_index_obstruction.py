#!/usr/bin/env python3
"""Exact rank-30 common-carrier irreducibility and parity-index obstruction.

On one fixed C4 quadrature, the minimum selected cotangent witness that
contains both the symmetric tensor curl and the Maxwell moments closes at
dimension thirty: a tensor-20 collision closure plus an independent
Maxwell-10 collision closure.  This certificate tests the *streaming
generator* on that fixed-slice carrier.  The phase-complete successor has
rank fifty and a different C4-enforced block structure.

The result is sharper than a direct-sum picture.  The three exact first
moments have a one-dimensional joint commutant, spanned by the identity, so
there is no nontrivial momentum-independent projector separating Maxwell and
tensor modes.  Spatial inversion still supplies an energy-orthogonal parity
grading of dimensions 17+13 and anticommutes with every first moment.
Eighty-six of the 98 registered nonzero primitive wavevectors saturate the
resulting rectangular parity maps at rank 26, leaving four parity-even zero
modes.  The twelve FCC face diagonals fall further to rank 24, with a 5+1
even/odd kernel.  Those kernels have no common momentum-independent vector.

A generic eight-dimensional transverse-Maxwell plus TT seed generates all
thirty carrier dimensions.  The normalized nonzero characteristic
polynomials on an axis and a body diagonal are coprime, excluding an exact
isotropic linear cone in this selected generator.  Conditional Dirac
accounting therefore requires 2F+S=22 to reach the desired eight-dimensional
Maxwell-plus-helicity-two phase space, but constraints cannot repair an
eigenvalue absent from the parent symbol.  The four parity-index zero modes
do not by themselves supply that reduction and are not promoted here to
first-class constraints.

This is a selected layer-zero linear boundary theorem, not a native gauge
algebra, Maxwell, spin-2, gravity, lensing, or unified-action derivation.
"""

from __future__ import annotations

from sympy import Matrix, Poly, eye, kronecker_product, symbols
from sympy.polys.matrices import DomainMatrix

from proof_c18_tensor_doublet_tt_reduction import primitive_wavevectors
from proof_cotangent_common_maxwell_tensor_collision_closure_price import (
    maxwell_rows,
)
from proof_cotangent_rank20_chiral_commutant_parity_pair import (
    tt_seed as tensor_tt_seed,
)
from proof_cotangent_rank20_collision_closure_tt_leakage import (
    SELECTED_COLLISION_INDEX,
    displacement_sum,
    krylov_dimension,
    tensor_rows,
)
from proof_cotangent_right_regular_collision_spin2_closure_obstruction import (
    FLAGS,
    FLAG_INDEX,
    GROUP,
    right_regular_permutation,
)
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag


COMMON_PHASE_SPACE_DIMENSION = 30
PHYSICAL_MAXWELL_TENSOR_PHASE_SPACE_DIMENSION = 8


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def selected_common_rank30_carrier():
    """Return a low-height integer basis for the selected common closure."""
    collision = right_regular_permutation(GROUP[SELECTED_COLLISION_INDEX])
    tensor = tensor_rows(0)
    maxwell = maxwell_rows(0)

    tensor_closure = Matrix.vstack(tensor, tensor * collision)
    # In this selected collision, the three collided B rows are precisely the
    # three independent Maxwell fast copies missing from the original seven.
    maxwell_closure = Matrix.vstack(maxwell, maxwell[4:7, :] * collision)
    common = Matrix.vstack(tensor_closure, maxwell_closure)

    checks = 0
    assert GROUP[SELECTED_COLLISION_INDEX] == (
        (-1, 0, 0),
        (0, 0, -1),
        (0, -1, 0),
    )
    assert collision * collision == eye(48)
    checks += 2

    assert tensor_closure.shape == (20, 48)
    assert exact_rank(tensor_closure) == 20
    assert exact_rank(Matrix.vstack(tensor_closure, tensor_closure * collision)) == 20
    checks += 3

    assert maxwell_closure.shape == (10, 48)
    assert exact_rank(maxwell_closure) == 10
    assert exact_rank(Matrix.vstack(maxwell_closure, maxwell_closure * collision)) == 10
    checks += 3

    assert common.shape == (30, 48)
    assert exact_rank(common) == 30
    assert exact_rank(Matrix.vstack(common, common * collision)) == 30
    checks += 3
    return common, collision, checks


def common_axes(common: Matrix, collision: Matrix):
    gram = common * common.T
    metric = gram.inv()
    carrier_collision = common * collision * common.T * metric
    assert common * collision == carrier_collision * common
    assert carrier_collision * carrier_collision == eye(30)
    checks = 2

    axes = []
    for axis in range(3):
        raw_moment = (
            common
            * displacement_sum(axis)
            * collision
            * common.T
            * metric
        )
        co_rotating = carrier_collision.inv() * raw_moment
        assert co_rotating.T * metric == metric * co_rotating
        assert exact_rank(co_rotating) == 26
        axes.append(co_rotating)
        checks += 2
    return tuple(axes), metric, checks


def verify_scalar_commutant(axes) -> int:
    identity = eye(30)
    system = Matrix.vstack(
        *(
            kronecker_product(axis.T, identity)
            - kronecker_product(identity, axis)
            for axis in axes
        )
    )
    assert system.shape == (2700, 900)
    assert exact_rank(system) == 899
    assert system * identity.reshape(900, 1) == Matrix.zeros(2700, 1)
    assert all(axis * identity == identity * axis for axis in axes)
    return 6


def inversion_on_carrier(common: Matrix, metric: Matrix) -> Matrix:
    inversion = ((-1, 0, 0), (0, -1, 0), (0, 0, -1))
    permutation = Matrix.zeros(48, 48)
    for source, flag in enumerate(FLAGS):
        target = transform_flag(inversion, flag)
        permutation[FLAG_INDEX[target], source] = 1
    parity = common * permutation * common.T * metric
    assert common * permutation == parity * common
    return parity


def verify_parity(common: Matrix, axes, metric: Matrix):
    identity = eye(30)
    parity = inversion_on_carrier(common, metric)
    assert parity * parity == identity
    assert parity.T * metric * parity == metric
    assert parity.trace() == 4
    assert all(parity * axis == -axis * parity for axis in axes)
    even = (identity + parity) / 2
    odd = (identity - parity) / 2
    assert even * even == even
    assert odd * odd == odd
    assert exact_rank(even) == 17
    assert exact_rank(odd) == 13
    assert all(even * axis * even == Matrix.zeros(30, 30) for axis in axes)
    assert all(odd * axis * odd == Matrix.zeros(30, 30) for axis in axes)
    return parity, even, odd, 13


def verify_wavevector_kernels(axes, metric: Matrix, parity: Matrix, even, odd) -> int:
    wavevectors = primitive_wavevectors(2)
    assert len(wavevectors) == 98
    checks = 1

    # There is no fixed algebraic zero-mode subspace shared by all momenta.
    assert exact_rank(Matrix.vstack(*axes)) == 30
    assert exact_rank(Matrix.vstack(*(axis.T for axis in axes))) == 30
    assert exact_rank(Matrix.hstack(*axes)) == 30
    checks += 3

    rank_census = {24: 0, 26: 0}
    for wavevector in wavevectors:
        operator = sum(
            (wavevector[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(30, 30),
        )
        kernel = operator.nullspace()
        kernel_basis = Matrix.hstack(*kernel)
        is_fcc_face_diagonal = (
            sum(component == 0 for component in wavevector) == 1
            and sorted(abs(component) for component in wavevector) == [0, 1, 1]
        )
        expected_rank = 24 if is_fcc_face_diagonal else 26
        expected_block_rank = 12 if is_fcc_face_diagonal else 13
        expected_even_kernel = 5 if is_fcc_face_diagonal else 4
        expected_odd_kernel = 1 if is_fcc_face_diagonal else 0
        rank_census[expected_rank] += 1

        assert exact_rank(operator) == expected_rank
        assert len(kernel) == 30 - expected_rank
        assert exact_rank(odd * operator * even) == expected_block_rank
        assert exact_rank(even * operator * odd) == expected_block_rank
        assert exact_rank(even * kernel_basis) == expected_even_kernel
        assert exact_rank(odd * kernel_basis) == expected_odd_kernel
        assert exact_rank(Matrix.hstack(kernel_basis, parity * kernel_basis)) == len(kernel)
        checks += 7

        for vector in kernel:
            conserved_row = vector.T * metric
            assert conserved_row * operator == Matrix.zeros(1, 30)
            checks += 1
    assert rank_census == {24: 12, 26: 86}
    checks += 1
    return checks


def common_tt_seed(wavevector: Matrix) -> Matrix:
    seed = Matrix.zeros(30, 4)
    seed[0:20, :] = tensor_tt_seed(wavevector)
    assert seed.rank() == 4
    return seed


def common_maxwell_seed(wavevector: Matrix) -> Matrix:
    transverse_vectors = Matrix([list(wavevector)]).nullspace()
    transverse = Matrix.hstack(*transverse_vectors)
    assert transverse.shape == (3, 2)
    seed = Matrix.zeros(30, 4)
    # The common basis is tensor-20 followed by
    # (number, E_x,E_y,E_z, B_x,B_y,B_z, three collided B copies).
    seed[21:24, 0:2] = transverse
    seed[24:27, 2:4] = transverse
    assert seed.rank() == 4
    return seed


def verify_seed_leakage(axes, even, odd) -> int:
    expected = {
        (1, 0, 0): (8, 10, 18),
        (1, 1, 1): (22, 18, 22),
        (1, 2, 3): (28, 28, 30),
    }
    checks = 0
    for components, dimensions in expected.items():
        wavevector = Matrix(components)
        operator = sum(
            (components[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(30, 30),
        )
        tensor_seed = common_tt_seed(wavevector)
        maxwell_seed = common_maxwell_seed(wavevector)
        combined_seed = Matrix.hstack(tensor_seed, maxwell_seed)
        assert combined_seed.rank() == 8
        assert even * combined_seed + odd * combined_seed == combined_seed
        assert exact_rank(even * combined_seed) == 4
        assert exact_rank(odd * combined_seed) == 4
        assert krylov_dimension(operator, tensor_seed) == dimensions[0]
        assert krylov_dimension(operator, maxwell_seed) == dimensions[1]
        assert krylov_dimension(operator, combined_seed) == dimensions[2]
        checks += 7
    return checks


def normalized_nonzero_speed_polynomial(axes, components, speed_squared) -> Poly:
    wavevector = Matrix(components)
    operator = sum(
        (components[axis] * axes[axis] for axis in range(3)),
        Matrix.zeros(30, 30),
    )
    eigenvalue = symbols("lambda")
    characteristic = Poly(operator.charpoly(eigenvalue).as_expr(), eigenvalue)
    nullity = 30 - exact_rank(operator)
    polynomial = 0
    for (degree,), coefficient in characteristic.terms():
        if degree < nullity:
            assert coefficient == 0
            continue
        assert (degree - nullity) % 2 == 0
        polynomial += coefficient * speed_squared ** ((degree - nullity) // 2)
    norm_squared = sum(component * component for component in components)
    return Poly(polynomial.subs(speed_squared, norm_squared * speed_squared), speed_squared).monic()


def verify_no_exact_isotropic_linear_cone(axes) -> int:
    speed_squared = symbols("x")
    axis = normalized_nonzero_speed_polynomial(
        axes, (1, 0, 0), speed_squared
    )
    body_diagonal = normalized_nonzero_speed_polynomial(
        axes, (1, 1, 1), speed_squared
    )
    expected_axis = Poly(
        (speed_squared - 1)
        * (2 * speed_squared - 5) ** 2
        * (2 * speed_squared - 1) ** 4
        * (3 * speed_squared**2 - 10 * speed_squared + 4) ** 3,
        speed_squared,
    ).monic()
    expected_body_diagonal = Poly(
        (speed_squared - 2)
        * (3 * speed_squared - 4) ** 2
        * (3 * speed_squared - 1) ** 2
        * (
            81 * speed_squared**4
            - 513 * speed_squared**3
            + 846 * speed_squared**2
            - 351 * speed_squared
            + 38
        )
        ** 2,
        speed_squared,
    ).monic()
    assert axis == expected_axis
    assert body_diagonal == expected_body_diagonal
    assert axis.gcd(body_diagonal).as_expr() == 1
    return 3


def verify_conditional_constraint_price() -> int:
    required_reduction = (
        COMMON_PHASE_SPACE_DIMENSION
        - PHYSICAL_MAXWELL_TENSOR_PHASE_SPACE_DIMENSION
    )
    assert required_reduction == 22
    solutions = tuple(
        (first_class, second_class)
        for first_class in range(12)
        for second_class in range(23)
        if 2 * first_class + second_class == required_reduction
    )
    assert solutions == tuple((first, 22 - 2 * first) for first in range(12))
    assert (4, 14) in solutions
    assert min(first + second for first, second in solutions) == 11
    assert COMMON_PHASE_SPACE_DIMENSION - 2 * 4 == 22
    assert COMMON_PHASE_SPACE_DIMENSION - 2 * 11 == 8
    return 6


def main() -> None:
    common, collision, checks = selected_common_rank30_carrier()
    axes, metric, axis_checks = common_axes(common, collision)
    checks += axis_checks
    checks += verify_scalar_commutant(axes)
    parity, even, odd, parity_checks = verify_parity(common, axes, metric)
    checks += parity_checks
    checks += verify_wavevector_kernels(axes, metric, parity, even, odd)
    checks += verify_seed_leakage(axes, even, odd)
    checks += verify_no_exact_isotropic_linear_cone(axes)
    checks += verify_conditional_constraint_price()

    print("selected fixed-C4-quadrature carrier: tensor20 + Maxwell10 = rank30")
    print("joint spatial-generator commutant dimension=1 (scalars only)")
    print("no nontrivial momentum-independent Maxwell/tensor projector")
    print("inversion grading: parity-even17 + parity-odd13")
    print("primitive-wavevector rank census: 86x rank26, 12 FCC diagonals x rank24")
    print("generic kernel parity split=4+0; FCC-diagonal split=5+1")
    print("the momentum-dependent kernels have zero common fixed intersection")
    print("TT/Maxwell/combined Krylov dimensions:")
    print("  axis: 8 / 10 / 18")
    print("  body diagonal: 22 / 18 / 22")
    print("  generic: 28 / 28 / 30")
    print("axis/body-diagonal normalized nonzero spectra are coprime")
    print("selected unconstrained carrier has no exact isotropic linear cone")
    print("conditional fixed-quadrature physical reduction price: 2F+S=22")
    print("four optimistic first-class constraints still require S=14")
    print(
        "PASS: cotangent rank30 common irreducibility/parity-index obstruction "
        f"({checks} exact checks)"
    )
    print(
        "Open: momentum-dependent constraint/gauge complex, layer-covariant lift, "
        "isolated Maxwell and spin-2 poles, static gravity, and lensing"
    )


if __name__ == "__main__":
    main()
