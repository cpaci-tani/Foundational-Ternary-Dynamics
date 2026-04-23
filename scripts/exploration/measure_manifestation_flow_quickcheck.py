"""Tier-1 quickcheck driver for the manifestation scale-flow campaign (Plan B reroute).

Runs benchmark_manifestation_flow_cpu.exe across a density x seed sweep at
small L, collects the JSON rows, and writes a combined raw-data JSON.

Gate before Tier-2 CUDA sweep: if SC3 does not trigger at 2-sigma on
flux_energy_ratio deviation from 1, we close Plan B as a null result.

Usage (from repo root):
    python scripts/exploration/measure_manifestation_flow_quickcheck.py
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "engine" / "build" / "Release" / "benchmark_manifestation_flow_cpu.exe"
DEFAULT_OUT = REPO / "scripts" / "exploration" / "outputs" / "manifestation_flow_quickcheck.json"

DEFAULT_DENSITIES = [0.0, 1e-3, 1e-2, 1e-1]
DEFAULT_L = 32
DEFAULT_M = 16
DEFAULT_SETTLE = 200


def run_one(exe, L, density, seed, settle):
    cmd = [str(exe),
           f"--L={L}",
           f"--density={density}",
           f"--seed={seed}",
           f"--settle={settle}"]
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=900)
    if proc.returncode != 0:
        return [{"L": L, "n": density, "level": 0, "seed": seed,
                 "status": "harness_crash",
                 "stderr": proc.stderr[:500]}]
    try:
        return json.loads(proc.stdout)
    except Exception as e:
        return [{"L": L, "n": density, "level": 0, "seed": seed,
                 "status": "parse_error",
                 "error": str(e),
                 "stdout": proc.stdout[:500]}]


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    p.add_argument("--L", type=int, default=DEFAULT_L)
    p.add_argument("--M", type=int, default=DEFAULT_M)
    p.add_argument("--settle", type=int, default=DEFAULT_SETTLE)
    p.add_argument("--densities", type=float, nargs="+",
                   default=DEFAULT_DENSITIES)
    args = p.parse_args()

    if not args.exe.exists():
        print(f"ERROR: exe not found at {args.exe}", file=sys.stderr)
        print(f"Build first: cmake --build engine/build --config Release "
              f"--target benchmark_manifestation_flow_cpu", file=sys.stderr)
        sys.exit(1)
    args.out.parent.mkdir(parents=True, exist_ok=True)

    all_rows = []
    total = len(args.densities) * args.M
    i = 0
    for density in args.densities:
        for seed_idx in range(args.M):
            i += 1
            seed = args.L * 1_000_000 + int(density * 1_000_000) * 1_000 + seed_idx
            print(f"[{i}/{total}] L={args.L} n={density:.3g} seed_idx={seed_idx} ...",
                  flush=True)
            rows = run_one(args.exe, args.L, density, seed, args.settle)
            all_rows.extend(rows)

    with open(args.out, "w") as f:
        json.dump(all_rows, f, indent=2)
    print(f"wrote {args.out} ({len(all_rows)} rows)")


if __name__ == "__main__":
    main()
