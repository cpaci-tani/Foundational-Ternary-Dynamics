"""
Julia Set Structure in TRD Complexified Flux Space

This module investigates whether the TRD iteration map, when restricted to
the complexified flux psi = J_x + i*J_y, exhibits Julia set structure.

The key insight: TRD dynamics create a nonlinear map on C (with auxiliary
parameters), and the boundary of bounded orbits may have fractal structure
analogous to Julia sets.

Connection to Consciousness Conjecture:
- Julia set boundary = region of maximal dynamical complexity
- Experience correlates with proximity to this boundary
- Meta-sLoop = self-modeling of boundary dynamics

Author: Investigation initiated 2026-01-21
"""

import numpy as np
from dataclasses import dataclass
from typing import Tuple, Optional, Callable
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
import warnings


# =============================================================================
# TRD ITERATION ON COMPLEX PLANE
# =============================================================================

@dataclass
class TRDComplexParams:
    """
    Parameters for TRD dynamics restricted to complex plane.

    The full TRD has J in R^3, but we can study the (J_x, J_y) plane
    with J_z treated as a parameter.
    """
    K_B: float = 1.2          # Manifestation threshold
    damping: float = 0.05     # Per-tick dissipation
    C: float = 1.0            # Wave speed
    J_z: float = 0.0          # Fixed z-component
    neighbor_coupling: float = 0.1  # Mean-field neighbor strength


def trd_complex_map(psi: complex, w: complex, params: TRDComplexParams,
                    neighbor_psi: complex = 0.0) -> Tuple[complex, complex]:
    """
    One iteration of TRD dynamics on the complex plane.

    Maps (psi, w) -> (psi', w') where:
      psi = J_x + i*J_y (complexified flux)
      w = w_x + i*w_y (complexified wave velocity)

    The map incorporates:
    1. Wave equation (Laplacian as mean-field)
    2. Damping
    3. Nonlinear manifestation threshold
    """
    # Effective Laplacian (mean-field approximation)
    # nabla^2 psi ≈ coupling * (neighbor_psi - psi)
    laplacian = params.neighbor_coupling * (neighbor_psi - psi)

    # Wave equation: acceleration term
    acc = params.C**2 * laplacian

    # Update velocity and position
    w_new = w + acc
    psi_new = psi + w_new

    # Damping
    psi_new *= (1 - params.damping)

    # Nonlinear manifestation effect:
    # When |psi| crosses K_B, there's a "snap" that redistributes flux
    rho = abs(psi_new)
    if rho > params.K_B:
        # Manifestation creates a nonlinear perturbation
        # This is where chaos can enter
        excess = rho - params.K_B
        # Redistribute excess as phase rotation (simplified model)
        phase_kick = 0.1 * excess / params.K_B
        psi_new *= np.exp(1j * phase_kick)

    return psi_new, w_new


def iterate_trd_complex(psi_0: complex, params: TRDComplexParams,
                        n_iter: int = 100,
                        neighbor_psi: complex = 0.0) -> np.ndarray:
    """
    Iterate the TRD complex map and return trajectory.
    """
    trajectory = np.zeros(n_iter + 1, dtype=complex)
    trajectory[0] = psi_0

    psi = psi_0
    w = 0.0 + 0.0j  # Start with zero velocity

    for i in range(n_iter):
        psi, w = trd_complex_map(psi, w, params, neighbor_psi)
        trajectory[i + 1] = psi

        # Check for escape
        if abs(psi) > 1e6:
            trajectory[i + 1:] = np.nan
            break

    return trajectory


# =============================================================================
# JULIA SET COMPUTATION
# =============================================================================

def compute_julia_set(resolution: int = 500,
                      psi_range: Tuple[float, float] = (-3.0, 3.0),
                      params: TRDComplexParams = None,
                      max_iter: int = 100,
                      escape_radius: float = 10.0) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Compute the Julia set for TRD complex dynamics.

    Returns:
        X, Y: Coordinate grids (real and imaginary parts)
        escape_time: 2D array of iteration counts until escape (or max_iter)
    """
    if params is None:
        params = TRDComplexParams()

    x = np.linspace(psi_range[0], psi_range[1], resolution)
    y = np.linspace(psi_range[0], psi_range[1], resolution)
    X, Y = np.meshgrid(x, y)

    # Complex initial conditions
    psi_grid = X + 1j * Y

    escape_time = np.full((resolution, resolution), max_iter, dtype=float)

    for i in range(resolution):
        for j in range(resolution):
            psi = psi_grid[i, j]
            w = 0.0 + 0.0j

            for n in range(max_iter):
                psi, w = trd_complex_map(psi, w, params)

                if abs(psi) > escape_radius:
                    # Smooth coloring using continuous escape time
                    escape_time[i, j] = n + 1 - np.log2(np.log2(abs(psi) + 1))
                    break

    return X, Y, escape_time


def compute_mandelbrot_analog(resolution: int = 500,
                              c_range: Tuple[float, float] = (-2.5, 1.5),
                              params: TRDComplexParams = None,
                              max_iter: int = 100) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Compute a Mandelbrot-like set where the parameter c varies.

    In standard Mandelbrot: z -> z^2 + c, starting from z=0
    In TRD analog: psi -> TRD_map(psi, c), where c is neighbor_psi

    This explores which "neighbor environments" lead to bounded behavior.
    """
    if params is None:
        params = TRDComplexParams()

    x = np.linspace(c_range[0], c_range[1], resolution)
    y = np.linspace(-2.0, 2.0, resolution)
    X, Y = np.meshgrid(x, y)

    c_grid = X + 1j * Y
    escape_time = np.full((resolution, resolution), max_iter, dtype=float)

    for i in range(resolution):
        for j in range(resolution):
            c = c_grid[i, j]  # This acts as neighbor_psi
            psi = 0.5 + 0.0j  # Start near threshold
            w = 0.0 + 0.0j

            for n in range(max_iter):
                psi, w = trd_complex_map(psi, w, params, neighbor_psi=c)

                if abs(psi) > 10.0:
                    escape_time[i, j] = n + 1 - np.log2(np.log2(abs(psi) + 1))
                    break

    return X, Y, escape_time


# =============================================================================
# LEMNISCATIC CONNECTION
# =============================================================================

def lemniscate_level_set(resolution: int = 500,
                         range_val: float = 2.0) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Compute the lemniscate of Bernoulli: |z^2 - 1| = 1

    This is connected to G* through the lemniscatic integral.
    The conjecture: TRD Julia sets have lemniscatic structure.
    """
    x = np.linspace(-range_val, range_val, resolution)
    y = np.linspace(-range_val, range_val, resolution)
    X, Y = np.meshgrid(x, y)

    Z = X + 1j * Y
    lemniscate_distance = np.abs(np.abs(Z**2 - 1) - 1)

    return X, Y, lemniscate_distance


def compute_g_star_curve(n_points: int = 1000) -> Tuple[np.ndarray, np.ndarray]:
    """
    Compute the Lemniscate-Alpha curve from the TRD framework.

    x(t) = cos(t) + 0.5*cos(2t) + 0.5*cos(4t) + 0.4*cos(8t) + 0.0625*cos(16t)
    y(t) = sin(t) - 0.5*sin(2t) + 0.5*sin(4t) - 0.35*sin(8t) + 0.0625*sin(16t)
    """
    t = np.linspace(0, 2 * np.pi, n_points)

    x = (np.cos(t) + 0.5 * np.cos(2*t) + 0.5 * np.cos(4*t) +
         0.4 * np.cos(8*t) + 0.0625 * np.cos(16*t))
    y = (np.sin(t) - 0.5 * np.sin(2*t) + 0.5 * np.sin(4*t) -
         0.35 * np.sin(8*t) + 0.0625 * np.sin(16*t))

    return x, y


# =============================================================================
# BOUNDARY PROXIMITY (EXPERIENCE MEASURE)
# =============================================================================

def compute_boundary_proximity(escape_time: np.ndarray,
                               max_iter: int = 100) -> np.ndarray:
    """
    Compute beta(c) - the boundary proximity function.

    beta ≈ 1 near the Julia set boundary (where escape times vary rapidly)
    beta ≈ 0 in the interior (always bounded) or exterior (always escapes quickly)

    This is proposed to correlate with "experience intensity".
    """
    # Gradient of escape time indicates boundary
    grad_y, grad_x = np.gradient(escape_time)
    gradient_magnitude = np.sqrt(grad_x**2 + grad_y**2)

    # Normalize to [0, 1]
    beta = gradient_magnitude / (gradient_magnitude.max() + 1e-10)

    return beta


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_fractal_colormap() -> LinearSegmentedColormap:
    """Create a colormap suitable for fractal visualization."""
    colors = [
        (0.0, 0.0, 0.2),    # Deep blue (interior)
        (0.0, 0.3, 0.6),    # Blue
        (0.0, 0.6, 0.8),    # Cyan
        (0.2, 0.8, 0.4),    # Green
        (0.8, 0.8, 0.0),    # Yellow
        (1.0, 0.4, 0.0),    # Orange
        (0.8, 0.0, 0.2),    # Red
        (0.4, 0.0, 0.4),    # Purple
        (0.0, 0.0, 0.0),    # Black (escaped)
    ]
    return LinearSegmentedColormap.from_list('fractal', colors, N=256)


def plot_julia_set(X: np.ndarray, Y: np.ndarray, escape_time: np.ndarray,
                   title: str = "TRD Julia Set",
                   save_path: Optional[str] = None):
    """Visualize the Julia set."""
    fig, ax = plt.subplots(figsize=(12, 12))

    cmap = create_fractal_colormap()
    im = ax.imshow(escape_time, extent=[X.min(), X.max(), Y.min(), Y.max()],
                   origin='lower', cmap=cmap, aspect='equal')

    ax.set_xlabel(r'Re($\psi$) = $J_x$', fontsize=14)
    ax.set_ylabel(r'Im($\psi$) = $J_y$', fontsize=14)
    ax.set_title(title, fontsize=16)

    plt.colorbar(im, ax=ax, label='Escape Time', shrink=0.8)
    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=200, bbox_inches='tight')

    return fig, ax


def plot_experience_map(X: np.ndarray, Y: np.ndarray, beta: np.ndarray,
                        title: str = "Experience Intensity (Boundary Proximity)",
                        save_path: Optional[str] = None):
    """Visualize the boundary proximity / experience map."""
    fig, ax = plt.subplots(figsize=(12, 12))

    im = ax.imshow(beta, extent=[X.min(), X.max(), Y.min(), Y.max()],
                   origin='lower', cmap='hot', aspect='equal')

    ax.set_xlabel(r'Re($\psi$) = $J_x$', fontsize=14)
    ax.set_ylabel(r'Im($\psi$) = $J_y$', fontsize=14)
    ax.set_title(title, fontsize=16)

    plt.colorbar(im, ax=ax, label=r'$\beta$ (Experience Intensity)', shrink=0.8)
    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=200, bbox_inches='tight')

    return fig, ax


def plot_lemniscatic_comparison(escape_time: np.ndarray,
                                X: np.ndarray, Y: np.ndarray,
                                save_path: Optional[str] = None):
    """
    Compare TRD Julia set with lemniscatic structure.
    """
    fig, axes = plt.subplots(1, 3, figsize=(18, 6))

    # TRD Julia set
    cmap = create_fractal_colormap()
    im0 = axes[0].imshow(escape_time, extent=[X.min(), X.max(), Y.min(), Y.max()],
                         origin='lower', cmap=cmap, aspect='equal')
    axes[0].set_title('TRD Julia Set', fontsize=14)
    axes[0].set_xlabel(r'$J_x$')
    axes[0].set_ylabel(r'$J_y$')

    # Bernoulli lemniscate level set
    X_lem, Y_lem, lem_dist = lemniscate_level_set(resolution=escape_time.shape[0],
                                                   range_val=max(abs(X.min()), X.max()))
    im1 = axes[1].imshow(lem_dist, extent=[X_lem.min(), X_lem.max(), Y_lem.min(), Y_lem.max()],
                         origin='lower', cmap='viridis', aspect='equal')
    axes[1].contour(X_lem, Y_lem, lem_dist, levels=[0.0], colors='white', linewidths=2)
    axes[1].set_title(r'Lemniscate $|z^2-1|=1$', fontsize=14)
    axes[1].set_xlabel('Re(z)')
    axes[1].set_ylabel('Im(z)')

    # Lemniscate-Alpha curve overlay
    x_curve, y_curve = compute_g_star_curve()
    # Scale to match Julia set range
    scale = (X.max() - X.min()) / (x_curve.max() - x_curve.min()) * 0.4
    x_scaled = x_curve * scale
    y_scaled = y_curve * scale

    im2 = axes[2].imshow(escape_time, extent=[X.min(), X.max(), Y.min(), Y.max()],
                         origin='lower', cmap=cmap, aspect='equal', alpha=0.7)
    axes[2].plot(x_scaled, y_scaled, 'w-', linewidth=2, label='Lemniscate-Alpha')
    axes[2].set_title('Julia Set + Lemniscate-Alpha', fontsize=14)
    axes[2].set_xlabel(r'$J_x$')
    axes[2].set_ylabel(r'$J_y$')
    axes[2].legend(loc='upper right')

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')

    return fig, axes


# =============================================================================
# INVESTIGATION RUNNER
# =============================================================================

def run_julia_investigation(output_dir: str = "investigation_results"):
    """
    Run the Julia set investigation for TRD dynamics.
    """
    import os
    os.makedirs(output_dir, exist_ok=True)

    print("=" * 60)
    print("JULIA SET INVESTIGATION")
    print("Complexified Flux Dynamics")
    print("=" * 60)

    params = TRDComplexParams(K_B=1.2, damping=0.05)
    print(f"\nParameters: K_B={params.K_B}, damping={params.damping}")

    # Phase 1: Standard Julia set
    print("\n[Phase 1] Computing TRD Julia set...")
    X, Y, escape_time = compute_julia_set(resolution=500, params=params, max_iter=100)

    plot_julia_set(X, Y, escape_time,
                   title=f"TRD Julia Set (K_B={params.K_B})",
                   save_path=os.path.join(output_dir, "trd_julia_set.png"))

    # Phase 2: Experience map (boundary proximity)
    print("[Phase 2] Computing experience intensity map...")
    beta = compute_boundary_proximity(escape_time)

    plot_experience_map(X, Y, beta,
                        save_path=os.path.join(output_dir, "experience_intensity.png"))

    # Statistics
    high_experience_fraction = np.mean(beta > 0.5)
    print(f"  Fraction with high experience (beta > 0.5): {high_experience_fraction:.2%}")

    # Phase 3: Mandelbrot analog
    print("\n[Phase 3] Computing Mandelbrot analog (neighbor parameter space)...")
    X_m, Y_m, escape_time_m = compute_mandelbrot_analog(resolution=500, params=params)

    plot_julia_set(X_m, Y_m, escape_time_m,
                   title="TRD Mandelbrot Analog (Neighbor Environment Space)",
                   save_path=os.path.join(output_dir, "trd_mandelbrot.png"))

    # Phase 4: Lemniscatic comparison
    print("\n[Phase 4] Comparing with lemniscatic structure...")
    plot_lemniscatic_comparison(escape_time, X, Y,
                                save_path=os.path.join(output_dir, "lemniscatic_comparison.png"))

    # Phase 5: Parameter sweep
    print("\n[Phase 5] Parameter sweep (varying K_B)...")
    K_B_values = [0.5, 1.0, 1.5, 2.0]

    fig, axes = plt.subplots(2, 2, figsize=(14, 14))
    cmap = create_fractal_colormap()

    for idx, K_B in enumerate(K_B_values):
        params_sweep = TRDComplexParams(K_B=K_B)
        X_s, Y_s, escape_s = compute_julia_set(resolution=300, params=params_sweep, max_iter=80)

        ax = axes[idx // 2, idx % 2]
        ax.imshow(escape_s, extent=[X_s.min(), X_s.max(), Y_s.min(), Y_s.max()],
                  origin='lower', cmap=cmap, aspect='equal')
        ax.set_title(f'K_B = {K_B}', fontsize=14)
        ax.set_xlabel(r'$J_x$')
        ax.set_ylabel(r'$J_y$')

    plt.suptitle('Julia Set Structure vs Manifestation Threshold', fontsize=16)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "julia_kb_sweep.png"), dpi=150, bbox_inches='tight')

    print("\n" + "=" * 60)
    print("JULIA INVESTIGATION COMPLETE")
    print(f"Results saved to: {output_dir}/")
    print("=" * 60)

    return {
        'high_experience_fraction': high_experience_fraction,
        'escape_time_range': (escape_time.min(), escape_time.max())
    }


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    results = run_julia_investigation()
    print(f"\nKey findings:")
    print(f"  High-experience fraction: {results['high_experience_fraction']:.2%}")
