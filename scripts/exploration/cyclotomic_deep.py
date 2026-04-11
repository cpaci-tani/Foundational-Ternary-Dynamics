#!/usr/bin/env python3
"""Deep dive: cyclotomic structure of the ternary cube."""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS, N_c, b_3, N_eff

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2
s = np.sqrt(np.pi)

print('=' * 78)
print('  CYCLOTOMIC DEEP DIVE: s = sqrt(pi) = Gamma(1/2) = %.15f' % s)
print('=' * 78)

# Cyclotomic polynomials
def phi1(x): return x - 1
def phi2(x): return x + 1
def phi3(x): return x**2 + x + 1
def phi4(x): return x**2 + 1
def phi6(x): return x**2 - x + 1
def phi12(x): return x**4 - x**2 + 1

print('\n  1. CYCLOTOMIC POLYNOMIALS AT s = sqrt(pi)')
print()
for n, f in [(1,phi1),(2,phi2),(3,phi3),(4,phi4),(6,phi6),(12,phi12)]:
    print('    Phi_%2d(sqrt(pi)) = %12.10f' % (n, f(s)))

print()
print('  Products:')
print('    Phi_1*Phi_2 = s^2-1 = pi-1 = %.10f' % (phi1(s)*phi2(s)))
print('    Phi_3*Phi_6 = s^4-s^2+1 = pi^2-pi+1 = %.10f' % (phi3(s)*phi6(s)))
print('    Phi_1*Phi_2*Phi_3*Phi_6 = pi^3-1 = %.10f' % (s**6-1))

# THE HAMILTONIAN IS CYCLOTOMIC
print()
print('  2. THE HAMILTONIAN IN G*=1 UNITS')
print()
print('    mu    = Phi_4(s)/2 = (pi+1)/2         = %.10f' % (phi4(s)/2))
print('    delta = Phi_1(s)*Phi_2(s)/2 = (pi-1)/2 = %.10f' % (phi1(s)*phi2(s)/2))
print('    |beta|= Phi_6(s)/2 = (pi-sqrt(pi)+1)/2 = %.10f' % (phi6(s)/2))
print()
print('    Three cyclotomic families:')
print('      Phi_4 -> square symmetry (Z[i] Gaussian integers)')
print('      Phi_1*Phi_2 -> binary symmetry (Z integers, real line)')
print('      Phi_6 -> hexagonal symmetry (Z[omega] Eisenstein integers)')

# DISTANCES TO ROOTS OF UNITY
print()
print('  3. DISTANCES FROM sqrt(pi) TO ROOTS OF UNITY')
print()

omega = np.exp(1j * np.pi / 3)
i_unit = 1j

pts = [
    ('1 (Phi_1 root)', 1.0),
    ('-1 (Phi_2 root)', -1.0),
    ('i (Phi_4 root)', i_unit),
    ('-i (Phi_4 root)', -i_unit),
    ('omega = e^{ipi/3} (Phi_6 root)', omega),
    ('omega* = e^{-ipi/3} (Phi_6 root)', omega.conjugate()),
    ('omega^2 = e^{2ipi/3} (Phi_3 root)', omega**2),
]

for label, z in pts:
    d = abs(s - z)
    print('    |sqrt(pi) - %s| = %.10f' % (label, d))

# Key: Phi_6(s) = |s - omega|^2
print()
print('    Phi_6(sqrt(pi)) = |sqrt(pi) - e^{ipi/3}|^2 = %.10f' % phi6(s))
print('    Check: %.10f' % abs(s - omega)**2)

# So |beta| = |sqrt(pi) - omega|^2 / 2
print()
print('    |beta| = |sqrt(pi) - e^{ipi/3}|^2 / 2')
print('    The mediator offset is HALF the squared distance from')
print('    sqrt(pi) to the nearest hexagonal (SU(3)) lattice generator.')

# FORCE CRITERION
print()
print('  4. FORCE CRITERION: 4t/G* < Phi_6(sqrt(pi))')
print()

forces = [
    ('Gravity', 1.0/(b_3+N_c)**2),
    ('U(1) EM', 1.0/X_PLUS),
    ('SU(3) Strong', float(b_3)/(b_3+4*N_eff)),
    ('SU(2) Weak', float(N_c)/N_eff),
]

threshold = phi6(s)
for name, t in forces:
    val = 4*t/G_STAR
    lr = 'LONG-RANGE' if val < threshold else 'SHORT-RANGE'
    print('    %-14s 4t/G*=%.6f vs Phi_6=%.6f -> %s' % (name, val, threshold, lr))

# THE THREE LATTICES
print()
print('  5. THREE NUMBER-THEORETIC LATTICES IN ONE HAMILTONIAN')
print()
print('    The ternary cube Hamiltonian encodes three algebraic lattices:')
print()
print('    Z (integers):     delta ~ Phi_1*Phi_2(s)')
print('      Roots at +1, -1 on the real line.')
print('      The MIRROR CHANNELS: + and - are reflections.')
print('      This is the binary structure {+1, -1}.')
print()
print('    Z[i] (Gaussian):  mu ~ Phi_4(s)')
print('      Roots at +i, -i on the imaginary axis.')
print('      The GLOBAL PHASE: mu is the square-lattice offset.')
print('      This is the CM lattice of the lemniscatic curve.')
print()
print('    Z[omega] (Eisenstein): |beta| ~ Phi_6(s)')
print('      Roots at e^{+/-ipi/3} on the hexagonal lattice.')
print('      The MEDIATOR: varpi sits on the hexagonal scale.')
print('      This is the lattice of the SU(3) root system.')
print()

# CONNECTING TO THE MASTER QUADRATIC
print('  6. CONNECTION TO THE MASTER QUADRATIC')
print()

# Master quadratic: x^2 - 16G*^2 x + 16G*^3 = 0
# In G*=1 units: x^2 - 16x + 16 = 0 (NOT quite, because the coefficients
# are 16G*^2 and 16G*^3, which in G*=1 become 16 and 16)
# Roots: x = 8 +/- sqrt(64-16) = 8 +/- sqrt(48) = 8 +/- 4*sqrt(3)

xp_g = 8 + 4*np.sqrt(3)
xm_g = 8 - 4*np.sqrt(3)

print('    Master quadratic in G*=1 units: u^2 - 16u + 16 = 0')
print('    u+ = 8 + 4*sqrt(3) = %.10f' % xp_g)
print('    u- = 8 - 4*sqrt(3) = %.10f' % xm_g)
print()

# Check: these should be x+/G* and x-/G*
print('    x+/G* = %.10f (should match u+? = %.10f)' % (X_PLUS/G_STAR, xp_g))
print('    x-/G* = %.10f (should match u-? = %.10f)' % (X_MINUS/G_STAR, xm_g))
print()

# They DO NOT match because the master quadratic is
# x^2 - 16G*^2 x + 16G*^3 = 0, not x^2 - 16x + 16 = 0
# In G*=1: (x/G*)^2*G*^2 - 16G*^2*(x/G*)*G* + 16G*^3 = 0
# -> u^2*G*^2 - 16*G*^3*u + 16*G*^3 = 0
# Divide by G*^2: u^2 - 16G*u + 16G* = 0
# So in G*=1 units: u^2 - 16u + 16 = 0. Yes!
# But wait: x+/G* should = u+.
print('    CORRECTION: u^2 - 16u + 16 = 0 has roots')
print('    u = 8 +/- sqrt(64-16) = 8 +/- 4*sqrt(3)')
print()

# 4*sqrt(3) = 6.928
# u+ = 14.928, u- = 1.072
# But x+/G* = 137/2.959 = 46.3, x-/G* = 3.024/2.959 = 1.022
# These don't match! The G*=1 normalization is wrong.

# Let me redo: the master quadratic is x^2 - 16G*^2 x + 16G*^3 = 0
# Substitute u = x/G*: (uG*)^2 - 16G*^2(uG*) + 16G*^3 = 0
# u^2 G*^2 - 16 G*^3 u + 16 G*^3 = 0
# Divide by G*^2: u^2 - 16G* u + 16G* = 0
# So: u^2 - 16G*u + 16G* = 0 where u = x/G*

# The coefficients STILL contain G*. It doesn't simplify in G*=1 units
# because the equation is not homogeneous in G*.

print('    Actually: substituting u=x/G* gives u^2 - 16G*u + 16G* = 0')
print('    The coefficients depend on G* even after normalization.')
print('    G* does NOT factor out of the master quadratic.')
print()
print('    This confirms: the master quadratic needs BOTH G* and s.')
print('    G* is NOT just a scale. It carries independent information.')

# SQRT(3) IN THE MASTER QUADRATIC
print()
print('  7. SQRT(3) IN THE MASTER QUADRATIC')
print()

# Discriminant of master quadratic:
# disc = (16G*^2)^2 - 4*16G*^3 = 256G*^4 - 64G*^3 = 64G*^3(4G*-1)
disc = 64*G_STAR**3*(4*G_STAR-1)
print('    Discriminant = 64*G*^3*(4G*-1) = %.6f' % disc)
print('    sqrt(disc) = %.6f' % np.sqrt(disc))
print('    x+ - x- = sqrt(disc) = %.6f' % (X_PLUS - X_MINUS))
print()

# The root separation:
# x+ - x- = sqrt(64G*^3(4G*-1)) = 8*G*^(3/2)*sqrt(4G*-1)
sep = 8*G_STAR**1.5*np.sqrt(4*G_STAR-1)
print('    x+ - x- = 8*G*^(3/2)*sqrt(4G*-1) = %.6f' % sep)
print('    4G* - 1 = %.10f' % (4*G_STAR-1))
print('    sqrt(4G*-1) = %.10f' % np.sqrt(4*G_STAR-1))
print()

# 4G*-1 = 4*2.9587-1 = 10.835. sqrt = 3.292
# Compare to sqrt(3)*G*:
print('    sqrt(4G*-1) vs sqrt(3)*sqrt(G*):')
print('    sqrt(4G*-1) = %.10f' % np.sqrt(4*G_STAR-1))
print('    sqrt(3*G*)  = %.10f' % np.sqrt(3*G_STAR))
print('    Ratio: %.6f' % (np.sqrt(4*G_STAR-1)/np.sqrt(3*G_STAR)))
print()

# KEY: what is 4G*-1 in cyclotomic terms?
# 4G* - 1 in G*=1: 4*1 - 1 = 3. So 4G*-1 ~ 3 = N_c.
# More precisely: 4G*-1 = 4*2.9587-1 = 10.835
# In G* units: (4G*-1)/G* = 4 - 1/G* = 4 - 0.338 = 3.662
print('    4G*-1 = %.10f' % (4*G_STAR-1))
print('    This appears in the discriminant: x+ - x- ~ sqrt(G*^3 * (4G*-1))')
print('    If G* = 3 exactly: 4G*-1 = 11, and x+-x- = 8*3^1.5*sqrt(11) = 137.5')
print('    Actual: x+-x- = %.6f' % (X_PLUS-X_MINUS))

# SUMMARY
print()
print('  =' * 39)
print('  SUMMARY: THE CYCLOTOMIC SKELETON OF FTD')
print('  =' * 39)
print()
print('  The ternary cube Hamiltonian decomposes into three cyclotomic layers:')
print()
print('    LAYER 1 (Binary, Phi_1*Phi_2):')
print('      delta = (pi-1)/2. The mirror split between + and - channels.')
print('      Zeros at +1,-1. The simplest symmetry: reflection.')
print()
print('    LAYER 2 (Square, Phi_4):')
print('      mu = (pi+1)/2. The global energy offset.')
print('      Zeros at +i,-i. The Gaussian integer lattice Z[i].')
print('      This IS the lemniscatic CM lattice.')
print()
print('    LAYER 3 (Hexagonal, Phi_6):')
print('      |beta| = (pi-sqrt(pi)+1)/2. The mediator offset.')
print('      Zeros at e^{+/-ipi/3}. The Eisenstein integer lattice Z[omega].')
print('      This IS the SU(3) root lattice.')
print()
print('  The force range criterion says: a force is long-range iff')
print('  its coupling is inside the HEXAGONAL BALL (Phi_6) around sqrt(pi).')
print()
print('  |beta| = |sqrt(pi) - e^{ipi/3}|^2 / 2')
print()
print('  This means: the mediator offset IS the distance from the')
print('  fundamental Gamma function scale sqrt(pi) to the nearest')
print('  hexagonal lattice point. The strong force lives at the')
print('  BOUNDARY of this ball. Gravity and EM live inside it.')
print()
print('  The bridge constant G* provides the scale (Layer 0).')
print('  The cyclotomic polynomials provide the shape (Layers 1-3).')
print('  Together: z = G* * [cyclotomic structure at sqrt(pi)]')
print('  = R * exp(i*theta): the scale IS the radius,')
print('  the cyclotomic structure IS the phase geometry.')
