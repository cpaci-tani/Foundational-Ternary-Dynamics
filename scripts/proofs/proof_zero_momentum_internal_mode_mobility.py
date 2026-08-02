#!/usr/bin/env python3
"""Independent record certificate for FTD-0615."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_ZERO_MOMENTUM_INTERNAL_MODE_MOBILITY_v1.md"
PARENT = ROOT / "engine/results/ftd_0614/ftd_0614_refined_core_peierls_landscape_v1.json"
RESULT = ROOT / "engine/results/ftd_0615/ftd_0615_zero_momentum_internal_modes_v1.json"
ARMS = ROOT / "engine/results/ftd_0615/ftd_0615_zero_momentum_internal_mode_arms_v1.csv"
EXPECTED_PROTOCOL = "1F8B86C20FFAC79381F2DA4B69085E5DC4B360BFAC379281D3F272C87387104B"
EXPECTED_PARENT = "8A2866361FAECED8358DD8BB59A62F01CA583273D62235436A0600796520BA45"


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    return hashlib.sha256(raw[: raw.index(b"`protocol_sha256=")]).hexdigest().upper()


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def arm_key(arm: dict) -> tuple[int, int, int]:
    return int(arm["mode"]), int(arm["sign"]), int(arm["ratio"])


def csv_key(row: dict[str, str]) -> tuple[int, int, int]:
    return int(row["mode"]), int(row["sign"]), int(row["ratio"])


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    parent = json.loads(PARENT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    arms = record["arms"]
    arm_map = {arm_key(arm): arm for arm in arms}
    csv_map = {csv_key(row): row for row in rows}
    expected_keys = {
        (mode, sign, ratio)
        for mode in range(6)
        for sign in (-1, 1)
        for ratio in (1, 4)
    }

    def csv_matches(arm: dict, row: dict[str, str]) -> bool:
        exact = {
            "complete": "1" if arm["complete"] else "0",
            "base_pass": "1" if arm["base_pass"] else "0",
            "intact": "1" if arm["intact"] else "0",
            "walker": "1" if arm["walker"] else "0",
            "bounded": "1" if arm["bounded"] else "0",
            "forward_ticks": str(arm["forward_ticks"]),
            "reverse_ticks": str(arm["reverse_ticks"]),
            "anchor_changes": str(arm["anchor_changes"]),
            "max_multiplicity": str(arm["maximum_anchor_multiplicity"]),
        }
        if any(row[name] != value for name, value in exact.items()):
            return False
        numeric = {
            "target_excitation": "target_excitation",
            "amplitude": "amplitude",
            "excitation_residual": "excitation_residual",
            "initial_momentum": "initial_momentum_residual",
            "net_displacement": "net_displacement",
            "max_excursion": "maximum_excursion",
            "path_length": "center_path_length",
            "max_center_momentum": "maximum_center_momentum",
            "min_distance": "minimum_pair_distance",
            "max_distance": "maximum_pair_distance",
            "worst_gate": "worst_common_gate",
            "energy_drift": "maximum_energy_drift",
            "pseudomomentum_defect": "maximum_pseudomomentum_defect",
            "recovery": "reverse_recovery",
        }
        return all(float(row[csv_name]) == arm[json_name] for csv_name, json_name in numeric.items())

    walkers = [arm for arm in arms if arm["walker"]]
    bounded = [arm for arm in arms if arm["bounded"]]
    intermediate = [arm for arm in arms if not arm["walker"] and not arm["bounded"]]
    strains = [arm for arm in arms if arm["mode"] in (3, 4, 5)]
    axial_high = [arm for arm in arms if arm["mode"] == 2 and arm["ratio"] == 4]
    checks = {
        "protocol_hash": protocol_hash() == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "parent_cryptographic_hash": file_hash(PARENT) == EXPECTED_PARENT,
        "record_parent_hash": record["parent_result_sha256"] == EXPECTED_PARENT,
        "parent_identity": parent["ftd_id"] == "FTD-0614",
        "identity": record["ftd_id"] == "FTD-0615",
        "production_unchanged": record["production_changed"] is False,
        "parent_and_rest_fingerprints": record["parent_hash_pass"]
        and record["rest_fingerprint_pass"]
        and record["rest_gate_pass"],
        "basis_coverage": record["basis_coverage"]
        and record["basis_zero_sum_residual"] <= 1e-12
        and record["basis_norm_residual"] <= 1e-12,
        "basis_full_rank": len(record["gram_eigenvalues"]) == 6
        and min(record["gram_eigenvalues"]) > 1e-8
        and record["minimum_gram_eigenvalue"] == min(record["gram_eigenvalues"]),
        "arm_cardinality": record["arm_coverage"]
        and len(arms) == 24
        and len(rows) == 24
        and set(arm_map) == expected_keys
        and set(csv_map) == expected_keys,
        "csv_json_identity": all(csv_matches(arm_map[key], csv_map[key]) for key in expected_keys),
        "exact_excitation_and_zero_momentum": all(
            arm["excitation_valid"]
            and arm["excitation_residual"] <= 1e-12
            and arm["initial_momentum_residual"] <= 1e-12
            for arm in arms
        ),
        "all_transaction_gates": all(
            arm["complete"]
            and arm["base_pass"]
            and arm["intact"]
            and arm["forward_ticks"] == 128
            and arm["reverse_ticks"] == 128
            and arm["worst_common_gate"] <= 1e-12
            and arm["maximum_energy_drift"] <= 1e-10
            and arm["reverse_recovery"] <= 1e-9
            and arm["maximum_anchor_multiplicity"] <= 2
            and arm["minimum_pair_distance"] >= 0.5
            and arm["maximum_pair_distance"] <= 2.0
            for arm in arms
        ),
        "walker_count": len(walkers) == record["walker_count"] == 4,
        "walker_selection": {arm_key(arm) for arm in walkers}
        == {(mode, sign, 4) for mode in (0, 1) for sign in (-1, 1)},
        "walker_thresholds": all(
            arm["net_displacement"] >= 0.75
            and arm["maximum_excursion"] >= 1.0
            and arm["anchor_changes"] >= 3
            for arm in walkers
        ),
        "bounded_count": len(bounded) == record["bounded_count"] == 18,
        "strain_modes_are_vibrational": len(strains) == 12
        and all(arm["bounded"] and arm["maximum_excursion"] < 3e-4 for arm in strains),
        "axial_high_is_intermediate": len(intermediate) == record["intermediate_count"] == 2
        and {arm_key(arm) for arm in intermediate} == {arm_key(arm) for arm in axial_high}
        and all(0.5 <= arm["maximum_excursion"] < 0.75 for arm in axial_high),
        "no_broken_geometry": record["broken_geometry_count"] == 0,
        "pseudomomentum_not_closed": max(arm["maximum_pseudomomentum_defect"] for arm in arms) > 1e-4,
        "locked_verdict": record["verdict"] == "ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0615 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"walker_modes={sorted({arm['mode'] for arm in walkers})}")
    print(f"walker_displacement_range={min(arm['net_displacement'] for arm in walkers):.17g},{max(arm['net_displacement'] for arm in walkers):.17g}")
    print(f"maximum_strain_excursion={max(arm['maximum_excursion'] for arm in strains):.17g}")
    print(f"maximum_pseudomomentum_defect={max(arm['maximum_pseudomomentum_defect'] for arm in arms):.17g}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE")
    print("scope=selected externally-neutralized common-action model; direction and recurrence not recorded")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
