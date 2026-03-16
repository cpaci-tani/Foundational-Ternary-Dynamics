#!/usr/bin/env python3
"""Run CTest suite and produce structured JSON for the test dashboard.

Usage:
    python engine/run_tests_json.py
    python engine/run_tests_json.py --build-dir engine/build_cuda --config Debug
    python engine/run_tests_json.py --output /tmp/results.json
"""

import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

# ── Category map: test name → category ──────────────────────────────────

CATEGORY_RULES = [
    # Exact matches first
    ({"constants", "lorentz", "lattice", "ontic_chain"}, "Core"),
    ({"born_infeld", "energy", "gauss", "stress_energy", "thermodynamics"}, "Core"),
    ({"lagrangian", "magnetic_lagrangian", "dissipation", "variational_coulomb"}, "Lagrangian"),
    ({"maxwell", "em_energy_conservation", "continuity", "poynting", "larmor"}, "Electromagnetism"),
    ({"dipole_radiation", "dispersion_relation", "thomson_scattering", "em_fields"}, "Electromagnetism"),
    ({"gauss_convergence", "lorentz_force", "selective_damping"}, "Electromagnetism"),
    ({"wave_collapse", "wave_speed", "interference", "gauge", "polarization"}, "Waves & Gauge"),
    ({"momentum", "magnetic", "flux_mediated", "entanglement"}, "Waves & Gauge"),
    ({"genesis", "gravity_dynamics", "annihilation", "annihilation_conservation"}, "Dynamics"),
    ({"portable_field", "particle_lifetime", "vortex"}, "Dynamics"),
    ({"voxel_properties", "lattice_operators", "discrete_operators"}, "Operators"),
    ({"bridge_dynamics", "csv_export", "logic_engine"}, "Infrastructure"),
    ({"poisson_coulomb", "energy_tracking", "energy_conservation"}, "Energy & Poisson"),
    ({"selffield_profile", "wavepacket"}, "Energy & Poisson"),
    ({"particle_engine", "scale_bridge", "hydrogen_scale1", "multiscale_bridge"}, "Multi-Scale"),
    ({"atom_engine", "atom_scale_bridge"}, "Atom Engine"),
    ({"dual_substrate"}, "Dual Substrate"),
    ({"latency_field"}, "Latency"),
    ({"falsifiability"}, "Falsifiability"),
    ({"inflation", "dark_matter", "cosmological_constant"}, "Cosmology"),
    ({"consciousness", "sloop"}, "Consciousness"),
    ({"lorentz_invariance", "electroweak", "hydrogen_em_only"}, "Precision"),
    ({"correlations", "ensemble", "spectral", "tracker", "light", "benchmark"}, "Analysis"),
]

# Prefix-based rules for tests not caught above
PREFIX_RULES = [
    ("pe_", "PE Extensions"),
    ("ae_", "AE Extensions"),
    ("campaign_ae_", "AE Campaigns"),
    ("campaign_pe_", "PE Campaigns"),
    ("campaign_poisson_", "Poisson Campaigns"),
    ("campaign_", "Campaigns"),
    ("test_gpu_", "GPU"),
]


def categorize(name: str) -> str:
    """Map a test name to its category."""
    for name_set, cat in CATEGORY_RULES:
        if name in name_set:
            return cat
    for prefix, cat in PREFIX_RULES:
        if name.startswith(prefix):
            return cat
    return "Other"


# ── CTest output parsing ────────────────────────────────────────────────

# Matches: "1: Test command: ..."
RE_TEST_START = re.compile(r"^(\d+): Test command:")
# Matches: "    Start 1: constants"
RE_START_LINE = re.compile(r"^\s+Start\s+(\d+):\s+(.+)")
# Matches: "1:   PASS  some check name" or "1:   FAIL  some check name"
RE_CHECK = re.compile(r"^(\d+):\s{2,}(PASS|FAIL)\s{2}(.+)")
# Matches: " 1/134 Test  #1: constants ........................   Passed    0.01 sec"
# Also matches: " 4/14 Test  #11: dual_substrate ...................***Failed    6.85 sec"
RE_RESULT = re.compile(
    r"\s*(\d+)/(\d+)\s+Test\s+#(\d+):\s+(\S+)\s+\.+\s*(\*{3}Failed|Passed|Failed)\s+([\d.]+)\s+sec"
)
# Section headers inside test output: "1: === TEST: Foo ==="
RE_SECTION = re.compile(r"^(\d+):\s*=+\s*(.*?)\s*=+\s*$")
# Stdout line belonging to a test: "1: <text>"
RE_STDOUT = re.compile(r"^(\d+):\s?(.*)")


def parse_ctest_output(text: str) -> dict:
    """Parse verbose CTest output into structured results."""
    tests = {}          # test_number -> dict
    current_num = None  # which test's stdout we're capturing

    for line in text.splitlines():
        # Test start
        m = RE_START_LINE.match(line)
        if m:
            num = int(m.group(1))
            name = m.group(2).strip()
            tests[num] = {
                "name": name,
                "status": "unknown",
                "duration_sec": 0.0,
                "checks": [],
                "sections": [],
                "stdout_lines": [],
            }
            current_num = num
            continue

        # Check line (PASS/FAIL)
        m = RE_CHECK.match(line)
        if m:
            num = int(m.group(1))
            status = m.group(2).lower()
            check_name = m.group(3).strip()
            if num in tests:
                tests[num]["checks"].append({"name": check_name, "status": status})
            continue

        # Section header
        m = RE_SECTION.match(line)
        if m:
            num = int(m.group(1))
            section_name = m.group(2).strip()
            if num in tests and section_name:
                tests[num]["sections"].append(section_name)

        # Result line
        m = RE_RESULT.match(line)
        if m:
            num = int(m.group(3))
            name = m.group(4)
            passed = m.group(5) == "Passed"
            duration = float(m.group(6))
            if num in tests:
                tests[num]["status"] = "passed" if passed else "failed"
                tests[num]["duration_sec"] = duration
            else:
                tests[num] = {
                    "name": name,
                    "status": "passed" if passed else "failed",
                    "duration_sec": duration,
                    "checks": [],
                    "sections": [],
                    "stdout_lines": [],
                }
            current_num = None
            continue

        # Capture stdout for current test
        m = RE_STDOUT.match(line)
        if m and current_num and current_num in tests:
            tests[current_num]["stdout_lines"].append(m.group(2))

    return tests


def build_json(tests: dict, duration_total: float, build_dir: str) -> dict:
    """Assemble the final JSON structure grouped by category."""
    categories = {}
    total = 0
    passed = 0
    failed = 0

    for num in sorted(tests.keys()):
        t = tests[num]
        name = t["name"]
        if t["status"] == "unknown":
            continue  # skip incomplete tests (e.g. killed mid-run)
        cat = categorize(name)

        if cat not in categories:
            categories[cat] = {"passed": 0, "failed": 0, "tests": []}

        status = t["status"]
        if status == "passed":
            passed += 1
            categories[cat]["passed"] += 1
        else:
            failed += 1
            categories[cat]["failed"] += 1
        total += 1

        # Join stdout lines
        stdout = "\n".join(t["stdout_lines"])

        categories[cat]["tests"].append({
            "name": name,
            "status": status,
            "duration_sec": t["duration_sec"],
            "checks": t["checks"],
            "sections": t.get("sections", []),
            "stdout": stdout,
        })

    # Sort categories: failing first, then alphabetical
    sorted_cats = {}
    for cat in sorted(categories.keys(), key=lambda c: (categories[c]["failed"] == 0, c)):
        sorted_cats[cat] = categories[cat]

    return {
        "timestamp": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "total": total,
        "passed": passed,
        "failed": failed,
        "duration_sec": round(duration_total, 2),
        "build_dir": build_dir,
        "categories": sorted_cats,
    }


def main():
    parser = argparse.ArgumentParser(description="Run CTests and output JSON results")
    parser.add_argument("--build-dir", default="engine/build", help="CMake build directory")
    parser.add_argument("--config", default="Release", help="Build configuration")
    parser.add_argument("--output", default="engine/web/test_results.json", help="Output JSON path")
    parser.add_argument("--no-run", action="store_true", help="Parse existing output from stdin instead of running ctest")
    args = parser.parse_args()

    if args.no_run:
        text = sys.stdin.read()
        duration_total = 0.0
    else:
        cmd = [
            "ctest",
            "--test-dir", args.build_dir,
            "-C", args.config,
            "--output-on-failure",
            "--verbose",
        ]
        print(f"Running: {' '.join(cmd)}")
        start = datetime.now()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=1800)
        duration_total = (datetime.now() - start).total_seconds()
        text = result.stdout + "\n" + result.stderr
        print(f"CTest exit code: {result.returncode}")

    tests = parse_ctest_output(text)
    data = build_json(tests, duration_total, args.build_dir)

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(data, indent=2), encoding="utf-8")

    print(f"\nResults: {data['passed']}/{data['total']} passed, {data['failed']} failed")
    print(f"Duration: {data['duration_sec']}s")
    print(f"Categories: {len(data['categories'])}")
    print(f"Written to: {out_path}")

    # Print failing categories
    for cat, info in data["categories"].items():
        if info["failed"] > 0:
            names = [t["name"] for t in info["tests"] if t["status"] == "failed"]
            print(f"  FAIL {cat}: {', '.join(names)}")

    return 0 if data["failed"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
