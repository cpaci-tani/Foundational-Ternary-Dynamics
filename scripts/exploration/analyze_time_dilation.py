#!/usr/bin/env python3
"""
analyze_time_dilation.py — verdict analysis for CAMPAIGN 2 (dynamical time
dilation). LOCKED before the verdict per PREREG_DYNAMICAL_TIME_DILATION_v1.

Question: does a moving lattice clock's co-moving (proper) oscillation dilate as
  L²/γ :  dilation = √(1 − v²)        (wave-dispersion / second-order prediction)
  L¹    :  dilation = 1 − v           (FTD-0208 discrete single-event budget)
where dilation = ω_proper/ω₀ and v = v_g/c (c_lattice = C_WAVE = 1/√3).

The runner (campaign_time_dilation.cpp) already emits, per row, the measured
dilation (`dilation_meas`) and the two PARAMETER-FREE predictions evaluated at
the measured velocity (`dilation_L2`, `dilation_L1`). This script does NOT fit
free parameters; it compares the measured curve to each parameter-free law by
residual RMS, decides the winner per pre-registered thresholds, checks the IR
convergence (T2) and the directional isotropy (T3), and prints a verdict.

PRE-REGISTERED DECISION THRESHOLDS (do not tune post-hoc):
  WIN_RATIO   = 0.30   # winner median |resid| must be < 0.30 × runner-up's
  FOUND_TOL   = 0.02   # winner overall median |resid| must be < this for FOUND
Outcomes: L2_FOUND / L1_FOUND / OTHER (per PREREG §6).

Usage:
  python analyze_time_dilation.py --results-dir <dir> [--out <report.md>]
"""
import argparse
import glob
import os
import sys

import numpy as np
import pandas as pd

WIN_RATIO = 0.30
FOUND_TOL = 0.02
IR_BAND = (0.25, 0.55)   # velocity band for the IR-convergence check (T2)


def load(results_dir):
    files = sorted(glob.glob(os.path.join(results_dir, "*.csv")))
    if not files:
        sys.exit(f"no CSV found in {results_dir}")
    df = pd.concat([pd.read_csv(f) for f in files], ignore_index=True)
    # moving-clock rows only (rest n_z=0 is the v=0 control, dilation≡1)
    df = df[df["n_z"] > 0].copy()
    df = df[df["v_norm"] > 0].copy()
    # competitor predictions evaluated at the MEASURED velocity
    v = df["v_norm"].to_numpy()
    df["pred_L2"] = np.sqrt(np.clip(1.0 - v * v, 0.0, None))   # √(1−v²)
    df["pred_L1"] = 1.0 - v                                    # 1−v
    df["pred_taylor"] = 1.0 - 0.5 * v * v                      # 1−v²/2
    df["pred_const"] = 1.0                                     # no dilation
    for k in ("L2", "L1", "taylor", "const"):
        df[f"res_{k}"] = df["dilation_meas"] - df[f"pred_{k}"]
    return df


def med_abs(s):
    return float(np.median(np.abs(s))) if len(s) else float("nan")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", required=True)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    df = load(args.results_dir)
    laws = ["L2", "L1", "taylor", "const"]
    out = []
    def p(s=""):
        out.append(s)
        print(s)

    p("# Campaign 2 — Dynamical Time Dilation: verdict analysis")
    p("")
    p(f"Moving-clock data points: {len(df)} "
      f"(directions {sorted(df['direction'].unique())}, "
      f"L {sorted(df['L'].unique())})")
    p("")

    # ---- overall residuals per law ----
    overall = {k: med_abs(df[f"res_{k}"]) for k in laws}
    ranked = sorted(overall.items(), key=lambda kv: kv[1])
    winner, win_val = ranked[0]
    runner, run_val = ranked[1]
    p("## Overall median |residual| per parameter-free law")
    p("")
    p("| law | prediction | median \\|resid\\| |")
    p("|---|---|---|")
    names = {"L2": "√(1−v²)  [L²/γ]", "L1": "1−v  [L¹/FTD-0208]",
             "taylor": "1−v²/2", "const": "1 (none)"}
    for k, val in ranked:
        p(f"| {k} | {names[k]} | {val:.5f} |")
    p("")
    ratio0 = win_val / run_val if run_val > 0 else 0.0
    p(f"**Best law: `{winner}` ({names[winner]}), median |resid| = "
      f"{win_val:.5f}; runner-up `{runner}` = {run_val:.5f} "
      f"(ratio {ratio0:.3f}).**")
    p("")

    # ---- per-(direction,L,n_perp) winner table ----
    p("## Per-configuration winner (min RMS residual)")
    p("")
    p("| direction | L | n⊥ | npts | winner | rms_L2 | rms_L1 |")
    p("|---|---|---|---|---|---|---|")
    for (d, L, npp), g in df.groupby(["direction", "L", "n_perp"]):
        rms = {k: float(np.sqrt(np.mean(g[f"res_{k}"] ** 2))) for k in laws}
        w = min(rms, key=rms.get)
        p(f"| {d} | {L} | {npp} | {len(g)} | {w} | {rms['L2']:.4f} | {rms['L1']:.4f} |")
    p("")

    # ---- T2: IR convergence of the L² residual (does γ get exact as L→∞?) ----
    p("## T2 — IR convergence (|resid_L2| vs L, axis ⟨100⟩, v∈"
      f"[{IR_BAND[0]},{IR_BAND[1]}])")
    p("")
    band = df[(df["direction"] == "100") &
              (df["v_norm"] >= IR_BAND[0]) & (df["v_norm"] <= IR_BAND[1])]
    ir_rows = []
    p("| L | npts | median \\|resid_L2\\| |")
    p("|---|---|---|")
    for L, g in band.groupby("L"):
        m = med_abs(g["res_L2"])
        ir_rows.append((int(L), m))
        p(f"| {L} | {len(g)} | {m:.5f} |")
    ir_converges = (len(ir_rows) >= 2 and ir_rows[-1][1] <= ir_rows[0][1] + 1e-9)
    p("")
    p(f"IR convergence (largest-L |resid_L2| ≤ smallest-L): **{ir_converges}**")
    p("")

    # ---- T3: anisotropy (L² residual per direction) ----
    p("## T3 — Isotropy (median |resid_L2| per motion direction)")
    p("")
    p("| direction | median \\|resid_L2\\| |")
    p("|---|---|")
    for d, g in df.groupby("direction"):
        p(f"| {d} | {med_abs(g['res_L2']):.5f} |")
    p("")

    # ---- VERDICT (pre-registered) ----
    p("## Verdict (PREREG §6 thresholds)")
    p("")
    ratio = win_val / run_val if run_val > 0 else 0.0
    if winner == "L2" and win_val < FOUND_TOL and ratio < WIN_RATIO and ir_converges:
        verdict = ("**L2_FOUND** — the moving clock dilates as √(1−v²) (γ); "
                   "the wave-dispersion (L²) law governs, refining FTD-0208's "
                   "single-event L¹ budget. [MEASURED — γ emerges in the IR]")
    elif winner == "L1" and win_val < FOUND_TOL and ratio < WIN_RATIO:
        verdict = ("**L1_FOUND** — the moving clock dilates as 1−v (linear), "
                   "vindicating FTD-0208's discrete budget dynamically. "
                   "[MEASURED — linear budget]")
    else:
        verdict = ("**OTHER** — neither parameter-free law wins cleanly under "
                   "the pre-registered thresholds; report the measured form. "
                   "[OBSERVATION]")
    p(f"- WIN_RATIO < {WIN_RATIO}: {ratio:.3f} → {ratio < WIN_RATIO}")
    p(f"- FOUND_TOL < {FOUND_TOL}: winner |resid| {win_val:.5f} → {win_val < FOUND_TOL}")
    p(f"- IR convergence (T2): {ir_converges}")
    p("")
    p(verdict)

    if args.out:
        with open(args.out, "w", encoding="utf-8") as f:
            f.write("\n".join(out) + "\n")
        print(f"\n[wrote {args.out}]")


if __name__ == "__main__":
    main()
