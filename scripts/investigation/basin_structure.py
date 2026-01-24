"""
Basin Structure Investigation for the Mandelbrot-Consciousness Conjecture

This module explores whether TRD configuration space exhibits fractal basin
boundaries separating stable (bound structure) and divergent (evaporation)
trajectories.

Key Questions:
1. Does the boundary between stability and divergence have fractal dimension > 1?
2. How does boundary structure depend on parameters (K_B, damping, etc.)?
3. Can we identify the "edge of chaos" region where complexity is maximal?

Author: Investigation initiated 2026-01-21
Status: Active development
"""

import numpy as np
from dataclasses import dataclass
from typing import Tuple, List, Optional, Callable
from enum import Enum, auto
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import warnings


# =============================================================================
# CONFIGURATION
# =============================================================================

@dataclass
class TRDParameters:
    """Parameters for TRD dynamics."""
    K_B: float = 1.2          # Manifestation threshold
    damping: float = 0.05     # Flux damping per tick
    decay_rate: float = 0.001 # Unlocked voxel decay
    C: float = 1.0            # Propagation speed

    def __post_init__(self):
        # Stability criterion: C^2 <= 1/6 for 3D wave equation
        if self.C**2 > 1/6:
            warnings.warn(f"C={self.C} may cause numerical instability")


class AsymptoticFate(Enum):
    """Classification of trajectory endpoints."""
    EVAPORATED = auto()      # All flux dissipated
    STABLE_POINT = auto()    # Fixed point attractor
    STABLE_CYCLE = auto()    # Periodic orbit
    CHAOTIC = auto()         # Bounded but aperiodic
    DIVERGENT = auto()       # Unbounded growth (numerical issue)
    UNDETERMINED = auto()    # Didn't converge in time limit


# =============================================================================
# SIMPLIFIED TRD DYNAMICS (Single Voxel + Mean Field)
# =============================================================================

@dataclass
class VoxelState:
    """State of a single voxel with mean-field neighbors."""
    s: int              # Ternary state: -1, 0, +1
    J: np.ndarray       # Flux vector (3D)
    w: np.ndarray       # Wave velocity (3D)

    @property
    def rho(self) -> float:
        """Flux density (magnitude)."""
        return np.linalg.norm(self.J)

    @property
    def psi(self) -> complex:
        """Complexified flux (J_x + i*J_y)."""
        return self.J[0] + 1j * self.J[1]

    def copy(self) -> 'VoxelState':
        return VoxelState(
            s=self.s,
            J=self.J.copy(),
            w=self.w.copy()
        )


def trd_step(state: VoxelState, params: TRDParameters,
             neighbor_flux: np.ndarray = None) -> VoxelState:
    """
    Execute one tick of simplified TRD dynamics.

    Uses mean-field approximation: neighbor_flux is the average flux
    of the 6 face-neighbors (or zero if not provided).
    """
    if neighbor_flux is None:
        neighbor_flux = np.zeros(3)

    new_state = state.copy()

    # Discrete Laplacian (6-connected, mean-field)
    # nabla^2 J ≈ 6 * (J_neighbor_avg - J)
    laplacian = 6 * (neighbor_flux - state.J)

    # Wave equation update (velocity-Verlet style)
    acc = params.C**2 * laplacian
    new_state.w = state.w + acc
    new_state.J = state.J + new_state.w

    # Damping
    new_state.J *= (1 - params.damping)

    # Compute density for manifestation check
    rho = new_state.rho

    # Manifestation dynamics
    if new_state.s == 0 and rho > params.K_B:
        # Genesis: void -> manifested
        p_manifest = 1.0 - np.exp(-(rho - params.K_B) / params.K_B)
        if np.random.random() < p_manifest:
            # Polarity from divergence sign (simplified: use J_z sign)
            div_sign = np.sign(neighbor_flux.sum() - state.J.sum())
            new_state.s = 1 if div_sign >= 0 else -1

    elif new_state.s != 0 and rho < params.K_B:
        # Evaporation: manifested -> void
        new_state.s = 0

    # Decay for unbound manifested voxels
    if new_state.s != 0:
        new_state.J *= (1 - params.decay_rate)

    return new_state


def classify_trajectory(trajectory: List[VoxelState],
                       tolerance: float = 1e-6) -> AsymptoticFate:
    """
    Classify the asymptotic behavior of a trajectory.
    """
    if len(trajectory) < 10:
        return AsymptoticFate.UNDETERMINED

    final_states = trajectory[-50:]  # Last 50 states

    # Check for evaporation
    final_rhos = [s.rho for s in final_states]
    if max(final_rhos) < tolerance:
        return AsymptoticFate.EVAPORATED

    # Check for divergence
    if max(final_rhos) > 1e10:
        return AsymptoticFate.DIVERGENT

    # Check for fixed point
    final_Js = np.array([s.J for s in final_states])
    variations = np.std(final_Js, axis=0)
    if np.all(variations < tolerance):
        return AsymptoticFate.STABLE_POINT

    # Check for periodic orbit (up to period 20)
    for period in range(1, 21):
        is_periodic = True
        for i in range(min(10, len(final_states) - period)):
            diff = np.linalg.norm(final_states[-(i+1)].J - final_states[-(i+1+period)].J)
            if diff > tolerance:
                is_periodic = False
                break
        if is_periodic:
            return AsymptoticFate.STABLE_CYCLE

    # Bounded but aperiodic = chaotic
    return AsymptoticFate.CHAOTIC


# =============================================================================
# BASIN BOUNDARY DETECTION
# =============================================================================

def compute_basin_map(resolution: int = 100,
                     J_range: Tuple[float, float] = (-3.0, 3.0),
                     max_ticks: int = 500,
                     params: TRDParameters = None,
                     seed: int = 42) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Compute the basin of attraction map in the (J_x, J_y) plane.

    Args:
        resolution: Grid resolution in each dimension
        J_range: Range for J_x and J_y components
        max_ticks: Maximum simulation time
        params: TRD parameters
        seed: Random seed for reproducibility

    Returns:
        X, Y: Coordinate grids
        fate_map: 2D array of AsymptoticFate values (as integers)
    """
    np.random.seed(seed)

    if params is None:
        params = TRDParameters()

    x = np.linspace(J_range[0], J_range[1], resolution)
    y = np.linspace(J_range[0], J_range[1], resolution)
    X, Y = np.meshgrid(x, y)

    fate_map = np.zeros((resolution, resolution), dtype=int)

    for i in range(resolution):
        for j in range(resolution):
            # Initialize with J = (J_x, J_y, 0), s=0, w=0
            initial_J = np.array([X[i, j], Y[i, j], 0.0])
            state = VoxelState(s=0, J=initial_J, w=np.zeros(3))

            # Evolve trajectory
            trajectory = [state]
            for _ in range(max_ticks):
                state = trd_step(state, params)
                trajectory.append(state)

                # Early termination conditions
                if state.rho < 1e-10:  # Evaporated
                    break
                if state.rho > 1e6:    # Diverged
                    break

            fate = classify_trajectory(trajectory)
            fate_map[i, j] = fate.value

    return X, Y, fate_map


def estimate_fractal_dimension(boundary_mask: np.ndarray,
                               scales: List[int] = None) -> float:
    """
    Estimate fractal dimension using box-counting method.

    Args:
        boundary_mask: Boolean array where True indicates boundary pixels
        scales: Box sizes to use (default: powers of 2)

    Returns:
        Estimated fractal dimension
    """
    if scales is None:
        max_scale = min(boundary_mask.shape) // 4
        scales = [2**i for i in range(int(np.log2(max_scale)) + 1) if 2**i <= max_scale]

    counts = []
    for scale in scales:
        # Count boxes containing boundary points
        n_boxes = 0
        for i in range(0, boundary_mask.shape[0] - scale + 1, scale):
            for j in range(0, boundary_mask.shape[1] - scale + 1, scale):
                if boundary_mask[i:i+scale, j:j+scale].any():
                    n_boxes += 1
        counts.append(n_boxes)

    # Linear regression in log-log space
    log_scales = np.log(scales)
    log_counts = np.log(np.array(counts) + 1)  # +1 to avoid log(0)

    # D = -slope of log(N) vs log(scale)
    slope, _ = np.polyfit(log_scales, log_counts, 1)
    fractal_dim = -slope

    return fractal_dim


def find_basin_boundary(fate_map: np.ndarray) -> np.ndarray:
    """
    Find pixels on the boundary between different basins.

    Returns boolean mask where True indicates boundary pixels.
    """
    from scipy.ndimage import generic_filter

    def is_boundary(values):
        """Check if center pixel is on boundary (neighbors have different fates)."""
        center = values[4]  # 3x3 kernel, center is index 4
        return len(set(values)) > 1

    # Use 3x3 neighborhood to detect boundaries
    boundary = generic_filter(
        fate_map.astype(float),
        lambda x: 1.0 if len(set(x.astype(int))) > 1 else 0.0,
        size=3,
        mode='constant',
        cval=0
    )

    return boundary > 0.5


# =============================================================================
# LYAPUNOV EXPONENT ESTIMATION
# =============================================================================

def estimate_lyapunov(initial_state: VoxelState,
                      params: TRDParameters,
                      n_steps: int = 1000,
                      perturbation: float = 1e-8) -> float:
    """
    Estimate the maximal Lyapunov exponent for a trajectory.

    Uses the standard method of tracking separation of nearby trajectories.
    """
    state1 = initial_state.copy()

    # Perturbed initial condition
    state2 = initial_state.copy()
    state2.J = state2.J + perturbation * np.random.randn(3)
    state2.J /= np.linalg.norm(state2.J) if np.linalg.norm(state2.J) > 0 else 1
    state2.J *= np.linalg.norm(initial_state.J) + perturbation

    lyap_sum = 0.0
    n_valid = 0

    for _ in range(n_steps):
        state1 = trd_step(state1, params)
        state2 = trd_step(state2, params)

        # Compute separation
        separation = np.linalg.norm(state2.J - state1.J)

        if separation > 1e-15 and separation < 1e10:
            lyap_sum += np.log(separation / perturbation)
            n_valid += 1

            # Renormalize perturbation
            direction = (state2.J - state1.J) / separation
            state2.J = state1.J + perturbation * direction

    if n_valid == 0:
        return 0.0

    return lyap_sum / n_valid


def compute_lyapunov_map(resolution: int = 50,
                         J_range: Tuple[float, float] = (-3.0, 3.0),
                         params: TRDParameters = None,
                         n_steps: int = 500) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Compute Lyapunov exponent across the (J_x, J_y) plane.
    """
    if params is None:
        params = TRDParameters()

    x = np.linspace(J_range[0], J_range[1], resolution)
    y = np.linspace(J_range[0], J_range[1], resolution)
    X, Y = np.meshgrid(x, y)

    lyap_map = np.zeros((resolution, resolution))

    for i in range(resolution):
        for j in range(resolution):
            initial_J = np.array([X[i, j], Y[i, j], 0.0])
            state = VoxelState(s=0, J=initial_J, w=np.zeros(3))
            lyap_map[i, j] = estimate_lyapunov(state, params, n_steps)

    return X, Y, lyap_map


# =============================================================================
# VISUALIZATION
# =============================================================================

def plot_basin_map(X: np.ndarray, Y: np.ndarray, fate_map: np.ndarray,
                   title: str = "TRD Basin Structure",
                   save_path: Optional[str] = None):
    """Visualize the basin of attraction map."""

    # Color map for different fates
    colors = ['white', 'blue', 'green', 'yellow', 'red', 'gray']
    cmap = ListedColormap(colors[:len(np.unique(fate_map))])

    fig, ax = plt.subplots(figsize=(10, 10))

    im = ax.pcolormesh(X, Y, fate_map, cmap=cmap, shading='auto')
    ax.set_xlabel(r'$J_x$', fontsize=14)
    ax.set_ylabel(r'$J_y$', fontsize=14)
    ax.set_title(title, fontsize=16)
    ax.set_aspect('equal')

    # Add colorbar with fate labels
    fate_names = [f.name for f in AsymptoticFate]
    cbar = plt.colorbar(im, ax=ax, ticks=range(len(np.unique(fate_map))))
    unique_fates = np.unique(fate_map)
    cbar.ax.set_yticklabels([fate_names[f-1] for f in unique_fates])

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')

    return fig, ax


def plot_boundary_analysis(X: np.ndarray, Y: np.ndarray, fate_map: np.ndarray,
                          save_path: Optional[str] = None):
    """Analyze and visualize basin boundaries."""

    boundary_mask = find_basin_boundary(fate_map)
    fractal_dim = estimate_fractal_dimension(boundary_mask)

    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    # Basin map
    colors = ['white', 'blue', 'green', 'yellow', 'red', 'gray']
    cmap = ListedColormap(colors[:len(np.unique(fate_map))])
    axes[0].pcolormesh(X, Y, fate_map, cmap=cmap, shading='auto')
    axes[0].set_xlabel(r'$J_x$', fontsize=14)
    axes[0].set_ylabel(r'$J_y$', fontsize=14)
    axes[0].set_title('Basin of Attraction Map', fontsize=16)
    axes[0].set_aspect('equal')

    # Boundary with fractal dimension
    axes[1].imshow(boundary_mask, extent=[X.min(), X.max(), Y.min(), Y.max()],
                   origin='lower', cmap='hot')
    axes[1].set_xlabel(r'$J_x$', fontsize=14)
    axes[1].set_ylabel(r'$J_y$', fontsize=14)
    axes[1].set_title(f'Basin Boundary (Fractal Dim ≈ {fractal_dim:.3f})', fontsize=16)
    axes[1].set_aspect('equal')

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')

    return fig, axes, fractal_dim


# =============================================================================
# INVESTIGATION RUNNER
# =============================================================================

def run_investigation(output_dir: str = "investigation_results"):
    """
    Run the full basin structure investigation.
    """
    import os
    os.makedirs(output_dir, exist_ok=True)

    print("=" * 60)
    print("MANDELBROT-CONSCIOUSNESS CONJECTURE INVESTIGATION")
    print("Basin Structure Analysis")
    print("=" * 60)

    # Standard parameters
    params = TRDParameters(K_B=1.2, damping=0.05)

    print(f"\nParameters: K_B={params.K_B}, damping={params.damping}")

    # Phase 1: Basin map
    print("\n[Phase 1] Computing basin map...")
    X, Y, fate_map = compute_basin_map(resolution=100, params=params)

    # Analyze boundaries
    print("[Phase 1] Analyzing boundaries...")
    fig, axes, fractal_dim = plot_boundary_analysis(
        X, Y, fate_map,
        save_path=os.path.join(output_dir, "basin_boundary_analysis.png")
    )
    print(f"  Estimated fractal dimension: {fractal_dim:.4f}")

    # Phase 2: Lyapunov map
    print("\n[Phase 2] Computing Lyapunov exponents...")
    X_lyap, Y_lyap, lyap_map = compute_lyapunov_map(resolution=50, params=params)

    fig, ax = plt.subplots(figsize=(10, 10))
    im = ax.pcolormesh(X_lyap, Y_lyap, lyap_map, cmap='RdBu_r', shading='auto',
                       vmin=-1, vmax=1)
    ax.set_xlabel(r'$J_x$', fontsize=14)
    ax.set_ylabel(r'$J_y$', fontsize=14)
    ax.set_title('Lyapunov Exponent Map', fontsize=16)
    ax.set_aspect('equal')
    plt.colorbar(im, ax=ax, label=r'$\lambda_{max}$')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "lyapunov_map.png"), dpi=150, bbox_inches='tight')

    # Summary statistics
    n_chaotic = np.sum(lyap_map > 0)
    n_total = lyap_map.size
    chaos_fraction = n_chaotic / n_total

    print(f"  Fraction of chaotic region: {chaos_fraction:.2%}")
    print(f"  Max Lyapunov exponent: {lyap_map.max():.4f}")

    # Phase 3: Parameter sweep (K_B)
    print("\n[Phase 3] Parameter sweep (K_B)...")
    K_B_values = [0.5, 1.0, 1.5, 2.0, 2.5]
    fractal_dims = []

    for K_B in K_B_values:
        params_sweep = TRDParameters(K_B=K_B)
        X, Y, fate_map = compute_basin_map(resolution=50, params=params_sweep)
        boundary_mask = find_basin_boundary(fate_map)
        fd = estimate_fractal_dimension(boundary_mask)
        fractal_dims.append(fd)
        print(f"  K_B={K_B:.1f}: fractal_dim={fd:.4f}")

    # Plot parameter dependence
    fig, ax = plt.subplots(figsize=(8, 6))
    ax.plot(K_B_values, fractal_dims, 'bo-', markersize=10)
    ax.axhline(y=1.0, color='gray', linestyle='--', label='D=1 (smooth)')
    ax.axhline(y=2.0, color='gray', linestyle=':', label='D=2 (space-filling)')
    ax.set_xlabel(r'$K_B$ (Manifestation Threshold)', fontsize=14)
    ax.set_ylabel('Fractal Dimension', fontsize=14)
    ax.set_title('Boundary Fractal Dimension vs Threshold', fontsize=16)
    ax.legend()
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "fractal_dim_vs_KB.png"), dpi=150, bbox_inches='tight')

    print("\n" + "=" * 60)
    print("INVESTIGATION COMPLETE")
    print(f"Results saved to: {output_dir}/")
    print("=" * 60)

    return {
        'fractal_dimension': fractal_dim,
        'chaos_fraction': chaos_fraction,
        'parameter_sweep': dict(zip(K_B_values, fractal_dims))
    }


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    results = run_investigation()
    print(f"\nKey findings:")
    print(f"  Fractal dimension: {results['fractal_dimension']:.4f}")
    print(f"  Chaos fraction: {results['chaos_fraction']:.2%}")
