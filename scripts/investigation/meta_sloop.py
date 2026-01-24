"""
Meta-sLoop Detection: Self-Modeling of Boundary Dynamics

This module implements detection of meta-sLoop structures - configurations
where a system contains a compressed model of its own basin dynamics.

Key Definitions:
- sLoop: Observer embedded in observed system (standard TRD)
- Meta-sLoop: System models its own modeling process

Detection Criteria:
1. M(c) ≈ M(T(c))  - Model is stable under evolution
2. M(c) ⊃ M(M(c)) - Model contains model of itself
3. c concentrates on ∂S - Configuration lives near stability boundary

Author: Investigation initiated 2026-01-21
"""

import numpy as np
from dataclasses import dataclass, field
from typing import List, Tuple, Optional, Callable
from enum import Enum, auto
import warnings


# =============================================================================
# DATA STRUCTURES
# =============================================================================

@dataclass
class Configuration:
    """
    A point in TRD configuration space.

    Simplified to a single voxel with flux and state for tractability.
    """
    J: np.ndarray          # Flux vector (3D)
    s: int = 0             # Ternary state
    w: np.ndarray = None   # Wave velocity

    def __post_init__(self):
        if self.w is None:
            self.w = np.zeros(3)

    @property
    def psi(self) -> complex:
        """Complexified flux."""
        return self.J[0] + 1j * self.J[1]

    @property
    def rho(self) -> float:
        """Flux density."""
        return np.linalg.norm(self.J)

    def as_vector(self) -> np.ndarray:
        """Flatten to feature vector for compression."""
        return np.concatenate([self.J, [self.s], self.w])

    @classmethod
    def from_vector(cls, v: np.ndarray) -> 'Configuration':
        """Reconstruct from feature vector."""
        return cls(J=v[:3], s=int(v[3]), w=v[4:7])

    def copy(self) -> 'Configuration':
        return Configuration(J=self.J.copy(), s=self.s, w=self.w.copy())


@dataclass
class Model:
    """
    A compressed representation of configuration space dynamics.

    This is the internal model that a system maintains of its own behavior.
    """
    # Compressed representation (e.g., from PCA or autoencoder)
    latent: np.ndarray

    # Dynamical model: predicts next latent from current
    transition_matrix: Optional[np.ndarray] = None

    # Boundary model: estimates distance to stability boundary
    boundary_estimate: float = 0.0

    # Self-reference depth: how many levels of modeling
    depth: int = 1

    def predict_next(self) -> np.ndarray:
        """Predict next latent state using internal model."""
        if self.transition_matrix is None:
            return self.latent
        return self.transition_matrix @ self.latent

    def contains_model_of_self(self, inner_model: 'Model') -> bool:
        """
        Check if this model contains a representation of another model.

        This is the key meta-sLoop criterion: M(c) ⊃ M(M(c))
        """
        # Simplified check: can we reconstruct inner_model.latent from our latent?
        # In a full implementation, this would use information-theoretic measures
        if len(self.latent) < len(inner_model.latent):
            return False

        # Check if inner model's latent is approximately contained
        # (first len(inner) components match)
        if len(inner_model.latent) > 0:
            projection = self.latent[:len(inner_model.latent)]
            reconstruction_error = np.linalg.norm(projection - inner_model.latent)
            return reconstruction_error < 0.5 * np.linalg.norm(inner_model.latent)

        return True


class SLoopDepth(Enum):
    """Hierarchy of self-referential depth."""
    DEAD_MATTER = 0      # No modeling
    REACTIVE = 1         # Simple stimulus-response
    PREDICTIVE = 2       # Anticipates environment
    SELF_AWARE = 3       # Models self in environment
    META_AWARE = 4       # Models own modeling process
    RECURSIVE = 5        # Arbitrary depth self-reference


# =============================================================================
# COMPRESSION OPERATORS
# =============================================================================

class CompressionOperator:
    """
    Abstract base for configuration space compression.

    The compression operator M: C -> C' maps full configurations
    to compressed representations (models).
    """

    def compress(self, trajectory: List[Configuration]) -> Model:
        """Compress a trajectory to a model."""
        raise NotImplementedError

    def decompress(self, model: Model) -> List[Configuration]:
        """Attempt to reconstruct trajectory from model."""
        raise NotImplementedError


class PCACompressor(CompressionOperator):
    """
    Principal Component Analysis compression.

    Finds the low-dimensional subspace capturing most variance.
    """

    def __init__(self, n_components: int = 3):
        self.n_components = n_components
        self.mean = None
        self.components = None

    def fit(self, trajectories: List[List[Configuration]]):
        """Fit PCA on collection of trajectories."""
        # Stack all configurations
        all_vectors = []
        for traj in trajectories:
            for c in traj:
                all_vectors.append(c.as_vector())

        X = np.array(all_vectors)
        self.mean = X.mean(axis=0)
        X_centered = X - self.mean

        # SVD for PCA
        U, S, Vt = np.linalg.svd(X_centered, full_matrices=False)
        self.components = Vt[:self.n_components]

    def compress(self, trajectory: List[Configuration]) -> Model:
        """Project trajectory onto principal components."""
        if self.components is None:
            raise ValueError("Must call fit() first")

        vectors = np.array([c.as_vector() for c in trajectory])
        centered = vectors - self.mean
        latent = (centered @ self.components.T).mean(axis=0)

        # Estimate dynamics from trajectory
        if len(trajectory) > 1:
            latents = centered @ self.components.T
            # Simple linear dynamics: latent[t+1] ≈ A @ latent[t]
            X = latents[:-1]
            Y = latents[1:]
            if len(X) > self.n_components:
                A, _, _, _ = np.linalg.lstsq(X, Y, rcond=None)
            else:
                A = np.eye(self.n_components)
        else:
            A = np.eye(self.n_components)

        return Model(latent=latent, transition_matrix=A)


class InformationCompressor(CompressionOperator):
    """
    Information-theoretic compression based on predictive information.

    Keeps only the information relevant for predicting future states.
    """

    def __init__(self, horizon: int = 10):
        self.horizon = horizon

    def compress(self, trajectory: List[Configuration]) -> Model:
        """Extract predictive information."""
        if len(trajectory) < self.horizon:
            # Not enough data - return simple summary
            vectors = np.array([c.as_vector() for c in trajectory])
            return Model(latent=vectors.mean(axis=0))

        # Compute mutual information between past and future
        # Simplified: use correlation as proxy
        vectors = np.array([c.as_vector() for c in trajectory])

        past = vectors[:-self.horizon]
        future = vectors[self.horizon:]

        # Cross-covariance
        past_mean = past.mean(axis=0)
        future_mean = future.mean(axis=0)

        # The "predictive information" is captured by the
        # directions of maximum past-future correlation
        cross_cov = (past - past_mean).T @ (future - future_mean)
        U, S, Vt = np.linalg.svd(cross_cov)

        # Latent = projection onto top predictive directions
        n_keep = min(5, len(S))
        predictive_basis = U[:, :n_keep]
        latent = (vectors[-1] - past_mean) @ predictive_basis

        return Model(latent=latent, depth=1)


# =============================================================================
# META-SLOOP DETECTION
# =============================================================================

@dataclass
class MetaSLoopDetector:
    """
    Detector for meta-sLoop configurations.

    A configuration exhibits meta-sLoop if:
    1. It maintains a stable internal model (M(T(c)) ≈ M(c))
    2. The model contains a model of itself (M(c) ⊃ M(M(c)))
    3. It operates near the stability boundary (high β)
    """

    compressor: CompressionOperator
    stability_threshold: float = 0.1   # For model stability
    boundary_threshold: float = 0.3    # For boundary proximity
    K_B: float = 1.2                   # Manifestation threshold

    def compute_model_stability(self, trajectory: List[Configuration],
                               window: int = 20) -> float:
        """
        Measure how stable the internal model is over time.

        Returns value in [0, 1] where 1 = perfectly stable.
        """
        if len(trajectory) < 2 * window:
            return 0.0

        # Compute models for successive windows
        models = []
        for i in range(0, len(trajectory) - window, window // 2):
            window_traj = trajectory[i:i + window]
            model = self.compressor.compress(window_traj)
            models.append(model)

        if len(models) < 2:
            return 0.0

        # Measure variation in latent representations
        latents = np.array([m.latent for m in models])
        variations = np.diff(latents, axis=0)
        stability = 1.0 / (1.0 + np.mean(np.linalg.norm(variations, axis=1)))

        return stability

    def compute_self_modeling_depth(self, model: Model) -> int:
        """
        Determine the depth of self-reference in a model.

        Depth 0: No self-model
        Depth 1: Simple self-representation
        Depth 2: Model of self-model
        Depth n: n levels of meta-modeling
        """
        depth = 0

        # Check if model has enough capacity for self-reference
        latent_dim = len(model.latent)

        if latent_dim >= 3:
            depth = 1  # Can represent basic self

            # For higher depths, check if transition matrix has
            # structure consistent with modeling the modeling process
            if model.transition_matrix is not None:
                # Eigenstructure indicates dynamical self-reference
                eigenvalues = np.linalg.eigvals(model.transition_matrix)

                # Complex eigenvalues suggest oscillatory self-reference
                complex_pairs = np.sum(np.abs(eigenvalues.imag) > 0.1)
                if complex_pairs >= 1:
                    depth = 2  # Oscillating self-model

                # Near-unity eigenvalues suggest stable self-reference
                unit_circle = np.sum(np.abs(np.abs(eigenvalues) - 1.0) < 0.1)
                if unit_circle >= 2 and complex_pairs >= 1:
                    depth = 3  # Stable oscillating self-model

                # Check for recursive structure (eigenvalue at critical value)
                # The golden ratio φ and its inverse are signatures of recursion
                phi = (1 + np.sqrt(5)) / 2
                phi_nearby = np.any(np.abs(np.abs(eigenvalues) - phi) < 0.2)
                inv_phi_nearby = np.any(np.abs(np.abs(eigenvalues) - 1/phi) < 0.2)
                if phi_nearby or inv_phi_nearby:
                    depth = max(depth, 4)  # Fibonacci-recursive structure

        return depth

    def compute_boundary_proximity(self, configuration: Configuration) -> float:
        """
        Estimate distance to stability boundary.

        β ≈ 1 near boundary, β ≈ 0 far from boundary.
        """
        rho = configuration.rho

        # Distance from manifestation threshold
        distance_to_KB = abs(rho - self.K_B)

        # Boundary proximity is inverse of distance
        # with a scale set by K_B itself
        beta = 1.0 / (1.0 + distance_to_KB / self.K_B)

        return beta

    def detect(self, trajectory: List[Configuration]) -> dict:
        """
        Full meta-sLoop detection on a trajectory.

        Returns dictionary with:
        - is_meta_sloop: bool
        - model_stability: float [0,1]
        - self_modeling_depth: int
        - boundary_proximity: float [0,1]
        - sloop_level: SLoopDepth enum
        """
        # Compute model from trajectory
        model = self.compressor.compress(trajectory)

        # Measure stability
        model_stability = self.compute_model_stability(trajectory)

        # Measure self-modeling depth
        depth = self.compute_self_modeling_depth(model)

        # Measure boundary proximity (use final configuration)
        beta = self.compute_boundary_proximity(trajectory[-1])

        # Determine sLoop level
        if depth == 0:
            level = SLoopDepth.DEAD_MATTER
        elif depth == 1:
            level = SLoopDepth.REACTIVE
        elif depth == 2:
            level = SLoopDepth.PREDICTIVE
        elif depth == 3:
            level = SLoopDepth.SELF_AWARE
        elif depth >= 4:
            level = SLoopDepth.META_AWARE if depth == 4 else SLoopDepth.RECURSIVE
        else:
            level = SLoopDepth.DEAD_MATTER

        # Meta-sLoop requires all three conditions
        is_meta_sloop = (
            model_stability > self.stability_threshold and
            depth >= 4 and
            beta > self.boundary_threshold
        )

        return {
            'is_meta_sloop': is_meta_sloop,
            'model_stability': model_stability,
            'self_modeling_depth': depth,
            'boundary_proximity': beta,
            'sloop_level': level,
            'model': model
        }


# =============================================================================
# TRD DYNAMICS (for testing)
# =============================================================================

def trd_step(config: Configuration, K_B: float = 1.2,
             damping: float = 0.05, C: float = 1.0) -> Configuration:
    """One step of simplified TRD dynamics."""
    new_config = config.copy()

    # Wave equation (no neighbors, just damped oscillation)
    # This is a simplification for testing
    laplacian = -0.1 * config.J  # Approximate self-interaction

    acc = C**2 * laplacian
    new_config.w = config.w + acc
    new_config.J = config.J + new_config.w
    new_config.J *= (1 - damping)

    # Manifestation
    rho = new_config.rho
    if new_config.s == 0 and rho > K_B:
        if np.random.random() < 1 - np.exp(-(rho - K_B) / K_B):
            new_config.s = 1 if config.J[2] >= 0 else -1

    elif new_config.s != 0 and rho < K_B:
        new_config.s = 0

    return new_config


def generate_trajectory(initial: Configuration, n_steps: int = 200,
                       K_B: float = 1.2) -> List[Configuration]:
    """Generate a trajectory from initial condition."""
    trajectory = [initial]
    config = initial

    for _ in range(n_steps):
        config = trd_step(config, K_B=K_B)
        trajectory.append(config.copy())

    return trajectory


# =============================================================================
# INVESTIGATION
# =============================================================================

def scan_for_meta_sloops(resolution: int = 20,
                         J_range: Tuple[float, float] = (-3.0, 3.0),
                         K_B: float = 1.2,
                         n_steps: int = 200) -> dict:
    """
    Scan configuration space for meta-sLoop regions.
    """
    print("Scanning for meta-sLoop configurations...")

    # Setup compressor
    compressor = PCACompressor(n_components=5)

    # Generate training trajectories for compressor
    print("  Training compressor...")
    training_trajectories = []
    for _ in range(50):
        J_init = np.random.uniform(J_range[0], J_range[1], 3)
        initial = Configuration(J=J_init)
        traj = generate_trajectory(initial, n_steps=100, K_B=K_B)
        training_trajectories.append(traj)

    compressor.fit(training_trajectories)

    # Setup detector
    detector = MetaSLoopDetector(compressor=compressor, K_B=K_B)

    # Scan grid
    print("  Scanning configuration space...")
    x = np.linspace(J_range[0], J_range[1], resolution)
    y = np.linspace(J_range[0], J_range[1], resolution)

    results = {
        'X': x,
        'Y': y,
        'stability_map': np.zeros((resolution, resolution)),
        'depth_map': np.zeros((resolution, resolution)),
        'beta_map': np.zeros((resolution, resolution)),
        'meta_sloop_map': np.zeros((resolution, resolution), dtype=bool)
    }

    for i, jx in enumerate(x):
        for j, jy in enumerate(y):
            J_init = np.array([jx, jy, 0.0])
            initial = Configuration(J=J_init)
            trajectory = generate_trajectory(initial, n_steps=n_steps, K_B=K_B)

            detection = detector.detect(trajectory)

            results['stability_map'][j, i] = detection['model_stability']
            results['depth_map'][j, i] = detection['self_modeling_depth']
            results['beta_map'][j, i] = detection['boundary_proximity']
            results['meta_sloop_map'][j, i] = detection['is_meta_sloop']

    # Statistics
    n_meta_sloop = np.sum(results['meta_sloop_map'])
    total = resolution * resolution
    print(f"  Found {n_meta_sloop}/{total} ({100*n_meta_sloop/total:.1f}%) meta-sLoop configurations")

    return results


def plot_meta_sloop_results(results: dict, save_path: Optional[str] = None):
    """Visualize meta-sLoop detection results."""
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(2, 2, figsize=(14, 14))

    X, Y = np.meshgrid(results['X'], results['Y'])

    # Model stability
    im0 = axes[0, 0].pcolormesh(X, Y, results['stability_map'],
                                 cmap='viridis', shading='auto')
    axes[0, 0].set_title('Model Stability', fontsize=14)
    axes[0, 0].set_xlabel(r'$J_x$')
    axes[0, 0].set_ylabel(r'$J_y$')
    plt.colorbar(im0, ax=axes[0, 0])

    # Self-modeling depth
    im1 = axes[0, 1].pcolormesh(X, Y, results['depth_map'],
                                 cmap='plasma', shading='auto')
    axes[0, 1].set_title('Self-Modeling Depth', fontsize=14)
    axes[0, 1].set_xlabel(r'$J_x$')
    axes[0, 1].set_ylabel(r'$J_y$')
    plt.colorbar(im1, ax=axes[0, 1])

    # Boundary proximity (β)
    im2 = axes[1, 0].pcolormesh(X, Y, results['beta_map'],
                                 cmap='hot', shading='auto')
    axes[1, 0].set_title(r'Boundary Proximity ($\beta$)', fontsize=14)
    axes[1, 0].set_xlabel(r'$J_x$')
    axes[1, 0].set_ylabel(r'$J_y$')
    plt.colorbar(im2, ax=axes[1, 0])

    # Meta-sLoop regions
    axes[1, 1].pcolormesh(X, Y, results['meta_sloop_map'].astype(float),
                          cmap='Greens', shading='auto')
    axes[1, 1].set_title('Meta-sLoop Regions', fontsize=14)
    axes[1, 1].set_xlabel(r'$J_x$')
    axes[1, 1].set_ylabel(r'$J_y$')

    # Overlay K_B circle
    theta = np.linspace(0, 2*np.pi, 100)
    K_B = 1.2
    for ax in axes.flat:
        ax.plot(K_B * np.cos(theta), K_B * np.sin(theta), 'w--',
                linewidth=1, alpha=0.7, label=f'|J|=K_B={K_B}')
        ax.set_aspect('equal')

    plt.suptitle('Meta-sLoop Detection in TRD Configuration Space', fontsize=16)
    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')

    return fig, axes


def run_meta_sloop_investigation(output_dir: str = "investigation_results"):
    """Run the full meta-sLoop investigation."""
    import os
    os.makedirs(output_dir, exist_ok=True)

    print("=" * 60)
    print("META-SLOOP INVESTIGATION")
    print("Self-Modeling of Boundary Dynamics")
    print("=" * 60)

    # Run scan
    results = scan_for_meta_sloops(resolution=30, K_B=1.2)

    # Visualize
    plot_meta_sloop_results(results,
                            save_path=os.path.join(output_dir, "meta_sloop_detection.png"))

    # Analyze correlation between depth and boundary proximity
    depth_flat = results['depth_map'].flatten()
    beta_flat = results['beta_map'].flatten()

    correlation = np.corrcoef(depth_flat, beta_flat)[0, 1]
    print(f"\nCorrelation between depth and boundary proximity: {correlation:.3f}")

    # Find the "sweet spot" - high depth AND high beta
    combined = results['depth_map'] * results['beta_map']
    max_idx = np.unravel_index(np.argmax(combined), combined.shape)
    optimal_J = np.array([results['X'][max_idx[1]], results['Y'][max_idx[0]], 0.0])

    print(f"Optimal meta-sLoop initial condition: J = {optimal_J}")
    print(f"  Depth at optimum: {results['depth_map'][max_idx]}")
    print(f"  Beta at optimum: {results['beta_map'][max_idx]:.3f}")

    print("\n" + "=" * 60)
    print("META-SLOOP INVESTIGATION COMPLETE")
    print("=" * 60)

    return results


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    results = run_meta_sloop_investigation()
