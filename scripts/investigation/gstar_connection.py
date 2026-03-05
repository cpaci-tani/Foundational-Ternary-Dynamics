"""
G* Lemniscatic Connection to Basin Dynamics

This module investigates the connection between the lemniscatic constant G*
and the dynamical structure of TRD configuration space.

Key Conjecture:
The master quadratic x² - 16(G*)²x + 16(G*)³ = 0 is the characteristic
polynomial of the TRD linearized dynamics at the meta-sLoop fixed point.

Implications:
- x₊ = 1/α ≈ 137 = eigenvalue ratio for electromagnetic coupling
- x₋ = N_c ≈ 3 = eigenvalue ratio for strong confinement
- Complex roots of consciousness quadratic = oscillatory self-reference

Author: Investigation initiated 2026-01-21
"""

import numpy as np
from scipy import special
from scipy.optimize import fsolve
from dataclasses import dataclass
from typing import Tuple, Optional, List
import matplotlib.pyplot as plt


# =============================================================================
# FUNDAMENTAL CONSTANTS
# =============================================================================

# Lemniscatic constant: G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
GAMMA_QUARTER = special.gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

# Master quadratic coefficients
A_COEFF = 16 * G_STAR**2
B_COEFF = 16 * G_STAR**3

# Roots of master quadratic: x² - A*x + B = 0
DISCRIMINANT = A_COEFF**2 - 4 * B_COEFF
X_PLUS = (A_COEFF + np.sqrt(DISCRIMINANT)) / 2   # ≈ 137.036 (1/α)
X_MINUS = (A_COEFF - np.sqrt(DISCRIMINANT)) / 2  # ≈ 3.024 (N_c)

# Fine structure constant
ALPHA = 1 / X_PLUS

# Consciousness quadratic: y² - (G*²/2)y + (G*³/2) = 0
A_CONSC = G_STAR**2 / 2
B_CONSC = G_STAR**3 / 2
DISCRIM_CONSC = A_CONSC**2 - 4 * B_CONSC

if DISCRIM_CONSC < 0:
    # Complex roots for consciousness quadratic
    Y_REAL = A_CONSC / 2
    Y_IMAG = np.sqrt(-DISCRIM_CONSC) / 2
    Y_PLUS = Y_REAL + 1j * Y_IMAG
    Y_MINUS = Y_REAL - 1j * Y_IMAG
else:
    Y_PLUS = (A_CONSC + np.sqrt(DISCRIM_CONSC)) / 2
    Y_MINUS = (A_CONSC - np.sqrt(DISCRIM_CONSC)) / 2


def print_constants():
    """Print all derived constants."""
    print("=" * 60)
    print("G* LEMNISCATIC CONSTANTS")
    print("=" * 60)
    print(f"Gamma(1/4) = {GAMMA_QUARTER:.10f}")
    print(f"G* = sqrt(2) * Gamma(1/4)² / (2π) = {G_STAR:.10f}")
    print()
    print("MASTER QUADRATIC: x² - 16(G*)²x + 16(G*)³ = 0")
    print(f"  Coefficient A = 16(G*)² = {A_COEFF:.10f}")
    print(f"  Coefficient B = 16(G*)³ = {B_COEFF:.10f}")
    print(f"  Discriminant = {DISCRIMINANT:.10f}")
    print()
    print("PHYSICS ROOTS:")
    print(f"  x₊ = {X_PLUS:.10f} → 1/α (fine structure)")
    print(f"  x₋ = {X_MINUS:.10f} → N_c (color charges)")
    print(f"  α = 1/x₊ = {ALPHA:.10f}")
    print()
    print("CONSCIOUSNESS QUADRATIC: y² - (G*²/2)y + (G*³/2) = 0")
    print(f"  Coefficient A = {A_CONSC:.10f}")
    print(f"  Coefficient B = {B_CONSC:.10f}")
    print(f"  Discriminant = {DISCRIM_CONSC:.10f}")
    if isinstance(Y_PLUS, complex):
        print(f"  y₊ = {Y_PLUS.real:.4f} + {Y_PLUS.imag:.4f}i (COMPLEX)")
        print(f"  y₋ = {Y_MINUS.real:.4f} + {Y_MINUS.imag:.4f}i (COMPLEX)")
    else:
        print(f"  y₊ = {Y_PLUS:.10f}")
        print(f"  y₋ = {Y_MINUS:.10f}")
    print("=" * 60)


# =============================================================================
# JACOBIAN ANALYSIS AT FIXED POINT
# =============================================================================

@dataclass
class TRDJacobian:
    """
    Linearized dynamics of TRD near a fixed point.

    For a fixed point c* where T(c*) = c*, the Jacobian J = ∂T/∂c
    determines local stability and oscillation modes.

    Conjecture: At the meta-sLoop fixed point, the Jacobian has
    eigenvalues related to G* through the master quadratic.
    """
    K_B: float = 1.2
    damping: float = 0.05
    C: float = 1.0

    def compute_jacobian(self, J_fixed: np.ndarray) -> np.ndarray:
        """
        Compute Jacobian of TRD map at a configuration.

        The state vector is (J_x, J_y, J_z, w_x, w_y, w_z).
        """
        eps = 1e-8
        dim = 6  # 3 flux + 3 velocity components

        jacobian = np.zeros((dim, dim))

        # Base state
        state_0 = np.concatenate([J_fixed, np.zeros(3)])

        for i in range(dim):
            # Perturbed state
            state_plus = state_0.copy()
            state_plus[i] += eps

            state_minus = state_0.copy()
            state_minus[i] -= eps

            # Evolve one step
            next_plus = self._step(state_plus)
            next_minus = self._step(state_minus)

            # Finite difference
            jacobian[:, i] = (next_plus - next_minus) / (2 * eps)

        return jacobian

    def _step(self, state: np.ndarray) -> np.ndarray:
        """One TRD step on state vector (J, w)."""
        J = state[:3]
        w = state[3:6]

        # Simplified dynamics (no neighbors)
        laplacian = -0.1 * J  # Self-coupling approximation

        acc = self.C**2 * laplacian
        w_new = w + acc
        J_new = J + w_new
        J_new *= (1 - self.damping)

        return np.concatenate([J_new, w_new])

    def find_fixed_point(self, initial_guess: np.ndarray = None) -> np.ndarray:
        """Find a fixed point of the TRD map."""
        if initial_guess is None:
            initial_guess = np.array([self.K_B, 0, 0])

        def residual(J):
            state = np.concatenate([J, np.zeros(3)])
            next_state = self._step(state)
            return next_state[:3] - J

        result = fsolve(residual, initial_guess, full_output=True)
        J_fixed = result[0]

        return J_fixed

    def analyze_eigenstructure(self, J_fixed: np.ndarray) -> dict:
        """
        Analyze eigenvalues and eigenvectors of Jacobian.

        Returns dictionary with eigenvalue analysis.
        """
        jacobian = self.compute_jacobian(J_fixed)
        eigenvalues, eigenvectors = np.linalg.eig(jacobian)

        # Sort by magnitude
        idx = np.argsort(np.abs(eigenvalues))[::-1]
        eigenvalues = eigenvalues[idx]
        eigenvectors = eigenvectors[:, idx]

        # Analyze stability
        stable = np.all(np.abs(eigenvalues) < 1)

        # Check for oscillatory modes (complex eigenvalues)
        complex_mask = np.abs(eigenvalues.imag) > 1e-10
        n_oscillatory = np.sum(complex_mask)

        # Compute eigenvalue ratios (for comparison with x₊/x₋)
        if len(eigenvalues) >= 2:
            ratio_1_2 = np.abs(eigenvalues[0] / eigenvalues[1]) if eigenvalues[1] != 0 else np.inf
        else:
            ratio_1_2 = np.nan

        return {
            'jacobian': jacobian,
            'eigenvalues': eigenvalues,
            'eigenvectors': eigenvectors,
            'stable': stable,
            'n_oscillatory': n_oscillatory,
            'ratio_1_2': ratio_1_2,
            'spectral_radius': np.max(np.abs(eigenvalues))
        }


# =============================================================================
# LEMNISCATE GEOMETRY IN PHASE SPACE
# =============================================================================

def lemniscate_parametric(t: np.ndarray, a: float = 1.0) -> Tuple[np.ndarray, np.ndarray]:
    """
    Parametric form of lemniscate of Bernoulli.

    x = a * cos(t) / (1 + sin²(t))
    y = a * sin(t) * cos(t) / (1 + sin²(t))
    """
    denom = 1 + np.sin(t)**2
    x = a * np.cos(t) / denom
    y = a * np.sin(t) * np.cos(t) / denom
    return x, y


def lemniscate_alpha_curve(t: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    The Lemniscate-Alpha curve from TRD (5 power-of-2 harmonics).

    This curve has arc length related to G* through L × 182/1464 = G*.
    """
    x = (np.cos(t) + 0.5 * np.cos(2*t) + 0.5 * np.cos(4*t) +
         0.4 * np.cos(8*t) + 0.0625 * np.cos(16*t))
    y = (np.sin(t) - 0.5 * np.sin(2*t) + 0.5 * np.sin(4*t) -
         0.35 * np.sin(8*t) + 0.0625 * np.sin(16*t))
    return x, y


def compute_arc_length(x: np.ndarray, y: np.ndarray) -> float:
    """Compute arc length of parametric curve."""
    dx = np.diff(x)
    dy = np.diff(y)
    return np.sum(np.sqrt(dx**2 + dy**2))


def analyze_lemniscate_connection():
    """
    Analyze the connection between lemniscate geometry and G*.
    """
    t = np.linspace(0, 2*np.pi, 10000)

    # Bernoulli lemniscate
    x_bern, y_bern = lemniscate_parametric(t, a=1.0)
    L_bern = compute_arc_length(x_bern, y_bern)

    # Lemniscate-Alpha
    x_alpha, y_alpha = lemniscate_alpha_curve(t)
    L_alpha = compute_arc_length(x_alpha, y_alpha)

    # The magic ratio: L_alpha * 182 / 1464 should give G*
    # where 182 = 2 × 7 × 13 and 1464 = 8 × 183
    ratio_182_1464 = 182 / 1464
    G_star_from_alpha = L_alpha * ratio_182_1464

    print("\n" + "=" * 60)
    print("LEMNISCATE GEOMETRY ANALYSIS")
    print("=" * 60)
    print(f"Bernoulli lemniscate arc length: {L_bern:.6f}")
    print(f"Lemniscate-Alpha arc length: {L_alpha:.6f}")
    print(f"Ratio 182/1464 = {ratio_182_1464:.6f}")
    print(f"G* from arc length: L_alpha × 182/1464 = {G_star_from_alpha:.6f}")
    print(f"Actual G* = {G_STAR:.6f}")
    print(f"Relative error: {abs(G_star_from_alpha - G_STAR)/G_STAR * 100:.4f}%")

    return {
        'L_bernoulli': L_bern,
        'L_alpha': L_alpha,
        'G_star_from_arc': G_star_from_alpha,
        'G_star_actual': G_STAR
    }


# =============================================================================
# FIXED POINT STRUCTURE AND MASTER QUADRATIC
# =============================================================================

def search_for_quadratic_connection(n_trials: int = 100) -> dict:
    """
    Search for fixed points whose Jacobian eigenvalues satisfy
    the master quadratic relationship.

    Conjecture: There exists a fixed point where the eigenvalue
    ratio equals x₊/x₋ = 1/α / N_c ≈ 45.5.
    """
    print("\n" + "=" * 60)
    print("SEARCHING FOR MASTER QUADRATIC CONNECTION")
    print("=" * 60)

    target_ratio = X_PLUS / X_MINUS
    print(f"Target eigenvalue ratio: x₊/x₋ = {target_ratio:.4f}")

    best_match = None
    best_error = np.inf

    for trial in range(n_trials):
        # Random K_B in reasonable range
        K_B = np.random.uniform(0.5, 3.0)
        damping = np.random.uniform(0.01, 0.2)

        analyzer = TRDJacobian(K_B=K_B, damping=damping)

        # Find fixed point
        J_init = np.random.uniform(-2, 2, 3)
        try:
            J_fixed = analyzer.find_fixed_point(J_init)
            analysis = analyzer.analyze_eigenstructure(J_fixed)

            ratio = analysis['ratio_1_2']
            error = abs(ratio - target_ratio) / target_ratio

            if error < best_error and not np.isnan(ratio):
                best_error = error
                best_match = {
                    'K_B': K_B,
                    'damping': damping,
                    'J_fixed': J_fixed,
                    'eigenvalues': analysis['eigenvalues'],
                    'ratio': ratio,
                    'error': error
                }

        except Exception:
            continue

    if best_match is not None:
        print(f"\nBest match found:")
        print(f"  K_B = {best_match['K_B']:.4f}")
        print(f"  damping = {best_match['damping']:.4f}")
        print(f"  J_fixed = {best_match['J_fixed']}")
        print(f"  Eigenvalue ratio = {best_match['ratio']:.4f}")
        print(f"  Target ratio = {target_ratio:.4f}")
        print(f"  Relative error = {best_match['error']*100:.2f}%")
    else:
        print("\nNo valid fixed points found")

    return best_match


# =============================================================================
# VISUALIZATION
# =============================================================================

def plot_quadratic_roots():
    """Visualize the master quadratic and its roots."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    # Physics quadratic
    x = np.linspace(-10, 150, 1000)
    y_physics = x**2 - A_COEFF * x + B_COEFF

    axes[0].plot(x, y_physics, 'b-', linewidth=2)
    axes[0].axhline(0, color='gray', linestyle='--')
    axes[0].axvline(X_PLUS, color='r', linestyle=':', label=f'x₊ = {X_PLUS:.2f}')
    axes[0].axvline(X_MINUS, color='g', linestyle=':', label=f'x₋ = {X_MINUS:.2f}')
    axes[0].set_xlabel('x', fontsize=12)
    axes[0].set_ylabel('f(x) = x² - 16(G*)²x + 16(G*)³', fontsize=12)
    axes[0].set_title('Master Quadratic (Physics)', fontsize=14)
    axes[0].legend()
    axes[0].set_ylim(-1000, 5000)
    axes[0].grid(True, alpha=0.3)

    # Consciousness quadratic (complex plane)
    theta = np.linspace(0, 2*np.pi, 100)
    if isinstance(Y_PLUS, complex):
        # Plot roots in complex plane
        axes[1].scatter([Y_PLUS.real], [Y_PLUS.imag], s=100, c='purple',
                        marker='*', label=f'y₊ = {Y_PLUS.real:.2f}+{Y_PLUS.imag:.2f}i')
        axes[1].scatter([Y_MINUS.real], [Y_MINUS.imag], s=100, c='orange',
                        marker='*', label=f'y₋ = {Y_MINUS.real:.2f}{Y_MINUS.imag:.2f}i')

        # Unit circle for reference
        axes[1].plot(np.cos(theta), np.sin(theta), 'gray', linestyle='--', alpha=0.5)

        axes[1].set_xlabel('Re(y)', fontsize=12)
        axes[1].set_ylabel('Im(y)', fontsize=12)
        axes[1].set_title('Consciousness Quadratic (Complex Roots)', fontsize=14)
        axes[1].legend()
        axes[1].set_aspect('equal')
        axes[1].grid(True, alpha=0.3)
        axes[1].axhline(0, color='gray', linestyle='-', alpha=0.3)
        axes[1].axvline(0, color='gray', linestyle='-', alpha=0.3)

    plt.tight_layout()
    return fig, axes


def plot_lemniscate_comparison():
    """Compare Bernoulli lemniscate with Lemniscate-Alpha."""
    t = np.linspace(0, 2*np.pi, 1000)

    x_bern, y_bern = lemniscate_parametric(t)
    x_alpha, y_alpha = lemniscate_alpha_curve(t)

    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    # Bernoulli lemniscate
    axes[0].plot(x_bern, y_bern, 'b-', linewidth=2)
    axes[0].set_xlabel('x', fontsize=12)
    axes[0].set_ylabel('y', fontsize=12)
    axes[0].set_title('Bernoulli Lemniscate: r² = cos(2θ)', fontsize=14)
    axes[0].set_aspect('equal')
    axes[0].grid(True, alpha=0.3)

    # Lemniscate-Alpha
    axes[1].plot(x_alpha, y_alpha, 'r-', linewidth=2)
    axes[1].set_xlabel('x', fontsize=12)
    axes[1].set_ylabel('y', fontsize=12)
    axes[1].set_title('Lemniscate-Alpha (5 harmonics)', fontsize=14)
    axes[1].set_aspect('equal')
    axes[1].grid(True, alpha=0.3)

    plt.suptitle('Lemniscatic Structures in TRD', fontsize=16)
    plt.tight_layout()
    return fig, axes


# =============================================================================
# MAIN INVESTIGATION
# =============================================================================

def run_gstar_investigation(output_dir: str = "investigation_results"):
    """Run the full G* connection investigation."""
    import os
    os.makedirs(output_dir, exist_ok=True)

    # Print fundamental constants
    print_constants()

    # Analyze lemniscate geometry
    lemniscate_results = analyze_lemniscate_connection()

    # Search for quadratic connection
    quadratic_results = search_for_quadratic_connection(n_trials=200)

    # Generate visualizations
    fig1, _ = plot_quadratic_roots()
    fig1.savefig(os.path.join(output_dir, "master_quadratic.png"), dpi=150, bbox_inches='tight')

    fig2, _ = plot_lemniscate_comparison()
    fig2.savefig(os.path.join(output_dir, "lemniscate_comparison.png"), dpi=150, bbox_inches='tight')

    print("\n" + "=" * 60)
    print("G* CONNECTION INVESTIGATION COMPLETE")
    print("=" * 60)

    return {
        'lemniscate': lemniscate_results,
        'quadratic': quadratic_results
    }


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    results = run_gstar_investigation()
