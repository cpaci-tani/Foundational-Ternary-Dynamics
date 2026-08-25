#!/usr/bin/env python3
"""Exact C4-trivial field sector and unique directional-port handoff metric.

The current cotangent E/B readout is independent of the transported C4 phase
address.  Its channel metric must therefore factor through the trivial phase
kernel J_2.  On the two internal-handedness channels the most general
normalized exchange-invariant metric is [[1,a],[a,1]], giving the four-channel
Gram parameters

    (a,b,c) = (a,1,a).

Exact outgoing-port to separated-ray energy conservation requires c=-a, so
a=0.  The unique metric in this registered phase-blind quadratic class is

    (a,b,c) = (0,1,0).

It is positive semidefinite and gives

    H_standing=1/2, P_standing=0,
    H_outgoing=H_free=1, P_outgoing=P_free=r/2.

Thus the port-to-free handoff closes and emitted field work is 1/2 in the
canonical chart.  Including a unit cubic recoil with kinetic coefficient mu
gives source work I*=(Gamma+mu)/2 and field-work fraction Gamma/(Gamma+mu).
The ratio mu/Gamma and a translational Noether/Legendre momentum map remain
undetermined; no alpha value is used or inferred.
"""

from __future__ import annotations

from sympy import Matrix, Rational, kronecker_product, simplify, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_cotangent_framed_plaquette_radiation_release import plaquette_edges
from proof_cotangent_handed_directional_radiation_port import (
    magnetic_direction,
    propagation_direction,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import cross, dot


def add(left, right):
    return tuple(a + b for a, b in zip(left, right))


def scale(factor, vector):
    return tuple(factor * entry for entry in vector)


def gram_energy(channels, gram):
    total = 0
    for left_index, (electric_left, magnetic_left) in enumerate(channels):
        for right_index, (electric_right, magnetic_right) in enumerate(channels):
            total += gram[left_index, right_index] * (
                dot(electric_left, electric_right)
                + dot(magnetic_left, magnetic_right)
            )
    return simplify(total / 64)


def gram_poynting(channels, gram):
    total = (0, 0, 0)
    for left_index, (electric_left, _magnetic_left) in enumerate(channels):
        for right_index, (_electric_right, magnetic_right) in enumerate(channels):
            total = add(
                total,
                scale(
                    gram[left_index, right_index],
                    cross(electric_left, magnetic_right),
                ),
            )
    return tuple(simplify(component / 64) for component in total)


def main() -> None:
    checks = 0
    a = symbols("a", real=True)

    phase_trivial = Matrix([[1, 1], [1, 1]])
    handed_metric = Matrix([[1, a], [a, 1]])
    gram = kronecker_product(phase_trivial, handed_metric)
    expected_gram = Matrix(
        [
            [1, a, 1, a],
            [a, 1, a, 1],
            [1, a, 1, a],
            [a, 1, a, 1],
        ]
    )
    assert gram == expected_gram
    checks += 1

    # In the prior notation this phase-blind factorization fixes b=1,c=a.
    b = gram[0, 2]
    c = gram[0, 3]
    assert b == 1
    assert c == a
    checks += 2

    eigenvectors = (
        Matrix([1, 1, 1, 1]),
        Matrix([1, -1, 1, -1]),
        Matrix([1, 1, -1, -1]),
        Matrix([1, -1, -1, 1]),
    )
    eigenvalues = (2 * (1 + a), 2 * (1 - a), 0, 0)
    for vector, eigenvalue in zip(eigenvectors, eigenvalues):
        assert gram * vector == eigenvalue * vector
        checks += 1

    # Port/free handoff conservation from the predecessor is c=-a.  Together
    # with c=a from exact phase blindness this uniquely fixes a=c=0.
    assert simplify(c - (-a)) == 2 * a
    selected_a = Rational(0)
    selected = gram.subs(a, selected_a)
    assert selected == Matrix(
        [
            [1, 0, 1, 0],
            [0, 1, 0, 1],
            [1, 0, 1, 0],
            [0, 1, 0, 1],
        ]
    )
    assert selected.eigenvals() == {Rational(2): 2, Rational(0): 2}
    checks += 3

    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    # Evaluate every edge in every signed-cubic directional frame. Channel
    # order is (h+,p),(h-,p),(h+,p+2),(h-,p+2).
    for frame in frames:
        for chirality in (-1, 1):
            propagation = propagation_direction(frame, chirality)
            standing_energy = Rational(0)
            outgoing_energy = Rational(0)
            standing_momentum = (0, 0, 0)
            outgoing_momentum = (0, 0, 0)
            for _tail, electric in plaquette_edges(frame):
                magnetic = magnetic_direction(propagation, electric)
                standing_channels = (
                    (electric, magnetic),
                    (electric, magnetic),
                    (electric, scale(-1, magnetic)),
                    (electric, scale(-1, magnetic)),
                )
                outgoing_channels = (
                    (electric, magnetic),
                    (electric, magnetic),
                    (electric, magnetic),
                    (electric, magnetic),
                )
                standing_energy += gram_energy(standing_channels, selected)
                outgoing_energy += gram_energy(outgoing_channels, selected)
                standing_momentum = add(
                    standing_momentum,
                    gram_poynting(standing_channels, selected),
                )
                outgoing_momentum = add(
                    outgoing_momentum,
                    gram_poynting(outgoing_channels, selected),
                )

            free_energy = Rational(1)
            free_momentum = scale(Rational(1, 2), propagation)
            assert standing_energy == Rational(1, 2)
            assert outgoing_energy == free_energy == 1
            assert standing_momentum == (0, 0, 0)
            assert outgoing_momentum == free_momentum
            assert outgoing_energy - standing_energy == Rational(1, 2)
            checks += 5

    # Revised source/field/recoil energy partition. Momentum matching is not
    # asserted because the normalized Poynting moment lacks a translational
    # Noether/Legendre scale and the carrier speed conversion.
    gamma, mu = symbols("Gamma mu", positive=True)
    field_work = gamma / 2
    recoil_work = mu / 2
    source_work = simplify(field_work + recoil_work)
    work_fraction = simplify(field_work / source_work)
    assert source_work == (gamma + mu) / 2
    assert work_fraction == gamma / (gamma + mu)
    assert simplify(work_fraction - (1 - mu / (2 * source_work))) == 0
    checks += 3

    print("phase-blind C4 field sector fixes b=1 and c=a")
    print("exact handoff conservation c=-a uniquely gives (a,b,c)=(0,1,0)")
    print("selected metric = trivial phase projector tensor resolved handedness")
    print("standing: H=1/2, P=0; outgoing/free: H=1, P=r/2")
    print("canonical emitted field work=1/2 and port-to-free handoff defect=0")
    print("with unit recoil: I*=(Gamma+mu)/2, field-work fraction=Gamma/(Gamma+mu)")
    print(
        f"PASS: C4-trivial unique directional-port handoff metric ({checks} exact checks)"
    )
    print(
        "Boundary: derive mu/Gamma, canonical translational momentum, Maxwell mode "
        "reduction, and the common action; no alpha/root comparison is licensed"
    )


if __name__ == "__main__":
    main()
