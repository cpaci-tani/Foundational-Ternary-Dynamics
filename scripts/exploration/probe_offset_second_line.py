#!/usr/bin/env python3
"""
probe_offset_second_line.py -- FTD-0281 rung-b (off-center 2p excitation probe)

Deep-floor diagnostic for the off-center atomic-spectroscopy runs. The campaign's
console FFT only prints peaks above 1e-3 of the (1s-dominated) maximum, which can
bury a weak 2p/2s line. This script:

  (1) loads C(t), removes DC, computes the rfft PSD;
  (2) lists EVERY local maximum down to a user floor (default 1e-7 of max),
      converted to omega_phys = (2/dt) sin(Omega/2);
  (3) computes the operator eigenvalues (k=8) from the dumped phi_C so the exact
      1s/2s/2p targets are available per L;
  (4) tags each engine peak with its nearest operator level and the power ratio
      relative to the 1s peak (= the excitation strength of the second line).

Reuses build_L18 / operator_levels / read_phi / read_ct from the canonical
analyzer (imported), so the operator is bit-identical to Path 2 there.

Usage:
  python probe_offset_second_line.py --ct <Ct.csv> --phi <phiC.csv> \
      --omega0 1.5 --dt 0.5 [--k 8] [--floor 1e-7]
"""
import argparse
import math
import sys

import numpy as np

sys.path.insert(0, __import__("os").path.dirname(__file__))
from analyze_atomic_spectroscopy import read_ct, read_phi, operator_levels


def psd_peaks(corr, omega0, dt, floor_frac):
    x = corr - corr.mean()
    Nfft = 1
    while Nfft < len(x):
        Nfft <<= 1
    xp = np.zeros(Nfft)
    xp[:len(x)] = x
    X = np.fft.rfft(xp)
    psd = (np.abs(X) ** 2) / Nfft
    omega_raw = 2.0 * math.pi * np.arange(len(psd)) / Nfft
    omega_phys = (2.0 / dt) * np.sin(0.5 * omega_raw)

    pmax = psd[1:].max() if len(psd) > 1 else 0.0
    floor = floor_frac * pmax
    peaks = []
    for i in range(1, len(psd) - 1):
        if psd[i] > floor and psd[i] >= psd[i - 1] and psd[i] > psd[i + 1]:
            peaks.append((i, float(omega_phys[i]), float(psd[i])))
    return peaks, pmax, Nfft


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ct", required=True)
    ap.add_argument("--phi", required=True)
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--dt", type=float, default=0.5)
    ap.add_argument("--k", type=int, default=8)
    ap.add_argument("--floor", type=float, default=1e-7,
                    help="PSD peak floor as a fraction of the max bin power")
    args = ap.parse_args()

    print("=" * 78)
    print(f"OFFSET 2nd-LINE PROBE   ct={args.ct}")
    print(f"omega0={args.omega0}  dt={args.dt}  k={args.k}  floor={args.floor:g}")
    print("=" * 78)

    # operator targets (k lowest)
    L, phi = read_phi(args.phi)
    a, omega_op, E_op, n_bound = operator_levels(L, phi, args.omega0, k=args.k)
    print(f"[op] L={L}  n_bound={n_bound}")
    for n in range(len(omega_op)):
        tag = " [BOUND]" if a[n] < -1e-12 else ""
        print(f"     op level {n}:  a={a[n]:+.6f}  omega={omega_op[n]:.6f}  "
              f"binding={args.omega0 - omega_op[n]:+.6f}{tag}")

    # engine peaks, deep floor
    corr = read_ct(args.ct)
    peaks, pmax, Nfft = psd_peaks(corr, args.omega0, args.dt, args.floor)
    print(f"[eng] C(t) samples={len(corr)}  Nfft={Nfft}  PSD_max={pmax:.4e}")
    print(f"[eng] ALL peaks above floor (sorted by omega):")
    # 1s power = the strongest BOUND peak (the deepest, highest-power line)
    bound = [p for p in peaks if 0.0 < p[1] < args.omega0]
    p1s = max(bound, key=lambda p: p[2]) if bound else None
    print(f"       {'omega':>10} {'binding':>10} {'power':>13} {'pow/1s':>11}  nearest-op")
    for (b, om, pw) in sorted(peaks, key=lambda p: p[1]):
        if om >= args.omega0 + 1e-9:
            continue  # ignore continuum/above-omega0 noise for clarity
        ratio = (pw / p1s[2]) if p1s else float("nan")
        # nearest operator level
        j = int(np.argmin(np.abs(omega_op - om)))
        relerr = abs(om - omega_op[j]) / omega_op[j] if omega_op[j] > 0 else float("inf")
        binding = args.omega0 - om
        is1s = "  <-1s" if (p1s and abs(om - p1s[1]) < 1e-9) else ""
        print(f"       {om:>10.6f} {binding:>+10.6f} {pw:>13.4e} {ratio:>11.3e}"
              f"  op[{j}] om={omega_op[j]:.6f} relerr={relerr*100:.2f}%{is1s}")
    print("=" * 78)
    return 0


if __name__ == "__main__":
    sys.exit(main())
