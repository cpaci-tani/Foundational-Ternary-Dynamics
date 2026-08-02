#!/usr/bin/env python3
"""Independent FTD-0751 record and earliest-stage certificate.

This script reads frozen records only.  It does not import or rerun the C++
classifier or the dynamics.
"""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0751"
MANIFEST = RESULTS / "manifest.json"
PROTOCOL = ROOT / "docs" / "theory" / "10_eft_program" / "preregistrations" / "PREREG_STAGEWISE_E1_CPU_CUDA_PARITY_v1.md"
RUNNER = ROOT / "engine" / "tests" / "campaign_stagewise_e1_cpu_cuda_parity.cpp"
CUDA_HEADER = ROOT / "engine" / "include" / "ftd" / "eft" / "cuda_matched_field_pipeline.h"
CUDA_SOURCE = ROOT / "engine" / "cuda" / "cuda_matched_field_pipeline.cu"
GEOMETRY_SOURCE = ROOT / "engine" / "tests" / "test_multipass_formation_persistence.cpp"
EXECUTABLE = ROOT / "engine" / "build_wsl" / "campaign_stagewise_e1_cpu_cuda_parity"

EXPECTED_HASHES = {
    PROTOCOL: "AD2F4DBD0843152B6398CEB9A9EF7C92B559D3FC171B167994AD5BA509103FB7",
    CUDA_HEADER: "B7EBCF382BEDED20921267FD30BC3B7AF501BF4DDD933E272D66CC799B79B5C5",
    CUDA_SOURCE: "62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022",
    GEOMETRY_SOURCE: "CE40EFAC3ED27A3101B205104331F0407A5D90A12694F3D33E9A63E00B575266",
    EXECUTABLE: "5C8DEE74FF6409B141A7B85A1A547DE4A9CB0F8FCCFD09235E460A7D8EB554F7",
}
FROZEN_RUNNER_HASH = "FF37881F387F82973987C941082B5FA4B471C7F4F5A0ED97ABD190D9F9CD4689"
PROTOCOL_HASH = EXPECTED_HASHES[PROTOCOL]
VOLUMES = (33, 65)
DIRECTIONS = ("face", "edge", "body")
STAGES = (
    "initial_electric",
    "initial_magnetic",
    "magnetic_prepare",
    "electric_prepare",
    "matter_root",
    "ordered_current",
    "state_transfer",
    "diagnostics",
)
CLASSIFICATION = {
    "initial_electric": "STATE_TRANSFER_DIVERGENCE",
    "initial_magnetic": "STATE_TRANSFER_DIVERGENCE",
    "magnetic_prepare": "SOURCE_FREE_MAGNETIC_PREPARE_DIVERGENCE",
    "electric_prepare": "SOURCE_FREE_ELECTRIC_PREPARE_DIVERGENCE",
    "matter_root": "MATTER_ROOT_DIVERGENCE",
    "ordered_current": "ORDERED_CURRENT_DIVERGENCE",
    "state_transfer": "STATE_TRANSFER_DIVERGENCE",
    "diagnostics": "DIAGNOSTIC_ONLY_DIVERGENCE",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def check(condition: bool, label: str, checks: list[tuple[str, bool]]) -> None:
    checks.append((label, condition))


def main() -> int:
    checks: list[tuple[str, bool]] = []
    for path, expected in EXPECTED_HASHES.items():
        check(path.is_file(), f"hash target exists: {path.relative_to(ROOT)}", checks)
        if path.is_file():
            check(sha256(path) == expected, f"frozen hash: {path.relative_to(ROOT)}", checks)
    check(MANIFEST.is_file(), "frozen manifest exists", checks)
    if MANIFEST.is_file():
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
        check(manifest["protocol_sha256"] == EXPECTED_HASHES[PROTOCOL],
              "manifest protocol hash", checks)
        check(manifest["runner_sha256"] == FROZEN_RUNNER_HASH,
              "manifest runner hash", checks)
        check(manifest["cuda_header_sha256"] == EXPECTED_HASHES[CUDA_HEADER],
              "manifest CUDA header hash", checks)
        check(manifest["cuda_source_sha256"] == EXPECTED_HASHES[CUDA_SOURCE],
              "manifest CUDA source hash", checks)
        check(manifest["geometry_source_sha256"] == EXPECTED_HASHES[GEOMETRY_SOURCE],
              "manifest geometry hash", checks)
        check(manifest["wsl2_executable_sha256"] == EXPECTED_HASHES[EXECUTABLE],
              "manifest executable hash", checks)

    classifications: dict[str, str] = {}
    first_differences: dict[str, dict[str, str]] = {}
    maximum_by_stage = {stage: 0.0 for stage in STAGES}
    for volume in VOLUMES:
        for direction in DIRECTIONS:
            arm = f"L{volume}_{direction}"
            stem = f"ftd_0751_stagewise_e1_parity_v1_{arm}"
            csv_path = RESULTS / f"{stem}.csv"
            json_path = RESULTS / f"{stem}.json"
            check(csv_path.is_file(), f"{arm} CSV exists", checks)
            check(json_path.is_file(), f"{arm} JSON exists", checks)
            if not csv_path.is_file() or not json_path.is_file():
                continue
            with csv_path.open(newline="", encoding="utf-8") as source:
                rows = list(csv.DictReader(source))
            summary = json.loads(json_path.read_text(encoding="utf-8"))
            check(len(rows) == 64, f"{arm} has 8 ticks x 8 rows", checks)
            expected_order = [
                (str(tick), stage) for tick in range(1, 9) for stage in STAGES
            ]
            actual_order = [(row["tick"], row["stage"]) for row in rows]
            check(actual_order == expected_order, f"{arm} stage order", checks)
            check(summary["ftd_id"] == "FTD-0751", f"{arm} FTD id", checks)
            check(summary["protocol_sha256"] == PROTOCOL_HASH,
                  f"{arm} protocol hash", checks)
            check(summary["volume"] == volume, f"{arm} volume", checks)
            check(summary["direction"] == direction, f"{arm} direction", checks)
            check(summary["ticks"] == 8 and summary["row_count"] == 64,
                  f"{arm} dimensions", checks)
            check(summary["executed"] is True, f"{arm} executed", checks)

            first = next((row for row in rows if row["exact"] == "0"), None)
            derived = "EXACT_STAGE_PARITY" if first is None else CLASSIFICATION[first["stage"]]
            classifications[arm] = derived
            check(summary["classification"] == derived,
                  f"{arm} independent classification", checks)
            if first is not None:
                first_differences[arm] = first
            for row in rows:
                maximum_by_stage[row["stage"]] = max(
                    maximum_by_stage[row["stage"]],
                    float(row["maximum_absolute"]),
                )

            tick_one = {row["stage"]: row for row in rows if row["tick"] == "1"}
            check(tick_one["initial_electric"]["exact"] == "1",
                  f"{arm} initial electric exact", checks)
            check(tick_one["initial_magnetic"]["exact"] == "1",
                  f"{arm} initial magnetic exact", checks)
            check(tick_one["magnetic_prepare"]["exact"] == "1",
                  f"{arm} tick-1 magnetic preparation exact", checks)
            check(tick_one["electric_prepare"]["exact"] == "0",
                  f"{arm} tick-1 electric preparation differs", checks)

    expected_class = "SOURCE_FREE_ELECTRIC_PREPARE_DIVERGENCE"
    check(len(classifications) == 6, "six-arm matrix complete", checks)
    check(all(value == expected_class for value in classifications.values()),
          "universal earliest-stage classification", checks)

    failed = [label for label, passed in checks if not passed]
    for index, (label, passed) in enumerate(checks, 1):
        print(f"{index:03d} {'PASS' if passed else 'FAIL'} {label}")
    print("\nCLASSIFICATIONS")
    for arm, value in sorted(classifications.items()):
        row = first_differences[arm]
        print(
            f"  {arm}: {value}; tick={row['tick']} stage={row['stage']} "
            f"location={row['first_location']} max_abs={float(row['maximum_absolute']):.17g} "
            f"max_ulp={row['maximum_ulp']}"
        )
    print("\nMAXIMUM ABSOLUTE DIFFERENCE BY STAGE")
    for stage in STAGES:
        print(f"  {stage}: {maximum_by_stage[stage]:.17g}")
    print(f"\nFTD-0751: {len(checks)-len(failed)}/{len(checks)} checks passed")
    if failed:
        print("FAILED CHECKS")
        for label in failed:
            print(f"  {label}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
