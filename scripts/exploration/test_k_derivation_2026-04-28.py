"""
Derive k = 1/N_base = 1/4 from O_h representation theory.

Test: project the 4x4 18-point Laplacian onto the A_{1g} subspace of the
27-voxel Moore block. Diagonalize. Check whether the initial condition
delta_center distributes evenly across the 4 eigenvectors.

If yes -> k = 1/4 is structural (equipartition across O_h orbits).
If no -> the equipartition hypothesis needs refinement.

[STRUCTURAL HYPOTHESIS] -- in pursuit of FTD-0110 [THEOREM] tag, 2026-04-28.
"""

import numpy as np

# Construct the 4x4 Laplacian matrix on the A_{1g} subspace.
# Basis: e_0 = center, e_1 = SC orbit, e_2 = FCC orbit, e_3 = BCC orbit.
# All normalized so that ||e_i|| = 1.
#
# Matrix elements derived by direct counting:
#   18-point Laplacian: face weight 1/3, edge weight 1/6, self -4
#   Acting on functions defined on the 3^3 Moore block.

s2 = np.sqrt(2)
s3 = np.sqrt(3)
s6 = np.sqrt(6)

M = np.array([
    [-4.0,    s6/3,   s3/3,   0.0   ],
    [ s6/3,  -10/3,   2*s2/3, s3/3  ],
    [ s3/3,   2*s2/3,-10/3,   s6/3  ],
    [ 0.0,    s3/3,   s6/3,   -4.0  ],
])

print("=" * 70)
print("k = 1/N_base = 1/4 derivation test")
print("Project 18-point Laplacian onto A_{1g} subspace; check equipartition.")
print("=" * 70)

print("\n4x4 Laplacian M on A_{1g} subspace:")
print(M)
print(f"\nSymmetric? {np.allclose(M, M.T)}")
print(f"Trace = {np.trace(M):.4f}  (expect -44/3 = {-44/3:.4f})")

# Diagonalize
eigvals, eigvecs = np.linalg.eigh(M)
print(f"\nEigenvalues (sorted ascending): {eigvals}")
print(f"Sum: {sum(eigvals):.4f}  (should match trace -44/3 = {-44/3:.4f})")

# Project the initial condition delta_center = e_0 = (1, 0, 0, 0) onto each eigvec
v0 = np.array([1.0, 0.0, 0.0, 0.0])
print("\n--- Projection of delta_center = e_0 onto each eigenvector ---")
projs = eigvecs.T @ v0    # coefficients in eigenbasis
projs_sq = projs ** 2
for i, lam in enumerate(eigvals):
    print(f"  Eigenvalue {lam:+.4f}  |proj|^2 = {projs_sq[i]:.6f}    "
          f"eigvec = {eigvecs[:, i]}")

print(f"\nSum of projection energies: {sum(projs_sq):.6f}  (should be 1.0)")
print(f"\n--- Equipartition test ---")
print(f"Projection-energy fractions: {projs_sq}")
print(f"Mean: {np.mean(projs_sq):.4f}, Std: {np.std(projs_sq):.4f}")
print(f"Equipartition prediction:    0.25 (each)")
ratio = max(projs_sq) / max(min(projs_sq), 1e-12)
print(f"Max/min ratio: {ratio:.2f}")

if ratio < 1.5:
    print("VERDICT: PASS -- roughly equipartitioned (energy spreads evenly)")
elif ratio < 4:
    print("VERDICT: PARTIAL -- structure but not full equipartition")
else:
    print("VERDICT: FAIL -- strongly unequal; simple equipartition refuted")

# Mass analysis: which eigenvector is the 'manifestation channel'?
print("\n=== Slowest mode (longest-lived; closest to DC) ===")
slowest_idx = np.argmin(np.abs(eigvals))
print(f"  Eigenvalue = {eigvals[slowest_idx]:+.4f}  (smallest |lambda|)")
print(f"  Eigenvector: {eigvecs[:, slowest_idx]}")
print(f"  Energy fraction in this mode: {projs_sq[slowest_idx]:.4f}")
print(f"  Cf 1/N_base = 1/4 = 0.25")

# Slowest mode would dominate at long times -- if cluster manifests via this mode
# only, its energy contains projs_sq[slowest_idx] of the initial delta-energy.

# Check what fraction of e_0 (the injection) overlaps with the FULL A_{1g} subspace.
# Since we projected ONTO A_{1g}, this is 1.0 by construction.
# But the REAL injection in the 27-block is delta_center, which IS purely A_{1g}
# (a single voxel at the high-symmetry point is O_h-invariant).
print("\n=== Sanity check ===")
print(f"||delta_center||^2 in full 27-dim space: 1.0")
print(f"Sum of A_{{1g}} projections: {sum(projs_sq):.4f}")
print(f"  (= 1.0 confirms delta_center lives entirely in A_{{1g}}; expected.)")

# Now: the claim k=1/4 says cluster size = 1/4 * A^2 in steady state.
# If slowest-mode energy fraction = 1/4 AND cluster manifests only on this
# eigenvector, the chain holds.
# But if energies are unequal, we need a different mechanism.

# Compute also: the projection onto the 'uniform-on-Moore-3^3 block' eigvec.
# That would be the e_0 + e_1 + e_2 + e_3 direction, normalized.
uniform = np.array([1, np.sqrt(6), np.sqrt(12), np.sqrt(8)])  # weights to make uniform = const on all 27 voxels
uniform = uniform / np.linalg.norm(uniform)
overlap_uniform = uniform @ v0
print(f"\n=== Uniform-on-block eigenvector ===")
print(f"  uniform = {uniform}")
print(f"  Is uniform an eigenvector?")
Lu = M @ uniform
ratio_check = Lu / uniform
print(f"  M*uniform / uniform = {ratio_check}")
if np.allclose(ratio_check, ratio_check[0], atol=1e-3):
    print(f"  YES, eigenvalue = {ratio_check[0]:.4f}")
    print(f"  Energy fraction of e_0 along this mode: {overlap_uniform**2:.4f}")
else:
    print(f"  NO (ratios not constant; uniform is mixed across eigenvectors)")
    print(f"  Components of uniform in eigenbasis:")
    for i, lam in enumerate(eigvals):
        c = eigvecs[:, i] @ uniform
        print(f"    {lam:+.4f}: |c|^2 = {c**2:.4f}")
