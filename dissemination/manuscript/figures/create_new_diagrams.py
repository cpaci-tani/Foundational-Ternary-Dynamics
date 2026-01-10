"""
Create PNG diagrams to replace ASCII art in chapters.
"""
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, Circle, FancyArrowPatch
import numpy as np

# Set style
plt.style.use('default')
plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['font.size'] = 11

def create_bell_sloop_diagram():
    """Create diagram comparing Bell's setup vs TRD sLoop."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    # Left panel: Bell's Setup
    ax1 = axes[0]
    ax1.set_xlim(0, 10)
    ax1.set_ylim(0, 8)
    ax1.set_aspect('equal')
    ax1.axis('off')
    ax1.set_title("Bell's Setup (implicit assumption)", fontsize=14, fontweight='bold', pad=20)

    # Particles
    ax1.add_patch(Circle((3, 5), 0.4, color='#3498db', ec='black', lw=2))
    ax1.text(3, 5, 'A', ha='center', va='center', fontsize=12, fontweight='bold', color='white')
    ax1.add_patch(Circle((7, 5), 0.4, color='#e74c3c', ec='black', lw=2))
    ax1.text(7, 5, 'B', ha='center', va='center', fontsize=12, fontweight='bold', color='white')

    # Hidden variable connection
    ax1.annotate('', xy=(6.5, 5), xytext=(3.5, 5),
                arrowprops=dict(arrowstyle='<->', color='#9b59b6', lw=2, ls='--'))
    ax1.text(5, 5.4, 'hidden λ', ha='center', va='bottom', fontsize=10, color='#9b59b6', style='italic')

    # Apparatuses (external boxes)
    ax1.add_patch(FancyBboxPatch((1.5, 1.5), 3, 1.5, boxstyle="round,pad=0.05",
                                  facecolor='#ecf0f1', edgecolor='#7f8c8d', lw=2))
    ax1.text(3, 2.25, 'Apparatus α\n(external)', ha='center', va='center', fontsize=10)

    ax1.add_patch(FancyBboxPatch((5.5, 1.5), 3, 1.5, boxstyle="round,pad=0.05",
                                  facecolor='#ecf0f1', edgecolor='#7f8c8d', lw=2))
    ax1.text(7, 2.25, 'Apparatus β\n(external)', ha='center', va='center', fontsize=10)

    # Arrows to apparatuses
    ax1.annotate('', xy=(3, 3), xytext=(3, 4.5),
                arrowprops=dict(arrowstyle='->', color='black', lw=1.5))
    ax1.annotate('', xy=(7, 3), xytext=(7, 4.5),
                arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Outcomes
    ax1.text(3, 0.8, 'Outcome a', ha='center', va='center', fontsize=10,
             bbox=dict(boxstyle='round', facecolor='#2ecc71', alpha=0.7))
    ax1.text(7, 0.8, 'Outcome b', ha='center', va='center', fontsize=10,
             bbox=dict(boxstyle='round', facecolor='#2ecc71', alpha=0.7))
    ax1.annotate('', xy=(3, 1.2), xytext=(3, 1.5),
                arrowprops=dict(arrowstyle='->', color='black', lw=1.5))
    ax1.annotate('', xy=(7, 1.2), xytext=(7, 1.5),
                arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Right panel: TRD sLoop
    ax2 = axes[1]
    ax2.set_xlim(0, 10)
    ax2.set_ylim(0, 8)
    ax2.set_aspect('equal')
    ax2.axis('off')
    ax2.set_title("TRD sLoop", fontsize=14, fontweight='bold', pad=20)

    # Shared substrate box
    ax2.add_patch(FancyBboxPatch((0.5, 0.5), 9, 7, boxstyle="round,pad=0.1",
                                  facecolor='#e8f4f8', edgecolor='#3498db', lw=3))
    ax2.text(5, 7.2, 'SHARED FLUX SUBSTRATE', ha='center', va='center',
             fontsize=12, fontweight='bold', color='#2980b9')

    # Apparatuses (inside the substrate)
    ax2.add_patch(FancyBboxPatch((1.2, 4), 2.6, 1.2, boxstyle="round,pad=0.05",
                                  facecolor='#d5dbdb', edgecolor='#566573', lw=2))
    ax2.text(2.5, 4.6, 'Apparatus α', ha='center', va='center', fontsize=10)

    ax2.add_patch(FancyBboxPatch((6.2, 4), 2.6, 1.2, boxstyle="round,pad=0.05",
                                  facecolor='#d5dbdb', edgecolor='#566573', lw=2))
    ax2.text(7.5, 4.6, 'Apparatus β', ha='center', va='center', fontsize=10)

    # Entangled pair (central)
    ax2.add_patch(Circle((5, 2.5), 0.5, color='#9b59b6', ec='black', lw=2))
    ax2.text(5, 2.5, 'ψ', ha='center', va='center', fontsize=14, fontweight='bold', color='white')
    ax2.text(5, 1.6, '(entangled pair)', ha='center', va='center', fontsize=9, style='italic')

    # Flux connections (bidirectional wavy lines represented as curved arrows)
    # Apparatus α to ψ
    ax2.annotate('', xy=(4.5, 2.7), xytext=(2.5, 4),
                arrowprops=dict(arrowstyle='<->', color='#3498db', lw=2,
                               connectionstyle='arc3,rad=0.2'))
    # Apparatus β to ψ
    ax2.annotate('', xy=(5.5, 2.7), xytext=(7.5, 4),
                arrowprops=dict(arrowstyle='<->', color='#3498db', lw=2,
                               connectionstyle='arc3,rad=-0.2'))
    # Between apparatuses
    ax2.annotate('', xy=(6.2, 4.6), xytext=(3.8, 4.6),
                arrowprops=dict(arrowstyle='<->', color='#3498db', lw=2,
                               connectionstyle='arc3,rad=-0.3'))

    # Flux labels
    ax2.text(3.2, 3.5, 'flux', fontsize=9, color='#3498db', rotation=45)
    ax2.text(6.5, 3.5, 'flux', fontsize=9, color='#3498db', rotation=-45)
    ax2.text(5, 5.3, 'flux', fontsize=9, color='#3498db')

    plt.tight_layout()
    plt.savefig('ch02/fig-bell-sloop-comparison.png', dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.savefig('ch02/fig-bell-sloop-comparison.svg', bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.close()
    print("Created: ch02/fig-bell-sloop-comparison.png")

def create_hydrogen_diagram():
    """Create diagram of hydrogen atom structure."""
    fig, ax = plt.subplots(figsize=(6, 6))
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')

    # Title
    ax.set_title("Hydrogen: The First Atom", fontsize=14, fontweight='bold', pad=15)

    # Proton (nucleus)
    proton = Circle((0, 0.5), 0.4, color='#e74c3c', ec='black', lw=2, zorder=10)
    ax.add_patch(proton)
    ax.text(0, 0.5, 'p⁺', ha='center', va='center', fontsize=14, fontweight='bold', color='white', zorder=11)

    # Electron orbit (dashed circle)
    orbit = Circle((0, 0), 1.5, fill=False, ec='#95a5a6', ls='--', lw=1.5)
    ax.add_patch(orbit)

    # Electron
    electron = Circle((1.5, 0), 0.25, color='#3498db', ec='black', lw=2, zorder=10)
    ax.add_patch(electron)
    ax.text(1.5, 0, 'e⁻', ha='center', va='center', fontsize=10, fontweight='bold', color='white', zorder=11)

    # Connection line
    ax.plot([0, 0], [0.1, 0.5-0.4], 'k-', lw=1.5, alpha=0.5)

    # Labels
    ax.text(0, -2.2, 'One proton, one electron', ha='center', va='center', fontsize=11)
    ax.text(0, -2.7, 'The building block of stars', ha='center', va='center', fontsize=10, style='italic', color='#7f8c8d')

    # Shell label
    ax.text(2.0, 1.2, '1s¹', ha='center', va='center', fontsize=10, color='#7f8c8d')

    plt.tight_layout()
    plt.savefig('ch03/fig-hydrogen-atom.png', dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.savefig('ch03/fig-hydrogen-atom.svg', bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.close()
    print("Created: ch03/fig-hydrogen-atom.png")

def create_helium_diagram():
    """Create diagram of helium atom structure."""
    fig, ax = plt.subplots(figsize=(6, 6))
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')

    # Title
    ax.set_title("Helium: The Inert Witness", fontsize=14, fontweight='bold', pad=15)

    # Nucleus (2 protons, 2 neutrons)
    # Protons
    ax.add_patch(Circle((-0.2, 0.6), 0.3, color='#e74c3c', ec='black', lw=2, zorder=10))
    ax.text(-0.2, 0.6, 'p⁺', ha='center', va='center', fontsize=10, fontweight='bold', color='white', zorder=11)
    ax.add_patch(Circle((0.2, 0.3), 0.3, color='#e74c3c', ec='black', lw=2, zorder=10))
    ax.text(0.2, 0.3, 'p⁺', ha='center', va='center', fontsize=10, fontweight='bold', color='white', zorder=11)

    # Neutrons
    ax.add_patch(Circle((0.25, 0.7), 0.25, color='#95a5a6', ec='black', lw=2, zorder=9))
    ax.text(0.25, 0.7, 'n', ha='center', va='center', fontsize=9, fontweight='bold', color='white', zorder=11)
    ax.add_patch(Circle((-0.15, 0.25), 0.25, color='#95a5a6', ec='black', lw=2, zorder=9))
    ax.text(-0.15, 0.25, 'n', ha='center', va='center', fontsize=9, fontweight='bold', color='white', zorder=11)

    # Electron orbit (complete shell)
    orbit = Circle((0, 0.45), 1.5, fill=False, ec='#27ae60', ls='-', lw=2)
    ax.add_patch(orbit)

    # Electrons (opposite sides to show filled shell)
    ax.add_patch(Circle((-1.5, 0.45), 0.25, color='#3498db', ec='black', lw=2, zorder=10))
    ax.text(-1.5, 0.45, 'e⁻', ha='center', va='center', fontsize=9, fontweight='bold', color='white', zorder=11)
    ax.add_patch(Circle((1.5, 0.45), 0.25, color='#3498db', ec='black', lw=2, zorder=10))
    ax.text(1.5, 0.45, 'e⁻', ha='center', va='center', fontsize=9, fontweight='bold', color='white', zorder=11)

    # Labels
    ax.text(0, -1.5, 'Two protons, two neutrons, two electrons', ha='center', va='center', fontsize=11)
    ax.text(0, -2.0, 'Full 1s shell → chemically inert', ha='center', va='center', fontsize=10, color='#27ae60')

    # Shell label
    ax.text(2.0, 1.5, '1s²', ha='center', va='center', fontsize=10, color='#27ae60', fontweight='bold')
    ax.text(2.0, 1.1, '(complete)', ha='center', va='center', fontsize=9, color='#7f8c8d')

    plt.tight_layout()
    plt.savefig('ch03/fig-helium-atom.png', dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.savefig('ch03/fig-helium-atom.svg', bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.close()
    print("Created: ch03/fig-helium-atom.png")

if __name__ == '__main__':
    import os
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    # Create directories if needed
    os.makedirs('ch02', exist_ok=True)
    os.makedirs('ch03', exist_ok=True)

    create_bell_sloop_diagram()
    create_hydrogen_diagram()
    create_helium_diagram()
    print("\nAll diagrams created successfully!")
