"""
FTD Framework Configuration
Derived from Chapter 7 of CLAUDE.md

Phase 2.4 Update: Extended configuration for full 12-phase update cycle.
"""
from dataclasses import dataclass
import itertools

@dataclass
class PhysicsConfig:
    # === Fundamental Constants ===
    C: float = 0.5           # Speed of Causality (voxels/tick) - Reduced for CFL Stability
    H: float = 1.0           # Planck unit (lattice spacing)

    # === Thresholds ===
    KB: float = 1.2          # Manifestation threshold (increased to prevent runaway genesis)
    WEAK_THRESHOLD: float = 10.0  # Stress threshold for transmutation (Phase 10)

    # === Coupling Constants ===
    ALPHA: float = 0.00729   # Fine structure constant
    BETA: float = 0.01       # Magnetic coupling (Lorentz force)
    G_STRONG: float = 1.0    # Strong coupling (Yukawa)
    M_PI: float = 0.15       # Effective pion mass scale (sets Yukawa range)

    # === Derived parameters ===
    # DECAY_RATE must be << alpha^2 ~ 5e-5 for particle stability
    # See scripts/verification/particle_stability.py for derivation
    DECAY_RATE: float = 5.3e-7   # Fixed: was 0.001, now << alpha^2 for atomic stability
    GRAVITY_BIAS: float = 0.01   # Gravitational coupling strength

    # === System Settings ===
    GRID_SIZE: int = 256     # Default grid size (16M voxels for 256³)
    DAMPING: float = 0.05    # Wave equation damping

    # === Time Gating ===
    MIN_GAMMA: float = 0.01  # Minimum Lorentz factor (prevents division issues)


# Generate neighborhood offsets as module-level constants
def _generate_moore_shifts():
    """Generate 26-connected Moore neighborhood offsets."""
    offsets = list(itertools.product([-1, 0, 1], repeat=3))
    offsets.remove((0, 0, 0))
    return offsets

def _generate_von_neumann_shifts():
    """Generate 6-connected Von Neumann neighborhood offsets."""
    return [
        (-1, 0, 0), (1, 0, 0),
        (0, -1, 0), (0, 1, 0),
        (0, 0, -1), (0, 0, 1)
    ]

# Neighborhood constants
MOORE_SHIFTS = _generate_moore_shifts()       # 26 neighbors
VON_NEUMANN_SHIFTS = _generate_von_neumann_shifts()  # 6 neighbors



# Global singleton instance
CONSTANTS = PhysicsConfig()


# Convenience function to create a smaller grid for testing
def get_test_config(grid_size: int = 32):
    """Return a config suitable for fast testing."""
    return PhysicsConfig(GRID_SIZE=grid_size)
