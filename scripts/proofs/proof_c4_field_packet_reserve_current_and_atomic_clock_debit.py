#!/usr/bin/env python3
"""Exact C4 field-packet reserve current and atomic clock-debit certificate.

The selected half-admitted outgoing carrier has eight phase-paired energy
groups of weight 1/8.  Treat their spatial ownership as a reserve density and
their local hold/SC-hop transitions as a signed bond current.  This proves an
exact pointwise discrete continuity equation, finite-domain flux balance,
phase-complete inverse transport, and the FTD-0999 packet-count resource law.

Whole-packet ownership swaps then provide atomic debit/refill with no copied
or deleted payload and explicit double-spend rejection.  The carrier metric,
admission schedule, absorption vertex, and field/clock scale compliance remain
selected.  Therefore the locked outcome is determined by the gates, not by
this docstring; no alpha value or master-root comparison is performed.
"""

from __future__ import annotations

import hashlib
from collections import Counter, defaultdict
from itertools import combinations, product
from pathlib import Path

from sympy import Rational, Symbol, pi, simplify

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_c4_half_admitted_energy_current_momentum_boundary import energy_groups
from proof_c4_phase_parity_half_admitted_two_polarization_carrier import (
    gated_stream,
    inverse_gated_stream,
)
from proof_cotangent_handed_directional_radiation_port import (
    DirectionalPortState,
    port_records,
    propagation_direction,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "common_action_mechanics_reciprocity/"
    "PREREG_C4_FIELD_PACKET_RESERVE_CURRENT_AND_CLOCK_DEBIT_v1.md"
)
LOCKED_HASHES = {
    PREREG: "A76FA492E9B8DB022F0F708ABBC94EFD4F9372062E91C4D464A9D00568D81C80",
    ROOT / "scripts/proofs/proof_c4_phase_parity_half_admitted_two_polarization_carrier.py":
        "743CE826C259905DEF31CE1F2324EE8C5DB6EF8E04A8AE9B94227833D31F6000",
    ROOT / "scripts/proofs/proof_c4_half_admitted_energy_current_momentum_boundary.py":
        "D9E0E2C4FF595A56F85712CA5195BE1406BCB7F90B8B2D5D8E66CBC2F05AE3CA",
    ROOT / "scripts/proofs/proof_cotangent_handed_directional_radiation_port.py":
        "CE36C2FCF26607528C40DA0CD6DC85940B6A1D4FEB6FB6A78EE9E2B1D26155E2",
    ROOT / "scripts/proofs/proof_global_c3_cotangent_layer_hodge_maxwell_target.py":
        "975433E0FD496DE68EF500595D6FA927CD55B8CFEDFB0158D307053C661A6C4A",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def add(left, right):
    return tuple(a + b for a, b in zip(left, right))


def subtract(left, right):
    return tuple(a - b for a, b in zip(left, right))


def scale(factor, vector):
    return tuple(factor * component for component in vector)


def grouped_payload(records):
    groups = defaultdict(list)
    for position, record in records:
        groups[(position, record[0])].append(record)
    output = {}
    for key, payload in groups.items():
        ordered = tuple(sorted(payload, key=lambda record: record[1]))
        assert len(ordered) == 2
        phases = tuple(record[1] for record in ordered)
        assert (phases[1] - phases[0]) % 4 == 2
        output[key] = ordered
    return output


def density(records, layer: int):
    result = defaultdict(lambda: Rational(0))
    groups = energy_groups(records, layer)
    for position, _flag, energy in groups:
        assert energy == Rational(1, 8)
        result[position] += energy
    return dict(result), groups


def group_transitions(records, layer: int, parity: int):
    old_payload = grouped_payload(records)
    advanced = gated_stream(records, parity)
    new_payload = grouped_payload(advanced)
    new_layer = (layer - 1) % 3
    old_density, old_energy_groups = density(records, layer)
    new_density, new_energy_groups = density(advanced, new_layer)
    old_energy = {
        (position, flag): energy for position, flag, energy in old_energy_groups
    }
    new_energy = {
        (position, flag): energy for position, flag, energy in new_energy_groups
    }

    transitions = []
    reached = set()
    for old_key, payload in old_payload.items():
        moved = gated_stream(
            tuple((old_key[0], record) for record in payload), parity
        )
        moved_keys = {(position, record[0]) for position, record in moved}
        assert len(moved_keys) == 1
        new_key = next(iter(moved_keys))
        assert new_key in new_payload
        assert tuple(sorted(record for _position, record in moved)) == tuple(
            sorted(new_payload[new_key])
        )
        assert old_energy[old_key] == new_energy[new_key] == Rational(1, 8)
        transitions.append((old_key[0], new_key[0], Rational(1, 8)))
        reached.add(new_key)

    assert reached == set(new_payload)
    assert inverse_gated_stream(advanced, parity) == records
    return tuple(transitions), advanced, new_layer, old_density, new_density


def current_from_transitions(transitions):
    current = defaultdict(lambda: Rational(0))
    for tail, head, energy in transitions:
        if tail == head:
            continue
        current[(tail, head)] += energy
        current[(head, tail)] -= energy
    return dict(current)


def verify_continuity(records, layer: int, parity: int) -> tuple[int, tuple]:
    checks = 0
    transitions, advanced, new_layer, old_density, new_density = group_transitions(
        records, layer, parity
    )
    current = current_from_transitions(transitions)

    assert len(transitions) == 8
    assert all(value >= 0 for value in old_density.values())
    assert all(value >= 0 for value in new_density.values())
    assert sum(old_density.values(), Rational(0)) == 1
    assert sum(new_density.values(), Rational(0)) == 1
    checks += 5

    sites = set(old_density) | set(new_density)
    for tail, head, _energy in transitions:
        sites.add(tail)
        sites.add(head)
        displacement = subtract(head, tail)
        assert displacement == (0, 0, 0) or displacement in SC_DIRECTIONS
        checks += 1

    for (tail, head), value in current.items():
        assert current.get((head, tail), Rational(0)) == -value
        checks += 1

    for site in sites:
        outward = sum(
            (value for (tail, _head), value in current.items() if tail == site),
            Rational(0),
        )
        assert new_density.get(site, 0) - old_density.get(site, 0) + outward == 0
        checks += 1

    # Pointwise continuity implies every finite-domain balance.  Check the
    # full support and every coordinate half-space induced by this step.
    domains = [set(sites)]
    for axis in range(3):
        coordinates = sorted({site[axis] for site in sites})
        for threshold in coordinates:
            domains.append({site for site in sites if site[axis] <= threshold})
    for domain in domains:
        outward = sum(
            (
                value
                for (tail, head), value in current.items()
                if tail in domain and head not in domain
            ),
            Rational(0),
        )
        inward = -outward
        delta = sum(
            (
                new_density.get(site, 0) - old_density.get(site, 0)
                for site in domain
            ),
            Rational(0),
        )
        assert delta == inward
        checks += 1

    moment = (Rational(0), Rational(0), Rational(0))
    for tail, head, energy in transitions:
        moment = add(moment, scale(energy, subtract(head, tail)))
    return checks, (advanced, new_layer, moment)


def swap_batch(packets, selected, source: str, target: str):
    selected = tuple(selected)
    if len(set(selected)) != len(selected):
        return None
    if any(index < 0 or index >= len(packets) for index in selected):
        return None
    if any(packets[index][0] != source for index in selected):
        return None
    updated = list(packets)
    for index in selected:
        _owner, payload = updated[index]
        updated[index] = (target, payload)
    return tuple(updated)


def owner_count(packets, owner: str) -> int:
    return sum(value[0] == owner for value in packets)


def verify_atomic_ownership() -> int:
    checks = 0
    for size in range(1, 7):
        payloads = tuple(
            (index, index % 4, (index + 2) % 4, index % 8) for index in range(size)
        )
        for reserve_count in range(size + 1):
            owners = tuple(
                "reserve" if index < reserve_count else "environment"
                for index in range(size)
            )
            packets = tuple(zip(owners, payloads))
            for demand in range(size + 1):
                selected = tuple(range(demand))
                before = tuple(packets)
                debited = swap_batch(packets, selected, "reserve", "clock-port")
                if demand <= reserve_count:
                    assert debited is not None
                    assert owner_count(debited, "reserve") == reserve_count - demand
                    assert owner_count(debited, "clock-port") == demand
                    restored = swap_batch(
                        debited, selected, "clock-port", "reserve"
                    )
                    assert restored == before
                    assert tuple(payload for _owner, payload in debited) == payloads
                    checks += 5
                else:
                    assert debited is None
                    assert packets == before
                    checks += 2

    # Disjoint batches commute.  Overlap is a same-tick double spend and the
    # second attempted mutation fails against the changed ownership state.
    payloads = tuple(
        (index, index % 4, (index + 2) % 4, index % 8) for index in range(6)
    )
    owners = tuple(("reserve", payload) for payload in payloads)
    for split in range(1, 6):
        left = tuple(range(split))
        right = tuple(range(split, 6))
        first_left = swap_batch(owners, left, "reserve", "clock-port")
        first_right = swap_batch(owners, right, "reserve", "clock-port")
        assert first_left is not None and first_right is not None
        lr = swap_batch(first_left, right, "reserve", "clock-port")
        rl = swap_batch(first_right, left, "reserve", "clock-port")
        assert lr == rl == tuple(("clock-port", payload) for payload in payloads)
        checks += 2

        overlap = (left[-1], right[0])
        spent = swap_batch(owners, overlap, "reserve", "clock-port")
        assert spent is not None
        assert swap_batch(spent, overlap, "reserve", "clock-port") is None
        checks += 2
    return checks


def verify_resource_law() -> int:
    checks = 0
    gamma = Symbol("Gamma", positive=True)
    for reserve in range(9):
        for inward in range(5):
            for outward in range(reserve + 1):
                after_boundary = reserve + inward - outward
                for formed in range(5):
                    available = after_boundary + formed
                    for demand in range(available + 2):
                        admitted = demand <= available
                        if admitted:
                            final = available - demand
                            assert final == reserve + (inward - outward) + formed - demand
                            assert simplify(
                                gamma * final
                                - gamma
                                * (reserve + (inward - outward) + formed - demand)
                            ) == 0
                            checks += 2
                        else:
                            assert available - demand < 0
                            checks += 1

    action_quantum = Symbol("I_star", positive=True)
    clock_frequency = Symbol("omega_0", positive=True)
    packet_count = Symbol("d", positive=True, integer=True)
    compliance = simplify(clock_frequency * action_quantum - packet_count * gamma)
    solved_curvature = simplify(clock_frequency / packet_count)
    assert compliance.subs(
        gamma, clock_frequency * action_quantum / packet_count
    ) == 0
    assert simplify(
        (gamma / action_quantum).subs(
            gamma, clock_frequency * action_quantum / packet_count
        )
        - solved_curvature
    ) == 0
    c_eff = Rational(1, 6)
    conditional_alpha = simplify(solved_curvature / (4 * pi * c_eff))
    assert conditional_alpha == 3 * clock_frequency / (2 * pi * packet_count)
    checks += 3
    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        assert sha256(path) == expected, (path, sha256(path), expected)
        checks += 1

    frames = tuple(
        (direction, second)
        for direction in SC_DIRECTIONS
        for second in SC_DIRECTIONS
        if dot(direction, second) == 0
    )
    assert len(frames) == 24
    checks += 1

    for frame, chirality, phase, stage, orientation, parity in product(
        frames, (-1, 1), range(4), range(12), (-1, 1), (0, 1)
    ):
        state = DirectionalPortState(
            frame, chirality, phase, stage, orientation, True, 0
        )
        records = port_records(state)
        layer = (-stage) % 3
        six_tick_moment = (Rational(0), Rational(0), Rational(0))
        for _tick in range(6):
            step_checks, (records, layer, moment) = verify_continuity(
                records, layer, parity
            )
            checks += step_checks
            six_tick_moment = add(six_tick_moment, moment)
        propagation = propagation_direction(frame, chirality)
        assert six_tick_moment == propagation
        checks += 1

    checks += verify_atomic_ownership()
    checks += verify_resource_law()

    print("C4 packet density is pointwise nonnegative and totals one")
    print("phase-paired group motion obeys exact local discrete continuity")
    print("finite-domain reserve change equals signed Moore-boundary inflow")
    print("six-tick transported reserve current is r/6")
    print("whole-packet ownership debit is atomic, reversible, and double-spend safe")
    print("packet counts realize B'=B+Phi+U-D exactly")
    print("conditional scale compliance gives chi_EM=omega_0/d, but fixes no value")
    print(
        "PASS: C4 field-packet reserve current and atomic clock debit "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME B: exact carrier/interface; common-action selection, absorption, "
        "and field/clock scale compliance remain open"
    )


if __name__ == "__main__":
    main()
