"""
01_phase_entropy.py — Shannon entropy of the phase field

WHY THIS MATTERS:
The full wavefunction psi has phase theta(x,y) at every pixel — a rich
structured distribution encoding the entire interference geometry.
|psi|^2 retains zero phase information.  A boolean detector that only
records click / no-click destroys ALL of this structure.

Quantifies the destruction: H(theta) bits lost per pixel.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    phase_field, amplitude_field, shannon_entropy,
    setup_style, make_figure, save_json, detector_dots_image,
    phase_to_rgb, DEFAULTS,
)


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("01  PHASE ENTROPY — Shannon entropy of the phase field")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  The full psi has phase theta(x,y) at every pixel — a rich")
    print("  structured distribution encoding the entire interference")
    print("  geometry.  |psi|^2 has zero phase information.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    # 2. Extract phase and amplitude
    theta = phase_field(psi_re, psi_im)           # [-pi, pi]
    amp = amplitude_field(psi_re, psi_im)
    born = born_rule(psi_re, psi_im)

    # 3. Shannon entropy of phase (256 bins)
    #    Mask out near-zero amplitude where phase is numerically meaningless
    amp_threshold = np.percentile(amp[amp > 0], 5) if (amp > 0).any() else 0.0
    valid_mask = amp > amp_threshold
    theta_valid = theta[valid_mask]

    H_phase = shannon_entropy(theta_valid, bins=256)

    # For |psi|^2: phase is undefined -> H = 0
    H_born = 0.0

    # For detector clicks: phase unrecoverable -> H = 0
    clicks = sample_detector_clicks(born, d['N_clicks'])
    H_clicks = 0.0

    bits_destroyed = H_phase - H_born

    # ------------------------------------------------------------------
    # Print results
    print(f"  Phase entropy  H(theta)       = {H_phase:.4f} bits")
    print(f"  Born-rule      H(theta|born)  = {H_born:.4f} bits  (phase undefined)")
    print(f"  Detector       H(theta|clicks)= {H_clicks:.4f} bits  (phase unrecoverable)")
    print(f"  Bits destroyed per pixel      = {bits_destroyed:.4f} bits")
    print()

    # ------------------------------------------------------------------
    # Figure
    plt = setup_style()

    # Panel (a): phase field as HSV color
    phase_rgb = phase_to_rgb(theta, amplitude=amp)

    # Panel (b): |psi|^2 grayscale
    born_display = born / max(born.max(), 1e-12)

    # Panel (c): detector dots
    dots_img = detector_dots_image(clicks, W, H)

    # Panel (d): phase histogram with entropy annotation
    #   We build this as a custom overlay on a blank panel
    fig_hist = plt.figure(figsize=(4, 3.5))
    fig_hist.patch.set_facecolor('#0a0a0f')
    ax_hist = fig_hist.add_subplot(111)
    ax_hist.set_facecolor('#0a0a0f')
    counts, bin_edges, _ = ax_hist.hist(
        theta_valid, bins=256, color='#a0b8d8', alpha=0.85,
        edgecolor='none', density=True,
    )
    ax_hist.set_xlabel('Phase theta (rad)', fontsize=9)
    ax_hist.set_ylabel('Density', fontsize=9)
    ax_hist.set_title('Phase distribution', color='#c8d8e8', fontsize=9)
    ax_hist.annotate(
        f'H(theta) = {H_phase:.3f} bits',
        xy=(0.95, 0.92), xycoords='axes fraction',
        ha='right', va='top', fontsize=10, color='#ffcc66',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e', edgecolor='#667788'),
    )
    ax_hist.set_xlim(-np.pi, np.pi)
    plt.tight_layout()
    from experiments.detector_information_loss.field_engine import OUTPUT_DIR
    hist_path = OUTPUT_DIR / "01_phase_histogram.png"
    fig_hist.savefig(hist_path, dpi=200, facecolor='#0a0a0f',
                     bbox_inches='tight', pad_inches=0.1)
    plt.close(fig_hist)
    print(f"  Saved: {hist_path}")

    # Main 3-panel figure
    metrics_text = (
        f"H(phase) = {H_phase:.4f} bits  |  "
        f"H(born) = {H_born:.1f} bits  |  "
        f"H(detector) = {H_clicks:.1f} bits  |  "
        f"Bits destroyed = {bits_destroyed:.4f}"
    )

    panels = [
        {
            'data': phase_rgb,
            'title': '(a) Phase field theta(x,y)',
            'cmap': None,
            'colorbar': False,
        },
        {
            'data': born_display,
            'title': '(b) |psi|^2  (phase = 0 bits)',
            'cmap': 'gray',
            'colorbar': True,
        },
        {
            'data': dots_img,
            'title': f'(c) Detector clicks (N={d["N_clicks"]:,})',
            'cmap': 'hot',
            'colorbar': False,
        },
    ]

    make_figure(
        title="Phase Entropy: What the Detector Destroys",
        panels=panels,
        metrics_text=metrics_text,
        filename="01_phase_entropy.png",
    )

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '01_phase_entropy.py',
        'description': 'Shannon entropy of the phase field vs Born rule vs detector',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'H_phase_bits': float(H_phase),
            'H_born_bits': float(H_born),
            'H_detector_bits': float(H_clicks),
            'bits_destroyed': float(bits_destroyed),
            'phase_bins': 256,
            'valid_pixels': int(valid_mask.sum()),
            'total_pixels': int(W * H),
        },
    }
    save_json('01_phase_entropy', summary)

    print()
    print(f"  RESULT: The full phase field carries {H_phase:.3f} bits of Shannon entropy.")
    print(f"  The Born rule and detector destroy ALL of it.")
    print()


if __name__ == '__main__':
    main()
