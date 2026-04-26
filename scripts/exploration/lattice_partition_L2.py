"""
Lattice Partition Function on the L=2 Torus — From Scratch

The central open problem in FTD: derive the master quadratic from Z.

Setup:
  - Lattice: (Z/2Z)^3 = 8 sites, periodic boundaries
  - Each site: state s(v) ∈ {-1, 0, +1} (ternary)
  - Flux field J on links: integrated out exactly (Gaussian)
  - 3^8 = 6561 state configurations, each weighted by the effective action

The effective action after integrating out J is:
  S_eff[s, x] = -(1/(2x)) s^T G s

where x = 1/g^2 (inverse coupling squared) and G is the lattice Green's function.

We compute Z(x) exactly and look for self-consistency structure.
"""

import numpy as np
from itertools import product
from scipy import linalg
import json

# =====================================================
# STEP 1: Build the L=2 torus lattice
# =====================================================

L = 2
N_sites = L**3  # 8

# Site indices: (i,j,k) -> flat index
def site_idx(i, j, k):
    return (i % L) * L * L + (j % L) * L + (k % L)

# Build the scalar Laplacian (-Δ) on the 8-site torus
# (-Δf)(v) = 6f(v) - Sigma_{neighbors} f(v')
laplacian = np.zeros((N_sites, N_sites))
for i in range(L):
    for j in range(L):
        for k in range(L):
            v = site_idx(i, j, k)
            laplacian[v, v] = 6.0  # diagonal: 2D = 6 for D=3
            # 6 neighbors (periodic):
            for di, dj, dk in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
                nb = site_idx(i+di, j+dj, k+dk)
                laplacian[v, nb] -= 1.0

print("Laplacian eigenvalues:", sorted(np.linalg.eigvalsh(laplacian)))
# Expected: 0, 4, 4, 4, 8, 8, 8, 12

# Green's function: G = (-Δ)^{-1} with zero mode projected out
eigvals, eigvecs = np.linalg.eigh(laplacian)
# Project out zero mode
G = np.zeros((N_sites, N_sites))
for i in range(N_sites):
    if eigvals[i] > 0.01:  # skip zero eigenvalue
        G += np.outer(eigvecs[:, i], eigvecs[:, i]) / eigvals[i]

G_at_origin = G[0, 0]
print(f"\nGreen's function at origin G(0) = {G_at_origin:.6f}")
print(f"  (Compare: cubic lattice G(0) = 0.2527... in large-L regime)")

# Watson integral I_3 (SC) for cubic lattice (large-L regime)
W3_SC_inf = 0.505462  # 2 * G(0)_largeL = 2 * 0.2527
print(f"  Watson I_3 (SC, large-L) = {W3_SC_inf:.6f}")
import sys as _sys, os as _os
_sys.path.insert(0, _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__))))
from constants import G_STAR as _GSTAR
print(f"  G* = {_GSTAR:.7f}, G*^2/(2*pi) = {_GSTAR**2/(2*np.pi):.6f}")

# =====================================================
# STEP 2: Enumerate all 6561 ternary configurations
# =====================================================

print("\n--- Enumerating 6561 configurations ---")

states_list = list(product([-1, 0, 1], repeat=N_sites))
assert len(states_list) == 3**N_sites == 6561

# For each configuration, compute s^T G s (the quadratic form)
quad_forms = np.zeros(len(states_list))
for idx, s in enumerate(states_list):
    s_vec = np.array(s, dtype=float)
    quad_forms[idx] = s_vec @ G @ s_vec

print(f"Quadratic form s^T G s: min={quad_forms.min():.4f}, max={quad_forms.max():.4f}")
print(f"  Non-zero configurations: {np.count_nonzero(quad_forms)}")

# Distribution of q = s^T G s values
unique_q = np.unique(np.round(quad_forms, 6))
print(f"  Distinct values of s^T G s: {len(unique_q)}")
for q in sorted(unique_q)[:15]:
    count = np.sum(np.abs(quad_forms - q) < 0.0001)
    print(f"    q = {q:8.4f} : {count:5d} configs")
if len(unique_q) > 15:
    print(f"    ... ({len(unique_q) - 15} more values)")

# =====================================================
# STEP 3: Compute Z(x) as a function of bare coupling x
# =====================================================

print("\n--- Computing Z(x) ---")

def Z(x):
    """Partition function at coupling x = 1/g^2"""
    if x <= 0:
        return float('nan')
    weights = np.exp(quad_forms / (2 * x))
    return np.sum(weights)

def log_Z(x):
    """Log partition function (more stable)"""
    if x <= 0:
        return float('nan')
    exponents = quad_forms / (2 * x)
    max_exp = np.max(exponents)
    return max_exp + np.log(np.sum(np.exp(exponents - max_exp)))

def d_log_Z(x, dx=0.001):
    """Numerical derivative of log Z"""
    return (log_Z(x + dx) - log_Z(x - dx)) / (2 * dx)

def d2_log_Z(x, dx=0.001):
    """Second derivative of log Z"""
    return (log_Z(x + dx) - 2 * log_Z(x) + log_Z(x - dx)) / (dx**2)

# The effective coupling (susceptibility)
def chi(x):
    """<s^T G s> / N = expectation of quadratic form per site"""
    exponents = quad_forms / (2 * x)
    max_exp = np.max(exponents)
    weights = np.exp(exponents - max_exp)
    total = np.sum(weights)
    return np.sum(weights * quad_forms) / (total * N_sites)

# The "self-energy" Sigma(x)
def sigma(x):
    """Self-energy: x * <s^2> * G(0) or derived from susceptibility"""
    exponents = quad_forms / (2 * x)
    max_exp = np.max(exponents)
    weights = np.exp(exponents - max_exp)
    total = np.sum(weights)

    # <s_0^2> = expectation of s(0)^2
    s2_vals = np.array([s[0]**2 for s in states_list], dtype=float)
    mean_s2 = np.sum(weights * s2_vals) / total

    # <s_0>^2
    s1_vals = np.array([s[0] for s in states_list], dtype=float)
    mean_s1 = np.sum(weights * s1_vals) / total

    return mean_s2, mean_s1

print("\nZ(x) and observables at various couplings:")
print(f"{'x':>10} {'log Z':>12} {'chi/G(0)':>12} {'<s^2>':>10} {'<s>':>10}")
print("-" * 60)

x_values = [0.5, 1, 2, 5, 10, 20, 50, 100, 137, 200, 500, 1000]
results = []
for x in x_values:
    lz = log_Z(x)
    c = chi(x)
    s2, s1 = sigma(x)
    results.append({'x': x, 'logZ': lz, 'chi': c, 'chi_over_G0': c / G_at_origin if G_at_origin > 0 else 0,
                    's2': s2, 's1': s1})
    print(f"{x:10.1f} {lz:12.4f} {c/G_at_origin:12.6f} {s2:10.6f} {s1:10.6f}")

# =====================================================
# STEP 4: Look for self-consistency
# =====================================================

print("\n--- Self-consistency analysis ---")

# The free-field limit: x -> inf
# <s^2> -> 2/3 (ternary: average of 1, 0, 1)
# <s> -> 0 (symmetric)
s2_free, s1_free = sigma(10000)
print(f"Free-field limit (x=10000): <s^2> = {s2_free:.6f} (expected 2/3 = {2/3:.6f})")

# Approach 1: Define effective coupling x_eff from the susceptibility
# In mean-field: x_eff = x / (1 + Sigma) where Sigma is the self-energy correction
# The self-energy is related to <s^2> deviation from free-field value

print("\nSelf-energy Sigma(x) = x * (<s^2>(x) - <s^2>(inf)) / <s^2>(inf):")
print(f"{'x':>10} {'<s^2>':>12} {'Sigma(x)':>12} {'x_eff=x-Sigma':>12}")
print("-" * 50)
for x in x_values:
    s2, _ = sigma(x)
    self_energy = x * (s2 - s2_free) / s2_free
    x_eff = x - self_energy
    print(f"{x:10.1f} {s2:12.6f} {self_energy:12.4f} {x_eff:12.4f}")

# Approach 2: The ratio <s^T G s> / <s^T s> as an effective G_eff
print("\n--- Approach 2: Effective Green's function ---")
for x in [1, 5, 10, 50, 100, 137, 500]:
    exponents = quad_forms / (2 * x)
    max_exp = np.max(exponents)
    weights = np.exp(exponents - max_exp)
    total = np.sum(weights)

    sGs = np.sum(weights * quad_forms) / total  # <s^T G s>

    # <s^T s> = <Sigma s(v)²>
    s_sq_total = np.array([sum(si**2 for si in s) for s in states_list], dtype=float)
    sTs = np.sum(weights * s_sq_total) / total

    G_eff = sGs / sTs if sTs > 0 else 0
    print(f"  x={x:6.1f}: <s^T G s>={sGs:.4f}, <s^T s>={sTs:.4f}, G_eff={G_eff:.6f}, G(0)={G_at_origin:.6f}")

# Approach 3: Look at the equation x * d(log Z)/dx = <s^T G s>/(2x²)
# This is the "gap equation" — when does it have a self-consistent solution?
print("\n--- Approach 3: Gap equation d(log Z)/dx ---")
print(f"{'x':>10} {'d(logZ)/dx':>14} {'x²*d(logZ)':>14} {'G(0)*<s^2>':>14}")
print("-" * 56)
for x in [1, 2, 5, 10, 20, 50, 100, 137, 200, 500]:
    dlz = d_log_Z(x)
    s2, _ = sigma(x)
    print(f"{x:10.1f} {dlz:14.6f} {x**2 * dlz:14.4f} {G_at_origin * s2:14.6f}")

# =====================================================
# STEP 5: The crucial test — does the master quadratic emerge?
# =====================================================

print("\n" + "=" * 60)
print("CRUCIAL TEST: Does x² - 16G*²x + 16G*³ = 0 emerge from Z?")
print("=" * 60)

G_star = 2.9586751191886388
K_expected = 16 * G_star**2  # ≈ 140.06
print(f"\nExpected from master quadratic:")
print(f"  K = 16G*² = {K_expected:.4f}")
print(f"  x+ = {(K_expected + np.sqrt(K_expected**2 - 4*K_expected*G_star))/2:.4f}")
print(f"  x- = {(K_expected - np.sqrt(K_expected**2 - 4*K_expected*G_star))/2:.4f}")

# On the L=2 torus, G(0) = specific finite-size value.
# The question is: can we extract a self-consistent coupling from Z?

# Define F(x) = the "effective coupling produced by the theory"
# In the spec, F(x) = K(1 - G*/x). Self-consistency: x = F(x).
# But we want to DERIVE F(x), not assume it.

# From the partition function:
# Z(x) = Sigma_s exp(s^T G s / (2x))
# The "free energy" F(x) = -log Z(x)
# The susceptibility chi(x) = (1/N) ∂²F/∂h² |_{h=0} (response to external field)
# The effective coupling can be defined as x_eff where the response matches the free theory

print("\n--- Scanning for self-consistency in x_eff(x) = x ---")
print("  Define: x_eff = x * <s^2>(inf) / <s^2>(x)")
print("  Self-consistency: x_eff(x) = x, i.e., <s^2>(x) = <s^2>(inf)")
print("  This is trivially satisfied at x -> inf. Looking for non-trivial solutions.\n")

# Scan x_eff(x) for a range of x values
print(f"{'x':>10} {'<s^2>':>12} {'x_eff':>12} {'x - x_eff':>12}")
print("-" * 50)
for x in np.concatenate([np.arange(0.5, 5, 0.5), np.arange(5, 20, 2), np.arange(20, 200, 10), [137, 140]]):
    s2, _ = sigma(x)
    x_eff = x * s2_free / s2 if s2 > 0 else 0
    print(f"{x:10.2f} {s2:12.6f} {x_eff:12.4f} {x - x_eff:12.4f}")

# Approach 4: The COUPLING RENORMALIZATION
# g_eff² = g² + dg^2 where dg^2 comes from vacuum polarization
# In terms of x = 1/g²: 1/x_eff = 1/x + Pi_pol(x) where Pi_pol is the polarization
# On the finite lattice, Pi_pol = G(0) * <s^2> / x
# Self-consistency: x_eff = x means Pi_pol = 0, i.e., no correction. Boring.
#
# The INTERESTING equation is: x = K * (1 - something/x)
# where K involves the total number of modes and the Green's function.
# Let's try to find what K should be from the DATA.

print("\n--- Approach 4: What K would make the quadratic work on this torus? ---")
# If x² - Kx + K*G(0)*N_phys = 0 on this torus, what K fits?
# The finite-torus version: G(0) plays the role of G* on this lattice.
# K = n_DOF * something. On L=2: n_DOF depends on gauge fixing.

# Total vector DOF: 3 * 8 = 24
# Gauss constraints: rank of divergence operator
D = np.zeros((N_sites, 3 * N_sites))  # divergence: 8 × 24
for v_i in range(L):
    for v_j in range(L):
        for v_k in range(L):
            v = site_idx(v_i, v_j, v_k)
            # J_x at (v) - J_x at (v - e_x)
            link_x_plus = v * 3 + 0
            link_x_minus = site_idx(v_i - 1, v_j, v_k) * 3 + 0
            D[v, link_x_plus] += 1
            D[v, link_x_minus] -= 1
            # J_y
            link_y_plus = v * 3 + 1
            link_y_minus = site_idx(v_i, v_j - 1, v_k) * 3 + 1
            D[v, link_y_plus] += 1
            D[v, link_y_minus] -= 1
            # J_z
            link_z_plus = v * 3 + 2
            link_z_minus = site_idx(v_i, v_j, v_k - 1) * 3 + 2
            D[v, link_z_plus] += 1
            D[v, link_z_minus] -= 1

rank_D = np.linalg.matrix_rank(D)
print(f"\nDivergence operator D: {D.shape}, rank = {rank_D}")
print(f"Total link DOF: 24")
print(f"Gauss constraints: {rank_D}")
print(f"Physical DOF (temporal gauge, -1): {24 - rank_D - 1}")
print(f"Physical DOF (Coulomb gauge, -3): {24 - rank_D - 3}")
print(f"G(0) on L=2 torus: {G_at_origin:.6f}")
print(f"G(0) * N_phys(16) = {G_at_origin * 16:.6f}")
print(f"G(0) * N_phys(14) = {G_at_origin * 14:.6f}")

# If K = 16 * (2π * W₃) on the cubic lattice (large-L regime):
# On L=2 torus, what's the equivalent?
K_L2_16 = 16 * G_at_origin  # using torus G(0)
K_L2_14 = 14 * G_at_origin

print(f"\nFinite-torus quadratic with K=16*G(0):")
disc16 = K_L2_16**2 - 4 * K_L2_16 * G_at_origin
if disc16 >= 0:
    x_plus_L2 = (K_L2_16 + np.sqrt(disc16)) / 2
    x_minus_L2 = (K_L2_16 - np.sqrt(disc16)) / 2
    print(f"  x+ = {x_plus_L2:.4f}, x- = {x_minus_L2:.4f}")
else:
    print(f"  Discriminant negative: {disc16:.4f}")

print(f"\nFinite-torus quadratic with K=14*G(0):")
disc14 = K_L2_14**2 - 4 * K_L2_14 * G_at_origin
if disc14 >= 0:
    x_plus_L2_14 = (K_L2_14 + np.sqrt(disc14)) / 2
    x_minus_L2_14 = (K_L2_14 - np.sqrt(disc14)) / 2
    print(f"  x+ = {x_plus_L2_14:.4f}, x- = {x_minus_L2_14:.4f}")
else:
    print(f"  Discriminant negative: {disc14:.4f}")

print("\n" + "=" * 60)
print("SUMMARY")
print("=" * 60)
print("""
The L=2 torus computation gives us:
1. The exact partition function Z(x) as a sum over 6561 configs
2. The Green's function G(0) on this finite lattice
3. The expectation values <s^2>(x) and <s^T G s>(x)

The question: does Z(x) contain the master quadratic structure?
""")
