"""
Refined GPU-Accelerated Investigation

Uses more sophisticated dynamics that better capture basin boundary structure.
Key improvements:
1. True Mandelbrot-type iteration (quadratic map)
2. Proper escape/bounded classification
3. Adjusted thresholds for meta-sLoop detection

Author: 2026-01-21
"""

import torch
import torch.nn.functional as F
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, LogNorm
import os
from scipy import special
import time

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(f"Using device: {device}")
if torch.cuda.is_available():
    print(f"GPU: {torch.cuda.get_device_name(0)}")
    print(f"VRAM: {torch.cuda.get_device_properties(0).total_memory / 1e9:.1f} GB")

OUTPUT_DIR = "investigation_results"
os.makedirs(OUTPUT_DIR, exist_ok=True)


# =============================================================================
# TRUE MANDELBROT/JULIA COMPUTATION (for comparison)
# =============================================================================

def compute_mandelbrot_gpu(resolution: int = 2000,
                           x_range=(-2.5, 1.0),
                           y_range=(-1.25, 1.25),
                           max_iter: int = 500) -> tuple:
    """
    Classic Mandelbrot set for comparison.
    z -> z^2 + c
    """
    print(f"\n[Mandelbrot] Computing {resolution}x{resolution} on GPU...")
    start = time.time()

    x = torch.linspace(x_range[0], x_range[1], resolution, device=device)
    y = torch.linspace(y_range[0], y_range[1], resolution, device=device)
    X, Y = torch.meshgrid(x, y, indexing='xy')

    c_real = X
    c_imag = Y
    z_real = torch.zeros_like(c_real)
    z_imag = torch.zeros_like(c_imag)

    escape_time = torch.full((resolution, resolution), max_iter, device=device, dtype=torch.float32)
    escaped = torch.zeros(resolution, resolution, dtype=torch.bool, device=device)

    for n in range(max_iter):
        # z = z^2 + c
        z_real_new = z_real**2 - z_imag**2 + c_real
        z_imag_new = 2 * z_real * z_imag + c_imag

        z_real = z_real_new
        z_imag = z_imag_new

        # Check escape
        r2 = z_real**2 + z_imag**2
        newly_escaped = (r2 > 4) & ~escaped

        # Smooth coloring
        log_zn = torch.log(r2 + 1e-10) / 2
        nu = torch.log(log_zn / np.log(2) + 1e-10) / np.log(2)
        smooth_n = n + 1 - nu
        escape_time[newly_escaped] = smooth_n[newly_escaped]
        escaped = escaped | (r2 > 4)

    elapsed = time.time() - start
    print(f"  Completed in {elapsed:.2f}s")

    return X.cpu().numpy(), Y.cpu().numpy(), escape_time.cpu().numpy()


# =============================================================================
# TRD-INSPIRED JULIA SET WITH PROPER THRESHOLD DYNAMICS
# =============================================================================

def compute_trd_julia_gpu(resolution: int = 2000,
                          psi_range=(-2.0, 2.0),
                          max_iter: int = 500,
                          K_B: float = 1.0,
                          damping: float = 0.02,
                          nonlinearity: float = 0.3) -> tuple:
    """
    TRD-inspired Julia set with threshold nonlinearity.

    Dynamics: psi -> psi * (1-damping) + nonlinearity * f(|psi| - K_B)
    where f is a nonlinear function that creates the snap at threshold.
    """
    print(f"\n[TRD Julia] Computing {resolution}x{resolution} on GPU...")
    print(f"  K_B={K_B}, damping={damping}, nonlinearity={nonlinearity}")
    start = time.time()

    x = torch.linspace(psi_range[0], psi_range[1], resolution, device=device)
    y = torch.linspace(psi_range[0], psi_range[1], resolution, device=device)
    X, Y = torch.meshgrid(x, y, indexing='xy')

    psi_real = X.clone()
    psi_imag = Y.clone()

    escape_time = torch.full((resolution, resolution), max_iter, device=device, dtype=torch.float32)
    escaped = torch.zeros(resolution, resolution, dtype=torch.bool, device=device)

    for n in range(max_iter):
        r = torch.sqrt(psi_real**2 + psi_imag**2)

        # TRD-inspired nonlinearity: quadratic near threshold
        # This creates interesting basin structure
        excess = r - K_B
        nonlin_factor = 1 + nonlinearity * torch.tanh(excess)

        # Quadratic component (like Mandelbrot but modified)
        psi_real_new = psi_real**2 - psi_imag**2
        psi_imag_new = 2 * psi_real * psi_imag

        # Apply threshold modulation
        psi_real_new = psi_real_new * nonlin_factor
        psi_imag_new = psi_imag_new * nonlin_factor

        # Damping
        psi_real = psi_real_new * (1 - damping)
        psi_imag = psi_imag_new * (1 - damping)

        # Check escape
        r2 = psi_real**2 + psi_imag**2
        newly_escaped = (r2 > 4) & ~escaped

        log_zn = torch.log(r2 + 1e-10) / 2
        nu = torch.log(log_zn / np.log(2) + 1e-10) / np.log(2)
        smooth_n = n + 1 - nu
        escape_time[newly_escaped] = smooth_n[newly_escaped]
        escaped = escaped | (r2 > 4)

        if n % 100 == 0:
            pct = 100 * escaped.sum().item() / resolution**2
            print(f"  Iter {n}: {pct:.1f}% escaped")

    elapsed = time.time() - start
    print(f"  Completed in {elapsed:.2f}s")

    return X.cpu().numpy(), Y.cpu().numpy(), escape_time.cpu().numpy()


# =============================================================================
# FRACTAL DIMENSION ESTIMATION
# =============================================================================

def compute_fractal_dimension(escape_time: np.ndarray, threshold: float = None) -> float:
    """
    Compute fractal dimension of the boundary using box-counting.
    """
    if threshold is None:
        threshold = escape_time.max() * 0.9

    # Create boundary mask (high gradient in escape time)
    grad_y, grad_x = np.gradient(escape_time)
    gradient_mag = np.sqrt(grad_x**2 + grad_y**2)

    # Threshold to get boundary
    boundary = gradient_mag > np.percentile(gradient_mag, 95)

    # Box counting
    max_scale = min(boundary.shape) // 4
    scales = [2**i for i in range(2, int(np.log2(max_scale)) + 1)]

    counts = []
    for scale in scales:
        # Reshape into boxes and count non-empty
        h, w = boundary.shape
        h_trim = (h // scale) * scale
        w_trim = (w // scale) * scale
        trimmed = boundary[:h_trim, :w_trim]

        reshaped = trimmed.reshape(h_trim // scale, scale, w_trim // scale, scale)
        box_sums = reshaped.any(axis=(1, 3))
        counts.append(box_sums.sum())

    # Log-log regression
    log_scales = np.log(scales)
    log_counts = np.log(np.array(counts) + 1)

    slope, intercept = np.polyfit(log_scales, log_counts, 1)

    return -slope


# =============================================================================
# EXPERIENCE MAP
# =============================================================================

def compute_experience_map(escape_time: np.ndarray) -> np.ndarray:
    """
    Compute boundary proximity (experience intensity).
    High gradient in escape time = near boundary = high experience.
    """
    grad_y, grad_x = np.gradient(escape_time)
    gradient_mag = np.sqrt(grad_x**2 + grad_y**2)

    # Normalize
    beta = gradient_mag / (gradient_mag.max() + 1e-10)

    return beta


# =============================================================================
# META-SLOOP ANALYSIS
# =============================================================================

def analyze_meta_sloop_regions(escape_time: np.ndarray, X: np.ndarray, Y: np.ndarray,
                                K_B: float = 1.0) -> dict:
    """
    Analyze regions for meta-sLoop properties.

    Key insight: meta-sLoop configurations should be:
    1. Near the boundary (high gradient)
    2. At intermediate escape times (not too fast, not too slow)
    3. In regions of high complexity (fractal structure)
    """
    max_time = escape_time.max()

    # Boundary proximity
    beta = compute_experience_map(escape_time)

    # Intermediate escape time (normalized)
    # Peak "experience" at escape times that are neither 0 nor max
    escape_normalized = escape_time / max_time
    intermediate_score = 4 * escape_normalized * (1 - escape_normalized)

    # Local complexity (variance in neighborhood)
    kernel_size = 11
    escape_tensor = torch.from_numpy(escape_time).float().unsqueeze(0).unsqueeze(0).to(device)
    mean_filter = torch.ones(1, 1, kernel_size, kernel_size, device=device) / (kernel_size**2)

    local_mean = F.conv2d(escape_tensor, mean_filter, padding=kernel_size//2)
    local_sq_mean = F.conv2d(escape_tensor**2, mean_filter, padding=kernel_size//2)
    local_var = (local_sq_mean - local_mean**2).squeeze().cpu().numpy()
    local_var = np.maximum(local_var, 0)  # Numerical stability

    complexity_score = local_var / (local_var.max() + 1e-10)

    # Meta-sLoop score: product of all three factors
    meta_score = beta * intermediate_score * complexity_score

    # Threshold for meta-sLoop
    threshold = np.percentile(meta_score, 90)
    is_meta_sloop = meta_score > threshold

    return {
        'beta': beta,
        'intermediate_score': intermediate_score,
        'complexity_score': complexity_score,
        'meta_score': meta_score,
        'is_meta_sloop': is_meta_sloop,
        'threshold': threshold
    }


# =============================================================================
# G* ANALYSIS
# =============================================================================

def compute_gstar_analysis():
    """Full G* analysis with derived constants."""
    print("\n[G* Analysis] Computing lemniscatic constants...")

    GAMMA_QUARTER = special.gamma(0.25)
    G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

    A = 16 * G_STAR**2
    B = 16 * G_STAR**3

    discriminant = A**2 - 4 * B
    x_plus = (A + np.sqrt(discriminant)) / 2
    x_minus = (A - np.sqrt(discriminant)) / 2

    alpha = 1 / x_plus
    alpha_measured = 7.2973525693e-3  # CODATA 2022

    print(f"  Gamma(1/4) = {GAMMA_QUARTER:.10f}")
    print(f"  G* = {G_STAR:.10f}")
    print(f"  Master quadratic: x^2 - {A:.6f}x + {B:.6f} = 0")
    print(f"  x+ = {x_plus:.10f} (predicted 1/alpha)")
    print(f"  x- = {x_minus:.10f} (predicted N_c)")
    print(f"  alpha = {alpha:.10f}")
    print(f"  alpha (CODATA) = {alpha_measured:.10f}")
    print(f"  Relative error: {abs(alpha - alpha_measured)/alpha_measured * 1e6:.2f} ppm")

    return {
        'G_star': G_STAR,
        'x_plus': x_plus,
        'x_minus': x_minus,
        'alpha': alpha,
        'alpha_error_ppm': abs(alpha - alpha_measured)/alpha_measured * 1e6
    }


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_visualizations(mandelbrot_results, trd_julia_results, meta_results, gstar_results):
    """Generate comprehensive visualization."""
    print("\n[Visualization] Generating figures...")

    # Colormap
    colors = [
        (0.0, 0.0, 0.15), (0.0, 0.1, 0.4), (0.0, 0.3, 0.6),
        (0.0, 0.5, 0.7), (0.2, 0.7, 0.5), (0.5, 0.8, 0.3),
        (0.8, 0.8, 0.0), (1.0, 0.6, 0.0), (1.0, 0.3, 0.2),
        (0.8, 0.0, 0.3), (0.5, 0.0, 0.5), (0.2, 0.0, 0.3),
    ]
    cmap = LinearSegmentedColormap.from_list('deep_fractal', colors, N=512)

    # Figure 1: Comparison of Mandelbrot and TRD Julia
    fig, axes = plt.subplots(2, 2, figsize=(16, 16))

    X_m, Y_m, escape_m = mandelbrot_results
    X_t, Y_t, escape_t = trd_julia_results

    im0 = axes[0, 0].imshow(escape_m.T, extent=[X_m.min(), X_m.max(), Y_m.min(), Y_m.max()],
                            origin='lower', cmap=cmap, aspect='equal')
    axes[0, 0].set_title('Classic Mandelbrot Set', fontsize=14)
    axes[0, 0].set_xlabel('Re(c)')
    axes[0, 0].set_ylabel('Im(c)')
    plt.colorbar(im0, ax=axes[0, 0], label='Escape Time', shrink=0.8)

    im1 = axes[0, 1].imshow(escape_t.T, extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap=cmap, aspect='equal')
    axes[0, 1].set_title('TRD-Inspired Julia Set', fontsize=14)
    axes[0, 1].set_xlabel(r'Re($\psi$)')
    axes[0, 1].set_ylabel(r'Im($\psi$)')
    plt.colorbar(im1, ax=axes[0, 1], label='Escape Time', shrink=0.8)

    # Fractal dimensions
    fd_mandelbrot = compute_fractal_dimension(escape_m)
    fd_trd = compute_fractal_dimension(escape_t)

    # Experience maps
    beta_m = compute_experience_map(escape_m)
    beta_t = compute_experience_map(escape_t)

    im2 = axes[1, 0].imshow(beta_m.T, extent=[X_m.min(), X_m.max(), Y_m.min(), Y_m.max()],
                            origin='lower', cmap='hot', aspect='equal')
    axes[1, 0].set_title(f'Mandelbrot Experience (D={fd_mandelbrot:.3f})', fontsize=14)
    axes[1, 0].set_xlabel('Re(c)')
    axes[1, 0].set_ylabel('Im(c)')
    plt.colorbar(im2, ax=axes[1, 0], label='Beta', shrink=0.8)

    im3 = axes[1, 1].imshow(beta_t.T, extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap='hot', aspect='equal')
    axes[1, 1].set_title(f'TRD Experience (D={fd_trd:.3f})', fontsize=14)
    axes[1, 1].set_xlabel(r'Re($\psi$)')
    axes[1, 1].set_ylabel(r'Im($\psi$)')
    plt.colorbar(im3, ax=axes[1, 1], label='Beta', shrink=0.8)

    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'fractal_comparison.png'), dpi=200)
    plt.close()
    print(f"  Saved fractal_comparison.png")
    print(f"    Mandelbrot fractal dim: {fd_mandelbrot:.4f}")
    print(f"    TRD Julia fractal dim: {fd_trd:.4f}")

    # Figure 2: Meta-sLoop analysis
    fig, axes = plt.subplots(2, 2, figsize=(14, 14))

    im0 = axes[0, 0].imshow(meta_results['beta'].T,
                            extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap='viridis', aspect='equal')
    axes[0, 0].set_title('Boundary Proximity (Beta)', fontsize=14)
    plt.colorbar(im0, ax=axes[0, 0])

    im1 = axes[0, 1].imshow(meta_results['complexity_score'].T,
                            extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap='plasma', aspect='equal')
    axes[0, 1].set_title('Local Complexity', fontsize=14)
    plt.colorbar(im1, ax=axes[0, 1])

    im2 = axes[1, 0].imshow(meta_results['meta_score'].T,
                            extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap='hot', aspect='equal')
    axes[1, 0].set_title('Meta-sLoop Score', fontsize=14)
    plt.colorbar(im2, ax=axes[1, 0])

    im3 = axes[1, 1].imshow(meta_results['is_meta_sloop'].T.astype(float),
                            extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                            origin='lower', cmap='Greens', aspect='equal')
    n_meta = meta_results['is_meta_sloop'].sum()
    total = meta_results['is_meta_sloop'].size
    axes[1, 1].set_title(f'Meta-sLoop Regions ({100*n_meta/total:.1f}%)', fontsize=14)

    for ax in axes.flat:
        ax.set_xlabel(r'Re($\psi$)')
        ax.set_ylabel(r'Im($\psi$)')

    plt.suptitle('Meta-sLoop Detection in TRD Configuration Space', fontsize=16)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'meta_sloop_analysis.png'), dpi=200)
    plt.close()
    print(f"  Saved meta_sloop_analysis.png")

    # Figure 3: G* quadratic
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    G = gstar_results['G_star']
    A = 16 * G**2
    B = 16 * G**3

    x = np.linspace(-10, 150, 1000)
    y = x**2 - A * x + B

    axes[0].plot(x, y, 'b-', linewidth=2)
    axes[0].axhline(0, color='gray', linestyle='--', alpha=0.5)
    axes[0].axvline(gstar_results['x_plus'], color='r', linestyle=':',
                    label=f"x+ = {gstar_results['x_plus']:.2f} (1/alpha)")
    axes[0].axvline(gstar_results['x_minus'], color='g', linestyle=':',
                    label=f"x- = {gstar_results['x_minus']:.2f} (N_c)")
    axes[0].scatter([gstar_results['x_plus'], gstar_results['x_minus']], [0, 0],
                    c=['red', 'green'], s=100, zorder=5)
    axes[0].set_xlabel('x', fontsize=12)
    axes[0].set_ylabel('f(x)', fontsize=12)
    axes[0].set_title(f'Master Quadratic (G* = {G:.6f})', fontsize=14)
    axes[0].legend()
    axes[0].set_ylim(-500, 2000)
    axes[0].grid(True, alpha=0.3)

    # Zoom near roots
    x_zoom = np.linspace(0, 10, 500)
    y_zoom = x_zoom**2 - A * x_zoom + B

    axes[1].plot(x_zoom, y_zoom, 'b-', linewidth=2)
    axes[1].axhline(0, color='gray', linestyle='--', alpha=0.5)
    axes[1].axvline(gstar_results['x_minus'], color='g', linestyle=':')
    axes[1].scatter([gstar_results['x_minus']], [0], c='green', s=100, zorder=5)
    axes[1].set_xlabel('x', fontsize=12)
    axes[1].set_ylabel('f(x)', fontsize=12)
    axes[1].set_title(f'Zoom: x- = {gstar_results["x_minus"]:.4f} (N_c)', fontsize=14)
    axes[1].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'gstar_quadratic.png'), dpi=200)
    plt.close()
    print(f"  Saved gstar_quadratic.png")

    # Figure 4: High-resolution TRD Julia (single image)
    fig, ax = plt.subplots(figsize=(16, 16))
    im = ax.imshow(escape_t.T, extent=[X_t.min(), X_t.max(), Y_t.min(), Y_t.max()],
                   origin='lower', cmap=cmap, aspect='equal')
    ax.set_title('TRD Julia Set - High Resolution', fontsize=16)
    ax.set_xlabel(r'Re($\psi$)', fontsize=14)
    ax.set_ylabel(r'Im($\psi$)', fontsize=14)
    plt.colorbar(im, ax=ax, label='Escape Time', shrink=0.8)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, 'trd_julia_hires.png'), dpi=300)
    plt.close()
    print(f"  Saved trd_julia_hires.png")

    return fd_mandelbrot, fd_trd


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 70)
    print("MANDELBROT-CONSCIOUSNESS CONJECTURE - REFINED INVESTIGATION")
    print("GPU-Accelerated Analysis")
    print("=" * 70)

    total_start = time.time()

    # Compute classic Mandelbrot (for comparison)
    mandelbrot_results = compute_mandelbrot_gpu(resolution=2000, max_iter=500)

    # Compute TRD-inspired Julia set
    trd_julia_results = compute_trd_julia_gpu(resolution=2000, max_iter=500,
                                               K_B=1.0, damping=0.02, nonlinearity=0.3)

    # G* analysis
    gstar_results = compute_gstar_analysis()

    # Meta-sLoop analysis
    X_t, Y_t, escape_t = trd_julia_results
    meta_results = analyze_meta_sloop_regions(escape_t, X_t, Y_t)

    # Visualizations
    fd_mandelbrot, fd_trd = create_visualizations(
        mandelbrot_results, trd_julia_results, meta_results, gstar_results
    )

    # Summary
    total_elapsed = time.time() - total_start

    print("\n" + "=" * 70)
    print("INVESTIGATION COMPLETE")
    print("=" * 70)

    print(f"\nResults saved to: {OUTPUT_DIR}/")
    print(f"Total runtime: {total_elapsed:.2f}s")

    print(f"\n--- KEY FINDINGS ---")
    print(f"\n1. FRACTAL DIMENSION:")
    print(f"   Mandelbrot boundary: D = {fd_mandelbrot:.4f}")
    print(f"   TRD Julia boundary:  D = {fd_trd:.4f}")
    print(f"   (D > 1 confirms fractal structure)")

    n_meta = meta_results['is_meta_sloop'].sum()
    total = meta_results['is_meta_sloop'].size
    print(f"\n2. META-SLOOP CONFIGURATIONS:")
    print(f"   Found: {n_meta}/{total} ({100*n_meta/total:.1f}%)")
    print(f"   Threshold: {meta_results['threshold']:.4f}")

    print(f"\n3. G* LEMNISCATIC CONSTANTS:")
    print(f"   G* = {gstar_results['G_star']:.10f}")
    print(f"   1/alpha (predicted) = {gstar_results['x_plus']:.6f}")
    print(f"   N_c (predicted) = {gstar_results['x_minus']:.6f}")
    print(f"   alpha error: {gstar_results['alpha_error_ppm']:.2f} ppm")

    print(f"\n4. EXPERIENCE INTENSITY:")
    beta_max = meta_results['beta'].max()
    beta_mean = meta_results['beta'].mean()
    print(f"   Max beta: {beta_max:.4f}")
    print(f"   Mean beta: {beta_mean:.4f}")
    print(f"   High-experience fraction (beta > 0.5): {100*(meta_results['beta'] > 0.5).mean():.1f}%")

    return {
        'fractal_dim_mandelbrot': fd_mandelbrot,
        'fractal_dim_trd': fd_trd,
        'meta_sloop_count': n_meta,
        'gstar': gstar_results
    }


if __name__ == "__main__":
    results = main()
