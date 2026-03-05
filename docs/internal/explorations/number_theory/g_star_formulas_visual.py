#!/usr/bin/env python3
"""
Visualization of the remarkable G*-based formulas for physical constants.
"""

import numpy as np
import matplotlib.pyplot as plt
from math import gamma

# Constants
G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)

# Set up the figure
fig = plt.figure(figsize=(16, 12))
fig.patch.set_facecolor('black')

# Create grid
gs = fig.add_gridspec(3, 3, hspace=0.3, wspace=0.3)

# =============================================================================
# Panel 1: The accuracy hierarchy
# =============================================================================
ax1 = fig.add_subplot(gs[0, :])
ax1.set_facecolor('black')

formulas = [
    ("m_p/m_e = 6pi^5", 0.002),
    ("sin^2(theta_W) = G*/12.8", 0.027),
    ("m_tau/m_e = (2G*)^7/73", 0.077),
    ("m_mu/m_e = (2piG*)^2/1.67", 0.081),
    ("alpha_s = G*/(8pi)", 0.151),
    ("d_min = G*^2/32", 0.19),
    ("|V_cb| = G*/70", 0.158),
]

names = [f[0] for f in formulas]
errors = [f[1] for f in formulas]

colors = plt.cm.plasma(np.linspace(0.2, 0.9, len(formulas)))
bars = ax1.barh(range(len(formulas)), errors, color=colors, edgecolor='white', linewidth=0.5)

ax1.set_yticks(range(len(formulas)))
ax1.set_yticklabels(names, color='white', fontsize=11, family='monospace')
ax1.set_xlabel('Error (%)', color='white', fontsize=12)
ax1.set_title('G*-Based Formulas: Accuracy Hierarchy', color='gold', fontsize=16, fontweight='bold')
ax1.tick_params(colors='white')
ax1.set_xlim(0, 0.25)
ax1.axvline(x=0.1, color='gray', linestyle='--', alpha=0.5, linewidth=1)
ax1.text(0.1, 6.5, '0.1%', color='gray', fontsize=9)

for spine in ax1.spines.values():
    spine.set_color('white')

# Add error values on bars
for i, (bar, error) in enumerate(zip(bars, errors)):
    ax1.text(error + 0.005, i, f'{error:.3f}%', va='center', color='white', fontsize=9)

# =============================================================================
# Panel 2: The proton mass pi^5 relationship
# =============================================================================
ax2 = fig.add_subplot(gs[1, 0])
ax2.set_facecolor('black')

# Show pi powers
n_powers = np.arange(1, 7)
pi_powers = np.pi ** n_powers

ax2.semilogy(n_powers, pi_powers, 'o-', color='cyan', markersize=10, linewidth=2)
ax2.axhline(y=6 * np.pi**5 / 6, color='gold', linestyle='--', alpha=0.7)
ax2.axhline(y=1836.15 / 6, color='red', linestyle=':', alpha=0.7)

ax2.set_xlabel('n', color='white', fontsize=11)
ax2.set_ylabel('pi^n', color='white', fontsize=11)
ax2.set_title('Powers of pi\nm_p/m_e = 6*pi^5', color='cyan', fontsize=12)
ax2.tick_params(colors='white')
ax2.text(5.1, np.pi**5, f'pi^5 = {np.pi**5:.1f}', color='gold', fontsize=10)

for spine in ax2.spines.values():
    spine.set_color('white')

# =============================================================================
# Panel 3: G* in various formulas
# =============================================================================
ax3 = fig.add_subplot(gs[1, 1])
ax3.set_facecolor('black')

# Create a "web" showing G* connections
angles = np.linspace(0, 2*np.pi, 7, endpoint=False)
labels = ['alpha_s', 'theta_W', 'm_tau', 'm_mu', 'd_min', 'V_cb']
values = [G_STAR/(8*np.pi), G_STAR/12.8, (2*G_STAR)**7/73/1000, (2*np.pi*G_STAR)**2/1.67/100, G_STAR**2/32, G_STAR/70]

# Normalize for visualization
max_val = max(values)
norm_values = [v/max_val for v in values]

# Plot as polar-like radar
for i, (angle, val, label) in enumerate(zip(angles, norm_values, labels)):
    x = val * np.cos(angle)
    y = val * np.sin(angle)
    ax3.plot([0, x], [0, y], 'o-', color=colors[i+1], linewidth=2, markersize=8)
    ax3.text(1.2*np.cos(angle), 1.2*np.sin(angle), label, color='white',
             ha='center', va='center', fontsize=9)

# Central G*
circle = plt.Circle((0, 0), 0.15, color='gold', zorder=10)
ax3.add_patch(circle)
ax3.text(0, 0, 'G*', ha='center', va='center', color='black', fontsize=12, fontweight='bold')

ax3.set_xlim(-1.5, 1.5)
ax3.set_ylim(-1.5, 1.5)
ax3.set_aspect('equal')
ax3.axis('off')
ax3.set_title('G* Connection Web', color='gold', fontsize=12)

# =============================================================================
# Panel 4: The G*/pi relationship
# =============================================================================
ax4 = fig.add_subplot(gs[1, 2])
ax4.set_facecolor('black')

# G*/pi plot
x = np.linspace(0.8, 1.1, 100)
y_circle = np.sqrt(1 - (x-1)**2 * 25)  # Scaled circle

ax4.axhline(y=G_STAR/np.pi, color='gold', linewidth=2, label=f'G*/pi = {G_STAR/np.pi:.4f}')
ax4.axhline(y=15/16, color='cyan', linewidth=1, linestyle='--', label='15/16 = 0.9375')
ax4.axhline(y=np.sqrt(0.887), color='magenta', linewidth=1, linestyle=':', label='sqrt(0.887)')

ax4.set_xlim(0, 1)
ax4.set_ylim(0.9, 0.97)
ax4.set_xlabel('Reference', color='white')
ax4.set_ylabel('Ratio value', color='white')
ax4.set_title('G*/pi ~ 0.9418\nG* ~ pi*sqrt(0.887)', color='gold', fontsize=12)
ax4.legend(loc='lower right', fontsize=9, facecolor='black', labelcolor='white')
ax4.tick_params(colors='white')

for spine in ax4.spines.values():
    spine.set_color('white')

# =============================================================================
# Panel 5: The consciousness angle
# =============================================================================
ax5 = fig.add_subplot(gs[2, 0], projection='polar')
ax5.set_facecolor('black')

k_c = 4 / G_STAR
theta_consciousness = np.arctan(np.sqrt(k_c - 1))
theta_deg = np.degrees(theta_consciousness)

# Draw the consciousness angle
theta = np.linspace(0, theta_consciousness, 100)
r = np.ones_like(theta)
ax5.plot(theta, r, color='gold', linewidth=3)
ax5.plot([0, theta_consciousness], [0, 1], color='gold', linewidth=2)

# Mark special angles
ax5.axvline(x=np.pi/6, color='cyan', linestyle='--', alpha=0.7, linewidth=1)
ax5.axvline(x=np.pi/3, color='magenta', linestyle='--', alpha=0.7, linewidth=1)

ax5.set_ylim(0, 1.2)
ax5.set_theta_zero_location('E')
ax5.set_theta_direction(-1)
ax5.set_rticks([])
ax5.tick_params(colors='white', labelsize=8)
ax5.set_title(f'Consciousness Angle\ntheta = {theta_deg:.2f} deg (close to 30)',
              color='gold', fontsize=11, pad=15)

# =============================================================================
# Panel 6: d_min = G*^2/32 visualization
# =============================================================================
ax6 = fig.add_subplot(gs[2, 1])
ax6.set_facecolor('black')

# Draw the lemniscate and highlight d_min
FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

t = np.linspace(0, 2*np.pi, 1000)
x = sum(X_AMPS[j] * np.cos(FREQS[j] * t) for j in range(5))
y = sum(Y_AMPS[j] * np.sin(FREQS[j] * t) for j in range(5))

ax6.plot(x, y, color='cyan', linewidth=1.5, alpha=0.8)
ax6.plot(0, 0, 'o', color='white', markersize=8)

# Find and mark minimum distance point
distances = np.sqrt(x**2 + y**2)
min_idx = np.argmin(distances)
d_min = distances[min_idx]

ax6.plot([0, x[min_idx]], [0, y[min_idx]], 'r-', linewidth=2)
ax6.plot(x[min_idx], y[min_idx], 'ro', markersize=8)

# Draw circle at d_min
theta_circle = np.linspace(0, 2*np.pi, 100)
ax6.plot(d_min * np.cos(theta_circle), d_min * np.sin(theta_circle),
         'r--', alpha=0.5, linewidth=1)

ax6.set_xlim(-2.5, 2.5)
ax6.set_ylim(-1.8, 1.8)
ax6.set_aspect('equal')
ax6.axis('off')
ax6.set_title(f'd_min = G*^2/32 = {G_STAR**2/32:.4f}\nThe irreducible gap',
              color='red', fontsize=11)

# =============================================================================
# Panel 7: Summary equation
# =============================================================================
ax7 = fig.add_subplot(gs[2, 2])
ax7.set_facecolor('black')
ax7.axis('off')

summary_text = """
    MASTER RELATIONSHIPS

    Proton Mass:
    m_p/m_e = 6 * pi^5

    Strong Coupling:
    alpha_s = G* / (8*pi)

    Weinberg Angle:
    sin^2(theta_W) = G* / 12.8

    Bridge Equation:
    k_c * c_cusp * G* = 1

    Center Avoidance:
    d_min = G*^2 / 32

    G* ~ pi * sqrt(0.887)
"""

ax7.text(0.5, 0.5, summary_text, transform=ax7.transAxes,
         color='white', fontsize=11, family='monospace',
         ha='center', va='center',
         bbox=dict(boxstyle='round', facecolor='#222222', edgecolor='gold', linewidth=2))

# Main title
fig.suptitle('G* = 2.9587: The Lemniscatic Constant and Physics',
             color='gold', fontsize=18, fontweight='bold', y=0.98)

plt.savefig('g_star_formulas_summary.png', dpi=150, bbox_inches='tight',
            facecolor='black', edgecolor='none')
plt.close()

print("Saved: g_star_formulas_summary.png")

# =============================================================================
# SECOND FIGURE: The pi-G* interplay
# =============================================================================

fig2, axes = plt.subplots(2, 2, figsize=(14, 12))
fig2.patch.set_facecolor('black')

# Panel A: Error comparison
ax = axes[0, 0]
ax.set_facecolor('black')

# Compare different formula types
formula_types = {
    'Pure pi': [('6*pi^5 (m_p/m_e)', 0.002)],
    'G*/pi': [('G*/(8*pi) (alpha_s)', 0.151), ('G*/12.8 (theta_W)', 0.027)],
    'Powers of G*': [('(2G*)^7/73 (tau)', 0.077), ('G*^2/32 (d_min)', 0.19)],
    'G*-pi combo': [('(2*pi*G*)^2/1.67 (mu)', 0.081)],
}

colors_type = {'Pure pi': 'cyan', 'G*/pi': 'gold', 'Powers of G*': 'magenta', 'G*-pi combo': 'lime'}

y_pos = 0
for typ, formulas in formula_types.items():
    for name, error in formulas:
        ax.barh(y_pos, error, color=colors_type[typ], edgecolor='white', height=0.6)
        ax.text(error + 0.01, y_pos, f'{error:.3f}%', color='white', va='center', fontsize=10)
        ax.text(-0.01, y_pos, name, color='white', va='center', ha='right', fontsize=9)
        y_pos += 1

ax.set_xlim(0, 0.25)
ax.set_ylim(-0.5, y_pos - 0.5)
ax.set_xlabel('Error (%)', color='white')
ax.set_title('Formula Types by Accuracy', color='white', fontsize=14)
ax.tick_params(colors='white')
ax.set_yticks([])

# Legend
from matplotlib.patches import Patch
legend_elements = [Patch(facecolor=c, label=t) for t, c in colors_type.items()]
ax.legend(handles=legend_elements, loc='lower right', fontsize=9, facecolor='black', labelcolor='white')

for spine in ax.spines.values():
    spine.set_color('white')

# Panel B: The G* spectrum
ax = axes[0, 1]
ax.set_facecolor('black')

powers = np.arange(-3, 9)
g_powers = G_STAR ** powers

ax.semilogy(powers, g_powers, 'o-', color='gold', markersize=10, linewidth=2)

# Mark significant values
significant = {
    7: ('G*^7 ~ m_p/m_e', 'red'),
    2: ('G*^2 ~ 8.75', 'cyan'),
    1: ('G* ~ 2.96', 'lime'),
    0: ('1', 'white'),
}

for p, (label, color) in significant.items():
    ax.plot(p, G_STAR**p, 'o', color=color, markersize=15, zorder=10)
    ax.annotate(label, (p, G_STAR**p), xytext=(5, 10), textcoords='offset points',
                color=color, fontsize=9)

ax.set_xlabel('Power n', color='white', fontsize=12)
ax.set_ylabel('G*^n', color='white', fontsize=12)
ax.set_title('Powers of G*', color='gold', fontsize=14)
ax.tick_params(colors='white')
ax.grid(True, alpha=0.2)

for spine in ax.spines.values():
    spine.set_color('white')

# Panel C: Physical constants on log scale
ax = axes[1, 0]
ax.set_facecolor('black')

constants = {
    'alpha_em': 1/137.036,
    'alpha_s': 0.1179,
    'sin^2(tw)': 0.2312,
    'm_mu/m_e / 1000': 0.2068,
    'm_tau/m_e / 10000': 0.3477,
    'm_p/m_e / 10000': 0.1836,
}

predictions = {
    'G*/(8*pi)': G_STAR/(8*np.pi),
    'G*/25': G_STAR/25,
    'G*/12.8': G_STAR/12.8,
    '(2*pi*G*)^2/1670': (2*np.pi*G_STAR)**2/1670,
    '(2G*)^7/730000': (2*G_STAR)**7/730000,
    '6*pi^5/10000': 6*np.pi**5/10000,
}

y_positions = np.arange(len(constants))
measured = list(constants.values())
predicted = list(predictions.values())

ax.barh(y_positions - 0.2, measured, height=0.35, color='white', alpha=0.7, label='Measured')
ax.barh(y_positions + 0.2, predicted, height=0.35, color='gold', alpha=0.7, label='G*-based')

ax.set_yticks(y_positions)
ax.set_yticklabels(list(constants.keys()), color='white', fontsize=10)
ax.set_xlabel('Value (normalized)', color='white')
ax.set_title('Measured vs G*-Predicted', color='white', fontsize=14)
ax.legend(loc='lower right', fontsize=10, facecolor='black', labelcolor='white')
ax.tick_params(colors='white')

for spine in ax.spines.values():
    spine.set_color('white')

# Panel D: The fundamental relationship
ax = axes[1, 1]
ax.set_facecolor('black')
ax.axis('off')

text = """
           THE DEEP STRUCTURE


    G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)

       = 2.9586751192...

       ~ pi * sqrt(0.887)


    Key Insight:

    Both G* (elliptic) and pi (circular)
    appear to be equally fundamental
    in encoding physical constants.


    QCD (proton mass): Pure pi
       m_p/m_e = 6 * pi^5

    QCD (coupling): G*/pi
       alpha_s = G* / (8*pi)

    Electroweak: G*/pi
       sin^2(tw) = G* / (4*pi + eps)

    Leptons: Powers of (pi*G*)
       m_mu ~ (pi*G*)^2
       m_tau ~ G*^7
"""

ax.text(0.5, 0.5, text, transform=ax.transAxes,
        color='white', fontsize=12, family='monospace',
        ha='center', va='center',
        bbox=dict(boxstyle='round', facecolor='#111111', edgecolor='gold', linewidth=2))

fig2.suptitle('The pi-G* Duality in Physics', color='gold', fontsize=18, fontweight='bold')

plt.savefig('pi_gstar_duality.png', dpi=150, bbox_inches='tight',
            facecolor='black', edgecolor='none')
plt.close()

print("Saved: pi_gstar_duality.png")
print("\nDone!")
