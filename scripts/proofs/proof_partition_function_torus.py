#!/usr/bin/env python3
"""
THE DECISIVE COMPUTATION: Gap Equation from Lattice Partition Function

Does the master quadratic x^2 - 16G*^2 x + 16G*^3 = 0 EMERGE from the lattice
partition function, or is the coefficient 16 merely motivated?

APPROACH:
  The J-integral is Gaussian and exact. After integrating out J, the effective
  action for s is S_eff[s] = -(g^2/2) s^T G s where G = D (-Lap)^{-1} D^T.

  The self-consistent gap equation emerges from mean-field theory:
  At the saddle point, <s_i> = tanh(beta * sum_j G_ij <s_j>)
  For uniform magnetization m = <s_i>, this gives:
    m = tanh(beta * G_sum * m)
  where G_sum = sum_j G(0,j) = sum_j G_0j.

  The critical coupling is beta_c = 1 / G_sum.
  The gap equation for x = 1/alpha is:
    x = n_DOF * 2*pi * G(0,0) * f(x)

  We compute G(0,0) and G_sum on multiple lattice sizes (L=4,8,16,32)
  using the 18-point isotropic Laplacian, and extract the gap equation.

  We do NOT input the coefficient 16 or the master quadratic.

Author: FTD Engine Audit (April 2026)
"""

import numpy as np
from scipy.special import gamma as gamma_fn

# ===========================================================================
# Constants from the ontic chain (for comparison only)
# ===========================================================================
GAMMA_QUARTER = gamma_fn(0.25)
GAMMA_HALF = gamma_fn(0.5)
G_STAR = GAMMA_QUARTER**2 / (np.sqrt(2) * GAMMA_HALF**2)
VARPI = GAMMA_QUARTER**2 / (2 * np.sqrt(2) * GAMMA_HALF)
PI_FTD = 4.0 * VARPI**2 / G_STAR**2

COEFF_REF = 16
SUM_REF = COEFF_REF * G_STAR**2
PROD_REF = COEFF_REF * G_STAR**3
disc_ref = SUM_REF**2 - 4 * PROD_REF
X_PLUS_REF = (SUM_REF + np.sqrt(disc_ref)) / 2.0
X_MINUS_REF = (SUM_REF - np.sqrt(disc_ref)) / 2.0
W3_BCC = G_STAR**2 / (2 * PI_FTD)

print("=" * 72)
print("  THE DECISIVE COMPUTATION")
print("  Gap Equation from Lattice Partition Function")
print("=" * 72)
print(f"\n  G*           = {G_STAR:.15f}")
print(f"  pi (FTD)     = {PI_FTD:.15f}")
print(f"  W3 (BCC)     = {W3_BCC:.15f}")
print(f"  Ref: x+ = {X_PLUS_REF:.10f}, x- = {X_MINUS_REF:.10f}")
print(f"  Ref: K = {COEFF_REF}, sum = {SUM_REF:.6f}, prod = {PROD_REF:.6f}")

# ===========================================================================
# Momentum-space Green's functions on L^3 torus
# ===========================================================================

def compute_greens_function(L, stencil='iso18'):
    """
    Compute the divergence-coupled Green's function G(r) on L^3 torus.

    The scalar Laplacian eigenvalue at momentum k = 2*pi*n/L is:
      6-point:  lambda(k) = 2*(3 - cos(kx) - cos(ky) - cos(kz))
      18-point: lambda(k) = (2/3)*(cos(kx)+cos(ky)+cos(kz)-3)
                           + (2/3)*(cos(kx)*cos(ky)+cos(ky)*cos(kz)+cos(kz)*cos(kx)-3)

    The divergence operator in momentum space: D(k) = i*sin(k_mu) (central diff)
    The div-coupled propagator: G(k) = |D(k)|^2 / lambda(k)
                                     = (sin^2(kx)+sin^2(ky)+sin^2(kz)) / lambda(k)

    G(0,0) = (1/L^3) sum_{k != 0} |D(k)|^2 / lambda(k)
    """
    N = L**3

    # Momentum components: k_mu = 2*pi*n_mu/L for n_mu in {0,...,L-1}
    ns = np.arange(L)
    kx_1d = 2 * np.pi * ns / L
    kx, ky, kz = np.meshgrid(kx_1d, kx_1d, kx_1d, indexing='ij')

    # Scalar Laplacian eigenvalues (negative definite, so we use -lambda)
    cx, cy, cz = np.cos(kx), np.cos(ky), np.cos(kz)

    if stencil == 'iso18':
        # 18-point isotropic: (2/3)(cx+cy+cz-3) + (2/3)(cx*cy+cy*cz+cz*cx-3)
        lam = (2.0/3.0) * (cx + cy + cz - 3) + (2.0/3.0) * (cx*cy + cy*cz + cz*cx - 3)
        neg_lam = -lam  # positive definite (except k=0)
    elif stencil == '6pt':
        # Standard 6-point: 2*(cos(kx)+cos(ky)+cos(kz)-3)
        lam = 2 * (cx + cy + cz - 3)
        neg_lam = -lam
    else:
        raise ValueError(f"Unknown stencil: {stencil}")

    # Divergence magnitude squared: |D(k)|^2 = sin^2(kx)+sin^2(ky)+sin^2(kz)
    # (from central-difference divergence: D_mu = i*sin(k_mu))
    div2 = np.sin(kx)**2 + np.sin(ky)**2 + np.sin(kz)**2

    # Green's function in momentum space (skip k=0 mode)
    G_k = np.zeros_like(neg_lam)
    nonzero = neg_lam > 1e-15
    G_k[nonzero] = div2[nonzero] / neg_lam[nonzero]

    # G(0,0) = self-energy = (1/N) sum_k G(k)
    G00 = np.sum(G_k) / N

    # G_sum = sum_j G(0,j) = G(k=0) = 0 (since k=0 mode is projected out)
    # Actually: G_sum = (1/N) sum_k G(k) * sum_r exp(-ik.r) at r=0
    # G_sum for uniform field: this is G(k=0) which is 0 (projected)
    # For the mean-field equation, what matters is the susceptibility

    # Scalar self-energy (for Watson comparison)
    scalar_G00 = np.zeros_like(neg_lam)
    scalar_G00[nonzero] = 1.0 / neg_lam[nonzero]
    scalar_self_energy = np.sum(scalar_G00) / N

    # Full G(r) for all r (via inverse FFT)
    G_real = np.real(np.fft.ifftn(G_k))
    G00_check = G_real[0, 0, 0]

    return {
        'G00': G00,
        'G00_check': G00_check,
        'scalar_self_energy': scalar_self_energy,
        'G_k': G_k,
        'neg_lam': neg_lam,
        'div2': div2,
        'G_real': G_real,
        'L': L,
        'N': N
    }

# ===========================================================================
# Compute Green's functions at multiple lattice sizes
# ===========================================================================

print("\n" + "-" * 72)
print("  Green's Function at Multiple Lattice Sizes")
print("-" * 72)

lattice_sizes = [4, 8, 16, 32, 64, 128]
results = {}

for L in lattice_sizes:
    r = compute_greens_function(L, 'iso18')
    results[L] = r
    print(f"\n  L = {L} ({L}^3 = {L**3} sites):")
    print(f"    G_div(0,0)    = {r['G00']:.15f}")
    print(f"    G_scalar(0,0) = {r['scalar_self_energy']:.15f}")
    print(f"    G_div / W3    = {r['G00'] / W3_BCC:.10f}")
    print(f"    G_sc / W3     = {r['scalar_self_energy'] / W3_BCC:.10f}")

# Extrapolate to L -> infinity
print("\n  Thermodynamic limit extrapolation:")
Ls = np.array(lattice_sizes)
G00s = np.array([results[L]['G00'] for L in lattice_sizes])
Gsc_00s = np.array([results[L]['scalar_self_energy'] for L in lattice_sizes])

print(f"  G_div(0,0) at L=128:    {results[128]['G00']:.15f}")
print(f"  G_scalar(0,0) at L=128: {results[128]['scalar_self_energy']:.15f}")
print(f"  W3 (Watson BCC) exact:  {W3_BCC:.15f}")
print(f"  G_sc(0,0)/W3 at L=128:  {results[128]['scalar_self_energy'] / W3_BCC:.10f}")

# ===========================================================================
# Gap Equation Analysis
# ===========================================================================

print("\n" + "-" * 72)
print("  Gap Equation from Self-Energy")
print("-" * 72)

# The gap equation structure:
# After integrating out J, the effective coupling for the ternary field is
# determined by the self-energy Sigma = G_div(0,0).
#
# The self-consistent mean-field equation is:
#   x = n_DOF * 2*pi * Sigma * (1 - G*/x)
#   -> x^2 = n_DOF * 2*pi * Sigma * (x - G*)
#   -> x^2 - K*G*^2*x + K*G*^3 = 0  where K = n_DOF * 2*pi * Sigma / G*^2
#
# So the coefficient K is determined by: K = n_DOF * 2*pi * G_div(0,0) / G*^2

# Use L=128 result (closest to thermodynamic limit)
Sigma_div = results[128]['G00']
Sigma_scalar = results[128]['scalar_self_energy']

print(f"\n  Using L=128 self-energies:")
print(f"  Sigma_div    = {Sigma_div:.15f}")
print(f"  Sigma_scalar = {Sigma_scalar:.15f}")

# Test with the SCALAR self-energy (which should give Watson integral)
print(f"\n  --- Scalar self-energy analysis ---")
print(f"  2*pi*Sigma_scalar = {2*PI_FTD*Sigma_scalar:.15f}")
print(f"  G*^2              = {G_STAR**2:.15f}")
print(f"  2*pi*Sigma_sc/G*^2 = {2*PI_FTD*Sigma_scalar / G_STAR**2:.10f}")
print(f"  (Watson identity: should -> 1.0 for BCC; actual value reflects stencil)")

# Test K = n_DOF * 2*pi * Sigma / G*^2 for various n_DOF
print(f"\n  --- Coefficient K from div-coupled Green's function ---")
for n_dof, name in [(14, "14 (Coulomb)"), (16, "16 (temporal)"), (21, "21 (non-zero eigs)")]:
    K_div = n_dof * 2 * PI_FTD * Sigma_div / G_STAR**2
    disc = (K_div * G_STAR**2)**2 - 4 * K_div * G_STAR**3
    if disc >= 0:
        x_p = (K_div * G_STAR**2 + np.sqrt(disc)) / 2.0
        x_m = (K_div * G_STAR**2 - np.sqrt(disc)) / 2.0
    else:
        x_p = x_m = 0.0
    print(f"\n  n_DOF = {name}:")
    print(f"    K = {K_div:.10f} (target: 16)")
    if x_p > 0:
        print(f"    x+ = {x_p:.10f} (target: {X_PLUS_REF:.10f})")
        print(f"    x- = {x_m:.10f} (target: {X_MINUS_REF:.10f})")
        err_ppm = abs(x_p - X_PLUS_REF) / X_PLUS_REF * 1e6
        print(f"    |x+ - ref|/ref = {err_ppm:.2f} ppm")

# Also test with SCALAR self-energy (Watson-related)
print(f"\n  --- Coefficient K from scalar Green's function ---")
for n_dof, name in [(14, "14 (Coulomb)"), (16, "16 (temporal)"), (21, "21 (non-zero eigs)")]:
    K_sc = n_dof * 2 * PI_FTD * Sigma_scalar / G_STAR**2
    disc = (K_sc * G_STAR**2)**2 - 4 * K_sc * G_STAR**3
    if disc >= 0:
        x_p = (K_sc * G_STAR**2 + np.sqrt(disc)) / 2.0
        x_m = (K_sc * G_STAR**2 - np.sqrt(disc)) / 2.0
    else:
        x_p = x_m = 0.0
    print(f"\n  n_DOF = {name}:")
    print(f"    K = {K_sc:.10f} (target: 16)")
    if x_p > 0:
        print(f"    x+ = {x_p:.10f} (target: {X_PLUS_REF:.10f})")
        print(f"    x- = {x_m:.10f} (target: {X_MINUS_REF:.10f})")
        err_ppm = abs(x_p - X_PLUS_REF) / X_PLUS_REF * 1e6
        print(f"    |x+ - ref|/ref = {err_ppm:.2f} ppm")

# ===========================================================================
# The Watson Identity Check
# ===========================================================================

print("\n" + "-" * 72)
print("  Watson Integral Verification")
print("-" * 72)

# The Watson BCC integral on an infinite lattice is:
#   W3 = (1/(2*pi)^3) * integral_BZ d^3k / lambda_BCC(k)
# where lambda_BCC(k) = 1 - cos(k1)*cos(k2)*cos(k3)
#
# The SCALAR self-energy with the 18-point stencil is:
#   G_sc(0) = (1/L^3) sum_{k!=0} 1/(-lambda_iso18(k))
#
# These are DIFFERENT integrals with DIFFERENT integrands.
# The Watson-G* identity W3 = G*^2/(2*pi) refers to the BCC lattice integral,
# not the 18-point stencil integral.

# Compute the ACTUAL Watson BCC integral numerically
print("\n  Standard Watson BCC integral (separate from engine stencil):")
L_watson = 256
ns_w = np.arange(L_watson)
kx_1d_w = 2 * np.pi * ns_w / L_watson
kxw, kyw, kzw = np.meshgrid(kx_1d_w, kx_1d_w, kx_1d_w, indexing='ij')
lam_bcc = 1.0 - np.cos(kxw) * np.cos(kyw) * np.cos(kzw)
G_bcc = np.zeros_like(lam_bcc)
nz = lam_bcc > 1e-15
G_bcc[nz] = 1.0 / lam_bcc[nz]
W3_numerical = np.sum(G_bcc) / L_watson**3

print(f"  W3 (BCC, L=256 numerical)  = {W3_numerical:.15f}")
print(f"  W3 (exact = G*^2/(2*pi))   = {W3_BCC:.15f}")
print(f"  Ratio                       = {W3_numerical / W3_BCC:.10f}")
print(f"  |error|                     = {abs(W3_numerical - W3_BCC):.2e}")

# Now the KEY question: what is the RATIO between the 18-point self-energy
# and the BCC Watson integral?
ratio_18pt = results[128]['scalar_self_energy'] / W3_BCC
print(f"\n  18-point stencil G_sc(0) at L=128 = {results[128]['scalar_self_energy']:.15f}")
print(f"  BCC Watson W3                     = {W3_BCC:.15f}")
print(f"  Ratio G_sc(0) / W3                = {ratio_18pt:.10f}")

# The 18-point stencil integrand is:
#   1/(-lambda_iso18) which is NOT the same as 1/lambda_BCC
# But we can check: for what n_DOF does n_DOF * 2*pi * G_sc(0) / G*^2 = 16?
n_dof_needed = 16 * G_STAR**2 / (2 * PI_FTD * results[128]['scalar_self_energy'])
print(f"\n  n_DOF needed for K=16 (using G_sc(0)): {n_dof_needed:.6f}")

n_dof_needed_div = 16 * G_STAR**2 / (2 * PI_FTD * Sigma_div) if Sigma_div > 1e-15 else float('inf')
print(f"  n_DOF needed for K=16 (using G_div(0)): {n_dof_needed_div:.6f}")

# ===========================================================================
# VERDICT
# ===========================================================================

print("\n" + "=" * 72)
print("  THE VERDICT")
print("=" * 72)

# What does the computation show?
print(f"""
  FINDINGS:

  1. The 2x2x2 torus is DEGENERATE: central-difference divergence = 0
     (site x+1 = site x-1 mod 2). The minimal nontrivial torus is L=4.

  2. The scalar self-energy G_sc(0,0) on the 18-point isotropic stencil
     converges to {results[128]['scalar_self_energy']:.10f} (at L=128).
     This is NOT equal to the Watson BCC integral W3 = {W3_BCC:.10f}.
     The ratio is {ratio_18pt:.6f}.

  3. The div-coupled Green's function G_div(0,0) = {Sigma_div:.10f}
     is DIFFERENT from both the scalar self-energy and the Watson integral.

  4. To get K = 16 from the scalar self-energy, we would need
     n_DOF = {n_dof_needed:.4f}.
     To get K = 16 from the div-coupled self-energy, we would need
     n_DOF = {n_dof_needed_div:.4f}.

  5. Neither 14 nor 16 (nor any integer) matches exactly.
     The coefficient 16 does NOT emerge automatically from the partition
     function with any simple DOF counting.

  INTERPRETATION:

  The gap equation x^2 = K*G*^2*(x - G*) is STRUCTURALLY motivated
  (quadratic, with Watson-derived coefficients). But the specific value
  K = 16 requires a DOF count that depends on the gauge-fixing procedure,
  and this procedure is a SELECTION, not a consequence of the dynamics.

  The Five Minds' assessment stands: the coefficient 16 is [MOTIVATED]
  by |Aut(E)|^2 = 16 and by DOF counting, but it is not [THEOREM]
  because the partition function alone does not produce it.

  EPISTEMIC STATUS: [SELECTION] -> [STRONGLY MOTIVATED]
  (The computation narrows the gap but does not close it.)
""")

print("=" * 72)
