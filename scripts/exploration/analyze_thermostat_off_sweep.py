"""FTD-0260 thermostat-OFF sweep — pre-registered analysis & verdict.

Pre-registration: docs/theory/03_derivations/foundational_mechanics/
  PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md
Runner: engine/tests/campaign_thermostat_off_sweep.cpp

This script is hash-locked WITH the pre-registration BEFORE the campaign runs.
All decision rules below are mechanical; no thresholds may be adjusted post-run.

Usage: python analyze_thermostat_off_sweep.py --results-dir=PATH
"""
import argparse
import csv
import glob
import os
import sys
from collections import defaultdict

# ---- frozen reference: the historical k(A) table (FOUND_LATTICE_SPACING_
# GAUGE_FREEDOM.md section 6.5; thermostat-active runs, gamma=0.02, T=0.005)
A_GRID = [2.00, 10.00, 15.00, 20.00, 28.77, 30.00, 33.05, 50.00, 62.42, 85.70, 117.93]
K_HIST = {2.00: 0.250, 10.00: 0.252, 15.00: 0.224, 20.00: 0.234, 28.77: 0.253,
          30.00: 0.262, 33.05: 0.245, 50.00: 0.222, 62.42: 0.224, 85.70: 0.212,
          117.93: 0.206}
FLAT_BAND = [2.00, 10.00]
DEEP_BAND = [50.00, 62.42, 85.70, 117.93]
K_LIN = 0.25  # linear-theorem value

# ---- frozen decision rules (PREREG section 4) ----
RIG_TOL = 0.025          # V-1: |k_C(A) - k_hist(A)| <= RIG_TOL for >= RIG_MIN of 11
RIG_MIN = 8
DET_TOL = 1e-9           # V-2: off-arm seed determinism on n_mean
A_RATIO = 0.25           # Outcome A: D_X <= A_RATIO * D_C ...
A_ABS = 0.02             # ... AND |k_X(A) - 0.25| <= A_ABS for all DEEP_BAND A
B_RATIO = 0.75           # Outcome B: D_X >= B_RATIO * D_C


def load(results_dir):
    rows = []
    for path in glob.glob(os.path.join(results_dir, "sweep_*.csv")):
        with open(path, newline="") as fh:
            for r in csv.DictReader(fh):
                r["A"] = float(r["A"]); r["k_mean"] = float(r["k_mean"])
                r["n_mean"] = float(r["n_mean"])
                rows.append(r)
    return rows


def arm_table(rows, tag):
    by_A = defaultdict(list)
    for r in rows:
        if r["tag"] == tag:
            by_A[r["A"]].append(r)
    return by_A


def kbar(by_A, A):
    rs = by_A.get(A, [])
    return sum(r["k_mean"] for r in rs) / len(rs) if rs else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    args = ap.parse_args()
    rows = load(args.results_dir)
    if not rows:
        print("NO DATA"); sys.exit(2)

    C = arm_table(rows, "C")   # control: thermostat on, historical params
    X = arm_table(rows, "X")   # treatment: thermostat off

    print("=== per-amplitude table (k = N_mean/A^2, mean over seeds) ===")
    print("   A    | k_hist | k_C (on) | k_X (off) | X-seeds n_mean spread")
    for A in A_GRID:
        kC, kX = kbar(C, A), kbar(X, A)
        xs = [r["n_mean"] for r in X.get(A, [])]
        spread = (max(xs) - min(xs)) if len(xs) > 1 else 0.0
        print(f"{A:7.2f} | {K_HIST[A]:.3f}  | "
              f"{('%.4f' % kC) if kC is not None else '  -   '}  | "
              f"{('%.4f' % kX) if kX is not None else '  -   '}   | {spread:.6g}")

    # ---- V-1 rig gate ----
    hits = sum(1 for A in A_GRID
               if kbar(C, A) is not None and abs(kbar(C, A) - K_HIST[A]) <= RIG_TOL)
    v1 = hits >= RIG_MIN
    print(f"\nV-1 rig gate: {hits}/11 within {RIG_TOL} of historical "
          f"-> {'PASS' if v1 else 'FAIL (RUN INVALID - no outcome may be claimed)'}")

    # ---- V-2 determinism check (off-arm) ----
    v2 = True
    for A, rs in X.items():
        ns = [r["n_mean"] for r in rs]
        if len(ns) > 1 and max(ns) - min(ns) > DET_TOL:
            v2 = False
            print(f"V-2 determinism: FAIL at A={A} (spread {max(ns)-min(ns):.3g})")
    if v2:
        print("V-2 determinism (off-arm seeds identical): PASS")

    # ---- drift metrics ----
    D_C = sum(K_LIN - kbar(C, A) for A in DEEP_BAND if kbar(C, A) is not None) / len(DEEP_BAND)
    D_X = sum(K_LIN - kbar(X, A) for A in DEEP_BAND if kbar(X, A) is not None) / len(DEEP_BAND)
    print(f"\nDeep-band drift: D_C = {D_C:.4f}   D_X = {D_X:.4f}   "
          f"ratio D_X/D_C = {D_X / D_C if D_C else float('nan'):.3f}")
    knee_C, knee_X = kbar(C, 15.00), kbar(X, 15.00)
    print(f"Knee (A=15, descriptive): k_C = {knee_C}, k_X = {knee_X} (linear value 0.25)")

    # ---- verdict ----
    if not v1:
        print("\nVERDICT: INVALID RUN (V-1 failed). Diagnose the rig; no outcome claimed.")
        sys.exit(3)
    abs_ok = all(abs(kbar(X, A) - K_LIN) <= A_ABS for A in DEEP_BAND if kbar(X, A) is not None)
    if D_X <= A_RATIO * D_C and abs_ok:
        print("\nVERDICT: OUTCOME A — drift is thermostat physics (Mechanism gamma "
              "dominant at engine level). Substrate-native k consistent with the "
              "linear theorem's 1/4 in the deep band.")
    elif D_X >= B_RATIO * D_C:
        print("\nVERDICT: OUTCOME B — drift persists without the thermostat; "
              "Mechanism gamma CLOSED NEGATIVE as the dominant mechanism.")
    else:
        print("\nVERDICT: OUTCOME C — partial: thermostat contributes but does not "
              "exhaust the drift. Attribution arms (G/T) are descriptive input "
              "for the follow-up; no closure claimed.")

    # ---- descriptive attribution arms (no verdict power) ----
    for tag, label in (("G", "gamma-dose (A=50, T=0.005)"),
                       ("T", "T-dose (A=50, gamma=0.02)")):
        tb = arm_table(rows, tag)
        if tb:
            print(f"\n[descriptive] arm {tag} — {label}:")
            for r in sorted((r for rs in tb.values() for r in rs),
                            key=lambda r: (float(r["gamma"]), float(r["T"]))):
                print(f"  gamma={r['gamma']} T={r['T']} seed={r['seed']}: "
                      f"k={r['k_mean']:.4f}")


if __name__ == "__main__":
    main()
