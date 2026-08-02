#!/usr/bin/env python3
"""Independent FTD-0752 record, hash, and PTX certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0752"
MANIFEST = RESULTS / "manifest.json"
PROTOCOL = ROOT / "docs" / "theory" / "10_eft_program" / "preregistrations" / "PREREG_EXPLICIT_ROUNDING_E1_CUDA_PARITY_v1.md"
RUNNER = ROOT / "engine" / "tests" / "campaign_explicit_rounding_e1_cuda_parity.cpp"
HARNESS = ROOT / "engine" / "tests" / "campaign_stagewise_e1_cpu_cuda_parity.cpp"
CUDA_CMAKE = ROOT / "engine" / "cuda" / "CMakeLists.txt"
CUDA_SOURCE = ROOT / "engine" / "cuda" / "cuda_matched_field_pipeline.cu"
EXECUTABLE = ROOT / "engine" / "build_wsl" / "campaign_explicit_rounding_e1_cuda_parity"
CUDA_LIBRARY = ROOT / "engine" / "build_wsl" / "cuda" / "libftd_cuda_explicit_rounding.a"
PTX = RESULTS / "explicit_rounding_sm120.ptx"

EXPECTED_HASHES = {
    PROTOCOL: "A12929B5C50CFD5586345BF78C5E943B21C430EDA32ECBFB5B9DE98DD23E791E",
    RUNNER: "9E8A954226174812945C391A28091C38124E6866D25080B99CC48693A001C040",
    HARNESS: "B7BB351A664BAA7E35867E8B2984A54F8CBEEA055A7200493C35C9781E5A7CC9",
    CUDA_CMAKE: "D2CE82260C37B95956FA79DF045E1A4E776442AA7E64808DBDA809060605D5AC",
    CUDA_SOURCE: "62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022",
    EXECUTABLE: "FD5212D581C00229F8604614D4D9233E18B49619F6F04868AD3E08C223DECD48",
    CUDA_LIBRARY: "EE50D5C9C1746A063661658FD816D9CA09B3625EC043495D8F311034CFC409D0",
    PTX: "BB19CFC2FD937E7B4427AF80405F8EF95B957E9D9B13BDA456B2E25A514F5BEA",
}
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
DYNAMIC_STAGES = set(STAGES) - {"diagnostics"}
DIAGNOSTIC_GATE = 2e-15


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def check(condition: bool, label: str, checks: list[tuple[str, bool]]) -> None:
    checks.append((label, condition))


def ptx_entry(text: str, name_fragment: str) -> str:
    starts = list(re.finditer(r"(?m)^(?:\.visible\s+)?\.entry\s+([^\s(]+)", text))
    for index, match in enumerate(starts):
        if name_fragment in match.group(1):
            end = starts[index + 1].start() if index + 1 < len(starts) else len(text)
            return text[match.start():end]
    return ""


def main() -> int:
    checks: list[tuple[str, bool]] = []
    for path, expected in EXPECTED_HASHES.items():
        check(path.is_file(), f"hash target exists: {path.relative_to(ROOT)}", checks)
        if path.is_file():
            check(sha256(path) == expected, f"frozen hash: {path.relative_to(ROOT)}", checks)

    check(MANIFEST.is_file(), "manifest exists", checks)
    if MANIFEST.is_file():
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
        check(manifest["protocol_sha256"] == EXPECTED_HASHES[PROTOCOL],
              "manifest protocol hash", checks)
        check(manifest["runner_sha256"] == EXPECTED_HASHES[RUNNER],
              "manifest runner hash", checks)
        check(manifest["stagewise_harness_sha256"] == EXPECTED_HASHES[HARNESS],
              "manifest harness hash", checks)
        check(manifest["cuda_cmake_sha256"] == EXPECTED_HASHES[CUDA_CMAKE],
              "manifest CUDA CMake hash", checks)
        check(manifest["cuda_source_sha256"] == EXPECTED_HASHES[CUDA_SOURCE],
              "manifest CUDA source hash", checks)
        check(manifest["wsl2_executable_sha256"] == EXPECTED_HASHES[EXECUTABLE],
              "manifest executable hash", checks)
        check(manifest["explicit_rounding_library_sha256"] == EXPECTED_HASHES[CUDA_LIBRARY],
              "manifest explicit-rounding library hash", checks)
        check(manifest["sm120_ptx_sha256"] == EXPECTED_HASHES[PTX],
              "manifest PTX hash", checks)
        check(manifest["cuda_flag"] == "--fmad=false", "manifest CUDA flag", checks)

    ptx = PTX.read_text(encoding="utf-8") if PTX.is_file() else ""
    electric = ptx_entry(ptx, "prepare_electric_kernel")
    magnetic = ptx_entry(ptx, "prepare_magnetic_kernel")
    check(bool(electric), "electric prepare PTX entry exists", checks)
    check(bool(magnetic), "magnetic prepare PTX entry exists", checks)
    check("fma.rn.f64" not in electric, "electric prepare has no fused f64 operation", checks)
    check("fma.rn.f64" not in magnetic, "magnetic prepare has no fused f64 operation", checks)
    check(electric.count("mul.rn.f64") >= 3 and electric.count("add.rn.f64") >= 3,
          "electric prepare has separate rounded multiply/add", checks)
    check(magnetic.count("mul.rn.f64") >= 3 and magnetic.count("sub.rn.f64") >= 3,
          "magnetic prepare has separate rounded multiply/subtract", checks)

    diagnostic_maximum = 0.0
    arm_verdicts: dict[str, str] = {}
    for volume in (33, 65):
        for direction in ("face", "edge", "body"):
            arm = f"L{volume}_{direction}"
            stem = f"ftd_0752_explicit_rounding_e1_parity_v1_{arm}"
            csv_path = RESULTS / f"{stem}.csv"
            json_path = RESULTS / f"{stem}.json"
            check(csv_path.is_file(), f"{arm} CSV exists", checks)
            check(json_path.is_file(), f"{arm} JSON exists", checks)
            if not csv_path.is_file() or not json_path.is_file():
                continue
            rows = list(csv.DictReader(csv_path.open(newline="", encoding="utf-8")))
            summary = json.loads(json_path.read_text(encoding="utf-8"))
            expected_order = [(str(tick), stage) for tick in range(1, 9) for stage in STAGES]
            check(len(rows) == 64, f"{arm} has 64 stage rows", checks)
            check([(row["tick"], row["stage"]) for row in rows] == expected_order,
                  f"{arm} stage order", checks)
            check(summary["protocol_sha256"] == EXPECTED_HASHES[PROTOCOL],
                  f"{arm} protocol hash", checks)
            check(summary["volume"] == volume and summary["direction"] == direction,
                  f"{arm} identity", checks)
            check(summary["executed"] is True and summary["row_count"] == 64,
                  f"{arm} execution complete", checks)
            dynamic_exact = all(
                row["exact"] == "1" for row in rows if row["stage"] in DYNAMIC_STAGES
            )
            check(dynamic_exact, f"{arm} every dynamic row bit-identical", checks)
            arm_diagnostic = max(
                float(row["maximum_absolute"])
                for row in rows if row["stage"] == "diagnostics"
            )
            diagnostic_maximum = max(diagnostic_maximum, arm_diagnostic)
            check(arm_diagnostic <= DIAGNOSTIC_GATE,
                  f"{arm} diagnostic difference within gate", checks)
            check(abs(summary["diagnostic_maximum"] - arm_diagnostic) <= 1e-30,
                  f"{arm} diagnostic summary agrees", checks)
            expected_verdict = (
                "EXACT_DYNAMIC_AND_DIAGNOSTIC_PARITY"
                if arm_diagnostic == 0.0
                else "EXACT_DYNAMIC_PARITY_DIAGNOSTIC_BOUNDED"
            )
            arm_verdicts[arm] = summary["verdict"]
            check(summary["verdict"] == expected_verdict,
                  f"{arm} independent verdict", checks)

    check(len(arm_verdicts) == 6, "six-arm result matrix complete", checks)
    check(all(value == "EXACT_DYNAMIC_PARITY_DIAGNOSTIC_BOUNDED"
              for value in arm_verdicts.values()),
          "six-arm exact dynamic conjunction", checks)
    check(diagnostic_maximum <= DIAGNOSTIC_GATE,
          "global diagnostic bound", checks)

    failures = [label for label, passed in checks if not passed]
    for index, (label, passed) in enumerate(checks, 1):
        print(f"{index:03d} {'PASS' if passed else 'FAIL'} {label}")
    print(f"\nmaximum_diagnostic_difference={diagnostic_maximum:.17g}")
    print(f"FTD-0752: {len(checks)-len(failures)}/{len(checks)} checks passed")
    if failures:
        print("FAILED CHECKS")
        for label in failures:
            print(f"  {label}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
