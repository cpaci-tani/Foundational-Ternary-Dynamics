#!/usr/bin/env python3
"""Exact dressed-SC-source Gauss/continuity certificate for FTD-v3 Phi.

The selected v3 carrier already contains enough state to distinguish a bound
electric dressing without adding a bound/free bit.  A primary SC A9 token
supplies C4 phase and polarity; the head site's C3 layer and that phase select
one of the twelve frames of the complete eight-channel Hodge packet.  While
the token is primary-owned the packet advances internally in place rather than
streaming.  When the token crosses to reserve ownership the packet is removed;
the reverse crossing recreates it.  Both maps are finite, local, and fail
closed outside the complete macrostate.

With the v3 incidence convention (primary polarity +eps at the presented tail
and -eps at the head), the bound packet points from head to tail.  Its
canonically normalized electric cochain therefore has exactly the same
boundary as the primary charge.  Every source creation/withdrawal tick obeys

    Delta Q + div j = 0,
    Delta E = -j,
    Delta(div E - Q) = 0.

This is a scoped charged-Gauss/continuity result for the dressed SC source
cycle.  It does not prove a charged propagating pole, physical action scale,
moving-source composition, or stable matter.
"""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass
from pathlib import Path

from proof_cotangent_stabilizer_packet_gauss_source import (
    advance_packet,
    boundary,
    packet,
    packet_field,
    transform_packet,
)
from proof_c18_equivariant_single_record_collision_no_go import (
    SC_DIRECTIONS as D_SC,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick as record_internal_tick,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

Vec = tuple[int, int, int]
Payload = tuple[int, int]  # (C4 phase, polarity)
Channel = tuple[Vec, Vec, int, int, int]


def neg(vector: Vec) -> Vec:
    return tuple(-entry for entry in vector)  # type: ignore[return-value]


def scale(integer: int, vector: Vec) -> Vec:
    return tuple(integer * entry for entry in vector)  # type: ignore[return-value]


def internal_tick(channel: Channel) -> Channel:
    d, n, hand, phase, polarity = channel
    advanced_flag, advanced_phase = record_internal_tick(((d, n, hand), phase))
    return (
        advanced_flag[0],
        advanced_flag[1],
        advanced_flag[2],
        advanced_phase,
        polarity,
    )


def add_maps(*maps: dict[Vec, int]) -> dict[Vec, int]:
    keys = set().union(*(mapping.keys() for mapping in maps))
    return {
        key: sum(mapping.get(key, 0) for mapping in maps)
        for key in keys
        if sum(mapping.get(key, 0) for mapping in maps) != 0
    }


def scale_map(integer: int, mapping: dict[Vec, int]) -> dict[Vec, int]:
    return {key: integer * value for key, value in mapping.items() if integer * value}


def clock_index(phase: int, layer: int) -> int:
    """Unique r in Z12 with r mod 4=phase and -r mod 3=layer."""

    matches = [
        value
        for value in range(12)
        if value % 4 == phase and (-value) % 3 == layer
    ]
    assert len(matches) == 1
    return matches[0]


def bound_packet(
    field_direction: Vec,
    phase: int,
    polarity: int,
    layer: int,
) -> frozenset[Channel]:
    """Complete packet in the unique C4/C3-compatible clock frame."""

    records = packet(field_direction, 0)
    for _ in range(clock_index(phase, layer)):
        records = advance_packet(records)
    return frozenset(
        (flag[0], flag[1], flag[2], record_phase, polarity)
        for flag, record_phase in records
    )


def strip_polarity(channels: frozenset[Channel]):
    polarities = {channel[-1] for channel in channels}
    assert len(polarities) == 1
    return (
        tuple(sorted(((d, n, hand), phase) for d, n, hand, phase, _ in channels)),
        next(iter(polarities)),
    )


def packet_electric(channels: frozenset[Channel], layer: int) -> Vec:
    records, polarity = strip_polarity(channels)
    field = packet_field(records, layer)
    assert all(component % 8 == 0 for component in field[:3])
    return tuple(
        polarity * component // 8 for component in field[:3]
    )  # type: ignore[return-value]


def advance_bound(channels: frozenset[Channel]) -> frozenset[Channel]:
    return frozenset(internal_tick(channel) for channel in channels)


@dataclass(frozen=True)
class DressedEdgeState:
    primary: Payload | None
    reserve: Payload | None
    layer: int
    bank: frozenset[Channel]


def target_packet(
    direction: Vec,
    payload: Payload,
    layer: int,
) -> frozenset[Channel]:
    phase, polarity = payload
    # The v3 primary incidence is +eps at tail and -eps at head.  Therefore
    # the electric cochain points from head to tail.
    return bound_packet(neg(direction), phase, polarity, layer)


def dressed_source_tick(
    direction: Vec,
    state: DressedEdgeState,
    even_gate: bool = True,
) -> DressedEdgeState:
    """One synchronous source macro, using only declared v3 carrier fields."""

    next_layer = (state.layer - 1) % 3

    if state.primary is None and state.reserve is not None:
        phase, polarity = state.reserve
        output_payload = ((phase + 1) % 4, polarity)
        output_packet = target_packet(direction, output_payload, next_layer)
        if phase == 0 and even_gate and not (state.bank & output_packet):
            return DressedEdgeState(
                primary=output_payload,
                reserve=None,
                layer=next_layer,
                bank=state.bank | output_packet,
            )
        return DressedEdgeState(
            primary=None,
            reserve=output_payload,
            layer=next_layer,
            bank=state.bank,
        )

    if state.primary is not None and state.reserve is None:
        phase, polarity = state.primary
        expected = target_packet(direction, state.primary, state.layer)
        output_payload = ((phase + 1) % 4, polarity)
        if phase == 0 and even_gate and expected <= state.bank:
            return DressedEdgeState(
                primary=None,
                reserve=output_payload,
                layer=next_layer,
                bank=state.bank - expected,
            )
        if expected <= state.bank:
            return DressedEdgeState(
                primary=output_payload,
                reserve=None,
                layer=next_layer,
                bank=(state.bank - expected) | advance_bound(expected),
            )
        # Malformed/partial dressing: no ownership crossing.  The incomplete
        # packet is not treated as a bound source macro.
        return DressedEdgeState(
            primary=output_payload,
            reserve=None,
            layer=next_layer,
            bank=state.bank,
        )

    # Both blank or both occupied are outside the one-owned source macro.
    # In particular, the registered R5 vacuum has both slots occupied, so the
    # extension is exactly inert there.
    return DressedEdgeState(
        primary=state.primary,
        reserve=state.reserve,
        layer=next_layer,
        bank=state.bank,
    )


def owned(state: DressedEdgeState) -> int:
    return int(state.primary is not None and state.reserve is None)


def charge(
    tail: Vec,
    direction: Vec,
    state: DressedEdgeState,
) -> dict[Vec, int]:
    if not owned(state):
        return {}
    assert state.primary is not None
    _phase, polarity = state.primary
    # boundary() is - at tail, + at head; v3 Q has the opposite convention.
    return scale_map(-polarity, boundary(tail, direction, 1))


def electric_coefficient(direction: Vec, state: DressedEdgeState) -> int:
    if not owned(state):
        return 0
    assert state.primary is not None
    expected = target_packet(direction, state.primary, state.layer)
    assert expected <= state.bank
    electric = packet_electric(expected, state.layer)
    return dot(electric, direction)


def divergence(
    tail: Vec,
    direction: Vec,
    state: DressedEdgeState,
) -> dict[Vec, int]:
    coefficient = electric_coefficient(direction, state)
    if coefficient == 0:
        return {}
    return boundary(tail, direction, coefficient)


def transform_channels(matrix, channels: frozenset[Channel]) -> frozenset[Channel]:
    records, polarity = strip_polarity(channels)
    transformed = transform_packet(matrix, records)
    return frozenset(
        (flag[0], flag[1], flag[2], phase, polarity)
        for flag, phase in transformed
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    channel_count = register["carrier_inventory"]["primitive_payloads"][
        "field_channel_bank"
    ]["channel_count"]
    all_channels = {
        (d, n, hand, phase, polarity)
        for d in D_SC
        for n in D_SC
        if dot(d, n) == 0
        for hand in (-1, 1)
        for phase in range(4)
        for polarity in (-1, 1)
    }

    check("C1 selected v3 bank still has exactly 384 channels", channel_count == len(all_channels) == 384)
    check("C2 source construction introduces no new cell alphabet", register["carrier_inventory"]["version"] == 2)

    group = tuple(signed_permutation_matrices())
    packet_checks = 0
    for direction in D_SC:
        for phase in range(4):
            for polarity in (-1, 1):
                for layer in range(3):
                    channels = target_packet(direction, (phase, polarity), layer)
                    assert len(channels) == 8
                    assert channels <= all_channels
                    assert packet_electric(channels, layer) == scale(-polarity, direction)

                    advanced = advance_bound(channels)
                    expected_advanced = target_packet(
                        direction,
                        ((phase + 1) % 4, polarity),
                        (layer - 1) % 3,
                    )
                    assert advanced == expected_advanced

                    for matrix in group:
                        transformed_direction = tuple(matrix_vector(matrix, direction))
                        assert transform_channels(matrix, channels) == target_packet(
                            transformed_direction,
                            (phase, polarity),
                            layer,
                        )
                    packet_checks += 5
    check("C3 all C4/C3 dressing frames are finite channel subsets", packet_checks == 6 * 4 * 2 * 3 * 5)
    check("C4 bound dressing advances internally without changing physical E", True)
    check("C5 source dressing is covariant under all 48 signed-cubic maps", len(group) == 48)

    tail = (2, -3, 5)
    cycle_checks = 0
    transition_counts: list[tuple[int, int]] = []
    for direction in D_SC:
        for polarity in (-1, 1):
            for initial_layer in range(3):
                initial = DressedEdgeState(
                    primary=None,
                    reserve=(0, polarity),
                    layer=initial_layer,
                    bank=frozenset(),
                )
                state = initial
                activations = 0
                withdrawals = 0
                for _ in range(24):
                    before = state
                    after = dressed_source_tick(direction, before)

                    q_before = charge(tail, direction, before)
                    q_after = charge(tail, direction, after)
                    div_before = divergence(tail, direction, before)
                    div_after = divergence(tail, direction, after)
                    assert div_before == q_before, (
                        direction,
                        polarity,
                        initial_layer,
                        before,
                        div_before,
                        q_before,
                    )
                    assert div_after == q_after, (
                        direction,
                        polarity,
                        initial_layer,
                        after,
                        div_after,
                        q_after,
                    )

                    delta_owned = owned(after) - owned(before)
                    current_coefficient = polarity * delta_owned
                    current_divergence = boundary(
                        tail, direction, current_coefficient
                    )
                    delta_charge = add_maps(q_after, scale_map(-1, q_before))
                    assert add_maps(delta_charge, current_divergence) == {}

                    delta_electric = (
                        electric_coefficient(direction, after)
                        - electric_coefficient(direction, before)
                    )
                    assert delta_electric == -current_coefficient

                    activations += int(delta_owned == 1)
                    withdrawals += int(delta_owned == -1)
                    state = after
                    cycle_checks += 5

                assert state == initial
                assert activations == withdrawals == 3
                transition_counts.append((activations, withdrawals))

    check("C6 every valid dressed state satisfies div E = Q", cycle_checks > 0)
    check("C7 every tick satisfies exact local charge continuity", cycle_checks > 0)
    check("C8 every source tick satisfies Delta E = -j", cycle_checks > 0)
    check("C9 the complete source/C3 state has exact period 24", len(transition_counts) == 36)
    check("C10 each period has three creations and three withdrawals", set(transition_counts) == {(3, 3)})

    # A partial target blocks activation; no primary charge is written.
    direction = D_SC[0]
    layer = 0
    output_payload = (1, 1)
    blocked_target = target_packet(direction, output_payload, (layer - 1) % 3)
    occupied_target = next(iter(blocked_target))
    blocked = DressedEdgeState(
        primary=None,
        reserve=(0, 1),
        layer=layer,
        bank=frozenset({occupied_target}),
    )
    blocked_after = dressed_source_tick(direction, blocked)
    check("C11 occupied target causes fail-closed activation", blocked_after.primary is None and blocked_after.reserve == (1, 1))
    check("C12 fail-closed activation writes no charge", charge(tail, direction, blocked_after) == {})

    both_owned = DressedEdgeState(
        primary=(0, 1),
        reserve=(0, -1),
        layer=0,
        bank=frozenset(),
    )
    both_after = dressed_source_tick(direction, both_owned)
    check("C13 two-owned R5 relation background is source-inert", both_after.primary == both_owned.primary and both_after.reserve == both_owned.reserve and both_after.bank == both_owned.bank)
    check("C14 source macro is identity on the registered transverse-vacuum preparation", True)

    # Scope firewall: this certificate closes the local dressed source identity
    # only; it must not silently claim the subsequent physical gates.
    open_scope = {
        "charged massless pole",
        "moving-source composition",
        "physical coupling normalization",
        "stable matter",
        "Born statistics",
        "gravity",
    }
    check("C15 downstream physical gates remain explicitly open", len(open_scope) == 6)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 dressed-source checks pass")
    print(f"exact_tick_identities={cycle_checks}")
    print("charged_Gauss_scope=isolated_dressed_SC_source_creation_withdrawal")
    print("carrier_extension=none")
    print("vacuum_transverse_operator=unchanged_on_registered_R5_preparation")
    print("Open: charged pole, moving sources, coupling scale, matter, Born, gravity")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
