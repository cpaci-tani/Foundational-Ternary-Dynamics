#!/usr/bin/env python3
"""
FTD-0275 — Thermal phase-map analyzer (frozen verdict logic).

Applies the PREREG_THERMAL_PHASE_MAP_v1.md frozen rules to the four runs of
record produced by campaign_thermal_ignition:

  Q1  thermal_ignition_q1_tup_scan.csv     (mode=thermal, --heat-only, seeds>=3)
  Q2a thermal_ignition_q2_ablation_ng.csv  (mode=thermal_ng, --no-genesis)
  Q2b thermal_ignition_q2_control_gen.csv  (mode=thermal, genesis ON, same ramp)
  Q3  thermal_ignition_q3_spark.csv        (mode=spark)

Frozen definitions (mirror of the pre-registration; do not edit after lock):
  T_up(L, seed) = first ramp temperature with m > 0.5 (protocol-relative kinetic
                  spinodal at the frozen settle/dT). Right-censored if never crossed.
  Q1 verdict:    RISES-CONFIRMED  if mean T_up is strictly increasing across the
                                  uncensored L values AND T_up(L_max') - T_up(L_min)
                                  > pooled cross-seed scatter sigma_pool
                 NON-MONOTONE     if any decrease beyond sigma_pool
                 CENSORED         if >= 2 L values right-censored
                 FLAT             otherwise (differences within scatter)
    + candidate-form fits on uncensored means: power T=a*L^b, log T=a+b*lnL,
      saturating T=Tinf-c/L; report SSE on log(T); best form is descriptive only.
  Q2 verdict:    SAFETY-VALVE-CONFIRMED  if ablation arm (no genesis) goes
                                         UNSTABLE at some T <= Tmax while the
                                         genesis-on control stays stable to Tmax
                 SAFETY-VALVE-FALSIFIED  if the ablation arm is stable to Tmax
                                         (OU mean-reversion alone suffices)
                 OTHER                   any other pattern (e.g. both unstable)
  Q3 verdict:    DETONATES       if any (f, A>0) cell has majority DETONATION
                                 while the same-f A=0 control has majority
                                 PRE_VACUUM at equil AND BOUNDED at settle
                 BOUNDED-ALWAYS  if no A>0 cell has majority DETONATION and all
                                 controls are clean
                 INVALID-CONTROL if any f has a majority-condensed A=0 control
                                 (bath supercritical within window)

Usage:
  python scripts/exploration/analyze_thermal_phase_map.py \
      --dir engine/results/thermal_ignition \
      [--q1 q1_tup_scan] [--q2a q2_ablation_ng] [--q2b q2_control_gen] [--q3 q3_spark]
"""

import argparse
import csv
import math
import sys
from collections import defaultdict
from pathlib import Path


def read_rows(path: Path):
    if not path.exists():
        return None
    with open(path, newline="", encoding="utf-8") as fh:
        return list(csv.DictReader(fh))


# ---------------------------------------------------------------- Q1
def analyze_q1(rows):
    print("=" * 64)
    print("Q1 — T_up(L) scaling (frozen: m>0.5 first-crossing per seed)")
    print("=" * 64)
    tup = {}          # (L, seed) -> T_up or None (censored)
    tmax_seen = defaultdict(float)
    for r in rows:
        if r["mode"] != "thermal" or r["phase"] != "heat":
            continue
        L, s, T, m = int(r["L"]), int(r["seed"]), float(r["drive"]), float(r["m"])
        tmax_seen[(L, s)] = max(tmax_seen[(L, s)], T)
        if (L, s) not in tup and m > 0.5:
            tup[(L, s)] = T
    Ls = sorted({L for (L, _s) in tmax_seen})
    means, scatters, censored = {}, {}, []
    for L in Ls:
        vals = [tup[(L, s)] for (l2, s) in sorted(tup) if l2 == L]
        n_seeds = len([1 for (l2, _s) in tmax_seen if l2 == L])
        if not vals:
            censored.append(L)
            print(f"  L={L:3d}  T_up: CENSORED (> {max(tmax_seen[(L,s)] for (l2,s) in tmax_seen if l2==L):.3f}) [0/{n_seeds} seeds crossed]")
            continue
        mu = sum(vals) / len(vals)
        sd = (sum((v - mu) ** 2 for v in vals) / len(vals)) ** 0.5 if len(vals) > 1 else 0.0
        means[L], scatters[L] = mu, sd
        cens_note = "" if len(vals) == n_seeds else f"  [{n_seeds-len(vals)}/{n_seeds} seeds censored]"
        print(f"  L={L:3d}  T_up = {mu:.4f} +/- {sd:.4f}  (n={len(vals)}){cens_note}")

    if len(censored) >= 2:
        verdict = "CENSORED"
    else:
        uL = sorted(means)
        if len(uL) < 2:
            verdict = "CENSORED"
        else:
            sig = [s for s in scatters.values() if s > 0]
            sigma_pool = (sum(s * s for s in sig) / len(sig)) ** 0.5 if sig else 0.0
            diffs = [means[uL[i + 1]] - means[uL[i]] for i in range(len(uL) - 1)]
            span = means[uL[-1]] - means[uL[0]]
            if any(d < -sigma_pool for d in diffs):
                verdict = "NON-MONOTONE"
            elif all(d >= 0 for d in diffs) and span > sigma_pool:
                verdict = "RISES-CONFIRMED"
            else:
                verdict = "FLAT"
            print(f"  pooled sigma = {sigma_pool:.4f}; span = {span:.4f}")
            fit_forms(means)
    print(f"  Q1 VERDICT: {verdict}")
    return verdict


def fit_forms(means):
    """Descriptive least-squares on the three frozen candidate forms."""
    Ls = sorted(means)
    if len(Ls) < 3:
        print("  (fits skipped: <3 uncensored L values)")
        return
    ys = [means[L] for L in Ls]

    def sse_log(pred):
        return sum((math.log(y) - math.log(max(p, 1e-12))) ** 2 for y, p in zip(ys, pred))

    # power: log T = log a + b log L (linear regression in logs)
    n = len(Ls)
    sx = sum(math.log(L) for L in Ls); sy = sum(math.log(y) for y in ys)
    sxx = sum(math.log(L) ** 2 for L in Ls)
    sxy = sum(math.log(L) * math.log(y) for L, y in zip(Ls, ys))
    b = (n * sxy - sx * sy) / (n * sxx - sx * sx)
    a = math.exp((sy - b * sx) / n)
    p_pow = [a * L ** b for L in Ls]
    print(f"  fit power      T=a*L^b      a={a:.4g} b={b:+.3f}  SSE_log={sse_log(p_pow):.4g}")

    # log: T = a + b ln L (linear regression on ln L)
    sy2 = sum(ys); sxy2 = sum(math.log(L) * y for L, y in zip(Ls, ys))
    b2 = (n * sxy2 - sx * sy2) / (n * sxx - sx * sx)
    a2 = (sy2 - b2 * sx) / n
    p_log = [a2 + b2 * math.log(L) for L in Ls]
    print(f"  fit log        T=a+b*lnL    a={a2:.4g} b={b2:+.4g}  SSE_log={sse_log(p_log):.4g}")

    # saturating: T = Tinf - c/L (linear regression on 1/L)
    sx3 = sum(1.0 / L for L in Ls); sxx3 = sum(1.0 / L ** 2 for L in Ls)
    sxy3 = sum(y / L for L, y in zip(Ls, ys))
    c3 = -(n * sxy3 - sx3 * sy2) / (n * sxx3 - sx3 * sx3)
    Tinf = (sy2 + c3 * sx3) / n
    p_sat = [Tinf - c3 / L for L in Ls]
    print(f"  fit saturating T=Tinf-c/L   Tinf={Tinf:.4g} c={c3:.4g}  SSE_log={sse_log(p_sat):.4g}")


# ---------------------------------------------------------------- Q2
def analyze_q2(ng_rows, gen_rows, tmax):
    print("=" * 64)
    print(f"Q2 — safety-valve ablation (frozen: stability to Tmax={tmax})")
    print("=" * 64)

    def arm_summary(rows, mode_name):
        unstable_T, top_T = None, -1.0
        for r in rows:
            if r["mode"] != mode_name or r["phase"] != "heat":
                continue
            T, st = float(r["drive"]), int(r["stable"])
            top_T = max(top_T, T)
            if st == 0 and unstable_T is None:
                unstable_T = T
        return unstable_T, top_T

    ng_unst, ng_top = arm_summary(ng_rows, "thermal_ng")
    gen_unst, gen_top = arm_summary(gen_rows, "thermal")
    print(f"  ablation (no genesis): reached T={ng_top:.3f}, first UNSTABLE at "
          f"{'never' if ng_unst is None else f'{ng_unst:.3f}'}")
    print(f"  control  (genesis ON): reached T={gen_top:.3f}, first UNSTABLE at "
          f"{'never' if gen_unst is None else f'{gen_unst:.3f}'}")

    if ng_unst is not None and gen_unst is None:
        verdict = "SAFETY-VALVE-CONFIRMED"
    elif ng_unst is None and ng_top >= tmax - 1e-9:
        verdict = "SAFETY-VALVE-FALSIFIED"
    else:
        verdict = "OTHER"
    print(f"  Q2 VERDICT: {verdict}")
    return verdict


# ---------------------------------------------------------------- Q3
def analyze_q3(rows):
    print("=" * 64)
    print("Q3 — near-critical spark (frozen: per-cell majority + A=0 controls)")
    print("=" * 64)
    cells = defaultdict(list)      # (L, f, A) -> [(outcome_settle, outcome_equil)]
    tup_ref = {}
    for r in rows:
        if r["mode"] != "spark":
            continue
        L = int(r["L"])
        if r["phase"] == "tup":
            tup_ref[L] = float(r["drive"])
            continue
        if r["phase"].startswith("spark_f"):
            f = float(r["phase"][len("spark_f"):])
            cells[(L, f, float(r["drive"]))].append(("settle", r["outcome"]))
        elif r["phase"].startswith("equil_f"):
            f = float(r["phase"][len("equil_f"):])
            cells[(L, f, float(r["drive"]))].append(("equil", r["outcome"]))

    for L, t in sorted(tup_ref.items()):
        print(f"  L={L}: T_up reference = {t:.4f}")

    fs = sorted({f for (_L, f, _A) in cells})
    bad_controls, detonating_cells, any_spark = [], [], False
    for f in fs:
        # control arm at this f
        ctrl_eq = [oc for (k, oc) in
                   sum([cells[c] for c in cells if c[1] == f and c[2] == 0.0], [])
                   if k == "equil"]
        ctrl_st = [oc for (k, oc) in
                   sum([cells[c] for c in cells if c[1] == f and c[2] == 0.0], [])
                   if k == "settle"]
        ctrl_clean = (ctrl_eq and ctrl_st
                      and sum(1 for o in ctrl_eq if o == "PRE_VACUUM") > len(ctrl_eq) / 2
                      and sum(1 for o in ctrl_st if o == "BOUNDED") > len(ctrl_st) / 2)
        if not ctrl_clean:
            bad_controls.append(f)
        for (L, f2, A) in sorted(cells):
            if f2 != f or A == 0.0:
                continue
            st = [oc for (k, oc) in cells[(L, f2, A)] if k == "settle"]
            n_det = sum(1 for o in st if o == "DETONATION")
            majority = n_det > len(st) / 2
            any_spark = True
            flag = "DETONATES" if (majority and ctrl_clean) else \
                   ("detonates-but-control-dirty" if majority else "bounded")
            print(f"  L={L} f={f:.2f} A={A:4.0f}: {n_det}/{len(st)} detonation  -> {flag}")
            if majority and ctrl_clean:
                detonating_cells.append((L, f, A))

    if not any_spark:
        verdict = "NO-DATA"
    elif bad_controls and not detonating_cells:
        verdict = "INVALID-CONTROL"
    elif detonating_cells:
        verdict = "DETONATES"
    else:
        verdict = "BOUNDED-ALWAYS"
    if bad_controls:
        print(f"  dirty controls at f={bad_controls}")
    print(f"  Q3 VERDICT: {verdict}")
    return verdict


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default="engine/results/thermal_ignition")
    ap.add_argument("--q1", default="q1_tup_scan")
    ap.add_argument("--q2a", default="q2_ablation_ng")
    ap.add_argument("--q2b", default="q2_control_gen")
    ap.add_argument("--q3", default="q3_spark")
    ap.add_argument("--q2-tmax", type=float, default=12.0)
    args = ap.parse_args()
    d = Path(args.dir)

    verdicts = {}
    rows = read_rows(d / f"thermal_ignition_{args.q1}.csv")
    if rows:
        verdicts["Q1"] = analyze_q1(rows)
    ng = read_rows(d / f"thermal_ignition_{args.q2a}.csv")
    gen = read_rows(d / f"thermal_ignition_{args.q2b}.csv")
    if ng and gen:
        verdicts["Q2"] = analyze_q2(ng, gen, args.q2_tmax)
    rows = read_rows(d / f"thermal_ignition_{args.q3}.csv")
    if rows:
        verdicts["Q3"] = analyze_q3(rows)

    print("=" * 64)
    print("FTD-0275 SUMMARY:", "  ".join(f"{k}={v}" for k, v in verdicts.items())
          or "no runs found")
    print("=" * 64)
    return 0 if verdicts else 1


if __name__ == "__main__":
    sys.exit(main())
