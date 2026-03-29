"""
12_reconstruction_impossibility.py — What can you recover from clicks?

WHY THIS MATTERS:
Given N clicks, you recover |psi|^2 to sqrt(N) resolution. Phase is
provably unrecoverable — infinitely many psi fields produce the same |psi|^2.
This is fundamental: the Born rule is a many-to-one map from C -> R+, and
no amount of click data can invert it.

This script constructs K=4 alternative fields that share the exact same
|psi|^2 as the reference field but have wildly different phase patterns,
proving that detector data cannot distinguish them.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    phase_field, amplitude_field, shannon_entropy,
    setup_style, save_json, detector_dots_image,
    phase_to_rgb, DEFAULTS, OUTPUT_DIR,
)


def generate_smooth_random_phase(H, W, max_k=8, rng=None):
    """Generate a smooth random phase field using low-frequency Fourier components.

    Only modes with |kx| <= max_k and |ky| <= max_k are populated,
    producing slowly varying phase landscapes.
    """
    if rng is None:
        rng = np.random.default_rng()

    # Build random low-frequency Fourier coefficients
    coeffs = np.zeros((H, W), dtype=np.complex128)
    for ky in range(-max_k, max_k + 1):
        for kx in range(-max_k, max_k + 1):
            if kx == 0 and ky == 0:
                continue
            amp = rng.standard_normal() + 1j * rng.standard_normal()
            # Weight by inverse frequency for smoother fields
            freq = np.sqrt(kx ** 2 + ky ** 2)
            amp /= (1.0 + freq)
            coeffs[ky % H, kx % W] = amp

    # Inverse FFT to get a real-valued smooth field
    smooth = np.fft.ifft2(coeffs).real
    # Scale to [-pi, pi]
    smooth = smooth / (np.abs(smooth).max() + 1e-12) * np.pi
    return smooth


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("12  RECONSTRUCTION IMPOSSIBILITY — What can you recover from clicks?")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Given N clicks, you recover |psi|^2 to sqrt(N) resolution.")
    print("  Phase is provably unrecoverable: infinitely many psi fields")
    print("  produce the same |psi|^2. This is fundamental.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']
    K = 4  # number of alternative phase fields
    rng = np.random.default_rng(2026)

    # ------------------------------------------------------------------
    # 1. Compute the reference field
    # ------------------------------------------------------------------
    print("  Computing reference field ...")
    psi_re_0, psi_im_0 = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )
    amp_0 = amplitude_field(psi_re_0, psi_im_0)
    born_0 = born_rule(psi_re_0, psi_im_0)
    phase_0 = phase_field(psi_re_0, psi_im_0)

    # ------------------------------------------------------------------
    # 2. Generate K alternative fields with the SAME |psi|^2
    # ------------------------------------------------------------------
    print(f"  Generating {K} alternative fields with identical |psi|^2 ...")
    alt_phases = []
    alt_fields = []  # (re, im) pairs
    for k_idx in range(K):
        phi_k = generate_smooth_random_phase(H, W, max_k=8, rng=rng)
        # New psi = |psi_0| * exp(i * phi_k)
        re_k = amp_0 * np.cos(phi_k)
        im_k = amp_0 * np.sin(phi_k)
        # Verify: |psi_k|^2 = |psi_0|^2
        born_k = born_rule(re_k, im_k)
        residual = np.max(np.abs(born_k - born_0))
        print(f"    Alt field {k_idx + 1}: max |born_k - born_0| = {residual:.2e}")
        alt_phases.append(phi_k)
        alt_fields.append((re_k, im_k))

    # ------------------------------------------------------------------
    # 3. Sample clicks from the reference |psi|^2
    # ------------------------------------------------------------------
    N_clicks = d['N_clicks']
    print(f"  Sampling {N_clicks:,} clicks from |psi|^2 ...")
    clicks = sample_detector_clicks(born_0, N_clicks, rng=rng)

    # ------------------------------------------------------------------
    # 4. Reconstruct |psi|^2 from clicks at coarse resolution
    # ------------------------------------------------------------------
    coarse_bins = 64
    bin_w = W / coarse_bins
    bin_h = H / coarse_bins

    # Coarse histogram from clicks
    click_hist = np.zeros((coarse_bins, coarse_bins), dtype=np.float64)
    if len(clicks) > 0:
        bx = np.clip((clicks[:, 0] / bin_w).astype(int), 0, coarse_bins - 1)
        by = np.clip((clicks[:, 1] / bin_h).astype(int), 0, coarse_bins - 1)
        np.add.at(click_hist, (by, bx), 1.0)

    # Normalize
    click_hist_norm = click_hist / max(click_hist.sum(), 1e-12)

    # True |psi|^2 at coarse resolution (block average)
    # Downsample born_0 to coarse_bins x coarse_bins
    born_coarse = np.zeros((coarse_bins, coarse_bins))
    for iy in range(coarse_bins):
        for ix in range(coarse_bins):
            y0, y1 = int(iy * bin_h), int((iy + 1) * bin_h)
            x0, x1 = int(ix * bin_w), int((ix + 1) * bin_w)
            born_coarse[iy, ix] = born_0[y0:y1, x0:x1].mean()
    born_coarse_norm = born_coarse / max(born_coarse.sum(), 1e-12)

    # Correlation between reconstruction and truth
    corr_default = np.corrcoef(click_hist_norm.ravel(), born_coarse_norm.ravel())[0, 1]
    print(f"  Reconstruction fidelity ({N_clicks:,} clicks, {coarse_bins}x{coarse_bins}): "
          f"r = {corr_default:.4f}")

    # ------------------------------------------------------------------
    # 5. Reconstruction fidelity vs N
    # ------------------------------------------------------------------
    print("  Computing reconstruction fidelity vs N ...")
    N_values = [10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000]
    fidelities = []
    for N_test in N_values:
        clicks_test = sample_detector_clicks(born_0, N_test, rng=np.random.default_rng(42))
        hist_test = np.zeros((coarse_bins, coarse_bins), dtype=np.float64)
        if len(clicks_test) > 0:
            bx_t = np.clip((clicks_test[:, 0] / bin_w).astype(int), 0, coarse_bins - 1)
            by_t = np.clip((clicks_test[:, 1] / bin_h).astype(int), 0, coarse_bins - 1)
            np.add.at(hist_test, (by_t, bx_t), 1.0)
        hist_test_norm = hist_test / max(hist_test.sum(), 1e-12)
        r = np.corrcoef(hist_test_norm.ravel(), born_coarse_norm.ravel())[0, 1]
        fidelities.append(float(r))
        print(f"    N = {N_test:>7,}:  r = {r:.4f}")

    # ------------------------------------------------------------------
    # Figure
    # ------------------------------------------------------------------
    plt = setup_style()
    import matplotlib.gridspec as gridspec

    fig = plt.figure(figsize=(16, 10))
    fig.patch.set_facecolor('#0a0a0f')
    gs = gridspec.GridSpec(2, 4, figure=fig, hspace=0.35, wspace=0.25,
                           left=0.05, right=0.97, top=0.92, bottom=0.08)

    fig.suptitle("Reconstruction Impossibility: Phase Is Provably Lost",
                 color='#a0b8d8', fontsize=13, fontweight='bold')

    # ---- Row 1: Phase fields ----
    # (a) Reference phase
    phase_rgb_0 = phase_to_rgb(phase_0, amplitude=amp_0)
    ax_a = fig.add_subplot(gs[0, 0])
    ax_a.imshow(phase_rgb_0, origin='lower', aspect='equal')
    ax_a.set_title('(a) Reference phase', color='#c8d8e8', fontsize=9)
    ax_a.set_xticks([])
    ax_a.set_yticks([])

    # (b) Alternative phase 1
    phase_rgb_1 = phase_to_rgb(alt_phases[0], amplitude=amp_0)
    ax_b = fig.add_subplot(gs[0, 1])
    ax_b.imshow(phase_rgb_1, origin='lower', aspect='equal')
    ax_b.set_title('(b) Alt phase 1 — same |psi|^2', color='#c8d8e8', fontsize=9)
    ax_b.set_xticks([])
    ax_b.set_yticks([])

    # (c) Alternative phase 2
    phase_rgb_2 = phase_to_rgb(alt_phases[1], amplitude=amp_0)
    ax_c = fig.add_subplot(gs[0, 2])
    ax_c.imshow(phase_rgb_2, origin='lower', aspect='equal')
    ax_c.set_title('(c) Alt phase 2 — same |psi|^2', color='#c8d8e8', fontsize=9)
    ax_c.set_xticks([])
    ax_c.set_yticks([])

    # (d) Reconstruction from clicks vs true |psi|^2
    ax_d = fig.add_subplot(gs[0, 3])
    ax_d.set_facecolor('#0a0a0f')
    # Show as side-by-side in one panel: left half = truth, right half = reconstruction
    combined = np.zeros((coarse_bins, coarse_bins * 2))
    combined[:, :coarse_bins] = born_coarse_norm / max(born_coarse_norm.max(), 1e-12)
    combined[:, coarse_bins:] = click_hist_norm / max(click_hist_norm.max(), 1e-12)
    ax_d.imshow(combined, origin='lower', cmap='inferno', aspect='equal')
    ax_d.axvline(x=coarse_bins - 0.5, color='#a0b8d8', linewidth=1.5, linestyle='-')
    ax_d.text(coarse_bins * 0.25, coarse_bins * 0.95, 'True',
              ha='center', va='top', color='#c8d8e8', fontsize=8, fontweight='bold')
    ax_d.text(coarse_bins * 0.75 + coarse_bins, coarse_bins * 0.95, f'{N_clicks:,} clicks',
              ha='center', va='top', color='#c8d8e8', fontsize=8, fontweight='bold')
    ax_d.set_title(f'(d) Reconstruction (r={corr_default:.3f})',
                   color='#c8d8e8', fontsize=9)
    ax_d.set_xticks([])
    ax_d.set_yticks([])

    # ---- Row 2 left: All 4 alternative phases as small multiples ----
    ax_multi = fig.add_subplot(gs[1, 0:2])
    ax_multi.set_facecolor('#0a0a0f')

    # Create a 1x5 strip: reference + 4 alternatives
    strip_size = min(128, H)
    # Downsample for the strip
    step = max(1, H // strip_size)
    small_phases = [phase_0[::step, ::step]]
    small_amp = amp_0[::step, ::step]
    for phi_k in alt_phases:
        small_phases.append(phi_k[::step, ::step])

    # Build RGB strips
    strips = []
    for sp in small_phases:
        strips.append(phase_to_rgb(sp, amplitude=small_amp))
    # Add thin white separator
    sep_width = 2
    separator = np.ones((strips[0].shape[0], sep_width, 3)) * 0.15

    combined_strip = strips[0]
    for s in strips[1:]:
        combined_strip = np.concatenate([combined_strip, separator, s], axis=1)

    ax_multi.imshow(combined_strip, origin='lower', aspect='equal')
    ax_multi.set_title('(e) Reference + 4 alt phases — ALL produce identical |psi|^2',
                       color='#c8d8e8', fontsize=9)
    ax_multi.set_xticks([])
    ax_multi.set_yticks([])
    # Labels
    strip_w = strips[0].shape[1]
    for idx, label in enumerate(['Ref', 'Alt1', 'Alt2', 'Alt3', 'Alt4']):
        x_pos = idx * (strip_w + sep_width) + strip_w / 2
        ax_multi.text(x_pos, -5, label, ha='center', va='top',
                      color='#667788', fontsize=7)

    # ---- Row 2 right: Fidelity vs N curve ----
    ax_fid = fig.add_subplot(gs[1, 2:4])
    ax_fid.set_facecolor('#0a0a0f')

    ax_fid.semilogx(N_values, fidelities, 'o-', color='#cc8844',
                     linewidth=2.0, markersize=6, markerfacecolor='#ffcc66',
                     markeredgecolor='#cc8844')

    # Theoretical sqrt(N) convergence reference
    # r ~ 1 - C/sqrt(N), so plot a guide
    N_arr = np.array(N_values, dtype=float)
    if len(fidelities) >= 2 and fidelities[-1] > 0.5:
        # Fit: r = 1 - C/sqrt(N)
        # C = (1-r) * sqrt(N) at the last point
        C_fit = (1 - fidelities[-1]) * np.sqrt(N_values[-1])
        r_theory = 1.0 - C_fit / np.sqrt(N_arr)
        r_theory = np.clip(r_theory, 0, 1)
        ax_fid.semilogx(N_values, r_theory, '--', color='#667788',
                         linewidth=1.0, label=r'$r \approx 1 - C/\sqrt{N}$')

    ax_fid.axhline(y=1.0, color='#44aa88', linewidth=0.8, linestyle=':',
                   alpha=0.5, label='Perfect reconstruction')

    ax_fid.set_xlabel('Number of clicks  N', color='#c8d8e8', fontsize=10)
    ax_fid.set_ylabel('Correlation with true |psi|^2', color='#c8d8e8', fontsize=10)
    ax_fid.set_title('(f) Amplitude reconstruction fidelity vs N',
                     color='#c8d8e8', fontsize=10)
    ax_fid.set_ylim(-0.05, 1.05)
    ax_fid.legend(loc='lower right', fontsize=8, facecolor='#0d0d15',
                  edgecolor='#667788', labelcolor='#c8d8e8')
    ax_fid.grid(True, alpha=0.2, color='#1a1a2e')
    ax_fid.spines['top'].set_visible(False)
    ax_fid.spines['right'].set_visible(False)
    ax_fid.spines['left'].set_color('#667788')
    ax_fid.spines['bottom'].set_color('#667788')

    # Annotation
    ax_fid.annotate('Amplitude recoverable\nPhase NEVER recoverable',
                    xy=(0.98, 0.15), xycoords='axes fraction',
                    ha='right', va='bottom', fontsize=9, color='#cc4444',
                    fontweight='bold',
                    bbox=dict(boxstyle='round,pad=0.4', facecolor='#1a1a2e',
                             edgecolor='#cc4444', alpha=0.9))

    # Bottom metrics
    metrics_text = (
        f"K = {K} alternative phase fields  |  "
        f"All produce identical |psi|^2  |  "
        f"Reconstruction r({N_clicks:,} clicks) = {corr_default:.4f}  |  "
        f"Phase: unrecoverable (infinite degeneracy)"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color='#667788', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                      edgecolor='#1a1a2e'))

    outpath = OUTPUT_DIR / "12_reconstruction_impossibility.png"
    fig.savefig(outpath, dpi=200, facecolor='#0a0a0f',
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    # ------------------------------------------------------------------
    summary = {
        'script': '12_reconstruction_impossibility.py',
        'description': 'Phase unrecoverability and amplitude reconstruction fidelity vs N',
        'parameters': {
            'W': W, 'H': H,
            'K_alternatives': K,
            'coarse_bins': coarse_bins,
            'N_clicks': int(N_clicks),
            'lam': float(d['lam']),
            'separation': float(d['separation']),
        },
        'results': {
            'reconstruction_fidelity_default': float(corr_default),
            'fidelity_vs_N': {
                'N_values': N_values,
                'correlations': fidelities,
            },
            'phase_degeneracy': {
                'K_alternatives': K,
                'all_share_same_born': True,
                'max_born_residual': float(max(
                    np.max(np.abs(born_rule(*alt_fields[i]) - born_0))
                    for i in range(K)
                )),
            },
            'conclusion': (
                'Amplitude (|psi|^2) converges as 1/sqrt(N). '
                'Phase is provably unrecoverable: infinitely many complex fields '
                'produce the same |psi|^2.'
            ),
        },
    }
    save_json('12_reconstruction_impossibility', summary)

    print()
    print(f"  RESULT: {K} fields with wildly different phase produce identical |psi|^2.")
    print(f"  Amplitude reconstruction converges: r = {corr_default:.4f} at {N_clicks:,} clicks.")
    print(f"  Phase is provably unrecoverable from any number of clicks.")
    print()


if __name__ == '__main__':
    main()
