#!/usr/bin/env python3
"""Independent FTD-0754 hash, algebra, and discovery-record certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0754"
MANIFEST = RESULTS / "manifest.json"
PROTOCOL = (ROOT / "docs" / "theory" / "10_eft_program" /
            "preregistrations" / "PREREG_M3_STATE_ONLY_OBSERVER_DISCOVERY_v1.md")
PROTOCOL_HASH = "D0861537AE33953169AD220E2E3416DF4D6B0BABFBDFF82CC553B85139879EC0"
ARMS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
TICKS = (0, 80, 96, 115, 160, 240, 297, 312)
RADII = (8, 12, 16, 24, 32, 48)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def check(condition: bool, label: str,
          checks: list[tuple[str, bool]]) -> None:
    checks.append((label, bool(condition)))


def f(row: dict[str, str], key: str) -> float:
    return float(row[key])


def relative_gate(*values: float) -> float:
    return 1e-12 * max(1.0, *(abs(value) for value in values))


def main() -> int:
    checks: list[tuple[str, bool]] = []
    check(PROTOCOL.is_file(), "protocol exists", checks)
    if PROTOCOL.is_file():
        check(sha256(PROTOCOL) == PROTOCOL_HASH, "protocol hash", checks)
    check(MANIFEST.is_file(), "manifest exists", checks)
    if not MANIFEST.is_file():
        return 1
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    check(manifest["ftd_id"] == "FTD-0754", "manifest identifier", checks)
    check(manifest["protocol_sha256"] == PROTOCOL_HASH,
          "manifest protocol hash", checks)
    check(manifest["scope"] == "discovery_replay_not_validation",
          "manifest discovery scope", checks)
    for relative, expected in manifest["frozen_inputs"].items():
        path = ROOT / relative
        check(path.is_file(), f"frozen input exists: {relative}", checks)
        if path.is_file():
            check(sha256(path) == expected,
                  f"frozen input hash: {relative}", checks)
    for name, expected in manifest["artifacts"].items():
        path = RESULTS / name
        check(path.is_file(), f"artifact exists: {name}", checks)
        if path.is_file():
            check(sha256(path) == expected, f"artifact hash: {name}", checks)

    extrema: dict[str, dict[str, float]] = {}
    for arm, direction in ARMS.items():
        stem = f"ftd_0754_state_only_observer_discovery_v1_{arm}"
        rows = list(csv.DictReader((RESULTS / f"{stem}.csv").open(
            newline="", encoding="utf-8")))
        summary = json.loads((RESULTS / f"{stem}.json").read_text(
            encoding="utf-8"))
        check(len(rows) == 8, f"{arm}: eight discovery snapshots", checks)
        check(tuple(int(row["tick"]) for row in rows) == TICKS,
              f"{arm}: frozen tick order", checks)
        check(all(row["arm"] == arm and row["direction"] == direction
                  for row in rows), f"{arm}: row identity", checks)
        check(all(row["valid"] == "1" for row in rows),
              f"{arm}: observer validity", checks)
        check(all(row["scalar_replay_exact"] == "1" for row in rows),
              f"{arm}: exact scalar replay rows", checks)
        check(summary["ftd_id"] == "FTD-0754"
              and summary["protocol_sha256"] == PROTOCOL_HASH,
              f"{arm}: summary identity", checks)
        check(summary["scope"] == "discovery_replay_not_validation",
              f"{arm}: summary scope", checks)
        check(summary["backend"] == "wsl2_cuda_explicit_rounding_ordered",
              f"{arm}: backend identity", checks)
        check(summary["scalar_replay_exact"] == 1
              and summary["scalar_rows_compared"] == 313,
              f"{arm}: 313 exact legacy rows", checks)
        check(summary["execution_pass"] == 1
              and summary["aggregation_pass"] == 1
              and summary["observer_pass"] == 1,
              f"{arm}: recorded discovery gates", checks)
        check(tuple(summary["observer_ticks"]) == TICKS,
              f"{arm}: summary ticks", checks)
        check(summary["readout"] == "odd_volume_centered_maxwell_characteristic"
              and summary["primitive_cochain_uniqueness_claimed"] is False,
              f"{arm}: readout scope boundary", checks)

        max_reconstruction = max(f(row, "reconstruction_residual")
                                 for row in rows)
        max_gauss = max(f(row, "gauss_residual") for row in rows)
        max_energy_partition = max(abs(f(row, "energy_partition_residual"))
                                   for row in rows)
        max_flux_identity = max(abs(f(row, "characteristic_flux_residual"))
                                for row in rows)
        check(all(f(row, "reconstruction_residual") <= relative_gate(
                      f(row, "residual_energy"), f(row, "outgoing_energy"),
                      f(row, "background_energy")) for row in rows),
              f"{arm}: exact centered reconstruction", checks)
        check(max_gauss <= 1e-12,
              f"{arm}: actual/bound Gauss compatibility", checks)
        check(all(abs(f(row, "energy_partition_residual")) <= relative_gate(
                      f(row, "residual_energy"), f(row, "outgoing_energy"),
                      f(row, "background_energy")) for row in rows),
              f"{arm}: quadratic energy partition", checks)
        check(all(abs(f(row, "characteristic_flux_residual")) <= relative_gate(
                      f(row, "outgoing_energy"), f(row, "incoming_energy"))
                  for row in rows),
              f"{arm}: characteristic flux identity", checks)
        check(all(abs(f(row, "background_energy")
                      - f(row, "incoming_energy")
                      - f(row, "radial_energy")) <= relative_gate(
                          f(row, "background_energy")) for row in rows),
              f"{arm}: background is incoming plus radial", checks)
        for radius in RADII:
            check(all(abs(f(row, f"shell_{radius}_signed")
                          - f(row, f"shell_{radius}_out")
                          + f(row, f"shell_{radius}_in")) <= relative_gate(
                              f(row, f"shell_{radius}_out"),
                              f(row, f"shell_{radius}_in")) for row in rows),
                  f"{arm}: shell-{radius} characteristic identity", checks)
        initial = rows[0]
        check(all(f(initial, key) == 0.0 for key in (
            "residual_energy", "outgoing_energy", "incoming_energy",
            "radial_energy", "background_energy")),
            f"{arm}: initial selected bound representative has zero residual",
            checks)
        check(all(f(rows[-1], key) > 0.0 for key in (
            "residual_energy", "outgoing_energy", "incoming_energy",
            "radial_energy")), f"{arm}: late split is nontrivial", checks)

        final_out = f(rows[-1], "shell_48_out")
        final_in = f(rows[-1], "shell_48_in")
        extrema[arm] = {
            "max_reconstruction": max_reconstruction,
            "max_gauss": max_gauss,
            "max_energy_partition": max_energy_partition,
            "max_flux_identity": max_flux_identity,
            "final_out_fraction": f(rows[-1], "outgoing_energy")
                                  / f(rows[-1], "residual_energy"),
            "final_in_fraction": f(rows[-1], "incoming_energy")
                                 / f(rows[-1], "residual_energy"),
            "final_radial_fraction": f(rows[-1], "radial_energy")
                                     / f(rows[-1], "residual_energy"),
            "shell48_out": final_out,
            "shell48_in": final_in,
            "shell48_ratio": final_out / final_in,
        }

    check(len(extrema) == 3, "three-ray discovery conjunction", checks)
    failures = [label for label, passed in checks if not passed]
    for index, (label, passed) in enumerate(checks, 1):
        print(f"{index:03d} {'PASS' if passed else 'FAIL'} {label}")
    print("\nFTD-0754 independent extrema (descriptive, not validation gates)")
    for arm, values in extrema.items():
        print(arm, " ".join(f"{key}={value:.17g}"
                            for key, value in values.items()))
    print(f"FTD-0754: {len(checks)-len(failures)}/{len(checks)} checks passed")
    if failures:
        print("FAILED CHECKS")
        for label in failures:
            print(f"  {label}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
