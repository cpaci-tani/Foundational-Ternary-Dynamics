#!/usr/bin/env python3
"""Exact phase-neutral shared charge/stress actualization vertex.

The C4 selection theorem forbids a vacuum linear intertwiner between the
phase-independent Maxwell carrier and the tensor quadratures.  The existing
actualization transaction nevertheless supplies the allowed matter-mediated
bridge.  Contract each vector/tensor source doublet with the manifested
token's own phase covector (u,v).  For every C18 line, phase, and orientation:

    j_event = u Delta R_u + v Delta R_v = epsilon d / 9,
    t_event = u Delta Q   + v Delta P   = dd^T / 18 = -Delta K.

The current is C4 invariant and charge odd.  The tensor/capacity source is C4
invariant and charge even.  Their exact norm ledger obeys

    j j^T = dd^T / 81 = 4 t^2.

The orthogonal phase contractions vanish, so no second source coefficient is
hidden in the construction.  The inverse transaction negates every signed
source increment.

This is a reversible kinematic common source vertex, not reciprocal field
work, a Maxwell or spin-2 pole, gravity, lensing, Born, or native alpha.
"""

from __future__ import annotations

from sympy import Matrix, Rational

from proof_c18_actualization_moment_source_vertex import (
    LINE_DIRECTIONS,
    LINE_DYADS,
    PHASE_COORDINATES,
    subtract,
    zero_chart,
)
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
)


def phase_neutral_sources(delta, phase: int):
    u, v = PHASE_COORDINATES[phase]
    current = u * delta.relative_u + v * delta.relative_v
    tensor = u * delta.tensor_q + v * delta.tensor_p
    vector_cross = u * delta.relative_v - v * delta.relative_u
    tensor_cross = u * delta.tensor_p - v * delta.tensor_q
    return current, tensor, vector_cross, tensor_cross


def event_delta(line_index: int, phase: int, orientation: int):
    token = Token(phase, orientation)
    reserve = ActualizationState(0, 0, 0, None, token)
    manifested = actualization_macro(reserve, True)
    delta = subtract(
        zero_chart(manifested, line_index),
        zero_chart(reserve, line_index),
    )
    return token, reserve, manifested, delta


def main() -> None:
    checks = 0
    for line_index, (direction, dyad) in enumerate(
        zip(LINE_DIRECTIONS, LINE_DYADS)
    ):
        for phase in range(4):
            next_phase = (phase + 1) % 4
            for orientation in (-1, 1):
                token, reserve, manifested, delta = event_delta(
                    line_index, phase, orientation
                )
                current, tensor, vector_cross, tensor_cross = (
                    phase_neutral_sources(delta, phase)
                )

                assert current == Rational(orientation, 9) * direction
                assert tensor == dyad / 18
                assert tensor == -delta.capacity
                assert vector_cross == Matrix.zeros(3, 1)
                assert tensor_cross == Matrix.zeros(3, 3)
                assert current * current.T == dyad / 81
                assert tensor * tensor == dyad / 324
                assert current * current.T == 4 * tensor * tensor
                assert (delta.state_left, delta.state_right) == (
                    orientation,
                    -orientation,
                )
                checks += 9

                # Charge conjugation reverses the directed current and
                # endpoint pair but preserves tensor/capacity sourcing.
                _ct, _cr, _cm, conjugate_delta = event_delta(
                    line_index, phase, -orientation
                )
                conjugate_sources = phase_neutral_sources(
                    conjugate_delta, phase
                )
                assert conjugate_sources[0] == -current
                assert conjugate_sources[1] == tensor
                assert conjugate_delta.capacity == delta.capacity
                assert (
                    conjugate_delta.state_left,
                    conjugate_delta.state_right,
                ) == (-delta.state_left, -delta.state_right)
                checks += 4

                # A global C4 phase advance rotates both the token covector
                # and source doublets, leaving their dot contractions fixed.
                _rt, _rr, _rm, rotated_delta = event_delta(
                    line_index, next_phase, orientation
                )
                rotated_sources = phase_neutral_sources(
                    rotated_delta, next_phase
                )
                assert rotated_sources[0] == current
                assert rotated_sources[1] == tensor
                assert rotated_sources[2] == vector_cross
                assert rotated_sources[3] == tensor_cross
                checks += 4

                # The inverse ownership transfer negates every linear source
                # increment while returning the same token to reserve.
                restored = actualization_macro(manifested, True)
                inverse_delta = subtract(
                    zero_chart(restored, line_index),
                    zero_chart(manifested, line_index),
                )
                inverse_sources = phase_neutral_sources(inverse_delta, phase)
                assert restored == reserve
                assert inverse_sources[0] == -current
                assert inverse_sources[1] == -tensor
                assert inverse_delta.capacity == -delta.capacity
                assert (inverse_delta.state_left, inverse_delta.state_right) == (
                    -delta.state_left,
                    -delta.state_right,
                )
                assert restored.reserve == token
                checks += 6

    print("phase-neutral event current: j=epsilon*d/9 (charge odd)")
    print("phase-neutral event tensor: t=dd^T/18=-DeltaK (charge even)")
    print("orthogonal C4 contractions vanish exactly")
    print("source ledger: j*j^T=dd^T/81=4*t^2")
    print("global C4 advance leaves j and t invariant")
    print("inverse actualization negates every signed source increment")
    print(
        "PASS: C18 phase-neutral shared charge/stress vertex "
        f"({checks} exact checks)"
    )
    print(
        "Open here: physical work, Maxwell backreaction, propagating constrained "
        "poles, stable composites, lensing, physical Born preparation, and alpha"
    )


if __name__ == "__main__":
    main()
