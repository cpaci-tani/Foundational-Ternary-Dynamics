#!/usr/bin/env python3
"""
Run the FTD empirical verification suite.

This runner intentionally stitches together only pre-existing verification
lanes: Python contract tests, CTest-labelled engine tests, and the Playwright
web/WASM empirical specs. It does not launch exploration searches,
near-miss scans, or coincidence-hunting scripts.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
WEB_TESTS = ROOT / "engine" / "web" / "tests"

EMPIRICAL_WEB_SPECS = [
    "scenario-parity.spec.js",
    "wasm-scenario-coverage.spec.js",
    "verify-panel.spec.js",
    "audit-regression.spec.js",
    "force-field-samplers.spec.js",
]

SUBSTRATE_PROTOCOL_SPECS = ["scale0-substrate-protocol-v2.spec.js"]

QUICK_PYTEST_TARGETS = [
    "scripts/tests/test_verify_manifest_builder.py",
    "scripts/tests/test_constants_parity.py",
    "scripts/tests/test_dimensional_map.py",
]

ENGINE_SMOKE_REGEX = (
    "^(render_bridge_golden|gauss|energy_conservation_tight|determinism|"
    "closed_negatives|strict_validation|master_quadratic_identities|constants)$"
)


@dataclass
class LaneResult:
    name: str
    command: list[str]
    cwd: str
    returncode: int
    seconds: float

    @property
    def passed(self) -> bool:
        return self.returncode == 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run FTD empirical verification lanes without numerical search scripts."
    )
    parser.add_argument(
        "--profile",
        choices=["quick", "full", "web", "engine", "python", "substrate"],
        default="quick",
        help=(
            "quick: focused contract + engine empirical smoke + web empirical specs; "
            "engine: broader CTest empirical label sweep; "
            "full: all Python tests, all non-GPU/non-benchmark CTests, and all web specs."
        ),
    )
    parser.add_argument("--build", action="store_true", help="Configure/build the C++ engine before CTest.")
    parser.add_argument("--config", default="Release", help="CMake/CTest configuration name.")
    parser.add_argument("--jobs", type=int, default=24, help="CTest/build parallelism.")
    parser.add_argument(
        "--engine-build-dir",
        default=str(ROOT / "engine" / "build"),
        help="Path to the native CMake build directory.",
    )
    parser.add_argument("--dry-run", action="store_true", help="Print commands without executing them.")
    parser.add_argument(
        "--include-substrate-protocol",
        action="store_true",
        help=(
            "Also run the draft Scale-0 substrate falsifier protocol. This lane is "
            "expected to fail when the apparatus deviates from the pre-stated predictions."
        ),
    )
    parser.add_argument("--report-json", help="Optional path for a JSON run report.")
    return parser.parse_args()


def run_lane(name: str, command: list[str], cwd: Path, dry_run: bool) -> LaneResult:
    print(f"\n=== {name} ===")
    print(f"cwd: {cwd}")
    print("cmd:", " ".join(command))
    if dry_run:
        return LaneResult(name, command, str(cwd), 0, 0.0)

    start = time.perf_counter()
    proc = subprocess.run(command, cwd=str(cwd), check=False)
    elapsed = time.perf_counter() - start
    status = "PASS" if proc.returncode == 0 else "FAIL"
    print(f"=== {name}: {status} ({elapsed:.1f}s) ===")
    return LaneResult(name, command, str(cwd), proc.returncode, elapsed)


def add_engine_build_lanes(args: argparse.Namespace, lanes: list[tuple[str, list[str], Path]]) -> None:
    build_dir = Path(args.engine_build_dir).resolve()
    ctest_file = build_dir / "CTestTestfile.cmake"
    if not args.build and ctest_file.exists():
        return

    lanes.append(
        (
            "engine configure",
            [
                "cmake",
                "-S",
                str(ROOT / "engine"),
                "-B",
                str(build_dir),
                f"-DCMAKE_BUILD_TYPE={args.config}",
            ],
            ROOT,
        )
    )
    lanes.append(
        (
            "engine build",
            ["cmake", "--build", str(build_dir), "--config", args.config, "--parallel", str(args.jobs)],
            ROOT,
        )
    )


def build_lanes(args: argparse.Namespace) -> list[tuple[str, list[str], Path]]:
    lanes: list[tuple[str, list[str], Path]] = []
    profile = args.profile
    build_dir = Path(args.engine_build_dir).resolve()

    if profile in {"quick", "python"}:
        lanes.append(("python contract tests", [sys.executable, "-m", "pytest", *QUICK_PYTEST_TARGETS], ROOT))
    elif profile == "full":
        lanes.append(("python full tests", [sys.executable, "-m", "pytest", "scripts/tests"], ROOT))

    if profile in {"quick", "engine", "full"}:
        add_engine_build_lanes(args, lanes)
        if profile == "quick":
            ctest = [
                "ctest",
                "--output-on-failure",
                "-C",
                args.config,
                "-j",
                str(args.jobs),
                "-R",
                ENGINE_SMOKE_REGEX,
            ]
            lanes.append(("engine empirical smoke", ctest, build_dir))
        elif profile == "full":
            ctest = [
                "ctest",
                "--output-on-failure",
                "-C",
                args.config,
                "-j",
                str(args.jobs),
                "-LE",
                "gpu|benchmark",
            ]
            lanes.append(("engine full CPU empirical sweep", ctest, build_dir))
        else:
            ctest = [
                "ctest",
                "--output-on-failure",
                "-C",
                args.config,
                "-j",
                str(args.jobs),
                "-L",
                "foundation|physics|golden|engine_as_instrument",
                "-LE",
                "gpu|benchmark|slow",
            ]
            lanes.append(("engine focused empirical labels", ctest, build_dir))

    if profile in {"quick", "web"}:
        lanes.append(
            (
                "web empirical specs",
                ["npx", "playwright", "test", *EMPIRICAL_WEB_SPECS, "--reporter=list"],
                WEB_TESTS,
            )
        )
    elif profile == "full":
        lanes.append(("web full Playwright suite", ["npx", "playwright", "test", "--reporter=list"], WEB_TESTS))
    elif profile == "substrate":
        lanes.append(
            (
                "web Scale-0 substrate protocol",
                ["npx", "playwright", "test", *SUBSTRATE_PROTOCOL_SPECS, "--reporter=list"],
                WEB_TESTS,
            )
        )

    if args.include_substrate_protocol and profile != "substrate":
        lanes.append(
            (
                "web Scale-0 substrate protocol",
                ["npx", "playwright", "test", *SUBSTRATE_PROTOCOL_SPECS, "--reporter=list"],
                WEB_TESTS,
            )
        )

    return lanes


def write_report(path: str, results: list[LaneResult]) -> None:
    report_path = Path(path)
    if not report_path.is_absolute():
        report_path = ROOT / report_path
    report_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "schema_version": 1,
        "runner": "scripts/runners/run_empirical_verification_suite.py",
        "passed": all(r.passed for r in results),
        "lanes": [asdict(r) | {"passed": r.passed} for r in results],
    }
    report_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"\nWrote report: {report_path}")


def main() -> int:
    args = parse_args()
    lanes = build_lanes(args)
    if not lanes:
        print("No lanes selected.")
        return 1

    print("FTD empirical verification suite")
    print("Profile:", args.profile)
    print("Repo:", ROOT)
    print("Policy: no numerical search scripts, near-miss scans, or coincidence hunts.")

    results = [run_lane(name, command, cwd, args.dry_run) for name, command, cwd in lanes]

    print("\n=== Summary ===")
    for result in results:
        status = "PASS" if result.passed else "FAIL"
        print(f"{status:4} {result.name:34} {result.seconds:7.1f}s")

    if args.report_json:
        write_report(args.report_json, results)

    return 0 if all(r.passed for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
