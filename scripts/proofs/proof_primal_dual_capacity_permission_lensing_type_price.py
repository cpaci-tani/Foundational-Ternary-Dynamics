#!/usr/bin/env python3
"""Exact permission-idempotence obstruction and primal/dual factor price.

A single retained binary permission cannot supply two independent weak
responses: reading the same bit for temporal admission and spatial transit
gives g*g=g.  This certificate proves that statement on every finite history
through ten ticks and verifies a minimal reversible two-permission lift.

For retained primal/time and dual/space permissions (g_t,g_s), a material
clock advances on g_t while the first-order Maxwell phase advances on
g_t*g_s.  A deterministic reversible L x L product enumerator realizes exact
factorized frequencies without randomness or a target probability table.
Equal marginal admission nu then gives clock rate nu and Maxwell speed
nu^2/6.  In a weak capacity depth this conditionally supplies the temporal
plus spatial response, while leaving native permission generation and the
inhomogeneous Hodge operator open.
"""

from __future__ import annotations

from itertools import product

from sympy import Rational, limit, simplify, symbols


C_EFF = Rational(1, 6)
CLOCK_PERIOD = 8
WAVE_TEST_PERIOD = 17


def apply_pair_history(state, history):
    """Apply the retained controlled clock/spatial test permutation."""
    clock_phase, wave_phase = state
    for time_permission, space_permission in history:
        clock_phase = (clock_phase + time_permission) % CLOCK_PERIOD
        wave_phase = (
            wave_phase + time_permission * space_permission
        ) % WAVE_TEST_PERIOD
    return clock_phase, wave_phase


def reverse_pair_history(state, history):
    """Invert using the retained permissions in reverse order."""
    clock_phase, wave_phase = state
    for time_permission, space_permission in reversed(history):
        clock_phase = (clock_phase - time_permission) % CLOCK_PERIOD
        wave_phase = (
            wave_phase - time_permission * space_permission
        ) % WAVE_TEST_PERIOD
    return clock_phase, wave_phase


def verify_single_permission_idempotence() -> int:
    checks = 0
    for length in range(11):
        for word in product((0, 1), repeat=length):
            temporal_count = sum(word)
            duplicated_read_count = sum(permission * permission for permission in word)
            assert duplicated_read_count == temporal_count
            checks += 1
            if temporal_count:
                # Conditional spatial admission among admitted temporal ticks
                # is exactly one, not a second copy of the marginal fraction.
                assert Rational(duplicated_read_count, temporal_count) == 1
                checks += 1
    return checks


def verify_reversible_two_permission_lift() -> int:
    checks = 0
    pair_alphabet = tuple(product((0, 1), repeat=2))
    for length in range(7):
        for history in product(pair_alphabet, repeat=length):
            time_count = sum(pair[0] for pair in history)
            space_count = sum(pair[1] for pair in history)
            joint_count = sum(pair[0] * pair[1] for pair in history)
            assert 0 <= joint_count <= min(time_count, space_count)
            assert joint_count >= max(0, time_count + space_count - length)
            checks += 2

            for clock_phase in range(CLOCK_PERIOD):
                for wave_phase in range(WAVE_TEST_PERIOD):
                    initial = (clock_phase, wave_phase)
                    final = apply_pair_history(initial, history)
                    assert final == (
                        (clock_phase + time_count) % CLOCK_PERIOD,
                        (wave_phase + joint_count) % WAVE_TEST_PERIOD,
                    )
                    assert reverse_pair_history(final, history) == initial
                    checks += 2
    return checks


def product_enumerator(length: int):
    """One reversible mixed-radix orbit through every permission pair."""
    for pointer in range(length * length):
        yield pointer % length, pointer // length


def verify_exact_factorized_product_enumerator() -> int:
    checks = 0
    for length in range(1, 25):
        pairs = tuple(product_enumerator(length))
        assert len(pairs) == length * length
        assert len(set(pairs)) == length * length
        checks += 2

        # The pointer successor n -> n+1 mod L^2 is a finite permutation; its
        # predecessor is retained exactly.  Threshold subsets are merely a
        # census basis for arbitrary rational marginals, not a fitted law.
        for pointer in range(length * length):
            successor = (pointer + 1) % (length * length)
            predecessor = (successor - 1) % (length * length)
            assert predecessor == pointer
            checks += 1

        for admitted_time_states in range(length + 1):
            for admitted_space_states in range(length + 1):
                time_count = sum(
                    time_state < admitted_time_states
                    for time_state, _ in pairs
                )
                space_count = sum(
                    space_state < admitted_space_states
                    for _, space_state in pairs
                )
                joint_count = sum(
                    time_state < admitted_time_states
                    and space_state < admitted_space_states
                    for time_state, space_state in pairs
                )
                assert time_count == admitted_time_states * length
                assert space_count == admitted_space_states * length
                assert joint_count == admitted_time_states * admitted_space_states
                assert Rational(joint_count, length * length) == (
                    Rational(admitted_time_states, length)
                    * Rational(admitted_space_states, length)
                )
                checks += 4
    return checks


def verify_weak_response_and_lensing_class() -> int:
    checks = 0
    capacity_depth = symbols("capacity_depth")
    a_m, a_t, a_s = symbols("a_m a_t a_s", nonzero=True)

    temporal_admission = 1 - a_t * capacity_depth
    spatial_admission = 1 - a_s * capacity_depth

    # One binary gate read twice remains one gate after blocking.  It supplies
    # only the temporal optical coefficient.
    single_gate_index = 1 / temporal_admission
    assert limit(
        (single_gate_index - 1) / capacity_depth,
        capacity_depth,
        0,
    ) == a_t
    checks += 1

    # Distinct factorized primal/dual permissions multiply at the first-order
    # propagation vertex.
    dual_gate_speed = C_EFF * temporal_admission * spatial_admission
    dual_gate_index = C_EFF / dual_gate_speed
    assert limit(
        (dual_gate_index - 1) / capacity_depth,
        capacity_depth,
        0,
    ) == a_t + a_s
    checks += 1

    single_discriminator = simplify(a_t / a_m)
    dual_discriminator = simplify((a_t + a_s) / a_m)
    assert single_discriminator.subs(a_t, a_m) == 1
    assert dual_discriminator.subs({a_t: a_m, a_s: a_m}) == 2
    checks += 2

    # Equal primal/dual marginal response is a symmetry condition, not a
    # consequence of two-bit existence alone.
    equal_marginal_index = dual_gate_index.subs(a_s, a_t)
    assert limit(
        (equal_marginal_index - 1) / capacity_depth,
        capacity_depth,
        0,
    ) == 2 * a_t
    checks += 1
    return checks


def main() -> None:
    checks = verify_single_permission_idempotence()
    checks += verify_reversible_two_permission_lift()
    checks += verify_exact_factorized_product_enumerator()
    checks += verify_weak_response_and_lensing_class()

    print("one retained binary permission: g^2=g, so no second weak response")
    print("primal/time plus dual/space permissions: clock count=N_t, wave count=N_11")
    print("reversible LxL product orbit: N_11/N=(N_t/N)(N_s/N) exactly")
    print("equal marginal nu: clock rate=nu, Maxwell speed=nu^2/6")
    print("weak equal response: a_0=a_t and a_s=a_t; with a_t=a_m, class=2")
    print(
        "PASS: primal/dual capacity-permission lensing type price "
        f"({checks} exact checks)"
    )
    print(
        "Open: action-generated permissions, primal/dual symmetry, finite "
        "cotangent lift, inhomogeneous interfaces, static pole, and lensing"
    )


if __name__ == "__main__":
    main()
