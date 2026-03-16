"""
THE DECISIVE COMPUTATION: Does the partition function produce the master quadratic?

Setup: FTD lattice on the 2x2x2 periodic torus.
- 8 sites, each with state s ∈ {-1, 0, +1} and flux J ∈ R³
- The J-integral is Gaussian (exact)
- After integrating out J, we get an effective action S_eff[s]
- Leave the coupling g as a FREE PARAMETER
- Impose self-consistency: the coupling that enters = the coupling that emerges
- Check if the self-consistency equation IS the master quadratic

The key insight from our analysis:
- The div-coupling gives G_charge = 1/c² = 3 (trivial, k-independent)
- The PHYSICAL self-energy comes from the Coulomb potential (scalar propagator)
- We need to find the self-consistency condition on the FULL effective action

Strategy: Compute the free energy F(g) = -ln Z(g) for the full ternary sum
over all 3^8 = 6561 configurations, with g as a free parameter. Look for
structure in F(g) that reproduces the master quadratic.

Multiple approaches tried simultaneously:
1. Extremum of F(g): dF/dg = 0
2. Self-consistent coupling: g²_out = g²_in where g²_out comes from the effective action
3. The effective mass / pole of the propagator as a function of g
4. The ratio of partition functions Z(g)/Z(0) and its structure
"""

import numpy as np
from scipy.special import gamma
from scipy.optimize import minimize_scalar, brentq
import itertools
import time

# ===========================================================================
# Constants
# ===========================================================================
GAMMA_Q = gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_Q**2 / (2 * np.pi)
ALPHA = 1.0 / (8 * G_STAR**2 * (1 + np.sqrt(1 - 1/(4*G_STAR))))
X_PLUS = 1.0 / ALPHA
X_MINUS = 16*G_STAR**3 / X_PLUS  # from Vieta product
C2 = 1.0/3.0

print("=" * 80)
print("THE DECISIVE COMPUTATION")
print("Does the 2x2x2 torus partition function produce the master quadratic?")
print("=" * 80)
print(f"  G*      = {G_STAR:.10f}")
print(f"  alpha   = {ALPHA:.10f}")
print(f"  x+      = {X_PLUS:.6f}")
print(f"  x-      = {X_MINUS:.6f}")
print(f"  c^2     = {C2}")
print()

# ===========================================================================
# Build the lattice
# ===========================================================================
L = 2
N = L**3  # 8 sites
D = 3

sites = [(x,y,z) for x in range(L) for y in range(L) for z in range(L)]
idx = {s:i for i,s in enumerate(sites)}

# 6-neighbor scalar Laplacian
Delta = np.zeros((N, N))
for i, (x,y,z) in enumerate(sites):
    Delta[i,i] = -6
    for dx,dy,dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
        j = idx[((x+dx)%L, (y+dy)%L, (z+dz)%L)]
        Delta[i,j] += 1

# Wave operator M = -c^2 * Delta
M = -C2 * Delta

# Divergence operator (N x 3N)
Div = np.zeros((N, D*N))
for i, (x,y,z) in enumerate(sites):
    for mu in range(D):
        Div[i, mu*N + i] -= 1
        if mu == 0: fwd = idx[((x+1)%L, y, z)]
        elif mu == 1: fwd = idx[(x, (y+1)%L, z)]
        else: fwd = idx[(x, y, (z+1)%L)]
        Div[i, mu*N + fwd] += 1

# Vector wave operator
M_vec = np.kron(np.eye(D), M)

# Pseudoinverse (handles zero modes)
M_pinv = np.linalg.pinv(M_vec, rcond=1e-10)

# Charge-charge Green's function: G_cc = Div @ M_pinv @ Div^T
G_cc = Div @ M_pinv @ Div.T

# Scalar Coulomb Green's function: G_coulomb = M_scalar_pinv
M_scalar_pinv = np.linalg.pinv(M, rcond=1e-10)

print("LATTICE CONSTRUCTED")
print(f"  Sites: {N}, DOF: {D*N}")
print(f"  G_charge[0,0] = {G_cc[0,0]:.6f} (div-coupling self-energy)")
print(f"  G_coulomb[0,0] = {M_scalar_pinv[0,0]:.6f} (scalar Coulomb self-energy)")
print()

# ===========================================================================
# Approach 1: Full partition function Z(g) over all 3^8 configs
# ===========================================================================
print("=" * 80)
print("APPROACH 1: Full Z(g) = Sum_s exp(g^2/2 * s^T G s)")
print("=" * 80)

def compute_Z(g_sq, G_matrix):
    """Z(g²) summing over all 3^8 ternary configurations."""
    Z = 0.0
    for config in itertools.product([-1, 0, 1], repeat=N):
        s = np.array(config, dtype=float)
        exponent = g_sq / 2.0 * s @ G_matrix @ s
        Z += np.exp(exponent)
    return Z

def compute_Z_and_moments(g_sq, G_matrix):
    """Compute Z, <s^T G s>, and <(s^T G s)^2> for self-consistency analysis."""
    Z = 0.0
    moment1 = 0.0
    moment2 = 0.0
    for config in itertools.product([-1, 0, 1], repeat=N):
        s = np.array(config, dtype=float)
        Q = s @ G_matrix @ s
        w = np.exp(g_sq / 2.0 * Q)
        Z += w
        moment1 += Q * w
        moment2 += Q**2 * w
    return Z, moment1/Z, moment2/Z

# Compute with both Green's functions
print("\nComputing Z(g²) with charge-charge Green's function...")
t0 = time.time()

g_sq_values = np.linspace(0.0, 0.05, 50)
results_cc = []
results_coulomb = []

for g_sq in g_sq_values:
    Z_cc, m1_cc, m2_cc = compute_Z_and_moments(g_sq, G_cc)
    Z_co, m1_co, m2_co = compute_Z_and_moments(g_sq, M_scalar_pinv)
    results_cc.append((g_sq, Z_cc, m1_cc, m2_cc))
    results_coulomb.append((g_sq, Z_co, m1_co, m2_co))

dt = time.time() - t0
print(f"  Done in {dt:.1f}s")
print()

# ===========================================================================
# Approach 2: Self-consistency from the effective coupling
# ===========================================================================
print("=" * 80)
print("APPROACH 2: Self-consistent coupling")
print("=" * 80)
print()
print("The effective coupling g²_eff is defined by the susceptibility:")
print("  g²_eff = d²F/d(h)² where F = -ln Z and h is an external source")
print()
print("Equivalently, for the Gaussian effective action:")
print("  g²_eff(g²_in) = g²_in * <s^T G s> / N_active")
print()
print("Self-consistency: g²_eff = g²_in => <s^T G s> / N_active = 1")
print()

# Check <s^T G s> as function of g²
print("g²_in      <s^T G_cc s>    <s^T G_co s>    <s^T G_cc s>/N")
print("-" * 70)
for g_sq, Z, m1, m2 in results_cc:
    _, _, m1_co, _ = [(g,z,m,m2) for g,z,m,m2 in results_coulomb if abs(g-g_sq)<1e-12][0]
    if g_sq > 0:
        print(f"{g_sq:10.6f}  {m1:14.6f}  {m1_co:14.6f}  {m1/N:14.6f}")

# ===========================================================================
# Approach 3: The susceptibility ratio
# ===========================================================================
print()
print("=" * 80)
print("APPROACH 3: Susceptibility and effective coupling")
print("=" * 80)
print()

# The susceptibility chi = d<Q>/dg² where Q = s^T G s
# chi = <Q²> - <Q>² (the connected correlator)
# The effective coupling: g²_eff = g²_in * chi / chi_0 where chi_0 = chi(g=0)

# At g=0, all configs equally weighted:
# <Q>_0 = (1/3^N) Sum_s Q(s)
# <Q²>_0 = (1/3^N) Sum_s Q(s)²

Z0, m1_0_cc, m2_0_cc = compute_Z_and_moments(0.0, G_cc)
Z0, m1_0_co, m2_0_co = compute_Z_and_moments(0.0, M_scalar_pinv)
chi_0_cc = m2_0_cc - m1_0_cc**2
chi_0_co = m2_0_co - m1_0_co**2

print(f"At g=0 (uniform measure over 3^{N} configs):")
print(f"  G_cc:     <Q> = {m1_0_cc:.6f}, <Q²> = {m2_0_cc:.6f}, chi = {chi_0_cc:.6f}")
print(f"  G_coulomb: <Q> = {m1_0_co:.6f}, <Q²> = {m2_0_co:.6f}, chi = {chi_0_co:.6f}")
print()

# The self-consistency equation from the partition function:
# The "renormalized" coupling at one loop is:
#   1/g²_R = 1/g²_bare - Sigma(g²)
# where Sigma is the self-energy correction from summing over ternary configs.
#
# If the theory is self-consistent, g²_R = g²_bare, so Sigma = 0.
# This gives a fixed-point equation.

# But more directly: the free energy F(g²) = -ln Z(g²) encodes everything.
# Let's look at its structure.

print("FREE ENERGY STRUCTURE:")
print(f"{'g_sq':>10} {'F_cc':>14} {'F_co':>14} {'dF/dg_cc':>14} {'dF/dg_co':>14}")
print("-" * 70)

F_cc = [-np.log(Z) for _, Z, _, _ in results_cc]
F_co = [-np.log(Z) for _, Z, _, _ in results_coulomb]

for i, g_sq in enumerate(g_sq_values):
    if i > 0 and i < len(g_sq_values)-1:
        dg = g_sq_values[1] - g_sq_values[0]
        dF_cc = (F_cc[i+1] - F_cc[i-1]) / (2*dg)
        dF_co = (F_co[i+1] - F_co[i-1]) / (2*dg)
    else:
        dF_cc = dF_co = float('nan')
    if i % 5 == 0:
        print(f"{g_sq:10.6f} {F_cc[i]:14.6f} {F_co[i]:14.6f} {dF_cc:14.6f} {dF_co:14.6f}")

# ===========================================================================
# Approach 4: The ratio Z(g)/Z(0) and its polynomial structure
# ===========================================================================
print()
print("=" * 80)
print("APPROACH 4: ln[Z(g)/Z(0)] as polynomial in g²")
print("=" * 80)
print()

# Z(g²)/Z(0) = <exp(g²/2 * Q)>_0 where <>_0 is uniform average
# ln[Z(g²)/Z(0)] = sum of connected cumulants
# = (g²/2) <Q>_0 + (g²/2)² (<Q²>_0 - <Q>²_0)/2 + ...
# = (g²/2) * m1_0 + (g⁴/8) * chi_0 + ...

# If this truncates at degree 2 in g², the self-consistency would be quadratic

print("Cumulant expansion of ln[Z(g)/Z(0)] for G_cc:")
print(f"  c_1 = <Q>/2 = {m1_0_cc/2:.6f}")
print(f"  c_2 = chi/8 = {chi_0_cc/8:.6f}")
print(f"  Predicted: ln[Z(g)/Z(0)] ~ {m1_0_cc/2:.4f}*g² + {chi_0_cc/8:.4f}*g⁴ + ...")
print()

print("Cumulant expansion for G_coulomb:")
print(f"  c_1 = <Q>/2 = {m1_0_co/2:.6f}")
print(f"  c_2 = chi/8 = {chi_0_co/8:.6f}")
print(f"  Predicted: ln[Z(g)/Z(0)] ~ {m1_0_co/2:.4f}*g² + {chi_0_co/8:.4f}*g⁴ + ...")
print()

# Verify against actual values
print("Verification (G_cc):")
print(f"{'g_sq':>10} {'ln[Z/Z0] actual':>18} {'c1*g² + c2*g⁴':>18} {'error':>12}")
print("-" * 60)
for i, g_sq in enumerate(g_sq_values):
    if g_sq == 0: continue
    actual = np.log(results_cc[i][1] / results_cc[0][1])
    predicted = (m1_0_cc/2)*g_sq + (chi_0_cc/8)*g_sq**2
    err = abs(actual - predicted)
    if i % 5 == 0:
        print(f"{g_sq:10.6f} {actual:18.10f} {predicted:18.10f} {err:12.2e}")

# ===========================================================================
# Approach 5: What coupling satisfies Q_eff = N * G_star^2 * (something)?
# ===========================================================================
print()
print("=" * 80)
print("APPROACH 5: Does the effective action encode G*?")
print("=" * 80)
print()

# The question: at what g² does <s^T G_cc s> = some function of G*?
# Or: at what g² does F(g²) have a special relationship to G*?

# From the gap equation: x = 1/g², and x² = 16*G*²*(x - G*)
# So g² = 1/x, and we need x to satisfy the master quadratic
# The physical value is g² = alpha = 1/x+ ≈ 0.0073

# Let's look at what F(alpha) and F'(alpha) look like
g_alpha = ALPHA
Z_alpha_cc, m1_alpha_cc, m2_alpha_cc = compute_Z_and_moments(g_alpha, G_cc)
Z_alpha_co, m1_alpha_co, m2_alpha_co = compute_Z_and_moments(g_alpha, M_scalar_pinv)

print(f"At g² = alpha = {g_alpha:.8f}:")
print(f"  G_cc:     Z = {Z_alpha_cc:.6f}, <Q> = {m1_alpha_cc:.6f}, <Q²> = {m2_alpha_cc:.6f}")
print(f"  G_coulomb: Z = {Z_alpha_co:.6f}, <Q> = {m1_alpha_co:.6f}, <Q²> = {m2_alpha_co:.6f}")
print()

# Check: does <Q> at g²=alpha relate to G* or the master quadratic?
print(f"  <Q_cc>/N = {m1_alpha_cc/N:.6f}")
print(f"  G*² = {G_STAR**2:.6f}")
print(f"  16*G*² = {16*G_STAR**2:.6f}")
print(f"  <Q_cc> = {m1_alpha_cc:.6f}")
print(f"  <Q_co> = {m1_alpha_co:.6f}")
print()

# ===========================================================================
# Approach 6: Direct check — does the cumulant structure give the quadratic?
# ===========================================================================
print("=" * 80)
print("APPROACH 6: Cumulant structure → master quadratic?")
print("=" * 80)
print()

# The free energy is F(g²) = F(0) - c1*g² - c2*g⁴ - ...
# The self-consistency condition dF/d(g²) = 0 gives:
# c1 + 2*c2*g² + ... = 0
# => g² = -c1/(2*c2) at leading order

# For G_cc:
g_sq_fixed_cc = -m1_0_cc / (2 * (chi_0_cc/4)) if chi_0_cc != 0 else float('inf')
# Wait, let me be more careful with the cumulant expansion
# F(g²) = -ln Z(g²) = -ln Z(0) - (g²/2)*<Q>_0 - (g²)²/8 * var(Q)_0 - ...
# dF/d(g²) = -<Q>_0/2 - g²/4 * var(Q)_0 - ...
# Setting = 0: g² = -2<Q>_0 / var(Q)_0

if chi_0_cc > 0:
    g_sq_sc_cc = -2 * m1_0_cc / chi_0_cc
    x_sc_cc = 1.0 / g_sq_sc_cc if g_sq_sc_cc > 0 else float('inf')
    print(f"G_cc self-consistent coupling (from cumulants):")
    print(f"  g²_sc = -2<Q>₀/var(Q)₀ = {g_sq_sc_cc:.8f}")
    print(f"  1/g²_sc = x = {x_sc_cc:.6f}")
    print(f"  Compare: x+ = {X_PLUS:.6f}")
    print()

if chi_0_co > 0:
    g_sq_sc_co = -2 * m1_0_co / chi_0_co
    x_sc_co = 1.0 / g_sq_sc_co if g_sq_sc_co > 0 else float('inf')
    print(f"G_coulomb self-consistent coupling (from cumulants):")
    print(f"  g²_sc = -2<Q>₀/var(Q)₀ = {g_sq_sc_co:.8f}")
    print(f"  1/g²_sc = x = {x_sc_co:.6f}")
    print(f"  Compare: x+ = {X_PLUS:.6f}")
    print()

# ===========================================================================
# Summary
# ===========================================================================
print("=" * 80)
print("SUMMARY")
print("=" * 80)
print()
print("This computation evaluates Z(g²) exactly for all 3^8 = 6561 ternary")
print("configurations on the 2x2x2 periodic torus, with coupling g as a")
print("free parameter. Multiple self-consistency conditions are tested.")
print()
print("The key numbers to check:")
print(f"  Target x+ = {X_PLUS:.6f} (should match some self-consistency condition)")
print(f"  Target x- = {X_MINUS:.6f}")
print(f"  G* = {G_STAR:.6f}")
print(f"  16*G*² = {16*G_STAR**2:.6f}")
print()
print("Status: [EXPLORATORY] — documenting what the lattice produces.")
