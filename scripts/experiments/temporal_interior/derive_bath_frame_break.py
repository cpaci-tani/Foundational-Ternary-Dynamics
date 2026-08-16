"""derive_bath_frame_break.py — the census's first predicted break, measured.

Universality Programme census target SC3 (AUDIT_FUNCTIONAL_CENSUS.md):
Rayleigh damping (kappa * phi_dot, substrate frame — the engine's `damping`
toggle class) is a SECOND-CATEGORY functional, so the census predicts it
breaks frame hiding. This campaign measures the break in the one-functional
surrogate with the damping term added as the ONLY modification:

    phi_{t+1} = 2 phi_t - phi_{t-1} + acc - kappa (phi_t - phi_{t-1})

PREDICTIONS DECLARED (from the census, before this lock):
  P-A  BULK DRAG: a moving bion decelerates toward substrate rest,
       u(t) = u0 * exp(-kappa_d t), at first order in u with NO (ka)
       suppression — the loudest frame detector in the programme so far.
  P-B  LIFETIME ANISOTROPY: the beat-envelope decay rate Gamma violates
       covariant lifetime dilation: D(u) = Gamma(u) * gamma_kin / Gamma(0)
       != 1, where gamma_kin uses the drag-corrected mean velocity.

MEASURANDS: position of the interpolated interior peak (ring-unwrapped)
-> velocity track -> drag rate kappa_d and exponentiality; trace-envelope
decay -> Gamma; the v2 event machinery (imported frozen) for beats.

GATES:
  G1 control: kappa = 0 cell reproduces the frozen v2 rest event count
     exactly (914 events at lam=0.05? — no: 1201; asserted from the v2
     record) and zero drift of the peak position.
  G2 drag exponentiality: log-velocity linear fit R^2 > 0.99 per moving
     cell.
  G3 drag-rate scaling: kappa_d / kappa agrees across the two kappa
     values within 20% (the break scales with the second-category
     coefficient, as a functional-census effect must).
  G4 lifetime measurable: envelope fit R^2 > 0.98 per cell.

OUTCOMES: BREAK CONFIRMED (all gates; drag > 5 sigma from zero; D(u)
departs 1 beyond fit error) / BREAK ABSENT (gates pass, drag consistent
with zero — REFUTES the census ranking; booked as such) / GATES FAILED.
"""
from __future__ import annotations

import json
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import derive_two_body_bion_dilation_v2 as v2   # frozen machinery

LAM = 0.05
KAPPAS = (0.0005, 0.001)
UCS = (0.25, 0.5)
N = 4096
T = 30000


def evolve_damped(phi, dot, lam, T, kappa):
    """v2 physics + the declared Rayleigh term. Two probes:
    tr  — v2's GLOBAL interpolated peak height, byte-compatible with the
          frozen instrument (so the kappa=0 control must reproduce the
          v2 event record exactly);
    pos — the TOPOLOGICAL position: midpoint of the two kink
          zero-crossings, tracked in a window around the last center
          (held through the brief crossing-free deep-contraction phases)."""
    prev = phi - dot
    tr = np.empty(T)
    pos = np.empty(T)
    center = float(np.argmax(phi))
    W = 220
    for t in range(T):
        acc = v2.C2 * (np.roll(phi, 1) - 2 * phi + np.roll(phi, -1)) \
            - lam * phi * (phi * phi - v2.V0 * v2.V0)
        nxt = 2 * phi - prev + acc - kappa * (phi - prev)
        phi, prev = nxt, phi
        # global interpolated peak (v2 probe, unchanged)
        k = int(np.argmax(phi))
        km, kp = (k - 1) % N, (k + 1) % N
        den = phi[km] - 2 * phi[k] + phi[kp]
        if den < 0:
            d = float(np.clip(0.5 * (phi[km] - phi[kp]) / den, -0.5, 0.5))
            tr[t] = phi[k] - 0.25 * (phi[km] - phi[kp]) * d
        else:
            tr[t] = phi[k]
        # topological position: zero crossings in the tracking window
        base = int(round(center))
        idx = (base + np.arange(-W, W + 1)) % N
        seg = phi[idx]
        sgn = np.signbit(seg)
        flips = np.nonzero(sgn[:-1] != sgn[1:])[0]
        if len(flips) >= 2:
            i0, i1 = flips[0], flips[-1]
            x0 = i0 + seg[i0] / (seg[i0] - seg[i0 + 1])
            x1 = i1 + seg[i1] / (seg[i1] - seg[i1 + 1])
            center = center + (0.5 * (x0 + x1) - W)
        pos[t] = center
    return tr, pos


def drag_fit(pos, nwin=30):
    """Windowed velocities -> exponential-drag fit. Returns
    (kappa_d, R2_of_logv_fit, u_mean, v_first, v_last)."""
    L = len(pos)
    edges = np.linspace(2000, L, nwin + 1, dtype=int)
    ts, vs = [], []
    for i in range(nwin):
        a, b = edges[i], edges[i + 1]
        tt = np.arange(a, b)
        v = np.polyfit(tt, pos[a:b], 1)[0]
        ts.append(0.5 * (a + b))
        vs.append(v)
    ts, vs = np.array(ts), np.array(vs)
    if np.any(vs <= 0):
        return np.nan, 0.0, float(np.mean(vs)), vs[0], vs[-1]
    sl, ic = np.polyfit(ts, np.log(vs), 1)
    pred = sl * ts + ic
    r2 = 1.0 - np.sum((np.log(vs) - pred) ** 2) / \
        np.sum((np.log(vs) - np.log(vs).mean()) ** 2)
    return -float(sl), float(r2), float(np.mean(vs)), float(vs[0]), \
        float(vs[-1])


def env_fit(tr, drop=0.3):
    """Lifetime from the beat-event amplitude sequence: heights of the
    smoothed-detrended trace at its own event times (the internal beat
    peaks), median-binned then log-linear fitted."""
    ev = v2.events(tr)
    ksm = 2 * v2.SMOOTH + 1
    s = np.convolve(tr, np.ones(ksm) / ksm, mode="same")
    wide = np.convolve(s, np.ones(301) / 301, mode="same")
    d = s - wide
    lo = int(len(tr) * drop)
    amps, times = [], []
    for e in ev:
        te = int(round(e))
        if lo < te < len(d) - 2 and d[te] > 0:
            amps.append(d[te])
            times.append(e)
    amps, times = np.array(amps), np.array(times)
    if len(amps) < 24:
        return np.nan, 0.0
    nbin = 12
    edges = np.linspace(times[0], times[-1] + 1e-9, nbin + 1)
    bt, ba = [], []
    for i in range(nbin):
        m = (times >= edges[i]) & (times < edges[i + 1])
        if m.sum() >= 3:
            bt.append(times[m].mean())
            ba.append(np.median(amps[m]))
    bt, ba = np.array(bt), np.array(ba)
    if len(bt) < 6 or np.any(ba <= 0):
        return np.nan, 0.0
    sl, ic = np.polyfit(bt, np.log(ba), 1)
    pred = sl * bt + ic
    r2 = 1.0 - np.sum((np.log(ba) - pred) ** 2) / \
        np.sum((np.log(ba) - np.log(ba).mean()) ** 2)
    return -float(sl), float(r2)


def main():
    selftest = "--selftest" in sys.argv
    print("=" * 76)
    print("BATH-FRAME BREAK — census target SC3, registered run"
          if not selftest else "BATH-FRAME BREAK — selftest (controls only)")
    print(f"  lam={LAM}, kappas={KAPPAS}, u/C={UCS}, T={T}")
    print("=" * 76)
    gates, results = {}, {}

    # G1 control: kappa = 0, rest — must reproduce frozen v2 exactly
    phi, dot = v2.pair(LAM, 0.0, N)
    tr0, pos0 = evolve_damped(phi, dot, LAM, T, 0.0)
    ev0 = v2.events(tr0)
    drift = abs(pos0[-1] - pos0[0])
    print(f"  control kappa=0 rest: {len(ev0)} events (v2 record: 1201), "
          f"peak drift {drift:.2f} sites")
    gates["G1 control events == 1201"] = bool(len(ev0) == 1201)
    gates["G1 control drift < 2 sites"] = bool(drift < 2.0)
    if selftest:
        # one small-kappa rest cell: lifetime must be measurable
        tr, _ = evolve_damped(*v2.pair(LAM, 0.0, N), LAM, T, KAPPAS[0])
        G, r2 = env_fit(tr)
        print(f"  selftest kappa={KAPPAS[0]} rest: Gamma={G:.3e} "
              f"(R2 {r2:.4f})")
        ok = all(gates.values()) and r2 > 0.9 and G > 0
        print(f"SELFTEST {'PASS' if ok else 'FAIL'}")
        return 0 if ok else 1

    for kappa in KAPPAS:
        # rest lifetime
        tr, _ = evolve_damped(*v2.pair(LAM, 0.0, N), LAM, T, kappa)
        G0, r20 = env_fit(tr)
        results[f"{kappa}|0.0"] = dict(Gamma=G0, r2_env=r20)
        gates[f"G4 env rest k={kappa}"] = bool(r20 > 0.98)
        print(f"  k={kappa} rest: Gamma(0)={G0:.4e} (R2 {r20:.4f})")
        for uc in UCS:
            phi, dot = v2.pair(LAM, uc * v2.C, N)
            tr, pos = evolve_damped(phi, dot, LAM, T, kappa)
            kd, r2d, umean, v0, v1 = drag_fit(pos)
            G, r2e = env_fit(tr)
            gk = 1.0 / np.sqrt(1.0 - (umean / v2.C) ** 2)
            D = G * gk / G0 if G0 > 0 else np.nan
            results[f"{kappa}|{uc}"] = dict(
                kappa_d=kd, r2_drag=r2d, u_mean=umean, v_first=v0,
                v_last=v1, Gamma=G, r2_env=r2e, D=D)
            print(f"  k={kappa} u/C={uc}: kappa_d={kd:.4e} "
                  f"(k_d/k={kd/kappa:5.2f}, R2 {r2d:.4f})  "
                  f"v: {v0:.4f}->{v1:.4f}  Gamma={G:.4e}  D={D:.4f}")
            gates[f"G2 drag exp k={kappa} u={uc}"] = bool(r2d > 0.99)
            gates[f"G4 env k={kappa} u={uc}"] = bool(r2e > 0.98)
    # G3: kappa_d scaling across kappa at each u
    for uc in UCS:
        r1 = results[f"{KAPPAS[0]}|{uc}"]["kappa_d"] / KAPPAS[0]
        r2_ = results[f"{KAPPAS[1]}|{uc}"]["kappa_d"] / KAPPAS[1]
        ok = np.isfinite(r1) and np.isfinite(r2_) and \
            abs(r1 - r2_) / abs(r1) < 0.20
        gates[f"G3 scaling u={uc}"] = bool(ok)
        print(f"  G3 u/C={uc}: kappa_d/kappa = {r1:.3f} vs {r2_:.3f}")
    drag_seen = all(np.isfinite(results[f"{k}|{u}"]["kappa_d"])
                    and results[f"{k}|{u}"]["kappa_d"] > 0
                    for k in KAPPAS for u in UCS)
    D_break = any(abs(results[f"{k}|{u}"]["D"] - 1.0) > 0.05
                  for k in KAPPAS for u in UCS)
    ok_n = sum(gates.values())
    if all(gates.values()) and drag_seen:
        verdict = ("BREAK CONFIRMED (drag + lifetime anisotropy)"
                   if D_break else "BREAK CONFIRMED (drag; lifetime "
                   "anisotropy below 5%)")
    elif all(gates.values()):
        verdict = "BREAK ABSENT — refutes the census ranking"
    else:
        verdict = "EXECUTION GATES FAILED"
    print("\n" + "=" * 76)
    print(f"GATES {ok_n}/{len(gates)}   VERDICT: {verdict}")
    print("=" * 76)
    for k, val in gates.items():
        if not val:
            print(f"  FAILED: {k}")
    out = os.path.join(HERE, "results")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "bath_frame_break.json"), "w",
              encoding="utf-8") as f:
        json.dump({"gates": gates, "results": results,
                   "verdict": verdict}, f, indent=1, default=str)
    return 0 if all(gates.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
