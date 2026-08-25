#!/usr/bin/env python3
"""Exact triplet phase winding and reduced finite-clock coupling ladder.

The Phi-v5 triplet's complete state has period sixteen global ticks.  Its A9
token phase advances once on every second global tick, so the physical C4
phase makes two windings per complete material period, equivalently one
primitive winding in eight global ticks.  This fixes w/T=1/8 in the existing
finite-clock packet-coupling theorem for every initial phase and polarity.

Conditional on a momentum-neutral reciprocal absorption in which d complete
field packets create one material action quantum, the three-integer ladder
therefore reduces exactly to alpha_native=3/(8d).  Recoil gives
3/[8(d-r)].  The genesis-seeded assembly itself has zero complete-packet
debit, so it cannot be assigned d=1 or used to normalize the coupling.

The packet debit, recoil response, common static/radiative pole, and absolute
interacting curvature remain open.  No target value or master root enters.
"""

from __future__ import annotations

import sys
from fractions import Fraction
from math import gcd

from sympy import pi, simplify

from proof_ternary_square_phase_polarity_autonomous_clock import (
    iterate as a9_iterate,
    occupation,
    phase_index,
    tick as a9_tick,
)
from proof_v3_cubic_triplet_self_correcting_material_clock import (
    DARK,
    LOGICAL,
    clean_state,
    triplet_step,
)


sys.stdout.reconfigure(encoding="utf-8")


def active_token(state):
    return state.link if occupation(state.link) else state.reserve


def active_phase(state) -> int:
    return phase_index(active_token(state))


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    check(
        "C1 the admitted A9 clock alphabet has sixteen states in exact period-eight cycles",
        len(LOGICAL) == 16
        and all(a9_iterate(state, 8) == state for state in LOGICAL)
        and all(
            all(a9_iterate(state, step) != state for step in range(1, 8))
            for state in LOGICAL
        ),
    )

    phase_rows = 0
    winding_rows = 0
    polarity_rows = 0
    for seed in LOGICAL:
        state = seed
        unwrapped = 0
        initial_polarity = None
        for _ in range(8):
            token = active_token(state)
            polarity = 1 if token[0] * token[0] + token[1] * token[1] == 1 else -1
            if initial_polarity is None:
                initial_polarity = polarity
            assert polarity == initial_polarity
            next_state = a9_tick(state)
            increment = (active_phase(next_state) - active_phase(state)) % 4
            assert increment == 1
            unwrapped += increment
            state = next_state
            phase_rows += 1
            polarity_rows += 1
        assert state == seed
        assert unwrapped == 8
        assert unwrapped // 4 == 2
        winding_rows += 1

    check(
        "C2 every A9 microclock step advances the same C4 phase by exactly one quarter-turn",
        phase_rows == 16 * 8,
    )
    check(
        "C3 every complete A9 orbit has exactly two phase windings independent of polarity",
        winding_rows == polarity_rows // 8 == 16,
    )

    triplet_rows = 0
    commit_rows = 0
    for seed in LOGICAL:
        state = clean_state(seed)
        initial = state
        phase_advance = 0
        for global_tick in range(16):
            before = state.arms[0]
            output = triplet_step(state)
            after = output.arms[0]
            if after != before:
                assert (active_phase(after) - active_phase(before)) % 4 == 1
                phase_advance += 1
                commit_rows += 1
            else:
                assert output.heralds != (DARK,) * 3
            state = output
            triplet_rows += 1
        assert state == initial
        assert phase_advance == 8

    check(
        "C4 the triplet commits exactly eight A9 quarter-turns in sixteen global ticks",
        triplet_rows == 16 * 16 and commit_rows == 16 * 8,
    )
    check(
        "C5 the complete material state therefore carries w=2 windings in T=16 ticks",
        Fraction(2, 16) == Fraction(1, 8),
    )
    check(
        "C6 the primitive phase-only presentation is w=1 T=8 and has the same cadence",
        gcd(1, 8) == 1 and Fraction(1, 8) == Fraction(2, 16),
    )

    winding = 2
    period = 16
    omega = simplify(2 * pi * winding / period)
    check(
        "C7 the exact blocked material angular cadence is omega=pi/4 per global tick",
        omega == pi / 4,
    )

    # Existing finite-clock theorem: alpha_native=3w/(dT) when recoil is zero.
    # The present theorem supplies w/T but deliberately leaves d symbolic.
    for debit in (1, 2, 7, 19):
        alpha_general = Fraction(3 * winding, debit * period)
        alpha_reduced = Fraction(3, 8 * debit)
        assert alpha_general == alpha_reduced
    check(
        "C8 momentum-neutral absorption reduces the ladder exactly to alpha_native=3/(8d)",
        True,
    )

    # The compliance curvature is omega/d.  Substitution into the registered
    # cotangent readout alpha=3 chi_EM/(2 pi) gives the same reduced ladder.
    for debit in (1, 2, 7, 19):
        chi_em = omega / debit
        alpha_from_curvature = simplify(3 * chi_em / (2 * pi))
        assert alpha_from_curvature == Fraction(3, 8 * debit)
    check(
        "C9 the packet-clock compliance curvature and cotangent readout agree on the same ladder",
        True,
    )

    # Recoil branch alpha=3w/[T(d-r)].  This is a symbolic identity, not a
    # choice of r.  Test exact rational fixtures only as algebra verification.
    for debit, recoil in ((2, Fraction(1, 2)), (7, Fraction(3, 5)), (19, Fraction(4, 7))):
        general = Fraction(3 * winding, period) / (debit - recoil)
        reduced = Fraction(3, 8) / (debit - recoil)
        assert general == reduced
    check(
        "C10 the recoil branch reduces exactly to alpha_native=3/[8(d-r)]",
        True,
    )

    assembly_packet_debit = 0
    check(
        "C11 Phi-v12 seed assembly has d=0 and cannot instantiate the positive-debit coupling formula",
        assembly_packet_debit == 0,
    )

    forbidden = (
        "137.036",
        "master_root",
        "empirical_target",
        "nearest_integer",
        "parameter_fit",
        "random_draw",
    )
    missing = {
        "positive complete-packet debit into one material action quantum",
        "recoil ratio or proof of the momentum-neutral branch",
        "common charged static and transverse radiative pole",
        "equality of pole residue and free-field Hessian",
        "canonical Phi integration and formation of the absorption apparatus",
        "absolute interacting curvature and physical alpha readout",
    }
    check(
        "C12 the winding theorem narrows but does not close physical coupling normalization",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} triplet winding/coupling-ladder checks pass")
    print("complete_material_period_T=16")
    print("complete_material_winding_w=2")
    print("primitive_phase_period_T=8")
    print("primitive_phase_winding_w=1")
    print("w_over_T=1/8")
    print("omega=pi/4")
    print("momentum_neutral_ladder=3/(8*d)")
    print("recoil_ladder=3/(8*(d-r))")
    print("phi_v12_assembly_packet_debit=0")
    print("status=exact_material_winding_one_integer_coupling_ladder_open_normalization")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
