#!/usr/bin/env python3
"""
FTD-0310 — Rigidity audit of the load-bearing rational identifications. RUNNER.

Status: this runner is SHA-locked by PREREG_RIGIDITY_AUDIT_LOADBEARING_v1.md. The
verdict logic + thresholds below are FROZEN before the run. This is a uniqueness /
look-elsewhere test aimed at DEBUNKING (testing whether a rational identification is
statistically special), NOT a fishing scan for new matches — the FTD-0097 / FTD-0189
discipline. No threshold is tuned to any per-claim result.

QUESTION (the F10 "tagging is not resolution" gap). A LEDGER tag ([PARAMETRIC] /
[STRUCTURALLY MOTIVATED PARAMETRIC]) LABELS a claim but does not answer: is the match
to experiment statistically surprising, or is the space of simple rationals dense
enough that *some* low-complexity fraction would hit the target this well by chance?

TARGETS (load-bearing physics identifications; values from scripts/constants.py):
  sin2_theta_W = N_c/N_eff = 3/13   vs PDG effective 0.23122
  alpha_s      = b_3/(b_3+4 N_eff) = 7/59  vs 0.1179
  m_e prefactor = N_base^2/N_c = 16/3  vs k* = m_e/(m_P sqrt(2pi) alpha^11)

METHOD (per identification (T, p0/q0), e0 = |p0/q0 - T|/T):
  (1) MDL / Pareto dominance: enumerate reduced rationals p/q with q <= Q_MAX in a
      bracket around T; a DOMINATOR is any p/q with q < q0 AND relerr < e0 (strictly
      simpler AND strictly more accurate). A dominated claim means the framework chose
      a more-complex fraction to obtain its preferred integers while a simpler rational
      fits better -> the integer story is not what is doing the work.
  (2) Null-calibrated p-value: draw K random targets T' ~ U[T(1-W), T(1+W)]; for each,
      compute the best relative error achievable by ANY rational with q <= q0; p_value
      = fraction with best-error <= e0. This measures how routine it is to hit a random
      nearby target as well as the claim hits T, at the complexity the framework spent.

FROZEN VERDICT (pre-registered):
  MDL_DOMINATED  if a strictly-simpler rational (q<q0) fits strictly better (relerr<e0).
  CHANCE_LEVEL   elif p_value >= P_THRESH (the match is routine for its complexity).
  RIGID          else (not dominated AND p_value < P_THRESH).
FROZEN CONSTANTS: Q_MAX=120, W=0.30, K=200000, P_THRESH=0.05, RNG seed=20260622.

Run: python scripts/exploration/rigidity_audit_loadbearing.py
"""

from __future__ import annotations

import json
from fractions import Fraction

import mpmath as mp
import numpy as np

mp.mp.dps = 40

# ---- FROZEN constants -------------------------------------------------------
Q_MAX = 120
W = 0.30
K = 200_000
P_THRESH = 0.05
SEED = 20260622

# ---- targets (from scripts/constants.py canonical values) -------------------
ALPHA = 1 / mp.mpf("137.035999177")        # CODATA 2022
M_P = mp.mpf("1.220890e19")                # GeV
M_E = mp.mpf("0.51099895") * mp.mpf("1e-3")  # GeV
SQRT2PI = mp.sqrt(2 * mp.pi)

K_STAR = M_E / (M_P * SQRT2PI * ALPHA ** 11)   # the exact m_e prefactor target

IDENTIFICATIONS = [
    # name, target T (float), claim numerator p0, denom q0, framework story
    ("sin2_theta_W", float(mp.mpf("0.23122")), 3, 13, "N_c / N_eff"),
    ("alpha_s",      float(mp.mpf("0.1179")),  7, 59, "b_3 / (b_3 + 4 N_eff)"),
    ("m_e_prefactor", float(K_STAR),           16, 3, "N_base^2 / N_c"),
]


def best_rational_error(T: float, qmax: int) -> float:
    """Smallest relative error |p/q - T|/T over reduced rationals with q<=qmax,
    p the nearest integer to T*q (covers the best approximation at each denominator)."""
    best = float("inf")
    aT = abs(T)
    for q in range(1, qmax + 1):
        p = round(T * q)
        if p == 0:
            continue
        err = abs(p / q - T) / aT
        if err < best:
            best = err
    return best


def dominators(T: float, p0: int, q0: int, e0: float, qmax: int):
    """All reduced rationals p/q, q<q0, in the bracket, with relerr < e0 (strictly
    simpler AND strictly more accurate than the claim)."""
    out = []
    lo, hi = T * (1 - W), T * (1 + W)
    for q in range(1, q0):
        for p in range(max(1, int(lo * q)), int(hi * q) + 2):
            if Fraction(p, q).denominator != q:   # reduced only
                continue
            v = p / q
            if lo <= v <= hi:
                err = abs(v - T) / abs(T)
                if err < e0:
                    out.append((p, q, v, err))
    out.sort(key=lambda r: (r[1], r[3]))   # simplest first, then most accurate
    return out


def null_pvalue(T: float, q0: int, e0: float, rng: np.random.Generator) -> float:
    """Fraction of random targets in [T(1-W),T(1+W)] for which the best rational with
    q<=q0 achieves relative error <= e0."""
    lo, hi = T * (1 - W), T * (1 + W)
    samples = rng.uniform(lo, hi, K)
    # vectorized best-error over q=1..q0
    qs = np.arange(1, q0 + 1)
    hits = 0
    # process in chunks to bound memory
    chunk = 20_000
    for i in range(0, K, chunk):
        s = samples[i:i + chunk][:, None]            # (n,1)
        p = np.round(s * qs)                          # (n,q0)
        with np.errstate(divide="ignore", invalid="ignore"):
            err = np.abs(p / qs - s) / np.abs(s)
        err[p == 0] = np.inf
        best = err.min(axis=1)
        hits += int((best <= e0).sum())
    return hits / K


def audit():
    rng = np.random.default_rng(SEED)
    results = []
    for name, T, p0, q0, story in IDENTIFICATIONS:
        claim = p0 / q0
        e0 = abs(claim - T) / abs(T)
        doms = dominators(T, p0, q0, e0, Q_MAX)
        pval = null_pvalue(T, q0, e0, rng)
        # rank of the claim among ALL reduced rationals q<=Q_MAX in the bracket
        lo, hi = T * (1 - W), T * (1 + W)
        cands = []
        for q in range(1, Q_MAX + 1):
            for p in range(max(1, int(lo * q)), int(hi * q) + 2):
                if Fraction(p, q).denominator != q:
                    continue
                v = p / q
                if lo <= v <= hi:
                    cands.append((abs(v - T) / abs(T), q, p))
        cands.sort()
        rank = next((i + 1 for i, c in enumerate(cands)
                     if (c[2], c[1]) == (p0 // _g(p0, q0), q0 // _g(p0, q0))), None)

        if doms:
            verdict = "MDL_DOMINATED"
        elif pval >= P_THRESH:
            verdict = "CHANCE_LEVEL"
        else:
            verdict = "RIGID"

        results.append({
            "name": name, "story": story, "claim": f"{p0}/{q0}",
            "claim_value": claim, "target": T, "rel_err_pct": e0 * 100,
            "n_dominators": len(doms),
            "best_dominator": (f"{doms[0][0]}/{doms[0][1]}"
                               f" (relerr {doms[0][3]*100:.3f}%)") if doms else None,
            "null_pvalue": pval, "rank_all_qmax": rank, "verdict": verdict,
        })
    return results


def _g(a, b):
    from math import gcd
    return gcd(a, b)


def main():
    res = audit()
    print("=" * 78)
    print("FTD-0310 RIGIDITY AUDIT — load-bearing rational identifications")
    print(f"FROZEN: Q_MAX={Q_MAX} W={W} K={K} P_THRESH={P_THRESH} seed={SEED}")
    print("=" * 78)
    for r in res:
        print(f"\n[{r['name']}]  {r['story']} = {r['claim']} = {r['claim_value']:.6f}"
              f"  vs T={r['target']:.6f}  (relerr {r['rel_err_pct']:.3f}%)")
        print(f"   MDL dominators (simpler AND better): {r['n_dominators']}"
              + (f"  best = {r['best_dominator']}" if r['best_dominator'] else ""))
        print(f"   null p-value (random-target hit-rate at q<=q0): {r['null_pvalue']:.4f}"
              f"   rank among all q<=Q_MAX: {r['rank_all_qmax']}")
        print(f"   => VERDICT: {r['verdict']}")
    print("\n" + "=" * 78)
    summ = {r["name"]: r["verdict"] for r in res}
    print("SUMMARY:", json.dumps(summ))
    print("Machine JSON:")
    print(json.dumps(res, indent=2, default=float))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
