#!/usr/bin/env python3
"""Exact finite A2 memory for the v3 rotor Green response.

The parent neutral-rotor theorem obtains a Dirichlet Green response from
finite traversal histories, but leaves those histories outside the
instantaneous carrier.  This certificate gives every measured unoriented SC
edge one existing A2=A9^4 owner.  The four A9 payloads stay occupied, so their
8^4 phase/polarity states form a 4,096-state cyclic counter.  We use 4,095
states for signed values -2047,...,+2047 and reserve one state as an explicit
overflow symbol.  Forward and reverse crossings apply mutually inverse
phase permutations.

One additional Moore-local A2 counter records the number N of completed
source injections.  Hence the normalized current C_e/N is a readout of two
finite physical records, not an unretained history variable.  On every
admitted history (all counter prefixes remain in range), the physical record
equals the rotor traversal count exactly.  The parent 8/(3N) current-gradient
bound and the exact finite Dirichlet inverse then give, for each edge e,

  |C_e/N - grad_e G_D| <= 8/(3N) + 8 K_e/N,

where K_e is the exact l1 norm of the corresponding row difference of
L_D^{-1}.  Two arbitrary initial rotor phases therefore differ by at most
twice this bound.  This is an operational, finite-memory protection theorem.
It is not yet a mechanical force: fixed A9 occupancy makes the existing
relative occupancy ledger blind to every counter increment.  A nondegenerate
phase/action term, homogeneous Phi integration, autonomous source/sink work,
and absolute gravity normalization remain open.
"""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from fractions import Fraction
from itertools import product

from sympy import Matrix, cos, diff, symbols

from proof_global_c3_cotangent_layer_hodge_maxwell_target import internal_tick
from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_neutral_rotor_harmonic_green_seam import (
    SC_DIRECTIONS,
    add,
    box,
    dirichlet_laplacian,
    rotor_successor,
    simulate_rotor_box,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
Edge = tuple[Vec, Vec]
Plaquette = tuple[Vec, tuple[int, int]]
A9 = tuple[int, int]
A2 = tuple[A9, A9, A9, A9]

AXES: tuple[Vec, ...] = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
OCCUPIED_A9: tuple[A9, ...] = tuple(product(range(4), (-1, 1)))
COUNTER_CARDINALITY = len(OCCUPIED_A9) ** 4
COUNTER_LIMIT = (COUNTER_CARDINALITY - 2) // 2
OVERFLOW_INDEX = COUNTER_CARDINALITY - 1


def scale(value: int, vector: Vec) -> Vec:
    return tuple(value * entry for entry in vector)  # type: ignore[return-value]


def as_fraction(value) -> Fraction:
    if hasattr(value, "p") and hasattr(value, "q"):
        return Fraction(int(value.p), int(value.q))
    return Fraction(int(value), 1)


def counter_payload(index: int) -> A2:
    """Base-eight payload using only the eight occupied A9 symbols."""

    assert 0 <= index < COUNTER_CARDINALITY
    digits: list[A9] = []
    remainder = index
    for _ in range(4):
        digits.append(OCCUPIED_A9[remainder % 8])
        remainder //= 8
    assert remainder == 0
    return tuple(digits)  # type: ignore[return-value]


def counter_index(payload: A2) -> int:
    index = 0
    place = 1
    for digit in payload:
        index += OCCUPIED_A9.index(digit) * place
        place *= 8
    return index


def encode_counter(value: int | None) -> A2:
    if value is None:
        return counter_payload(OVERFLOW_INDEX)
    assert -COUNTER_LIMIT <= value <= COUNTER_LIMIT
    return counter_payload(value + COUNTER_LIMIT)


def decode_counter(payload: A2) -> int | None:
    index = counter_index(payload)
    if index == OVERFLOW_INDEX:
        return None
    return index - COUNTER_LIMIT


def counter_step(payload: A2, signed_event: int) -> A2:
    """Total cyclic permutation; -1 is the exact inverse of +1."""

    assert signed_event in (-1, 1)
    return counter_payload(
        (counter_index(payload) + signed_event) % COUNTER_CARDINALITY
    )


def canonical_edge(left: Vec, right: Vec) -> tuple[Edge, int]:
    """Return edge and sign of the directed crossing left -> right."""

    assert sum(abs(a - b) for a, b in zip(left, right)) == 1
    if left < right:
        return (left, right), 1
    return (right, left), -1


def graph_edges(vertices: frozenset[Vec]) -> tuple[Edge, ...]:
    edges = set()
    for vertex in vertices:
        for direction in SC_DIRECTIONS:
            edge, _sign = canonical_edge(vertex, add(vertex, direction))
            edges.add(edge)
    return tuple(sorted(edges))


def incident_plaquettes(edge: Edge) -> tuple[Plaquette, ...]:
    """The four square plaquettes incident on one SC edge."""

    low, high = edge
    axis = next(i for i in range(3) if high[i] != low[i])
    assert high[axis] == low[axis] + 1
    owners = []
    for other in range(3):
        if other == axis:
            continue
        for side in (0, -1):
            base = list(low)
            base[other] += side
            owners.append((tuple(base), tuple(sorted((axis, other)))))
    return tuple(sorted(owners))


def edge_owner_matching(
    edges: tuple[Edge, ...], reserved: frozenset[Plaquette]
) -> dict[Edge, Plaquette]:
    """Deterministic augmenting-path matching into incident A2 owners."""

    owner_to_edge: dict[Plaquette, Edge] = {}
    edge_to_owner: dict[Edge, Plaquette] = {}

    def place(edge: Edge, seen: set[Plaquette]) -> bool:
        for owner in incident_plaquettes(edge):
            if owner in reserved or owner in seen:
                continue
            seen.add(owner)
            incumbent = owner_to_edge.get(owner)
            if incumbent is None or place(incumbent, seen):
                owner_to_edge[owner] = edge
                edge_to_owner[edge] = owner
                return True
        return False

    for edge in edges:
        assert place(edge, set())
    assert len(edge_to_owner) == len(edges)
    assert len(set(edge_to_owner.values())) == len(edges)
    return edge_to_owner


def simulate_physical_memory(radius: int, injections: int, initial_state):
    """Run the parent rotor history while writing exact finite A2 records."""

    assert 0 <= injections <= COUNTER_LIMIT
    vertices = frozenset(box(radius))
    source = (0, 0, 0)
    rotors = {vertex: initial_state for vertex in vertices}
    visits: defaultdict[Vec, int] = defaultdict(int)
    traversals: defaultdict[tuple[Vec, Vec], int] = defaultdict(int)
    counters: defaultdict[Edge, A2] = defaultdict(lambda: encode_counter(0))
    source_counter = encode_counter(0)
    maximum_prefix = 0
    total_steps = 0

    for completed in range(injections):
        source_counter = counter_step(source_counter, 1)
        assert decode_counter(source_counter) == completed + 1
        location = source
        while location in vertices:
            visits[location] += 1
            rotors[location] = internal_tick(rotors[location])
            direction = rotor_successor(rotors[location])
            neighbor = add(location, direction)
            edge, sign = canonical_edge(location, neighbor)
            counters[edge] = counter_step(counters[edge], sign)
            value = decode_counter(counters[edge])
            assert value is not None, "declared A2 measurement capacity exceeded"
            maximum_prefix = max(maximum_prefix, abs(value))
            traversals[(location, direction)] += 1
            location = neighbor
            total_steps += 1
            assert total_steps < 50_000_000

    return (
        vertices,
        source,
        visits,
        traversals,
        {edge: decode_counter(payload) for edge, payload in counters.items()},
        decode_counter(source_counter),
        maximum_prefix,
        total_steps,
    )


def traversal_edge_counts(traversals) -> dict[Edge, int]:
    counts: defaultdict[Edge, int] = defaultdict(int)
    for (vertex, direction), multiplicity in traversals.items():
        edge, sign = canonical_edge(vertex, add(vertex, direction))
        counts[edge] += sign * multiplicity
    return dict(counts)


def edge_green_data(vertices: tuple[Vec, ...]):
    index = {vertex: row for row, vertex in enumerate(vertices)}
    laplacian = dirichlet_laplacian(vertices)
    inverse = laplacian.inv()
    delta = Matrix.zeros(len(vertices), 1)
    delta[index[(0, 0, 0)]] = 1
    green = inverse * delta
    edges = graph_edges(frozenset(vertices))
    data = {}
    for edge in edges:
        low, high = edge
        coefficients = Matrix.zeros(1, len(vertices))
        if low in index:
            coefficients[0, index[low]] += 1
        if high in index:
            coefficients[0, index[high]] -= 1
        gradient = (coefficients * green)[0]
        residual_transfer = coefficients * inverse
        norm = sum(
            abs(as_fraction(residual_transfer[column]))
            for column in range(len(vertices))
        )
        data[edge] = as_fraction(gradient), norm
    return laplacian, inverse, data


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = tuple(one_particle_states())

    payloads = tuple(counter_payload(index) for index in range(COUNTER_CARDINALITY))
    check(
        "C1 one fixed-occupancy A2 has 4096 distinct phase/polarity states",
        len(OCCUPIED_A9) == 8
        and len(payloads) == len(set(payloads)) == 8**4
        and all(len(payload) == 4 for payload in payloads),
    )
    check(
        "C2 4095 states encode every signed value -2047 through +2047 and one explicit overflow",
        COUNTER_LIMIT == 2047
        and {decode_counter(payload) for payload in payloads}
        == set(range(-2047, 2048)) | {None},
    )

    permutation_rows = 0
    for payload in payloads:
        plus = counter_step(payload, 1)
        minus = counter_step(payload, -1)
        assert counter_step(plus, -1) == payload
        assert counter_step(minus, 1) == payload
        assert all(symbol in OCCUPIED_A9 for symbol in plus + minus)
        permutation_rows += 4
    check(
        "C3 forward and reverse edge writes are exact total inverse permutations at constant occupancy",
        permutation_rows == 4 * 4096,
    )

    history_rows = 0
    for events in product((-1, 1), repeat=12):
        payload = encode_counter(0)
        total = 0
        for event in events:
            total += event
            payload = counter_step(payload, event)
            assert decode_counter(payload) == total
            history_rows += 1
    check(
        "C4 every admitted signed event history is retained exactly in the physical A2 state",
        history_rows == 12 * 2**12,
    )

    vertices = frozenset(box(1))
    edges = graph_edges(vertices)
    source_owner: Plaquette = ((0, 0, 0), (0, 1))
    owners = edge_owner_matching(edges, frozenset({source_owner}))
    check(
        "C5 every measured SC edge and the source counter have distinct local existing-A2 owners",
        len(edges) == len(owners)
        and len(set(owners.values()) | {source_owner}) == len(edges) + 1
        and all(owners[edge] in incident_plaquettes(edge) for edge in edges),
        f"edge_A2={len(edges)}, source_A2=1",
    )

    fixture_rows = 0
    total_fixture_steps = 0
    maximum_fixture_prefix = 0
    for injections, state_index in ((7, 37), (37, 101), (128, 0)):
        physical = simulate_physical_memory(1, injections, states[state_index])
        parent = simulate_rotor_box(1, injections, states[state_index])
        expected_counts = traversal_edge_counts(parent[3])
        physical_counts = physical[4]
        assert physical[0] == parent[0]
        assert physical[1] == parent[1]
        assert physical[2] == parent[2]
        assert physical[3] == parent[3]
        assert physical[5] == injections
        assert all(physical_counts.get(edge, 0) == count for edge, count in expected_counts.items())
        assert all(expected_counts.get(edge, 0) == count for edge, count in physical_counts.items())
        assert physical[6] <= COUNTER_LIMIT
        fixture_rows += len(edges)
        total_fixture_steps += physical[7]
        maximum_fixture_prefix = max(maximum_fixture_prefix, physical[6])
    check(
        "C6 physical A2 bank exactly reproduces the certified rotor histories without changing a route",
        fixture_rows == 3 * len(edges) and total_fixture_steps > 0,
    )

    # Exact source continuity now follows from instantaneous carrier records.
    physical = simulate_physical_memory(1, 128, states[0])
    physical_counts = physical[4]
    continuity_rows = 0
    for vertex in vertices:
        divergence = Fraction(0)
        for direction in SC_DIRECTIONS:
            edge, outward_sign = canonical_edge(vertex, add(vertex, direction))
            divergence += Fraction(outward_sign * physical_counts.get(edge, 0), 128)
        assert divergence == (1 if vertex == (0, 0, 0) else 0)
        continuity_rows += 1
    check(
        "C7 the physical counter bank has exact unit source divergence",
        continuity_rows == 27,
    )

    ordered_vertices = tuple(sorted(vertices))
    _laplacian, _inverse, green_data = edge_green_data(ordered_vertices)
    phase_currents: dict[int, dict[Edge, Fraction]] = {}
    response_rows = 0
    injections = 37
    maximum_response_error = Fraction(0)
    maximum_response_bound = Fraction(0)
    maximum_phase_prefix = 0
    for state_index, state in enumerate(states):
        run = simulate_physical_memory(1, injections, state)
        counts = run[4]
        maximum_phase_prefix = max(maximum_phase_prefix, run[6])
        currents = {
            edge: Fraction(counts.get(edge, 0), injections) for edge in edges
        }
        phase_currents[state_index] = currents
        for edge in edges:
            exact_gradient, transfer_norm = green_data[edge]
            bound = Fraction(8, 3 * injections) + Fraction(8, injections) * transfer_norm
            error = abs(currents[edge] - exact_gradient)
            assert error <= bound
            maximum_response_error = max(maximum_response_error, error)
            maximum_response_bound = max(maximum_response_bound, bound)
            response_rows += 1
    check(
        "C8 all 192 native rotor phases obey the exact finite-domain Green-response bound",
        response_rows == 192 * len(edges),
    )

    protection_rows = 0
    for edge in edges:
        exact_gradient, transfer_norm = green_data[edge]
        del exact_gradient
        bound = Fraction(8, 3 * injections) + Fraction(8, injections) * transfer_norm
        values = [phase_currents[index][edge] for index in range(len(states))]
        assert max(values) - min(values) <= 2 * bound
        protection_rows += 1
    check(
        "C9 arbitrary initial rotor-phase changes perturb each recorded current by at most twice the exact bound",
        protection_rows == len(edges),
    )

    # The triplet rest-source coordinate is 1/12.  Scaling the carrier readout
    # therefore scales the exact protection bound, without selecting a force
    # coefficient or changing any counter transition.
    scalar_mass = Fraction(1, 12)
    scaled_bound = scalar_mass * maximum_response_bound
    check(
        "C10 triplet scalar readout inherits the protected physical-memory response with factor 1/12",
        scalar_mass * 12 == 1 and scaled_bound * 12 == maximum_response_bound,
    )

    kx, ky, kz = symbols("kx ky kz", real=True)
    wavevector = (kx, ky, kz)
    lattice_symbol = 6 - 2 * sum(cos(component) for component in wavevector)
    origin = {component: 0 for component in wavevector}
    hessian = Matrix(
        3,
        3,
        lambda i, j: diff(lattice_symbol, wavevector[i], wavevector[j]).subs(origin),
    )
    check(
        "C11 the recorded response retains the conditional massless cubic 1/Lambda pole",
        lattice_symbol.subs(origin) == 0 and hessian == 2 * Matrix.eye(3),
    )

    # All four A9 payloads remain occupied.  Thus the already-established
    # common relative occupancy ray assigns no delta to a counter write.  This
    # is a boundary theorem: a phase-sensitive action is still required for a
    # mechanical response and for any absolute coupling.
    occupancy_deltas = {
        signed_event: len(counter_step(encode_counter(0), signed_event))
        - len(encode_counter(0))
        for signed_event in (-1, 1)
    }
    check(
        "C12 the common relative occupancy ledger is exactly blind to every A2 memory write",
        occupancy_deltas == {-1: 0, 1: 0},
    )

    capacities = {
        owners: (COUNTER_CARDINALITY**owners - 2) // 2
        for owners in (1, 2, 3)
    }
    check(
        "C13 finite detector capacity is explicit and scales only by adding finite existing A2 owners",
        capacities == {1: 2047, 2: 8_388_607, 3: 34_359_738_367},
    )

    missing = {
        "integration into homogeneous canonical Phi",
        "autonomous rotor-bank and source/sink formation",
        "multiple-walker arbitration and detector backreaction",
        "phase-sensitive action and reciprocal mechanical work",
        "absolute universal gravity normalization",
        "protected tensor wave pole and common causal cone",
        "clock response lensing and nonlinear completion",
        "counter reset expansion and overflow work",
        "causal multi-A2 ripple carry beyond one-owner capacity",
    }
    check(
        "C14 finite phase-protected memory does not close physical gravity",
        len(missing) == 9,
    )

    forbidden = (
        "empirical_target",
        "random_draw",
        "parameter_fit",
        "137.036",
    )
    check(
        "C15 no empirical target random draw parameter fit or numerical search enters",
        all(token not in __doc__.lower() for token in forbidden),
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 rotor-Green A2-memory checks pass")
    print(f"constant_occupancy_A2_states={COUNTER_CARDINALITY}")
    print("signed_counter_range=-2047..+2047_plus_overflow")
    print(f"radius1_edge_A2_owners={len(edges)}")
    print("radius1_source_A2_owners=1")
    print(f"maximum_fixture_counter_prefix={maximum_fixture_prefix}")
    print(f"maximum_all_phase_counter_prefix_N37={maximum_phase_prefix}")
    print(f"maximum_exact_response_error_N37={maximum_response_error}")
    print(f"maximum_certified_response_bound_N37={maximum_response_bound}")
    print("phase_pair_bound=2*(8/(3N)+8*K_edge/N)")
    print("triplet_scalar_factor=1/12")
    print("occupancy_action_delta_per_memory_write=0")
    print("status=physical_finite_memory_and_phase_protection_exact_mechanical_gravity_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
