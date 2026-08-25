#!/usr/bin/env python3
"""Exact source increments from one C4 token actualizing on one C18 line.

The existing reversible actualization macro moves one complete oriented C4
token between reserve and bond ownership.  Reading the same ownership change
through the C18 relative-vector, common-phase tensor-doublet, and capacity
moment maps gives an exact shared source vertex.

This is a kinematic finite-record theorem.  It does not derive propagation,
field work, stable matter, lensing, a physical Born selector, or alpha.
"""

from __future__ import annotations

from dataclasses import dataclass

from sympy import Matrix, Rational, sqrt

from proof_c18_uniform_token_blocking import C18_LINE_MOMENTS
from proof_c4_controlled_actualization_transaction import (
    ActualizationState,
    Token,
    actualization_macro,
    charge,
    payload,
    token_count,
)


PHASE_COORDINATES = ((1, 0), (0, 1), (-1, 0), (0, -1))
LINE_DIRECTIONS = (
    Matrix([1, 0, 0]),
    Matrix([0, 1, 0]),
    Matrix([0, 0, 1]),
    Matrix([1 / sqrt(2), 1 / sqrt(2), 0]),
    Matrix([1 / sqrt(2), -1 / sqrt(2), 0]),
    Matrix([1 / sqrt(2), 0, 1 / sqrt(2)]),
    Matrix([1 / sqrt(2), 0, -1 / sqrt(2)]),
    Matrix([0, 1 / sqrt(2), 1 / sqrt(2)]),
    Matrix([0, 1 / sqrt(2), -1 / sqrt(2)]),
)


@dataclass(frozen=True)
class MomentChart:
    relative_u: Matrix
    relative_v: Matrix
    tensor_q: Matrix
    tensor_p: Matrix
    capacity: Matrix
    state_left: int
    state_right: int


def fraction_matrix(moment) -> Matrix:
    xx, yy, zz, xy, xz, yz = moment
    return Matrix(
        [
            [Rational(xx.numerator, xx.denominator), Rational(xy.numerator, xy.denominator), Rational(xz.numerator, xz.denominator)],
            [Rational(xy.numerator, xy.denominator), Rational(yy.numerator, yy.denominator), Rational(yz.numerator, yz.denominator)],
            [Rational(xz.numerator, xz.denominator), Rational(yz.numerator, yz.denominator), Rational(zz.numerator, zz.denominator)],
        ]
    )


LINE_DYADS = tuple(fraction_matrix(moment) for moment in C18_LINE_MOMENTS)


def zero_chart(state: ActualizationState, line_index: int) -> MomentChart:
    """Moment chart for reserve ownership or manifested link ownership."""

    direction = LINE_DIRECTIONS[line_index]
    dyad = LINE_DYADS[line_index]
    if state.link is None:
        return MomentChart(
            Matrix.zeros(3, 1),
            Matrix.zeros(3, 1),
            Matrix.zeros(3, 3),
            Matrix.zeros(3, 3),
            dyad / 9,
            state.state_left,
            state.state_right,
        )

    token = state.link
    u, v = PHASE_COORDINATES[token.phase]
    return MomentChart(
        Rational(token.orientation * u, 9) * direction,
        Rational(token.orientation * v, 9) * direction,
        Rational(u, 18) * dyad,
        Rational(v, 18) * dyad,
        dyad / 18,
        state.state_left,
        state.state_right,
    )


def subtract(left: MomentChart, right: MomentChart) -> MomentChart:
    return MomentChart(
        left.relative_u - right.relative_u,
        left.relative_v - right.relative_v,
        left.tensor_q - right.tensor_q,
        left.tensor_p - right.tensor_p,
        left.capacity - right.capacity,
        left.state_left - right.state_left,
        left.state_right - right.state_right,
    )


def frobenius_squared(matrix: Matrix):
    return sum((entry * entry for entry in matrix), start=Rational(0))


def vector_squared(vector: Matrix):
    return (vector.T * vector)[0]


def main() -> None:
    checks = 0

    for direction, dyad in zip(LINE_DIRECTIONS, LINE_DYADS):
        assert (direction.T * direction)[0] == 1
        assert direction * direction.T == dyad
        checks += 2

    for line_index in range(9):
        direction = LINE_DIRECTIONS[line_index]
        dyad = LINE_DYADS[line_index]
        for phase in range(4):
            u, v = PHASE_COORDINATES[phase]
            for orientation in (-1, 1):
                token = Token(phase, orientation)
                reserve = ActualizationState(0, 0, 0, None, token)
                manifested = actualization_macro(reserve, True)
                assert manifested.link == token and manifested.reserve is None
                assert token_count(manifested) == token_count(reserve) == 1
                assert payload(manifested) == payload(reserve) == token
                assert charge(manifested) == charge(reserve) == 0
                checks += 4

                delta = subtract(
                    zero_chart(manifested, line_index),
                    zero_chart(reserve, line_index),
                )
                assert delta.relative_u == Rational(orientation * u, 9) * direction
                assert delta.relative_v == Rational(orientation * v, 9) * direction
                assert delta.tensor_q == Rational(u, 18) * dyad
                assert delta.tensor_p == Rational(v, 18) * dyad
                assert delta.capacity == -dyad / 18
                assert (delta.state_left, delta.state_right) == (orientation, -orientation)
                checks += 6

                # One finite token fixes the relative normalization of all
                # moment increments; no target coupling enters.
                tensor_phase_norm = frobenius_squared(delta.tensor_q) + frobenius_squared(
                    delta.tensor_p
                )
                capacity_debit_norm = frobenius_squared(delta.capacity)
                relative_phase_norm = vector_squared(delta.relative_u) + vector_squared(
                    delta.relative_v
                )
                assert tensor_phase_norm == capacity_debit_norm == Rational(1, 324)
                assert relative_phase_norm == Rational(1, 81) == 4 * capacity_debit_norm
                # Contracting the source with the bare relative-vector Hessian
                # 135/8 gives a target-free chart cost.  It is not a physical
                # coupling because no propagator or canonical field response
                # has been derived.
                bare_vector_insertion_cost = Rational(135, 8) * relative_phase_norm
                assert bare_vector_insertion_cost == Rational(5, 24)
                checks += 3

                # The same macro is the inverse and negates every increment.
                restored = actualization_macro(manifested, True)
                inverse_delta = subtract(
                    zero_chart(restored, line_index),
                    zero_chart(manifested, line_index),
                )
                assert restored == reserve
                assert inverse_delta.relative_u == -delta.relative_u
                assert inverse_delta.relative_v == -delta.relative_v
                assert inverse_delta.tensor_q == -delta.tensor_q
                assert inverse_delta.tensor_p == -delta.tensor_p
                assert inverse_delta.capacity == -delta.capacity
                assert (inverse_delta.state_left, inverse_delta.state_right) == (
                    -delta.state_left,
                    -delta.state_right,
                )
                checks += 7

                # An incompatible event changes no owned field or state.
                assert actualization_macro(reserve, False) == reserve
                checks += 1

                # C4 rotation acts covariantly on both vector and tensor
                # quadrature doublets while leaving capacity debit fixed.
                rotated_token = Token((phase + 1) % 4, orientation)
                rotated_reserve = ActualizationState(0, 0, 0, None, rotated_token)
                rotated_manifested = actualization_macro(rotated_reserve, True)
                rotated_delta = subtract(
                    zero_chart(rotated_manifested, line_index),
                    zero_chart(rotated_reserve, line_index),
                )
                assert rotated_delta.relative_u == -delta.relative_v
                assert rotated_delta.relative_v == delta.relative_u
                assert rotated_delta.tensor_q == -delta.tensor_p
                assert rotated_delta.tensor_p == delta.tensor_q
                assert rotated_delta.capacity == delta.capacity
                checks += 5

    print(f"PASS: C18 actualization moment-source vertex ({checks} exact checks)")
    print("one token transaction creates neutral ternary endpoints and exact shared moment increments")
    print("||Delta Q||^2+||Delta P||^2=||Delta K||^2=1/324")
    print("||Delta R_u||^2+||Delta R_v||^2=1/81")
    print("bare relative-vector Gaussian insertion cost=(135/8)(1/81)=5/24 (not alpha)")
    print("Open: autonomous compatibility, work, propagation, stable matter, lensing, Born, alpha")


if __name__ == "__main__":
    main()
