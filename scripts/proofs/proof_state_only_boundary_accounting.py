#!/usr/bin/env python3
"""Independent certificate for the FTD-0754B boundary-energy addendum.

This reads only the already-seen FTD-0754 discovery corpus and its post-hoc
observer decomposition.  It does not search parameters, inspect held-out M3
states, or promote a matter/particle claim.
"""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OLD = ROOT / "engine" / "results" / "ftd_0754"
NEW = ROOT / "engine" / "results" / "ftd_0754_boundary_accounting"
ARMS = ("face", "edge", "body")
TICKS = (0, 80, 96, 115, 160, 240, 297, 312)
GATE = 1.0e-12


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def finite(row: dict[str, str], keys: tuple[str, ...]) -> bool:
    return all(math.isfinite(float(row[key])) for key in keys)


def main() -> int:
    checks: list[tuple[str, bool]] = []
    new_rows: list[dict[str, str]] = []
    old_by_key: dict[tuple[str, int], dict[str, str]] = {}
    numeric_keys = (
        "bound_energy",
        "total_interference",
        "primitive_face_interference",
        "induced_boundary_interference",
        "centering_metric_interference",
        "centered_electric_interference",
        "centered_magnetic_interference",
        "boundary_flux_sum",
        "primitive_boundary_identity_residual",
        "readout_interference_reconstruction_residual",
    )

    for arm in ARMS:
        old_path = OLD / f"ftd_0754_state_only_observer_discovery_v1_{arm}.csv"
        new_path = NEW / f"ftd_0754b_boundary_accounting_v1_{arm}.csv"
        meta_path = NEW / f"ftd_0754b_boundary_accounting_v1_{arm}.json"
        checks.append((f"{arm}: artifacts exist",
                       old_path.is_file() and new_path.is_file()
                       and meta_path.is_file()))
        if not (old_path.is_file() and new_path.is_file()
                and meta_path.is_file()):
            continue
        old_rows = read_csv(old_path)
        arm_rows = read_csv(new_path)
        new_rows.extend(arm_rows)
        for row in old_rows:
            old_by_key[(row["arm"], int(row["tick"]))] = row
        metadata = json.loads(meta_path.read_text(encoding="utf-8"))
        checks.extend((
            (f"{arm}: exact tick set",
             tuple(int(row["tick"]) for row in arm_rows) == TICKS),
            (f"{arm}: post-hoc scope explicit",
             metadata.get("scope")
             == "posthoc_existing_discovery_corpus_no_validation"),
            (f"{arm}: no held-out validation consumed",
             metadata.get("held_out_validation_consumed") is False),
            (f"{arm}: no dynamics change",
             metadata.get("dynamics_changed") is False),
            (f"{arm}: 313 scalar rows replayed",
             metadata.get("scalar_replay_exact") == 1
             and metadata.get("scalar_rows_compared") == 313),
        ))

    checks.append(("24 decomposition rows", len(new_rows) == 24))
    checks.append(("all scalar strings replay exactly",
                   all(row["total_interference"]
                       == old_by_key[(row["arm"], int(row["tick"]))]
                           ["bound_residual_interference"]
                       for row in new_rows)))
    checks.append(("all rows finite",
                   all(finite(row, numeric_keys) for row in new_rows)))
    checks.append(("all old/new observers valid",
                   all(row["valid"] == "1"
                       and row["scalar_replay_exact"] == "1"
                       and row["boundary_ledger_valid"] == "1"
                       for row in new_rows)))

    max_boundary = max(abs(float(row["primitive_boundary_identity_residual"]))
                       for row in new_rows)
    max_readout = max(abs(float(
        row["readout_interference_reconstruction_residual"]))
        for row in new_rows)
    max_flux = max(abs(float(row["boundary_flux_sum"])) for row in new_rows)
    max_direct = 0.0
    for row in new_rows:
        total = float(row["total_interference"])
        reconstructed = (
            float(row["primitive_face_interference"])
            + float(row["centering_metric_interference"])
            + float(row["centered_magnetic_interference"])
        )
        max_direct = max(max_direct, abs(total - reconstructed))
    checks.extend((
        ("primitive/boundary identity closes", max_boundary <= GATE),
        ("readout interference closes", max_readout <= GATE),
        ("direct three-term reconstruction closes", max_direct <= GATE),
        ("closed support has zero net residual boundary flux",
         max_flux <= GATE),
    ))

    dynamic = [row for row in new_rows if int(row["tick"]) != 0]
    boundary_negative = sum(
        float(row["primitive_face_interference"]) < 0.0 for row in dynamic)
    dominant = {"boundary": 0, "centering": 0, "magnetic": 0}
    cancellation_rows: list[tuple[float, str, int]] = []
    boundary_l1_fractions: list[float] = []
    for row in dynamic:
        pieces = {
            "boundary": abs(float(row["primitive_face_interference"])),
            "centering": abs(float(row["centering_metric_interference"])),
            "magnetic": abs(float(row["centered_magnetic_interference"])),
        }
        dominant[max(pieces, key=pieces.get)] += 1
        component_l1 = sum(pieces.values())
        boundary_l1_fractions.append(pieces["boundary"] / component_l1)
        total_abs = abs(float(row["total_interference"]))
        cancellation_rows.append((component_l1 / total_abs,
                                  row["arm"], int(row["tick"])))
    checks.append(("dynamic primitive boundary term is nonzero and negative",
                   boundary_negative == len(dynamic) == 21))

    failures = [name for name, passed in checks if not passed]
    worst_cancellation = max(cancellation_rows)
    print(f"FTD-0754B boundary accounting: {len(checks)-len(failures)}/"
          f"{len(checks)} checks")
    print(f"rows={len(new_rows)} dynamic={len(dynamic)}")
    print(f"max_primitive_boundary_residual={max_boundary:.17g}")
    print(f"max_readout_reconstruction_residual={max_readout:.17g}")
    print(f"max_direct_three_term_residual={max_direct:.17g}")
    print(f"max_boundary_flux_sum={max_flux:.17g}")
    print("dominant_counts=" + json.dumps(dominant, sort_keys=True))
    print(f"mean_boundary_component_l1_fraction="
          f"{sum(boundary_l1_fractions)/len(boundary_l1_fractions):.17g}")
    print(f"maximum_cancellation_factor={worst_cancellation[0]:.17g} "
          f"arm={worst_cancellation[1]} tick={worst_cancellation[2]}")
    if failures:
        for name in failures:
            print(f"FAIL: {name}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
