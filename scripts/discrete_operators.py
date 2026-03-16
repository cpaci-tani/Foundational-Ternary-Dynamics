"""
Discrete Calculus Operators for FTD Lattice
===========================================

Implements discrete differential geometry operators on a 3D cubic lattice.
Uses numpy.roll for efficient vectorized neighbor access with periodic boundary conditions.

Definitions:
- Gradient: Central difference
- Laplacian: Support for 6-neighbor (Von Neumann) and 26-neighbor (Moore)
- Curl: Discrete circulation
- Divergence: Flux balance
"""

import numpy as np

def shift(field, shift_vec):
    """
    Shift field by shift_vec (dx, dy, dz) using periodic boundaries.
    shift_vec: (i, j, k) integers
    """
    # numpy.roll shift is (axis0_shift, axis1_shift, ...)
    # shift=+1 means moving elements RIGHT, so field[x] becomes field[x-1] spatially?
    # No, np.roll(a, 1) moves last element to first.
    # f(x+1) corresponds to rolling by -1 (element at index i comes from i+1)
    
    dx, dy, dz = shift_vec
    
    # We want to access f(r + d).
    # If we want the array representing f(x+1), we need to shift the data "left" (index i gets value from i+1)
    # So shift vector needs to be negated for np.roll
    return np.roll(field, shift=(-dx, -dy, -dz), axis=(0, 1, 2))


def discrete_gradient(scalar_field):
    """
    Compute gradient of scalar field using central differences.
    Returns: (grad_x, grad_y, grad_z)
    """
    # f(x+1) - f(x-1) / 2
    gx = (shift(scalar_field, (1,0,0)) - shift(scalar_field, (-1,0,0))) / 2.0
    gy = (shift(scalar_field, (0,1,0)) - shift(scalar_field, (0,-1,0))) / 2.0
    gz = (shift(scalar_field, (0,0,1)) - shift(scalar_field, (0,0,-1))) / 2.0
    return (gx, gy, gz)


def discrete_divergence(vector_field):
    """
    Compute divergence of vector field (Vx, Vy, Vz).
    Returns: scalar field
    """
    Vx, Vy, Vz = vector_field
    
    dx_Vx = (shift(Vx, (1,0,0)) - shift(Vx, (-1,0,0))) / 2.0
    dy_Vy = (shift(Vy, (0,1,0)) - shift(Vy, (0,-1,0))) / 2.0
    dz_Vz = (shift(Vz, (0,0,1)) - shift(Vz, (0,0,-1))) / 2.0
    
    return dx_Vx + dy_Vy + dz_Vz


def discrete_curl(vector_field):
    """
    Compute curl of vector field.
    Returns: (Curl_x, Curl_y, Curl_z)
    """
    Vx, Vy, Vz = vector_field
    
    # dy_Vz - dz_Vy
    dy_Vz = (shift(Vz, (0,1,0)) - shift(Vz, (0,-1,0))) / 2.0
    dz_Vy = (shift(Vy, (0,0,1)) - shift(Vy, (0,0,-1))) / 2.0
    Cx = dy_Vz - dz_Vy
    
    # dz_Vx - dx_Vz
    dz_Vx = (shift(Vx, (0,0,1)) - shift(Vx, (0,0,-1))) / 2.0
    dx_Vz = (shift(Vz, (1,0,0)) - shift(Vz, (-1,0,0))) / 2.0
    Cy = dz_Vx - dx_Vz
    
    # dx_Vy - dy_Vx
    dx_Vy = (shift(Vy, (1,0,0)) - shift(Vy, (-1,0,0))) / 2.0
    dy_Vx = (shift(Vx, (0,1,0)) - shift(Vx, (0,-1,0))) / 2.0
    Cz = dx_Vy - dy_Vx
    
    return (Cx, Cy, Cz)


def discrete_laplacian(scalar_field, stencil='moore_isotropic'):
    """
    Compute Laplacian.
    stencils:
        'von_neumann': 6 neighbors (1, -6)
        'moore_isotropic': 26 neighbors weighted by inverse squared distance
    """
    if stencil == 'von_neumann':
        # 6 neighbors
        acc = shift(scalar_field, (1,0,0)) + shift(scalar_field, (-1,0,0)) + \
              shift(scalar_field, (0,1,0)) + shift(scalar_field, (0,-1,0)) + \
              shift(scalar_field, (0,0,1)) + shift(scalar_field, (0,0,-1))
        return acc - 6.0 * scalar_field
        
    elif stencil == 'moore_isotropic':
        # 26 neighbors
        # Weights: 
        # d=1 (6 faces)   w = 3/13? No, let's use 1/d^2 geometric weights normalized
        # Standard Isotropic discretization for discrete Laplacian often uses:
        # 6 faces: 6/13? 
        # Actually in FTD we want maximum isotropy.
        # Let's use simple inverse r^2 weights:
        # face (r=1)   : w = 1
        # edge (r=sqrt2): w = 1/2
        # corner(r=sqrt3): w = 1/3
        #
        # Normalize so sum of weights cancels center?
        # Laplacian L u = (\sum w_i (u_i - u_0))
        # This form guarantees L(constant) = 0
        
        laplacian = np.zeros_like(scalar_field)
        
        # Iterate over 26 neighbors
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                for dz in [-1, 0, 1]:
                    if dx==0 and dy==0 and dz==0:
                        continue
                    
                    dist_sq = dx*dx + dy*dy + dz*dz
                    weight = 1.0 / dist_sq
                    
                    neighbor = shift(scalar_field, (dx, dy, dz))
                    # Add w_i * (u_i - u_0)
                    laplacian += weight * (neighbor - scalar_field)
                    
        return laplacian
    
    else:
        raise ValueError(f"Unknown stencil: {stencil}")
