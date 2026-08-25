#!/usr/bin/env python3
"""Exact global-C3 cotangent-layer Hodge/Maxwell target.

The shared-edge flag already carries an oriented polar triad and its axial
partner.  Instead of averaging all three legs into one BCC ray, use the global
clock modulo three as a cotangent layer q.  At layer q the field readout owns
one polar leg P_q and its perpendicular axial leg A_q.  The flag update is
paired with q -> q-1, making the physical readout invariant across ticks
without adding a per-record controller.

The three exact streaming moments split into:
  q=0: scalar--longitudinal electric transport,
  q=1: antisymmetric transverse edge--face curl,
  q=2: storage.
Their three-tick Floquet average contains two transverse Maxwell polarization
pairs with speed 1/6, plus one extra scalar--longitudinal pair and one magnetic
longitudinal zero mode.  This is a kinematic target; a finite layer-covariant
collision, Gauss/continuity constraint, work ledger, and sources remain open.

No physical coefficient, measured target, or numerical eigensolver is used.
"""

from __future__ import annotations

from sympy import I, Matrix, Rational, expand, sqrt, symbols

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_moore_bond_capacity_type_census import (
    determinant_3,
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross
from proof_shared_edge_hodge_flag_bcc_propagation import (
    scale,
    transform_flag,
    update_flag,
)


def add3(*vectors):
    return tuple(sum(vector[component] for vector in vectors) for component in range(3))


def polar_legs(flag):
    tangent, normal, handedness = flag
    return tangent, scale(handedness, normal), cross(tangent, normal)


def axial_legs(flag):
    tangent, normal, handedness = flag
    third = cross(tangent, normal)
    return normal, scale(handedness, third), scale(handedness, tangent)


def layer_value(state, layer: int) -> tuple[int, ...]:
    flag, _phase = state
    return polar_legs(flag)[layer] + axial_legs(flag)[layer]


def internal_tick(state):
    flag, phase = state
    return update_flag(flag), (phase + 1) % 4


def levi_civita(left: int, middle: int, right: int) -> int:
    if len({left, middle, right}) < 3:
        return 0
    return 1 if (left, middle, right) in ((0, 1, 2), (1, 2, 0), (2, 0, 1)) else -1


def main() -> None:
    checks = 0
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    group = tuple(signed_permutation_matrices())
    assert len(states) == 192
    checks += 1

    for state in states:
        for layer in range(3):
            value = layer_value(state, layer)
            electric = value[:3]
            magnetic = value[3:]
            assert sum(entry * entry for entry in electric) == 1
            assert sum(entry * entry for entry in magnetic) == 1
            assert sum(a * b for a, b in zip(electric, magnetic)) == 0
            assert layer_value(internal_tick(state), (layer - 1) % 3) == value
            for matrix in group:
                transformed = (transform_flag(matrix, state[0]), state[1])
                transformed_value = layer_value(transformed, layer)
                determinant = determinant_3(matrix)
                expected_electric = tuple(matrix_vector(matrix, electric))
                expected_magnetic = tuple(
                    determinant * entry
                    for entry in matrix_vector(matrix, magnetic)
                )
                assert transformed_value == expected_electric + expected_magnetic
                checks += 1
            checks += 4

    internal_image = tuple(state_index[internal_tick(state)] for state in states)
    internal = Matrix.zeros(192, 192)
    for source, target in enumerate(internal_image):
        internal[target, source] = 1
    assert internal**12 == Matrix.eye(192)
    checks += 1

    layer_rows = []
    for layer in range(3):
        rows = Matrix.vstack(
            Matrix([[1] * 192]),
            *(
                Matrix([[layer_value(state, layer)[component] for state in states]])
                for component in range(6)
            ),
        )
        assert rows * rows.T == Matrix.diag(192, 64, 64, 64, 64, 64, 64)
        layer_rows.append(rows)
        checks += 1
    for layer in range(3):
        assert layer_rows[(layer - 1) % 3] * internal == layer_rows[layer]
        checks += 1

    reduced = []
    for layer in range(3):
        left = layer_rows[layer]
        gram_inverse = (left * left.T).inv()
        axes = []
        for axis in range(3):
            displacement = Matrix.diag(
                *[state[0][0][axis] for state in states]
            )
            axes.append(left * displacement * left.T * gram_inverse)
        reduced.append(tuple(axes))

    for axis in range(3):
        expected_edge = Matrix.zeros(7, 7)
        expected_edge[0, 1 + axis] = 1
        expected_edge[1 + axis, 0] = Rational(1, 3)
        assert reduced[0][axis] == expected_edge

        expected_curl = Matrix.zeros(7, 7)
        for electric in range(3):
            for magnetic in range(3):
                entry = -Rational(1, 2) * levi_civita(electric, axis, magnetic)
                expected_curl[1 + electric, 4 + magnetic] = entry
                expected_curl[4 + magnetic, 1 + electric] = entry
        assert reduced[1][axis] == expected_curl
        assert reduced[2][axis] == Matrix.zeros(7, 7)
        checks += 3

    averaged_axes = tuple(
        sum((reduced[layer][axis] for layer in range(3)), Matrix.zeros(7, 7)) / 3
        for axis in range(3)
    )
    kx, ky, kz, eigenvalue = symbols("kx ky kz eigenvalue")
    averaged = kx * averaged_axes[0] + ky * averaged_axes[1] + kz * averaged_axes[2]
    wave_number_squared = kx**2 + ky**2 + kz**2
    generator = -I * averaged
    expected_characteristic = (
        eigenvalue
        * (eigenvalue**2 + wave_number_squared / 27)
        * (eigenvalue**2 + wave_number_squared / 36) ** 2
    )
    assert expand(
        generator.charpoly(eigenvalue).as_expr() - expected_characteristic
    ) == 0
    checks += 1

    # Freeze the z-axis transverse Maxwell block and its two equal-speed
    # polarization pairs.
    transverse_indices = (1, 2, 4, 5)
    transverse = averaged_axes[2].extract(transverse_indices, transverse_indices)
    expected_transverse = Matrix(
        [
            [0, 0, 0, Rational(1, 6)],
            [0, 0, Rational(-1, 6), 0],
            [0, Rational(-1, 6), 0, 0],
            [Rational(1, 6), 0, 0, 0],
        ]
    )
    assert transverse == expected_transverse
    assert expand(
        (-I * transverse).charpoly(eigenvalue).as_expr()
        - (eigenvalue**2 + Rational(1, 36)) ** 2
    ) == 0
    checks += 2

    scalar_speed = 1 / (3 * sqrt(3))
    transverse_speed = Rational(1, 6)
    print("global_cotangent_layers=3, per_layer_channels=192")
    print("layer_0=scalar_longitudinal_edge_transport")
    print("layer_1=antisymmetric_edge_face_curl")
    print("layer_2=storage")
    print(
        "three_tick_characteristic="
        "lambda*(lambda^2+|k|^2/27)*(lambda^2+|k|^2/36)^2"
    )
    print(f"scalar_longitudinal_speed={scalar_speed}")
    print(f"two_transverse_polarization_speeds={transverse_speed}")
    print(
        f"PASS: global-C3 cotangent-layer Hodge/Maxwell target ({checks} exact checks)"
    )
    print(
        "Open: layer-covariant finite collision, vacuum constraint projector, "
        "Gauss/charge continuity, work, and source closure"
    )


if __name__ == "__main__":
    main()
