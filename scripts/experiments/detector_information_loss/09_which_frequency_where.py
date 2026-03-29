"""
09_which_frequency_where.py — The spectrogram the detector smears

WHY THIS MATTERS:
STFT of psi along the detection axis gives a spectrogram — local frequency
content varying with position.  |psi|^2 spectrogram is degraded.  Detector
dots have almost no usable spectrogram.

A spectrogram answers "which frequencies are present WHERE?" — joint
position-frequency information that wave physics encodes richly.  The Born
rule partially preserves envelope structure but loses carrier phase, halving
usable bandwidth.  Binning into detector clicks adds Poisson noise and
destroys almost all local spectral structure.
"""

import sys
import os
import numpy as np
from scipy.signal import stft as scipy_stft

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    setup_style, save_json,
    DEFAULTS, BG_COLOR, TEXT_COLOR, SUBTLE_COLOR, ACCENT_COLOR, GRID_COLOR,
    OUTPUT_DIR,
)


def compute_stft(signal, nperseg=128, noverlap=None):
    """Compute STFT magnitude of a 1D signal.

    Returns (frequencies, times, magnitude) where magnitude = |STFT|.
    Uses scipy.signal.stft with a Hann window.
    """
    if noverlap is None:
        noverlap = nperseg // 2
    fs = 1.0  # 1 pixel per sample
    f, t, Zxx = scipy_stft(signal, fs=fs, nperseg=nperseg, noverlap=noverlap,
                           window='hann', return_onesided=False)
    return f, t, np.abs(Zxx)


def spectral_entropy_per_bin(stft_mag):
    """Compute Shannon spectral entropy for each time/position bin.

    For each position column of the STFT, normalize the power distribution
    across frequencies and compute its entropy in bits.

    Returns 1D array of entropy values (one per position bin).
    """
    power = stft_mag ** 2
    # Normalize each column to a probability distribution over frequencies
    col_sums = power.sum(axis=0)
    col_sums = np.maximum(col_sums, 1e-30)
    p = power / col_sums[np.newaxis, :]

    # Shannon entropy per column
    with np.errstate(divide='ignore', invalid='ignore'):
        log_p = np.where(p > 0, np.log2(p), 0.0)
    entropy = -np.sum(p * log_p, axis=0)
    return entropy


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("09  WHICH FREQUENCY WHERE — The spectrogram the detector smears")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  STFT of psi along the detection axis gives a spectrogram —")
    print("  local frequency content varying with position.  |psi|^2")
    print("  spectrogram is degraded.  Detector dots have almost no")
    print("  usable spectrogram.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    born = born_rule(psi_re, psi_im)
    clicks = sample_detector_clicks(born, d['N_clicks'])

    # 2. Extract midlines
    mid_y = H // 2
    psi_mid = psi_re[mid_y, :] + 1j * psi_im[mid_y, :]
    born_mid = born[mid_y, :]

    # Detector click histogram along x-axis
    click_hist = np.zeros(W, dtype=np.float64)
    if len(clicks) > 0:
        x_coords = clicks[:, 0].astype(int)
        x_coords = np.clip(x_coords, 0, W - 1)
        np.add.at(click_hist, x_coords, 1.0)

    # 3. STFT parameters: window = 4*lambda pixels
    nperseg = int(4 * d['lam'])  # 128 for lam=32
    nperseg = min(nperseg, W // 2)  # Safety clamp

    print(f"  STFT window: {nperseg} pixels ({nperseg / d['lam']:.1f} lambda)")
    print("  Computing STFTs ...")

    # STFT of complex psi midline
    f_psi, t_psi, stft_psi_mag = compute_stft(psi_mid, nperseg=nperseg)

    # STFT of |psi|^2 midline (real signal)
    f_born, t_born, stft_born_mag = compute_stft(born_mid, nperseg=nperseg)

    # STFT of detector histogram midline (real signal)
    f_det, t_det, stft_det_mag = compute_stft(click_hist, nperseg=nperseg)

    # 4. Spectral concentration per position bin
    #    concentration = 1 - H/H_max, where H_max = log2(N_freq).
    #    A concentrated spectrum (low entropy) means MORE useful structure.
    #    psi should score higher concentration than |psi|^2.
    print("  Computing spectral concentration per position ...")
    entropy_psi = spectral_entropy_per_bin(stft_psi_mag)
    entropy_born = spectral_entropy_per_bin(stft_born_mag)
    entropy_det = spectral_entropy_per_bin(stft_det_mag)

    n_freq = stft_psi_mag.shape[0]
    H_max = np.log2(n_freq)

    conc_psi = 1.0 - entropy_psi / H_max
    conc_born = 1.0 - entropy_born / H_max
    conc_det = 1.0 - entropy_det / H_max

    mean_conc_psi = float(np.mean(conc_psi))
    mean_conc_born = float(np.mean(conc_born))
    mean_conc_det = float(np.mean(conc_det))

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Mean spectral concentration per position bin (1 - H/H_max):")
    print(f"    psi (complex)  : {mean_conc_psi:.4f}")
    print(f"    |psi|^2 (Born) : {mean_conc_born:.4f}")
    print(f"    detector hist  : {mean_conc_det:.4f}")
    print(f"    (H_max = log2({n_freq}) = {H_max:.2f} bits)")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels (2x2)
    plt = setup_style()

    fig, axes = plt.subplots(2, 2, figsize=(12, 9))
    fig.suptitle("Which Frequency Where: The Spectrogram the Detector Smears",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.98)

    # Use fftshift for display so zero-frequency is in the centre
    def prepare_stft_display(stft_mag, freqs):
        """Shift zero-frequency to centre for display."""
        shift_order = np.argsort(np.fft.fftshift(freqs))
        shifted = stft_mag[shift_order, :]
        return shifted

    # (a) |STFT(psi)| spectrogram
    ax = axes[0, 0]
    display_psi = prepare_stft_display(stft_psi_mag, f_psi)
    extent_psi = [t_psi[0], t_psi[-1], -0.5, 0.5]
    im_a = ax.imshow(np.log10(display_psi + 1e-30), origin='lower', aspect='auto',
                     cmap='inferno', extent=extent_psi)
    ax.set_xlabel('Position (pixels)', color=TEXT_COLOR, fontsize=8)
    ax.set_ylabel('Frequency (cycles/px)', color=TEXT_COLOR, fontsize=8)
    ax.set_title('(a) |STFT(psi)|  (full complex field)', color=TEXT_COLOR, fontsize=9)
    cb_a = fig.colorbar(im_a, ax=ax, fraction=0.046, pad=0.04)
    cb_a.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (b) |STFT(|psi|^2)| spectrogram
    ax = axes[0, 1]
    display_born = prepare_stft_display(stft_born_mag, f_born)
    extent_born = [t_born[0], t_born[-1], -0.5, 0.5]
    im_b = ax.imshow(np.log10(display_born + 1e-30), origin='lower', aspect='auto',
                     cmap='inferno', extent=extent_born)
    ax.set_xlabel('Position (pixels)', color=TEXT_COLOR, fontsize=8)
    ax.set_ylabel('Frequency (cycles/px)', color=TEXT_COLOR, fontsize=8)
    ax.set_title('(b) |STFT(|psi|^2)|  (Born rule)', color=TEXT_COLOR, fontsize=9)
    cb_b = fig.colorbar(im_b, ax=ax, fraction=0.046, pad=0.04)
    cb_b.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (c) |STFT(detector histogram)| spectrogram
    ax = axes[1, 0]
    display_det = prepare_stft_display(stft_det_mag, f_det)
    extent_det = [t_det[0], t_det[-1], -0.5, 0.5]
    im_c = ax.imshow(np.log10(display_det + 1e-30), origin='lower', aspect='auto',
                     cmap='inferno', extent=extent_det)
    ax.set_xlabel('Position (pixels)', color=TEXT_COLOR, fontsize=8)
    ax.set_ylabel('Frequency (cycles/px)', color=TEXT_COLOR, fontsize=8)
    ax.set_title(f'(c) |STFT(detector)|  (N={d["N_clicks"]:,} clicks)',
                 color=TEXT_COLOR, fontsize=9)
    cb_c = fig.colorbar(im_c, ax=ax, fraction=0.046, pad=0.04)
    cb_c.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (d) Spectral concentration per position: three curves overlaid
    ax = axes[1, 1]

    # Map STFT time bins to approximate pixel positions
    pos_psi = t_psi
    pos_born = t_born
    pos_det = t_det

    ax.plot(pos_psi, conc_psi, color='#66bbff', linewidth=1.3,
            label='psi (complex)', alpha=0.9)
    ax.plot(pos_born, conc_born, color='#ff6666', linewidth=1.3,
            label='|psi|^2 (Born)', alpha=0.9)
    ax.plot(pos_det, conc_det, color='#88cc44', linewidth=1.3,
            label='detector hist', alpha=0.9)
    ax.set_xlabel('Position (pixels)', color=TEXT_COLOR, fontsize=8)
    ax.set_ylabel('Spectral concentration (1 - H/H_max)', color=TEXT_COLOR, fontsize=8)
    ax.set_title('(d) Spectral concentration per position', color=TEXT_COLOR, fontsize=9)
    ax.legend(fontsize=8, facecolor=BG_COLOR, edgecolor=GRID_COLOR,
              labelcolor=TEXT_COLOR, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Annotate mean concentrations
    ax.annotate(
        f'mean: {mean_conc_psi:.2f} / {mean_conc_born:.2f} / {mean_conc_det:.2f}',
        xy=(0.5, 0.05), xycoords='axes fraction',
        ha='center', va='bottom', fontsize=8, color='#ffcc66',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e', edgecolor=GRID_COLOR),
    )

    # Metrics text
    metrics_text = (
        f"Mean spectral concentration (1-H/Hmax):  psi={mean_conc_psi:.3f}  |  "
        f"|psi|^2={mean_conc_born:.3f}  |  "
        f"detector={mean_conc_det:.3f}  |  "
        f"window={nperseg}px"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.04, 1, 0.95])
    outpath = OUTPUT_DIR / "09_which_frequency_where.png"
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '09_which_frequency_where.py',
        'description': 'STFT spectrograms of psi, |psi|^2, and detector histogram along midline',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'stft_params': {
            'nperseg': nperseg,
            'window': 'hann',
            'noverlap': nperseg // 2,
        },
        'results': {
            'mean_spectral_concentration_psi': mean_conc_psi,
            'mean_spectral_concentration_born': mean_conc_born,
            'mean_spectral_concentration_detector': mean_conc_det,
            'H_max_bits': float(H_max),
            'n_position_bins_psi': len(t_psi),
            'n_frequency_bins': n_freq,
        },
    }
    save_json('09_which_frequency_where', summary)

    print()
    print(f"  RESULT: The psi spectrogram has mean spectral concentration {mean_conc_psi:.3f}")
    print(f"  (higher = more structured).  |psi|^2 degrades to {mean_conc_born:.3f}.")
    print(f"  The detector histogram collapses to {mean_conc_det:.3f} —")
    print(f"  almost no recoverable local frequency structure.")
    print()


if __name__ == '__main__':
    main()
