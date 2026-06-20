#!/usr/bin/env python3
"""
analyze_excited_spectroscopy.py -- FTD-0281 rung (b): hydrogen EXCITED states
on the engine FFT vs the lattice operator, at large L on the GPU.

For one (Ct, phiC) pair at a given L it:
  Path 2 (operator): builds A = -c^2 L18 + 2*omega0*V (V=-phi_C) from the
    engine-dumped phi_C, solves eigsh(k) for the k lowest eigenvalues a_n, and
    reports the predicted level frequencies omega_n = sqrt(omega0^2 + a_n) and
    n_bound = #{a_n < 0}.
  Path 1 (engine):   FFTs the C(t) shell-autocorrelation, lists peaks on the
    physical axis omega_phys = (2/dt) sin(Omega_raw/2), and flags those below
    omega0 (BOUND). The ground is the lowest bound peak; any further peak that
    is below omega0 AND above the ground is a candidate EXCITED line.

It then matches each engine bound peak to the nearest operator omega_n and
prints the residual. The verdict (does an excited line resolve, does n_bound
grow with L) is adjudicated by the user from the printed table; this script
reports raw numbers only.

[EPISTEMIC: [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]. omega0 + the scalar
 coupling are [IMPOSED] (FTD-0271/0281). engine<->operator consistency, NOT
 'FTD derives hydrogen'; FTD-0270 / FC-1 ceiling stands.]

Usage:
  python analyze_excited_spectroscopy.py --L 64 \
      --ct  gpu_L64/atomic_spectroscopy_Ct_L64.csv \
      --phi gpu_L64/atomic_spectroscopy_phiC_L64.csv \
      --omega0 1.5 --dt 0.5 --k 6
"""
import argparse
import csv
import math
import sys
import time

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla

C2 = 1.0 / 3.0

FACE = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
EDGE = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_L18(L):
    """Sparse 18-pt periodic O_h Laplacian on L^3 (X-major index)."""
    N = L * L * L
    idx = lambda x, y, z: (x * L + y) * L + z
    # vectorised COO assembly (the python triple loop is too slow at L=256)
    coords = np.arange(N)
    xs = coords // (L * L)
    ys = (coords // L) % L
    zs = coords % L
    rows = [coords]
    cols = [coords]
    vals = [np.full(N, -4.0)]
    for off, w in ((FACE, 1.0 / 3.0), (EDGE, 1.0 / 6.0)):
        for dx, dy, dz in off:
            nx = (xs + dx) % L
            ny = (ys + dy) % L
            nz = (zs + dz) % L
            rows.append(coords)
            cols.append((nx * L + ny) * L + nz)
            vals.append(np.full(N, w))
    rows = np.concatenate(rows)
    cols = np.concatenate(cols)
    vals = np.concatenate(vals)
    return sp.csr_matrix((vals, (rows, cols)), shape=(N, N))


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


def operator_levels(L, phi, omega0, k, solver="auto"):
    t0 = time.time()
    A_lap = build_L18(L)
    V = -phi
    A = (-C2) * A_lap + sp.diags(2.0 * omega0 * V)
    A = A.tocsr()
    N = A.shape[0]

    # Solver selection. eigsh(which='SA') is robust but slow at large N; LOBPCG
    # with a Jacobi (diagonal) preconditioner is memory-light (no factorisation,
    # the 11 GB shift-invert LU fill-in was the bottleneck) and converges the few
    # lowest modes fast. Use LOBPCG for large N, SA for small.
    use_lobpcg = (solver == "lobpcg")
    if use_lobpcg:
        diag = A.diagonal().copy()
        diag[np.abs(diag) < 1e-12] = 1e-12
        Minv = sp.diags(1.0 / diag)
        rng = np.random.default_rng(0x0281)
        Xg = rng.standard_normal((N, k))
        # seed one vector with a centred bump so the ground is captured
        Xg[:, 0] = np.exp(-((np.arange(N) - N // 2) ** 2) / (2.0 * (N / 20.0) ** 2))
        vals, _ = spla.lobpcg(A, Xg, M=Minv, largest=False, tol=1e-7,
                              maxiter=2000)
        vals = np.asarray(vals, dtype=float)
    else:
        vals = spla.eigsh(A, k=k, which="SA", return_eigenvectors=False)
    a = np.sort(vals)
    omega2 = omega0 ** 2 + a
    tach = int(np.sum(omega2 <= 0))
    omega = np.sqrt(np.clip(omega2, 0.0, None))
    n_bound = int(np.sum(a < -1e-12))
    return a, omega, n_bound, tach, time.time() - t0


def engine_peaks(corr, omega0, dt, rel_floor=1e-3):
    x = corr - np.nanmean(corr)
    x = np.nan_to_num(x, nan=0.0, posinf=0.0, neginf=0.0)
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
    floor = rel_floor * pmax
    peaks = []
    for i in range(1, len(psd) - 1):
        if psd[i] > floor and psd[i] >= psd[i - 1] and psd[i] > psd[i + 1]:
            peaks.append((float(omega_phys[i]), float(psd[i])))
    return peaks, Nfft, (2.0 / dt) * math.sin(0.5 * 2.0 * math.pi / Nfft)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--L", type=int, required=True)
    ap.add_argument("--ct", required=True)
    ap.add_argument("--phi", required=True)
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--dt", type=float, default=0.5)
    ap.add_argument("--k", type=int, default=6)
    args = ap.parse_args()

    print("=" * 74)
    print(f"FTD-0281 rung(b) EXCITED -- L={args.L}  omega0={args.omega0}  dt={args.dt}  k={args.k}")
    print("=" * 74)

    # --- operator path ---
    L, phi = read_phi(args.phi)
    assert L == args.L, f"phi L={L} != --L {args.L}"
    c = L // 2
    phic = phi[(c * L + c) * L + c]
    print(f"[phi] L={L}  phi_C(center)={phic:+.6e}  min={phi.min():+.6e}  "
          f"max={phi.max():+.6e}")

    a, omega_op, n_bound, tach, dt_op = operator_levels(L, phi, args.omega0, args.k)
    print(f"[op]  eigsh k={args.k} in {dt_op:.1f}s   n_bound={n_bound}"
          + (f"  (tachyonic={tach})" if tach else ""))
    print(f"[op]  a_n      = " + " ".join(f"{x:+.6f}" for x in a))
    print(f"[op]  omega_n  = " + " ".join(f"{x:.6f}" for x in omega_op))
    print(f"[op]  binding  = " + " ".join(f"{args.omega0 - x:+.6f}" for x in omega_op)
          + "   (omega0 - omega_n; >0 => bound)")

    # --- engine path ---
    corr = read_ct(args.ct)
    n_finite = int(np.sum(np.isfinite(corr)))
    blew = n_finite < len(corr)
    peaks, Nfft, dwphys = engine_peaks(corr, args.omega0, args.dt)
    print("-" * 74)
    print(f"[eng] C(t) samples={len(corr)}  finite={n_finite}"
          + ("  *** SERIES BLEW UP (non-finite) ***" if blew else ""))
    print(f"[eng] Nfft={Nfft}  FFT resolution near omega0 ~ {dwphys*1.86:.2e} rad/tick")
    bound = sorted([p for p in peaks if 0.0 < p[0] < args.omega0])
    above = sorted([p for p in peaks if p[0] >= args.omega0])
    print(f"[eng] BOUND peaks (omega<omega0), low->high:")
    for om, pw in bound[:12]:
        print(f"        omega={om:.6f}  binding={args.omega0-om:+.6f}  power={pw:.3e}")
    if not bound:
        print("        (none below omega0)")
    print(f"[eng] {len(above)} peak(s) at/above omega0 (continuum/vacuum); "
          f"strongest: "
          + (f"{max(above, key=lambda p: p[1])[0]:.4f}" if above else "-"))

    # --- match engine bound peaks to operator levels ---
    print("-" * 74)
    print("[match] engine bound peak -> nearest operator omega_n:")
    op_bound = [w for w, aa in zip(omega_op, a) if aa < -1e-12]
    if not op_bound:
        op_bound = list(omega_op[:1])
    for om, pw in bound[:6]:
        j = int(np.argmin([abs(om - w) for w in op_bound]))
        wn = op_bound[j]
        rel = abs(om - wn) / wn * 100 if wn > 0 else float("inf")
        print(f"        eng {om:.6f}  <->  op[n={j}] {wn:.6f}   "
              f"rel={rel:.3f}%")

    n_eng_bound = len(bound)
    print("-" * 74)
    print(f"[summary L={L}] op n_bound={n_bound}   eng bound-peaks={n_eng_bound}   "
          f"eng_ground={(bound[0][0] if bound else 0.0):.6f}   "
          f"op_ground={omega_op[0]:.6f}")
    print("=" * 74)
    return 0


if __name__ == "__main__":
    sys.exit(main())
