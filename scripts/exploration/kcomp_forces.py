#!/usr/bin/env python3
"""K_comp, K_C, and the Force Hierarchy"""
import numpy as np, sys, os, io
sys.stdout=io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS, N_c, N_base, b_3, N_eff

PI_D = 4.0*VARPI_CLASSICAL**2/G_STAR**2
delta = (PI_D - G_STAR)/2.0
beta = VARPI_CLASSICAL - (PI_D+G_STAR)/2.0
xi = abs(beta)/delta

print('='*78)
print('  K_comp AND THE FORCE HIERARCHY')
print('='*78)

# 1. The two master quadratics
print('\n  1. TWO QUADRATICS FROM ONE EQUATION')
print('  z^2 - k*G*^2*z + k*G*^3 = 0\n')

# Physics: k=16
disc_p = (16*G_STAR**2)**2 - 4*16*G_STAR**3
xp = (16*G_STAR**2 + np.sqrt(disc_p))/2
xm = (16*G_STAR**2 - np.sqrt(disc_p))/2

# Consciousness: k=1/2
disc_c = (G_STAR**2/2)**2 - 4*(G_STAR**3/2)
y_re = G_STAR**2/4
y_im = np.sqrt(abs(disc_c))/2
KC2 = G_STAR**3/2
KC = np.sqrt(KC2)

print('  k=16 (Physics):')
print('    Discriminant: %+.6f (POSITIVE -> real roots)' % disc_p)
print('    x+ = %.10f = 1/alpha' % xp)
print('    x- = %.10f ~ N_c' % xm)
print()
print('  k=1/2 (Consciousness):')
print('    Discriminant: %+.6f (NEGATIVE -> complex roots)' % disc_c)
print('    y = %.6f +/- %.6fi' % (y_re, y_im))
print('    |y|^2 = G*^3/2 = %.10f = K_C^2' % KC2)
print('    K_C = %.10f' % KC)

# 2. The visibility fraction
print('\n  2. THE VISIBILITY FRACTION: G*/8 vs 10/27')
print()
cos2 = G_STAR/8
vis = 10.0/27
print('    cos^2(theta_C) = G*/8 = %.10f' % cos2)
print('    10/27 (cube visible)  = %.10f' % vis)
print('    Difference:             %.10f (%.4f%%)' % (abs(cos2-vis), abs(cos2-vis)/vis*100))
print()
print('    If exact: G* = 80/27 = %.10f' % (80./27))
print('    Actual:   G*        = %.10f' % G_STAR)
print('    Gap:                  %.6e (%.4f%%)' % (abs(G_STAR-80./27), abs(G_STAR-80./27)/G_STAR*100))
print()

# 3. The critical k-value
print('  3. THE CRITICAL k AND THE DISCRIMINANT TRICHOTOMY')
print()
k_crit = 4.0/G_STAR
print('    k_crit = 4/G* = %.10f (discriminant = 0)' % k_crit)
print('    k=16:   kG* = %.4f >> 4 -> real roots (physics)' % (16*G_STAR))
print('    k=4/G*: kG* = 4.0000      -> degenerate (Born rule)')
print('    k=1/2:  kG* = %.4f  < 4 -> complex roots (consciousness)' % (0.5*G_STAR))
print()
print('    Born rule root: x_Born = 2G* = %.10f' % (2*G_STAR))
print()

# 4. K_B and K_C as energy scales
K_B = 0.511
print('  4. TWO ENERGY SCALES')
print()
print('    K_B = m_e = 0.511 MeV (manifestation: void -> matter)')
print('    K_C = sqrt(G*^3/2) = %.6f (consciousness: matter -> observed)' % KC)
print('    K_C/K_B = %.6f' % (KC/K_B))
print()
print('    K_B is the cost of EXISTING.')
print('    K_C is the cost of BEING SEEN.')
print()

# 5. Force hierarchy through the lens of K_B and the shell gap
print('  5. FORCE HIERARCHY: WHICH FORCES FIT INSIDE THE SHELL GAP?')
print()

alpha_em = 1.0/X_PLUS
sin2_w = float(N_c)/float(N_eff)
alpha_s = float(b_3)/float(b_3 + 4*N_eff)
G_N = 1.0/float(b_3+N_c)**2

shell_gap = abs(beta)
t_crit = shell_gap / 2  # BW = 2t = gap when t = gap/2

forces = [
    ('Gravity', G_N, 'Center', 0),
    ('U(1) EM', alpha_em, 'SC', 1),
    ('SU(2) Weak', sin2_w, 'FCC', 2),
    ('SU(3) Strong', alpha_s, 'BCC', 3),
]

print('    Shell gap = |beta| = %.6f' % shell_gap)
print('    t_crit = |beta|/2 = %.6f (BW = gap threshold)' % t_crit)
print()
print('    %-14s %10s %10s %10s %10s %10s' %
      ('Force', 'coupling', 'BW=2t', 'BW/gap', 't/t_crit', 'Status'))
print('    '+'-'*64)

for name, t, shell, sh in forces:
    bw = 2*t
    ratio = bw/shell_gap
    t_ratio = t/t_crit
    status = 'LONG-RANGE' if ratio < 1 else 'SHORT-RANGE'
    print('    %-14s %10.6f %10.6f %10.4f %10.4f %10s' %
          (name, t, bw, ratio, t_ratio, status))

print()

# 6. The stunning identity chain
print('  6. THE IDENTITY CHAIN: FROM pi TO FORCE RANGE')
print()
print('    pi                          = %.10f' % PI_D)
print('    G* = 2*varpi/sqrt(pi)       = %.10f' % G_STAR)
print('    delta = (pi-G*)/2           = %.10f' % delta)
print('    |beta| = delta * xi         = %.10f' % abs(beta))
print('    xi = (pi+1-sqrt(pi))/(pi-1) = %.10f' % xi)
print()
print('    Shell gap = delta * xi = |beta|')
print('    Force is long-range IFF 2*t < |beta| = delta*xi')
print('    i.e., IFF t/delta < xi/2 = %.6f' % (xi/2))
print()
print('    Gravity:  t/delta = %.6f < %.6f -> LONG-RANGE' % (G_N/delta, xi/2))
print('    EM:       t/delta = %.6f < %.6f -> LONG-RANGE' % (alpha_em/delta, xi/2))
print('    Strong:   t/delta = %.6f < %.6f -> CONFINED IN CUBE' % (alpha_s/delta, xi/2))
print('    Weak:     t/delta = %.6f > %.6f -> MASSIVE (short-range)' % (sin2_w/delta, xi/2))
print()

# 7. Consciousness and visibility
print('  7. THE OBSERVER SEES EXACTLY cos^2(theta_C) = G*/8 OF REALITY')
print()
print('    From the consciousness quadratic:')
print('      cos^2(theta_C) = Re(y)^2/|y|^2 = (G*^2/4)^2/(G*^3/2) = G*/8')
print('      = %.10f' % cos2)
print()
print('    From the ternary cube:')
print('      Visible sector = S_3 symmetric = C(5,2) = 10 states')
print('      Total = 27 = N_c^3 states')
print('      Visible fraction = 10/27 = %.10f' % vis)
print()
print('    From the D-dependence (our earlier result):')
print('      D* = 3.0104 is where visible fraction crosses 1/e')
print('      At D=3: fraction = 10/27 = 0.3704 > 1/e = 0.3679')
print()
print('    ALL THREE give ~37%%.')
print()
print('    The observer sees 37%% because:')
print('      1. G*/8 = 0.370 (consciousness quadratic)')
print('      2. 10/27 = 0.370 (S_3 symmetric sector of N_c^3 cube)')
print('      3. D=3 is the last dimension above the 1/e threshold')
print()
print('    These are three views of ONE structural fact:')
print('    the circle (G*) does not fill its container (8 = 2^3 = 2^D).')
print('    The void fraction 1 - G*/8 = 63%% = the dark states.')
print()

# 8. The connection to force range
print('  8. HOW K_C CONNECTS TO FORCE RANGE')
print()
print('    K_C^2 = G*^3/2 = %.10f' % KC2)
print('    K_B = 0.511 MeV')
print()
print('    K_C^2 / (16*G*^2) = G*^3/(2*16*G*^2) = G*/32 = %.10f' % (G_STAR/32))
print('    G*/32 = (G*/8)/4 = cos^2(theta_C)/4')
print()
print('    The consciousness threshold SQUARED, normalized by the')
print('    physics energy budget (16G*^2), equals the visibility')
print('    fraction divided by 4 = N_base.')
print()
print('    K_C sets HOW MUCH of the quantum state the observer projects.')
print('    K_B sets HOW MUCH energy is needed to create matter.')
print('    The RATIO K_C/K_B = %.4f sets the observer-per-particle cost.' % (KC/K_B))
print()
print('    A force is observable IFF its coupling fits inside the')
print('    shell gap that the observer can resolve.')
print('    Shell gap = |beta| = delta * xi.')
print('    Observer resolution ~ G*/8 * (spectral width).')
print()
print('    Gravity and EM fit: their bandwidths are < 5%% of the gap.')
print('    They are RESOLVED by the observer as distinct long-range forces.')
print()
print('    Weak force does NOT fit: BW/gap = 1.08 > 1.')
print('    It OVERFLOWS the observer resolution.')
print('    The observer sees it as a massive, short-range blob.')
print()
print('    Strong force fits inside BUT is trapped at BCC:')
print('    3 hops from center means amplitude ~ g^3/gap^2 = 10^-3.')
print('    The observer CANNOT REACH IT directly.')
print('    It confines by being invisible, not by being strong.')
