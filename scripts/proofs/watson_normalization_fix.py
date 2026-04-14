"""
NORMALIZATION CHECK: The Watson integral with sigma(k) vs hat_k^2

FTD document (DERIV_WATSON_GSTAR_IDENTITY.md) defines two forms:

Eq (1.1): W_3 = (1/(2pi)^3) int dk / hat_k^2  where hat_k^2 = 2*sum(1-cos ki)
Eq (1.2): W_3 = (1/(2pi)^3) int dk / sigma(k)  where sigma = 1-(1/3)sum(cos ki)

These are NOT the same!
  hat_k^2 = 6 - 2*sum(cos ki) = 6*sigma(k)  ...wait, let me check:
  sigma = 1 - (1/3)(c1+c2+c3) = 1 - (c1+c2+c3)/3
  6*sigma = 6 - 2(c1+c2+c3) = hat_k^2

So sigma = hat_k^2/6, and:
  1/sigma = 6/hat_k^2

Therefore:
  (1/(2pi)^3) int dk/sigma = 6 * (1/(2pi)^3) int dk/hat_k^2

The document claims BOTH equal W_3, but they differ by a factor of 6!

Let me compute both numerically.
"""
# Phase 8b (FTD Test Bench) -- converted to PyTorch with CUDA default.
# Original NumPy path preserved as fallback when torch is unavailable.
# The hot triple Python for-loops at N=400 and the convergence sweep at
# N in [100..400] are vectorized through broadcasting reductions, with
# chunking along i1 to bound peak memory.

import os
import sys
import numpy as np
from scipy.special import gamma
from scipy.integrate import tplquad

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


def _watson_norm_sums_torch(N):
    """Sum 1/hat_k^2, 1/sigma, 1/D over the N**3 midpoint grid (torch)."""
    dk = np.pi / N
    idx = TORCH.arange(N, device=DEVICE, dtype=DTYPE)
    c = TORCH.cos((idx + 0.5) * dk)                 # (N,)
    c23 = c.unsqueeze(0) + c.unsqueeze(1)           # (N, N)
    chunk = min(N, 128)
    t_hatk2 = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    t_sigma = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    t_watson = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c_sum = c[start:stop].view(-1, 1, 1) + c23.unsqueeze(0)  # c1+c2+c3
        hatk2 = 6.0 - 2.0 * c_sum
        sigma = 1.0 - c_sum / 3.0
        D = 3.0 - c_sum
        t_hatk2 = t_hatk2 + (1.0 / hatk2).sum()
        t_sigma = t_sigma + (1.0 / sigma).sum()
        t_watson = t_watson + (1.0 / D).sum()
    return float(t_hatk2.item()), float(t_sigma.item()), float(t_watson.item())


def _watson_norm_sums_numpy(N):
    """Same three sums, vectorized NumPy + chunking along i1."""
    dk = np.pi / N
    idx = np.arange(N, dtype=np.float64)
    c = np.cos((idx + 0.5) * dk)
    c23 = c[:, None] + c[None, :]                   # (N, N)
    chunk = min(N, 128)
    t_hatk2 = 0.0
    t_sigma = 0.0
    t_watson = 0.0
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c_sum = c[start:stop].reshape(-1, 1, 1) + c23[None, :, :]  # c1+c2+c3
        hatk2 = 6.0 - 2.0 * c_sum
        sigma = 1.0 - c_sum / 3.0
        D = 3.0 - c_sum
        t_hatk2 += float(np.sum(1.0 / hatk2))
        t_sigma += float(np.sum(1.0 / sigma))
        t_watson += float(np.sum(1.0 / D))
    return t_hatk2, t_sigma, t_watson


def _watson_norm_sums(N):
    if TORCH is not None:
        return _watson_norm_sums_torch(N)
    return _watson_norm_sums_numpy(N)


def _watson_hatk2_sum_torch(N):
    """Sum 1/hat_k^2 over the N**3 midpoint grid (torch)."""
    dk = np.pi / N
    idx = TORCH.arange(N, device=DEVICE, dtype=DTYPE)
    c = TORCH.cos((idx + 0.5) * dk)
    c23 = c.unsqueeze(0) + c.unsqueeze(1)
    chunk = min(N, 128)
    total = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c_sum = c[start:stop].view(-1, 1, 1) + c23.unsqueeze(0)
        hatk2 = 6.0 - 2.0 * c_sum
        total = total + (1.0 / hatk2).sum()
    return float(total.item())


def _watson_hatk2_sum_numpy(N):
    """Sum 1/hat_k^2 over the N**3 midpoint grid (NumPy)."""
    dk = np.pi / N
    idx = np.arange(N, dtype=np.float64)
    c = np.cos((idx + 0.5) * dk)
    c23 = c[:, None] + c[None, :]
    chunk = min(N, 128)
    total = 0.0
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c_sum = c[start:stop].reshape(-1, 1, 1) + c23[None, :, :]
        hatk2 = 6.0 - 2.0 * c_sum
        total += float(np.sum(1.0 / hatk2))
    return total


def _watson_hatk2_sum(N):
    if TORCH is not None:
        return _watson_hatk2_sum_torch(N)
    return _watson_hatk2_sum_numpy(N)


Gamma14 = gamma(0.25)
G14_4 = Gamma14**4
FTD_W3 = G14_4 / (4 * np.pi**3)

print(f"FTD's W_3 = Gamma(1/4)^4/(4*pi^3) = {FTD_W3:.12f}")
print()

# Form 1: (1/(2pi)^3) int dk / hat_k^2
# hat_k^2 = 2(1-c1) + 2(1-c2) + 2(1-c3) = 6 - 2(c1+c2+c3)
def integrand_hatk2(k3, k2, k1):
    hatk2 = 2*(1-np.cos(k1)) + 2*(1-np.cos(k2)) + 2*(1-np.cos(k3))
    if hatk2 < 1e-15:
        return 0.0
    return 1.0 / hatk2

# Form 2: (1/(2pi)^3) int dk / sigma
# sigma = 1 - (c1+c2+c3)/3
def integrand_sigma(k3, k2, k1):
    sigma = 1 - (np.cos(k1)+np.cos(k2)+np.cos(k3))/3
    if sigma < 1e-15:
        return 0.0
    return 1.0 / sigma

# Using dense grid (tplquad has issues near k=0)
N = 400
dk = np.pi / N
total_hatk2, total_sigma, total_watson_orig = _watson_norm_sums(N)

# Normalize: integral over [0,pi]^3 with midpoint rule
# The full BZ is [-pi,pi]^3, so 8x the [0,pi]^3 integral
# The full BZ normalization is (2pi)^3
# So: (1/(2pi)^3) * 8 * int_0^pi = (1/pi^3) * int_0^pi

cell_vol = dk**3
octant_vol = np.pi**3

# (1/(2pi)^3) int_{full BZ} = 8/(8pi^3) int_{[0,pi]^3} = (1/pi^3) int_{[0,pi]^3}
W3_hatk2 = total_hatk2 * cell_vol / octant_vol  # = (1/pi^3) int dk / hat_k^2
W3_sigma = total_sigma * cell_vol / octant_vol  # = (1/pi^3) int dk / sigma
W3_watson = total_watson_orig * cell_vol / octant_vol  # = (1/pi^3) int dk / (3-c1-c2-c3)

print("Numerical integrals (N=400, midpoint rule over [0,pi]^3):")
print(f"  (1/pi^3) int dk / hat_k^2         = {W3_hatk2:.12f}")
print(f"  (1/pi^3) int dk / sigma            = {W3_sigma:.12f}")
print(f"  (1/pi^3) int dk / (3-c1-c2-c3)    = {W3_watson:.12f}")
print()
print(f"  Ratio sigma/hatk2 = {W3_sigma/W3_hatk2:.10f} (should be 6)")
print(f"  Ratio sigma/watson = {W3_sigma/W3_watson:.10f}")
print(f"  Ratio watson/hatk2 = {W3_watson/W3_hatk2:.10f}")
print()

# sigma = hat_k^2/6, so 1/sigma = 6/hat_k^2
# Therefore (1/pi^3) int dk/sigma = 6 * (1/pi^3) int dk/hat_k^2
# And (1/pi^3) int dk/(3-c1-c2-c3) = (1/pi^3) int dk / (hat_k^2/2) = 2*(1/pi^3) int dk/hat_k^2

# Let me verify:
# hat_k^2 = 6 - 2(c1+c2+c3)
# 3 - c1 - c2 - c3 = hat_k^2/2
# sigma = hat_k^2/6

# So: 1/sigma = 6/hat_k^2 = 3/(3-c1-c2-c3) = 3*2/hat_k^2

print("Relationships:")
print(f"  hat_k^2 = 6*sigma = 2*(3-c1-c2-c3)")
print(f"  sigma = hat_k^2/6")
print(f"  3-c1-c2-c3 = hat_k^2/2 = 3*sigma")
print()

# So:
# (1/pi^3) int dk/sigma = 6 * (1/pi^3) int dk/hat_k^2
# (1/pi^3) int dk/(3-sum ci) = 2 * (1/pi^3) int dk/hat_k^2
# (1/pi^3) int dk/sigma = 3 * (1/pi^3) int dk/(3-sum ci)

print(f"Verify: sigma_integral / hatk2_integral = {W3_sigma/W3_hatk2:.10f} (expect 6)")
print(f"Verify: watson / hatk2 = {W3_watson/W3_hatk2:.10f} (expect 2)")
print(f"Verify: sigma / watson = {W3_sigma/W3_watson:.10f} (expect 3)")
print()

# Now: which one equals Gamma(1/4)^4/(4*pi^3)?
print(f"FTD's W_3 = {FTD_W3:.12f}")
print(f"(1/pi^3) int dk / hat_k^2 = {W3_hatk2:.12f}")
print(f"(1/pi^3) int dk / sigma = {W3_sigma:.12f}")
print(f"(1/pi^3) int dk / (3-sum ci) = {W3_watson:.12f}")
print()
print(f"Ratio FTD_W3 / hatk2 = {FTD_W3/W3_hatk2:.10f}")
print(f"Ratio FTD_W3 / sigma = {FTD_W3/W3_sigma:.10f}")
print(f"Ratio FTD_W3 / watson = {FTD_W3/W3_watson:.10f}")
print()

# Hmm, FTD_W3/sigma should be close to 1 if FTD's normalization is the sigma one.
# FTD_W3 = 1.3932
# sigma integral (numerical) ~ 6 * 0.252 ~ 1.51... let me check

# Actually let me use better convergence analysis
# The midpoint rule on a 1/k^2-like integrand in 3D converges slowly

# Let me try Richardson extrapolation
vals = {}
for N in [100, 200, 300, 400]:
    dk = np.pi / N
    t_hatk2 = _watson_hatk2_sum(N)
    # These replicate the original's pattern of computing sigma and watson
    # sums from hatk2: note that in the loop above the accumulators were
    #   t_sigma += 6.0/hatk2, t_watson += 2.0/hatk2
    # so t_sigma = 6 * t_hatk2 and t_watson = 2 * t_hatk2 exactly.
    t_sigma = 6.0 * t_hatk2
    t_watson = 2.0 * t_hatk2
    cell = dk**3
    vol = np.pi**3
    vals[N] = {
        'hatk2': t_hatk2 * cell / vol,
        'sigma': t_sigma * cell / vol,
        'watson': t_watson * cell / vol
    }
    print(f"  N={N}: hatk2={vals[N]['hatk2']:.10f}  sigma={vals[N]['sigma']:.10f}  watson={vals[N]['watson']:.10f}")

# Richardson extrapolation using N=200 and N=400 (error ~ 1/N for midpoint)
for key in ['hatk2', 'sigma', 'watson']:
    v200 = vals[200][key]
    v400 = vals[400][key]
    # If error ~ C/N, then extrapolated = (400*v400 - 200*v200)/(400-200) = 2*v400 - v200
    extr = 2*v400 - v200
    print(f"  Richardson {key}: {extr:.10f}")

print()

# For the sigma normalization, the extrapolated value should be ~1.51
# But FTD claims 1.393. There must be a different normalization at play.

# Actually, wait. Let me re-read the FTD document equation (1.1):
# W_3 = (1/(2pi)^3) int_{[-pi,pi]^3} dk / hat_k^2
# This is NOT the same as (1/pi^3) int_0^pi dk / hat_k^2!
# The BZ normalization is (2pi)^3, not pi^3.

# (1/(2pi)^3) int_{[-pi,pi]^3} dk/hat_k^2
# = (1/(2pi)^3) * 8 * int_0^pi dk/hat_k^2
# = (8/(8pi^3)) * int_0^pi dk/hat_k^2
# = (1/pi^3) * int_0^pi dk/hat_k^2
# = W3_hatk2 (what I computed above)

# So FTD's definition (1.1) gives:
# W3 = (1/pi^3) int_0^pi dk/hat_k^2 ~ 0.252

# But FTD claims this equals Gamma(1/4)^4/(4*pi^3) = 1.393.
# This is a factor of ~5.5 off!

# SOMETHING IS WRONG with either the claim or my computation.

# Let me try to compute the integral more carefully using scipy.
# Actually, the integral 1/hat_k^2 has an integrable singularity at k=0 (in 3D).
# The midpoint rule might converge very slowly.

# Let me subtract the singularity.
# Near k=0: hat_k^2 ~ k^2, so 1/hat_k^2 ~ 1/k^2.
# The integral of 1/k^2 over a small sphere of radius R in 3D is:
# 4pi * int_0^R dk = 4*pi*R
# So the singularity is integrable (not logarithmic). Midpoint should converge ~ 1/N.

# But the ACTUAL convergence is slow because the integrand is large near k=0.
# Let me check: at k=(dk/2, dk/2, dk/2), hat_k^2 ~ 3*(dk/2)^2 ~ 3*pi^2/(4N^2)
# 1/hat_k^2 ~ 4N^2/(3*pi^2)
# This is O(N^2), and we sum N^3 terms each O(N^2)/(N^3 * pi^3) = O(1/N)...

# Actually the issue might be that the integral really DOES converge to 0.252
# and Watson's result Gamma(1/4)^4/(4*pi^3) = 1.393 refers to a DIFFERENT quantity.

# ACTUALLY: wait. Let me look up Watson's original paper definition.
# Watson's integral I_3 for the simple cubic lattice:
# I_3 = (1/pi^3) int_0^pi dk1 dk2 dk3 / (3 - cos k1 - cos k2 - cos k3)
# = (1/pi^3) int_0^pi dk / D(k)

# His result: I_3 = sqrt(6)/32pi^3 * Gamma(1/4)^4 * 24 ???
# Let me check:
# sqrt(6)/32pi^3 * Gamma(1/4)^4 = 0.4266
# But I numerically get I_3 ~ 0.505.

# Actually the KNOWN result from DLMF/literature:
# The Watson integral W for the SC lattice is:
# W = (1/pi^3) int dk / (3-c1-c2-c3) = sqrt(6)/(32pi^3) * Gamma(1/4)^4 ??

# Let me check: sqrt(6)/(32*pi^3) * Gamma(1/4)^4
val_check = np.sqrt(6)/(32*np.pi**3) * G14_4
print(f"\nsqrt(6)/(32*pi^3) * Gamma(1/4)^4 = {val_check:.12f}")
print(f"Watson numerical (N=400) = {vals[400]['watson']:.12f}")
print(f"Watson Richardson = {2*vals[400]['watson']-vals[200]['watson']:.12f}")
print()

# Neither matches. Let me try yet another form.
# From Joyce (2001/2002), the CORRECT result is:
# I_S = sqrt(6)/(96pi^3) * Gamma(1/4)^4 * 24 ??
# Wait, that's the same as sqrt(6)/(4pi^3) * Gamma(1/4)^4 = 3.4126

# Hmm. Let me just try all reasonable combinations:
print("Trying all sqrt(6)/C * Gamma(1/4)^4 / pi^3:")
for C in [1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96]:
    val = np.sqrt(6) * G14_4 / (C * np.pi**3)
    flag = ""
    for name, target in [("FTD", FTD_W3), ("watson_orig", 0.50546), ("hatk2", 0.2527)]:
        if abs(val - target) / target < 0.005:
            flag += f" <-- matches {name}"
    print(f"  C={C:3d}: {val:.10f}{flag}")

print()
# Also try without sqrt(6):
print("Trying Gamma(1/4)^4 / (C * pi^3):")
for C in [1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64]:
    val = G14_4 / (C * np.pi**3)
    flag = ""
    for name, target in [("FTD", FTD_W3), ("watson_orig", 0.50546), ("hatk2", 0.2527)]:
        if abs(val - target) / target < 0.005:
            flag += f" <-- matches {name}"
    print(f"  C={C:3d}: {val:.10f}{flag}")
