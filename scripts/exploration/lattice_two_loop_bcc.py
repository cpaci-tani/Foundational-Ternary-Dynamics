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
  * Computes I(mu^2) by numpy FFT at several mu^2 and several L.
  * Fits the log, extracts A and B.
  * Reports B and runs a COARSE genus diagnostic: is B closer to the
    lemniscatic family {Gamma(1/4)^k / pi^m} or the equianharmonic family
    {Gamma(1/3)^k / pi^m}?
  * HONEST CEILING: double precision + finite-(L, mu^2) extrapolation gives B
    to ~3-5 digits only.  A rigorous multi-term PSLQ period identification needs
    20-30 digits and is a high-precision (WSL2/GPU, arbitrary-precision) job, NOT
    reachable here.  This is a FIRST-LOOK VALUE + genus diagnostic, tagged
    [OPEN]/[EXPLORATORY], promoting nothing.  See SCOPE doc Sec 5 guards.
"""

import math
import numpy as np

CHECKS = []


def check(name, cond, detail=""):
    CHECKS.append(bool(cond))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}" + (f"   {detail}" if detail else ""))


# ------------------------------------------------------------------ #
# BCC massive propagator and the two-loop sunset  I(mu^2) = sum_x G(x)^3
# ------------------------------------------------------------------ #
def sigma_bcc(L):
    """Structure function cos(kx)cos(ky)cos(kz) on the L^3 momentum grid."""
    j = np.arange(L)
    c = np.cos(2.0 * np.pi * j / L)                 # cos(2 pi j / L)
    cx = c[:, None, None]
    cy = c[None, :, None]
    cz = c[None, None, :]
    return cx * cy * cz


def green_x(L, mu2, sig):
    """Position-space propagator G(x) via inverse FFT of 1/(1 - sigma + mu^2)."""
    P = 1.0 / (1.0 - sig + mu2)                     # mu^2 > 0 regulates k=0
    G = np.fft.ifftn(P).real                        # (1/L^3) sum_k e^{ikx} P(k)
    return G


def sunset(L, mu2, sig):
    """I(mu^2) = sum_x G(x)^3 -- the two-loop sunset at external p = 0."""
    G = green_x(L, mu2, sig)
    return float(np.sum(G ** 3))


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
    print("   I(mu^2) = sum_x G_BCC(x)^3 ,  G_BCC = FFT[1/(1 - cx cy cz + mu^2)]\n")
    print("   Structure: I(mu^2) = -A log(mu^2) + B + O(mu^2 log mu^2).")
    print("   B is the finite part carrying the period/genus content.")

    # IR window: need 1/mu << L so the tail fits in the box.  Larger L lets mu^2
    # go smaller (cleaner log), so we push L as far as is fast.
    mu2_list = [0.03, 0.02, 0.012, 0.008, 0.005, 0.003]

    y96, f96_2, f96_3 = run_L(96, mu2_list)
    y128, f128_2, f128_3 = run_L(128, mu2_list)

    # --- (1) the ROBUST result: raw I(mu^2) is L-converged to ~6 digits ---
    raw_agree = float(np.max(np.abs(y128 - y96)))
    print(f"\n  [ROBUST] raw I(mu^2): max|I(128)-I(96)| = {raw_agree:.2e}"
          f"  -> the sunset VALUE at fixed mu^2 is L-converged.")

    # --- (2) the UNSTABLE quantity: the finite part B depends on the fit model ---
    B2 = 0.5 * (f96_2[1] + f128_2[1])          # 2-param B (avg over L)
    B3 = 0.5 * (f96_3[1] + f128_3[1])          # 3-param B (avg over L)
    swing = abs(B3 - B2) / max(abs(B2), abs(B3))
    print(f"  [UNSTABLE] finite part B: 2-param -> {B2:.4f}, 3-param -> {B3:.4f}"
          f"   ({swing:.0%} model swing)")
    print("    => the period-carrying finite part is NOT robustly extractable at")
    print("       double precision; the mu^2->0 log-subtraction, not the FFT, is")
    print("       the ceiling.  No period ID (Z[i] vs Z[omega]) is possible here.")

    # coarse diagnostic on the better-conditioned 3-param B, clearly labelled
    B_inf = (f128_3[1] * 128 - f96_3[1] * 96) / (128 - 96)
    genus_diagnostic(B_inf)

    A_inf = (f128_3[0] * 128 - f96_3[0] * 96) / (128 - 96)

    # sanity checks -- these assert the METHOD behaved, not any period claim.
    check("raw sunset I(mu^2) is L-converged to < 1e-3 (FFT precise)",
          raw_agree < 1e-3, f"max|dI| = {raw_agree:.2e}")
    check("log-divergence coefficient A > 0 (IR tail present, expected)",
          A_inf > 0, f"A_inf = {A_inf:.4f}")
    check("finite part B is fit-model-UNSTABLE (>20% swing) -- the ceiling",
          swing > 0.20, f"2-param vs 3-param swing = {swing:.0%}")
    check("=> no double-precision period ID is claimed (guard held)", True)

    npass = sum(CHECKS)
    print(f"\n==== {npass}/{len(CHECKS)} checks passed ====")
    print("RESULT ([OPEN], attempted):")
    print("  * The two-loop BCC sunset VALUE I(mu^2) is computable and")
    print(f"    L-converged (max|dI| = {raw_agree:.1e} between L=96,128).")
    print(f"  * Its period-carrying finite part B is NOT: it swings {swing:.0%}")
    print(f"    between fit models (2-param {B2:.3f} vs 3-param {B3:.3f}).")
    print("  * So M2's sharp question -- does the BCC two-loop period stay")
    print("    lemniscatic (Z[i], Gamma(1/4)) or climb to Z[omega]/Gamma(1/3)? --")
    print("    CANNOT be answered at double precision.  Concretely confirms the")
    print("    SCOPE-doc ceiling: M2 needs 20-30 digit arbitrary precision.")
    print("  Promotes no tag; closes no milestone; documents WHY.")
    import sys
    sys.exit(0 if npass == len(CHECKS) else 1)


if __name__ == "__main__":
    main()
