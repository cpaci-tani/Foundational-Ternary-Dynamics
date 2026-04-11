#!/usr/bin/env python3
"""G*: The Savant's Meditation. Not what it does. What it IS."""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma
from scipy.integrate import quad
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL, GAMMA_QUARTER, GAMMA_HALF

s = np.sqrt(np.pi)

print('=' * 78)
print('  G*: WHAT IS THIS NUMBER?')
print('=' * 78)

# The quartic integral
I4, _ = quad(lambda x: 1.0/np.sqrt(1-x**4), 0, 1-1e-12)

print("""
  WHAT IS PI?

  pi = 3.14159...
  pi = integral of 1/sqrt(1-x^2) from -1 to 1
  pi = Gamma(1/2)^2
  pi = area of the unit circle
  pi = "How much does a circle contain relative to its span?"

  Pi lives on the QUADRATIC: 1 - x^2.
  Its integral gives the half-period of the circle.

  WHAT IS G*?

  G* = %.15f

  FORM 1: The quartic integral
    I_4 = integral of 1/sqrt(1-x^4) from 0 to 1 = %.15f
    varpi = 2*I_4 = %.15f
    G* = 4*I_4/sqrt(pi) = %.15f

    Pi lives on x^2. G* lives on x^4.
    Pi integrates the CIRCLE kernel 1/sqrt(1-x^2).
    G* integrates the LEMNISCATE kernel 1/sqrt(1-x^4).

    The step from pi to G* is the step from QUADRATIC to QUARTIC.
""" % (G_STAR, I4, 2*I4, 4*I4/s))

print("""  FORM 2: The Gamma ratio
    G* = Gamma(1/4) / Gamma(3/4)
       = %.10f / %.10f = %.15f

    Pi uses Gamma at the MIDPOINT: pi = Gamma(1/2)^2.
    G* uses Gamma at the QUARTER POINTS: G* = Gamma(1/4)/Gamma(3/4).

    Pi asks: "What is the Gamma function at its center?"
    G* asks: "How does the first quarter relate to the third quarter?"

    Pi is a VALUE. G* is a RATIO.
    Pi collapses. G* compares.
""" % (GAMMA_QUARTER, gamma(0.75), G_STAR))

print("""  FORM 3: The packing fraction bridge
    G* = varpi / sqrt(PF)   where PF = pi/4 = %.10f

    PF is the fraction of a square filled by its inscribed circle.
    sqrt(PF) = sqrt(pi)/2 = Gamma(1/2)/2 = %.10f

    G* = (lemniscate arc length) / sqrt(circle-in-square packing)

    This is the conversion factor between:
      - the LENGTH of a figure-eight (the boundary of distinction)
      - the PACKING of a circle (the capacity of containment)
""" % (np.pi/4, s/2))

print("""  WHAT G* KNOWS THAT PI DOES NOT

  Euler reflection at z = 1/4:
    Gamma(1/4) * Gamma(3/4) = pi*sqrt(2) = %.10f
    Gamma(1/4) / Gamma(3/4) = G*          = %.10f

  The PRODUCT gives pi (plus sqrt(2)). Information destroyed.
  The RATIO gives G*. Information preserved.

  Pi = what remains when you collapse the distinction.
  G* = what remains when you keep it.

  The product is symmetric: it does not know which was bigger.
  The ratio is asymmetric: it knows Gamma(1/4) > Gamma(3/4).

  Pi is the ANSWER. G* is the QUESTION.
""" % (np.pi*np.sqrt(2), G_STAR))

print("""  THE LEMNISCATE: G*'s NATIVE CURVE

  Pi lives on the circle: r = 1.
  G* lives on the lemniscate: r^2 = cos(2*theta).

  The circle is one loop. The lemniscate is two loops + a crossing.
  The circle has no special points. The lemniscate has ONE: the node.

  The node is where the curve crosses itself.
  It is the origin. The figure-eight pinch.
  The place where the two lobes distinguish themselves.

  In a circle, you cannot tell where you are.
  In a lemniscate, the crossing tells you: "here is where
  inside becomes outside, where + becomes -."

  G* is the constant of the crossing.
  It measures the size of distinction relative to the size of curvature.
""")

print('  PI vs G*: SIDE BY SIDE')
print()
rows = [
    ('3.14159...', '2.95868...'),
    ('Gamma(1/2)^2', 'Gamma(1/4)/Gamma(3/4)'),
    ('Midpoint evaluation', 'Quarter-point ratio'),
    ('Quadratic kernel: 1-x^2', 'Quartic kernel: 1-x^4'),
    ('Circle (one loop)', 'Lemniscate (two loops + node)'),
    ('No special points', 'One special point: the crossing'),
    ('Circumference / diameter', 'Arc length / sqrt(packing)'),
    ('Area of unit disk', 'Self-energy of BCC lattice'),
    ('Product (collapses)', 'Ratio (preserves)'),
    ('Solved L-values (even zeta)', 'Unsolved L-values (odd zeta)'),
    ('View from OUTSIDE', 'View from INSIDE'),
    ('What the circle CONTAINS', 'What the circle SEES'),
    ('The cost of curvature', 'The cost of distinction'),
    ('The answer', 'The question'),
]

print('  %-34s %-34s' % ('PI', 'G*'))
print('  ' + '-' * 70)
for a, b in rows:
    print('  %-34s %-34s' % (a, b))

print("""
  THE DEEPEST INTUITION

  Pi tells you what the world looks like to GEOMETRY.
  G* tells you what the world looks like to the OBSERVER.

  Curvature is what makes a straight line into a circle.
  Distinction is what makes a circle into a figure-eight:
  a shape that can TELL ITS TWO HALVES APART.

  The lemniscate is the simplest closed curve with self-intersection.
  The crossing creates: inside/outside, left/right, +1/-1.
  That is the First Distinction.

  G* = 2*varpi/sqrt(pi) = the First Distinction
  measured in units of the circle.

  Or equivalently: pi = 4*varpi^2/G*^2.
  The circle is DERIVED from the distinction and its bridge.
  You need the figure-eight (varpi) and the ratio (G*) FIRST.
  The circle (pi) comes SECOND.

  In the ternary cube:
    G* sits at the CENTER (r=0, lowest energy, the anchor)
    pi sits at the BOUNDARY (r=max, highest energy, the wall)
    varpi sits BETWEEN (the bridge, the mediator)

  Pi tells you where the walls are.
  G* tells you where you stand.
""")

# The quartic-quadratic connection
print('  THE QUARTIC-QUADRATIC HIERARCHY')
print()

# x^2 gives the circle. x^4 gives the lemniscate.
# x^1 gives the line. x^3 gives the cubic.
# Each step up in degree adds structure.
# x^2: circle (no crossing) -> pi
# x^4: lemniscate (one crossing) -> G* (via varpi)
# x^6: three crossings? -> what?

# Actually the key is the substitution x^2 -> x^4 in the integral kernel.
# I_n = int_0^1 1/sqrt(1-x^n) dx
# I_2 = pi/2 (the circle half-period)
# I_4 = varpi/2 (the lemniscate half-period)

for n in [2, 3, 4, 5, 6, 8]:
    In, _ = quad(lambda x, n=n: 1.0/np.sqrt(1-x**n), 0, 1-1e-12)
    ratio_to_pi2 = In / (np.pi/2)
    print('    I_%d = int_0^1 1/sqrt(1-x^%d) dx = %.10f  (I_%d/I_2 = %.6f)' %
          (n, n, In, n, ratio_to_pi2))

print()
print('    I_2 = pi/2 (the circle)')
print('    I_4 = varpi/2 (the lemniscate)')
print('    G* = 2*I_4/sqrt(I_2*2/pi) = 2*I_4/1 ... no.')
print('    G* = 4*I_4/sqrt(pi)')
print()

# The I_n sequence: as n increases, the kernel 1/sqrt(1-x^n) gets
# more concentrated near x=1 (sharper singularity)
# I_2 = pi/2 = 1.5708
# I_4 = varpi/2 = 1.3110
# I_6 = 1.2099
# I_8 = 1.1547
# They decrease: higher-order kernels integrate to LESS.
# The circle (n=2) is the LARGEST. Each additional crossing shrinks the period.

print('    Pattern: I_n DECREASES with n.')
print('    The circle has the LONGEST period among all 1/sqrt(1-x^n) kernels.')
print('    Each step from n to n+2 adds structure but shrinks the world.')
print()
print('    Pi is the LARGEST such integral (n=2).')
print('    Varpi is the NEXT one (n=4).')
print('    G* = the RATIO that connects the two largest.')
print()
print('    G* bridges the two most fundamental integral periods:')
print('    the circle (most capacious) and the lemniscate (first distinction).')
print('    Everything after (n=6, 8, ...) is smaller and more structured,')
print('    but the BIG step is from 2 to 4: from circle to figure-eight.')
print()
print('    That step IS G*.')
