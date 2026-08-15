"""derive_two_body_bion_dilation_v1_1.py — the event-map repair.

WHY v1.1 (see PREREG_TWO_BODY_BION_DILATION_v1_1.md). The v1 run failed its
own execution gates (14/20): spectral diagnosis showed the bion trace is a
MODULATED carrier — beat line + the phi^4 resonant-energy-exchange envelope
(~0.045-0.05) + sidebands — and the naive global FFT peak jumps lines under
boost. Deeper: the bion is an ANHARMONIC, chirping clock, so single-number
frequencies conflate dilation with proper-stage drift.

THE REPAIRED MEASURAND. Beat-event times. Events = times of local maxima of
the smoothed, detrended probe trace. If the composite dilates, the ENTIRE
event sequence stretches uniformly:

    t_n(u) = gamma_hat * t_n(0)   at matched cycle number n,

so cycle n is compared to cycle n — the same proper stage — making the
measurand immune to amplitude dependence, chirp, envelope, and sidebands.
Estimand: gamma_hat from a through-origin fit of t_n(u) on t_n(0) over a
declared matched range; relativity requires gamma_hat = gamma(u) with the
same value for every lam. No frequency window references gamma (no
expectation-biased search).

Physics cells identical to v1 (same lattice, integrator, preparation,
grid, held-out, volume check). Time-domain event machinery is new.

GATES: G1 capture (>= N_MIN events per cell); G2 uniformity (R^2 of the
through-origin fit > 0.999 — a non-uniform stretch is itself a failure of
the clean dilation picture); G3 volume (gamma_hat at N=8192 within 1%);
G4 universality (per-lam gamma_hat at each u within 3% of each other);
G5 held-out (blind gamma_hat(0.60) prediction from the p fit within 3%).
"""
from __future__ import annotations

import json
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))

C = 1.0 / np.sqrt(3.0)
C2 = C * C
V0 = 1.0
T_TICKS = 30000
LAMS = (0.03, 0.05)
US_FIT = (0.25, 0.40, 0.50)
U_HELD = 0.60
SEP_W = 3.0
SMOOTH = 7                       # boxcar half-width (ticks), << beat period
N_SKIP = 10                      # settling events discarded
N_MIN = 60                       # minimum matched events required
N_USE = 200                      # matched-range cap


def width(lam):
    return C * np.sqrt(2.0 / lam) / V0


def pair(lam, u, N, sep_w=SEP_W):
    X = (np.arange(N) - N // 2).astype(float)
    w = width(lam)
    D = sep_w * w
    g = 1.0 / np.sqrt(1.0 - (u / C) ** 2)
    a = np.clip(g * (X + D) / w, -300, 300)
    b = np.clip(g * (X - D) / w, -300, 300)
    phi = V0 * (np.tanh(a) - np.tanh(b) - 1.0)
    dot = -(u * g * V0 / w) * (np.cosh(a) ** -2 - np.cosh(b) ** -2)
    return phi, dot


def evolve(phi, dot, lam, T):
    prev = phi - dot
    tr = np.empty(T)
    for t in range(T):
        acc = C2 * (np.roll(phi, 1) - 2 * phi + np.roll(phi, -1)) \
            - lam * phi * (phi * phi - V0 * V0)
        phi, prev = 2 * phi - prev + acc, phi
        gx = 0.5 * (np.roll(phi, -1) - np.roll(phi, 1))
        tr[t] = np.abs(gx).max()
    return tr


def events(tr):
    """Times of local maxima of the smoothed, detrended trace (parabolic
    sub-tick refinement). Pure time-domain; no frequency window anywhere."""
    k = 2 * SMOOTH + 1
    s = np.convolve(tr, np.ones(k) / k, mode="same")
    # detrend with a wide moving mean so the envelope does not shift peaks
    wide = np.convolve(s, np.ones(301) / 301, mode="same")
    d = s - wide
    ts = []
    for t in range(1, len(d) - 1):
        if d[t] > d[t - 1] and d[t] >= d[t + 1] and d[t] > 0:
            den = d[t - 1] - 2 * d[t] + d[t + 1]
            off = 0.5 * (d[t - 1] - d[t + 1]) / den if den < 0 else 0.0
            ts.append(t + float(np.clip(off, -0.5, 0.5)))
    return np.array(ts)


def gamma_hat_fit(t0, tu):
    """Through-origin fit tu = g * t0 over the matched range; returns
    (g, R^2, n_matched). Events are matched by index after skipping."""
    n = min(len(t0), len(tu), N_USE)
    if n <= N_SKIP + 10:
        return np.nan, 0.0, n
    a, b = t0[N_SKIP:n], tu[N_SKIP:n]
    g = float(np.dot(a, b) / np.dot(a, a))
    resid = b - g * a
    r2 = 1.0 - float(np.sum(resid ** 2) / np.sum((b - b.mean()) ** 2))
    return g, r2, n


def run_cell(lam, uc, N):
    phi, dot = pair(lam, uc * C, N)
    tr = evolve(phi, dot, lam, T_TICKS)
    return events(tr)


def selftest():
    print("SELFTEST — event machinery known-answers (no boosted cell)")
    ok = True
    t = np.arange(30000, dtype=float)
    base = 1.0 + 0.2 * np.exp(-t / 6e4) * np.sin(0.2 * t) \
        + 0.08 * np.sin(0.048 * t)                    # carrier + envelope
    e0 = events(base)
    g_true = 1.10
    stretched = 1.0 + 0.2 * np.exp(-t / (6e4 * g_true)) \
        * np.sin(0.2 * t / g_true) + 0.08 * np.sin(0.048 * t / g_true)
    e1 = events(stretched)
    g, r2, n = gamma_hat_fit(e0, e1)
    err = abs(g - g_true) / g_true
    print(f"  S1 synthetic modulated chirp, true stretch {g_true}: "
          f"gamma_hat {g:.5f} (err {err:.2e}), R^2 {r2:.6f}, n {n}: "
          f"{'PASS' if err < 5e-3 and r2 > 0.999 else 'FAIL'}")
    ok &= err < 5e-3 and r2 > 0.999
    for lam in LAMS:
        ev = run_cell(lam, 0.0, 4096)
        per = np.diff(ev[N_SKIP:N_SKIP + 50])
        print(f"  S2 lam={lam:.2f} rest: {len(ev)} events, median period "
              f"{np.median(per):.2f} ticks, IQR {np.percentile(per,75)-np.percentile(per,25):.2f}: "
              f"{'PASS' if len(ev) >= N_MIN else 'FAIL'}")
        ok &= len(ev) >= N_MIN
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    print("=" * 76)
    print("TWO-BODY BION DILATION v1.1 — event-map registered run")
    print(f"  C=1/sqrt3, T={T_TICKS}, sep={SEP_W}w, lams={LAMS}, "
          f"fit u/C={US_FIT}, held-out {U_HELD}, events {N_SKIP}..{N_USE}")
    print("=" * 76)
    gates, ghats = {}, {}
    ev0 = {}
    for lam in LAMS:
        ev0[lam] = run_cell(lam, 0.0, 4096)
        n0 = len(ev0[lam])
        print(f"  lam={lam:.2f} rest: {n0} events")
        gates[f"G1 rest events lam={lam}"] = bool(n0 >= N_MIN)
    for lam in LAMS:
        for uc in US_FIT:
            ev = run_cell(lam, uc, 4096)
            g, r2, n = gamma_hat_fit(ev0[lam], ev)
            ghats[(lam, uc)] = g
            gam = 1.0 / np.sqrt(1.0 - uc ** 2)
            print(f"  lam={lam:.2f} u/C={uc:4.2f}  gamma={gam:7.5f}  "
                  f"gamma_hat={g:7.5f}  ratio={g/gam:7.4f}  R2={r2:.6f}  "
                  f"n={n}")
            gates[f"G1 events lam={lam} u={uc}"] = bool(n >= N_MIN)
            gates[f"G2 uniform lam={lam} u={uc}"] = bool(r2 > 0.999)
    for uc in US_FIT:
        a, b = ghats[(LAMS[0], uc)], ghats[(LAMS[1], uc)]
        gates[f"G4 universality u={uc}"] = bool(abs(a - b) / a < 0.03)
    # exponent p from log gamma_hat = p' log gamma  (SR: p' = +1 here)
    xs = np.array([np.log(1.0 / np.sqrt(1.0 - uc ** 2))
                   for lam in LAMS for uc in US_FIT])
    ys = np.array([np.log(ghats[(lam, uc)]) for lam in LAMS for uc in US_FIT])
    p_hat = float(np.dot(xs, ys) / np.dot(xs, xs))
    print(f"\n  pooled stretch exponent p_hat = {p_hat:+.4f}  "
          f"(the adopted law requires +1: t_n stretches by gamma)")
    g6 = 1.0 / np.sqrt(1.0 - U_HELD ** 2)
    pred = g6 ** p_hat
    print(f"  BLIND held-out prediction: gamma_hat({U_HELD}) = {pred:.5f}")
    for lam in LAMS:
        ev = run_cell(lam, U_HELD, 4096)
        g, r2, n = gamma_hat_fit(ev0[lam], ev)
        err = abs(g - pred) / pred
        print(f"  held-out lam={lam:.2f}: gamma_hat {g:.5f}  "
              f"pred {pred:.5f}  err {err:.3%}  R2 {r2:.6f}  n {n}")
        gates[f"G5 held-out lam={lam}"] = bool(err < 0.03)
    ev_big = run_cell(LAMS[0], US_FIT[1], 8192)
    ev0_big = run_cell(LAMS[0], 0.0, 8192)
    g_big, r2b, nb = gamma_hat_fit(ev0_big, ev_big)
    fv = abs(g_big - ghats[(LAMS[0], US_FIT[1])]) / ghats[(LAMS[0], US_FIT[1])]
    gates["G3 volume"] = bool(fv < 0.01)
    print(f"  volume check: N=8192 gamma_hat {g_big:.5f} vs "
          f"{ghats[(LAMS[0], US_FIT[1])]:.5f} ({fv:.3%})")
    n_ok = sum(gates.values())
    verdict = ("CONSISTENT WITH THE ADOPTED LAW (uniform stretch, p = +1)"
               if all(gates.values()) and abs(p_hat - 1.0) <= 0.05
               else ("DEVIATION CANDIDATE" if all(gates.values())
                     else "EXECUTION GATES FAILED"))
    print("\n" + "=" * 76)
    print(f"GATES {n_ok}/{len(gates)}   p_hat {p_hat:+.4f}   "
          f"VERDICT: {verdict}")
    print("=" * 76)
    for k, v in gates.items():
        if not v:
            print(f"  FAILED: {k}")
    out = os.path.join(HERE, "results")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "two_body_bion_dilation_v1_1.json"), "w",
              encoding="utf-8") as f:
        json.dump({"p_hat": p_hat, "gates": gates, "verdict": verdict,
                   "gamma_hats": {f"{l}|{u}": g for (l, u), g
                                  in ghats.items()}},
                  f, indent=1, default=str)
    return 0 if all(gates.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
