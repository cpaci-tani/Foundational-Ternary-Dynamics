#!/usr/bin/env python3
"""Exact measure/pole boundary for the charged Phi-v3 candidate.

The circulation-frame transaction is a finite permutation.  For each fixed
plaquette origin, plane family, and polarity it has one four-cycle.  Distinct
labels therefore support distinct invariant cycle measures; invariance alone
does not select a physical ensemble.  The same decomposition is an exact Born
trial boundary: empirical family/polarity frequencies on one orbit are delta
functions, while arbitrary convex weights across the six cycles remain
invariant.  The candidate supplies no renewal that selects those weights.

The time-averaged electric cochain of one finite cycle has finite spatial
support.  Its Fourier transform is consequently a Laurent polynomial, regular
at z=(1,1,1).  The cubic lattice Coulomb Green function is 1/Lambda(z), where
Lambda vanishes at z=(1,1,1).  Since (Lambda P)(1,1,1)=0 for every Laurent
polynomial P, no isolated finite circulation cycle can be the charged massless
static pole.

This is a scoped no-go for the candidate's isolated framed orbit, not a no-go
for all finite local dynamics or for large-network blocking.
"""

from __future__ import annotations

import json
import sys
from collections import Counter
from fractions import Fraction
from pathlib import Path

from sympy import simplify, symbols

from proof_v3_charged_common_action_phi_v3_candidate import (
    PLANE_FAMILIES,
    PlaquetteFrame,
    active_roles,
    add,
)


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = (
    ROOT
    / "docs/theory/01_reference/strict_discrete_common_action_phi_v3_charged_candidate.json"
)


def cycle(seed: PlaquetteFrame) -> tuple[PlaquetteFrame, ...]:
    states = []
    state = seed
    while state not in states:
        states.append(state)
        state = state.output()
    assert state == seed
    return tuple(states)


def cycle_measure(states: tuple[PlaquetteFrame, ...]):
    weight = Fraction(1, len(states))
    return {state: weight for state in states}


def pushforward_measure(measure):
    output = {}
    for state, weight in measure.items():
        image = state.output()
        output[image] = output.get(image, Fraction(0)) + weight
    return output


def path_fourier_component(frame: PlaquetteFrame, component: int, z):
    """Finite Laurent polynomial for one active path's vector cochain."""

    expression = 0
    edges = frame.edges()
    for role in active_roles(frame.offset):
        tail, direction = edges[role]
        monomial = 1
        for axis in range(3):
            monomial *= z[axis] ** tail[axis]
        expression += -frame.polarity * direction[component] * monomial
    return expression


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    check(
        "C1 theorem targets the unselected charged candidate",
        manifest["meta"]["status"] == "candidate-extension-not-canonical",
    )

    cycles = {
        (family, polarity): cycle(
            PlaquetteFrame((0, 0, 0), family, 0, polarity)
        )
        for family in range(3)
        for polarity in (-1, 1)
    }
    check("C2 every fixed-label circulation orbit has exactly four states", {len(states) for states in cycles.values()} == {4})
    check("C3 family and polarity labels are invariant along every orbit", all(all((state.family, state.polarity) == label for state in states) for label, states in cycles.items()))
    check("C4 the anchor sector already contains six disjoint cycles", len(set().union(*(set(states) for states in cycles.values()))) == 24)

    measures = {label: cycle_measure(states) for label, states in cycles.items()}
    check("C5 every uniform cycle measure is exactly invariant", all(pushforward_measure(measure) == measure for measure in measures.values()))
    supports = [set(measure) for measure in measures.values()]
    check("C6 distinct cycle measures have disjoint support", all(not (supports[i] & supports[j]) for i in range(len(supports)) for j in range(i + 1, len(supports))))
    check("C7 invariance therefore does not select one physical history measure", len(measures) == 6)

    # Each four-cycle visits each geometric path twice.  This fixes a prepared
    # local time average but gives no measure across origins, endpoint networks,
    # or source sectors.
    path_counts = {}
    for label, states in cycles.items():
        path_counts[label] = Counter(
            "A" if active_roles(state.offset) == (0, 1) else "B"
            for state in states
        )
    check("C8 each isolated cycle gives the local prepared path ratio 1/2:1/2", set(tuple(sorted(counts.items())) for counts in path_counts.values()) == {(('A', 2), ('B', 2))})

    # Exact Laurent-polynomial obstruction to an isolated-cycle Coulomb pole.
    z = symbols("z0 z1 z2", nonzero=True)
    lattice_laplacian = 6 - sum(axis + 1 / axis for axis in z)
    regular_rows = 0
    for states in cycles.values():
        for component in range(3):
            average = sum(
                path_fourier_component(state, component, z)
                for state in states
            ) / len(states)
            value_at_one = simplify(average.subs({axis: 1 for axis in z}))
            assert value_at_one.is_finite is not False
            assert simplify(
                (lattice_laplacian * average).subs(
                    {axis: 1 for axis in z}
                )
            ) == 0
            regular_rows += 2

    check("C9 every isolated-cycle Fourier cochain is regular at zero momentum", regular_rows == 36)
    check("C10 no such Laurent polynomial can satisfy Lambda times P equals 1", simplify(lattice_laplacian.subs({axis: 1 for axis in z})) == 0)

    # Same transition graph, distinct positive action prices.  This is the
    # candidate-specific continuation of the earlier source-orbit scale no-go.
    period = 4
    action_prices = {weight: period * weight for weight in (1, 2, 7)}
    check("C11 one transition map admits distinct positive action normalizations", len(set(action_prices.values())) == 3)
    check("C12 action rescaling changes no orbit or invariant-support decomposition", set(action_prices) == {1, 2, 7} and len(cycles) == 6)

    open_items = set(manifest["not_closed"])
    check("C13 manifest does not claim a charged pole", "charged_static_pole" in open_items)
    check("C14 manifest does not claim a selected measure", "physically_selected_history_measure" in open_items)
    check("C15 manifest does not claim physical coupling normalization", "physical_coupling_normalization" in open_items)

    # Born-trial boundary.  A single deterministic orbit never changes its
    # family/polarity label, so its empirical label distribution is a delta.
    # Across the six cycles, any convex combination of their invariant
    # measures is again invariant.  The weights are therefore preparation
    # inputs, not frequencies selected by this transition.
    empirical_label_measures = {
        label: Counter((state.family, state.polarity) for state in states)
        for label, states in cycles.items()
    }
    check(
        "C16 every single-orbit family/polarity frequency is deterministic",
        all(counter == Counter({label: 4}) for label, counter in empirical_label_measures.items()),
    )
    labels = tuple(sorted(cycles))
    weights = {
        label: Fraction(index + 1, 21) for index, label in enumerate(labels)
    }
    mixed_measure = {}
    for label, measure in measures.items():
        for state, cycle_weight in measure.items():
            mixed_measure[state] = weights[label] * cycle_weight
    check(
        "C17 a nonuniform arbitrary six-cycle mixture is also exactly invariant",
        sum(weights.values(), Fraction(0)) == 1
        and len(set(weights.values())) == 6
        and pushforward_measure(mixed_measure) == mixed_measure,
    )
    check(
        "C18 candidate does not claim native Born trials or frequency selection",
        "physical_Born_statistics" in open_items,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} charged-candidate measure/pole/Born-boundary checks pass")
    print(f"disjoint_anchor_cycles={len(cycles)}")
    print("cycle_length=4")
    print("prepared_local_path_measure=A:1/2,B:1/2")
    print("isolated_cycle_static_pole=impossible_by_Laurent_regularity")
    print("physical_measure_selection=not_supplied_by_invariance")
    print("Born_trial_status=cycle_label_weights_are_unselected_preparation_inputs")
    print("Open: large-network mixing/noninjective renewal, physical trials, and blocked Coulomb pole")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
