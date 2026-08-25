#!/usr/bin/env python3
"""Exact dual-A9 skew capacity/clock permission generator.

Two independently owned A9 one-token carriers are placed on primal and dual
ownership sites.  A retained structural orientation chooses a receiver and a
driver.  The driver advances every global tick; its residual link capacity
admits or stalls the receiver's autonomous A9 clock.  The receiver's residual
link capacity supplies the separate spatial permission.

The triangular update is exactly invertible because the previous driver is
recovered first.  Every physical orbit has length sixteen and exact counts
N_t=N_s=8, N_11=4, so the clock and spatial marginals factorize on each
individual deterministic history.  Primal/dual exchange swaps the structural
orientation and commutes with the update.

This is a finite reference permutation and homogeneous factorization witness,
not a variational derivation, sourced weak-capacity law, cotangent Hodge lift,
spin-2 completion, or lensing result.
"""

from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction

from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    capacity,
    charge,
    conjugate_state,
    iterate,
    occupation,
    tick,
    token_count,
    valid_owned_states,
)


@dataclass(frozen=True)
class DualA9State:
    primal: LocalState
    dual: LocalState
    orientation: int

    def __post_init__(self) -> None:
        assert self.orientation in (0, 1)


def residual_permission(state: LocalState) -> int:
    """Scalar blank capacity on the currently manifested link placement."""
    return capacity(state.link)


def permissions(state: DualA9State) -> tuple[int, int]:
    """Return temporal/receiver and spatial/receiver capacity permissions."""
    if state.orientation == 0:
        receiver, driver = state.primal, state.dual
    else:
        receiver, driver = state.dual, state.primal
    return residual_permission(driver), residual_permission(receiver)


def advance(state: DualA9State) -> DualA9State:
    """One retained triangular capacity-controller permutation."""
    if state.orientation == 0:
        admitted = residual_permission(state.dual)
        return DualA9State(
            tick(state.primal) if admitted else state.primal,
            tick(state.dual),
            state.orientation,
        )

    admitted = residual_permission(state.primal)
    return DualA9State(
        tick(state.primal),
        tick(state.dual) if admitted else state.dual,
        state.orientation,
    )


def inverse_advance(state: DualA9State) -> DualA9State:
    """Recover the previous driver first, then undo the controlled receiver."""
    if state.orientation == 0:
        previous_dual = iterate(state.dual, 7)
        admitted = residual_permission(previous_dual)
        previous_primal = iterate(state.primal, 7) if admitted else state.primal
        return DualA9State(previous_primal, previous_dual, state.orientation)

    previous_primal = iterate(state.primal, 7)
    admitted = residual_permission(previous_primal)
    previous_dual = iterate(state.dual, 7) if admitted else state.dual
    return DualA9State(previous_primal, previous_dual, state.orientation)


def exchange(state: DualA9State) -> DualA9State:
    return DualA9State(state.dual, state.primal, 1 - state.orientation)


def conjugate(state: DualA9State) -> DualA9State:
    return DualA9State(
        conjugate_state(state.primal),
        conjugate_state(state.dual),
        state.orientation,
    )


def verify_total_permutation_and_symmetries() -> int:
    checks = 0
    states = tuple(
        DualA9State(primal, dual, orientation)
        for primal in valid_owned_states()
        for dual in valid_owned_states()
        for orientation in (0, 1)
    )
    assert len(states) == 512
    checks += 1

    images = []
    for state in states:
        output = advance(state)
        images.append(output)
        assert inverse_advance(output) == state
        assert advance(inverse_advance(state)) == state
        assert advance(exchange(state)) == exchange(output)
        assert advance(conjugate(state)) == conjugate(output)
        assert token_count(output.primal) == token_count(state.primal) == 1
        assert token_count(output.dual) == token_count(state.dual) == 1
        assert charge(output.primal) == charge(state.primal) == 0
        assert charge(output.dual) == charge(state.dual) == 0
        assert permissions(state)[0] in (0, 1)
        assert permissions(state)[1] in (0, 1)
        checks += 10

    assert len(set(images)) == len(states)
    checks += 1
    return checks


def verify_every_orbit_factorizes() -> int:
    checks = 0
    all_states = {
        DualA9State(primal, dual, orientation)
        for primal in valid_owned_states()
        for dual in valid_owned_states()
        for orientation in (0, 1)
    }
    unseen = set(all_states)
    orbit_count = 0

    while unseen:
        start = min(
            unseen,
            key=lambda item: (
                item.orientation,
                item.primal.left,
                item.primal.right,
                item.primal.link,
                item.primal.reserve,
                item.dual.left,
                item.dual.right,
                item.dual.link,
                item.dual.reserve,
            ),
        )
        orbit = []
        state = start
        while state not in orbit:
            orbit.append(state)
            state = advance(state)
        assert state == start
        assert len(orbit) == 16
        unseen -= set(orbit)
        orbit_count += 1
        checks += 2

        temporal = tuple(permissions(state)[0] for state in orbit)
        spatial = tuple(permissions(state)[1] for state in orbit)
        joint = tuple(left * right for left, right in zip(temporal, spatial))
        assert sum(temporal) == 8
        assert sum(spatial) == 8
        assert sum(joint) == 4
        assert sum(joint) * len(orbit) == sum(temporal) * sum(spatial)
        checks += 4

        # The receiver completes one physical A9 period in sixteen global
        # ticks, while the capacity driver completes two periods.
        if start.orientation == 0:
            assert orbit[-1].dual != start.dual
            assert advance(orbit[-1]).dual == start.dual
        else:
            assert orbit[-1].primal != start.primal
            assert advance(orbit[-1]).primal == start.primal
        checks += 2

    assert orbit_count == 32
    checks += 1
    return checks


def verify_blocked_rate_ledger() -> int:
    checks = 0
    global_ticks = 16
    temporal_count = 8
    spatial_count = 8
    joint_count = 4

    temporal_rate = Fraction(temporal_count, global_ticks)
    spatial_rate = Fraction(spatial_count, global_ticks)
    wave_rate = Fraction(joint_count, global_ticks)
    assert temporal_rate == spatial_rate == Fraction(1, 2)
    assert wave_rate == temporal_rate * spatial_rate == Fraction(1, 4)
    checks += 2

    base_cone = Fraction(1, 6)
    maxwell_cone = base_cone * wave_rate
    tensor_cone = base_cone * wave_rate
    assert maxwell_cone == tensor_cone == Fraction(1, 24)
    checks += 1
    return checks


def main() -> None:
    checks = verify_total_permutation_and_symmetries()
    checks += verify_every_orbit_factorizes()
    checks += verify_blocked_rate_ledger()

    print("dual-A9 skew map: driver advances; its residual capacity gates receiver")
    print("inverse: recover previous driver, reread permission, undo receiver")
    print("all 32 physical orbits: length=16, N_t=8, N_s=8, N_11=4")
    print("primal/dual exchange covariance: swap copies and orientation sector")
    print("homogeneous fixture: clock=1/2, spatial=1/2, Maxwell/tensor cone=1/24")
    print(
        "PASS: dual-A9 skew capacity/clock generator "
        f"({checks} exact checks)"
    )
    print(
        "Open: variational selection, orientation ownership, variable sourced "
        "marginals, cotangent/TT lift, static response, and lensing"
    )


if __name__ == "__main__":
    main()
