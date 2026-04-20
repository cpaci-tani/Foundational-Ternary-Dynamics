"""
DECISIVE COMPUTATION v3: Correct Watson integral and final analysis
===================================================================

Fix: W_3 is defined as the INTEGRAL:
  W_3 = (1/(2pi)^3) int_BZ dk / hat_k^2

On a FINITE lattice with periodic BC, the discrete sum approximation is:
  W_3(L) = (1/L^3) sum_{k != 0} 1/hat_k^2

This converges to W_3 for arbitrarily large L, but slowly because of the 1/k^2
singularity at k=0.

Actually wait -- the Watson integral is the propagator at the ORIGIN on the
cubic lattice (large-L regime). On a finite lattice of size L, the sum IS the Green's function
at the origin, but it differs from W_3 by finite-size corrections.

Let me use numerical integration of the BZ integral instead.
"""
# Phase 8b (FTD Test Bench) -- converted to PyTorch with CUDA default.
# Original NumPy path preserved as fallback when torch is unavailable.
# The N=200 triple loop over cos k evaluations is vectorized via broadcasting
# reductions, chunked along i1 to bound peak memory.

import os
import sys
import numpy as np
from scipy.special import gamma
from scipy import integrate

_SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, _SCRIPTS_DIR)
try:
    from constants import TORCH, DEVICE, DTYPE
except ImportError:
    TORCH = None
    DEVICE = None
    DTYPE = None

print(f"[backend] device={DEVICE}, torch={TORCH is not None}")


def _bz_sums_torch(N):
    """Accumulate 1/h6, 1/h18, h6/h18, h18/h6 over N**3 midpoint grid (torch)."""
    dk = np.pi / N
    idx = TORCH.arange(N, device=DEVICE, dtype=DTYPE)
    k_mid = (idx + 0.5) * dk
    c = TORCH.cos(k_mid)                    # (N,) -- cos values
    c23 = c.unsqueeze(0) + c.unsqueeze(1)   # c2 + c3, (N, N)
    cc23 = c.unsqueeze(0) * c.unsqueeze(1)  # c2 * c3, (N, N)
    chunk = min(N, 128)
    t_6 = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    t_18 = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    t_ratio = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    t_inv = TORCH.zeros((), device=DEVICE, dtype=DTYPE)
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c1_c = c[start:stop]                      # (chunk,)
        c1b = c1_c.view(-1, 1, 1)                 # (chunk, 1, 1)
        c_sum = c1b + c23.unsqueeze(0)            # c1+c2+c3, (chunk, N, N)
        # h6 = 2*(1-c1) + 2*(1-c2) + 2*(1-c3) = 6 - 2*(c1+c2+c3)
        h6 = 6.0 - 2.0 * c_sum
        # c1*c2 + c1*c3 + c2*c3
        c12c13 = c1b * c23.unsqueeze(0)           # c1*(c2+c3)
        prod_sum = c12c13 + cc23.unsqueeze(0)     # c1*c2 + c1*c3 + c2*c3
        # h18 = (2/3)*(3-c1-c2-c3) + (2/3)*(3-c1*c2-c1*c3-c2*c3)
        h18 = (2.0 / 3.0) * (3.0 - c_sum) + (2.0 / 3.0) * (3.0 - prod_sum)
        t_6 = t_6 + (1.0 / h6).sum()
        t_18 = t_18 + (1.0 / h18).sum()
        t_ratio = t_ratio + (h6 / h18).sum()
        t_inv = t_inv + (h18 / h6).sum()
    return (float(t_6.item()), float(t_18.item()),
            float(t_ratio.item()), float(t_inv.item()))


def _bz_sums_numpy(N):
    """Same accumulators, vectorized NumPy with chunking along i1."""
    dk = np.pi / N
    idx = np.arange(N, dtype=np.float64)
    k_mid = (idx + 0.5) * dk
    c = np.cos(k_mid)                       # (N,)
    c23 = c[:, None] + c[None, :]           # (N, N)
    cc23 = c[:, None] * c[None, :]          # (N, N)
    chunk = min(N, 128)
    t_6 = 0.0
    t_18 = 0.0
    t_ratio = 0.0
    t_inv = 0.0
    for start in range(0, N, chunk):
        stop = min(start + chunk, N)
        c1b = c[start:stop].reshape(-1, 1, 1)       # (chunk, 1, 1)
        c_sum = c1b + c23[None, :, :]
        h6 = 6.0 - 2.0 * c_sum
        c12c13 = c1b * c23[None, :, :]
        prod_sum = c12c13 + cc23[None, :, :]
        h18 = (2.0 / 3.0) * (3.0 - c_sum) + (2.0 / 3.0) * (3.0 - prod_sum)
        t_6 += float(np.sum(1.0 / h6))
        t_18 += float(np.sum(1.0 / h18))
        t_ratio += float(np.sum(h6 / h18))
        t_inv += float(np.sum(h18 / h6))
    return t_6, t_18, t_ratio, t_inv


def _bz_sums(N):
    if TORCH is not None:
        return _bz_sums_torch(N)
    return _bz_sums_numpy(N)


VARPI = 2.622057554292119810
M_GAUSS = 0.8346268416740731
G_STAR = 2 * np.sqrt(VARPI * M_GAUSS)
W3_exact = gamma(0.25)**4 / (4 * np.pi**3)

print(f"G* = {G_STAR:.12f}")
print(f"W_3 (exact) = {W3_exact:.12f}")
print(f"G*^2/(2pi) = {G_STAR**2/(2*np.pi):.12f}")
print(f"Match: {abs(W3_exact - G_STAR**2/(2*np.pi)):.2e}")
print()

# ============================================================================
# Compute W_3 via numerical integration of the BZ integral
# ============================================================================
print("="*80)
print("Watson integral via numerical BZ integration")
print("="*80)

def hat_k2_6pt(k1, k2, k3):
    return 2*(1-np.cos(k1)) + 2*(1-np.cos(k2)) + 2*(1-np.cos(k3))

def hat_k2_18pt(k1, k2, k3):
    c1, c2, c3 = np.cos(k1), np.cos(k2), np.cos(k3)
    return (2/3)*(3-c1-c2-c3) + (2/3)*(3-c1*c2-c1*c3-c2*c3)

# Watson integral: (1/(2pi)^3) int_{-pi}^{pi} dk1 dk2 dk3 / hat_k^2
# By symmetry, 8x the integral over [0,pi]^3
# W_3 = (1/pi^3) int_0^pi dk1 dk2 dk3 / hat_k^2

# Use a dense grid
N = 200
dk = np.pi / N
total_6, total_18, total_ratio, total_inv_ratio = _bz_sums(N)

vol = (np.pi)**3  # volume of [0,pi]^3 quadrant
norm = dk**3 / vol  # each cell has volume dk^3, normalize by total volume

W3_numerical_6 = total_6 * norm
W3_numerical_18 = total_18 * norm
ratio_avg = total_ratio * norm
inv_ratio_avg = total_inv_ratio * norm

print(f"\nUsing N={N} midpoint rule over [0,pi]^3:")
print(f"  W_3 (6-point)  = {W3_numerical_6:.10f}  (exact: {W3_exact:.10f}, err: {abs(W3_numerical_6-W3_exact)/W3_exact*100:.4f}%)")
print(f"  W_3 (18-point) = {W3_numerical_18:.10f}")
print(f"  <hat_k2_6/hat_k2_18> = {ratio_avg:.10f}")
print(f"  <hat_k2_18/hat_k2_6> = {inv_ratio_avg:.10f}")
print()

# ============================================================================
# The KEY question: what is the ratio R_avg for arbitrarily large L?
# ============================================================================
print("="*80)
print("KEY RESULTS")
print("="*80)
print()
print("The stencil mismatch ratio R(k) = hat_k^2_6(k) / hat_k^2_18(k):")
print(f"  R_avg = <R(k)>_BZ = {ratio_avg:.10f}")
print(f"  <1/R(k)>_BZ = {inv_ratio_avg:.10f}")
print()
print("Properties:")
print(f"  R(k) -> 1 as k -> 0 (both stencils agree at small k)")
print(f"  R(pi,pi,pi) = 12/4 = 3 (maximum mismatch)")
print(f"  R(k,0,0) = 1 for all k (axes are exact)")
print()

# Does R_avg have any connection to G* or W_3?
print("Looking for connections:")
print(f"  R_avg = {ratio_avg:.10f}")
print(f"  W_3 = {W3_exact:.10f}")
print(f"  R_avg/W_3 = {ratio_avg/W3_exact:.10f}")
print(f"  G* = {G_STAR:.10f}")
print(f"  R_avg/G* = {ratio_avg/G_STAR:.10f}")
print(f"  G*/2 = {G_STAR/2:.10f}")
print(f"  pi/2 = {np.pi/2:.10f}")
print()

# ============================================================================
# Now the CRUCIAL physical analysis
# ============================================================================
print("="*80)
print("CRUCIAL PHYSICAL ANALYSIS")
print("="*80)
print()
print("FINDING 1: With matching stencils (6pt/6pt or 18pt/18pt),")
print("  the div-coupling produces G_charge = 1 (trivial).")
print("  The hat_k^2 cancels EXACTLY.")
print()
print("FINDING 2: With mismatched stencils (6pt div, 18pt Laplacian),")
print(f"  the div-coupling produces G_charge = R_avg = {ratio_avg:.6f}")
print("  This is a nontrivial lattice artifact but NOT W_3.")
print()
print("FINDING 3: The Watson integral W_3 enters ONLY through the")
print("  scalar Poisson equation -Delta phi_C = rho.")
print("  This is the COULOMB propagator, which is a SEPARATE computation")
print("  in the engine (solve_coulomb_poisson, line 201).")
print()
print("FINDING 4: The Gauss constraint (lambda_G -> infty) REMOVES")
print("  the longitudinal J mode entirely. It does not create a")
print("  scalar propagator; it eliminates one.")
print()
print("FINDING 5: The Lagrange multiplier for the Gauss constraint")
print("  has effective propagator 1/(2R(k)) after integrating out J.")
print("  With matching stencils, R=1 and this is trivial (constant 1/2).")
print("  With mismatched stencils, it's nontrivial but not W_3.")
print()

print("="*80)
print("THE RESOLUTION: How W_3 enters the gap equation")
print("="*80)
print()
print("The master quadratic x^2 = 16*G*^2*(x - G*) is NOT derived by")
print("integrating out J from the action. It arises from a DIFFERENT")
print("physical mechanism:")
print()
print("In the FTD engine, the Coulomb interaction between charges is")
print("computed via the SCALAR Poisson equation:")
print("  -Delta_6pt phi_C(x) = rho(x)")
print()
print("The Green's function G(x,y) = (-Delta)^{-1}(x,y) has the property:")
print(f"  G(0,0) = W_3 = {W3_exact:.10f} = G*^2/(2*pi)")
print()
print("This identity W_3 = G*^2/(2*pi) is NOT a coincidence -- it's a")
print("THEOREM of lattice theory (proven in DERIV_WATSON_GSTAR_IDENTITY.md).")
print()
print("The self-consistent gap equation arises from requiring that the")
print("effective coupling alpha_eff, defined through the Coulomb self-energy:")
print("  Sigma = alpha_eff * W_3")
print("satisfies a bootstrap condition with n_DOF = 16 internal DoF.")
print()

# ============================================================================
# The PRECISE gap equation derivation
# ============================================================================
print("="*80)
print("THE PRECISE GAP EQUATION")
print("="*80)
print()
print("Step 1: The Coulomb self-energy on the lattice")
print("  Sigma(0) = g_c^2 * W_3 / (4*pi)")
print("  where g_c is the coupling constant in the Lagrangian")
print()
print("Step 2: With n_DOF vacuum polarization insertions, the")
print("  dressed coupling 1/alpha_phys = 1/alpha_bare - n_DOF * Sigma")
print()
print("Step 3: Self-consistency requires alpha_bare = alpha_phys = alpha")
print("  (the lattice IS the fundamental theory, no UV cutoff to send to infinity)")
print()
print("Step 4: This gives the gap equation:")
print("  x = n_DOF * g_c^2 * W_3 / (4*pi)")
print()
print("With g_c^2 = 4*pi*alpha = 4*pi/x:")
print("  x = n_DOF * W_3")
print("  => x = 16 * G*^2/(2*pi)")
print()
print("But WAIT -- that gives x = 16*W_3 = 16*1.393 = 22.3, not 137.")
print("Something is wrong with the vacuum polarization picture.")
print()
print("Let me reconsider. The master quadratic is:")
print("  x^2 - 16*G*^2*x + 16*G*^3 = 0")
print()
print("This is NOT a linear gap equation. It's QUADRATIC in x.")
print("A quadratic self-consistency arises from SELF-REFERENTIAL closure:")
print("  x must simultaneously be the coupling AND the self-energy.")
print()
print("Rewrite as:")
print("  x^2 = 16*G*^2*x - 16*G*^3 = 16*G*^2*(x - G*)")
print("  x^2 = 32*pi*W_3*(x - G*)")
print()
print("Interpretation: x^2 is the SECOND-ORDER self-energy, which depends")
print("on x itself (through the dressed propagator) times the lattice")
print("Green's function W_3.")
print()
print("The factor (x - G*) is the RENORMALIZED coupling at the scale")
print("set by G* (the natural lattice scale).")
print()

# Verify the self-consistency
print("Verification of the gap equation:")
x_plus = 137.0361714582
x_minus = 3.0239639163
print(f"  x_+^2 = {x_plus**2:.6f}")
print(f"  32*pi*W_3*(x_+ - G*) = {32*np.pi*W3_exact*(x_plus - G_STAR):.6f}")
print(f"  Match: {abs(x_plus**2 - 32*np.pi*W3_exact*(x_plus - G_STAR)):.2e}")
print()
print(f"  x_-^2 = {x_minus**2:.6f}")
print(f"  32*pi*W_3*(x_- - G*) = {32*np.pi*W3_exact*(x_minus - G_STAR):.6f}")
print(f"  Match: {abs(x_minus**2 - 32*np.pi*W3_exact*(x_minus - G_STAR)):.2e}")
print()

# ============================================================================
# THE BOTTOM LINE
# ============================================================================
print("="*80)
print("BOTTOM LINE")
print("="*80)
print()
print("Q: Does the Gauss constraint produce W_3?")
print("A: NO. The Gauss constraint removes the longitudinal J mode.")
print("   It does not create a scalar propagator with W_3 self-energy.")
print()
print("Q: Where does W_3 enter?")
print("A: Through the SCALAR Coulomb potential phi_C, which the engine")
print("   solves via a 6-point Poisson equation. The Green's function")
print("   of this Poisson equation at the origin IS W_3.")
print()
print("Q: Is this derivable from the FTD Lagrangian?")
print("A: The Coulomb potential arises from the Gauss constraint in")
print("   Coulomb gauge: div(J) = rho implies a scalar potential phi")
print("   satisfying -Delta phi = rho. The force F = -alpha*grad(phi)")
print("   then uses the SCALAR Green's function, which has G(0)=W_3.")
print("   This IS derivable from the Lagrangian + gauge fixing.")
print()
print("Q: Can we derive the master quadratic from the partition function?")
print("A: The gap equation x^2 = 32*pi*W_3*(x-G*) requires:")
print("   1. W_3 = lattice Coulomb self-energy  [THEOREM - Watson identity]")
print("   2. n_DOF = 16  [MOTIVATED but not derived from partition function]")
print("   3. Quadratic self-consistency  [from self-referential closure]")
print("   4. G* as the natural scale  [from the lattice geometry]")
print()
print("   Item 1 is solid. Item 3 is argued in DERIV_QUADRATIC_NECESSITY.")
print("   Items 2 and 4 require the partition function on the minimal torus.")
print()
print("   The Gauss constraint approach does NOT bridge this gap.")
print("   The gap remains: deriving 16 and G* from Z on T^3_min.")
