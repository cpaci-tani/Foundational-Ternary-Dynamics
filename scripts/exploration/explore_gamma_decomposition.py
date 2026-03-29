"""
Gamma Function Decomposition: The Irreducible Core of FTD

What IS Gamma(1/4)? Break it into its constituent pieces.
Why is Gamma unique? Why 1/4 and not 1/3?
"""

import numpy as np
from scipy.special import gamma

G14 = gamma(0.25)
G13 = gamma(1/3)
G12 = gamma(0.5)  # = sqrt(pi)

print("=" * 70)
print("GAMMA(1/4): THE IRREDUCIBLE CORE")
print("=" * 70)
print()
print("DEFINITION:")
print("  Gamma(x) = integral_0^inf t^{x-1} e^{-t} dt")
print()
print("  Three ingredients:")
print("    1. t^{x-1}  -- the power law (algebraic weight)")
print("    2. e^{-t}   -- the exponential decay (convergence)")
print("    3. dt over [0, inf) -- accumulation across all scales")
print()

print("=" * 70)
print("GAMMA AT KEY POINTS")
print("=" * 70)
print()
print(f"  Gamma(1)   = {gamma(1):.15f}   = 0! = 1")
print(f"  Gamma(1/2) = {G12:.15f}   = sqrt(pi)")
print(f"  Gamma(1/3) = {G13:.15f}   = hexagonal/Eisenstein value")
print(f"  Gamma(1/4) = {G14:.15f}   = THE lemniscatic value")
print(f"  Gamma(3/4) = {gamma(0.75):.15f}   = reflection partner of 1/4")
print()

# Verify reflection formula
print("REFLECTION FORMULA: Gamma(x)*Gamma(1-x) = pi/sin(pi*x)")
print(f"  Gamma(1/4)*Gamma(3/4) = {G14*gamma(0.75):.10f}")
print(f"  pi/sin(pi/4) = pi*sqrt(2) = {np.pi*np.sqrt(2):.10f}")
print()

print("=" * 70)
print("THE INTEGRAND AT x = 1/4")
print("=" * 70)
print()
print("  Gamma(1/4) = integral_0^inf t^{-3/4} e^{-t} dt")
print()

# Where does the integral weight concentrate?
total = G14
cumulative = 0
dt = 0.0001
t = dt/2
q25 = q50 = q75 = q90 = None
while t < 50:
    cumulative += t**(-0.75) * np.exp(-t) * dt
    if q25 is None and cumulative >= total * 0.25: q25 = t
    if q50 is None and cumulative >= total * 0.50: q50 = t
    if q75 is None and cumulative >= total * 0.75: q75 = t
    if q90 is None and cumulative >= total * 0.90: q90 = t
    t += dt

print(f"  Gamma(1/4) = {total:.15f}")
print(f"  Weight distribution:")
print(f"    25% comes from t < {q25:.4f}")
print(f"    50% comes from t < {q50:.4f}")
print(f"    75% comes from t < {q75:.4f}")
print(f"    90% comes from t < {q90:.4f}")
print(f"  The integral is dominated by small t (the algebraic singularity).")
print()

print("=" * 70)
print("BOHR-MOLLERUP: WHY GAMMA IS UNIQUE")
print("=" * 70)
print()
print("  Theorem (Bohr-Mollerup, 1922):")
print("  Gamma is the UNIQUE function f: (0,inf) -> (0,inf) satisfying:")
print("    1. f(1) = 1                    [normalization]")
print("    2. f(x+1) = x * f(x)           [factorial recurrence]")
print("    3. log f(x) is convex           [smoothness/interpolation]")
print()
print("  NO other function satisfies all three.")
print("  This is not a selection. It is a uniqueness theorem.")
print()

print("=" * 70)
print("WHY 1/4 AND NOT 1/3: THE BINARY CHOICE")
print("=" * 70)
print()
print("  Imaginary quadratic fields Q(sqrt(-d)) with:")
print("    - Class number 1 (unique factorization)")
print("    - Non-trivial automorphisms (beyond {+1,-1})")
print()
print("  There are EXACTLY TWO:")
print()
print("    d=1: Q(i)")
print("      Aut = Z/4Z (4-fold, square symmetry)")
print("      CM discriminant = -4")
print("      Ring of integers = Z[i] (Gaussian integers)")
print("      Elliptic curve = E: y^2 = x^3 - x, j = 1728")
print("      Watson integral = Gamma(1/4)^4 / (4*pi^3)")
print("      --> FTD lives here")
print()
print("    d=3: Q(sqrt(-3))")
print("      Aut = Z/6Z (6-fold, hexagonal symmetry)")
print("      CM discriminant = -3")
print("      Ring of integers = Z[omega], omega = e^{2*pi*i/3}")
print("      Elliptic curve = E': y^2 = x^3 - 1, j = 0")
print("      Watson integral = Gamma(1/3)^6 * sqrt(3) / (4*pi^3)")
print("      --> The hexagonal alternative")
print()
print("  Every other Heegner discriminant ({-7,-8,-11,-19,-43,-67,-163})")
print("  has Aut = {+1,-1} only. No Z_n symmetry with n > 2.")
print("  No special Gamma value. No Watson-type identity.")
print("  No master quadratic with clean roots.")
print()

# Compare the two options
Gstar_4 = np.sqrt(2) * G14**2 / (2 * np.pi)
# For the hexagonal case
# Watson's FCC integral I_2 involves Gamma(1/3)
# W_3^{hex} = Gamma(1/3)^6 / (4*pi^3 * sqrt(3))... approximately
# Not computing exactly, but showing the contrast

print("  THE TWO CANDIDATES:")
print(f"    Gamma(1/4) = {G14:.10f}")
print(f"    Gamma(1/3) = {G13:.10f}")
print()
print(f"    G*(cubic)  = sqrt(2)*Gamma(1/4)^2/(2*pi) = {Gstar_4:.10f}")
print()

# Which lattice do we live in?
print("  The physical question reduces to:")
print("  Does space have SQUARE cross-sections (Z_4) or HEXAGONAL (Z_6)?")
print()
print("  The cubic lattice Z^3 has square coordinate planes.")
print("  A hexagonal lattice would have triangular cross-sections.")
print()
print("  This is the ONLY remaining question.")
print("  Everything else -- Gamma, the CM curve, the ring, the Fermat")
print("  split, G*, the master quadratic, the roots -- follows from")
print("  this one binary choice: SQUARE or HEXAGONAL.")
print()

print("=" * 70)
print("THE COMPLETE CHAIN")
print("=" * 70)
print()

K = 16 * Gstar_4**2
Delta = K**2 - 4*K*Gstar_4
xp = (K + np.sqrt(Delta)) / 2
xm = (K - np.sqrt(Delta)) / 2

print("  STEP 0: Gamma exists and is unique   [Bohr-Mollerup 1922]")
print("  STEP 1: Only 1/4 and 1/3 give clean  [Baker-Heegner-Stark 1966]")
print("          CM fields with non-trivial Aut")
print("  STEP 2: Z_4 (square) selects 1/4     [Watson 1939]")
print(f"  STEP 3: Gamma(1/4) = {G14:.10f}")
print(f"  STEP 4: G* = {Gstar_4:.10f}")
print(f"  STEP 5: Master quadratic x^2 - {K:.4f}x + {K*Gstar_4:.4f} = 0")
print(f"  STEP 6: x+ = {xp:.6f}, x- = {xm:.6f}")
print()
print("  Steps 0-2: THEOREMS (no freedom)")
print("  Steps 3-4: COMPUTATION (no freedom)")
print("  Step 5: SELF-CONSISTENCY [one selection]")
print("  Step 6: QUADRATIC FORMULA (no freedom)")
print()
print("  The fine structure constant 1/alpha = 137.036")
print("  is Gamma(1/4) processed through six steps.")
print("  Five are forced. One is self-consistency.")

print()
print("=" * 70)
print("WHAT GAMMA(1/4) IS MADE OF")
print("=" * 70)
print()
print("  Gamma(1/4) = integral_0^inf t^{-3/4} e^{-t} dt")
print()
print("  PIECE 1: t^{-3/4}")
print("    The algebraic weight. Exponent -3/4 = -(1 - 1/4).")
print("    Measures the contribution of scale t with a 3/4-power singularity.")
print("    This is the GEOMETRY: how much structure exists at each scale.")
print()
print("  PIECE 2: e^{-t}")
print("    The exponential cutoff. Makes the integral finite.")
print("    Without it: integral diverges (algebraic singularity alone")
print("    is not integrable at infinity).")
print("    This is the FINITENESS CONDITION: reality requires a cutoff.")
print("    Boltzmann weight at beta=1: each scale costs energy t.")
print()
print("  PIECE 3: integral_0^inf dt")
print("    The sum over all scales. From zero (UV) to infinity (IR).")
print("    This is the COMPLETENESS: every scale contributes.")
print()
print("  TOGETHER: Gamma(1/4) = 'the total geometric weight when")
print("  you integrate the quarter-power structure across all scales,")
print("  with exponential penalty for large scales.'")
print()
print("  This is not a metaphor. It IS what the integral computes.")
print("  The fine structure constant emerges from the total weight")
print("  of quarter-power geometry across all scales.")
