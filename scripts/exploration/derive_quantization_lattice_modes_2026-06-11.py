#!/usr/bin/env python3
"""
derive_quantization_lattice_modes_2026-06-11.py  --  FTD-0270

Attempt to derive discrete energy-level QUANTIZATION from the FTD lattice
substrate WITHOUT importing hbar, and locate the exact boundary where the
substrate stops. Frozen under PREREG_QUANTIZATION_LATTICE_MODES_v1.

THE PHYSICS. FTD's flux field obeys a CLASSICAL wave equation, 2nd-order in
time (leapfrog in phase_write.cpp):  d^2 J/dt^2 = c^2 * Lap18(J),  c^2 = 1/3.
The 18-point O_h Laplacian has Fourier symbol
    M(k) = (2/3)(cx+cy+cz) + (2/3)(cx*cy + cy*cz + cz*cx) - 4,   ci = cos(k_i).
A bound region on the lattice has DISCRETE standing-wave eigenmodes (pure linear
algebra -- a string has discrete harmonics), so discreteness needs no hbar. The
question is the LEVEL PATTERN / DISPERSION:

  * 2nd-order wave (FTD):   physical excitation omega = c*sqrt(-M)  ~ |k|  (LINEAR)
                            => box ground mode omega_1 ~ 1/L   (finite-size s=1)
  * Schrodinger (atoms):    energy  E = -c^2 M  ~ k^2          (QUADRATIC)
                            => box ground level E_1 ~ 1/L^2   (s=2),  Rydberg 1/n^2

So the finite-size scaling exponent s of the ground mode is a clean dispersion
discriminator: s~1 = FTD wave/cavity (WRONG for atoms), s~2 = Schrodinger.
Candidate 2 (de Broglie) is the same fact seen kinetically: a linear/non-
dispersive medium has no lambda ~ 1/v de Broglie wavelength.

DELIVERABLE: not a win -- a sharp BOUNDARY. The lattice quantizes, but with the
wrong dispersion; atomic spectra are not substrate-derivable; hbar enters only
as E=hbar*omega (the scale), which the substrate never fixes.

[EPISTEMIC: forward analysis / quick-check tier. The eigenvalue dispersion is a
 rigorous property of the engine's exact operator; C1b is an IMPORTED Schrodinger
 operator used only as a discriminating diagnostic, never as "the substrate."]
"""

import argparse
import csv
import math
import sys

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla

C2 = 1.0 / 3.0                 # c^2, CFL wave speed squared  [framework]
C = math.sqrt(C2)
G_C = math.sqrt(1.0 / 137.036)  # state-flux coupling = sqrt(alpha)

FACE = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
EDGE = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_L18(L, periodic):
    """Sparse 18-pt O_h Laplacian on L^3. Dirichlet (drop out-of-box) or periodic."""
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
    """C1-periodic: confirm the discrete operator's eigenvalues are exactly the
    closed-form symbol M(k). Pure correctness check (a math identity, not a result)."""
    A = build_L18(L, periodic=True)
    # pick a few k-vectors; the periodic eigenvalue at k = 2*pi*n/L is M(k)
    worst = 0.0
    v = np.zeros(L * L * L, dtype=complex)
    idx = lambda x, y, z: (x * L + y) * L + z
    for (nx, ny, nz) in [(0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1), (2, 1, 0)]:
        kx, ky, kz = 2 * math.pi * nx / L, 2 * math.pi * ny / L, 2 * math.pi * nz / L
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    v[idx(x, y, z)] = np.exp(1j * (kx * x + ky * y + kz * z))
        Av = A @ v
        eig = (np.vdot(v, Av) / np.vdot(v, v)).real
        worst = max(worst, abs(eig - symbol_M(kx, ky, kz)))
    return worst


def box_ground_mode(L):
    """Lowest standing-wave mode of the free Dirichlet box.
    Returns (omega_1, E_schrod_1): the FTD 2nd-order-wave frequency c*sqrt(-M),
    and the Schrodinger-analog 'energy' c^2*(-M) from the SAME eigenvector."""
    A = build_L18(L, periodic=False)
    # eigenvalues closest to 0 from below = lowest-frequency modes => 'LA' (largest algebraic)
    vals = spla.eigsh(A, k=4, which='LA', return_eigenvectors=False)
    M1 = float(np.max(vals))           # closest to 0
    M1 = min(M1, -1e-12)               # guard
    omega1 = C * math.sqrt(-M1)        # FTD 2nd-order-wave dispersion
    E1 = C2 * (-M1)                    # Schrodinger-analog (diagnostic)
    return omega1, E1


def fit_powerlaw(Ls, ys):
    """Fit y = A * L^(-s); return s and its 1-sigma from log-log LSQ."""
    lx = np.log(np.array(Ls, float))
    ly = np.log(np.array(ys, float))
    n = len(lx)
    A = np.vstack([lx, np.ones(n)]).T
    coef, res, *_ = np.linalg.lstsq(A, ly, rcond=None)
    slope = coef[0]
    s = -slope
    # std error of slope
    yhat = A @ coef
    dof = max(1, n - 2)
    sigma2 = np.sum((ly - yhat) ** 2) / dof
    sxx = np.sum((lx - lx.mean()) ** 2)
    se = math.sqrt(sigma2 / sxx) if sxx > 0 else float('nan')
    return s, se


def candidate2_debroglie(L=48, ticks=140, vlist=(0.04, 0.06, 0.09, 0.13, 0.19)):
    """A 1D faithful leapfrog of the flux wave with a moving Gaussian state source
    (G_C * d s/dx), measuring the radiated wavelength lambda(v). de Broglie => r~1
    (lambda ~ 1/v); a linear/non-dispersive medium => r~0 (no characteristic lambda).
    1D suffices: the dispersion (omega=c|k|, non-dispersive) is the deciding property."""
    out = []
    x = np.arange(L)
    for v in vlist:
        J = np.zeros(L); Jv = np.zeros(L)
        sig = 2.0
        peak_k = []
        for t in range(ticks):
            x0 = (L * 0.25 + v * t) % L
            s = np.exp(-0.5 * ((x - x0) / sig) ** 2)
            grad_s = np.gradient(s)
            lap = np.roll(J, 1) + np.roll(J, -1) - 2.0 * J   # 1D Laplacian
            Jv += (C2 * lap + G_C * grad_s)
            J += Jv
            J *= 0.999
            if t > ticks // 2:
                sp_amp = np.abs(np.fft.rfft(J - J.mean()))
                if sp_amp[1:].max() > 3.0 * sp_amp[1:].mean():
                    peak_k.append(int(np.argmax(sp_amp[1:]) + 1))
        if peak_k:
            kdom = np.median(peak_k)
            lam = L / kdom if kdom > 0 else float('nan')
        else:
            lam = float('nan')
        out.append((v, lam))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--box-Ls", default="12,16,20,24,32")
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    print("=" * 68)
    print("FTD-0270 -- lattice quantization & dispersion (does the substrate quantize?)")
    print("=" * 68)

    # --- C1-periodic: operator correctness ---------------------------------
    werr = validate_symbol(8)
    print(f"\n[C1-periodic] |eig(L18) - M(k)| max = {werr:.2e}  "
          f"({'PASS (machine precision)' if werr < 1e-10 else 'FAIL'})")

    # --- C1-box: dispersion via finite-size scaling ------------------------
    Ls = [int(s) for s in args.box_Ls.split(",")]
    print(f"\n[C1-box] free Dirichlet box, ground standing-wave mode vs L:")
    print(f"  {'L':>4} {'omega_1 (FTD wave)':>20} {'E_1 (Schrod-analog)':>22}")
    omg, esh = [], []
    for L in Ls:
        o, e = box_ground_mode(L)
        omg.append(o); esh.append(e)
        print(f"  {L:>4} {o:>20.6f} {e:>22.6f}")
    s_ftd, se_ftd = fit_powerlaw(Ls, omg)
    s_sch, se_sch = fit_powerlaw(Ls, esh)
    print(f"\n  finite-size exponent s  (mode ~ L^-s):")
    print(f"    FTD 2nd-order wave  omega_1 ~ L^-{s_ftd:.3f} +/- {se_ftd:.3f}   "
          f"(LINEAR dispersion => s~1)")
    print(f"    Schrodinger analog  E_1     ~ L^-{s_sch:.3f} +/- {se_sch:.3f}   "
          f"(QUADRATIC => s~2) [imported diagnostic]")

    # --- C2: de Broglie wake test ------------------------------------------
    print(f"\n[C2] de Broglie test: wavelength of a moving cluster's flux wake vs v")
    db = candidate2_debroglie()
    print(f"  {'v':>6} {'lambda':>10}")
    vv, ll = [], []
    for v, lam in db:
        print(f"  {v:>6.3f} {lam:>10.3f}")
        if not math.isnan(lam):
            vv.append(v); ll.append(lam)
    if len(vv) >= 3:
        # lambda ~ v^-r ; de Broglie r~1, linear medium r~0
        lr = np.polyfit(np.log(vv), np.log(ll), 1)
        r = -lr[0]
        print(f"  de Broglie exponent  lambda ~ v^-{r:.3f}   "
              f"(de Broglie => r~1 ; linear medium => r~0)")
    else:
        r = float('nan')
        print("  de Broglie: no clean wake wavelength (NULL) -- consistent with a "
              "non-dispersive medium")

    # --- verdict -----------------------------------------------------------
    print("\n" + "=" * 68)
    print("VERDICT")
    print("=" * 68)
    c1 = ("WAVE-CAVITY-BOUNDARY" if abs(s_ftd - 1.0) < 0.2 else
          ("RYDBERG-SHAPE" if abs(s_ftd - 2.0) < 0.2 else "AMBIGUOUS"))
    c2 = ("DE-BROGLIE-CONFIRMED" if (not math.isnan(r) and abs(r - 1.0) < 0.3) else
          ("DE-BROGLIE-FAILED" if (math.isnan(r) or r < 0.3) else "AMBIGUOUS"))
    diag_ok = abs(s_sch - 2.0) < 0.3
    print(f"  C1 = {c1}   (FTD s={s_ftd:.2f}; Schrodinger-diagnostic s={s_sch:.2f}, "
          f"discriminates={diag_ok})")
    print(f"  C2 = {c2}   (r={r if not math.isnan(r) else float('nan'):.2f})")
    if c1 == "WAVE-CAVITY-BOUNDARY" and c2 == "DE-BROGLIE-FAILED":
        print("  => QUANTIZATION EXISTS, WRONG DISPERSION. The lattice DOES quantize")
        print("     (discrete modes), but with LINEAR (cavity/EM) dispersion, not the")
        print("     Schrodinger quadratic dispersion the hydrogen Rydberg needs. A moving")
        print("     cluster sources no de Broglie wave. Discrete atomic energy levels are")
        print("     NOT substrate-derivable; the boundary IS the 2nd-order dispersion law.")
        print("     hbar enters only as E=hbar*omega (the scale) -- never fixed by the substrate.")
    elif c1 == "RYDBERG-SHAPE":
        print("  => RYDBERG-SHAPE on the FTD operator -- UNEXPECTED; re-audit before any claim.")
    else:
        print("  => see exponents above; partial/ambiguous.")

    if args.out:
        with open(args.out, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["construction", "L_or_v", "value", "kind"])
            for L, o in zip(Ls, omg): w.writerow(["box_omega1", L, o, "ftd_wave"])
            for L, e in zip(Ls, esh): w.writerow(["box_E1", L, e, "schrod_diag"])
            for v, lam in db: w.writerow(["debroglie", v, lam, "wake_lambda"])
            w.writerow(["fit", "s_ftd", s_ftd, "exponent"])
            w.writerow(["fit", "s_schrod", s_sch, "exponent"])
            w.writerow(["fit", "r_debroglie", r, "exponent"])
        print(f"\n# wrote {args.out}")


if __name__ == "__main__":
    sys.exit(main())
