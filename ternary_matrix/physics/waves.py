"""
FTD Flux Propagation
Implements the discrete wave equation for the Flux field J.
Equation: d^2J/dt^2 = c^2 * Laplacian(J)
"""
import numpy as np
from ..config import CONSTANTS

def laplacian_3d_vector(field):
    """
    Compute discrete Laplacian for a 3D vector field (Nx, Ny, Nz, 3).
    Kernel: 6-connected neighbors - 6*center
    """
    # Initialize with center contribution
    lap = -6.0 * field.copy()
    
    # Add neighbors (Cyclic boundary conditions for now to avoid edge artifacts)
    # X axis
    lap += np.roll(field, 1, axis=0)
    lap += np.roll(field, -1, axis=0)
    
    # Y axis
    lap += np.roll(field, 1, axis=1)
    lap += np.roll(field, -1, axis=1)
    
    # Z axis
    lap += np.roll(field, 1, axis=2)
    lap += np.roll(field, -1, axis=2)
    
    return lap

def propagate_flux(universe):
    """
    Update flux field according to wave equation.
    Modifies universe.flux and universe.wave_velocity in-place.
    """
    # 1. Calculate acceleration from Laplacian
    # acc = c^2 * ∇²J
    acc = (CONSTANTS.C ** 2) * laplacian_3d_vector(universe.flux)
    
    # 2. Update Velocity (Verlet integration step 1)
    # v(t+1) = v(t) + a(t)
    universe.wave_velocity += acc
    
    # 3. Update Position (Flux)
    # J(t+1) = J(t) + v(t+1)
    universe.flux += universe.wave_velocity
    
    # 4. Apply Damping (Phenomenological decay to prevent runaway energy)
    decay_factor = 1.0 - CONSTANTS.DAMPING
    universe.flux *= decay_factor
    universe.wave_velocity *= decay_factor
