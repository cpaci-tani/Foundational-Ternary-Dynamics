#!/usr/bin/env python3
"""
analyze_time_dilation_v2.py — IR-convergence analysis for CAMPAIGN 2 v2.

v1 (analyze_time_dilation.py) could not test the IR limit: the mass n⊥ scaled
with L, pinning k⊥, and v1 had a `direction` int/str filter bug + a Windows
Unicode crash. v2 fixes all three and answers the genuine open question:

  Does the lattice's departure from exact γ vanish as the mode softens (k→0)?

Method: the v2 runner mode (--nperp-fixed=N) holds n⊥ FIXED, so k⊥ = 2π·n⊥/L → 0
as L grows. For each MATCHED (direction, n⊥, n_z) the continuum velocity is ~fixed
while k shrinks with L; if the dynamics is relativistic in the IR, the residual
R(L) = |D_meas − √(1−v²)| must decrease toward 0 as L grows.

PRE-REGISTERED decision (PREREG ...v2):
  IR_CONVERGES if (median over matched groups of R(L_max)/R(L_min)) < 0.5
  AND median R(L_max) < 0.005.
Outcomes: IR_CONFIRMED ("γ emerges in the IR" [MEASURED]) / IR_OPEN (not shown).

Usage: python analyze_time_dilation_v2.py --results-dir <dir> [--out <md>]
"""
import argparse
import glob
import os
import sys

import numpy as np
import pandas as pd

# v2 fix #3: never crash on √ on a cp1252 console.
try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

RATIO_TOL = 0.5     # residual must at least halve from smallest to largest L
ABS_TOL = 0.005     # and the largest-L residual must be this small


def load(results_dir):
    files = sorted(glob.glob(os.path.join(results_dir, "*.csv")))
    if not files:
        sys.exit(f"no CSV in {results_dir}")
    df = pd.concat([pd.read_csv(f) for f in files], ignore_index=True)
    df = df[df["n_z"] > 0].copy()
    # v2 fix #2: direction parses as int; keep it numeric and label explicitly.
    df["direction"] = df["direction"].astype(int)
    v = df["v_norm"].to_numpy()
    df["pred_L2"] = np.sqrt(np.clip(1.0 - v * v, 0.0, None))
    df["res_L2"] = df["dilation_meas"] - df["pred_L2"]
    df["absres_L2"] = np.abs(df["res_L2"])
    return df


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()
    df = load(args.results_dir)

    out = []
    def p(s=""):
        out.append(s); print(s)

    Ls = sorted(df["L"].unique())
    p("# Campaign 2 v2 — IR convergence of the moving-clock dilation")
    p("")
    p(f"L sweep: {Ls}   directions: {sorted(df['direction'].unique())}   "
      f"points: {len(df)}")
    p("")
    p("For each matched (direction, n⊥, n_z): k = 2π·n/L shrinks as L grows "
      "(n⊥ fixed), so a fixed (n⊥,n_z) point softens toward the continuum. "
      "R(L) = |D_meas − √(1−v²)| should fall toward 0 if γ is the IR limit.")
    p("")

    # ---- matched-group convergence ----
    ratios = []
    absmax = []
    rows = []
    for (d, npp, nz), g in df.groupby(["direction", "n_perp", "n_z"]):
        g = g.sort_values("L")
        if g["L"].nunique() < 2:
            continue
        r_lo = float(g.iloc[0]["absres_L2"])     # smallest L
        r_hi = float(g.iloc[-1]["absres_L2"])    # largest L
        v_lo = float(g.iloc[0]["v_norm"])
        v_hi = float(g.iloc[-1]["v_norm"])
        ratio = r_hi / r_lo if r_lo > 1e-12 else np.nan
        ratios.append(ratio)
        absmax.append(r_hi)
        rows.append((d, npp, nz, v_lo, v_hi, r_lo, r_hi, ratio))

    p("## Residual vs L per matched (dir, n⊥, n_z)  [L_min → L_max]")
    p("")
    p("| dir | n⊥ | n_z | v(L_min) | v(L_max) | R(L_min) | R(L_max) | R_hi/R_lo |")
    p("|---|---|---|---|---|---|---|---|")
    for (d, npp, nz, vlo, vhi, rlo, rhi, ratio) in sorted(rows):
        p(f"| {d} | {npp} | {nz} | {vlo:.3f} | {vhi:.3f} | "
          f"{rlo:.5f} | {rhi:.5f} | {ratio:.3f} |")
    p("")

    med_ratio = float(np.nanmedian(ratios)) if ratios else float("nan")
    med_absmax = float(np.median(absmax)) if absmax else float("nan")

    # explicit R(L) trend per direction at a representative mid-velocity point
    p("## R(L) trend (median |resid_L2| at each L, all matched points)")
    p("")
    p("| L | median \\|resid_L2\\| |")
    p("|---|---|")
    for L in Ls:
        g = df[df["L"] == L]
        p(f"| {L} | {float(np.median(g['absres_L2'])):.5f} |")
    p("")

    # ---- verdict ----
    converges = (med_ratio < RATIO_TOL) and (med_absmax < ABS_TOL)
    p("## Verdict (PREREG v2)")
    p("")
    p(f"- median R(L_max)/R(L_min) = {med_ratio:.3f}  (< {RATIO_TOL} ? {med_ratio < RATIO_TOL})")
    p(f"- median R(L_max)         = {med_absmax:.5f}  (< {ABS_TOL} ? {med_absmax < ABS_TOL})")
    p("")
    if converges:
        p("**IR_CONFIRMED** — the departure from exact γ shrinks toward 0 as the "
          "mode softens; the moving clock dilates as √(1−v²) in the IR limit. "
          "[MEASURED — γ emerges in the IR]")
    else:
        p("**IR_OPEN** — clean IR convergence to exact γ is NOT demonstrated at "
          "these (L, k); report the measured trend. [OBSERVATION]")

    if args.out:
        with open(args.out, "w", encoding="utf-8") as f:
            f.write("\n".join(out) + "\n")
        print(f"\n[wrote {args.out}]")


if __name__ == "__main__":
    main()
