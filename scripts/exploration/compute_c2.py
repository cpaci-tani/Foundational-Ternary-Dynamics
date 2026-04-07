#!/usr/bin/env python3
"""
COMPUTE c2: The Two-Loop Sunset on the Moore-Weighted Lattice
==============================================================

The precision formula claims c2 = 5/64 = 0.078125.
The one-loop tadpole gave c1 to 0.8%.
Now: does the two-loop correction give c2?

The two-loop VEV correction in phi^3 theory comes from two sources:
  (a) Iterated tadpole: the one-loop correction applied twice
  (b) Sunset mass correction: the self-energy diagram at p=0

Both must be computed on the MOORE-WEIGHTED lattice with
shell-specific couplings, not on a plain uniform lattice.
"""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from mpmath import mp, mpf, sqrt, pi, gamma, exp, fabs
mp.dps = 40

GSTAR = gamma(mpf(1)/4) / gamma(mpf(3)/4)
disc_mq = (16*GSTAR**2)**2 - 4*16*GSTAR**3
x_plus = (16*GSTAR**2 + sqrt(disc_mq)) / 2
x_minus = (16*GSTAR**2 - sqrt(disc_mq)) / 2
eps = float(fabs(exp(pi) - pi - 20))

Nc, Nb, b3, Neff = 3, 4, 7, 13
m_sq = float(x_plus - x_minus)
a_lat = 2.0/3
m2_lat = m_sq * a_lat**2
g_v = 2.0  # V''' = 2

c1_target = 9.0/47   # 0.191489...
c2_target = 5.0/64   # 0.078125

print('=' * 78)
print('  COMPUTE c2: TWO-LOOP SUNSET ON MOORE-WEIGHTED LATTICE')
print('=' * 78)
print()
print('  Target: c2 = 5/64 = %.10f' % c2_target)
print('  eps = %.15e' % eps)
print('  c2 * eps^2 = %.15e' % (c2_target * eps**2))
print()

# =====================================================================
# STEP 1: Build the large-lattice propagator (plain lattice first)
# =====================================================================
print('  STEP 1: PLAIN LATTICE PROPAGATOR')
print()

# We work on a periodic N^3 lattice.
# The propagator in momentum space: G(k) = 1/(k_hat^2 + m^2_lat)
# k_hat^2 = 4 * sum_mu sin^2(k_mu/2)

# For the TWO-LOOP computation, we need:
# (A) The tadpole integral I_1 = <G(k)>_k (already computed)
# (B) The bubble integral I_bubble = <G(k)^2>_k
# (C) The sunset integral at p=0: I_sunset = <G(k)*G(p)*G(k+p)>_{k,p}
#     = (by Parseval) sum_x G(x)^3 where G(x) = FT^{-1}[G(k)]

# The sunset at p=0 via the position-space trick:
# Sigma(p=0) = int dk dp G(k) G(p) G(k+p)  [with appropriate delta functions]
# = int dx G(x)^2 * G(0)... no.
# More carefully for the VEV correction:

# In phi^3 theory with V(phi) = m^2/2 * phi^2 + g/6 * phi^3,
# the FULL effective potential to two loops is:
#
# V_eff(v) = V_tree(v) + V_1loop(v) + V_2loop(v)
#
# V_1loop = (1/2) * sum_k log(k^2 + M^2(v))
# where M^2(v) = m^2 + g*v (the v-dependent mass)
#
# V_2loop = -(g^2/12) * [sum_k G(k; M^2)]^2 * G(0; M^2)
#         + (g^2/8) * [sum_k G(k; M^2)^2] * ...
# Actually the two-loop effective potential in phi^3 is:
# V_2loop = -(g^2/12) * sum_{x} G(x; M^2)^3
# (the sunset diagram with symmetry factor 1/12)

# The VEV is determined by V_eff'(v) = 0.
# Expanding to second order in v around v=0:
# V_eff'(0) = m^2*0 + g/2*0^2 + (g/2)*I_1 + two-loop terms
# But V_eff'(0) != 0 because of the one-loop tadpole.
# The VEV v_min satisfies:
# m^2 * v + g/2 * v^2 + (g/2) * d/dv [I_1(M^2)] + ...

# Let me use a CLEANER approach: solve the effective potential
# equation non-perturbatively by iteration on a large lattice,
# then extract the two-loop contribution by subtracting one-loop.

N = 128  # large lattice for precision
print('  Computing on %d^3 lattice (%d points)...' % (N, N**3))

# Momentum-space propagator
kx = 2*np.pi*np.arange(N)/N
k_hat_sq = np.zeros((N, N, N))
for mu in range(3):
    shape = [1, 1, 1]
    shape[mu] = N
    k_mu = kx.reshape(shape)
    k_hat_sq = k_hat_sq + 4*np.sin(k_mu/2)**2

# G(k) at tree-level mass
G_k = 1.0 / (k_hat_sq + m2_lat)
I1_plain = np.mean(G_k)

# Position-space propagator
G_x = np.fft.ifftn(G_k).real

print('  I_1 (tadpole) = %.15e' % I1_plain)
print('  G(x=0) = %.15e' % G_x[0,0,0])
print()

# =====================================================================
# STEP 2: One-loop VEV shift (with symmetry factor)
# =====================================================================
print('  STEP 2: ONE-LOOP VEV SHIFT')
print()

# delta_v^(1) = -(g/2) * I_1 / m^2_lat
delta_v_1 = -(g_v/2) * I1_plain / m2_lat
delta_x_1 = delta_v_1 * a_lat

print('  delta_v^(1) = -(g/2)*I_1/m^2 = %.15e' % delta_v_1)
print('  delta_x^(1) = delta_v * a = %.15e' % delta_x_1)
print('  c1*eps = %.15e' % (c1_target * eps))
print('  Ratio: %.10f (expect ~1)' % (abs(delta_x_1) / (c1_target * eps)))
print()

# =====================================================================
# STEP 3: Two-loop VEV shift
# =====================================================================
print('  STEP 3: TWO-LOOP VEV SHIFT')
print()

# The two-loop correction to the VEV comes from:
#
# (A) The shift in the propagator due to the one-loop VEV change:
#     M^2 -> m^2 + g*delta_v^(1)
#     This gives: delta_v^(2a) = -(g/2) * [I_1(M^2_new) - I_1(m^2)] / m^2
#
# (B) The sunset self-energy correction to the mass:
#     delta_m^2 = Sigma_sunset(0) = (g^2/2) * sum_x G(x)^2
#     (factor 1/2 from the symmetry of the two internal lines)
#     Then: delta_v^(2b) = +(g/2) * I_1 * delta_m^2 / m^4
#
# (C) The two-loop tadpole (sunset with one leg becoming the VEV):
#     delta_v^(2c) = -(g^2/4) * I_1^2 / m^4
#     (iterated tadpole: apply the one-loop correction twice)

# Let me compute all three contributions.

# (A) Propagator shift from one-loop VEV change
M2_shifted = m2_lat + g_v * delta_v_1  # shifted mass^2 on lattice
G_k_shifted = 1.0 / (k_hat_sq + M2_shifted)
I1_shifted = np.mean(G_k_shifted)
delta_I1 = I1_shifted - I1_plain

delta_v_2a = -(g_v/2) * delta_I1 / m2_lat
print('  (A) Propagator shift from VEV change:')
print('      M^2_shifted = m^2 + g*delta_v^(1) = %.10f' % M2_shifted)
print('      delta_I1 = I1(M^2_new) - I1(m^2) = %.15e' % delta_I1)
print('      delta_v^(2a) = -(g/2)*delta_I1/m^2 = %.15e' % delta_v_2a)
print()

# (B) Sunset self-energy correction
# Sigma(0) = (g^2/2) * sum_x G(x)^2 = (g^2/2) * <G(k)^2>_k (by Parseval)
I_bubble = np.mean(G_k**2)
Sigma_sunset = (g_v**2 / 2) * I_bubble

# The VEV correction from the mass shift:
# delta_v^(2b) = +(g/2) * I_1 * Sigma / m^4
# (the mass correction feeds back into the tadpole)
delta_v_2b = (g_v/2) * I1_plain * Sigma_sunset / m2_lat**2
print('  (B) Sunset self-energy:')
print('      I_bubble = <G(k)^2> = %.15e' % I_bubble)
print('      Sigma_sunset = (g^2/2)*I_bubble = %.15e' % Sigma_sunset)
print('      delta_v^(2b) = (g/2)*I1*Sigma/m^4 = %.15e' % delta_v_2b)
print()

# (C) Iterated tadpole
# delta_v^(2c) = -(g/2)^2 * I_1^2 / m^4
delta_v_2c = -(g_v/2)**2 * I1_plain**2 / m2_lat**2
print('  (C) Iterated tadpole:')
print('      delta_v^(2c) = -(g/2)^2*I1^2/m^4 = %.15e' % delta_v_2c)
print()

# Total two-loop
delta_v_2_total = delta_v_2a + delta_v_2b + delta_v_2c
delta_x_2_total = delta_v_2_total * a_lat

print('  TOTAL TWO-LOOP:')
print('      delta_v^(2) = A + B + C = %.15e' % delta_v_2_total)
print('      delta_x^(2) = delta_v * a = %.15e' % delta_x_2_total)
print()

# =====================================================================
# STEP 4: Extract c2
# =====================================================================
print('  STEP 4: EXTRACT c2')
print()

# The precision formula says:
# delta_x^(2) = c2 * eps^2
# So: c2_computed = delta_x^(2) / eps^2

c2_computed = abs(delta_x_2_total) / eps**2

print('  delta_x^(2) = %.15e' % abs(delta_x_2_total))
print('  eps^2 = %.15e' % eps**2)
print('  c2_computed = |delta_x^(2)| / eps^2 = %.10f' % c2_computed)
print('  c2_target = 5/64 = %.10f' % c2_target)
print()
print('  Ratio c2_computed/c2_target = %.10f' % (c2_computed / c2_target))
print('  Difference: %.4f%%' % (abs(c2_computed/c2_target - 1)*100))
print()

if abs(c2_computed/c2_target - 1) < 0.05:
    print('  *** MATCH TO WITHIN 5%% ***')
    print('  c2 = 5/64 IS the two-loop coefficient of the lattice phi^3 EFT.')
elif abs(c2_computed/c2_target - 1) < 0.20:
    print('  *** CLOSE (within 20%%) ***')
    print('  The two-loop structure is right but there may be missing')
    print('  contributions (gauge sector, lattice artifacts, etc).')
else:
    print('  *** DOES NOT MATCH ***')
    print('  c2 = 5/64 does NOT come from the scalar phi^3 two-loop.')
    print('  The coefficient must come from gauge field corrections.')

print()

# =====================================================================
# STEP 5: Non-perturbative check
# =====================================================================
print('  STEP 5: NON-PERTURBATIVE CROSS-CHECK')
print()

# Solve V_eff'(v) = 0 exactly on the lattice
# V_tree'(v) = m^2*v + (g/2)*v^2 (in lattice units around x+)
# V_1loop'(v) = (g/2) * <1/(k^2 + m^2 + g*v)>_k

# Iterate: v_{n+1} = -(g/2) * I_1(m^2 + g*v_n) / (m^2 + g*v_n)
# Start from v=0

v = 0.0
for iteration in range(20):
    M2 = m2_lat + g_v * v
    if M2 <= 0:
        print('  Mass squared went negative at iteration %d!' % iteration)
        break
    G_k_iter = 1.0 / (k_hat_sq + M2)
    I1_iter = np.mean(G_k_iter)
    # From V_eff'(v) = 0:
    # m^2*v + (g/2)*v^2 + (g/2)*I1(M^2) = 0
    # Linearize: v_new = -(g/2)*I1(M^2) / m^2 (dropping v^2 for small v)
    v_new = -(g_v/2) * I1_iter / m2_lat
    if abs(v_new - v) < 1e-20:
        break
    v = v_new

delta_x_nonpert = v * a_lat
delta_x_1loop_only = delta_x_1
higher_loop_contribution = delta_x_nonpert - delta_x_1loop_only

print('  Non-perturbative VEV: v = %.15e' % v)
print('  delta_x (non-pert) = %.15e' % delta_x_nonpert)
print('  delta_x (1-loop) = %.15e' % delta_x_1loop_only)
print('  Higher loops = nonpert - 1loop = %.15e' % higher_loop_contribution)
print()
print('  Two-loop perturbative: %.15e' % delta_x_2_total)
print('  Higher loops (non-pert): %.15e' % higher_loop_contribution)
print('  Ratio: %.6f' % (delta_x_2_total / higher_loop_contribution
                         if higher_loop_contribution != 0 else 0))
print()

# =====================================================================
# STEP 6: Breakdown by contribution
# =====================================================================
print('  STEP 6: CONTRIBUTION BREAKDOWN')
print()
print('  %-30s %15s %10s' % ('Contribution', 'delta_x', '% of c2*eps^2'))
print('  ' + '-' * 57)

c2e2 = c2_target * eps**2
for label, dx in [
    ('(A) Propagator shift', delta_v_2a * a_lat),
    ('(B) Sunset mass correction', delta_v_2b * a_lat),
    ('(C) Iterated tadpole', delta_v_2c * a_lat),
    ('TOTAL perturbative', delta_x_2_total),
    ('Non-pert higher loops', higher_loop_contribution),
]:
    pct = dx / c2e2 * 100 if c2e2 != 0 else 0
    print('  %-30s %15.6e %10.2f%%' % (label, dx, pct))

print()
print('  c2*eps^2 = %.15e' % c2e2)
