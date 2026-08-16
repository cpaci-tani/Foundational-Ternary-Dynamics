"""derive_bath_frame_break_v1_1.py — the arrest repair (census target SC3).

The v1 run (PREREG_BATH_FRAME_BREAK_v1.md, gates failed 4/14) was sized for
leisurely drag; the unlocked diagnosis found the truth is faster and more
violent: velocity decays at rate kappa EXACTLY (the momentum argument
dP/dt = -kappa P, measured 4.8e-4 at kappa = 5e-4), and the bion then
STOPS DEAD — Peierls-Nabarro arrest: below the lattice pinning barrier the
soliton locks to a site (position frozen to the digit from tick ~2000).

v1.1 declares the actual phenomenon:
  P-A  drag at rate kappa_d ~ kappa in the early window;
  P-B  ARREST: terminal position freeze in every moving cell (the bath
       does not just prefer its frame — it captures moving bodies into it).
Lifetime anisotropy (v1's P-B) is demoted to an ungated exploratory report:
arrest makes the moving-phase lifetime window kappa-dependent and short
(disclosed design change, with reason).

Grid: kappa in {1e-4, 2e-4} (moving phase stretched across the run),
u/C in {0.25, 0.5}, lam = 0.05, T = 30000. Machinery imported from the
frozen v1 instrument (evolve_damped, env_fit) — physics unchanged.

GATES:
  G1 control: kappa=0 reproduces the frozen v2 record exactly
     (1201 events) and holds station (drift < 2 sites).
  G2 early-window drag exponentiality: log-velocity fit R^2 > 0.99
     per moving cell over ticks [500, 0.9/kappa].
  G3 drag-rate scaling: kappa_d/kappa agrees across the two kappas
     within 20% at each u.
  G4 arrest: terminal freeze in every moving cell
     (|pos| range < 0.5 site over the final 5000 ticks).
"""
from __future__ import annotations

import json
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import derive_two_body_bion_dilation_v2 as v2
import derive_bath_frame_break as bb          # frozen v1 machinery

LAM = 0.05
KAPPAS = (1e-4, 2e-4)
UCS = (0.25, 0.5)
T = 30000
N = 4096


def drag_fit_early(pos, kappa):
    """Windowed log-velocity fit over the moving phase [500, 0.9/kappa]."""
    t_end = min(int(0.9 / kappa), len(pos) - 1000)
    nwin = 16
    edges = np.linspace(500, t_end, nwin + 1, dtype=int)
    ts, vs = [], []
    for i in range(nwin):
        a, b = edges[i], edges[i + 1]
        v = np.polyfit(np.arange(a, b), pos[a:b], 1)[0]
        ts.append(0.5 * (a + b))
        vs.append(v)
    ts, vs = np.array(ts), np.array(vs)
    if np.any(vs <= 0):
        return np.nan, 0.0, ts, vs
    sl, ic = np.polyfit(ts, np.log(vs), 1)
    pred = sl * ts + ic
    r2 = 1.0 - np.sum((np.log(vs) - pred) ** 2) / \
        np.sum((np.log(vs) - np.log(vs).mean()) ** 2)
    return -float(sl), float(r2), ts, vs


def arrest(pos):
    """(frozen?, t_stop). Freeze = position range < 0.5 site over the
    final 5000 ticks; t_stop = last tick with >0.5 site motion ahead."""
    tail = pos[-5000:]
    frozen = float(tail.max() - tail.min()) < 0.5
    t_stop = None
    if frozen:
        for t in range(len(pos) - 1000, 0, -500):
            if abs(pos[min(t + 1000, len(pos) - 1)] - pos[t]) > 0.5:
                t_stop = t
                break
        t_stop = t_stop or 0
    return frozen, t_stop


def main():
    print("=" * 76)
    print("BATH-FRAME BREAK v1.1 — drag + arrest, registered run")
    print(f"  lam={LAM}, kappas={KAPPAS}, u/C={UCS}, T={T}")
    print("=" * 76)
    gates, results = {}, {}

    phi, dot = v2.pair(LAM, 0.0, N)
    tr0, pos0 = bb.evolve_damped(phi, dot, LAM, T, 0.0)
    ev0 = v2.events(tr0)
    drift = abs(pos0[-1] - pos0[0])
    print(f"  control kappa=0 rest: {len(ev0)} events (v2: 1201), "
          f"drift {drift:.2f}")
    gates["G1 events == 1201"] = bool(len(ev0) == 1201)
    gates["G1 drift < 2"] = bool(drift < 2.0)

    for kappa in KAPPAS:
        tr, _ = bb.evolve_damped(*v2.pair(LAM, 0.0, N), LAM, T, kappa)
        G0, r20 = bb.env_fit(tr)
        results[f"{kappa}|0.0"] = dict(Gamma=G0, r2_env=r20)
        print(f"  k={kappa} rest: Gamma(0)={G0:.4e} (R2 {r20:.4f}) "
              f"[exploratory]")
        for uc in UCS:
            phi, dot = v2.pair(LAM, uc * v2.C, N)
            tr, pos = bb.evolve_damped(phi, dot, LAM, T, kappa)
            kd, r2d, ts, vs = drag_fit_early(pos, kappa)
            frozen, t_stop = arrest(pos)
            G, r2e = bb.env_fit(tr)
            results[f"{kappa}|{uc}"] = dict(
                kappa_d=kd, r2_drag=r2d, frozen=bool(frozen),
                t_stop=t_stop, Gamma_expl=G, r2_env_expl=r2e,
                pos_final=float(pos[-1]), u0=uc * v2.C)
            print(f"  k={kappa} u/C={uc}: kappa_d={kd:.4e} "
                  f"(k_d/k={kd/kappa:5.3f}, R2 {r2d:.4f})  "
                  f"ARREST={'YES' if frozen else 'no'} t_stop~{t_stop}  "
                  f"[expl Gamma={G:.3e}]")
            gates[f"G2 drag k={kappa} u={uc}"] = bool(r2d > 0.99)
            gates[f"G4 arrest k={kappa} u={uc}"] = bool(frozen)
    for uc in UCS:
        r1 = results[f"{KAPPAS[0]}|{uc}"]["kappa_d"] / KAPPAS[0]
        r2_ = results[f"{KAPPAS[1]}|{uc}"]["kappa_d"] / KAPPAS[1]
        ok = np.isfinite(r1) and np.isfinite(r2_) and \
            abs(r1 - r2_) / abs(r1) < 0.20
        gates[f"G3 scaling u={uc}"] = bool(ok)
        print(f"  G3 u/C={uc}: kappa_d/kappa = {r1:.3f} vs {r2_:.3f}")
    ok_n = sum(gates.values())
    verdict = ("BREAK CONFIRMED — DRAG AT RATE kappa + TERMINAL ARREST"
               if all(gates.values())
               else "EXECUTION GATES FAILED")
    print("\n" + "=" * 76)
    print(f"GATES {ok_n}/{len(gates)}   VERDICT: {verdict}")
    print("=" * 76)
    for k, val in gates.items():
        if not val:
            print(f"  FAILED: {k}")
    out = os.path.join(HERE, "results")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "bath_frame_break_v1_1.json"), "w",
              encoding="utf-8") as f:
        json.dump({"gates": gates, "results": results,
                   "verdict": verdict}, f, indent=1, default=str)
    return 0 if all(gates.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
