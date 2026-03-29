"""
Three Visualizations of the Fermat Prime Split

1. The Prime Race: Chebyshev's bias between primes ≡ 1 vs 3 (mod 4)
2. Gaussian Primes in the Complex Plane: inert vs split
3. The Paraboloid of Primes (Blender script, saved separately)

Connection to FTD: The Fermat two-square theorem classifies primes
by their splitting behavior in Z[i] = End(E), the endomorphism ring
of the CM curve E: y^2 = x^3 - x. Inert primes (≡ 3 mod 4) are the
confinement-sector integers {3, 7, 47}. Split primes (≡ 1 mod 4)
are the electromagnetic-sector integers {13, 137}.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import os

# =====================================================
# UTILITY: Prime sieve
# =====================================================

def sieve(n):
    """Sieve of Eratosthenes up to n."""
    is_prime = [False, False] + [True] * (n - 1)
    for i in range(2, int(n**0.5) + 1):
        if is_prime[i]:
            for j in range(i*i, n + 1, i):
                is_prime[j] = False
    return [i for i in range(2, n + 1) if is_prime[i]]

def is_prime(n):
    """Check if n is prime."""
    if n < 2:
        return False
    if n < 4:
        return True
    if n % 2 == 0 or n % 3 == 0:
        return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

def is_gaussian_prime(a, b):
    """
    Check if a + bi is a Gaussian prime.
    Cases:
    1. If b == 0: |a| is a Gaussian prime iff |a| is a standard prime ≡ 3 (mod 4)
    2. If a == 0: |b| is a Gaussian prime iff |b| is a standard prime ≡ 3 (mod 4)
    3. If a != 0 and b != 0: a + bi is a Gaussian prime iff a^2 + b^2 is a standard prime
    """
    if a == 0 and b == 0:
        return False
    if b == 0:
        return is_prime(abs(a)) and abs(a) % 4 == 3
    if a == 0:
        return is_prime(abs(b)) and abs(b) % 4 == 3
    return is_prime(a * a + b * b)


# =====================================================
# VISUALIZATION 1: The Prime Race
# =====================================================

def viz_prime_race(limit=10000):
    """
    Cumulative count of primes ≡ 1 (mod 4) vs ≡ 3 (mod 4).
    Demonstrates Chebyshev's bias.
    """
    primes = sieve(limit)

    xs = []
    count_1 = []
    count_3 = []
    c1, c3 = 0, 0

    for p in primes:
        if p == 2:
            continue
        if p % 4 == 1:
            c1 += 1
        else:
            c3 += 1
        xs.append(p)
        count_1.append(c1)
        count_3.append(c3)

    fig, ax = plt.subplots(figsize=(14, 7))

    ax.plot(xs, count_1, color='#2196F3', linewidth=1.2, label=r'$p \equiv 1 \;(\mathrm{mod}\;4)$ (split in $\mathbb{Z}[i]$)')
    ax.plot(xs, count_3, color='#FF9800', linewidth=1.2, label=r'$p \equiv 3 \;(\mathrm{mod}\;4)$ (inert in $\mathbb{Z}[i]$)')

    # Mark FTD framework primes
    ftd_primes = {
        3: ('$N_c=3$', '#FF5722'),
        7: ('$b_3=7$', '#FF5722'),
        13: ('$N_{eff}=13$', '#1565C0'),
        47: ('$D=47$', '#FF5722'),
        137: ('$1/\\alpha \\approx 137$', '#1565C0'),
    }

    for p, (label, color) in ftd_primes.items():
        idx = next((i for i, x in enumerate(xs) if x >= p), None)
        if idx is not None:
            # Find which line this prime is on
            if p % 4 == 1:
                y_val = count_1[idx]
            else:
                y_val = count_3[idx]
            ax.annotate(label, xy=(p, y_val), fontsize=9, fontweight='bold',
                       color=color, ha='left', va='bottom',
                       xytext=(10, 10), textcoords='offset points',
                       arrowprops=dict(arrowstyle='->', color=color, lw=1.2))

    # Shade the regions where 3 mod 4 leads (Chebyshev's bias)
    diff = np.array(count_3) - np.array(count_1)
    bias_regions = diff > 0

    for i in range(len(xs) - 1):
        if bias_regions[i]:
            ax.axvspan(xs[i], xs[i+1], alpha=0.03, color='#FF9800')

    ax.set_xlabel('Prime $p$', fontsize=13)
    ax.set_ylabel('Cumulative count', fontsize=13)
    ax.set_title("The Prime Race: Chebyshev's Bias and the Fermat Split", fontsize=15, fontweight='bold')
    ax.legend(fontsize=11, loc='upper left')
    ax.grid(True, alpha=0.3)

    # Add text box with FTD context
    textstr = (r'$\mathbf{Fermat\ Split:}$' + '\n'
               r'Inert ($3\,\mathrm{mod}\, 4$): $\{3, 7, 47\}$ — confinement' + '\n'
               r'Split ($1\,\mathrm{mod}\, 4$): $\{13, 137\}$ — electromagnetic')
    props = dict(boxstyle='round,pad=0.5', facecolor='#f5f5f5', alpha=0.9, edgecolor='#999')
    ax.text(0.98, 0.35, textstr, transform=ax.transAxes, fontsize=9,
            verticalalignment='top', horizontalalignment='right', bbox=props)

    plt.tight_layout()
    return fig


# =====================================================
# VISUALIZATION 2: Gaussian Primes in the Complex Plane
# =====================================================

def viz_gaussian_primes(bound=50):
    """
    Scatter plot of Gaussian primes, colored by Fermat type.
    Red squares: axis primes (inert, p ≡ 3 mod 4)
    Blue dots: quadrant primes (split, a^2+b^2 = p ≡ 1 mod 4)
    """
    inert_a, inert_b = [], []
    split_a, split_b = [], []

    for a in range(-bound, bound + 1):
        for b in range(-bound, bound + 1):
            if not is_gaussian_prime(a, b):
                continue

            if a == 0 or b == 0:
                # Axis prime: inert (p ≡ 3 mod 4)
                inert_a.append(a)
                inert_b.append(b)
            else:
                # Quadrant prime: split (a^2 + b^2 is prime ≡ 1 mod 4, or = 2)
                split_a.append(a)
                split_b.append(b)

    fig, ax = plt.subplots(figsize=(12, 12))

    # Plot split primes (small blue dots)
    ax.scatter(split_a, split_b, s=4, c='#2196F3', alpha=0.7, zorder=2,
              label=f'Split: $a^2+b^2 = p \\equiv 1$ (mod 4) [{len(split_a)} points]')

    # Plot inert primes (red squares, slightly larger)
    ax.scatter(inert_a, inert_b, s=25, c='#F44336', marker='s', alpha=0.9, zorder=3,
              label=f'Inert: $p \\equiv 3$ (mod 4), on axes [{len(inert_a)} points]')

    # Mark FTD framework primes on axes
    ftd_axis_primes = [3, 7, 47]
    for p in ftd_axis_primes:
        if p <= bound:
            for sign in [1, -1]:
                ax.annotate(f'{p}', xy=(sign * p, 0), fontsize=7,
                           color='#B71C1C', fontweight='bold',
                           ha='center', va='bottom' if sign > 0 else 'top',
                           xytext=(0, 8 * sign), textcoords='offset points')

    # Mark 13 = 2^2 + 3^2 in the quadrant
    for a, b in [(2, 3), (3, 2), (-2, 3), (-3, 2), (2, -3), (3, -2), (-2, -3), (-3, -2)]:
        if a == 2 and b == 3:
            ax.annotate('$13=2^2\\!+\\!3^2$', xy=(a, b), fontsize=7,
                       color='#0D47A1', fontweight='bold',
                       xytext=(8, 8), textcoords='offset points',
                       arrowprops=dict(arrowstyle='->', color='#0D47A1', lw=0.8))

    # Circle at radius sqrt(5) — the knight's move shell
    theta = np.linspace(0, 2 * np.pi, 200)
    ax.plot(np.sqrt(5) * np.cos(theta), np.sqrt(5) * np.sin(theta),
            'g--', alpha=0.4, linewidth=1, label=r'$r = \sqrt{5}$ (knight\'s move shell)')

    ax.set_xlim(-bound - 2, bound + 2)
    ax.set_ylim(-bound - 2, bound + 2)
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.15)
    ax.axhline(y=0, color='k', linewidth=0.5)
    ax.axvline(x=0, color='k', linewidth=0.5)
    ax.set_xlabel('Re$(z)$', fontsize=13)
    ax.set_ylabel('Im$(z)$', fontsize=13)
    ax.set_title('Gaussian Primes: The Fermat Split in $\\mathbb{Z}[i]$', fontsize=15, fontweight='bold')
    ax.legend(fontsize=10, loc='upper right')

    # Symmetry annotation
    ax.text(0.02, 0.02, 'Eight-fold symmetry from\n$\\mathbb{Z}/4\\mathbb{Z} = \\mathrm{Aut}(E)$',
            transform=ax.transAxes, fontsize=9, style='italic', color='#666',
            verticalalignment='bottom')

    plt.tight_layout()
    return fig


# =====================================================
# VISUALIZATION 3: Blender Script (saved to file)
# =====================================================

BLENDER_SCRIPT = '''"""
Paraboloid of Primes — Blender Python Script

Run inside Blender: Scripting workspace > Open > Run Script
Generates a 3D paraboloid of Gaussian primes where Z = a^2 + b^2 = p.

The discrete rings correspond to prime orbits on the paraboloid.
"""

import bpy
import math

def is_prime(n):
    if n < 2:
        return False
    if n < 4:
        return True
    if n % 2 == 0 or n % 3 == 0:
        return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

# Clear existing mesh objects
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

# Parameters
BOUND = 30
SPHERE_RADIUS = 0.15
SCALE_Z = 0.01  # Scale down Z to keep paraboloid manageable

# Create glowing emission material
mat = bpy.data.materials.new(name="PrimeGlow")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links

# Clear default nodes
for node in nodes:
    nodes.remove(node)

# Add emission shader
emission = nodes.new(type='ShaderNodeEmission')
emission.inputs['Color'].default_value = (0.2, 0.6, 1.0, 1.0)  # Blue glow
emission.inputs['Strength'].default_value = 3.0

# Add output
output = nodes.new(type='ShaderNodeOutputMaterial')
links.new(emission.outputs['Emission'], output.inputs['Surface'])

# Create a different material for inert axis primes
mat_inert = bpy.data.materials.new(name="InertGlow")
mat_inert.use_nodes = True
nodes_i = mat_inert.node_tree.nodes
links_i = mat_inert.node_tree.links

for node in nodes_i:
    nodes_i.remove(node)

emission_i = nodes_i.new(type='ShaderNodeEmission')
emission_i.inputs['Color'].default_value = (1.0, 0.3, 0.2, 1.0)  # Red glow
emission_i.inputs['Strength'].default_value = 3.0

output_i = nodes_i.new(type='ShaderNodeOutputMaterial')
links_i.new(emission_i.outputs['Emission'], output_i.inputs['Surface'])

# Generate primes on the paraboloid
count = 0
for a in range(-BOUND, BOUND + 1):
    for b in range(-BOUND, BOUND + 1):
        norm2 = a * a + b * b
        if norm2 == 0:
            continue

        # Check if this is a Gaussian prime
        if a == 0 or b == 0:
            # Axis: check if |a| or |b| is prime ≡ 3 mod 4
            val = abs(a) if b == 0 else abs(b)
            if is_prime(val) and val % 4 == 3:
                bpy.ops.mesh.primitive_uv_sphere_add(
                    radius=SPHERE_RADIUS * 1.5,
                    location=(a, b, val * SCALE_Z),
                    segments=12, ring_count=8
                )
                obj = bpy.context.active_object
                obj.name = f"Inert_p{val}_({a},{b})"
                obj.data.materials.append(mat_inert)
                count += 1
        else:
            # Quadrant: check if a^2 + b^2 is prime
            if is_prime(norm2):
                bpy.ops.mesh.primitive_uv_sphere_add(
                    radius=SPHERE_RADIUS,
                    location=(a, b, norm2 * SCALE_Z),
                    segments=8, ring_count=6
                )
                obj = bpy.context.active_object
                obj.name = f"Split_p{norm2}_({a},{b})"
                obj.data.materials.append(mat)
                count += 1

print(f"Generated {count} Gaussian prime spheres on the paraboloid.")
print(f"Blue (split, quadrant): p ≡ 1 mod 4, Z = a^2 + b^2")
print(f"Red (inert, axis): p ≡ 3 mod 4, on coordinate axes")

# Set viewport shading to Material Preview for glow visibility
for area in bpy.context.screen.areas:
    if area.type == 'VIEW_3D':
        for space in area.spaces:
            if space.type == 'VIEW_3D':
                space.shading.type = 'MATERIAL'

# Add a camera looking at the paraboloid
bpy.ops.object.camera_add(location=(40, -40, 20))
cam = bpy.context.active_object
cam.rotation_euler = (math.radians(60), 0, math.radians(45))
bpy.context.scene.camera = cam

# Set dark background
bpy.context.scene.world.use_nodes = True
bg = bpy.context.scene.world.node_tree.nodes['Background']
bg.inputs['Color'].default_value = (0.01, 0.01, 0.02, 1.0)
bg.inputs['Strength'].default_value = 0.5
'''


def save_blender_script():
    """Save the Blender script to a separate file."""
    script_path = os.path.join(os.path.dirname(__file__), 'blender_paraboloid_primes.py')
    with open(script_path, 'w', encoding='utf-8') as f:
        f.write(BLENDER_SCRIPT)
    print(f"Blender script saved to: {script_path}")
    return script_path


# =====================================================
# MAIN
# =====================================================

if __name__ == '__main__':
    output_dir = os.path.dirname(os.path.abspath(__file__))

    print("Generating Visualization 1: The Prime Race...")
    fig1 = viz_prime_race(10000)
    path1 = os.path.join(output_dir, 'fermat_prime_race.png')
    fig1.savefig(path1, dpi=200, bbox_inches='tight')
    print(f"  Saved to {path1}")
    plt.close(fig1)

    print("Generating Visualization 2: Gaussian Primes...")
    fig2 = viz_gaussian_primes(50)
    path2 = os.path.join(output_dir, 'fermat_gaussian_primes.png')
    fig2.savefig(path2, dpi=200, bbox_inches='tight')
    print(f"  Saved to {path2}")
    plt.close(fig2)

    print("Saving Visualization 3: Blender Script...")
    path3 = save_blender_script()

    print()
    print("All visualizations complete.")
    print(f"  1. Prime Race:      {path1}")
    print(f"  2. Gaussian Primes: {path2}")
    print(f"  3. Blender Script:  {path3}")
    print()
    print("To generate the 3D paraboloid:")
    print("  1. Open Blender")
    print("  2. Go to the Scripting workspace")
    print("  3. Open blender_paraboloid_primes.py")
    print("  4. Click Run Script")
