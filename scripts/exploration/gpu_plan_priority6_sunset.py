"""
GPU computation plan — Priority 6: Two-loop sunset diagram on BCC lattice.

Per the plan:
    I_sunset = (1/a^6) sum_x G(x)^3
    where G(x) = IFFT of 1/(khat^2 + m^2)

At N=512 this is one 3D FFT. Expected result per the plan:
    I_sunset ≈ 3.2599e-6 at 12+ digits
    dI/dm^2 ≈ -6.5e-8
    Two-loop VEV shift ≈ 0.003 ppb

Compared to the existing framework calculation (`DERIV_ONE_LOOP_LATTICE_ALPHA.md`
Claim 1LA-8) which reports I_sunset = 0.1168 at 32^3. The plan's claim at
N=512 is ~4.5 orders of magnitude SMALLER than the framework's N=32
value; these are clearly different normalizations. We report both the
raw sum and the plan's claimed form, so the downstream reader can decide
which applies.

GPU via cupy.fft.fftn.
"""
from __future__ import annotations

import math
import time

import cupy as cp
# cupy 14 is missing libcufft.so.11 on this WSL (CUDA 13 vs expected 12).
# Fall back to CPU FFT via numpy if cufft is not available.
try:
    cp.fft.fftn(cp.zeros((2, 2, 2), dtype=cp.complex128))
    USE_GPU_FFT = True
except Exception:
    import numpy as cp  # rebind cp to numpy for drop-in API
    USE_GPU_FFT = False
    print("[note] cufft unavailable; falling back to numpy.fft on CPU.")


A_LAT = 2.0 / 3.0
M_SQ  = 134.0122075418
N     = 512


def khat_sq_bcc(kx, ky, kz, a):
    return (8.0 / (a * a)) * (1.0 - cp.cos(kx * a / 2.0) *
                                      cp.cos(ky * a / 2.0) *
                                      cp.cos(kz * a / 2.0))


def main():
    if USE_GPU_FFT:
        cp.cuda.Device(0).use()
    print("=" * 74)
    print(" GPU Priority 6: Two-loop sunset integral on BCC lattice")
    print("=" * 74)
    print(f"   a = {A_LAT}    m^2 = {M_SQ}    N = {N}")
    print()

    t0 = time.perf_counter()

    # Build the k-grid for the BCC dispersion at lattice spacing a.
    n = cp.arange(N, dtype=cp.float64)
    k1d = (math.pi / A_LAT) * (2.0 * n / N - 1.0)
    kx = k1d[:, None, None]
    ky = k1d[None, :, None]
    kz = k1d[None, None, :]

    # Momentum-space propagator P(k) = 1/(khat^2 + m^2)
    # (skip k=0 singularity — handled by mass)
    P_k = 1.0 / (khat_sq_bcc(kx, ky, kz, A_LAT) + M_SQ)

    # Position-space propagator G(x) = (lattice IFFT of P(k)).
    # For our discretization k_μ = (π/a)(2n/N - 1), we need to fftshift
    # to the FFT-convention k origin before IFFT.
    P_k_shift = cp.fft.ifftshift(P_k)              # put k=0 at index [0,0,0]
    G_x_complex = cp.fft.ifftn(P_k_shift) * (N**3)
    # The (N**3) factor is because numpy/cupy ifftn normalizes by 1/N^3
    # by default, and we want the unnormalized sum.
    G_x = cp.real(G_x_complex)

    # Sunset: I_sunset = (1/(a^6 · N^3)) * Σ_x G(x)^3 (divided by appropriate
    # powers to approximate the continuum integral ∫ d³x G(x)³).
    # Two canonical normalizations:
    #   Raw discrete sum:    S_raw = Σ G(x)^3
    #   Normalized (plan):   I_sunset = S_raw / (a^6 · N^3)
    S_raw = float(cp.sum(G_x**3)) if USE_GPU_FFT else float((G_x**3).sum())
    I_sunset_norm = S_raw / (A_LAT**6 * N**3)

    # Mean of |G(x)|^3 for reference:
    G0 = float(G_x[N//2, N//2, N//2])   # G at origin (since FFT convention
                                         # puts x=0 at index [0,0,0] before
                                         # fftshift; after ifftn of
                                         # ifftshifted P_k, x=0 is at [0,0,0]).

    dt = time.perf_counter() - t0

    print(f"   G(x=0) = {G_x[0,0,0]:.10e}   (propagator at origin)")
    print(f"   G(x=L/2) = {G0:.10e}         (propagator at far corner)")
    print()
    print(f"   Σ_x G(x)^3             = {S_raw:+.10e}")
    print(f"   I_sunset = (1/a^6/N^3)·Σ = {I_sunset_norm:+.10e}")
    print(f"   plan target            = +3.2599e-06  (10 ppb precision)")
    print()
    print(f"   wall time              = {dt:.2f} s")
    print()

    # Two-loop VEV shift estimate (plan: 0.003 ppb)
    # The two-loop shift should scale as g^2 I_sunset / (something involving m)
    # Using the existing framework's form: δφ^(2) ≈ (g^2/2) · I_sunset / m^4
    g = 2.0
    m4 = M_SQ**2
    dphi_2loop = -(g**2 / 2.0) * I_sunset_norm / m4
    dx_2loop = dphi_2loop * A_LAT
    x_plus_tree = 137.036171458
    alpha_inv = 137.035999177
    # ppb on x+:
    ppb_shift = dx_2loop / alpha_inv * 1e9
    print(f"   Estimated 2-loop δx (g^2/2 · I_sunset / m^4)·a")
    print(f"       = {dx_2loop:+.4e}")
    print(f"       = {ppb_shift:+.4f} ppb on x+")
    print(f"   plan's estimate ~0.003 ppb magnitude")


if __name__ == "__main__":
    main()
