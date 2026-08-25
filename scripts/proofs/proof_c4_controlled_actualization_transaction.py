#!/usr/bin/env python3
"""Exact checks for a reversible C4-controlled manifestation transaction.

A nondestructive compatibility CNOT arms a local event bit, a controlled
manifestation involution moves one oriented C4 token between reserve and bond
ownership, and the same CNOT disarms the bit.  The palindromic macro is itself
an involution and conserves charge, record payload, capacity complement, and a
positive one-token energy ledger.

This is a microscopic finite-token work certificate.  It does not prove that
the continuous bond Hamiltonian's switching work has been recovered or that
stable matter forms.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import product


@dataclass(frozen=True)
class Token:
    phase: int
    orientation: int

    def __post_init__(self) -> None:
        if self.phase not in range(4):
            raise ValueError("phase must be a C4 exponent")
        if self.orientation not in (-1, 1):
            raise ValueError("orientation must be signed")


@dataclass(frozen=True)
class ActualizationState:
    event_bit: int
    state_left: int
    state_right: int
    link: Token | None
    reserve: Token | None

    def __post_init__(self) -> None:
        if self.event_bit not in (0, 1):
            raise ValueError("event bit must be binary")
        if self.state_left not in (-1, 0, 1) or self.state_right not in (-1, 0, 1):
            raise ValueError("node states must be ternary")


def token_count(state: ActualizationState) -> int:
    return int(state.link is not None) + int(state.reserve is not None)


def capacity(state: ActualizationState) -> int:
    return int(state.link is None)


def charge(state: ActualizationState) -> int:
    return state.state_left + state.state_right


def payload(state: ActualizationState) -> Token | None:
    if state.link is not None and state.reserve is not None:
        return None
    return state.link if state.link is not None else state.reserve


def manifestation_involution(state: ActualizationState) -> ActualizationState:
    """Move one complete token between reserve and manifested bond ownership."""

    if (
        state.state_left == 0
        and state.state_right == 0
        and state.link is None
        and state.reserve is not None
    ):
        token = state.reserve
        return ActualizationState(
            state.event_bit,
            token.orientation,
            -token.orientation,
            token,
            None,
        )

    if (
        state.link is not None
        and state.reserve is None
        and state.state_left == state.link.orientation
        and state.state_right == -state.link.orientation
    ):
        token = state.link
        return ActualizationState(state.event_bit, 0, 0, None, token)

    return state


def compatibility_cnot(state: ActualizationState, compatible: bool) -> ActualizationState:
    return ActualizationState(
        state.event_bit ^ int(compatible),
        state.state_left,
        state.state_right,
        state.link,
        state.reserve,
    )


def controlled_manifestation(state: ActualizationState) -> ActualizationState:
    return manifestation_involution(state) if state.event_bit == 1 else state


def actualization_macro(
    state: ActualizationState,
    compatible: bool,
) -> ActualizationState:
    """Palindromic CNOT-manifest-CNOT transaction with no residual event bit."""

    armed = compatibility_cnot(state, compatible)
    transferred = controlled_manifestation(armed)
    return compatibility_cnot(transferred, compatible)


def rotate_token(token: Token | None, turns: int) -> Token | None:
    if token is None:
        return None
    return Token((token.phase + turns) % 4, token.orientation)


def rotate_state(state: ActualizationState, turns: int) -> ActualizationState:
    return ActualizationState(
        state.event_bit,
        state.state_left,
        state.state_right,
        rotate_token(state.link, turns),
        rotate_token(state.reserve, turns),
    )


def valid_states() -> tuple[ActualizationState, ...]:
    states: list[ActualizationState] = []
    for event_bit, phase, orientation in product((0, 1), range(4), (-1, 1)):
        token = Token(phase, orientation)
        states.append(ActualizationState(event_bit, 0, 0, None, token))
        states.append(
            ActualizationState(
                event_bit,
                orientation,
                -orientation,
                token,
                None,
            )
        )
    return tuple(states)


def main() -> None:
    checks = 0

    for state in valid_states():
        moved = manifestation_involution(state)
        assert manifestation_involution(moved) == state
        assert token_count(moved) == token_count(state) == 1
        assert payload(moved) == payload(state)
        assert charge(moved) == charge(state) == 0
        assert capacity(state) + int(state.link is not None) == 1
        assert capacity(moved) + int(moved.link is not None) == 1
        checks += 6

        for compatible in (False, True):
            output = actualization_macro(state, compatible)
            assert actualization_macro(output, compatible) == state
            assert output.event_bit == state.event_bit
            assert token_count(output) == 1
            assert payload(output) == payload(state)
            assert charge(output) == 0
            checks += 5

            if state.event_bit == 0:
                if compatible:
                    assert output == manifestation_involution(state)
                else:
                    assert output == state
                checks += 1

            for turns in range(4):
                assert actualization_macro(
                    rotate_state(state, turns), compatible
                ) == rotate_state(output, turns)
                checks += 1

    # Collision/backpressure states fail closed and remain exactly invertible.
    tokens = (Token(0, 1), Token(1, -1))
    invalid_states = (
        ActualizationState(0, 0, 0, None, None),
        ActualizationState(0, 0, 0, tokens[0], tokens[1]),
        ActualizationState(0, 1, 1, tokens[0], None),
        ActualizationState(0, -1, 1, tokens[0], None),
    )
    for state in invalid_states:
        assert manifestation_involution(state) == state
        assert actualization_macro(actualization_macro(state, True), True) == state
        checks += 2

    print(f"PASS: C4 controlled actualization transaction ({checks} exact checks)")
    print("Token ledger only: continuous Hamiltonian work and stable matter remain open")


if __name__ == "__main__":
    main()
