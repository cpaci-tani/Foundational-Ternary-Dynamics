"""
Figure: Atomic Spectra from TRD Principles
==========================================
Derives hydrogen energy levels from TRD and shows how alpha = 1/137
appears in spectral lines.

TRD Derivation:
- Energy levels: E_n = -m_e * alpha^2 * c^2 / (2n^2)
- The factor alpha^2 comes from the electromagnetic coupling
- In TRD, alpha = 1/137.036 is derived from the master quadratic
- Spectral transitions: E_photon = E_n - E_m = 13.6 eV * (1/m^2 - 1/n^2)
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Rectangle, FancyBboxPatch
import matplotlib.patches as mpatches
from matplotlib.collections import LineCollection
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / 'manuscript' / 'figures'))

try:
    from utils.style import COLORS, apply_trd_style
    from utils.physics_constants import ALPHA, ALPHA_INV, G_STAR
except ImportError:
    # Fallback definitions
    COLORS = {
        'void': '#888888',
        'matter': '#DD4444',
        'antimatter': '#4488DD',
        'background': '#FFFFFF',
        'text': '#222222',
        'accent1': '#44BB44',
        'accent2': '#9944BB',
        'grid': '#CCCCCC',
        'highlight': '#FFAA00',
    }
    ALPHA = 1 / 137.036
    ALPHA_INV = 137.036
    G_STAR = 2.9587

    def apply_trd_style(ax, title=None, xlabel=None, ylabel=None, grid=True):
        if title:
            ax.set_title(title, fontsize=14, fontweight='bold', pad=10)
        if xlabel:
            ax.set_xlabel(xlabel, fontsize=11)
        if ylabel:
            ax.set_ylabel(ylabel, fontsize=11)
        if grid:
            ax.grid(True, alpha=0.3, color='#CCCCCC', linestyle='-', linewidth=0.5)
            ax.set_axisbelow(True)
        ax.set_facecolor('#FFFFFF')


# =============================================================================
# PHYSICS CONSTANTS AND CALCULATIONS
# =============================================================================

# Physical constants (SI units)
M_E = 9.109e-31      # Electron mass (kg)
C = 2.998e8          # Speed of light (m/s)
H_PLANCK = 6.626e-34 # Planck constant (J·s)
E_CHARGE = 1.602e-19 # Elementary charge (C)

# Rydberg energy in eV (derived from alpha)
# E_R = m_e * c^2 * alpha^2 / 2 = 13.6 eV
RYDBERG_EV = 13.6056  # eV

# Spectral line wavelengths for common transitions
SPECTRAL_SERIES = {
    'Lyman': {
        'n_final': 1,
        'color': '#8B00FF',  # UV (violet/invisible)
        'range': 'Ultraviolet',
        'transitions': [(2,1), (3,1), (4,1), (5,1)],
    },
    'Balmer': {
        'n_final': 2,
        'color': '#FF0000',  # Visible
        'range': 'Visible',
        'transitions': [(3,2), (4,2), (5,2), (6,2)],
    },
    'Paschen': {
        'n_final': 3,
        'color': '#FF4500',  # IR
        'range': 'Infrared',
        'transitions': [(4,3), (5,3), (6,3), (7,3)],
    },
    'Brackett': {
        'n_final': 4,
        'color': '#8B0000',  # Far IR
        'range': 'Far Infrared',
        'transitions': [(5,4), (6,4), (7,4), (8,4)],
    },
}

# Named Balmer lines with their colors
BALMER_LINES = {
    'H-alpha': {'transition': (3, 2), 'wavelength': 656.3, 'color': '#FF0000'},
    'H-beta': {'transition': (4, 2), 'wavelength': 486.1, 'color': '#00FFFF'},
    'H-gamma': {'transition': (5, 2), 'wavelength': 434.0, 'color': '#0000FF'},
    'H-delta': {'transition': (6, 2), 'wavelength': 410.2, 'color': '#9400D3'},
}


def energy_level(n):
    """
    Calculate hydrogen energy level using TRD-derived formula.

    E_n = -m_e * alpha^2 * c^2 / (2n^2) = -13.6 / n^2 eV

    The alpha^2 factor is the key TRD contribution!
    """
    return -RYDBERG_EV / (n ** 2)


def wavelength_nm(n_upper, n_lower):
    """
    Calculate photon wavelength for transition.

    1/lambda = R_H * (1/n_lower^2 - 1/n_upper^2)

    Where R_H = m_e * c * alpha^2 / (2 * h) is the Rydberg constant
    """
    # Energy released = E_upper - E_lower (both negative, upper is less negative)
    delta_E_eV = energy_level(n_upper) - energy_level(n_lower)  # This is positive
    # E = hc/lambda => lambda = hc/E
    # Convert eV to Joules, then to nm
    delta_E_J = delta_E_eV * E_CHARGE
    wavelength_m = H_PLANCK * C / delta_E_J
    return wavelength_m * 1e9  # Convert to nm


def wavelength_to_rgb(wavelength):
    """Convert wavelength (nm) to approximate RGB color."""
    if wavelength < 380:
        return '#8B00FF'  # UV - show as deep violet
    elif wavelength < 440:
        r = (440 - wavelength) / 60
        g = 0
        b = 1
    elif wavelength < 490:
        r = 0
        g = (wavelength - 440) / 50
        b = 1
    elif wavelength < 510:
        r = 0
        g = 1
        b = (510 - wavelength) / 20
    elif wavelength < 580:
        r = (wavelength - 510) / 70
        g = 1
        b = 0
    elif wavelength < 645:
        r = 1
        g = (645 - wavelength) / 65
        b = 0
    elif wavelength <= 780:
        r = 1
        g = 0
        b = 0
    else:
        return '#8B0000'  # IR - show as dark red

    return f'#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}'


# =============================================================================
# MAIN VISUALIZATION
# =============================================================================

def generate_atomic_spectra_trd():
    """
    Generate comprehensive atomic spectra visualization showing TRD derivation.

    Layout:
    - Top left: Energy level diagram with alpha^2 derivation
    - Top right: Spectral emission lines (rainbow)
    - Bottom left: TRD derivation chain
    - Bottom right: Alpha appearance in formulas
    """
    fig = plt.figure(figsize=(16, 14), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Create grid of subplots with more vertical space
    gs = fig.add_gridspec(3, 2, height_ratios=[1.3, 0.7, 0.5],
                          width_ratios=[1, 1], hspace=0.4, wspace=0.3)

    ax_levels = fig.add_subplot(gs[0, 0])      # Energy levels
    ax_spectrum = fig.add_subplot(gs[0, 1])    # Emission spectrum
    ax_series = fig.add_subplot(gs[1, :])      # All spectral series
    ax_derivation = fig.add_subplot(gs[2, :])  # TRD derivation

    # =========================================================================
    # Panel 1: Energy Level Diagram (Top Left)
    # =========================================================================
    ax_levels.set_xlim(-0.5, 8)
    ax_levels.set_ylim(-15, 2)
    ax_levels.set_facecolor('#F8F8F8')

    # Draw energy levels
    n_levels = [1, 2, 3, 4, 5, 6]
    level_width = 6

    for n in n_levels:
        E = energy_level(n)

        # Draw level line
        ax_levels.hlines(E, 0.5, level_width + 0.5, colors='black', linewidth=2.5)

        # Label: quantum number
        ax_levels.text(0.2, E, f'n={n}', fontsize=12, va='center', ha='right',
                      fontweight='bold')

        # Label: energy value
        ax_levels.text(level_width + 0.8, E, f'{E:.3f} eV', fontsize=10,
                      va='center', ha='left', color=COLORS['text'])

        # Show degeneracy only for n=1,2,3 to avoid clutter
        if n <= 3:
            degeneracy = 2 * n**2
            ax_levels.text(level_width/2, E + 0.3, f'({degeneracy} states)',
                          fontsize=7, va='bottom', ha='center',
                          color=COLORS['void'], style='italic')

    # Draw Balmer series transitions with wavy photon lines
    transitions_to_draw = [
        (3, 2, 'H-alpha\n656 nm', '#FF0000', 2.0),
        (4, 2, 'H-beta\n486 nm', '#00FFFF', 3.0),
        (5, 2, 'H-gamma\n434 nm', '#0000FF', 4.0),
        (2, 1, 'Ly-alpha\n122 nm', '#8B00FF', 5.0),
    ]

    for n_up, n_down, label, color, x_pos in transitions_to_draw:
        E_up = energy_level(n_up)
        E_down = energy_level(n_down)

        # Draw transition arrow
        ax_levels.annotate('', xy=(x_pos, E_down + 0.3),
                          xytext=(x_pos, E_up - 0.3),
                          arrowprops=dict(arrowstyle='->', color=color, lw=2.5))

        # Draw wavy photon line
        t = np.linspace(0, 1, 50)
        y_wave = E_up + (E_down - E_up) * t
        x_wave = x_pos + 0.4 + 0.2 * np.sin(15 * np.pi * t)
        ax_levels.plot(x_wave, y_wave, color=color, linewidth=1.5, alpha=0.8)

        # Label
        ax_levels.text(x_pos + 0.8, (E_up + E_down)/2, label, fontsize=8,
                      color=color, va='center', ha='left')

    # Ionization continuum
    ax_levels.axhline(y=0, color='gray', linewidth=2, linestyle='--')
    ax_levels.fill_between([-0.5, 8], [0, 0], [2, 2], color='gray', alpha=0.15)
    ax_levels.text(3.5, 1, 'Continuum (ionized)', fontsize=10, ha='center',
                  style='italic', color=COLORS['void'])

    # TRD Formula box (more compact)
    formula_box = (
        r'$\mathbf{TRD\ Formula:}$' + '\n'
        r'$E_n = -\frac{m_e c^2 \alpha^2}{2n^2} = -\frac{13.6}{n^2}$ eV' + '\n'
        r'$\alpha = 1/137.036$ (from $G^*$)'
    )
    ax_levels.text(0.98, 0.98, formula_box, transform=ax_levels.transAxes,
                  fontsize=10, va='top', ha='right',
                  bbox=dict(boxstyle='round,pad=0.4', facecolor='#FFFFD0',
                           edgecolor=COLORS['highlight'], linewidth=1.5))

    ax_levels.set_ylabel('Energy (eV)', fontsize=12)
    ax_levels.set_title('Hydrogen Energy Levels\n(Bohr Model from TRD)',
                       fontsize=14, fontweight='bold')
    ax_levels.set_xticks([])
    ax_levels.spines['top'].set_visible(False)
    ax_levels.spines['right'].set_visible(False)
    ax_levels.spines['bottom'].set_visible(False)

    # =========================================================================
    # Panel 2: Emission Spectrum (Top Right)
    # =========================================================================
    ax_spectrum.set_xlim(350, 750)
    ax_spectrum.set_ylim(0, 1)
    ax_spectrum.set_facecolor('#1a1a2e')

    # Draw visible spectrum background
    wavelengths = np.linspace(380, 700, 500)
    for i, wl in enumerate(wavelengths[:-1]):
        color = wavelength_to_rgb(wl)
        ax_spectrum.axvspan(wl, wavelengths[i+1], alpha=0.3, color=color, linewidth=0)

    # Draw emission lines
    for name, data in BALMER_LINES.items():
        wl = data['wavelength']
        color = data['color']

        # Main emission line (bright)
        ax_spectrum.axvline(x=wl, color=color, linewidth=4, alpha=1.0)

        # Glow effect
        for offset in [1, 2, 3]:
            ax_spectrum.axvline(x=wl, color=color, linewidth=4 + offset*2,
                               alpha=0.3 - offset*0.08)

        # Label (staggered heights to avoid overlap)
        y_offset = 0.75 if name in ['H-alpha', 'H-gamma'] else 0.55
        ax_spectrum.text(wl, y_offset, f'{name}\n{wl:.0f} nm', fontsize=8,
                        color='white', ha='center', va='bottom',
                        fontweight='bold')

    # Add UV indicator
    ax_spectrum.annotate('', xy=(355, 0.25), xytext=(375, 0.25),
                        arrowprops=dict(arrowstyle='->', color='#8B00FF', lw=2))
    ax_spectrum.text(365, 0.32, 'UV', fontsize=8, color='#8B00FF',
                    ha='center', va='bottom')

    # Add IR indicator (moved left to stay in view)
    ax_spectrum.annotate('', xy=(740, 0.25), xytext=(700, 0.25),
                        arrowprops=dict(arrowstyle='->', color='#8B0000', lw=2))
    ax_spectrum.text(720, 0.32, 'IR', fontsize=8, color='#8B0000',
                    ha='center', va='bottom')

    ax_spectrum.set_xlabel('Wavelength (nm)', fontsize=12, color='white')
    ax_spectrum.set_title('Hydrogen Emission Spectrum (Balmer Series)',
                         fontsize=14, fontweight='bold', color='white')
    ax_spectrum.set_yticks([])
    ax_spectrum.tick_params(colors='white')
    for spine in ax_spectrum.spines.values():
        spine.set_color('white')

    # Wavelength formula
    ax_spectrum.text(0.02, 0.12,
                    r'$\frac{1}{\lambda} = R_H \left(\frac{1}{n_f^2} - \frac{1}{n_i^2}\right)$'
                    + '\n' + r'$R_H = \frac{m_e c \alpha^2}{2h}$',
                    transform=ax_spectrum.transAxes, fontsize=10, color='white',
                    va='bottom', ha='left',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='black', alpha=0.7))

    # =========================================================================
    # Panel 3: All Spectral Series (Middle) - Simplified
    # =========================================================================
    ax_series.set_xlim(50, 2200)
    ax_series.set_ylim(-0.3, 4.2)
    ax_series.set_facecolor('#F5F5F5')

    series_y = {'Lyman': 3.2, 'Balmer': 2.2, 'Paschen': 1.2, 'Brackett': 0.2}

    for series_name, series_data in SPECTRAL_SERIES.items():
        y = series_y[series_name]

        # Series label on left
        ax_series.text(120, y, f'{series_name} (n→{series_data["n_final"]})',
                      fontsize=9, va='center', ha='left', fontweight='bold')

        # Draw transition lines
        for idx, (n_up, n_down) in enumerate(series_data['transitions']):
            wl = wavelength_nm(n_up, n_down)

            if wl < 2200:
                color = wavelength_to_rgb(wl) if 380 <= wl <= 780 else series_data['color']

                # Line
                ax_series.plot([wl, wl], [y - 0.25, y + 0.25], color=color, linewidth=3)

                # Label wavelength below (only first 2 transitions per series)
                if idx < 2:
                    ax_series.text(wl, y - 0.35, f'{wl:.0f}', fontsize=7,
                                  ha='center', va='top', color=COLORS['void'])

        # Series limit
        limit_wl = wavelength_nm(1000, series_data['n_final'])
        if limit_wl < 2200:
            ax_series.plot([limit_wl, limit_wl], [y - 0.2, y + 0.2],
                          color='black', linewidth=1, linestyle='--')

    # Visible range shading
    ax_series.axvspan(380, 780, alpha=0.2, color='#FFFF00')
    ax_series.text(580, 3.9, 'Visible', fontsize=9, ha='center', style='italic')

    ax_series.set_xlabel('Wavelength (nm)', fontsize=11)
    ax_series.set_title('Hydrogen Spectral Series', fontsize=13, fontweight='bold')
    ax_series.set_yticks([])
    for spine in ['left', 'top', 'right']:
        ax_series.spines[spine].set_visible(False)

    # =========================================================================
    # Panel 4: TRD Derivation Chain (Bottom) - Cleaner layout
    # =========================================================================
    ax_derivation.set_xlim(0, 10)
    ax_derivation.set_ylim(0, 2.5)
    ax_derivation.axis('off')
    ax_derivation.set_facecolor(COLORS['background'])

    # Create derivation boxes - horizontal flow
    boxes = [
        (1.0, 1.3, 'TRD\nQuadratic', '#E8E8FF'),
        (3.0, 1.3, f'$G^*$={G_STAR:.3f}', '#FFE8E8'),
        (5.0, 1.3, r'$\alpha=\frac{1}{137}$', '#E8FFE8'),
        (7.0, 1.3, r'$E_n \propto \alpha^2$', '#FFFFE8'),
        (9.0, 1.3, 'Spectrum', '#E8FFFF'),
    ]

    for x, y, text, color in boxes:
        box = FancyBboxPatch((x-0.6, y-0.4), 1.2, 0.8,
                             boxstyle="round,pad=0.05,rounding_size=0.15",
                             facecolor=color, edgecolor='black', linewidth=1.5)
        ax_derivation.add_patch(box)
        ax_derivation.text(x, y, text, ha='center', va='center', fontsize=9,
                          fontweight='bold')

    # Arrows between boxes
    for i in range(len(boxes) - 1):
        ax_derivation.annotate('', xy=(boxes[i+1][0]-0.6, 1.3),
                              xytext=(boxes[i][0]+0.6, 1.3),
                              arrowprops=dict(arrowstyle='->', lw=2,
                                            color=COLORS['highlight']))

    # Key insight - single line below
    insight_text = (
        r'$\mathbf{Key:}$ $\alpha = 1/137.036$ is derived from $G^*$ (lemniscatic constant), '
        r'giving $E_n = -13.6/n^2$ eV'
    )
    ax_derivation.text(5, 0.5, insight_text, ha='center', va='center', fontsize=10,
                      bbox=dict(boxstyle='round,pad=0.4', facecolor='#FFFFD0',
                               edgecolor=COLORS['highlight'], linewidth=1.5))

    ax_derivation.set_title('TRD Derivation Chain', fontsize=12, fontweight='bold', y=0.95)

    # =========================================================================
    # Overall title
    # =========================================================================
    fig.suptitle('Atomic Spectra from Ternary Realization Dynamics',
                fontsize=16, fontweight='bold', y=0.97)

    # Adjust layout - more space between panels
    plt.subplots_adjust(left=0.06, right=0.96, top=0.93, bottom=0.03,
                       hspace=0.45, wspace=0.25)

    return fig


if __name__ == '__main__':
    fig = generate_atomic_spectra_trd()

    # Save to multiple locations
    output_paths = [
        Path(__file__).parent / 'fig_atomic_spectra_trd.png',
    ]

    for output_path in output_paths:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(output_path, dpi=150, bbox_inches='tight',
                   facecolor='white', edgecolor='none')
        print(f"Saved: {output_path}")

    plt.show()
