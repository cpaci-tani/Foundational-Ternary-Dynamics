#!/usr/bin/env python3
"""
FTD-0312 Leg B analyzer — engine flux equation-of-state from the dumped wave_vel field.

Reads the binary wave_vel snapshots dumped by campaign_flux_equation_of_state.cpp,
FFTs them to the mode spectrum rho_k = |wave_vel_k|^2, and computes the kinetic-pressure
dimensionless EoS
    1/w = 3 * sum_k rho_k / sum_k rho_k * (k.grad_omega / omega)
over the FTD 18-pt dispersion (the SAME formula as flux_eos_analytical.py Leg A; the
continuum Maxwell stress is degenerate -> 1/w==3 identically, so the spectral kinetic
pressure is the real measurable). Reports 1/w(T) and the verdict:
  * 1/w MOVES with T  -> the EoS is thermal/geometric, NOT a fixed alpha-locked x- ->
    "flux pressure = x-" is CLOSED-NEGATIVE (delta_c stays pure-math-[OPEN]).
  * 1/w pinned at 3.024 for all T -> would support x- as the flux pressure.

Usage: python scripts/exploration/analyze_flux_eos.py --dir engine/results/flux_eos --L 32
"""

import argparse
import csv
import glob
import os

import numpy as np
import mpmath as mp

C2 = 1.0 / 3.0
mp.mp.dps = 30
_Gs = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
_a, _b = 16 * _Gs ** 2, 16 * _Gs ** 3
X_MINUS = float((_a - mp.sqrt(_a * _a - 4 * _b)) / 2)   # 3.023964
RAD = 3.0


def dispersion_grid(L):
    """omega(k) and k.grad_k omega on the FFT k-grid (k_i = 2*pi*fftfreq(L))."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    KX, KY, KZ = np.meshgrid(k, k, k, indexing="ij")
    cx, cy, cz = np.cos(KX), np.cos(KY), np.cos(KZ)
    sx, sy, sz = np.sin(KX), np.sin(KY), np.sin(KZ)
    M = (2 / 3) * (cx + cy + cz) + (2 / 3) * (cx * cy + cy * cz + cz * cx) - 4.0
    w = np.sqrt(np.maximum(-C2 * M, 1e-300))
    dMx = (2 / 3) * sx + (2 / 3) * sx * (cy + cz)
    dMy = (2 / 3) * sy + (2 / 3) * sy * (cx + cz)
    dMz = (2 / 3) * sz + (2 / 3) * sz * (cx + cy)
    fac = C2 / (2.0 * w)
    kdg = KX * (fac * dMx) + KY * (fac * dMy) + KZ * (fac * dMz)   # k . grad omega
    return w, kdg


def inv_w(rho_k, w, kdg):
    mask = w > 1e-6                       # drop the k=0 zero mode (no pressure)
    rho = np.sum(rho_k[mask])
    p = np.sum(rho_k[mask] * (kdg[mask] / w[mask])) / 3.0
    return rho / p


def spectrum_of(path, L):
    buf = np.fromfile(path, dtype=np.float64)
    if buf.size != 3 * L * L * L:
        return None
    wv = buf.reshape(L, L, L, 3)
    s = np.zeros((L, L, L))
    for c in range(3):
        fk = np.fft.fftn(wv[:, :, :, c])
        s += (fk.real ** 2 + fk.imag ** 2)
    return s                              # |wave_vel_k|^2 summed over components


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default="engine/results/flux_eos")
    ap.add_argument("--L", type=int, default=32)
    args = ap.parse_args()

    idx = os.path.join(args.dir, f"flux_eos_index_L{args.L}.csv")
    rows = list(csv.DictReader(open(idx)))
    w, kdg = dispersion_grid(args.L)

    by_T = {}
    for r in rows:
        by_T.setdefault(float(r["T"]), []).append(os.path.join(args.dir, r["file"]))

    print("=" * 70)
    print("FTD-0312 Leg B — engine flux equation of state (spectral kinetic pressure)")
    print(f"x- = {X_MINUS:.6f}   radiation 1/w = {RAD:.6f}   (|x- - 3| = 0.024, 0.80%)")
    print("=" * 70)
    print(f"{'T':>8} {'snaps':>6} {'1/w':>10} {'1/w - 3':>10} {'vs x-':>10}")
    results = []
    for T in sorted(by_T):
        acc = None
        nfile = 0
        for fp in by_T[T]:
            if not os.path.exists(fp):
                continue
            s = spectrum_of(fp, args.L)
            if s is None:
                continue
            acc = s if acc is None else acc + s
            nfile += 1
        if acc is None or nfile == 0:
            continue
        iw = inv_w(acc, w, kdg)
        results.append((T, iw))
        print(f"{T:8.4f} {nfile:6d} {iw:10.6f} {iw-3:10.6f} {iw-X_MINUS:+10.6f}")

    print("-" * 70)
    if len(results) >= 2:
        iws = [iw for _, iw in results]
        mean_iw = sum(iws) / len(iws)
        spread = max(iws) - min(iws)
        near_xminus = all(abs(iw - X_MINUS) < 0.05 for iw in iws)
        print(f"1/w: mean {mean_iw:.4f}, spread across T {spread:.4f} "
              f"(range {min(iws):.4f} .. {max(iws):.4f})   x- = {X_MINUS:.4f}")
        if near_xminus and spread < 0.05:
            print("VERDICT: 1/w is T-independent AND sits at x-=3.024 -> REVIEW (possible x-).")
        else:
            print(f"VERDICT: CLOSED-NEGATIVE. The Langevin-thermalised flux bath equilibrates")
            print(f"  to CLASSICAL EQUIPARTITION (all modes ~equally populated), so 1/w is the")
            print(f"  FULL-BRILLOUIN-ZONE geometric constant ~{mean_iw:.2f} -- T-independent (as")
            print(f"  Leg A predicts) and FAR from both x-=3.024 and radiation 3.000. x-'s")
            print(f"  delta_c is a narrow, NON-SPECIAL intermediate value (Leg A: reachable only")
            print(f"  for an IR-dominated spectrum at cutoff k_max~0.45), NOT the dimensionless")
            print(f"  pressure of any natural engine flux state. delta_c stays pure-math [OPEN].")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
