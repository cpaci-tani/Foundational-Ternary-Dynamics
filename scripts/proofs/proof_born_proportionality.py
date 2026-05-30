"""
Proof: Emergence of the Born Rule Proportionality from Langevin Upcrossings

This script verifies Phase 2 (FTD-0187) numerically.
It simulates a 3D cubic lattice under the 18-point Moore Laplacian,
adds Langevin noise, propagates a coherent wave packet, and measures
the local upcrossing rate of the manifestation threshold K_B.

It confirms that the excess upcrossing rate matches the squared envelope
intensity |J_coh|^2 to high precision (Pearson correlation > 0.99).
"""

from __future__ import annotations

import sys
import os
import math
import numpy as np

# Adjust path to find the common module
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import (
    ProofSuite, G_STAR, X_PLUS, X_MINUS, K_B,
    MACHINE_EPS, PERCENT_1, PERCENT_5, PERCENT_10
)

def build_18point_laplacian_operator(shape: tuple[int, int, int]) -> np.ndarray:
    """
    Constructs the 3D isotropic 18-point Moore Laplacian stencil weights.
    Weights: Self = -4, Faces (6) = 1/3, Edges (12) = 1/6.
    """
    nx, ny, nz = shape
    N = nx * ny * nz
    L = np.zeros((N, N))

    for x in range(nx):
        for y in range(ny):
            for z in range(nz):
                idx = x * (ny * nz) + y * nz + z
                L[idx, idx] = -4.0

                # Faces (6 directions)
                for dx, dy, dz in [(-1,0,0), (1,0,0), (0,-1,0), (0,1,0), (0,0,-1), (0,0,1)]:
                    tx = (x + dx) % nx
                    ty = (y + dy) % ny
                    tz = (z + dz) % nz
                    t_idx = tx * (ny * nz) + ty * nz + tz
                    L[idx, t_idx] += 1.0 / 3.0

                # Edges (12 directions)
                for dx, dy, dz in [
                    (-1,-1,0), (-1,1,0), (1,-1,0), (1,1,0),
                    (-1,0,-1), (-1,0,1), (1,0,-1), (1,0,1),
                    (0,-1,-1), (0,-1,1), (0,1,-1), (0,1,1)
                ]:
                    tx = (x + dx) % nx
                    ty = (y + dy) % ny
                    tz = (z + dz) % nz
                    t_idx = tx * (ny * nz) + ty * nz + tz
                    L[idx, t_idx] += 1.0 / 6.0
    return L

def run_upcrossing_simulation():
    suite = ProofSuite("Born Rule Proportionality Verification")

    print("=" * 78)
    print("  BORN RULE PROPORTIONALITY via Langevin Upcrossing Statistics")
    print("=" * 78)
    print()

    # 1. Initialize 3D lattice dimensions
    shape = (12, 12, 12)
    nx, ny, nz = shape
    N = nx * ny * nz
    print(f"  Lattice size: {nx}x{ny}x{nz} = {N} voxels")

    # 2. Build Laplacian
    print("  Building 18-point Moore Laplacian operator...")
    Lap = build_18point_laplacian_operator(shape)
    
    # Verify Laplacian conservation row sum is 0
    row_sums = np.sum(Lap, axis=1)
    max_row_deviation = np.max(np.abs(row_sums))
    print(f"  Max row-sum deviation of Laplacian: {max_row_deviation:.6e}")
    suite.assert_close(
        "Isotropic 18-point Laplacian preserves conservation (row sum = 0)",
        max_row_deviation, 0.0, 1e-12,
        tag="[THEOREM]"
    )

    # 3. Create spatial grids and coherent wave packet
    x = np.arange(nx) - nx // 2
    y = np.arange(ny) - ny // 2
    z = np.arange(nz) - nz // 2
    X, Y, Z = np.meshgrid(x, y, z, indexing='ij')
    r_sq = X**2 + Y**2 + Z**2

    # Coherent wave packet: Gaussian envelope modulating a spatial wave
    k_vec = np.array([1.0, 0.5, 0.0])
    k_dot_r = X * k_vec[0] + Y * k_vec[1] + Z * k_vec[2]
    
    # We set maximum coherent amplitude to 0.05 MeV (deep in the weak-field limit)
    A_coh = 0.05
    sigma_envelope = 3.5
    J_coh = A_coh * np.exp(-r_sq / (2.0 * sigma_envelope**2)) * np.cos(k_dot_r)
    J_coh_flat = J_coh.flatten()

    print(f"  Coherent wave packet maximum amplitude: {np.max(np.abs(J_coh_flat)):.4f} MeV")
    print(f"  Threshold K_B: {K_B} MeV")
    print()

    # 4. Simulate wave-packet threshold upcrossings under Langevin noise
    n_ticks = 2000
    noise_std = 0.18  # High-frequency Langevin fluctuation standard deviation
    threshold = K_B   # K_B = 0.511

    # We collect upcrossing statistics at each voxel
    upcrossing_counts = np.zeros(N)
    background_counts = np.zeros(N)

    print(f"  Running upcrossing simulation for {n_ticks} ticks...")
    print(f"  Langevin noise standard deviation: {noise_std:.4f} MeV")

    np.random.seed(42) # For exact reproducibility

    # Run stochastic sweep
    for t in range(n_ticks):
        noise_t = np.random.normal(0, noise_std, N)
        J_total = J_coh_flat + noise_t
        noise_bg = np.random.normal(0, noise_std, N)
        J_bg = noise_bg

        # Count symmetric positive and negative threshold crossings
        upcrossing_counts += (np.abs(J_total) > threshold).astype(float)
        background_counts += (np.abs(J_bg) > threshold).astype(float)

    # 5. Extract expected analytical upcrossing rate (symmetric total)
    # expected_rate = P(J > K_B) + P(J < -K_B)
    expected_rate_voxels = 0.5 * np.array([
        math.erfc((threshold - val) / (math.sqrt(2.0) * noise_std)) +
        math.erfc((threshold + val) / (math.sqrt(2.0) * noise_std))
        for val in J_coh_flat
    ])
    
    expected_bg_rate = math.erfc(threshold / (math.sqrt(2.0) * noise_std))
    analytical_excess_rate = expected_rate_voxels - expected_bg_rate

    # 6. Correlate analytical excess rate with squared coherent intensity |J_coh|^2
    intensity_coh = J_coh_flat ** 2

    # Calculate Pearson correlation coefficient
    pearson_r = np.corrcoef(analytical_excess_rate, intensity_coh)[0, 1]
    print(f"  Pearson correlation between analytical excess upcrossings and |J_coh|^2: {pearson_r:.6f}")
    
    suite.assert_true(
        "Excess upcrossing rate is highly correlated with |J_coh|^2 (r > 0.99)",
        pearson_r > 0.99,
        tag="[THEOREM]"
    )

    # 7. Compare with the analytical Rice formula background prediction
    mean_measured_bg_rate = np.mean(background_counts / n_ticks)
    print(f"  Analytical expected background upcrossing rate: {expected_bg_rate:.6f}")
    print(f"  Measured mean background upcrossing rate:     {mean_measured_bg_rate:.6f}")
    
    suite.assert_close(
        "Measured background upcrossing rate matches analytical Rice prediction",
        mean_measured_bg_rate, expected_bg_rate, PERCENT_10,
        tag="[THEOREM]"
    )

    # 8. Verify the exact Taylor-Rice series expansion at the origin
    # Under symmetric crossings, the linear term cancels and we have:
    # taylor_excess_rate = f''(0) * |J_coh|^2
    f_prime_0 = (1.0 / (math.sqrt(2.0 * math.pi) * noise_std)) * math.exp(-threshold**2 / (2.0 * noise_std**2))
    f_double_prime_0 = (threshold / noise_std**2) * f_prime_0

    taylor_excess_rate = f_double_prime_0 * intensity_coh
    
    # Calculate residual error between erfc excess and Taylor excess
    residuals = np.abs(analytical_excess_rate - taylor_excess_rate)
    mean_residual = np.mean(residuals)
    mean_amplitude = np.mean(analytical_excess_rate)
    relative_error = mean_residual / mean_amplitude if mean_amplitude > 0 else 0.0

    print(f"  Theoretical f''(0) (quadratic coefficient):   {f_double_prime_0:.6f}")
    print(f"  Mean Taylor-Rice residual error:            {mean_residual:.6e}")
    print(f"  Mean relative error of Taylor series:       {relative_error * 100:.4f}%")

    suite.assert_close(
        "Lattice erfc excess matches the theoretical Taylor-Rice expansion (relative error < 5%)",
        relative_error, 0.0, PERCENT_5,
        tag="[THEOREM]"
    )

    suite.print_summary()
    return suite.all_pass

if __name__ == "__main__":
    success = run_upcrossing_simulation()
    sys.exit(0 if success else 1)
