"""
FTD Field Calculations
Computes derived scalar and vector fields from the primary Flux field.
"""
import numpy as np

def calculate_density(universe):
    """
    Compute flux magnitude (Energy Density).
    rho = |J|
    Updates universe.density in-place.
    """
    # np.linalg.norm is slower than explicit sqrt(sum squares) for this shape
    universe.density = np.sqrt(np.sum(universe.flux**2, axis=3))

def calculate_divergence(universe):
    """
    Compute discrete divergence of Flux.
    div(J) = (Jx(x+1) - Jx(x-1))/2 + ...
    Returns a scalar field (Nx, Ny, Nz).
    """
    J = universe.flux
    
    # Gradient in X: (J(x+1) - J(x-1))/2
    # roll(-1) shifts left (gets x+1 value to x)
    # roll(1) shifts right (gets x-1 value to x)
    dx = (np.roll(J[..., 0], -1, axis=0) - np.roll(J[..., 0], 1, axis=0)) / 2.0
    # Gradient in Y
    dy = (np.roll(J[..., 1], -1, axis=1) - np.roll(J[..., 1], 1, axis=1)) / 2.0
    # Gradient in Z
    dz = (np.roll(J[..., 2], -1, axis=2) - np.roll(J[..., 2], 1, axis=2)) / 2.0

    
    return dx + dy + dz
