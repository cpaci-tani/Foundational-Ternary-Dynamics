#!/usr/bin/env python3
"""
compare_spectroscopy_backends.py -- CPU vs GPU parity gate for the atomic
spectroscopy campaign (FTD-0281 rung-(a) GPU port, 2026-06-20).

Loads the C(t) autocorrelation series and the static phi_C field produced by
campaign_atomic_spectroscopy --backend {cpu,gpu} for the SAME parameters, and
reports:
  - max / RMS relative difference of the C(t) time series
  - max / RMS absolute & relative difference of the phi_C field
  - the FFT ground peak (omega_phys) of each backend and their relative diff

GATE (task spec): <0.1% relative on the C(t) series, <0.05% on the FFT ground
peak. The phi_C diff is reported as the root cause if the series diverges
(GPU uses a float-precision FFT Poisson solve; CPU uses double-precision SOR).

Usage:
  python compare_spectroscopy_backends.py \
      --cpu-ct  cpu_L32/atomic_spectroscopy_Ct_L32.csv \
      --cpu-phi cpu_L32/atomic_spectroscopy_phiC_L32.csv \
      --gpu-ct  gpu_L32/atomic_spectroscopy_Ct_L32.csv \
      --gpu-phi gpu_L32/atomic_spectroscopy_phiC_L32.csv \
      --omega0 1.5 --dt 0.5
"""
import argparse
import csv
import math
import sys

import numpy as np


def read_ct(path):
    corr = []
    with open(path, newline="") as fh:
        r = csv.DictReader(fh)
        for row in r:
            corr.append(float(row["corr"]))
    return np.array(corr, dtype=float)


def read_phi(path):
    coords = {}
    Lmax = 0
    with open(path, newline="") as fh:
        r = csv.DictReader(fh)
        for row in r:
            x, y, z = int(row["x"]), int(row["y"]), int(row["z"])
            coords[(x, y, z)] = float(row["phi"])
            Lmax = max(Lmax, x + 1, y + 1, z + 1)
    L = Lmax
    phi = np.zeros(L * L * L)
    for (x, y, z), v in coords.items():
        phi[(x * L + y) * L + z] = v
    return L, phi


def fft_ground(corr, omega0, dt):
    x = corr - corr.mean()
    Nfft = 1
    while Nfft < len(x):
        Nfft <<= 1
    xp = np.zeros(Nfft)
    xp[: len(x)] = x
    X = np.fft.rfft(xp)
    psd = (np.abs(X) ** 2) / Nfft
    omega_raw = 2.0 * math.pi * np.arange(len(psd)) / Nfft
    omega_phys = (2.0 / dt) * np.sin(0.5 * omega_raw)
    pmax = psd[1:].max() if len(psd) > 1 else 0.0
    floor = 1e-3 * pmax
    peaks = []
    for i in range(1, len(psd) - 1):
        if psd[i] > floor and psd[i] >= psd[i - 1] and psd[i] > psd[i + 1]:
            peaks.append((float(omega_phys[i]), float(psd[i])))
    bound = [p for p in peaks if 0.0 < p[0] < omega0]
    if bound:
        ground = min(bound, key=lambda p: p[0])[0]
    elif peaks:
        ground = max(peaks, key=lambda p: p[1])[0]
    else:
        ground = 0.0
    return ground, peaks


def rel_stats(a, b):
    """max/RMS relative difference of two equal-length arrays, normalised by
    the RMS magnitude of `a` (robust against zero-crossings in C(t))."""
    d = a - b
    scale = math.sqrt(np.mean(a ** 2)) or 1.0
    max_abs = float(np.max(np.abs(d)))
    rms_abs = float(math.sqrt(np.mean(d ** 2)))
    return max_abs, rms_abs, max_abs / scale, rms_abs / scale, scale


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cpu-ct", required=True)
    ap.add_argument("--cpu-phi", required=True)
    ap.add_argument("--gpu-ct", required=True)
    ap.add_argument("--gpu-phi", required=True)
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--dt", type=float, default=0.5)
    ap.add_argument("--ct-tol", type=float, default=1e-3, help="series rel tol")
    ap.add_argument("--peak-tol", type=float, default=5e-4, help="ground-peak rel tol")
    args = ap.parse_args()

    print("=" * 72)
    print("CPU vs GPU parity — atomic spectroscopy (FTD-0281 GPU port)")
    print(f"omega0={args.omega0}  dt={args.dt}")
    print("=" * 72)

    cpu_ct = read_ct(args.cpu_ct)
    gpu_ct = read_ct(args.gpu_ct)
    n = min(len(cpu_ct), len(gpu_ct))
    if len(cpu_ct) != len(gpu_ct):
        print(f"[warn] series length mismatch: cpu={len(cpu_ct)} gpu={len(gpu_ct)} "
              f"-> comparing first {n}")
    cpu_ct, gpu_ct = cpu_ct[:n], gpu_ct[:n]

    # --- phi_C field parity ---
    Lc, cpu_phi = read_phi(args.cpu_phi)
    Lg, gpu_phi = read_phi(args.gpu_phi)
    assert Lc == Lg, f"L mismatch cpu={Lc} gpu={Lg}"
    pmax_abs, prms_abs, pmax_rel, prms_rel, pscale = rel_stats(cpu_phi, gpu_phi)
    print(f"[phi_C] L={Lc}  RMS|phi_cpu|={pscale:.6e}")
    print(f"[phi_C] max|d|={pmax_abs:.3e}  rms|d|={prms_abs:.3e}  "
          f"max-rel={pmax_rel*100:.4f}%  rms-rel={prms_rel*100:.4f}%")
    c = Lc // 2
    ci = (c * Lc + c) * Lc + c
    print(f"[phi_C] center: cpu={cpu_phi[ci]:+.8e}  gpu={gpu_phi[ci]:+.8e}  "
          f"rel={abs(cpu_phi[ci]-gpu_phi[ci])/abs(cpu_phi[ci])*100:.4f}%")

    # --- C(t) series parity ---
    cmax_abs, crms_abs, cmax_rel, crms_rel, cscale = rel_stats(cpu_ct, gpu_ct)
    print("-" * 72)
    print(f"[C(t)]  samples={n}  RMS|C_cpu|={cscale:.6e}")
    print(f"[C(t)]  max|d|={cmax_abs:.3e}  rms|d|={crms_abs:.3e}  "
          f"max-rel={cmax_rel*100:.4f}%  rms-rel={crms_rel*100:.4f}%")

    # --- FFT ground peaks ---
    cpu_g, cpu_pk = fft_ground(cpu_ct, args.omega0, args.dt)
    gpu_g, gpu_pk = fft_ground(gpu_ct, args.omega0, args.dt)
    pk_rel = abs(cpu_g - gpu_g) / cpu_g if cpu_g > 0 else float("inf")
    print("-" * 72)
    print(f"[FFT]   CPU ground omega_1s = {cpu_g:.6f}")
    print(f"[FFT]   GPU ground omega_1s = {gpu_g:.6f}")
    print(f"[FFT]   |cpu-gpu|/cpu       = {pk_rel*100:.5f}%  "
          f"(tol {args.peak_tol*100:.3f}%)")

    # show the top peaks side by side
    print(f"[FFT]   CPU peaks: " +
          " ".join(f"{om:.4f}" for om, _ in sorted(cpu_pk)[:6]))
    print(f"[FFT]   GPU peaks: " +
          " ".join(f"{om:.4f}" for om, _ in sorted(gpu_pk)[:6]))

    print("=" * 72)
    series_ok = crms_rel <= args.ct_tol
    peak_ok = pk_rel <= args.peak_tol
    print(f"  GATE series (rms-rel <= {args.ct_tol*100:.3f}%): "
          f"{'PASS' if series_ok else 'FAIL'} ({crms_rel*100:.4f}%)")
    print(f"  GATE ground peak (rel <= {args.peak_tol*100:.3f}%): "
          f"{'PASS' if peak_ok else 'FAIL'} ({pk_rel*100:.5f}%)")
    print("=" * 72)
    return 0 if (series_ok and peak_ok) else 1


if __name__ == "__main__":
    sys.exit(main())
