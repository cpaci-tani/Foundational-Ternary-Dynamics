#!/usr/bin/env python3
"""Exact mean-field linearization of the selected C18 two-record collision.

The local reference ensemble is the target-blind product measure assigning
probability 1/5 to blank and each C4 phase on every directed channel.  The
collision is the exact-two-occupancy phase-complete involution certified in
proof_c18_two_record_momentum_sector_census.py.

The script derives the 48x48 FCC occupied-phase tangent Jacobian exactly.  SC
channels are spectators under this minimum collision and are reported as an
explicit boundary rather than folded into the kernel.
"""

from __future__ import annotations

from sympy import Matrix, Rational

from proof_c18_two_record_momentum_sector_census import (
    FCC_DIRECTIONS,
    PhasePair,
    build_sectors,
    canonical_pair,
    canonical_phase_pair,
    phase_complete_collision,
)
from proof_c18_equivariant_single_record_collision_no_go import (
    FCC_DIRECTIONS as FCC_SHELL,
    SC_DIRECTIONS,
)


PHASES = tuple(range(4))
FCC_INDEX = {direction: index for index, direction in enumerate(FCC_DIRECTIONS)}
TANGENT_INDEX = {
    (direction, phase): 4 * FCC_INDEX[direction] + phase
    for direction in FCC_DIRECTIONS
    for phase in PHASES
}


def phase_states() -> tuple[PhasePair, ...]:
    return tuple(
        canonical_phase_pair((left, left_phase), (right, right_phase))
        for left_index, left in enumerate(FCC_DIRECTIONS)
        for right in FCC_DIRECTIONS[left_index + 1 :]
        for left_phase in PHASES
        for right_phase in PHASES
    )


def occupied_phase_vector(state: PhasePair) -> tuple[int, ...]:
    vector = [0] * 48
    for direction, phase in state:
        vector[TANGENT_INDEX[(direction, phase)]] = 1
    return tuple(vector)


def reduced_score(state: PhasePair, direction, phase: int) -> int:
    """Derivative score for p_phase with p_blank=1-sum(p_phase)."""

    phase_by_direction = {record_direction: record_phase for record_direction, record_phase in state}
    if direction not in phase_by_direction:
        return -1
    return int(phase_by_direction[direction] == phase)


def tangent_mode(spatial_function, phase_weights: tuple[int, int, int, int]) -> Matrix:
    return Matrix(
        [
            spatial_function(direction) * phase_weights[phase]
            for direction, phase in TANGENT_INDEX
        ]
    )


def exact_eigenvalue(matrix: Matrix, vector: Matrix) -> Rational | None:
    image = matrix * vector
    pivot = next((index for index, entry in enumerate(vector) if entry != 0), None)
    if pivot is None:
        raise ValueError("zero vector has no eigenvalue")
    value = Rational(image[pivot], vector[pivot])
    return value if image == value * vector else None


def main() -> None:
    checks = 0
    all_directions = SC_DIRECTIONS + FCC_SHELL
    sectors = build_sectors(
        tuple(
            canonical_pair(left, right)
            for left_index, left in enumerate(all_directions)
            for right in all_directions[left_index + 1 :]
        )
    )

    states = phase_states()
    assert len(states) == 66 * 16
    changed = tuple(
        state for state in states if phase_complete_collision(state, sectors) != state
    )
    assert len(changed) == 432
    checks += 2

    correction = [[0 for _ in range(48)] for _ in range(48)]
    for state in changed:
        outgoing = phase_complete_collision(state, sectors)
        before = occupied_phase_vector(state)
        after = occupied_phase_vector(outgoing)
        delta = tuple(after[index] - before[index] for index in range(48))
        for row, delta_entry in enumerate(delta):
            if delta_entry == 0:
                continue
            for direction in FCC_DIRECTIONS:
                for phase in PHASES:
                    column = TANGENT_INDEX[(direction, phase)]
                    correction[row][column] += delta_entry * reduced_score(
                        state, direction, phase
                    )

    n_matrix = Matrix(correction)
    assert n_matrix == n_matrix.T
    checks += 1

    exact_spectrum = {-180: 3, -160: 3, -30: 8, -28: 9, -16: 9, -8: 9, 0: 7}
    annihilator = Matrix.eye(48)
    for eigenvalue in exact_spectrum:
        annihilator = annihilator * (n_matrix - eigenvalue * Matrix.eye(48))
    assert annihilator == Matrix.zeros(48, 48)
    power = Matrix.eye(48)
    for exponent in range(7):
        assert power.trace() == sum(
            multiplicity * eigenvalue**exponent
            for eigenvalue, multiplicity in exact_spectrum.items()
        )
        power = power * n_matrix
        checks += 1
    # A square-free annihilator makes the integer matrix diagonalizable; the
    # seven exact moments then fix the multiplicities by the Vandermonde
    # system on the seven distinct roots.
    checks += 1
    assert sum(exact_spectrum.values()) == 48
    assert sum(
        multiplicity for eigenvalue, multiplicity in exact_spectrum.items() if eigenvalue != 0
    ) == 41
    assert min(exact_spectrum) > -(5**17)
    checks += 3

    # Exact collision Jacobian on the normalized occupied-phase tangent chart.
    identity = Matrix.eye(48)
    jacobian = identity + Rational(1, 5**17) * n_matrix
    assert jacobian.shape == (48, 48)
    checks += 1

    # Four phase-number left invariants and three directed-momentum left
    # invariants must annihilate the collision correction exactly.
    phase_invariants = []
    for phase in PHASES:
        row = Matrix(
            [[int(candidate_phase == phase) for _direction, candidate_phase in TANGENT_INDEX]]
        )
        assert row * n_matrix == Matrix.zeros(1, 48)
        phase_invariants.append(row)
        checks += 1

    momentum_invariants = []
    for component in range(3):
        row = Matrix(
            [[direction[component] for direction, _phase in TANGENT_INDEX]]
        )
        assert row * n_matrix == Matrix.zeros(1, 48)
        momentum_invariants.append(row)
        checks += 1

    invariant_stack = Matrix.vstack(*(phase_invariants + momentum_invariants))
    assert invariant_stack.rank() == 7
    checks += 1

    # Global C4 covariance makes the four phase species spectrally equivalent.
    # Verify the correction commutes with the phase-cycle permutation.
    phase_cycle = Matrix.zeros(48, 48)
    for direction in FCC_DIRECTIONS:
        for phase in PHASES:
            phase_cycle[
                TANGENT_INDEX[(direction, (phase + 1) % 4)],
                TANGENT_INDEX[(direction, phase)],
            ] = 1
    assert n_matrix * phase_cycle == phase_cycle * n_matrix
    checks += 1

    # The phase-summed FCC capacity shear is not gapless.  Its Eg and T2g
    # components are exact collision eigenmodes with distinct relaxation
    # rates, so the selected collision retains cubic shear anisotropy.
    phase_sum = (1, 1, 1, 1)
    eg_modes = (
        tangent_mode(lambda d: d[0] ** 2 - d[1] ** 2, phase_sum),
        tangent_mode(lambda d: 2 * d[2] ** 2 - d[0] ** 2 - d[1] ** 2, phase_sum),
    )
    t2g_modes = tuple(
        tangent_mode(lambda d, left=left, right=right: d[left] * d[right], phase_sum)
        for left, right in ((0, 1), (0, 2), (1, 2))
    )
    eg_eigenvalues = tuple(exact_eigenvalue(n_matrix, mode) for mode in eg_modes)
    t2g_eigenvalues = tuple(exact_eigenvalue(n_matrix, mode) for mode in t2g_modes)
    assert eg_eigenvalues == (-30, -30)
    assert t2g_eigenvalues == (-160, -160, -160)
    checks += 5

    # Resolve the simplest phase-character-weighted vector moments.  The
    # phase-blind vector is total momentum and is protected.  The three
    # nontrivial real C4 character charts test whether the collision protects
    # an electromagnetic-like oriented phase current.
    phase_characters = {
        "blind": (1, 1, 1, 1),
        "real": (1, 0, -1, 0),
        "imag": (0, 1, 0, -1),
        "alternating": (1, -1, 1, -1),
    }
    vector_character_eigenvalues = {}
    for name, weights in phase_characters.items():
        values = tuple(
            exact_eigenvalue(
                n_matrix,
                tangent_mode(lambda d, component=component: d[component], weights),
            )
            for component in range(3)
        )
        vector_character_eigenvalues[name] = values
    assert vector_character_eigenvalues["blind"] == (0, 0, 0)
    assert vector_character_eigenvalues["real"] == (-8, -8, -8)
    assert vector_character_eigenvalues["imag"] == (-8, -8, -8)
    assert vector_character_eigenvalues["alternating"] == (-8, -8, -8)
    checks += 12

    print(f"PASS: C18 two-record linearized collision kernel ({checks} exact checks)")
    print(f"jacobian_scale=I + N/5^17, tangent_dimension=48")
    print(f"exact_correction_spectrum={sorted(exact_spectrum.items())}")
    print("rank=41, nullity=7; every Jacobian eigenvalue lies in (0, 1]")
    print(f"certified_left_invariants=7 (four phase counts + three momentum components)")
    print("phase-vector currents: blind momentum=0; real/imag/alternating=-8 (relaxing)")
    print("capacity_shear: Eg=-30, T2g=-160 (relaxing, cubically split, not gapless)")
    print("uniform changed-state weight=432/5^18")
    print("SC boundary: all six SC channel marginals are collision spectators")


if __name__ == "__main__":
    main()
