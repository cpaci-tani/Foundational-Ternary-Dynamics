#!/usr/bin/env python3
"""
analyze_determinism_gate.py -- FTD-0273 Phase-0 gate adjudication.

Reads determinism_gate_<tag>.csv
  (thread_mode,A,seed,repeat,manifested,field_energy,wave_energy,idx_hash,flux_hash)
and asserts the two invariants that GATE the energy-spectroscopy program:

  (1) REPRODUCIBILITY: for every (thread_mode, A, seed), all repeats are
      bit-identical (same manifested, field_energy, wave_energy, idx_hash,
      flux_hash). genesis is probabilistic per voxel (voxel_uniform), so N IS
      allowed to vary with the SEED -- but a fixed (A, seed) must be frozen.
  (2) THREAD-INVARIANCE: for every (A, seed), the omp1 result equals the pool
      result bit-for-bit (the races the golden gate fixed must stay fixed on
      the supercritical-injection path).

VERDICT: DETERMINISM PASS iff both hold for every cell. Otherwise FAIL with the
offending cells -- the bug report is then the deliverable (Phase 1 does NOT run).

Usage: python analyze_determinism_gate.py determinism_gate_pool.csv [more.csv ...]
"""

import csv
import sys
from collections import defaultdict


def main():
    if len(sys.argv) < 2:
        print("usage: analyze_determinism_gate.py determinism_gate_*.csv")
        return 1

    # (mode, A, seed) -> list of full-row fingerprints
    cells = defaultdict(list)
    for path in sys.argv[1:]:
        with open(path, newline="") as fh:
            for r in csv.DictReader(fh):
                key = (r["thread_mode"], r["A"], r["seed"])
                fp = (r["manifested"], r["field_energy"], r["wave_energy"],
                      r["idx_hash"], r["flux_hash"])
                cells[key].append(fp)

    repro_fail = []
    for key, fps in sorted(cells.items()):
        if len(set(fps)) != 1:
            repro_fail.append((key, fps))

    # thread-invariance: compare omp1 vs pool per (A, seed)
    by_aseed = defaultdict(dict)  # (A,seed) -> mode -> fingerprint
    for (mode, A, seed), fps in cells.items():
        by_aseed[(A, seed)][mode] = fps[0]
    thread_fail = []
    for (A, seed), modes in sorted(by_aseed.items()):
        if "omp1" in modes and "pool" in modes and modes["omp1"] != modes["pool"]:
            thread_fail.append(((A, seed), modes["omp1"], modes["pool"]))

    print("=" * 88)
    print("FTD-0273 Phase-0 determinism gate")
    print("=" * 88)
    print(f"{'mode':>5} {'A':>5} {'seed':>12} {'N':>5}  field_energy")
    for (mode, A, seed), fps in sorted(cells.items()):
        n, fe = fps[0][0], fps[0][1]
        print(f"{mode:>5} {A:>5} {seed:>12} {n:>5}  {fe}")

    print("-" * 88)
    if repro_fail:
        print(f"  REPRODUCIBILITY: FAIL ({len(repro_fail)} cells vary across repeats)")
        for key, fps in repro_fail[:10]:
            print(f"    {key}: {sorted(set(fps))}")
    else:
        print(f"  REPRODUCIBILITY: PASS ({len(cells)} cells frozen across repeats)")

    if thread_fail:
        print(f"  THREAD-INVARIANCE: FAIL ({len(thread_fail)} (A,seed) omp1 != pool)")
        for (k, a, b) in thread_fail[:10]:
            print(f"    {k}: omp1={a}  pool={b}")
    else:
        npairs = sum(1 for m in by_aseed.values() if "omp1" in m and "pool" in m)
        print(f"  THREAD-INVARIANCE: PASS ({npairs} (A,seed) omp1==pool)")

    ok = not repro_fail and not thread_fail
    print("=" * 88)
    print(f"  ===> DETERMINISM: {'PASS' if ok else 'FAIL'}")
    print("=" * 88)
    if ok:
        print("  The langevin-OFF supercritical-injection genesis harness is bit-")
        print("  deterministic per (A,seed) and invariant to OpenMP thread count.")
        print("  N varies with SEED (genesis flip is probabilistic) -- so Phase 1")
        print("  reports per-seed / seed-averaged energies. Phase 1 may proceed.")
    else:
        print("  STOP. The harness is non-deterministic; no energy number is trusted.")
        print("  The bug is the deliverable. Check: langevin still on / seed reset,")
        print("  stale binary, per-bridge global state, thread count, uninit memory.")
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
