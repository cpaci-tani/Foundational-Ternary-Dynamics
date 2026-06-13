#!/usr/bin/env python3
"""
FTD-0276 Leg A - drain-scaling analyzer (frozen verdict logic).

Tests the FOUND_LATTICE_SPACING_GAUGE_FREEDOM §12 hypothesis that the cluster
efficiency k_eff = N/A^2 scales as the kinetic drain squared (k = drain^2 = 0.25 at
the physical drain 0.5 = 1/N_base). Reads drain_scan_<tag>.csv
(columns: drain,A,seed,N,settle,L).

Two frozen readings, both reported:

  (R1) SCALING - fit k_eff(drain) ~ drain^p in the SUB-KNEE regime (A in the
       frozen sub-knee window) by log-log least squares on the per-drain mean
       k_eff(drain) = mean over sub-knee A of [N_bar(A,drain)/A^2].
       Verdict on the exponent p:
         drain^2-CONFIRMED   if p ∈ [1.8, 2.2]
         CLOSED-NEGATIVE    otherwise (the drain^2 scaling does not hold)

  (R2) VALUE COINCIDENCE - at drain = 0.5, is the measured sub-knee k_eff equal
       to drain^2 = 0.25 (the 1/N_base coincidence)?
         COINCIDENCE-HOLDS  if |k_eff(0.5) − 0.25| / 0.25 < 0.20
         COINCIDENCE-FAILS  otherwise

The overall Leg-A verdict is drain^2-CONFIRMED only if BOTH R1 and R2 hold;
otherwise CLOSED-NEGATIVE (the drain does not explain the ¼ coefficient via a
square law).

Frozen sub-knee window: A <= 16 (the FTD-0261 knee). This is declared here, before
the run of record, and must not be changed post hoc.

Usage:
  python scripts/exploration/analyze_drain_scan.py \
      --csv engine/results/drain_scan/drain_scan_v1.csv
"""

import argparse
import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

SUBKNEE_A_MAX = 16.0      # frozen: FTD-0261 knee
EXP_LO, EXP_HI = 1.8, 2.2  # frozen drain^2-CONFIRMED band on the scaling exponent
COINC_TOL = 0.20          # frozen tolerance on the k_eff(0.5)=0.25 coincidence


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default="engine/results/drain_scan/drain_scan_v1.csv")
    args = ap.parse_args()
    path = Path(args.csv)
    if not path.exists():
        print(f"no CSV at {path}")
        return 1

    rows = list(csv.DictReader(open(path, encoding="utf-8")))
    # per (drain, A): mean N over seeds
    byda = defaultdict(list)
    for r in rows:
        byda[(float(r["drain"]), float(r["A"]))].append(int(r["N"]))
    drains = sorted({d for (d, _A) in byda})
    As = sorted({a for (_d, a) in byda})

    print("=" * 64)
    print("FTD-0276 Leg A - drain-scaling analysis")
    print("=" * 64)
    print(f"drains = {drains}")
    print(f"A grid = {As}; sub-knee window A <= {SUBKNEE_A_MAX}")
    print()

    # per-drain sub-knee mean k_eff
    k_of_drain = {}
    print("  drain   sub-knee k_eff (mean of N_bar(A)/A^2 over A<=%g)" % SUBKNEE_A_MAX)
    for d in drains:
        ks = []
        for a in As:
            if a > SUBKNEE_A_MAX:
                continue
            ns = byda.get((d, a))
            if not ns:
                continue
            nbar = sum(ns) / len(ns)
            ks.append(nbar / (a * a))
        if ks:
            k_of_drain[d] = sum(ks) / len(ks)
            print(f"  {d:.3f}   {k_of_drain[d]:.4f}   (from {len(ks)} sub-knee A)")
    print()

    # (R1) log-log fit k_eff(drain) ~ drain^p
    pts = [(d, k) for d, k in sorted(k_of_drain.items()) if d > 0 and k > 0]
    verdict_r1 = "INSUFFICIENT"
    p = float("nan")
    if len(pts) >= 2:
        xs = [math.log(d) for d, _k in pts]
        ys = [math.log(k) for _d, k in pts]
        n = len(pts)
        sx, sy = sum(xs), sum(ys)
        sxx = sum(x * x for x in xs)
        sxy = sum(x * y for x, y in zip(xs, ys))
        p = (n * sxy - sx * sy) / (n * sxx - sx * sx)
        a0 = math.exp((sy - p * sx) / n)
        # R^2
        ybar = sy / n
        ss_tot = sum((y - ybar) ** 2 for y in ys)
        ss_res = sum((y - (math.log(a0) + p * x)) ** 2 for x, y in zip(xs, ys))
        r2 = 1 - ss_res / ss_tot if ss_tot > 0 else float("nan")
        print(f"(R1) SCALING: k_eff(drain) ~ {a0:.4g}*drain^{p:+.3f}  (R^2={r2:.3f})")
        verdict_r1 = "drain^2-CONFIRMED" if EXP_LO <= p <= EXP_HI else "CLOSED-NEGATIVE"
        print(f"     exponent p = {p:+.3f}; band [{EXP_LO},{EXP_HI}] -> {verdict_r1}")
    print()

    # (R2) value coincidence at drain = 0.5
    verdict_r2 = "NO-0.5-POINT"
    if 0.5 in k_of_drain:
        k05 = k_of_drain[0.5]
        rel = abs(k05 - 0.25) / 0.25
        verdict_r2 = "COINCIDENCE-HOLDS" if rel < COINC_TOL else "COINCIDENCE-FAILS"
        print(f"(R2) VALUE COINCIDENCE: k_eff(0.5) = {k05:.4f} vs drain^2=0.25 "
              f"(rel.dev {rel:.2f}) -> {verdict_r2}")
    print()

    overall = ("drain^2-CONFIRMED"
               if (verdict_r1 == "drain^2-CONFIRMED" and verdict_r2 == "COINCIDENCE-HOLDS")
               else "CLOSED-NEGATIVE")
    print("=" * 64)
    print(f"FTD-0276 Leg A VERDICT: {overall}   (R1={verdict_r1}, R2={verdict_r2})")
    print("=" * 64)
    return 0


if __name__ == "__main__":
    sys.exit(main())
