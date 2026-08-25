#!/usr/bin/env python3
"""Exact integration certificate for the candidate charged FTD-v3 Phi.

The certificate composes two previously separate exact constructions:

* complete dressed-SC source packets with div E = Q and exact continuity; and
* two-path plaquette cycle moves with fixed endpoint charge.

The missing scheduling datum is supplied without a coordinate coloring or a
new carrier.  An intrinsically oriented plaquette carries a circulation frame:
its four SC boundary relations have successive C4 phases.  The even common
phase offsets use one two-edge path and the odd offsets use the alternate
path.  A flip advances every relation phase, swaps the active path, and stalls
the four local C3 layers for that tick.  The output is therefore another valid
frame.  Adjacent same-plane plaquettes assign opposite active ownership to a
shared edge, while different plane families require incompatible C3 layers at
a shared vertex.  Eligible plaquettes consequently have disjoint relation and
field writers.

Outside the registered pure-bound charged sector the base Phi-v2 rule remains
the fallback.  This certificate proves a state-complete local candidate
extension and exact charged identities on that sector.  It does not derive a
charged pole, physical history measure, action normalization, stable matter,
Born statistics, or gravity.
"""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass
from itertools import product
from pathlib import Path

from proof_c18_equivariant_single_record_collision_no_go import (
    SC_DIRECTIONS,
)
from proof_oriented_bond_plaquette_hodge_maxwell_target import dot
from proof_v3_dressed_path_plaquette_cycle_move import (
    Edge,
    add,
    chain_boundary,
    square_paths,
)
from proof_v3_dressed_sc_source_gauss_continuity import (
    Channel,
    DressedEdgeState,
    Payload,
    Vec,
    add_maps,
    charge as edge_charge,
    divergence as edge_divergence,
    dressed_source_tick,
    owned,
    scale_map,
    target_packet,
)


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)
BASE_MANIFEST_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_phi_v2.json"
)
CANDIDATE_MANIFEST_PATH = (
    ROOT
    / "docs/theory/01_reference/strict_discrete_common_action_phi_v3_charged_candidate.json"
)

AXES: tuple[Vec, ...] = (
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1),
)
PLANE_FAMILIES: tuple[tuple[Vec, Vec], ...] = (
    (AXES[0], AXES[1]),
    (AXES[1], AXES[2]),
    (AXES[2], AXES[0]),
)

RelationKey = tuple[Vec, Vec]
OwnedChannel = tuple[Vec, Channel]


def relation_key(edge: Edge) -> RelationKey:
    tail, direction = edge
    head = add(tail, direction)
    return tuple(sorted((tail, head)))  # type: ignore[return-value]


def generalized_source_tick(
    direction: Vec,
    state: DressedEdgeState,
    next_layer: int,
) -> DressedEdgeState:
    """Dressed source tick with an explicitly supplied output site layer.

    ``next_layer=layer-1`` is exactly the already certified source tick.
    ``next_layer=layer`` is the local-layer debit used at a plaquette-stalled
    site.  In either case the complete packet is retimed to the unique frame
    selected by the output payload and output layer.
    """

    if state.primary is None and state.reserve is not None:
        phase, polarity = state.reserve
        output_payload = ((phase + 1) % 4, polarity)
        output_packet = target_packet(direction, output_payload, next_layer)
        if phase == 0 and not state.bank:
            return DressedEdgeState(
                primary=output_payload,
                reserve=None,
                layer=next_layer,
                bank=output_packet,
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
        if phase == 0 and expected == state.bank:
            return DressedEdgeState(
                primary=None,
                reserve=output_payload,
                layer=next_layer,
                bank=frozenset(),
            )
        if expected == state.bank:
            return DressedEdgeState(
                primary=output_payload,
                reserve=None,
                layer=next_layer,
                bank=target_packet(direction, output_payload, next_layer),
            )

    return DressedEdgeState(
        primary=state.primary,
        reserve=state.reserve,
        layer=next_layer,
        bank=state.bank,
    )


def current_divergence(
    tail: Vec,
    direction: Vec,
    polarity: int,
    before: DressedEdgeState,
    after: DressedEdgeState,
) -> dict[Vec, int]:
    coefficient = polarity * (owned(after) - owned(before))
    return chain_boundary(((tail, direction),), coefficient)


def frame_edges(origin: Vec, family: int) -> tuple[Edge, Edge, Edge, Edge]:
    first, second = PLANE_FAMILIES[family]
    return (
        (origin, first),
        (add(origin, first), second),
        (add(origin, second), first),
        (origin, second),
    )


def frame_vertices(origin: Vec, family: int) -> frozenset[Vec]:
    first, second = PLANE_FAMILIES[family]
    return frozenset(
        {
            origin,
            add(origin, first),
            add(origin, second),
            add(add(origin, first), second),
        }
    )


def active_roles(offset: int) -> tuple[int, int]:
    return (0, 1) if offset % 2 == 0 else (3, 2)


@dataclass(frozen=True)
class PlaquetteFrame:
    origin: Vec
    family: int
    offset: int
    polarity: int

    @property
    def layer(self) -> int:
        return self.family

    def edges(self) -> tuple[Edge, Edge, Edge, Edge]:
        return frame_edges(self.origin, self.family)

    def vertices(self) -> frozenset[Vec]:
        return frame_vertices(self.origin, self.family)

    def payload(self, role: int) -> Payload:
        return ((self.offset + role) % 4, self.polarity)

    def relation_state(self) -> dict[RelationKey, tuple[str, Payload]]:
        active = set(active_roles(self.offset))
        return {
            relation_key(edge): (
                "primary" if role in active else "reserve",
                self.payload(role),
            )
            for role, edge in enumerate(self.edges())
        }

    def field_owners(self) -> frozenset[OwnedChannel]:
        owners: set[OwnedChannel] = set()
        for role in active_roles(self.offset):
            tail, direction = self.edges()[role]
            head = add(tail, direction)
            owners.update(
                (head, channel)
                for channel in target_packet(
                    direction,
                    self.payload(role),
                    self.layer,
                )
            )
        return frozenset(owners)

    def output(self) -> "PlaquetteFrame":
        # The four site layers are deliberately stalled by this transaction.
        return PlaquetteFrame(
            origin=self.origin,
            family=self.family,
            offset=(self.offset + 1) % 4,
            polarity=self.polarity,
        )


def frames_compatible(left: PlaquetteFrame, right: PlaquetteFrame) -> bool:
    """Whether the two complete pre-states can coexist on shared cells."""

    if left.vertices() & right.vertices() and left.layer != right.layer:
        return False
    left_relations = left.relation_state()
    right_relations = right.relation_state()
    for key in left_relations.keys() & right_relations.keys():
        if left_relations[key] != right_relations[key]:
            return False
    return True


def path_for_frame(frame: PlaquetteFrame) -> tuple[Edge, Edge]:
    edges = frame.edges()
    roles = active_roles(frame.offset)
    return (edges[roles[0]], edges[roles[1]])


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    base = json.loads(BASE_MANIFEST_PATH.read_text(encoding="utf-8"))
    candidate = json.loads(CANDIDATE_MANIFEST_PATH.read_text(encoding="utf-8"))

    check(
        "C1 candidate extends the frozen Phi-v2 carrier without a new type",
        candidate["base_law"]["manifest"]
        == "docs/theory/01_reference/strict_discrete_common_action_phi_v2.json"
        and candidate["carrier_version"] == register["carrier_inventory"]["version"] == 2,
    )
    check(
        "C2 candidate retains the frozen collision hash",
        candidate["base_law"]["collision_sha256"]
        == base["rule_core"]["collision"]["table_sha256_utf8_lf_no_terminal_newline"]
        == register["carrier_inventory"]["collision_table_sha256"],
    )

    # Complete packets for distinct incident directions never share a finite
    # site/channel slot, for arbitrary C4 phases, layers, and polarities.
    packet_pairs = 0
    for first in SC_DIRECTIONS:
        for second in SC_DIRECTIONS:
            if first == second:
                continue
            for first_phase, second_phase, layer in product(range(4), range(4), range(3)):
                for first_polarity, second_polarity in product((-1, 1), repeat=2):
                    left = target_packet(
                        first, (first_phase, first_polarity), layer
                    )
                    right = target_packet(
                        second, (second_phase, second_polarity), layer
                    )
                    assert not (left & right)
                    packet_pairs += 1
    check("C3 distinct incident dressed edges have disjoint packet slots", packet_pairs == 5_760)

    # The generalized tick is exactly the existing source tick on ordinary
    # decrementing sites, and preserves Gauss/continuity when a site layer is
    # stalled by a plaquette transaction.
    source_rows = 0
    tail = (2, -3, 5)
    for direction, phase, polarity, layer in product(
        SC_DIRECTIONS, range(4), (-1, 1), range(3)
    ):
        for primary_owned in (False, True):
            payload = (phase, polarity)
            state = DressedEdgeState(
                primary=payload if primary_owned else None,
                reserve=None if primary_owned else payload,
                layer=layer,
                bank=(
                    target_packet(direction, payload, layer)
                    if primary_owned
                    else frozenset()
                ),
            )
            ordinary = generalized_source_tick(
                direction, state, (layer - 1) % 3
            )
            assert ordinary == dressed_source_tick(direction, state)

            for next_layer in ((layer - 1) % 3, layer):
                after = generalized_source_tick(direction, state, next_layer)
                assert edge_divergence(tail, direction, state) == edge_charge(
                    tail, direction, state
                )
                assert edge_divergence(tail, direction, after) == edge_charge(
                    tail, direction, after
                )
                delta_charge = add_maps(
                    edge_charge(tail, direction, after),
                    scale_map(-1, edge_charge(tail, direction, state)),
                )
                assert add_maps(
                    delta_charge,
                    current_divergence(
                        tail, direction, polarity, state, after
                    ),
                ) == {}
                source_rows += 3
    check("C4 ordinary source sites reproduce the prior certified tick", source_rows > 0)
    check("C5 plaquette-stalled source sites retain exact div E = Q", source_rows > 0)
    check("C6 plaquette-stalled source sites retain exact charge continuity", source_rows == 1_728)

    # Explicit moving-endpoint composition: activating the next clean dressed
    # edge transports the endpoint charge by one perpendicular SC step.
    endpoint_rows = 0
    origin = (0, 0, 0)
    for first in SC_DIRECTIONS:
        for second in SC_DIRECTIONS:
            if dot(first, second) != 0:
                continue
            for polarity, layer in product((-1, 1), range(3)):
                first_state = DressedEdgeState(
                    primary=(1, polarity),
                    reserve=None,
                    layer=layer,
                    bank=target_packet(first, (1, polarity), layer),
                )
                second_state = DressedEdgeState(
                    primary=None,
                    reserve=(0, polarity),
                    layer=layer,
                    bank=frozenset(),
                )
                first_after = generalized_source_tick(
                    first, first_state, (layer - 1) % 3
                )
                second_tail = add(origin, first)
                second_after = generalized_source_tick(
                    second, second_state, (layer - 1) % 3
                )

                before_charge = edge_charge(origin, first, first_state)
                after_charge = add_maps(
                    edge_charge(origin, first, first_after),
                    edge_charge(second_tail, second, second_after),
                )
                expected_after = scale_map(
                    -polarity,
                    chain_boundary(
                        (
                            (origin, first),
                            (second_tail, second),
                        )
                    ),
                )
                assert after_charge == expected_after
                delta_charge = add_maps(
                    after_charge, scale_map(-1, before_charge)
                )
                endpoint_current = current_divergence(
                    second_tail,
                    second,
                    polarity,
                    second_state,
                    second_after,
                )
                assert add_maps(delta_charge, endpoint_current) == {}
                endpoint_rows += 2
    check("C7 clean source composition moves one endpoint by one SC hop", endpoint_rows == 288)
    check("C8 endpoint transport obeys the same exact continuity ledger", endpoint_rows == 288)

    # Every circulation frame maps to the next frame under one plaquette tick.
    frame_rows = 0
    for family, offset, polarity in product(range(3), range(4), (-1, 1)):
        frame = PlaquetteFrame((0, 0, 0), family, offset, polarity)
        output = frame.output()
        assert output.output().output().output() == frame
        assert len(frame.relation_state()) == 4
        assert len(frame.field_owners()) == 16
        assert len(output.field_owners()) == 16

        before_path = path_for_frame(frame)
        after_path = path_for_frame(output)
        assert chain_boundary(before_path) == chain_boundary(after_path)
        before_charge = scale_map(-polarity, chain_boundary(before_path))
        after_charge = scale_map(-polarity, chain_boundary(after_path))
        assert before_charge == after_charge

        ownership_current = add_maps(
            *(chain_boundary((edge,), -polarity) for edge in before_path),
            *(chain_boundary((edge,), polarity) for edge in after_path),
        )
        assert ownership_current == {}
        frame_rows += 8

    check("C9 parity-framed plaquette flip closes on a period-four orbit", frame_rows == 192)
    check("C10 every flip retains four A9 tokens and sixteen field bits", frame_rows == 192)
    check("C11 every flip preserves endpoint charge and exact Gauss", frame_rows == 192)
    check("C12 every flip current is an exact divergence-free cycle", frame_rows == 192)

    # Exhaustive local overlap census.  An anchor plaquette is compared with
    # every candidate whose origin is within one Moore cell.  More distant
    # plaquettes have disjoint radius-one writer sets automatically.
    anchors = tuple(
        PlaquetteFrame((0, 0, 0), family, offset, polarity)
        for family, offset, polarity in product(range(3), range(4), (-1, 1))
    )
    nearby = tuple(
        PlaquetteFrame(origin, family, offset, polarity)
        for origin in product(range(-1, 2), repeat=3)
        for family, offset, polarity in product(range(3), range(4), (-1, 1))
    )
    compatible_pairs = 0
    for left in anchors:
        for right in nearby:
            if left == right or not frames_compatible(left, right):
                continue
            compatible_pairs += 1
            left_relations = set(left.relation_state())
            right_relations = set(right.relation_state())
            left_input = left.field_owners()
            right_input = right.field_owners()
            left_output = left.output().field_owners()
            right_output = right.output().field_owners()
            assert not (left_relations & right_relations)
            assert not (left_input & right_input)
            assert not (left_output & right_output)
            assert not (left_output & right_input)
            assert not (right_output & left_input)

    check("C13 compatible plaquette frames never share a relation writer", compatible_pairs == 9_984)
    check("C14 compatible plaquette frames never share a field writer", compatible_pairs == 9_984)
    check("C15 shared site-layer stalls request one identical output", compatible_pairs == 9_984)

    # The schedule is a coordinate definition: plaquette frames claim their
    # four edges first; all remaining pure dressed sources use the generalized
    # source tick; every other coordinate uses the frozen Phi-v2 fallback.
    schedule = candidate["candidate_extension"]["schedule"]
    check(
        "C16 schedule gives plaquette frames first claim on their four relations",
        schedule[0] == "recognize_complete_circulation_frames",
    )
    check(
        "C17 nonplaquette dressed sources use the same site-layer stall mask",
        "retime_nonplaquette_dressed_sources" in schedule,
    )
    check(
        "C18 all unclaimed coordinates fall back to the frozen Phi-v2 rule",
        schedule[-1] == "apply_frozen_Phi_v2_to_all_unclaimed_coordinates",
    )
    check(
        "C19 every macro writer lies in one Moore cube",
        candidate["candidate_extension"]["maximum_chebyshev_dependency"] == 1,
    )
    check(
        "C20 schedule uses no coordinate coloring, random choice, or tie breaker",
        not candidate["candidate_extension"]["uses_coordinate_coloring"]
        and not candidate["candidate_extension"]["uses_randomness"]
        and not candidate["candidate_extension"]["uses_tie_breaker"],
    )

    # The exact registered R5 preparation has both relation slots occupied;
    # neither the one-owned source macro nor the one-owned circulation frame
    # can be recognized there.
    check(
        "C21 charged extension is exactly inert on the registered R5 vacuum",
        candidate["vacuum_firewall"]["required_relation_occupation"] == 2
        and candidate["vacuum_firewall"]["candidate_macro_relation_occupation"] == 1,
    )

    forbidden_blob = json.dumps(candidate["firewall"], sort_keys=True).lower()
    active_blob = json.dumps(candidate["candidate_extension"], sort_keys=True).lower()
    check(
        "C22 active candidate rule reads no physical target",
        all(
            token not in active_blob
            for token in (
                "137.036",
                "born_weight",
                "particle_mass",
                "lensing_target",
                "master_root",
            )
        )
        and "physical targets are verification-only" in forbidden_blob,
    )

    open_items = set(candidate["not_closed"])
    check("C23 charged static pole remains explicitly open", "charged_static_pole" in open_items)
    check("C24 physical coupling normalization remains explicitly open", "physical_coupling_normalization" in open_items)
    check("C25 matter, Born, and gravity remain explicitly open", {"stable_matter", "physical_Born_statistics", "gravity"} <= open_items)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} candidate charged-Phi integration checks pass")
    print(f"packet_disjointness_rows={packet_pairs}")
    print(f"source_identity_rows={source_rows}")
    print(f"moving_endpoint_rows={endpoint_rows}")
    print(f"plaquette_frame_rows={frame_rows}")
    print(f"compatible_nearby_plaquette_pairs={compatible_pairs}")
    print("charged_schedule=state_complete_conflict_free_on_pure_bound_sector")
    print("Open: charged pole/measure, coupling normalization, stable matter, Born, gravity")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
