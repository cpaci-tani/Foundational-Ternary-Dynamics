"""
FTD Lattice Model
Implements the cubic voxel grid using dense NumPy arrays.

Phase 2.4 Update: Extended data structure for full 12-phase update cycle.
"""
import numpy as np
from ..config import CONSTANTS

class Universe:
    def __init__(self, size: int = CONSTANTS.GRID_SIZE):
        self.size = size

        # === Primary State Fields ===

        # State Grid: Stores {-1, 0, +1}
        # Using int8 for minimal memory usage
        self.states = np.zeros((size, size, size), dtype=np.int8)

        # Flux Grid: Stores J vector (Jx, Jy, Jz)
        # Using float32 for performance/precision balance
        self.flux = np.zeros((size, size, size, 3), dtype=np.float32)

        # === Secondary Fields (scratch buffers) ===

        self.wave_velocity = np.zeros_like(self.flux)
        self.density = np.zeros((size, size, size), dtype=np.float32)

        # === Phase 2.4 Extensions ===

        # Particle velocities (for Phase 7 integration)
        self.velocity = np.zeros((size, size, size, 3), dtype=np.float32)

        # Sub-lattice position remainders (for Phase 8 movement)
        self.position_rem = np.zeros((size, size, size, 3), dtype=np.float32)

        # Force accumulator per tick (for Phase 6)
        self.force_accum = np.zeros((size, size, size, 3), dtype=np.float32)

        # Phase accumulator for time gating (Phase 1)
        self.phase_accum = np.zeros((size, size, size), dtype=np.float32)

        # Active flag - passed time gate this tick (Phase 1)
        self.is_active = np.ones((size, size, size), dtype=bool)

        # Charge values - fractional charges for quarks etc.
        self.charge = np.zeros((size, size, size), dtype=np.float32)

        # === Metadata ===

        # Locking flag (bound structures don't decay)
        self.is_locked = np.zeros((size, size, size), dtype=bool)

        # Global tick counter (Phase 12)
        self.tick = 0

    @property
    def shape(self):
        return (self.size, self.size, self.size)

    def reset(self):
        """Clear all fields to vacuum state."""
        self.states.fill(0)
        self.flux.fill(0)
        self.wave_velocity.fill(0)
        self.density.fill(0)
        self.velocity.fill(0)
        self.position_rem.fill(0)
        self.force_accum.fill(0)
        self.phase_accum.fill(0)
        self.is_active.fill(True)
        self.charge.fill(0)
        self.is_locked.fill(False)
        self.tick = 0

    def sync_charge_from_state(self):
        """
        Set charge based on state for simple particles.
        +1 state -> +1 charge, -1 state -> -1 charge
        For fractional charges (quarks), must be set explicitly.
        """
        self.charge = self.states.astype(np.float32)

    def get_manifested_count(self):
        """Return count of manifested voxels (state != 0)."""
        return np.count_nonzero(self.states)

    def get_positive_count(self):
        """Return count of positive state voxels."""
        return np.count_nonzero(self.states == 1)

    def get_negative_count(self):
        """Return count of negative state voxels."""
        return np.count_nonzero(self.states == -1)

    def get_total_flux_magnitude(self):
        """Return sum of all flux magnitudes (total 'energy')."""
        return np.sum(self.density)

    def get_total_charge(self):
        """Return net charge (should be conserved)."""
        return np.sum(self.charge)
