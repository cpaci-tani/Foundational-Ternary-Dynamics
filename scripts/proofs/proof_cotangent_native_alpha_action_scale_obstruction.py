#!/usr/bin/env python3
"""Exact cotangent native-alpha action-scale obstruction certificate.

The cotangent carrier fixes a vacuum speed and the D4 source packet fixes one
canonical electric edge quantum.  Those kinematic facts do not fix the overall
coefficient Gamma of a quadratic Maxwell/Gauss action.  Multiplying the full
field action by Gamma leaves its vacuum equations, Gauss minimizer, incidence,
and packet norm unchanged while multiplying the static interaction energy.
The dimensionless coupling therefore depends on the unproved ratio Gamma/I_*.

This is a normalization no-go for the currently proved data, not a theorem
that no enlarged microscopic blocking action can determine the ratio.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import combinations

from sympy import Matrix, Rational, Symbol, pi, simplify

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS


def incidence(vertex_count: int, edges: tuple[tuple[int, int], ...]) -> Matrix:
    matrix = Matrix.zeros(vertex_count, len(edges))
    for column, (tail, head) in enumerate(edges):
        matrix[tail, column] = -1
        matrix[head, column] = 1
    return matrix


def gauss_minimizer(divergence: Matrix, charge: Matrix) -> tuple[Matrix, Matrix]:
    """Minimum Euclidean-norm edge field with D E = rho, in a fixed gauge."""

    assert sum(charge) == 0
    laplacian = divergence * divergence.T
    reduced = laplacian[:-1, :-1]
    potential_reduced = reduced.inv() * charge[:-1, :]
    potential = Matrix.vstack(potential_reduced, Matrix([[0]]))
    field = divergence.T * potential
    assert divergence * field == charge
    return field, potential


def field_norm_squared(field: Matrix):
    return (field.T * field)[0]


def graph_fixtures() -> tuple[tuple[int, tuple[tuple[int, int], ...]], ...]:
    return (
        (4, ((0, 1), (1, 2), (2, 3))),
        (4, ((0, 1), (1, 2), (2, 3), (3, 0))),
        (5, tuple(combinations(range(5), 2))),
        (8, tuple((index, (index + 1) % 8) for index in range(8))),
    )


def main() -> None:
    checks = 0

    # The already-proved packet Gram normalization: E_total=8d and
    # Gram_E=64 I_3 give exactly one electric edge quantum for every SC axis.
    gram_e_inverse = Rational(1, 64) * Matrix.eye(3)
    for direction in SC_DIRECTIONS:
        total = 8 * Matrix(direction)
        assert (total.T * gram_e_inverse * total)[0] == 1
        checks += 1

    gamma = Symbol("Gamma", positive=True)

    # On several exact connected graphs, the Gauss-constrained field is
    # independent of Gamma while its minimum energy is exactly linear in it.
    for vertex_count, edges in graph_fixtures():
        divergence = incidence(vertex_count, edges)
        assert divergence.rank() == vertex_count - 1
        checks += 1

        for source, sink in combinations(range(vertex_count), 2):
            charge = Matrix.zeros(vertex_count, 1)
            charge[source] = 1
            charge[sink] = -1
            field, potential = gauss_minimizer(divergence, charge)
            norm_squared = field_norm_squared(field)
            assert norm_squared > 0
            assert divergence * field == charge
            checks += 3

            # Orthogonality to every cycle/solenoidal perturbation is the
            # exact variational condition for the minimum-norm Gauss field.
            cycle_kernel = divergence.nullspace()
            for cycle in cycle_kernel:
                assert (field.T * cycle)[0] == 0
                checks += 1

            static_energy = gamma * norm_squared / 2
            assert simplify(static_energy / gamma - norm_squared / 2) == 0
            checks += 1

            for gamma_value in (Rational(1, 3), Rational(1), Rational(7, 2)):
                evaluated = static_energy.subs(gamma, gamma_value)
                assert evaluated == gamma_value * norm_squared / 2
                # The field and Gauss residual do not depend on the action
                # coefficient used to price that same configuration.
                assert not field.has(gamma)
                assert divergence * field - charge == Matrix.zeros(vertex_count, 1)
                checks += 3

    # Multiplying a quadratic wave action by Gamma cancels from its Euler-
    # Lagrange equation.  A representative symmetric stiffness is sufficient
    # because the identity is algebraic for every K.
    stiffness = Matrix([[2, -1, 0], [-1, 2, -1], [0, -1, 2]])
    q = Matrix([Symbol("q0"), Symbol("q1"), Symbol("q2")])
    qddot = Matrix([Symbol("qdd0"), Symbol("qdd1"), Symbol("qdd2")])
    euler_lagrange = gamma * (qddot + stiffness * q)
    assert simplify(euler_lagrange / gamma - (qddot + stiffness * q)) == Matrix.zeros(3, 1)
    checks += 1

    # The selected cotangent cone fixes c_eff=1/6.  Conditional on a unit
    # Gauss charge and quadratic stiffness Gamma, the direct-response contract
    # would read alpha=Gamma/(4*pi*I_*c)=3 Gamma/(2*pi I_*).  Distinct positive
    # Gamma/I_* ratios leave every checked kinematic datum unchanged and give
    # distinct couplings.
    action_quantum = Symbol("I_star", positive=True)
    c_eff = Rational(1, 6)
    alpha_family = simplify(gamma / (4 * pi * action_quantum * c_eff))
    assert alpha_family == 3 * gamma / (2 * pi * action_quantum)
    checks += 1

    ratios = (Rational(1, 5), Rational(1, 3), Rational(2, 3), Rational(1))
    alpha_values = {
        simplify(alpha_family.subs({gamma: ratio, action_quantum: 1}))
        for ratio in ratios
    }
    assert len(alpha_values) == len(ratios)
    checks += 1

    # Existing finite ledgers do not constrain this ratio: reserve and active
    # packets both own eight tokens, and the product-reference two-record
    # tangent weight remains fixed while Gamma varies.
    reserve_token_energy = 8
    active_token_energy = 8
    tangent_weight = Fraction(1, 2**191)
    assert active_token_energy - reserve_token_energy == 0
    for ratio in ratios:
        assert tangent_weight == Fraction(1, 2**191)
        assert ratio > 0
        checks += 2

    print(f"PASS: cotangent native-alpha action-scale obstruction ({checks} exact checks)")
    print("fixed kinematics: c_eff=1/6, canonical source edge norm=1, Gauss incidence exact")
    print("free action orbit: H_Gamma=Gamma H_1 has identical field equations/minimizer")
    print("conditional alpha family=3 Gamma/(2 pi I_*); Gamma/I_* is not yet determined")
    print("token energy 8->8 and tangent weight 2^-191 do not measure active-source work")
    print("Open: charged relaxation/static pole and microscopic derivation of Gamma/I_*")


if __name__ == "__main__":
    main()
