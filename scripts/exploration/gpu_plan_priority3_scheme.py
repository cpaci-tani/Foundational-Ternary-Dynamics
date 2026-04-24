"""
GPU computation plan — Priority 3: Scheme independence of the BCC tadpole.

Computes T_latt for the same (a, m^2) under three lattice actions:
  (1) Naive BCC       : sigma = (8/a^2)(1 - cos cos cos)
  (2) Wilson-improved : sigma_W = sigma + (r/2) a^2 sigma^2           (r = 1)
  (3) Symanzik-style  : sigma_S = sigma - (a^2/12) c1 sigma^2         (c1 = 1)
                        (generic two-term improvement; sign opposite to Wilson)

Expected per the plan: all three give residuals in 9.0–10.5 ppb range,
differing from naive BCC by at most ~1 ppb. If they differ by >5 ppb,
the 9.68 ppb result is a cutoff artifact.

GPU via cupy. N = 512 per the plan.
"""
from __future__ import annotations

import math
import time

import cupy as cp


A_LAT  = 2.0 / 3.0
M_SQ   = 134.0122075418
N      = 512


def run_scheme(name, denom_fn, a=A_LAT, m_sq=M_SQ, N=N):
    t0 = time.perf_counter()
    n = cp.arange(N, dtype=cp.float64)
    k1d = (math.pi / a) * (2.0 * n / N - 1.0)
    total = cp.zeros((), dtype=cp.float64)
    # chunk kx to cap memory
    bytes_per_plane = N * N * 8
    chunk = max(1, int(4e9) // bytes_per_plane)
    for start in range(0, N, chunk):
        stop = min(N, start + chunk)
        kx = k1d[start:stop][:, None, None]
        ky = k1d[None, :, None]
        kz = k1d[None, None, :]
        sigma = (8.0 / (a * a)) * (1.0 - cp.cos(kx * a / 2.0) *
                                            cp.cos(ky * a / 2.0) *
                                            cp.cos(kz * a / 2.0))
        denom = denom_fn(sigma, a) + m_sq
        total += cp.sum(1.0 / denom)
    cp.cuda.Device().synchronize()
    T = float(total) / (N**3 * a**3)
    dt = time.perf_counter() - t0
    return T, dt


def main():
    cp.cuda.Device(0).use()
    print("=" * 74)
    print(" GPU Priority 3: Scheme independence of BCC tadpole")
    print("=" * 74)
    print(f"   a   = {A_LAT}   m^2 = {M_SQ}   N = {N}")
    print()

    # Scheme 1: naive BCC (baseline from Priority 1)
    def naive(sigma, a): return sigma

    # Scheme 2: Wilson-improved, r=1.  denom = sigma + (a^2/2) sigma^2 + m^2
    def wilson(sigma, a): return sigma + (a * a / 2.0) * sigma * sigma

    # Scheme 3: Symanzik-style, c1=1, opposite sign. denom = sigma - (a^2/12) sigma^2 + m^2
    def symanzik(sigma, a): return sigma - (a * a / 12.0) * sigma * sigma

    print(f"{'scheme':<22}  {'T_latt':>22}  {'delta vs naive':>14}  {'wall (s)':>10}")
    print("-" * 74)

    T_naive, dt = run_scheme("naive BCC", naive)
    print(f"{'naive BCC':<22}  {T_naive:>22.15f}  {'(baseline)':>14}  {dt:>10.2f}")

    T_wilson, dt = run_scheme("Wilson r=1",  wilson)
    d_wilson = T_wilson - T_naive
    print(f"{'Wilson r=1':<22}  {T_wilson:>22.15f}  {d_wilson:>+14.3e}  {dt:>10.2f}")

    T_symanzik, dt = run_scheme("Symanzik c1=1", symanzik)
    d_symanzik = T_symanzik - T_naive
    print(f"{'Symanzik c1=1':<22}  {T_symanzik:>22.15f}  {d_symanzik:>+14.3e}  {dt:>10.2f}")

    print()
    print("Interpretation:")
    # Map each T to an implied ppb residual. Using the one-loop chain:
    #   delta_phi = -I_1 / m_lat^2,  m_lat^2 = m^2 * a^2 = 134.0122 * 4/9 = 59.561
    #   delta_x = delta_phi * a = -(I_1 * a) / m_lat^2
    # We report the absolute delta_x and the shift in the residual of x_+ vs 1/alpha.
    m_lat_sq = M_SQ * A_LAT**2
    def tad_to_deltax(T): return -(T * A_LAT) / m_lat_sq
    X_PLUS_TREE = 137.036171458
    ALPHA_INV = 137.035999177
    tree_gap = X_PLUS_TREE - ALPHA_INV      # ~+1.72e-4 = 1.26 ppm

    for label, T in [("naive BCC", T_naive),
                     ("Wilson r=1", T_wilson),
                     ("Symanzik c1=1", T_symanzik)]:
        dx = tad_to_deltax(T)
        x_corrected = X_PLUS_TREE + dx
        res_ppm = (x_corrected - ALPHA_INV) / ALPHA_INV * 1e6
        res_ppb = res_ppm * 1000
        print(f"   {label:<15} T_latt={T:.9f}  delta_x={dx:+.6e}  "
              f"residual vs 1/alpha = {res_ppb:+.3f} ppb")

    # Scheme-independence verdict
    schemes_T = [T_naive, T_wilson, T_symanzik]
    max_diff = max(schemes_T) - min(schemes_T)
    # Convert max_diff to ppb on x+
    max_ppb_diff = abs(tad_to_deltax(max(schemes_T)) -
                       tad_to_deltax(min(schemes_T))) / ALPHA_INV * 1e9
    print()
    print(f"   max scheme spread in T_latt: {max_diff:+.3e}")
    print(f"   max scheme spread in ppb on x+: {max_ppb_diff:.3f} ppb")
    if max_ppb_diff < 5.0:
        print("   => Scheme spread < 5 ppb: CONSISTENT with physical-feature interpretation.")
    else:
        print("   => Scheme spread >= 5 ppb: INCONSISTENT with physical-feature claim;")
        print("      residual appears to be a cutoff artifact.")


if __name__ == "__main__":
    main()
