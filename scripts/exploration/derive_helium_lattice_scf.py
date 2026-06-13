#!/usr/bin/env python3
"""
derive_helium_lattice_scf.py -- FTD-0279: HELIUM on the FTD lattice.

THE HELIUM CHALLENGE, ANSWERED AT ITS HONEST CEILING. The original helium
challenge produced FTD-0270's verdict ("atomic dynamics ~0% substrate-derived").
FTD-0278 (HYDROGEN-CONFIRMED) opened the conditional sector: GIVEN the de
Broglie clock + the scalar-potential coupling, the engine's exact machinery
produces hydrogen. This script asks the next question: does the SAME machinery,
plus ONE further declared import, produce HELIUM?

THE IMPORT REGISTER (cumulative, each motivated):
  I1. clock scalar omega0 ~ M_REST            [IMPOSED, FTD-0271]
  I2. scalar-potential coupling 2*omega0*V    [IMPOSED, FTD-0278 -- the engine's
      gravity-clock move applied to the Gauss phi]
  I3. MODE-OCCUPANCY import (NEW, FTD-0279): "two electrons" = two unit-norm
      occupations of the field's bound modes, each sourcing the engine's Gauss
      law and feeling the OTHER's Hartree potential. A classical field J(x) has
      NO two-particle configuration space (L^6); occupancy-with-mutual-sourcing
      is the minimal slice of second quantization needed for a 2-electron atom.
      Ground state 1s^2: spatially symmetric, spin-singlet bookkeeping imported;
      NO exchange term, NO correlation claimed.

WHAT REMAINS FTD-EXACT: the one-body operator (18-pt L18, machine-precision
symbol), the nuclear attraction AND the electron-electron repulsion both via
the SAME mean-free periodic lattice Green's function phiG (OT-1.4 [THEOREM] --
the engine's own Gauss solution), with the e-e Hartree potential computed by
FFT convolution against the engine symbol.

CONSTRUCTION (restricted Hartree, closed-shell 1s^2):
    V_eff[rho](r) = 2q*phiG(r)  -  q*(phiG (*) rho)(r)
    H[rho] = -(c^2/(2 omega0)) L18 + V_eff[rho]        (envelope operator)
    psi = ground eigenvector;  rho = |psi|^2;  mix; iterate to convergence.
    eps  = <psi|H[rho]|psi>            (orbital energy; -eps ~ Koopmans IP)
    E_ee = q * sum rho * (-(phiG (*) rho))             (one electron pair)
    E_He = 2*eps - E_ee                                 (no double counting)
He+ is the one-body problem at nuclear strength 2q. Non-interacting control:
E_nonint = 2*E1(2q) (exactly recovered when the V_ee term is switched off).

CONTINUUM DIMENSIONLESS TARGETS (Hartree-Fock level; for 1s^2 the exchange
term cancels against self-interaction in restricted Hartree, so Hartree==HF
here): E_He/E_nonint = 2.8617/4 = 0.7154 (the Z_eff = Z - 5/16 screening
physics); I_He/I_He+ = 0.9037/2.0 = 0.4519. Lattice values approach these as
a0 grows; the same-L 1/r-reference SCF carries the discretization honestly
(ratio-of-ratios, the FTD-0278 F-A pattern).

THE BOUNDARY BEYOND (declared, not attempted): correlation energy (continuum
-0.0420 Hartree, 1.4% of E_He) lives in genuine two-particle configuration
space -- entanglement the classical substrate does not carry (FC-1). Mean-field
is the ceiling of the mode-occupancy import; that ceiling is the finding.

[EPISTEMIC: development/quick-check tier until the pre-registration lock.
 Result class [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]; Hartree-for-helium
 is textbook 1928; NEVER 'FTD derives helium' unconditionally.]

Usage:
  python derive_helium_lattice_scf.py --dev
  python derive_helium_lattice_scf.py --record --out results.csv
"""

import argparse
import csv
import math
import sys

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla

# reuse the LOCKED hydrogen module's engine-exact machinery (import, not copy)
import derive_hydrogen_lattice_spectrum as hyd

C2 = hyd.C2
OMEGA0 = 1.5                       # same record clock scalar as FTD-0278

# continuum (HF-level) dimensionless targets
SIGMA_CONT = 2.8617 / 4.0          # E_He / E_nonint = 0.7154
ION_CONT = 0.9037 / 2.0            # I_He / I_He+    = 0.4519


def continuum_well_spectral(L, q):
    """REFERENCE (diagnostic, not FTD): the SPECTRAL periodized continuum
    Coulomb -- IFFT of -1/|k|^2 over the lattice k-grid (mean-free). Same torus,
    same FFT, only the symbol differs from the engine's M(k): this isolates the
    operator-symbol difference and carries the full periodic image sum (the
    minimum-image 1/r of the hydrogen module under-counts images at shallow
    binding -- measured in development, disclosed). V = +q*phi (attractive)."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    KX = k[:, None, None]; KY = k[None, :, None]; KZ = k[None, None, :]
    k2 = KX ** 2 + KY ** 2 + KZ ** 2
    inv = np.zeros_like(k2)
    nz = k2 > 1e-12
    inv[nz] = -1.0 / k2[nz]          # continuum symbol of the Laplacian
    src = np.zeros((L, L, L)); src[0, 0, 0] = 1.0
    phi = np.fft.ifftn(np.fft.fftn(src) * inv).real
    phi -= phi.mean()
    return q * phi


def hartree_potential(rho, q, green_inv_cache={}):
    """-q*(phiG (*) rho): the repulsive Hartree potential of one electron's
    density, via FFT against the ENGINE symbol (mean-free). rho is (L,L,L)."""
    L = rho.shape[0]
    if L not in green_inv_cache:
        k = 2.0 * np.pi * np.fft.fftfreq(L)
        cx = np.cos(k)
        CX = cx[:, None, None]; CY = cx[None, :, None]; CZ = cx[None, None, :]
        M = (2.0 / 3.0) * (CX + CY + CZ) + (2.0 / 3.0) * (CX * CY + CY * CZ + CZ * CX) - 4.0
        inv = np.zeros_like(M)
        nz = np.abs(M) > 1e-12
        inv[nz] = 1.0 / M[nz]
        inv[0, 0, 0] = 0.0
        green_inv_cache[L] = inv
    phi_rho = np.fft.ifftn(np.fft.fftn(rho) * green_inv_cache[L]).real
    return -q * phi_rho             # phiG<0 near the source -> repulsion > 0


def he_scf(L, q, omega0=OMEGA0, vee_on=True, max_iter=60, tol=1e-9,
           mix=0.5, potential="lattice", verbose=False):
    """Restricted Hartree SCF for 1s^2 helium on the L^3 torus.
    Nuclear strength 2q via the lattice (or 1/r-reference) well; e-e repulsion
    via the engine-symbol Hartree convolution (vee_on=False -> non-interacting
    control). Returns dict(E_He, eps, E_ee, iters, converged)."""
    A_lap = hyd.build_L18(L, periodic=True)
    if potential == "lattice":
        Vnuc, _, _ = hyd.coulomb_well(L, 2.0 * q)
    else:
        Vnuc = continuum_well_spectral(L, 2.0 * q)
    K = (-C2 / (2.0 * omega0)) * A_lap          # envelope kinetic operator

    rho = None
    E_prev = None
    eps = float("nan"); E_ee = 0.0
    for it in range(max_iter):
        if rho is None:
            VH = np.zeros((L, L, L))
        elif vee_on:
            VH = hartree_potential(rho, q)
        else:
            VH = np.zeros((L, L, L))
        H = K + sp.diags((Vnuc + VH).reshape(-1))
        vals, vecs = spla.eigsh(H, k=1, which='SA')
        eps = float(vals[0])
        psi = vecs[:, 0]
        rho_new = (psi * psi).reshape(L, L, L)
        rho_new /= rho_new.sum()                 # unit occupation
        rho = rho_new if rho is None else (1 - mix) * rho + mix * rho_new
        rho /= rho.sum()
        if vee_on:
            VH_now = hartree_potential(rho, q)
            E_ee = float(np.sum(rho * VH_now))
        else:
            E_ee = 0.0
        E_tot = 2.0 * eps - E_ee
        if verbose:
            print(f"    it={it:2d} eps={eps:+.6f} E_ee={E_ee:+.6f} E={E_tot:+.6f}")
        if E_prev is not None and abs(E_tot - E_prev) < tol:
            return dict(E_He=E_tot, eps=eps, E_ee=E_ee, iters=it + 1,
                        converged=True)
        E_prev = E_tot
    return dict(E_He=E_prev, eps=eps, E_ee=E_ee, iters=max_iter, converged=False)


def he_plus_xcheck(L, q, omega0=OMEGA0):
    """He+ ground envelope energy via the LOCKED hydrogen module's eigenpath
    (independent code path, same lattice operator) -- the F-He-D cross-check."""
    res = hyd.spectrum(L, 2.0 * q, omega0, k_eigs=4, potential="lattice")
    if res is None:
        return None
    a, omega, E, vmin, nb = res
    return float(a[0] / (2.0 * omega0))         # envelope convention


def cell(L, q, omega0=OMEGA0, potential="lattice", verbose=False):
    """One (L, q, potential) helium cell: SCF + non-interacting control.
    All energies in the first-order envelope convention (eps-consistent).
    He+ energy = E_nonint/2 by construction (one-body at 2q)."""
    scf = he_scf(L, q, omega0, vee_on=True, potential=potential, verbose=verbose)
    ctrl = he_scf(L, q, omega0, vee_on=False, max_iter=2, potential=potential)
    E_nonint = ctrl["E_He"]                     # = 2*E1(2q) exactly
    E1_2q = E_nonint / 2.0
    sigma = scf["E_He"] / E_nonint if E_nonint else float("nan")
    I_He = E1_2q - scf["E_He"]                  # ionization: He -> He+ + e
    I_Hep = -E1_2q                              # He+ -> nucleus + e
    ion_ratio = I_He / I_Hep if I_Hep else float("nan")
    return dict(scf=scf, E_nonint=E_nonint, E1_2q=E1_2q, sigma=sigma,
                I_He=I_He, I_Hep=I_Hep, ion_ratio=ion_ratio)


def dev_survey(Ls, qs, omega0=OMEGA0):
    print("=" * 76)
    print("FTD-0279 HELIUM -- DEVELOPMENT survey (no verdicts)")
    print(f"omega0={omega0}; continuum targets sigma={SIGMA_CONT:.4f} "
          f"ion_ratio={ION_CONT:.4f}")
    print("=" * 76)
    m = omega0 / C2
    for q in qs:
        a0H = 4.0 * math.pi / (m * q)
        print(f"\n--- q={q:.4f} (hydrogen a0={a0H:.1f}; He+ a0={a0H/2:.2f}; "
              f"He 1s ~ a0/1.69={a0H/1.6875:.2f}) ---")
        for L in Ls:
            for pot in ("lattice", "continuum"):
                c = cell(L, q, omega0, potential=pot)
                s = c["scf"]
                tag = "ENG" if pot == "lattice" else "REF"
                print(f"  L={L:3d} [{tag}] E_He={s['E_He']:+.6f} "
                      f"(eps={s['eps']:+.6f}, E_ee={s['E_ee']:+.6f}, "
                      f"it={s['iters']}{'' if s['converged'] else ' NOCONV'})  "
                      f"E_nonint={c['E_nonint']:+.6f}  E1(2q)={c['E1_2q']:+.6f}")
                print(f"          sigma=E_He/E_nonint={c['sigma']:.4f} "
                      f"(cont {SIGMA_CONT:.4f})   "
                      f"I_He/I_He+={c['ion_ratio']:.4f} (cont {ION_CONT:.4f})")


# ---- FROZEN RECORD PROTOCOL (locked by PREREG_HELIUM_LATTICE_SCF_v1) --------
# Development finding (disclosed): ABSOLUTE E_He is core-convention-sensitive
# (three reference conventions gave three absolute energies), but the
# DIMENSIONLESS observables (sigma, ion_ratio) agree across conventions to ~2%
# -- core sensitivity cancels in the ratios. The falsifiers therefore live on
# the dimensionless observables; absolute ENG/REF is reported descriptively.
RECORD_QS = [0.4654, 0.3490]       # hydrogen a0 = {6, 8} -> He 1s ~ {3.6, 4.7}
RECORD_LS = [48, 64]
FA_SIGMA_TOL = 0.03                # F-He-A: |sigma_ENG - sigma_REF| <= 0.03
FA_ION_TOL = 0.05                  #         and |ion_ENG - ion_REF| <= 0.05
FB_BAND = (0.60, 0.80)             # F-He-B: screening sigma_ENG in band, all cells
FB_TREND = True                    # ...and sigma(a0=8) closer to 0.7154 than a0=6 at L=64
FC_BAND = (0.30, 0.60)             # F-He-C: ionization ratio band, all cells
FD_CTRL_TOL = 1e-6                 # F-He-D: independent-eigenpath He+ cross-check


def record_run(out_csv=None):
    print("=" * 76)
    print("FTD-0279 HELIUM -- RUN OF RECORD (frozen protocol)")
    print(f"omega0={OMEGA0}  qs={RECORD_QS}  Ls={RECORD_LS}")
    print("=" * 76)
    rows, fa_pairs, sigmas, ions, fd_flags = [], [], {}, [], []
    for q in RECORD_QS:
        for L in RECORD_LS:
            ce = cell(L, q, OMEGA0, potential="lattice")
            cr = cell(L, q, OMEGA0, potential="continuum")
            if not (ce["scf"]["converged"] and cr["scf"]["converged"]):
                print(f"  q={q} L={L}: SCF NOCONV -> cell INVALID")
                rows.append((f"q{q}_L{L}", "noconv", 1.0, "INVALID"))
                continue
            ratio = ce["scf"]["E_He"] / cr["scf"]["E_He"]   # descriptive only
            ds = abs(ce["sigma"] - cr["sigma"])
            di = abs(ce["ion_ratio"] - cr["ion_ratio"])
            fa_pairs.append((ds, di))
            sigmas[(q, L)] = ce["sigma"]
            ions.append(ce["ion_ratio"])
            # F-He-D cross-check: He+ via the LOCKED hydrogen module's
            # independent eigenpath equals E_nonint/2 from this script's SCF path
            xc = he_plus_xcheck(L, q, OMEGA0)
            rel = abs(xc - ce["E1_2q"]) / abs(ce["E1_2q"])
            fd_flags.append(rel <= FD_CTRL_TOL)
            print(f"  q={q:.4f} L={L}: E_He(ENG)={ce['scf']['E_He']:+.6f} "
                  f"sigma={ce['sigma']:.4f} (REF {cr['sigma']:.4f}, d={ds:.4f}) "
                  f"ion={ce['ion_ratio']:.4f} (REF {cr['ion_ratio']:.4f}, d={di:.4f}) "
                  f"ENG/REF_abs={ratio:.3f} ctrl_rel={rel:.2e}")
            for kk, vv in (("E_He", ce["scf"]["E_He"]), ("eps", ce["scf"]["eps"]),
                           ("E_ee", ce["scf"]["E_ee"]), ("E_nonint", ce["E_nonint"]),
                           ("E1_2q", ce["E1_2q"]), ("sigma", ce["sigma"]),
                           ("sigma_ref", cr["sigma"]), ("ion_ratio", ce["ion_ratio"]),
                           ("ion_ref", cr["ion_ratio"]), ("eng_ref_abs", ratio)):
                rows.append((f"q{q}_L{L}", kk, vv, ""))

    n_cells = len(RECORD_QS) * len(RECORD_LS)
    fa = (len(fa_pairs) == n_cells
          and all(ds <= FA_SIGMA_TOL and di <= FA_ION_TOL for ds, di in fa_pairs))
    fb_in = len(sigmas) == n_cells and all(FB_BAND[0] <= s <= FB_BAND[1]
                                           for s in sigmas.values())
    Lmax = max(RECORD_LS)
    s6 = sigmas.get((RECORD_QS[0], Lmax)); s8 = sigmas.get((RECORD_QS[1], Lmax))
    fb_tr = (s6 is not None and s8 is not None
             and abs(s8 - SIGMA_CONT) <= abs(s6 - SIGMA_CONT)) if FB_TREND else True
    fb = fb_in and fb_tr
    fc = len(ions) == n_cells and all(FC_BAND[0] <= i <= FC_BAND[1] for i in ions)
    fd = len(fd_flags) == n_cells and all(fd_flags)
    print("-" * 76)
    print(f"F-He-A (dimensionless ENG~REF: |d_sigma|<={FA_SIGMA_TOL}, "
          f"|d_ion|<={FA_ION_TOL}): {'PASS' if fa else 'FAIL'}  "
          f"{[('%.4f' % ds, '%.4f' % di) for ds, di in fa_pairs]}")
    print(f"F-He-B (screening sigma in {FB_BAND} + a0-trend toward "
          f"{SIGMA_CONT:.4f}): {'PASS' if fb else 'FAIL'}  "
          f"{dict(((str(k), round(v, 4)) for k, v in sigmas.items()))}")
    print(f"F-He-C (ionization ratio in {FC_BAND}, cont {ION_CONT:.4f}): "
          f"{'PASS' if fc else 'FAIL'}")
    print(f"F-He-D (vee-off control exact): {'PASS' if fd else 'FAIL'}")
    verdict = ("HELIUM-CONFIRMED" if (fa and fb and fc and fd)
               else ("PARTIAL" if (fa and fd) else "CLOSED-NEGATIVE"))
    print("=" * 76)
    print(f"FTD-0279 VERDICT: {verdict}")
    print("=" * 76)
    if out_csv:
        with open(out_csv, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["cell", "quantity", "value", "note"])
            w.writerows(rows)
            w.writerow(["VERDICT", verdict, "", ""])
        print(f"# wrote {out_csv}")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dev", action="store_true")
    ap.add_argument("--record", action="store_true")
    ap.add_argument("--Ls", default="32,48")
    ap.add_argument("--qs", default="0.9308,0.6981")
    ap.add_argument("--omega0", type=float, default=OMEGA0)
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    if args.record:
        return record_run(args.out)
    if args.dev:
        Ls = [int(s) for s in args.Ls.split(",")]
        qs = [float(s) for s in args.qs.split(",")]
        dev_survey(Ls, qs, args.omega0)
        return 0
    print("use --dev or --record", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
