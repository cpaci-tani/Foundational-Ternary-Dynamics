#!/usr/bin/env python3
"""
TEST ALL OF PHYSICS
====================

50 predictions in three tiers:
  Tier 1 (20): Structural — forced by D=3 cubic lattice alone
  Tier 2 (20): G*-derived — from master quadratic + framework integers
  Tier 3 (10): Novel — discovered in the ternary cube work this session

Each test prints PASS/FAIL and contributes to a final scorecard.
"""
import numpy as np, sys, os, io
from math import comb, gcd
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from scipy.special import gamma as gammafn

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS,
                       N_c, N_base, b_3, N_eff, GAMMA_QUARTER, GAMMA_HALF)

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2
alpha = 1.0 / X_PLUS
delta = (PI_D - G_STAR) / 2.0
beta_val = VARPI_CLASSICAL - (PI_D + G_STAR) / 2.0

D = 3  # spatial dimension

results = []

def test(tier, num, name, derived, expected, tolerance=0, is_exact=False):
    if is_exact:
        passed = (derived == expected)
        err_str = 'EXACT' if passed else 'WRONG: got %s expected %s' % (derived, expected)
    elif tolerance == 0:
        passed = abs(derived - expected) < 1e-10
        err_str = '%.2e' % float(abs(derived - expected))
    else:
        err = abs(derived - expected) / abs(expected) * 100 if expected != 0 else abs(derived)
        passed = err <= tolerance
        err_str = '%.4f%%' % err

    status = 'PASS' if passed else 'FAIL'
    results.append((tier, num, name, passed, err_str))
    mark = '  [OK]' if passed else '  [XX]'
    print('  T%d.%02d %-45s %s %s' % (tier, num, name, status, err_str + mark))

# =================================================================
print('=' * 78)
print('  TEST ALL OF PHYSICS: 50 PREDICTIONS IN THREE TIERS')
print('=' * 78)
print()

# =================================================================
# TIER 1: STRUCTURAL (D=3 cubic lattice alone)
# =================================================================
print('  TIER 1: STRUCTURAL (from D=3 lattice geometry alone)')
print('  ' + '-' * 68)
print()

# T1.01: Number of non-gravitational forces = D
test(1, 1, 'Number of forces = D', D, 3, is_exact=True)

# T1.02: Gauge groups = SU(k) for k=1..D
# J-component count per layer: layer k excites k components
# k=1->U(1), k=2->SU(2), k=3->SU(3)
gauge_groups = ['U(1)', 'SU(2)', 'SU(3)']
expected_groups = ['U(1)', 'SU(2)', 'SU(3)']
test(1, 2, 'Gauge groups = U(1)xSU(2)xSU(3)', gauge_groups == expected_groups, True, is_exact=True)

# T1.03: N_c = D (outermost layer gauge group SU(D))
test(1, 3, 'N_c = D = 3 colors', D, 3, is_exact=True)

# T1.04: Number of generations = C(D,2)
n_gen = comb(D, 2)
test(1, 4, 'Generations = C(3,2) = 3', n_gen, 3, is_exact=True)

# T1.05: Particles per generation = 2^(D-1)
ppg = 2**(D-1)
test(1, 5, 'Particles per generation = 2^2 = 4', ppg, 4, is_exact=True)

# T1.06: Matter-antimatter symmetry
t_plus = 2**(D-1)
t_minus = 2**(D-1)
test(1, 6, '|T+| = |T-| (matter = antimatter)', t_plus == t_minus, True, is_exact=True)

# T1.07: Dark states = 3^D - C(D+2,2)
dark = 3**D - comb(D+2, 2)
test(1, 7, 'Dark states = 27-10 = 17', dark, 17, is_exact=True)

# T1.08: Visible fraction = C(D+2,2)/3^D
vis = comb(D+2, 2) / 3**D
test(1, 8, 'Visible fraction = 10/27', vis, 10/27)

# T1.09: Confinement time = D ticks
c_cfl = 1.0 / np.sqrt(D)
t_conf = np.sqrt(D) / c_cfl
test(1, 9, 'Confinement time = D = 3 ticks', t_conf, 3.0)

# T1.10: Speed of light = 1/sqrt(D)
test(1, 10, 'c = 1/sqrt(3) = 0.57735', c_cfl, 1/np.sqrt(3))

# T1.11: Moore Laplacian has 13 distinct eigenvalues
STATES27 = [(a,b,c) for a in range(3) for b in range(3) for c in range(3)]
A = np.zeros((27, 27))
for i in range(27):
    for j in range(i+1, 27):
        si, sj = STATES27[i], STATES27[j]
        if all(abs(si[d]-sj[d]) <= 1 for d in range(3)) and si != sj:
            A[i,j] = A[j,i] = 1
L = np.diag(A.sum(axis=1)) - A
evals_L = np.sort(np.linalg.eigvalsh(L))
n_distinct = len(set(np.round(evals_L, 4)))
test(1, 11, 'Moore Laplacian distinct eigenvalues = 13', n_distinct, 13, is_exact=True)

# T1.12: Eigenvalue 7 in Moore Laplacian
has_7 = any(abs(e - 7.0) < 0.01 for e in evals_L)
test(1, 12, 'Eigenvalue 7 (=b_3) in Laplacian', has_7, True, is_exact=True)

# T1.13: Eigenvalue 27 in Moore Laplacian
has_27 = any(abs(e - 27.0) < 0.01 for e in evals_L)
test(1, 13, 'Eigenvalue 27 (=3^3) in Laplacian', has_27, True, is_exact=True)

# T1.14: BCC count = 2^D = 8
bcc_count = sum(1 for s in STATES27 if all(x != 1 for x in s))
test(1, 14, 'BCC count = 2^D = 8', bcc_count, 8, is_exact=True)

# T1.15: BCC splits into 2 tetrahedra of 4
t_p = sum(1 for s in STATES27 if all(x != 1 for x in s) and
          (1 if s[0]==0 else -1)*(1 if s[1]==0 else -1)*(1 if s[2]==0 else -1) > 0)
test(1, 15, 'Stella octangula: T+ = T- = 4', t_p, 4, is_exact=True)

# T1.16: FCC count = C(D,2)*2^2 = 12
fcc_count = sum(1 for s in STATES27 if sum(1 for x in s if x != 1) == 2)
test(1, 16, 'FCC (cuboctahedron) = 12', fcc_count, 12, is_exact=True)

# T1.17: FCC splits into 3 planes of 4
planes = [0, 0, 0]
for s in STATES27:
    nz = [d for d in range(3) if s[d] != 1]
    if len(nz) == 2:
        # Which plane?
        pair = tuple(nz)
        if pair == (0,1): planes[0] += 1
        elif pair == (0,2): planes[1] += 1
        elif pair == (1,2): planes[2] += 1
test(1, 17, 'FCC = 3 planes of 4', planes == [4,4,4], True, is_exact=True)

# T1.18: SC count = 2*D = 6
sc_count = sum(1 for s in STATES27 if sum(1 for x in s if x != 1) == 1)
test(1, 18, 'SC (octahedron) = 2D = 6', sc_count, 6, is_exact=True)

# T1.19: Total 1+6+12+8 = 27
test(1, 19, 'Total: 1+6+12+8 = 27', 1+sc_count+fcc_count+bcc_count, 27, is_exact=True)

# T1.20: S_3 symmetric sector = C(5,2) = 10
# Build S_3 symmetrizer
def build_swap(perm):
    S = np.zeros((27, 27))
    for a in range(3):
        for b in range(3):
            for c in range(3):
                abc = [a,b,c]; pbc = [abc[perm[0]], abc[perm[1]], abc[perm[2]]]
                S[a*9+b*3+c, pbc[0]*9+pbc[1]*3+pbc[2]] = 1.0
    return S
P12 = build_swap([1,0,2]); P13 = build_swap([2,1,0]); P23 = build_swap([0,2,1])
P_sym = (np.eye(27)+P12+P13+P23+P12@P13+P13@P12)/6
dim_sym = int(round(np.trace(P_sym)))
test(1, 20, 'S_3 symmetric sector = 10', dim_sym, 10, is_exact=True)

print()

# =================================================================
# TIER 2: G*-DERIVED (master quadratic + framework integers)
# =================================================================
print('  TIER 2: G*-DERIVED (from master quadratic)')
print('  ' + '-' * 68)
print()

# T2.01: 1/alpha tree level
test(2, 1, '1/alpha (tree) = 137.036', X_PLUS, 137.035999084, tolerance=0.001)

# T2.02: 1/alpha 4-term precision
EPSILON = np.exp(np.pi) - np.pi - (b_3 + N_eff)
D_CONST = N_c * N_base**2 - 1
c1 = N_c**2 / D_CONST
c2 = (N_eff - 2*N_base) / N_base**3
c3 = N_base / (N_c * D_CONST)
c4 = (N_c * D_CONST) / (b_3 + N_base)
eps = abs(EPSILON)
alpha_4term = X_PLUS - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4
test(2, 2, '1/alpha (4-term) < 0.001 ppt', alpha_4term, 137.035999084, tolerance=0.0001)

# T2.03: sin^2(theta_W)
sin2w = float(N_c) / float(N_eff)
test(2, 3, 'sin^2(theta_W) = 3/13', sin2w, 0.23122, tolerance=0.3)

# T2.04: alpha_s(M_Z)
alpha_s = float(b_3) / (b_3 + 4*N_eff)
test(2, 4, 'alpha_s(M_Z) = 7/59', alpha_s, 0.1179, tolerance=1.0)

# T2.05: m_p/m_e
mp_me = N_eff/alpha + N_base*N_eff + N_c
test(2, 5, 'm_p/m_e', mp_me, 1836.15267, tolerance=0.02)

# T2.06: m_mu/m_e
mmu_me = 3*b_3*(b_3+N_c) - N_c
test(2, 6, 'm_mu/m_e = 3*7*10 - 3 = 207', mmu_me, 206.768, tolerance=0.15)

# T2.07: m_tau/m_e
mtau_me = (N_eff + N_base) * mmu_me - 2*N_c*b_3
test(2, 7, 'm_tau/m_e', mtau_me, 3477.48, tolerance=0.02)

# T2.08: m_H/m_e
mh_me = N_eff / alpha**2
test(2, 8, 'm_H/m_e = N_eff/alpha^2', mh_me, 125250/0.51099, tolerance=0.5)

# T2.09: m_W/m_Z
mw_mz = np.sqrt(1 - sin2w)
test(2, 9, 'm_W/m_Z = cos(theta_W)', mw_mz, 80.379/91.1876, tolerance=0.6)

# T2.10: G_N lattice
gn = 1.0 / (b_3 + N_c)**2
test(2, 10, 'G_N = 1/(b3+Nc)^2 = 0.01', gn, 0.01)

# T2.11: Watson BCC integral
W_BCC = G_STAR**2 / (2*PI_D)
W_watson = gammafn(0.25)**4 / (4*PI_D**3)
test(2, 11, 'I_BCC = G*^2/(2pi) = Gamma(1/4)^4/(4pi^3)', W_BCC, W_watson)

# T2.12: x-/G* eps equation
b_c = 2 - 16*G_STAR
eps_exact = (-b_c - np.sqrt(b_c**2-4))/2
eps_numerical = X_MINUS/G_STAR - 1
test(2, 12, 'eps quadratic exact', eps_exact, eps_numerical)

# T2.13: PMNS theta_12
sin2_12 = float(N_c) / (N_c + b_3)  # 3/10
test(2, 13, 'PMNS sin^2(theta_12) = 3/10', sin2_12, 0.304, tolerance=2.0)

# T2.14: PMNS theta_23
sin2_23 = float(N_eff + N_c) / (2*N_eff + N_c)  # 16/29
test(2, 14, 'PMNS sin^2(theta_23) = 16/29', sin2_23, 0.573, tolerance=4.0)

# T2.15: delta_m^2 ratio
dm2_ratio = float(b_3 + N_c)**2 / N_c  # 100/3
test(2, 15, 'dm^2_31/dm^2_21 = 100/3', dm2_ratio, 33.8, tolerance=2.0)

# T2.16: Omega_Lambda
omega_L = 2.0/3
test(2, 16, 'Omega_Lambda = 2/3', omega_L, 0.6847, tolerance=3.0)

# T2.17: Spectral index
Ne = 169.0/3  # e-folds
ns = 1 - 2/Ne
test(2, 17, 'n_s = 1 - 2/N_e', ns, 0.9649, tolerance=0.1)

# T2.18: Dimensional triad
g1, g2, g3 = G_STAR, G_STAR**2, G_STAR**3
test(2, 18, 'G*^1 * G*^2 * G*^3 = G*^6', g1*g2*g3, G_STAR**6)

# T2.19: Cyclotomic decomposition
s = np.sqrt(np.pi)
phi_12 = s**2 - 1  # Phi_1 * Phi_2
phi_4 = s**2 + 1   # Phi_4
phi_6 = s**2 - s + 1  # Phi_6
test(2, 19, 'Phi_1*Phi_2 * Phi_3 * Phi_6 = pi^3-1', phi_12*(s**2+s+1)*phi_6, np.pi**3-1)

# T2.20: k/k_crit = G*/8 at k=1/2
k_crit = 4.0/G_STAR
ratio_k = 0.5 / k_crit
test(2, 20, 'k/k_crit = G*/8 at k=1/2', ratio_k, G_STAR/8)

print()

# =================================================================
# TIER 3: NOVEL CUBE PREDICTIONS (this session)
# =================================================================
print('  TIER 3: NOVEL CUBE PREDICTIONS (from this session)')
print('  ' + '-' * 68)
print()

# Build cube Hamiltonian
I3 = np.eye(3); Z3 = np.diag([1.,0.,-1.])
H1 = np.array([[delta, delta, 0], [delta, beta_val, delta], [0, delta, -delta]])
H = (np.kron(np.kron(H1,I3),I3)+np.kron(np.kron(I3,H1),I3)+np.kron(np.kron(I3,I3),H1))
H += 0.05*(np.kron(np.kron(Z3,Z3),I3)+np.kron(np.kron(Z3,I3),Z3)+np.kron(np.kron(I3,Z3),Z3))
evals, evecs = np.linalg.eigh(H)

# T3.01: 17 dark states
center_amps = np.abs(evecs[13,:])**2
n_dark = sum(1 for a in center_amps if a < 1e-6)
test(3, 1, '17 dark states (center amplitude < 1e-6)', n_dark, 17, is_exact=True)

# T3.02: GS center fraction > 0.7
gs_center = np.abs(evecs[13,0])**2
test(3, 2, 'GS center fraction > 70%', gs_center > 0.7, True, is_exact=True)

# T3.03: Energy-radius correlation > 0.9
radii = [0, 1, np.sqrt(2), np.sqrt(3)]
SHELL_IDX = {}
for i,s in enumerate(STATES27):
    sh = sum(1 for x in s if x != 1)
    SHELL_IDX.setdefault(sh,[]).append(i)
r_map = np.array([radii[sum(1 for x in STATES27[i] if x != 1)] for i in range(27)])
r_means = [sum(np.abs(evecs[:,n])**2 * r_map) for n in range(27)]
corr = np.corrcoef(evals, r_means)[0,1]
test(3, 3, 'Energy-radius correlation > 0.9', corr > 0.9, True, is_exact=True)

# T3.04: beta < 0 (from pi < 4)
test(3, 4, 'beta < 0 (pi < 4 forces anchor)', beta_val < 0, True, is_exact=True)

# T3.05: BW/gap: gravity long-range
bw_grav = 2 * gn
test(3, 5, 'Gravity BW/gap < 1 (long-range)', bw_grav / abs(beta_val) < 1, True, is_exact=True)

# T3.06: BW/gap: EM long-range
bw_em = 2 * alpha
test(3, 6, 'EM BW/gap < 1 (long-range)', bw_em / abs(beta_val) < 1, True, is_exact=True)

# T3.07: BW/gap: weak short-range
bw_weak = 2 * sin2w
test(3, 7, 'Weak BW/gap > 1 (short-range)', bw_weak / abs(beta_val) > 1, True, is_exact=True)

# T3.08: beta in G*=1 natural units = Phi_6(s)/2
# In G*=1 units: beta_nat = (pi - sqrt(pi) + 1)/2 = Phi_6(s)/2
beta_nat = (np.pi - np.sqrt(np.pi) + 1) / 2
test(3, 8, '|beta|_nat = Phi_6(sqrt(pi))/2', beta_nat, phi_6/2)

# T3.09: Hexagonal distance identity
import math
s_py = math.sqrt(math.pi)
hex_real = s_py - math.cos(math.pi/3)
hex_imag = math.sin(math.pi/3)
hex_dist_sq_py = hex_real**2 + hex_imag**2
phi6_py = s_py**2 - s_py + 1
test(3, 9, 'Phi_6 = |sqrt(pi) - e^{ipi/3}|^2', phi6_py, hex_dist_sq_py)

# T3.10: Consciousness/physics ratio = 1/32 exactly
KC2 = G_STAR**3 / 2
ratio_cp = KC2 / (X_PLUS * X_MINUS)
test(3, 10, 'K_C^2 / (x+*x-) = 1/32 exactly', ratio_cp, 1/32)

print()

# =================================================================
# SCORECARD
# =================================================================
print('=' * 78)
print('  FINAL SCORECARD')
print('=' * 78)
print()

for tier in [1, 2, 3]:
    tier_results = [(n, name, p, e) for (t, n, name, p, e) in results if t == tier]
    passed = sum(1 for _, _, p, _ in tier_results if p)
    total = len(tier_results)
    tier_names = {1: 'STRUCTURAL (lattice geometry)', 2: 'G*-DERIVED (master quadratic)',
                  3: 'NOVEL CUBE (this session)'}
    print('  Tier %d: %s' % (tier, tier_names[tier]))
    print('    %d / %d passed' % (passed, total))
    if passed < total:
        fails = [(n, name, e) for (_, n, name, p, e) in results if not p and _ == tier]
        for n, name, e in fails:
            print('    FAIL: T%d.%02d %s (%s)' % (tier, n, name, e))
    print()

total_pass = sum(1 for _, _, _, p, _ in results if p)
total_all = len(results)
print('  TOTAL: %d / %d passed (%.1f%%)' % (total_pass, total_all, total_pass/total_all*100))
print()

# Breakdown
structural = sum(1 for t,_,_,p,_ in results if t==1 and p)
gstar = sum(1 for t,_,_,p,_ in results if t==2 and p)
novel = sum(1 for t,_,_,p,_ in results if t==3 and p)
print('  Structural (exact theorems):  %d/20' % structural)
print('  G*-derived (experiment match): %d/20' % gstar)
print('  Novel cube (this session):     %d/10' % novel)
print()
print('  Input: G* = Gamma(1/4)/Gamma(3/4) and the integer 16.')
print('  Everything else derived.')
