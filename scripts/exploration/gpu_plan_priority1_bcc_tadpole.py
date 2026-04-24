"""
GPU computation plan — Priority 1: BCC tadpole continuum-limit scaling.

Per `C:\\Users\\cpaci\\Downloads\\gpu_computation_plan.md`:

    T_latt^(N) = (1/(N^3 a^3)) sum_{k in BZ modes} 1/(khat^2_BCC(k) + m^2)

with
    a     = 2/3
    m^2   = 134.0122075418
    khat^2_BCC(k) = (8/a^2) (1 - cos(kx a/2) cos(ky a/2) cos(kz a/2))
    k_mu  in [-pi/a, pi/a)  sampled at N points per axis

Target: T_latt -> 0.02292245997... independent of N to ~12 digits for N >= 32.

GPU via cupy on the WSL2 / CUDA 13 path (FTD-0051 infrastructure).
Memory-safe: builds k-arrays by axis (1D each), uses broadcasting, does NOT
allocate the full N^3 array if it would exceed ~20 GB. At N=4096, N^3 = 6.87e10
doubles = 550 GB — too big. Must chunk over kx (outer loop) to fit.

Output: (N, T_latt, residual_from_0.02292245997, wall-time) table.
"""
from __future__ import annotations

import math
import time

import cupy as cp
import numpy as np


# ---- constants (per the plan, verbatim) -----------------------------------
A_LAT  = 2.0 / 3.0
M_SQ   = 134.0122075418
TARGET = 0.02292245997        # plan's asymptotic target


def khat_sq_bcc(kx, ky, kz, a):
    """(8/a^2)(1 - cos(kx a/2)cos(ky a/2)cos(kz a/2))."""
    return (8.0 / (a * a)) * (1.0 - cp.cos(kx * a / 2.0) *
                                      cp.cos(ky * a / 2.0) *
                                      cp.cos(kz * a / 2.0))


def bcc_tadpole(N, a=A_LAT, m_sq=M_SQ, chunk_max_bytes=int(4e9)):
    """Compute T_latt^BCC(N) on GPU via cupy, chunked over kx axis.

    BZ sampling: k_mu = (pi/a) * (2n/N - 1) for n=0..N-1.
    This gives k_mu in [-pi/a, pi/a) and avoids the IR k=0 mode inside
    the sum (we SKIP it and add the standard IR-safe contribution 0 —
    mass-gap m>0 regulates it; k=0 propagator is just 1/m^2 which
    contributes 1/(N^3 a^3 m^2); we include it explicitly).
    """
    n = cp.arange(N, dtype=cp.float64)
    k1d = (math.pi / a) * (2.0 * n / N - 1.0)   # shape (N,)

    # Chunk over kx: choose chunk_x so chunk_x * N * N * 8 bytes <= chunk_max_bytes
    bytes_per_plane = N * N * 8
    chunk_x = max(1, min(N, chunk_max_bytes // bytes_per_plane))

    total = cp.zeros((), dtype=cp.float64)
    for start in range(0, N, chunk_x):
        stop = min(N, start + chunk_x)
        kx = k1d[start:stop][:, None, None]     # (cx, 1, 1)
        ky = k1d[None, :, None]                  # (1, N, 1)
        kz = k1d[None, None, :]                  # (1, 1, N)
        khat2 = khat_sq_bcc(kx, ky, kz, a)       # (cx, N, N)
        total += cp.sum(1.0 / (khat2 + m_sq))
    cp.cuda.Device().synchronize()
    # Normalization: (1 / (N^3 a^3))  * sum
    T = float(total) / (N**3 * a**3)
    return T


def main():
    cp.cuda.Device(0).use()
    print("=" * 74)
    print(" GPU Priority 1: BCC tadpole continuum-limit scaling")
    print("=" * 74)
    print(f"   a       = {A_LAT}")
    print(f"   m^2     = {M_SQ}")
    print(f"   target  = {TARGET} (plan asymptotic)")
    print(f"   GPU     = {cp.cuda.runtime.getDeviceProperties(0)['name'].decode()}")
    print()
    print(f"{'N':>6}  {'T_latt':>24}  {'residual':>14}  {'rel.err':>12}  {'wall (s)':>10}")
    print("-" * 74)

    sizes = [32, 64, 128, 256, 512, 1024, 2048, 4096]
    results = []
    for N in sizes:
        try:
            t0 = time.perf_counter()
            T = bcc_tadpole(N)
            dt = time.perf_counter() - t0
            res = T - TARGET
            rel = res / TARGET if TARGET != 0 else float('nan')
            print(f"{N:>6}  {T:>24.18f}  {res:>+14.3e}  {rel:>+12.3e}  {dt:>10.2f}")
            results.append((N, T, res, rel, dt))
        except cp.cuda.memory.OutOfMemoryError as e:
            print(f"{N:>6}  OOM: {e}")
            break
        except Exception as e:
            print(f"{N:>6}  ERROR: {e}")
            break

    print()
    if len(results) >= 2:
        # N-independence check: pairwise differences between successive N
        print("N-independence check (successive differences):")
        for i in range(1, len(results)):
            N_a, T_a, *_ = results[i-1]
            N_b, T_b, *_ = results[i]
            dT = T_b - T_a
            print(f"   T({N_b}) - T({N_a}) = {dT:+.3e}")

    print()
    print("Interpretation:")
    if results:
        _, T_last, res_last, rel_last, _ = results[-1]
        print(f"   Largest N = {results[-1][0]}: T_latt = {T_last:.12f}")
        print(f"   Residual from plan target: {res_last:+.3e}  ({rel_last * 1e9:+.3f} ppb)")
        if abs(rel_last) < 1e-10:
            print("   => CONFIRMS the 12-digit convergence claim.")
        elif abs(rel_last) < 1e-6:
            print("   => Close to target but not 12-digit converged; may need larger N")
            print("      or different normalization / BZ convention.")
        else:
            print("   => Disagrees with plan target. Check (a) BZ convention,")
            print("         (b) normalization, (c) the plan's m^2 value.")
    return results


if __name__ == "__main__":
    main()
