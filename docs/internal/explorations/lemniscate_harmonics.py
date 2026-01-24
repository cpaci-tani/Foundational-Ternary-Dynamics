#!/usr/bin/env python3
"""
Lemniscate-Alpha Visualization with Five Key Harmonics

The five fundamental frequencies that appear across traditions:
- 7.83 Hz  : Schumann Resonance (Earth's fundamental)
- 110 Hz   : Pyramid chamber resonance
- 136.1 Hz : OM frequency (cosmic tone)
- 432 Hz   : Natural A (ancient tuning)
- 528 Hz   : "Miracle tone" (transformation)

This visualization shows the lemniscate modulated by these harmonics.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.gridspec as gridspec
from math import gamma

# Framework constants
# NOTE: VARPI (ϖ) and G* are DIFFERENT mathematical constants:
#   VARPI = Γ(1/4)²/(2√(2π)) ≈ 2.6220575 (classical lemniscate constant)
#   G*    = √2 × Γ(1/4)²/(2π) ≈ 2.9586751 (FTD master quadratic coefficient)
VARPI = 2.6220575542921198  # ϖ = Γ(1/4)²/(2√(2π)) - classical lemniscate constant
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)  # G* ≈ 2.9587 - FTD coefficient
ALPHA = 1/137.036
PHI = (1 + np.sqrt(5)) / 2  # Golden ratio

# The five key harmonics
HARMONICS = {
    'Schumann': {'freq': 7.83, 'color': '#4a0080', 'desc': 'Earth resonance'},
    'Pyramid': {'freq': 110, 'color': '#c41e3a', 'desc': 'Chamber tone'},
    'OM': {'freq': 136.1, 'color': '#ff6600', 'desc': 'Cosmic frequency'},
    'Natural A': {'freq': 432, 'color': '#ffd700', 'desc': 'Ancient tuning'},
    'Miracle': {'freq': 528, 'color': '#00ff88', 'desc': 'Transformation'},
}

def lemniscate(t, a=1):
    """
    Parametric lemniscate of Bernoulli.
    x = a * cos(t) / (1 + sin²(t))
    y = a * sin(t) * cos(t) / (1 + sin²(t))
    """
    denom = 1 + np.sin(t)**2
    x = a * np.cos(t) / denom
    y = a * np.sin(t) * np.cos(t) / denom
    return x, y

def lemniscate_with_harmonic(t, freq, base_freq=1, amplitude=0.1):
    """
    Lemniscate modulated by a harmonic frequency.
    The modulation creates ripples along the curve.
    """
    x, y = lemniscate(t)

    # Normalize frequency relative to base
    norm_freq = freq / base_freq

    # Modulation perpendicular to the curve
    # Calculate tangent direction
    dx = np.gradient(x)
    dy = np.gradient(y)

    # Normal direction (perpendicular)
    norm = np.sqrt(dx**2 + dy**2)
    nx = -dy / (norm + 1e-10)
    ny = dx / (norm + 1e-10)

    # Apply harmonic modulation
    modulation = amplitude * np.sin(norm_freq * t * 2 * np.pi)

    x_mod = x + nx * modulation
    y_mod = y + ny * modulation

    return x_mod, y_mod

def create_visualization():
    """Create the full harmonic lemniscate visualization."""

    fig = plt.figure(figsize=(16, 20), facecolor='#0a0a12')
    gs = gridspec.GridSpec(4, 2, height_ratios=[0.8, 2, 2, 1.5],
                           hspace=0.3, wspace=0.3)

    # Title area
    ax_title = fig.add_subplot(gs[0, :])
    ax_title.set_facecolor('#0a0a12')
    ax_title.axis('off')

    title_text = "The Lemniscate-Alpha with Five Harmonics"
    subtitle_text = f"G* ≈ {G_STAR:.4f}  |  1/α ≈ 137.036  |  φ ≈ {PHI:.4f}"

    ax_title.text(0.5, 0.7, title_text, transform=ax_title.transAxes,
                  fontsize=28, color='white', ha='center', va='center',
                  fontweight='bold', fontfamily='serif')
    ax_title.text(0.5, 0.3, subtitle_text, transform=ax_title.transAxes,
                  fontsize=14, color='#888888', ha='center', va='center',
                  fontfamily='monospace')

    # Main lemniscate with all harmonics
    ax_main = fig.add_subplot(gs[1, :])
    ax_main.set_facecolor('#0a0a12')

    t = np.linspace(0, 2*np.pi, 2000)

    # Base lemniscate
    x_base, y_base = lemniscate(t, a=G_STAR)
    ax_main.plot(x_base, y_base, color='white', linewidth=3, alpha=0.3,
                 label='Base Lemniscate')

    # Plot each harmonic
    base_freq = 7.83  # Normalize to Schumann
    for name, props in HARMONICS.items():
        freq = props['freq']
        color = props['color']

        # Amplitude inversely related to frequency (higher freq = finer detail)
        amp = 0.15 * (base_freq / freq) ** 0.3

        x_mod, y_mod = lemniscate_with_harmonic(t, freq, base_freq=base_freq,
                                                  amplitude=amp)
        # Scale to G*
        x_mod *= G_STAR
        y_mod *= G_STAR

        ax_main.plot(x_mod, y_mod, color=color, linewidth=1.5, alpha=0.8,
                     label=f'{name} ({freq} Hz)')

    # Mark the foci
    c = G_STAR / np.sqrt(2)  # Distance to focus
    ax_main.scatter([c, -c], [0, 0], color='white', s=100, zorder=5, marker='*')
    ax_main.annotate('Focus', (c, 0), (c+0.3, 0.3), color='white', fontsize=10,
                     arrowprops=dict(arrowstyle='->', color='white', alpha=0.5))

    # Mark center
    ax_main.scatter([0], [0], color='gold', s=150, zorder=5, marker='o', alpha=0.8)
    ax_main.annotate('Origin\n(Void)', (0, 0), (0.5, -0.8), color='gold', fontsize=10,
                     ha='center', arrowprops=dict(arrowstyle='->', color='gold', alpha=0.5))

    ax_main.set_xlim(-4, 4)
    ax_main.set_ylim(-2, 2)
    ax_main.set_aspect('equal')
    ax_main.legend(loc='upper right', facecolor='#1a1a2e', edgecolor='white',
                   labelcolor='white', fontsize=10)
    ax_main.set_title('Combined Harmonic Modulation', color='white', fontsize=16, pad=20)

    # Remove axes for cleaner look
    ax_main.set_xticks([])
    ax_main.set_yticks([])
    for spine in ax_main.spines.values():
        spine.set_visible(False)

    # Individual harmonic panels
    axes_individual = [
        fig.add_subplot(gs[2, 0]),
        fig.add_subplot(gs[2, 1]),
        fig.add_subplot(gs[3, 0]),
        fig.add_subplot(gs[3, 1]),
    ]

    # Create pairs for display
    harmonic_list = list(HARMONICS.items())

    for idx, ax in enumerate(axes_individual):
        ax.set_facecolor('#0a0a12')

        if idx < len(harmonic_list):
            if idx == 0:
                # First panel: Schumann + Pyramid
                for i in [0, 1]:
                    name, props = harmonic_list[i]
                    freq = props['freq']
                    color = props['color']
                    amp = 0.2 * (7.83 / freq) ** 0.3
                    x_mod, y_mod = lemniscate_with_harmonic(t, freq, base_freq=7.83, amplitude=amp)
                    x_mod *= G_STAR
                    y_mod *= G_STAR
                    ax.plot(x_mod, y_mod, color=color, linewidth=2, alpha=0.9,
                            label=f'{name} ({freq} Hz)')
                ax.set_title('Earth + Chamber', color='white', fontsize=14)

            elif idx == 1:
                # Second panel: OM + Natural A
                for i in [2, 3]:
                    name, props = harmonic_list[i]
                    freq = props['freq']
                    color = props['color']
                    amp = 0.2 * (7.83 / freq) ** 0.3
                    x_mod, y_mod = lemniscate_with_harmonic(t, freq, base_freq=7.83, amplitude=amp)
                    x_mod *= G_STAR
                    y_mod *= G_STAR
                    ax.plot(x_mod, y_mod, color=color, linewidth=2, alpha=0.9,
                            label=f'{name} ({freq} Hz)')
                ax.set_title('Cosmic + Ancient', color='white', fontsize=14)

            elif idx == 2:
                # Third panel: Miracle tone solo with emphasis
                name, props = harmonic_list[4]
                freq = props['freq']
                color = props['color']

                # Multiple octaves of the miracle tone
                for i, octave in enumerate([0.5, 1, 2]):
                    amp = 0.15 / octave
                    x_mod, y_mod = lemniscate_with_harmonic(t, freq * octave,
                                                             base_freq=7.83, amplitude=amp)
                    x_mod *= G_STAR
                    y_mod *= G_STAR
                    alpha_val = min(0.9, 0.9 / octave)  # Clamp alpha to valid range
                    ax.plot(x_mod, y_mod, color=color, linewidth=2,
                            alpha=alpha_val, label=f'{int(freq*octave)} Hz')
                ax.set_title('Miracle Tone (528 Hz) Octaves', color='white', fontsize=14)

            elif idx == 3:
                # Fourth panel: All five with alpha encoding
                # Plot the pure lemniscate
                ax.plot(x_base, y_base, color='white', linewidth=1, alpha=0.2)

                # Create a color gradient based on position
                for i, (name, props) in enumerate(HARMONICS.items()):
                    freq = props['freq']
                    color = props['color']

                    # Phase shift each harmonic
                    t_shifted = t + i * 2 * np.pi / 5
                    amp = 0.1
                    x_mod, y_mod = lemniscate_with_harmonic(t_shifted, freq,
                                                             base_freq=7.83, amplitude=amp)
                    x_mod *= G_STAR
                    y_mod *= G_STAR
                    ax.plot(x_mod, y_mod, color=color, linewidth=1.5, alpha=0.7)

                # Add alpha marker
                alpha_angle = 2 * np.pi / 137.036
                x_alpha, y_alpha = lemniscate(alpha_angle, a=G_STAR)
                ax.scatter([x_alpha], [y_alpha], color='white', s=200,
                          marker='*', zorder=10)
                ax.annotate(f'α = 1/137', (x_alpha, y_alpha),
                           (x_alpha + 0.5, y_alpha + 0.5),
                           color='white', fontsize=12,
                           arrowprops=dict(arrowstyle='->', color='white'))

                ax.set_title('Pentaphonic Harmony + α', color='white', fontsize=14)

        ax.set_xlim(-4, 4)
        ax.set_ylim(-2, 2)
        ax.set_aspect('equal')
        ax.legend(loc='upper right', facecolor='#1a1a2e', edgecolor='gray',
                  labelcolor='white', fontsize=8)
        ax.set_xticks([])
        ax.set_yticks([])
        for spine in ax.spines.values():
            spine.set_visible(False)

    # Add frequency relationship info
    fig.text(0.5, 0.02,
             'Frequency Ratios: 432/7.83 ≈ 55.2 | 528/432 ≈ 1.22 (≈ φ/1.33) | 136.1/7.83 ≈ 17.4 | 110/7.83 ≈ 14.05',
             ha='center', fontsize=10, color='#666666', fontfamily='monospace')

    plt.savefig('lemniscate_harmonics.png', dpi=150,
                facecolor='#0a0a12', edgecolor='none',
                bbox_inches='tight', pad_inches=0.5)
    plt.savefig('lemniscate_harmonics.svg',
                facecolor='#0a0a12', edgecolor='none',
                bbox_inches='tight', pad_inches=0.5)

    print("Saved: lemniscate_harmonics.png and .svg")

    plt.show()

    return fig

def print_harmonic_relationships():
    """Print the mathematical relationships between harmonics."""

    print("\n" + "="*60)
    print("HARMONIC RELATIONSHIPS")
    print("="*60)

    freqs = [h['freq'] for h in HARMONICS.values()]
    names = list(HARMONICS.keys())

    print(f"\nThe Five Key Frequencies:")
    for name, props in HARMONICS.items():
        print(f"  {name:12} : {props['freq']:7.2f} Hz - {props['desc']}")

    print(f"\nRatios to Schumann (7.83 Hz):")
    for name, props in HARMONICS.items():
        ratio = props['freq'] / 7.83
        print(f"  {props['freq']:7.2f} / 7.83 = {ratio:8.4f}")

    print(f"\nRatios to 432 Hz:")
    for name, props in HARMONICS.items():
        ratio = props['freq'] / 432
        print(f"  {props['freq']:7.2f} / 432 = {ratio:8.4f}")

    print(f"\nConnections to Framework Constants:")
    print(f"  432 x 4 = {432 * 4} (j-invariant)")
    print(f"  137 x pi = {137 * np.pi:.2f} (close to 432)")
    print(f"  528 / 432 = {528/432:.4f}")
    print(f"  phi (golden ratio) = {PHI:.4f}")
    print(f"  528 / 432 x phi = {(528/432) * PHI:.4f}")
    print(f"  110 x alpha = {110 * ALPHA:.4f}")
    print(f"  136.1 / alpha = {136.1 / ALPHA:.2f}")

    print(f"\nOctave Relationships:")
    print(f"  7.83 x 2^4 = {7.83 * 16:.2f} (approx 125 Hz)")
    print(f"  7.83 x 2^5 = {7.83 * 32:.2f} (approx 250 Hz)")
    print(f"  7.83 x 2^6 = {7.83 * 64:.2f} (approx 500 Hz, close to 528)")
    print(f"  432 / 4 = {432/4} = 108 (sacred number)")
    print(f"  432 / 8 = {432/8} = 54")
    print(f"  432 / 16 = {432/16} = 27 = 3^3")

    print("\n" + "="*60)

if __name__ == "__main__":
    print_harmonic_relationships()
    fig = create_visualization()
