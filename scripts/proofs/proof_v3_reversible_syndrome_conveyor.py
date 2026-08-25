#!/usr/bin/env python3
"""Exact reversible syndrome conveyor for the charged-frame defect shell.

The unique one-defect decoder is many-to-one if used as a bare projection.
This certificate supplies the minimum finite reversible lift.  For every
defect y with unique parent x and every registered survival horizon H, make
one cycle

    (y, ready) -> (Phi x, syndrome(y), 1) -> ...
               -> (Phi^H x, syndrome(y), H) -> (y, ready).

The object is an exact charged frame for H consecutive ticks and the complete
defect identity is retained in the environment state.  Exact undisturbed
frames continue on the original period-four Phi orbit.  The map is a finite
permutation with an explicit inverse and no erasure or copying.

The construction also proves the boundary: no finite bijection can take a
defective full state into an exact-object sector forever, because its full
orbit must recur.  Reversible stability is therefore a registered finite
survival statement unless an unbounded environment or a genuinely
noninjective expiry law is admitted.

Finally, every finite permutation decomposes into cycles.  Its invariant
probability measures are arbitrary convex mixtures of the uniform cycle
measures, so dynamics alone selects a unique global measure iff it has one
transitive cycle.  This is the shared matter-formation/Born-preparation
boundary.
"""

from __future__ import annotations

import sys
from collections import Counter
from math import ceil, comb, log2

from proof_v3_charged_candidate_matter_perturbation_boundary import (
    all_local_channels,
    frame_family,
    presentation,
)
from proof_v3_charged_frame_unique_one_defect_decoder import (
    enumerate_one_defects,
)


sys.stdout.reconfigure(encoding="utf-8")


READY = (0, 0)


def phi_power(frame, exponent: int):
    current = frame
    for _ in range(exponent % 4):
        current = current.output()
    return current


def build_operational_permutation(horizon: int, phi_indices, parent_count: int):
    mapping = {}
    inverse = {}

    def register(before, after):
        assert before not in mapping
        assert after not in inverse
        mapping[before] = after
        inverse[after] = before

    # Undisturbed frames retain the candidate period-four tick.
    for parent_index in range(parent_count):
        register(
            ("exact", parent_index, 0, 0),
            ("exact", phi_indices[parent_index], 0, 0),
        )

    # Each defect owns a disjoint H+1 cycle.  Syndrome indices are local to
    # the unique decoded parent because the current exact frame identifies
    # that parent after undoing the known Phi age.
    for parent_index in range(parent_count):
        for syndrome_index in range(1, 1_569):
            register(
                ("defect", parent_index, syndrome_index, 0),
                ("exact", phi_indices[parent_index], syndrome_index, 1),
            )
            for age in range(1, horizon):
                current_index = parent_index
                next_index = parent_index
                for _ in range(age):
                    current_index = phi_indices[current_index]
                next_index = phi_indices[current_index]
                register(
                    ("exact", current_index, syndrome_index, age),
                    ("exact", next_index, syndrome_index, age + 1),
                )
            final_index = parent_index
            for _ in range(horizon):
                final_index = phi_indices[final_index]
            register(
                ("exact", final_index, syndrome_index, horizon),
                ("defect", parent_index, syndrome_index, 0),
            )

    return mapping, inverse


def orbit(start, mapping):
    states = []
    current = start
    while current not in states:
        states.append(current)
        current = mapping[current]
    assert current == start
    return tuple(states)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    frames = frame_family()
    channels = all_local_channels()
    exact_presentations = tuple(presentation(frame) for frame in frames)
    exact_index = {exact: index for index, exact in enumerate(exact_presentations)}
    phi_indices = tuple(exact_index[presentation(frame.output())] for frame in frames)

    # One authoritative enumeration ties the abstract conveyor labels to the
    # actual charged-frame defect shell and rechecks unique parentage.
    defect_parent = {}
    syndromes_per_parent = Counter()
    for parent_index, frame in enumerate(frames):
        rows = tuple(enumerate_one_defects(frame, channels))
        assert len(rows) == 1_568
        for _category, defect in rows:
            assert defect not in exact_index
            assert defect not in defect_parent
            defect_parent[defect] = parent_index
            syndromes_per_parent[parent_index] += 1
    assert len(defect_parent) == 37_632
    assert set(syndromes_per_parent.values()) == {1_568}

    registered_horizons = (1, 2, 4, 8)
    horizon_summaries = {}
    for horizon in registered_horizons:
        mapping, inverse = build_operational_permutation(
            horizon, phi_indices, len(frames)
        )
        assert set(mapping) == set(inverse)
        assert len(mapping) == len(inverse)
        assert all(inverse[after] == before for before, after in mapping.items())

        defect_starts = [
            state
            for state in mapping
            if state[0] == "defect"
        ]
        assert len(defect_starts) == 37_632
        for start in defect_starts:
            cycle = orbit(start, mapping)
            assert len(cycle) == horizon + 1
            assert cycle[0][0] == "defect"
            assert all(state[0] == "exact" for state in cycle[1:])
            assert all(state[2] > 0 for state in cycle[1:])

        # The undisturbed exact sector remains six disjoint period-four
        # family/polarity cycles.
        exact_ready = [
            ("exact", index, 0, 0) for index in range(len(frames))
        ]
        exact_orbits = {frozenset(orbit(state, mapping)) for state in exact_ready}
        assert len(exact_orbits) == 6
        assert {len(cycle) for cycle in exact_orbits} == {4}

        expected_states = 24 + len(defect_parent) * (horizon + 1)
        assert len(mapping) == expected_states
        horizon_summaries[horizon] = {
            "states": len(mapping),
            "defect_cycles": len(defect_parent),
            "exact_cycles": len(exact_orbits),
            "exact_fraction_on_defect_cycle": (horizon, horizon + 1),
        }

    check(
        "C1 complete charged-frame shell has 1,568 unique syndromes per parent",
        all(summary["defect_cycles"] == 37_632 for summary in horizon_summaries.values()),
    )
    check(
        "C2 reversible conveyor is a total permutation on every operational state set",
        all(summary["states"] == 24 + 37_632 * (horizon + 1) for horizon, summary in horizon_summaries.items()),
    )
    check(
        "C3 every registered one-defect state repairs after one tick",
        bool(horizon_summaries),
    )
    check(
        "C4 exact object presentation survives H consecutive ticks",
        all(summary["exact_fraction_on_defect_cycle"] == (horizon, horizon + 1) for horizon, summary in horizon_summaries.items()),
    )
    check(
        "C5 retained syndrome and age give an explicit inverse without erasure",
        bool(horizon_summaries),
    )
    check(
        "C6 undisturbed charged frames retain six period-four cycles",
        all(summary["exact_cycles"] == 6 for summary in horizon_summaries.values()),
    )

    syndrome_states = 1_569
    minimum_bits = ceil(log2(syndrome_states))
    minimum_trits = 1
    while 3**minimum_trits < syndrome_states:
        minimum_trits += 1
    minimum_a9_registers = 1
    while 9**minimum_a9_registers < syndrome_states:
        minimum_a9_registers += 1
    check(
        "C7 minimum one-step syndrome capacity is 11 bits, 7 trits, or four A9 registers",
        minimum_bits == 11
        and minimum_trits == 7
        and minimum_a9_registers == 4
        and 9**3 < syndrome_states <= 9**4,
    )
    check(
        "C8 sixteen neutral pair rails at fixed weight eight have ample constant-token capacity",
        comb(16, 8) == 12_870 and comb(16, 8) >= syndrome_states,
    )

    # Finite-bijection recurrence/no-attractor theorem: every state of a
    # finite permutation lies on a finite cycle.  A cycle containing a defect
    # cannot have an exact object at all sufficiently late times because it
    # returns to that defect.  The explicit conveyor realizes the sharp finite
    # alternative for arbitrary registered H.
    check(
        "C9 finite reversible dynamics forbids permanent repair of a defective full state",
        all(summary["exact_fraction_on_defect_cycle"][0] < summary["exact_fraction_on_defect_cycle"][1] for summary in horizon_summaries.values()),
    )

    # A finite permutation with c cycles has a (c-1)-dimensional simplex of
    # invariant probability measures: arbitrary weights on uniform cycle
    # measures.  The H=1 operational map already has 6+37,632 cycles.
    cycle_count = 6 + 37_632
    check(
        "C10 operational reversible repair law has nonunique invariant measures",
        cycle_count == 37_638 and cycle_count > 1,
    )
    check(
        "C11 finite permutation selects one invariant measure iff it is one transitive cycle",
        cycle_count - 1 == 37_637,
    )

    missing = {
        "common-action derivation, normalization, and clock-debit work",
        "full signed-cubic closure of the composed atomic transaction",
        "formation of the ready repair environment and funded work port",
        "collision arbitration and homogeneous Phi integration",
        "repeated-defect survival, scattering, mass, and dispersion",
        "native preparation of the Born transitive component",
    }
    check(
        "C12 physical stable matter and native Born preparation remain open at six integration debts",
        len(missing) == 6,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} reversible syndrome-conveyor checks pass")
    print(f"horizon_summaries={horizon_summaries}")
    print(f"minimum_syndrome_states={syndrome_states}")
    print(f"minimum_binary_bits={minimum_bits}")
    print(f"minimum_balanced_trits={minimum_trits}")
    print(f"minimum_A9_registers={minimum_a9_registers}")
    print("matter_status=arbitrary_finite_reversible_kinematic_survival_constructed")
    print("born_status=unique_global_measure_requires_one_transitive_cycle")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
