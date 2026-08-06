#!/usr/bin/env python3
"""Independent FTD-0753 hash, record, and gate certificate."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0753"
MANIFEST = RESULTS / "manifest.json"
PROTOCOL = ROOT / "docs" / "theory" / "10_eft_program" / "preregistrations" / "PREREG_EXPLICIT_ROUNDING_CAUSAL_HORIZON_M2_v1.md"
RUNNER = ROOT / "engine" / "tests" / "campaign_explicit_rounding_causal_horizon_m2.cpp"
EXECUTABLE = ROOT / "engine" / "build_wsl" / "campaign_explicit_rounding_causal_horizon_m2"
CUDA_LIBRARY = ROOT / "engine" / "build_wsl" / "cuda" / "libftd_cuda_explicit_rounding.a"
CUDA_SOURCE = ROOT / "engine" / "cuda" / "cuda_matched_field_pipeline.cu"
CUDA_CMAKE = ROOT / "engine" / "cuda" / "CMakeLists.txt"

PROTOCOL_HASH = "66D64B1A09AAB3243C5BA06991B9979C10C03EA8B8B4A01BA3803260BF3822A4"
EXPECTED_HASHES = {
    PROTOCOL: PROTOCOL_HASH,
    RUNNER: "B8AC5DED34953F8F59D9036EED9F72266DAF218842DA21CDA226666357986562",
    EXECUTABLE: "878D752B4C4422A865B5C08EC1DC55C50610ECB2F743AFA6793A29303606F4D6",
    CUDA_LIBRARY: "EE50D5C9C1746A063661658FD816D9CA09B3625EC043495D8F311034CFC409D0",
    CUDA_SOURCE: "62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022",
    CUDA_CMAKE: "D2CE82260C37B95956FA79DF045E1A4E776442AA7E64808DBDA809060605D5AC",
}
ARMS = {
    "face": "0_0_1",
    "edge": "0_1_-1",
    "body": "1_1_1",
}
RADII = (8, 12, 16, 24, 32, 48)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def check(condition: bool, label: str, checks: list[tuple[str, bool]]) -> None:
    checks.append((label, bool(condition)))


def f(row: dict[str, str], key: str) -> float:
    return float(row[key])


def b(row: dict[str, str], key: str) -> bool:
    return row[key] == "1"


def first_tail(rows: list[dict[str, str]], radius: int) -> int:
    return next(
        (int(row["tick"]) for row in rows if f(row, f"outside_{radius}") > 1e-8),
        -1,
    )


def negative_onset(rows: list[dict[str, str]]) -> int:
    for index, row in enumerate(rows):
        tail = rows[index:]
        if all(f(later, "pair_energy") < -1e-6 and b(later, "graph_inside")
               for later in tail):
            return int(row["tick"])
    return -1


def main() -> int:
    checks: list[tuple[str, bool]] = []
    for path, expected in EXPECTED_HASHES.items():
        check(path.is_file(), f"hash target exists: {path.relative_to(ROOT)}", checks)
        if path.is_file():
            check(sha256(path) == expected,
                  f"frozen hash: {path.relative_to(ROOT)}", checks)

    check(MANIFEST.is_file(), "manifest exists", checks)
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    check(manifest["ftd_id"] == "FTD-0753", "manifest identifier", checks)
    check(manifest["protocol_sha256"] == PROTOCOL_HASH,
          "manifest protocol hash", checks)
    check(manifest["runner_sha256"] == EXPECTED_HASHES[RUNNER],
          "manifest runner hash", checks)
    check(manifest["wsl2_executable_sha256"] == EXPECTED_HASHES[EXECUTABLE],
          "manifest executable hash", checks)
    check(manifest["explicit_rounding_library_sha256"] == EXPECTED_HASHES[CUDA_LIBRARY],
          "manifest research-library hash", checks)
    check(manifest["cuda_source_sha256"] == EXPECTED_HASHES[CUDA_SOURCE],
          "manifest CUDA source hash", checks)
    check(manifest["cuda_cmake_sha256"] == EXPECTED_HASHES[CUDA_CMAKE],
          "manifest CUDA CMake hash", checks)
    check(manifest["cuda_flag"] == "--fmad=false", "manifest CUDA flag", checks)
    check(len(manifest["artifacts"]) == 12, "manifest has twelve run artifacts", checks)
    for name, expected in manifest["artifacts"].items():
        path = RESULTS / name
        check(path.is_file(), f"artifact exists: {name}", checks)
        if path.is_file():
            check(sha256(path) == expected, f"artifact hash: {name}", checks)

    constructive = []
    extrema: dict[str, dict[str, float | int]] = {}
    for arm, direction in ARMS.items():
        stem = f"ftd_0753_explicit_rounding_causal_horizon_m2_v1_{arm}"
        rows = list(csv.DictReader((RESULTS / f"{stem}.csv").open(
            newline="", encoding="utf-8")))
        support = list(csv.DictReader((RESULTS / f"{stem}_support.csv").open(
            newline="", encoding="utf-8")))
        summary = json.loads((RESULTS / f"{stem}.json").read_text(encoding="utf-8"))
        support_summary = json.loads((RESULTS / f"{stem}_support.json").read_text(
            encoding="utf-8"))

        check(len(rows) == 313, f"{arm}: 313 trajectory rows", checks)
        check(len(support) == 313, f"{arm}: 313 support rows", checks)
        expected_ticks = list(range(313))
        check([int(row["tick"]) for row in rows] == expected_ticks,
              f"{arm}: trajectory tick order", checks)
        check([int(row["tick"]) for row in support] == expected_ticks,
              f"{arm}: support tick order", checks)
        check(all(row["arm"] == arm and row["direction"] == direction
                  and row["polarity"] == "plus_minus" for row in rows),
              f"{arm}: row identity", checks)
        check(summary["ftd_id"] == "FTD-0753"
              and summary["protocol_sha256"] == PROTOCOL_HASH,
              f"{arm}: summary protocol identity", checks)
        check(summary["backend"] == "wsl2_cuda_explicit_rounding_ordered",
              f"{arm}: backend identity", checks)
        check(summary["volume"] == 321 and summary["horizon"] == 312
              and summary["contact_tick"] == 313,
              f"{arm}: volume and causal horizon", checks)
        check(tuple(summary["radii"]) == RADII,
              f"{arm}: registered radii", checks)

        common_max = max(f(row, "max_residual") for row in rows)
        energy_max = max(f(row, "total_energy_residual") for row in rows)
        recoil_max = max(f(row, "recoil_defect") for row in rows)
        speed_max = max(f(row, "speed_excess") for row in rows)
        regional_max = max(f(row, "regional_residual") for row in rows)
        outside_source_max = max(f(row, "outside_source_residual") for row in rows)
        source_radius_max = max(int(row["source_radius"]) for row in rows)
        pair_field_balance = abs(
            f(rows[-1], "pair_energy") - f(rows[0], "pair_energy")
            + f(rows[-1], "field_energy") - f(rows[0], "field_energy")
        )
        h0 = (
            all(b(row, "valid") and b(row, "common") and b(row, "regional_valid")
                for row in rows)
            and common_max <= 1e-10 and energy_max <= 1e-8
            and recoil_max <= 1e-9 and speed_max <= 1e-12
            and regional_max <= 1e-10 and outside_source_max <= 1e-10
            and source_radius_max <= 3 and pair_field_balance <= 1e-8
            and 312 < 313
        )
        check(h0, f"{arm}: H0 independently reconstructed", checks)

        support_discarded_max = max(f(row, "discarded_l1") for row in support)
        support_moment_max = max(f(row, "moment_residual") for row in support)
        support_radius_max = max(int(row["source_radius"]) for row in support)
        support_net_max = max(int(row["net_support"]) for row in support)
        a0 = (
            all(b(row, "valid") for row in support)
            and support_discarded_max <= 1e-10
            and support_moment_max <= 1e-12
            and support_radius_max <= 3
        )
        check(a0, f"{arm}: A0 independently reconstructed", checks)

        onset = negative_onset(rows)
        h2 = onset >= 0 and 312 - onset + 1 >= 160
        check(h2, f"{arm}: H2 persistent energetic core", checks)
        late = rows[281:313]
        near_min = min(f(row, "inside_8") for row in late)
        near_max = max(f(row, "inside_8") for row in late)
        h3 = near_min >= 5e-4 and near_max <= 4.0 * near_min
        check(h3, f"{arm}: H3 stable late near field", checks)

        arrival = first_tail(rows, 48)
        outside48_max = max(f(row, "outside_48") for row in rows)
        h4 = (
            f(rows[0], "outside_48") <= 1e-12
            and outside48_max > 1e-8
            and 0 <= arrival <= 300
            and outside_source_max <= 1e-10
        )
        check(h4, f"{arm}: H4 radius-48 causal arrival", checks)
        outward_min = min(
            -f(row, "transport_into_48")
            for row in rows if int(row["tick"]) >= arrival
        )
        post_min = min(f(row, "outside_48") for row in rows[301:313])
        h5 = (
            outward_min >= -1e-10
            and post_min > 1e-9
            and f(rows[-1], "outside_48") > 1e-9
        )
        check(h5, f"{arm}: H5 post-arrival outward persistence", checks)

        derived = h0 and a0 and h2 and h3 and h4 and h5
        constructive.append(derived)
        check(summary["verdict"] == "M2_CAUSAL_HORIZON_WITNESS_CONSTRUCTIVE",
              f"{arm}: recorded constructive verdict", checks)
        check(derived, f"{arm}: independent constructive verdict", checks)
        check(all(summary[key] == 1 for key in (
            "initialized", "preparation_pass", "initial_pass", "forward_executed",
            "exact_pass", "support_pass", "core_pass", "near_field_pass",
            "arrival_pass", "post_arrival_pass")),
            f"{arm}: recorded physical gates pass", checks)
        check(summary["energetic_onset_tick"] == onset,
              f"{arm}: onset summary agrees", checks)
        check(summary["first_tail_ticks"][5] == arrival,
              f"{arm}: radius-48 first passage agrees", checks)
        check(abs(summary["maximum_common_residual"] - common_max) <= 1e-30,
              f"{arm}: common residual summary agrees", checks)
        check(abs(summary["maximum_energy_residual"] - energy_max) <= 1e-30,
              f"{arm}: energy residual summary agrees", checks)
        check(abs(summary["maximum_recoil_defect"] - recoil_max) <= 1e-30,
              f"{arm}: recoil summary agrees", checks)
        check(abs(summary["pair_field_balance"] - pair_field_balance) <= 1e-30,
              f"{arm}: pair-field balance summary agrees", checks)
        check(support_summary["aggregation_pass"] == 1,
              f"{arm}: support verdict pass", checks)
        check(support_summary["maximum_net_support"] == support_net_max,
              f"{arm}: support maximum agrees", checks)
        check(abs(support_summary["maximum_discarded_l1"] - support_discarded_max)
              <= 1e-30, f"{arm}: discarded-current summary agrees", checks)
        check(abs(support_summary["maximum_moment_residual"] - support_moment_max)
              <= 1e-30, f"{arm}: moment summary agrees", checks)
        # The historical CPU-prefix diagnostic remains explicitly non-operative.
        check("prefix_pass" in summary and "prefix_scalar_difference" in summary,
              f"{arm}: non-operative CPU-prefix diagnostic disclosed", checks)

        extrema[arm] = {
            "onset": onset,
            "r48": arrival,
            "near_min": near_min,
            "near_max": near_max,
            "post_min": post_min,
            "outward_min": outward_min,
            "common_max": common_max,
            "energy_max": energy_max,
            "recoil_max": recoil_max,
            "pair_field_balance": pair_field_balance,
        }

    check(len(constructive) == 3 and all(constructive),
          "three-ray causal-horizon conjunction constructive", checks)

    failures = [label for label, passed in checks if not passed]
    for index, (label, passed) in enumerate(checks, 1):
        print(f"{index:03d} {'PASS' if passed else 'FAIL'} {label}")
    print("\nFTD-0753 independent extrema")
    for arm, values in extrema.items():
        print(arm, " ".join(f"{key}={value:.17g}" if isinstance(value, float)
                            else f"{key}={value}" for key, value in values.items()))
    print(f"FTD-0753: {len(checks)-len(failures)}/{len(checks)} checks passed")
    if failures:
        print("FAILED CHECKS")
        for label in failures:
            print(f"  {label}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
