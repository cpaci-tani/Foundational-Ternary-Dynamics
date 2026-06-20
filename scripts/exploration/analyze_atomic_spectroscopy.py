#!/usr/bin/env python3
"""
analyze_atomic_spectroscopy.py -- FTD-0281 rung 1 (engine <-> operator)

Validates the ENGINE hydrogen-1s spectroscopy campaign
(campaign_atomic_spectroscopy.cpp) against the lattice operator, using the
ENGINE'S OWN dumped Coulomb potential phi_C as the bridge. This is a two-path
consistency check on a SINGLE physical potential:

  Path 1 (engine leapfrog):  the db_clock_coulomb wave equation
      J'' = c² L18 J - (ω₀² + 2 ω₀ V) J,   V = -phi_C
  is integrated by the engine; the campaign records the shell-autocorrelation
  C(t) = Σ_probe J(0)·J(t) and FFTs it. The bound 1s envelope rings at the
  smallest frequency below ω₀.

  Path 2 (operator):  we read the engine-dumped phi_C field, build the SAME
  second-order operator
      A = -c² L18 + 2 ω₀ V,   V = -phi_C    (c² = 1/3)
  as a sparse symmetric matrix, and solve eigsh(k) for the lowest eigenvalues
  a_n. Levels are ω_n = √(ω₀² + a_n); bound states have a_n < 0 (ω_n < ω₀).

  GATE: the engine FFT ground peak ω_0 must (a) lie below ω₀ (bound) and
  (b) match the operator's lowest ω_n within a few percent.

The L18 stencil and the V=-phi_C convention are the engine's exact
phase_read.cpp:195-197 convention (omega_eff² = omega0² - 2*omega0*phi_C, i.e.
V = -phi_C). c² = 1/3 (C_WAVE = 1/√3). The L18 + eigsh machinery is reused from
derive_hydrogen_lattice_spectrum.py (which validates L18's symbol to machine
precision, gate G-1).

[EPISTEMIC: [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]. The clock ω₀ and the
 scalar-potential coupling are [IMPOSED] (FTD-0271/0281). This validates
 engine↔operator consistency, NOT 'FTD derives hydrogen' (FTD-0270 / FC-1
 ceiling + linear-dispersion caveat stand).]

Usage:
  python analyze_atomic_spectroscopy.py \
      --ct  <atomic_spectroscopy_Ct_L32.csv> \
      --phi <atomic_spectroscopy_phiC_L32.csv> \
      --omega0 1.5 [--k 3] [--tol 0.05]
"""

import argparse
import csv
import math
import sys

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla

C2 = 1.0 / 3.0           # engine C_WAVE² (CFL on cubic lattice)

FACE = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
EDGE = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_L18(L, periodic=True):
    """Sparse 18-pt O_h Laplacian on L^3 (engine phase_read stencil).

    X-major index: idx(x,y,z) = (x*L + y)*L + z  (matches Lattice::index)."""
    N = L * L * L
    idx = lambda x, y, z: (x * L + y) * L + z
    rows, cols, vals = [], [], []
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx(x, y, z)
                rows.append(i); cols.append(i); vals.append(-4.0)
                for off, w in ((FACE, 1.0 / 3.0), (EDGE, 1.0 / 6.0)):
                    for dx, dy, dz in off:
                        nx, ny, nz = x + dx, y + dy, z + dz
                        if periodic:
                            nx %= L; ny %= L; nz %= L
                        elif not (0 <= nx < L and 0 <= ny < L and 0 <= nz < L):
                            continue
                        rows.append(i); cols.append(idx(nx, ny, nz)); vals.append(w)
    return sp.csr_matrix((vals, (rows, cols)), shape=(N, N))


def symbol_M(kx, ky, kz):
    cx, cy, cz = math.cos(kx), math.cos(ky), math.cos(kz)
    return (2.0 / 3.0) * (cx + cy + cz) + (2.0 / 3.0) * (cx * cy + cy * cz + cz * cx) - 4.0


def validate_symbol(L=8):
    """G-1: periodic L18 eigenvalues match the closed-form symbol (machine eps)."""
    A = build_L18(L, periodic=True)
    idx = lambda x, y, z: (x * L + y) * L + z
    worst = 0.0
    v = np.zeros(L * L * L, dtype=complex)
    for (nx, ny, nz) in [(0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1), (2, 1, 0)]:
        kx, ky, kz = 2 * math.pi * nx / L, 2 * math.pi * ny / L, 2 * math.pi * nz / L
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    v[idx(x, y, z)] = np.exp(1j * (kx * x + ky * y + kz * z))
        eig = (np.vdot(v, A @ v) / np.vdot(v, v)).real
        worst = max(worst, abs(eig - symbol_M(kx, ky, kz)))
    return worst


def read_phi(phi_csv):
    """Read the engine-dumped phi_C field (x,y,z,phi). Returns (L, phi_flat)
    in X-major order matching Lattice::index."""
    coords = {}
    Lmax = 0
    with open(phi_csv, newline="") as fh:
        r = csv.DictReader(fh)
        for row in r:
            x, y, z = int(row["x"]), int(row["y"]), int(row["z"])
            coords[(x, y, z)] = float(row["phi"])
            Lmax = max(Lmax, x + 1, y + 1, z + 1)
    L = Lmax
    phi = np.zeros(L * L * L)
    for (x, y, z), v in coords.items():
        phi[(x * L + y) * L + z] = v
    if len(coords) != L * L * L:
        raise ValueError(f"phi field has {len(coords)} entries, expected {L**3}")
    return L, phi


def read_ct(ct_csv):
    """Read the C(t) autocorrelation series (tick,corr). Returns float array."""
    ticks, corr = [], []
    with open(ct_csv, newline="") as fh:
        r = csv.DictReader(fh)
        for row in r:
            ticks.append(int(row["tick"]))
            corr.append(float(row["corr"]))
    return np.array(corr, dtype=float)


def operator_levels(L, phi, omega0, k=3):
    """Build A = -c² L18 + 2 ω₀ V with V = -phi (engine convention), solve the
    k lowest eigenvalues. Returns (a_n, omega_n, E_n, n_bound)."""
    A_lap = build_L18(L, periodic=True)
    V = -phi                                   # phase_read.cpp:195-197 convention
    Vdiag = sp.diags(2.0 * omega0 * V)
    A = (-C2) * A_lap + Vdiag
    vals = spla.eigsh(A, k=k, which="SA", return_eigenvectors=False)
    a = np.sort(vals)
    omega2 = omega0 ** 2 + a
    if np.any(omega2 <= 0):
        # tachyonic eigenvalue (well too deep for this omega0) -- report which
        bad = np.where(omega2 <= 0)[0]
        print(f"  [warn] {len(bad)} tachyonic eigenvalue(s) (omega_eff²<=0): a={a[bad]}")
    omega = np.sqrt(np.clip(omega2, 0.0, None))
    E = omega - omega0
    n_bound = int(np.sum(a < -1e-12))
    return a, omega, E, n_bound


def engine_fft_ground(corr, omega0, dt):
    """FFT the (mean-subtracted) C(t) series, return (omega_ground, peak_list).

    The symplectic-leapfrog integrator at substep dt discretizes a cos(Ω·n) mode
    whose PHYSICAL frequency satisfies 2·sin(Ω/2) = ω_phys·dt, i.e.
        ω_phys = (2/dt)·sin(Ω/2),   Ω = 2π·bin/Nfft  [rad/step].
    We convert each peak to ω_phys (the axis the operator predicts).

    peak_list = list of (omega_phys, power) for local maxima above 1e-3 of max.
    omega_ground = lowest-frequency peak below omega0 (the bound 1s envelope),
    falling back to the strongest peak if none is below omega0."""
    x = corr - corr.mean()
    Nfft = 1
    while Nfft < len(x):
        Nfft <<= 1
    xp = np.zeros(Nfft)
    xp[:len(x)] = x
    X = np.fft.rfft(xp)
    psd = (np.abs(X) ** 2) / Nfft
    omega_raw = 2.0 * math.pi * np.arange(len(psd)) / Nfft       # rad/step
    omega_phys = (2.0 / dt) * np.sin(0.5 * omega_raw)            # rad/tick

    pmax = psd[1:].max() if len(psd) > 1 else 0.0
    floor = 1e-3 * pmax
    peaks = []
    for i in range(1, len(psd) - 1):
        if psd[i] > floor and psd[i] >= psd[i - 1] and psd[i] > psd[i + 1]:
            peaks.append((float(omega_phys[i]), float(psd[i])))

    bound_peaks = [p for p in peaks if 0.0 < p[0] < omega0]
    if bound_peaks:
        # lowest-frequency bound peak = ground envelope
        omega_ground = min(bound_peaks, key=lambda p: p[0])[0]
    elif peaks:
        omega_ground = max(peaks, key=lambda p: p[1])[0]    # strongest
    else:
        omega_ground = 0.0
    return omega_ground, peaks


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ct", required=True, help="C(t) autocorrelation CSV")
    ap.add_argument("--phi", required=True, help="static phi_C field CSV")
    ap.add_argument("--omega0", type=float, default=1.5)
    ap.add_argument("--dt", type=float, default=0.5,
                    help="symplectic-leapfrog substep used by the campaign")
    ap.add_argument("--k", type=int, default=3, help="num operator eigenvalues")
    ap.add_argument("--tol", type=float, default=0.05,
                    help="relative-error match threshold (default 5%)")
    args = ap.parse_args()

    print("=" * 72)
    print("FTD-0281 rung 1 -- analyze_atomic_spectroscopy (ENGINE <-> OPERATOR)")
    print(f"omega0 = {args.omega0}   dt = {args.dt}   k = {args.k}")
    print("=" * 72)

    # G-1: L18 operator correctness (machine precision)
    werr = validate_symbol(8)
    print(f"[G-1] |eig(L18)-M(k)| = {werr:.2e}  "
          f"-> {'PASS' if werr < 1e-10 else 'FAIL'}")

    # --- operator path (Path 2): build A from the engine-dumped phi_C ---
    L, phi = read_phi(args.phi)
    c = L // 2
    phi_center = phi[(c * L + c) * L + c]
    print(f"[phi]  L={L}  phi_C(center)={phi_center:+.6e}  "
          f"min={phi.min():+.6e}  max={phi.max():+.6e}  mean={phi.mean():+.3e}")

    a, omega_op, E_op, n_bound = operator_levels(L, phi, args.omega0, k=args.k)
    print(f"[op]   lowest {args.k} eigenvalues a_n = "
          + " ".join(f"{x:+.6f}" for x in a))
    print(f"[op]   operator omega_n           = "
          + " ".join(f"{x:.6f}" for x in omega_op)
          + f"   (n_bound={n_bound})")
    print(f"[op]   envelope E_n = omega_n-omega0 = "
          + " ".join(f"{x:+.6f}" for x in E_op))
    omega_op_ground = float(omega_op[0])

    # --- engine path (Path 1): FFT the recorded C(t) ---
    corr = read_ct(args.ct)
    omega_eng_ground, peaks = engine_fft_ground(corr, args.omega0, args.dt)
    print(f"[eng]  C(t) samples = {len(corr)}")
    print(f"[eng]  FFT peaks (omega[rad/tick], power) below/above omega0:")
    for (om, pw) in sorted(peaks, key=lambda p: p[0])[:12]:
        tag = " [BOUND]" if om < args.omega0 else ""
        print(f"         omega={om:+.6f}  power={pw:.4e}{tag}")
    print(f"[eng]  engine ground peak omega_1s = {omega_eng_ground:.6f}")

    # --- the science gate ---
    print("-" * 72)
    bound_eng = 0.0 < omega_eng_ground < args.omega0
    bound_op = omega_op_ground < args.omega0
    rel_err = (abs(omega_eng_ground - omega_op_ground) / omega_op_ground
               if omega_op_ground > 0 else float("inf"))

    print(f"  omega0 (clock)              = {args.omega0:.6f}")
    print(f"  operator ground   omega_1s  = {omega_op_ground:.6f}  "
          f"{'[BOUND]' if bound_op else '[NOT bound]'}")
    print(f"  engine   ground   omega_1s  = {omega_eng_ground:.6f}  "
          f"{'[BOUND]' if bound_eng else '[NOT bound]'}")
    print(f"  relative error |eng-op|/op  = {rel_err*100:.3f} %  "
          f"(threshold {args.tol*100:.1f} %)")

    match = bound_eng and bound_op and rel_err <= args.tol
    print("-" * 72)
    if match:
        verdict = "ENGINE-OPERATOR MATCH (hydrogen-1s consistent)"
    elif bound_eng and bound_op:
        verdict = (f"BOUND-BUT-OFF: both below omega0 but rel_err "
                   f"{rel_err*100:.2f}% > {args.tol*100:.1f}%")
    else:
        verdict = "NO-MATCH: ground peak is not bound below omega0 in one/both paths"
    print(f"VERDICT: {verdict}")
    print("=" * 72)
    return 0 if match else 1


if __name__ == "__main__":
    sys.exit(main())
