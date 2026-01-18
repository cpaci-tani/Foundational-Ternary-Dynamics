"""
FTD Lattice Model
Implements the cubic voxel grid using dense NumPy arrays.
"""
import numpy as np
from ..config import CONSTANTS

class Universe:
    def __init__(self, size: int = CONSTANTS.GRID_SIZE):
        self.size = size
        
        # State Grid: Stores {-1, 0, +1}
        # Using int8 for minimal memory usage
        self.states = np.zeros((size, size, size), dtype=np.int8)
        
        # Flux Grid: Stores J vector (Jx, Jy, Jz)
        # Using float32 for performance/precision balance
        self.flux = np.zeros((size, size, size, 3), dtype=np.float32)
        
        # Secondary fields (scratch buffers for updates)
        self.wave_velocity = np.zeros_like(self.flux)
        self.density = np.zeros((size, size, size), dtype=np.float32)
        
        # Metadata per voxel (locking, etc)
        self.is_locked = np.zeros((size, size, size), dtype=bool)

    @property
    def shape(self):
        return (self.size, self.size, self.size)

    def reset(self):
        """Clear all fields to vacuum state."""
        self.states.fill(0)
        self.flux.fill(0)
        self.wave_velocity.fill(0)
        self.density.fill(0)
        self.is_locked.fill(False)
