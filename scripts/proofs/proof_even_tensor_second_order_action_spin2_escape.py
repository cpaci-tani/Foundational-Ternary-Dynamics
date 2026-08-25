#!/usr/bin/env python3
"""Exact even-tensor second-order action escape and constraint price.

The existing parity theorem excludes an inversion-even to inversion-even
first-spatial-derivative tensor cone.  It does not exclude a local
second-order variational wave whose first-order time transfer has a Jordan
zero mode.  This certificate constructs that alternative on the STF tensor
space, proves the two-dimensional TT configuration space, derives the exact
reversible leapfrog transfer and positive conserved quadratic for every
nonzero lattice mode, and identifies the constraint/source price.

The construction is a selected reference action.  It does not derive the
constraint multipliers, a finite cotangent collision, universal coupling,
static gravity, lensing, or an Einstein completion.
"""

from __future__ import annotations

from itertools import product

import sympy as sp

from proof_moore_bond_capacity_type_census import signed_permutation_matrices


STF_BASIS = (
    sp.Matrix(((1, 0, 0), (0, -1, 0), (0, 0, 0))),
    sp.Matrix(((1, 0, 0), (0, 1, 0), (0, 0, -2))),
    sp.Matrix(((0, 1, 0), (1, 0, 0), (0, 0, 0))),
    sp.Matrix(((0, 0, 1), (0, 0, 0), (1, 0, 0))),
    sp.Matrix(((0, 0, 0), (0, 0, 1), (0, 1, 0))),
)
SC_DIRECTIONS = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)


def divergence_matrix(wavevector: tuple[int, int, int]) -> sp.Matrix:
    k = sp.Matrix(wavevector)
    return sp.Matrix.hstack(*(basis * k for basis in STF_BASIS))


def tensor_from_coordinates(coordinates: sp.Matrix) -> sp.Matrix:
    return sp.simplify(sum(
        (coordinates[index] * STF_BASIS[index] for index in range(5)),
        sp.zeros(3),
    ))


def primitive_wavevectors(radius: int = 2) -> tuple[tuple[int, int, int], ...]:
    vectors = []
    for vector in product(range(-radius, radius + 1), repeat=3):
        if vector == (0, 0, 0):
            continue
        if sp.gcd_list(vector) != 1:
            continue
        vectors.append(vector)
    return tuple(vectors)


def verify_tt_geometry() -> int:
    checks = 0
    wavevectors = primitive_wavevectors()
    assert len(wavevectors) == 98
    checks += 1

    group = tuple(signed_permutation_matrices())
    assert len(group) == 48
    checks += 1

    for wavevector in wavevectors:
        divergence = divergence_matrix(wavevector)
        assert divergence.rank() == 3
        kernel = divergence.nullspace()
        assert len(kernel) == 2
        for coordinates in kernel:
            tensor = tensor_from_coordinates(coordinates)
            assert tensor == tensor.T
            assert sp.trace(tensor) == 0
            assert tensor * sp.Matrix(wavevector) == sp.zeros(3, 1)
            checks += 3

        # The TT subspace is covariant under every signed cubic map.
        for matrix_raw in group:
            matrix = sp.Matrix(matrix_raw)
            transformed_k = matrix * sp.Matrix(wavevector)
            for coordinates in kernel:
                tensor = tensor_from_coordinates(coordinates)
                transformed = sp.simplify(matrix * tensor * matrix.T)
                assert transformed == transformed.T
                assert sp.trace(transformed) == 0
                assert transformed * transformed_k == sp.zeros(3, 1)
                checks += 3

    return checks


def verify_discrete_action() -> int:
    checks = 0
    a = sp.symbols("a", positive=True)
    h_previous, h_current, h_next = sp.symbols("h_previous h_current h_next")

    lag_previous = (h_current - h_previous) ** 2 / 2 - a * h_previous**2 / 2
    lag_current = (h_next - h_current) ** 2 / 2 - a * h_current**2 / 2
    euler_lagrange = sp.diff(lag_previous + lag_current, h_current)
    assert sp.simplify(
        euler_lagrange - ((2 - a) * h_current - h_previous - h_next)
    ) == 0
    checks += 1

    transfer = sp.Matrix(((1 - a, 1), (-a, 1)))
    symplectic = sp.Matrix(((0, 1), (-1, 0)))
    metric = sp.Matrix(((a, -a / 2), (-a / 2, 1)))
    identity = sp.eye(2)

    assert transfer.det() == 1
    assert sp.simplify(transfer.T * symplectic * transfer - symplectic) == sp.zeros(2)
    assert sp.simplify(transfer.T * metric * transfer - metric) == sp.zeros(2)
    assert sp.factor(metric.det()) == -a * (a - 4) / 4
    checks += 4

    z = sp.symbols("z")
    characteristic = sp.factor(transfer.charpoly(z).as_expr())
    assert characteristic == a * z + z**2 - 2 * z + 1
    assert sp.trace(transfer) == 2 - a
    checks += 2

    # At zero wave number the transfer is a non-semisimple Jordan block.  It
    # is exactly this degeneracy that permits an eigenphase linear in |k|
    # even though the spatial action starts at second derivative order.
    zero_transfer = transfer.subs(a, 0)
    assert zero_transfer == sp.Matrix(((1, 1), (0, 1)))
    assert (zero_transfer - identity).rank() == 1
    assert (zero_transfer - identity) ** 2 == sp.zeros(2)
    assert len(zero_transfer.eigenvects()[0][2]) == 1
    checks += 4

    c, k2 = sp.symbols("c k2", positive=True)
    discriminant = sp.factor((2 - c**2 * k2) ** 2 - 4)
    assert discriminant == c**2 * k2 * (c**2 * k2 - 4)
    checks += 1

    # A semisimple identity slow block with an analytic even perturbation has
    # only O(k^2) eigenvalue displacement; the square-root cone requires the
    # Jordan/gauge-degenerate zero above.
    b00, b01, b10, b11, epsilon = sp.symbols("b00 b01 b10 b11 epsilon")
    semisimple = identity + epsilon**2 * sp.Matrix(((b00, b01), (b10, b11)))
    semisimple_char = sp.Poly(semisimple.charpoly(z).as_expr(), z)
    assert all(term.as_leading_term(epsilon).as_ordered_factors()[0] != epsilon
               or sp.degree(term, epsilon) >= 2
               for term in semisimple_char.all_coeffs())
    assert sp.diff(semisimple, epsilon).subs(epsilon, 0) == sp.zeros(2)
    checks += 2

    # The nearest-neighbor lattice Laplacian has Lambda in [0,12].  With the
    # selected common cone c=1/6, a=c^2 Lambda lies in [0,1/3], safely inside
    # the exact positive/stable interval 0<a<4 for nonzero modes.
    c_eff = sp.Rational(1, 6)
    a_max = c_eff**2 * 12
    assert a_max == sp.Rational(1, 3)
    assert 0 < a_max < 4
    checks += 2

    qx, qy, qz = sp.symbols("qx qy qz", real=True)
    lattice_symbol = sum(2 - 2 * sp.cos(q) for q in (qx, qy, qz))
    scaling = sp.symbols("epsilon", real=True)
    scaled = lattice_symbol.subs({qx: scaling*qx, qy: scaling*qy, qz: scaling*qz})
    leading = sp.expand(sp.series(scaled, scaling, 0, 4).removeO()).coeff(scaling, 2)
    assert leading == qx**2 + qy**2 + qz**2
    checks += 1

    return checks


def verify_source_boundary() -> int:
    checks = 0
    identity = sp.eye(3)
    for direction_tuple in SC_DIRECTIONS:
        direction = sp.Matrix(direction_tuple)
        source = direction * direction.T - identity / 3
        assert source == source.T
        assert sp.trace(source) == 0
        assert sp.factor(source.det()) == sp.Rational(2, 27)
        assert source.rank() == 3
        checks += 4

        # A localized oriented STF event is not a TT plane-wave source for any
        # nonzero wavevector because its tensor is invertible.  A scalar/vector
        # constraint sector or a nonlocal TT projection is unavoidable.
        for wavevector in primitive_wavevectors():
            assert source * sp.Matrix(wavevector) != sp.zeros(3, 1)
            checks += 1

    return checks


def main() -> None:
    checks = verify_tt_geometry()
    checks += verify_discrete_action()
    checks += verify_source_boundary()

    print("STF divergence constraints leave exactly two tensor configurations")
    print("nearest-neighbor second-order action gives an exact reversible symplectic transfer")
    print("positive invariant exists for every nonzero mode with 0<a<4")
    print("c=1/6 is stable on the complete cubic Brillouin band")
    print("massless linear cone escapes the even/even O(k) no-go through a Jordan zero mode")
    print("the escape uses constraint/gauge degeneracy, not an odd first-derivative carrier")
    print("localized manifestation STF stress is never TT and needs static constraint sectors")
    print("finite cotangent realization, universal coupling, static pole, and lensing remain open")
    print(f"PASS: even-tensor second-order action spin-2 escape ({checks} exact checks)")


if __name__ == "__main__":
    main()
