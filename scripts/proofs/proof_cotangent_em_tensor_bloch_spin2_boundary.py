#!/usr/bin/env python3
"""Exact first-order spin-2 boundary for the joint cotangent collision.

The selected layer-covariant collision preserves number, E/B, the rank-twelve
C4 FCC-dyad tensor doublet, and a forced phase-blind Eg shear pair.  This
certificate computes the exact co-rotating first spatial moment of all 21 slow
variables over the three cotangent layers.

The electromagnetic block retains its two transverse Maxwell pairs.  Every
row and column involving the twelve tensor modes or the two Eg modes vanishes
at O(k).  After removing the native C4 carrier rotation, the TT tensor envelope
therefore has zero linear group velocity: this selected two-record route does
not produce a massless spin-2 cone.

No physical target fit or numerical eigensolver is used.
"""

from __future__ import annotations

from sympy import I, Matrix, Rational, expand, symbols

import proof_cotangent_em_tensor_equivariant_collision as collision_proof
from proof_cotangent_em_tensor_pair_relation_and_activity_price import combined_value


def main() -> None:
    checks = 0
    collision_proof.main()
    data = collision_proof.CERTIFICATE_DATA
    assert data is not None
    states = data["states"]
    corrections = data["corrections"]
    internal_action = data["internal_action"]
    extra_eg_rows = data["extra_eg_rows"]
    size = len(states)
    assert size == 192
    checks += 2

    internal = Matrix.zeros(size, size)
    for source, target in enumerate(internal_action):
        internal[target, source] = 1

    slow_rows = []
    slow_columns = []
    grams = []
    for layer in range(3):
        known = Matrix.vstack(
            Matrix([[1] * size]),
            *(
                Matrix([[combined_value(state, layer)[component] for state in states]])
                for component in range(18)
            ),
        )
        left = Matrix.vstack(known, extra_eg_rows[layer])
        right = left.T
        gram = left * right
        assert left.shape == (21, 192)
        assert gram.det() != 0
        assert left * corrections[layer] == Matrix.zeros(21, size)
        assert corrections[layer] * right == Matrix.zeros(size, 21)
        slow_rows.append(left)
        slow_columns.append(right)
        grams.append(gram)
        checks += 4

    # Slow-coordinate carrier rotation: identity on number/E/B/Eg and the
    # native quarter turn (Q,P)->(-P,Q) on the twelve tensor coordinates.
    carrier = Matrix.eye(21)
    carrier[7:19, 7:19] = Matrix.zeros(12, 12)
    carrier[7:13, 13:19] = -Matrix.eye(6)
    carrier[13:19, 7:13] = Matrix.eye(6)
    assert carrier**4 == Matrix.eye(21)
    checks += 1

    for layer in range(3):
        next_layer = (layer - 1) % 3
        assert slow_rows[next_layer] * internal == carrier * slow_rows[layer]
        assert internal * slow_columns[layer] == slow_columns[next_layer] * carrier
        assert carrier * grams[layer] == grams[next_layer] * carrier
        checks += 3

    collision_weight = Rational(1, 2**191)
    jacobians = tuple(
        Matrix.eye(size) + collision_weight * correction
        for correction in corrections
    )
    for layer in range(3):
        assert jacobians[layer] * slow_columns[layer] == slow_columns[layer]
        checks += 1

    displacement_axes = tuple(
        Matrix.diag(*[state[0][0][axis] for state in states])
        for axis in range(3)
    )

    layer_moments = []
    for layer in range(3):
        moments = tuple(
            slow_rows[layer]
            * displacement_axes[axis]
            * slow_columns[layer]
            * grams[layer].inv()
            for axis in range(3)
        )
        layer_moments.append(moments)

    expected_layer_ranks = ((2, 2, 2), (4, 4, 4), (0, 0, 0))
    tensor_indices = tuple(range(7, 19))
    for layer in range(3):
        for axis in range(3):
            moment = layer_moments[layer][axis]
            assert moment.rank() == expected_layer_ranks[layer][axis]
            assert moment[:, 7:19] == Matrix.zeros(21, 12)
            assert moment[7:19, :] == Matrix.zeros(12, 21)
            assert moment.extract(tensor_indices, tensor_indices) == Matrix.zeros(12, 12)
            if layer in (1, 2):
                assert moment[:, 19:21] == Matrix.zeros(21, 2)
                assert moment[19:21, :] == Matrix.zeros(2, 21)
            checks += 4

    averaged_axes = tuple(
        -I
        * sum((layer_moments[layer][axis] for layer in range(3)), Matrix.zeros(21, 21))
        / 3
        for axis in range(3)
    )
    kx, ky, kz, eigenvalue = symbols("kx ky kz eigenvalue")
    generator = kx * averaged_axes[0] + ky * averaged_axes[1] + kz * averaged_axes[2]
    wave_number_squared = kx**2 + ky**2 + kz**2
    expected_characteristic = (
        eigenvalue**15
        * (36 * eigenvalue**2 + wave_number_squared + kx**2)
        * (36 * eigenvalue**2 + wave_number_squared + ky**2)
        * (36 * eigenvalue**2 + wave_number_squared + kz**2)
        / 36**3
    )
    assert expand(
        generator.charpoly(eigenvalue).as_expr() - expected_characteristic
    ) == 0
    assert generator[:, 7:19] == Matrix.zeros(21, 12)
    assert generator[7:19, :] == Matrix.zeros(12, 21)
    checks += 3

    # The TT phase-space basis is contained entirely in tensor indices, so its
    # first-order co-rotating generator is identically zero.
    tensor_generator = generator.extract(tensor_indices, tensor_indices)
    assert tensor_generator == Matrix.zeros(12, 12)
    assert tensor_generator.rank() == 0
    checks += 2

    print("joint_slow_space=21: number+E/B+Q/P+phase_blind_Eg")
    print("co_rotating generator retains axis transverse pairs but is generically anisotropic")
    print(
        "characteristic=lambda^15*product_a(36lambda^2+|k|^2+k_a^2)/36^3"
    )
    print("C4_tensor_first_order_rows_columns=0")
    print("phase_blind_Eg mixes only with the scalar-longitudinal sector")
    print("generic propagation has cubic birefringence; Maxwell degeneracy survives only on symmetry directions")
    print("TT_tensor_envelope_group_velocity_at_k0=0")
    print(
        f"PASS: cotangent EM+tensor spin-2 Bloch boundary ({checks} exact checks plus parent)"
    )
    print(
        "Scoped closed negative: this two-record tensor-preserving collision "
        "has no spin-2 cone and spoils generic Maxwell isotropy"
    )
    print(
        "Open: higher-range/staggered tensor transport, static gravity, universal "
        "clock coupling, and lensing"
    )


if __name__ == "__main__":
    main()
