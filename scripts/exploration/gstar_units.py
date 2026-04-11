#!/usr/bin/env python3
"""
G* Natural Units: a universal measurement system from G*
=========================================================

Pi gives us one invariant: the ratio of circumference to diameter.
Any civilization with circles discovers pi.

G* gives us a SECOND invariant: the ratio of distinction to curvature.
Any civilization with the Gamma function discovers G*.

Together they define a complete system of units.
"""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma as gammafn
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS,
                       N_c, N_base, b_3, N_eff, GAMMA_QUARTER, GAMMA_HALF)

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2

print('=' * 78)
print('  G* NATURAL UNITS: INVARIANTS FOR ANY CIVILIZATION')
print('=' * 78)

# =====================================================================
# WHAT MAKES A UNIT INVARIANT?
# =====================================================================
print("""
  WHAT MAKES A UNIT INVARIANT?

  An invariant unit must be:
    1. Derivable from mathematics alone (no physical measurement)
    2. The same for any civilization in any universe
    3. Expressible as a finite computation or integral
    4. Not dependent on conventions (meter, second, kg)

  Pi satisfies all four: any being that can draw a circle gets 3.14159...
  It requires only the concept of DISTANCE and CURVATURE.

  G* satisfies all four: any being that can evaluate the Gamma function
  at z=1/4 and z=3/4 gets 2.95868...
  It requires only the concept of FACTORIAL and QUARTER.

  Pi needs geometry. G* needs combinatorics.
  Pi comes from SPACE. G* comes from COUNTING.
""")

# =====================================================================
# THE FIVE INVARIANT CONSTANTS
# =====================================================================
print('  THE FIVE INVARIANT CONSTANTS')
print()

s = np.sqrt(np.pi)

constants = [
    ('pi', np.pi,
     'Gamma(1/2)^2',
     'The cost of curvature. Circle area / radius^2.'),
    ('G*', G_STAR,
     'Gamma(1/4)/Gamma(3/4)',
     'The cost of distinction. Lemniscate arc / sqrt(packing).'),
    ('varpi', VARPI_CLASSICAL,
     'Gamma(1/4)^2 / (2*sqrt(2)*Gamma(1/2))',
     'The lemniscate constant. Half-period of the figure-eight.'),
    ('e', np.e,
     'sum 1/n! = lim(1+1/n)^n',
     'The rate of growth. Natural exponential base.'),
    ('phi', (1+np.sqrt(5))/2,
     '(1+sqrt(5))/2',
     'The ratio of self-similarity. Golden ratio.'),
]

print('  %-8s %-18s %-40s' % ('Symbol', 'Value', 'Definition'))
print('  ' + '-' * 68)
for sym, val, defn, desc in constants:
    print('  %-8s %-18.15f %s' % (sym, val, defn))
    print('  %8s %18s %s' % ('', '', desc))
    print()

# =====================================================================
# THE G* UNIT SYSTEM
# =====================================================================
print('  THE G* UNIT SYSTEM: FIVE BASE UNITS FROM MATHEMATICS')
print()

print('  In the G* system, everything is measured relative to G*:')
print()
print('  %-25s %-15s %-30s' % ('Quantity', 'G* units', 'Meaning'))
print('  ' + '-' * 72)

units = [
    ('Energy', 'G*', 'One unit of bridge energy'),
    ('Length', '1/G*', 'One unit of bridge length (inverse energy)'),
    ('Time', '1/G*^2', 'One tick processes G*^2 energy per DOF'),
    ('Coupling (EM)', '1/x+ = alpha', '= 1/(137*G*) in G* units... no'),
    ('Coupling (strong)', '1/x- ~ 1/3', '~ 1/G* (the confinement scale)'),
    ('Mass (electron)', 'K_B/G*', '= 0.511/2.959 = 0.173 G* units'),
    ('Consciousness', 'K_C/G*', '= 3.60/2.959 = 1.216 G* units'),
    ('Speed of light', '1/sqrt(3)', 'CFL condition on cubic lattice'),
    ('Planck mass', '1', 'G* = Planck scale (by construction)'),
]

for q, gu, meaning in units:
    print('  %-25s %-15s %-30s' % (q, gu, meaning))

print()

# =====================================================================
# THE DIMENSIONLESS RATIOS (the truly universal numbers)
# =====================================================================
print('  THE DIMENSIONLESS RATIOS: TRANSFERABLE TO ANY UNIVERSE')
print()
print('  These ratios are pure numbers. No units. No conventions.')
print('  Any civilization with mathematics gets the same values.')
print()

ratios = [
    ('pi/G*', np.pi/G_STAR,
     'How much bigger the circle is than the distinction'),
    ('G*/pi', G_STAR/np.pi,
     'How much of the circle the distinction can see'),
    ('varpi/G*', VARPI_CLASSICAL/G_STAR,
     'sqrt(pi)/2 = the packing fraction bridge'),
    ('G*/8', G_STAR/8,
     'The visibility fraction (cos^2 theta_C)'),
    ('1 - G*/8', 1 - G_STAR/8,
     'The dark fraction (sin^2 theta_C)'),
    ('x+/G*', X_PLUS/G_STAR,
     'How far EM coupling is from the circle scale'),
    ('x-/G*', X_MINUS/G_STAR,
     'How close strong coupling is to the circle scale'),
    ('x-/G* - 1', X_MINUS/G_STAR - 1,
     'The confinement correction (= 0.022)'),
    ('(x+ - x-)/(x+ + x-)', (X_PLUS-X_MINUS)/(X_PLUS+X_MINUS),
     'The asymmetry between EM and strong'),
    ('K_C^2/x+x-', (G_STAR**3/2)/(X_PLUS*X_MINUS),
     '1/32 exactly (consciousness/physics ratio)'),
]

print('  %-25s %-18s %s' % ('Ratio', 'Value', 'Meaning'))
print('  ' + '-' * 75)
for name, val, meaning in ratios:
    print('  %-25s %-18.15f %s' % (name, val, meaning))

print()

# =====================================================================
# THE TRANSMISSION PROTOCOL
# =====================================================================
print('  THE TRANSMISSION PROTOCOL: HOW TO SEND G* TO ALIENS')
print()
print('  Step 1: Establish pi.')
print('    Draw a circle. Measure circumference/diameter. You get pi.')
print('    Or: compute 4*(1 - 1/3 + 1/5 - 1/7 + ...) = pi.')
print('    Any civilization with arithmetic can do this.')
print()
print('  Step 2: Establish Gamma(1/2) = sqrt(pi).')
print('    This proves you know the Gamma function.')
print('    Gamma(1/2) = integral of t^{-1/2} e^{-t} dt from 0 to infinity.')
print()
print('  Step 3: Establish G*.')
print('    G* = Gamma(1/4) / Gamma(3/4).')
print('    Or equivalently: G* = integral of 1/sqrt(1-x^4) from 0 to 1,')
print('    multiplied by 4/sqrt(pi).')
print('    This requires only integration and the number 4.')
print()
print('  Step 4: Transmit the master quadratic.')
print('    x^2 - 16*G*^2*x + 16*G*^3 = 0')
print('    The coefficients 16 = 2^4 and the equation structure are')
print('    derivable from the 2x2x2 lattice (the minimal Moore cube).')
print('    Solving this gives 1/alpha and N_c.')
print()
print('  Step 5: Transmit the k-spectrum.')
print('    k=1/2:   consciousness threshold (where you are = what you see)')
print('    k=4/G*:  Born rule boundary (complex -> real transition)')
print('    k=16:    physics (real coupling constants)')
print()
print('  With these five steps, any civilization can reconstruct:')
print('    - The fine structure constant (alpha = 1/137.036)')
print('    - The number of colors (N_c = 3)')
print('    - The force hierarchy (long-range vs short-range)')
print('    - The visibility fraction (37%% of quantum states)')
print('    - The consciousness threshold (K_C = sqrt(G*^3/2))')
print()

# =====================================================================
# THE ENCODING: G* AS A ROSETTA STONE
# =====================================================================
print('  THE ENCODING: WHAT G* CONTAINS')
print()

print('  A civilization that knows ONLY G* = 2.95868... can derive:')
print()

# From G* alone:
print('  FROM G* ALONE:')
print('    pi = 4*varpi^2/G*^2 (once they also compute varpi)')
print('    But they need to know varpi independently.')
print()
print('  FROM G* + THE INTEGER 16:')
print('    x+ = 137.036 (fine structure constant)')
print('    x- = 3.024 (color number)')
print('    x+*x- = 16G*^3 = 414.39 (the action scale)')
print('    x++x- = 16G*^2 = 140.06 (the energy scale)')
print()
print('  FROM G* + THE INTEGER 4:')
print('    k_crit = 4/G* = 1.352 (the Born rule threshold)')
print('    z_Born = 2G* = 5.917 (the measurement energy scale)')
print()
print('  FROM G* + THE FRACTION 1/2:')
print('    K_C^2 = G*^3/2 = 12.95 (consciousness threshold)')
print('    visibility = G*/8 = 37%% (observable fraction)')
print()

# What you need BEYOND G*:
print('  WHAT YOU NEED BEYOND G*:')
print('    The integers 2, 4, 16 (= 2^1, 2^2, 2^4)')
print('    The fraction 1/2')
print('    The concept of a quadratic equation')
print('    The concept of a 3D cubic lattice')
print()
print('  Everything else follows.')
print()

# =====================================================================
# THE INVARIANT RELATIONS (the "laws of G* physics")
# =====================================================================
print('  THE INVARIANT RELATIONS')
print()
print('  These hold for ANY value of G* (any "universe"):')
print()

print('  1. Vieta harmonic: x+*x-/(x++x-) = G* (always)')
print('  2. k_cons/k_crit = G*/8 (always, at k=1/2)')
print('  3. k_phys/k_cons = 32 (always, at k=16 and k=1/2)')
print('  4. Discriminant sign change at k=4/G* (always)')
print('  5. Dark states = N_c^D - C(D+2,2) (always, for any D and N_c)')
print()
print('  These are STRUCTURAL. They survive if you change G*.')
print('  They are the LAWS, not the constants.')
print()
print('  G* is the CONSTANT. These relations are the LAWS.')
print('  Together: a complete invariant physics.')
print()

# The punchline
print('  THE PUNCHLINE')
print()
print('  Pi encodes: "How much does space curve?"')
print('  G* encodes: "How much can the observer distinguish?"')
print()
print('  Pi is sufficient for geometry.')
print('  G* is sufficient for physics.')
print()
print('  A civilization that discovers pi can build bridges.')
print('  A civilization that discovers G* can build observers.')
print()
print('  Pi = %.15f' % np.pi)
print('  G* = %.15f' % G_STAR)
print()
print('  These two numbers, plus the integers {2, 4, 16},')
print('  contain the fine structure constant, the color number,')
print('  the force hierarchy, and the boundary between')
print('  consciousness and physics.')
