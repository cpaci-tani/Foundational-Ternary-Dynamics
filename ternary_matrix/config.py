"""
FTD Framework Configuration
Derived from Chapter 7 of CLAUDE.md
"""
from dataclasses import dataclass

@dataclass
class PhysicsConfig:
    # Fundamental Constants
    C: float = 0.5           # Speed of Causality (voxels/tick) - Reduced for CFL Stability
    H: float = 1.0           # Planck unit (lattice spacing)
    
    # Thresholds
    KB: float = 1.2          # Increased to prevent runaway genesis
    
    # Coupling Constants
    ALPHA: float = 0.00729   # Fine structure constant
    
    # Derived parameters
    DECAY_RATE: float = 0.001    # Reduced decay so we don't need massive flux inputs
    GRAVITY_BIAS: float = 0.01   # Gravitational coupling strength
    
    # System Settings
    GRID_SIZE: int = 256     # Infinite Scale (16M voxels)
    DAMPING: float = 0.05    # Keep damping high






# Global singleton instance
CONSTANTS = PhysicsConfig()
