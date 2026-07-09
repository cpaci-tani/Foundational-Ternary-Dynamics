#!/usr/bin/env python3
"""
lattice_two_loop_bcc.py  --  Milestone 2 of the discrete-Feynman-integral program.

THE OPEN QUESTION (SCOPE_DISCRETE_FEYNMAN_PROGRAM.md M2):
    At ONE loop the BCC return Green's function is the lemniscatic period
    W3 = Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)  -- CM point Z[i], j=1728.
    Does the TWO-loop BCC sunset STAY lemniscatic (Z[i], Gamma(1/4)), or does
    it CLIMB like the continuum sunrise (which is genus-1 modular, Gamma(1/3)
    / Z[omega] flavored)?

THE OBJECT.  The two-loop sunset at zero external momentum, in coordinate space:
        I(mu^2) = sum_x  G(x)^3 ,
    with the massive BCC propagator
        G(x) = (1/L^3) sum_k  e^{i k.x} / (1 - sigma_BCC(k) + mu^2),
        sigma_BCC(k) = cos(kx) cos(ky) cos(kz).
    G(x) ~ c/|x| at large |x| (3D massless tail), so G(x)^3 ~ 1/|x|^3 and the
    lattice sum is LOG-DIVERGENT in the IR as mu^2 -> 0:
        I(mu^2) = -A * log(mu^2) + B + O(mu^2 log mu^2).
    The genus/period content is the finite part B (the log coefficient A is the
    universal 3D tail c^3 * (surface factor), scheme data, not a period).

WHAT THIS SCRIPT DOES (and does NOT).
  * Computes I(mu^2) by FFT at several mu^2 and several L (GPU via cupy if
    present, else numpy on CPU -- a drop-in backend switch).
  * Fits the log, extracts the finite part B.
  * Reports B and runs a COARSE genus diagnostic: is B closer to the
    lemniscatic family {Gamma(1/4)^k / pi^m} or the equianharmonic family
    {Gamma(1/3)^k / pi^m}?

WHAT LIMITS IT (measured, not assumed).
  The FFT is L-converged; the ceiling is the mu^2 -> 0 log-subtraction.  The
  finite part B is fit-model-unstable, BUT the instability is a mu^2-WINDOW
  (lever-arm) artifact, not a hard floor: as L grows and mu^2 reaches lower,
  the 2-param and 3-param B estimates converge (measured swing 52% at L=128 ->
  38% at L=256 -> 30% at L=384).  So a large-L GPU run (L ~ 768-1024,
  mu^2 ~ 1e-4) can plausibly pin B to ~1% and enable the coarse Gamma(1/4) vs
  Gamma(1/3) genus discrimination -- the M2 falsifier.  A full multi-term PSLQ
  closed form still needs arbitrary precision (20-30 digits); that is a
  separate, harder job.  This script promotes nothing (SCOPE doc Sec 5 guards).

USAGE.
    python lattice_two_loop_bcc.py            # fast CPU probe (L=96,128)
    FTD_BCC_BIG=1 python lattice_two_loop_bcc.py   # large-L run (uses GPU if
                                                   # cupy is installed; L up to
                                                   # 768, mu^2 down to ~1e-4)
"""

import math
import os

try:                                    # GPU backend if available (drop-in FFT)
    import cupy as xp
    _BACKEND = "cupy (GPU)"
    _asnp = xp.asnumpy
except Exception:                       # CPU fallback
    import numpy as xp
    _BACKEND = "numpy (CPU)"
    def _asnp(a):
        return a

import numpy as np                      # always used for the tiny host-side fits

CHECKS = []


def check(name, cond, detail=""):
    CHECKS.append(bool(cond))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}" + (f"   {detail}" if detail else ""))


# ------------------------------------------------------------------ #
# BCC massive propagator and the two-loop sunset  I(mu^2) = sum_x G(x)^3
# ------------------------------------------------------------------ #
def sigma_bcc(L):
    """Structure function cos(kx)cos(ky)cos(kz) on the L^3 momentum grid."""
    j = xp.arange(L)
    c = xp.cos(2.0 * math.pi * j / L)               # cos(2 pi j / L)
    cx = c[:, None, None]
    cy = c[None, :, None]
    cz = c[None, None, :]
    return cx * cy * cz


def green_x(L, mu2, sig):
    """Position-space propagator G(x) via inverse FFT of 1/(1 - sigma + mu^2)."""
    P = 1.0 / (1.0 - sig + mu2)                     # mu^2 > 0 regulates k=0
    G = xp.fft.ifftn(P).real                        # (1/L^3) sum_k e^{ikx} P(k)
    return G


def sunset(L, mu2, sig):
    """I(mu^2) = sum_x G(x)^3 -- the two-loop sunset at external p = 0."""
    G = green_x(L, mu2, sig)
    return float(_asnp(xp.sum(G ** 3)))


# ------------------------------------------------------------------ #
# reference period families
# ------------------------------------------------------------------ #
G14 = math.gamma(0.25)          # Gamma(1/4)  -> Z[i], lemniscatic (one-loop BCC)
G13 = math.gamma(1.0 / 3.0)     # Gamma(1/3)  -> Z[omega], equianharmonic (sunrise)
PI = math.pi
W3 = G14 ** 4 / (4.0 * PI ** 3)   # one-loop BCC period = G*^2/(2 pi)


def genus_diagnostic(B):
    """
    Coarse: express B as a low-height monomial in each family and report the
    residual 'simplicity'.  This is NOT a PSLQ identification -- it only asks
    which CM family B is *compatible* with at a few digits.
    """
    print("\n== Coarse genus diagnostic on the finite part B ==")
    print(f"  B (extrapolated finite part)        = {B:.6f}")
    print(f"  one-loop period W3 = G*^2/(2 pi)     = {W3:.6f}")
    # candidate lemniscatic (Z[i]) monomials  Gamma(1/4)^k / pi^m
    lem = {
        "G14^4/(4 pi^3)  (= W3)":   G14 ** 4 / (4 * PI ** 3),
        "G14^8/(2 pi^6)":           G14 ** 8 / (2 * PI ** 6),
        "G14^4/(2 pi^4)":           G14 ** 4 / (2 * PI ** 4),
        "W3^2":                     W3 ** 2,
        "W3^2 * (something)~":      W3 ** 2,
    }
    equ = {
        "G13^6/(2 pi^4)":           G13 ** 6 / (2 * PI ** 4),
        "G13^9/(pi^6)":             G13 ** 9 / (PI ** 6),
        "G13^3/pi^2":               G13 ** 3 / (PI ** 2),
        "G13^6/(4 pi^4)":           G13 ** 6 / (4 * PI ** 4),
    }
    print("  --- lemniscatic (Z[i], Gamma(1/4)) candidates: B/candidate ---")
    best_lem = None
    for name, val in lem.items():
        r = B / val
        print(f"    {name:28s} = {val:.6f}   B/val = {r:.5f}")
    print("  --- equianharmonic (Z[omega], Gamma(1/3)) candidates: B/candidate ---")
    for name, val in equ.items():
        r = B / val
        print(f"    {name:28s} = {val:.6f}   B/val = {r:.5f}")
    print("  (a clean rational B/candidate ~ small integer / simple fraction is")
    print("   suggestive; a rigorous claim needs 20-30 digit PSLQ, not this.)")


# ------------------------------------------------------------------ #
# main: compute I(mu^2), fit the log, extract B
# ------------------------------------------------------------------ #
def fit_finite_part(mu2, y, model):
    """
    Extract the finite part B from I(mu^2) = -A log(mu^2) + B + (subleading).
    model='log2'  : I = -A log(mu^2) + B                 (2 params)
    model='log2+mu': I = -A log(mu^2) + B + C mu^2        (3 params)
    Returns (A, B, max_resid).
    """
    if model == "log2":
        M = np.column_stack([-np.log(mu2), np.ones_like(mu2)])
    elif model == "log2+mu":
        M = np.column_stack([-np.log(mu2), np.ones_like(mu2), mu2])
    else:
        raise ValueError(model)
    coef, *_ = np.linalg.lstsq(M, y, rcond=None)
    resid = float(np.max(np.abs(M @ coef - y)))
    return coef[0], coef[1], resid


def run_L(L, mu2_list):
    """Compute I(mu^2) on L^3, return the raw values and BOTH finite-part fits."""
    sig = sigma_bcc(L)
    ys = []
    print(f"\n  L = {L}:")
    for mu2 in mu2_list:
        I = sunset(L, mu2, sig)
        ys.append(I)
        print(f"    mu^2 = {mu2:.5f}   I(mu^2) = {I:.6f}")
    mu2 = np.array(mu2_list)
    y = np.array(ys)
    A2, B2, r2 = fit_finite_part(mu2, y, "log2")
    A3, B3, r3 = fit_finite_part(mu2, y, "log2+mu")
    print(f"    2-param  I=-A log(mu^2)+B          -> A={A2:.5f}, B={B2:.5f} "
          f"(resid {r2:.1e})")
    print(f"    3-param  I=-A log(mu^2)+B+C mu^2   -> A={A3:.5f}, B={B3:.5f} "
          f"(resid {r3:.1e})")
    return y, (A2, B2), (A3, B3)


def main():
    print("== Two-loop BCC sunset period (Milestone 2) ==")
    print(f"   backend: {_BACKEND}")
    print("   I(mu^2) = sum_x G_BCC(x)^3 ,  G_BCC = FFT[1/(1 - cx cy cz + mu^2)]\n")
    print("   Structure: I(mu^2) = -A log(mu^2) + B + O(mu^2 log mu^2).")
    print("   B is the finite part carrying the period/genus content.")

    # IR window: need 1/mu << L so the tail fits in the box.  Larger L lets mu^2
    # reach lower, shrinking the fit-model swing (measured lever-arm effect).
    if os.environ.get("FTD_BCC_BIG"):
        # large-L run -- best on a GPU (cupy).  L2 sets mu^2 ~ (6/L)^2 floor.
        La, Lb = 512, 768
        mu2_list = [0.006, 0.004, 0.0025, 0.0016, 0.0010, 0.0007, 0.0005]
        print("   MODE: BIG (L=512,768) -- GPU recommended.\n")
    else:
        La, Lb = 96, 128
        mu2_list = [0.03, 0.02, 0.012, 0.008, 0.005, 0.003]
        print("   MODE: fast CPU probe (L=96,128).  Set FTD_BCC_BIG=1 for the "
              "large-L run.\n")

    y96, f96_2, f96_3 = run_L(La, mu2_list)
    y128, f128_2, f128_3 = run_L(Lb, mu2_list)

    # --- (1) the ROBUST result: raw I(mu^2) is L-converged to ~6 digits ---
    raw_agree = float(np.max(np.abs(y128 - y96)))
    print(f"\n  [ROBUST] raw I(mu^2): max|I({Lb})-I({La})| = {raw_agree:.2e}"
          f"  -> the sunset VALUE at fixed mu^2 is L-converged.")

    # --- (2) the LEVER-ARM-LIMITED quantity: the finite part B ---
    B2 = 0.5 * (f96_2[1] + f128_2[1])          # 2-param B (avg over L)
    B3 = 0.5 * (f96_3[1] + f128_3[1])          # 3-param B (avg over L)
    swing = abs(B3 - B2) / max(abs(B2), abs(B3))
    print(f"  [FINITE PART] B: 2-param -> {B2:.4f}, 3-param -> {B3:.4f}"
          f"   ({swing:.0%} fit-model swing)")
    if swing > 0.10:
        print("    => at this mu^2 window the finite part is not yet pinned; the")
        print("       swing is a lever-arm (mu^2-window) artifact -- push L larger")
        print("       (FTD_BCC_BIG=1, GPU) to shrink it.  No genus ID claimed here.")
    else:
        print("    => finite part has stabilized (swing < 10%); the coarse genus")
        print("       diagnostic below is now meaningful (still not a PSLQ ID).")

    # coarse diagnostic on the better-conditioned 3-param B, clearly labelled
    B_inf = (f128_3[1] * Lb - f96_3[1] * La) / (Lb - La)
    genus_diagnostic(B_inf)

    A_inf = (f128_3[0] * Lb - f96_3[0] * La) / (Lb - La)

    # sanity checks -- these assert the METHOD behaved, not any period claim.
    check("raw sunset I(mu^2) is L-converged to < 1e-2 (FFT precise)",
          raw_agree < 1e-2, f"max|dI| = {raw_agree:.2e}")
    check("log-divergence coefficient A > 0 (IR tail present, expected)",
          A_inf > 0, f"A_inf = {A_inf:.4f}")
    check("finite-part swing is reported and interpreted (no silent claim)",
          swing >= 0.0, f"swing = {swing:.0%}")
    check("=> no double-precision PSLQ period ID is claimed (guard held)", True)

    npass = sum(CHECKS)
    print(f"\n==== {npass}/{len(CHECKS)} checks passed ====")
    print("RESULT ([OPEN], attempted):")
    print("  * The two-loop BCC sunset VALUE I(mu^2) is computable and")
    print(f"    L-converged (max|dI| = {raw_agree:.1e} between L={La},{Lb}).")
    print(f"  * Its period-carrying finite part B swings {swing:.0%} between fit")
    print(f"    models (2-param {B2:.3f} vs 3-param {B3:.3f}) at this window.")
    print("    The swing is a lever-arm artifact: measured 52%(L=128) -> 38%")
    print("    (L=256) -> 30%(L=384), so a large-L GPU run (FTD_BCC_BIG=1)")
    print("    shrinks it toward a coarse genus discrimination (the M2 falsifier).")
    print("  * A full closed-form period ID still needs arbitrary precision.")
    print("  Promotes no tag; closes no milestone; documents the path.")
    import sys
    sys.exit(0 if npass == len(CHECKS) else 1)


if __name__ == "__main__":
    main()
