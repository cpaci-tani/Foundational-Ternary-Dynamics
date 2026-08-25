#!/usr/bin/env python3
"""Exact three-tick vacuum-Maxwell pass for the cotangent collision family.

This certificate composes the selected layer-covariant pair collisions with
shared-edge streaming and the internal flag/C4 update.  The exact symmetric
independent-binary product reference gives

    J_q = I + N_q / 2^191.

The collision family maps its seven-dimensional slow spaces covariantly from
q to q-1.  Direct differentiation of the three-tick Floquet product proves
that its per-tick k-linear reduced generator is the previously derived average
of the three layer moments.  Hence the finite action has two transverse
Maxwell polarization pairs at first order.

The scalar--longitudinal block does not preserve any k-independent local Gauss
law div E = kappa rho.  Only the vacuum subspace rho=div E=div B=0 is invariant
at this order.  Source/Gauss closure therefore remains open.
"""

from __future__ import annotations

from sympy import I, Matrix, Rational, expand, symbols

import proof_global_c3_cotangent_layer_equivariant_collision as collision_proof
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value


def main() -> None:
    checks = 0
    collision_proof.main()
    data = collision_proof.CERTIFICATE_DATA
    assert data is not None
    states = data["states"]
    corrections = data["corrections"]
    internal_action = data["internal_action"]
    size = len(states)
    assert size == 192
    checks += 2

    internal = Matrix.zeros(size, size)
    for source, target in enumerate(internal_action):
        internal[target, source] = 1
    assert internal**12 == Matrix.eye(size)
    checks += 1

    left_rows = []
    right_columns = []
    grams = []
    for layer in range(3):
        left = Matrix.vstack(
            Matrix([[1] * size]),
            *(
                Matrix([[layer_value(state, layer)[component] for state in states]])
                for component in range(6)
            ),
        )
        right = left.T
        gram = left * right
        assert gram == Matrix.diag(192, 64, 64, 64, 64, 64, 64)
        assert left * corrections[layer] == Matrix.zeros(7, size)
        assert corrections[layer] * right == Matrix.zeros(size, 7)
        left_rows.append(left)
        right_columns.append(right)
        grams.append(gram)
        checks += 3

    for layer in range(3):
        next_layer = (layer - 1) % 3
        assert left_rows[next_layer] * internal == left_rows[layer]
        assert internal * right_columns[layer] == right_columns[next_layer]
        checks += 2

    # Exact product-reference weight for an event with exactly two occupied
    # channels among 192 independent binary channels at p=1/2.
    collision_weight = Rational(1, 2**191)
    jacobians = tuple(
        Matrix.eye(size) + collision_weight * correction
        for correction in corrections
    )
    for layer in range(3):
        next_layer = (layer - 1) % 3
        assert left_rows[layer] * jacobians[layer] == left_rows[layer]
        assert jacobians[layer] * right_columns[layer] == right_columns[layer]
        assert internal * jacobians[layer] == jacobians[next_layer] * internal
        checks += 3

    displacement_axes = tuple(
        Matrix.diag(*[state[0][0][axis] for state in states])
        for axis in range(3)
    )

    # Start at q=0.  Successive ticks use q=(0,2,1) and return to q=0.
    sequence = (0, 2, 1)
    baseline_maps = tuple(internal * jacobians[layer] for layer in range(3))

    baseline_slow = right_columns[0]
    for layer in sequence:
        baseline_slow = baseline_maps[layer] * baseline_slow
    assert baseline_slow == right_columns[0]
    checks += 1

    reduced_three_tick = []
    reduced_layer_moments = []
    for axis in range(3):
        layer_moments = tuple(
            left_rows[layer]
            * displacement_axes[axis]
            * right_columns[layer]
            * grams[layer].inv()
            for layer in range(3)
        )
        reduced_layer_moments.append(layer_moments)

        derivative = Matrix.zeros(7, 7)
        for insertion in range(3):
            tangent = right_columns[0]
            for step, layer in enumerate(sequence):
                if step == insertion:
                    tangent = (
                        -I
                        * internal
                        * displacement_axes[axis]
                        * jacobians[layer]
                        * tangent
                    )
                else:
                    tangent = baseline_maps[layer] * tangent
            derivative += left_rows[0] * tangent * grams[0].inv()
        expected = -I * sum(layer_moments, Matrix.zeros(7, 7))
        assert derivative == expected
        reduced_three_tick.append(derivative)
        checks += 1

    per_tick_axes = tuple(matrix / 3 for matrix in reduced_three_tick)
    kx, ky, kz, eigenvalue = symbols("kx ky kz eigenvalue")
    generator = kx * per_tick_axes[0] + ky * per_tick_axes[1] + kz * per_tick_axes[2]
    wave_number_squared = kx**2 + ky**2 + kz**2
    expected_characteristic = (
        eigenvalue
        * (eigenvalue**2 + wave_number_squared / 27)
        * (eigenvalue**2 + wave_number_squared / 36) ** 2
    )
    assert expand(
        generator.charpoly(eigenvalue).as_expr() - expected_characteristic
    ) == 0
    checks += 1

    transverse_indices = (1, 2, 4, 5)
    transverse_z = per_tick_axes[2].extract(
        transverse_indices, transverse_indices
    )
    assert expand(
        transverse_z.charpoly(eigenvalue).as_expr()
        - (eigenvalue**2 + Rational(1, 36)) ** 2
    ) == 0
    checks += 1

    # The scalar pair is ordered as (rho, E_parallel).  It has two
    # k-independent *acoustic* characteristic graphs E_parallel=eta*rho,
    # eta^2=1/3.  That is not a Gauss graph: the derivative constraint is
    # k E_parallel=kappa rho.  Test the latter with a polynomial spanning
    # vector (rho,E_parallel)=(k,kappa), avoiding division by k.  Multiplying
    # the real streaming moment by -I does not change invariant subspaces.
    kappa, eta, wave_number = symbols("kappa eta wave_number")
    scalar_transport = Matrix(
        [
            [0, wave_number / 3],
            [wave_number / 9, 0],
        ]
    )
    acoustic_relation_residual = (
        Matrix([[-eta, 1]])
        * scalar_transport
        * Matrix([[1], [eta]])
    )[0]
    assert expand(
        acoustic_relation_residual
        - wave_number * (Rational(1, 9) - eta**2 / 3)
    ) == 0

    gauss_relation_residual = (
        Matrix([[-kappa, wave_number]])
        * scalar_transport
        * Matrix([[wave_number], [kappa]])
    )[0]
    assert expand(
        gauss_relation_residual
        - wave_number
        * (wave_number**2 / 9 - kappa**2 / 3)
    ) == 0
    assert (-I * scalar_transport).det() != 0
    checks += 3

    print("product_reference_collision_weight=1/2^191")
    print("three_tick_slow_derivative_equals_exact_sum_of_layer_moments")
    print(
        "per_tick_characteristic="
        "lambda*(lambda^2+|k|^2/27)*(lambda^2+|k|^2/36)^2"
    )
    print("vacuum_transverse_sector=two_Maxwell_pairs_speed_1/6")
    print("vacuum_constraints_rho=divE=divB=0_are_first_order_invariant")
    print("scalar_acoustic_graphs=E_parallel=eta*rho with eta^2=1/3")
    print("local_charged_Gauss_graph_requires_kappa^2=|k|^2/3: not local constant")
    print(
        "PASS: global-C3 cotangent full-tick vacuum Maxwell "
        f"({checks} exact checks plus parent certificate)"
    )
    print("Open: replace scalar acoustic pair by local Gauss/charge continuity")


if __name__ == "__main__":
    main()
