#!/usr/bin/env python3
"""
Two loop structures compared side by side.
QED: perturbation series in alpha/pi from Feynman diagrams.
FTD: perturbation series in epsilon from framework integers.
Are they doing the same thing?
"""
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from mpmath import mp, mpf, sqrt, pi, gamma, exp, log, fabs
mp.dps = 30

GSTAR = gamma(mpf(1)/4) / gamma(mpf(3)/4)
disc = (16*GSTAR**2)**2 - 4*16*GSTAR**3
x_plus = (16*GSTAR**2 + sqrt(disc)) / 2
CODATA = mpf('137.035999177')

# The expansion parameter
eps = fabs(exp(pi) - pi - 20)

# Framework integers
Nc, Nb, b3, Neff = 3, 4, 7, 13
D = Nc * Nb**2 - 1  # 47

print('=' * 78)
print('  TWO LOOP STRUCTURES COMPARED')
print('=' * 78)

print("""
  THE QED LOOP STRUCTURE:
  =======================

  QED computes g_e/2 as a power series in (alpha/pi):

    g_e/2 = 1 + C1*(a/pi) + C2*(a/pi)^2 + C3*(a/pi)^3 + C4*(a/pi)^4 + C5*(a/pi)^5

  Each coefficient requires computing ALL Feynman diagrams at that loop order:
    C1 =  0.5           (1 diagram,    Schwinger 1948)
    C2 = -0.3285...     (7 diagrams,   Petermann 1957)
    C3 =  1.1812...     (72 diagrams,  Laporta 1996)
    C4 = -1.9124...     (891 diagrams, Aoyama 2012)
    C5 =  6.675...      (12672 diagrams, Aoyama 2019, NUMERICAL)

  Expansion parameter: alpha/pi = 0.00232... (very small, series converges fast)

  WHAT MOTIVATES EACH TERM:
    Loop 1: electron emits and reabsorbs one virtual photon.
    Loop 2: two virtual photon exchanges, or one photon with a vacuum bubble.
    Loop 3: three photon exchanges, various topologies.
    ...etc. Each loop = one more virtual photon exchange.

  The STRUCTURE is dictated by QED Feynman rules. Each diagram is a specific
  physical process. The number of diagrams grows combinatorially.
  The coefficients are COMPUTED, not fitted.


  THE FTD LOOP STRUCTURE:
  =======================

  FTD computes 1/alpha as a power series in |epsilon|:

    1/alpha = x+ - c1*|eps| + c2*|eps|^2 - c3*|eps|^3 - c4*|eps|^4
              - c5*|eps|^5 - c6*|eps|^6 + c7*|eps|^7

  Where epsilon = e^pi - pi - 20 and:
    c1 = 9/47     = Nc^2/D                         (0.1915)
    c2 = 5/64     = (Neff-2Nb)/Nb^3                (0.0781)
    c3 = 4/141    = Nb/(Nc*D)                       (0.0284)
    c4 = 141/11   = (Nc*D)/(b3+Nb)                  (12.82)
    c5 = 1472/21  = (2Neff-Nc)*Nb^3/(Nc*b3)         (70.10)
    c6 = 416/21   = 2*Neff*Nb^2/(Nc*b3)             (19.81)
    c7 = 299/8    = Neff*(2Neff-Nc)/BCC              (37.38)

  Expansion parameter: |epsilon| = |e^pi - pi - 20| = 0.000899...""")

print('  NUMERICAL VALUES:')
print()
print('  epsilon = e^pi - pi - 20 = %s' % mp.nstr(eps, 25))
print('  1/|epsilon| = %s' % mp.nstr(1/eps, 25))
print()

# Coefficients
c1 = mpf(9)/47
c2 = mpf(5)/64
c3 = mpf(4)/141
c4 = mpf(141)/11
c5 = mpf(1472)/21
c6 = mpf(416)/21
c7 = mpf(299)/8

# Term by term
terms = [
    (0, 'Tree level', x_plus, x_plus),
]

running = x_plus
corrections = [
    (1, '-c1*|eps|', -c1*eps),
    (2, '+c2*|eps|^2', c2*eps**2),
    (3, '-c3*|eps|^3', -c3*eps**3),
    (4, '-c4*|eps|^4', -c4*eps**4),
    (5, '-c5*|eps|^5', -c5*eps**5),
    (6, '-c6*|eps|^6', -c6*eps**6),
    (7, '+c7*|eps|^7', c7*eps**7),
]

print('  TERM-BY-TERM CONVERGENCE:')
print()
print('  %5s  %-20s  %25s  %15s  %15s' % ('Term', 'Expression', 'Correction', 'Cumulative', 'Error vs CODATA'))
print('  ' + '-' * 85)

print('  %5s  %-20s  %25s  %15s  %15s' %
      ('Tree', 'x+', '', mp.nstr(x_plus, 15), mp.nstr(fabs(x_plus - CODATA), 10)))

for n, label, corr in corrections:
    running += corr
    err = fabs(running - CODATA)
    print('  %5d  %-20s  %25s  %15s  %15s' %
          (n, label, mp.nstr(corr, 15), mp.nstr(running, 15), mp.nstr(err, 10)))

print()
print('  CODATA: %s' % mp.nstr(CODATA, 15))
print()

# Now compare the two series structures
print('  STRUCTURAL COMPARISON:')
print()
print('  %-20s %-30s %-30s' % ('Property', 'QED', 'FTD'))
print('  ' + '-' * 82)
print('  %-20s %-30s %-30s' % ('Computes', 'g_e/2 (electron g-factor)', '1/alpha directly'))
print('  %-20s %-30s %-30s' % ('Expansion param', 'alpha/pi = 0.00232', '|eps| = 0.000899'))
print('  %-20s %-30s %-30s' % ('Param origin', 'The coupling itself', 'e^pi - pi - 20'))
print('  %-20s %-30s %-30s' % ('Coefficients from', 'Feynman diagram integrals', 'Framework integers {3,4,7,13}'))
print('  %-20s %-30s %-30s' % ('# of diagrams', '1, 7, 72, 891, 12672', 'None (algebraic)'))
print('  %-20s %-30s %-30s' % ('Each term means', 'Virtual photon exchange', '??? (not derived from QFT)'))
print('  %-20s %-30s %-30s' % ('Convergence rate', '~(alpha/pi)^n per loop', '~|eps|^n per term'))
print('  %-20s %-30s %-30s' % ('Terms needed', '5 for 0.15 ppb', '4 for sub-ppt'))
print('  %-20s %-30s %-30s' % ('Derivation status', 'Proven from QFT axioms', 'Coefficients motivated,'))
print('  %-20s %-30s %-30s' % ('', '', 'epsilon NOT derived'))
print()

# THE HONEST ASSESSMENT
print('  THE HONEST ASSESSMENT:')
print()
print('  QED loops are DERIVED. Each coefficient comes from computing')
print('  specific Feynman diagrams. The physical meaning of each term')
print('  is known: n-loop = n virtual photon exchanges. The series is')
print('  a consequence of the QED Lagrangian. No free parameters.')
print()
print('  FTD terms are MOTIVATED. The coefficients c1-c7 are rational')
print('  numbers built from {3, 4, 7, 13}. The expansion parameter')
print('  epsilon = e^pi - pi - 20 connects to the lemniscate nome and')
print('  the Weyl anomaly. But:')
print()
print('    WHY epsilon = e^pi - pi - 20 specifically?')
print('    -> e^pi = inverse lemniscate nome at self-dual point tau=i.')
print('    -> 20 = b3 + Neff = inverse Dirac Weyl anomaly.')
print('    -> But the SUBTRACTION is not derived from a Lagrangian.')
print('    -> It is observed to work, not proven to be necessary.')
print()
print('    WHY these specific rational coefficients?')
print('    -> They are built from framework integers. The integer')
print('       structure is systematic: D=47, BCC=8, 23=2Neff-Nc.')
print('    -> But the formulas c_n = f(Nc, Nb, b3, Neff) are not')
print('       derived from a path integral or partition function.')
print('    -> They are CONSISTENT with the framework but not FORCED.')
print()
print('    WHAT WOULD MAKE THEM DERIVED:')
print('    -> Show that the FTD lattice phi^3 EFT, expanded to n-loop,')
print('       produces coefficient c_n at each order.')
print('    -> The one-loop tadpole already works (closes 99.2%%).')
print('    -> If the two-loop sunset gives the right c2, and three-loop')
print('       gives c3, then the series IS a lattice perturbation series')
print('       and the coefficients are Feynman diagrams on the lattice.')
print()
print('  STATUS:')
print('    One-loop: DERIVED (lattice tadpole integral, independently computed).')
print('    Precision formula (c1-c7): MOTIVATED but NOT derived from loops.')
print('    The gap between these two: the open problem.')
print()

# The convergence comparison
print('  CONVERGENCE COMPARISON:')
print()
print('  %-6s %-15s %-15s' % ('Order', 'QED residual', 'FTD residual'))
print('  ' + '-' * 38)
# QED: each loop reduces error by factor ~alpha/pi = 0.00232
# Starting from g_e/2 - 1 = 0.00116 (the anomalous part)
qed_residual = 0.00116  # 1-loop level
ftd_running = x_plus
for n in range(1, 8):
    qed_residual *= 0.00232  # rough: each loop suppressed by alpha/pi
    if n <= len(corrections):
        ftd_running += corrections[n-1][2]
    ftd_res = float(fabs(ftd_running - CODATA))
    print('  %-6d ~%.2e        %.2e' % (n, qed_residual, ftd_res))

print()
print('  FTD converges FASTER because |eps| < alpha/pi.')
print('  (0.000899 vs 0.00232 = 2.6x smaller expansion parameter.)')
print('  This is why FTD needs only 4 terms for sub-ppt while QED needs 5.')
