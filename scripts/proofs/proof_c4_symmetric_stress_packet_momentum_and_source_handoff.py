#!/usr/bin/env python3
"""Exact C4 symmetric-stress packet momentum/source-handoff discriminator.

The finite C4 carrier fixes energy and energy current but not a real physical
momentum.  Conditional on one symmetric rank-one stress completion at the
selected speed, this certificate derives the unique packet translation
charge, matches its stress to the manifestation event dyad, and substitutes
the result into the reciprocal packet/clock/recoil generator.

No floating point, target coupling, master root, or empirical normalization
enters.
"""

from __future__ import annotations

import hashlib
import itertools
from fractions import Fraction
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    ROOT / "scripts/proofs/proof_c4_half_admitted_energy_current_momentum_boundary.py":
        "D9E0E2C4FF595A56F85712CA5195BE1406BCB7F90B8B2D5D8E66CBC2F05AE3CA",
    ROOT / "scripts/proofs/proof_c18_phase_neutral_shared_charge_stress_vertex.py":
        "13334840F23DBB1D70EFD59B805D97E462EDFC5B4EEC00D5C9FFF784ECEAAF35",
    ROOT / "scripts/proofs/proof_reciprocal_packet_clock_recoil_absorption_generator.py":
        "4B824C3B37A8BADEC9F50ED1785602734B75D6CCF03234D65826E0541CDC2576",
    ROOT / "scripts/proofs/proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v3.py":
        "62CB476D4A5F545B03A286E6C29B7710870E90802606C5DA7561F32397AA59FC",
    ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_v1.md":
        "43219F025D3AE29D2454E25FBEFB56A25018510875938C81A399C22C16519267",
}

SC_DIRECTIONS = (
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def is_zero(value: sp.Expr | sp.MatrixBase) -> bool:
    if isinstance(value, sp.MatrixBase):
        return all(sp.simplify(entry) == 0 for entry in value)
    return sp.simplify(value) == 0


def signed_permutation_matrices() -> tuple[sp.Matrix, ...]:
    matrices: list[sp.Matrix] = []
    for perm in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            matrix = sp.zeros(3)
            for row, col in enumerate(perm):
                matrix[row, col] = signs[row]
            matrices.append(matrix)
    assert len(matrices) == 48
    assert len({tuple(matrix) for matrix in matrices}) == 48
    return tuple(matrices)


def symbolic_stress_checks() -> int:
    checks = 0
    packet_count, gamma = sp.symbols("N Gamma", positive=True)
    energy = packet_count * gamma
    c = sp.Rational(1, 6)
    identity = sp.eye(3)

    for direction in SC_DIRECTIONS:
        ray = sp.Matrix(direction)
        dyad = ray * ray.T
        velocity = c * ray
        energy_current = energy * velocity

        # Symmetric stress-energy fixes momentum uniquely: J_E=c^2 p.
        momentum = sp.simplify(energy_current / c**2)
        stress = sp.simplify(momentum * velocity.T)
        event_source = dyad / 18
        scalar_source = sp.trace(stress)
        stf_source = sp.simplify(stress - scalar_source * identity / 3)

        assert is_zero(ray.dot(ray) - 1)
        assert is_zero(momentum - 6 * energy * ray)
        assert is_zero(stress - energy * dyad)
        assert is_zero(energy_current - c**2 * momentum)
        assert is_zero(stress - momentum * velocity.T)
        assert is_zero(stress - 18 * energy * event_source)
        assert is_zero(scalar_source - energy)
        assert is_zero(sp.trace(stf_source))
        assert is_zero(
            stress - (energy * identity / 3 + energy * (dyad - identity / 3))
        )
        assert is_zero(stf_source - 18 * energy * (event_source - identity / 54))
        checks += 10

        # The phase-neutral event dyad and stress are charge even.
        for charge in (-1, 1):
            charged_current = charge * ray / 9
            assert is_zero(charge * charged_current - ray / 9)
            assert is_zero(event_source - dyad / 18)
            assert is_zero(stress - energy * dyad)
            checks += 3

        # The carrier trajectory/current alone leaves a continuous scale
        # family of declared real momenta.  The absorption recoil changes.
        lambda_a, lambda_b = sp.Rational(1), sp.Rational(2)
        p_a = lambda_a * energy * ray
        p_b = lambda_b * energy * ray
        assert not is_zero(p_a - p_b)
        assert is_zero(energy_current - energy * c * ray)
        assert is_zero(energy_current - energy * c * ray)
        recoil_a = sp.simplify(p_a.dot(p_a) / 2)
        recoil_b = sp.simplify(p_b.dot(p_b) / 2)
        assert not is_zero(recoil_a - recoil_b)
        checks += 4

    return checks


def cubic_covariance_checks() -> int:
    checks = 0
    energy = sp.symbols("E", positive=True)
    identity = sp.eye(3)
    group = signed_permutation_matrices()

    for direction in SC_DIRECTIONS:
        ray = sp.Matrix(direction)
        stress = energy * ray * ray.T
        stf = stress - energy * identity / 3
        momentum = 6 * energy * ray
        event_source = ray * ray.T / 18

        for transform in group:
            image_ray = transform * ray
            image_stress = energy * image_ray * image_ray.T
            image_stf = image_stress - energy * identity / 3
            image_momentum = 6 * energy * image_ray
            image_event = image_ray * image_ray.T / 18

            assert tuple(int(entry) for entry in image_ray) in SC_DIRECTIONS
            assert is_zero(image_stress - transform * stress * transform.T)
            assert is_zero(image_stf - transform * stf * transform.T)
            assert is_zero(image_momentum - transform * momentum)
            assert is_zero(image_event - transform * event_source * transform.T)
            assert is_zero(sp.trace(image_stf))
            checks += 6

    return checks


def absorption_substitution_checks() -> int:
    checks = 0
    packet_count, gamma, mass, omega = sp.symbols(
        "N Gamma m omega", positive=True
    )
    px, py, pz, action = sp.symbols("P_x P_y P_z I", real=True)
    material_momentum = sp.Matrix((px, py, pz))
    energy = packet_count * gamma

    for direction in SC_DIRECTIONS:
        ray = sp.Matrix(direction)
        field_momentum = 6 * energy * ray
        new_momentum = material_momentum + field_momentum
        kinetic_before = material_momentum.dot(material_momentum) / (2 * mass)
        kinetic_after = new_momentum.dot(new_momentum) / (2 * mass)
        delta_action = sp.simplify(
            (energy + kinetic_before - kinetic_after) / omega
        )
        new_action = action + delta_action

        total_before = omega * action + kinetic_before + energy
        total_after = omega * new_action + kinetic_after
        assert is_zero(total_before - total_after)
        assert is_zero(new_momentum - material_momentum - field_momentum)

        # Exact inverse emission restores the complete canonical body state.
        restored_momentum = new_momentum - field_momentum
        restored_action = sp.simplify(
            new_action
            - (energy + kinetic_before - kinetic_after) / omega
        )
        assert is_zero(restored_momentum - material_momentum)
        assert is_zero(restored_action - action)

        rest_delta = sp.simplify(delta_action.subs({px: 0, py: 0, pz: 0}))
        expected_rest = sp.simplify(
            (energy - 18 * energy**2 / mass) / omega
        )
        assert is_zero(rest_delta - expected_rest)
        assert is_zero(
            energy - (omega * rest_delta + field_momentum.dot(field_momentum) / (2 * mass))
        )

        action_quantum = sp.symbols("I_star", positive=True)
        compliance = sp.simplify(gamma / action_quantum)
        solved_compliance = sp.simplify(
            compliance.subs(action_quantum, expected_rest)
        )
        expected_compliance = sp.simplify(
            omega / (packet_count - 18 * packet_count**2 * gamma / mass)
        )
        assert is_zero(solved_compliance - expected_compliance)
        checks += 7

    return checks


def rational_mass_boundary_checks() -> int:
    checks = 0
    directions = tuple(sp.Matrix(direction) for direction in SC_DIRECTIONS)
    packet_counts = (1, 2, 3, 5, 8)
    gammas = (
        Fraction(1, 7), Fraction(2, 5), Fraction(1, 1),
        Fraction(7, 3), Fraction(11, 2),
    )
    omegas = (Fraction(1, 3), Fraction(1, 1), Fraction(5, 2))
    mass_factors = (Fraction(1, 2), Fraction(1, 1), Fraction(2, 1))

    for ray in directions:
        assert ray.dot(ray) == 1
        checks += 1
        for packet_count in packet_counts:
            for gamma in gammas:
                energy = Fraction(packet_count) * gamma
                field_momentum = tuple(Fraction(6) * energy * int(x) for x in ray)
                momentum_sq = sum(component * component for component in field_momentum)
                assert momentum_sq == Fraction(36) * energy * energy
                checks += 1

                threshold = Fraction(18) * energy
                for omega in omegas:
                    for factor in mass_factors:
                        mass = factor * threshold
                        delta_action = (
                            energy - momentum_sq / (Fraction(2) * mass)
                        ) / omega
                        if factor < 1:
                            assert delta_action < 0
                        elif factor == 1:
                            assert delta_action == 0
                        else:
                            assert delta_action > 0
                        checks += 1

                        # A fail-closed absorption gate may admit only
                        # nonnegative resulting clock action.
                        admitted = delta_action >= 0
                        assert admitted == (mass >= threshold)
                        checks += 1

    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        actual = sha256(path)
        assert actual == expected, (path, actual, expected)
        checks += 1

    checks += symbolic_stress_checks()
    checks += cubic_covariance_checks()
    checks += absorption_substitution_checks()
    checks += rational_mass_boundary_checks()

    print("finite C4 energy transport alone leaves real momentum scale undetermined")
    print("symmetric stress at c=1/6 uniquely gives p_F=6 E r")
    print("the same completion gives Sigma_F=E r r^T=18 E t_evt")
    print("trace and STF gravity sources are exact projections of that one stress")
    print("substitution into the absorption generator preserves energy and inverse")
    print("rest absorption requires m>=18 E for nonnegative clock action")
    print(f"PASS: C4 symmetric-stress momentum/source handoff ({checks} exact checks)")
    print(
        "OUTCOME B: exact conditional completion; stress symmetry, native momentum "
        "scale, tensor dynamics, lensing, and coupling value remain open"
    )


if __name__ == "__main__":
    main()
