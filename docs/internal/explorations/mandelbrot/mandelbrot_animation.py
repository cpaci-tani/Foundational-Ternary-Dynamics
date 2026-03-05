"""
Mandelbrot Set Animation
========================
Exploring the connection between fractal self-similarity and consciousness.

The Mandelbrot set emerges from the simple iteration z = z^2 + c,
yet contains infinite complexity - a mathematical analog of how
simple rules can give rise to emergent consciousness.

Key connections to consciousness (TRD perspective):
- Self-reference: z feeds back into itself (like the sLoop)
- Boundary complexity: infinite detail at the edge (like observer-observed boundary)
- Universality: same patterns appear at all scales (scale-invariant consciousness)

Run this file directly for an auto-zoom animation.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.colors import LinearSegmentedColormap


def compute_mandelbrot(xmin, xmax, ymin, ymax, width, height, max_iter):
    """Compute the Mandelbrot set for a given region using vectorized operations."""
    x = np.linspace(xmin, xmax, width)
    y = np.linspace(ymin, ymax, height)

    # Create complex plane grid
    X, Y = np.meshgrid(x, y)
    C = X + 1j * Y

    # Vectorized computation for speed
    Z = np.zeros_like(C)
    M = np.zeros(C.shape)

    for i in range(max_iter):
        mask = np.abs(Z) <= 2
        Z[mask] = Z[mask] ** 2 + C[mask]
        M[mask] = i

    # Smooth coloring for aesthetic gradients
    mask = M < max_iter - 1
    with np.errstate(divide='ignore', invalid='ignore'):
        smooth = np.log(np.log2(np.abs(Z) + 1))
        smooth = np.nan_to_num(smooth, nan=0, posinf=0, neginf=0)
    M[mask] = M[mask] + 1 - smooth[mask]

    return M


def create_consciousness_colormap():
    """
    Create a colormap inspired by consciousness themes.

    Colors transition from void (deep blue/black) through
    manifestation (gold/white) - echoing the TRD ontology
    of void -> manifestation -> complexity.
    """
    colors = [
        (0.0, 0.0, 0.1),      # Deep void
        (0.1, 0.0, 0.3),      # Pre-manifestation
        (0.2, 0.0, 0.5),      # Potential
        (0.4, 0.2, 0.6),      # Emerging
        (0.6, 0.4, 0.7),      # Manifesting
        (0.8, 0.6, 0.3),      # Golden ratio region
        (1.0, 0.9, 0.5),      # Full manifestation
        (1.0, 1.0, 1.0),      # Pure consciousness
    ]
    return LinearSegmentedColormap.from_list('consciousness', colors, N=256)


def run_zoom_animation():
    """
    Create and display a zoom animation demonstrating
    the infinite self-similarity of the Mandelbrot set.
    """
    print("=" * 60)
    print("MANDELBROT SET: Self-Reference and Consciousness")
    print("=" * 60)
    print()
    print("The Mandelbrot set is defined by: z_{n+1} = z_n^2 + c")
    print()
    print("Connection to consciousness (TRD perspective):")
    print("  - Self-reference: output feeds back as input (sLoop)")
    print("  - Boundary complexity: infinite detail at the edge")
    print("  - Universality: same patterns at all scales")
    print("  - Emergence: simple rule -> infinite complexity")
    print()
    print("Starting animation... (close window to exit)")
    print()

    # Parameters
    width, height = 600, 450
    num_frames = 150

    # Zoom target: Seahorse Valley (beautiful spirals)
    target_x, target_y = -0.745, 0.113

    # Create figure with dark theme
    fig, ax = plt.subplots(figsize=(10, 7.5))
    fig.patch.set_facecolor('black')
    ax.set_facecolor('black')

    cmap = create_consciousness_colormap()

    # Initial view
    xmin, xmax = -2.5, 1.0
    ymin, ymax = -1.25, 1.25
    max_iter = 100

    # Initial computation
    data = compute_mandelbrot(xmin, xmax, ymin, ymax, width, height, max_iter)
    im = ax.imshow(data, extent=[xmin, xmax, ymin, ymax],
                   cmap=cmap, origin='lower', aspect='equal')

    ax.set_xlabel('Real(c)', color='white', fontsize=11)
    ax.set_ylabel('Imag(c)', color='white', fontsize=11)
    ax.tick_params(colors='white', labelsize=9)

    for spine in ax.spines.values():
        spine.set_color('white')

    title = ax.set_title('Mandelbrot Set: z = z² + c',
                         color='white', fontsize=14, fontweight='bold')

    # Info text box
    info_text = ax.text(
        0.02, 0.98, '', transform=ax.transAxes,
        color='white', fontsize=9, verticalalignment='top',
        fontfamily='monospace',
        bbox=dict(boxstyle='round', facecolor='black', alpha=0.8, edgecolor='gray')
    )

    # Animation state
    state = {
        'xmin': xmin, 'xmax': xmax,
        'ymin': ymin, 'ymax': ymax,
        'max_iter': max_iter
    }

    def animate(frame):
        # Calculate zoom progress (0 to 1)
        progress = frame / num_frames

        # Exponential zoom factor
        total_zoom = 5000  # Final zoom level
        current_zoom = total_zoom ** progress

        # Calculate span at current zoom
        initial_span_x = 3.5
        initial_span_y = 2.5
        span_x = initial_span_x / current_zoom
        span_y = initial_span_y / current_zoom

        # Smooth center interpolation toward target
        initial_cx, initial_cy = -0.75, 0.0
        t = min(1, progress * 1.5)  # Ease into target
        t = t * t * (3 - 2 * t)  # Smoothstep
        cx = initial_cx + (target_x - initial_cx) * t
        cy = initial_cy + (target_y - initial_cy) * t

        # Update bounds
        state['xmin'] = cx - span_x / 2
        state['xmax'] = cx + span_x / 2
        state['ymin'] = cy - span_y / 2
        state['ymax'] = cy + span_y / 2

        # Increase iterations as we zoom deeper for detail
        state['max_iter'] = int(100 + progress * 300)

        # Compute new frame
        data = compute_mandelbrot(
            state['xmin'], state['xmax'],
            state['ymin'], state['ymax'],
            width, height, state['max_iter']
        )

        im.set_data(data)
        im.set_extent([state['xmin'], state['xmax'],
                       state['ymin'], state['ymax']])
        im.set_clim(0, state['max_iter'])

        # Update title
        title.set_text(f'Mandelbrot Zoom: {current_zoom:.0f}x')

        # Update info
        info = (
            f"Center: {cx:.6f} + {cy:.6f}i\n"
            f"Span: {span_x:.2e}\n"
            f"Iterations: {state['max_iter']}\n"
            f"\nSelf-reference:\n"
            f"z → z² + c"
        )
        info_text.set_text(info)

        return [im, title, info_text]

    anim = FuncAnimation(fig, animate, frames=num_frames,
                         interval=80, blit=True, repeat=True)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    run_zoom_animation()
