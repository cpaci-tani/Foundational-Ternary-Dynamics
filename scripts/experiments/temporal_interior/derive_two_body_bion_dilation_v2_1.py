"""derive_two_body_bion_dilation_v2_1.py — the declared deviation model.

P1 of the Universality Programme (PREREG_TWO_BODY_BION_DILATION_v2_1.md).
The FTD-1009 chain ended gates-failed with a clean signal; the P1a
diagnostic (commit 2412115e) found the residual structure SECULAR-dominant
(quadratic proper-stage drift, u-scaling exponent +2.91; the apparent
periodic line is an FFT-bin artifact). The declared model, fixed by that
diagnostic before this lock:

    t_n(u) = gamma_hat * t_n(0) * (1 + b1*m + b2*m^2),
    m = (n - n_mid) / n_mid   over the matched range n in [N_SKIP, N_USE),

with (b1, b2) per-cell nuisance parameters and gamma_hat the physics
estimand. Linear least squares in (gamma_hat, gamma_hat*b1, gamma_hat*b2)
— exact, no iteration. Physics cells byte-identical to the frozen v2
instrument (imported); event arrays are persisted this time.

GATES (declared):
  G1 events >= 60 per cell (inherited)
  G2 model residual R^2 > 0.9995 per fit cell (the model must absorb what
     broke the raw-uniformity gate; tighter than v2's 0.999)
  G3 volume: gamma_hat at N=8192 within 1% (inherited)
  G4 universality: per-lam gamma_hat within 3% at each fit u (inherited)
  G5 blind held-out: pooled p_hat predicts gamma_hat(0.60) within 3%
     BEFORE the held-out cells are read (inherited)

Estimands: gamma_hat per cell (primary), pooled stretch exponent p_hat
(law requires +1), secular amplitude scaling exponent (secondary, no gate).
Outcomes per the prereg: CONSISTENT / DEVIATION CANDIDATE / GATES FAILED.
"""
from __future__ import annotations

import json
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import derive_two_body_bion_dilation_v2 as v2   # frozen physics cells

LAMS = v2.LAMS
US_FIT = v2.US_FIT
U_HELD = v2.U_HELD
N_SKIP, N_USE, N_MIN = v2.N_SKIP, v2.N_USE, v2.N_MIN


def model_fit(t0, tu):
    """Exact LS of tu = g*t0*(1 + b1*m + b2*m^2); returns
    (gamma_hat, b1, b2, R^2, n_matched)."""
    n = min(len(t0), len(tu), N_USE)
    if n <= N_SKIP + 10:
        return np.nan, 0.0, 0.0, 0.0, n
    a, b = np.asarray(t0[N_SKIP:n]), np.asarray(tu[N_SKIP:n])
    idx = np.arange(N_SKIP, n, dtype=float)
    n_mid = 0.5 * (N_SKIP + n - 1)
    m = (idx - n_mid) / n_mid
    X = np.column_stack([a, a * m, a * m * m])
    coef, *_ = np.linalg.lstsq(X, b, rcond=None)
    g = float(coef[0])
    b1, b2 = float(coef[1] / g), float(coef[2] / g)
    resid = b - X @ coef
    r2 = 1.0 - float(np.sum(resid ** 2) / np.sum((b - b.mean()) ** 2))
    return g, b1, b2, r2, n


def main():
    print("=" * 76)
    print("TWO-BODY BION DILATION v2.1 — declared deviation model, "
          "registered run")
    print(f"  model: t_n(u) = ghat*t_n(0)*(1 + b1*m + b2*m^2), "
          f"events {N_SKIP}..{N_USE}")
    print("=" * 76)
    gates, ghats, cells = {}, {}, {}
    ev = {}
    for lam in LAMS:
        ev[(lam, 0.0)] = v2.run_cell(lam, 0.0, 4096)
        n0 = len(ev[(lam, 0.0)])
        print(f"  lam={lam:.2f} rest: {n0} events")
        gates[f"G1 rest lam={lam}"] = bool(n0 >= N_MIN)
    for lam in LAMS:
        for uc in US_FIT:
            ev[(lam, uc)] = v2.run_cell(lam, uc, 4096)
            g, b1, b2, r2, n = model_fit(ev[(lam, 0.0)], ev[(lam, uc)])
            gam = 1.0 / np.sqrt(1.0 - uc ** 2)
            ghats[(lam, uc)] = g
            cells[f"{lam}|{uc}"] = dict(ghat=g, b1=b1, b2=b2, r2=r2, n=n)
            print(f"  lam={lam:.2f} u/C={uc:4.2f}  gamma={gam:7.5f}  "
                  f"ghat={g:7.5f}  ratio={g/gam:6.4f}  b1={b1:+.4f}  "
                  f"b2={b2:+.4f}  R2={r2:.6f}")
            gates[f"G1 events lam={lam} u={uc}"] = bool(n >= N_MIN)
            gates[f"G2 model lam={lam} u={uc}"] = bool(r2 > 0.9995)
    for uc in US_FIT:
        a, b = ghats[(LAMS[0], uc)], ghats[(LAMS[1], uc)]
        gates[f"G4 universality u={uc}"] = bool(abs(a - b) / a < 0.03)
    xs = np.array([np.log(1.0 / np.sqrt(1.0 - uc ** 2))
                   for lam in LAMS for uc in US_FIT])
    ys = np.array([np.log(ghats[(lam, uc)]) for lam in LAMS
                   for uc in US_FIT])
    p_hat = float(np.dot(xs, ys) / np.dot(xs, xs))
    g6 = 1.0 / np.sqrt(1.0 - U_HELD ** 2)
    pred = g6 ** p_hat
    print(f"\n  pooled p_hat = {p_hat:+.4f}  (law requires +1)")
    print(f"  BLIND held-out prediction: ghat({U_HELD}) = {pred:.5f}")
    for lam in LAMS:
        ev[(lam, U_HELD)] = v2.run_cell(lam, U_HELD, 4096)
        g, b1, b2, r2, n = model_fit(ev[(lam, 0.0)], ev[(lam, U_HELD)])
        err = abs(g - pred) / pred
        cells[f"{lam}|{U_HELD}"] = dict(ghat=g, b1=b1, b2=b2, r2=r2, n=n)
        print(f"  held-out lam={lam:.2f}: ghat {g:.5f}  pred {pred:.5f}  "
              f"err {err:.3%}  R2 {r2:.6f}")
        gates[f"G5 held-out lam={lam}"] = bool(err < 0.03)
    ev_big0 = v2.run_cell(LAMS[0], 0.0, 8192)
    ev_big = v2.run_cell(LAMS[0], US_FIT[1], 8192)
    g_big, *_ = model_fit(ev_big0, ev_big)
    fv = abs(g_big - ghats[(LAMS[0], US_FIT[1])]) / ghats[(LAMS[0],
                                                          US_FIT[1])]
    gates["G3 volume"] = bool(fv < 0.01)
    print(f"  volume: N=8192 ghat {g_big:.5f} ({fv:.3%})")
    # secondary estimand: secular amplitude scaling (no gate)
    amp = [abs(cells[f"{lam}|{uc}"]["b2"]) + abs(cells[f"{lam}|{uc}"]["b1"])
           for lam in LAMS for uc in US_FIT]
    slope = float(np.polyfit(np.log([uc for _ in LAMS for uc in US_FIT]),
                             np.log(np.maximum(amp, 1e-12)), 1)[0])
    print(f"  secular amplitude u-scaling exponent (secondary): "
          f"{slope:+.2f}")
    ok_n = sum(gates.values())
    verdict = ("CONSISTENT WITH THE ADOPTED LAW + DECLARED LATTICE MODEL"
               if all(gates.values()) and abs(p_hat - 1.0) <= 0.05
               else ("DEVIATION CANDIDATE" if all(gates.values())
                     else "EXECUTION GATES FAILED"))
    print("\n" + "=" * 76)
    print(f"GATES {ok_n}/{len(gates)}   p_hat {p_hat:+.4f}   "
          f"VERDICT: {verdict}")
    print("=" * 76)
    for k, val in gates.items():
        if not val:
            print(f"  FAILED: {k}")
    out = os.path.join(HERE, "results")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "two_body_bion_dilation_v2_1.json"), "w",
              encoding="utf-8") as f:
        json.dump({"p_hat": p_hat, "gates": gates, "cells": cells,
                   "verdict": verdict, "secular_scaling": slope,
                   "events": {f"{l}|{u}": [round(float(t), 3) for t in e]
                              for (l, u), e in ev.items()}},
                  f, indent=1, default=str)
    return 0 if all(gates.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
