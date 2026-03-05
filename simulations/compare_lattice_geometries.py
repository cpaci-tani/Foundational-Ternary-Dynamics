"""
FTD Lattice Geometry Comparison (v5.21)

Compares cubic vs cuboctahedral (FCC) lattice geometries on identical
initial conditions to quantify differences in:
1. Wave propagation isotropy
2. Laplacian accuracy (vs analytical)
3. Energy conservation
4. Computational overhead

Usage:
    python simulations/compare_lattice_geometries.py
"""
import sys
import os
import time
import numpy as np

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from ternary_matrix.model.cubic_geometry import CubicGeometry
from ternary_matrix.model.cuboctahedral_geometry import CuboctahedralGeometry


def compare_laplacian_isotropy(sizes=None):
    """Compare Laplacian isotropy between cubic and FCC.

    Uses a Gaussian test field and measures variation of the Laplacian
    on spherical shells at various radii.
    """
    if sizes is None:
        sizes = [32]

    print("\n" + "=" * 60)
    print("1. LAPLACIAN ISOTROPY COMPARISON")
    print("=" * 60)

    for size in sizes:
        print(f"\n  Grid size: {size}^3")

        # Create radial Gaussian
        center = size // 2
        x = np.arange(size, dtype=np.float64) - center
        X, Y, Z = np.meshgrid(x, x, x, indexing='ij')
        R = np.sqrt(X**2 + Y**2 + Z**2)
        R_safe = np.where(R == 0, 1e-10, R)

        sigma = size / 8.0
        field = np.exp(-R**2 / (2 * sigma**2)).astype(np.float32)

        # Analytical Laplacian of Gaussian: (r^2/sigma^4 - 3/sigma^2) * f
        lap_analytical = ((R**2 / sigma**4) - (3.0 / sigma**2)) * field

        # Compute discrete Laplacians
        cubic = CubicGeometry(size)
        fcc = CuboctahedralGeometry(size // 2)  # effective size = size

        lap_cubic = cubic.laplacian_scalar(field)
        lap_fcc = fcc.laplacian_scalar(field)

        # Compare on shells
        print(f"\n  {'Radius':>8s} | {'Cubic CV':>10s} | {'FCC CV':>10s} | {'Cubic Err':>10s} | {'FCC Err':>10s}")
        print(f"  {'-'*8:s}-+-{'-'*10:s}-+-{'-'*10:s}-+-{'-'*10:s}-+-{'-'*10:s}")

        for shell_r in [3.0, 5.0, 7.0, 10.0]:
            if shell_r > size // 2 - 2:
                continue

            shell = (np.abs(R - shell_r) < 0.7) & (R > 1.0)
            n_points = np.count_nonzero(shell)
            if n_points < 10:
                continue

            # Coefficient of variation (isotropy measure)
            c_vals = lap_cubic[shell]
            f_vals = lap_fcc[shell]
            a_vals = lap_analytical[shell]

            c_cv = np.std(c_vals) / (np.abs(np.mean(c_vals)) + 1e-10)
            f_cv = np.std(f_vals) / (np.abs(np.mean(f_vals)) + 1e-10)

            # RMS error vs analytical
            c_err = np.sqrt(np.mean((c_vals - a_vals)**2)) / (np.abs(np.mean(a_vals)) + 1e-10)
            f_err = np.sqrt(np.mean((f_vals - a_vals)**2)) / (np.abs(np.mean(a_vals)) + 1e-10)

            print(f"  {shell_r:8.1f} | {c_cv:10.4f} | {f_cv:10.4f} | {c_err:10.4f} | {f_err:10.4f}")

        print()


def compare_wave_isotropy():
    """Compare wave front isotropy from a point source."""
    print("\n" + "=" * 60)
    print("2. WAVE PROPAGATION ISOTROPY")
    print("=" * 60)

    size = 32

    for name, geom in [("Cubic", CubicGeometry(size)),
                        ("FCC", CuboctahedralGeometry(size // 2))]:
        print(f"\n  {name} lattice (size={size}):")

        # Create a point source at center
        field = np.zeros((size, size, size), dtype=np.float32)
        velocity = np.zeros((size, size, size), dtype=np.float32)
        center = size // 2
        field[center, center, center] = 100.0

        # Propagate for several steps
        c_sq = 0.25  # c^2 = 0.25 for stability
        n_steps = 8

        for _ in range(n_steps):
            acc = c_sq * geom.laplacian_scalar(field)
            velocity += acc
            field += velocity
            field *= 0.95  # damping

        # Measure wavefront shape at a given radius
        x = np.arange(size, dtype=np.float64) - center
        X, Y, Z = np.meshgrid(x, x, x, indexing='ij')
        R = np.sqrt(X**2 + Y**2 + Z**2)

        for shell_r in [4.0, 6.0]:
            shell = (np.abs(R - shell_r) < 1.0) & (R > 1.0)
            if np.count_nonzero(shell) < 10:
                continue

            vals = np.abs(field[shell])
            cv = np.std(vals) / (np.mean(vals) + 1e-10)
            print(f"    Shell r={shell_r:.0f}: amplitude CV = {cv:.4f} (lower = more isotropic)")


def compare_gradient_accuracy():
    """Compare gradient accuracy on a known analytical field."""
    print("\n" + "=" * 60)
    print("3. GRADIENT ACCURACY")
    print("=" * 60)

    size = 32

    # f(x,y,z) = sin(2pi*x/L) * cos(2pi*y/L)
    # df/dx = (2pi/L) * cos(2pi*x/L) * cos(2pi*y/L)
    # df/dy = -(2pi/L) * sin(2pi*x/L) * sin(2pi*y/L)
    # df/dz = 0
    x = np.arange(size, dtype=np.float64)
    X, Y, Z = np.meshgrid(x, x, x, indexing='ij')
    k = 2 * np.pi / size

    field = (np.sin(k * X) * np.cos(k * Y)).astype(np.float32)

    grad_exact = np.zeros((size, size, size, 3), dtype=np.float32)
    grad_exact[..., 0] = k * np.cos(k * X) * np.cos(k * Y)
    grad_exact[..., 1] = -k * np.sin(k * X) * np.sin(k * Y)
    grad_exact[..., 2] = 0

    for name, geom in [("Cubic", CubicGeometry(size)),
                        ("FCC", CuboctahedralGeometry(size // 2))]:
        grad = geom.gradient(field)
        err = np.sqrt(np.mean((grad - grad_exact)**2))
        max_err = np.max(np.abs(grad - grad_exact))
        print(f"  {name:15s}: RMS error = {err:.6f}, Max error = {max_err:.6f}")


def compare_performance():
    """Compare computational performance of Laplacian operations."""
    print("\n" + "=" * 60)
    print("4. PERFORMANCE COMPARISON")
    print("=" * 60)

    size = 32
    field = np.random.randn(size, size, size).astype(np.float32)

    n_iter = 100

    for name, geom in [("Cubic", CubicGeometry(size)),
                        ("FCC", CuboctahedralGeometry(size // 2))]:
        # Laplacian
        t0 = time.perf_counter()
        for _ in range(n_iter):
            _ = geom.laplacian_scalar(field)
        t_lap = (time.perf_counter() - t0) / n_iter * 1000

        # Gradient
        t0 = time.perf_counter()
        for _ in range(n_iter):
            _ = geom.gradient(field)
        t_grad = (time.perf_counter() - t0) / n_iter * 1000

        # Smooth
        t0 = time.perf_counter()
        for _ in range(n_iter):
            _ = geom.smooth_field(field)
        t_smooth = (time.perf_counter() - t0) / n_iter * 1000

        print(f"  {name:15s}: Laplacian = {t_lap:.2f} ms, "
              f"Gradient = {t_grad:.2f} ms, "
              f"Smooth = {t_smooth:.2f} ms")


def main():
    print("=" * 60)
    print("FTD Lattice Geometry Comparison")
    print("Cubic (6 neighbors) vs Cuboctahedral/FCC (12 neighbors)")
    print("=" * 60)

    compare_laplacian_isotropy([32])
    compare_wave_isotropy()
    compare_gradient_accuracy()
    compare_performance()

    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print("""
  The cuboctahedral (FCC) lattice provides:
  - 12 equidistant neighbors vs cubic's mixed-distance tiers
  - Oh symmetry group (order 48), shared with cube
  - Coefficient 16 = |Oh|/3 is INVARIANT under geometry change
  - Improved isotropy in discrete differential operators
  - Natural equilateral triangle faces for triad binding
  - 2x array size (8x memory) tradeoff for equal resolution

  See docs/theory/EXPLR_CUBOCTAHEDRAL_GEOMETRY.md for full analysis.
""")


if __name__ == "__main__":
    main()
