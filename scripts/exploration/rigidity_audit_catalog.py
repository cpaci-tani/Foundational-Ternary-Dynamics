#!/usr/bin/env python3
"""
FTD-0320 — Rigidity audit of the CATALOG rational identifications. RUNNER.

Pre-registered by PREREG_RIGIDITY_CATALOG_v1.md. The verdict logic + thresholds below
are FROZEN before the run (identical to the FTD-0310 runner; only the target set and the
RNG seed differ). This is a DEBUNKING / look-elsewhere test (is a rational identification
statistically special?), NOT a fishing scan for new matches — the FTD-0097/0189/0310
discipline. No threshold is tuned to any per-claim result.

QUESTION (the F10 "tagging is not resolution" gap, extended from FTD-0310's 3 load-bearing
ratios to the remaining simple-rational identifications in CATALOG_PARAMETRIC_INSERTIONS.md
§7/§7.1 that still carry [STRUCTURALLY MOTIVATED PARAMETRIC]). The integer-COMBINATION
families (quark/hadron masses, m_p/m_e, CKM) are out of scope here (deferred to a v2
combinatorial scan; FTD-0097 already found the monomial catalog over-rich/NULL).

TARGETS (framework rational vs PDG-2024 experimental value; LOCKED in the pre-reg):
  sin2_theta_12 = N_c/(N_c+b_3)            = 3/10  vs 0.307
  sin2_theta_23 = (N_eff+N_c)/(2N_eff+N_c) = 16/29 vs 0.546
  dm2_ratio     = (b_3+N_c)^2/N_c          = 100/3 vs 32.8
  sin2_theta_13 = 1/(N_base*N_eff)         = 1/52  vs 0.0220   (CONTROL: already [PARAMETRIC])

METHOD (per identification (T, p0/q0), e0 = |p0/q0 - T|/T):
  (1) MDL / Pareto dominance: a DOMINATOR is a reduced p/q with q < q0 (strictly simpler)
      AND relerr < e0 (strictly more accurate), inside the bracket [T(1-W), T(1+W)].
  (2) Null-calibrated p-value: draw K random targets T' ~ U[T(1-W),T(1+W)]; p = fraction
      for which the best rational with q <= q0 achieves relerr <= e0.

FROZEN VERDICT (pre-registered):
  MDL_DOMINATED  if a strictly-simpler rational (q<q0) fits strictly better (relerr<e0).
  CHANCE_LEVEL   elif p_value >= P_THRESH (the match is routine for its complexity).
  RIGID          else (not dominated AND p_value < P_THRESH).
FROZEN CONSTANTS: Q_MAX=120, W=0.30, K=200000, P_THRESH=0.05, RNG seed=20260624.

Deflationary only: a CHANCE_LEVEL/MDL_DOMINATED verdict demotes
[STRUCTURALLY MOTIVATED PARAMETRIC] -> [PARAMETRIC]. RIGID does NOT auto-promote.

Run: python scripts/exploration/rigidity_audit_catalog.py
"""

from __future__ import annotations

import json
import sys
from fractions import Fraction
from math import gcd

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

import mpmath as mp
import numpy as np

mp.mp.dps = 40

# ---- FROZEN constants (Q_MAX/W/K/P_THRESH identical to FTD-0310; seed set pre-run) ----
Q_MAX = 120
W = 0.30
K = 200_000
P_THRESH = 0.05
SEED = 20260624

# ---- targets (framework rational p0/q0, experimental T from PDG 2024 / catalog) -------
IDENTIFICATIONS = [
    # name, target T, claim numerator p0, denom q0, framework story, current_tag
    ("sin2_theta_12", 0.307, 3, 10, "N_c/(N_c+b_3)", "SMP"),
    ("sin2_theta_23", 0.546, 16, 29, "(N_eff+N_c)/(2N_eff+N_c)", "SMP"),
    ("dm2_ratio",     32.8, 100, 3, "(b_3+N_c)^2/N_c", "SMP"),
    ("sin2_theta_13", 0.0220, 1, 52, "1/(N_base*N_eff)", "PARAMETRIC(control)"),
]


def dominators(T, p0, q0, e0, w):
    """Reduced rationals p/q, q<q0, in [T(1-w),T(1+w)], with relerr < e0 (strictly simpler
    AND strictly more accurate than the claim)."""
    out = []
    lo, hi = T * (1 - w), T * (1 + w)
    for q in range(1, q0):
        for p in range(max(1, int(lo * q)), int(hi * q) + 2):
            if Fraction(p, q).denominator != q:
                continue
            v = p / q
            if lo <= v <= hi:
                err = abs(v - T) / abs(T)
                if err < e0:
                    out.append((p, q, v, err))
    out.sort(key=lambda r: (r[1], r[3]))
    return out


def null_pvalue(T, q0, e0, w, rng):
    """Fraction of random targets in [T(1-w),T(1+w)] for which the best rational with
    q<=q0 achieves relative error <= e0."""
    lo, hi = T * (1 - w), T * (1 + w)
    samples = rng.uniform(lo, hi, K)
    qs = np.arange(1, q0 + 1)
    hits = 0
    chunk = 20_000
    for i in range(0, K, chunk):
        s = samples[i:i + chunk][:, None]
        p = np.round(s * qs)
        with np.errstate(divide="ignore", invalid="ignore"):
            err = np.abs(p / qs - s) / np.abs(s)
        err[p == 0] = np.inf
        best = err.min(axis=1)
        hits += int((best <= e0).sum())
    return hits / K


def audit():
    results = []
    for name, T, p0, q0, story, tag in IDENTIFICATIONS:
        g = gcd(p0, q0)
        p0r, q0r = p0 // g, q0 // g
        claim = p0 / q0
        e0 = abs(claim - T) / abs(T)
        # verdict uses the frozen W=0.30; a fresh RNG per claim keeps each independent + reproducible
        rng = np.random.default_rng(SEED + hash(name) % 1000)
        doms = dominators(T, p0r, q0r, e0, W)
        pval = null_pvalue(T, q0r, e0, W, rng)
        # robustness disclosure: p-value over W in {0.2,0.3,0.4,0.5}
        robust = {}
        for w in (0.2, 0.3, 0.4, 0.5):
            rng_w = np.random.default_rng(SEED + hash(name) % 1000 + int(w * 100))
            robust[w] = round(null_pvalue(T, q0r, e0, w, rng_w), 4)

        if doms:
            verdict = "MDL_DOMINATED"
        elif pval >= P_THRESH:
            verdict = "CHANCE_LEVEL"
        else:
            verdict = "RIGID"

        results.append({
            "name": name, "story": story, "current_tag": tag,
            "claim": f"{p0r}/{q0r}", "claim_value": claim, "target": T,
            "rel_err_pct": e0 * 100, "n_dominators": len(doms),
            "best_dominator": (f"{doms[0][0]}/{doms[0][1]} (relerr {doms[0][3]*100:.3f}%)"
                               if doms else None),
            "null_pvalue": pval, "robustness_pvalue_byW": robust, "verdict": verdict,
        })
    return results


def main():
    res = audit()
    print("=" * 78)
    print("FTD-0320 RIGIDITY AUDIT — catalog rational identifications")
    print(f"FROZEN: Q_MAX={Q_MAX} W={W} K={K} P_THRESH={P_THRESH} seed={SEED}")
    print("=" * 78)
    for r in res:
        print(f"\n[{r['name']}]  {r['story']} = {r['claim']} = {r['claim_value']:.6f}"
              f"  vs T={r['target']:.6f}  (relerr {r['rel_err_pct']:.3f}%)  tag={r['current_tag']}")
        print(f"   MDL dominators (simpler AND better): {r['n_dominators']}"
              + (f"  best = {r['best_dominator']}" if r['best_dominator'] else ""))
        print(f"   null p-value (q<=q0): {r['null_pvalue']:.4f}"
              f"   robustness (W=0.2/0.3/0.4/0.5): {r['robustness_pvalue_byW']}")
        print(f"   => VERDICT: {r['verdict']}")
    print("\n" + "=" * 78)
    summ = {r["name"]: r["verdict"] for r in res}
    print("SUMMARY:", json.dumps(summ))
    n_rigid = sum(1 for r in res if r["verdict"] == "RIGID")
    print(f"RIGID count: {n_rigid}  (expected 0 — deflationary; any RIGID flags follow-up, NOT a promotion)")
    print("Machine JSON:")
    print(json.dumps(res, indent=2, default=float))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
