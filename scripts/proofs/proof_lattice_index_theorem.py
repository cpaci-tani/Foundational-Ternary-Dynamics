"""
proof_lattice_index_theorem.py — FTD-0230 Verification.

Numerical demonstration of the Ginsparg-Wilson relation and the Atiyah-Singer 
index theorem on a 2D compact torus U(1) lattice (Schwinger model analogue).

Verifies:
1. Hermiticity and chiral-hermiticity of the Wilson operator H_W.
2. The Overlap Dirac operator D_ov satisfies the Ginsparg-Wilson relation exactly:
       gamma_5 D_ov + D_ov gamma_5 = a D_ov gamma_5 D_ov
   to machine precision (< 10^-12), whereas the standard Wilson operator D_W fails it.
3. The Atiyah-Singer index theorem:
       index(D_ov) = N_+ - N_- = q
   for topological sectors q in {-2, -1, 0, 1, 2}, where q is the total magnetic flux.
4. Exact zero-modes possess pure chirality eigenvalues (+1 or -1).

Usage:
    python scripts/proofs/proof_lattice_index_theorem.py
"""

from __future__ import annotations

import sys
import numpy as np


# =============================================================================
# Dirac Algebra in 2D
# =============================================================================
# gamma_1 = sigma_x, gamma_2 = sigma_y
# gamma_5 = -sigma_z = i * gamma_1 * gamma_2
GAMMA_1 = np.array([[0.0, 1.0], [1.0, 0.0]], dtype=complex)
GAMMA_2 = np.array([[0.0, -1j], [1j, 0.0]], dtype=complex)
GAMMA_5 = np.array([[-1.0, 0.0], [0.0, 1.0]], dtype=complex)
EYE_2 = np.eye(2, dtype=complex)


def build_lattice_operators(L: int, q: int) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Constructs the 2D Wilson-Dirac operator D_W and global Gamma_5 on an L x L periodic torus
    under a uniform magnetic flux background of topological charge q.
    """
    Ns = L * L
    dim = 2 * Ns
    D_W = np.zeros((dim, dim), dtype=complex)
    Gamma_5 = np.zeros((dim, dim), dtype=complex)
    
    # Plaquette flux phase
    phi = 2.0 * np.pi * q / Ns
    
    # 1. Construct U(1) Gauge Links
    U1 = np.zeros((L, L), dtype=complex)
    U2 = np.zeros((L, L), dtype=complex)
    
    for x in range(L):
        for y in range(L):
            U1[x, y] = np.exp(-1j * phi * y)
            if y == L - 1:
                U2[x, y] = np.exp(1j * 2.0 * np.pi * q * x / L)
            else:
                U2[x, y] = 1.0
                
    # Verify that all plaquettes have the exact same phase
    for x in range(L):
        for y in range(L):
            x_next = (x + 1) % L
            y_next = (y + 1) % L
            plaq = U1[x, y] * U2[x_next, y] * np.conj(U1[x, y_next]) * np.conj(U2[x, y])
            plaq_phase = np.angle(plaq)
            assert np.abs(plaq_phase - phi) < 1e-10 or np.abs(np.abs(plaq_phase - phi) - 2.0*np.pi) < 1e-10, \
                f"Plaquette at ({x},{y}) has phase {plaq_phase}, expected {phi}"

    # 2. Build Operators
    for x in range(L):
        for y in range(L):
            i = x + L * y
            
            # Diagonal block Gamma_5
            Gamma_5[2*i:2*i+2, 2*i:2*i+2] = GAMMA_5
            
            # Wilson self-interaction term (2 * I)
            D_W[2*i:2*i+2, 2*i:2*i+2] = 2.0 * EYE_2
            
            # Directions: 1 = x, 2 = y
            # Step in +x
            x_next = (x + 1) % L
            j_x_next = x_next + L * y
            D_W[2*i:2*i+2, 2*j_x_next:2*j_x_next+2] = 0.5 * (GAMMA_1 - EYE_2) * U1[x, y]
            
            # Step in -x
            x_prev = (x - 1) % L
            j_x_prev = x_prev + L * y
            D_W[2*i:2*i+2, 2*j_x_prev:2*j_x_prev+2] = -0.5 * (GAMMA_1 + EYE_2) * np.conj(U1[x_prev, y])
            
            # Step in +y
            y_next = (y + 1) % L
            j_y_next = x + L * y_next
            D_W[2*i:2*i+2, 2*j_y_next:2*j_y_next+2] = 0.5 * (GAMMA_2 - EYE_2) * U2[x, y]
            
            # Step in -y
            y_prev = (y - 1) % L
            j_y_prev = x + L * y_prev
            D_W[2*i:2*i+2, 2*j_y_prev:2*j_y_prev+2] = -0.5 * (GAMMA_2 + EYE_2) * np.conj(U2[x, y_prev])

    return D_W, Gamma_5, np.eye(dim, dtype=complex)


def build_overlap_operator(D_W: np.ndarray, Gamma_5: np.ndarray, Eye: np.ndarray, m_0: float = 1.0) -> tuple[np.ndarray, np.ndarray]:
    """
    Constructs the Overlap Dirac operator D_ov from D_W.
    Returns D_ov and the Hermitian sign matrix sgn(H_W).
    """
    # H_W = Gamma_5 (D_W - m_0 * I)
    H_W = Gamma_5 @ (D_W - m_0 * Eye)
    
    # 1. Verify Hermiticity of H_W
    herm_residual = np.linalg.norm(H_W - H_W.conj().T)
    assert herm_residual < 1e-12, f"H_W is not Hermitian, residual: {herm_residual}"
    
    # 2. Verify Chiral Hermiticity of D_W: Gamma_5 D_W Gamma_5 = D_W^dagger
    chiral_herm_dw = np.linalg.norm(Gamma_5 @ D_W @ Gamma_5 - D_W.conj().T)
    assert chiral_herm_dw < 1e-12, f"D_W chiral hermiticity failed: {chiral_herm_dw}"

    # 3. Compute sign(H_W) via exact spectral decomposition
    eigenvalues, eigenvectors = np.linalg.eigh(H_W)
    assert np.all(np.abs(eigenvalues) > 1e-10), "H_W has near-zero eigenvalues, matrix sign function singular!"
    
    sgn_val = np.sign(eigenvalues)
    sgn_H = eigenvectors @ np.diag(sgn_val) @ eigenvectors.conj().T
    
    # 4. Construct D_ov = 1 + Gamma_5 * sgn(H_W) (setting a = 1)
    D_ov = Eye + Gamma_5 @ sgn_H
    
    # 5. Verify Chiral Hermiticity of D_ov: Gamma_5 D_ov Gamma_5 = D_ov^dagger
    chiral_herm_dov = np.linalg.norm(Gamma_5 @ D_ov @ Gamma_5 - D_ov.conj().T)
    assert chiral_herm_dov < 1e-12, f"D_ov chiral hermiticity failed: {chiral_herm_dov}"
    
    return D_ov, sgn_H


def run_checks_for_sector(L: int, q: int) -> dict:
    """
    Runs the verification suite for a given topological sector q on an L x L grid.
    """
    print(f"\n--- Running Verification for Sector q = {q} on a {L}x{L} Lattice ---")
    Ns = L * L
    D_W, Gamma_5, Eye = build_lattice_operators(L, q)
    
    # Construct Overlap Operator
    D_ov, sgn_H = build_overlap_operator(D_W, Gamma_5, Eye, m_0=1.0)
    
    # 1. Verify Ginsparg-Wilson relation: Gamma_5 D_ov + D_ov Gamma_5 - D_ov Gamma_5 D_ov = 0 (a = 1)
    lhs_gw = Gamma_5 @ D_ov + D_ov @ Gamma_5
    rhs_gw = D_ov @ Gamma_5 @ D_ov
    gw_residual = np.linalg.norm(lhs_gw - rhs_gw)
    
    # Check standard Wilson D_W for contrast
    lhs_dw_gw = Gamma_5 @ D_W + D_W @ Gamma_5
    rhs_dw_gw = D_W @ Gamma_5 @ D_W
    dw_gw_residual = np.linalg.norm(lhs_dw_gw - rhs_dw_gw)
    
    # 2. Compute Trace Index: index(D_ov) = Tr( Gamma_5 (I - 0.5 * D_ov) )
    trace_operator = Gamma_5 @ (Eye - 0.5 * D_ov)
    trace_index = np.real(np.trace(trace_operator))
    
    # 3. Direct Zero-Mode Eigenvalue and Chirality Audit
    # Non-zero modes lie on the Ginsparg-Wilson circle: |z - 1| = 1
    # Zero-modes lie exactly at z = 0.
    ov_evals, ov_evecs = np.linalg.eig(D_ov)
    
    # Find zero-modes with threshold
    zero_mode_indices = np.where(np.abs(ov_evals) < 1e-10)[0]
    num_zero_modes = len(zero_mode_indices)
    
    n_plus = 0
    n_minus = 0
    zero_mode_chiralities = []
    
    if num_zero_modes > 0:
        # Project Gamma_5 onto the zero-mode subspace (orthonormalized via QR)
        P_0 = ov_evecs[:, zero_mode_indices]
        Q_0, _ = np.linalg.qr(P_0)
        sub_gamma5 = Q_0.conj().T @ Gamma_5 @ Q_0
        
        # Diagonalize Gamma_5 in the zero-mode subspace to find definite chirality eigenvalues
        chirality_eigenvalues = np.real(np.linalg.eigvalsh(sub_gamma5))
        zero_mode_chiralities = list(chirality_eigenvalues)
        
        # Assert that every eigenvalue in this subspace is exactly +1.0 or -1.0
        for val in chirality_eigenvalues:
            assert np.abs(np.abs(val) - 1.0) < 1e-10, f"Zero-mode has fractional chirality: {val}"
            if val > 0.0:
                n_plus += 1
            else:
                n_minus += 1
            
    spectral_index = n_plus - n_minus
    
    print(f"  Overlap GW Residual:  {gw_residual:.2e}")
    print(f"  Wilson GW Residual:   {dw_gw_residual:.2f}")
    print(f"  Trace Index:          {trace_index:.6f}")
    print(f"  Number of Zero-Modes: {num_zero_modes} (N_+ = {n_plus}, N_- = {n_minus})")
    print(f"  Spectral Index:       {spectral_index}")
    if zero_mode_chiralities:
        print(f"  Zero-Mode Chiralities: {['+1' if c > 0 else '-1' for c in zero_mode_chiralities]}")
        
    return {
        "gw_residual": gw_residual,
        "dw_gw_residual": dw_gw_residual,
        "trace_index": trace_index,
        "spectral_index": spectral_index,
        "n_plus": n_plus,
        "n_minus": n_minus,
        "num_zero_modes": num_zero_modes
    }


def main() -> int:
    print("=" * 80)
    print("proof_lattice_index_theorem.py — FTD-0230 Verification")
    print("=" * 80)
    
    L = 8  # 8 x 8 grid
    sectors = [-2, -1, 0, 1, 2]
    all_passed = True
    
    results = {}
    for q in sectors:
        res = run_checks_for_sector(L, q)
        results[q] = res
        
        # 1. Assert Ginsparg-Wilson relation holds to high precision
        if res["gw_residual"] > 1e-12:
            print(f"  [FAIL] Ginsparg-Wilson relation not satisfied (residual {res['gw_residual']:.2e} > 1e-12)")
            all_passed = False
            
        # 2. Assert Wilson operator fails the GW relation by a wide margin
        if res["dw_gw_residual"] < 0.1:
            print(f"  [FAIL] Wilson operator didn't fail GW relation (residual {res['dw_gw_residual']:.2e} < 0.1)")
            all_passed = False
            
        # 3. Assert Trace Index matches the topological charge q exactly (rounded)
        if int(np.round(res["trace_index"])) != q:
            print(f"  [FAIL] Trace Index {res['trace_index']:.6f} does not match charge q = {q}")
            all_passed = False
            
        # 4. Assert Spectral Index matches q exactly
        if res["spectral_index"] != q:
            print(f"  [FAIL] Spectral Index {res['spectral_index']} does not match charge q = {q}")
            all_passed = False
            
        # 5. Assert the Index Theorem matches: index(D) = N_+ - N_- = q
        # and that the index is stable
        expected_n_plus = q if q > 0 else (1 if q == 0 else 0)
        expected_n_minus = -q if q < 0 else (1 if q == 0 else 0)
        if res["n_plus"] != expected_n_plus:
            print(f"  [FAIL] N_+ = {res['n_plus']} mismatch for q = {q} (expected {expected_n_plus})")
            all_passed = False
        if res["n_minus"] != expected_n_minus:
            print(f"  [FAIL] N_- = {res['n_minus']} mismatch for q = {q} (expected {expected_n_minus})")
            all_passed = False

    print("\n" + "=" * 80)
    print("VERIFICATION SUMMARY")
    print("=" * 80)
    if all_passed:
        print("\n*** ALL GINSPARG-WILSON & INDEX THEOREM CHECKS PASSED (100% SUCCESS) ***")
        print("  - Proved: gamma_5 D_ov + D_ov gamma_5 = a D_ov gamma_5 D_ov to machine precision (< 1e-12).")
        print("  - Proved: index(D_ov) = N_+ - N_- = q exactly for q = -2, -1, 0, 1, 2.")
        print("  - Proved: Exact zero-modes exhibit pure chirality eigenvalues (+1 or -1).")
        return 0
    else:
        print("\n*** SOME CHECKS FAILED ***")
        return 1


if __name__ == "__main__":
    sys.exit(main())
