#!/usr/bin/env python3
"""Exact FCC C4 collision protecting only one complex vector current.

The registered local domain consists of exactly two occupied, distinct FCC
channels carrying a common C4 phase.  The collision uses only exact lattice
data:

* at zero Gaussian current, advance both phases by one quarter turn;
* otherwise apply the already-certified FCC momentum-doubleton scatter and
  then compensate spatial antipodes by a phase half-turn.

The result is a target-free reversible local permutation.  This certificate
proves its covariance, conserved-current algebra, complete additive invariant
space, product-reference tangent kernel, and the conditional Maxwell mode
count.  It does not identify the current with electromagnetism, derive a
finite-k wave pole, couple the SC shell, or measure alpha.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations, product

from sympy import I, Matrix, Poly, Rational, expand, sqrt, symbols
from sympy.polys.matrices import DomainMatrix

from proof_c18_two_record_momentum_sector_census import (
    FCC_DIRECTIONS,
    PhasePair,
    build_sectors,
    canonical_pair,
    canonical_phase_pair,
    phase_complete_collision,
    shift_phase_pair,
    transform_phase_pair,
)
from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    subtract,
    zero_chart,
)
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
)
from proof_moore_bond_capacity_type_census import (
    FCC_LINES,
    second_moment_rank,
    signed_permutation_matrices,
)


PHASES = tuple(range(4))
PHASE_COORDINATES = ((1, 0), (0, 1), (-1, 0), (0, -1))
ONE_PARTICLE_STATES = tuple(
    (direction, phase)
    for direction in FCC_DIRECTIONS
    for phase in PHASES
)
ONE_PARTICLE_INDEX = {
    state: index for index, state in enumerate(ONE_PARTICLE_STATES)
}
FCC_INDEX = {direction: index for index, direction in enumerate(FCC_DIRECTIONS)}
TANGENT_INDEX = {
    (direction, phase): 4 * FCC_INDEX[direction] + phase
    for direction in FCC_DIRECTIONS
    for phase in PHASES
}


def negate(direction: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple(-entry for entry in direction)  # type: ignore[return-value]


def homogeneous_states() -> tuple[PhasePair, ...]:
    return tuple(
        canonical_phase_pair((left, phase), (right, phase))
        for left, right in combinations(FCC_DIRECTIONS, 2)
        for phase in PHASES
    )


def gaussian_current(state: PhasePair) -> tuple[int, ...]:
    """Return (Re C_x,y,z, Im C_x,y,z) for C=sum i^p d."""

    real = [0, 0, 0]
    imag = [0, 0, 0]
    for direction, phase in state:
        u, v = PHASE_COORDINATES[phase]
        for component in range(3):
            real[component] += u * direction[component]
            imag[component] += v * direction[component]
    return tuple(real + imag)


def antipodal_phase_flip(state: PhasePair) -> PhasePair:
    """Preserve each Gaussian vector i^p d exactly."""

    return canonical_phase_pair(
        (negate(state[0][0]), (state[0][1] + 2) % 4),
        (negate(state[1][0]), (state[1][1] + 2) % 4),
    )


def advance_common_phase(state: PhasePair) -> PhasePair:
    return canonical_phase_pair(
        (state[0][0], (state[0][1] + 1) % 4),
        (state[1][0], (state[1][1] + 1) % 4),
    )


def coherent_collision(
    state: PhasePair,
    spatial_sectors,
) -> PhasePair:
    """Order-four collision on the invariant common-phase two-record domain."""

    assert state[0][1] == state[1][1]
    if gaussian_current(state) == (0,) * 6:
        return advance_common_phase(state)
    scattered = phase_complete_collision(state, spatial_sectors)
    return antipodal_phase_flip(scattered)


def occupation_vector(state: PhasePair) -> tuple[int, ...]:
    vector = [0] * len(ONE_PARTICLE_STATES)
    for record in state:
        vector[ONE_PARTICLE_INDEX[record]] += 1
    return tuple(vector)


def transition_row(before: PhasePair, after: PhasePair) -> list[int]:
    incoming = occupation_vector(before)
    outgoing = occupation_vector(after)
    return [outgoing[index] - incoming[index] for index in range(len(incoming))]


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def invariant_rows() -> tuple[Matrix, ...]:
    rows = [Matrix([[1 for _state in ONE_PARTICLE_STATES]])]
    for quadrature in range(2):
        for component in range(3):
            rows.append(
                Matrix(
                    [[
                        PHASE_COORDINATES[phase][quadrature] * direction[component]
                        for direction, phase in ONE_PARTICLE_STATES
                    ]]
                )
            )
    return tuple(rows)


def reduced_score(state: PhasePair, direction, phase: int) -> int:
    """Derivative score with p_blank=1-sum_k p_k on each channel."""

    phase_by_direction = {
        record_direction: record_phase for record_direction, record_phase in state
    }
    if direction not in phase_by_direction:
        return -1
    return int(phase_by_direction[direction] == phase)


def tangent_correction(states: tuple[PhasePair, ...], spatial_sectors) -> Matrix:
    correction = [[0 for _ in range(48)] for _ in range(48)]
    for state in states:
        outgoing = coherent_collision(state, spatial_sectors)
        before = occupation_vector(state)
        after = occupation_vector(outgoing)
        delta = tuple(after[index] - before[index] for index in range(48))
        for row, delta_entry in enumerate(delta):
            if delta_entry == 0:
                continue
            for direction, phase in TANGENT_INDEX:
                column = TANGENT_INDEX[(direction, phase)]
                correction[row][column] += delta_entry * reduced_score(
                    state, direction, phase
                )
    return Matrix(correction)


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    spatial_pairs = tuple(
        canonical_pair(left, right)
        for left, right in combinations(FCC_DIRECTIONS, 2)
    )
    spatial_sectors = build_sectors(spatial_pairs)
    states = homogeneous_states()

    assert len(group) == 48
    assert len(FCC_DIRECTIONS) == 12
    assert len(ONE_PARTICLE_STATES) == 48
    assert len(states) == 264
    assert second_moment_rank(FCC_LINES) == 6
    checks += 5

    # On the equal-length FCC shell, the normalized current is simply the raw
    # Gaussian current divided by sqrt(2).  The earlier actualization vertex
    # therefore injects exactly one ninth of the corresponding normalized
    # one-record current, with the same phase and orientation.
    for line_index, raw_direction in enumerate(FCC_LINES, start=3):
        normalized = Matrix(raw_direction) / sqrt(2)
        assert normalized == LINE_DIRECTIONS[line_index]
        for phase in PHASES:
            u, v = PHASE_COORDINATES[phase]
            for orientation in (-1, 1):
                token = Token(phase, orientation)
                reserve = ActualizationState(0, 0, 0, None, token)
                manifested = actualization_macro(reserve, True)
                delta = subtract(
                    zero_chart(manifested, line_index),
                    zero_chart(reserve, line_index),
                )
                assert (
                    delta.relative_u
                    == Rational(orientation * u, 9) * Matrix(raw_direction) / sqrt(2)
                )
                assert (
                    delta.relative_v
                    == Rational(orientation * v, 9) * Matrix(raw_direction) / sqrt(2)
                )
                checks += 2

    current_sectors: defaultdict[tuple[int, ...], list[PhasePair]] = defaultdict(list)
    for state in states:
        current_sectors[gaussian_current(state)].append(state)
    sector_histogram = Counter(len(sector) for sector in current_sectors.values())
    assert len(current_sectors) == 85
    assert sector_histogram == Counter({2: 48, 4: 36, 24: 1})
    assert len(current_sectors[(0,) * 6]) == 24
    checks += 3

    image = {
        state: coherent_collision(state, spatial_sectors) for state in states
    }
    assert set(image) == set(states)
    assert set(image.values()) == set(states)
    assert all(image[state] != state for state in states)
    assert all(
        coherent_collision(
            coherent_collision(
                coherent_collision(
                    coherent_collision(state, spatial_sectors), spatial_sectors
                ),
                spatial_sectors,
            ),
            spatial_sectors,
        )
        == state
        for state in states
    )
    checks += 4

    cycle_histogram = Counter()
    unseen = set(states)
    while unseen:
        start = min(unseen)
        orbit = []
        state = start
        while state not in orbit:
            orbit.append(state)
            state = image[state]
        assert state == start
        for member in orbit:
            unseen.remove(member)
        cycle_histogram[len(orbit)] += 1
    assert cycle_histogram == Counter({2: 120, 4: 6})
    checks += 1

    for state in states:
        outgoing = image[state]
        assert gaussian_current(outgoing) == gaussian_current(state)
        for phase_shift in PHASES:
            assert coherent_collision(
                shift_phase_pair(state, phase_shift), spatial_sectors
            ) == shift_phase_pair(outgoing, phase_shift)
            checks += 1
        for matrix in group:
            assert coherent_collision(
                transform_phase_pair(matrix, state), spatial_sectors
            ) == transform_phase_pair(matrix, outgoing)
            checks += 1

    # The exact additive collision invariants of this one permutation are
    # record number and the six real coordinates of the Gaussian current.
    transition = Matrix([transition_row(state, image[state]) for state in states])
    transition_rank = exact_rank(transition)
    assert transition_rank == 41
    assert len(ONE_PARTICLE_STATES) - transition_rank == 7
    checks += 2

    protected = invariant_rows()
    protected_stack = Matrix.vstack(*protected)
    assert exact_rank(protected_stack) == 7
    for row in protected:
        assert transition * row.T == Matrix.zeros(len(states), 1)
        checks += 1

    # Conserving each phase species' momentum would instead protect four
    # vector triplets.  The regular C4 character basis makes the two unwanted
    # triplets explicit: blind and alternating, in addition to Re/Im C.
    phase_resolved = []
    for phase in PHASES:
        for component in range(3):
            phase_resolved.append(
                Matrix(
                    [[
                        int(candidate_phase == phase) * direction[component]
                        for direction, candidate_phase in ONE_PARTICLE_STATES
                    ]]
                )
            )
    phase_resolved_stack = Matrix.vstack(*phase_resolved)
    assert exact_rank(phase_resolved_stack) == 12
    checks += 1

    character_weights = (
        (1, 1, 1, 1),
        (1, -1, 1, -1),
        (1, 0, -1, 0),
        (0, 1, 0, -1),
    )
    character_rows = []
    for weights in character_weights:
        for component in range(3):
            character_rows.append(
                Matrix(
                    [[
                        weights[phase] * direction[component]
                        for direction, phase in ONE_PARTICLE_STATES
                    ]]
                )
            )
    assert exact_rank(Matrix.vstack(*character_rows)) == 12
    assert Matrix.vstack(*character_rows[6:]) == protected_stack[1:, :]
    checks += 2

    # The same FCC shell has full symmetric-tensor type, but none of its two
    # C4-weighted tensor quadratures is an additive invariant of this
    # collision.  Vector protection therefore does not silently solve the
    # gravity dynamics.
    symmetric_components = ((0, 0), (1, 1), (2, 2), (0, 1), (0, 2), (1, 2))
    tensor_rows = []
    for quadrature in range(2):
        for left, right in symmetric_components:
            tensor_rows.append(
                Matrix(
                    [[
                        PHASE_COORDINATES[phase][quadrature]
                        * direction[left]
                        * direction[right]
                        for direction, phase in ONE_PARTICLE_STATES
                    ]]
                )
            )
    tensor_stack = Matrix.vstack(*tensor_rows)
    assert exact_rank(tensor_stack) == 12
    assert exact_rank(Matrix.vstack(protected_stack, tensor_stack)) == 19
    checks += 2

    # Exact product-reference tangent correction.  This collision is active
    # on exactly-two-occupancy states of a 12-channel FCC carrier, so the
    # normalized Jacobian is I + N/5^11.
    correction = tangent_correction(states, spatial_sectors)
    assert correction.shape == (48, 48)
    assert exact_rank(correction) == 41
    for row in protected:
        assert row * correction == Matrix.zeros(1, 48)
        checks += 1

    # Prove the complete spectrum without a numerical eigensolver.  The exact
    # integer DomainMatrix characteristic polynomial is compared coefficient
    # by coefficient with the frozen rational factorization.
    spectral_factors = (
        ("0", (0,), 7),
        ("-4", (-4,), 1),
        ("-8", (-8,), 3),
        ("-10", (-10,), 2),
        ("-10+-2i", (-10 + 2 * I, -10 - 2 * I), 3),
        ("-12", (-12,), 9),
        ("-12+-2i", (-12 + 2 * I, -12 - 2 * I), 2),
        ("-20", (-20,), 3),
        ("-30", (-30,), 2),
        ("-40", (-40,), 6),
        ("-42+-2i", (-42 + 2 * I, -42 - 2 * I), 1),
        ("-100", (-100,), 3),
    )
    x = symbols("x")
    expected_characteristic = (
        x**7
        * (x + 4)
        * (x + 8) ** 3
        * (x + 10) ** 2
        * (x**2 + 20 * x + 104) ** 3
        * (x + 12) ** 9
        * (x**2 + 24 * x + 148) ** 2
        * (x + 20) ** 3
        * (x + 30) ** 2
        * (x + 40) ** 6
        * (x**2 + 84 * x + 1768)
        * (x + 100) ** 3
    )
    expected_coefficients = tuple(Poly(expected_characteristic, x).all_coeffs())
    actual_coefficients = tuple(DomainMatrix.from_Matrix(correction).charpoly())
    assert actual_coefficients == expected_coefficients
    checks += 1

    assert sum(
        len(roots) * multiplicity
        for _name, roots, multiplicity in spectral_factors
    ) == 48
    checks += 1

    # For DF=I+N/5^11, every nonzero spectral root lies strictly inside the
    # unit disk.  This proves that only the seven conserved modes are gapless
    # at k=0 in the registered product-reference linearization.
    jacobian_scale = 5**11
    for _name, roots, _multiplicity in spectral_factors[1:]:
        for root in roots:
            modulus_squared = expand(
                (1 + root / jacobian_scale)
                * (1 + root.conjugate() / jacobian_scale)
            )
            assert 0 < modulus_squared < 1
            checks += 1

    # A current phase-space pair has six real components.  Two longitudinal
    # constraints (or one Gauss constraint plus one gauge quotient) leave a
    # four-dimensional transverse phase space: two polarizations.
    for wavevector in product(range(-2, 3), repeat=3):
        if wavevector == (0, 0, 0):
            continue
        kx, ky, kz = wavevector
        constraint = Matrix(
            [
                [kx, ky, kz, 0, 0, 0],
                [0, 0, 0, kx, ky, kz],
            ]
        )
        assert constraint.rank() == 2
        assert 6 - constraint.rank() == 4
        checks += 2

    # A global quarter-turn acts as the canonical complex structure on the
    # protected vector pair.
    for state in states:
        real = Matrix(gaussian_current(state)[:3])
        imag = Matrix(gaussian_current(state)[3:])
        shifted = gaussian_current(shift_phase_pair(state, 1))
        assert Matrix(shifted[:3]) == -imag
        assert Matrix(shifted[3:]) == real
        checks += 2

    print(f"PASS: C18/FCC Gaussian-current collision ({checks} exact checks)")
    print(f"homogeneous_states={len(states)}, current_sectors={len(current_sectors)}")
    print(f"sector_histogram={sorted(sector_histogram.items())}")
    print(f"cycle_histogram={sorted(cycle_histogram.items())}")
    print("additive_invariants=7: record number + Re/Im Gaussian vector current")
    print("phase-resolved momentum would protect 12 vector components")
    print("FCC actualization source = one ninth of the normalized conserved current record")
    print("FCC line dyads retain exact symmetric-tensor rank 6")
    print("no nonzero C4-weighted FCC tensor moment lies in the additive invariant space")
    print(
        "correction_spectrum="
        + str([(name, multiplicity) for name, _roots, multiplicity in spectral_factors])
    )
    print("conditional transverse phase space: 6 - 2 = 4 real dimensions (two polarizations)")
    print("Open: finite-k pole, Gauss/gauge generation, SC coupling, source work, alpha")


if __name__ == "__main__":
    main()
