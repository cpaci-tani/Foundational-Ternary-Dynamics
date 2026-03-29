#!/usr/bin/env python3
"""01_phase_entropy — Shannon entropy of the phase field

WHY THIS MATTERS:
The full psi has phase theta(x,y) at every pixel -- a rich structured
distribution encoding the entire interference geometry.  |psi|^2 has
zero phase information.  The gap between H(theta) and H(nothing) is
the irreversible cost of the absolute value bars.

EPISTEMIC STATUS: [EXPLORATION]
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    phase_field, amplitude_field, shannon_entropy,
    setup_style, make_figure, save_json, detector_dots_image,
    phase_to_rgb, DEFAULTS, OUTPUT_DIR,
    BG_COLOR, TEXT_COLOR, SUBTLE_COLOR, ACCENT_COLOR, GRID_COLOR,
)


def main():
    print("=" * 64)
    print("01  PHASE ENTROPY -- Shannon entropy of the phase field")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  The full psi has phase theta(x,y) at every pixel -- a rich")
    print("  structured distribution encoding the entire interference")
    print("  geometry.  |psi|^2 has zero phase information.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    # Phase field
    theta = phase_field(psi_re, psi_im)
    amp = amplitude_field(psi_re, psi_im)

    # Mask low-amplitude regions where phase is noise
    amp_threshold = np.percentile(amp[amp > 0], 5) if (amp > 0).any() else 0
    valid = amp > amp_threshold
    theta_valid = theta[valid]

    # Shannon entropy of the phase distribution
    H_phase = shannon_entropy(theta_valid, bins=256)

    # Born rule: phase destroyed
    H_born = 0.0

    # Detector clicks: phase unrecoverable
    H_det = 0.0

    bits_destroyed = H_phase - H_born

    print(f"  Phase entropy  H(theta)       = {H_phase:.4f} bits")
    print(f"  Born-rule      H(theta|born)  = {H_born:.4f} bits  (phase undefined)")
    print(f"  Detector       H(theta|clicks)= {H_det:.4f} bits  (phase unrecoverable)")
    print(f"  Bits destroyed per pixel      = {bits_destroyed:.4f} bits")
    print()

    # Born rule and detector outputs
    born = born_rule(psi_re, psi_im)
    clicks = sample_detector_clicks(born, d['N_clicks'])
    dots_img = detector_dots_image(clicks, W, H)

    # Phase histogram figure
    plt = setup_style()
    fig_hist, ax_hist = plt.subplots(figsize=(6, 3))
    ax_hist.hist(theta_valid, bins=256, color='#4488ff', alpha=0.8,
                 edgecolor='none', density=True)
    ax_hist.set_xlabel('Phase theta (radians)', color=TEXT_COLOR)
    ax_hist.set_ylabel('Density', color=TEXT_COLOR)
    ax_hist.set_title(f'Phase distribution  H = {H_phase:.3f} bits',
                      color=ACCENT_COLOR, fontsize=10)
    ax_hist.axhline(y=1/(2*np.pi), color='#ff4444', ls='--', alpha=0.5,
                    label=f'Uniform: {np.log2(256):.1f} bits max')
    ax_hist.legend(fontsize=7)
    hist_path = OUTPUT_DIR / '01_phase_histogram.png'
    fig_hist.savefig(hist_path, dpi=200, facecolor=BG_COLOR,
                     bbox_inches='tight', pad_inches=0.2)
    plt.close(fig_hist)
    print(f"  Saved: {hist_path}")

    # Main figure: 3 panels
    phase_rgb = phase_to_rgb(theta, amp)
    make_figure(
        title="Phase Entropy: What the Detector Destroys",
        panels=[
            {'data': phase_rgb,
             'title': '(a) Phase field theta(x,y)'},
            {'data': born,
             'title': f'(b) |psi|^2  (phase = 0 bits)',
             'cmap': 'gray', 'colorbar': True},
            {'data': np.clip(dots_img, 0, dots_img.max()),
             'title': f'(c) Detector clicks (N={d["N_clicks"]:,})',
             'cmap': 'hot'},
        ],
        metrics_text=(
            f"H(phase) = {H_phase:.3f} bits  |  H(born) = 0.0 bits  |  "
            f"H(detector) = 0.0 bits  |  bits destroyed = {bits_destroyed:.3f}"
        ),
        filename='01_phase_entropy.png',
    )

    # Save JSON
    save_json('01_phase_entropy', {
        'script': '01_phase_entropy.py',
        'description': 'Shannon entropy of the phase field destroyed by Born rule',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'H_phase_bits': H_phase,
            'H_born_bits': H_born,
            'H_detector_bits': H_det,
            'bits_destroyed': bits_destroyed,
            'phase_bins': 256,
            'valid_pixels': int(valid.sum()),
            'total_pixels': W * H,
        },
    })

    print()
    print(f"  RESULT: The full phase field carries {H_phase:.3f} bits of Shannon entropy.")
    print(f"  The Born rule and detector destroy ALL of it.")


if __name__ == '__main__':
    main()
