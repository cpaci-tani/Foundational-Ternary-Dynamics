"""derive_two_body_bion_dilation.py — does the TWO-BODY bound state dilate?

THE OBLIGATION (SCOPE_TWO_BODY_LORENTZ_CAMPAIGN_v1.md, S2). One-body is
closed: the phi^4 soliton shape mode dilates universally (FTD-0814,
p = -1.000 +/- 0.002). The open item is the COMPOSITE: two bodies plus
binding, one dynamics. The phi^4 kink-antikink pair at small separation
captures into a BION — a genuinely two-body bound state whose internal
beat IS the relative oscillation. Same lattice, same integrator, same
translation-invariant probe as the booked one-body instrument
(derive_soliton_shape_mode_dilation.py, conventions inherited verbatim).

THE TEST. Prepare the boosted contracted pair at small separation; it
captures; the bion translates at ~u while its beat Omega(u) is read from
the FFT peak of max|d phi/dx| over preregistered windows. Relativity
requires Omega(u) = Omega(0)/gamma — exponent p = -1 with the SAME value
for every lam (universality), no analytic rest frequency needed.

ESTIMANDS (preregistered):
  p_hat        pooled exponent from u/C in {0.25, 0.40, 0.50}
  held-out     predict Omega(0.60) = Omega(0) * gamma_.6^p_hat BEFORE
               reading the held-out cell; compare within tolerance
  C_eff        secondary: two-parameter fit Omega = Omega0 (1-u^2/Ceff^2)^{1/2}
               — a two-body data point for FTD-0814's open ~6% item

GATES (all declared in PREREG_TWO_BODY_BION_DILATION_v1.md):
  G1 capture   the rest pair must capture (beat present, no re-escape)
  G2 drift     per cell, split-window frequencies agree within 2%
  G3 volume    N = 8192 spot-check reproduces the N = 4096 cell within 1%
  G4 universality  per-lam p means agree within 0.10
  G5 held-out  measured Omega(0.60) within 3% of the blind prediction

Modes: --selftest  (synthetic known-answer for the frequency extractor +
                    rest-frame capture calibration ONLY; no boosted cell)
       (default)   the registered run
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
DROP = 0.30                      # settling fraction discarded before FFT
LAMS = (0.03, 0.05)
US_FIT = (0.25, 0.40, 0.50)     # in units of C
U_HELD = 0.60
SEP_W = 3.0                      # initial half-separation in kink widths


def width(lam):
    return C * np.sqrt(2.0 / lam) / V0


def pair(lam, u, N, sep_w=SEP_W):
    X = (np.arange(N) - N // 2).astype(float)
    w = width(lam)
    D = sep_w * w
    g = 1.0 / np.sqrt(1.0 - (u / C) ** 2)
    a, b = g * (X + D) / w, g * (X - D) / w
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


def freq_window(s):
    s = s - s.mean()
    P = np.abs(np.fft.rfft(s * np.hanning(len(s)))) ** 2
    f = np.fft.rfftfreq(len(s), d=1.0)
    k = np.argmax(P[3:]) + 3
    # parabolic interpolation of the log-power peak (sub-bin precision)
    if 1 <= k < len(P) - 1 and P[k - 1] > 0 and P[k + 1] > 0:
        la, lb, lc = np.log(P[k - 1]), np.log(P[k]), np.log(P[k + 1])
        den = la - 2 * lb + lc
        d = 0.5 * (la - lc) / den if abs(den) > 1e-30 else 0.0
        d = float(np.clip(d, -0.5, 0.5))
    else:
        d = 0.0
    return 2 * np.pi * (f[k] + d * (f[1] - f[0]))


def measure(tr):
    """Frequency + split-window drift diagnostic on the settled trace."""
    s = tr[int(len(tr) * DROP):]
    om = freq_window(s)
    h = len(s) // 2
    o1, o2 = freq_window(s[:h]), freq_window(s[h:])
    drift = abs(o1 - o2) / om if om > 0 else np.inf
    return om, drift


def run_cell(lam, uc, N):
    phi, dot = pair(lam, uc * C, N)
    tr = evolve(phi, dot, lam, T_TICKS)
    om, drift = measure(tr)
    return om, drift, tr


def selftest():
    print("SELFTEST — extractor known-answers + rest capture calibration")
    ok = True
    # S1: pure tone
    t = np.arange(20000, dtype=float)
    om_true = 0.0173
    s = 1.0 + 0.2 * np.sin(om_true * t)
    om, drift = measure(s)
    e = abs(om - om_true) / om_true
    print(f"  S1 pure tone: measured {om:.5f} vs {om_true:.5f} "
          f"(err {e:.2e}, drift {drift:.2e}): {'PASS' if e < 2e-3 else 'FAIL'}")
    ok &= e < 2e-3
    # S2: decaying, slowly-chirping tone (bion-like): extractor must land
    # near the mean instantaneous frequency and the drift gate must catch a
    # 5% chirp
    s2 = 1.0 + 0.2 * np.exp(-t / 4e4) * np.sin(om_true * t * (1 - 2.5e-6 * t / 2))
    om2, drift2 = measure(s2)
    print(f"  S2 chirped tone: measured {om2:.5f}, drift {drift2:.3f} "
          f"(gate would {'flag' if drift2 > 0.02 else 'accept'})")
    # S3: rest-frame capture at both lams (calibration; disclosed)
    for lam in LAMS:
        om, drift, tr = run_cell(lam, 0.0, 4096)
        tail = tr[-3000:]
        alive = tail.std() > 1e-6
        print(f"  S3 lam={lam:.2f} rest: Omega(0)={om:.5f}, drift={drift:.3f},"
              f" beat alive at T end: {alive} : "
              f"{'PASS' if (alive and drift < 0.02 and om > 0) else 'FAIL'}")
        ok &= alive and drift < 0.02 and om > 0
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    print("=" * 76)
    print("TWO-BODY BION DILATION — registered run "
          "(PREREG_TWO_BODY_BION_DILATION_v1)")
    print(f"  C=1/sqrt3, T={T_TICKS}, drop={DROP}, sep={SEP_W}w, "
          f"lams={LAMS}, fit u/C={US_FIT}, held-out u/C={U_HELD}")
    print("=" * 76)
    results = {}
    gates = {}
    # rest + fit cells
    for lam in LAMS:
        rows = {}
        for uc in (0.0,) + US_FIT:
            om, drift, tr = run_cell(lam, uc, 4096)
            g = 1.0 / np.sqrt(1.0 - uc ** 2)
            rows[uc] = (om, drift)
            tail_alive = tr[-3000:].std() > 1e-6
            print(f"  lam={lam:.2f} u/C={uc:4.2f}  gamma={g:7.5f}  "
                  f"Omega={om:.6f}  drift={drift:.3f}  "
                  f"beat-alive={tail_alive}")
            gates[f"G1 capture lam={lam} u={uc}"] = bool(tail_alive)
            gates[f"G2 drift lam={lam} u={uc}"] = bool(drift < 0.02)
        results[lam] = rows
    # exponent fit (pooled over fit cells only)
    ps = []
    for lam in LAMS:
        om0 = results[lam][0.0][0]
        for uc in US_FIT:
            g = 1.0 / np.sqrt(1.0 - uc ** 2)
            ps.append(np.log(results[lam][uc][0] / om0) / np.log(g))
    p_hat = float(np.mean(ps))
    p_std = float(np.std(ps))
    per_lam = {lam: float(np.mean([
        np.log(results[lam][uc][0] / results[lam][0.0][0])
        / np.log(1.0 / np.sqrt(1.0 - uc ** 2)) for uc in US_FIT]))
        for lam in LAMS}
    uni_spread = max(per_lam.values()) - min(per_lam.values())
    gates["G4 universality (spread<0.10)"] = bool(uni_spread < 0.10)
    print(f"\n  pooled p_hat = {p_hat:+.4f} +/- {p_std:.4f}   "
          f"per-lam means {per_lam}   spread {uni_spread:.4f}")
    # blind held-out prediction BEFORE measuring
    g6 = 1.0 / np.sqrt(1.0 - U_HELD ** 2)
    pred = {lam: results[lam][0.0][0] * g6 ** p_hat for lam in LAMS}
    print(f"  BLIND held-out predictions at u/C={U_HELD}: "
          + "  ".join(f"lam={l}: {p:.6f}" for l, p in pred.items()))
    for lam in LAMS:
        om, drift, tr = run_cell(lam, U_HELD, 4096)
        err = abs(om - pred[lam]) / pred[lam]
        print(f"  held-out lam={lam:.2f}: measured {om:.6f}  "
              f"predicted {pred[lam]:.6f}  err {err:.3%}  drift {drift:.3f}")
        gates[f"G5 held-out lam={lam} (err<3%)"] = bool(err < 0.03)
        results[lam][U_HELD] = (om, drift)
    # finite-volume spot check
    om_big, drift_big, _ = run_cell(LAMS[0], US_FIT[1], 8192)
    om_ref = results[LAMS[0]][US_FIT[1]][0]
    fv = abs(om_big - om_ref) / om_ref
    gates["G3 volume (N=8192 within 1%)"] = bool(fv < 0.01)
    print(f"  volume check lam={LAMS[0]} u/C={US_FIT[1]}: N=8192 gives "
          f"{om_big:.6f} vs {om_ref:.6f} ({fv:.3%})")
    # secondary estimand: C_eff
    print("\n  secondary C_eff fit (Omega = Om0 sqrt(1 - u^2/Ceff^2)):")
    ceffs = {}
    for lam in LAMS:
        us = np.array([uc * C for uc in US_FIT + (U_HELD,)])
        oms = np.array([results[lam][uc][0] for uc in US_FIT + (U_HELD,)])
        om0 = results[lam][0.0][0]
        y = 1.0 - (oms / om0) ** 2          # = u^2/Ceff^2 under the law
        A = (us ** 2)[:, None]
        slope = float(np.linalg.lstsq(A, y, rcond=None)[0][0])
        ceff = 1.0 / np.sqrt(slope) if slope > 0 else np.nan
        ceffs[lam] = ceff
        print(f"    lam={lam:.2f}: C_eff = {ceff:.5f} = "
              f"{ceff / C:.4f} C  (FTD-0814 one-body open item: ~6% high)")
    n_ok = sum(gates.values())
    verdict = ("CONSISTENT WITH THE ADOPTED LAW (p = -1)"
               if abs(p_hat + 1.0) <= max(0.05, 2 * p_std)
               and all(gates.values())
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
    with open(os.path.join(out, "two_body_bion_dilation.json"), "w",
              encoding="utf-8") as f:
        json.dump({"p_hat": p_hat, "p_std": p_std, "per_lam": per_lam,
                   "gates": gates, "ceff": ceffs, "verdict": verdict,
                   "results": {str(l): {str(u): r for u, r in d.items()}
                               for l, d in results.items()}},
                  f, indent=1, default=str)
    return 0 if all(gates.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
