"""
03_phase_singularities.py — Topological defects the detector cannot see

WHY THIS MATTERS:
Phase vortices (points where psi=0 with winding number +/-1) carry
quantized angular momentum.  They are topological invariants of the
wavefield — you cannot remove them by smooth deformation.  A detector
recording |psi|^2 sees them as "dark spots" indistinguishable from
any other low-intensity region.  The winding number — the sign of the
circulation — is invisible.

Quantifies: vortex census (N+, N-), topological charge, spatial map.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, phase_field, amplitude_field,
    setup_style, make_figure, save_json, phase_to_rgb, DEFAULTS,
)


def _wrap(angle):
    """Wrap angle difference to (-pi, pi]."""
    return (angle + np.pi) % (2 * np.pi) - np.pi


def detect_vortices(theta):
    """Detect phase vortices via 2x2 plaquette winding numbers.

    For each plaquette (i,j)-(i+1,j)-(i+1,j+1)-(i,j+1), sum the
    wrapped phase differences around the loop.  If |sum| > pi, it's
    a vortex with sign = sign(sum).

    Returns:
        vortex_plus:  (N, 2) array of (x, y) for +1 winding
        vortex_minus: (N, 2) array of (x, y) for -1 winding
        winding_map:  (H-1, W-1) array of winding numbers
    """
    # Phase differences along edges of each 2x2 plaquette
    # Corners: A=(i,j), B=(i,j+1), C=(i+1,j+1), D=(i+1,j)
    # Loop: A->B->C->D->A
    dAB = _wrap(theta[:-1, 1:] - theta[:-1, :-1])   # A -> B (right)
    dBC = _wrap(theta[1:, 1:] - theta[:-1, 1:])      # B -> C (down)
    dCD = _wrap(theta[1:, :-1] - theta[1:, 1:])      # C -> D (left)
    dDA = _wrap(theta[:-1, :-1] - theta[1:, :-1])    # D -> A (up)

    winding = dAB + dBC + dCD + dDA  # Should be ~ 0, +2pi, or -2pi

    # Vortex if |winding| > pi
    winding_number = np.round(winding / (2 * np.pi)).astype(int)

    plus_yx = np.argwhere(winding_number > 0)
    minus_yx = np.argwhere(winding_number < 0)

    # Convert to (x, y) placing vortex at center of plaquette
    vortex_plus = plus_yx[:, ::-1].astype(float) + 0.5   # (x, y)
    vortex_minus = minus_yx[:, ::-1].astype(float) + 0.5

    return vortex_plus, vortex_minus, winding_number


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("03  PHASE SINGULARITIES — Topological defects the detector misses")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Phase vortices (psi=0, winding +/-1) carry quantized angular")
    print("  momentum.  A detector sees them as 'dark spots' with no way")
    print("  to distinguish +1 from -1 winding.  The topological charge")
    print("  is invisible to |psi|^2.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    theta = phase_field(psi_re, psi_im)
    amp = amplitude_field(psi_re, psi_im)
    born = born_rule(psi_re, psi_im)

    # 2. Detect vortices
    print("  Detecting phase vortices ...")
    vortex_plus, vortex_minus, winding_map = detect_vortices(theta)

    N_plus = len(vortex_plus)
    N_minus = len(vortex_minus)
    N_total = N_plus + N_minus
    net_charge = N_plus - N_minus

    print(f"  Vortices found:  N_+ = {N_plus},  N_- = {N_minus},  total = {N_total}")
    print(f"  Net topological charge:  {net_charge}")
    print(f"  In |psi|^2: these are just {N_total} dark spots, winding invisible.")
    print()

    # ------------------------------------------------------------------
    # Figure
    plt = setup_style()

    # Panel (a): phase field + vortex markers
    phase_rgb = phase_to_rgb(theta, amplitude=amp)

    def overlay_vortices_phase(ax):
        if len(vortex_plus) > 0:
            ax.scatter(vortex_plus[:, 0], vortex_plus[:, 1],
                       c='red', s=12, marker='o', linewidths=0.5,
                       edgecolors='white', zorder=5, label=f'+1 ({N_plus})')
        if len(vortex_minus) > 0:
            ax.scatter(vortex_minus[:, 0], vortex_minus[:, 1],
                       c='#4488ff', s=12, marker='o', linewidths=0.5,
                       edgecolors='white', zorder=5, label=f'-1 ({N_minus})')
        ax.legend(loc='upper right', fontsize=7, framealpha=0.7,
                  facecolor='#0d0d15', edgecolor='#667788',
                  labelcolor='#c8d8e8')

    # Panel (b): amplitude with vortex markers
    amp_display = amp / max(amp.max(), 1e-12)

    def overlay_vortices_amp(ax):
        if len(vortex_plus) > 0:
            ax.scatter(vortex_plus[:, 0], vortex_plus[:, 1],
                       c='red', s=12, marker='o', linewidths=0.5,
                       edgecolors='white', zorder=5)
        if len(vortex_minus) > 0:
            ax.scatter(vortex_minus[:, 0], vortex_minus[:, 1],
                       c='#4488ff', s=12, marker='o', linewidths=0.5,
                       edgecolors='white', zorder=5)

    # Panel (c): |psi|^2 — dark spots unmarked
    born_display = born / max(born.max(), 1e-12)

    # Panel (d): vortex census
    def census_overlay(ax):
        ax.clear()
        ax.set_facecolor('#0a0a0f')
        labels = ['+1 vortices', '-1 vortices', 'Net charge']
        vals = [N_plus, N_minus, net_charge]
        colors = ['#ff4444', '#4488ff', '#ffcc66']
        bars = ax.bar(labels, vals, color=colors, width=0.5, edgecolor='none')
        ax.set_ylabel('Count', fontsize=8)
        ax.set_title('Vortex census', color='#c8d8e8', fontsize=9)
        for bar, v in zip(bars, vals):
            y_pos = bar.get_height() if bar.get_height() >= 0 else bar.get_height() - 0.5
            va = 'bottom' if v >= 0 else 'top'
            ax.text(bar.get_x() + bar.get_width() / 2, y_pos,
                    str(v), ha='center', va=va, color='#c8d8e8', fontsize=10,
                    fontweight='bold')
        ax.axhline(0, color='#667788', linewidth=0.5)
        y_max = max(N_plus, N_minus, abs(net_charge), 1) * 1.3
        ax.set_ylim(-y_max * 0.3, y_max)
        ax.tick_params(colors='#667788')

    metrics_text = (
        f"N_+ = {N_plus}  |  N_- = {N_minus}  |  "
        f"Total = {N_total}  |  Net charge = {net_charge}  |  "
        f"|psi|^2 sees: {N_total} dark spots (no winding info)"
    )

    panels = [
        {
            'data': phase_rgb,
            'title': '(a) Phase + vortex markers (red=+1, blue=-1)',
            'cmap': None,
            'colorbar': False,
            'overlay_fn': overlay_vortices_phase,
        },
        {
            'data': amp_display,
            'title': '(b) Amplitude + vortices',
            'cmap': 'gray',
            'colorbar': False,
            'overlay_fn': overlay_vortices_amp,
        },
        {
            'data': born_display,
            'title': '(c) |psi|^2 — dark spots only (no topology)',
            'cmap': 'gray',
            'colorbar': True,
        },
        {
            'data': np.zeros((H, W)),  # placeholder for bar chart
            'title': '',
            'cmap': 'gray',
            'colorbar': False,
            'overlay_fn': census_overlay,
        },
    ]

    make_figure(
        title="Phase Singularities: Topological Defects the Detector Cannot See",
        panels=panels,
        metrics_text=metrics_text,
        filename="03_phase_singularities.png",
    )

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '03_phase_singularities.py',
        'description': 'Phase vortex detection and topological charge census',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'N_plus': int(N_plus),
            'N_minus': int(N_minus),
            'N_total': int(N_total),
            'net_topological_charge': int(net_charge),
            'vortex_plus_positions': vortex_plus.tolist() if N_plus > 0 else [],
            'vortex_minus_positions': vortex_minus.tolist() if N_minus > 0 else [],
            'born_rule_sees': f'{N_total} dark spots, no winding information',
        },
    }
    save_json('03_phase_singularities', summary)

    print(f"  RESULT: {N_total} phase vortices found ({N_plus} positive, {N_minus} negative).")
    print(f"  Net topological charge = {net_charge}.")
    print(f"  |psi|^2 reduces all of these to indistinguishable dark spots.")
    print()


if __name__ == '__main__':
    main()
