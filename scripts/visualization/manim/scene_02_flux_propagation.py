"""
Simulation 2: Flux Field Propagation - Waves in the Substrate
=============================================================
A PyVista-based animation showing the discrete wave equation governing flux dynamics.

Design: 50x50x50 lattice showing spherical wave propagation from central disturbance
- Isosurfaces of |J| = const as expanding shells
- Velocity vectors showing c = 1 voxel/tick limit
- Interference patterns on boundary reflection

Run with: python scene_02_flux_propagation.py

Author: FTD Visualization Suite
Date: January 2026
"""

import numpy as np
import pyvista as pv
from pyvista import themes
import os
import sys

# Add utils to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'utils'))

# Constants
GRID_SIZE = 50
C_SQUARED = 1.0  # Speed of causality squared
DAMPING = 0.001  # Small damping for stability
N_FRAMES = 200
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'videos')

# Colors
BACKGROUND = '#0D1117'
FLUX_LOW = '#FFE066'
FLUX_MID = '#FFD700'
FLUX_HIGH = '#CC9900'


class FluxFieldSimulator:
    """Simulates the discrete wave equation on a 3D cubic lattice."""

    def __init__(self, size=GRID_SIZE):
        self.size = size
        self.center = size // 2

        # Flux field J (3 components) and its velocity
        self.J = np.zeros((size, size, size, 3), dtype=np.float64)
        self.J_velocity = np.zeros((size, size, size, 3), dtype=np.float64)

        # Scalar magnitude field for visualization
        self.magnitude = np.zeros((size, size, size), dtype=np.float64)

        # Initialize with central disturbance
        self._initialize_disturbance()

    def _initialize_disturbance(self):
        """Create initial Gaussian disturbance at center."""
        x = np.arange(self.size)
        y = np.arange(self.size)
        z = np.arange(self.size)
        X, Y, Z = np.meshgrid(x, y, z, indexing='ij')

        # Distance from center
        r = np.sqrt(
            (X - self.center)**2 +
            (Y - self.center)**2 +
            (Z - self.center)**2
        )

        # Gaussian profile
        sigma = 3.0
        amplitude = 2.0
        gaussian = amplitude * np.exp(-r**2 / (2 * sigma**2))

        # Set radial flux (pointing outward from center)
        for i in range(3):
            direction = [X - self.center, Y - self.center, Z - self.center][i]
            direction_normalized = np.where(r > 0, direction / (r + 1e-10), 0)
            self.J[:, :, :, i] = gaussian * direction_normalized

        self._update_magnitude()

    def _update_magnitude(self):
        """Compute |J| at each point."""
        self.magnitude = np.sqrt(np.sum(self.J**2, axis=3))

    def discrete_laplacian(self, field):
        """Compute discrete Laplacian using 6-connected neighborhood."""
        laplacian = np.zeros_like(field)

        # For each component
        for c in range(3):
            f = field[:, :, :, c]

            # Sum of neighbors minus 6 times center
            laplacian[:, :, :, c] = (
                np.roll(f, 1, axis=0) + np.roll(f, -1, axis=0) +
                np.roll(f, 1, axis=1) + np.roll(f, -1, axis=1) +
                np.roll(f, 1, axis=2) + np.roll(f, -1, axis=2) -
                6 * f
            )

        return laplacian

    def step(self):
        """Perform one time step of the wave equation."""
        # Wave equation: ∂²J/∂t² = c² ∇²J
        # Discretized: J_velocity += c² * laplacian(J)
        #              J += J_velocity
        #              J *= (1 - damping)

        laplacian = self.discrete_laplacian(self.J)
        self.J_velocity += C_SQUARED * laplacian
        self.J += self.J_velocity
        self.J *= (1 - DAMPING)

        self._update_magnitude()

    def get_isosurface_mesh(self, level=0.3):
        """Generate isosurface mesh at given flux magnitude level."""
        # Create PyVista uniform grid
        grid = pv.ImageData(
            dimensions=(self.size, self.size, self.size),
            spacing=(1, 1, 1),
            origin=(0, 0, 0)
        )
        grid['flux_magnitude'] = self.magnitude.flatten(order='F')

        # Extract isosurface
        try:
            contour = grid.contour([level], scalars='flux_magnitude')
            return contour
        except Exception:
            return None

    def get_vector_field(self, stride=5):
        """Get flux vectors for visualization (subsampled)."""
        # Subsample for visualization
        x = np.arange(0, self.size, stride)
        y = np.arange(0, self.size, stride)
        z = np.arange(0, self.size, stride)
        X, Y, Z = np.meshgrid(x, y, z, indexing='ij')

        points = np.column_stack([
            X.flatten(),
            Y.flatten(),
            Z.flatten()
        ])

        # Get vectors at these points
        vectors = np.zeros((len(points), 3))
        for i, (px, py, pz) in enumerate(points.astype(int)):
            vectors[i] = self.J[px, py, pz, :]

        return points, vectors


def create_visualization():
    """Create the main visualization."""
    print("Initializing flux field simulation...")
    sim = FluxFieldSimulator(GRID_SIZE)

    # Set up PyVista plotter
    pv.set_plot_theme('dark')

    plotter = pv.Plotter(
        off_screen=True,
        window_size=[1920, 1080]
    )
    plotter.set_background(BACKGROUND)

    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print(f"Rendering {N_FRAMES} frames...")

    # Open movie file
    movie_path = os.path.join(OUTPUT_DIR, 'flux_propagation.mp4')
    plotter.open_movie(movie_path, framerate=30, quality=9)

    # Add title text
    plotter.add_text(
        "Flux Field Propagation",
        position='upper_left',
        font_size=14,
        color='white'
    )

    # Add equation text
    plotter.add_text(
        "Wave equation: d²J/dt² = c² ∇²J",
        position='lower_left',
        font_size=10,
        color='#FFD700'
    )

    # Main animation loop
    for frame in range(N_FRAMES):
        plotter.clear_actors()

        # Add bounding box
        bounds = pv.Box(bounds=[0, GRID_SIZE, 0, GRID_SIZE, 0, GRID_SIZE])
        plotter.add_mesh(
            bounds,
            style='wireframe',
            color='gray',
            opacity=0.3,
            line_width=1
        )

        # Get isosurfaces at different levels
        levels = [0.1, 0.3, 0.6]
        opacities = [0.2, 0.5, 0.8]
        colors = [FLUX_LOW, FLUX_MID, FLUX_HIGH]

        for level, opacity, color in zip(levels, opacities, colors):
            mesh = sim.get_isosurface_mesh(level)
            if mesh is not None and mesh.n_points > 0:
                plotter.add_mesh(
                    mesh,
                    color=color,
                    opacity=opacity,
                    smooth_shading=True
                )

        # Add vector field (every 10 frames for performance)
        if frame % 10 == 0:
            points, vectors = sim.get_vector_field(stride=8)
            # Normalize vectors for visualization
            magnitudes = np.linalg.norm(vectors, axis=1, keepdims=True)
            magnitudes = np.where(magnitudes > 0, magnitudes, 1)
            normalized = vectors / magnitudes * 2  # Scale for visibility

            # Create glyphs
            if len(points) > 0:
                arrows = pv.PolyData(points)
                arrows['vectors'] = normalized
                arrows['magnitude'] = magnitudes.flatten()

                glyphs = arrows.glyph(
                    orient='vectors',
                    scale='magnitude',
                    factor=0.5
                )
                plotter.add_mesh(
                    glyphs,
                    color=FLUX_MID,
                    opacity=0.6
                )

        # Update camera
        angle = frame * 0.5
        radius = GRID_SIZE * 1.5
        cam_x = GRID_SIZE/2 + radius * np.cos(np.radians(angle))
        cam_y = GRID_SIZE/2 + radius * np.sin(np.radians(angle))
        cam_z = GRID_SIZE * 0.8

        plotter.camera.position = (cam_x, cam_y, cam_z)
        plotter.camera.focal_point = (GRID_SIZE/2, GRID_SIZE/2, GRID_SIZE/2)
        plotter.camera.up = (0, 0, 1)

        # Add time indicator
        plotter.add_text(
            f"t = {frame}",
            position='upper_right',
            font_size=12,
            color='white',
            name='time_text'
        )

        # Write frame
        plotter.write_frame()

        # Step simulation
        sim.step()

        if frame % 20 == 0:
            print(f"  Frame {frame}/{N_FRAMES} ({100*frame/N_FRAMES:.0f}%)")

    # Close movie
    plotter.close()
    print(f"\nAnimation saved to: {movie_path}")


def create_static_visualization():
    """Create a single static image for quick preview."""
    print("Creating static flux field visualization...")
    sim = FluxFieldSimulator(GRID_SIZE)

    # Run a few steps to get interesting pattern
    for _ in range(30):
        sim.step()

    # Set up plotter
    pv.set_plot_theme('dark')
    plotter = pv.Plotter(window_size=[1920, 1080])
    plotter.set_background(BACKGROUND)

    # Add bounding box
    bounds = pv.Box(bounds=[0, GRID_SIZE, 0, GRID_SIZE, 0, GRID_SIZE])
    plotter.add_mesh(bounds, style='wireframe', color='gray', opacity=0.3)

    # Add isosurfaces
    for level, opacity, color in [(0.1, 0.3, FLUX_LOW), (0.3, 0.6, FLUX_MID), (0.6, 0.9, FLUX_HIGH)]:
        mesh = sim.get_isosurface_mesh(level)
        if mesh is not None and mesh.n_points > 0:
            plotter.add_mesh(mesh, color=color, opacity=opacity, smooth_shading=True)

    # Add title
    plotter.add_text("Flux Field |J(v,t)| - Wave Propagation", position='upper_left', font_size=14, color='white')
    plotter.add_text("∂²J/∂t² = c² ∇²J", position='lower_left', font_size=12, color='#FFD700')

    # Save static image
    static_path = os.path.join(os.path.dirname(__file__), '..', 'static', 'flux_propagation.png')
    os.makedirs(os.path.dirname(static_path), exist_ok=True)

    plotter.show(screenshot=static_path, auto_close=True)
    print(f"Static image saved to: {static_path}")


def create_matplotlib_fallback():
    """Create a matplotlib-based visualization as fallback."""
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D

    print("Creating matplotlib fallback visualization...")
    sim = FluxFieldSimulator(30)  # Smaller for matplotlib

    # Run simulation
    for _ in range(20):
        sim.step()

    # Create figure
    fig = plt.figure(figsize=(16, 9), facecolor=BACKGROUND)
    ax = fig.add_subplot(111, projection='3d', facecolor=BACKGROUND)

    # Get points where magnitude > threshold
    threshold = 0.2
    x, y, z = np.where(sim.magnitude > threshold)
    magnitudes = sim.magnitude[x, y, z]

    # Scatter plot
    scatter = ax.scatter(
        x, y, z,
        c=magnitudes,
        cmap='YlOrRd',
        alpha=0.6,
        s=magnitudes * 50
    )

    # Styling
    ax.set_xlabel('X', color='white')
    ax.set_ylabel('Y', color='white')
    ax.set_zlabel('Z', color='white')
    ax.set_title('Flux Field Propagation |J(v,t)|', color='white', fontsize=16)

    ax.tick_params(colors='white')
    ax.xaxis.pane.fill = False
    ax.yaxis.pane.fill = False
    ax.zaxis.pane.fill = False

    plt.colorbar(scatter, label='|J|', shrink=0.5)

    # Save
    output_path = os.path.join(os.path.dirname(__file__), '..', 'static', 'flux_propagation_mpl.png')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, dpi=150, facecolor=BACKGROUND, edgecolor='none', bbox_inches='tight')
    plt.close()

    print(f"Matplotlib visualization saved to: {output_path}")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='FTD Flux Field Propagation Visualization')
    parser.add_argument('--mode', choices=['video', 'static', 'matplotlib'], default='matplotlib',
                        help='Visualization mode')
    args = parser.parse_args()

    if args.mode == 'video':
        create_visualization()
    elif args.mode == 'static':
        try:
            create_static_visualization()
        except Exception as e:
            print(f"PyVista failed ({e}), falling back to matplotlib...")
            create_matplotlib_fallback()
    else:
        create_matplotlib_fallback()
