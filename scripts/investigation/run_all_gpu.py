"""
GPU-Accelerated Investigation Runner

Runs all Mandelbrot-Consciousness Conjecture investigations using CUDA.
Optimized for RTX 5090.

Author: 2026-01-21
"""

import torch
import torch.nn.functional as F
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
import os
from dataclasses import dataclass
from typing import Tuple, Optional
from scipy import special
import time

# Check GPU
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(f"Using device: {device}")
if torch.cuda.is_available():
    print(f"GPU: {torch.cuda.get_device_name(0)}")
    print(f"VRAM: {torch.cuda.get_device_properties(0).total_memory / 1e9:.1f} GB")

# Output directory
OUTPUT_DIR = "investigation_results"
os.makedirs(OUTPUT_DIR, exist_ok=True)


# =============================================================================
# PART 1: GPU-ACCELERATED BASIN STRUCTURE
# =============================================================================

def compute_basin_map_gpu(resolution: int = 500,
                          J_range: Tuple[float, float] = (-3.0, 3.0),
                          max_ticks: int = 500,
                          K_B: float = 1.2,
                          damping: float = 0.05,
                          C: float = 1.0) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    GPU-accelerated basin of attraction computation.

    Processes entire grid in parallel on GPU.
    """
    print(f"\n[Basin Structure] Computing {resolution}x{resolution} grid on GPU...")
    start_time = time.time()

    # Create coordinate grid
    x = torch.linspace(J_range[0], J_range[1], resolution, device=device)
    y = torch.linspace(J_range[0], J_range[1], resolution, device=device)
    X, Y = torch.meshgrid(x, y, indexing='ij')

    # Initialize flux field: J = (J_x, J_y, 0) for each point
    # Shape: (resolution, resolution, 3)
    J = torch.zeros(resolution, resolution, 3, device=device)
    J[:, :, 0] = X
    J[:, :, 1] = Y

    # Wave velocity
    w = torch.zeros_like(J)

    # Track trajectory properties
    max_rho = torch.zeros(resolution, resolution, device=device)
    final_rho = torch.zeros(resolution, resolution, device=device)
    escape_time = torch.full((resolution, resolution), max_ticks, device=device, dtype=torch.float32)
    escaped = torch.zeros(resolution, resolution, dtype=torch.bool, device=device)

    # Iterate
    for t in range(max_ticks):
        # Compute density
        rho = torch.norm(J, dim=2)

        # Track max and check escape
        max_rho = torch.maximum(max_rho, rho)
        newly_escaped = (rho < 1e-6) & ~escaped
        escape_time[newly_escaped] = t
        escaped = escaped | newly_escaped

        # Simple dynamics (no spatial coupling for speed)
        laplacian = -0.1 * J  # Self-interaction
        acc = C**2 * laplacian
        w = w + acc
        J = J + w
        J = J * (1 - damping)

        # Manifestation nonlinearity
        over_threshold = rho > K_B
        excess = torch.clamp(rho - K_B, min=0)
        phase_kick = 0.1 * excess / K_B

        # Apply phase rotation to (J_x, J_y)
        cos_kick = torch.cos(phase_kick)
        sin_kick = torch.sin(phase_kick)
        J_x_new = J[:, :, 0] * cos_kick - J[:, :, 1] * sin_kick
        J_y_new = J[:, :, 0] * sin_kick + J[:, :, 1] * cos_kick
        J[:, :, 0] = torch.where(over_threshold, J_x_new, J[:, :, 0])
        J[:, :, 1] = torch.where(over_threshold, J_y_new, J[:, :, 1])

        if t % 100 == 0:
            n_escaped = escaped.sum().item()
            print(f"  Tick {t}/{max_ticks}: {n_escaped}/{resolution**2} escaped ({100*n_escaped/resolution**2:.1f}%)")

    # Final density
    final_rho = torch.norm(J, dim=2)

    # Classify fates
    # 0 = evaporated, 1 = stable, 2 = chaotic (bounded but not converged)
    fate_map = torch.zeros(resolution, resolution, device=device, dtype=torch.int32)
    fate_map[final_rho < 1e-4] = 0  # Evaporated
    fate_map[(final_rho >= 1e-4) & (final_rho < 10)] = 1  # Stable
    fate_map[final_rho >= 10] = 2  # Divergent/chaotic

    elapsed = time.time() - start_time
    print(f"  Completed in {elapsed:.2f}s")

    return (X.cpu().numpy(), Y.cpu().numpy(),
            fate_map.cpu().numpy(), escape_time.cpu().numpy())


def compute_fractal_dimension_gpu(boundary_mask: torch.Tensor) -> float:
    """GPU-accelerated box-counting fractal dimension."""
    boundary_mask = boundary_mask.to(device)

    max_scale = min(boundary_mask.shape) // 4
    scales = [2**i for i in range(int(np.log2(max_scale)) + 1) if 2**i <= max_scale]

    counts = []
    for scale in scales:
        # Use max pooling to count occupied boxes
        pooled = F.max_pool2d(
            boundary_mask.float().unsqueeze(0).unsqueeze(0),
            kernel_size=scale,
            stride=scale
        )
        n_boxes = (pooled > 0).sum().item()
        counts.append(n_boxes)

    # Linear regression
    log_scales = np.log(scales)
    log_counts = np.log(np.array(counts) + 1)
    slope, _ = np.polyfit(log_scales, log_counts, 1)

    return -slope


# =============================================================================
# PART 2: GPU-ACCELERATED JULIA SET
# =============================================================================

def compute_julia_set_gpu(resolution: int = 1000,
                          psi_range: Tuple[float, float] = (-3.0, 3.0),
                          max_iter: int = 200,
                          K_B: float = 1.2,
                          damping: float = 0.05,
                          C: float = 1.0,
                          escape_radius: float = 10.0) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    GPU-accelerated Julia set computation for TRD dynamics.
    """
    print(f"\n[Julia Set] Computing {resolution}x{resolution} Julia set on GPU...")
    start_time = time.time()

    # Complex grid
    x = torch.linspace(psi_range[0], psi_range[1], resolution, device=device)
    y = torch.linspace(psi_range[0], psi_range[1], resolution, device=device)
    X, Y = torch.meshgrid(x, y, indexing='ij')

    # Complex initial conditions (as real + imag components)
    psi_real = X.clone()
    psi_imag = Y.clone()
    w_real = torch.zeros_like(X)
    w_imag = torch.zeros_like(Y)

    # Escape time tracking
    escape_time = torch.full((resolution, resolution), max_iter, device=device, dtype=torch.float32)
    escaped = torch.zeros(resolution, resolution, dtype=torch.bool, device=device)

    for n in range(max_iter):
        # Magnitude
        rho = torch.sqrt(psi_real**2 + psi_imag**2)

        # Check escape
        newly_escaped = (rho > escape_radius) & ~escaped
        # Smooth coloring
        smooth_escape = n + 1 - torch.log2(torch.log2(rho + 1).clamp(min=1e-10))
        escape_time[newly_escaped] = smooth_escape[newly_escaped]
        escaped = escaped | (rho > escape_radius)

        # TRD dynamics on complex plane
        # Laplacian approximation (self-coupling)
        laplacian_real = -0.1 * psi_real
        laplacian_imag = -0.1 * psi_imag

        acc_real = C**2 * laplacian_real
        acc_imag = C**2 * laplacian_imag

        w_real = w_real + acc_real
        w_imag = w_imag + acc_imag

        psi_real = psi_real + w_real
        psi_imag = psi_imag + w_imag

        psi_real = psi_real * (1 - damping)
        psi_imag = psi_imag * (1 - damping)

        # Manifestation nonlinearity
        rho = torch.sqrt(psi_real**2 + psi_imag**2)
        over_threshold = rho > K_B
        excess = torch.clamp(rho - K_B, min=0)
        phase_kick = 0.1 * excess / K_B

        cos_kick = torch.cos(phase_kick)
        sin_kick = torch.sin(phase_kick)
        psi_real_new = psi_real * cos_kick - psi_imag * sin_kick
        psi_imag_new = psi_real * sin_kick + psi_imag * cos_kick

        psi_real = torch.where(over_threshold, psi_real_new, psi_real)
        psi_imag = torch.where(over_threshold, psi_imag_new, psi_imag)

        if n % 50 == 0:
            n_escaped = escaped.sum().item()
            print(f"  Iter {n}/{max_iter}: {n_escaped}/{resolution**2} escaped")

    elapsed = time.time() - start_time
    print(f"  Completed in {elapsed:.2f}s")

    return X.cpu().numpy(), Y.cpu().numpy(), escape_time.cpu().numpy()


def compute_experience_map_gpu(escape_time: torch.Tensor) -> torch.Tensor:
    """Compute boundary proximity (experience intensity) on GPU."""
    escape_time = escape_time.to(device).float()

    # Sobel gradients
    sobel_x = torch.tensor([[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]],
                           device=device, dtype=torch.float32).view(1, 1, 3, 3)
    sobel_y = torch.tensor([[-1, -2, -1], [0, 0, 0], [1, 2, 1]],
                           device=device, dtype=torch.float32).view(1, 1, 3, 3)

    et = escape_time.unsqueeze(0).unsqueeze(0)
    grad_x = F.conv2d(et, sobel_x, padding=1)
    grad_y = F.conv2d(et, sobel_y, padding=1)

    gradient_mag = torch.sqrt(grad_x**2 + grad_y**2).squeeze()
    beta = gradient_mag / (gradient_mag.max() + 1e-10)

    return beta


# =============================================================================
# PART 3: GPU-ACCELERATED META-SLOOP DETECTION
# =============================================================================

def scan_meta_sloop_gpu(resolution: int = 100,
                        J_range: Tuple[float, float] = (-3.0, 3.0),
                        n_steps: int = 200,
                        K_B: float = 1.2,
                        damping: float = 0.05) -> dict:
    """
    GPU-accelerated meta-sLoop detection across configuration space.
    """
    print(f"\n[Meta-sLoop] Scanning {resolution}x{resolution} grid on GPU...")
    start_time = time.time()

    # Create grid
    x = torch.linspace(J_range[0], J_range[1], resolution, device=device)
    y = torch.linspace(J_range[0], J_range[1], resolution, device=device)
    X, Y = torch.meshgrid(x, y, indexing='ij')

    # Initialize: each grid point is a separate trajectory
    J = torch.zeros(resolution, resolution, 3, device=device)
    J[:, :, 0] = X
    J[:, :, 1] = Y
    w = torch.zeros_like(J)

    # Track trajectory statistics for model analysis
    trajectory_mean = torch.zeros(resolution, resolution, 3, device=device)
    trajectory_var = torch.zeros(resolution, resolution, 3, device=device)

    # For self-modeling depth: track oscillation properties
    prev_rho = torch.norm(J, dim=2)
    oscillation_count = torch.zeros(resolution, resolution, device=device)
    prev_sign = torch.zeros(resolution, resolution, device=device)

    # Evolve all trajectories in parallel
    for t in range(n_steps):
        rho = torch.norm(J, dim=2)

        # Track oscillations (sign changes in d(rho)/dt)
        drho = rho - prev_rho
        current_sign = torch.sign(drho)
        sign_change = (current_sign != prev_sign) & (prev_sign != 0)
        oscillation_count += sign_change.float()
        prev_sign = current_sign
        prev_rho = rho.clone()

        # Running mean and variance
        delta = J - trajectory_mean
        trajectory_mean += delta / (t + 1)
        trajectory_var += delta * (J - trajectory_mean)

        # TRD step
        laplacian = -0.1 * J
        acc = 1.0 * laplacian
        w = w + acc
        J = J + w
        J = J * (1 - damping)

        # Manifestation
        over_threshold = rho.unsqueeze(-1).expand_as(J) > K_B
        excess = torch.clamp(rho - K_B, min=0)
        phase_kick = 0.1 * excess / K_B

        cos_kick = torch.cos(phase_kick)
        sin_kick = torch.sin(phase_kick)
        J_x_new = J[:, :, 0] * cos_kick - J[:, :, 1] * sin_kick
        J_y_new = J[:, :, 0] * sin_kick + J[:, :, 1] * cos_kick

        mask = rho > K_B
        J[:, :, 0] = torch.where(mask, J_x_new, J[:, :, 0])
        J[:, :, 1] = torch.where(mask, J_y_new, J[:, :, 1])

    # Compute metrics

    # 1. Model stability: low variance = stable model
    variance = trajectory_var / (n_steps - 1)
    total_var = variance.sum(dim=2)
    stability = 1.0 / (1.0 + total_var)

    # 2. Self-modeling depth: based on oscillation frequency
    # More oscillations = higher complexity = deeper self-modeling
    depth = torch.clamp(oscillation_count / 10, 0, 5).int()

    # 3. Boundary proximity
    final_rho = torch.norm(J, dim=2)
    distance_to_KB = torch.abs(final_rho - K_B)
    beta = 1.0 / (1.0 + distance_to_KB / K_B)

    # 4. Meta-sLoop detection
    is_meta_sloop = (stability > 0.3) & (depth >= 3) & (beta > 0.4)

    elapsed = time.time() - start_time
    print(f"  Completed in {elapsed:.2f}s")
    print(f"  Found {is_meta_sloop.sum().item()} meta-sLoop configurations")

    return {
        'X': X.cpu().numpy(),
        'Y': Y.cpu().numpy(),
        'stability': stability.cpu().numpy(),
        'depth': depth.cpu().numpy().astype(float),
        'beta': beta.cpu().numpy(),
        'is_meta_sloop': is_meta_sloop.cpu().numpy()
    }


# =============================================================================
# PART 4: G* CONNECTION (CPU - not parallelizable)
# =============================================================================

def compute_gstar_constants():
    """Compute and display G* derived constants."""
    print("\n[G* Connection] Computing lemniscatic constants...")

    GAMMA_QUARTER = special.gamma(0.25)
    G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

    A_COEFF = 16 * G_STAR**2
    B_COEFF = 16 * G_STAR**3
    DISCRIMINANT = A_COEFF**2 - 4 * B_COEFF

    X_PLUS = (A_COEFF + np.sqrt(DISCRIMINANT)) / 2
    X_MINUS = (A_COEFF - np.sqrt(DISCRIMINANT)) / 2

    print(f"  G* = {G_STAR:.10f}")
    print(f"  x+ = {X_PLUS:.6f} (1/alpha)")
    print(f"  x- = {X_MINUS:.6f} (N_c)")
    print(f"  alpha = {1/X_PLUS:.10f}")

    return {
        'G_star': G_STAR,
        'x_plus': X_PLUS,
        'x_minus': X_MINUS,
        'alpha': 1/X_PLUS
    }


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_fractal_colormap():
    """Create a colormap suitable for fractal visualization."""
    colors = [
        (0.0, 0.0, 0.2), (0.0, 0.3, 0.6), (0.0, 0.6, 0.8),
        (0.2, 0.8, 0.4), (0.8, 0.8, 0.0), (1.0, 0.4, 0.0),
        (0.8, 0.0, 0.2), (0.4, 0.0, 0.4), (0.0, 0.0, 0.0),
    ]
    return LinearSegmentedColormap.from_list('fractal', colors, N=256)


def save_all_figures(basin_results, julia_results, meta_results, gstar_results):
    """Generate and save all visualization figures."""
    print("\n[Visualization] Generating figures...")

    cmap = create_fractal_colormap()

    # Figure 1: Basin structure
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    X, Y, fate_map, escape_time = basin_results

    im0 = axes[0].pcolormesh(X, Y, fate_map, cmap='viridis', shading='auto')
    axes[0].set_title('Basin of Attraction', fontsize=14)
    axes[0].set_xlabel(r'$J_x$')
    axes[0].set_ylabel(r'$J_y$')
    plt.colorbar(im0, ax=axes[0], label='Fate (0=evap, 1=stable, 2=div)')

    # Find boundary
    boundary = np.abs(np.diff(fate_map, axis=0, prepend=fate_map[0:1])) + \
               np.abs(np.diff(fate_map, axis=1, prepend=fate_map[:, 0:1]))
    boundary = boundary > 0

    axes[1].imshow(boundary, extent=[X.min(), X.max(), Y.min(), Y.max()],
                   origin='lower', cmap='hot')

    # Estimate fractal dimension
    boundary_tensor = torch.from_numpy(boundary.astype(np.float32))
    frac_dim = compute_fractal_dimension_gpu(boundary_tensor)

    axes[1].set_title(f'Basin Boundary (D ≈ {frac_dim:.3f})', fontsize=14)
    axes[1].set_xlabel(r'$J_x$')
    axes[1].set_ylabel(r'$J_y$')

    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'basin_structure.png'), dpi=200)
    plt.close()
    print(f"  Saved basin_structure.png (fractal dim = {frac_dim:.3f})")

    # Figure 2: Julia set
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    X_j, Y_j, escape_j = julia_results

    im1 = axes[0].imshow(escape_j, extent=[X_j.min(), X_j.max(), Y_j.min(), Y_j.max()],
                         origin='lower', cmap=cmap, aspect='equal')
    axes[0].set_title('TRD Julia Set', fontsize=14)
    axes[0].set_xlabel(r'Re($\psi$)')
    axes[0].set_ylabel(r'Im($\psi$)')
    plt.colorbar(im1, ax=axes[0], label='Escape Time')

    # Experience map
    beta = compute_experience_map_gpu(torch.from_numpy(escape_j))

    im2 = axes[1].imshow(beta.cpu().numpy(),
                         extent=[X_j.min(), X_j.max(), Y_j.min(), Y_j.max()],
                         origin='lower', cmap='hot', aspect='equal')
    axes[1].set_title('Experience Intensity (β)', fontsize=14)
    axes[1].set_xlabel(r'Re($\psi$)')
    axes[1].set_ylabel(r'Im($\psi$)')
    plt.colorbar(im2, ax=axes[1], label='β')

    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'julia_set.png'), dpi=200)
    plt.close()
    print("  Saved julia_set.png")

    # Figure 3: Meta-sLoop detection
    fig, axes = plt.subplots(2, 2, figsize=(14, 14))

    X_m, Y_m = np.meshgrid(meta_results['X'][0, :], meta_results['Y'][:, 0])

    im0 = axes[0, 0].pcolormesh(X_m, Y_m, meta_results['stability'],
                                 cmap='viridis', shading='auto')
    axes[0, 0].set_title('Model Stability', fontsize=14)
    plt.colorbar(im0, ax=axes[0, 0])

    im1 = axes[0, 1].pcolormesh(X_m, Y_m, meta_results['depth'],
                                 cmap='plasma', shading='auto')
    axes[0, 1].set_title('Self-Modeling Depth', fontsize=14)
    plt.colorbar(im1, ax=axes[0, 1])

    im2 = axes[1, 0].pcolormesh(X_m, Y_m, meta_results['beta'],
                                 cmap='hot', shading='auto')
    axes[1, 0].set_title('Boundary Proximity (β)', fontsize=14)
    plt.colorbar(im2, ax=axes[1, 0])

    im3 = axes[1, 1].pcolormesh(X_m, Y_m, meta_results['is_meta_sloop'].astype(float),
                                 cmap='Greens', shading='auto')
    axes[1, 1].set_title('Meta-sLoop Regions', fontsize=14)

    # Add K_B circle
    theta = np.linspace(0, 2*np.pi, 100)
    K_B = 1.2
    for ax in axes.flat:
        ax.plot(K_B * np.cos(theta), K_B * np.sin(theta), 'w--',
                linewidth=1, alpha=0.7)
        ax.set_xlabel(r'$J_x$')
        ax.set_ylabel(r'$J_y$')
        ax.set_aspect('equal')

    plt.suptitle('Meta-sLoop Detection in Configuration Space', fontsize=16)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'meta_sloop.png'), dpi=200)
    plt.close()
    print("  Saved meta_sloop.png")

    # Figure 4: G* quadratic
    fig, ax = plt.subplots(figsize=(10, 6))

    G_STAR = gstar_results['G_star']
    A = 16 * G_STAR**2
    B = 16 * G_STAR**3

    x = np.linspace(-10, 150, 1000)
    y = x**2 - A * x + B

    ax.plot(x, y, 'b-', linewidth=2)
    ax.axhline(0, color='gray', linestyle='--')
    ax.axvline(gstar_results['x_plus'], color='r', linestyle=':',
               label=f"x+ = {gstar_results['x_plus']:.2f} (1/alpha)")
    ax.axvline(gstar_results['x_minus'], color='g', linestyle=':',
               label=f"x- = {gstar_results['x_minus']:.2f} (N_c)")

    ax.set_xlabel('x', fontsize=12)
    ax.set_ylabel(r'$f(x) = x^2 - 16(G^*)^2 x + 16(G^*)^3$', fontsize=12)
    ax.set_title(f'Master Quadratic (G* = {G_STAR:.6f})', fontsize=14)
    ax.legend()
    ax.set_ylim(-1000, 5000)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'master_quadratic.png'), dpi=200)
    plt.close()
    print("  Saved master_quadratic.png")

    return frac_dim


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 70)
    print("MANDELBROT-CONSCIOUSNESS CONJECTURE INVESTIGATION")
    print("GPU-Accelerated Analysis on RTX 5090")
    print("=" * 70)

    total_start = time.time()

    # Run all investigations
    basin_results = compute_basin_map_gpu(resolution=500, max_ticks=500)
    julia_results = compute_julia_set_gpu(resolution=1000, max_iter=200)
    meta_results = scan_meta_sloop_gpu(resolution=100, n_steps=200)
    gstar_results = compute_gstar_constants()

    # Generate visualizations
    frac_dim = save_all_figures(basin_results, julia_results, meta_results, gstar_results)

    # Summary statistics
    print("\n" + "=" * 70)
    print("INVESTIGATION COMPLETE")
    print("=" * 70)

    total_elapsed = time.time() - total_start

    print(f"\nResults saved to: {OUTPUT_DIR}/")
    print(f"Total runtime: {total_elapsed:.2f}s")

    print(f"\nKey Findings:")
    print(f"  Basin boundary fractal dimension: {frac_dim:.4f}")
    print(f"  (D > 1 indicates fractal structure)")

    n_meta = meta_results['is_meta_sloop'].sum()
    total = meta_results['is_meta_sloop'].size
    print(f"  Meta-sLoop configurations: {n_meta}/{total} ({100*n_meta/total:.1f}%)")

    # Correlation between depth and beta
    depth_flat = meta_results['depth'].flatten()
    beta_flat = meta_results['beta'].flatten()
    correlation = np.corrcoef(depth_flat, beta_flat)[0, 1]
    print(f"  Depth-boundary correlation: {correlation:.3f}")

    print(f"\n  G* = {gstar_results['G_star']:.10f}")
    print(f"  1/alpha = {gstar_results['x_plus']:.6f}")
    print(f"  N_c = {gstar_results['x_minus']:.6f}")

    return {
        'fractal_dimension': frac_dim,
        'meta_sloop_count': n_meta,
        'depth_beta_correlation': correlation,
        'gstar': gstar_results
    }


if __name__ == "__main__":
    results = main()
