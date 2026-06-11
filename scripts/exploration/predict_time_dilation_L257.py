#!/usr/bin/env python3
"""
predict_time_dilation_L257.py — BLIND prediction + scorer for the PL-4 L=257
extension (PREREG_TIME_DILATION_L257_BLIND_v1).

FTD-0252 v2 measured R(L) = |D_meas - sqrt(1-v^2)| falling toward 0 on matched
(direction, n_perp, n_z) groups for L in {33, 65, 97, 129, 193} (IR_CONFIRMED).
This script EXTRAPOLATES each <100>/n_perp=3 matched group's residual to the
UNMEASURED lattice size L=257 via a per-group log-log least-squares fit over
the five measured points, and emits a frozen prediction table with proper
out-of-sample 95% prediction intervals (t_{0.975, n-2} * RMSE * leverage).

Modes
  --predict                 print + (optionally --out) write the locked table
  --score --new-csv PATH    score a fresh L=257 run against the locked rules

LOCKED verdict rules (see PREREG; no tuning after the L=257 run):
  PREDICTION_CONFIRMED : >= 7 of 9 <100> groups land inside their 95% PI
                         AND median_g R_g(257) < median_g R_g(193)
  PREDICTION_BENT      : median_g R_g(257) < median_g R_g(193) but < 7/9 in PI
  CONVERGENCE_STALLED  : median_g R_g(257) >= median_g R_g(193)
The scorer recomputes the fits deterministically from the SAME frozen v1 data
(engine/results/time_dilation_v2_2026-06-07/), so locked numbers in the PREREG
and scored numbers cannot drift.

Discipline: voxel.tau is never read anywhere in this chain; R is computed from
dilation_meas and v_norm exactly as the frozen analyze_time_dilation_v2.py does.
"""
import argparse
import glob
import os
import sys

import numpy as np
import pandas as pd

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

BASE_DIR = "engine/results/time_dilation_v2_2026-06-07"
L_NEW = 257
L_PREV = 193
DIRECTION = 100          # scoring scope: the IR_CONFIRMED principal axis
N_PERP = 3
# two-sided 95% Student-t by dof = n_points - 2 (n_z>=7 groups have only the
# four L in {65,97,129,193}: the K_MAX cap excludes them at L=33)
T975 = {2: 4.302653, 3: 3.182446, 4: 2.776445}
MIN_POINTS = 4
MIN_HALFWIDTH_DEX = 0.05 # floor vs cross-build float drift (~12% in ratio)
CONFIRM_MIN_HITS = 7     # of the 9 <100> matched groups


def load(results_dir):
    files = sorted(glob.glob(os.path.join(results_dir, "*.csv")))
    if not files:
        sys.exit(f"no CSV in {results_dir}")
    df = pd.concat([pd.read_csv(f) for f in files], ignore_index=True)
    df = df[df["n_z"] > 0].copy()
    df["direction"] = df["direction"].astype(int)
    v = df["v_norm"].to_numpy()
    df["pred_L2"] = np.sqrt(np.clip(1.0 - v * v, 0.0, None))
    df["absres_L2"] = np.abs(df["dilation_meas"] - df["pred_L2"])
    return df


def fit_groups(df):
    """Per-(<100>, n_perp=3, n_z) log10 R vs log10 L least squares + 95% PI at L=257."""
    sub = df[(df["direction"] == DIRECTION) & (df["n_perp"] == N_PERP)]
    out = []
    for nz, g in sub.groupby("n_z"):
        g = g.sort_values("L")
        if g["L"].nunique() < MIN_POINTS:
            continue
        x = np.log10(g["L"].to_numpy(dtype=float))
        y = np.log10(g["absres_L2"].to_numpy(dtype=float))
        n = len(x)
        b, a = np.polyfit(x, y, 1)            # y = a + b x  (b = -p)
        yhat = a + b * x
        rmse = float(np.sqrt(np.sum((y - yhat) ** 2) / (n - 2)))
        xbar = float(np.mean(x))
        sxx = float(np.sum((x - xbar) ** 2))
        xstar = np.log10(float(L_NEW))
        lev = np.sqrt(1.0 + 1.0 / n + (xstar - xbar) ** 2 / sxx)
        half = max(T975[n - 2] * rmse * lev, MIN_HALFWIDTH_DEX)
        ystar = a + b * xstar
        r193 = float(g[g["L"] == L_PREV]["absres_L2"].iloc[0])
        out.append({
            "n_z": int(nz), "p_fit": -b, "rmse_dex": rmse,
            "R_193": r193, "R_pred_257": 10.0 ** ystar,
            "lo": 10.0 ** (ystar - half), "hi": 10.0 ** (ystar + half),
            "half_dex": half,
        })
    return out


def print_table(rows, lines):
    def p(s=""):
        lines.append(s); print(s)
    p("| n_z | p_fit | rmse(dex) | R(193) | R_pred(257) | 95% PI lo | 95% PI hi |")
    p("|---|---|---|---|---|---|---|")
    for r in rows:
        p(f"| {r['n_z']} | {r['p_fit']:.3f} | {r['rmse_dex']:.4f} | "
          f"{r['R_193']:.6f} | {r['R_pred_257']:.6f} | "
          f"{r['lo']:.6f} | {r['hi']:.6f} |")
    med_pred = float(np.median([r["R_pred_257"] for r in rows]))
    med_193 = float(np.median([r["R_193"] for r in rows]))
    p()
    p(f"median R_pred(257) over groups = {med_pred:.6f}")
    p(f"median R(193)      over groups = {med_193:.6f}")
    return med_pred, med_193


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base-dir", default=BASE_DIR)
    ap.add_argument("--predict", action="store_true")
    ap.add_argument("--score", action="store_true")
    ap.add_argument("--new-csv", default=None)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    df = load(args.base_dir)
    rows = fit_groups(df)
    if len(rows) != 9:
        sys.exit(f"expected 9 <100> matched groups, got {len(rows)} — abort")

    lines = []
    def p(s=""):
        lines.append(s); print(s)

    p("# PL-4 blind extension — locked predictions for L=257 (<100>, n_perp=3)")
    p()
    p(f"Fit: per-group log10 R vs log10 L over L in {sorted(df['L'].unique())}; "
      f"95% PI half-width = max(t(0.975,3)*RMSE*leverage, {MIN_HALFWIDTH_DEX} dex).")
    p()
    print_table(rows, lines)

    if args.score:
        if not args.new_csv:
            sys.exit("--score requires --new-csv")
        new = pd.read_csv(args.new_csv)
        new = new[new["n_z"] > 0].copy()
        new["direction"] = new["direction"].astype(int)
        v = new["v_norm"].to_numpy()
        new["pred_L2"] = np.sqrt(np.clip(1.0 - v * v, 0.0, None))
        new["absres_L2"] = np.abs(new["dilation_meas"] - new["pred_L2"])
        sub = new[(new["direction"] == DIRECTION) & (new["n_perp"] == N_PERP)
                  & (new["L"] == L_NEW)]
        p()
        p("## Scoring against the locked bands")
        p()
        p("| n_z | R_obs(257) | inside 95% PI? | R_obs(257) < R(193)? |")
        p("|---|---|---|---|")
        hits = 0
        obs = []
        for r in rows:
            gg = sub[sub["n_z"] == r["n_z"]]
            if gg.empty:
                p(f"| {r['n_z']} | MISSING | — | — |")
                continue
            ro = float(gg["absres_L2"].iloc[0])
            obs.append(ro)
            inside = r["lo"] <= ro <= r["hi"]
            hits += int(inside)
            p(f"| {r['n_z']} | {ro:.6f} | {inside} | {ro < r['R_193']} |")
        med_obs = float(np.median(obs)) if obs else float("nan")
        med_193 = float(np.median([r["R_193"] for r in rows]))
        p()
        p(f"groups inside PI: {hits}/9   (CONFIRM needs >= {CONFIRM_MIN_HITS})")
        p(f"median R_obs(257) = {med_obs:.6f}   median R(193) = {med_193:.6f}")
        p()
        if med_obs >= med_193:
            p("**VERDICT: CONVERGENCE_STALLED** — escalate per PREREG §6 "
              "(PL-4 kill-condition relevance). [OBSERVATION]")
        elif hits >= CONFIRM_MIN_HITS:
            p("**VERDICT: PREDICTION_CONFIRMED** — the locked extrapolation of "
              "the FTD-0252 residual law held at the unmeasured L=257. "
              "[MEASURED — blind extension]")
        else:
            p("**VERDICT: PREDICTION_BENT** — convergence continues but the "
              "power-law shape missed the locked bands. [OBSERVATION]")

    if args.out:
        with open(args.out, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        print(f"\n[wrote {args.out}]")


if __name__ == "__main__":
    main()
