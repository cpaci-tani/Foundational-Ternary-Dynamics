"""
Find the EXACT Watson integral value and resolve the normalization.

The Watson integral for the simple cubic lattice is defined as:
  W_3 = (1/pi^3) int_0^pi dk1 dk2 dk3 / (3 - cos k1 - cos k2 - cos k3)

Known results:
  Watson (1939): W_3 in terms of complete elliptic integrals
  Glasser & Zucker (1977): W_3 = sqrt(6)/(96*pi^3) * Gamma(1/4)^4 ??

Actually, looking up the CORRECT formula:
  Joyce (2002): The Watson integral for the simple cubic lattice =
  (sqrt(6)/96pi^3) * Gamma(1/4)^4 * 24 ??

Let me just check what number matches.
"""
import numpy as np
from scipy.special import gamma

Gamma14 = gamma(0.25)
G14_4 = Gamma14**4

# My numerical computation gives W_3 ~ 0.5054
# Converging to... let me extrapolate

# From N=100 to N=500:
# N=100: 0.5040713224
# N=200: 0.5047666822
# N=300: 0.5049984627
# N=400: 0.5051143523
# N=500: 0.5051838859

# Richardson extrapolation (errors go as 1/N for midpoint rule):
# W_3 ~ 0.5054...

# The exact value:
# sqrt(6)/(96*pi^3) * Gamma(1/4)^4 = 0.14219
# This is too small by a factor of about 3.55

# sqrt(6)/(32*pi^3) * Gamma(1/4)^4 = 0.42658
# Still too small

# Let me try: Gamma(1/4)^4 * sqrt(6) / (some normalization)

target = 0.5054  # approximate

for denom in [4, 8, 12, 16, 24, 32, 48, 64, 96, 192]:
    val = np.sqrt(6) * G14_4 / (denom * np.pi**3)
    print(f"  sqrt(6)*G(1/4)^4 / ({denom:3d}*pi^3) = {val:.10f}  {'<-- MATCH' if abs(val - target)/target < 0.01 else ''}")

print()

# Let me also try without sqrt(6)
for denom in range(330, 350):
    val = G14_4 / (denom)
    if abs(val - target)/target < 0.001:
        print(f"  G(1/4)^4 / {denom} = {val:.10f}  <-- MATCH")

print()

# Actually, let me look at this from the FTD perspective.
# FTD claims: W_3 = G*^2/(2pi) = Gamma(1/4)^4 / (4*pi^3)
# But our numerical integral gives W_3 ~ 0.505

# So the question is: what is the CORRECT W_3?

# Let me check: the NORMALIZED lattice Green's function is usually written as
# G(0) = (1/(2pi)^3) int dk / hat_k^2
# where hat_k^2 = sum_mu 2(1-cos k_mu)
# = (1/(2pi)^3) int dk / (2*(3-c1-c2-c3))

# With the normalization (1/(2pi)^3):
# G(0) = (1/(8*pi^3)) * 8 * int_0^pi dk / (2*(3-c1-c2-c3))
# = (1/(pi^3)) * (1/2) * int_0^pi dk / (3-c1-c2-c3)
# = W_3_numeric / 2

# So G(0) = 0.505/2 = 0.253

# Now, what does FTD ACTUALLY claim?
# From ontic.h line 143-157:
# "W_3 = G*^2/(2pi) = Gamma(1/4)^4/(4pi^3)"
# W_3 = Gamma(1/4)^4 / (4*pi^3) = 1.3932

# The STANDARD Watson integral is:
# W_sc = (1/pi^3) int dk / (3-c1-c2-c3) = 0.505...

# So FTD's "W_3" is NOT the standard Watson integral!
# The ratio: 1.3932 / 0.505 = 2.759

# Actually, let me check: maybe Watson defined it differently.
# Watson (1939) actually computed three integrals:
# I_SC = (1/pi^3) int dk / (3-c1-c2-c3) for simple cubic
# And the known exact result is:
# I_SC = sqrt(6)/(32pi^3) * Gamma(1/4)^4 * SOMETHING

# Let me compute various forms:
print("Looking for the exact formula:")
print()

# The standard result from Glasser & Zucker (1977) and Joyce (2002):
# W_SC = (sqrt(6)/(96*pi^3)) * Gamma(1/4)^4
# But this gives 0.14219, much less than 0.505

# Actually, I think the confusion is about WHICH Watson integral.
# Watson (1939) actually computed three separate lattice integrals.
# The one for the simple cubic lattice is:
# w_s = (3 * sqrt(6))/(32*pi^3) * Gamma(1/4)^4 - 1
# No that doesn't seem right either.

# Let me look at it from a different angle.
# The EXACT value of the integral I want is known:
# I = (1/pi^3) int_0^pi dk / (3-c1-c2-c3)

# I can compute this to high accuracy using the known reduction to
# a product of complete elliptic integrals.

# From Joyce (2002), the Watson integral for the simple cubic lattice:
# Watson proved that the 3D integral can be reduced to:
# I_SC = (2/pi^2) * K(k0)^2
# where k0 = (2-sqrt(3))*(sqrt(3)-sqrt(2)) ...
# Actually this is getting complicated. Let me just look at the numerical value.

# Let me use scipy quadrature
from scipy.integrate import tplquad

def integrand(k3, k2, k1):
    return 1.0 / (3 - np.cos(k1) - np.cos(k2) - np.cos(k3))

# This is slow but accurate
print("Computing Watson integral via scipy quadrature...")
result, error = tplquad(integrand, 0, np.pi, 0, np.pi, 0, np.pi)
watson_exact_numerical = result / np.pi**3
print(f"  Watson = {watson_exact_numerical:.12f} +/- {error/np.pi**3:.2e}")
print()

# Now compare with various formulas
print("Comparison with formulas:")
print(f"  Numerical: {watson_exact_numerical:.12f}")
print(f"  sqrt(6)/(96*pi^3) * G(1/4)^4 = {np.sqrt(6)/(96*np.pi**3)*G14_4:.12f}")
print(f"  sqrt(6)/(32*pi^3) * G(1/4)^4 = {np.sqrt(6)/(32*np.pi**3)*G14_4:.12f}")
print(f"  G(1/4)^4/(4*pi^3) = {G14_4/(4*np.pi**3):.12f}")
print()

# The Green's function at origin
G0 = watson_exact_numerical / 2  # because hat_k^2 = 2*(3-c1-c2-c3)
print(f"  Lattice Green's function G(0) = W/2 = {G0:.12f}")
print()

# G*^2/(2pi) = FTD's "W_3"
VARPI = 2.622057554292119810
M_GAUSS = 0.8346268416740731
G_STAR = 2 * np.sqrt(VARPI * M_GAUSS)
FTD_W3 = G_STAR**2 / (2*np.pi)
print(f"  FTD's W_3 = G*^2/(2pi) = {FTD_W3:.12f}")
print(f"  Gamma(1/4)^4/(4pi^3) = {G14_4/(4*np.pi**3):.12f}")
print()

# What is the RATIO?
ratio = FTD_W3 / watson_exact_numerical
print(f"  FTD_W3 / Watson_numerical = {ratio:.10f}")
print(f"  This is approximately {ratio:.4f}")
print(f"  2*pi/4 = {2*np.pi/4:.10f}")
print(f"  pi/2 = {np.pi/2:.10f}")
print()

# So FTD's "W_3" = Watson_numerical * (some factor)
# Let me find what factor
# FTD_W3 = G(1/4)^4 / (4*pi^3)
# Watson = (1/pi^3) int / (3-c1-c2-c3)
# So FTD_W3 / Watson = G(1/4)^4 / (4 * Watson_numerical * pi^3 / 1)
# Hmm, let me just check ratios

print("Key ratios:")
print(f"  FTD_W3 / Watson = {FTD_W3 / watson_exact_numerical:.10f}")
print(f"  FTD_W3 * 2 / Watson = {FTD_W3 * 2 / watson_exact_numerical:.10f}")
print(f"  Watson / G0 = {watson_exact_numerical / G0:.10f}")  # should be 2
print()

# Actually, let me check the IDENTITY claimed in ontic.h:
# W_3 = Gamma(1/4)^4 / (4*pi^3)
# But the standard Watson integral is different.
# Let me check if Gamma(1/4)^4 / (4*pi^3) is actually some OTHER lattice quantity.

# Recall: Gamma(1/4)^4 = 16 * pi * M^2 * sqrt(pi) / ...
# Actually: varpi = Gamma(1/4)^2 / (2*sqrt(2*pi))
# So Gamma(1/4)^2 = 2*varpi*sqrt(2*pi)
# Gamma(1/4)^4 = 4*varpi^2 * 2*pi = 8*pi*varpi^2
# Then: Gamma(1/4)^4 / (4*pi^3) = 8*pi*varpi^2 / (4*pi^3) = 2*varpi^2/pi^2

val = 2*VARPI**2/np.pi**2
print(f"  2*varpi^2/pi^2 = {val:.12f}")
print(f"  FTD W_3 = {FTD_W3:.12f}")
print(f"  Match: {abs(val-FTD_W3):.2e}")
print()

# So FTD's "W_3" = 2*varpi^2/pi^2.
# The ACTUAL Watson integral W_sc ~ 0.5054...
# And G(0) = W_sc / 2 ~ 0.2527...

# FTD's W_3 / G(0) = 1.393 / 0.253 = 5.51
# FTD's W_3 / W_sc = 1.393 / 0.505 = 2.76

print(f"  FTD_W3 / G(0) = {FTD_W3/G0:.10f}")
print(f"  FTD_W3 / W_sc = {FTD_W3/watson_exact_numerical:.10f}")
print()

# CONCLUSION: FTD's "W_3" is NOT the Watson integral!
# It IS Gamma(1/4)^4/(4*pi^3), and this IS related to the lattice,
# but it's a different quantity.

# Actually wait. Let me re-examine. The Watson integral is ALSO written as:
# W_SC = (1/pi^3) * int_0^pi dk1 dk2 dk3 / (3 - cos k1 - cos k2 - cos k3)
#       = (1/pi^3) * int_0^pi dk / D(k)

# The KNOWN exact value from Borwein et al:
# Watson (1939) showed:
# W_FCC = 1/(4*pi^3) * Gamma(1/4)^4  [for the FCC lattice, NOT simple cubic!]

# Let me check: is Gamma(1/4)^4/(4*pi^3) the Watson integral for FCC?
# The FCC integral is:
# W_FCC = (1/pi^3) int_0^pi dk / (4 - cos k1 cos k2 - cos k1 cos k3 - cos k2 cos k3)

def fcc_integrand(k3, k2, k1):
    c1, c2, c3 = np.cos(k1), np.cos(k2), np.cos(k3)
    return 1.0 / (4 - c1*c2 - c1*c3 - c2*c3)

# Actually, the denominator for FCC should be different.
# Watson classified three types:
# I_F (face-centered) with denominator 3-cos(y)cos(z)-cos(z)cos(x)-cos(x)cos(y)
# I_B (body-centered)
# I_S (simple cubic)

print("Computing FCC Watson integral...")
result_fcc, error_fcc = tplquad(fcc_integrand, 0, np.pi, 0, np.pi, 0, np.pi)
watson_fcc = result_fcc / np.pi**3
print(f"  Watson FCC: {watson_fcc:.12f}")
print(f"  Gamma(1/4)^4/(4*pi^3) = {G14_4/(4*np.pi**3):.12f}")
print(f"  Match FCC? {abs(watson_fcc - G14_4/(4*np.pi**3)):.2e}")
print()

# AHAA! Let me check if the FCC Watson IS Gamma(1/4)^4/(4*pi^3)
# Watson (1939) proved:
# I_F = (1/pi^3) * integral = Gamma(1/4)^4 / (4*pi^3) * (something)

# Actually Watson's result for I_F is:
# I_F = 1/(4*pi^3) * Gamma(1/4)^4 ?? Let me see.
# From Watson (1939): I_F = sqrt(2) * K(k')^2 / pi where k' = sin(pi/12)
# Hmm, this is getting complicated. Let me just check numerically.

# If watson_fcc matches G14_4/(4*pi^3), that's very significant.

# For completeness, check the BCC Watson integral too:
# BCC denominator: 3 - cos(x)cos(y)cos(z) - ... wait, BCC has a different structure.
# Watson's I_B: denominator = 3 - cos(x+y)cos(z) - cos(x-y)cos(z) - cos(x)cos(y)cos(z)*0...
# Actually: I_B uses 3 - cos(y-z)cos(x) - cos(z-x)cos(y) - cos(x-y)cos(z)

def bcc_integrand(k3, k2, k1):
    return 1.0 / (3 - np.cos(k2-k3)*np.cos(k1) - np.cos(k3-k1)*np.cos(k2) - np.cos(k1-k2)*np.cos(k3))

print("Computing BCC Watson integral...")
result_bcc, error_bcc = tplquad(bcc_integrand, 0, np.pi, 0, np.pi, 0, np.pi)
watson_bcc = result_bcc / np.pi**3
print(f"  Watson BCC: {watson_bcc:.12f}")
print()

print("Summary of Watson integrals:")
print(f"  SC  (simple cubic):     {watson_exact_numerical:.12f}")
print(f"  FCC (face-centered):    {watson_fcc:.12f}")
print(f"  BCC (body-centered):    {watson_bcc:.12f}")
print(f"  Gamma(1/4)^4/(4*pi^3): {G14_4/(4*np.pi**3):.12f}")
print()

# Check if any of these is 1/2 * Gamma(1/4)^4/(4*pi^3) etc.
print("Comparing each to G(1/4)^4/(4*pi^3):")
for name, val in [("SC", watson_exact_numerical), ("FCC", watson_fcc), ("BCC", watson_bcc)]:
    ratio = G14_4/(4*np.pi**3) / val
    print(f"  {name}: ratio = {ratio:.10f}")
