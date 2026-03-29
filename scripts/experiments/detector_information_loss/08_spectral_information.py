"""
08_spectral_information.py — Power spectrum of psi vs |psi|^2

WHY THIS MATTERS:
F[psi] gives sharp peaks at source k-vectors with full phase information.
F[|psi|^2] is the autocorrelation spectrum — beat frequencies and
self-convolution artifacts but all absolute phase lost.

The Fourier transform of the full complex field preserves every spectral
degree of freedom: each (kx, ky) bin holds an amplitude AND a phase.
Squaring to |psi|^2 before transforming collapses this to the
autocorrelation power spectrum, which is the self-convolution of F[psi].
The result has fewer independent spectral components, broader peaks, and
zero absolute-phase content.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule,
    setup_style, make_figure, save_json,
    DEFAULTS, BG_COLOR, TEXT_COLOR, SUBTLE_COLOR, ACCENT_COLOR, GRID_COLOR,
    OUTPUT_DIR,
)


def radial_power_spectrum(power_2d):
    """Compute radially averaged power spectrum from a 2D power spectrum.

    Returns (k_bins, radial_power) where k_bins are radial wavenumber bin
    centres and radial_power is the mean power in each annular bin.
    """
    H, W = power_2d.shape
    cy, cx = H // 2, W // 2

    yy, xx = np.mgrid[0:H, 0:W]
    r = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2)

    max_r = int(min(cx, cy))
    k_bins = np.arange(1, max_r + 1)
    radial_power = np.zeros(len(k_bins))

    for i, rk in enumerate(k_bins):
        mask = (r >= rk - 0.5) & (r < rk + 0.5)
        if mask.any():
            radial_power[i] = power_2d[mask].mean()

    return k_bins, radial_power


def count_spectral_components(power_2d, threshold_frac=0.001):
    """Count spectral bins with power above threshold_frac * max."""
    pmax = power_2d.max()
    if pmax <= 0:
        return 0
    return int(np.sum(power_2d > threshold_frac * pmax))


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("08  SPECTRAL INFORMATION — Power spectrum of psi vs |psi|^2")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  F[psi] gives sharp peaks at source k-vectors with full phase")
    print("  information.  F[|psi|^2] is the autocorrelation spectrum —")
    print("  beat frequencies and self-convolution artifacts but all")
    print("  absolute phase lost.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute full complex field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    psi = psi_re + 1j * psi_im
    born = born_rule(psi_re, psi_im)

    # 2. Compute 2D FFTs
    print("  Computing 2D FFTs ...")
    F_psi = np.fft.fftshift(np.fft.fft2(psi))
    F_born = np.fft.fftshift(np.fft.fft2(born))

    # Power spectra |F|^2
    power_psi = np.abs(F_psi) ** 2
    power_born = np.abs(F_born) ** 2

    # 3. Count independent spectral components above noise floor
    threshold = 0.001  # 0.1% of max
    n_components_psi = count_spectral_components(power_psi, threshold)
    n_components_born = count_spectral_components(power_born, threshold)
    dof_lost = n_components_psi - n_components_born

    # 4. Total spectral power
    total_power_psi = float(power_psi.sum())
    total_power_born = float(power_born.sum())

    # 5. Radial power spectra
    print("  Computing radial power spectra ...")
    k_bins_psi, radial_psi = radial_power_spectrum(power_psi)
    k_bins_born, radial_born = radial_power_spectrum(power_born)

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Spectral components (>{threshold*100:.1f}% of max):")
    print(f"    F[psi]     : {n_components_psi:,} components")
    print(f"    F[|psi|^2] : {n_components_born:,} components")
    print(f"    DOF lost   : {dof_lost:,} spectral degrees of freedom")
    print()
    print(f"  Total spectral power:")
    print(f"    F[psi]     : {total_power_psi:.4e}")
    print(f"    F[|psi|^2] : {total_power_born:.4e}")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    # Log power spectra for display (avoid log(0))
    log_power_psi = np.log10(power_psi + 1e-30)
    log_power_born = np.log10(power_born + 1e-30)

    fig, axes = plt.subplots(2, 2, figsize=(11, 10))
    fig.suptitle("Spectral Information: F[psi] vs F[|psi|^2]",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.98)

    # (a) log |F[psi]|^2
    ax = axes[0, 0]
    im_a = ax.imshow(log_power_psi, origin='lower', cmap='viridis', aspect='equal')
    ax.set_title('(a) log |F[psi]|^2  (complex field)', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_a = fig.colorbar(im_a, ax=ax, fraction=0.046, pad=0.04)
    cb_a.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (b) log |F[|psi|^2]|^2
    ax = axes[0, 1]
    im_b = ax.imshow(log_power_born, origin='lower', cmap='viridis', aspect='equal')
    ax.set_title('(b) log |F[|psi|^2]|^2  (autocorrelation)', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_b = fig.colorbar(im_b, ax=ax, fraction=0.046, pad=0.04)
    cb_b.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (c) Radial power spectrum comparison
    ax = axes[1, 0]
    ax.semilogy(k_bins_psi, radial_psi + 1e-30, color='#66bbff', linewidth=1.2,
                label='F[psi]', alpha=0.9)
    ax.semilogy(k_bins_born, radial_born + 1e-30, color='#ff6666', linewidth=1.2,
                label='F[|psi|^2]', alpha=0.9)
    ax.set_xlabel('Radial wavenumber k', color=TEXT_COLOR, fontsize=9)
    ax.set_ylabel('Mean power', color=TEXT_COLOR, fontsize=9)
    ax.set_title('(c) Radial power spectrum', color=TEXT_COLOR, fontsize=9)
    ax.legend(fontsize=8, facecolor=BG_COLOR, edgecolor=GRID_COLOR,
              labelcolor=TEXT_COLOR)
    ax.grid(True, alpha=0.3)

    # (d) Bar chart of spectral component counts
    ax = axes[1, 1]
    labels = ['F[psi]', 'F[|psi|^2]']
    counts = [n_components_psi, n_components_born]
    colors = ['#66bbff', '#ff6666']
    bars = ax.bar(labels, counts, color=colors, edgecolor='none', width=0.5)
    ax.set_ylabel('Components above noise floor', color=TEXT_COLOR, fontsize=9)
    ax.set_title('(d) Spectral degrees of freedom', color=TEXT_COLOR, fontsize=9)
    ax.grid(True, axis='y', alpha=0.3)

    # Annotate bars with counts
    for bar, count in zip(bars, counts):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + max(counts) * 0.02,
                f'{count:,}', ha='center', va='bottom', color=TEXT_COLOR, fontsize=9)

    # Annotate DOF lost
    ax.annotate(
        f'DOF lost: {dof_lost:,}',
        xy=(0.5, 0.88), xycoords='axes fraction',
        ha='center', va='top', fontsize=10, color='#ffcc66',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e', edgecolor=GRID_COLOR),
    )

    # Metrics text
    metrics_text = (
        f"F[psi]: {n_components_psi:,} components  |  "
        f"F[|psi|^2]: {n_components_born:,} components  |  "
        f"DOF lost: {dof_lost:,}  |  "
        f"threshold: {threshold*100:.1f}% of max"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.04, 1, 0.95])
    outpath = OUTPUT_DIR / "08_spectral_information.png"
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '08_spectral_information.py',
        'description': 'Power spectrum of psi vs |psi|^2 — spectral DOF destroyed by Born rule',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'n_components_psi': n_components_psi,
            'n_components_born': n_components_born,
            'spectral_dof_lost': dof_lost,
            'total_power_psi': total_power_psi,
            'total_power_born': total_power_born,
            'threshold_fraction': threshold,
        },
    }
    save_json('08_spectral_information', summary)

    print()
    print(f"  RESULT: The complex field F[psi] has {n_components_psi:,} spectral")
    print(f"  components above noise floor.  The Born rule collapses this to")
    print(f"  {n_components_born:,} in the autocorrelation spectrum.")
    print(f"  {dof_lost:,} spectral degrees of freedom are destroyed.")
    print()


if __name__ == '__main__':
    main()
