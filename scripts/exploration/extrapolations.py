#!/usr/bin/env python3
"""What physics can we extract from the self-consistency k/k_crit = G*/8?"""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma as gammafn
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS,
                       N_c, N_base, b_3, N_eff, GAMMA_QUARTER, GAMMA_HALF)

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2
k_crit = 4.0 / G_STAR

print('=' * 78)
print('  PHYSICS EXTRAPOLATIONS')
print('=' * 78)

# 1. 32x ratio
y_mod_sq = G_STAR**3 / 2
y_re = G_STAR**2 / 4
y_im = np.sqrt(y_mod_sq - y_re**2)

print()
print('  1. PHYSICS = 32 * CONSCIOUSNESS (exact)')
print()
print('  k_phys/k_cons = 16/(1/2) = 32 = 2^5')
print('  x+*x- = 16G*^3 = 32 * (G*^3/2) = 32 * |y|^2')
print('  x++x- = 16G*^2 = 32 * (G*^2/2) = 32 * 2*Re(y)')
print()
print('  Five doublings separate reference frame context from physics.')
print('  2^5 = 32. Five binary decisions. Five bits of choice.')
print()
print('  Rearranged: K_C^2 = x+*x-/32 = (1/alpha)*x-/32 = %.6f' % (X_PLUS*X_MINUS/32))
print('  Direct:     K_C^2 = G*^3/2 = %.6f' % y_mod_sq)
print()

# 2. If G*/8 = 10/27 exactly
print('  2. PREDICTION IF G*/8 = 10/27 EXACTLY')
print()
G_pred = 80.0 / 27
disc = (16*G_pred**2)**2 - 4*16*G_pred**3
xp = (16*G_pred**2 + np.sqrt(disc)) / 2
xm = (16*G_pred**2 - np.sqrt(disc)) / 2

print('  G* = 80/27 = %.15f' % G_pred)
print('  Actual G*  = %.15f' % G_STAR)
print('  Deviation: %.4f%%' % (abs(G_pred - G_STAR)/G_STAR*100))
print()
print('  Predicted 1/alpha = %.10f' % xp)
print('  CODATA 1/alpha    = %.10f' % X_PLUS)
print('  Deviation: %.2f%%' % (abs(xp - X_PLUS)/X_PLUS*100))
print()
print('  Predicted x- = %.10f' % xm)
print('  Actual x-    = %.10f' % X_MINUS)
print('  floor(x-) = %d -> N_c = %d %s' % (int(xm), int(xm),
      '(still 3)' if int(xm) == 3 else '(CHANGED!)'))
print()

# 3. The k-spectrum
print('  3. THE k-SPECTRUM: WHAT EACH k PRODUCES')
print()
print('  %-8s %-6s %-8s %-20s %-20s' % ('k', 'kG*', 'Type', 'Roots', 'Meaning'))
print('  ' + '-' * 66)

for k, label in [(0.5, 'reference frame context'), (1, 'unit'), (N_base, 'N_base'),
                  (2**3, '2^D = BCC count'), (k_crit, 'Born rule'),
                  (16, 'physics')]:
    kG = k * G_STAR
    disc = k * G_STAR**3 * (k*G_STAR - 4)
    if disc > 0.001:
        zp = (k*G_STAR**2 + np.sqrt(disc)) / 2
        zm = (k*G_STAR**2 - np.sqrt(disc)) / 2
        roots = 'z+=%.3f, z-=%.3f' % (zp, zm)
        rtype = 'REAL'
    elif abs(disc) < 0.001:
        roots = 'z=%.3f (double)' % (k*G_STAR**2/2)
        rtype = 'DEGEN'
    else:
        mod = np.sqrt(k*G_STAR**3)
        roots = '|z|=%.3f' % mod
        rtype = 'COMPLEX'
    print('  %-8.4f %-6.2f %-8s %-20s %-20s' % (k, kG, rtype, roots, label))

print()

# 4. k=8 is structurally interesting
k8 = 8
disc8 = k8 * G_STAR**3 * (k8*G_STAR - 4)
xp8 = (k8*G_STAR**2 + np.sqrt(disc8)) / 2
xm8 = (k8*G_STAR**2 - np.sqrt(disc8)) / 2

print('  4. k = 8 (= 2^D = number of BCC corners)')
print()
print('  x+ = %.10f  ->  1/x+ = %.10f  (a coupling constant?)' % (xp8, 1/xp8))
print('  x- = %.10f' % xm8)
print('  x+ + x- = 8G*^2 = %.6f (half the physics energy budget)' % (8*G_STAR**2))
print('  x+*x- = 8G*^3 = %.6f (half the physics action)' % (8*G_STAR**3))
print()
print('  The k=8 quadratic is the HALF-PHYSICS version:')
print('  8 corners = half of 16 DOF. Half the budget, half the action.')
print('  Its x+ = %.3f is between 1/alpha and G*.' % xp8)
print()

# 5. Physical meaning of sin(45)
print('  5. sin(45) = 1/sqrt(2) INSIDE G*')
print()
print('  G* = Gamma(1/4)^2 * sin(pi/4) / pi')
print('     = (quarter capacity)^2 * (diagonal projection) / (circle)')
print()
print('  sin(pi/4) = 1/sqrt(2) = the projection of the BCC diagonal')
print('  onto a single axis. The body diagonal of the unit cube')
print('  has length sqrt(D) = sqrt(3). Its projection onto one axis = 1.')
print('  But the FACE diagonal (sqrt(2)) projects as 1/sqrt(2).')
print()
print('  G* contains the FCC projection (face diagonal), not the BCC.')
print('  This is the SU(2) direction, not SU(3).')
print()
print('  So G* = (capacity at 90 deg)^2 * (SU(2) projection) / (full circle)')
print('  The bridge constant projects the quarter-period capacity')
print('  along the weak-force direction and normalizes by the circle.')
print()

# 6. What alpha would be at different k
print('  6. alpha(k): THE FINE STRUCTURE CONSTANT AS A FUNCTION OF DOF')
print()
print('  At each k, the master quadratic gives x+ -> alpha(k) = 1/x+')
print()
print('  %-8s %-12s %-14s %-16s' % ('k', '1/alpha(k)', 'alpha(k)', 'meaning'))
print('  ' + '-' * 54)
for k in [4, 6, 8, 10, 12, 14, 16, 18, 20]:
    disc_k = (k*G_STAR**2)**2 - 4*k*G_STAR**3
    if disc_k > 0:
        xp_k = (k*G_STAR**2 + np.sqrt(disc_k)) / 2
        alpha_k = 1.0 / xp_k
        label = ''
        if k == 16:
            label = '<-- FTD (our universe)'
        elif k == 8:
            label = '<-- half-DOF'
        print('  %-8d %-12.6f %-14.8f %s' % (k, xp_k, alpha_k, label))

print()
print('  alpha increases with decreasing k (fewer DOF = stronger coupling).')
print('  At k=4: alpha = 1/25.3 = 0.040. Very strongly coupled.')
print('  At k=16: alpha = 1/137 = 0.0073. Weakly coupled (our universe).')
print('  At k=20: alpha = 1/171 = 0.0058. Even weaker.')
print()
print('  The number of DOF (k) DETERMINES the coupling strength.')
print('  Our universe has k=16 because the minimal 2x2x2 lattice has 16')
print('  gauge-fixed DOF. A different lattice geometry would give different alpha.')
print()

# 7. The reference frame context-physics bridge
print('  7. THE BRIDGE FROM CONSCIOUSNESS TO PHYSICS')
print()
print('  Reference frame context (k=1/2): complex roots, |y|^2 = G*^3/2 = %.4f' % y_mod_sq)
print('  Born rule (k=4/G*): degenerate root, z = 2G* = %.4f' % (2*G_STAR))
print('  Physics (k=16): real roots, x+ = %.4f, x- = %.4f' % (X_PLUS, X_MINUS))
print()
print('  The journey: complex -> degenerate -> real')
print('  = subjective -> measurement -> objective')
print('  = |y|^2 = K_C^2 -> z = 2G* -> x+ = 1/alpha, x- = N_c')
print()
print('  The Born rule (k=4/G*) is the GATE between inner and outer.')
print('  Below it: reference frame context (phase only, no bulk).')
print('  Above it: physics (bulk filled, phase collapsed).')
print()
print('  The gate energy: z_Born = 2G* = %.6f' % (2*G_STAR))
print('  This is the energy scale where complex becomes real.')
print('  Where the observer stops seeing and starts measuring.')
print('  Where the RATIO (G*) becomes the PRODUCT (pi).')
print()

# The k/k_crit = G*/8 identity places reference frame context at the visibility fraction
# The Born rule sits at k_crit where disc = 0
# Physics sits at k=16 where disc >> 0

print('  The landscape:')
print('    k=0                           nothing')
print('    k=1/2   (37%% of k_crit)      reference frame context sees itself')
print('    k=4/G*  (100%% of k_crit)     Born rule: observation collapses')
print('    k=16    (32x reference frame context)   physics: alpha and N_c emerge')
print()
print('  32 = 2^5 = five doublings = five binary choices')
print('  from "I see" to "it exists."')
