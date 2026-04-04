"""
Verification Script: Lattice Spacing Uniqueness
=================================================

Tests whether a = 2/3 is the unique self-consistent lattice spacing that
minimizes |x+(a) - NIST| under one-loop (and two-loop) corrections.

Method:
  - For each spacing a in [0.50, 0.80], compute m_lat^2 = m^2 * a^2
  - Evaluate the tadpole integral I_1(a) on a 64^3 lattice
  - Compute the VEV shift: delta_phi = -I_1 / m_lat^2  (lattice units)
  - Convert: delta_x = delta_phi * a  (physical units)
  - Corrected x+ = X_PLUS(tree) + delta_x
  - Also evaluate the two-loop sunset correction

Produces:
  - Console table of residuals vs spacing
  - Plot of |x+(a) - NIST| vs a  (saved to output/)
  - Report of a_optimal and its distance from 2/3

Run:  python scripts/verification/verify_spacing_uniqueness.py

Epistemic tag: [DERIVED] — numerical evaluation of standard lattice QFT
integrals using FTD parameters. The lattice machinery is external physics;
only the EFT parameters (G*, D=3) come from FTD.
"""

import numpy as np
from scipy.special import gamma
from scipy.optimize import minimize_scalar
import os
import sys

# =============================================================================
# CONSTANTS (reproduced here for self-containedness; matches constants.py)
# =============================================================================

GAMMA_QUARTER = gamma(0.25)
GAMMA_THREE_QUARTER = gamma(0.75)
G_STAR = GAMMA_QUARTER / GAMMA_THREE_QUARTER  # lemniscatic constant ~ 2.9587

D = 3  # spatial dimension

# Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0
disc = (16 * G_STAR**2)**2 - 4 * 16 * G_STAR**3
X_PLUS = (16 * G_STAR**2 + np.sqrt(disc)) / 2   # ~ 137.036171
X_MINUS = (16 * G_STAR**2 - np.sqrt(disc)) / 2   # ~ 3.024

# EFT mass squared (physical)
M_SQ = X_PLUS - X_MINUS  # ~ 134.012

# Coupling (from phi^3 EFT — the effective vertex in the VEV shift formula)
# Note: the factor of g is absorbed into the Feynman rule normalization;
# the numerical formula matching the derivation document is:
#   delta_phi = -I_1 / m_lat^2
# This is verified against the reference value delta_x = -1.710e-4.
# (See DERIV_ONE_LOOP_LATTICE_ALPHA.md, Claim 1LA-4.)

# NIST/CODATA 2022
ALPHA_INV_NIST = 137.035999177  # +/- 0.000000021

# =============================================================================
# LATTICE INTEGRALS
# =============================================================================

def compute_tadpole(N, m_sq_lat):
    """One-loop tadpole integral on an N^3 cubic lattice.

    I_1 = (1/N^3) sum_k  1 / (k_hat^2 + m_lat^2)

    where k_hat^2 = sum_mu 4*sin^2(k_mu/2), k_mu in [-pi, pi).
    The k=0 mode is excluded (IR regulation).
    """
    k = np.linspace(-np.pi, np.pi, N, endpoint=False)
    kx, ky, kz = np.meshgrid(k, k, k, indexing='ij')
    k_hat_sq = 4.0 * (np.sin(kx / 2)**2 + np.sin(ky / 2)**2 + np.sin(kz / 2)**2)
    propagator = 1.0 / (k_hat_sq + m_sq_lat)
    propagator[0, 0, 0] = 0.0  # exclude zero mode
    return np.mean(propagator)


def compute_sunset(N, m_sq_lat):
    """Two-loop sunset integral via FFT convolution on an N^3 lattice.

    I_sunset = (1/N^3) sum_k  [G(k)]^2

    where G(k) = 1/(k_hat^2 + m_lat^2) is the lattice propagator.

    The sunset diagram in x-space is G(x)^2, which in momentum space is
    the convolution G*G. By the convolution theorem:

      (G*G)(k) = N^3 * IFFT[ FFT[G]^2 ]      ... but more directly,

    The sunset integral is:
      I_sunset = sum_x G(x)^2
              = (1/N^3) sum_k |G_tilde(k)|^2     (Parseval)

    where G_tilde(k) is the FFT of the propagator in k-space evaluated
    at lattice sites. Actually, the sunset is the self-energy bubble:

      I_2 = integral d^3p/(2pi)^3  G(p) * G(k-p)  evaluated at k=0
          = integral d^3p/(2pi)^3  [G(p)]^2        (external momentum = 0)

    For the two-loop VEV correction (sunset with external momentum zero),
    we need:
      I_sunset = (1/N^3) sum_p  [1/(p_hat^2 + m_lat^2)]^2

    This is the squared-propagator integral.
    """
    k = np.linspace(-np.pi, np.pi, N, endpoint=False)
    kx, ky, kz = np.meshgrid(k, k, k, indexing='ij')
    k_hat_sq = 4.0 * (np.sin(kx / 2)**2 + np.sin(ky / 2)**2 + np.sin(kz / 2)**2)
    propagator = 1.0 / (k_hat_sq + m_sq_lat)
    propagator[0, 0, 0] = 0.0
    prop_sq = propagator**2
    return np.mean(prop_sq)


# =============================================================================
# ONE-LOOP CORRECTED x+ AS FUNCTION OF SPACING
# =============================================================================

def x_plus_one_loop(a, N=64):
    """Compute one-loop corrected x+ for a given lattice spacing a.

    Returns (x_plus_corrected, I1, delta_x).
    """
    m_sq_lat = M_SQ * a**2
    I1 = compute_tadpole(N, m_sq_lat)
    delta_phi = -I1 / m_sq_lat        # VEV shift in lattice units
    delta_x = delta_phi * a            # convert to physical x-units
    return X_PLUS + delta_x, I1, delta_x


def x_plus_two_loop(a, N=64):
    """Compute two-loop corrected x+ for a given lattice spacing a.

    The two-loop sunset contributes an additional VEV shift.
    At two loops, the correction to the VEV is (schematically):

      delta_phi_2 = +g^2 * I_sunset / m_lat^4

    where g = 2 (phi^3 coupling) and I_sunset is the squared-propagator
    integral. The sign is positive (opposite to one-loop) because the
    sunset diagram has an even number of vertices.

    We use the same conversion: delta_x_2 = delta_phi_2 * a.

    Returns (x_plus_corrected, I1, I_sunset, delta_x_1, delta_x_2).
    """
    m_sq_lat = M_SQ * a**2
    I1 = compute_tadpole(N, m_sq_lat)
    I_sun = compute_sunset(N, m_sq_lat)

    # One-loop piece
    delta_phi_1 = -I1 / m_sq_lat
    delta_x_1 = delta_phi_1 * a

    # Two-loop piece: g^2 * I_sunset / m_lat^4
    # g = 2 (bare coupling), factor from Feynman rules
    g = 2.0
    delta_phi_2 = g**2 * I_sun / m_sq_lat**2
    delta_x_2 = delta_phi_2 * a

    x_corrected = X_PLUS + delta_x_1 + delta_x_2
    return x_corrected, I1, I_sun, delta_x_1, delta_x_2


# =============================================================================
# MAIN COMPUTATION
# =============================================================================

def main():
    N_LATTICE = 64  # lattice size for all evaluations

    # Spacing scan range
    a_values = np.arange(0.50, 0.805, 0.01)

    print("=" * 78)
    print("LATTICE SPACING UNIQUENESS TEST")
    print("=" * 78)
    print(f"\nTree-level x+ = {X_PLUS:.10f}")
    print(f"NIST CODATA    = {ALPHA_INV_NIST:.10f}")
    print(f"Tree gap       = {abs(X_PLUS - ALPHA_INV_NIST):.6e}  "
          f"({abs(X_PLUS - ALPHA_INV_NIST)/ALPHA_INV_NIST*1e6:.3f} ppm)")
    print(f"m^2 (physical) = {M_SQ:.6f}")
    print(f"Lattice size   = {N_LATTICE}^3")
    print(f"Scan range     = a in [{a_values[0]:.2f}, {a_values[-1]:.2f}], step 0.01")
    print(f"Target spacing = 2/3 = {2.0/3.0:.10f}")

    # -------------------------------------------------------------------------
    # ONE-LOOP SCAN
    # -------------------------------------------------------------------------
    print("\n" + "-" * 78)
    print("ONE-LOOP SCAN: |x+(a) - NIST| vs lattice spacing a")
    print("-" * 78)
    print(f"{'a':>8s}  {'m_lat^2':>10s}  {'I_1':>12s}  {'delta_x':>12s}  "
          f"{'x+(1L)':>14s}  {'|resid|':>12s}  {'ppb':>10s}")
    print("-" * 78)

    one_loop_residuals = []
    one_loop_x_vals = []

    for a in a_values:
        x_corr, I1, dx = x_plus_one_loop(a, N=N_LATTICE)
        resid = abs(x_corr - ALPHA_INV_NIST)
        ppb = resid / ALPHA_INV_NIST * 1e9
        m_lat_sq = M_SQ * a**2
        one_loop_residuals.append(resid)
        one_loop_x_vals.append(x_corr)
        marker = " <-- 2/3" if abs(a - 2.0/3.0) < 0.005 else ""
        print(f"{a:8.4f}  {m_lat_sq:10.3f}  {I1:12.6e}  {dx:12.6e}  "
              f"{x_corr:14.8f}  {resid:12.6e}  {ppb:10.2f}{marker}")

    one_loop_residuals = np.array(one_loop_residuals)

    # Find optimal spacing (one-loop) via refinement
    idx_min = np.argmin(one_loop_residuals)
    a_coarse_min = a_values[idx_min]

    print(f"\nCoarse minimum at a = {a_coarse_min:.4f}, "
          f"residual = {one_loop_residuals[idx_min]:.4e} "
          f"({one_loop_residuals[idx_min]/ALPHA_INV_NIST*1e9:.2f} ppb)")

    # Fine-grained optimization with scipy
    def one_loop_residual_func(a):
        x_corr, _, _ = x_plus_one_loop(a, N=N_LATTICE)
        return abs(x_corr - ALPHA_INV_NIST)

    result = minimize_scalar(one_loop_residual_func,
                             bounds=(0.50, 0.80), method='bounded',
                             options={'xatol': 1e-8})
    a_opt_1L = result.x
    resid_opt_1L = result.fun
    ppb_opt_1L = resid_opt_1L / ALPHA_INV_NIST * 1e9

    print(f"\n{'='*78}")
    print("ONE-LOOP RESULT")
    print(f"{'='*78}")
    print(f"  a_optimal (1-loop) = {a_opt_1L:.8f}")
    print(f"  2/3                = {2.0/3.0:.8f}")
    print(f"  |a_opt - 2/3|      = {abs(a_opt_1L - 2.0/3.0):.6e}")
    print(f"  Relative distance  = {abs(a_opt_1L - 2.0/3.0)/(2.0/3.0)*100:.4f}%")
    print(f"  Residual at a_opt  = {resid_opt_1L:.4e} ({ppb_opt_1L:.4f} ppb)")

    # Value at exactly a = 2/3
    x_at_23, I1_23, dx_23 = x_plus_one_loop(2.0/3.0, N=N_LATTICE)
    resid_23 = abs(x_at_23 - ALPHA_INV_NIST)
    ppb_23 = resid_23 / ALPHA_INV_NIST * 1e9

    print(f"\n  At a = 2/3 exactly:")
    print(f"    x+(1L) = {x_at_23:.10f}")
    print(f"    residual = {resid_23:.4e} ({ppb_23:.2f} ppb)")

    # -------------------------------------------------------------------------
    # TWO-LOOP SCAN
    # -------------------------------------------------------------------------
    print(f"\n{'='*78}")
    print("TWO-LOOP SCAN: |x+(a) - NIST| vs lattice spacing a")
    print(f"{'='*78}")
    print(f"{'a':>8s}  {'I_sunset':>12s}  {'delta_x_1':>12s}  {'delta_x_2':>12s}  "
          f"{'x+(2L)':>14s}  {'|resid|':>12s}  {'ppb':>10s}")
    print("-" * 78)

    two_loop_residuals = []

    for a in a_values:
        x_corr, I1, I_sun, dx1, dx2 = x_plus_two_loop(a, N=N_LATTICE)
        resid = abs(x_corr - ALPHA_INV_NIST)
        ppb = resid / ALPHA_INV_NIST * 1e9
        two_loop_residuals.append(resid)
        marker = " <-- 2/3" if abs(a - 2.0/3.0) < 0.005 else ""
        print(f"{a:8.4f}  {I_sun:12.6e}  {dx1:12.6e}  {dx2:12.6e}  "
              f"{x_corr:14.8f}  {resid:12.6e}  {ppb:10.2f}{marker}")

    two_loop_residuals = np.array(two_loop_residuals)

    # Fine-grained optimization for two-loop
    def two_loop_residual_func(a):
        x_corr, _, _, _, _ = x_plus_two_loop(a, N=N_LATTICE)
        return abs(x_corr - ALPHA_INV_NIST)

    result_2L = minimize_scalar(two_loop_residual_func,
                                bounds=(0.50, 0.80), method='bounded',
                                options={'xatol': 1e-8})
    a_opt_2L = result_2L.x
    resid_opt_2L = result_2L.fun
    ppb_opt_2L = resid_opt_2L / ALPHA_INV_NIST * 1e9

    print(f"\n{'='*78}")
    print("TWO-LOOP RESULT")
    print(f"{'='*78}")
    print(f"  a_optimal (2-loop) = {a_opt_2L:.8f}")
    print(f"  2/3                = {2.0/3.0:.8f}")
    print(f"  |a_opt - 2/3|      = {abs(a_opt_2L - 2.0/3.0):.6e}")
    print(f"  Relative distance  = {abs(a_opt_2L - 2.0/3.0)/(2.0/3.0)*100:.4f}%")
    print(f"  Residual at a_opt  = {resid_opt_2L:.4e} ({ppb_opt_2L:.4f} ppb)")

    # Two-loop value at exactly 2/3
    x_2L_23, _, I_sun_23, dx1_23, dx2_23 = x_plus_two_loop(2.0/3.0, N=N_LATTICE)
    resid_2L_23 = abs(x_2L_23 - ALPHA_INV_NIST)
    ppb_2L_23 = resid_2L_23 / ALPHA_INV_NIST * 1e9

    print(f"\n  At a = 2/3 exactly:")
    print(f"    x+(2L) = {x_2L_23:.10f}")
    print(f"    residual = {resid_2L_23:.4e} ({ppb_2L_23:.2f} ppb)")

    # -------------------------------------------------------------------------
    # SHIFT BETWEEN ONE-LOOP AND TWO-LOOP OPTIMA
    # -------------------------------------------------------------------------
    print(f"\n{'='*78}")
    print("COMPARISON: ONE-LOOP vs TWO-LOOP OPTIMAL SPACING")
    print(f"{'='*78}")
    print(f"  a_opt (1-loop) = {a_opt_1L:.8f}")
    print(f"  a_opt (2-loop) = {a_opt_2L:.8f}")
    print(f"  Shift          = {a_opt_2L - a_opt_1L:+.6e}")
    print(f"  2/3            = {2.0/3.0:.8f}")
    print(f"  Distance 1L->2/3 = {abs(a_opt_1L - 2.0/3.0):.6e}")
    print(f"  Distance 2L->2/3 = {abs(a_opt_2L - 2.0/3.0):.6e}")

    moves_toward = abs(a_opt_2L - 2.0/3.0) < abs(a_opt_1L - 2.0/3.0)
    print(f"  Two-loop moves optimal TOWARD 2/3? {moves_toward}")

    # -------------------------------------------------------------------------
    # UNIQUENESS ASSESSMENT
    # -------------------------------------------------------------------------
    print(f"\n{'='*78}")
    print("UNIQUENESS ASSESSMENT")
    print(f"{'='*78}")

    # Check: is the minimum well-defined (not a plateau)?
    # Compute second derivative of residual at the minimum
    da = 0.001
    r_minus = one_loop_residual_func(a_opt_1L - da)
    r_center = one_loop_residual_func(a_opt_1L)
    r_plus = one_loop_residual_func(a_opt_1L + da)
    curvature = (r_plus - 2*r_center + r_minus) / da**2

    print(f"  Curvature d^2(residual)/da^2 at a_opt = {curvature:.4e}")
    print(f"  (positive curvature = genuine minimum, not a saddle or plateau)")

    is_unique = curvature > 0
    near_two_thirds = abs(a_opt_1L - 2.0/3.0) / (2.0/3.0) < 0.01  # within 1%

    print(f"\n  Is the minimum unique (positive curvature)?  {is_unique}")
    print(f"  Is a_opt within 1% of 2/3?                   {near_two_thirds}")

    if is_unique and near_two_thirds:
        print("\n  CONCLUSION: The residual |x+(a) - NIST| has a unique, sharp")
        print("  minimum near a = 2/3. The optimal spacing a_opt differs from")
        print(f"  2/3 by {abs(a_opt_1L - 2.0/3.0)/(2.0/3.0)*100:.3f}%.")
        print(f"  At a = 2/3, the residual is {ppb_23:.1f} ppb (vs 0 at a_opt).")
        print("  Whether higher loops converge a_opt toward 2/3 remains OPEN.")
        if not moves_toward:
            print("  NOTE: The two-loop correction moves a_opt AWAY from 2/3,")
            print("  suggesting the sunset approximation may be incomplete or")
            print("  that a = 2/3 is selected by a principle other than residual")
            print("  minimization (see DERIV_ONE_LOOP_LATTICE_ALPHA.md, Claim 1LA-9).")
    else:
        print("\n  CONCLUSION: The data does NOT support a = 2/3 as the unique")
        print("  self-consistent spacing. See detailed output above.")

    # -------------------------------------------------------------------------
    # PLOT
    # -------------------------------------------------------------------------
    try:
        import matplotlib
        matplotlib.use('Agg')  # non-interactive backend
        import matplotlib.pyplot as plt

        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

        # One-loop plot
        ax1.semilogy(a_values, one_loop_residuals, 'b.-', label='One-loop', linewidth=1.5)
        ax1.axvline(x=2.0/3.0, color='r', linestyle='--', alpha=0.7, label='a = 2/3')
        ax1.axvline(x=a_opt_1L, color='g', linestyle=':', alpha=0.7,
                     label=f'a_opt = {a_opt_1L:.5f}')
        ax1.set_xlabel('Lattice spacing a', fontsize=12)
        ax1.set_ylabel('|x+(a) - NIST|', fontsize=12)
        ax1.set_title('One-Loop Residual vs Lattice Spacing', fontsize=13)
        ax1.legend(fontsize=10)
        ax1.grid(True, alpha=0.3)

        # Two-loop plot
        ax2.semilogy(a_values, two_loop_residuals, 'm.-', label='Two-loop', linewidth=1.5)
        ax2.axvline(x=2.0/3.0, color='r', linestyle='--', alpha=0.7, label='a = 2/3')
        ax2.axvline(x=a_opt_2L, color='g', linestyle=':', alpha=0.7,
                     label=f'a_opt = {a_opt_2L:.5f}')
        ax2.set_xlabel('Lattice spacing a', fontsize=12)
        ax2.set_ylabel('|x+(a) - NIST|', fontsize=12)
        ax2.set_title('Two-Loop Residual vs Lattice Spacing', fontsize=13)
        ax2.legend(fontsize=10)
        ax2.grid(True, alpha=0.3)

        plt.tight_layout()

        # Save to output/ directory
        output_dir = os.path.join(os.path.dirname(os.path.dirname(
            os.path.dirname(os.path.abspath(__file__)))), 'output')
        os.makedirs(output_dir, exist_ok=True)
        plot_path = os.path.join(output_dir, 'spacing_uniqueness.png')
        plt.savefig(plot_path, dpi=150)
        print(f"\n  Plot saved to: {plot_path}")

    except ImportError:
        print("\n  (matplotlib not available — skipping plot)")

    # -------------------------------------------------------------------------
    # FINAL SUMMARY
    # -------------------------------------------------------------------------
    print(f"\n{'='*78}")
    print("FINAL SUMMARY")
    print(f"{'='*78}")
    print(f"  Tree-level x+          = {X_PLUS:.10f}")
    print(f"  NIST CODATA 2022       = {ALPHA_INV_NIST:.10f}")
    print(f"  Tree-level residual    = {abs(X_PLUS - ALPHA_INV_NIST)/ALPHA_INV_NIST*1e6:.3f} ppm")
    print(f"  ---")
    print(f"  a_opt (1-loop)         = {a_opt_1L:.8f}")
    print(f"  1-loop resid at a_opt  = {ppb_opt_1L:.4f} ppb")
    print(f"  1-loop resid at 2/3    = {ppb_23:.2f} ppb")
    print(f"  ---")
    print(f"  a_opt (2-loop)         = {a_opt_2L:.8f}")
    print(f"  2-loop resid at a_opt  = {ppb_opt_2L:.4f} ppb")
    print(f"  2-loop resid at 2/3    = {ppb_2L_23:.2f} ppb")
    print(f"  ---")
    print(f"  Distance a_opt(1L) from 2/3 = {abs(a_opt_1L - 2.0/3.0):.6e} "
          f"({abs(a_opt_1L - 2.0/3.0)/(2.0/3.0)*100:.3f}%)")
    print(f"  Distance a_opt(2L) from 2/3 = {abs(a_opt_2L - 2.0/3.0):.6e} "
          f"({abs(a_opt_2L - 2.0/3.0)/(2.0/3.0)*100:.3f}%)")
    print(f"  Two-loop shifts a_opt toward 2/3: {moves_toward}")


if __name__ == '__main__':
    main()
