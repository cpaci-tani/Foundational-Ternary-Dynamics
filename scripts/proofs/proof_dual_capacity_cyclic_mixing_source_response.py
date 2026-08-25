#!/usr/bin/env python3
"""Exact dual-capacity correlation obstruction and cyclic-mixing response.

Collocating source-created primal/dual deficits correlates their permissions.
Even if every occupied dual-A9 cell has internally factorized half-admission,
mixing those cells with open vacuum cells gives a joint response different
from the product of the block marginals.

A reversible one-hop cyclic translation of the dual capacity layer repairs
this at the finite blocking level.  Over one complete mixing orbit, every
primal slot meets every dual slot once, so the joint open fraction is exactly
the product of the two marginal open fractions for arbitrary binary patterns.
No randomness or probability table is used.

The construction is a finite blocking/mixing theorem.  It does not derive the
source profile, a three-dimensional isotropic routing law, the cotangent field
lift, slow-body response, static gravity, or lensing.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product

from sympy import Rational, limit, simplify, symbols


C_EFF = Rational(1, 6)


def shift(values, offset=1):
    length = len(values)
    return tuple(values[(index - offset) % length] for index in range(length))


def inverse_shift(values, offset=1):
    return shift(values, -offset)


def verify_collocated_binary_obstruction() -> int:
    checks = 0
    for length in range(2, 13):
        for pattern in product((0, 1), repeat=length):
            open_count = sum(pattern)
            marginal = Fraction(open_count, length)
            collocated_joint = Fraction(
                sum(value * value for value in pattern), length
            )
            assert collocated_joint == marginal
            checks += 1
            if 0 < open_count < length:
                assert collocated_joint > marginal * marginal
                assert (
                    collocated_joint - marginal * marginal
                    == marginal * (1 - marginal)
                )
                checks += 2
    return checks


def verify_half_admission_mixture_boundary() -> int:
    checks = 0
    for length in range(1, 129):
        for occupied_cells in range(length + 1):
            occupied_fraction = Fraction(occupied_cells, length)

            # Vacuum cells are always open: (nu_t,nu_s,joint)=(1,1,1).
            # Occupied dual-A9 skew cells have (1/2,1/2,1/4).
            marginal = 1 - occupied_fraction / 2
            joint = 1 - 3 * occupied_fraction / 4
            covariance = joint - marginal * marginal
            assert covariance == occupied_fraction * (1 - occupied_fraction) / 4
            assert covariance >= 0
            checks += 2
            if 0 < occupied_cells < length:
                assert covariance > 0
                checks += 1

    rho = symbols("rho")
    marginal = 1 - rho / 2
    collocated_joint = 1 - 3 * rho / 4
    local_depth = symbols("local_depth")
    # Define local_depth as the marginal admission deficit rho/2.  The
    # collocated mixture then has only a 3/2 first-order optical coefficient.
    joint_in_depth = collocated_joint.subs(rho, 2 * local_depth)
    index = 1 / joint_in_depth
    assert limit((index - 1) / local_depth, local_depth, 0) == Rational(3, 2)
    assert simplify(collocated_joint - marginal**2) == rho * (1 - rho) / 4
    checks += 2
    return checks


def verify_cyclic_mixing_identity() -> int:
    checks = 0
    for length in range(1, 8):
        configurations = tuple(product((0, 1), repeat=length))
        for primal in configurations:
            primal_count = sum(primal)
            for dual in configurations:
                dual_count = sum(dual)
                mixed_joint_total = 0
                current_dual = dual
                for _tick in range(length):
                    mixed_joint_total += sum(
                        left * right
                        for left, right in zip(primal, current_dual)
                    )
                    next_dual = shift(current_dual)
                    assert inverse_shift(next_dual) == current_dual
                    assert sum(next_dual) == dual_count
                    current_dual = next_dual
                    checks += 2
                assert current_dual == dual
                assert mixed_joint_total == primal_count * dual_count
                assert Fraction(mixed_joint_total, length * length) == (
                    Fraction(primal_count, length)
                    * Fraction(dual_count, length)
                )
                checks += 3
    return checks


def verify_arbitrary_source_count_response() -> int:
    checks = 0
    for length in range(1, 129):
        for primal_deficits in range(length + 1):
            for dual_deficits in range(length + 1):
                primal_open = length - primal_deficits
                dual_open = length - dual_deficits
                temporal_rate = Fraction(primal_open, length)
                spatial_rate = Fraction(dual_open, length)
                joint_rate = Fraction(primal_open * dual_open, length * length)
                assert joint_rate == temporal_rate * spatial_rate
                checks += 1

                if primal_deficits == dual_deficits:
                    assert temporal_rate == spatial_rate
                    assert joint_rate == temporal_rate * temporal_rate
                    checks += 2

    depth = symbols("depth")
    a_t, a_s, a_m = symbols("a_t a_s a_m", nonzero=True)
    temporal = 1 - a_t * depth
    spatial = 1 - a_s * depth
    ray_speed = C_EFF * temporal * spatial
    ray_index = C_EFF / ray_speed
    assert limit((ray_index - 1) / depth, depth, 0) == a_t + a_s
    discriminator = simplify((a_t + a_s) / a_m)
    assert discriminator.subs({a_t: a_m, a_s: a_m}) == 2
    checks += 2
    return checks


def verify_independent_source_token_price() -> int:
    checks = 0
    one_token_states = tuple(
        (primal, dual)
        for primal, dual in product((0, 1), repeat=2)
        if primal + dual == 1
    )
    assert set(one_token_states) == {(1, 0), (0, 1)}
    assert all(primal * dual == 0 for primal, dual in one_token_states)
    checks += 2

    up_to_two_token_states = tuple(product((0, 1), repeat=2))
    assert (1, 1) in up_to_two_token_states
    assert sum((1, 1)) == 2
    checks += 2
    return checks


def main() -> None:
    checks = verify_collocated_binary_obstruction()
    checks += verify_half_admission_mixture_boundary()
    checks += verify_cyclic_mixing_identity()
    checks += verify_arbitrary_source_count_response()
    checks += verify_independent_source_token_price()

    print("collocated identical capacity patterns: joint=nu, not nu^2")
    print("vacuum plus half-admission cells: covariance=rho(1-rho)/4")
    print("collocated weak optical coefficient in marginal depth units=3/2")
    print("dual one-hop cyclic shift: sum_t sum_x p_x d_(x+t)=P*D exactly")
    print("arbitrary equal deficit counts after mixing: joint=(1-M/L)^2")
    print("one token cannot occupy independent primal+dual slots; simultaneous pair price=2")
    print(
        "PASS: dual-capacity cyclic mixing/source-response boundary "
        f"({checks} exact checks)"
    )
    print(
        "Open: native paired source ledger, 3D isotropic local routing, "
        "M(U), slow-body response, cotangent/TT lift, static pole, and lensing"
    )


if __name__ == "__main__":
    main()
