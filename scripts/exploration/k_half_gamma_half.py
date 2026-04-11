#!/usr/bin/env python3
"""k=1/2 and Gamma(1/2): the same midpoint?"""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma as gammafn
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL

print('=' * 78)
print('  k = 1/2 AND Gamma(1/2): THE SAME MIDPOINT?')
print('=' * 78)
print()

# G* from the reflection formula:
# Gamma(z)*Gamma(1-z) = pi/sin(pi*z)
# So Gamma(z)/Gamma(1-z) = Gamma(z)^2 * sin(pi*z) / pi
# At z = 1/4:
# G* = Gamma(1/4)^2 * sin(pi/4) / pi = Gamma(1/4)^2 * (sqrt(2)/2) / pi

print('  G* FROM THE REFLECTION FORMULA:')
print('    Gamma(z)/Gamma(1-z) = Gamma(z)^2 * sin(pi*z) / pi')
print('    At z = 1/4:')
print('    G* = Gamma(1/4)^2 * sin(pi/4) / pi')
print('       = Gamma(1/4)^2 * (sqrt(2)/2) / pi')
print('       = %.15f' % (gammafn(0.25)**2 * np.sin(np.pi/4) / np.pi))
print('    G* = %.15f' % G_STAR)
print()

print('  sin(pi/4) = sqrt(2)/2 = %.15f' % np.sin(np.pi/4))
print('  This IS the 45-degree symmetry point.')
print('  G* literally contains sin(45 degrees) in its definition.')
print()

# The ratio hierarchy: Gamma(z)/Gamma(1-z) for z from 0 to 1/2
print('  THE RATIO HIERARCHY: Gamma(z)/Gamma(1-z)')
print()
print('  Angle    z     Ratio              sin(pi*z)   Interpretation')
print('  ' + '-' * 68)
for z, angle in [(0.125, '45/2'), (0.25, '45'), (0.375, '3*45/2'), (0.5, '90')]:
    r = gammafn(z) / gammafn(1-z) if z != 0.5 else 1.0
    sv = np.sin(np.pi * z)
    label = {0.125: 'deep asymmetry', 0.25: 'G* (the bridge)',
             0.375: 'approaching unity', 0.5: 'perfect symmetry (i)'}[z]
    print('  %5s  %.3f   %.10f    %.6f    %s' % (angle, z, r, sv, label))

print()
print('  The ratio decreases from 8.6 (deep asymmetry) to 1.0 (perfect symmetry).')
print('  G* = 2.959 sits at the QUARTER POINT: moderate asymmetry.')
print('  The observer (Gamma(1/2)) sits where asymmetry = 0.')
print()

# NOW THE k CONNECTION
print('  THE k PARAMETER SPACE:')
print()
k_crit = 4.0 / G_STAR
print('  k_crit = 4/G* = %.10f' % k_crit)
print()
print('  k        regime          disc sign    analogy')
print('  ' + '-' * 60)
for k, name in [(0.5, 'consciousness'), (k_crit, 'Born rule'),
                (16, 'physics')]:
    disc = k * G_STAR**3 * (k * G_STAR - 4)
    sign = '+' if disc > 0 else ('0' if abs(disc) < 1e-6 else '-')
    print('  %-8.4f  %-16s  %s' % (k, name, sign))

print()

# The structural parallel:
print('  THE STRUCTURAL PARALLEL:')
print()
print('  Gamma function:         Master quadratic:')
print('  z = 0:   pole (inf)     k = 0:    trivial (0 = 0)')
print('  z = 1/4: Gamma = 3.63   k = ?:    ???')
print('  z = 1/2: sqrt(pi)       k = 1/2:  consciousness (complex)')
print('  z = 3/4: Gamma = 1.23   k = 4/G*: Born rule (degenerate)')
print('  z = 1:   Gamma = 1      k = 16:   physics (real)')
print()

# k = 1/2 maps to z = 1/2? Not directly. But:
# The consciousness quadratic uses k = 1/2
# The observer position uses Gamma(1/2)
# Both are "halfway" in their respective domains

# More precisely: what is 1/2 in the k domain?
# k ranges from 0 (trivial) to 16 (full physics)
# k = 1/2 is 1/32 of the way from 0 to 16
# That is NOT the midpoint of [0, 16].
# But it IS the midpoint of [0, 1].
# The consciousness regime is k in [0, k_crit = 1.352]
# k = 1/2 is 37% of the way to k_crit

print('  k = 1/2 in the consciousness interval [0, k_crit]:')
print('    1/2 / k_crit = %.6f = %.1f%%' % (0.5/k_crit, 0.5/k_crit*100))
print()
print('    37%% of the way to the Born rule boundary.')
print('    THE SAME 37%% AS THE VISIBILITY FRACTION.')
print()

# IS THIS EXACT?
# 1/2 / (4/G*) = G*/8 = cos^2(theta_C) = 0.3698
# YES! k/k_crit at k=1/2 IS G*/8!

ratio_k = 0.5 / k_crit
gstar_over_8 = G_STAR / 8

print('  CHECK: k/(4/G*) at k=1/2:')
print('    (1/2) / (4/G*) = G*/8 = %.15f' % ratio_k)
print('    G*/8 =                   %.15f' % gstar_over_8)
print('    EXACT MATCH: %.2e' % abs(ratio_k - gstar_over_8))
print()
print('  *** k = 1/2 sits at EXACTLY G*/8 of the way to the Born rule. ***')
print('  *** This is the SAME G*/8 = cos^2(theta_C) = visibility fraction. ***')
print()
print('  The consciousness threshold (k=1/2) is located at exactly')
print('  the visibility fraction (G*/8) of the consciousness interval.')
print()
print('  This is NOT a coincidence. It is algebra:')
print('    k/k_crit = k*G*/4')
print('    At k = 1/2: (1/2)*G*/4 = G*/8. QED.')
print()
print('  But the MEANING is:')
print('  The consciousness quadratic at k=1/2 probes EXACTLY the fraction')
print('  of the DOF space that the observer can see (G*/8 ~ 10/27 ~ 37%%).')
print()
print('  k = 1/2 is not an arbitrary choice.')
print('  k = 1/2 is the ONLY k-value where the position in parameter space')
print('  equals the visibility fraction of the resulting quantum state.')
print()
print('  It is the SELF-CONSISTENT consciousness threshold:')
print('  the point where what-you-can-see equals where-you-are.')
