"""
Mandelbrot Set Explorer with Animation
=======================================
Exploring the connection between fractal self-similarity and consciousness.

The Mandelbrot set emerges from the simple iteration z = z^2 + c,
yet contains infinite complexity - a mathematical analog of how
simple rules can give rise to emergent consciousness.

Key connections to consciousness (TRD perspective):
- Self-reference: z feeds back into itself (like the sLoop)
- Boundary complexity: infinite detail at the edge (like observer-observed boundary)
- Universality: same patterns appear at all scales (scale-invariant consciousness)
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.widgets as widgets


def mandelbrot(c, max_iter):
    """
    Compute escape time for a single complex point.

    The iteration z_{n+1} = z_n^2 + c is the simplest non-trivial
    polynomial recurrence - analogous to the minimal ternary structure
    in TRD that generates maximal complexity.
    """
    z = 0
    for n in range(max_iter):
        if abs(z) > 2:
            # Smooth coloring using continuous escape time
            return n + 1 - np.log(np.log2(abs(z)))
        z = z * z + c
    return max_iter


def compute_mandelbrot(xmin, xmax, ymin, ymax, width, height, max_iter):
    """Compute the Mandelbrot set for a given region."""
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

    # Smooth coloring
    mask = M < max_iter - 1
    M[mask] = M[mask] + 1 - np.log(np.log2(np.abs(Z[mask]) + 1))

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


class MandelbrotExplorer:
    """
    Interactive Mandelbrot set explorer with zoom animation.

    The infinite zoom reveals the self-similar structure -
    each zoom level contains the whole, like consciousness
    reflecting on itself (the sLoop).
    """

    def __init__(self, width=800, height=600, max_iter=100):
        self.width = width
        self.height = height
        self.max_iter = max_iter

        # Initial view (full Mandelbrot set)
        self.xmin, self.xmax = -2.5, 1.0
        self.ymin, self.ymax = -1.25, 1.25

        # Interesting zoom targets (areas of high complexity)
        self.zoom_targets = [
            # Seahorse Valley - infinite spirals
            (-0.75, 0.1),
            # Elephant Valley
            (0.275, 0.0),
            # Double spiral
            (-0.745, 0.113),
            # Mini Mandelbrot
            (-1.75, 0.0),
            # Antenna region
            (-1.941, 0.0),
        ]

        self.current_target = 0
        self.zoom_factor = 1.0
        self.animating = False

        # Setup figure
        self.fig, self.ax = plt.subplots(figsize=(12, 9))
        self.fig.patch.set_facecolor('black')
        self.ax.set_facecolor('black')

        # Create colormap
        self.cmap = create_consciousness_colormap()

        # Initial computation
        self.data = compute_mandelbrot(
            self.xmin, self.xmax, self.ymin, self.ymax,
            self.width, self.height, self.max_iter
        )

        # Display
        self.im = self.ax.imshow(
            self.data, extent=[self.xmin, self.xmax, self.ymin, self.ymax],
            cmap=self.cmap, origin='lower', aspect='equal'
        )

        self.ax.set_xlabel('Real(c)', color='white', fontsize=12)
        self.ax.set_ylabel('Imag(c)', color='white', fontsize=12)
        self.ax.tick_params(colors='white')

        self.title = self.ax.set_title(
            'Mandelbrot Set: z = z² + c\n'
            'Self-reference generates infinite complexity',
            color='white', fontsize=14, pad=20
        )

        # Add info text
        self.info_text = self.ax.text(
            0.02, 0.98, '', transform=self.ax.transAxes,
            color='white', fontsize=10, verticalalignment='top',
            fontfamily='monospace',
            bbox=dict(boxstyle='round', facecolor='black', alpha=0.7)
        )

        self.update_info()

        # Connect events
        self.fig.canvas.mpl_connect('button_press_event', self.on_click)
        self.fig.canvas.mpl_connect('key_press_event', self.on_key)

    def update_info(self):
        """Update the information display."""
        cx = (self.xmin + self.xmax) / 2
        cy = (self.ymin + self.ymax) / 2
        span = self.xmax - self.xmin

        info = (
            f'Center: {cx:.10f} + {cy:.10f}i\n'
            f'Span: {span:.2e}\n'
            f'Zoom: {3.5/span:.1f}x\n'
            f'Iterations: {self.max_iter}\n'
            f'\nControls:\n'
            f'Click: Zoom in\n'
            f'Right-click: Zoom out\n'
            f'A: Animate zoom\n'
            f'R: Reset view\n'
            f'+/-: Change iterations'
        )
        self.info_text.set_text(info)

    def compute_and_update(self):
        """Recompute and update the display."""
        self.data = compute_mandelbrot(
            self.xmin, self.xmax, self.ymin, self.ymax,
            self.width, self.height, self.max_iter
        )
        self.im.set_data(self.data)
        self.im.set_extent([self.xmin, self.xmax, self.ymin, self.ymax])
        self.im.set_clim(0, self.max_iter)
        self.update_info()
        self.fig.canvas.draw_idle()

    def zoom_to(self, cx, cy, factor):
        """Zoom to a point by a given factor."""
        # Current span
        dx = (self.xmax - self.xmin) / 2
        dy = (self.ymax - self.ymin) / 2

        # New span (smaller = zoom in)
        dx /= factor
        dy /= factor

        # Set new bounds centered on (cx, cy)
        self.xmin = cx - dx
        self.xmax = cx + dx
        self.ymin = cy - dy
        self.ymax = cy + dy

        self.compute_and_update()

    def on_click(self, event):
        """Handle mouse click for zooming."""
        if event.inaxes != self.ax:
            return

        cx, cy = event.xdata, event.ydata

        if event.button == 1:  # Left click - zoom in
            self.zoom_to(cx, cy, 2.0)
        elif event.button == 3:  # Right click - zoom out
            self.zoom_to(cx, cy, 0.5)

    def on_key(self, event):
        """Handle keyboard events."""
        if event.key == 'r':
            # Reset to full view
            self.xmin, self.xmax = -2.5, 1.0
            self.ymin, self.ymax = -1.25, 1.25
            self.max_iter = 100
            self.compute_and_update()

        elif event.key == '+' or event.key == '=':
            self.max_iter = min(1000, self.max_iter + 50)
            self.compute_and_update()

        elif event.key == '-':
            self.max_iter = max(50, self.max_iter - 50)
            self.compute_and_update()

        elif event.key == 'a':
            self.start_zoom_animation()

        elif event.key == 'n':
            # Next zoom target
            self.current_target = (self.current_target + 1) % len(self.zoom_targets)
            target = self.zoom_targets[self.current_target]
            self.start_zoom_animation(target)

    def start_zoom_animation(self, target=None):
        """Start an animated zoom to a target point."""
        if self.animating:
            return

        if target is None:
            target = self.zoom_targets[self.current_target]

        self.animating = True
        target_cx, target_cy = target

        # Reset view first
        self.xmin, self.xmax = -2.5, 1.0
        self.ymin, self.ymax = -1.25, 1.25

        def animate(frame):
            if frame >= 60:
                self.animating = False
                return [self.im]

            # Exponential zoom
            zoom_factor = 1.15

            # Current center
            cx = (self.xmin + self.xmax) / 2
            cy = (self.ymin + self.ymax) / 2

            # Move toward target
            cx = cx + (target_cx - cx) * 0.1
            cy = cy + (target_cy - cy) * 0.1

            # Zoom
            self.zoom_to(cx, cy, zoom_factor)

            # Increase iterations as we zoom deeper
            if frame % 10 == 0:
                self.max_iter = min(500, self.max_iter + 20)

            return [self.im]

        self.anim = FuncAnimation(
            self.fig, animate, frames=60,
            interval=100, blit=True, repeat=False
        )
        plt.draw()

    def run(self):
        """Start the explorer."""
        plt.tight_layout()
        plt.show()


def demo_zoom_animation():
    """
    Create a simple zoom animation to demonstrate
    the infinite self-similarity of the Mandelbrot set.
    """
    print("Creating Mandelbrot zoom animation...")
    print("This demonstrates the sLoop principle:")
    print("  - Self-reference (z = z^2 + c)")
    print("  - Infinite regress (zoom forever)")
    print("  - Boundary complexity (where consciousness meets reality)")
    print()

    # Parameters
    width, height = 600, 450
    max_iter = 100
    num_frames = 120

    # Zoom target: Seahorse Valley (beautiful spirals)
    target_x, target_y = -0.745, 0.113

    # Initial view
    xmin, xmax = -2.5, 1.0
    ymin, ymax = -1.25, 1.25

    # Create figure
    fig, ax = plt.subplots(figsize=(10, 7.5))
    fig.patch.set_facecolor('black')
    ax.set_facecolor('black')

    cmap = create_consciousness_colormap()

    # Initial computation
    data = compute_mandelbrot(xmin, xmax, ymin, ymax, width, height, max_iter)
    im = ax.imshow(data, extent=[xmin, xmax, ymin, ymax],
                   cmap=cmap, origin='lower', aspect='equal')

    ax.set_xlabel('Real(c)', color='white')
    ax.set_ylabel('Imag(c)', color='white')
    ax.tick_params(colors='white')

    title = ax.set_title('Mandelbrot Set: Infinite Self-Similarity',
                         color='white', fontsize=14)

    # Zoom state
    zoom_state = {
        'xmin': xmin, 'xmax': xmax,
        'ymin': ymin, 'ymax': ymax,
        'max_iter': max_iter
    }

    def animate(frame):
        # Calculate zoom progress
        progress = frame / num_frames

        # Exponential zoom factor
        total_zoom = 1000  # Final zoom level
        current_zoom = total_zoom ** progress

        # Calculate span
        initial_span_x = 3.5
        initial_span_y = 2.5
        span_x = initial_span_x / current_zoom
        span_y = initial_span_y / current_zoom

        # Center interpolation (move toward target)
        initial_cx, initial_cy = -0.75, 0.0
        cx = initial_cx + (target_x - initial_cx) * min(1, progress * 2)
        cy = initial_cy + (target_y - initial_cy) * min(1, progress * 2)

        # Update bounds
        zoom_state['xmin'] = cx - span_x / 2
        zoom_state['xmax'] = cx + span_x / 2
        zoom_state['ymin'] = cy - span_y / 2
        zoom_state['ymax'] = cy + span_y / 2

        # Increase iterations as we zoom
        zoom_state['max_iter'] = int(100 + progress * 200)

        # Compute
        data = compute_mandelbrot(
            zoom_state['xmin'], zoom_state['xmax'],
            zoom_state['ymin'], zoom_state['ymax'],
            width, height, zoom_state['max_iter']
        )

        im.set_data(data)
        im.set_extent([zoom_state['xmin'], zoom_state['xmax'],
                       zoom_state['ymin'], zoom_state['ymax']])
        im.set_clim(0, zoom_state['max_iter'])

        title.set_text(f'Mandelbrot Zoom: {current_zoom:.0f}x | '
                       f'Iterations: {zoom_state["max_iter"]}')

        return [im, title]

    print("Starting animation... (close window to exit)")
    anim = FuncAnimation(fig, animate, frames=num_frames,
                         interval=100, blit=True, repeat=True)

    plt.tight_layout()
    plt.show()

    return anim


def main():
    """Main entry point."""
    print("=" * 60)
    print("MANDELBROT SET EXPLORER")
    print("Exploring the mathematics of self-reference and complexity")
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
    print("Choose mode:")
    print("  1. Interactive Explorer (click to zoom)")
    print("  2. Auto-zoom Animation")
    print()

    choice = input("Enter choice (1 or 2): ").strip()

    if choice == "2":
        demo_zoom_animation()
    else:
        explorer = MandelbrotExplorer()
        explorer.run()


if __name__ == "__main__":
    main()
