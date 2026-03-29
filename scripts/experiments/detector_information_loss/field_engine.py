"""
Shared field computation and analysis engine for detector information loss suite.

Provides:
  - Huygens-Fresnel dual-source field computation (matching fermat_dual_source.html)
  - Degradation pipeline: born_rule, sample_detector_clicks
  - Information-theoretic measures: Shannon entropy, mutual information, Fisher info
  - Consistent figure generation with dark FTD palette

All constants imported from scripts/constants.py where applicable.
"""

import sys
import os
import json
import numpy as np
from pathlib import Path

# Import FTD constants
_project_root = Path(__file__).resolve().parents[2]
if str(_project_root) not in sys.path:
    sys.path.insert(0, str(_project_root))

# c = 1/sqrt(3) from FTD lattice CFL stability
C_SPEED = 1.0 / np.sqrt(3.0)

# Default simulation parameters
DEFAULTS = dict(
    W=512, H=512,
    lam=32.0,
    separation=160.0,
    phase_offset=np.pi,
    t=600.0,
    K_B=0.05,
    N_clicks=10_000,
)

OUTPUT_DIR = Path(__file__).parent / "output"
OUTPUT_DIR.mkdir(exist_ok=True)


# ============================================================================
# Field computation
# ============================================================================

def compute_dual_source_field(W=512, H=512, lam=32.0, separation=80.0,
                              phase_offset=np.pi, t=400.0):
    """Compute complex field from two counter-phase point sources.

    Uses Huygens-Fresnel: psi(r) = sum_s A_s / sqrt(r) * exp(i(kr - wt + phi_s))
    with causal wavefront envelope.

    Returns (psi_re, psi_im) each shaped (H, W).
    """
    k = 2.0 * np.pi / lam
    omega = C_SPEED * k
    cx, cy = W / 2.0, H / 2.0

    sources = [
        (cx - separation / 2.0, cy, 0.0),
        (cx + separation / 2.0, cy, phase_offset),
    ]

    yy, xx = np.mgrid[0:H, 0:W].astype(np.float64)
    psi_re = np.zeros((H, W), dtype=np.float64)
    psi_im = np.zeros((H, W), dtype=np.float64)

    front = C_SPEED * t

    for sx, sy, phi_s in sources:
        dx = xx - sx
        dy = yy - sy
        r = np.sqrt(dx * dx + dy * dy)
        r = np.maximum(r, 0.5)

        # Causal envelope: smoothstep at wavefront
        edge = np.clip((front - r) / 8.0, 0.0, 1.0)
        causal = edge * edge * (3.0 - 2.0 * edge)

        # Skip pixels too close to source
        mask = r >= 2.0
        envelope = np.where(mask, causal / np.sqrt(r), 0.0)

        phase = k * r - omega * t + phi_s
        psi_re += envelope * np.cos(phase)
        psi_im += envelope * np.sin(phase)

    return psi_re, psi_im


def compute_single_source_field(W=512, H=512, lam=32.0, source_x=None,
                                source_y=None, phase=0.0, t=400.0):
    """Compute field from a single point source (for decomposition tests)."""
    k = 2.0 * np.pi / lam
    omega = C_SPEED * k
    if source_x is None:
        source_x = W / 2.0
    if source_y is None:
        source_y = H / 2.0

    yy, xx = np.mgrid[0:H, 0:W].astype(np.float64)
    dx = xx - source_x
    dy = yy - source_y
    r = np.sqrt(dx * dx + dy * dy)
    r = np.maximum(r, 0.5)

    front = C_SPEED * t
    edge = np.clip((front - r) / 8.0, 0.0, 1.0)
    causal = edge * edge * (3.0 - 2.0 * edge)

    mask = r >= 2.0
    envelope = np.where(mask, causal / np.sqrt(r), 0.0)

    ph = k * r - omega * t + phase
    return envelope * np.cos(ph), envelope * np.sin(ph)


# ============================================================================
# Degradation pipeline
# ============================================================================

def born_rule(psi_re, psi_im):
    """Apply Born rule: |psi|^2 = Re^2 + Im^2. Phase destroyed."""
    return psi_re ** 2 + psi_im ** 2


def amplitude_field(psi_re, psi_im):
    """Compute |psi| = sqrt(Re^2 + Im^2)."""
    return np.sqrt(psi_re ** 2 + psi_im ** 2)


def phase_field(psi_re, psi_im):
    """Compute phase theta = arctan2(Im, Re)."""
    return np.arctan2(psi_im, psi_re)


def sample_detector_clicks(born, N_clicks, rng=None):
    """Sample N detector clicks from |psi|^2 via rejection sampling.

    Returns array of shape (N_clicks, 2) with (x, y) coordinates.
    """
    if rng is None:
        rng = np.random.default_rng(42)

    H, W = born.shape
    max_val = born.max()
    if max_val <= 0:
        return np.zeros((0, 2))

    prob = born / max_val
    clicks = []
    batch_size = N_clicks * 4

    while len(clicks) < N_clicks:
        xs = rng.integers(0, W, size=batch_size)
        ys = rng.integers(0, H, size=batch_size)
        accept = rng.random(batch_size) < prob[ys, xs]
        accepted = np.column_stack([xs[accept], ys[accept]])
        clicks.append(accepted)

    clicks = np.concatenate(clicks, axis=0)[:N_clicks]
    return clicks


# ============================================================================
# Information measures
# ============================================================================

def shannon_entropy(data, bins=256):
    """Shannon entropy of a distribution in bits."""
    flat = data.ravel()
    flat = flat[np.isfinite(flat)]
    if len(flat) == 0:
        return 0.0
    counts, _ = np.histogram(flat, bins=bins)
    p = counts / counts.sum()
    p = p[p > 0]
    return -np.sum(p * np.log2(p))


def mutual_information(field_a, field_b, bins=64):
    """Mutual information I(A;B) in bits from joint histogram."""
    a = field_a.ravel()
    b = field_b.ravel()
    mask = np.isfinite(a) & np.isfinite(b)
    a, b = a[mask], b[mask]
    if len(a) == 0:
        return 0.0

    joint, _, _ = np.histogram2d(a, b, bins=bins)
    joint = joint / joint.sum()

    pa = joint.sum(axis=1)
    pb = joint.sum(axis=0)

    # I(A;B) = sum p(a,b) log2(p(a,b) / (p(a)*p(b)))
    mi = 0.0
    for i in range(bins):
        for j in range(bins):
            if joint[i, j] > 0 and pa[i] > 0 and pb[j] > 0:
                mi += joint[i, j] * np.log2(joint[i, j] / (pa[i] * pb[j]))
    return mi


def fisher_information(field, field_deriv):
    """Classical Fisher information F = integral (df/dp)^2 / f dx.

    field: the probability density (|psi|^2 or similar)
    field_deriv: derivative of field w.r.t. parameter
    """
    f = field.ravel()
    df = field_deriv.ravel()
    mask = f > 1e-12
    return np.sum(df[mask] ** 2 / f[mask])


def kl_divergence(p, q, bins=256):
    """KL divergence D_KL(P||Q) in bits."""
    p_hist, edges = np.histogram(p.ravel(), bins=bins, density=True)
    q_hist, _ = np.histogram(q.ravel(), bins=edges, density=True)

    p_hist = p_hist + 1e-12
    q_hist = q_hist + 1e-12
    p_norm = p_hist / p_hist.sum()
    q_norm = q_hist / q_hist.sum()

    return np.sum(p_norm * np.log2(p_norm / q_norm))


# ============================================================================
# Visualization
# ============================================================================

# FTD dark palette
BG_COLOR = '#0a0a0f'
TEXT_COLOR = '#c8d8e8'
SUBTLE_COLOR = '#667788'
ACCENT_COLOR = '#a0b8d8'
GRID_COLOR = '#1a1a2e'


def phase_to_rgb(phase, amplitude=None):
    """Convert phase field to RGB using hue colormap, amplitude as luminance."""
    h = (phase / (2 * np.pi)) % 1.0
    s = np.ones_like(h)
    if amplitude is not None:
        amax = np.percentile(amplitude[amplitude > 0], 99) if (amplitude > 0).any() else 1.0
        v = np.clip(amplitude / max(amax, 1e-12), 0, 1)
    else:
        v = np.ones_like(h)

    # HSV to RGB
    i = (h * 6.0).astype(int) % 6
    f = h * 6.0 - np.floor(h * 6.0)
    p = v * (1.0 - s)
    q = v * (1.0 - f * s)
    t_val = v * (1.0 - (1.0 - f) * s)

    rgb = np.zeros((*h.shape, 3))
    for idx, (r, g, b) in enumerate([(v, t_val, p), (q, v, p), (p, v, t_val),
                                      (p, q, v), (t_val, p, v), (v, p, q)]):
        mask = i == idx
        rgb[mask, 0] = r[mask]
        rgb[mask, 1] = g[mask]
        rgb[mask, 2] = b[mask]
    return rgb


def setup_style():
    """Apply FTD dark style to matplotlib."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    plt.rcParams.update({
        'figure.facecolor': BG_COLOR,
        'axes.facecolor': BG_COLOR,
        'text.color': TEXT_COLOR,
        'axes.labelcolor': TEXT_COLOR,
        'xtick.color': SUBTLE_COLOR,
        'ytick.color': SUBTLE_COLOR,
        'axes.edgecolor': GRID_COLOR,
        'grid.color': GRID_COLOR,
        'font.family': 'sans-serif',
        'font.size': 9,
    })
    return plt


def make_figure(title, panels, metrics_text, filename, figsize=None):
    """Create a multi-panel figure with consistent FTD styling.

    panels: list of dicts with keys:
        'data': 2D array or RGB array
        'title': panel title
        'cmap': colormap name (ignored if data is RGB)
        'colorbar': bool
        Optional: 'overlay_fn': callable(ax) for extra drawing
    metrics_text: string shown below the figure
    filename: saved to OUTPUT_DIR/filename
    """
    plt = setup_style()

    n = len(panels)
    if figsize is None:
        figsize = (4.5 * n, 5.5)
    fig, axes = plt.subplots(1, n, figsize=figsize)
    if n == 1:
        axes = [axes]

    fig.suptitle(title, color=ACCENT_COLOR, fontsize=12, fontweight='bold', y=0.97)

    for ax, panel in zip(axes, panels):
        data = panel['data']
        cmap = panel.get('cmap', 'inferno')
        vmin = panel.get('vmin', None)
        vmax = panel.get('vmax', None)

        if data.ndim == 3:
            ax.imshow(data, origin='lower', aspect='equal')
        else:
            im = ax.imshow(data, origin='lower', cmap=cmap, aspect='equal',
                          vmin=vmin, vmax=vmax)
            if panel.get('colorbar', False):
                cb = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
                cb.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

        ax.set_title(panel.get('title', ''), color=TEXT_COLOR, fontsize=9)
        ax.set_xticks([])
        ax.set_yticks([])

        if 'overlay_fn' in panel:
            panel['overlay_fn'](ax)

    if metrics_text:
        fig.text(0.5, 0.02, metrics_text, ha='center', va='bottom',
                 color=SUBTLE_COLOR, fontsize=8, family='monospace',
                 bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                          edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.06, 1, 0.97])
    outpath = OUTPUT_DIR / filename
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")
    return outpath


def save_json(name, data):
    """Save metrics dict as JSON to output directory."""
    outpath = OUTPUT_DIR / f"{name}.json"
    with open(outpath, 'w') as f:
        json.dump(data, f, indent=2, default=lambda x: float(x) if hasattr(x, 'item') else str(x))
    print(f"  Saved: {outpath}")
    return outpath


def detector_dots_image(clicks, W, H):
    """Create an image from detector click positions."""
    img = np.zeros((H, W), dtype=np.float64)
    if len(clicks) > 0:
        np.add.at(img, (clicks[:, 1].astype(int) % H, clicks[:, 0].astype(int) % W), 1.0)
    return img
