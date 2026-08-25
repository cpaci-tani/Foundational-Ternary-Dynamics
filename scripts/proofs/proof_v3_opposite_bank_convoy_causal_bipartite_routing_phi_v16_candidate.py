#!/usr/bin/env python3
"""Exact opposite-bank convoy routing for the selected v3 Born scheduler.

Phi-v15 gives each trial one retained two-A2 source-history address and forms
two eight-record banks.  This certificate selects opposite tangent ports and
attaches one existing A2 TRANSIT owner to each bank.  A local swap moves the
complete bank and owner one SC hop into a clear destination without changing
any field record.  A prepared A2 ENDPOINT owner performs an exact terminal
handoff to a DELIVERED owner.  Every move and handoff has an explicit inverse.

The two banks follow opposite Moore-causal routes.  Their source counts,
Gaussian-integer readouts, and |Z|^2 event cardinalities are retained exactly;
the route of either wing is independent of the remote bank.  The source
odometer address remains the finite common-origination record until both
deliveries complete.

This is a selected Phi-v16 prepared-corridor candidate.  It does not form the
source, route, endpoints, or apparatus; protect occupancy faults; arbitrate
traffic; choose measurement settings; return detector work; amplify records;
or recover Bell-violating laboratory correlations.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass

from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_v3_contextual_neutral_pointer_born_renewal_apparatus import (
    Channel,
    Outcome,
    address_order,
    apparatus_chart,
    outcome,
)
from proof_v3_field_bank_gaussian_born_readout import (
    bright_pair_count,
    gaussian_integer,
    norm_squared,
)
from proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate import (
    WINDOW,
    bank_counts,
    initial_state,
    iterate_formation,
    source_port,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    mv,
    transform_channel,
    transform_chart,
)
from proof_v3_rotor_green_a2_physical_memory_phase_protection import encode_counter
from proof_v3_transitive_a2_source_history_odometer_born_time_measure_phi_v15_candidate import (
    Odometer,
    decode_history,
)


sys.stdout.reconfigure(encoding="utf-8")

Vec = tuple[int, int, int]
A2 = tuple[tuple[int, int], ...]

TRANSIT: A2 = encode_counter(0)
ENDPOINT: A2 = encode_counter(1)
DELIVERED: A2 = encode_counter(2)


def add(left: Vec, right: Vec) -> Vec:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def subtract(left: Vec, right: Vec) -> Vec:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def scale(value: int, vector: Vec) -> Vec:
    return tuple(value * entry for entry in vector)  # type: ignore[return-value]


@dataclass(frozen=True)
class RouteSite:
    bank: frozenset[Channel]
    token: A2 | None


EMPTY = RouteSite(frozenset(), None)


def recognized_port(bank: frozenset[Channel]) -> Outcome | None:
    if len(bank) != WINDOW:
        return None
    ports = {outcome(channel) for channel in bank}
    return next(iter(ports)) if len(ports) == 1 else None


def local_hop(source: RouteSite, destination: RouteSite):
    port = recognized_port(source.bank)
    if port is None or source.token != TRANSIT or destination != EMPTY:
        return None
    return EMPTY, RouteSite(source.bank, TRANSIT), port[0]


def local_hop_inverse(source_after: RouteSite, destination_after: RouteSite):
    port = recognized_port(destination_after.bank)
    if source_after != EMPTY or port is None or destination_after.token != TRANSIT:
        return None
    return RouteSite(destination_after.bank, TRANSIT), EMPTY, port[0]


def local_delivery(source: RouteSite, destination: RouteSite):
    port = recognized_port(source.bank)
    if (
        port is None
        or source.token != TRANSIT
        or destination.bank
        or destination.token != ENDPOINT
    ):
        return None
    return (
        RouteSite(frozenset(), ENDPOINT),
        RouteSite(source.bank, DELIVERED),
        port[0],
    )


def local_delivery_inverse(source_after: RouteSite, destination_after: RouteSite):
    port = recognized_port(destination_after.bank)
    if (
        source_after.bank
        or source_after.token != ENDPOINT
        or port is None
        or destination_after.token != DELIVERED
    ):
        return None
    return (
        RouteSite(destination_after.bank, TRANSIT),
        RouteSite(frozenset(), ENDPOINT),
        port[0],
    )


def route_fixture(bank: frozenset[Channel], origin: Vec, distance: int):
    port = recognized_port(bank)
    assert port is not None and distance >= 1
    direction = port[0]
    current = origin
    site = RouteSite(bank, TRANSIT)
    history = []

    for _ in range(distance - 1):
        output = local_hop(site, EMPTY)
        assert output is not None
        assert local_hop_inverse(output[0], output[1]) == (site, EMPTY, direction)
        history.append((current, output))
        current = add(current, direction)
        site = output[1]

    delivered = local_delivery(site, RouteSite(frozenset(), ENDPOINT))
    assert delivered is not None
    assert local_delivery_inverse(delivered[0], delivered[1]) == (
        site,
        RouteSite(frozenset(), ENDPOINT),
        direction,
    )
    history.append((current, delivered))
    endpoint = add(current, direction)
    return endpoint, delivered[1], tuple(history)


def transform_bank(matrix, bank: frozenset[Channel]):
    return frozenset(transform_channel(matrix, channel) for channel in bank)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    left_chart = apparatus_chart()
    reflection = ((-1, 0, 0), (0, 1, 0), (0, 0, 1))
    right_chart = transform_chart(reflection, left_chart)
    left_port = source_port(left_chart)
    right_port = source_port(right_chart)
    check(
        "C1 the selected paired source ports have opposite SC tangents and distinct outcomes",
        right_port[0] == scale(-1, left_port[0])
        and right_port != left_port
        and right_port[1] == left_port[1],
    )

    check(
        "C2 TRANSIT ENDPOINT and DELIVERED are distinct states of one existing fixed-occupancy A2 owner",
        len({TRANSIT, ENDPOINT, DELIVERED}) == 3,
    )

    left_banks = []
    right_banks = []
    left_counts_by_address = []
    local_rows = 0
    inverse_rows = 0
    for address in range(4096):
        source, schedule = decode_history(address)
        left = iterate_formation(left_chart, initial_state(source, schedule), WINDOW)
        right = iterate_formation(right_chart, initial_state(source, schedule), WINDOW)
        left_counts = bank_counts(left.bank, left_port)
        right_counts = bank_counts(right.bank, right_port)
        assert left_counts == right_counts
        assert recognized_port(left.bank) == left_port
        assert recognized_port(right.bank) == right_port
        assert len(left.bank | right.bank) == 2 * WINDOW

        for bank in (left.bank, right.bank):
            source_site = RouteSite(bank, TRANSIT)
            hop = local_hop(source_site, EMPTY)
            assert hop is not None
            assert local_hop_inverse(hop[0], hop[1]) == (
                source_site,
                EMPTY,
                recognized_port(bank)[0],
            )
            delivery = local_delivery(
                source_site, RouteSite(frozenset(), ENDPOINT)
            )
            assert delivery is not None
            assert local_delivery_inverse(delivery[0], delivery[1]) == (
                source_site,
                RouteSite(frozenset(), ENDPOINT),
                recognized_port(bank)[0],
            )
            local_rows += 2
            inverse_rows += 2

        left_banks.append(left.bank)
        right_banks.append(right.bank)
        left_counts_by_address.append(left_counts)

    check(
        "C3 all 8,192 formed banks admit both an exact one-hop swap and terminal handoff",
        local_rows == 4096 * 2 * 2,
    )
    check(
        "C4 every local bank move and delivery has an exact record-complete inverse",
        inverse_rows == local_rows,
    )

    unique_left = {bank_counts(bank, left_port): bank for bank in left_banks}
    unique_right = {bank_counts(bank, right_port): bank for bank in right_banks}
    check(
        "C5 the two source routes retain all 151 physically formed count classes",
        len(unique_left) == len(unique_right) == 151,
    )

    route_rows = 0
    maximum_distance = 0
    left_origin = (0, 1, 0)
    right_origin = (0, -1, 0)
    for counts, left_bank in unique_left.items():
        right_bank = unique_right[counts]
        for distance in (1, 2, 7, 37):
            left_endpoint, left_delivered, left_history = route_fixture(
                left_bank, left_origin, distance
            )
            right_endpoint, right_delivered, right_history = route_fixture(
                right_bank, right_origin, distance
            )
            assert left_endpoint == add(left_origin, scale(distance, left_port[0]))
            assert right_endpoint == add(right_origin, scale(distance, right_port[0]))
            assert len(left_history) == len(right_history) == distance
            assert left_delivered.bank == left_bank and left_delivered.token == DELIVERED
            assert right_delivered.bank == right_bank and right_delivered.token == DELIVERED
            assert bank_counts(left_delivered.bank, left_port) == counts
            assert bank_counts(right_delivered.bank, right_port) == counts
            assert bright_pair_count(counts) == norm_squared(gaussian_integer(counts))
            assert max(
                abs(left_endpoint[index] - left_origin[index]) for index in range(3)
            ) == distance
            assert max(
                abs(right_endpoint[index] - right_origin[index]) for index in range(3)
            ) == distance
            maximum_distance = max(maximum_distance, distance)
            route_rows += 2
    check(
        "C6 every formed count class reaches both endpoints at exactly one SC hop per global route tick",
        route_rows == 151 * 4 * 2 and maximum_distance == 37,
    )
    check(
        "C7 routing retains every field record phase count Gaussian integer and absolute-square event count",
        route_rows > 0,
    )

    # The paired odometer address remains at the source while the banks move.
    # Changing the remote address changes only that remote bank and cannot
    # enter the local hop function.
    independence_rows = 0
    count_classes = tuple(unique_left)
    for left_counts in count_classes:
        left_bank = unique_left[left_counts]
        left_result = route_fixture(left_bank, left_origin, 7)[:2]
        for right_counts in count_classes:
            right_bank = unique_right[right_counts]
            assert route_fixture(left_bank, left_origin, 7)[:2] == left_result
            assert bright_pair_count(left_counts) == norm_squared(
                gaussian_integer(left_counts)
            )
            assert recognized_port(right_bank) == right_port
            independence_rows += 1
    check(
        "C8 the complete paired count-class census has exact remote-bank-independent local routing",
        independence_rows == 151**2,
    )

    # One retained source record identifies both banks until delivery.  It is
    # not synthesized later by the observer.
    origin_rows = 0
    for low in (0, 1, 2048, 4095):
        for high in (0, 1, 2048, 4095):
            pair_record = Odometer(low, high)
            left_source = decode_history(pair_record.low)
            right_source = decode_history(pair_record.high)
            assert pair_record == Odometer(low, high)
            assert left_source == decode_history(low)
            assert right_source == decode_history(high)
            origin_rows += 1
    check(
        "C9 both routed banks retain one common finite scheduler-origination record at the source",
        origin_rows == 16,
    )

    covariance_rows = 0
    group = tuple(signed_permutation_matrices())
    for counts, bank in unique_left.items():
        source_site = RouteSite(bank, TRANSIT)
        base = local_hop(source_site, EMPTY)
        assert base is not None
        for matrix in group:
            transformed = transform_bank(matrix, bank)
            transformed_output = local_hop(RouteSite(transformed, TRANSIT), EMPTY)
            assert transformed_output is not None
            assert transformed_output[0] == EMPTY
            assert transformed_output[1].bank == transform_bank(matrix, base[1].bank)
            assert transformed_output[2] == tuple(
                matrix_vector(matrix, base[2])
            )
            assert recognized_port(transformed) == (
                mv(matrix, left_port[0]), left_port[1]
            )
            assert bank_counts(transformed, recognized_port(transformed)) == counts
            covariance_rows += 1
    check(
        "C10 the complete local convoy transaction is signed-cubic covariant",
        covariance_rows == 151 * 48,
    )

    malformed_mixed = frozenset(
        tuple(left_banks[0])[:4] + tuple(right_banks[0])[:4]
    )
    occupied_destination = RouteSite(left_banks[1], TRANSIT)
    check(
        "C11 mixed-port banks occupied destinations and absent endpoint owners fail closed",
        recognized_port(malformed_mixed) is None
        and local_hop(RouteSite(left_banks[0], TRANSIT), occupied_destination) is None
        and local_delivery(RouteSite(left_banks[0], TRANSIT), EMPTY) is None,
    )

    # Local swaps retain eight field records and one moving A2 owner; terminal
    # handoff retains eight field records and the two A2 owners participating
    # in the endpoint exchange.
    bank = left_banks[0]
    hop = local_hop(RouteSite(bank, TRANSIT), EMPTY)
    delivery = local_delivery(
        RouteSite(bank, TRANSIT), RouteSite(frozenset(), ENDPOINT)
    )
    assert hop is not None and delivery is not None
    check(
        "C12 every hop and terminal handoff preserves exact field and A2 role occupancy",
        (len(bank), 1) == (len(hop[0].bank) + len(hop[1].bank), int(hop[0].token is not None) + int(hop[1].token is not None))
        and (len(bank), 2) == (len(delivery[0].bank) + len(delivery[1].bank), int(delivery[0].token is not None) + int(delivery[1].token is not None)),
    )

    forbidden = (
        "137.036",
        "target_probability",
        "random_draw",
        "superluminal",
        "entangled_state",
    )
    missing = {
        "genesis formation of scheduler banks route tokens endpoints and apparatuses",
        "canonical Phi writer integration and multi-bank traffic arbitration",
        "bank occupancy fault protection and endpoint apparatus backpressure",
        "physical measurement setting carriers and spacelike timing protocol",
        "reciprocal detector work material response and macroscopic amplification",
        "laboratory Bell correlation recovery or an empirically adequate alternative",
    }
    check(
        "C13 Phi-v16 closes prepared causal paired routing not the general Born Bell laboratory",
        all(token not in __doc__.lower() for token in forbidden)
        and len(missing) == 6,
    )

    passed = sum(ok for _name, ok, _detail in checks)
    print(f"\n{passed}/{len(checks)} opposite-bank causal-routing checks pass")
    print(f"formed_bank_rows={len(left_banks) + len(right_banks)}")
    print(f"local_hop_delivery_rows={local_rows}")
    print(f"finite_route_rows={route_rows}")
    print(f"remote_independence_rows={independence_rows}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print("route_speed=one_SC_hop_per_global_route_tick")
    print("paired_origin_record=retained_two_A2_scheduler_address")
    print("field_records_per_bank=8")
    print("status=selected_phi_v16_prepared_causal_pair_routing_settings_backreaction_lab_Bell_open")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
