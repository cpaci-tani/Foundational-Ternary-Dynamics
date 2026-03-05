#!/usr/bin/env python3
"""
Deep Analysis of the User's Parametric Curve Variant

Curve Definition:
    x(t) = cos(t) + 0.5*cos(2t) + 0.5*cos(4t) + 0.375*cos(8t)
    y(t) = 2*sin(t) - sin(2t) + sin(4t) - 0.75*sin(8t)

This script performs comprehensive analysis including:
- Basic geometric properties (arc length, bounding box, area)
- Topological features (winding number, self-intersections)
- Comparison with Lemniscate-Alpha
- Constant hunting (does arc length encode something?)
"""

import numpy as np
from math import gamma, sqrt, pi, gcd
from scipy import optimize
from scipy.spatial.distance import cdist
import matplotlib.pyplot as plt

# =============================================================================
# CONSTANTS
# =============================================================================

# Lemniscatic constant
G_STAR = (sqrt(2) * gamma(0.25)**2) / (2 * pi)

# FTD integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13

# Fine structure constant
ALPHA = 1 / 137.035999084

# Golden ratio
PHI = (1 + sqrt(5)) / 2

# =============================================================================
# USER'S CURVE DEFINITION
# =============================================================================

# Frequencies (powers of 2)
USER_FREQS = np.array([1, 2, 4, 8])

# User's coefficients
USER_X_AMPS = np.array([1.0, 0.5, 0.5, 0.375])
USER_Y_AMPS = np.array([2.0, -1.0, 1.0, -0.75])

# Lemniscate-Alpha coefficients for comparison
LA_FREQS = np.array([1, 2, 4, 8, 16])
LA_X_AMPS = np.array([1.0, 0.5, 0.5, 0.4, 0.0625])
LA_Y_AMPS = np.array([1.0, -0.5, 0.5, -0.35, 0.0625])


def user_curve(t):
    """Compute the user's parametric curve."""
    t = np.asarray(t)
    x = np.sum([USER_X_AMPS[j] * np.cos(USER_FREQS[j] * t) for j in range(4)], axis=0)
    y = np.sum([USER_Y_AMPS[j] * np.sin(USER_FREQS[j] * t) for j in range(4)], axis=0)
    return x, y


def user_curve_derivative(t):
    """Compute dx/dt and dy/dt for user's curve."""
    t = np.asarray(t)
    dx = np.sum([-USER_FREQS[j] * USER_X_AMPS[j] * np.sin(USER_FREQS[j] * t) for j in range(4)], axis=0)
    dy = np.sum([USER_FREQS[j] * USER_Y_AMPS[j] * np.cos(USER_FREQS[j] * t) for j in range(4)], axis=0)
    return dx, dy


def lemniscate_alpha(t):
    """Compute the Lemniscate-Alpha curve for comparison."""
    t = np.asarray(t)
    x = np.sum([LA_X_AMPS[j] * np.cos(LA_FREQS[j] * t) for j in range(5)], axis=0)
    y = np.sum([LA_Y_AMPS[j] * np.sin(LA_FREQS[j] * t) for j in range(5)], axis=0)
    return x, y


def lemniscate_alpha_derivative(t):
    """Compute dx/dt and dy/dt for Lemniscate-Alpha."""
    t = np.asarray(t)
    dx = np.sum([-LA_FREQS[j] * LA_X_AMPS[j] * np.sin(LA_FREQS[j] * t) for j in range(5)], axis=0)
    dy = np.sum([LA_FREQS[j] * LA_Y_AMPS[j] * np.cos(LA_FREQS[j] * t) for j in range(5)], axis=0)
    return dx, dy


# =============================================================================
# PHASE 1: BASIC PROPERTIES
# =============================================================================

def compute_arc_length(curve_func, deriv_func, n_points=100000):
    """Compute arc length by numerical integration."""
    t = np.linspace(0, 2*pi, n_points)
    dx, dy = deriv_func(t)
    dt = 2 * pi / n_points
    L = np.sum(np.sqrt(dx**2 + dy**2)) * dt
    return L


def compute_bounding_box(curve_func, n_points=10000):
    """Compute exact bounding box and related properties."""
    t = np.linspace(0, 2*pi, n_points)
    x, y = curve_func(t)

    x_min, x_max = np.min(x), np.max(x)
    y_min, y_max = np.min(y), np.max(y)

    width = x_max - x_min
    height = y_max - y_min
    aspect = height / width

    # Center of bounding box
    cx = (x_min + x_max) / 2
    cy = (y_min + y_max) / 2

    # Centroid (center of mass of curve points)
    centroid_x = np.mean(x)
    centroid_y = np.mean(y)

    return {
        'x_min': x_min, 'x_max': x_max,
        'y_min': y_min, 'y_max': y_max,
        'width': width, 'height': height,
        'aspect_ratio': aspect,
        'box_center': (cx, cy),
        'centroid': (centroid_x, centroid_y)
    }


def compute_winding_number(curve_func, n_points=10000):
    """Compute winding number around origin."""
    t = np.linspace(0, 2*pi, n_points)
    x, y = curve_func(t)

    # Angle from origin
    angles = np.arctan2(y, x)
    angles_unwrapped = np.unwrap(angles)

    # Winding = total angle change / 2*pi
    winding = (angles_unwrapped[-1] - angles_unwrapped[0]) / (2 * pi)

    return winding


def find_minimum_distance_to_origin(curve_func, n_points=10000):
    """Find minimum distance from curve to origin."""
    t = np.linspace(0, 2*pi, n_points)
    x, y = curve_func(t)
    distances = np.sqrt(x**2 + y**2)

    min_idx = np.argmin(distances)
    return {
        'min_dist': distances[min_idx],
        't_min': t[min_idx],
        'point': (x[min_idx], y[min_idx])
    }


def compute_signed_area(curve_func, deriv_func, n_points=10000):
    """Compute signed area using Green's theorem."""
    t = np.linspace(0, 2*pi, n_points)
    x, y = curve_func(t)
    dx, dy = deriv_func(t)
    dt = 2 * pi / n_points

    # Green's theorem: Area = 0.5 * integral(x*dy - y*dx)
    area = 0.5 * np.sum(x * dy - y * dx) * dt
    return area


# =============================================================================
# PHASE 2: TOPOLOGICAL ANALYSIS
# =============================================================================

def find_self_intersections(curve_func, n_points=2000, tolerance=0.01):
    """Find self-intersection points."""
    t = np.linspace(0, 2*pi, n_points, endpoint=False)
    x, y = curve_func(t)
    points = np.column_stack([x, y])

    intersections = []

    # Check pairs of points that are far apart in parameter but close in space
    for i in range(n_points):
        for j in range(i + n_points//10, n_points):  # Skip nearby points
            if np.abs(t[i] - t[j]) > 0.5:  # Only consider well-separated parameters
                dist = np.sqrt((x[i] - x[j])**2 + (y[i] - y[j])**2)
                if dist < tolerance:
                    intersections.append({
                        't1': t[i], 't2': t[j],
                        'point': ((x[i] + x[j])/2, (y[i] + y[j])/2),
                        'distance': dist
                    })

    # Remove duplicates (nearby intersections)
    unique = []
    for inter in intersections:
        is_dup = False
        for u in unique:
            if np.sqrt((inter['point'][0] - u['point'][0])**2 +
                       (inter['point'][1] - u['point'][1])**2) < 0.1:
                is_dup = True
                break
        if not is_dup:
            unique.append(inter)

    return unique


def find_critical_points(deriv_func, n_points=10000):
    """Find critical points (where dx/dt = 0 or dy/dt = 0)."""
    t = np.linspace(0, 2*pi, n_points, endpoint=False)
    dx, dy = deriv_func(t)

    # Find sign changes in dx (vertical tangent)
    vertical_tangents = []
    for i in range(len(dx) - 1):
        if dx[i] * dx[i+1] < 0:
            # Linear interpolation to find zero
            t_zero = t[i] - dx[i] * (t[i+1] - t[i]) / (dx[i+1] - dx[i])
            vertical_tangents.append(t_zero)

    # Find sign changes in dy (horizontal tangent)
    horizontal_tangents = []
    for i in range(len(dy) - 1):
        if dy[i] * dy[i+1] < 0:
            t_zero = t[i] - dy[i] * (t[i+1] - t[i]) / (dy[i+1] - dy[i])
            horizontal_tangents.append(t_zero)

    return {
        'vertical_tangents': vertical_tangents,
        'horizontal_tangents': horizontal_tangents
    }


# =============================================================================
# PHASE 3: COEFFICIENT ANALYSIS
# =============================================================================

def analyze_coefficients():
    """Analyze the coefficient structure."""
    print("\n" + "="*70)
    print("COEFFICIENT ANALYSIS")
    print("="*70)

    print("\nUser's curve coefficients:")
    print("X-amplitudes:", USER_X_AMPS)
    print("Y-amplitudes:", USER_Y_AMPS)

    print("\nLemniscate-Alpha coefficients:")
    print("X-amplitudes:", LA_X_AMPS[:4])  # Only first 4 for comparison
    print("Y-amplitudes:", LA_Y_AMPS[:4])

    print("\nRatios (User_Y / LA_Y):")
    for i, f in enumerate(USER_FREQS):
        ratio = USER_Y_AMPS[i] / LA_Y_AMPS[i]
        print(f"  Harmonic {f}: {USER_Y_AMPS[i]} / {LA_Y_AMPS[i]} = {ratio:.4f}")

    print("\nFractional representations:")
    print(f"  User X_8 = {USER_X_AMPS[3]} = 3/8 = {3/8}")
    print(f"  User Y_8 = {USER_Y_AMPS[3]} = -3/4 = {-3/4}")

    print("\nFTD integer connections:")
    print(f"  3/8 = N_c / (2 * N_base) = {N_C} / {2 * N_BASE} = {N_C / (2 * N_BASE)}")
    print(f"  3/4 = N_c / N_base = {N_C} / {N_BASE} = {N_C / N_BASE}")

    # Amplitude sums
    user_x_sum = np.sum(USER_X_AMPS)
    user_y_sum = np.sum(np.abs(USER_Y_AMPS))
    la_x_sum = np.sum(LA_X_AMPS)
    la_y_sum = np.sum(np.abs(LA_Y_AMPS))

    print(f"\nAmplitude sums:")
    print(f"  User X: {user_x_sum:.4f}")
    print(f"  User |Y|: {user_y_sum:.4f}")
    print(f"  LA X: {la_x_sum:.4f}")
    print(f"  LA |Y|: {la_y_sum:.4f}")


# =============================================================================
# PHASE 4: CONSTANT HUNTING
# =============================================================================

def hunt_for_constants(arc_length):
    """Search for mathematical constants encoded in arc length."""
    print("\n" + "="*70)
    print("CONSTANT HUNTING")
    print("="*70)

    print(f"\nArc length L = {arc_length:.10f}")

    # List of ratios to test
    constants = {
        'G*': G_STAR,
        'pi': pi,
        'phi': PHI,
        'sqrt(2)': sqrt(2),
        'sqrt(3)': sqrt(3),
        'e': np.e,
        '2*pi': 2*pi,
        'LA arc length': 23.7994,  # Lemniscate-Alpha
    }

    print("\nRatios L / constant:")
    for name, val in constants.items():
        ratio = arc_length / val
        # Check if ratio is close to simple fraction
        for num in range(1, 20):
            for den in range(1, 20):
                expected = num / den
                if abs(ratio - expected) / expected < 0.001:  # 0.1% match
                    print(f"  L / {name} = {ratio:.6f} ~ {num}/{den} (error: {abs(ratio - expected)/expected*100:.3f}%)")

    print("\nRatios constant / L:")
    for name, val in constants.items():
        ratio = val / arc_length
        for num in range(1, 20):
            for den in range(1, 20):
                expected = num / den
                if abs(ratio - expected) / expected < 0.01:  # 1% match
                    print(f"  {name} / L = {ratio:.6f} ~ {num}/{den}")

    # Special FTD-related ratios
    print("\nFTD integer combinations:")
    for n1 in [N_C, N_BASE, B_3, N_EFF]:
        for n2 in [N_C, N_BASE, B_3, N_EFF]:
            if n1 != n2:
                ratio = arc_length * n1 / n2
                if abs(ratio - round(ratio)) < 0.01:
                    print(f"  L * {n1}/{n2} = {ratio:.4f} ~ {round(ratio)}")


# =============================================================================
# VISUALIZATION
# =============================================================================

def create_comparison_plot():
    """Create side-by-side comparison with Lemniscate-Alpha."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 7))

    t = np.linspace(0, 2*pi, 3000)

    # User's curve
    ax1 = axes[0]
    x, y = user_curve(t)
    ax1.plot(x, y, 'b-', linewidth=1.5)
    ax1.scatter([0], [0], color='red', s=50, zorder=10, marker='o', label='Origin')
    ax1.set_title("User's Curve\nx = cos(t) + 0.5cos(2t) + 0.5cos(4t) + 0.375cos(8t)\n"
                  "y = 2sin(t) - sin(2t) + sin(4t) - 0.75sin(8t)")
    ax1.set_aspect('equal')
    ax1.grid(True, alpha=0.3)
    ax1.axhline(y=0, color='k', linewidth=0.5)
    ax1.axvline(x=0, color='k', linewidth=0.5)

    # Lemniscate-Alpha
    ax2 = axes[1]
    x, y = lemniscate_alpha(t)
    ax2.plot(x, y, 'g-', linewidth=1.5)
    ax2.scatter([0], [0], color='red', s=50, zorder=10, marker='o', label='Origin')
    ax2.set_title("Lemniscate-Alpha\n(5 harmonics: 1, 2, 4, 8, 16)")
    ax2.set_aspect('equal')
    ax2.grid(True, alpha=0.3)
    ax2.axhline(y=0, color='k', linewidth=0.5)
    ax2.axvline(x=0, color='k', linewidth=0.5)

    plt.tight_layout()
    plt.savefig('user_curve_comparison.png', dpi=150, bbox_inches='tight')
    print("\nSaved: user_curve_comparison.png")
    plt.close()


def create_analysis_plot(intersections, critical_points, min_dist_info):
    """Create annotated plot of user's curve."""
    fig, ax = plt.subplots(figsize=(12, 10))

    t = np.linspace(0, 2*pi, 3000)
    x, y = user_curve(t)

    # Main curve
    ax.plot(x, y, 'b-', linewidth=1.5, label="User's Curve")

    # Origin
    ax.scatter([0], [0], color='red', s=100, zorder=10, marker='o', label='Origin')

    # Minimum distance point
    ax.scatter([min_dist_info['point'][0]], [min_dist_info['point'][1]],
               color='orange', s=100, zorder=10, marker='*',
               label=f"Min dist: {min_dist_info['min_dist']:.4f}")
    ax.plot([0, min_dist_info['point'][0]], [0, min_dist_info['point'][1]],
            'orange', linestyle='--', alpha=0.5)

    # Self-intersections
    for i, inter in enumerate(intersections):
        ax.scatter([inter['point'][0]], [inter['point'][1]],
                   color='purple', s=80, zorder=10, marker='x')
    if intersections:
        ax.scatter([], [], color='purple', marker='x', label=f'Self-intersections ({len(intersections)})')

    # Critical points - vertical tangents
    for t_crit in critical_points['vertical_tangents'][:6]:  # Limit to avoid clutter
        xc, yc = user_curve(np.array([t_crit]))
        ax.scatter(xc, yc, color='green', s=50, zorder=10, marker='^')

    # Critical points - horizontal tangents
    for t_crit in critical_points['horizontal_tangents'][:6]:
        xc, yc = user_curve(np.array([t_crit]))
        ax.scatter(xc, yc, color='cyan', s=50, zorder=10, marker='v')

    ax.legend(loc='upper right')
    ax.set_title("User's Curve - Analysis", fontsize=14)
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.3)
    ax.axhline(y=0, color='k', linewidth=0.5)
    ax.axvline(x=0, color='k', linewidth=0.5)

    plt.tight_layout()
    plt.savefig('user_curve_analysis.png', dpi=150, bbox_inches='tight')
    print("Saved: user_curve_analysis.png")
    plt.close()


# =============================================================================
# MAIN ANALYSIS
# =============================================================================

def main():
    print("\n" + "="*70)
    print("DEEP ANALYSIS: USER'S PARAMETRIC CURVE VARIANT")
    print("="*70)

    # =========================================================================
    # PHASE 1: BASIC PROPERTIES
    # =========================================================================
    print("\n" + "="*70)
    print("PHASE 1: BASIC PROPERTIES")
    print("="*70)

    # Arc length
    user_L = compute_arc_length(user_curve, user_curve_derivative)
    la_L = compute_arc_length(lemniscate_alpha, lemniscate_alpha_derivative)

    print(f"\nArc Length:")
    print(f"  User's curve: {user_L:.10f}")
    print(f"  Lemniscate-Alpha: {la_L:.10f}")
    print(f"  Ratio: {user_L / la_L:.6f}")

    # Bounding box
    user_box = compute_bounding_box(user_curve)
    la_box = compute_bounding_box(lemniscate_alpha)

    print(f"\nBounding Box (User):")
    print(f"  X: [{user_box['x_min']:.4f}, {user_box['x_max']:.4f}]")
    print(f"  Y: [{user_box['y_min']:.4f}, {user_box['y_max']:.4f}]")
    print(f"  Width: {user_box['width']:.4f}, Height: {user_box['height']:.4f}")
    print(f"  Aspect ratio: {user_box['aspect_ratio']:.4f}")
    print(f"  Centroid: ({user_box['centroid'][0]:.4f}, {user_box['centroid'][1]:.4f})")

    print(f"\nBounding Box (LA):")
    print(f"  X: [{la_box['x_min']:.4f}, {la_box['x_max']:.4f}]")
    print(f"  Y: [{la_box['y_min']:.4f}, {la_box['y_max']:.4f}]")
    print(f"  Aspect ratio: {la_box['aspect_ratio']:.4f}")

    # Winding number
    user_winding = compute_winding_number(user_curve)
    la_winding = compute_winding_number(lemniscate_alpha)

    print(f"\nWinding Number (around origin):")
    print(f"  User's curve: {user_winding:.4f}")
    print(f"  Lemniscate-Alpha: {la_winding:.4f}")

    # Minimum distance to origin
    user_min = find_minimum_distance_to_origin(user_curve)
    la_min = find_minimum_distance_to_origin(lemniscate_alpha)

    print(f"\nMinimum Distance to Origin:")
    print(f"  User's curve: {user_min['min_dist']:.6f} at t={user_min['t_min']:.4f}")
    print(f"  Lemniscate-Alpha: {la_min['min_dist']:.6f} (G*^2/32 = {G_STAR**2/32:.6f})")

    # Signed area
    user_area = compute_signed_area(user_curve, user_curve_derivative)
    la_area = compute_signed_area(lemniscate_alpha, lemniscate_alpha_derivative)

    print(f"\nSigned Area:")
    print(f"  User's curve: {user_area:.6f}")
    print(f"  Lemniscate-Alpha: {la_area:.6f}")

    # =========================================================================
    # PHASE 2: TOPOLOGICAL ANALYSIS
    # =========================================================================
    print("\n" + "="*70)
    print("PHASE 2: TOPOLOGICAL ANALYSIS")
    print("="*70)

    # Self-intersections
    intersections = find_self_intersections(user_curve)
    print(f"\nSelf-intersections found: {len(intersections)}")
    for i, inter in enumerate(intersections):
        print(f"  {i+1}. t1={inter['t1']:.4f}, t2={inter['t2']:.4f}, "
              f"point=({inter['point'][0]:.4f}, {inter['point'][1]:.4f})")

    # Critical points
    critical = find_critical_points(user_curve_derivative)
    print(f"\nCritical Points:")
    print(f"  Vertical tangents (dx/dt=0): {len(critical['vertical_tangents'])}")
    print(f"  Horizontal tangents (dy/dt=0): {len(critical['horizontal_tangents'])}")

    # =========================================================================
    # PHASE 3: COEFFICIENT ANALYSIS
    # =========================================================================
    analyze_coefficients()

    # =========================================================================
    # PHASE 4: CONSTANT HUNTING
    # =========================================================================
    hunt_for_constants(user_L)

    # =========================================================================
    # COMPARISON TABLE
    # =========================================================================
    print("\n" + "="*70)
    print("COMPARISON SUMMARY")
    print("="*70)

    print("\n| Property | User's Curve | Lemniscate-Alpha |")
    print("|----------|--------------|------------------|")
    print(f"| Arc length | {user_L:.4f} | {la_L:.4f} |")
    print(f"| Winding number | {user_winding:.2f} | {la_winding:.2f} |")
    print(f"| Min dist to origin | {user_min['min_dist']:.4f} | {la_min['min_dist']:.4f} |")
    print(f"| Signed area | {user_area:.4f} | {la_area:.4f} |")
    print(f"| Self-intersections | {len(intersections)} | 0 |")
    print(f"| Aspect ratio | {user_box['aspect_ratio']:.2f} | {la_box['aspect_ratio']:.2f} |")
    print(f"| Harmonics | 4 (1,2,4,8) | 5 (1,2,4,8,16) |")

    # =========================================================================
    # KEY FINDINGS
    # =========================================================================
    print("\n" + "="*70)
    print("KEY FINDINGS")
    print("="*70)

    print(f"""
1. TOPOLOGICAL DIFFERENCE:
   - User's curve has {len(intersections)} self-intersection(s)
   - Lemniscate-Alpha has 0 (loops without crossing)
   - This represents a fundamental topological distinction

2. WINDING NUMBER:
   - User: {user_winding:.2f} vs LA: {la_winding:.2f}
   - Different winding = different encirclement of origin

3. Y-AMPLITUDE DOUBLING:
   - User Y-amps are exactly 2x the LA Y-amps (for harmonics 1,2,4)
   - This creates the vertical elongation (aspect ratio {user_box['aspect_ratio']:.2f})

4. COEFFICIENT STRUCTURE:
   - X_8 = 3/8 = N_c / (2*N_base) -- involves FTD integers!
   - Y_8 = -3/4 = -N_c / N_base -- also FTD integers!
   - The number 3 appears prominently

5. ARC LENGTH:
   - User: {user_L:.4f}
   - Ratio to LA: {user_L/la_L:.4f}
   - Searching for constant encoding...
""")

    # =========================================================================
    # VISUALIZATIONS
    # =========================================================================
    print("\nGenerating visualizations...")
    create_comparison_plot()
    create_analysis_plot(intersections, critical, user_min)

    print("\n" + "="*70)
    print("ANALYSIS COMPLETE")
    print("="*70)

    return {
        'arc_length': user_L,
        'winding': user_winding,
        'min_dist': user_min,
        'area': user_area,
        'intersections': intersections,
        'bounding_box': user_box,
        'critical_points': critical
    }


if __name__ == "__main__":
    results = main()
