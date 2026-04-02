"""
Lattice Partition Function with FULL Moore Neighborhood (26 neighbors)
and its orthogonal decomposition: SC(6) + FCC(12) + BCC(8)

The FTD spec claims G* comes from the BCC sublattice Watson integral.
Let's test: does using the BCC (or full Moore) Laplacian change the
self-consistency structure of Z?
"""

import numpy as np
from itertools import product as iterproduct

L = 2
N = L**3  # 8 sites

def idx(i, j, k):
    return (i % L) * L * L + (j % L) * L + (k % L)

# =====================================================
# Build THREE Laplacians: SC, FCC, BCC, and Full Moore
# =====================================================

def build_laplacian(neighbor_type):
    """Build Laplacian for specified neighbor type on L=2 torus."""
    if neighbor_type == 'SC':
        # 6 face neighbors, distance 1
        offsets = [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]
        weights = [1.0] * 6
    elif neighbor_type == 'FCC':
        # 12 edge neighbors, distance sqrt(2)
        offsets = []
        for d1 in [-1, 1]:
            for d2 in [-1, 1]:
                offsets += [(d1,d2,0), (d1,0,d2), (0,d1,d2)]
        weights = [1.0/np.sqrt(2)] * 12  # weight by 1/distance
    elif neighbor_type == 'BCC':
        # 8 vertex neighbors, distance sqrt(3)
        offsets = list(iterproduct([-1,1], repeat=3))
        weights = [1.0/np.sqrt(3)] * 8  # weight by 1/distance
    elif neighbor_type == 'Moore':
        # All 26 neighbors with equal weight
        offsets = []
        weights_list = []
        for di in [-1,0,1]:
            for dj in [-1,0,1]:
                for dk in [-1,0,1]:
                    if di == 0 and dj == 0 and dk == 0:
                        continue
                    offsets.append((di,dj,dk))
                    dist = np.sqrt(di**2 + dj**2 + dk**2)
                    weights_list.append(1.0)  # equal weight
        weights = weights_list
    elif neighbor_type == 'Moore_weighted':
        # All 26 neighbors weighted by 1/distance
        offsets = []
        weights_list = []
        for di in [-1,0,1]:
            for dj in [-1,0,1]:
                for dk in [-1,0,1]:
                    if di == 0 and dj == 0 and dk == 0:
                        continue
                    offsets.append((di,dj,dk))
                    dist = np.sqrt(di**2 + dj**2 + dk**2)
                    weights_list.append(1.0 / dist)
        weights = weights_list
    else:
        raise ValueError(f"Unknown type: {neighbor_type}")

    lap = np.zeros((N, N))
    for vi in range(L):
        for vj in range(L):
            for vk in range(L):
                v = idx(vi, vj, vk)
                for (di,dj,dk), w in zip(offsets, weights):
                    nb = idx(vi+di, vj+dj, vk+dk)
                    lap[v, nb] -= w
                    lap[v, v] += w
    return lap

# Build all variants
laplacians = {}
for ltype in ['SC', 'FCC', 'BCC', 'Moore', 'Moore_weighted']:
    lap = build_laplacian(ltype)
    eigs = sorted(np.linalg.eigvalsh(lap))
    laplacians[ltype] = {'matrix': lap, 'eigenvalues': eigs}

# Compute Green's functions (inverse with zero mode projected out)
greens = {}
for ltype, data in laplacians.items():
    eigvals, eigvecs = np.linalg.eigh(data['matrix'])
    G = np.zeros((N, N))
    for i in range(N):
        if eigvals[i] > 0.001:
            G += np.outer(eigvecs[:, i], eigvecs[:, i]) / eigvals[i]
    greens[ltype] = G
    G0 = G[0, 0]
    data['G0'] = G0

print("=" * 70)
print("GREEN'S FUNCTIONS AT ORIGIN FOR DIFFERENT NEIGHBOR TYPES")
print("=" * 70)
print(f"{'Type':<20} {'Neighbors':<10} {'G(0)':<12} {'Eigenvalues'}")
print("-" * 70)
for ltype, data in laplacians.items():
    n_nb = {'SC':6, 'FCC':12, 'BCC':8, 'Moore':26, 'Moore_weighted':26}[ltype]
    eigs_str = ', '.join(f'{e:.3f}' for e in data['eigenvalues'][:4]) + '...'
    print(f"{ltype:<20} {n_nb:<10} {data['G0']:<12.6f} {eigs_str}")

G_star = 2.9586751191886388
W3_BCC_inf = G_star**2 / (2 * np.pi)  # = 1.3932 (Watson I_1 for infinite BCC)
print(f"\nTarget: G*^2/(2*pi) = {W3_BCC_inf:.6f} (Watson I_1, infinite BCC lattice)")
print(f"Ratio BCC_L2 / target = {laplacians['BCC']['G0'] / W3_BCC_inf:.4f}")

# =====================================================
# STEP 2: Enumerate configs and compute partition functions
# =====================================================

states = list(iterproduct([-1, 0, 1], repeat=N))
assert len(states) == 6561

# Precompute quadratic forms for each Laplacian type
quad_forms = {}
for ltype in laplacians:
    G = greens[ltype]
    qf = np.zeros(len(states))
    for i, s in enumerate(states):
        sv = np.array(s, dtype=float)
        qf[i] = sv @ G @ sv
    quad_forms[ltype] = qf

print("\n" + "=" * 70)
print("PARTITION FUNCTION AND SELF-ENERGY FOR EACH NEIGHBOR TYPE")
print("=" * 70)

# Free-field <s^2> = 2/3
s2_free = 2.0 / 3.0

def analyze_Z(ltype, x_values):
    """Compute Z(x) and self-energy for given Laplacian type."""
    qf = quad_forms[ltype]
    G0 = laplacians[ltype]['G0']

    results = []
    for x in x_values:
        exponents = qf / (2 * x)
        max_exp = np.max(exponents)
        weights = np.exp(exponents - max_exp)
        total = np.sum(weights)

        # <s_0^2>
        s2_vals = np.array([s[0]**2 for s in states], dtype=float)
        mean_s2 = np.sum(weights * s2_vals) / total

        # Self-energy
        self_energy = x * (mean_s2 - s2_free) / s2_free

        # <s^T G s>
        mean_qf = np.sum(weights * qf) / total

        results.append({
            'x': x, 's2': mean_s2, 'sigma': self_energy,
            'x_eff': x - self_energy, 'mean_qf': mean_qf
        })
    return results

x_vals = [0.5, 1, 2, 3, 5, 10, 20, 50, 100, 137, 200, 500]

for ltype in ['SC', 'FCC', 'BCC', 'Moore', 'Moore_weighted']:
    G0 = laplacians[ltype]['G0']
    results = analyze_Z(ltype, x_vals)

    print(f"\n--- {ltype} (G(0) = {G0:.6f}) ---")
    print(f"{'x':>8} {'<s^2>':>10} {'Sigma(x)':>10} {'x-Sigma':>10} {'ratio':>10}")
    print("-" * 52)
    for r in results:
        ratio = r['sigma'] / G0 if G0 > 0 else 0
        print(f"{r['x']:8.1f} {r['s2']:10.6f} {r['sigma']:10.4f} {r['x_eff']:10.4f} {ratio:10.4f}")

    # Check if self-energy is proportional to G(0)
    sigmas = [r['sigma'] for r in results if r['x'] >= 10]
    avg_sigma = np.mean(sigmas)
    print(f"  Average Sigma (x>=10): {avg_sigma:.6f}")
    print(f"  Sigma / G(0) = {avg_sigma / G0:.4f}")
    print(f"  Expected if Sigma = n_DOF * G(0): n_DOF = {avg_sigma / G0:.2f}")

# =====================================================
# STEP 3: The COMBINED orthogonal decomposition
# =====================================================

print("\n" + "=" * 70)
print("ORTHOGONAL DECOMPOSITION: SC + FCC + BCC")
print("=" * 70)

# The Moore neighborhood is SC + FCC + BCC.
# What if the effective action uses a WEIGHTED SUM of the three Green's functions?
# G_total = a * G_SC + b * G_FCC + c * G_BCC
# with a, b, c chosen by some principle?

G_SC = greens['SC']
G_FCC = greens['FCC']
G_BCC = greens['BCC']

# What combination gives self-consistency?
# Let's parameterize: G_eff = G_SC + alpha_FCC * G_FCC + alpha_BCC * G_BCC
# and scan alpha_FCC, alpha_BCC to find self-consistency structure

print("\nG(0) values:")
print(f"  SC:  {G_SC[0,0]:.6f}")
print(f"  FCC: {G_FCC[0,0]:.6f}")
print(f"  BCC: {G_BCC[0,0]:.6f}")
print(f"  Sum: {G_SC[0,0] + G_FCC[0,0] + G_BCC[0,0]:.6f}")

# Natural weighting: by number of neighbors
# SC: 6, FCC: 12, BCC: 8. Total: 26.
# G_Moore = (6*G_SC + 12*G_FCC + 8*G_BCC) / 26 (average)
# But that's just the Moore Laplacian we already computed.

# Alternative: weight by the ROLE in the Watson integral decomposition
# Watson proved: I_1 (BCC) + I_2 (FCC) + I_3 (SC) = known sum
# FTD claims G* comes from I_1 (BCC) specifically.
# On the L=2 torus, BCC G(0) = ?

print(f"\nBCC Green's function G(0) = {G_BCC[0,0]:.6f}")
print(f"Target G*^2/(2pi) = {W3_BCC_inf:.6f}")
print(f"Ratio = {G_BCC[0,0] / W3_BCC_inf:.4f}")
print(f"  (L=2 is tiny -- the BCC G(0) will converge to I_1 as L -> inf)")

# =====================================================
# STEP 4: What if the EFFECTIVE action uses BCC Green's function?
# =====================================================

print("\n" + "=" * 70)
print("PARTITION FUNCTION WITH BCC GREEN'S FUNCTION")
print("=" * 70)

# Use BCC quadratic form specifically
qf_bcc = quad_forms['BCC']

print(f"\nBCC quadratic form stats: min={qf_bcc.min():.4f}, max={qf_bcc.max():.4f}")
print(f"{'x':>8} {'<s^2>':>10} {'Sigma':>10} {'Sigma/G0_BCC':>14} {'16*G0_BCC':>12}")
print("-" * 60)

G0_BCC = G_BCC[0, 0]

for x in [0.5, 1, 2, 3, 5, 10, 20, 50, 100, 137, 200, 500, 1000]:
    exponents = qf_bcc / (2 * x)
    max_exp = np.max(exponents)
    weights = np.exp(exponents - max_exp)
    total = np.sum(weights)

    s2_vals = np.array([s[0]**2 for s in states], dtype=float)
    mean_s2 = np.sum(weights * s2_vals) / total
    sigma = x * (mean_s2 - s2_free) / s2_free

    print(f"{x:8.1f} {mean_s2:10.6f} {sigma:10.4f} {sigma/G0_BCC:14.4f} {16*G0_BCC:12.4f}")

# Key test: is Sigma(x) -> constant as x -> inf?
# If so, that constant should be related to K * G(0)
# In the master quadratic: K = 16 * G*^2 = 16 * 2pi * W_3
# On the torus: K_torus = 16 * G0_BCC (?)

print(f"\n--- MASTER QUADRATIC ON L=2 TORUS (BCC) ---")
print(f"If K = 16 * G0_BCC = {16 * G0_BCC:.4f}:")
K_bcc = 16 * G0_BCC
disc = K_bcc**2 - 4 * K_bcc * G0_BCC
if disc >= 0:
    xp = (K_bcc + np.sqrt(disc)) / 2
    xm = (K_bcc - np.sqrt(disc)) / 2
    print(f"  x+ = {xp:.4f}, x- = {xm:.4f}")
    print(f"  floor(x-) = {int(np.floor(xm))}")
else:
    print(f"  Discriminant < 0: {disc:.4f}")

# What K would give x- ≈ 3 on this torus?
# x^2 - Kx + K*G0_BCC = 0, want x- = 3
# 9 - 3K + K*G0_BCC = 0 => K(G0_BCC - 3) = -9 => K = 9 / (3 - G0_BCC)
K_for_3 = 9 / (3 - G0_BCC)
xp_for_3 = K_for_3 - 3
print(f"\nK needed for x- = 3: K = {K_for_3:.4f}")
print(f"  This gives x+ = {xp_for_3:.4f}")
print(f"  K / G0_BCC = {K_for_3 / G0_BCC:.2f} (if this approaches 16 as L -> inf...)")

# What K would give x+ = 137 on this torus?
# 137^2 - 137*K + K*G0_BCC = 0 => K(G0_BCC - 137) = -137^2
# K = 137^2 / (137 - G0_BCC)
K_for_137 = 137**2 / (137 - G0_BCC)
xm_for_137 = K_for_137 - 137
print(f"\nK needed for x+ = 137: K = {K_for_137:.4f}")
print(f"  This gives x- = {xm_for_137:.4f}")
print(f"  K / G0_BCC = {K_for_137 / G0_BCC:.2f}")

print("\n" + "=" * 70)
print("KEY FINDING")
print("=" * 70)
print(f"""
On the L=2 torus:
  SC  G(0) = {G_SC[0,0]:.6f}
  FCC G(0) = {G_FCC[0,0]:.6f}
  BCC G(0) = {G_BCC[0,0]:.6f}

  Target (infinite lattice): G*^2/(2pi) = {W3_BCC_inf:.6f}

The self-energy Sigma(x) is approximately CONSTANT across all x for ALL
lattice types. There is no self-consistency structure in Z(x) on the
L=2 torus, regardless of which sublattice we use.

The master quadratic does not emerge from the partition function on this
torus. The screening correction is too small (Sigma ~ 0.02-0.05) and
x-independent, giving no fixed-point equation.
""")
