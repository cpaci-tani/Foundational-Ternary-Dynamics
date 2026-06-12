#!/usr/bin/env python3
"""
FTD-0276 Leg B — friction-knob (γ) map of the N(A) forward model.

FTD-0269 found the Langevin friction γ "load-bearing (super-knee normalization)":
the friction-free forward model over-predicts the super-knee branch by ~1.8×
(curve-RMS 0.170). This driver converts that qualitative boundary into a
quantitative map: it runs the FTD-0269 forward model (genesis_na_law_forward.py,
with its post-lock γ knob) over a γ grid and reports, per γ:

  - the broken-power-law knee A_knee
  - the sub-knee and super-knee exponents (p_lo, p_hi)
  - the super-knee curve ratio vs the FTD-0261 engine law (the over-prediction
    factor the friction is meant to remove)

This is an ENGINE-FREE forward-model map (Leg B of FTD-0276); it makes no claim
the friction is framework-derived — γ is an engine-tuning constant (that is the
FTD-0269 BOUNDARY). The value is the calibrated knee(γ)/exponent(γ) curve.

Usage:
  python scripts/exploration/genesis_na_law_gamma_sweep.py \
      --L 32 --seeds 5 --gammas 0,0.01,0.02,0.05,0.1
"""

import argparse
import math
import sys

import numpy as np

import genesis_na_law_forward as fwd

# FTD-0261 current-stack reference (the engine law the forward model targets).
FTD0261_A = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
FTD0261_N = [4.0, 8.4, 16.4, 21.6, 27.4, 32.6, 45.0, 91.8, 130.2, 260.2, 383.3]
FTD0261_KNEE = 16.0
FTD0261_PLO = 3.69
FTD0261_PHI = 1.86


def fit_broken_power(A, N):
    """Segmented log-log fit -> (knee, p_lo, p_hi, log10_rms). Mirrors
    analyze_na_law.fit_broken_power."""
    A = np.asarray(A, float); N = np.asarray(N, float)
    m = N > 0
    A, N = A[m], N[m]
    lA, lN = np.log10(A), np.log10(N)
    best = None
    for ki in range(1, len(A) - 1):
        lo = slice(0, ki + 1); hi = slice(ki, len(A))
        if (lo.stop - lo.start) < 2 or (len(A) - hi.start) < 2:
            continue
        p_lo, b_lo = np.polyfit(lA[lo], lN[lo], 1)
        p_hi, b_hi = np.polyfit(lA[hi], lN[hi], 1)
        res_lo = lN[lo] - (p_lo * lA[lo] + b_lo)
        res_hi = lN[hi] - (p_hi * lA[hi] + b_hi)
        rms = math.sqrt((np.sum(res_lo ** 2) + np.sum(res_hi ** 2)) / len(A))
        if best is None or rms < best[3]:
            best = (float(A[ki]), float(p_lo), float(p_hi), float(rms))
    return best


def superknee_ratio(A, N):
    """Geometric-mean ratio model/engine over the super-knee branch (A >= knee)."""
    ratios = []
    ref = dict(zip(FTD0261_A, FTD0261_N))
    for a, n in zip(A, N):
        if a >= FTD0261_KNEE and a in ref and ref[a] > 0 and n > 0:
            ratios.append(n / ref[a])
    if not ratios:
        return float("nan")
    return math.exp(sum(math.log(r) for r in ratios) / len(ratios))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--L", type=int, default=32)
    ap.add_argument("--seeds", type=int, default=5)
    ap.add_argument("--ticks", type=int, default=60)
    ap.add_argument("--gammas", default="0,0.01,0.02,0.05,0.1")
    args = ap.parse_args()
    gammas = [float(x) for x in args.gammas.split(",")]
    grid = FTD0261_A

    print("=" * 72)
    print("FTD-0276 Leg B - friction-knob (gamma) map of the N(A) forward model")
    print(f"L={args.L} seeds={args.seeds} ticks={args.ticks}")
    print(f"FTD-0261 reference: knee={FTD0261_KNEE} p_lo={FTD0261_PLO} p_hi={FTD0261_PHI}")
    print("=" * 72)
    print(f"{'gamma':>6s} {'knee':>5s} {'p_lo':>6s} {'p_hi':>6s} "
          f"{'superknee_ratio':>15s} {'fit_rms':>8s}")

    results = []
    for g in gammas:
        kw = dict(use_gauss=True, gauss_mode="fft", use_coupling=True,
                  drain=fwd.DEFAULT_DRAIN, gamma=g)
        Ns = []
        for A in grid:
            mean, _std, _counts = fwd.run_amplitude(A, args.L, args.seeds,
                                                    ticks=args.ticks, **kw)
            Ns.append(mean)
        fit = fit_broken_power(grid, Ns)
        if fit is None:
            print(f"{g:6.3f}  (fit failed)")
            continue
        knee, p_lo, p_hi, rms = fit
        sk = superknee_ratio(grid, Ns)
        print(f"{g:6.3f} {knee:5.0f} {p_lo:6.2f} {p_hi:6.2f} {sk:15.3f} {rms:8.3f}")
        results.append((g, knee, p_lo, p_hi, sk, rms))

    # find the gamma whose super-knee ratio is closest to 1 (engine-matched)
    if results:
        best = min(results, key=lambda r: abs(math.log(r[4])) if r[4] == r[4] else 9)
        print("-" * 72)
        print(f"super-knee best-matched at gamma={best[0]:.3f} "
              f"(ratio {best[4]:.3f}; knee {best[1]:.0f}, p_hi {best[3]:.2f})")
        print("Note: gamma is an engine-tuning constant (FTD-0269 BOUNDARY), not")
        print("framework-derived; this map calibrates it, it does not derive it.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
