#!/usr/bin/env python3
"""
explore_alpha_readout_kernel.py
Numerical experiment to construct the covariant Moore Laplacian Delta_{A_J} 
around a $2\pi$ defect, invert it, and extract the projected 2x2 response matrix.
"""

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla
from scipy.special import gamma

def get_idx(x, y, z, L):
    return x * L * L + y * L + z

def get_theta(x, y, c):
    if x == c and y == c:
        return 0.0
    return np.arctan2(y - c, x - c)

def build_laplacian(L, use_defect=True, epsilon=1e-5):
    c = L // 2
    N = L**3
    
    rows = []
    cols = []
    data = []
    
    # 18-point Moore stencil offsets and weights
    # 6 faces (weight 2)
    faces = []
    for dx, dy, dz in [(1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1)]:
        faces.append( (dx, dy, dz, 2.0) )
        
    # 12 edges (weight 1)
    edges = []
    for a1 in range(3):
        for a2 in range(a1 + 1, 3):
            for s1 in (1, -1):
                for s2 in (1, -1):
                    offset = [0, 0, 0]
                    offset[a1] = s1
                    offset[a2] = s2
                    edges.append( (offset[0], offset[1], offset[2], 1.0) )
                    
    offsets = faces + edges
    
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = get_idx(x, y, z, L)
                
                theta_i = get_theta(x, y, c)
                
                diag_sum = 0.0
                
                for dx, dy, dz, weight in offsets:
                    nx, ny, nz = x + dx, y + dy, z + dz
                    
                    # Neumann boundaries: if outside, it reflects, so J_nx = J_x, no flux flows.
                    # We only add to diagonal if the neighbor exists.
                    if 0 <= nx < L and 0 <= ny < L and 0 <= nz < L:
                        j = get_idx(nx, ny, nz, L)
                        
                        if use_defect:
                            if (x == c and y == c) or (nx == c and ny == c):
                                continue
                                
                            theta_j = get_theta(nx, ny, c)
                            phase = np.exp(1j * (theta_j - theta_i))
                        else:
                            phase = 1.0
                            
                        rows.append(i)
                        cols.append(j)
                        data.append(weight * phase)
                        diag_sum -= weight
                
                # Diagonal
                rows.append(i)
                cols.append(i)
                data.append(diag_sum - epsilon)

    return sp.coo_matrix((data, (rows, cols)), shape=(N, N)).tocsc()

def main():
    G_star = gamma(0.25) / gamma(0.75)
    T_target = 16 * G_star**2
    D_target = 16 * G_star**3
    
    print("=" * 60)
    print("FTD Alpha Readout Kernel Evaluator")
    print("=" * 60)
    print(f"Target 16G*^2 (Trace)       : {T_target:.6f}")
    print(f"Target 16G*^3 (Determinant) : {D_target:.6f}")
    print("-" * 60)
    
    L = 3
    epsilon = 1e-4
    c = 1
    
    print(f"Lattice size: {L}^3, Defect at ({c},{c},z), epsilon={epsilon}")
    
    # Baseline (No Defect)
    L_vacuum = build_laplacian(L, use_defect=False, epsilon=epsilon)
    
    # We need to compute W_U = Pi K_AJ Pi.
    # We will project onto the two transverse states adjacent to the core:
    # Say, state 1: (c+1, c, c) and state 2: (c, c+1, c)
    # The Green's function elements are K_{11}, K_{12}, K_{21}, K_{22}
    
    idx_1 = get_idx(c+1, c, c, L)
    idx_2 = get_idx(c, c+1, c, L)
    
    def extract_W(L_matrix):
        # We solve L_matrix * x = b for b_1 and b_2
        b1 = np.zeros(L**3, dtype=complex)
        b1[idx_1] = 1.0
        x1 = spla.spsolve(L_matrix, b1)
        
        b2 = np.zeros(L**3, dtype=complex)
        b2[idx_2] = 1.0
        x2 = spla.spsolve(L_matrix, b2)
        
        W = np.zeros((2, 2), dtype=complex)
        W[0, 0] = x1[idx_1]
        W[1, 0] = x1[idx_2]
        W[0, 1] = x2[idx_1]
        W[1, 1] = x2[idx_2]
        
        # Multiply by -1 since we defined Laplacian with negative diagonal
        return -W

    W_vac = extract_W(L_vacuum)
    print("\nVACUUM SECTOR (Trivial Holonomy):")
    print(f"  Tr(W_vac) = {np.trace(W_vac):.6f}")
    print(f"  Det(W_vac) = {np.linalg.det(W_vac):.6f}")
    
    L_defect = build_laplacian(L, use_defect=True, epsilon=epsilon)
    W_def = extract_W(L_defect)
    print("\nDEFECT SECTOR (2*pi Holonomy):")
    print(f"  Tr(W_def) = {np.trace(W_def):.6f}")
    print(f"  Det(W_def) = {np.linalg.det(W_def):.6f}")

if __name__ == "__main__":
    main()
