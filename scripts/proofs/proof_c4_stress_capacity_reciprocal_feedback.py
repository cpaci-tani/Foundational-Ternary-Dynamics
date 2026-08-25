#!/usr/bin/env python3
"""Exact C4 stress/capacity feedback and Maxwell parity-price certificate.

Two independently owned A9 carriers are used.  The response carrier's current
capacity admits or stalls the material carrier's autonomous tick.  The
post-drift manifested state then toggles the response ownership, and the
response phase advances once on every global tick.  This drift--kick--clock
composition is a permutation: undo the global phase advance, undo the
source-controlled response kick, reread the recovered permission, and undo
the admitted material tick.

On each C18 line the persistent manifested state has the same phase-neutral
charge-odd current and charge-even tensor readouts as the shared
actualization vertex.  The response capacity changes iff the tensor source is
present, with equal one-token tensor norm.  Thus the construction closes a
finite local source -> response -> next-admission loop without an external
permission word.

The final exact group check prices an important boundary.  No involution of a
single C4 phase orbit can both commute with the forward clock and conjugate a
charge-odd unit kick to its inverse.  The even stress/capacity response may use
the clock carrier, but a charge-odd Maxwell response requires a distinct
signed/cotangent carrier.  This certificate is not a variational selection,
physical work normalization, propagating pole, gravity, lensing, Born rule,
or native-alpha measurement.
"""

from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from itertools import permutations

from sympy import Matrix, Rational

from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    LINE_DYADS,
    zero_chart,
)
from proof_c18_phase_neutral_shared_charge_stress_vertex import (
    phase_neutral_sources,
)
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
)
from proof_ternary_square_phase_polarity_autonomous_clock import (
    LocalState,
    actualize,
    capacity,
    charge,
    conjugate_state,
    iterate,
    occupation,
    phase_index,
    polarity,
    rotate_state,
    tick,
    token_count,
    valid_owned_states,
)


@dataclass(frozen=True)
class FeedbackState:
    matter: LocalState
    response: LocalState


def owned_token(state: LocalState) -> tuple[int, int]:
    token = state.link if occupation(state.link) else state.reserve
    assert occupation(token) == 1
    return token


def as_actualization_state(state: LocalState) -> ActualizationState:
    """Translate one valid A9 ownership state into the shared source chart."""

    token_state = owned_token(state)
    token = Token(phase_index(token_state), polarity(token_state))
    if occupation(state.link):
        return ActualizationState(
            0,
            state.left,
            state.right,
            token,
            None,
        )
    return ActualizationState(0, state.left, state.right, None, token)


def persistent_sources(state: LocalState, line_index: int):
    """Phase-neutral current/tensor carried by the present ownership state."""

    token_state = owned_token(state)
    phase = phase_index(token_state)
    chart = zero_chart(as_actualization_state(state), line_index)
    return phase_neutral_sources(chart, phase)


def feedback_step(state: FeedbackState) -> FeedbackState:
    """One local drift--source kick--global response-clock permutation."""

    admitted = capacity(state.response.link)
    matter_after = tick(state.matter) if admitted else state.matter
    source_present = occupation(matter_after.link)
    response_kicked = (
        actualize(state.response) if source_present else state.response
    )
    return FeedbackState(matter_after, rotate_state(response_kicked, 1))


def feedback_inverse(state: FeedbackState) -> FeedbackState:
    """Undo clock, source kick, and drift in the reverse triangular order."""

    response_kicked = rotate_state(state.response, -1)
    source_present = occupation(state.matter.link)
    response_before = (
        actualize(response_kicked) if source_present else response_kicked
    )
    admitted = capacity(response_before.link)
    matter_before = iterate(state.matter, 7) if admitted else state.matter
    return FeedbackState(matter_before, response_before)


def response_capacity_chart(state: LocalState, line_index: int) -> Matrix:
    return zero_chart(as_actualization_state(state), line_index).capacity


def frobenius_squared(matrix: Matrix):
    return sum((entry * entry for entry in matrix), start=Rational(0))


def verify_total_permutation_sources_and_reciprocity() -> int:
    checks = 0
    local_states = valid_owned_states()
    states = tuple(
        FeedbackState(matter, response)
        for matter in local_states
        for response in local_states
    )
    assert len(states) == 256
    checks += 1

    images = []
    for state in states:
        output = feedback_step(state)
        images.append(output)
        assert feedback_inverse(output) == state
        assert feedback_step(feedback_inverse(state)) == state
        assert token_count(output.matter) == token_count(state.matter) == 1
        assert token_count(output.response) == token_count(state.response) == 1
        assert charge(output.matter) == charge(state.matter) == 0
        assert charge(output.response) == charge(state.response) == 0
        checks += 6

        admitted = capacity(state.response.link)
        expected_matter = tick(state.matter) if admitted else state.matter
        source_present = occupation(expected_matter.link)
        response_kicked = (
            actualize(state.response) if source_present else state.response
        )
        assert output.matter == expected_matter
        assert output.response == rotate_state(response_kicked, 1)
        assert capacity(output.response.link) == capacity(response_kicked.link)
        assert (
            capacity(output.response.link) != capacity(state.response.link)
        ) == bool(source_present)
        checks += 4

        # Matter charge conjugation leaves the stress/capacity response fixed.
        conjugated_input = FeedbackState(
            conjugate_state(state.matter), state.response
        )
        conjugated_output = feedback_step(conjugated_input)
        assert conjugated_output == FeedbackState(
            conjugate_state(output.matter), output.response
        )
        checks += 1

        for line_index, (direction, dyad) in enumerate(
            zip(LINE_DIRECTIONS, LINE_DYADS)
        ):
            current, tensor, vector_cross, tensor_cross = persistent_sources(
                output.matter, line_index
            )
            epsilon = polarity(owned_token(output.matter))
            assert current == Rational(source_present * epsilon, 9) * direction
            assert tensor == Rational(source_present, 18) * dyad
            assert vector_cross == Matrix.zeros(3, 1)
            assert tensor_cross == Matrix.zeros(3, 3)
            checks += 4

            response_delta = (
                response_capacity_chart(output.response, line_index)
                - response_capacity_chart(state.response, line_index)
            )
            if source_present:
                assert response_delta in (dyad / 18, -dyad / 18)
                assert frobenius_squared(response_delta) == (
                    frobenius_squared(tensor)
                )
                assert tensor.trace() == Rational(1, 18)
                checks += 3
            else:
                assert response_delta == Matrix.zeros(3, 3)
                assert tensor == Matrix.zeros(3, 3)
                checks += 2

            conjugate_current, conjugate_tensor, _vc, _tc = persistent_sources(
                conjugate_state(output.matter), line_index
            )
            assert conjugate_current == -current
            assert conjugate_tensor == tensor
            checks += 2

    assert len(set(images)) == len(states)
    checks += 1
    return checks


def verify_complete_orbit_ledger() -> int:
    checks = 0
    all_states = {
        FeedbackState(matter, response)
        for matter in valid_owned_states()
        for response in valid_owned_states()
    }
    unseen = set(all_states)
    orbit_histogram: dict[int, int] = {}

    while unseen:
        start = min(unseen, key=repr)
        orbit = []
        state = start
        while state not in orbit:
            orbit.append(state)
            state = feedback_step(state)
        assert state == start
        unseen -= set(orbit)
        orbit_histogram[len(orbit)] = orbit_histogram.get(len(orbit), 0) + 1
        checks += 2

        admissions = sum(capacity(item.response.link) for item in orbit)
        sources = sum(
            occupation(feedback_step(item).matter.link) for item in orbit
        )
        event_deltas = tuple(
            occupation(feedback_step(item).matter.link)
            - occupation(item.matter.link)
            for item in orbit
        )
        response_kicks = sources

        if len(orbit) == 12:
            assert admissions == sources == response_kicks == 8
            assert event_deltas.count(1) == event_deltas.count(-1) == 1
            assert event_deltas.count(0) == 10
            assert Fraction(admissions, len(orbit)) == Fraction(2, 3)
            checks += 5
        else:
            assert len(orbit) == 4
            assert admissions == sources == response_kicks == 0
            assert event_deltas == (0, 0, 0, 0)
            checks += 3

    assert orbit_histogram == {4: 16, 12: 16}
    checks += 1
    return checks


def verify_no_additive_cross_sector_work_invariant() -> int:
    """Classify every conserved energy additive over the two A9 carriers."""

    checks = 0
    local_states = valid_owned_states()
    local_index = {state: index for index, state in enumerate(local_states)}
    states = tuple(
        FeedbackState(matter, response)
        for matter in local_states
        for response in local_states
    )

    # An arbitrary additive candidate has 32 coefficients:
    # E(M,G)=e_M(M)+e_G(G).  One row enforces E(F(M,G))-E(M,G)=0.
    rows = []
    for state in states:
        output = feedback_step(state)
        row = [0] * 32
        row[local_index[output.matter]] += 1
        row[local_index[state.matter]] -= 1
        row[16 + local_index[output.response]] += 1
        row[16 + local_index[state.response]] -= 1
        rows.append(row)
    invariance = Matrix(rows)
    assert invariance.shape == (256, 32)
    assert invariance.rank() == 28
    assert 32 - invariance.rank() == 4
    checks += 3

    # The full kernel is exhausted by independent functions of the two
    # separately conserved token-polarity labels.  Hence no additive invariant
    # depends on phase or link/reserve ownership, and none can record work
    # exchanged by the capacity kick.
    expected_columns = []
    for carrier_offset in (0, 16):
        for epsilon in (-1, 1):
            column = Matrix.zeros(32, 1)
            for state in local_states:
                if polarity(owned_token(state)) == epsilon:
                    column[carrier_offset + local_index[state], 0] = 1
            expected_columns.append(column)
    expected_kernel = Matrix.hstack(*expected_columns)
    assert expected_kernel.rank() == 4
    assert invariance * expected_kernel == Matrix.zeros(256, 4)
    assert len(invariance.nullspace()) == expected_kernel.rank()
    checks += 3
    return checks


def compose(left: tuple[int, ...], right: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(left[right[index]] for index in range(4))


def inverse(permutation: tuple[int, ...]) -> tuple[int, ...]:
    output = [0] * 4
    for index, image in enumerate(permutation):
        output[image] = index
    return tuple(output)


def verify_single_c4_maxwell_parity_price() -> int:
    """Clock-commuting and charge-odd conjugations are disjoint on one C4."""

    checks = 0
    identity = (0, 1, 2, 3)
    rotation = (1, 2, 3, 0)
    rotation_inverse = inverse(rotation)
    assert rotation_inverse == (3, 0, 1, 2)
    assert rotation != rotation_inverse
    checks += 2

    involutions = tuple(
        permutation
        for permutation in permutations(range(4))
        if compose(permutation, permutation) == identity
    )
    commuting = tuple(
        conjugation
        for conjugation in involutions
        if compose(conjugation, rotation)
        == compose(rotation, conjugation)
    )
    charge_odd = tuple(
        conjugation
        for conjugation in involutions
        if compose(
            compose(conjugation, rotation), inverse(conjugation)
        )
        == rotation_inverse
    )

    assert len(involutions) == 10
    assert set(commuting) == {identity, (2, 3, 0, 1)}
    assert len(charge_odd) == 4
    assert set(commuting).isdisjoint(charge_odd)
    checks += 4

    # Abstractly, [C,R]=0 and C R C^-1=R^-1 would imply R=R^-1,
    # contradicting the exact order-four clock above.
    for conjugation in commuting:
        conjugated_rotation = compose(
            compose(conjugation, rotation), inverse(conjugation)
        )
        assert conjugated_rotation == rotation
        assert conjugated_rotation != rotation_inverse
        checks += 2
    return checks


def main() -> None:
    checks = verify_total_permutation_sources_and_reciprocity()
    checks += verify_complete_orbit_ledger()
    checks += verify_no_additive_cross_sector_work_invariant()
    checks += verify_single_c4_maxwell_parity_price()

    print("drift: response capacity admits or stalls the material A9 clock")
    print("kick: persistent charge-even tensor source toggles response capacity")
    print("clock: response phase advances on every global tick")
    print("inverse: undo clock, kick, reread permission, undo material drift")
    print("complete feedback census: 16 period-12 sourced orbits, 16 period-4 closed orbits")
    print("sourced orbits: 8/12 admitted proper ticks and 8/12 stress kicks")
    print("additive invariant census: only separately conserved token polarities")
    print("no additive A9 phase/ownership energy can carry cross-sector work")
    print("one C4 orbit cannot carry both a forward clock and charge-odd Maxwell kick")
    print(
        "PASS: C4 stress/capacity reciprocal feedback and Maxwell parity price "
        f"({checks} exact checks)"
    )
    print(
        "Open: variational selection, physical work normalization, distinct "
        "Maxwell/cotangent feedback, propagation, static pole, lensing, Born, alpha"
    )


if __name__ == "__main__":
    main()
