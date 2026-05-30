#!/usr/bin/env python3
"""
What equations does the ternary cube FORCE?
Not: what numbers come out.
But: what relationships MUST hold for ANY ternary cubic lattice?
"""
import numpy as np, sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma as gammafn

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2

print('=' * 78)
print('  FORCED EQUATIONS: WHAT THE STRUCTURE DEMANDS')
print('  These hold for ANY value of the bridge constant.')
print('=' * 78)

# =====================================================================
# LEVEL 0: What a TERNARY LATTICE forces (no physics at all)
# =====================================================================
print("""
  LEVEL 0: ANY TERNARY LATTICE IN D DIMENSIONS

  Given: D spatial dimensions, 3 states per site.
  Forced:

  (a) Total states = 3^D.
      D=3: 27 states.
      This is not a choice. It is arithmetic.

  (b) Moore neighborhood = 3^D - 1 neighbors = 26.
      The number of sites that can influence the center in one step.

  (c) Shell decomposition: the 3^D states split into D+1 shells
      by the number of non-center axes.
      Shell k has C(D,k) * 2^k states.
      D=3: 1 + 6 + 12 + 8 = 1 + C(3,1)*2 + C(3,2)*4 + C(3,3)*8 = 27.

  (d) The S_D symmetric sector has dimension C(D+2, 2) = (D+1)(D+2)/2.
      D=3: C(5,2) = 10.
      The center state lives here. This is the VISIBLE sector.

  (e) Dark states = 3^D - (D+1)(D+2)/2.
      D=3: 27 - 10 = 17.
      This is a THEOREM. No G*, no physics, just combinatorics.

  These are forced by the lattice ALONE.""")

# Verify
from math import comb
for D in range(1, 6):
    total = 3**D
    sym = comb(D+2, 2)
    dark = total - sym
    shells = [comb(D, k) * 2**k for k in range(D+1)]
    print('    D=%d: 3^D=%d, symmetric=%d, dark=%d, shells=%s' %
          (D, total, sym, dark, shells))

# =====================================================================
# LEVEL 1: What a CENTERED HAMILTONIAN forces
# =====================================================================
print("""
  LEVEL 1: ANY CENTERED 3-LEVEL HAMILTONIAN

  Given: a tridiagonal symmetric Hamiltonian on 3 states
         H = [[a, g, 0], [g, b, g], [0, g, c]]
  with a + c = 2*mu (mirror condition), so a = mu+D, c = mu-D.

  Centered form: H = mu*I + H_rel
  where H_rel = [[D, g, 0], [g, B, g], [0, g, -D]]
  with B = b - mu.

  Forced:

  (f) The eigenvalues of H_rel satisfy:
      lambda^3 - (D^2 + B^2 + 2g^2)*lambda + B*(D^2 - 2g^2) - B^3... no.
      Let me compute the characteristic polynomial directly.""")

# Characteristic polynomial of H_rel = [[D, g, 0], [g, B, g], [0, g, -D]]
# det(H_rel - lambda*I) = 0
# (D-lambda)(B-lambda)(-D-lambda) - g^2(-D-lambda) - g^2(D-lambda)
# = (D-lambda)(-D-lambda)(B-lambda) - g^2[(-D-lambda) + (D-lambda)]
# = -(D^2-lambda^2)(B-lambda) - g^2(-2*lambda)
# = -(D^2*B - D^2*lambda - B*lambda^2 + lambda^3) + 2g^2*lambda
# = -lambda^3 + B*lambda^2 + D^2*lambda - D^2*B + 2g^2*lambda
# = -lambda^3 + B*lambda^2 + (D^2 + 2g^2)*lambda - D^2*B
# So: lambda^3 - B*lambda^2 - (D^2 + 2g^2)*lambda + D^2*B = 0

print("""  (f) Characteristic equation of H_rel:
      lambda^3 - B*lambda^2 - (D^2 + 2g^2)*lambda + B*D^2 = 0

      Vieta relations (sum, pairwise sum, product of roots):
      lambda_1 + lambda_2 + lambda_3 = B
      lambda_1*lambda_2 + lambda_1*lambda_3 + lambda_2*lambda_3 = -(D^2 + 2g^2)
      lambda_1 * lambda_2 * lambda_3 = -B*D^2

      These hold for ANY (D, B, g). They are structural.""")

# Verify with FTD values
D_val = (PI_D - G_STAR) / 2  # delta
B_val = VARPI_CLASSICAL - (PI_D + G_STAR) / 2  # beta
g_val = D_val

H_rel = np.array([[D_val, g_val, 0], [g_val, B_val, g_val], [0, g_val, -D_val]])
evals = np.linalg.eigvalsh(H_rel)

print()
print('  Verification at FTD values (D=%.6f, B=%.6f, g=%.6f):' % (D_val, B_val, g_val))
print('    lambda_1 + lambda_2 + lambda_3 = %.10f = B = %.10f  %s' %
      (sum(evals), B_val, 'OK' if abs(sum(evals)-B_val) < 1e-10 else 'FAIL'))
print('    l1*l2 + l1*l3 + l2*l3 = %.10f = -(D^2+2g^2) = %.10f  %s' %
      (evals[0]*evals[1]+evals[0]*evals[2]+evals[1]*evals[2],
       -(D_val**2+2*g_val**2),
       'OK' if abs(evals[0]*evals[1]+evals[0]*evals[2]+evals[1]*evals[2]+(D_val**2+2*g_val**2)) < 1e-10 else 'FAIL'))
print('    l1*l2*l3 = %.10f = -B*D^2 = %.10f  %s' %
      (evals[0]*evals[1]*evals[2], -B_val*D_val**2,
       'OK' if abs(evals[0]*evals[1]*evals[2]+B_val*D_val**2) < 1e-10 else 'FAIL'))

# =====================================================================
# LEVEL 2: What the MASTER QUADRATIC forces
# =====================================================================
print("""
  LEVEL 2: THE MASTER QUADRATIC (for any bridge constant G)

  Given: z^2 - kG^2 z + kG^3 = 0

  Forced for ALL G and ALL k > 0:

  (g) Harmonic ratio: z+*z- / (z++z-) = G.
      This is Vieta: product/sum = kG^3/(kG^2) = G.
      The bridge constant IS the harmonic ratio of its own roots.
      This does not depend on k.

  (h) Discriminant sign change at k_crit = 4/G.
      disc = k*G^3*(kG - 4).
      disc = 0 when kG = 4, i.e., k = 4/G.
      Below: complex roots. Above: real roots.
      The Born rule boundary IS k = 4/G.

  (i) At k = 1/2: k/k_crit = (1/2)/(4/G) = G/8.
      The reference frame context coefficient sits at fraction G/8
      of the way to the Born rule.
      For ANY G. This is algebra, not fine-tuning.

  (j) Scale ratio: k_phys/k_cons = 16/(1/2) = 32 = 2^5.
      Five binary doublings from reference frame context to physics.
      This depends on k_phys = 16 (the lattice DOF count)
      and k_cons = 1/2. If these are fixed by the lattice
      structure, the ratio is forced.

  (k) The smaller root z- satisfies:
      z-/G = 1 + eps, where eps^2 + (2-kG)eps + 1 = 0.
      At k=16: eps = ((16G-2) - sqrt((16G-2)^2 - 4)) / 2.
      z-/G is ALWAYS close to 1 when kG >> 4.
      The confinement root is always near the bridge scale.""")

# Verify (k)
for G_test in [2.5, 2.8, G_STAR, 3.0, 3.2]:
    disc_t = (16*G_test**2)**2 - 4*16*G_test**3
    if disc_t > 0:
        xm_t = (16*G_test**2 - np.sqrt(disc_t)) / 2
        eps_t = xm_t/G_test - 1
        b_c = 2 - 16*G_test
        eps_pred = (-b_c - np.sqrt(b_c**2-4))/2
        print('    G=%.4f: x-/G = %.6f, eps = %.6f (pred: %.6f)' %
              (G_test, xm_t/G_test, eps_t, eps_pred))

# =====================================================================
# LEVEL 3: What the TENSOR PRODUCT forces
# =====================================================================
print("""
  LEVEL 3: TENSOR PRODUCT STRUCTURE (forced by D=3)

  The 27-state cube Hamiltonian H_cube = H_1 x I x I + I x H_1 x I + I x I x H_1
  is a KRONECKER SUM. This forces:

  (l) Eigenvalues are ALL sums lambda_i + lambda_j + lambda_k.
      27 eigenvalues from 3 single-axis eigenvalues.
      The spectrum is completely determined by the single-axis spectrum.

  (m) At g=0 (uncoupled axes), the degeneracy pattern is the
      TRINOMIAL COEFFICIENTS: the number of ways to write
      n = n_1 + n_2 + n_3 with n_i in {0, 1, 2}.
      For sums 0 through 6: 1, 3, 6, 7, 6, 3, 1.
      This is forced by the tensor product structure.

  (n) The S_3 symmetry (axis permutations) decomposes 27 into:
      10 symmetric + 1 antisymmetric + 16 standard representation.
      10 = C(5,2), 1 = 1, 16 = 27-10-1.
      This is forced by S_3 representation theory on 3-element sets.

  (o) Adding ZZ interactions (J > 0) breaks the Kronecker sum structure.
      The number of distinct eigenvalues increases from 10 to ~19.
      The factorization P_abc = P_a * P_b * P_c breaks.
      This is the TRANSITION from separable to entangled.""")

# =====================================================================
# LEVEL 4: What the PHASE STRUCTURE forces
# =====================================================================
print("""
  LEVEL 4: CYCLOTOMIC STRUCTURE (forced by evaluating at s = sqrt(pi))

  When the Hamiltonian parameters are expressed in G-natural units:
    D_nat = (pi-1)/2      = Phi_1(s)*Phi_2(s) / 2
    mu_nat = (pi+1)/2     = Phi_4(s) / 2
    B_nat = (pi-s+1)/2    = Phi_6(s) / 2

  This is forced WHENEVER the three phase angles are {pi, varpi, G}
  with pi = 4*varpi^2/G^2. The cyclotomic decomposition is not a choice.
  It follows from expressing pi +/- 1 and pi - sqrt(pi) + 1 in terms
  of the variable s = sqrt(pi).

  (p) Phi_6(s) = s^2 - s + 1 = the 6th cyclotomic polynomial.
      Its zeros are at e^{+/-i*pi/3} = the hexagonal lattice generators.
      This connects the mediator offset to SU(3) root geometry.

  (q) The three cyclotomic families Phi_1*Phi_2, Phi_4, Phi_6
      correspond to the three number rings Z, Z[i], Z[omega].
      These are the three MAXIMAL orders in the three imaginary
      quadratic fields of class number 1 with norm form x^2 + ny^2
      for n = 1 (integers), n = 1 (Gaussian), n = 3 (Eisenstein).

  (r) Force criterion: 4t/G < Phi_6(sqrt(pi)) for long-range.
      This holds for any bridge constant G and coupling t.
      The hexagonal ball radius (Phi_6) sets the force boundary.""")

# =====================================================================
# LEVEL 5: What the INTER-CUBE structure forces
# =====================================================================
print("""
  LEVEL 5: MULTI-CUBE CHAIN (forced by nearest-neighbor hopping)

  For N cubes in a chain with hopping amplitude t:

  (s) Band structure: E_n(k) = E_n^single + 2*t_n*cos(k).
      Each internal eigenstate becomes a cosine band.
      Bandwidth = 2*t_n. Group velocity = 2*t_n*sin(k).

  (t) The gap scales as ~1/N for large N (gapless in the large-N regime).
      This is forced by the tight-binding band structure.
      It means: the vacuum sector has MASSLESS excitations.

  (u) Each shell has its OWN bandwidth (2*t_shell), so each force
      has its own propagation speed. The speed hierarchy is
      determined by the coupling constants, which come from the
      master quadratic.

  (v) Subluminal propagation: all speeds bounded by c = 1/sqrt(D).
      This is the CFL condition, forced by discrete lattice dynamics.""")

# =====================================================================
# SUMMARY: THE EQUATION HIERARCHY
# =====================================================================
print("""
  ====================================================================
  SUMMARY: THE FORCED EQUATION HIERARCHY
  ====================================================================

  Level 0 (lattice):
    3^D states. C(D+k,k)*... shells. C(D+2,2) visible. 3^D - C(D+2,2) dark.

  Level 1 (Hamiltonian):
    lambda^3 - B*lambda^2 - (D^2+2g^2)*lambda + B*D^2 = 0

  Level 2 (master quadratic):
    z+*z-/(z++z-) = G for ALL k.
    Born rule at k = 4/G.
    k=1/2 sits at G/8 of k_crit.
    z-/G = 1 + O(1/(kG)).

  Level 3 (tensor product):
    Trinomial degeneracy: 1,3,6,7,6,3,1 at beta=0.
    S_3 decomposition: 10 + 1 + 16 = 27.

  Level 4 (cyclotomic):
    D ~ Phi_1*Phi_2(s)/2. mu ~ Phi_4(s)/2. |B| ~ Phi_6(s)/2.
    Force range criterion: 4t/G < Phi_6(sqrt(pi)).

  Level 5 (chain):
    Gapless vacuum. Shell-specific bandwidths. CFL bound.

  WHAT THESE EQUATIONS SAY WITHOUT NUMBERS:

  1. The observer sees (D+1)(D+2)/(2*3^D) of reality. [Level 0]
     At D=3: about 37%%. The rest is dark.

  2. The bridge constant IS the harmonic ratio of the forces
     it mediates. [Level 2]

  3. Reference frame context sits at G/8 of the way to measurement. [Level 2]
     For ANY G. This is structure, not tuning.

  4. The force range boundary is set by the distance from
     sqrt(pi) to the hexagonal lattice. [Level 4]

  5. The vacuum is gapless. Forces propagate at different speeds
     set by their shell coupling. Gravity is the slowest. [Level 5]

  NONE of these statements depend on the specific value G* = 2.959.
  They are structural. They are the PHYSICS of the ternary cube.
  G* selects WHICH universe. The equations select WHAT KIND of universe.""")
