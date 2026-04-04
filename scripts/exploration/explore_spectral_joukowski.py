#!/usr/bin/env python3
"""
Spectral Transformation Analysis: F[psi] -> F[|psi|^2] and the Joukowski Connection

[CONJECTURE] The Born rule squaring |psi|^2 induces a spectral transformation from
circular support to lemniscate-like support via self-convolution. This script
investigates whether the Joukowski map z -> z + 1/z provides the correct
geometric description.

Observation from simulation:
    F[psi]    : circular spectral support, 1270 components above noise
    F[|psi|^2]: lemniscate (figure-eight) support, 263 components
    DOF lost  : 1007 (79.3%)

Mathematical framework:
    If psi(x) has Fourier transform Psi(k), then:
        |psi(x)|^2 = psi(x) * psi*(x)
        F[|psi|^2](k) = (Psi * Psi*)(k)  [convolution theorem]

    For Psi supported on a circle |k| = k0, the autocorrelation Psi * Psi*
    maps through the geometry of the Joukowski transform.
"""

import sys
import os
import numpy as np
from scipy.signal import fftconvolve
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

# Import FTD constants
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (
    G_STAR, VARPI_CLASSICAL, PF, GAMMA_QUARTER, GAMMA_HALF,
    N_c, N_base, b_3, N_eff
)

# =============================================================================
# SECTION 1: Theoretical spectral self-convolution
# =============================================================================

def circular_spectral_field(N, k0, width=0.02):
    """
    Construct a 2D complex field whose Fourier transform has circular support
    at |k| = k0, with thin annular width and random phases.

    This models F[psi] = Psi(k) supported on a circle.
    """
    kx = np.fft.fftfreq(N, d=1.0/N)
    ky = np.fft.fftfreq(N, d=1.0/N)
    KX, KY = np.meshgrid(kx, ky)
    K_mag = np.sqrt(KX**2 + KY**2)

    # Annular support: thin ring at |k| = k0
    annulus = np.exp(-0.5 * ((K_mag - k0) / (width * k0))**2)

    # Random phases on the ring (complex Gaussian filtered to ring)
    rng = np.random.default_rng(seed=42)
    random_phase = rng.standard_normal((N, N)) + 1j * rng.standard_normal((N, N))

    Psi_k = annulus * random_phase
    psi_x = np.fft.ifft2(Psi_k)

    return psi_x, Psi_k


def compute_born_spectrum(psi_x):
    """
    Compute F[|psi|^2] = Psi * conj(Psi) via FFT of |psi|^2.
    """
    density = np.abs(psi_x)**2
    return np.fft.fft2(density)


def joukowski_map(z):
    """The Joukowski map: w = z + 1/z."""
    return z + 1.0 / z


def joukowski_unit_circle(n_points=1000):
    """
    Map the unit circle through z -> z + 1/z.
    For |z| = 1: z = e^{i*theta}, w = 2*cos(theta) -- collapses to [-2, 2].
    For |z| = r != 1: produces an ellipse.
    """
    theta = np.linspace(0, 2*np.pi, n_points, endpoint=False)
    z = np.exp(1j * theta)
    w = joukowski_map(z)
    return theta, z, w


def joukowski_offset_circle(r, n_points=1000):
    """Map a circle of radius r through the Joukowski transform."""
    theta = np.linspace(0, 2*np.pi, n_points, endpoint=False)
    z = r * np.exp(1j * theta)
    w = joukowski_map(z)
    return theta, z, w


# =============================================================================
# SECTION 2: Lemniscate of Bernoulli
# =============================================================================

def lemniscate_parametric(n_points=1000):
    """
    Lemniscate of Bernoulli: |z^2 - 1| = 1, equivalently (x^2+y^2)^2 = 2(x^2-y^2).
    Parametric form: r^2 = 2*cos(2*theta).
    """
    theta = np.linspace(-np.pi/4, np.pi/4, n_points)
    r2 = 2 * np.cos(2 * theta)
    r = np.sqrt(np.maximum(r2, 0))
    # Both lobes
    x_right = r * np.cos(theta)
    y_right = r * np.sin(theta)
    x_left = -r * np.cos(theta)
    y_left = -r * np.sin(theta)
    return x_right, y_right, x_left, y_left


# =============================================================================
# SECTION 3: Numerical self-convolution analysis
# =============================================================================

def analyze_spectral_support(spectrum_2d, threshold_fraction=0.01):
    """
    Count components above noise threshold and characterize support geometry.
    Returns count, and the (kx, ky) positions of above-threshold components.
    """
    power = np.abs(spectrum_2d)**2
    threshold = threshold_fraction * np.max(power)
    mask = power > threshold
    count = np.sum(mask)

    N = spectrum_2d.shape[0]
    kx = np.fft.fftfreq(N, d=1.0/N)
    ky = np.fft.fftfreq(N, d=1.0/N)
    KX, KY = np.meshgrid(kx, ky)

    return count, KX[mask], KY[mask], power


def measure_geometry(kx_pts, ky_pts):
    """
    Characterize spectral support geometry:
    - circularity: ratio of eigenvalues of inertia tensor
    - lemniscate score: correlation with figure-eight shape
    """
    if len(kx_pts) == 0:
        return {'circularity': 0, 'aspect': 0}

    # Inertia tensor
    cov = np.cov(kx_pts, ky_pts)
    eigvals = np.linalg.eigvalsh(cov)
    eigvals = np.sort(eigvals)[::-1]

    aspect = eigvals[1] / eigvals[0] if eigvals[0] > 0 else 0

    # Check for bimodality in angle distribution (lemniscate signature)
    angles = np.arctan2(ky_pts, kx_pts)
    radii = np.sqrt(kx_pts**2 + ky_pts**2)

    # For a lemniscate, r^2 ~ cos(2*theta), so radii should peak at theta=0, pi
    # and vanish at theta=pi/4, 3pi/4
    return {
        'circularity': aspect,
        'mean_radius': np.mean(radii),
        'std_radius': np.std(radii),
        'n_components': len(kx_pts),
    }


# =============================================================================
# SECTION 4: DOF ratio analysis
# =============================================================================

def analyze_dof_ratio():
    """
    Analyze the observed DOF ratio 263/1270.
    """
    N_born = 263
    N_psi = 1270
    N_lost = N_psi - N_born

    ratio_surviving = N_born / N_psi
    ratio_lost = N_lost / N_psi

    print("=" * 70)
    print("DOF RATIO ANALYSIS")
    print("=" * 70)
    print(f"  F[psi] components   : {N_psi}")
    print(f"  F[|psi|^2] components: {N_born}")
    print(f"  Lost                 : {N_lost}")
    print(f"  Surviving fraction   : {ratio_surviving:.6f}")
    print(f"  Lost fraction        : {ratio_lost:.6f}")
    print()

    # Framework constants to compare
    alpha = 1.0 / 137.036
    sin2_thetaW = 3.0 / N_eff   # = 3/13
    sigma_conf = 0.209           # string tension from area-law
    pi_over_4 = PF               # = pi/4, the packing fraction
    one_minus_PF = 1.0 - PF
    gstar_inv = 1.0 / G_STAR
    varpi_over_pi = VARPI_CLASSICAL / np.pi

    print("  Comparison with framework constants:")
    print(f"    alpha                = {alpha:.6f}")
    print(f"    sin^2(theta_W) = 3/13= {sin2_thetaW:.6f}")
    print(f"    sigma (confinement)  = {sigma_conf:.6f}")
    print(f"    pi/4 (PF)            = {pi_over_4:.6f}")
    print(f"    1 - pi/4             = {one_minus_PF:.6f}")
    print(f"    1/G*                 = {gstar_inv:.6f}")
    print(f"    varpi/pi             = {varpi_over_pi:.6f}")
    print()

    # Closest matches
    candidates = {
        'alpha': alpha,
        'sin^2(theta_W)': sin2_thetaW,
        'sigma': sigma_conf,
        'pi/4': pi_over_4,
        '1 - pi/4': one_minus_PF,
        '1/G*': gstar_inv,
        'varpi/pi': varpi_over_pi,
        '1/N_base': 1.0/N_base,
        '1/N_eff': 1.0/N_eff,
        '1/b_3': 1.0/b_3,
        'N_c/N_eff': N_c/N_eff,
        'N_base/N_eff': N_base/N_eff,
        '2/N_eff': 2.0/N_eff,
    }

    print("  Surviving fraction matches (263/1270 = {:.6f}):".format(ratio_surviving))
    for name, val in sorted(candidates.items(), key=lambda x: abs(x[1] - ratio_surviving)):
        delta = ratio_surviving - val
        pct = 100 * delta / val if val != 0 else float('inf')
        print(f"    {name:25s} = {val:.6f}  delta = {delta:+.6f}  ({pct:+.2f}%)")

    print()
    print("  Lost fraction matches (1007/1270 = {:.6f}):".format(ratio_lost))
    for name, val in sorted(candidates.items(), key=lambda x: abs(x[1] - ratio_lost)):
        delta = ratio_lost - val
        pct = 100 * delta / val if val != 0 else float('inf')
        print(f"    {name:25s} = {val:.6f}  delta = {delta:+.6f}  ({pct:+.2f}%)")

    return ratio_surviving, ratio_lost


# =============================================================================
# SECTION 5: Full numerical computation
# =============================================================================

def run_spectral_analysis(N=512, k0=40):
    """
    Full spectral analysis: construct circular-support field, compute
    Born rule spectrum, measure support geometry.
    """
    print("=" * 70)
    print("SPECTRAL SELF-CONVOLUTION ANALYSIS")
    print("=" * 70)
    print(f"  Grid size: {N}x{N}")
    print(f"  Spectral radius k0 = {k0}")
    print()

    # Step 1: Construct field with circular spectral support
    psi_x, Psi_k = circular_spectral_field(N, k0, width=0.02)

    # Step 2: Compute Born spectrum F[|psi|^2]
    Born_k = compute_born_spectrum(psi_x)

    # Step 3: Analyze spectral support
    threshold = 0.01
    n_psi, kx_psi, ky_psi, pow_psi = analyze_spectral_support(Psi_k, threshold)
    n_born, kx_born, ky_born, pow_born = analyze_spectral_support(Born_k, threshold)

    print(f"  F[psi] components above {threshold:.0%} threshold: {n_psi}")
    print(f"  F[|psi|^2] components above {threshold:.0%} threshold: {n_born}")
    if n_psi > 0:
        print(f"  Numerical DOF ratio: {n_born}/{n_psi} = {n_born/n_psi:.4f}")
    print()

    # Step 4: Geometry characterization
    geom_psi = measure_geometry(kx_psi, ky_psi)
    geom_born = measure_geometry(kx_born, ky_born)

    print("  F[psi] geometry:")
    for k, v in geom_psi.items():
        print(f"    {k}: {v:.4f}")
    print("  F[|psi|^2] geometry:")
    for k, v in geom_born.items():
        print(f"    {k}: {v:.4f}")

    return psi_x, Psi_k, Born_k, pow_psi, pow_born


def verify_joukowski_connection():
    """
    Verify the Joukowski map connection analytically.

    Key insight: For a field with circular spectral support at |k| = k0,
    the autocorrelation integral Psi * Psi*(q) = integral Psi(k) Psi*(k-q) dk
    is nonzero only where two copies of the circle overlap.

    The overlap region of circle |k|=k0 and circle |k-q|=k0 exists iff |q| <= 2*k0.
    This gives a DISK of radius 2*k0.

    However, with coherent phase structure (not random), the cancellation pattern
    produces figure-eight (lemniscate) nodes. The Joukowski map z -> z + 1/z
    maps the unit circle to [-2,2] (a degenerate lemniscate), while circles
    of radius r -> ellipses. The PHASE of the convolution traces lemniscate curves.
    """
    print("=" * 70)
    print("JOUKOWSKI MAP VERIFICATION")
    print("=" * 70)

    # Joukowski on unit circle: collapses to [-2, 2]
    theta, z_circ, w_jouk = joukowski_unit_circle(1000)

    print(f"  Unit circle |z|=1:")
    print(f"    w = z + 1/z range: [{np.min(w_jouk.real):.4f}, {np.max(w_jouk.real):.4f}]")
    print(f"    Imaginary part max: {np.max(np.abs(w_jouk.imag)):.2e} (should be ~0)")
    print()

    # Joukowski on r = 1 + epsilon: thin ellipse (lemniscate-like for small eps)
    for eps in [0.01, 0.05, 0.1, 0.2]:
        r = 1.0 + eps
        _, _, w = joukowski_offset_circle(r)
        a_semi = np.max(w.real)  # semi-major axis
        b_semi = np.max(w.imag)  # semi-minor axis
        aspect = b_semi / a_semi if a_semi > 0 else 0
        print(f"  |z|={r:.2f}: semi-major={a_semi:.4f}, semi-minor={b_semi:.4f}, aspect={aspect:.4f}")

    print()

    # Connection to lemniscate: the Joukowski image of two circles
    # tangent at origin traces a lemniscate of Bernoulli
    print("  Lemniscate of Bernoulli: (x^2+y^2)^2 = 2(x^2 - y^2)")
    x_r, y_r, x_l, y_l = lemniscate_parametric(500)
    area_right = np.abs(np.trapezoid(y_r, x_r))
    area_left = np.abs(np.trapezoid(y_l, x_l))
    total_area = area_right + area_left
    # Exact area of lemniscate with a^2=2: A = a^2 = 2
    print(f"  Numerical area: {total_area:.4f} (exact: 2.0000)")
    print()

    # The key ratio: lemniscate area / enclosing circle area
    # Enclosing circle has radius sqrt(2), area = 2*pi
    enclosing_area = 2 * np.pi
    area_ratio = total_area / enclosing_area
    print(f"  Lemniscate/circle area ratio: {area_ratio:.6f}")
    print(f"  1/pi = {1/np.pi:.6f}")
    print(f"  Ratio matches 1/pi: {abs(area_ratio - 1/np.pi) < 0.01}")
    print()

    # The spectral DOF connection:
    # If circular support has N components, and lemniscate support has
    # N * (area_lemniscate / area_circle) components, then the ratio is 1/pi
    predicted_ratio = 1.0 / np.pi
    observed_ratio = 263.0 / 1270.0
    print(f"  Predicted surviving fraction (1/pi): {predicted_ratio:.6f}")
    print(f"  Observed surviving fraction:          {observed_ratio:.6f}")
    print(f"  Discrepancy: {abs(predicted_ratio - observed_ratio)/observed_ratio * 100:.2f}%")
    print()

    # Alternative: sigma_confinement from area-law
    sigma = 0.209
    print(f"  sigma (confinement string tension):   {sigma:.6f}")
    print(f"  Discrepancy from sigma: {abs(sigma - observed_ratio)/observed_ratio * 100:.2f}%")

    return area_ratio


# =============================================================================
# SECTION 6: Visualization
# =============================================================================

def generate_figure(Psi_k, Born_k, save_path=None):
    """
    Three-panel figure:
    (a) Circular support of F[psi]
    (b) Born rule support of F[|psi|^2]
    (c) Joukowski map and lemniscate overlay
    """
    fig = plt.figure(figsize=(16, 5.5))
    gs = GridSpec(1, 3, figure=fig, wspace=0.35)

    N = Psi_k.shape[0]

    # Shift for display
    pow_psi = np.fft.fftshift(np.abs(Psi_k)**2)
    pow_born = np.fft.fftshift(np.abs(Born_k)**2)

    extent_half = N // 2
    extent = [-extent_half, extent_half, -extent_half, extent_half]

    # Panel (a): F[psi] circular support
    ax1 = fig.add_subplot(gs[0])
    # Log scale for visibility
    im1 = ax1.imshow(
        np.log10(pow_psi / np.max(pow_psi) + 1e-10),
        extent=extent, origin='lower', cmap='inferno',
        vmin=-3, vmax=0, aspect='equal'
    )
    ax1.set_title(r'(a) $|\hat{\psi}(\mathbf{k})|^2$ — Circular support', fontsize=12)
    ax1.set_xlabel(r'$k_x$')
    ax1.set_ylabel(r'$k_y$')
    ax1.set_xlim(-80, 80)
    ax1.set_ylim(-80, 80)
    plt.colorbar(im1, ax=ax1, label=r'$\log_{10}$ (norm. power)', shrink=0.8)

    # Panel (b): F[|psi|^2] support
    ax2 = fig.add_subplot(gs[1])
    im2 = ax2.imshow(
        np.log10(pow_born / np.max(pow_born) + 1e-10),
        extent=extent, origin='lower', cmap='inferno',
        vmin=-3, vmax=0, aspect='equal'
    )
    ax2.set_title(r'(b) $|\widehat{|\psi|^2}(\mathbf{k})|^2$ — Born support', fontsize=12)
    ax2.set_xlabel(r'$k_x$')
    ax2.set_ylabel(r'$k_y$')
    ax2.set_xlim(-80, 80)
    ax2.set_ylim(-80, 80)
    plt.colorbar(im2, ax=ax2, label=r'$\log_{10}$ (norm. power)', shrink=0.8)

    # Panel (c): Joukowski map + lemniscate
    ax3 = fig.add_subplot(gs[2])

    # Unit circle
    theta_c = np.linspace(0, 2*np.pi, 500)
    ax3.plot(np.cos(theta_c), np.sin(theta_c), 'b-', alpha=0.4, linewidth=1, label='Unit circle')

    # Joukowski images of circles at various radii
    for r, alpha_val, ls in [(1.05, 0.6, '-'), (1.1, 0.5, '-'), (1.2, 0.4, '--'), (1.5, 0.3, '--')]:
        _, _, w = joukowski_offset_circle(r, 500)
        ax3.plot(w.real, w.imag, color='steelblue', alpha=alpha_val,
                 linewidth=1.2, linestyle=ls,
                 label=f'Joukowski(|z|={r:.2f})' if r == 1.05 else None)

    # Joukowski of unit circle -> [-2, 2]
    _, _, w_unit = joukowski_unit_circle(500)
    ax3.plot(w_unit.real, w_unit.imag, 'r-', linewidth=2.5, label=r'Joukowski($|z|=1$)$\to[-2,2]$')

    # Lemniscate of Bernoulli
    x_r, y_r, x_l, y_l = lemniscate_parametric(500)
    scale = 1.0 / np.sqrt(2)  # normalize to fit with Joukowski scale
    ax3.plot(x_r * scale, y_r * scale, 'g-', linewidth=2, label='Lemniscate')
    ax3.plot(x_l * scale, y_l * scale, 'g-', linewidth=2)

    ax3.set_title(r'(c) Joukowski map $z \mapsto z + 1/z$', fontsize=12)
    ax3.set_xlabel(r'Re($w$)')
    ax3.set_ylabel(r'Im($w$)')
    ax3.set_xlim(-3, 3)
    ax3.set_ylim(-1.5, 1.5)
    ax3.set_aspect('equal')
    ax3.legend(fontsize=8, loc='upper right')
    ax3.grid(True, alpha=0.3)

    plt.suptitle(
        r'Spectral Transformation $\mathcal{F}[\psi] \to \mathcal{F}[|\psi|^2]$: '
        'Joukowski Connection',
        fontsize=14, y=1.02
    )

    plt.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=150, bbox_inches='tight')
        print(f"\n  Figure saved to: {save_path}")

    plt.close(fig)


# =============================================================================
# MAIN
# =============================================================================

def main():
    print()
    print("#" * 70)
    print("#  SPECTRAL JOUKOWSKI ANALYSIS")
    print("#  F[psi] -> F[|psi|^2]: Circle -> Lemniscate via self-convolution")
    print("#" * 70)
    print()

    # Part 1: DOF ratio analysis
    ratio_surv, ratio_lost = analyze_dof_ratio()
    print()

    # Part 2: Joukowski verification
    area_ratio = verify_joukowski_connection()
    print()

    # Part 3: Full numerical simulation
    psi_x, Psi_k, Born_k, pow_psi, pow_born = run_spectral_analysis(N=512, k0=40)
    print()

    # Part 4: Generate figure
    script_dir = os.path.dirname(os.path.abspath(__file__))
    fig_path = os.path.join(script_dir, 'spectral_joukowski.png')
    generate_figure(Psi_k, Born_k, save_path=fig_path)

    # Part 5: Summary
    print()
    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)
    print()
    print("  Mathematical result:")
    print("    The autocorrelation Psi * Psi*(q) of a circular-support spectrum")
    print("    produces a DISK of radius 2*k0 (not a circle or lemniscate).")
    print("    With random phases, the full disk is populated.")
    print("    With COHERENT phases, destructive interference carves the disk")
    print("    into structured support whose geometry depends on the phase pattern.")
    print()
    print("  Joukowski connection:")
    print("    The Joukowski map z -> z + 1/z maps circles to ellipses and")
    print("    the unit circle to the degenerate interval [-2,2].")
    print("    For the Born rule convolution, the phase-coherent case produces")
    print("    support whose shape interpolates between the full disk (random phase)")
    print("    and the degenerate line (fully coherent), passing through")
    print("    lemniscate-like figures at intermediate coherence.")
    print()

    observed_ratio = 263.0 / 1270.0
    pi_inv = 1.0 / np.pi
    sigma = 0.209
    sin2tw = 3.0 / 13.0

    print("  DOF ratio connections:")
    print(f"    Observed: 263/1270 = {observed_ratio:.6f}")
    print(f"    1/pi             = {pi_inv:.6f}  (off by {abs(pi_inv - observed_ratio)/observed_ratio*100:.1f}%)")
    print(f"    sigma (confine.) = {sigma:.6f}  (off by {abs(sigma - observed_ratio)/observed_ratio*100:.1f}%)")
    print(f"    sin^2(theta_W)   = {sin2tw:.6f}  (off by {abs(sin2tw - observed_ratio)/observed_ratio*100:.1f}%)")
    print()

    lost_ratio = 1007.0 / 1270.0
    pi_4 = np.pi / 4
    print(f"    Lost: 1007/1270  = {lost_ratio:.6f}")
    print(f"    pi/4             = {pi_4:.6f}  (off by {abs(pi_4 - lost_ratio)/lost_ratio*100:.1f}%)")
    print()

    # Epistemic assessment
    print("  [CONJECTURE] Epistemic status:")
    print("    - The circle-to-disk convolution theorem is RIGOROUS (standard Fourier analysis)")
    print("    - The lemniscate appearance requires COHERENT phase structure — not generic")
    print("    - The DOF ratio 263/1270 is closest to sigma (confinement) at 0.9% discrepancy")
    print("    - The lost fraction 1007/1270 is closest to pi/4 at 0.9% discrepancy")
    print("    - These may be coincidences from discrete grid effects (N=512) or threshold choice")
    print("    - The Joukowski map provides geometric intuition but is NOT a derivation")
    print("    - Status: suggestive pattern requiring independent confirmation")


if __name__ == '__main__':
    main()
