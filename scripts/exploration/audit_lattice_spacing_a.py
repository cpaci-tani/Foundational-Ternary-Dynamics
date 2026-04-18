"""
audit_lattice_spacing_a.py

Investigate whether a = 2/D = 2/3 is forced from lattice first principles,
or merely "close to a_opt" with the residual absorbed by higher loops.

Current state: DERIV_ONE_LOOP_LATTICE_ALPHA.md tags a = 2/D as [SELECTION]
with only the "(D-1)/D is the boundary/bulk ratio" geometric justification.
This script asks three questions:

  (Q1) What is a_opt — the value of a that makes the one-loop correction
       exactly close the CODATA gap? The existing doc reports a_opt = 0.66486.
       Reproduce this via numerical BZ integration.

  (Q2) Among low-height rationals expressible in the base-integer set
       {N_c=3, N_base=4, b_3=7, N_eff=13, D=47, BCC=8}, which is closest
       to a_opt? Is it uniquely 2/3, or are there competitors?

  (Q3) Does the BZ tadpole I_1(m^2 a^2) / (m^2 a) — as a function of a —
       have a special point (minimum, inflection, etc.) at a = 2/D that
       would motivate the choice beyond "closes the gap"?

The script computes I_1(m^2) for the 3D cubic lattice Brillouin zone
integral via mpmath triple integration at moderate precision.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product
import math
import numpy as np
from scipy import integrate
from mpmath import mp, mpf, gamma, pi as mp_pi, sqrt as mp_sqrt


# -------- Compute tree-level x+ once at high precision via mpmath, use float elsewhere --------
mp.dps = 30
_Gstar_mp = mp_sqrt(mpf(2)) * gamma(mpf(1) / 4) ** 2 / (2 * mp_pi)
_xplus_mp = 8 * _Gstar_mp ** 2 + 4 * _Gstar_mp ** (mpf(3) / 2) * mp_sqrt(4 * _Gstar_mp - 1)
_xminus_mp = 8 * _Gstar_mp ** 2 - 4 * _Gstar_mp ** (mpf(3) / 2) * mp_sqrt(4 * _Gstar_mp - 1)

# Convert to high-precision floats for fast scipy integration
Gstar = float(_Gstar_mp)
x_plus = float(_xplus_mp)
x_minus = float(_xminus_mp)
m2_phys = x_plus - x_minus       # ~134.012
g_coupling = 2.0                 # V''' at tree level
codata_alpha_inv = 137.035999177
required_delta = codata_alpha_inv - x_plus


# -------- Base-integer set --------
N_c = 3
N_base = 4
b3 = 7
N_eff = 13
D = N_c * N_base ** 2 - 1  # 47
BCC = 8
BASE_INTS = (N_c, N_base, b3, N_eff, D, BCC)


# -------- BZ tadpole integral (scipy, fast) --------
def I1_tadpole(m2_lat):
    """
    I_1(m^2_lat) = int_{[0,pi]^3} dk / pi^3 * 1/(k_hat^2 + m^2_lat)

    where k_hat^2 = 4 (sin^2(k1/2) + sin^2(k2/2) + sin^2(k3/2)).
    Using symmetry: BZ [-pi,pi]^3 -> [0,pi]^3 with factor 8; divide by
    (2 pi)^3 overall -> prefactor 1/pi^3.

    Use scipy.integrate.tplquad with modest tolerance for speed.
    """
    def integrand(k3, k2, k1):  # tplquad signature is (z, y, x)
        khat2 = 4 * (math.sin(k1 / 2) ** 2
                     + math.sin(k2 / 2) ** 2
                     + math.sin(k3 / 2) ** 2)
        return 1.0 / (khat2 + m2_lat)

    # tplquad(func, x_lo, x_hi, y_lo, y_hi, z_lo, z_hi)
    val, _err = integrate.tplquad(
        integrand,
        0, math.pi,
        0, math.pi,
        0, math.pi,
        epsabs=1e-10, epsrel=1e-10,
    )
    return val / math.pi ** 3


def delta_x_one_loop(a):
    """
    VEV shift from the one-loop tadpole in a cubic phi^3 theory:

      delta_phi_lat = -g * I_1(m^2_lat) / (2 * m^2_lat)    [symmetry factor 1/2]
      delta_x       = delta_phi_lat * a

    So:
      delta_x(a) = -g * I_1(m^2 a^2) * a / (2 * m^2 a^2)
                 = -g * I_1(m^2 a^2) / (2 * m^2 * a)

    Reproduces the formula used in DERIV_ONE_LOOP_LATTICE_ALPHA.md
    (g = 2, a = 2/3 -> delta_x = -1.71e-4).
    """
    m2_lat = m2_phys * a * a
    I1 = I1_tadpole(m2_lat)
    delta = -g_coupling * I1 / (2.0 * m2_phys * a)
    return delta, I1


# -------- Q1: Parameter sweep --------
def sweep_a(a_values):
    print("=" * 70)
    print("(Q1) Parameter sweep: delta_x vs a")
    print("=" * 70)
    print(f"Tree-level x+ = {x_plus:.12f}")
    print(f"CODATA alpha^-1 = {codata_alpha_inv:.12f}")
    print(f"Required delta (CODATA - x+) = {required_delta:.6e}")
    print(f"m^2_phys = x+ - x- = {m2_phys:.6f}")
    print(f"g = V''' = {g_coupling}")
    print()
    print(f"{'a':>10}  {'m^2_lat':>12}  {'I_1':>14}  {'delta_x':>16}  {'gap_to_CODATA':>18}")
    print("-" * 80)
    results = []
    for a in a_values:
        a_f = float(a)
        delta, I1 = delta_x_one_loop(a_f)
        x_corrected = x_plus + delta
        gap = x_corrected - codata_alpha_inv
        results.append((a_f, delta, I1, x_corrected, gap))
        print(f"{a_f:>10.6f}  {m2_phys * a_f**2:>12.5f}  "
              f"{I1:>14.10f}  {delta:>16.10e}  {gap:>18.10e}")
    return results


def solve_a_opt():
    """Find a such that delta_x(a) = required_delta via bisection."""
    print()
    print("=" * 70)
    print("(Q1) Solve for a_opt such that one-loop closes the gap exactly")
    print("=" * 70)
    lo, hi = 0.5, 0.9
    delta_lo, _ = delta_x_one_loop(lo)
    delta_hi, _ = delta_x_one_loop(hi)
    print(f"delta_x({lo}) = {delta_lo:.6e}")
    print(f"delta_x({hi}) = {delta_hi:.6e}")
    target = required_delta
    for _ in range(40):
        mid = (lo + hi) / 2
        delta_mid, _ = delta_x_one_loop(mid)
        if (delta_mid - target) * (delta_lo - target) < 0:
            hi = mid
        else:
            lo = mid
            delta_lo = delta_mid
        if abs(hi - lo) < 1e-9:
            break
    a_opt = (lo + hi) / 2
    delta_opt, _ = delta_x_one_loop(a_opt)
    two_over_d = 2.0 / 3.0
    print(f"a_opt = {a_opt:.10f}")
    print(f"2/D = 2/3 = {two_over_d:.10f}")
    print(f"a_opt - 2/D = {a_opt - two_over_d:.6e}")
    print(f"|a_opt - 2/D| / (2/D) = {abs(a_opt - two_over_d) / two_over_d * 100:.6f} %")
    print(f"delta at a_opt = {delta_opt:.6e} (target: {target:.6e})")
    return a_opt


# -------- Q2: Low-height rational candidates --------
def enumerate_candidate_rationals_in_range(lo: float, hi: float,
                                           max_num: int = 200, max_den: int = 200):
    """Enumerate rationals p/q in [lo, hi] with p, q drawn from small products
    of BASE_INTS, bounded by max_num and max_den."""
    def candidate_integers(bound):
        seeds = set()
        for a in BASE_INTS:
            seeds.add(a)
            seeds.add(a * a)
        for a, b in product(BASE_INTS, repeat=2):
            for combo in (a * b, a + b, abs(a - b)):
                if 0 < combo <= bound:
                    seeds.add(combo)
        # Also simple small integers
        seeds.update(range(1, 30))
        return {s for s in seeds if 0 < s <= bound}

    nums = candidate_integers(max_num)
    dens = candidate_integers(max_den)
    rats = set()
    for p in nums:
        for q in dens:
            f = Fraction(p, q)
            if lo <= float(f) <= hi:
                rats.add(f)
    return rats


def rank_candidates_against_a_opt(a_opt):
    print()
    print("=" * 70)
    print("(Q2) Low-height base-integer rationals near a_opt")
    print("=" * 70)
    candidates = enumerate_candidate_rationals_in_range(0.55, 0.75,
                                                        max_num=200, max_den=200)
    ranked = []
    for r in candidates:
        height = max(abs(r.numerator), r.denominator)
        diff = abs(float(r) - a_opt)
        ranked.append((r, height, diff))
    # Sort by height first (prefer simpler), then by closeness
    ranked.sort(key=lambda t: (t[1], t[2]))
    print(f"{'rational':>10}  {'height':>8}  {'|r - a_opt|':>14}  notes")
    print("-" * 70)
    for r, height, diff in ranked[:15]:
        note = ""
        if r == Fraction(2, 3):
            note = "<-- 2/D (claimed)"
        elif r == Fraction(3, 4):
            note = "(N_c / N_base)"
        elif r == Fraction(4, 7):
            note = "(N_base / b_3)"
        elif r == Fraction(7, 13):
            note = "(b_3 / N_eff)"
        elif r == Fraction(13, 20):
            note = "(N_eff / 20)"
        print(f"{str(r):>10}  {height:>8}  {diff:>14.6e}  {note}")
    return ranked


# -------- Q3: Special-point analysis --------
def check_special_points_at_a_equals_2_over_D(a_values_around):
    print()
    print("=" * 70)
    print("(Q3) Is there a special point (minimum/inflection) at a = 2/D?")
    print("=" * 70)
    print(f"{'a':>12}  {'delta_x':>20}  {'d(delta_x)/da':>22}")
    print("-" * 65)
    results = []
    h = 1e-4
    for a in a_values_around:
        a_f = float(a)
        delta, _ = delta_x_one_loop(a_f)
        dplus, _ = delta_x_one_loop(a_f + h)
        dminus, _ = delta_x_one_loop(a_f - h)
        derivative = (dplus - dminus) / (2 * h)
        results.append((a_f, delta, derivative))
        print(f"{a_f:>12.6f}  {delta:>20.12e}  {derivative:>22.6e}")
    return results


def main():
    print("Lattice spacing a = 2/D first-principles audit (scipy-accelerated)")
    print()

    a_samples = [0.55, 0.60, 0.62, 0.64, 0.66, 2.0/3, 0.67, 0.68, 0.70, 0.75]
    sweep_results = sweep_a(a_samples)

    a_opt = solve_a_opt()

    candidates = rank_candidates_against_a_opt(a_opt)

    a_around = [2.0/3 - d for d in (0.05, 0.02, 0.005, 0.0, -0.005, -0.02, -0.05)]
    special_results = check_special_points_at_a_equals_2_over_D(a_around)

    print()
    print("=" * 70)
    print("Summary / verdict")
    print("=" * 70)
    two_over_d = 2.0/3
    print(f"1. a_opt = {a_opt:.8f}")
    print(f"2. 2/D = {two_over_d:.8f}")
    print(f"3. Discrepancy: {(a_opt - two_over_d) / two_over_d * 100:.4f}%")
    print()
    print("If a = 2/D were forced by first principles, we would expect:")
    print("  (a) a_opt == 2/D exactly (modulo higher-loop effects)")
    print("  (b) No other base-integer rational at comparable height closer to a_opt")
    print("  (c) d(delta_x)/da = 0 at a = 2/D (extremum)")
    print()
    print("Actual findings are printed above. Interpretation is in the")
    print("accompanying write-up (docs/theory/04_coupling/EXPLR_A_OVER_D_AUDIT.md).")


if __name__ == "__main__":
    main()
