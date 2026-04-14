"""
Check Watson integral normalization.

Watson (1939) defined:
  W_3 = (1/(pi^3)) int_0^pi int_0^pi int_0^pi dk1 dk2 dk3 / (3 - cos k1 - cos k2 - cos k3)

The lattice Green's function at the origin:
  G(0) = (1/(2pi)^3) int_{-pi}^{pi} dk / hat_k^2
       = (1/(2pi)^3) int dk / [2(1-cos k1) + 2(1-cos k2) + 2(1-cos k3)]
       = (1/(2pi)^3) int dk / [6 - 2cos k1 - 2cos k2 - 2cos k3]
       = (1/(2pi)^3) * (1/2) int dk / [3 - cos k1 - cos k2 - cos k3]
       = (1/(2 * (2pi)^3)) * 8 * int_0^pi ... / [3 - cos k1 - cos k2 - cos k3]
       = (1/((2pi)^3)) * 4 * int_0^pi ...
       = (4/(8 pi^3)) int_0^pi ...
       = (1/(2 pi^3)) int_0^pi ...
       = W_3 / 2

Wait, let me just be careful:
  G(0) = (1/(2pi)^3) int_{BZ} dk / hat_k^2

where hat_k^2 = 2(1-cos k1) + 2(1-cos k2) + 2(1-cos k3)
             = 2[3 - cos k1 - cos k2 - cos k3]

So G(0) = (1/(2pi)^3) int dk / (2[3 - c1 - c2 - c3])
        = 1/(2*(2pi)^3) * int dk / (3 - c1 - c2 - c3)

By symmetry (even function in all ki):
  = 1/(2*(2pi)^3) * 8 * int_0^pi dk / (3-c1-c2-c3)
  = 1/(2*pi^3) * int_0^pi dk / (3-c1-c2-c3)
  = W_3 / 2

So G(0) = W_3/2 ??

No wait. Watson defined:
  W_d = (1/pi^d) int_0^pi ... / (d - sum cos ki)

So W_3 = (1/pi^3) int_0^pi^3 dk / (3-c1-c2-c3)

And G(0) = (1/(2*pi^3)) int_0^pi^3 dk / (3-c1-c2-c3) = W_3/2

Hmm but that would give G(0) = 1.393/2 = 0.697, not matching standard results.

Actually let me look up the standard result more carefully.
"""

# Phase 8b (FTD Test Bench) -- converted to PyTorch with CUDA default.
# Original NumPy path preserved as fallback when torch is unavailable.
# The hot triple Python for-loops at N=500 and the convergence sweep at
# N in [50..500] are vectorized through broadcasting reductions, with
# chunking along i1 to bound peak memory.

import os
import sys
import numpy as np
from scipy.special import gamma

_SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, _SCRIPTS_DIR)
try:
    from constants import TORCH, DEVICE, DTYPE
except ImportError:
    TORCH = None
    DEVICE = None
    DTYPE = None

print(f"[backend] device={DEVICE}, torch={TORCH is not None}")


def _watson_sc_sum_torch(N):
    """Sum of 1 / (3 - c1 - c2 - c3) over the N**3 midpoint grid (torch)."""
    dk = np.pi / N
    idx = TORCH.arange(N, device=DEVICE, dtype=DTYPE)
    c = TORCH.cos((idx + 0.5) * dk)                 # (N,)
    c23 = c.unsqueeze(0) + c.unsqueeze(1)           # (N, N)
    chunk = min(N, 128)
    total = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c1_chunk = c[start:stop].view(-1, 1, 1)     # (chunk, 1, 1)
        D = 3.0 - c1_chunk - c23.unsqueeze(0)       # (chunk, N, N)
        total = total + (1.0 / D).sum()
    return float(total.item())


def _watson_sc_sum_numpy(N):
    """Same sum, vectorized with NumPy broadcasting + chunking along i1."""
    dk = np.pi / N
    idx = np.arange(N, dtype=np.float64)
    c = np.cos((idx + 0.5) * dk)
    c23 = c[:, None] + c[None, :]                   # (N, N)
    chunk = min(N, 128)
    total = 0.0
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c1_chunk = c[start:stop].reshape(-1, 1, 1)
        D = 3.0 - c1_chunk - c23[None, :, :]        # (chunk, N, N)
        total += float(np.sum(1.0 / D))
    return total


def _watson_sc_sum(N):
    if TORCH is not None:
        return _watson_sc_sum_torch(N)
    return _watson_sc_sum_numpy(N)


Gamma14 = gamma(0.25)
print(f"Gamma(1/4) = {Gamma14:.12f}")
print(f"Gamma(1/4)^4 = {Gamma14**4:.12f}")
print(f"4*pi^3 = {4*np.pi**3:.12f}")
print(f"Gamma(1/4)^4 / (4*pi^3) = {Gamma14**4/(4*np.pi**3):.12f}")
print()

# Let me check: what is sqrt(6)/(96*pi^3) * Gamma(1/4)^4?
val = np.sqrt(6)/(96*np.pi**3) * Gamma14**4
print(f"sqrt(6)/(96*pi^3) * Gamma(1/4)^4 = {val:.12f}")
print()

# The original Watson (1939) result for the simple cubic lattice:
# I_3 = (1/pi^3) int_0^pi^3 dk / (3 - cos k1 - cos k2 - cos k3)
# = sqrt(6)/(96*pi^3) * Gamma(1/4)^4 ??
# Actually Watson showed I_3 = sqrt(6)/(96*pi^3) * Gamma(1/4)^4 ... no, let me check.

# The correct Watson result (1939) is:
# W(3) = (sqrt(6)/(96*pi^3)) * Gamma(1/4)^4 = 0.505462...
# Wait, that's not matching either.

# Let me just compute numerically
N = 500
dk = np.pi / N
total = _watson_sc_sum(N)

# Watson's integral: (1/pi^3) * integral
watson_numerical = total * dk**3 / np.pi**3
print(f"Watson integral (numerical, N={N}):")
print(f"  (1/pi^3) int_0^pi dk / (3-c1-c2-c3) = {watson_numerical:.10f}")
print()

# The lattice Green's function at origin
G0 = watson_numerical / 2
print(f"Lattice Green's function G(0) = W/2 = {G0:.10f}")
print()

# Check against known formulas
# Zucker & Joyce (2001): Watson's integral for simple cubic =
# (sqrt(6)/(96*pi^3)) * Gamma(1/4)^4  -- this is what I see in references
val1 = np.sqrt(6)/(96*np.pi**3) * Gamma14**4
print(f"sqrt(6)/(96*pi^3) * Gamma(1/4)^4 = {val1:.10f}")

# But FTD claims W_3 = Gamma(1/4)^4 / (4*pi^3)
val2 = Gamma14**4 / (4*np.pi**3)
print(f"Gamma(1/4)^4 / (4*pi^3) = {val2:.10f}")
print()

# These are different! Let me check the ratio
print(f"Ratio val2/val1 = {val2/val1:.10f}")
print(f"Ratio = 96/4 / sqrt(6) = {96/4/np.sqrt(6):.10f} = 24/sqrt(6) = {24/np.sqrt(6):.10f}")
print()

# So which one matches the numerical integral?
print(f"Numerical Watson = {watson_numerical:.10f}")
print(f"Formula 1 (sqrt(6)/96pi^3 * Gamma(1/4)^4) = {val1:.10f}")
print(f"Formula 2 (Gamma(1/4)^4 / 4pi^3)           = {val2:.10f}")
print()

# So FTD's "W_3" is NOT the standard Watson integral!
# Let me check what the standard Watson integral actually is.
# Watson (1939) computed:
#   I = (1/pi^3) int_0^pi ... 1/(3-c1-c2-c3) dk
# The numerical value should be around 1.5163...

# Ah wait, the integral DIVERGES logarithmically at k=0!
# No it doesn't -- 3-c1-c2-c3 ~ k^2/2 at small k, and dk ~ k^2 dk in 3D
# so the integrand goes as k^2/(k^2) = 1, which is fine.
# Actually wait, in 3D: int dk1 dk2 dk3 / k^2 ~ int k^2 dk / k^2 = int dk -> log divergent?
# No, int_0^L dk1 dk2 dk3 / (k1^2+k2^2+k3^2) in 3D:
# = 4pi int_0 dr r^2/r^2 = 4pi int_0 dr -> linearly divergent!

# The lattice sum IS convergent because the lattice regulates the IR, but
# the continuum BZ integral 1/hat_k^2 has the same IR divergence as 1/k^2
# as k->0, which in 3D gives a LOGARITHMIC divergence.
# Actually no: d^3k/k^2 ~ 4pi*k^2*dk/k^2 = 4pi*dk which is LINEAR divergence.

# But Watson's integral IS finite! Because 3-c1-c2-c3 > 0 everywhere except
# at k=(0,0,0), and the measure near k=0 is ~ k^2 dk while 1/(3-c1-c2-c3) ~ 2/k^2
# so the integrand ~ 2/k^2 * k^2 dk = 2 dk... wait this is LINEAR divergent.

# No! I'm confusing myself. The integral is over a CUBE [0,pi]^3, not in
# spherical coordinates. The measure in Cartesian is just dk1 dk2 dk3.
# Near k=0: integrand ~ 1/(k^2/2) = 2/k^2.
# In 3D Cartesian: int_0^eps dk1 dk2 dk3 / k^2 = int_0^eps 4*pi*r^2 dr / r^2 = 4*pi*eps
# This is LINEAR in eps, so it's FINITE. Good, the integral converges.

print(f"Comparing numerical result with watson_numerical = {watson_numerical:.10f}")
print(f"to val1 = {val1:.10f} and val2 = {val2:.10f}")
print()

# Actually, it's well known that the Watson integral for the simple cubic lattice is
# W_sc = (sqrt(6)/32pi^3) * Gamma(1/4)^4 = 1.51639...
val3 = np.sqrt(6)/(32*np.pi**3) * Gamma14**4
print(f"sqrt(6)/(32*pi^3) * Gamma(1/4)^4 = {val3:.10f}")
print()

# Hmm. Let me just look at the actual value.
print("Let me try the Watson integral in a different form.")
print("Watson (1939) showed for the simple cubic lattice:")
print("  w_s = (1/pi^3) int ... 1/(3-c1-c2-c3) dk")
print()
print(f"Numerical: {watson_numerical:.10f}")
print()

# The answer should be W_3 = 1.516386...
# Let me check with higher resolution near k=0
# Actually my N=500 should be good enough. Let me see what we actually get.

# Ah! I think the issue is that N=500 is still not enough due to the
# near-singularity at k=0. Let me check convergence.
for N in [50, 100, 200, 300, 400, 500]:
    dk = np.pi / N
    total = _watson_sc_sum(N)
    result = total * dk**3 / np.pi**3
    print(f"  N={N:4d}: {result:.10f}")
