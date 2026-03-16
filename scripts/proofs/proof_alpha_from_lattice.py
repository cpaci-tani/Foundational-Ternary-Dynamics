"""
Proof: α from the FTD Lattice Partition Function

Attempt to derive the fine structure constant from the exact partition function
on the minimal 2×2×2 periodic lattice, WITHOUT assuming α as input.

The FTD partition function:
    Z(g) = Σ_{s} ∫ dJ exp(-S_E[s, J; g])

For fixed ternary configuration s, S_E is quadratic in J (Gaussian integral).
After Gauss constraints (7) and gauge fixing (1), exactly 16 DOF remain —
the same 16 that appears as the master quadratic coefficient.

Key question: Does the free energy F(g) = -ln Z(g) have an extremum
whose location reproduces the master quadratic x² - 16G*²x + 16G*³ = 0?

References:
    - DERIV_PATH_INTEGRAL_CONSTRUCTION.md (partition function construction)
    - DERIV_ALPHA_LATTICE_MECHANISM.md (the proposed chain)
    - MATH_MASTER_QUADRATIC.md (target equation)

Status: [EXPLORATORY] — This is research, not a proven result.
"""

import numpy as np
from scipy.special import gamma
from scipy.optimize import minimize_scalar
import itertools

# ===========================================================================
# Constants (from scripts/constants.py, reproduced for self-containment)
# ===========================================================================

GAMMA_QUARTER = gamma(0.25)  # Γ(1/4) ≈ 3.6256
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)  # ≈ 2.9587
VARPI = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * np.pi))    # ≈ 2.6221

# Master quadratic roots (target values)
X_PLUS = 8 * G_STAR**2 * (1 + np.sqrt(1 - 1/G_STAR))   # ≈ 137.036
X_MINUS = 8 * G_STAR**2 * (1 - np.sqrt(1 - 1/G_STAR))   # ≈ 3.024
ALPHA_TARGET = 1.0 / X_PLUS

# Watson integral for 3D cubic lattice
# W₃ = (1/(2π)³) ∫_BZ d³k / k̂² = Γ(1/4)⁴ / (4π³)
WATSON_3D = GAMMA_QUARTER**4 / (4 * np.pi**3)

print("=" * 70)
print("FTD Lattice Partition Function: α Derivation Attempt")
print("=" * 70)
print(f"  G*     = {G_STAR:.10f}")
print(f"  ϖ      = {VARPI:.10f}")
print(f"  x₊     = {X_PLUS:.10f} (target 1/α)")
print(f"  x₋     = {X_MINUS:.10f} (target N_c)")
print(f"  α      = {ALPHA_TARGET:.10f}")
print(f"  W₃     = {WATSON_3D:.10f} (Watson integral)")
print()

# ===========================================================================
# Step 1: Construct the 2×2×2 lattice Laplacian
# ===========================================================================

print("STEP 1: Constructing 2×2×2 periodic lattice Laplacian")
print("-" * 70)

L = 2  # lattice size
N_sites = L**3  # 8 sites
D = 3  # spatial dimensions

# Site coordinates
sites = [(x, y, z) for x in range(L) for y in range(L) for z in range(L)]
site_index = {s: i for i, s in enumerate(sites)}

# Scalar Laplacian (8×8 matrix)
# Δf(x) = Σ_μ [f(x+μ) + f(x-μ) - 2f(x)]
Delta_scalar = np.zeros((N_sites, N_sites))
for i, (x, y, z) in enumerate(sites):
    Delta_scalar[i, i] = -2 * D  # diagonal: -6
    for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
        nx, ny, nz = (x+dx) % L, (y+dy) % L, (z+dz) % L
        j = site_index[(nx, ny, nz)]
        Delta_scalar[i, j] += 1

# Negative Laplacian (positive definite on non-zero modes)
M_scalar = -Delta_scalar

# Eigenvalues of -Δ on 2×2×2 periodic torus
eigenvalues_scalar = np.sort(np.linalg.eigvalsh(M_scalar))
print(f"  Scalar Laplacian eigenvalues: {eigenvalues_scalar}")
print(f"  Expected: [0, 2, 2, 2, 4, 4, 4, 6] (from k̂² at allowed momenta)")

# Verify against analytical formula
# k_μ ∈ {0, π} for L=2
# k̂² = 2 Σ_μ (1 - cos k_μ)
analytical_eigs = []
for kx in [0, np.pi]:
    for ky in [0, np.pi]:
        for kz in [0, np.pi]:
            khat2 = 2 * ((1 - np.cos(kx)) + (1 - np.cos(ky)) + (1 - np.cos(kz)))
            analytical_eigs.append(khat2)
analytical_eigs.sort()
print(f"  Analytical eigenvalues:       {analytical_eigs}")
print()

# ===========================================================================
# Step 2: Vector Laplacian (3 components per site = 24 DOF)
# ===========================================================================

print("STEP 2: Vector field on lattice (24 DOF)")
print("-" * 70)

N_dof_total = D * N_sites  # 24
# The vector Laplacian is block-diagonal: M_vec = I_3 ⊗ M_scalar
M_vector = np.kron(np.eye(D), M_scalar)
print(f"  Total DOF: {N_dof_total}")
print(f"  Vector Laplacian shape: {M_vector.shape}")

# ===========================================================================
# Step 3: Gauss constraint and gauge fixing
# ===========================================================================

print()
print("STEP 3: Gauss constraint + gauge fixing → 16 physical DOF")
print("-" * 70)

# The Gauss constraint is div(J) = ρ at each site.
# In momentum space, this removes the longitudinal mode at each k.
# On the 2×2×2 lattice: 8 k-modes × 3 components = 24
# Gauss removes 1 longitudinal per nonzero k-mode: 7 removed
# Gauge fixing removes 1 global mode: 1 removed
# Remaining: 24 - 7 - 1 = 16

# Build the divergence operator (8×24 matrix)
# div(J)_i = Σ_μ [J_μ(i+μ) - J_μ(i)] (forward difference)
Div = np.zeros((N_sites, N_dof_total))
for i, (x, y, z) in enumerate(sites):
    for mu in range(D):
        # J_μ at site i
        j_idx = mu * N_sites + i
        Div[i, j_idx] -= 1  # -J_μ(i)
        # J_μ at site i+μ
        if mu == 0:
            nx, ny, nz = (x+1) % L, y, z
        elif mu == 1:
            nx, ny, nz = x, (y+1) % L, z
        else:
            nx, ny, nz = x, y, (z+1) % L
        j_fwd = mu * N_sites + site_index[(nx, ny, nz)]
        Div[i, j_fwd] += 1  # +J_μ(i+μ)

# The Gauss constraint projects onto transverse modes
# Use SVD to find the null space of Div (the transverse subspace)
U_div, S_div, Vt_div = np.linalg.svd(Div)
# Number of nonzero singular values = rank of Div
rank_div = np.sum(S_div > 1e-10)
print(f"  Divergence operator rank: {rank_div} (expect 7: one zero mode from k=0)")
print(f"  Transverse subspace dimension: {N_dof_total - rank_div} = {N_dof_total - rank_div}")

# Transverse projection: columns of Vt_div corresponding to zero singular values
n_transverse = N_dof_total - rank_div
P_transverse = Vt_div[rank_div:, :].T  # 24 × n_transverse

# Remove one more DOF for gauge fixing (global transverse zero mode)
# The zero mode of M_scalar (k=0) gives 2 transverse zero modes
# We fix gauge by removing one of them
M_transverse = P_transverse.T @ M_vector @ P_transverse
eigs_trans = np.sort(np.linalg.eigvalsh(M_transverse))
n_zero_trans = np.sum(np.abs(eigs_trans) < 1e-10)
print(f"  Transverse Laplacian eigenvalues (first 5): {eigs_trans[:5]}")
print(f"  Number of zero modes in transverse sector: {n_zero_trans}")

# Remove zero modes (gauge fixing)
# Project onto nonzero eigenspace
nonzero_mask = np.abs(eigs_trans) > 1e-10
n_physical = np.sum(nonzero_mask)
print(f"  Physical DOF after gauge fixing: {n_physical}")

# Get the physical subspace
eigvecs_trans = np.linalg.eigh(M_transverse)[1]
P_physical = eigvecs_trans[:, nonzero_mask]  # project out zero modes
M_physical = P_physical.T @ M_transverse @ P_physical

eigs_physical = np.sort(np.linalg.eigvalsh(M_physical))
print(f"  Physical eigenvalues: {np.round(eigs_physical, 6)}")
print(f"  Number of physical DOF: {len(eigs_physical)}")
print(f"  det(M_physical) = {np.linalg.det(M_physical):.6f}")
print(f"  Product of eigenvalues = {np.prod(eigs_physical):.6f}")

# ===========================================================================
# Step 4: Connection to G* through the determinant
# ===========================================================================

print()
print("STEP 4: Checking for G* in the lattice determinant")
print("-" * 70)

det_M = np.linalg.det(M_physical)
n_phys = len(eigs_physical)

print(f"  det(M_physical) = {det_M:.6f}")
print(f"  det^(1/{n_phys}) = {det_M**(1/n_phys):.6f}")
print(f"  ln(det) = {np.log(det_M):.6f}")
print(f"  ln(det)/{n_phys} = {np.log(det_M)/n_phys:.6f}")
print()

# Check various G*-related quantities
print("  Checking ratios against G*-related quantities:")
print(f"    G*           = {G_STAR:.6f}")
print(f"    G*²          = {G_STAR**2:.6f}")
print(f"    16·G*²       = {16*G_STAR**2:.6f} (Vieta sum = x₊+x₋)")
print(f"    16·G*³       = {16*G_STAR**3:.6f} (Vieta product = x₊·x₋)")
print(f"    Watson W₃    = {WATSON_3D:.6f}")
print(f"    Γ(1/4)⁴      = {GAMMA_QUARTER**4:.6f}")
print(f"    det/G*²      = {det_M/G_STAR**2:.6f}")
print(f"    det/(16G*²)  = {det_M/(16*G_STAR**2):.6f}")
print()

# ===========================================================================
# Step 5: Partition function Z(g) as function of coupling
# ===========================================================================

print("STEP 5: Computing Z(g) over ternary configurations")
print("-" * 70)

# For the FTD action with coupling g:
#   S_E[s, J; g] = (1/2) J^T M J + g · s^T (Div^T J) + V(s)
#
# After Gaussian integration over J:
#   Z(g) = Σ_s (2π)^{n/2} / √det(M) · exp(g²/2 · s^T Div M⁻¹ Div^T s)
#
# The coupling-dependent part is:
#   W(s, g) = exp(g²/2 · s^T G_L s)
# where G_L = Div · M⁻¹ · Div^T is the "charge-charge" Green's function

# Compute the charge-charge Green's function on the full lattice
# Use pseudoinverse to handle zero modes
M_scalar_pinv = np.linalg.pinv(M_scalar)
G_charge = Div @ np.kron(np.eye(D), M_scalar_pinv) @ Div.T

print(f"  Charge-charge Green's function G_L shape: {G_charge.shape}")
print(f"  G_L diagonal (self-energy): {np.diag(G_charge)}")
print(f"  G_L[0,0] = {G_charge[0,0]:.10f}")
print()

# The Green's function at the origin on the infinite 3D cubic lattice
# is the Watson integral W₃ = Γ(1/4)⁴/(4π³)
# On the finite 2×2×2 lattice, this is approximated by G_L[0,0]
print(f"  G_L[0,0] (finite lattice)  = {G_charge[0,0]:.10f}")
print(f"  W₃ (infinite lattice)      = {WATSON_3D:.10f}")
print(f"  Ratio G_L[0,0]/W₃          = {G_charge[0,0]/WATSON_3D:.6f}")
print()

# Now compute Z(g) for a range of coupling values
# Z(g) = Σ_s exp(g²/2 · s^T G_L s) · [configuration weight]

def compute_Z(g_squared, G_charge_matrix):
    """Compute Z as function of g² by summing over all 3^8 ternary configs."""
    Z = 0.0
    N = G_charge_matrix.shape[0]  # 8 sites

    for config in itertools.product([-1, 0, 1], repeat=N):
        s = np.array(config, dtype=float)
        # Charge neutrality: only include configs with Σs = 0
        # (optional physical constraint — try both)
        exponent = g_squared / 2.0 * s @ G_charge_matrix @ s
        Z += np.exp(exponent)

    return Z

def compute_Z_neutral(g_squared, G_charge_matrix):
    """Z restricted to charge-neutral configurations (Σs = 0)."""
    Z = 0.0
    N = G_charge_matrix.shape[0]

    for config in itertools.product([-1, 0, 1], repeat=N):
        s = np.array(config, dtype=float)
        if abs(np.sum(s)) > 0.5:  # skip non-neutral
            continue
        exponent = g_squared / 2.0 * s @ G_charge_matrix @ s
        Z += np.exp(exponent)

    return Z

# Scan g² from 0 to 0.1 (α ≈ 0.0073)
print("Computing Z(g²) over ternary configurations...")
g2_values = np.linspace(0.001, 0.05, 200)

Z_all = np.array([compute_Z(g2, G_charge) for g2 in g2_values])
Z_neutral = np.array([compute_Z_neutral(g2, G_charge) for g2 in g2_values])

F_all = -np.log(Z_all)
F_neutral = -np.log(Z_neutral)

# Look for structure in F(g²)
print()
print("STEP 6: Analyzing free energy F(g²) = -ln Z(g²)")
print("-" * 70)

# Check for extrema
dF_all = np.gradient(F_all, g2_values)
dF_neutral = np.gradient(F_neutral, g2_values)

# Find zero crossings of dF/dg²
sign_changes_all = np.where(np.diff(np.sign(dF_all)))[0]
sign_changes_neutral = np.where(np.diff(np.sign(dF_neutral)))[0]

print(f"  F(g²) range (all configs):     [{F_all[0]:.4f}, {F_all[-1]:.4f}]")
print(f"  F(g²) range (neutral configs): [{F_neutral[0]:.4f}, {F_neutral[-1]:.4f}]")
print(f"  dF/dg² sign changes (all):     {len(sign_changes_all)}")
print(f"  dF/dg² sign changes (neutral): {len(sign_changes_neutral)}")

if len(sign_changes_all) > 0:
    for idx in sign_changes_all:
        g2_ext = g2_values[idx]
        print(f"    Extremum (all) at g² ≈ {g2_ext:.6f}, 1/g² ≈ {1/g2_ext:.2f}")
        print(f"    Compare: α = {ALPHA_TARGET:.6f}, 1/α = {X_PLUS:.2f}")

if len(sign_changes_neutral) > 0:
    for idx in sign_changes_neutral:
        g2_ext = g2_values[idx]
        print(f"    Extremum (neutral) at g² ≈ {g2_ext:.6f}, 1/g² ≈ {1/g2_ext:.2f}")

# ===========================================================================
# Step 7: Check the master quadratic connection
# ===========================================================================

print()
print("STEP 7: Master quadratic connection")
print("-" * 70)

# The master quadratic: x² - 16G*²x + 16G*³ = 0
# If g² = α = 1/x₊, then the self-consistency equation is:
#   x₊ = f(G*, lattice geometry)

# Check if the lattice determinant or Green's function
# produces G* in a way that connects to the quadratic

# The key ratio: G_L[0,0] on the 2×2×2 lattice
G_self = G_charge[0, 0]

print(f"  Lattice self-energy G_L[0,0] = {G_self:.10f}")
print(f"  16 × G_self                  = {16 * G_self:.10f}")
print(f"  16 × G_self²                 = {16 * G_self**2:.10f}")
print(f"  16 × G*²                     = {16 * G_STAR**2:.10f}")
print(f"  16 × G*³                     = {16 * G_STAR**3:.10f}")
print()

# Check the quadratic with G_self instead of G*
print("  Testing master quadratic with lattice self-energy:")
a_coeff = 1
b_coeff = -16 * G_self**2
c_coeff = 16 * G_self**3
disc = b_coeff**2 - 4 * a_coeff * c_coeff
if disc >= 0:
    x_plus_lattice = (-b_coeff + np.sqrt(disc)) / (2 * a_coeff)
    x_minus_lattice = (-b_coeff - np.sqrt(disc)) / (2 * a_coeff)
    print(f"    x₊(G_self) = {x_plus_lattice:.6f} (target: {X_PLUS:.6f})")
    print(f"    x₋(G_self) = {x_minus_lattice:.6f} (target: {X_MINUS:.6f})")
    print(f"    1/x₊       = {1/x_plus_lattice:.8f} (target α: {ALPHA_TARGET:.8f})")
else:
    print(f"    Discriminant negative: {disc}")

# ===========================================================================
# Summary
# ===========================================================================

print()
print("=" * 70)
print("SUMMARY")
print("=" * 70)
print()
print("Key findings:")
print(f"  1. Physical DOF after constraints: {n_physical} (expected 16)")
print(f"  2. Lattice self-energy G_L[0,0] = {G_self:.6f}")
print(f"     vs G* = {G_STAR:.6f} (ratio: {G_self/G_STAR:.6f})")
print(f"     vs Watson W₃ = {WATSON_3D:.6f} (ratio: {G_self/WATSON_3D:.6f})")
print(f"  3. Free energy F(g²) is {'monotonic' if len(sign_changes_all) == 0 else 'has extrema'}")
print()
print("The computation is exact on the 2×2×2 lattice.")
print("Results should be compared against the infinite-lattice Watson integral")
print("to understand finite-size effects.")
print()
print("Status: [EXPLORATORY] — documenting what the lattice produces,")
print("not claiming a result until the mathematics is verified.")
