#!/usr/bin/env python3
"""
Analyze e+e- annihilation angular distribution from FTD flux field export.

Reads the CSV produced by campaign_annihilation_angular.cpp, computes the
angular distribution of outgoing radiation (Poynting-like flux), bins into
18 cos(theta) bins, fits to (1 + B cos^2 theta), and reports B and chi-squared.

Usage:
    python scripts/experiments/analyze_annihilation_angular.py <flux_csv>
    python scripts/experiments/analyze_annihilation_angular.py output/annihilation_flux_t87.csv

Theory:
    For e+e- -> gamma gamma in QED, the differential cross section goes as
    dsigma/dOmega ~ (1 + cos^2 theta) at high energy, giving B = 1.
    In the FTD lattice, the angular distribution of radiated flux energy
    should approximate this form. B close to 1 indicates QED-like radiation.

Output:
    - Table of 18 cos(theta) bins with flux intensity
    - Fitted B parameter and chi-squared
    - Publication-quality plot (if matplotlib available)
"""

import sys
import csv
import math
import os
from pathlib import Path

# Add project root for constants import
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "scripts"))

try:
    from constants import ALPHA, C_SPEED, K_B
except ImportError:
    # Fallback values from ontic chain
    ALPHA = 1.0 / 137.036
    C_SPEED = 1.0 / math.sqrt(3.0)
    K_B = 0.511


def load_flux_field(csv_path):
    """Load flux field CSV exported by the C++ campaign test.

    Returns list of dicts with keys: x, y, z, Jx, Jy, Jz, density, state.
    Only returns sites with nonzero density (to reduce memory for large lattices).
    """
    sites = []
    with open(csv_path, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            density = float(row["density"])
            if density < 1e-12:
                continue
            sites.append({
                "x": int(row["x"]),
                "y": int(row["y"]),
                "z": int(row["z"]),
                "Jx": float(row["Jx"]),
                "Jy": float(row["Jy"]),
                "Jz": float(row["Jz"]),
                "density": density,
                "state": int(row["state"]),
            })
    return sites


def compute_angular_distribution(sites, n_bins=18):
    """Compute angular distribution of radiated flux energy.

    For each nonzero-flux site, compute the angle theta between the
    displacement vector (from lattice center to site) and the collision
    axis (x-axis). Bin the flux energy density |J|^2 into cos(theta) bins.

    The collision axis is x because the e+ and e- were placed along x.

    Returns:
        cos_centers: array of bin centers in cos(theta)
        intensities: array of summed |J|^2 in each bin
        errors: array of estimated uncertainties (sqrt(sum |J|^4))
    """
    # Determine lattice size from max coordinate
    max_coord = max(max(s["x"], s["y"], s["z"]) for s in sites)
    L = max_coord + 1
    center = L / 2.0

    cos_edges = [-1.0 + 2.0 * i / n_bins for i in range(n_bins + 1)]
    cos_centers = [0.5 * (cos_edges[i] + cos_edges[i + 1]) for i in range(n_bins)]
    intensities = [0.0] * n_bins
    sum_J4 = [0.0] * n_bins  # For error estimation
    counts = [0] * n_bins

    for s in sites:
        # Skip manifested particles (state != 0) -- we want radiation only
        if s["state"] != 0:
            continue

        # Displacement from collision center
        dx = s["x"] - center
        dy = s["y"] - center
        dz = s["z"] - center
        r = math.sqrt(dx * dx + dy * dy + dz * dz)

        if r < 3.0:
            # Skip sites too close to collision point (near-field)
            continue

        # cos(theta) where theta is angle from x-axis (collision axis)
        cos_theta = dx / r

        # Flux energy density: |J|^2
        J2 = s["Jx"] ** 2 + s["Jy"] ** 2 + s["Jz"] ** 2

        # Find bin
        bin_idx = int((cos_theta + 1.0) / 2.0 * n_bins)
        bin_idx = max(0, min(n_bins - 1, bin_idx))

        # Weight by 1/r^2 to get solid-angle-normalized intensity
        # (Radiation intensity falls as 1/r^2, so multiply by r^2 to recover
        #  the angular pattern)
        weight = J2 * r * r
        intensities[bin_idx] += weight
        sum_J4[bin_idx] += weight * weight
        counts[bin_idx] += 1

    # Normalize by solid angle of each bin
    # Solid angle of a cos(theta) bin of width d(cos_theta) is 2*pi*d(cos_theta)
    d_cos = 2.0 / n_bins
    for i in range(n_bins):
        solid_angle = 2.0 * math.pi * d_cos
        if solid_angle > 0:
            intensities[i] /= solid_angle

    # Error estimate: sqrt(variance) / sqrt(N)
    errors = []
    for i in range(n_bins):
        if counts[i] > 1:
            mean = intensities[i]
            # Use Poisson-like estimate
            errors.append(intensities[i] / math.sqrt(counts[i]))
        else:
            errors.append(intensities[i] * 0.5 if intensities[i] > 0 else 1.0)

    return cos_centers, intensities, errors, counts


def fit_angular_distribution(cos_centers, intensities, errors):
    """Fit intensities to A * (1 + B * cos^2(theta)) using least squares.

    Returns A, B, chi_squared, ndof.
    """
    # Weighted least squares: I(cos_theta) = A * (1 + B * cos^2(theta))
    # Let x = cos_theta, then model = A + A*B*x^2
    # Two parameters: A and C = A*B
    # Model: y = A + C * x^2
    # Normal equations with weights w_i = 1/sigma_i^2

    n = len(cos_centers)

    # Filter out zero-intensity bins
    valid = [(cos_centers[i], intensities[i], errors[i])
             for i in range(n) if intensities[i] > 0 and errors[i] > 0]

    if len(valid) < 3:
        print("WARNING: Fewer than 3 valid bins for fitting.")
        return 0.0, 0.0, float("inf"), 0

    xs = [v[0] for v in valid]
    ys = [v[1] for v in valid]
    ws = [1.0 / (v[2] ** 2) for v in valid]

    # Normal equations for y = A + C * x^2
    S_w = sum(ws)
    S_wx2 = sum(w * x ** 2 for w, x in zip(ws, xs))
    S_wx4 = sum(w * x ** 4 for w, x in zip(ws, xs))
    S_wy = sum(w * y for w, y in zip(ws, ys))
    S_wyx2 = sum(w * y * x ** 2 for w, y, x in zip(ws, ys, xs))

    # Solve 2x2 system:
    # [S_w    S_wx2 ] [A] = [S_wy  ]
    # [S_wx2  S_wx4 ] [C]   [S_wyx2]
    det = S_w * S_wx4 - S_wx2 * S_wx2
    if abs(det) < 1e-30:
        print("WARNING: Singular normal equation matrix.")
        return 0.0, 0.0, float("inf"), 0

    A = (S_wx4 * S_wy - S_wx2 * S_wyx2) / det
    C = (S_w * S_wyx2 - S_wx2 * S_wy) / det

    # B = C / A (from model A*(1 + B*x^2) = A + C*x^2)
    B = C / A if abs(A) > 1e-30 else 0.0

    # Chi-squared
    chi2 = sum(w * (y - A - C * x ** 2) ** 2
               for w, y, x in zip(ws, ys, xs))
    ndof = len(valid) - 2

    return A, B, chi2, ndof


def make_plot(cos_centers, intensities, errors, A, B, output_path):
    """Generate publication-quality angular distribution plot."""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError:
        print("  matplotlib not available, skipping plot.")
        return

    fig, ax = plt.subplots(1, 1, figsize=(8, 5))

    x = np.array(cos_centers)
    y = np.array(intensities)
    yerr = np.array(errors)

    # Data points
    ax.errorbar(x, y, yerr=yerr, fmt="ko", markersize=5, capsize=3,
                label="FTD simulation")

    # Fit curve
    x_fine = np.linspace(-1, 1, 200)
    y_fit = A * (1.0 + B * x_fine ** 2)
    ax.plot(x_fine, y_fit, "r-", linewidth=2,
            label=f"Fit: A(1 + B cos$^2\\theta$)\nB = {B:.3f}")

    # QED prediction for reference
    y_qed = A * (1.0 + 1.0 * x_fine ** 2)
    ax.plot(x_fine, y_qed, "b--", linewidth=1, alpha=0.5,
            label="QED: B = 1.0")

    ax.set_xlabel(r"cos $\theta$", fontsize=13)
    ax.set_ylabel("Flux intensity (arb. units)", fontsize=13)
    ax.set_title(r"$e^+e^- \to \gamma\gamma$ Angular Distribution (FTD Lattice)",
                 fontsize=14)
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"  Plot saved to: {output_path}")
    plt.close()


def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_annihilation_angular.py <flux_csv>")
        print("  e.g.: python analyze_annihilation_angular.py output/annihilation_flux_t87.csv")
        sys.exit(1)

    csv_path = sys.argv[1]
    if not os.path.exists(csv_path):
        print(f"ERROR: File not found: {csv_path}")
        sys.exit(1)

    print("=" * 64)
    print("  e+e- Annihilation Angular Distribution Analysis")
    print("=" * 64)
    print(f"\n  Input: {csv_path}")

    # Load data
    print("\n--- Loading flux field ---")
    sites = load_flux_field(csv_path)
    print(f"  Loaded {len(sites)} nonzero-density sites")

    if len(sites) == 0:
        print("ERROR: No nonzero-density sites in flux field.")
        sys.exit(1)

    # Determine lattice size
    max_coord = max(max(s["x"], s["y"], s["z"]) for s in sites)
    L = max_coord + 1
    print(f"  Lattice size: {L}^3")

    # Count radiation vs manifested
    n_radiation = sum(1 for s in sites if s["state"] == 0)
    n_manifested = sum(1 for s in sites if s["state"] != 0)
    print(f"  Radiation sites: {n_radiation}")
    print(f"  Manifested sites: {n_manifested}")

    # Compute angular distribution
    print("\n--- Angular distribution (18 cos(theta) bins) ---")
    N_BINS = 18
    cos_centers, intensities, errors, counts = compute_angular_distribution(
        sites, n_bins=N_BINS
    )

    print(f"\n  {'cos(theta)':>12}  {'Intensity':>12}  {'Error':>10}  {'Sites':>6}")
    print("  " + "-" * 48)
    for i in range(N_BINS):
        print(f"  {cos_centers[i]:>12.4f}  {intensities[i]:>12.4e}  "
              f"{errors[i]:>10.4e}  {counts[i]:>6d}")

    # Total flux energy
    total_intensity = sum(intensities)
    print(f"\n  Total binned intensity: {total_intensity:.6e}")

    # Fit to (1 + B cos^2 theta)
    print("\n--- Fit: I(cos theta) = A * (1 + B * cos^2 theta) ---")
    A, B, chi2, ndof = fit_angular_distribution(cos_centers, intensities, errors)

    print(f"  A     = {A:.6e}")
    print(f"  B     = {B:.6f}")
    print(f"  chi^2 = {chi2:.4f}")
    print(f"  ndof  = {ndof}")
    if ndof > 0:
        print(f"  chi^2/ndof = {chi2 / ndof:.4f}")

    # Interpretation
    print("\n--- Interpretation ---")
    print(f"  B = {B:.4f}")
    if abs(B - 1.0) < 0.3:
        print("  => Close to QED prediction (B = 1.0): QED-like angular distribution")
    elif B > 0:
        print("  => Positive B: forward/backward enhancement present")
    else:
        print("  => Negative B: transverse enhancement (unexpected for QED)")

    # Generate plot
    print("\n--- Plot ---")
    plot_dir = os.path.dirname(csv_path) or "output"
    plot_path = os.path.join(plot_dir, "annihilation_angular_distribution.png")
    make_plot(cos_centers, intensities, errors, A, B, plot_path)

    # Summary
    print("\n" + "=" * 64)
    print("  SUMMARY")
    print(f"    B parameter:  {B:.4f}  (QED prediction: 1.0)")
    if ndof > 0:
        print(f"    chi^2/ndof:   {chi2 / ndof:.4f}")
    print(f"    Radiation sites analyzed: {n_radiation}")
    print("=" * 64)

    return 0


if __name__ == "__main__":
    sys.exit(main())
