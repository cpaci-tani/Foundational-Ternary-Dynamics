"""
Derive Manifestation Action Cost
======================================================
This script mathematically defines the exact latent energy cost
of a manifestation gate (Genesis/Evaporation) in the FTD engine.

Because `phase_write` sets `s=1` without extracting local flux,
the subsequent Gauss projection phase will stretch the continuous
J field to satisfy `div J = s`.

This script isolates a single source in a periodic N^3 lattice,
computes the resulting G18-projected longitudinal flux field,
and measures the exact macro-energy cost E = 1/2 sum(J^2) injected
into the continuous substrate.
"""

import numpy as np
import scipy.sparse as sps
import scipy.special as spc
import math

# Lemniscatic constant G*
G_star = 2 * np.sqrt(2) * (spc.gamma(0.25)**2 / (4 * np.sqrt(np.pi))) / np.sqrt(np.pi)

# Watson Integral W_BCC (often proportional to self-energy in 3D lattices)
# W_BCC = (Gamma(1/4)^4) / (4 * pi^3) = G_star^2 / (2 * pi) * wait, actually
W_BCC = (spc.gamma(0.25)**4) / (4 * np.pi**3)
print(f"Target Constants: G* = {G_star:.6f}, W_BCC = {W_BCC:.6f}")

def build_operators(N):
    V = N**3
    
    # 1D shift operator
    row = np.arange(N)
    col = (row + 1) % N
    data = np.ones(N)
    S1 = sps.csr_matrix((data, (row, col)), shape=(N, N))
    I1 = sps.eye(N)
    
    # 3D shifts
    Sx = sps.kron(S1, sps.kron(I1, I1))
    Sy = sps.kron(I1, sps.kron(S1, I1))
    Sz = sps.kron(I1, sps.kron(I1, S1))
    
    # Forward and backward differences
    Dx_f = Sx - sps.eye(V)
    Dx_b = sps.eye(V) - Sx.T
    
    Dy_f = Sy - sps.eye(V)
    Dy_b = sps.eye(V) - Sy.T
    
    Dz_f = Sz - sps.eye(V)
    Dz_b = sps.eye(V) - Sz.T
    
    # The G18 discrete Laplacian (c=0)
    # L_face = Sx + Sx.T + Sy + Sy.T + Sz + Sz.T - 6I
    # L_edge = Sx Sy + Sx.T Sy.T + ... - 12I
    L_face = (Sx + Sx.T + Sy + Sy.T + Sz + Sz.T - 6 * sps.eye(V))
    L_edge = (
        Sx@Sy + Sx.T@Sy.T + Sx@Sy.T + Sx.T@Sy +
        Sy@Sz + Sy.T@Sz.T + Sy@Sz.T + Sy.T@Sz +
        Sx@Sz + Sx.T@Sz.T + Sx@Sz.T + Sx.T@Sz - 12 * sps.eye(V)
    )
    
    a = 1.0/3.0
    b = 1.0/6.0
    L18 = a * L_face + b * L_edge
    
    # Divergence operator matching G18?
    # Actually, the FTD engine constructs the G18 Laplacian directly in the solver.
    # The gradient is taken using the same Moore shell weights.
    # The action energy E = 1/2 <phi, -L phi> if div J = -L phi.
    # Wait, E = 1/2 sum J^2.
    # If J = - grad_G18 phi, then sum J^2 = sum phi (-L18 phi).
    # Because L18 = grad_G18^T * grad_G18.
    # Therefore, E = 1/2 * source^T * L18_pinv * source !
    
    return L18

def measure_genesis_cost(N):
    print(f"\nEvaluating Manifestation Action Cost for N = {N}...")
    V = N**3
    L18 = build_operators(N)
    
    L_dense = L18.toarray()
    
    # To find the Green's function, we compute the pseudo-inverse
    # The Laplacian is negative semi-definite. E = 1/2 * s^T (-L)^(-1) s
    L_pinv = np.linalg.pinv(-L_dense)
    
    # Single point source +1 at origin
    s = np.zeros(V)
    s[0] = 1.0
    
    # E = 1/2 * s^T * L_pinv * s
    # Since s is 1 at origin and 0 elsewhere, E is simply 1/2 * L_pinv[0, 0]
    E_manifest = 0.5 * s @ (L_pinv @ s)
    
    print(f"Latent Energy Cost E_manifest = {E_manifest:.6f}")
    
    # Calculate G_00 (the Green's function at the origin)
    G_00 = L_pinv[0, 0]
    print(f"G_00 (Self-Energy Constant) = {G_00:.6f}")
    
    return G_00, E_manifest

if __name__ == "__main__":
    for N in [8, 12, 16, 20]:
        measure_genesis_cost(N)
        
    from scipy import integrate

    def integrand(kx, ky, kz):
        # -sigma_18(k)
        a = 1.0/3.0
        b = 1.0/6.0
        face = 2*a * (np.cos(kx) + np.cos(ky) + np.cos(kz) - 3)
        edge = 4*b * (np.cos(kx)*np.cos(ky) + np.cos(ky)*np.cos(kz) + np.cos(kz)*np.cos(kx) - 3)
        val = -(face + edge)
        if val < 1e-10:
            return 0.0
        return 1.0 / val

    print("\nComputing exact infinite-volume integral W_18...")
    # Because of symmetries, we can integrate over [0, pi]^3 and multiply by 8
    # However, nquad is easier. We use tplquad
    res, err = integrate.tplquad(integrand, 0, np.pi, 0, np.pi, 0, np.pi, epsabs=1e-4, epsrel=1e-4)
    W_18 = (res * 8) / (8 * np.pi**3)
    print(f"Infinite-volume Self-Energy W_18 = {W_18:.6f}")
    print(f"Infinite-volume Latent Energy Cost = {0.5 * W_18:.6f}")
    
    print("\n================================================================")
