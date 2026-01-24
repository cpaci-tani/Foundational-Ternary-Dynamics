#!/usr/bin/env python3
"""
Flux Field Dynamics Simulation

Implements the core FTD flux field equations on a 3D lattice.
Demonstrates wave propagation, gradient formation, and flux exclusion.

Based on FTD Chapter 3: The Flux Field
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from dataclasses import dataclass
from typing import Tuple, Optional
import json


# =============================================================================
# Physical Constants (FTD Natural Units)
# =============================================================================

C = 1.0              # Speed of causality (voxels/tick)
KB = 0.511           # Manifestation threshold
ALPHA = 1/137.036    # Fine structure constant
DAMPING = 0.001      # Flux damping rate
G_N = 0.01           # Gravitational coupling

# FTD Integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13


# =============================================================================
# Lattice and Field Classes
# =============================================================================

@dataclass
class FluxField:
    """3D flux field on a cubic lattice."""

    size: int
    J: np.ndarray      # Flux vector field (size, size, size, 3)
    velocity: np.ndarray  # Wave velocity field
    state: np.ndarray  # Manifestation state {-1, 0, +1}

    @classmethod
    def create(cls, size: int) -> 'FluxField':
        """Create an empty flux field."""
        return cls(
            size=size,
            J=np.zeros((size, size, size, 3)),
            velocity=np.zeros((size, size, size, 3)),
            state=np.zeros((size, size, size), dtype=np.int8)
        )

    def density(self) -> np.ndarray:
        """Compute flux density |J| at each point."""
        return np.linalg.norm(self.J, axis=-1)

    def divergence(self) -> np.ndarray:
        """Compute discrete divergence ∇·J."""
        div = np.zeros((self.size, self.size, self.size))
        for i in range(3):
            div += np.roll(self.J[..., i], -1, axis=i) - np.roll(self.J[..., i], 1, axis=i)
        return div / 2.0

    def curl(self) -> np.ndarray:
        """Compute discrete curl ∇×J."""
        curl = np.zeros_like(self.J)
        # curl_x = dJz/dy - dJy/dz
        curl[..., 0] = (np.roll(self.J[..., 2], -1, axis=1) - np.roll(self.J[..., 2], 1, axis=1) -
                        np.roll(self.J[..., 1], -1, axis=2) + np.roll(self.J[..., 1], 1, axis=2)) / 2.0
        # curl_y = dJx/dz - dJz/dx
        curl[..., 1] = (np.roll(self.J[..., 0], -1, axis=2) - np.roll(self.J[..., 0], 1, axis=2) -
                        np.roll(self.J[..., 2], -1, axis=0) + np.roll(self.J[..., 2], 1, axis=0)) / 2.0
        # curl_z = dJy/dx - dJx/dy
        curl[..., 2] = (np.roll(self.J[..., 1], -1, axis=0) - np.roll(self.J[..., 1], 1, axis=0) -
                        np.roll(self.J[..., 0], -1, axis=1) + np.roll(self.J[..., 0], 1, axis=1)) / 2.0
        return curl

    def laplacian(self) -> np.ndarray:
        """Compute discrete Laplacian ∇²J (6-connected)."""
        lap = np.zeros_like(self.J)
        for i in range(3):  # For each component
            for axis in range(3):  # For each spatial direction
                lap[..., i] += (np.roll(self.J[..., i], -1, axis=axis) +
                                np.roll(self.J[..., i], 1, axis=axis))
            lap[..., i] -= 6 * self.J[..., i]
        return lap

    def gradient_density(self) -> np.ndarray:
        """Compute gradient of density field ∇ρ."""
        rho = self.density()
        grad = np.zeros_like(self.J)
        for i in range(3):
            grad[..., i] = (np.roll(rho, -1, axis=i) - np.roll(rho, 1, axis=i)) / 2.0
        return grad


# =============================================================================
# Wave Propagation
# =============================================================================

def propagate_flux(field: FluxField, dt: float = 1.0) -> None:
    """
    Propagate flux field according to discrete wave equation.

    ∂²J/∂t² = C² ∇²J - γ ∂J/∂t

    Using velocity-Verlet integration.
    """
    # Compute Laplacian
    lap = field.laplacian()

    # Update velocity: v += C² * ∇²J * dt
    field.velocity += C**2 * lap * dt

    # Apply damping
    field.velocity *= (1 - DAMPING)

    # Update flux: J += v * dt
    field.J += field.velocity * dt


def add_flux_source(field: FluxField, position: Tuple[int, int, int],
                    amplitude: float, direction: np.ndarray) -> None:
    """Add a flux source at a given position."""
    x, y, z = position
    if 0 <= x < field.size and 0 <= y < field.size and 0 <= z < field.size:
        field.J[x, y, z] += amplitude * direction / np.linalg.norm(direction)


def add_gaussian_pulse(field: FluxField, center: Tuple[int, int, int],
                       sigma: float, amplitude: float, direction: np.ndarray) -> None:
    """Add a Gaussian flux pulse centered at a position."""
    cx, cy, cz = center
    direction = direction / np.linalg.norm(direction)

    for x in range(field.size):
        for y in range(field.size):
            for z in range(field.size):
                r2 = (x - cx)**2 + (y - cy)**2 + (z - cz)**2
                field.J[x, y, z] += amplitude * np.exp(-r2 / (2 * sigma**2)) * direction


# =============================================================================
# Flux Exclusion Zone
# =============================================================================

def create_exclusion_zone(field: FluxField, center: Tuple[int, int, int],
                          radius: float, strength: float) -> None:
    """
    Create a flux exclusion zone (flux band gap).

    In the exclusion zone, flux is damped more strongly,
    simulating the effect of 8 THz driving.
    """
    cx, cy, cz = center

    for x in range(field.size):
        for y in range(field.size):
            for z in range(field.size):
                r = np.sqrt((x - cx)**2 + (y - cy)**2 + (z - cz)**2)
                if r < radius:
                    # Exponentially damp flux inside exclusion zone
                    damping_factor = np.exp(-strength * (1 - r/radius))
                    field.J[x, y, z] *= damping_factor
                    field.velocity[x, y, z] *= damping_factor


def measure_flux_exclusion(field: FluxField, center: Tuple[int, int, int],
                           radius: float) -> dict:
    """Measure flux characteristics inside vs outside exclusion zone."""
    cx, cy, cz = center
    inside_flux = []
    outside_flux = []

    for x in range(field.size):
        for y in range(field.size):
            for z in range(field.size):
                r = np.sqrt((x - cx)**2 + (y - cy)**2 + (z - cz)**2)
                rho = np.linalg.norm(field.J[x, y, z])
                if r < radius:
                    inside_flux.append(rho)
                else:
                    outside_flux.append(rho)

    return {
        'inside_mean': np.mean(inside_flux) if inside_flux else 0,
        'inside_std': np.std(inside_flux) if inside_flux else 0,
        'outside_mean': np.mean(outside_flux) if outside_flux else 0,
        'outside_std': np.std(outside_flux) if outside_flux else 0,
        'exclusion_ratio': (np.mean(outside_flux) / np.mean(inside_flux)
                          if inside_flux and np.mean(inside_flux) > 0 else float('inf'))
    }


# =============================================================================
# Standing Wave Formation
# =============================================================================

def create_standing_wave(field: FluxField, wavelength: float,
                         direction: int = 0) -> None:
    """
    Initialize a standing wave pattern along one axis.

    J(x) = A * sin(2π x / λ)
    """
    k = 2 * np.pi / wavelength

    for x in range(field.size):
        for y in range(field.size):
            for z in range(field.size):
                pos = [x, y, z][direction]
                amplitude = np.sin(k * pos)
                # Set flux perpendicular to propagation direction
                perp_dir = (direction + 1) % 3
                field.J[x, y, z, perp_dir] = amplitude


def find_standing_wave_nodes(field: FluxField, axis: int = 0,
                             threshold: float = 0.1) -> list:
    """Find positions of standing wave nodes (low flux density)."""
    nodes = []

    # Average over perpendicular directions
    if axis == 0:
        profile = np.mean(field.density(), axis=(1, 2))
    elif axis == 1:
        profile = np.mean(field.density(), axis=(0, 2))
    else:
        profile = np.mean(field.density(), axis=(0, 1))

    # Find local minima below threshold
    for i in range(1, len(profile) - 1):
        if profile[i] < profile[i-1] and profile[i] < profile[i+1]:
            if profile[i] < threshold:
                nodes.append(i)

    return nodes


# =============================================================================
# Gravitational Effects
# =============================================================================

def compute_gravitational_force(field: FluxField) -> np.ndarray:
    """
    Compute gravity-like force from flux density gradient.

    F_grav = G_N * ∇ρ̄

    Where ρ̄ is the smoothed density field.
    """
    # Smooth density field (average over neighbors)
    rho = field.density()
    rho_smooth = np.zeros_like(rho)

    for axis in range(3):
        rho_smooth += np.roll(rho, -1, axis=axis) + np.roll(rho, 1, axis=axis)
    rho_smooth /= 6.0

    # Compute gradient
    force = np.zeros_like(field.J)
    for i in range(3):
        force[..., i] = G_N * (np.roll(rho_smooth, -1, axis=i) -
                               np.roll(rho_smooth, 1, axis=i)) / 2.0

    return force


def compute_effective_mass(field: FluxField, position: Tuple[int, int, int],
                           radius: float = 3.0) -> float:
    """
    Compute effective gravitational mass at a position.

    In FTD, mass = response to flux gradients.
    In an exclusion zone, effective mass → 0.
    """
    cx, cy, cz = position

    # Sum flux density in local region
    total_flux = 0.0
    count = 0

    for x in range(max(0, int(cx-radius)), min(field.size, int(cx+radius+1))):
        for y in range(max(0, int(cy-radius)), min(field.size, int(cy+radius+1))):
            for z in range(max(0, int(cz-radius)), min(field.size, int(cz+radius+1))):
                r = np.sqrt((x-cx)**2 + (y-cy)**2 + (z-cz)**2)
                if r <= radius:
                    total_flux += np.linalg.norm(field.J[x, y, z])
                    count += 1

    return total_flux / count if count > 0 else 0.0


# =============================================================================
# Visualization
# =============================================================================

def plot_flux_slice(field: FluxField, z_slice: int, save_path: Optional[str] = None):
    """Plot a 2D slice of the flux field."""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    # Density
    ax = axes[0]
    rho = field.density()[:, :, z_slice]
    im = ax.imshow(rho.T, origin='lower', cmap='viridis')
    ax.set_title(f'Flux Density |J| (z={z_slice})')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    plt.colorbar(im, ax=ax)

    # Divergence
    ax = axes[1]
    div = field.divergence()[:, :, z_slice]
    im = ax.imshow(div.T, origin='lower', cmap='RdBu', vmin=-div.max(), vmax=div.max())
    ax.set_title(f'Divergence ∇·J (z={z_slice})')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    plt.colorbar(im, ax=ax)

    # Vector field
    ax = axes[2]
    X, Y = np.meshgrid(range(field.size), range(field.size))
    U = field.J[:, :, z_slice, 0].T
    V = field.J[:, :, z_slice, 1].T
    ax.quiver(X[::2, ::2], Y[::2, ::2], U[::2, ::2], V[::2, ::2])
    ax.set_title(f'Flux Vectors (z={z_slice})')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_aspect('equal')

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=150)
        print(f"Saved to {save_path}")
    else:
        plt.show()

    plt.close()


def plot_density_3d(field: FluxField, threshold: float = 0.1,
                    save_path: Optional[str] = None):
    """Plot 3D isosurface of flux density."""
    from mpl_toolkits.mplot3d.art3d import Poly3DCollection
    from skimage import measure

    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')

    rho = field.density()

    # Create isosurface
    try:
        verts, faces, _, _ = measure.marching_cubes(rho, threshold)
        mesh = Poly3DCollection(verts[faces], alpha=0.3)
        mesh.set_facecolor('cyan')
        mesh.set_edgecolor('blue')
        ax.add_collection3d(mesh)
    except:
        print("Could not create isosurface (may need scikit-image)")

    ax.set_xlim(0, field.size)
    ax.set_ylim(0, field.size)
    ax.set_zlim(0, field.size)
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_title(f'Flux Density Isosurface (ρ = {threshold})')

    if save_path:
        plt.savefig(save_path, dpi=150)
        print(f"Saved to {save_path}")
    else:
        plt.show()

    plt.close()


# =============================================================================
# Simulation Scenarios
# =============================================================================

def simulate_wave_propagation(size: int = 64, steps: int = 100):
    """Demonstrate flux wave propagation from a point source."""
    print("=" * 60)
    print("SIMULATION: Flux Wave Propagation")
    print("=" * 60)

    field = FluxField.create(size)
    center = (size // 2, size // 2, size // 2)

    # Add initial pulse
    add_gaussian_pulse(field, center, sigma=3.0, amplitude=1.0,
                       direction=np.array([1.0, 0.0, 0.0]))

    print(f"Initial flux density at center: {field.density()[center]:.4f}")

    # Propagate
    energies = []
    for step in range(steps):
        propagate_flux(field)
        total_energy = np.sum(field.density()**2)
        energies.append(total_energy)

        if step % 20 == 0:
            print(f"Step {step}: Total energy = {total_energy:.4f}")

    # Final state
    print(f"\nFinal flux density at center: {field.density()[center]:.4f}")
    print(f"Energy conservation: {energies[-1]/energies[0]*100:.1f}% remaining")

    # Plot
    plot_flux_slice(field, size // 2, save_path='wave_propagation.png')

    return field, energies


def simulate_flux_exclusion(size: int = 64, steps: int = 50):
    """Demonstrate flux exclusion zone (antigravity effect)."""
    print("\n" + "=" * 60)
    print("SIMULATION: Flux Exclusion Zone")
    print("=" * 60)

    field = FluxField.create(size)
    center = (size // 2, size // 2, size // 2)
    exclusion_radius = 10

    # Create ambient flux field (uniform background)
    field.J[:, :, :, 0] = 0.5

    # Add some wave motion
    create_standing_wave(field, wavelength=size/4, direction=0)

    # Measure before exclusion
    before = measure_flux_exclusion(field, center, exclusion_radius)
    print(f"\nBefore exclusion zone:")
    print(f"  Inside mean flux:  {before['inside_mean']:.4f}")
    print(f"  Outside mean flux: {before['outside_mean']:.4f}")

    # Create exclusion zone
    create_exclusion_zone(field, center, exclusion_radius, strength=5.0)

    # Propagate with exclusion
    for step in range(steps):
        propagate_flux(field)
        create_exclusion_zone(field, center, exclusion_radius, strength=0.5)

    # Measure after exclusion
    after = measure_flux_exclusion(field, center, exclusion_radius)
    print(f"\nAfter exclusion zone ({steps} steps):")
    print(f"  Inside mean flux:  {after['inside_mean']:.4f}")
    print(f"  Outside mean flux: {after['outside_mean']:.4f}")
    print(f"  Exclusion ratio:   {after['exclusion_ratio']:.2f}x")

    # Compute effective mass
    mass_inside = compute_effective_mass(field, center, exclusion_radius)
    mass_outside = compute_effective_mass(field, (5, 5, 5), exclusion_radius)
    print(f"\nEffective mass comparison:")
    print(f"  Inside exclusion zone:  {mass_inside:.4f}")
    print(f"  Outside exclusion zone: {mass_outside:.4f}")
    print(f"  Mass reduction:         {(1 - mass_inside/mass_outside)*100:.1f}%")

    # Plot
    plot_flux_slice(field, size // 2, save_path='flux_exclusion.png')

    return field


def simulate_standing_waves(size: int = 64):
    """Demonstrate standing wave formation and node detection."""
    print("\n" + "=" * 60)
    print("SIMULATION: Standing Wave Formation")
    print("=" * 60)

    field = FluxField.create(size)

    # Test different wavelengths (FTD-significant)
    wavelengths = [
        size / N_C,      # 3 nodes
        size / N_BASE,   # 4 nodes
        size / B_3,      # 7 nodes
        size / N_EFF,    # 13 nodes
    ]

    for wl in wavelengths:
        field = FluxField.create(size)
        create_standing_wave(field, wavelength=wl, direction=0)
        nodes = find_standing_wave_nodes(field, axis=0, threshold=0.5)

        expected_nodes = int(size / wl)
        print(f"\nWavelength = {wl:.1f} (size/{size/wl:.0f}):")
        print(f"  Expected nodes: ~{expected_nodes}")
        print(f"  Found nodes:    {len(nodes)} at positions {nodes[:10]}...")

    # Create final visualization with N_eff wavelength
    field = FluxField.create(size)
    create_standing_wave(field, wavelength=size/N_EFF, direction=0)
    plot_flux_slice(field, size // 2, save_path='standing_waves.png')

    return field


def simulate_gravity_gradient(size: int = 64, steps: int = 100):
    """Demonstrate gravitational attraction from flux density gradient."""
    print("\n" + "=" * 60)
    print("SIMULATION: Gravitational Flux Gradient")
    print("=" * 60)

    field = FluxField.create(size)
    center = (size // 2, size // 2, size // 2)

    # Create a massive object (high flux concentration)
    add_gaussian_pulse(field, center, sigma=5.0, amplitude=2.0,
                       direction=np.array([1.0, 1.0, 1.0]))

    # Compute gravitational force field
    force = compute_gravitational_force(field)

    # Check force points toward center
    test_point = (size // 4, size // 2, size // 2)
    force_at_test = force[test_point]
    direction_to_center = np.array(center) - np.array(test_point)
    direction_to_center = direction_to_center / np.linalg.norm(direction_to_center)

    force_magnitude = np.linalg.norm(force_at_test)
    alignment = np.dot(force_at_test / force_magnitude if force_magnitude > 0 else force_at_test,
                       direction_to_center)

    print(f"\nMass center at: {center}")
    print(f"Test point at:  {test_point}")
    print(f"Force magnitude: {force_magnitude:.6f}")
    print(f"Direction alignment with center: {alignment:.4f}")
    print(f"  (1.0 = perfect attraction, -1.0 = repulsion)")

    # Visualize force field
    fig, ax = plt.subplots(figsize=(10, 8))
    z_slice = size // 2

    rho = field.density()[:, :, z_slice]
    ax.imshow(rho.T, origin='lower', cmap='viridis', alpha=0.5)

    X, Y = np.meshgrid(range(size), range(size))
    U = force[:, :, z_slice, 0].T
    V = force[:, :, z_slice, 1].T
    ax.quiver(X[::4, ::4], Y[::4, ::4], U[::4, ::4], V[::4, ::4],
              color='white', scale=0.1)

    ax.set_title('Gravitational Force Field (arrows) over Flux Density')
    ax.set_xlabel('x')
    ax.set_ylabel('y')

    plt.savefig('gravity_gradient.png', dpi=150)
    print("\nSaved to gravity_gradient.png")
    plt.close()

    return field, force


# =============================================================================
# Main
# =============================================================================

def run_all_simulations():
    """Run all demonstration simulations."""
    print("=" * 60)
    print("FTD FLUX FIELD DYNAMICS SIMULATIONS")
    print("=" * 60)
    print(f"\nFTD Parameters:")
    print(f"  C (speed of causality): {C}")
    print(f"  KB (manifestation threshold): {KB}")
    print(f"  α (fine structure): {ALPHA:.6f}")
    print(f"  G_N (gravity coupling): {G_N}")
    print(f"\nFTD Integers: N_c={N_C}, N_base={N_BASE}, b_3={B_3}, N_eff={N_EFF}")

    # Run simulations
    simulate_wave_propagation()
    simulate_flux_exclusion()
    simulate_standing_waves()
    simulate_gravity_gradient()

    print("\n" + "=" * 60)
    print("ALL SIMULATIONS COMPLETE")
    print("=" * 60)
    print("\nGenerated files:")
    print("  - wave_propagation.png")
    print("  - flux_exclusion.png")
    print("  - standing_waves.png")
    print("  - gravity_gradient.png")


if __name__ == "__main__":
    run_all_simulations()
