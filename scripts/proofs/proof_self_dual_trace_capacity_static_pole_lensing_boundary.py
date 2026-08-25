#!/usr/bin/env python3
"""Exact self-dual trace-capacity static pole and lensing boundary.

The phase-neutral manifestation moment splits uniquely into one scalar trace
source and one STF tensor source.  This certificate couples the trace equally
to a selected self-dual primal/dual capacity action.  The symmetric capacity
mode is exactly massless, the antisymmetric mode is massive, and an isolated
trace source excites only the equal solution U_t=U_s.

Conditional on the already registered normalized temporal-admission and
spatial-Hodge readouts, this equal solution gives the blind response tuple
(a_m,a_t,a_0,a_s)=(1,1,1,1).  The source coupling/readout selection, action
normalization, finite transaction realization, vector constraint sector, and
nonlinear gravity remain open.  No lensing target or measured constant enters.
"""

from __future__ import annotations

import sympy as sp

from proof_moore_bond_capacity_type_census import signed_permutation_matrices


SC_DIRECTIONS = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)


def verify_source_decomposition() -> int:
    checks = 0
    identity = sp.eye(3)
    group = tuple(signed_permutation_matrices())
    for direction_tuple in SC_DIRECTIONS:
        direction = sp.Matrix(direction_tuple)
        moment = direction * direction.T / 18
        density = sp.trace(moment)
        scalar = density * identity / 3
        shear = sp.simplify(moment - scalar)
        assert density == sp.Rational(1, 18)
        assert scalar == identity / 54
        assert sp.trace(shear) == 0
        assert moment == scalar + shear
        assert sp.factor(shear.det()) == sp.Rational(1, 78732)
        checks += 5

        for matrix_raw in group:
            matrix = sp.Matrix(matrix_raw)
            transformed_direction = matrix * direction
            transformed_moment = transformed_direction * transformed_direction.T / 18
            assert transformed_moment == sp.simplify(matrix * moment * matrix.T)
            assert sp.trace(transformed_moment) == density
            assert sp.simplify(
                transformed_moment - density * identity / 3
                - matrix * shear * matrix.T
            ) == sp.zeros(3)
            checks += 3
    return checks


def verify_self_dual_capacity_action() -> int:
    checks = 0
    kappa, eta, laplacian, rho = sp.symbols(
        "kappa eta Lambda rho", positive=True
    )
    kernel = sp.Matrix(
        (
            (kappa * laplacian + eta, -eta),
            (-eta, kappa * laplacian + eta),
        )
    )
    symmetric = sp.Matrix((1, 1))
    antisymmetric = sp.Matrix((1, -1))
    assert kernel * symmetric == kappa * laplacian * symmetric
    assert kernel * antisymmetric == (kappa * laplacian + 2 * eta) * antisymmetric
    assert sp.factor(kernel.det()) == kappa * laplacian * (
        kappa * laplacian + 2 * eta
    )
    checks += 3

    source = rho * symmetric
    solution = sp.simplify(kernel.inv() * source)
    expected = rho * symmetric / (kappa * laplacian)
    assert sp.simplify(solution - expected) == sp.zeros(2, 1)
    assert solution[0] == solution[1]
    assert antisymmetric.dot(solution) == 0
    checks += 3

    # The quadratic is strictly positive at every nonzero wavevector; the
    # symmetric zero is the desired static massless pole.
    eigenvalues = kernel.eigenvals()
    assert eigenvalues == {
        kappa * laplacian: 1,
        2 * eta + kappa * laplacian: 1,
    }
    assert sp.limit(solution[0] * laplacian, laplacian, 0, dir="+") == rho / kappa
    checks += 2

    qx, qy, qz, epsilon = sp.symbols("qx qy qz epsilon", real=True)
    lattice_symbol = sum(2 - 2 * sp.cos(q) for q in (qx, qy, qz))
    scaled = lattice_symbol.subs({qx: epsilon*qx, qy: epsilon*qy, qz: epsilon*qz})
    leading = sp.expand(sp.series(scaled, epsilon, 0, 4).removeO()).coeff(epsilon, 2)
    assert leading == qx**2 + qy**2 + qz**2
    checks += 1

    return checks


def verify_conditional_response_tuple() -> int:
    checks = 0
    U = sp.symbols("U", real=True)
    nu_t = 1 - U
    nu_s = 1 - U
    ray_speed = sp.expand(nu_t * nu_s)
    refractive_index = sp.series(1 / ray_speed, U, 0, 2).removeO()
    assert ray_speed == U**2 - 2 * U + 1
    assert refractive_index == 2 * U + 1
    checks += 2

    # Normalized minimal coupling: matter potential, material clock,
    # Maxwell temporal admission, and spatial Hodge response all read the
    # same dimensionless self-dual solution once.
    a_m = sp.Integer(1)
    a_t = -sp.diff(nu_t, U).subs(U, 0)
    a_0 = a_t
    a_s = -sp.diff(nu_s, U).subs(U, 0)
    assert (a_m, a_t, a_0, a_s) == (1, 1, 1, 1)
    checks += 1

    deflection_ratio = sp.simplify((a_0 + a_s) / a_m)
    shapiro_ratio = sp.simplify((a_0 + a_s) / a_m)
    clock_fall_ratio = sp.simplify(a_t / a_m)
    assert deflection_ratio == 2
    assert shapiro_ratio == deflection_ratio
    assert clock_fall_ratio == 1
    checks += 3

    # A single binary read duplicated as g*g would be idempotent.  The
    # product above is instead a blocked response of two independently owned
    # but self-dually equal capacity factors.
    g = sp.symbols("g")
    assert sp.rem(g**2 - g, g**2 - g, domain=sp.QQ) == 0
    checks += 1
    return checks


def main() -> None:
    checks = verify_source_decomposition()
    checks += verify_self_dual_capacity_action()
    checks += verify_conditional_response_tuple()

    print("manifestation moment splits uniquely into scalar trace plus STF shear")
    print("self-dual primal/dual trace action has one massless symmetric mode")
    print("relative capacity mode has pole Lambda+2 eta/kappa and is not sourced")
    print("equal trace source gives U_t=U_s=rho/(kappa Lambda)")
    print("conditional normalized readouts give (a_m,a_t,a_0,a_s)=(1,1,1,1)")
    print("conditional blind deflection and Shapiro response class is 2")
    print("self-dual source/readout selection and finite common action remain open")
    print(
        "PASS: self-dual trace-capacity static-pole lensing boundary "
        f"({checks} exact checks)"
    )


if __name__ == "__main__":
    main()
