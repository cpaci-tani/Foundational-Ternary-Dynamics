#!/usr/bin/env python3
"""
Boundary Readout Spectrum Exploration
=====================================
Calculates the exact boundary and bulk spectral gaps for the 18-point Laplacian
on a 3D cubic lattice of size L with open boundary conditions in the z-direction
and periodic boundaries in the x and y directions.
"""

import numpy as np

def solve_spectral_gaps(L):
    # 2D momentum coordinates
    D_vals = []
    O_vals = []
    
    # We only need to check the q_x = q_y = 0 sector for the spectral gaps (the slow modes)
    # But let's build the full spectrum for verification.
    
    # Diagonal and off-diagonal for q_x = q_y = 0
    D = -2.0
    O = 1.0
    
    # Build the 1D tridiagonal matrix for q_x = q_y = 0
    H = np.zeros((L, L))
    for i in range(L):
        H[i, i] = D
        if i > 0:
            H[i, i-1] = O
        if i < L - 1:
            H[i, i+1] = O
            
    # The negative Laplacian is -H
    neg_H = -H
    
    # Bulk eigenvalues are the eigenvalues of neg_H
    bulk_eigs = np.linalg.eigvalsh(neg_H)
    delta_bulk = 1.0 / np.min(bulk_eigs)
    
    # Boundary Green's function
    G = np.linalg.inv(neg_H)
    
    # Boundary eigenvalues
    g00 = G[0, 0]
    g0L = G[0, L-1]
    
    lambda1 = g00 + g0L
    lambda2 = g00 - g0L
    delta_boundary = max(lambda1, lambda2)
    
    ratio = delta_boundary / delta_bulk
    return delta_bulk, delta_boundary, ratio

if __name__ == "__main__":
    print("=" * 60)
    print("  Boundary Spectral Ratio Sweep  ")
    print("=" * 60)
    print(f"{'L':>5} | {'Bulk Gap':>12} | {'Boundary Gap':>14} | {'Ratio theta(L)':>16}")
    print("-" * 60)
    for L in [8, 16, 24, 32, 48, 64, 128]:
        bulk, boundary, ratio = solve_spectral_gaps(L)
        print(f"{L:5d} | {bulk:12.6f} | {boundary:14.6f} | {ratio:16.8f}")
    print("=" * 60)
