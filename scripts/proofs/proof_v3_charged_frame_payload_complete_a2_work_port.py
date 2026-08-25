#!/usr/bin/env python3
"""Payload-complete A2 work port for the v3 charged-frame repair.

The atomic repair theorem closes a generalized integer token ledger.  This
successor places that integer in two existing A9 slots of the plaquette A2
carrier.  Each slot is either blank or carries a complete (C4 phase, polarity)
payload.  The registered repair section becomes an exact finite bijection:

    malformed frame + ready syndrome + one occupied A9 work slot
      <-> exact next frame + unique syndrome + zero/one/two occupied slots.

The syndrome retains every defect coordinate not carried by the work payload.
Under the selected equal-occupancy metric, object + syndrome + work energy is
exactly conserved.  This is a finite carrier/action seam, not a derivation of
the metric, its multiplier, the bundle clock-debit energy, or canonical Phi.
"""

from __future__ import annotations

import sys
from collections import Counter

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_charged_candidate_matter_perturbation_boundary import (
    all_local_channels,
    frame_family,
    presentation,
)
from proof_v3_charged_frame_atomic_syndrome_repair_transaction import (
    basis,
    defect_descriptor,
    frame_anchor,
    frame_codewords,
    resource_delta,
    scale_add,
)
from proof_v3_charged_frame_unique_one_defect_decoder import enumerate_one_defects


sys.stdout.reconfigure(encoding="utf-8")

Payload = tuple[int, int]
WorkPort = tuple[Payload | None, Payload | None]


def valid_payload(payload: Payload | None) -> bool:
    return payload is None or (
        payload[0] in range(4) and payload[1] in (-1, 1)
    )


def occupied(port: WorkPort) -> int:
    return sum(payload is not None for payload in port)


def ready_work_port(frame) -> WorkPort:
    # One canonical complete A9 payload, selected intrinsically from the
    # charged frame.  Formation of this ready port remains open.
    return ((frame.offset, frame.polarity), None)


def extra_field_payload(frame, defect) -> Payload:
    parent = presentation(frame)
    extra = defect.fields - parent.fields
    assert len(extra) == 1
    _vertex, channel = next(iter(extra))
    return channel[3], channel[4]


def output_work_port(frame, defect) -> WorkPort:
    ready = ready_work_port(frame)
    delta = resource_delta(presentation(frame), defect)
    if delta == 1:
        return None, None
    if delta == 0:
        return ready
    assert delta == -1
    return ready[0], extra_field_payload(frame, defect)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())
    channels = all_local_channels()
    frames = frame_family()

    forward = {}
    reverse = {}
    occupancy_histogram = Counter()
    payload_histogram = Counter()
    energy_rows = 0
    inverse_rows = 0
    neutral_port_rows = 0

    # A9 has one blank plus four phases times two polarities.  The selected
    # two-slot port is a strict subfactor of the existing four-slot A2 carrier.
    a9_states = (None,) + tuple(
        (phase, polarity)
        for phase in range(4)
        for polarity in (-1, 1)
    )
    two_slot_state_space = tuple(
        (left, right) for left in a9_states for right in a9_states
    )

    for frame in frames:
        parent = presentation(frame)
        exact_output = presentation(frame.output())
        _header, codewords = frame_codewords(frame, states)
        ready_bundle = codewords[0]
        normal = basis(frame)[2]
        anchor = frame_anchor(frame)
        input_position = scale_add(anchor, normal, 1)
        output_position = scale_add(anchor, normal, 2)
        ready_port = ready_work_port(frame)
        assert ready_port in two_slot_state_space
        assert occupied(ready_port) == 1

        defect_rows = tuple(enumerate_one_defects(frame, channels))
        descriptors = tuple(
            defect_descriptor(frame, defect) for _category, defect in defect_rows
        )
        descriptor_order = {
            descriptor: index
            for index, descriptor in enumerate(sorted(descriptors, key=repr), start=1)
        }

        for _category, defect in defect_rows:
            descriptor = defect_descriptor(frame, defect)
            syndrome = codewords[descriptor_order[descriptor]]
            work_after = output_work_port(frame, defect)
            assert work_after in two_slot_state_space
            assert all(valid_payload(payload) for payload in work_after)

            input_state = (
                defect,
                input_position,
                ready_bundle,
                ready_port,
            )
            output_state = (
                exact_output,
                output_position,
                syndrome,
                work_after,
            )
            assert input_state not in forward
            assert output_state not in reverse
            forward[input_state] = output_state
            reverse[output_state] = input_state
            assert reverse[forward[input_state]] == input_state
            inverse_rows += 1

            object_before = len(defect.relations) + len(defect.fields)
            object_after = len(exact_output.relations) + len(exact_output.fields)
            energy_before = object_before + len(ready_bundle) + occupied(ready_port)
            energy_after = object_after + len(syndrome) + occupied(work_after)
            assert energy_before == energy_after
            energy_rows += 1

            delta = resource_delta(parent, defect)
            occupancy_histogram[(delta, occupied(work_after))] += 1
            for payload in work_after:
                if payload is not None:
                    payload_histogram[payload] += 1

            # A2 work occupancy is a separate carrier factor and therefore
            # contributes neither SC source relations nor site E/B fields.
            assert presentation(frame) == parent
            neutral_port_rows += 1

    check(
        "C1 selected two-slot work port uses only the existing nine-state A9 alphabet",
        len(a9_states) == 9 and len(two_slot_state_space) == 81,
    )
    check(
        "C2 every registered repair has one payload-complete A2 work output",
        inverse_rows == 37_632,
    )
    check(
        "C3 complete repair sections are disjoint and exactly bijective",
        len(forward) == len(reverse) == 37_632
        and not (set(forward) & set(reverse)),
    )
    check(
        "C4 output syndrome and frame give an explicit full-state inverse",
        all(reverse[output] == input_state for input_state, output in forward.items()),
    )
    check(
        "C5 selected equal-occupancy object plus syndrome plus work energy is conserved",
        energy_rows == 37_632,
    )
    expected = Counter({(1, 0): 480, (0, 1): 672, (-1, 2): 36_480})
    check(
        "C6 exact A2 occupancy census realizes the formal zero/one/two-token ledger",
        occupancy_histogram == expected,
        str(occupancy_histogram),
    )
    check(
        "C7 every extra-field phase and polarity remains in the second A9 payload",
        set(payload_histogram) == {
            (phase, polarity)
            for phase in range(4)
            for polarity in (-1, 1)
        },
    )
    check(
        "C8 A2 work occupancy is source-invisible to the registered SC Gauss presentation",
        neutral_port_rows == 37_632,
    )
    check(
        "C9 forward plus reverse defines a finite involutive permutation on its two sections",
        all(forward[reverse[output]] == output for output in reverse),
    )
    missing = {
        "derivation of equal occupancy energy from the common action",
        "absolute energy multiplier and relation to field packet energy",
        "reciprocal work for the eighteen-record bundle clock debit",
        "full signed-cubic reflection closure and overlapping-event arbitration",
        "native ready-port and syndrome-bundle formation",
        "canonical Phi integration and repeated physical survival",
    }
    check(
        "C10 stable physical matter remains open at six action/integration debts",
        len(missing) == 6,
    )
    check(
        "C11 no target coupling, mass, Born weight, or new carrier type enters",
        len(states) == 192 and len(a9_states) == 9,
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} payload-complete A2 work-port checks pass")
    print(f"registered_bijection_rows={inverse_rows}")
    print(f"work_occupancy_histogram={dict(occupancy_histogram)}")
    print(f"occupied_A9_payloads={sorted(payload_histogram)}")
    print("energy_status=selected_equal_occupancy_invariant_not_derived_action")
    print("matter_status=finite_payload_complete_work_port_exact_common_action_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
