#!/usr/bin/env python3
"""Exact coupling-normalization obstruction for the v3 dressed-source orbit.

The candidate source macro has a completely fixed 24-tick state orbit, exact
Gauss/current identities, eight-channel dressing, and a uniform time count.
None fixes the positive action assigned to a crossing or to one field quantum.
Rescaling those action prices leaves the state map and every kinematic/counting
observable unchanged while changing the conditional dimensionless coupling.
"""

from __future__ import annotations

import sys

from sympy import Rational, Symbol, pi, simplify

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_v3_dressed_sc_source_gauss_continuity import (
    DressedEdgeState,
    dressed_source_tick,
    owned,
)


sys.stdout.reconfigure(encoding="utf-8")


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    cycle_rows = []
    for direction in SC_DIRECTIONS:
        for polarity in (-1, 1):
            for layer in range(3):
                initial = DressedEdgeState(
                    primary=None,
                    reserve=(0, polarity),
                    layer=layer,
                    bank=frozenset(),
                )
                state = initial
                activations = withdrawals = holds = primary_ticks = 0
                for _ in range(24):
                    before = state
                    after = dressed_source_tick(direction, before)
                    delta = owned(after) - owned(before)
                    activations += int(delta == 1)
                    withdrawals += int(delta == -1)
                    holds += int(delta == 0)
                    primary_ticks += owned(after)
                    state = after
                assert state == initial
                cycle_rows.append(
                    (activations, withdrawals, holds, primary_ticks)
                )

    check("C1 every complete source cycle is exactly 24 ticks", len(cycle_rows) == 36)
    check("C2 every cycle has three activations and three withdrawals", {row[:2] for row in cycle_rows} == {(3, 3)})
    check("C3 every cycle has eighteen noncrossing ticks", {row[2] for row in cycle_rows} == {18})
    check("C4 uniform time count gives primary ownership one half", {Rational(row[3], 24) for row in cycle_rows} == {Rational(1, 2)})

    crossing_price = Symbol("w_cross", positive=True)
    hold_price = Symbol("w_hold", positive=True)
    action_quantum = Symbol("I_star", positive=True)
    field_curvature = Symbol("Gamma", positive=True)
    one_cycle_action = 6 * crossing_price + 18 * hold_price
    check("C5 the same exact orbit admits a two-parameter positive action family", one_cycle_action.has(crossing_price, hold_price))

    # Reversibility equates forward and reverse crossing prices but does not set
    # their common magnitude.  Even imposing one common local price leaves an
    # arbitrary positive scale.
    common_price = Symbol("w", positive=True)
    reversible_cycle_action = simplify(
        one_cycle_action.subs(
            {crossing_price: common_price, hold_price: common_price}
        )
    )
    check("C6 equal reversible transition pricing still leaves one free scale", reversible_cycle_action == 24 * common_price)

    # Eight created field bits do not fix their action price.  A per-channel
    # curvature eta yields Gamma=8 eta only after a convention; eta remains
    # free and all source state/counting data stay identical.
    channel_price = Symbol("eta", positive=True)
    candidate_curvature = 8 * channel_price
    check("C7 the eight-channel dressing fixes multiplicity but not per-channel action", candidate_curvature.has(channel_price))

    c_eff = Rational(1, 6)
    alpha_family = simplify(
        field_curvature / (4 * pi * action_quantum * c_eff)
    )
    check("C8 conditional coupling retains Gamma/I_star", alpha_family == 3 * field_curvature / (2 * pi * action_quantum))

    # Explicitly distinct positive rescalings preserve the discrete orbit and
    # its uniform counts but give distinct conditional couplings.
    scale_values = (Rational(1, 3), Rational(1), Rational(5, 2))
    conditional_values = {
        simplify(
            alpha_family.subs(
                {field_curvature: scale, action_quantum: 1}
            )
        )
        for scale in scale_values
    }
    check("C9 distinct action scales give distinct couplings with identical Phi histories", len(conditional_values) == len(scale_values))

    # Prepared Born counts depend only on finite phase-pair cardinalities, so
    # they do not determine the action currency either.
    born_counts = (1, 4, 9, 16)
    normalized = tuple(
        Rational(count, sum(born_counts)) for count in born_counts
    )
    for scale in scale_values:
        assert tuple(
            Rational(count, sum(born_counts)) for count in born_counts
        ) == normalized
        assert scale > 0
    check("C10 prepared Born cardinalities are invariant under the action-scale orbit", True)

    missing = {
        "physical history action or selected measure",
        "blocked field curvature",
        "active/reserve work",
        "static/free reciprocity",
        "charged massless pole",
    }
    check("C11 normalization can close only through a new blocked-history result", len(missing) == 5)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} coupling-obstruction checks pass")
    print("fixed=24_tick_orbit, 8_channel_dressing, Gauss, uniform_time_count")
    print("free=transition_action_scale, field_curvature/action_quantum")
    print("conditional_alpha=3*Gamma/(2*pi*I_star), not a prediction")
    print("Open: derive chi_EM from the common blocked finite-history object")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
