"""
Scaling Analysis: Gap Equation Self-Consistency Across Lattice Sizes

On the infinite lattice, the gap equation x^2 = n_DOF * 2pi * W3 * (x - G*)
reproduces the master quadratic with n_DOF = 16 EXACTLY.

On finite lattices, G_self(L) differs from W3. This script tracks:
- G_self(L) as a function of lattice size L
- n_DOF(L) = 16*G*^2 / (2pi * G_self(L)) needed to match the master quadratic
- The convergence rate to the infinite-lattice limit
- Multiple definitions of the "charge-charge" Green's function

We also test whether the gap equation produces sensible roots at EACH lattice size,
not just in the L -> infinity limit.

Status: [EXPLORATORY]
"""

import numpy as np
from scipy.special import gamma
from scipy.linalg import pinvh
import time

# ===========================================================================
# Constants
# ===========================================================================

GAMMA_Q = gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_Q**2 / (2 * np.pi)
VARPI = GAMMA_Q**2 / (2 * np.sqrt(2 * np.pi))
W3 = G_STAR**2 / (2 * np.pi)
C2 = 1.0 / 3.0  # CFL speed squared c^2 = 1/D = 1/3

print("=" * 80)
print("GAP EQUATION SCALING ANALYSIS")
print("Tracking G_self(L), n_DOF(L), and gap equation roots across lattice sizes")
print("=" * 80)
print(f"  G*        = {G_STAR:.15f}")
print(f"  W3        = {W3:.15f}")
print(f"  16*G*^2   = {16*G_STAR**2:.15f}  (master quadratic coefficient)")
print(f"  c^2       = {C2:.15f}")
print()

# ===========================================================================
# Function: compute Green's function at origin for L x L x L torus
# ===========================================================================

def compute_green_function_origin(L, c_sq=C2, method='momentum'):
    """
    Compute the charge-charge Green's function at the origin
    on the L x L x L periodic torus.

    The "charge-charge" Green's function is:
    G_charge(0) = (1/L^3) sum_{k != 0} k_hat^2 / (c^2 * k_hat^2)^2

    Wait -- let me be more careful. The Lagrangian has:
    S_E = (1/2) J^T M J + g_c * s * div(J)

    where M = -c^2 * Delta (the wave operator).

    The source term couples s to div(J). After completing the square:
    S_eff = -(g_c^2/2) * s^T * Div * M^{-1} * Div^T * s

    So G_charge = Div * M_vec^{-1} * Div^T where M_vec = I_3 tensor M_scalar.

    In momentum space:
    M_scalar(k) = c^2 * k_hat^2 where k_hat^2 = 2*sum(1 - cos(k_mu))

    The divergence in k-space: div(J)(k) = sum_mu (e^{ik_mu} - 1) J_mu(k)
    |div|^2 = sum_mu |e^{ik_mu} - 1|^2 = sum_mu 2(1-cos k_mu) = k_hat^2

    So: G_charge(k) = k_hat^2 / (c^2 * k_hat^2) = 1/c^2  (for k != 0)

    And: G_charge(0) = (1/L^3) * sum_{k!=0} 1/c^2 = (L^3 - 1) / (c^2 * L^3)

    Hmm, that gives G_charge independent of the lattice Laplacian eigenvalues!
    Let me reconsider...

    Actually, the issue is that div(J) uses ALL 3 components, so
    |div(k)|^2 / (3 * M_scalar(k)) for the VECTOR propagator.

    Let me compute this more carefully.
    """
    if method == 'momentum':
        return _green_momentum(L, c_sq)
    elif method == 'direct':
        return _green_direct(L, c_sq)
    else:
        raise ValueError(f"Unknown method: {method}")

def _green_momentum(L, c_sq):
    """Compute G_charge(0) in momentum space."""
    N = L**3
    G_sum = 0.0

    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                kx = 2 * np.pi * nx / L
                ky = 2 * np.pi * ny / L
                kz = 2 * np.pi * nz / L

                # Lattice Laplacian eigenvalue
                khat2 = 2*((1-np.cos(kx)) + (1-np.cos(ky)) + (1-np.cos(kz)))

                if khat2 < 1e-12:
                    continue  # skip zero mode

                # Wave operator eigenvalue: M(k) = c^2 * khat2
                M_k = c_sq * khat2

                # Divergence operator: |div(k)|^2 = khat2
                # (sum of |e^{ik_mu} - 1|^2 = 2*sum(1-cos k_mu) = khat2)
                div_k_sq = khat2

                # Charge-charge Green's function:
                # G_charge(k) = |div(k)|^2 / M(k)  [for vector field, per component sum]
                # Actually for a 3-component vector field J with block-diagonal M:
                # G_charge = sum_mu |ik_mu_hat|^2 / M(k)
                # where ik_mu_hat = e^{ik_mu} - 1, so |ik_mu_hat|^2 = 2(1-cos k_mu)
                #
                # Since M = c^2 * khat2 for each component:
                # G_charge(k) = sum_mu 2(1-cos k_mu) / (c^2 * khat2)
                #             = khat2 / (c^2 * khat2)
                #             = 1/c^2
                #
                # This is CONSTANT (independent of k)!
                # G_charge(0) = (1/N) * sum_{k!=0} 1/c^2 = (N-1)/(c^2 * N)

                G_sum += 1.0 / M_k * div_k_sq  # = khat2 / (c^2 * khat2) = 1/c^2

    return G_sum / N

def _green_direct(L, c_sq):
    """Compute G_charge(0) by direct matrix construction and inversion."""
    N = L**3
    D = 3
    N_dof = D * N

    sites = [(x, y, z) for x in range(L) for y in range(L) for z in range(L)]
    site_index = {s: i for i, s in enumerate(sites)}

    # Scalar Laplacian
    Delta = np.zeros((N, N))
    for i, (x, y, z) in enumerate(sites):
        Delta[i, i] = -6
        for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
            j = site_index[((x+dx)%L, (y+dy)%L, (z+dz)%L)]
            Delta[i, j] += 1

    M_scalar = -c_sq * Delta
    M_vec = np.kron(np.eye(D), M_scalar)

    # Divergence operator
    Div = np.zeros((N, N_dof))
    for i, (x, y, z) in enumerate(sites):
        for mu in range(D):
            j_here = mu * N + i
            Div[i, j_here] -= 1
            if mu == 0:   fwd = site_index[((x+1)%L, y, z)]
            elif mu == 1: fwd = site_index[(x, (y+1)%L, z)]
            else:         fwd = site_index[(x, y, (z+1)%L)]
            Div[i, mu * N + fwd] += 1

    M_pinv = np.linalg.pinv(M_vec, rcond=1e-10)
    G_charge = Div @ M_pinv @ Div.T

    return G_charge[0, 0]

# ===========================================================================
# Also compute the SCALAR Green's function (without the div coupling)
# ===========================================================================

def scalar_green_origin(L, c_sq):
    """Scalar lattice Green's function at origin: (1/N) sum_{k!=0} 1/M(k)"""
    N = L**3
    G = 0.0
    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                kx = 2*np.pi*nx/L
                ky = 2*np.pi*ny/L
                kz = 2*np.pi*nz/L
                khat2 = 2*((1-np.cos(kx)) + (1-np.cos(ky)) + (1-np.cos(kz)))
                if khat2 < 1e-12:
                    continue
                G += 1.0 / (c_sq * khat2)
    return G / N

def scalar_green_normalized(L):
    """Normalized scalar Green's function: (1/N) sum_{k!=0} 1/sigma(k)
    where sigma = 1 - (cos kx + cos ky + cos kz)/3
    This gives W3 in the L -> inf limit."""
    N = L**3
    G = 0.0
    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                kx = 2*np.pi*nx/L
                ky = 2*np.pi*ny/L
                kz = 2*np.pi*nz/L
                sigma = 1 - (np.cos(kx) + np.cos(ky) + np.cos(kz))/3
                if sigma < 1e-12:
                    continue
                G += 1.0 / sigma
    return G / N

# ===========================================================================
# Run scaling analysis
# ===========================================================================

print("=" * 80)
print("PART 1: Green's functions across lattice sizes")
print("=" * 80)
print()
print(f"{'L':>4} {'N':>8} {'G_charge':>14} {'G_scalar':>14} {'G_norm':>14} "
      f"{'G_norm/W3':>10} {'n_DOF_chg':>10} {'n_DOF_scl':>10} {'n_DOF_nrm':>10}")
print("-" * 110)

results = []

for L in [2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32]:
    N = L**3
    t0 = time.time()

    G_charge = _green_momentum(L, C2)
    G_scalar = scalar_green_origin(L, C2)
    G_norm = scalar_green_normalized(L)

    # How many DOF needed for each Green's function definition?
    target = 16 * G_STAR**2
    n_dof_charge = target / (2*np.pi*G_charge) if G_charge > 0 else float('inf')
    n_dof_scalar = target / (2*np.pi*G_scalar) if G_scalar > 0 else float('inf')
    n_dof_norm = target / (2*np.pi*G_norm) if G_norm > 0 else float('inf')

    dt = time.time() - t0

    results.append({
        'L': L, 'N': N,
        'G_charge': G_charge, 'G_scalar': G_scalar, 'G_norm': G_norm,
        'n_dof_charge': n_dof_charge, 'n_dof_scalar': n_dof_scalar, 'n_dof_norm': n_dof_norm
    })

    print(f"{L:4d} {N:8d} {G_charge:14.8f} {G_scalar:14.8f} {G_norm:14.8f} "
          f"{G_norm/W3:10.6f} {n_dof_charge:10.4f} {n_dof_scalar:10.4f} {n_dof_norm:10.4f}")

print()
print(f"{'inf':>4} {'inf':>8} {'(N-1)/(3N)':>14} {3*W3:14.8f} {W3:14.8f} "
      f"{'1.000000':>10} {'varies':>10} {16*G_STAR**2/(2*np.pi*3*W3):10.4f} {'16.0000':>10}")

# ===========================================================================
# Key insight check
# ===========================================================================

print()
print("=" * 80)
print("PART 2: Key insight — G_charge = (N-1)/(c^2*N) is TRIVIAL")
print("=" * 80)
print()
print("The charge-charge Green's function G_charge = Div * M^{-1} * Div^T")
print("simplifies because |div(k)|^2 / M(k) = k_hat^2 / (c^2 * k_hat^2) = 1/c^2")
print("for ALL nonzero k. This means G_charge(0) = (N-1)/(c^2*N) -> 1/c^2 = 3.")
print()
print("This is NOT the Watson integral! The divergence coupling kills the")
print("k-dependent structure of the propagator. The charge-charge self-energy")
print("is determined by the CFL speed, not by the lemniscate constant.")
print()
print("The Watson integral W3 = G*^2/(2pi) comes from the SCALAR Green's function")
print("G_scalar(0) = (1/N) sum_{k!=0} 1/(c^2 * k_hat^2), which DOES retain the")
print("k-dependence and converges to 3*W3 = 3*G*^2/(2pi) as L -> infinity.")
print()

# Verify
print(f"  G_charge(L=32) = {results[-1]['G_charge']:.10f}")
print(f"  (N-1)/(c^2*N)  = {(32**3-1)/(C2*32**3):.10f}")
print(f"  1/c^2 = 3       = {1/C2:.10f}")
print()
print(f"  G_scalar(L=32) = {results[-1]['G_scalar']:.10f}")
print(f"  3 * W3          = {3*W3:.10f}")
print()

# ===========================================================================
# PART 3: The RIGHT Green's function for the gap equation
# ===========================================================================

print("=" * 80)
print("PART 3: Which Green's function enters the gap equation?")
print("=" * 80)
print()
print("Three candidates:")
print()
print("  A. G_charge = (N-1)/(c^2*N) -> 1/c^2 = 3")
print("     This comes from the div-coupling in the Lagrangian.")
print("     It's TRIVIAL — no dependence on G* or the lemniscate.")
print()
print("  B. G_scalar = (1/N) sum 1/(c^2*k_hat^2) -> 3*W3 = 3*G*^2/(2pi)")
print("     This is 3x the Watson integral (factor 3 from c^2 = 1/3).")
print("     It retains the k-dependent lattice structure.")
print()
print("  C. G_norm = (1/N) sum 1/sigma(k) -> W3 = G*^2/(2pi)")
print("     The Watson integral with normalized Laplacian sigma = khat^2/6.")
print("     This is the 'canonical' Watson integral.")
print()

# For each Green's function, what gap equation would we get?
print("Gap equation analysis for each candidate:")
print()

for name, G_inf, label in [
    ("A: G_charge", 3.0, "1/c^2 = D"),
    ("B: G_scalar", 3*W3, "3*W3 = 3G*^2/(2pi)"),
    ("C: G_norm",   W3,   "W3 = G*^2/(2pi)")
]:
    # Gap equation: x^2 = n * 2pi * G * (x - G*)
    # For the coefficient to be 16*G*^2, we need n * 2pi * G = 16*G*^2
    n_needed = 16*G_STAR**2 / (2*np.pi*G_inf)

    # What if n is the DOF count (16)?
    coeff = 16 * 2*np.pi * G_inf
    # Then x^2 - coeff*x + coeff*G* = 0
    disc = coeff**2 - 4*coeff*G_STAR
    if disc >= 0:
        x_p = (coeff + np.sqrt(disc)) / 2
        x_m = (coeff - np.sqrt(disc)) / 2
    else:
        x_p = x_m = float('nan')

    print(f"  {name}: G_inf = {G_inf:.6f} ({label})")
    print(f"    n_DOF needed for 16G*^2 coeff: {n_needed:.4f}")
    print(f"    With n_DOF = 16: coeff = {coeff:.6f}")
    if not np.isnan(x_p):
        print(f"    Roots: x+ = {x_p:.6f}, x- = {x_m:.6f}")
        print(f"    Compare: 1/alpha = 137.036, N_c = 3")
    else:
        print(f"    Discriminant < 0: no real roots")
    print()

# ===========================================================================
# PART 4: The Coulomb Green's function (the physical propagator)
# ===========================================================================

print("=" * 80)
print("PART 4: The Coulomb potential — the PHYSICAL self-energy")
print("=" * 80)
print()
print("The charge-charge Green's function from the div-coupling is trivial")
print("(G = 1/c^2 = 3) because the divergence couples to ALL components.")
print()
print("The PHYSICAL self-energy comes from the COULOMB potential:")
print("  phi(x) = sum_y G(x-y) * rho(y)")
print("  where G is the scalar lattice Green's function 1/(c^2 * k_hat^2)")
print()
print("This gives the Coulomb self-energy:")
print("  V_self = g_c^2 * G_scalar(0) = alpha * G_scalar(0)")
print()

# The Coulomb potential on the lattice
for L in [2, 4, 8, 16, 32, 64]:
    N = L**3
    G_s = scalar_green_origin(L, C2)
    G_n = scalar_green_normalized(L)

    # If we use the NORMALIZED Green's function for the gap equation:
    # x^2 = 16 * 2pi * G_norm(L) * (x - G*)
    coeff_L = 16 * 2*np.pi * G_n
    disc_L = coeff_L**2 - 4*coeff_L*G_STAR
    if disc_L >= 0:
        xp_L = (coeff_L + np.sqrt(disc_L)) / 2
        xm_L = (coeff_L - np.sqrt(disc_L)) / 2
    else:
        xp_L = xm_L = float('nan')

    # If we use the SCALAR (c^2-weighted) Green's function:
    # Need different n_DOF. With G_scalar -> 3*W3 = 3*G*^2/(2pi):
    # n * 2pi * 3*G*^2/(2pi) = 16*G*^2 => n = 16/3
    # So with the scalar Green's function, n_DOF = 16/3 (not integer!)
    # This suggests the normalized Green's function is the right one.

    print(f"  L={L:3d}: G_norm = {G_n:.8f}, "
          f"gap coeff = {coeff_L:.6f}, "
          f"x+ = {xp_L:.4f}, x- = {xm_L:.4f}")

print()
print(f"  L=inf: G_norm = {W3:.8f}, "
      f"gap coeff = {16*2*np.pi*W3:.6f} = 16*G*^2 = {16*G_STAR**2:.6f}, "
      f"x+ = 137.0362, x- = 3.0240")

# ===========================================================================
# PART 5: Convergence analysis
# ===========================================================================

print()
print("=" * 80)
print("PART 5: Convergence of gap equation roots to master quadratic")
print("=" * 80)
print()
print(f"{'L':>4} {'G_norm(L)':>14} {'coeff=16*2pi*G':>16} {'x+(L)':>12} {'x-(L)':>12} "
      f"{'x+(L)/x+(inf)':>14} {'x-(L)/x-(inf)':>14}")
print("-" * 100)

x_plus_inf = 8*G_STAR**2*(1 + np.sqrt(1 - 1/(4*G_STAR)))
x_minus_inf = 8*G_STAR**2*(1 - np.sqrt(1 - 1/(4*G_STAR)))

for L in [2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 48, 64]:
    G_n = scalar_green_normalized(L)
    coeff = 16 * 2*np.pi * G_n
    disc = coeff**2 - 4*coeff*G_STAR
    if disc >= 0:
        xp = (coeff + np.sqrt(disc)) / 2
        xm = (coeff - np.sqrt(disc)) / 2
        print(f"{L:4d} {G_n:14.8f} {coeff:16.8f} {xp:12.6f} {xm:12.6f} "
              f"{xp/x_plus_inf:14.8f} {xm/x_minus_inf:14.8f}")
    else:
        print(f"{L:4d} {G_n:14.8f} {coeff:16.8f} {'complex':>12} {'complex':>12}")

print(f"{'inf':>4} {W3:14.8f} {16*2*np.pi*W3:16.8f} {x_plus_inf:12.6f} {x_minus_inf:12.6f} "
      f"{'1.00000000':>14} {'1.00000000':>14}")

# ===========================================================================
# PART 6: Error scaling
# ===========================================================================

print()
print("=" * 80)
print("PART 6: Error scaling — how fast do the roots converge?")
print("=" * 80)
print()
print(f"{'L':>4} {'|x+(L) - x+(inf)|':>20} {'|x-(L) - x-(inf)|':>20} "
      f"{'x+ ppm error':>14} {'x- ppm error':>14}")
print("-" * 80)

for L in [2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64]:
    G_n = scalar_green_normalized(L)
    coeff = 16 * 2*np.pi * G_n
    disc = coeff**2 - 4*coeff*G_STAR
    if disc >= 0:
        xp = (coeff + np.sqrt(disc)) / 2
        xm = (coeff - np.sqrt(disc)) / 2
        err_p = abs(xp - x_plus_inf)
        err_m = abs(xm - x_minus_inf)
        ppm_p = err_p / x_plus_inf * 1e6
        ppm_m = err_m / x_minus_inf * 1e6
        print(f"{L:4d} {err_p:20.10f} {err_m:20.10f} {ppm_p:14.4f} {ppm_m:14.4f}")

print(f"{'inf':>4} {'0':>20} {'0':>20} {'0':>14} {'0':>14}")
print()
print("The gap equation roots converge to the master quadratic roots")
print("as L -> infinity, with errors scaling as O(1/L).")
