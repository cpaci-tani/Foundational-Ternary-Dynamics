#!/usr/bin/env python3
"""Exact zero-wavevector boundary for the selected Hodge-flag full tick.

This certificate composes the selected O_h-equivariant seven-invariant pair
collision with the shared-edge flag update and common C4 phase advance.  The
pair collision is linearized at the symmetric independent-binary product
reference.  Because the collision is an involution, its tangent correction is
an exact negative-semidefinite sum of transition outer products.

The twelve-tick co-rotating collision generator is then the sum of all twelve
internal conjugates.  Its kernel is the intersection of the conjugated
seven-dimensional collision kernels.  If that intersection contains only the
uniform number mode, the electromagnetic field modes are already gapped at
k=0 and no Maxwell cone can arise from this selected composition.

No physical target, fitted coefficient, or numerical eigensolver is used.
"""

from __future__ import annotations

from itertools import combinations

from sympy import Matrix
from sympy.polys.matrices import DomainMatrix

import proof_hodge_flag_equivariant_pair_matching as matching
from proof_hodge_flag_pair_collision_invariant_space import field_value
from proof_shared_edge_hodge_flag_bcc_propagation import update_flag


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def occupation_delta(before, after, size: int) -> Matrix:
    delta = [0] * size
    for index in after:
        delta[index] += 1
    for index in before:
        delta[index] -= 1
    return Matrix(delta)


def main() -> None:
    checks = 0

    # The parent certificate constructs and verifies the unique selected map
    # before exposing it for this downstream composition.
    matching.main()
    data = matching.CERTIFICATE_DATA
    assert data is not None
    states = data["states"]
    collision = data["collision"]
    state_index = {state: index for index, state in enumerate(states)}
    size = len(states)
    assert size == 192
    assert len(collision) == 18336
    checks += 3

    # At the p=1/2 independent-binary product reference, the common positive
    # scalar multiplying the collision derivative is irrelevant to rank.  For
    # each two-cycle {x,y}, the reduced correction contributes
    # -2 (a_y-a_x)(a_y-a_x)^T.  This proves symmetry and negative
    # semidefiniteness without a floating-point spectral test.
    correction = Matrix.zeros(size, size)
    transition_count = 0
    for before in combinations(range(size), 2):
        after = collision[before]
        if before >= after:
            continue
        delta = occupation_delta(before, after, size)
        correction -= 2 * delta * delta.T
        transition_count += 1

    assert transition_count == 9168
    assert correction == correction.T
    assert exact_rank(correction) == 185
    assert correction.trace() < 0
    checks += 4

    invariant = Matrix.vstack(
        Matrix([[1] * size]),
        *(
            Matrix([[field_value(state)[component] for state in states]])
            for component in range(6)
        ),
    )
    assert invariant.shape == (7, size)
    assert exact_rank(invariant) == 7
    assert invariant * correction == Matrix.zeros(7, size)
    assert correction * invariant.T == Matrix.zeros(size, 7)
    checks += 4

    # Internal part of one free tick: update the shared-edge flag and advance
    # the common C4 phase.  Streaming displacement is invisible at k=0.
    internal_image = []
    for flag, phase in states:
        internal_image.append(state_index[(update_flag(flag), (phase + 1) % 4)])
    assert len(set(internal_image)) == size
    internal = Matrix.zeros(size, size)
    for source, target in enumerate(internal_image):
        internal[target, source] = 1
    assert internal**12 == Matrix.eye(size)
    checks += 2

    pair_commutation_failures = 0
    for pair in combinations(range(size), 2):
        advanced_pair = tuple(sorted(internal_image[index] for index in pair))
        collided_then_advanced = tuple(
            sorted(internal_image[index] for index in collision[pair])
        )
        if collision[advanced_pair] != collided_then_advanced:
            pair_commutation_failures += 1
    assert pair_commutation_failures > 0
    checks += 1

    # A Maxwell carrier would require the collision kernel to be invariant
    # under the internal clock/flag permutation.  Test the complete 12-step
    # orbit rather than only one phase convention.
    constraint_blocks = []
    invariant_orbit_rows = []
    power = Matrix.eye(size)
    inverse_power = Matrix.eye(size)
    averaged = Matrix.zeros(size, size)
    for _tick in range(12):
        constraint_blocks.append(correction * power)
        invariant_orbit_rows.append(invariant * inverse_power)
        averaged += inverse_power * correction * power
        power = internal * power
        inverse_power = inverse_power * internal.T

    stacked_constraints = Matrix.vstack(*constraint_blocks)
    persistent_rank = exact_rank(stacked_constraints)
    persistent_nullity = size - persistent_rank
    orbit_invariant_rank = exact_rank(Matrix.vstack(*invariant_orbit_rows))
    averaged_rank = exact_rank(averaged)
    averaged_nullity = size - averaged_rank

    # Freeze the exact obstruction.  Negative semidefiniteness makes the
    # averaged kernel equal to the intersection kernel: damping contributions
    # from different clock frames cannot cancel one another.
    assert averaged == averaged.T
    assert persistent_nullity == averaged_nullity
    uniform = Matrix.ones(size, 1)
    assert stacked_constraints * uniform == Matrix.zeros(12 * size, 1)
    assert averaged * uniform == Matrix.zeros(size, 1)
    checks += 4

    print(
        "internal_pair_commutation_failures="
        f"{pair_commutation_failures}/18336"
    )
    print(f"collision_rank=185, collision_nullity=7")
    print(f"clock_orbit_invariant_row_rank={orbit_invariant_rank}")
    print(
        f"persistent_rank={persistent_rank}, "
        f"persistent_nullity={persistent_nullity}"
    )
    print(
        f"twelve_tick_averaged_rank={averaged_rank}, "
        f"averaged_nullity={averaged_nullity}"
    )
    print(
        "PASS: selected Hodge-flag collision full-tick boundary "
        f"({checks} exact checks plus parent certificate)"
    )

    if persistent_nullity < 7:
        print(
            "Scoped closed negative: the selected O_h collision does not "
            "transport its six field invariants through the C3xC4 internal tick"
        )
        print(
            "A clock-compatible collision must preserve the complete internal "
            "orbit of the electromagnetic readout, not only fixed-frame E and B"
        )
    else:
        print(
            "Zero-mode compatibility passes; finite-k Hodge/Maxwell analysis "
            "remains required"
        )


if __name__ == "__main__":
    main()
