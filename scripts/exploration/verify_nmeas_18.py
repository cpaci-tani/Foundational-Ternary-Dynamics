"""
N_meas = 18 Verification: Does the Measurement Chain Length Follow from First Principles?

The von Neumann chain terminates at N_meas ~ 18 voxels (FOUND_VON_NEUMANN_CHAIN.md).
This coincides with |SC| + |FCC| = 6 + 12 = 18 (the non-BCC Moore neighbors).

This script tests three independent derivation routes:

Route 1: Gauss constraint degrees of freedom
  On an N-voxel cluster, the Gauss constraint div(J) = rho removes DOF.
  Does the number of independent flux modes equal some critical value at N = 18?

Route 2: Gap equation restricted to cluster size
  The Watson integral on a finite cluster of N voxels gives W(N).
  Does the gap equation have real roots (Domain A) only for N >= N_crit?

Route 3: Flux distribution threshold
  A single-quantum excitation (energy K_B = 0.511) spread across N voxels
  has J_peak ~ K_B/N. Does the measurement threshold require N ~ 18?

Status: [EXPLORATORY]
"""

import numpy as np
from scipy.special import gamma
import sys, os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, N_c, N_base, K_B

# Framework constants
W3_EXACT = G_STAR**2 / (2 * np.pi)
K_EXACT = 16 * G_STAR**2
DISC_EXACT = K_EXACT**2 - 4 * K_EXACT * G_STAR
X_PLUS = (K_EXACT + np.sqrt(DISC_EXACT)) / 2
X_MINUS = (K_EXACT - np.sqrt(DISC_EXACT)) / 2

# Moore sublattice neighbor lists
SC_OFFSETS = [
    (1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1)
]
FCC_OFFSETS = [
    (1,1,0), (1,-1,0), (-1,1,0), (-1,-1,0),
    (1,0,1), (1,0,-1), (-1,0,1), (-1,0,-1),
    (0,1,1), (0,1,-1), (0,-1,1), (0,-1,-1)
]
BCC_OFFSETS = [
    (1,1,1), (1,1,-1), (1,-1,1), (1,-1,-1),
    (-1,1,1), (-1,1,-1), (-1,-1,1), (-1,-1,-1)
]

print("=" * 78)
print("  N_MEAS = 18 VERIFICATION")
print("  Testing three independent routes to derive the measurement chain length")
print("=" * 78)
print()
print(f"  G*       = {G_STAR:.10f}")
print(f"  K_B      = {K_B}")
print(f"  W3       = {W3_EXACT:.10f}")
print(f"  x+       = {X_PLUS:.10f}")
print(f"  x-       = {X_MINUS:.10f}")
print()

# ============================================================================
# Route 1: Gauss constraint DOF counting
# ============================================================================

print("=" * 78)
print("  ROUTE 1: Gauss Constraint Degrees of Freedom")
print("=" * 78)
print()

# On an N-voxel cluster embedded in Z^3:
# - 3N flux DOF (J_x, J_y, J_z at each voxel)
# - N Gauss constraints (div(J) = rho at each voxel, one redundant)
# - N state DOF (s at each voxel, but ternary = log2(3) bits each)
# - 1 global constraint (total charge conservation)
#
# Independent flux DOF = 3N - (N-1) = 2N + 1
# For Type I algebra: need enough DOF for minimal projections
# The number of independent ternary configurations: 3^N
# After Gauss constraint: 3^N / N (approximately)

# The question: at what N does the gap equation become self-consistent?
# The gap equation coefficient K = n_DOF * 2pi * W(N) where:
# - n_DOF depends on the cluster
# - W(N) depends on the cluster's Green's function

# For the full infinite lattice: n_DOF = 16, W = W3, K = 16*G*^2
# For a cluster of N voxels: both n_DOF(N) and W(N) change

# On a cluster of N voxels:
# Free flux DOF = 3N - (N-1) = 2N + 1
# Ratio to full DOF: (2N+1) / (3N) = 2/3 + 1/(3N)

for N_cluster in [6, 8, 12, 14, 18, 19, 26, 27]:
    flux_dof = 3 * N_cluster
    gauss_constraints = N_cluster - 1  # one is redundant (charge conservation)
    free_flux_dof = flux_dof - gauss_constraints
    ternary_configs = 3**N_cluster
    ratio = free_flux_dof / flux_dof

    print(f"  N = {N_cluster:3d}: flux DOF = {flux_dof:3d}, "
          f"Gauss constraints = {gauss_constraints:3d}, "
          f"free DOF = {free_flux_dof:3d}, "
          f"ratio = {ratio:.4f}")

print()
print("  Note: free flux DOF = 2N+1 for all N (linear in N).")
print("  No special value at N = 18. Route 1 does NOT single out 18.")
print()

# ============================================================================
# Route 2: Discriminant progression
# ============================================================================

print("=" * 78)
print("  ROUTE 2: Discriminant-Based Chain Termination")
print("=" * 78)
print()

# The master quadratic Q_k(x) = x^2 - k*G*^2*x + k*G*^3 has:
# Discriminant Delta_k = k*G*^3*(k*G* - 4)
# Delta_k = 0 at k_meas = 4/G* ~ 1.352
#
# If we interpret k as scaling with chain length:
# k(N) starts at k_phys = 16 (full lattice, Domain A, real roots)
# and decreases as the chain extends (more voxels = more averaging)
# The chain terminates when k(N) = 4/G* (Domain C, Delta = 0)
#
# If k decreases linearly: k(N) = 16 - a*N for some rate a
# Then k(N_meas) = 4/G* gives:
# N_meas = (16 - 4/G*) / a

k_meas = 4.0 / G_STAR
k_phys = 16.0
print(f"  k_phys (full lattice) = {k_phys}")
print(f"  k_meas (Delta = 0)    = {k_meas:.6f} = 4/G*")
print(f"  k_phys - k_meas       = {k_phys - k_meas:.6f}")
print()

# If k decreases by 1 per chain link (simplest model):
# N_meas = floor(k_phys - k_meas) = floor(16 - 1.352) = floor(14.648) = 14
# Not 18.

# If k decreases by G*/N_eff = 2.959/13 ~ 0.228 per link:
a_model1 = G_STAR / 13  # N_eff
N_model1 = (k_phys - k_meas) / a_model1
print(f"  Model: k decreases by G*/N_eff = {a_model1:.4f} per link")
print(f"  N_meas = (16 - 4/G*) / (G*/13) = {N_model1:.2f}")

# If k decreases by (k_phys - k_meas)/(N_SC + N_FCC):
a_target = (k_phys - k_meas) / 18
print(f"  Reverse: if N_meas = 18, rate a = {a_target:.6f} per link")
print(f"  = (16 - 4/G*)/18 = {a_target:.6f}")
print(f"  = G*^2/(2*pi) * (something)? G*^2/(2pi) = {W3_EXACT:.6f}")
print(f"  Ratio a/W3 = {a_target/W3_EXACT:.6f}")
print()

# Check: does a = (k_phys - k_meas)/18 have a nice form?
ratio_a = a_target / G_STAR
print(f"  a / G* = {ratio_a:.6f}")
ratio_a2 = a_target * G_STAR
print(f"  a * G* = {ratio_a2:.6f}")
print(f"  k_phys - k_meas = {k_phys - k_meas:.6f}")
print(f"  = 16 - 4/G* = 4(4 - 1/G*) = 4*{4 - 1/G_STAR:.6f}")
print()

# ============================================================================
# Route 3: Flux distribution and K_B threshold
# ============================================================================

print("=" * 78)
print("  ROUTE 3: Flux Distribution and Manifestation Threshold")
print("=" * 78)
print()

# A single-quantum excitation has total energy K_B = 0.511 MeV.
# Spread across N voxels as a Gaussian, the peak flux is:
# J_peak = K_B * (3/(2*pi*sigma^2))^(3/2) for a 3D Gaussian with width sigma
#
# But for a lattice excitation in the Moore neighborhood,
# the relevant spread is across the N neighbors.
# The simplest model: uniform distribution J_peak ~ K_B / N
#
# For manifestation: |J| >= K_B (threshold)
# A SINGLE voxel must exceed K_B for the state to be nonzero.
# Cooperative manifestation: N voxels each contribute J_i,
# and the TOTAL |J|^2 = sum |J_i|^2 >= K_B^2
#
# If flux is equally distributed: N * (K_B/N)^2 = K_B^2/N >= K_B^2
# requires N <= 1 -- only single-voxel manifestation!
#
# But the Gauss constraint REQUIRES flux to spread (div J = rho).
# For a point charge at the center, the flux goes as 1/r^2.
# In the Moore neighborhood:
# SC shell (r=1): J ~ rho/(4*pi*1^2) = rho/(4*pi)
# FCC shell (r=sqrt(2)): J ~ rho/(4*pi*2)
# BCC shell (r=sqrt(3)): J ~ rho/(4*pi*3)

# For the Gauss constraint with a single unit charge at center:
# Total flux through each shell must equal the enclosed charge.
# Discrete version: sum over shell faces of J_n = Q

# SC shell: 6 faces, each carries J_n ~ Q/6
# FCC shell: 12 faces, each carries J_n ~ Q/12
# BCC shell: 8 faces, each carries J_n ~ Q/8

print("  Flux per voxel in discrete Gauss field of unit charge:")
print()
Q = K_B  # manifestation-threshold charge

# The question: at what shell does J_n drop below a critical value?
shells = [
    ("SC (6, r=1)", 6, 1.0),
    ("FCC (12, r=sqrt(2))", 12, np.sqrt(2)),
    ("BCC (8, r=sqrt(3))", 8, np.sqrt(3)),
]

for label, n_voxels, r in shells:
    J_per_voxel = Q / n_voxels  # uniform distribution
    J_coulomb = Q / (4 * np.pi * r**2 * n_voxels)  # Coulomb scaling
    # Actually for discrete Gauss: total flux through shell = Q
    # J_normal * area_per_face * n_faces = Q
    # On lattice: J_normal * n_faces = Q (unit area faces)
    J_gauss = Q / n_voxels
    print(f"  {label}: J_per_voxel = {J_gauss:.4f}  "
          f"(K_B/{n_voxels} = {K_B/n_voxels:.4f})")

print()

# Now: cumulative voxels that the Gauss field penetrates
# SC only: 6 voxels, each sees J = K_B/6 = 0.0852
# SC+FCC: 18 voxels, SC sees K_B/6, FCC sees K_B/12 (less)
# SC+FCC+BCC: 26 voxels, BCC sees K_B/8

# The COOPERATIVE threshold: sum of |J_i|^2 across the cluster
# For manifestation of the point charge, we need the self-energy
# to produce a detectable state field.

# Self-energy = sum_i |J(r_i)|^2 / (2 * K_B)
# For the Gauss field: J(r) ~ Q/(4*pi*r^2) in continuum
# On the lattice: the sum diverges at r=0 (self-energy)

# On the discrete lattice, the self-energy is the Green's function at origin:
# E_self = (g_c^2 / 2) * G(0) where G(0) is the lattice Green's function

# The measurement chain length might relate to the number of sites
# needed for the DISCRETE self-energy sum to converge to its continuum value.

print("  Cumulative |J|^2 contribution by Moore layer:")
print()

cumulative = 0.0
cumulative_voxels = 0

for label, n_voxels, r in shells:
    J_per = Q / (4 * np.pi * r**2)  # Coulomb
    contribution = n_voxels * J_per**2
    cumulative += contribution
    cumulative_voxels += n_voxels
    fraction = cumulative / (Q**2 * W3_EXACT)  # fraction of full self-energy

    print(f"  After {label}: {cumulative_voxels:2d} voxels, "
          f"sum |J|^2 = {cumulative:.6f}, "
          f"fraction of full self-energy = {fraction:.4f}")

print()
print(f"  Full self-energy (lattice) = Q^2 * W3 = {Q**2 * W3_EXACT:.6f}")
print()

# ============================================================================
# The discriminant chain — detailed progression
# ============================================================================

print("=" * 78)
print("  ROUTE 2b: Can we identify k with the measurement depth?")
print("=" * 78)
print()

# The von Neumann chain starts at the object (Domain A, k=16)
# and ends at the measurement (Domain C, k=4/G*).
# Each chain link adds one voxel to the measurement region.
#
# Hypothesis: k(N) = K_total / (2*pi*W(N)) where W(N) is the
# Watson integral on the N-voxel measurement cluster.
# At N -> inf: W(N) -> W3, k -> 16*G*^2 / (2*pi*W3) = 16 (exact)
# At N = 1: W(1) = 0 (trivial cluster), k -> inf
#
# The chain terminates when the discriminant Delta(k) = 0:
# k^2*G*^4 - 4*k*G*^3 = 0 -> k = 4/G*

# Let's compute W(N) for small clusters and track how k(N) evolves.
# For a cluster of N voxels on Z^3, the Green's function depends
# on the cluster geometry. The simplest model: the cluster is a
# sphere of radius r containing N ~ (4/3)*pi*r^3 voxels.

# Cluster Green's function: G_cluster(0) = sum_{r in cluster} 1/(4*pi*r^2)
# (Coulomb form, discrete)

# N_meas is where the k value crosses 4/G*

print(f"  k_phys = 16, k_meas = 4/G* = {k_meas:.6f}")
print()
print(f"  {'N':>4} {'G_cluster(0)':>14} {'k(N) = K/2piG':>16} {'Delta(k)':>14} {'Domain':>8}")
print("  " + "-" * 60)

# Build discrete shell model
all_offsets = []
# Shell 0: center
all_offsets.append([(0,0,0)])
# Shell 1: SC (6)
all_offsets.append(SC_OFFSETS[:])
# Shell 2: FCC (12)
all_offsets.append(FCC_OFFSETS[:])
# Shell 3: BCC (8)
all_offsets.append(BCC_OFFSETS[:])

# Cumulative cluster with Coulomb Green's function
G_cluster = 0.0
N_total = 0
for shell_idx, shell in enumerate(all_offsets):
    for offset in shell:
        N_total += 1
        r2 = offset[0]**2 + offset[1]**2 + offset[2]**2
        if r2 > 0:
            G_cluster += 1.0 / (4 * np.pi * r2)
        else:
            # Self-energy at origin: use lattice regularization
            # G(0) ~ W3 for the full lattice; for a single site, divergent
            # Skip center contribution for now
            pass

    # Compute k from G_cluster
    if G_cluster > 0:
        k_N = K_EXACT / (2 * np.pi * G_cluster)
    else:
        k_N = float('inf')

    delta_N = k_N**2 * G_STAR**4 - 4 * k_N * G_STAR**3

    if delta_N > 0:
        domain = "A (real)"
    elif abs(delta_N) < 1e-6:
        domain = "C (degen)"
    else:
        domain = "B (cplx)"

    shell_labels = ["center", "SC(6)", "FCC(12)", "BCC(8)"]
    print(f"  {N_total:4d} {G_cluster:14.8f} {k_N:16.6f} {delta_N:14.4f} {domain:>8}"
          f"  [{shell_labels[shell_idx]}]")

print()
print("  Note: k(N) decreases as more voxels are added because G_cluster grows.")
print("  The chain reaches Domain C (Delta=0) when k = 4/G* = %.4f" % k_meas)
print("  This does NOT occur at N=18 with the simple Coulomb model.")
print("  The model is too crude — the lattice Green's function is not 1/(4*pi*r^2).")

# ============================================================================
# Summary
# ============================================================================

print()
print("=" * 78)
print("  SUMMARY")
print("=" * 78)
print()
print("  Route 1 (Gauss DOF): Free DOF = 2N+1, linear in N. No special value at 18.")
print("  Route 2 (Discriminant): Crude Coulomb model does not terminate at N=18.")
print("  Route 3 (Flux threshold): Cumulative self-energy grows with shell count,")
print("           but no sharp transition at the SC+FCC boundary.")
print()
print("  CONCLUSION: N_meas = 18 does NOT follow from any of these simple routes.")
print("  The coincidence 18 = |SC| + |FCC| remains [CONJECTURE].")
print("  The measurement chain termination likely arises from the COMBINATION of")
print("  all four mechanisms (structural, algebraic, self-referential, discriminant)")
print("  acting together, not from any single mechanism in isolation.")
print("  A full lattice simulation with the engine's tick cycle may be needed")
print("  to determine the actual chain length dynamically.")
