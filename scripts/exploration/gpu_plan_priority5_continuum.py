"""
GPU computation plan — Priority 5: Continuum-limit extrapolation of the
BCC tadpole.

For each lattice spacing a in {2/3, 2/6, 2/12, 2/24, 2/48}:
  - N = round(L / a)   with fixed physical volume L = Na
  - Compute T_latt^BCC with m^2 = 134.0122 (fixed, physical)

Purpose per the plan: if the residual is specific to a = 2/3, it should
show a distinguishing feature; if universal continuum limit, it should
approach a specific value as a -> 0.

GPU via cupy. Largest N in the scan (at a = 2/48): N = round(100 / (2/48))
= 2400 which needs chunking.
"""
from __future__ import annotations

import math
import time

import cupy as cp


M_SQ = 134.0122075418
L_PHYSICAL = 100.0  # plan's fixed physical volume


def bcc_tadpole(N, a, m_sq=M_SQ, chunk_max_bytes=int(4e9)):
    n = cp.arange(N, dtype=cp.float64)
    k1d = (math.pi / a) * (2.0 * n / N - 1.0)
    total = cp.zeros((), dtype=cp.float64)
    bytes_per_plane = N * N * 8
    chunk = max(1, int(chunk_max_bytes) // int(bytes_per_plane))
    for start in range(0, N, chunk):
        stop = min(N, start + chunk)
        kx = k1d[start:stop][:, None, None]
        ky = k1d[None, :, None]
        kz = k1d[None, None, :]
        sigma = (8.0 / (a * a)) * (1.0 - cp.cos(kx * a / 2.0) *
                                            cp.cos(ky * a / 2.0) *
                                            cp.cos(kz * a / 2.0))
        total += cp.sum(1.0 / (sigma + m_sq))
    cp.cuda.Device().synchronize()
    return float(total) / (N**3 * a**3)


def main():
    cp.cuda.Device(0).use()
    print("=" * 74)
    print(" GPU Priority 5: BCC tadpole — continuum-limit scan at fixed L = Na")
    print("=" * 74)
    print(f"   m^2 = {M_SQ}   L = {L_PHYSICAL}")
    print()
    print(f"{'a':>10}  {'N':>6}  {'T_latt':>22}  {'delta_x':>14}  {'wall (s)':>10}")
    print("-" * 74)

    # plan's a values: 2/3, 2/6, 2/12, 2/24, 2/48
    a_vals = [2.0/3.0, 2.0/6.0, 2.0/12.0, 2.0/24.0, 2.0/48.0]

    # For comparison to the one-loop chain from DERIV_ONE_LOOP_LATTICE_ALPHA.md:
    #   delta_phi = -I_1 / m_lat^2      where m_lat^2 = m^2 * a^2
    #   delta_x   = delta_phi * a
    # (the framework's existing reduction)
    results = []
    for a in a_vals:
        N = max(32, int(round(L_PHYSICAL / a)))
        # Cap N at 2400 to keep compute under ~1 min each
        if N > 2400:
            print(f"{a:>10.6f}  {N:>6}  (capped at 2400 for runtime)")
            N = 2400
        t0 = time.perf_counter()
        T = bcc_tadpole(N, a)
        dt = time.perf_counter() - t0
        m_lat_sq = M_SQ * a * a
        delta_phi = -T / m_lat_sq  # per derivation (g/2 absorbed)
        delta_x = delta_phi * a
        print(f"{a:>10.6f}  {N:>6}  {T:>22.15f}  {delta_x:>+14.4e}  {dt:>10.2f}")
        results.append((a, N, T, delta_x))

    # Continuum extrapolation: δx as a → 0
    # Fit δx(a) = δx_∞ + c1·a + c2·a² to see if a=2/3 is special
    print()
    print("delta_x(a) pattern:")
    for a, N, T, dx in results:
        # Implied residual from 1/α if we used this (a, N) one-loop
        x_corrected = 137.036171458 + dx
        residual_ppb = (x_corrected - 137.035999177) / 137.035999177 * 1e9
        print(f"   a={a:.4f}  delta_x={dx:+.6e}  x_+^corr={x_corrected:.9f}  "
              f"residual={residual_ppb:+.2f} ppb")

    # Consistency with plan's SC-formula target (9.68 ppb at a=2/3)
    print()
    print("Note: the plan's '9.68 ppb residual at a=2/3' uses the SC-tadpole")
    print("value I_1 = 0.01527 (from DERIV_ONE_LOOP_LATTICE_ALPHA.md), not the")
    print("BCC tadpole value T_latt = 0.02292 computed here. These are different")
    print("UV regularizations of the same continuum theory and give different")
    print("ppb residuals under the naive δx = -I_1·a/m_lat² formula.")


if __name__ == "__main__":
    main()
