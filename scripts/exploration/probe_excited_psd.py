#!/usr/bin/env python3
"""
probe_excited_psd.py -- deep PSD dive: is there a sub-floor EXCITED line below
omega0 and above the 1s ground in the engine C(t)?

Lists ALL local maxima of the PSD in the window [omega_lo, omega0], with their
power RELATIVE to the ground peak, so a weak 2s line (excited by a centered
symmetric packet, much weaker than the 1s) is visible even at 1e-4..1e-6 of the
ground. Reports the ground peak, the next-strongest peak below omega0, and the
gap between them on the physical axis.

Usage:
  python probe_excited_psd.py --ct gpu_L128/...Ct_L128.csv --omega0 1.5 --dt 0.5
"""
import argparse
import csv
import math
import numpy as np


def read_ct(path):
    corr = []
    with open(path, newline="") as fh:
        r = csv.DictReader(fh)
        for row in r:
            try:
                corr.append(float(row["corr"]))
            except ValueError:
                corr.append(float("nan"))
    return np.array(corr, dtype=float)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ct", required=True)
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--dt", type=float, default=0.5)
    ap.add_argument("--rel-floor", type=float, default=1e-6,
                    help="report peaks down to this fraction of the ground power")
    args = ap.parse_args()

    corr = read_ct(args.ct)
    n_finite = int(np.sum(np.isfinite(corr)))
    x = np.nan_to_num(corr - np.nanmean(corr), nan=0.0, posinf=0.0, neginf=0.0)
    Nfft = 1
    while Nfft < len(x):
        Nfft <<= 1
    xp = np.zeros(Nfft)
    xp[: len(x)] = x
    X = np.fft.rfft(xp)
    psd = (np.abs(X) ** 2) / Nfft
    omega_raw = 2.0 * math.pi * np.arange(len(psd)) / Nfft
    omega_phys = (2.0 / args.dt) * np.sin(0.5 * omega_raw)

    # all local maxima
    peaks = []
    for i in range(1, len(psd) - 1):
        if psd[i] >= psd[i - 1] and psd[i] > psd[i + 1]:
            peaks.append((float(omega_phys[i]), float(psd[i]), i))
    if not peaks:
        print("no peaks")
        return

    gpow = max(p[1] for p in peaks)
    dwphys_bin = (2.0 / args.dt) * math.sin(0.5 * 2.0 * math.pi / Nfft)

    print("=" * 70)
    print(f"deep PSD: {args.ct}")
    print(f"samples={len(corr)} finite={n_finite}  Nfft={Nfft}  "
          f"bin~{dwphys_bin*1.86:.2e} rad/tick near omega0")
    print("=" * 70)

    below = sorted([p for p in peaks if 0.0 < p[0] < args.omega0])
    # ground = strongest below omega0
    if below:
        ground = max(below, key=lambda p: p[1])
    else:
        ground = max(peaks, key=lambda p: p[1])
    print(f"ground (strongest below omega0): omega={ground[0]:.6f}  "
          f"binding={args.omega0-ground[0]:+.6f}  power={ground[1]:.3e}")
    print("-" * 70)
    print(f"ALL peaks below omega0 with power >= {args.rel_floor:.0e} * ground, "
          f"strongest first:")
    shown = sorted([p for p in below if p[1] >= args.rel_floor * gpow],
                   key=lambda p: -p[1])
    for om, pw, b in shown[:25]:
        rel = pw / gpow
        tag = "  <-- GROUND" if (om, pw, b) == ground else (
            "  <-- candidate EXCITED" if om > ground[0] else "")
        print(f"  omega={om:.6f}  binding={args.omega0-om:+.6f}  "
              f"rel_power={rel:.3e}{tag}")

    # the strongest peak that is ABOVE the ground but still BELOW omega0
    excited = [p for p in below if p[0] > ground[0] + 2 * dwphys_bin]
    print("-" * 70)
    if excited:
        ex = max(excited, key=lambda p: p[1])
        print(f"strongest candidate EXCITED (above ground, below omega0): "
              f"omega={ex[0]:.6f}  binding={args.omega0-ex[0]:+.6f}  "
              f"rel_power={ex[1]/gpow:.3e}  gap_from_ground={ex[0]-ground[0]:+.6f}")
    else:
        print("NO peak resolved between the ground and omega0 (no excited line "
              "surfaced in the engine FFT).")
    print("=" * 70)


if __name__ == "__main__":
    main()
