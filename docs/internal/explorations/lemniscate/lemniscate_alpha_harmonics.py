#!/usr/bin/env python3
"""
Lemniscate-Alpha Curve with Five Key Harmonics

The Lemniscate-Alpha is a Fourier curve defined by:
  x(t) = sum of a_j * cos(f_j * t)
  y(t) = sum of b_j * sin(f_j * t)

where frequencies are [1, 2, 4, 8, 16] (power-of-2 sequence)
and coefficients encode the framework integers {3, 4, 7, 13}.

This visualization modulates the curve with five key frequencies:
- 7.83 Hz  : Schumann Resonance (Earth's fundamental)
- 110 Hz   : Pyramid chamber resonance
- 136.1 Hz : OM frequency (cosmic tone)
- 432 Hz   : Natural A (ancient tuning)
- 528 Hz   : "Miracle tone" (transformation)
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from math import gamma

# =============================================================================
# LEMNISCATE-ALPHA CURVE DEFINITION
# =============================================================================

# Frequencies (power of 2 sequence)
FREQS = np.array([1, 2, 4, 8, 16])

# Coefficients (derived from six constraints)
# x-amplitudes
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
# y-amplitudes (with chirality signs)
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

# Framework constants
G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)  # ≈ 2.9587
ALPHA = 1/137.036
PHI = (1 + np.sqrt(5)) / 2  # Golden ratio

# Framework integers
B3 = 7       # QCD beta coefficient (from y8 = -7/20)
N_C = 3      # Color charges
N_EFF = 13   # Effective dimension = b3 + 2*N_c

# The five key harmonics
HARMONICS = {
    'Schumann': {'freq': 7.83, 'color': '#8B5CF6', 'desc': 'Earth resonance'},
    'Pyramid': {'freq': 110, 'color': '#EC4899', 'desc': 'Chamber tone'},
    'OM': {'freq': 136.1, 'color': '#F97316', 'desc': 'Cosmic frequency'},
    'Natural A': {'freq': 432, 'color': '#EAB308', 'desc': 'Ancient tuning'},
    'Miracle': {'freq': 528, 'color': '#22C55E', 'desc': 'Transformation'},
}

# =============================================================================
# CURVE COMPUTATIONS
# =============================================================================

def lemniscate_alpha(t, scale=1.0):
    """
    Compute the Lemniscate-Alpha curve at parameter t.
    This is the Fourier curve, NOT the Bernoulli figure-8.
    """
    x = np.zeros_like(t)
    y = np.zeros_like(t)

    for j in range(5):
        x += X_AMPS[j] * np.cos(FREQS[j] * t)
        y += Y_AMPS[j] * np.sin(FREQS[j] * t)

    return x * scale, y * scale

def lemniscate_alpha_derivative(t):
    """Compute dx/dt and dy/dt for the Lemniscate-Alpha curve."""
    dx = np.zeros_like(t)
    dy = np.zeros_like(t)

    for j in range(5):
        dx += -FREQS[j] * X_AMPS[j] * np.sin(FREQS[j] * t)
        dy += FREQS[j] * Y_AMPS[j] * np.cos(FREQS[j] * t)

    return dx, dy

def modulate_curve(t, harmonic_freq, base_scale=1.0, mod_amplitude=0.08):
    """
    Modulate the Lemniscate-Alpha curve with a harmonic frequency.
    The modulation is applied perpendicular to the curve.
    """
    x, y = lemniscate_alpha(t, scale=base_scale)
    dx, dy = lemniscate_alpha_derivative(t)

    # Compute normal direction (perpendicular to tangent)
    norm = np.sqrt(dx**2 + dy**2) + 1e-10
    nx = -dy / norm
    ny = dx / norm

    # Modulation based on harmonic frequency
    # Normalize to a reasonable visual range
    normalized_freq = harmonic_freq / 7.83  # Normalize to Schumann
    modulation = mod_amplitude * np.sin(normalized_freq * t)

    x_mod = x + nx * modulation
    y_mod = y + ny * modulation

    return x_mod, y_mod

def compute_arc_length(n_points=100000):
    """Compute the arc length of the Lemniscate-Alpha curve."""
    t = np.linspace(0, 2*np.pi, n_points)
    dx, dy = lemniscate_alpha_derivative(t)
    dt = 2 * np.pi / n_points
    L = np.sum(np.sqrt(dx**2 + dy**2)) * dt
    return L

# =============================================================================
# VISUALIZATION
# =============================================================================

def create_visualization():
    """Create the full harmonic lemniscate-alpha visualization."""

    fig = plt.figure(figsize=(18, 22), facecolor='#0d1117')
    gs = gridspec.GridSpec(4, 2, height_ratios=[0.6, 2.5, 2, 1.8],
                           hspace=0.25, wspace=0.2)

    t = np.linspace(0, 2*np.pi, 3000)

    # =========================================================================
    # Title Panel
    # =========================================================================
    ax_title = fig.add_subplot(gs[0, :])
    ax_title.set_facecolor('#0d1117')
    ax_title.axis('off')

    title_text = "The Lemniscate-Alpha with Five Sacred Harmonics"
    subtitle_text = f"G* = {G_STAR:.4f}  |  1/alpha = 137.036  |  n_eff = {N_EFF}  |  phi = {PHI:.4f}"

    ax_title.text(0.5, 0.65, title_text, transform=ax_title.transAxes,
                  fontsize=26, color='white', ha='center', va='center',
                  fontweight='bold', fontfamily='serif')
    ax_title.text(0.5, 0.25, subtitle_text, transform=ax_title.transAxes,
                  fontsize=12, color='#8b949e', ha='center', va='center',
                  fontfamily='monospace')

    # =========================================================================
    # Main Panel: All harmonics overlaid
    # =========================================================================
    ax_main = fig.add_subplot(gs[1, :])
    ax_main.set_facecolor('#0d1117')

    # Base curve (white, subtle)
    x_base, y_base = lemniscate_alpha(t, scale=1.0)
    ax_main.plot(x_base, y_base, color='white', linewidth=2.5, alpha=0.15,
                 label='Pure Lemniscate-Alpha', zorder=1)

    # Plot each harmonic modulation
    for i, (name, props) in enumerate(HARMONICS.items()):
        freq = props['freq']
        color = props['color']

        # Amplitude scales inversely with frequency (higher = finer detail)
        amp = 0.12 * (7.83 / freq) ** 0.4

        x_mod, y_mod = modulate_curve(t, freq, base_scale=1.0, mod_amplitude=amp)
        ax_main.plot(x_mod, y_mod, color=color, linewidth=1.8, alpha=0.85,
                     label=f'{name} ({freq} Hz)', zorder=2+i)

    # Mark special points
    # Origin (void/source)
    ax_main.scatter([0], [0], color='#fbbf24', s=200, zorder=10,
                    marker='o', edgecolor='white', linewidth=2)
    ax_main.annotate('Origin\n(Source)', (0, 0), (0.4, -0.6),
                     color='#fbbf24', fontsize=11, ha='center',
                     arrowprops=dict(arrowstyle='->', color='#fbbf24', alpha=0.7))

    # Point at t = 2*pi/137 (alpha encoding)
    t_alpha = 2 * np.pi / 137
    x_a, y_a = lemniscate_alpha(np.array([t_alpha]), scale=1.0)
    ax_main.scatter(x_a, y_a, color='white', s=180, zorder=10,
                    marker='*', edgecolor='#22C55E', linewidth=2)
    ax_main.annotate(f'alpha = 1/137', (x_a[0], y_a[0]), (x_a[0]+0.4, y_a[0]+0.4),
                     color='white', fontsize=11,
                     arrowprops=dict(arrowstyle='->', color='white', alpha=0.7))

    ax_main.set_xlim(-2.8, 2.8)
    ax_main.set_ylim(-1.8, 1.8)
    ax_main.set_aspect('equal')
    ax_main.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
                   labelcolor='white', fontsize=10, framealpha=0.9)
    ax_main.set_title('Pentaphonic Modulation of the Lemniscate-Alpha',
                      color='white', fontsize=16, pad=15)

    for spine in ax_main.spines.values():
        spine.set_color('#30363d')
    ax_main.set_xticks([])
    ax_main.set_yticks([])

    # =========================================================================
    # Panel 2: Individual harmonics (2x2 grid in row 2)
    # =========================================================================
    harmonic_list = list(HARMONICS.items())

    # Left: Low frequencies (Schumann + Pyramid)
    ax_low = fig.add_subplot(gs[2, 0])
    ax_low.set_facecolor('#0d1117')
    ax_low.plot(x_base, y_base, color='white', linewidth=1.5, alpha=0.1)

    for name, props in [harmonic_list[0], harmonic_list[1]]:
        freq = props['freq']
        color = props['color']
        amp = 0.15 * (7.83 / freq) ** 0.3
        x_mod, y_mod = modulate_curve(t, freq, base_scale=1.0, mod_amplitude=amp)
        ax_low.plot(x_mod, y_mod, color=color, linewidth=2, alpha=0.9,
                    label=f'{name} ({freq} Hz)')

    ax_low.set_xlim(-2.8, 2.8)
    ax_low.set_ylim(-1.8, 1.8)
    ax_low.set_aspect('equal')
    ax_low.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
                  labelcolor='white', fontsize=9)
    ax_low.set_title('Earth + Chamber Resonance', color='white', fontsize=13, pad=10)
    for spine in ax_low.spines.values():
        spine.set_color('#30363d')
    ax_low.set_xticks([])
    ax_low.set_yticks([])

    # Right: Mid frequencies (OM + Natural A)
    ax_mid = fig.add_subplot(gs[2, 1])
    ax_mid.set_facecolor('#0d1117')
    ax_mid.plot(x_base, y_base, color='white', linewidth=1.5, alpha=0.1)

    for name, props in [harmonic_list[2], harmonic_list[3]]:
        freq = props['freq']
        color = props['color']
        amp = 0.15 * (7.83 / freq) ** 0.3
        x_mod, y_mod = modulate_curve(t, freq, base_scale=1.0, mod_amplitude=amp)
        ax_mid.plot(x_mod, y_mod, color=color, linewidth=2, alpha=0.9,
                    label=f'{name} ({freq} Hz)')

    ax_mid.set_xlim(-2.8, 2.8)
    ax_mid.set_ylim(-1.8, 1.8)
    ax_mid.set_aspect('equal')
    ax_mid.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
                  labelcolor='white', fontsize=9)
    ax_mid.set_title('Cosmic + Ancient Tuning', color='white', fontsize=13, pad=10)
    for spine in ax_mid.spines.values():
        spine.set_color('#30363d')
    ax_mid.set_xticks([])
    ax_mid.set_yticks([])

    # =========================================================================
    # Panel 3: Miracle Tone focus + Framework connections
    # =========================================================================

    # Left: Miracle tone with octaves
    ax_miracle = fig.add_subplot(gs[3, 0])
    ax_miracle.set_facecolor('#0d1117')
    ax_miracle.plot(x_base, y_base, color='white', linewidth=1.5, alpha=0.1)

    name, props = harmonic_list[4]  # Miracle tone
    base_color = props['color']

    for octave, alpha_val in [(0.5, 0.5), (1.0, 0.85), (2.0, 0.6)]:
        freq = props['freq'] * octave
        amp = 0.12 / (octave ** 0.5)
        x_mod, y_mod = modulate_curve(t, freq, base_scale=1.0, mod_amplitude=amp)
        ax_miracle.plot(x_mod, y_mod, color=base_color, linewidth=2,
                        alpha=alpha_val, label=f'{int(freq)} Hz')

    ax_miracle.set_xlim(-2.8, 2.8)
    ax_miracle.set_ylim(-1.8, 1.8)
    ax_miracle.set_aspect('equal')
    ax_miracle.legend(loc='upper right', facecolor='#161b22', edgecolor='#30363d',
                      labelcolor='white', fontsize=9)
    ax_miracle.set_title('Miracle Tone (528 Hz) Octaves', color='white', fontsize=13, pad=10)
    for spine in ax_miracle.spines.values():
        spine.set_color('#30363d')
    ax_miracle.set_xticks([])
    ax_miracle.set_yticks([])

    # Right: All five phase-shifted
    ax_phase = fig.add_subplot(gs[3, 1])
    ax_phase.set_facecolor('#0d1117')
    ax_phase.plot(x_base, y_base, color='white', linewidth=1.5, alpha=0.15)

    for i, (name, props) in enumerate(HARMONICS.items()):
        freq = props['freq']
        color = props['color']

        # Phase shift each harmonic
        t_shifted = t + i * 2 * np.pi / 5
        amp = 0.08
        x_mod, y_mod = modulate_curve(t_shifted, freq, base_scale=1.0, mod_amplitude=amp)
        ax_phase.plot(x_mod, y_mod, color=color, linewidth=1.5, alpha=0.8)

    # Add the j-invariant connection
    ax_phase.text(0.5, 0.92, '432 x 4 = 1728 (j-invariant)',
                  transform=ax_phase.transAxes, color='#EAB308', fontsize=10,
                  ha='center', fontfamily='monospace')
    ax_phase.text(0.5, 0.08, '137 x pi = 430.4 (near 432)',
                  transform=ax_phase.transAxes, color='white', fontsize=10,
                  ha='center', fontfamily='monospace', alpha=0.7)

    ax_phase.set_xlim(-2.8, 2.8)
    ax_phase.set_ylim(-1.8, 1.8)
    ax_phase.set_aspect('equal')
    ax_phase.set_title('Pentaphonic Phase Dance', color='white', fontsize=13, pad=10)
    for spine in ax_phase.spines.values():
        spine.set_color('#30363d')
    ax_phase.set_xticks([])
    ax_phase.set_yticks([])

    # =========================================================================
    # Footer: Relationships
    # =========================================================================
    fig.text(0.5, 0.02,
             'Arc Length L = 23.79  |  G*/L = 91/732  |  Frequencies: [1, 2, 4, 8, 16]  |  Coefficients encode {3, 4, 7, 13}',
             ha='center', fontsize=10, color='#6e7681', fontfamily='monospace')

    # Save
    plt.savefig('lemniscate_alpha_harmonics.png', dpi=200,
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)
    plt.savefig('lemniscate_alpha_harmonics.svg',
                facecolor='#0d1117', edgecolor='none',
                bbox_inches='tight', pad_inches=0.3)

    print("Saved: lemniscate_alpha_harmonics.png and .svg")

    plt.show()
    return fig

def print_framework_connections():
    """Print the mathematical relationships."""

    print("\n" + "="*70)
    print("LEMNISCATE-ALPHA FRAMEWORK CONNECTIONS")
    print("="*70)

    print("\nThe Lemniscate-Alpha Curve:")
    print("  x(t) = sum of a_j * cos(f_j * t)")
    print("  y(t) = sum of b_j * sin(f_j * t)")
    print(f"  Frequencies: {list(FREQS)}")
    print(f"  X-amplitudes: {list(X_AMPS)}")
    print(f"  Y-amplitudes: {list(Y_AMPS)}")

    L = compute_arc_length()
    print(f"\n  Arc Length L = {L:.4f}")
    print(f"  Lemniscatic constant G* = {G_STAR:.6f}")
    print(f"  G*/L = {G_STAR/L:.6f} = 91/732 = {91/732:.6f}")

    print("\nFramework Integers:")
    print(f"  b3 = {B3} (from y8 coefficient = -7/20)")
    print(f"  N_c = {N_C} (from quadratic root)")
    print(f"  n_eff = b3 + 2*N_c = {N_EFF}")
    print(f"  N_base = 4 (base modes)")

    print("\nThe Five Sacred Harmonics:")
    for name, props in HARMONICS.items():
        print(f"  {name:12} : {props['freq']:7.2f} Hz - {props['desc']}")

    print("\nFrequency Relationships:")
    print(f"  432 x 4 = {432*4} (j-invariant of lemniscatic curve)")
    print(f"  137 x pi = {137*np.pi:.2f} (near 432)")
    print(f"  528 / 432 = {528/432:.4f} (major third ratio 5/4 = 1.25)")
    print(f"  7.83 x 2^6 = {7.83 * 64:.2f} (close to 528)")
    print(f"  432 / 4 = {432/4} (sacred 108)")

    print("\nThe Master Quadratic:")
    print(f"  x^2 - 16*G*^2*x + 16*G*^3 = 0")
    print(f"  Roots: x+ = 137.036 (1/alpha), x- = 3.024 (N_c)")

    print("\n" + "="*70)

if __name__ == "__main__":
    print_framework_connections()
    fig = create_visualization()
