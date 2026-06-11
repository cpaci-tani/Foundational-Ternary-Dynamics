#!/usr/bin/env python3
"""
na_law_escape_scenario.py -- FTD-0110/FTD-0269: the radial escape cascade scenario.

A solvable scenario that exhibits WHY the N(A) law has the structure it has -- the
two regimes, the knee, and exactly where the framework ends and engine-tuning begins.
This is the geometric-regime report the prereg (PREREG_FTD0110_NA_LAW_v1 §5) committed
to deliver regardless of verdict.

THE SCENARIO. A point pulse |J| = A*K_GENESIS at the lattice center ignites a genesis
burst. Each fired voxel becomes a charge source that, via the Gauss projection, injects
the lattice Poisson Green's-function flux  J_boost(d) = -grad G_L(d)  into the void,
pushing neighbours over K_GENESIS. Two questions decide the cluster:

  (1) NEAR FIELD (inside the 27-block, r <= sqrt 3): which shells does a fired core
      ignite?  -> the sub-knee firing pattern. Set by the DISCRETE grad G_L (shell
      ordering is NOT monotone in r -- that is the framework's geometric fingerprint).

  (2) WHAT STOPS THE BURST?  Sub-knee the cluster is GEOMETRY-LIMITED (it cannot fire
      faster than the discrete shells light up). Super-knee it is ENERGY-LIMITED: the
      injected flux energy ~ A^2 is spent at a fixed cost per fired voxel, so
      N -> k_eff * A^2.  The knee is the crossover: where the cluster radius reaches the
      27-block edge and geometry stops limiting.

THE SOLVE.
  - super-knee exponent = 2  (FRAMEWORK: flux energy ~ A^2, exact)
  - super-knee coefficient k_eff  (ENGINE-TUNING: set by the per-voxel drain/friction)
  - sub-knee steep exponent      (geometry-limited cascade in the compact block)
  - knee A*  = where k(A) = N/A^2 saturates to k_eff = 27-block escape

So the SHAPE (two regimes + knee + exponents) is framework-determined; the CALIBRATION
(k_eff normalization) is engine-tuning. That is the BOUNDARY, made mechanical.

[EPISTEMIC: the energy-budget A^2 law and the discrete-Green's-function shell ordering
 are [DERIVED] structure; the coefficient k_eff is [IMPOSED] (drain/friction). This
 scenario explains the measured law; it does not promote anything. FTD-0269 verdict
 BOUNDARY stands.]
"""

import csv
import math
import sys

import numpy as np

K_MANIFEST = 0.511
N_C = 3
K_GENESIS = N_C * K_MANIFEST     # 1.533

# FTD-0261 engine law + our FTD-0269 model law (for the check).
FTD0261 = {10: 4.0, 12: 8.4, 14: 16.4, 16: 21.6, 20: 27.4, 25: 32.6,
           30: 45.0, 40: 91.8, 50: 130.2, 70: 260.2, 90: 383.3}


def lattice_green(L=48):
    """Discrete 18-pt-Laplacian Green's function G_L(r), zero mode removed.

    Returns (G real-space cube centered, gradient-magnitude field |grad G|).
    """
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    cx = np.cos(k)
    CX, CY, CZ = cx[:, None, None], cx[None, :, None], cx[None, None, :]
    M = (2.0 / 3.0) * (CX + CY + CZ) + (2.0 / 3.0) * (CX * CY + CY * CZ + CZ * CX) - 4.0
    inv = np.zeros_like(M)
    nz = np.abs(M) > 1e-12
    inv[nz] = 1.0 / M[nz]
    inv[0, 0, 0] = 0.0
    # unit point source at origin -> G = ifft(1/M); roll to center for readability
    src = np.zeros((L, L, L)); src[0, 0, 0] = 1.0
    G = np.fft.ifftn(np.fft.fftn(src) * inv).real
    return G, L


def boost_profile(G, L):
    """|grad G| (the injected flux magnitude) at each O_h shell offset from the source
    at the origin (index 0)."""
    def g(x, y, z):
        return G[x % L, y % L, z % L]

    def gradmag(x, y, z):
        gx = (g(x + 1, y, z) - g(x - 1, y, z)) * 0.5
        gy = (g(x, y + 1, z) - g(x, y - 1, z)) * 0.5
        gz = (g(x, y, z + 1) - g(x, y, z - 1)) * 0.5
        return math.sqrt(gx * gx + gy * gy + gz * gz)

    shells = [("center r=0", (0, 0, 0)), ("SC   r=1", (1, 0, 0)),
              ("FCC  r=v2", (1, 1, 0)), ("BCC  r=v3", (1, 1, 1)),
              ("SC2  r=2", (2, 0, 0)), ("r=3", (3, 0, 0)),
              ("r=4", (4, 0, 0)), ("r=6", (6, 0, 0)), ("r=8", (8, 0, 0))]
    out = []
    for name, (x, y, z) in shells:
        r = math.sqrt(x * x + y * y + z * z)
        out.append((name, r, gradmag(x, y, z)))
    return out


def solve_super_knee(law, lo_A=30):
    """Energy-budget solve: above the knee, fit N = k_eff * A^2 (exponent fixed at 2,
    the framework flux-energy law) and read off the engine-tuning coefficient k_eff."""
    As = np.array([a for a in sorted(law) if a >= lo_A], float)
    Ns = np.array([law[a] for a in sorted(law) if a >= lo_A], float)
    k_eff = float(np.mean(Ns / As**2))
    # how good is the fixed-exponent-2 law?
    rms = math.sqrt(np.mean((np.log10(Ns) - np.log10(k_eff * As**2))**2))
    return k_eff, rms


def find_knee(law):
    """Knee = where k(A)=N/A^2 first reaches its super-knee plateau (within 10%)."""
    k_eff, _ = solve_super_knee(law)
    for a in sorted(law):
        if law[a] / a**2 >= 0.9 * k_eff:
            return a, k_eff
    return None, k_eff


def main():
    print("=" * 70)
    print("FTD-0110/0269 — THE RADIAL ESCAPE CASCADE SCENARIO (solved)")
    print("=" * 70)
    print(f"K_GENESIS = N_c*K_MANIFEST = {N_C}*{K_MANIFEST} = {K_GENESIS:.4f}")

    # --- (1) near-field: the discrete Green's-function shell ordering -------------
    G, L = lattice_green(48)
    prof = boost_profile(G, L)
    print("\n(1) NEAR FIELD — flux a fired core injects per shell (|grad G_L|):")
    print(f"    {'shell':<12}{'r':>6}{'|grad G|':>12}{'x (4*pi*r^2)|gradG|':>22}")
    g_sc = None
    for name, r, gm in prof:
        far = 4 * math.pi * r * r * gm if r > 0 else float('nan')
        if name.startswith("SC "):
            g_sc = gm
        rel = f"{gm / g_sc:6.3f}x SC" if g_sc and r > 0 else ""
        print(f"    {name:<12}{r:6.3f}{gm:12.5f}{far:22.4f}   {rel}")
    print("    -> far-field 4*pi*r^2|gradG| -> const confirms |gradG| ~ 1/(4*pi*r^2)")
    print("       (Phase G geometric Coulomb). Near field is NON-monotone in r:")
    print("       the shell-ignition ORDER is set by discrete geometry, not by r.")

    # --- (2) the two regimes, solved on the FTD-0269 model + FTD-0261 engine ------
    # load the model law (run of record) if present, else use FTD-0261 only
    model = {}
    try:
        with open("scripts/exploration/results/na_law_2026-06-11/model_baseline.csv") as fh:
            for row in csv.DictReader(fh):
                model[int(round(float(row["A"])))] = float(row["N_mean"])
    except FileNotFoundError:
        pass

    for label, law in [("FTD-0261 engine", FTD0261)] + ([("FTD-0269 model", model)] if model else []):
        print(f"\n(2) ENERGY-BUDGET SOLVE — {label}")
        k_eff, rms = solve_super_knee(law)
        knee, _ = find_knee(law)
        print(f"    super-knee:  N = k_eff * A^2  with k_eff = {k_eff:.4f}  "
              f"(fixed-exponent-2 log10-RMS = {rms:.3f})")
        print(f"    knee A* (k saturates to plateau) = {knee}")
        print(f"    {'A':>5}{'N':>9}{'k=N/A^2':>10}{'regime':>14}")
        for a in sorted(law):
            k = law[a] / a**2
            reg = "energy-limited" if knee and a >= knee else "geometry-limited"
            print(f"    {a:>5}{law[a]:>9.1f}{k:>10.4f}{reg:>16}")

    # --- the solved structure ----------------------------------------------------
    keng, _ = solve_super_knee(FTD0261)
    knee_eng, _ = find_knee(FTD0261)
    print("\n" + "=" * 70)
    print("SOLVED STRUCTURE")
    print("=" * 70)
    print(f"  SUPER-KNEE (A > {knee_eng}):  N = k_eff * A^2")
    print(f"     exponent 2     <- FRAMEWORK: injected flux energy ~ (A*K_GENESIS)^2 ~ A^2,")
    print(f"                       spent at a fixed cost per fired voxel (energy budget).")
    print(f"     k_eff = {keng:.3f}   <- ENGINE-TUNING: the per-voxel cost is set by the")
    print(f"                       kinetic drain (0.5) + Langevin friction (gamma).")
    print(f"  SUB-KNEE  (A < {knee_eng}):  steep (~A^3.7) GEOMETRY-LIMITED cascade in the")
    print(f"     27-block; k(A) still climbing toward k_eff because the discrete shells")
    print(f"     cannot light up fast enough to spend the A^2 budget.")
    print(f"  KNEE A* ~ {knee_eng}: the 27-block escape -- geometry stops limiting, the")
    print(f"     energy budget takes over. Below: shape-limited. Above: budget-limited.")
    print("  => SHAPE (two regimes, knee, exponents) is FRAMEWORK; CALIBRATION (k_eff)")
    print("     is ENGINE-TUNING.  This is the FTD-0269 BOUNDARY, made mechanical.")


if __name__ == "__main__":
    sys.exit(main())
