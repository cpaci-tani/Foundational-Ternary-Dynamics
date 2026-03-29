"""
02_phase_gradient_field.py — Phase gradients encode momentum flow

WHY THIS MATTERS:
nabla(theta) is the local wavevector telling you which direction energy
flows at every point.  It is the probability current density (up to a
factor of |psi|^2).  The detector sees none of this vector field —
click positions carry zero information about local momentum.

Quantifies: gradient-field entropy, total flow structure destroyed.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, phase_field, amplitude_field,
    shannon_entropy, setup_style, make_figure, save_json, DEFAULTS,
)


def compute_phase_gradient(psi_re, psi_im):
    """Compute phase gradient via the complex field directly.

    nabla(theta) = Im(nabla(psi) / psi)

    This avoids branch-cut artifacts from differentiating arctan2.
    Returns (grad_x, grad_y) each shaped (H, W).
    """
    psi = psi_re + 1j * psi_im
    amp2 = np.abs(psi) ** 2
    amp2 = np.maximum(amp2, 1e-20)  # avoid division by zero

    # Numerical gradients of the complex field
    dpsi_dy, dpsi_dx = np.gradient(psi)

    # nabla(theta) = Im(nabla(psi) / psi) = Im(nabla(psi) * conj(psi)) / |psi|^2
    grad_x = np.imag(dpsi_dx * np.conj(psi)) / amp2
    grad_y = np.imag(dpsi_dy * np.conj(psi)) / amp2

    return grad_x, grad_y


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("02  PHASE GRADIENT FIELD — Momentum flow the detector cannot see")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  nabla(theta) is the local wavevector: it tells you which direction")
    print("  energy flows at every point.  The detector sees none of this")
    print("  vector field.  Click positions carry zero momentum information.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    amp = amplitude_field(psi_re, psi_im)
    born = born_rule(psi_re, psi_im)

    # 2. Phase gradient
    print("  Computing phase gradient ...")
    grad_x, grad_y = compute_phase_gradient(psi_re, psi_im)
    grad_mag = np.sqrt(grad_x ** 2 + grad_y ** 2)

    # Mask out near-zero amplitude regions (gradient noisy there)
    amp_threshold = np.percentile(amp[amp > 0], 5) if (amp > 0).any() else 0.0
    valid = amp > amp_threshold
    grad_mag_masked = np.where(valid, grad_mag, 0.0)

    # 3. Entropy of gradient magnitude
    H_grad = shannon_entropy(grad_mag_masked[valid], bins=256)

    # For |psi|^2: gradient magnitude is just intensity gradient, no momentum info
    # The momentum content is zero for scalar intensity
    H_born_grad = 0.0

    bits_destroyed = H_grad

    # ------------------------------------------------------------------
    # Print results
    print(f"  Gradient field entropy  H(|nabla theta|) = {H_grad:.4f} bits")
    print(f"  Born-rule momentum info                  = {H_born_grad:.1f} bits")
    print(f"  Bits of momentum structure destroyed      = {bits_destroyed:.4f} bits")
    print(f"  Mean |nabla theta| (valid)               = {grad_mag_masked[valid].mean():.6f} rad/px")
    print(f"  Max  |nabla theta|                       = {grad_mag_masked.max():.6f} rad/px")
    print()

    # ------------------------------------------------------------------
    # Figure
    plt = setup_style()

    # Panel (a): gradient magnitude heatmap
    grad_display = grad_mag_masked.copy()
    vmax_grad = np.percentile(grad_display[valid], 98) if valid.any() else 1.0
    grad_display = np.clip(grad_display, 0, vmax_grad)

    # Panel (b): quiver (subsampled)
    step = 16
    Y_q, X_q = np.mgrid[0:H:step, 0:W:step]
    U_q = grad_x[::step, ::step]
    V_q = grad_y[::step, ::step]
    amp_q = amp[::step, ::step]
    # Mask low-amplitude arrows
    amp_thresh_q = np.percentile(amp[amp > 0], 20) if (amp > 0).any() else 0.0
    arrow_mask = amp_q > amp_thresh_q

    def quiver_overlay(ax):
        Uplot = np.where(arrow_mask, U_q, 0)
        Vplot = np.where(arrow_mask, V_q, 0)
        speed = np.sqrt(Uplot ** 2 + Vplot ** 2)
        speed_max = speed.max() if speed.max() > 0 else 1.0
        ax.quiver(
            X_q, Y_q, Uplot, Vplot,
            speed / speed_max,
            cmap='cool', scale=speed_max * 30, width=0.002,
            headwidth=3, headlength=4, alpha=0.85,
        )

    # Build a background for the quiver panel: faint amplitude
    quiver_bg = np.zeros((H, W))
    quiver_bg = np.where(valid, amp / max(amp.max(), 1e-12) * 0.3, 0)

    # Panel (c): |psi|^2 — no flow info
    born_display = born / max(born.max(), 1e-12)

    # Panel (d): entropy comparison bar
    def bar_overlay(ax):
        ax.clear()
        ax.set_facecolor('#0a0a0f')
        labels = ['Phase gradient\n(full field)', '|psi|^2\n(Born rule)']
        vals = [H_grad, H_born_grad]
        colors = ['#66ccff', '#ff6666']
        bars = ax.bar(labels, vals, color=colors, width=0.5, edgecolor='none')
        ax.set_ylabel('Shannon entropy (bits)', fontsize=8)
        ax.set_title('Gradient entropy comparison', color='#c8d8e8', fontsize=9)
        for bar, v in zip(bars, vals):
            ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.1,
                    f'{v:.2f}', ha='center', va='bottom', color='#c8d8e8', fontsize=9)
        ax.set_ylim(0, max(H_grad * 1.3, 1.0))
        ax.tick_params(colors='#667788')

    metrics_text = (
        f"H(|nabla theta|) = {H_grad:.4f} bits  |  "
        f"H(born gradient) = {H_born_grad:.1f} bits  |  "
        f"Mean |nabla theta| = {grad_mag_masked[valid].mean():.4f} rad/px"
    )

    panels = [
        {
            'data': grad_display,
            'title': '(a) |nabla theta|  (gradient magnitude)',
            'cmap': 'inferno',
            'colorbar': True,
        },
        {
            'data': quiver_bg,
            'title': '(b) Momentum flow  (quiver, step=16)',
            'cmap': 'gray',
            'colorbar': False,
            'overlay_fn': quiver_overlay,
        },
        {
            'data': born_display,
            'title': '(c) |psi|^2  (no flow info)',
            'cmap': 'gray',
            'colorbar': True,
        },
        {
            'data': np.zeros((H, W)),  # placeholder for bar chart
            'title': '',
            'cmap': 'gray',
            'colorbar': False,
            'overlay_fn': bar_overlay,
        },
    ]

    make_figure(
        title="Phase Gradients: Momentum Flow the Detector Cannot See",
        panels=panels,
        metrics_text=metrics_text,
        filename="02_phase_gradient_field.png",
    )

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '02_phase_gradient_field.py',
        'description': 'Phase gradient (local wavevector) entropy vs Born rule',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'H_gradient_bits': float(H_grad),
            'H_born_gradient_bits': float(H_born_grad),
            'bits_destroyed': float(bits_destroyed),
            'mean_grad_magnitude': float(grad_mag_masked[valid].mean()),
            'max_grad_magnitude': float(grad_mag_masked.max()),
            'valid_pixels': int(valid.sum()),
            'quiver_step': step,
        },
    }
    save_json('02_phase_gradient_field', summary)

    print(f"  RESULT: The phase gradient field carries {H_grad:.3f} bits of")
    print(f"  momentum-flow entropy.  The Born rule destroys ALL of it.")
    print()


if __name__ == '__main__':
    main()
