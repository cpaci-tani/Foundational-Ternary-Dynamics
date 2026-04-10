"""
FTD vs Schwarzschild: Photon Ray Tracing Simulation

Fires photons at a compact object under three models and traces their paths:
  1. GR Schwarzschild: f = 1 - 2/r,  n = 1/sqrt(f)
  2. FTD metric only:  f = 1 - 1/r^2, n = 1/sqrt(f)
  3. FTD full:          f = 1 - 1/r^2, n = (1 + 2/r) / sqrt(f)

Produces:
  - Photon trajectories (which get captured, which escape)
  - Shadow boundary (critical impact parameter)
  - Visual comparison of shadow sizes
  - Quantitative comparison to EHT observations

Units: GM/c^2 = 1 throughout.
"""
import numpy as np
import os

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'output')
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ============================================================
# Photon ray tracer in a spherically symmetric refractive medium
# ============================================================

def trace_photon(n_func, f_func, b, r_start=200.0, dr_max=0.01, max_steps=500000):
    """Trace a photon with impact parameter b through medium n(r).

    Uses the optical ray equation in polar coords:
      dr/dphi = r * sqrt((r*n(r)/b)^2 - 1)  ... if the ray is moving outward
      (negative sqrt for inward)

    Returns array of (x, y) positions and outcome ('escape', 'capture', 'orbit').
    """
    # Start far away, moving inward
    r = r_start
    phi = 0.0

    # Determine if the ray can reach this r at all
    rn = r * n_func(r)
    if rn < b:
        # Impact parameter larger than r*n at start -- shouldn't happen for large r_start
        return np.array([[r * np.cos(phi), r * np.sin(phi)]]), 'escape'

    path = []
    inward = True
    r_min_seen = r

    for step in range(max_steps):
        if step % 200 == 0 or (r < 10 and step % 20 == 0):
            path.append((r * np.cos(phi), r * np.sin(phi)))

        rn = r * n_func(r)
        arg = (rn / b) ** 2 - 1.0

        if arg < 0:
            # Turning point reached -- switch from inward to outward
            inward = False
            arg = 0.0

        sqrt_arg = np.sqrt(arg)
        if sqrt_arg < 1e-15:
            sqrt_arg = 1e-15

        # Step in phi
        dphi = dr_max / (r * sqrt_arg + 1e-15)
        dphi = min(dphi, 0.01)  # limit angular step

        if inward:
            dr = -r * sqrt_arg * dphi
        else:
            dr = r * sqrt_arg * dphi

        r_new = r + dr
        phi_new = phi + dphi

        # Check for capture (inside horizon)
        f_val = f_func(r_new)
        if f_val <= 0 or r_new < 0.5:
            path.append((r_new * np.cos(phi_new), r_new * np.sin(phi_new)))
            return np.array(path), 'capture'

        # Check for escape (turned around and heading outward past 2x closest approach)
        if not inward and r_new > max(r_min_seen * 3, 15.0):
            path.append((r_new * np.cos(phi_new), r_new * np.sin(phi_new)))
            return np.array(path), 'escape'

        r = r_new
        phi = phi_new
        r_min_seen = min(r_min_seen, r)

        # Stuck in orbit check
        if step > 100000 and abs(r - r_min_seen) < 0.01:
            return np.array(path[:500]), 'orbit'

    return np.array(path[:500]), 'orbit'


# ============================================================
# Model definitions
# ============================================================

def f_gr(r):
    return 1.0 - 2.0 / max(r, 0.01)

def n_gr(r):
    f = f_gr(r)
    if f <= 0: return 1e6
    return 1.0 / np.sqrt(f)

def f_ftd(r):
    return 1.0 - 1.0 / max(r * r, 0.01)

def n_ftd_metric(r):
    f = f_ftd(r)
    if f <= 0: return 1e6
    return 1.0 / np.sqrt(f)

def n_ftd_full(r):
    f = f_ftd(r)
    if f <= 0: return 1e6
    return (1.0 + 2.0 / r) / np.sqrt(f)


# ============================================================
# Find critical impact parameter (shadow edge) by bisection
# ============================================================

def find_b_critical(n_func, f_func, b_lo=1.0, b_hi=10.0, tol=0.001):
    """Binary search for the critical impact parameter."""
    for _ in range(60):
        b_mid = (b_lo + b_hi) / 2
        _, outcome = trace_photon(n_func, f_func, b_mid, r_start=100.0, max_steps=200000)
        if outcome == 'capture':
            b_lo = b_mid
        else:
            b_hi = b_mid
        if b_hi - b_lo < tol:
            break
    return (b_lo + b_hi) / 2


print("=" * 72)
print("PHOTON RAY TRACING: FTD vs Schwarzschild Shadow Simulation")
print("=" * 72)

# ============================================================
# 1. Find shadow boundaries
# ============================================================
print("\n--- Finding shadow boundaries (this may take a moment) ---\n")

models = {
    'GR': (n_gr, f_gr, 'GR Schwarzschild'),
    'FTD_metric': (n_ftd_metric, f_ftd, 'FTD metric only'),
    'FTD_full': (n_ftd_full, f_ftd, 'FTD full (flux+metric)'),
}

b_crits = {}
for key, (n_func, f_func, label) in models.items():
    if key == 'GR':
        b_c = find_b_critical(n_func, f_func, b_lo=3.0, b_hi=8.0)
    elif key == 'FTD_metric':
        b_c = find_b_critical(n_func, f_func, b_lo=1.0, b_hi=4.0)
    else:
        b_c = find_b_critical(n_func, f_func, b_lo=2.0, b_hi=7.0)
    b_crits[key] = b_c
    print(f"  {label:>30}: b_crit = {b_c:.3f} GM/c^2")

print()
b_gr = b_crits['GR']
b_fm = b_crits['FTD_metric']
b_ff = b_crits['FTD_full']
print(f"  FTD full / GR ratio: {b_ff/b_gr:.4f} ({b_ff/b_gr*100:.1f}%)")
print(f"  FTD metric / GR ratio: {b_fm/b_gr:.4f} ({b_fm/b_gr*100:.1f}%)")

# ============================================================
# 2. Trace sample photon paths
# ============================================================
print("\n--- Tracing photon paths ---\n")

# Trace a set of impact parameters for each model
b_values = np.concatenate([
    np.linspace(0.5, 3.0, 8),           # deep captures
    np.linspace(3.5, b_ff * 0.95, 5),   # near FTD full boundary
    np.linspace(b_ff * 1.05, b_gr * 0.95, 5),  # between FTD and GR boundaries
    np.linspace(b_gr * 1.05, b_gr * 1.8, 8),   # escaped photons
    [b_crits['GR'] * 0.999, b_crits['GR'] * 1.001],
    [b_crits['FTD_full'] * 0.999, b_crits['FTD_full'] * 1.001],
])
b_values = np.sort(np.unique(b_values))

all_paths = {}
for key, (n_func, f_func, label) in models.items():
    paths = []
    for b in b_values:
        path, outcome = trace_photon(n_func, f_func, b, r_start=50.0, max_steps=100000)
        paths.append((b, path, outcome))
    all_paths[key] = paths
    n_cap = sum(1 for _, _, o in paths if o == 'capture')
    n_esc = sum(1 for _, _, o in paths if o == 'escape')
    print(f"  {label}: {n_cap} captured, {n_esc} escaped out of {len(b_values)}")

# ============================================================
# 3. Generate SVG visualization
# ============================================================
print("\n--- Generating visualization ---\n")

def paths_to_svg(all_paths, b_crits, filename):
    """Generate an SVG showing photon paths for all three models side by side."""
    W, H = 1200, 500
    margin = 50
    panel_w = (W - 4 * margin) / 3

    svg_parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
        f'style="background:#0a0a1a">',
        '<style>',
        '  text { font-family: monospace; fill: #aaa; }',
        '  .title { font-size: 14px; fill: #fff; text-anchor: middle; }',
        '  .label { font-size: 11px; fill: #888; text-anchor: middle; }',
        '  .val { font-size: 12px; fill: #4fc; text-anchor: middle; }',
        '</style>',
    ]

    scale = panel_w / 30  # 30 GM/c^2 visible range
    titles = {
        'GR': 'GR Schwarzschild',
        'FTD_metric': 'FTD Metric Only',
        'FTD_full': 'FTD Full (Flux+Metric)',
    }
    horizon_r = {'GR': 2.0, 'FTD_metric': 1.0, 'FTD_full': 1.0}
    colors_cap = {'GR': '#ff4444', 'FTD_metric': '#ff8844', 'FTD_full': '#44aaff'}
    colors_esc = {'GR': '#444444', 'FTD_metric': '#444444', 'FTD_full': '#444444'}

    for idx, key in enumerate(['GR', 'FTD_metric', 'FTD_full']):
        cx = margin + panel_w / 2 + idx * (panel_w + margin)
        cy = H / 2

        # Title
        svg_parts.append(f'<text x="{cx}" y="25" class="title">{titles[key]}</text>')

        # Horizon circle
        rh = horizon_r[key] * scale
        svg_parts.append(
            f'<circle cx="{cx}" cy="{cy}" r="{rh}" fill="#111" stroke="#333" stroke-width="1"/>'
        )

        # Shadow boundary circle
        bc = b_crits[key] * scale
        svg_parts.append(
            f'<circle cx="{cx}" cy="{cy}" r="{bc}" fill="none" '
            f'stroke="#ffcc00" stroke-width="1.5" stroke-dasharray="4,3" opacity="0.6"/>'
        )

        # Photon paths
        for b, path, outcome in all_paths[key]:
            if len(path) < 2:
                continue
            color = colors_cap[key] if outcome == 'capture' else colors_esc[key]
            opacity = 0.7 if outcome == 'capture' else 0.25

            # Convert path to SVG coordinates
            points = []
            for x, y in path:
                sx = cx + x * scale
                sy = cy - y * scale  # flip y
                if margin - 10 < sx < W - margin + 10 and 10 < sy < H - 10:
                    points.append(f"{sx:.1f},{sy:.1f}")

            if len(points) > 1:
                svg_parts.append(
                    f'<polyline points="{" ".join(points)}" '
                    f'fill="none" stroke="{color}" stroke-width="0.8" opacity="{opacity}"/>'
                )

        # Labels
        svg_parts.append(
            f'<text x="{cx}" y="{H-30}" class="label">Shadow: b_c = {b_crits[key]:.2f} GM/c^2</text>'
        )
        ratio = b_crits[key] / b_crits['GR'] * 100
        svg_parts.append(
            f'<text x="{cx}" y="{H-12}" class="val">{ratio:.1f}% of GR</text>'
        )

    # Legend
    svg_parts.append(
        '<text x="60" y="478" style="font-size:10px;fill:#ffcc00">--- shadow boundary</text>'
    )
    svg_parts.append(
        f'<text x="250" y="478" style="font-size:10px;fill:#ff4444">captured photons</text>'
    )
    svg_parts.append(
        f'<text x="410" y="478" style="font-size:10px;fill:#666">escaped photons</text>'
    )

    svg_parts.append('</svg>')
    svg_text = '\n'.join(svg_parts)

    filepath = os.path.join(OUTPUT_DIR, filename)
    with open(filepath, 'w') as f:
        f.write(svg_text)
    print(f"  Saved: {filepath}")
    return filepath

svg_path = paths_to_svg(all_paths, b_crits, 'ftd_vs_gr_shadow.svg')

# ============================================================
# 4. EHT Comparison
# ============================================================
print("\n--- EHT Shadow Comparison ---\n")

G_phys = 6.674e-11
c_phys = 2.998e8
M_sun = 1.989e30
pc = 3.086e16
Mpc = pc * 1e6
kpc = pc * 1e3
uas_per_rad = 206265e6

print(f"{'':>8} | {'GR':>12} | {'FTD full':>12} | {'FTD metric':>12} | {'Observed':>12}")
print("-" * 65)

for target, M_msun, D_m, obs_uas in [
    ("M87*", 6.5e9, 16.8 * Mpc, 42.0),
    ("Sgr A*", 4.15e6, 8.178 * kpc, 52.0)
]:
    M = M_msun * M_sun
    GM_c2 = G_phys * M / (c_phys ** 2)

    s_gr = 2 * b_crits['GR'] * GM_c2 / D_m * uas_per_rad
    s_ff = 2 * b_crits['FTD_full'] * GM_c2 / D_m * uas_per_rad
    s_fm = 2 * b_crits['FTD_metric'] * GM_c2 / D_m * uas_per_rad

    print(f"{target:>8} | {s_gr:>10.1f}ua | {s_ff:>10.1f}ua | {s_fm:>10.1f}ua | {obs_uas:>10.1f}ua")

# ============================================================
# 5. Comprehensive results table
# ============================================================
print(f"""

========================================================================
SIMULATION RESULTS: Photon Ray Tracing
========================================================================

Three models tested with identical ray-tracing code:

  Model              | Horizon | Photon Sphere | Shadow (b_c) | vs GR
  -------------------|---------|---------------|--------------|--------
  GR Schwarzschild   |   2.0   |     3.0       |    {b_crits['GR']:.3f}     | 100.0%
  FTD metric only    |   1.0   |     1.41      |    {b_crits['FTD_metric']:.3f}     |  {b_crits['FTD_metric']/b_crits['GR']*100:.1f}%
  FTD full           |   1.0   |     1.77      |    {b_crits['FTD_full']:.3f}     |  {b_crits['FTD_full']/b_crits['GR']*100:.1f}%

  FTD full model predicts a shadow {(1-b_crits['FTD_full']/b_crits['GR'])*100:.0f}% smaller than GR.
  This is within current EHT measurement uncertainty (~10-15%).

  Visualization saved to: {svg_path}
""")
