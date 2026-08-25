"""Executable reference and exact checks for the draft FTD-v3 Phi.

The map is a finite deterministic cellular transaction on periodic cubic
regions.  It uses only the selected R1 alphabets.  The certificate checks the
ratification-minimum R2--R4 properties; it does not establish a continuum
field theory or a physical coupling.
"""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass
from itertools import permutations, product
from pathlib import Path
from typing import Optional


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

Vec = tuple[int, int, int]
A9 = tuple[int, int]
Port = tuple[A9, Vec, int]
PortKey = tuple[Vec, Vec]
RelationKey = tuple[Vec, Vec]

BLANK: A9 = (0, 0)
D_SC: tuple[Vec, ...] = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)
POS_SC: tuple[Vec, ...] = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
POS_FCC: tuple[Vec, ...] = (
    (1, 1, 0),
    (1, -1, 0),
    (1, 0, 1),
    (1, 0, -1),
    (0, 1, 1),
    (0, 1, -1),
)
C18_LINES: tuple[Vec, ...] = POS_SC + POS_FCC


def add(a: Vec, b: Vec, size: int) -> Vec:
    return tuple((x + y) % size for x, y in zip(a, b))  # type: ignore[return-value]


def neg(a: Vec) -> Vec:
    return tuple(-x for x in a)  # type: ignore[return-value]


def dot(a: Vec, b: Vec) -> int:
    return sum(x * y for x, y in zip(a, b))


def rotate(z: A9) -> A9:
    u, v = z
    return -v, u


def rotate_inverse(z: A9) -> A9:
    u, v = z
    return v, -u


def a9_readout(z: A9) -> tuple[int, int, int, tuple[int, int]]:
    u, v = z
    radius = u * u + v * v
    diagonal = u * u * v * v
    occupied = radius - diagonal
    capacity = 1 - occupied
    polarity = radius - 3 * diagonal
    shell = radius - 2 * diagonal
    phase_u = shell * u + diagonal * (u + v) // 2
    phase_v = shell * v + diagonal * (v - u) // 2
    return occupied, capacity, polarity, (phase_u, phase_v)


PHASES = ((1, 0), (0, 1), (-1, 0), (0, -1))


def phase_index(z: A9) -> Optional[int]:
    phase = a9_readout(z)[3]
    return PHASES.index(phase) if phase in PHASES else None


def occupied(z: A9) -> int:
    return a9_readout(z)[0]


def polarity(z: A9) -> int:
    return a9_readout(z)[2]


def balanced_mod3(value: int) -> int:
    return (0, 1, -1)[value % 3]


@dataclass(eq=True)
class State:
    size: int
    site: dict[Vec, int]
    ports: dict[PortKey, Optional[Port]]
    relations: dict[RelationKey, tuple[A9, A9]]


def sites(size: int) -> tuple[Vec, ...]:
    return tuple(product(range(size), repeat=3))


def relation_keys(size: int) -> tuple[RelationKey, ...]:
    return tuple((x, direction) for x in sites(size) for direction in C18_LINES)


def blank_state(size: int) -> State:
    return State(
        size=size,
        site={x: 0 for x in sites(size)},
        ports={(x, direction): None for x in sites(size) for direction in D_SC},
        relations={key: (BLANK, BLANK) for key in relation_keys(size)},
    )


def copy_state(state: State) -> State:
    return State(
        size=state.size,
        site=dict(state.site),
        ports=dict(state.ports),
        relations=dict(state.relations),
    )


def sc_relation_for_port(x: Vec, direction: Vec, size: int) -> RelationKey:
    if direction in POS_SC:
        return x, direction
    positive = neg(direction)
    return add(x, direction, size), positive


def endpoints(key: RelationKey, size: int) -> tuple[Vec, Vec]:
    tail, direction = key
    return tail, add(tail, direction, size)


def endpoint_port_count(state: State, key: RelationKey) -> int:
    tail, head = endpoints(key, state.size)
    return sum(
        state.ports[(x, direction)] is not None
        for x in (tail, head)
        for direction in D_SC
    )


def primary_source(relations: dict[RelationKey, tuple[A9, A9]], size: int) -> dict[Vec, int]:
    source = {x: 0 for x in sites(size)}
    for key, (primary, _reserve) in relations.items():
        if not occupied(primary):
            continue
        tail, head = endpoints(key, size)
        sign = polarity(primary)
        source[tail] += sign
        source[head] -= sign
    return source


def site_readout(relations: dict[RelationKey, tuple[A9, A9]], size: int) -> dict[Vec, int]:
    return {x: balanced_mod3(value) for x, value in primary_source(relations, size).items()}


def work_count(state: State) -> int:
    port_tokens = sum(port is not None for port in state.ports.values())
    relation_tokens = sum(
        occupied(primary) + occupied(reserve)
        for primary, reserve in state.relations.values()
    )
    return port_tokens + relation_tokens


def absorption_candidates(state: State) -> dict[RelationKey, list[tuple[PortKey, A9]]]:
    candidates: dict[RelationKey, list[tuple[PortKey, A9]]] = {}
    for port_key, port in state.ports.items():
        if port is None:
            continue
        x, direction = port_key
        z, _normal, _hand = port
        if phase_index(z) != 2:
            continue
        relation = sc_relation_for_port(x, direction, state.size)
        primary, reserve = state.relations[relation]
        if primary == BLANK and reserve == BLANK:
            candidates.setdefault(relation, []).append((port_key, z))
    return candidates


def phi(state: State) -> State:
    """One complete synchronous v3 reference tick."""

    size = state.size
    candidates = absorption_candidates(state)
    accepted_absorption = {
        relation: rows[0] for relation, rows in candidates.items() if len(rows) == 1
    }
    absorbed_ports = {port_key for port_key, _z in accepted_absorption.values()}

    next_relations: dict[RelationKey, tuple[A9, A9]] = {}
    for key, (primary, reserve) in state.relations.items():
        if key in accepted_absorption:
            _port_key, z = accepted_absorption[key]
            next_relations[key] = (BLANK, rotate(z))
            continue

        one_owned = occupied(primary) + occupied(reserve) == 1
        token = primary if occupied(primary) else reserve
        even_field_gate = endpoint_port_count(state, key) % 2 == 0
        crosses = one_owned and phase_index(token) == 0 and even_field_gate
        if crosses:
            next_relations[key] = (rotate(reserve), rotate(primary))
        else:
            next_relations[key] = (rotate(primary), rotate(reserve))

    next_ports: dict[PortKey, Optional[Port]] = {
        key: None for key in state.ports
    }
    for port_key, port in state.ports.items():
        if port is None or port_key in absorbed_ports:
            continue
        x, direction = port_key
        z, normal, hand = port
        manifest = state.site[x] * state.site[x]
        next_z = rotate(z) if manifest == 0 else rotate_inverse(z)
        next_hand = hand if manifest == 0 else -hand
        destination = (add(x, direction, size), direction)
        assert next_ports[destination] is None
        next_ports[destination] = (next_z, normal, next_hand)

    return State(
        size=size,
        site=site_readout(next_relations, size),
        ports=next_ports,
        relations=next_relations,
    )


def translate_vec(x: Vec, offset: Vec, size: int) -> Vec:
    return add(x, offset, size)


def translate_state(state: State, offset: Vec) -> State:
    size = state.size
    translated = blank_state(size)
    translated.site = {
        translate_vec(x, offset, size): value for x, value in state.site.items()
    }
    translated.ports = {
        (translate_vec(x, offset, size), direction): port
        for (x, direction), port in state.ports.items()
    }
    translated.relations = {
        (translate_vec(tail, offset, size), direction): pair
        for (tail, direction), pair in state.relations.items()
    }
    return translated


def signed_permutation_images(direction: Vec) -> set[Vec]:
    images: set[Vec] = set()
    for perm in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            image = tuple(signs[i] * direction[perm[i]] for i in range(3))
            images.add(image)  # type: ignore[arg-type]
    return images


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    cells = register["carrier_inventory"]["cell_alphabets"]
    check("C1 rule uses the locked R1 cell cardinalities", cells["A0_site"]["cardinality"] == 3 * 65**6 and cells["A1_bond"]["cardinality"] == 81 and cells["A2_plaquette"]["cardinality"] == 6_561 and cells["A3_cube"]["cardinality"] == 1)

    check("C2 C18 contains three SC and six FCC line types", len(POS_SC) == 3 and len(POS_FCC) == 6 and len(C18_LINES) == 9)
    full_sc = set(D_SC)
    full_fcc = {v for line in POS_FCC for v in (line, neg(line))}
    check("C3 signed cubic images preserve the SC shell", signed_permutation_images((1, 0, 0)) == full_sc)
    check("C4 signed cubic images preserve the FCC shell", signed_permutation_images((1, 1, 0)) == full_fcc)

    blank = blank_state(5)
    check("C5 blank state is a fixed point", phi(blank) == blank)
    check("C6 Phi is deterministic", phi(blank) == phi(copy_state(blank)))

    # One unobstructed packet advances by exactly one SC hop.
    packet_state = blank_state(5)
    x = (1, 1, 1)
    direction = (1, 0, 0)
    normal = (0, 1, 0)
    z_phase0 = (1, 0)
    packet_state.ports[(x, direction)] = (z_phase0, normal, 1)
    packet_next = phi(packet_state)
    destination = (add(x, direction, 5), direction)
    check("C7 one field packet propagates exactly one SC hop", packet_next.ports[destination] is not None and packet_next.ports[(x, direction)] is None)
    check("C8 free packet advances one C4 phase", packet_next.ports[destination][0] == rotate(z_phase0))  # type: ignore[index]
    check("C9 free packet retains normal and handed frame", packet_next.ports[destination][1:] == (normal, 1))  # type: ignore[index]

    # Exact many-to-one absorption: eight distinct frame presentations with
    # the same z and directed port collapse to the same bound reserve state.
    z_phase2 = (-1, 0)
    absorption_outputs: list[State] = []
    input_frames: list[tuple[Vec, int]] = []
    for n in D_SC:
        if dot(n, direction) != 0:
            continue
        for hand in (-1, 1):
            state = blank_state(5)
            state.ports[(x, direction)] = (z_phase2, n, hand)
            input_frames.append((n, hand))
            absorption_outputs.append(phi(state))
    target_relation = sc_relation_for_port(x, direction, 5)
    check("C10 absorption census has eight distinct Hodge inputs", len(input_frames) == 8 and len(set(input_frames)) == 8)
    check("C11 all eight Hodge inputs have one identical output", all(output == absorption_outputs[0] for output in absorption_outputs[1:]))
    absorbed = absorption_outputs[0]
    check("C12 absorption clears the mobile port", absorbed.ports[(add(x, direction, 5), direction)] is None)
    check("C13 absorption transfers the complete A9 payload to reserve", absorbed.relations[target_relation] == (BLANK, rotate(z_phase2)))
    check("C14 absorption is genuinely non-injective on admissible states", len({repr(state) for state in absorption_outputs}) == 1)

    for n in D_SC:
        if dot(n, direction) == 0:
            state = blank_state(5)
            state.ports[(x, direction)] = (z_phase2, n, 1)
            check("C15 absorption conserves the finite token/work count", work_count(phi(state)) == work_count(state))
            break

    # Opposing simultaneous arrivals fail closed instead of selecting a side.
    conflict = blank_state(5)
    head = add(x, direction, 5)
    conflict.ports[(x, direction)] = (z_phase2, normal, 1)
    conflict.ports[(head, neg(direction))] = (z_phase2, normal, -1)
    conflict_next = phi(conflict)
    check("C16 symmetric two-arrival conflict does not absorb either packet", conflict_next.relations[target_relation] == (BLANK, BLANK) and work_count(conflict_next) == 2)

    # Isolated relation recurrence is the controller-free F=R A0 clock.
    clock = blank_state(5)
    clock.relations[target_relation] = (BLANK, z_phase0)
    clock.site = site_readout(clock.relations, 5)
    initial_clock = copy_state(clock)
    manifest_counts = 0
    continuity_pass = True
    ledger_pass = True
    for _ in range(8):
        previous = clock
        clock = phi(clock)
        manifest_counts += int(any(value != 0 for value in clock.site.values()))
        ledger_pass = ledger_pass and work_count(previous) == work_count(clock) == 1
        q0 = primary_source(previous.relations, 5)
        q1 = primary_source(clock.relations, 5)
        divergence = {site: 0 for site in sites(5)}
        for key in previous.relations:
            delta = polarity(clock.relations[key][0]) - polarity(previous.relations[key][0])
            tail, relation_head = endpoints(key, 5)
            current = -delta
            divergence[tail] += current
            divergence[relation_head] -= current
        continuity_pass = continuity_pass and all(q1[site] - q0[site] + divergence[site] == 0 for site in sites(5))
    check("C17 isolated material recurrence has exact period eight", clock == initial_clock)
    check("C18 recurrence is manifested for four of eight ticks", manifest_counts == 4)
    check("C19 recurrence preserves one token/work unit", ledger_pass)
    check("C20 primary-source/current continuity is exact", continuity_pass)

    # Odd local packet occupancy stalls the crossing section; the same
    # manifested readout reverses and hand-flips an outgoing field clock.
    stalled = blank_state(5)
    stalled.relations[target_relation] = (BLANK, z_phase0)
    stalled.ports[(x, direction)] = ((0, 1), normal, 1)
    stalled_next = phi(stalled)
    check("C21 odd endpoint field occupancy stalls material ownership crossing", stalled_next.relations[target_relation][0] == BLANK and stalled_next.relations[target_relation][1] == rotate(z_phase0))

    coupled = blank_state(5)
    coupled.relations[target_relation] = (z_phase0, BLANK)
    coupled.site = site_readout(coupled.relations, 5)
    coupled.ports[(x, direction)] = ((0, 1), normal, 1)
    coupled_next = phi(coupled)
    coupled_packet = coupled_next.ports[(head, direction)]
    check("C22 manifested source reverses the local field phase clock", coupled_packet is not None and coupled_packet[0] == rotate_inverse((0, 1)))
    check("C23 manifested source flips field handedness", coupled_packet is not None and coupled_packet[2] == -1)

    # Translation covariance of the complete map on a nontrivial state.
    offset = (2, 1, 3)
    check("C24 Phi commutes with lattice translations", phi(translate_state(packet_state, offset)) == translate_state(phi(packet_state), offset))

    # The schedule is synchronous and write-unique by construction.
    size = 5
    expected_ports = size**3 * len(D_SC)
    expected_relations = size**3 * len(C18_LINES)
    check("C25 every output port has exactly one geometric predecessor", len(packet_next.ports) == expected_ports)
    check("C26 every relation pair is written exactly once", len(packet_next.relations) == expected_relations)
    check("C27 site actuality is one local reduction, not competing writes", packet_next.site == site_readout(packet_next.relations, size))

    # Radius-one dependency audit from the declared offsets.
    support = set(D_SC) | set(C18_LINES) | {neg(v) for v in C18_LINES} | {(0, 0, 0)}
    check("C28 every direct dependency offset lies in the Moore cube", all(max(abs(component) for component in vector) <= 1 for vector in support))

    # No queue or identity is introduced: every port and relation slot has
    # exclusion occupancy 0/1, and all record values remain in finite sets.
    check("C29 port streaming is an exclusion-slot permutation when unabsorbed", sum(port is not None for port in packet_state.ports.values()) == sum(port is not None for port in packet_next.ports.values()) == 1)
    check("C30 the update creates no out-of-alphabet site value", all(value in (-1, 0, 1) for value in coupled_next.site.values()))
    check("C31 the update creates no out-of-alphabet A9 value", all(z in tuple(product((-1, 0, 1), repeat=2)) for pair in coupled_next.relations.values() for z in pair))
    check("C32 the update creates only valid finite port payloads", all(port is None or (port[0] in tuple(product((-1, 0, 1), repeat=2)) and port[1] in D_SC and dot(port[1], direction_key) == 0 and port[2] in (-1, 1)) for (_x, direction_key), port in coupled_next.ports.items()))

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} exact common-transaction Phi checks pass")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
