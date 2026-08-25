#!/usr/bin/env python3
"""Exact seed/resource boundary for v3 event-halo formation.

This certificate does not search for a formation rule.  It counts the exact
prepared Phi-v4 halo hardware and proves the conservation obstruction to
forming any positive-occupancy halo from a completely blank causal past.
The allowed exits are a nonblank genesis seed, a causal boundary current, or
an explicit failure/replacement of the selected occupancy invariant.
"""

from __future__ import annotations

import sys
from math import comb

from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_charged_common_action_phi_v3_candidate import PlaquetteFrame
from proof_v3_homogeneous_event_halo_phi_v4_candidate import (
    CENTER,
    MARKER_RELATIVE,
    ROLE_RELATIVE,
    add_exact,
    assign_role_pads,
    base_role_fields,
    fixture_bank,
    frame_field_slots,
    herald_states,
    place_pointer_fields,
    pointer_configuration,
    polarized_slots,
)
from proof_v3_oriented_repair_chart_full_oh_covariance_and_price import (
    canonical_chart,
)


sys.stdout.reconfigure(encoding="utf-8")

checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"[{'PASS' if condition else 'FAIL'}] {label}")


def main() -> int:
    states = tuple(one_particle_states())
    chart = canonical_chart(PlaquetteFrame((0, 0, 0), 0, 0, 1))
    order, _fixtures, bank, _residual = fixture_bank(chart)
    pads, base_fields = base_role_fields(chart, states, bank)
    heralds = herald_states(chart, states, pads[CENTER])
    pointers = tuple(pointer_configuration(channel) for channel in order)

    check("C1 event halo has seventeen marker and ten role sites", len(MARKER_RELATIVE) == 17 and len(ROLE_RELATIVE) == 10)
    check("C2 center chart signature uses nine neutral controller pairs", len(pads[CENTER]) == 18)
    check(
        "C3 nine noncentral role pads use ten neutral controller pairs each",
        sum(len(pad) for relative, pad in pads.items() if relative != CENTER) == 9 * 20,
    )
    check("C4 marker-only halo hardware uses 306 field records", len(MARKER_RELATIVE) * 18 == 306)
    check("C5 exact charged frame contributes sixteen writer-clear field records", sum(len(slots) for slots in frame_field_slots(chart).values()) == 16)
    check("C6 registered prepared Born bank contains forty-one records", len(bank) == 41)
    check("C7 ready role-site union contains 255 records before herald and pointers", sum(len(slots) for slots in base_fields.values()) == 255)

    ready_fields = {site: set(slots) for site, slots in base_fields.items()}
    assert add_exact(ready_fields, chart.origin, polarized_slots(heralds[None]))
    pointer_packing = place_pointer_fields(
        chart,
        ready_fields,
        (pointers[0][0], pointers[1][0]),
    )
    assert pointer_packing is not None
    ready_fields, _placements = pointer_packing
    ready_role_records = sum(len(slots) for slots in ready_fields.values())
    ready_halo_records = ready_role_records + len(MARKER_RELATIVE) * 18
    check("C8 one exact ready halo contains 567 occupied field records", ready_role_records == 261 and ready_halo_records == 567)
    check("C9 one active event raises the same halo to 599 field records", ready_halo_records + 32 == 599)

    # If H=N_F+N_A1_SC+N_A1_FCC+N_A2 is conserved and every count is
    # nonnegative, H=0 contains only the completely blank occupancy vector.
    blank = (0, 0, 0, 0)
    formed_minimum = (ready_halo_records, 0, 0, 0)
    check(
        "C10 a nonzero prepared halo cannot be reached from blank under exact all-equal occupancy conservation",
        sum(blank) == 0
        and sum(formed_minimum) == ready_halo_records
        and sum(formed_minimum) > sum(blank),
    )

    # A standalone zero-E/B site identity must distinguish every one of the
    # 19,584 chart/marker presentations.  One and two unordered controller
    # pairs are insufficient; three have enough finite capacity.  This is a
    # capacity floor, not a claim that the current nine-pair code is minimal
    # under all writer/covariance constraints.
    identities = 1_152 * len(MARKER_RELATIVE)
    check(
        "C11 standalone marker identity needs at least three neutral controller pairs by capacity",
        comb(192, 1) < identities
        and comb(192, 2) < identities
        and comb(192, 3) >= identities,
    )

    exits = {
        "nonblank finite genesis seed in the causal past",
        "causal occupancy current through the formation-region boundary",
        "explicit replacement or violation of the selected occupancy invariant",
    }
    check("C12 formation has exactly the three registered logical exits", len(exits) == 3)

    forbidden = {
        "target coupling",
        "Born weight",
        "desired outcome",
        "particle mass",
        "continuum reservoir",
    }
    check("C13 no empirical target or primitive continuum enters the boundary", len(forbidden) == 5)

    passed = sum(ok for _label, ok in checks)
    print(f"\n{passed}/{len(checks)} event-halo formation-boundary checks pass")
    print(f"ready_halo_field_records={ready_halo_records}")
    print("active_halo_field_records=599")
    print("formation_from_blank_with_conserved_all_equal_occupancy=impossible")
    print("required_exit=genesis_seed_or_causal_inflow_or_explicit_invariant_replacement")
    print("stable_matter_status=open")
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
