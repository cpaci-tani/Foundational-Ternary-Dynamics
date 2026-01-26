"""
FTD Quantum Entropy Module

Implements von Neumann entropy and density matrix formalism for the
FTD Hilbert space H_TRD = L²(Lattice, ℂ).

This module provides:
- Density matrix construction from pure and mixed states
- Von Neumann entropy calculation
- Entanglement entropy for bipartite systems
- Purity measures

Epistemic Status: [THEOREM] - Standard quantum information theory
applied to FTD Hilbert space construction.
"""

import numpy as np
from typing import Optional, Tuple, List
from dataclasses import dataclass


@dataclass
class DensityMatrix:
    """
    Density matrix representation of a quantum state in FTD.

    For pure states: ρ = |ψ⟩⟨ψ|
    For mixed states: ρ = Σ_i p_i |ψ_i⟩⟨ψ_i|

    Properties:
        - Tr(ρ) = 1 (normalization)
        - ρ† = ρ (Hermitian)
        - ρ ≥ 0 (positive semidefinite)
    """
    matrix: np.ndarray

    def __post_init__(self):
        """Validate density matrix properties."""
        # Check Hermiticity
        if not np.allclose(self.matrix, self.matrix.conj().T):
            raise ValueError("Density matrix must be Hermitian")

        # Check trace normalization
        trace = np.trace(self.matrix)
        if not np.isclose(trace, 1.0):
            raise ValueError(f"Density matrix trace must be 1, got {trace}")

        # Check positive semidefiniteness
        eigenvalues = np.linalg.eigvalsh(self.matrix)
        if np.any(eigenvalues < -1e-10):
            raise ValueError("Density matrix must be positive semidefinite")

    @classmethod
    def from_pure_state(cls, psi: np.ndarray) -> 'DensityMatrix':
        """
        Construct density matrix from pure state vector.

        ρ = |ψ⟩⟨ψ|

        Args:
            psi: Complex state vector (will be normalized)

        Returns:
            DensityMatrix for the pure state
        """
        psi = np.asarray(psi, dtype=complex)
        psi = psi / np.linalg.norm(psi)  # Normalize
        rho = np.outer(psi, psi.conj())
        return cls(matrix=rho)

    @classmethod
    def from_ensemble(cls, states: List[np.ndarray], probabilities: List[float]) -> 'DensityMatrix':
        """
        Construct density matrix from statistical ensemble of pure states.

        ρ = Σ_i p_i |ψ_i⟩⟨ψ_i|

        Args:
            states: List of pure state vectors
            probabilities: Corresponding probabilities (must sum to 1)

        Returns:
            DensityMatrix for the mixed state
        """
        if not np.isclose(sum(probabilities), 1.0):
            raise ValueError("Probabilities must sum to 1")

        dim = len(states[0])
        rho = np.zeros((dim, dim), dtype=complex)

        for psi, p in zip(states, probabilities):
            psi = np.asarray(psi, dtype=complex)
            psi = psi / np.linalg.norm(psi)
            rho += p * np.outer(psi, psi.conj())

        return cls(matrix=rho)

    @classmethod
    def maximally_mixed(cls, dim: int) -> 'DensityMatrix':
        """
        Construct maximally mixed state: ρ = I/d

        Args:
            dim: Dimension of Hilbert space

        Returns:
            DensityMatrix for maximally mixed state
        """
        rho = np.eye(dim, dtype=complex) / dim
        return cls(matrix=rho)

    @property
    def purity(self) -> float:
        """
        Calculate purity: Tr(ρ²)

        - Pure state: Tr(ρ²) = 1
        - Mixed state: Tr(ρ²) < 1
        - Maximally mixed: Tr(ρ²) = 1/d
        """
        return np.real(np.trace(self.matrix @ self.matrix))

    @property
    def is_pure(self) -> bool:
        """Check if state is pure (Tr(ρ²) ≈ 1)."""
        return np.isclose(self.purity, 1.0)

    @property
    def dimension(self) -> int:
        """Dimension of the Hilbert space."""
        return self.matrix.shape[0]


def von_neumann_entropy(rho: DensityMatrix, base: str = 'natural') -> float:
    """
    Calculate von Neumann entropy: S = -Tr(ρ ln ρ)

    For pure states: S = 0
    For maximally mixed: S = ln(d)

    Args:
        rho: Density matrix
        base: 'natural' (ln), 'bits' (log2), or 'dits' (log_d)

    Returns:
        Von Neumann entropy
    """
    # Get eigenvalues
    eigenvalues = np.linalg.eigvalsh(rho.matrix)

    # Filter out zeros (to avoid log(0))
    eigenvalues = eigenvalues[eigenvalues > 1e-15]

    # Compute entropy
    if base == 'natural':
        S = -np.sum(eigenvalues * np.log(eigenvalues))
    elif base == 'bits':
        S = -np.sum(eigenvalues * np.log2(eigenvalues))
    elif base == 'dits':
        d = rho.dimension
        S = -np.sum(eigenvalues * np.log(eigenvalues)) / np.log(d)
    else:
        raise ValueError(f"Unknown base: {base}")

    return float(np.real(S))


def entanglement_entropy(rho_AB: DensityMatrix, dim_A: int) -> float:
    """
    Calculate entanglement entropy of subsystem A.

    S_A = -Tr(ρ_A ln ρ_A)

    where ρ_A = Tr_B(ρ_AB) is the reduced density matrix.

    For maximally entangled states: S_A = ln(min(d_A, d_B))

    Args:
        rho_AB: Density matrix of composite system
        dim_A: Dimension of subsystem A

    Returns:
        Entanglement entropy of subsystem A
    """
    dim_total = rho_AB.dimension
    dim_B = dim_total // dim_A

    if dim_A * dim_B != dim_total:
        raise ValueError(f"Dimensions don't match: {dim_A} * {dim_B} != {dim_total}")

    # Compute reduced density matrix by partial trace over B
    rho_A = partial_trace_B(rho_AB.matrix, dim_A, dim_B)

    # Normalize and create DensityMatrix
    rho_A_dm = DensityMatrix(matrix=rho_A)

    return von_neumann_entropy(rho_A_dm)


def partial_trace_B(rho_AB: np.ndarray, dim_A: int, dim_B: int) -> np.ndarray:
    """
    Compute partial trace over subsystem B.

    ρ_A = Tr_B(ρ_AB)

    Args:
        rho_AB: Full density matrix (dim_A * dim_B) × (dim_A * dim_B)
        dim_A: Dimension of subsystem A
        dim_B: Dimension of subsystem B

    Returns:
        Reduced density matrix ρ_A (dim_A × dim_A)
    """
    # Reshape to tensor form
    rho_tensor = rho_AB.reshape(dim_A, dim_B, dim_A, dim_B)

    # Trace over B indices (1 and 3)
    rho_A = np.trace(rho_tensor, axis1=1, axis2=3)

    return rho_A


def bell_state(which: str = 'phi+') -> DensityMatrix:
    """
    Create a Bell state density matrix.

    The four Bell states are maximally entangled:
    |Φ+⟩ = (|00⟩ + |11⟩)/√2
    |Φ-⟩ = (|00⟩ - |11⟩)/√2
    |Ψ+⟩ = (|01⟩ + |10⟩)/√2
    |Ψ-⟩ = (|01⟩ - |10⟩)/√2

    Args:
        which: 'phi+', 'phi-', 'psi+', or 'psi-'

    Returns:
        DensityMatrix for the Bell state
    """
    # Basis states |00⟩, |01⟩, |10⟩, |11⟩
    sqrt2 = np.sqrt(2)

    if which == 'phi+':
        psi = np.array([1, 0, 0, 1], dtype=complex) / sqrt2
    elif which == 'phi-':
        psi = np.array([1, 0, 0, -1], dtype=complex) / sqrt2
    elif which == 'psi+':
        psi = np.array([0, 1, 1, 0], dtype=complex) / sqrt2
    elif which == 'psi-':
        psi = np.array([0, 1, -1, 0], dtype=complex) / sqrt2
    else:
        raise ValueError(f"Unknown Bell state: {which}")

    return DensityMatrix.from_pure_state(psi)


def verify_ftd_entanglement():
    """
    Verify FTD entanglement entropy predictions.

    For Bell states (maximally entangled):
    - S_vN(AB) = 0 (pure total state)
    - S_A = S_B = ln(2) (maximally mixed subsystems)

    This matches the quantum mechanical prediction.
    """
    print("FTD Entanglement Entropy Verification")
    print("=" * 50)

    # Create Bell state |Φ+⟩
    rho_AB = bell_state('phi+')

    # Total entropy (should be 0 for pure state)
    S_total = von_neumann_entropy(rho_AB)
    print(f"\nBell state |Phi+>:")
    print(f"  Total entropy S(AB) = {S_total:.6f}")
    print(f"  Expected: 0 (pure state)")
    print(f"  Match: {np.isclose(S_total, 0)}")

    # Entanglement entropy (should be ln(2))
    S_A = entanglement_entropy(rho_AB, dim_A=2)
    print(f"\n  Entanglement entropy S_A = {S_A:.6f}")
    print(f"  Expected: ln(2) = {np.log(2):.6f}")
    print(f"  Match: {np.isclose(S_A, np.log(2))}")

    # Purity check
    print(f"\n  Purity Tr(rho^2) = {rho_AB.purity:.6f}")
    print(f"  Is pure: {rho_AB.is_pure}")

    # Compare with maximally mixed state
    print("\n" + "=" * 50)
    print("Comparison: Maximally mixed state")
    rho_mixed = DensityMatrix.maximally_mixed(4)
    S_mixed = von_neumann_entropy(rho_mixed)
    print(f"  S_vN = {S_mixed:.6f}")
    print(f"  Expected: ln(4) = {np.log(4):.6f}")
    print(f"  Purity = {rho_mixed.purity:.6f}")
    print(f"  Expected: 1/4 = 0.25")

    return True


if __name__ == "__main__":
    verify_ftd_entanglement()
