#!/usr/bin/env python3
"""Exact local square-matter/stress/radiation work exchange.

The prepared square proto-matter clock supplies a persistent charge-even turn
stress and an ordered plane for the number-neutral cotangent radiation seed.
One A9 response carrier supplies a complementary capacity bit.  On the matched
physical subspace

    capacity(response.link) + radiation_active = 1,

each global tick toggles response ownership and the radiation seed together.
Old response capacity admits or stalls the material turn, response phase and
radiation stage advance globally, and the complete map has an explicit local
inverse.

The active 64-record plaquette has canonical slow-space norm 16.  Normalizing
that complete seed by 1/16 makes its field energy exactly complementary to the
one-unit A9 capacity energy, so Delta H_field = -Delta H_capacity on every
tick.  This is the first exact local cross-sector work invariant in the strict
construction.  The 1/16 matching is conditional on identifying one response
capacity unit with one complete plaquette excitation; it is not a measured
coupling or native alpha.

The active seed remains local and is reabsorbed on the following tick.  No
finite-amplitude collision/streaming schedule, momentum recoil, Lorentz force,
charged pole, gravity, lensing, Born completion, or alpha measurement follows.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass

from sympy import Rational

from proof_c4_square_material_turn_clock_radiation_frame import (
    SquareMatterState,
    chain_boundary,
    charge_distribution,
    conjugate as conjugate_matter,
    current_vector,
    inverse_step as inverse_matter_step,
    step as matter_step,
    stress_dyad,
    subtract_distributions,
    transform_state as transform_matter,
    transport_current,
    turn_frame,
)
from proof_c4_stress_capacity_reciprocal_feedback import owned_token
from proof_cotangent_framed_plaquette_radiation_release import (
    RadiationSeed,
    field_increment_on_edge,
    plaquette_edges,
    seed_divergence,
)
from proof_moore_bond_capacity_type_census import signed_permutation_matrices
from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    actualize,
    capacity,
    charge,
    phase_index,
    polarity,
    rotate_state,
    token_count,
    valid_owned_states,
)


@dataclass(frozen=True)
class WorkExchangeState:
    matter: SquareMatterState
    response: LocalState
    radiation_stage: int
    radiation_active: bool


def matched(state: WorkExchangeState) -> bool:
    return capacity(state.response.link) + int(state.radiation_active) == 1


def work_step(state: WorkExchangeState) -> WorkExchangeState:
    assert matched(state)
    admitted = capacity(state.response.link)
    matter_after = matter_step(state.matter) if admitted else state.matter
    response_kicked = actualize(state.response)
    output = WorkExchangeState(
        matter_after,
        rotate_state(response_kicked, 1),
        (state.radiation_stage + 1) % 12,
        not state.radiation_active,
    )
    assert matched(output)
    return output


def work_inverse(state: WorkExchangeState) -> WorkExchangeState:
    assert matched(state)
    response_kicked = rotate_state(state.response, -1)
    response_before = actualize(response_kicked)
    admitted = capacity(response_before.link)
    matter_before = (
        inverse_matter_step(state.matter) if admitted else state.matter
    )
    output = WorkExchangeState(
        matter_before,
        response_before,
        (state.radiation_stage - 1) % 12,
        not state.radiation_active,
    )
    assert matched(output)
    return output


def capacity_energy(state: WorkExchangeState) -> int:
    return capacity(state.response.link)


def radiation_energy(state: WorkExchangeState) -> int:
    return int(state.radiation_active)


def total_work_energy(state: WorkExchangeState) -> int:
    return capacity_energy(state) + radiation_energy(state)


def emission_matter(state: WorkExchangeState) -> SquareMatterState:
    """Recover the turn that emitted the currently active local seed."""

    assert state.radiation_active
    return inverse_matter_step(state.matter)


def active_seed(state: WorkExchangeState) -> RadiationSeed:
    source = emission_matter(state)
    return RadiationSeed(
        turn_frame(source),
        source.phase,
        state.radiation_stage,
        source.orientation,
        True,
    )


def conjugate(state: WorkExchangeState) -> WorkExchangeState:
    return WorkExchangeState(
        conjugate_matter(state.matter),
        state.response,
        state.radiation_stage,
        state.radiation_active,
    )


def transform_state(matrix, state: WorkExchangeState) -> WorkExchangeState:
    return WorkExchangeState(
        transform_matter(matrix, state.matter),
        state.response,
        state.radiation_stage,
        state.radiation_active,
    )


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    frames = tuple(
        (direction, second)
        for direction in (
            (1, 0, 0),
            (-1, 0, 0),
            (0, 1, 0),
            (0, -1, 0),
            (0, 0, 1),
            (0, 0, -1),
        )
        for second in (
            (1, 0, 0),
            (-1, 0, 0),
            (0, 1, 0),
            (0, -1, 0),
            (0, 0, 1),
            (0, 0, -1),
        )
        if sum(a * b for a, b in zip(direction, second)) == 0
    )
    matter_states = tuple(
        SquareMatterState(frame, phase, orientation)
        for frame in frames
        for phase in range(4)
        for orientation in (-1, 1)
    )
    response_states = valid_owned_states()
    assert len(matter_states) == 192
    assert len(response_states) == 16
    checks += 2

    # Canonical radiation norm.  The cotangent field Gram is 64 I_6, so one
    # active number-neutral edge increment 16*d has norm four; four distinct
    # plaquette edges give norm sixteen.
    # The parent release certificate exhausts phase, stage, and polarity.
    # Re-evaluate the action norm once on each of the 24 spatial frames.
    for frame in frames:
        matter = SquareMatterState(frame, 0, 1)
        seed = RadiationSeed(turn_frame(matter), 0, 0, 1, True)
        assert seed_divergence(seed) == {}
        norm = Rational(0)
        for _tail, direction in plaquette_edges(seed.frame):
            increment = field_increment_on_edge(seed, direction)
            assert increment[3:] == (0, 0, 0)
            edge_norm = Rational(
                sum(component * component for component in increment[:3]),
                64,
            )
            assert edge_norm == 4
            norm += edge_norm
            checks += 2
        assert norm == 16
        assert norm / 16 == 1
        checks += 2

    # The radiation stage is an independent C12 translation.  Exhaust the
    # interacting base map at one stage; the explicit inverse is stage-blind.
    base_states = tuple(
        WorkExchangeState(
            matter,
            response,
            0,
            not bool(capacity(response.link)),
        )
        for matter in matter_states
        for response in response_states
    )
    assert len(base_states) == 3_072
    assert len(base_states) * 12 == 36_864
    checks += 1

    images = []
    for state in base_states:
        assert matched(state)
        assert total_work_energy(state) == 1
        output = work_step(state)
        images.append(output)
        assert work_inverse(output) == state
        assert work_step(work_inverse(state)) == state
        assert total_work_energy(output) == total_work_energy(state) == 1
        assert radiation_energy(output) - radiation_energy(state) == -(
            capacity_energy(output) - capacity_energy(state)
        )
        assert token_count(output.response) == token_count(state.response) == 1
        assert charge(output.response) == charge(state.response) == 0
        assert polarity(owned_token(output.response)) == polarity(
            owned_token(state.response)
        )
        checks += 9

        admitted = capacity(state.response.link)
        expected_matter = matter_step(state.matter) if admitted else state.matter
        assert output.matter == expected_matter
        assert output.radiation_active == bool(admitted)
        assert capacity(output.response.link) == 1 - admitted
        assert phase_index(owned_token(output.response)) == (
            phase_index(owned_token(state.response)) + 1
        ) % 4
        checks += 4

        if admitted:
            delta_charge = subtract_distributions(
                charge_distribution(output.matter),
                charge_distribution(state.matter),
            )
            assert delta_charge == chain_boundary(transport_current(state.matter))
            seed = active_seed(output)
            assert seed.frame == turn_frame(state.matter)
            assert seed.orientation == state.matter.orientation
            assert seed_divergence(seed) == {}
            checks += 4
        else:
            assert output.matter == state.matter
            assert not output.radiation_active
            checks += 2

        conjugated_output = work_step(conjugate(state))
        assert conjugated_output == conjugate(output)
        checks += 1

    assert len(set(images)) == len(base_states)
    checks += 1

    # Cubic covariance is independent of response state and radiation stage;
    # check every material state against the complete signed cubic group.
    reference_response = response_states[0]
    reference_active = not bool(capacity(reference_response.link))
    for matter in matter_states:
        assert stress_dyad(conjugate_matter(matter)) == stress_dyad(matter)
        assert current_vector(conjugate_matter(matter)) == -current_vector(matter)
        checks += 2
        state = WorkExchangeState(matter, reference_response, 0, reference_active)
        for matrix in group:
            assert work_step(transform_state(matrix, state)) == transform_state(
                matrix, work_step(state)
            )
            checks += 1

    # Complete orbit census on the matched physical state space.
    orbit_states = {
        WorkExchangeState(
            matter,
            response,
            stage,
            not bool(capacity(response.link)),
        )
        for matter in matter_states
        for response in response_states
        for stage in range(12)
    }
    assert len(orbit_states) == 36_864
    unseen = set(orbit_states)
    histogram = Counter()
    while unseen:
        start = next(iter(unseen))
        orbit = []
        state = start
        while state not in orbit:
            orbit.append(state)
            state = work_step(state)
        assert state == start
        assert all(total_work_energy(item) == 1 for item in orbit)
        assert sum(item.radiation_active for item in orbit) * 2 == len(orbit)
        assert sum(capacity(item.response.link) for item in orbit) * 2 == len(orbit)
        unseen -= set(orbit)
        histogram[len(orbit)] += 1
        checks += 4

    assert histogram == Counter({24: 1536})
    checks += 1

    print("matched work domain: response_capacity + radiation_active = 1")
    print("active 64-record field seed canonical norm=16; normalized energy=norm/16=1")
    print("every tick: DeltaH_field=-DeltaH_capacity with total energy one")
    print("material turn admitted on capacity-owned half-ticks; local rate=1/2")
    print("complete census: 1,536 period-24 work-exchange orbits")
    print(
        f"PASS: C4 square matter/stress/radiation work exchange ({checks} exact checks)"
    )
    print(
        "Open: derive rather than select the 1/16 curvature match, release the "
        "seed into finite propagation, close momentum recoil/Lorentz force, "
        "charged pole, gravity/lensing, Born, and alpha"
    )


if __name__ == "__main__":
    main()
