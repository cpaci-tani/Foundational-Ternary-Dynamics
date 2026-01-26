"""
FTD Field Calculations and Force Computation
Computes derived scalar and vector fields from the primary Flux field.

Phase 2.4 Update: Implements all 5 force types from CLAUDE.md Chapter 6.
- Gravity-like (density gradient)
- Coulomb-like (charge gradient)
- Lorentz-like (curl × velocity)
- Strong-like (Yukawa form)
- Weak-like (stress calculation for transmutation)
"""
import numpy as np
from ..config import CONSTANTS


# =============================================================================
# PRIMARY FIELD CALCULATIONS
# =============================================================================

def calculate_density(universe):
    """
    Compute flux magnitude (Energy Density).
    rho = |J|
    Updates universe.density in-place.
    """
    # np.linalg.norm is slower than explicit sqrt(sum squares) for this shape
    universe.density = np.sqrt(np.sum(universe.flux ** 2, axis=3))


def calculate_divergence(universe):
    """
    Compute discrete divergence of Flux.
    div(J) = (Jx(x+1) - Jx(x-1))/2 + ...
    Returns a scalar field (Nx, Ny, Nz).
    """
    return divergence_3d(universe.flux)


# =============================================================================
# DISCRETE DIFFERENTIAL OPERATORS
# =============================================================================

def gradient_3d(field):
    """
    Compute discrete gradient of a scalar field.
    Uses central differences: (f(x+1) - f(x-1)) / 2

    Args:
        field: Scalar field of shape (Nx, Ny, Nz)

    Returns:
        Vector field of shape (Nx, Ny, Nz, 3)
    """
    grad = np.zeros((*field.shape, 3), dtype=np.float32)

    # Gradient in X
    grad[..., 0] = (np.roll(field, -1, axis=0) - np.roll(field, 1, axis=0)) / 2.0
    # Gradient in Y
    grad[..., 1] = (np.roll(field, -1, axis=1) - np.roll(field, 1, axis=1)) / 2.0
    # Gradient in Z
    grad[..., 2] = (np.roll(field, -1, axis=2) - np.roll(field, 1, axis=2)) / 2.0

    return grad


def divergence_3d(vector_field):
    """
    Compute discrete divergence of a vector field.
    div(J) = dJx/dx + dJy/dy + dJz/dz

    Args:
        vector_field: Vector field of shape (Nx, Ny, Nz, 3)

    Returns:
        Scalar field of shape (Nx, Ny, Nz)
    """
    J = vector_field

    # dJx/dx
    dx = (np.roll(J[..., 0], -1, axis=0) - np.roll(J[..., 0], 1, axis=0)) / 2.0
    # dJy/dy
    dy = (np.roll(J[..., 1], -1, axis=1) - np.roll(J[..., 1], 1, axis=1)) / 2.0
    # dJz/dz
    dz = (np.roll(J[..., 2], -1, axis=2) - np.roll(J[..., 2], 1, axis=2)) / 2.0

    return dx + dy + dz


def curl_3d(vector_field):
    """
    Compute discrete curl of a vector field.
    curl(J) = (dJz/dy - dJy/dz, dJx/dz - dJz/dx, dJy/dx - dJx/dy)

    This is the Levi-Civita contraction: (∇×J)_i = ε_ijk ∂_j J_k

    Args:
        vector_field: Vector field of shape (Nx, Ny, Nz, 3)

    Returns:
        Vector field of shape (Nx, Ny, Nz, 3)
    """
    J = vector_field
    curl = np.zeros_like(J)

    # Partial derivatives (central differences)
    # dJx/dy, dJx/dz
    dJx_dy = (np.roll(J[..., 0], -1, axis=1) - np.roll(J[..., 0], 1, axis=1)) / 2.0
    dJx_dz = (np.roll(J[..., 0], -1, axis=2) - np.roll(J[..., 0], 1, axis=2)) / 2.0

    # dJy/dx, dJy/dz
    dJy_dx = (np.roll(J[..., 1], -1, axis=0) - np.roll(J[..., 1], 1, axis=0)) / 2.0
    dJy_dz = (np.roll(J[..., 1], -1, axis=2) - np.roll(J[..., 1], 1, axis=2)) / 2.0

    # dJz/dx, dJz/dy
    dJz_dx = (np.roll(J[..., 2], -1, axis=0) - np.roll(J[..., 2], 1, axis=0)) / 2.0
    dJz_dy = (np.roll(J[..., 2], -1, axis=1) - np.roll(J[..., 2], 1, axis=1)) / 2.0

    # Curl components
    curl[..., 0] = dJz_dy - dJy_dz  # (∇×J)_x
    curl[..., 1] = dJx_dz - dJz_dx  # (∇×J)_y
    curl[..., 2] = dJy_dx - dJx_dy  # (∇×J)_z

    return curl


def smooth_field(field):
    """
    Apply 6-neighbor (Von Neumann) averaging to a scalar field.

    Args:
        field: Scalar field of shape (Nx, Ny, Nz)

    Returns:
        Smoothed scalar field of shape (Nx, Ny, Nz)
    """
    smoothed = (
        np.roll(field, 1, axis=0) + np.roll(field, -1, axis=0) +
        np.roll(field, 1, axis=1) + np.roll(field, -1, axis=1) +
        np.roll(field, 1, axis=2) + np.roll(field, -1, axis=2)
    ) / 6.0
    return smoothed


def normalize_field(vector_field, eps=1e-10):
    """
    Normalize a vector field to unit vectors.
    Handles zero vectors by returning zero.

    Args:
        vector_field: Vector field of shape (Nx, Ny, Nz, 3)
        eps: Small value to avoid division by zero

    Returns:
        Unit vector field of shape (Nx, Ny, Nz, 3)
    """
    magnitude = np.sqrt(np.sum(vector_field ** 2, axis=-1, keepdims=True))
    magnitude = np.maximum(magnitude, eps)  # Avoid division by zero
    return vector_field / magnitude


# =============================================================================
# FORCE CALCULATIONS (CLAUDE.md Chapter 6)
# =============================================================================

def gravity_force(universe):
    """
    Phase 6.2: Gravity-like Force

    F_grav(v) = G_N × ∇ρ̄(v)

    where ρ̄ is smoothed density (6-neighbor average).
    Produces attraction toward high-density regions.
    """
    # Smooth the density field
    rho_smoothed = smooth_field(universe.density)

    # Compute gradient of smoothed density
    grad_rho = gradient_3d(rho_smoothed)

    # Scale by gravitational coupling
    return CONSTANTS.GRAVITY_BIAS * grad_rho


def coulomb_force(universe):
    """
    Phase 6.3: Electromagnetic-like (Coulomb) Force

    F_elec(v) = -q(v) × ∇q̄(v)

    Like charges repel (positive gradient × positive charge = negative force).
    Opposite charges attract.
    """
    # Smooth the charge field
    q_smoothed = smooth_field(universe.charge)

    # Compute gradient of smoothed charge
    grad_q = gradient_3d(q_smoothed)

    # Scale by local charge (produces repulsion for same sign)
    # Reshape charge to (N, N, N, 1) for broadcasting
    return -universe.charge[..., np.newaxis] * grad_q


def lorentz_force(universe):
    """
    Phase 6.3: Magnetic-like (Lorentz) Force

    F_mag(v) = β × (∇×J) × ĵ(v)

    where ĵ is the unit vector in the direction of local flux.
    This produces the magnetic component of electromagnetic force.
    """
    # Compute curl of flux field
    curl_J = curl_3d(universe.flux)

    # Get unit vector of flux direction
    j_hat = normalize_field(universe.flux)

    # Cross product: curl × j_hat
    # Note: np.cross works on last axis by default
    force = CONSTANTS.BETA * np.cross(curl_J, j_hat)

    return force


def strong_force(universe):
    """
    Phase 6.4: Strong-like Force (Yukawa form)

    F_strong(r) = g_s² × exp(-m_π r) / r² × (1 + m_π r)

    Short-range, acts only on adjacent manifested voxels.
    This is computationally expensive for dense grids, so we
    approximate using a convolution-like approach.

    For the lattice, we compute the force from each of the 6
    nearest neighbors (Von Neumann neighborhood).
    """
    force = np.zeros_like(universe.flux)

    # Mask of manifested voxels
    manifested = universe.states != 0

    # Early exit if no manifested voxels
    if not np.any(manifested):
        return force

    # For each direction, compute Yukawa attraction
    g_s_sq = CONSTANTS.G_STRONG ** 2
    m_pi = CONSTANTS.M_PI

    # At r=1 (nearest neighbor), Yukawa factor
    r = 1.0
    yukawa_factor = g_s_sq * np.exp(-m_pi * r) / (r ** 2) * (1 + m_pi * r)

    # Direction vectors for 6 neighbors
    directions = [
        (1, 0, 0), (-1, 0, 0),
        (0, 1, 0), (0, -1, 0),
        (0, 0, 1), (0, 0, -1)
    ]
    dir_vectors = np.array([
        [1, 0, 0], [-1, 0, 0],
        [0, 1, 0], [0, -1, 0],
        [0, 0, 1], [0, 0, -1]
    ], dtype=np.float32)

    for (sx, sy, sz), direction in zip(directions, dir_vectors):
        # Check if neighbor is manifested and same sign (strong force binds like particles)
        neighbor_state = np.roll(universe.states, (sx, sy, sz), axis=(0, 1, 2))

        # Strong force acts between same-sign manifested particles
        # (quarks of same color attract at short range in simplified model)
        attracts = manifested & (neighbor_state == universe.states) & (neighbor_state != 0)

        # Skip if no attractions in this direction
        if not np.any(attracts):
            continue

        # Add attractive force toward neighbor (direction points toward neighbor)
        # Use direct assignment to avoid broadcasting issues
        force[attracts, 0] += yukawa_factor * direction[0]
        force[attracts, 1] += yukawa_factor * direction[1]
        force[attracts, 2] += yukawa_factor * direction[2]

    return force


def weak_stress(universe):
    """
    Phase 6.5: Weak-like Stress Calculation

    stress(v) = |∇·J| + |∇×J| + |∇ρ|

    High stress enables transmutation (polarity flip).
    This is used in Phase 10 to determine which voxels can transmute.

    Returns:
        Scalar stress field of shape (Nx, Ny, Nz)
    """
    # Divergence magnitude
    div_J = divergence_3d(universe.flux)
    div_magnitude = np.abs(div_J)

    # Curl magnitude
    curl_J = curl_3d(universe.flux)
    curl_magnitude = np.sqrt(np.sum(curl_J ** 2, axis=-1))

    # Density gradient magnitude
    grad_rho = gradient_3d(universe.density)
    grad_magnitude = np.sqrt(np.sum(grad_rho ** 2, axis=-1))

    return div_magnitude + curl_magnitude + grad_magnitude


def accumulate_forces(universe):
    """
    Phase 6: Force Accumulation

    Compute all forces and sum into force_accum.
    Only manifested voxels receive forces.

    Forces computed:
    1. Gravity-like (density gradient attraction)
    2. Coulomb-like (charge gradient)
    3. Lorentz-like (curl × flux direction)
    4. Strong-like (Yukawa short-range)

    Note: Weak stress is computed separately in transmutation phase.
    """
    # Clear force accumulator
    universe.force_accum.fill(0)

    # Mask of manifested voxels (only they feel forces)
    manifested = universe.states != 0
    manifested_3d = manifested[..., np.newaxis]

    # Compute each force contribution
    f_grav = gravity_force(universe)
    f_coulomb = coulomb_force(universe)
    f_lorentz = lorentz_force(universe)
    f_strong = strong_force(universe)

    # Sum all forces (only for manifested voxels)
    total_force = f_grav + f_coulomb + f_lorentz + f_strong

    # Apply only to manifested voxels
    universe.force_accum = np.where(manifested_3d, total_force, 0)
