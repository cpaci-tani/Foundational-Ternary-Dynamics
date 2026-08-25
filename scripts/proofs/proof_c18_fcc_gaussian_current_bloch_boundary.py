#!/usr/bin/env python3
"""Exact finite-k boundary for the FCC Gaussian-current collision.

This certificate composes the exact local collision from
proof_c18_fcc_gaussian_current_collision.py with phase-independent one-hop FCC
streaming.  It proves that the seven protected k=0 modes have zero first-order
Bloch generator and derives the exact second-order hydrodynamic matrix.

The result is scoped closed-negative for a Maxwell light cone: the transverse
current pair has chiral diffusion at O(k^2), not omega=c|k|.  No physical
constant or numerical spectral fit is used.
"""

from __future__ import annotations

from itertools import combinations

from sympy import Matrix, Rational, symbols

from proof_c18_fcc_gaussian_current_collision import (
    FCC_DIRECTIONS,
    ONE_PARTICLE_STATES,
    PHASES,
    TANGENT_INDEX,
    build_sectors,
    canonical_pair,
    exact_rank,
    homogeneous_states,
    invariant_rows,
    tangent_correction,
)


def collision_data() -> tuple[Matrix, Matrix, Matrix, Matrix]:
    spatial_pairs = tuple(
        canonical_pair(left, right)
        for left, right in combinations(FCC_DIRECTIONS, 2)
    )
    correction = tangent_correction(
        homogeneous_states(), build_sectors(spatial_pairs)
    )
    right = Matrix.hstack(*correction.nullspace())
    left = Matrix.vstack(*invariant_rows())
    gram = left * right
    assert gram.det() != 0
    return correction, left, right, gram


def streaming_generator(component: int) -> Matrix:
    return Matrix.diag(
        *[direction[component] for direction, _phase in ONE_PARTICLE_STATES]
    )


def second_order_reduced(
    correction: Matrix,
    left: Matrix,
    right: Matrix,
    gram: Matrix,
    component: int,
) -> Matrix:
    """Coefficient B in q'=q+B k_component^2 q+O(k^3)."""

    generator = streaming_generator(component)
    zero = Matrix.zeros(7, 7)
    bordered = correction.row_join(right).col_join(left.row_join(zero))
    rhs = (generator * right).col_join(Matrix.zeros(7, 7))
    solution = bordered.inv() * rhs
    auxiliary = solution[:48, :]
    assert correction * auxiliary == generator * right
    assert left * auxiliary == Matrix.zeros(7, 7)

    collision_weight = Rational(1, 5**11)
    coefficient = (
        generator * auxiliary / collision_weight
        + Rational(1, 2) * generator * generator * right
    )
    return left * coefficient * gram.inv()


def main() -> None:
    checks = 0
    correction, left, right, gram = collision_data()

    assert correction.shape == (48, 48)
    assert exact_rank(correction) == 41
    assert right.shape == (48, 7)
    assert left.shape == (7, 48)
    assert gram.shape == (7, 7)
    checks += 5

    # The k=0 unit eigenspace is semisimple: algebraic multiplicity seven was
    # certified by the exact characteristic polynomial in the parent proof,
    # and geometric multiplicity is 48-rank(N)=7 here.
    assert correction * right == Matrix.zeros(48, 7)
    assert left * correction == Matrix.zeros(7, 48)
    checks += 2

    # For M(k)=D(k)J with D=exp(-i k.d), the first reduced Bloch coefficient is
    # -i L K_a R (L R)^-1.  It vanishes identically on every cubic axis.
    for component in range(3):
        generator = streaming_generator(component)
        assert left * generator * right == Matrix.zeros(7, 7)
        checks += 1

    # Global C4 phase advance commutes with both collision and streaming.  It
    # can shift the carrier quasiphase but cannot change the missing k-linear
    # coefficient in a co-rotating frame.
    phase_cycle = Matrix.zeros(48, 48)
    for direction in FCC_DIRECTIONS:
        for phase in PHASES:
            phase_cycle[
                TANGENT_INDEX[(direction, (phase + 1) % 4)],
                TANGENT_INDEX[(direction, phase)],
            ] = 1
    assert phase_cycle**4 == Matrix.eye(48)
    assert correction * phase_cycle == phase_cycle * correction
    for component in range(3):
        generator = streaming_generator(component)
        assert generator * phase_cycle == phase_cycle * generator
        checks += 1
    checks += 2

    reduced = tuple(
        second_order_reduced(correction, left, right, gram, component)
        for component in range(3)
    )

    # Conserved-variable order: (number, Ux, Uy, Uz, Vx, Vy, Vz).  Freeze the
    # z-axis result; x/y are its cubic permutations.
    expected_z = Matrix(
        [
            [Rational(-651041, 2), 0, 0, 0, 0, 0, 0],
            [0, Rational(-244140599, 104), 0, 0, Rational(48828125, 104), 0, 0],
            [0, 0, Rational(-244140599, 104), 0, 0, Rational(48828125, 104), 0],
            [0, 0, 0, Rational(-17114253724, 8177), 0, 0, Rational(2099609375, 8177)],
            [0, Rational(-48828125, 104), 0, 0, Rational(-244140599, 104), 0, 0],
            [0, 0, Rational(-48828125, 104), 0, 0, Rational(-244140599, 104), 0],
            [0, 0, 0, Rational(-2099609375, 8177), 0, 0, Rational(-17114253724, 8177)],
        ]
    )
    assert reduced[2] == expected_z
    checks += 1

    spectral_parameter = symbols("beta")
    expected_characteristic = (
        (2 * spectral_parameter + 651041)
        * (
            416 * spectral_parameter**2
            + 1953124792 * spectral_parameter
            + 2384185302734401
        )
        ** 2
        * (
            8177 * spectral_parameter**2
            + 34228507448 * spectral_parameter
            + 36358816198732513
        )
        / 2830157824
    )
    for matrix in reduced:
        assert matrix.charpoly(spectral_parameter).as_expr().factor() == expected_characteristic
        checks += 1

    transverse_indices = (1, 2, 4, 5)
    longitudinal_indices = (0, 3, 6)
    transverse = expected_z.extract(transverse_indices, transverse_indices)
    longitudinal = expected_z.extract(longitudinal_indices, longitudinal_indices)
    assert transverse == Matrix.diag(
        Matrix(
            [
                [Rational(-244140599, 104), Rational(48828125, 104)],
                [Rational(-48828125, 104), Rational(-244140599, 104)],
            ]
        ),
        Matrix(
            [
                [Rational(-244140599, 104), Rational(48828125, 104)],
                [Rational(-48828125, 104), Rational(-244140599, 104)],
            ]
        ),
    ).permute_rows([0, 2, 1, 3]).permute_cols([0, 2, 1, 3])
    assert transverse.charpoly(spectral_parameter).as_expr().factor() == (
        416 * spectral_parameter**2
        + 1953124792 * spectral_parameter
        + 2384185302734401
    ) ** 2 / 173056
    assert longitudinal.charpoly(spectral_parameter).as_expr().factor() == (
        (2 * spectral_parameter + 651041)
        * (
            8177 * spectral_parameter**2
            + 34228507448 * spectral_parameter
            + 36358816198732513
        )
        / 16354
    )
    checks += 3

    # Each transverse polarization has beta=-a+-ib with a,b>0.  Hence its
    # Bloch eigenvalue is 1+beta k^2+O(k^3): damping and phase are quadratic,
    # and the group velocity at k=0 is exactly zero.
    transverse_real = Rational(-244140599, 104)
    transverse_imaginary_magnitude = Rational(48828125, 104)
    assert transverse_real < 0
    assert transverse_imaginary_magnitude > 0
    checks += 2

    print(f"PASS: C18/FCC Gaussian-current Bloch boundary ({checks} exact checks)")
    print("first_order_reduced_generator=0 on x,y,z")
    print("unit eigenspace at k=0 is semisimple with dimension 7")
    print(
        "transverse beta="
        f"{transverse_real} +/- i*{transverse_imaginary_magnitude} (each twice)"
    )
    print("dispersion begins at O(k^2): chiral diffusion, not omega=c|k|")
    print("common C4 clock commutes and does not repair the missing linear cone")
    print("Scoped closed negative: this collision + phase-independent streaming is not Maxwell")


if __name__ == "__main__":
    main()
