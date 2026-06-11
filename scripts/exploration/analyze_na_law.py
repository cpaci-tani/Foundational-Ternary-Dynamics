#!/usr/bin/env python3
"""
analyze_na_law.py -- FTD-0110 N(A) adjudicator (frozen under PREREG_FTD0110_NA_LAW_v1).

Mechanically evaluates the forward model (genesis_na_law_forward.py) and the engine
firing-geometry instrument (campaign_genesis_geometry.cpp) against the FROZEN
pre-registered criteria, and prints exactly one verdict: PROMOTE / BOUNDARY /
FALSIFY-or-UNDETERMINED.

Inputs (all produced AFTER the pre-registration tag exists):
  --model      model sweep CSV  (A, N_mean, N_std, k) -- baseline config
  --engine     engine geometry dir (geom_A<A>.csv from campaign_genesis_geometry)
  --model-firing   dir of model firing CSVs (firing_A<A>.csv) for Jaccard
  --drain-lo / --drain-hi   model sweep CSVs at drain=0.25 / 0.75 (BOUNDARY arm)
  --coupling-off            model sweep CSV with coupling off (BOUNDARY arm)

The criteria below are FROZEN. Editing them after the lock invalidates the prereg.

LEDGER: FTD-0110     Date: 2026-06-11
"""

import argparse
import csv
import glob
import math
import os
import sys

import numpy as np

# ============================================================================
# FROZEN TARGET + CRITERIA  (do not edit after lock)
# ============================================================================
# FTD-0261 current-stack measured law (the run of record we predict).
FTD0261_TARGET = {
    10: 4.0, 12: 8.4, 14: 16.4, 16: 21.6, 20: 27.4, 25: 32.6,
    30: 45.0, 40: 91.8, 50: 130.2, 70: 260.2, 90: 383.3,
}
FTD0261_KNEE = 16
FTD0261_P_LO = 3.69   # sub-knee exponent
FTD0261_P_HI = 1.86   # super-knee exponent

# PROMOTE bands (framework-only model must hit ALL of these).
KNEE_BAND = (14, 18)
P_LO_BAND = (3.3, 4.1)
P_HI_BAND = (1.6, 2.1)
CURVE_RMS_MAX = 0.10          # log10-RMS of model vs FTD0261_TARGET (3x engine's 0.037)
# Firing-geometry agreement uses the robust SHELL-OCCUPANCY profile (not exact
# voxel indices, which stochastic Langevin noise scrambles in the outer shells).
# The load-bearing FTD-0267 structural signature is the inner-shell pattern
# (center + SC + BCC occupied, FCC empty at A=14). We require the model and engine
# normalized shell-occupancy vectors to agree in L1 to within SHELL_L1_MAX.
SHELL_CLASSES = ["center", "SC", "FCC", "BCC", "SC2", "outer"]
SHELL_L1_MAX = 0.30
JACCARD_AMPS = (14, 30)

# BOUNDARY thresholds.
DRAIN_KNEE_SHIFT = 2.0        # |Delta knee| under drain sweep that flags load-bearing
COUPLING_RMS_DELTA = 0.10     # coupling-off curve diverging from coupling-on by > this

# RELAXED floor for FALSIFY (no framework config even gets close).
CURVE_RMS_RELAXED = 0.25


def read_sweep(path):
    A, N = [], []
    with open(path) as fh:
        for row in csv.DictReader(fh):
            A.append(float(row["A"]))
            N.append(float(row["N_mean"]))
    return np.array(A), np.array(N)


def fit_broken_power(A, N):
    """Segmented log-log fit: returns (knee, p_lo, p_hi, log10_rms).

    Grid-search the knee over interior grid points; each side is an OLS line in
    log10 space; total residual minimized. Continuity not enforced (the engine fit
    is two free lines too); knee is the grid point minimizing combined RMS.
    """
    mask = N > 0
    A, N = A[mask], N[mask]
    lA, lN = np.log10(A), np.log10(N)
    best = None
    for ki in range(1, len(A) - 1):
        lo = slice(0, ki + 1)
        hi = slice(ki, len(A))
        if (lo.stop - lo.start) < 2 or (len(A) - hi.start) < 2:
            continue
        p_lo, b_lo = np.polyfit(lA[lo], lN[lo], 1)
        p_hi, b_hi = np.polyfit(lA[hi], lN[hi], 1)
        res_lo = lN[lo] - (p_lo * lA[lo] + b_lo)
        res_hi = lN[hi] - (p_hi * lA[hi] + b_hi)
        rms = math.sqrt((np.sum(res_lo ** 2) + np.sum(res_hi ** 2)) / len(A))
        if best is None or rms < best[3]:
            best = (A[ki], p_lo, p_hi, rms)
    return best


def curve_rms_vs_target(A, N):
    """log10-RMS of model N(A) vs FTD-0261 target on the shared grid."""
    res = []
    for a, n in zip(A, N):
        key = int(round(a))
        if key in FTD0261_TARGET and n > 0:
            res.append(math.log10(n) - math.log10(FTD0261_TARGET[key]))
    if not res:
        return float("nan")
    return math.sqrt(np.mean(np.square(res)))


def shell_profile(path):
    """Normalized shell-occupancy vector over SHELL_CLASSES from a firing CSV.

    Aggregates fired voxels across all seeds, counts per shell, normalizes to a
    fraction vector. Returns None if the file is missing/empty.
    """
    if not os.path.exists(path):
        return None
    counts = {c: 0 for c in SHELL_CLASSES}
    total = 0
    with open(path) as fh:
        lines = [ln for ln in fh if not ln.lstrip().startswith("#")]
    for row in csv.DictReader(lines):
        if "shell" in row and row["shell"] is not None:
            sh = row["shell"]
            if sh not in counts:
                sh = "outer"
            counts[sh] += 1
            total += 1
    if total == 0:
        return None
    return np.array([counts[c] / total for c in SHELL_CLASSES])


def shell_l1(a, b):
    if a is None or b is None:
        return float("nan")
    return float(np.sum(np.abs(a - b)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True)
    ap.add_argument("--engine", default=None, help="engine geometry dir")
    ap.add_argument("--model-firing", default=None, help="model firing dir")
    ap.add_argument("--drain-lo", default=None)
    ap.add_argument("--drain-hi", default=None)
    ap.add_argument("--coupling-off", default=None)
    args = ap.parse_args()

    A, N = read_sweep(args.model)
    knee, p_lo, p_hi, rms_fit = fit_broken_power(A, N)
    curve_rms = curve_rms_vs_target(A, N)

    print("=" * 64)
    print("FTD-0110 N(A) ADJUDICATION (frozen criteria)")
    print("=" * 64)
    print(f"model broken-power fit: knee={knee:.0f}  p_lo={p_lo:.2f}  "
          f"p_hi={p_hi:.2f}  fit-RMS={rms_fit:.3f}")
    print(f"curve-RMS vs FTD-0261 target: {curve_rms:.3f}  (PROMOTE needs <= {CURVE_RMS_MAX})")
    print(f"FTD-0261 reference: knee={FTD0261_KNEE} p_lo={FTD0261_P_LO} p_hi={FTD0261_P_HI}")

    knee_ok = KNEE_BAND[0] <= knee <= KNEE_BAND[1]
    plo_ok = P_LO_BAND[0] <= p_lo <= P_LO_BAND[1]
    phi_ok = P_HI_BAND[0] <= p_hi <= P_HI_BAND[1]
    rms_ok = (not math.isnan(curve_rms)) and curve_rms <= CURVE_RMS_MAX
    print(f"  knee in {KNEE_BAND}: {knee_ok} | p_lo in {P_LO_BAND}: {plo_ok} | "
          f"p_hi in {P_HI_BAND}: {phi_ok} | curve-RMS ok: {rms_ok}")

    # Firing-geometry: shell-occupancy profile agreement (model vs engine)
    shell = {}
    if args.engine and args.model_firing:
        for a in JACCARD_AMPS:
            ep = shell_profile(os.path.join(args.engine, f"geom_A{a}.csv"))
            mp = shell_profile(os.path.join(args.model_firing, f"firing_A{a}.csv"))
            d = shell_l1(mp, ep)
            shell[a] = d
            if mp is not None and ep is not None:
                print(f"  shell A={a}: model={np.round(mp,2)} engine={np.round(ep,2)} "
                      f"L1={d:.3f} (need <= {SHELL_L1_MAX})")
            else:
                print(f"  shell A={a}: missing data (model={mp is not None} engine={ep is not None})")
    shell_ok = bool(shell) and all((not math.isnan(v)) and v <= SHELL_L1_MAX
                                   for v in shell.values())

    # BOUNDARY arms
    boundary_flags = []
    if args.drain_lo and args.drain_hi:
        klo = fit_broken_power(*read_sweep(args.drain_lo))[0]
        khi = fit_broken_power(*read_sweep(args.drain_hi))[0]
        dshift = max(abs(klo - knee), abs(khi - knee))
        print(f"  drain sweep: knee(0.25)={klo:.0f} knee(0.5)={knee:.0f} "
              f"knee(0.75)={khi:.0f}  |max shift|={dshift:.0f}")
        if dshift > DRAIN_KNEE_SHIFT:
            boundary_flags.append(f"drain shifts knee by {dshift:.0f} (> {DRAIN_KNEE_SHIFT})")
    if args.coupling_off:
        Ao, No = read_sweep(args.coupling_off)
        # compare coupling-off curve to coupling-on (this model) point-wise in log10
        on = {int(round(a)): n for a, n in zip(A, N)}
        res = [math.log10(no) - math.log10(on[int(round(a))])
               for a, no in zip(Ao, No)
               if int(round(a)) in on and no > 0 and on[int(round(a))] > 0]
        crd = math.sqrt(np.mean(np.square(res))) if res else float("nan")
        print(f"  coupling-off vs -on curve log10-RMS: {crd:.3f}  "
              f"(BOUNDARY if > {COUPLING_RMS_DELTA})")
        if (not math.isnan(crd)) and crd > COUPLING_RMS_DELTA:
            boundary_flags.append(f"coupling(alpha) load-bearing: off/on RMS {crd:.3f}")

    # ------------------------------------------------------------------ verdict
    print("-" * 64)
    promote = knee_ok and plo_ok and phi_ok and rms_ok and (shell_ok if shell else True)
    if promote and not boundary_flags:
        verdict = "PROMOTE"
        detail = "framework-only model reproduces knee + both exponents + curve."
    elif boundary_flags:
        verdict = "BOUNDARY"
        detail = "law is engine-emergent: " + "; ".join(boundary_flags)
    elif (not math.isnan(curve_rms)) and curve_rms > CURVE_RMS_RELAXED:
        verdict = "FALSIFY"
        detail = f"no framework config reaches relaxed floor (curve-RMS {curve_rms:.3f} > {CURVE_RMS_RELAXED})."
    else:
        verdict = "UNDETERMINED"
        detail = "partial: some bands hit, others missed; see flags above."
    print(f"VERDICT: {verdict}")
    print(f"  {detail}")
    print("=" * 64)
    return 0


if __name__ == "__main__":
    sys.exit(main())
