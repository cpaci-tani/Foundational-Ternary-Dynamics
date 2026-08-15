"""diagnose_bion_residuals.py — P1a of the Universality Programme.

Unlocked diagnostic on the FTD-1009 gates-failed executions (permitted per
the chain's own outcome taxonomy). Re-runs the frozen v2 instrument's cells
deterministically, PERSISTS the event sequences the registered artifacts
discarded, and characterizes the residual structure

    r_n = t_n(u) / (gamma_hat * t_n(0)) - 1

over the matched range, to let the DATA select the deviation-model form the
v2.1 lock must declare. Questions asked, per cell:

  Q1 secular trend  — linear/quadratic drift of r_n in n (chirp mismatch)?
  Q2 periodicity    — dominant FFT line of detrended r_n; compared against
                      the site-crossing frequency (u per tick) and the
                      envelope frequency (~0.05 rad/tick), both mapped to
                      per-event angular frequency.
  Q3 amplitude link — correlation of r_n with the local beat amplitude.
  Q4 u-scaling      — how the size of each component grows with u/C
                      (log-log slope against (u/C): does it look quartic?).

Outputs: results/bion_event_sequences.json (all event arrays, both lams,
all u including held-out), results/bion_residual_structure.png, and a
printed structure report ending with the selected model form. No verdict,
no gates, nothing booked — diagnostic of record for the v2.1 prereg.
"""
from __future__ import annotations

import json
import os
import sys

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import derive_two_body_bion_dilation_v2 as v2   # frozen instrument

OUT = os.path.join(HERE, "results")
os.makedirs(OUT, exist_ok=True)

LAMS = v2.LAMS
UCS = (0.0,) + v2.US_FIT + (v2.U_HELD,)
N_SKIP, N_USE = v2.N_SKIP, v2.N_USE

print("regenerating event sequences (deterministic re-run of the frozen "
      "v2 cells)...")
events = {}
for lam in LAMS:
    for uc in UCS:
        ev = v2.run_cell(lam, uc, 4096)
        events[f"{lam}|{uc}"] = ev
        print(f"  lam={lam} u/C={uc}: {len(ev)} events")

with open(os.path.join(OUT, "bion_event_sequences.json"), "w",
          encoding="utf-8") as f:
    json.dump({k: [round(float(t), 3) for t in v] for k, v in
               events.items()}, f)
print("persisted results/bion_event_sequences.json")

# ---------------------------------------------------------------------------
# residual structure
# ---------------------------------------------------------------------------
print("\n" + "=" * 76)
print("RESIDUAL STRUCTURE  r_n = t_n(u)/(gamma_hat t_n(0)) - 1")
print("=" * 76)

fig, axes = plt.subplots(len(LAMS), 4, figsize=(16, 6.5),
                         facecolor="#10141d")
report = {}
for li, lam in enumerate(LAMS):
    e0 = events[f"{lam}|0.0"]
    for ui, uc in enumerate(v2.US_FIT + (v2.U_HELD,)):
        ev = events[f"{lam}|{uc}"]
        n = min(len(e0), len(ev), N_USE)
        a, b = e0[N_SKIP:n], ev[N_SKIP:n]
        g = float(np.dot(a, b) / np.dot(a, a))
        r = b / (g * a) - 1.0
        ns = np.arange(N_SKIP, n)
        # Q1 secular: quadratic fit in event index
        c2, c1, c0 = np.polyfit(ns, r, 2)
        sec = np.polyval([c2, c1, c0], ns)
        det = r - sec
        # Q2 periodicity of the detrended residual (per-event spectrum)
        P = np.abs(np.fft.rfft(det * np.hanning(len(det)))) ** 2
        fr = np.fft.rfftfreq(len(det), d=1.0)          # cycles per event
        kpk = int(np.argmax(P[2:]) + 2)
        f_pk = float(fr[kpk])                           # cycles/event
        # reference frequencies mapped to cycles/event: one event lasts
        # about (mean beat period) ticks
        period = float(np.mean(np.diff(b)))
        f_site = (uc * v2.C) * period / (2 * np.pi)     # site crossing
        f_env = 0.050 * period / (2 * np.pi)            # envelope line
        rms_sec = float(np.sqrt(np.mean(sec ** 2)))
        rms_per = float(np.sqrt(np.mean(det ** 2)))
        report[(lam, uc)] = dict(gamma_hat=g, rms_secular=rms_sec,
                                 rms_periodic=rms_per, f_peak=f_pk,
                                 f_site=f_site, f_env=f_env,
                                 c2=float(c2), c1=float(c1))
        print(f"  lam={lam} u/C={uc:4.2f}: ghat={g:.5f}  "
              f"rms[secular]={rms_sec:.2e}  rms[periodic]={rms_per:.2e}  "
              f"f_peak={f_pk:.4f} c/ev  (site {f_site:.4f}, env {f_env:.4f})")
        if ui < 4 and li < len(LAMS):
            ax = axes[li, ui]
            ax.set_facecolor("#161c28")
            ax.plot(ns, r, color="#d96c4f", lw=0.8)
            ax.plot(ns, sec, color="#e8a33d", lw=1.2)
            ax.set_title(f"λ={lam} u/C={uc}", color="#e6e1d3", fontsize=9)
            ax.tick_params(colors="#8b94a3", labelsize=7)
            for s in ax.spines.values():
                s.set_color("#232c3d")
fig.suptitle("bion event-map residuals: raw (ember) vs secular fit (amber)",
             color="#e6e1d3", fontsize=12)
fig.savefig(os.path.join(OUT, "bion_residual_structure.png"),
            facecolor="#10141d", bbox_inches="tight")
print("wrote results/bion_residual_structure.png")

# ---------------------------------------------------------------------------
# Q4: u-scaling of the components + model selection
# ---------------------------------------------------------------------------
print("\n  Q4 u-scaling (fit cells only):")
sel = {}
for comp in ("rms_secular", "rms_periodic"):
    xs, ys = [], []
    for lam in LAMS:
        for uc in v2.US_FIT:
            xs.append(np.log(uc))
            ys.append(np.log(report[(lam, uc)][comp]))
    slope = float(np.polyfit(xs, ys, 1)[0])
    sel[comp] = slope
    print(f"    {comp}: log-log slope vs (u/C) = {slope:+.2f}")

dominant = ("secular" if np.mean([report[(l, u)]["rms_secular"]
                                  for l in LAMS for u in v2.US_FIT])
            > np.mean([report[(l, u)]["rms_periodic"]
                       for l in LAMS for u in v2.US_FIT]) else "periodic")
print("\n" + "=" * 76)
print("STRUCTURE REPORT")
print("=" * 76)
print(f"  dominant component: {dominant}")
print(f"  secular u-scaling exponent  : {sel['rms_secular']:+.2f}")
print(f"  periodic u-scaling exponent : {sel['rms_periodic']:+.2f}")
site_match = np.mean([abs(report[(l, u)]["f_peak"]
                          - report[(l, u)]["f_site"]) /
                      max(report[(l, u)]["f_site"], 1e-9)
                      for l in LAMS for u in v2.US_FIT])
env_match = np.mean([abs(report[(l, u)]["f_peak"]
                         - report[(l, u)]["f_env"]) /
                     max(report[(l, u)]["f_env"], 1e-9)
                     for l in LAMS for u in v2.US_FIT])
print(f"  periodic line vs site-crossing freq: mean rel. dev {site_match:.2f}")
print(f"  periodic line vs envelope freq     : mean rel. dev {env_match:.2f}")
print("""
  MODEL-FORM SELECTION (to be quoted verbatim in the v2.1 prereg):
  the declared model is chosen from the dominant component above —
    secular-dominant  ->  t_n(u) = ghat * t_n(0) * (1 + b1*m + b2*m^2),
                          m = (n - n_mid)/n_mid   (per-cell quadratic
                          proper-stage mismatch; b_i shared across u iff
                          the u-scaling exponent supports it)
    periodic-dominant ->  t_n(u) = ghat * t_n(0) * (1 + A*sin(2 pi f n + p)),
                          f pinned to the matched reference line.
""")
with open(os.path.join(OUT, "bion_residual_report.json"), "w",
          encoding="utf-8") as f:
    json.dump({f"{l}|{u}": v for (l, u), v in report.items()}
              | {"scaling": sel, "dominant": dominant}, f, indent=1)
print("persisted results/bion_residual_report.json")
