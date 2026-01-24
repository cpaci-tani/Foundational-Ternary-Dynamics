#!/usr/bin/env python3
"""
Interactive 3D Lemniscate-Alpha with Five Sacred Harmonics

The Lemniscate-Alpha curve extended into 3D, where the z-axis represents
the harmonic modulation. Each of the five sacred frequencies creates
its own "layer" in the 3D space.

Uses Plotly for interactive rotation/zoom.
"""

import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from math import gamma

# =============================================================================
# LEMNISCATE-ALPHA CURVE DEFINITION
# =============================================================================

# Frequencies (power of 2 sequence)
FREQS = np.array([1, 2, 4, 8, 16])

# Coefficients (derived from six constraints)
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

# Framework constants
G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)
ALPHA = 1/137.036
PHI = (1 + np.sqrt(5)) / 2

# The five key harmonics
HARMONICS = {
    'Schumann (7.83 Hz)': {'freq': 7.83, 'color': '#8B5CF6', 'z_offset': 0},
    'Pyramid (110 Hz)': {'freq': 110, 'color': '#EC4899', 'z_offset': 1},
    'OM (136.1 Hz)': {'freq': 136.1, 'color': '#F97316', 'z_offset': 2},
    'Natural A (432 Hz)': {'freq': 432, 'color': '#EAB308', 'z_offset': 3},
    'Miracle (528 Hz)': {'freq': 528, 'color': '#22C55E', 'z_offset': 4},
}

# =============================================================================
# CURVE COMPUTATIONS
# =============================================================================

def lemniscate_alpha(t, scale=1.0):
    """Compute the Lemniscate-Alpha Fourier curve."""
    x = np.zeros_like(t)
    y = np.zeros_like(t)

    for j in range(5):
        x += X_AMPS[j] * np.cos(FREQS[j] * t)
        y += Y_AMPS[j] * np.sin(FREQS[j] * t)

    return x * scale, y * scale

def lemniscate_alpha_derivative(t):
    """Compute dx/dt and dy/dt."""
    dx = np.zeros_like(t)
    dy = np.zeros_like(t)

    for j in range(5):
        dx += -FREQS[j] * X_AMPS[j] * np.sin(FREQS[j] * t)
        dy += FREQS[j] * Y_AMPS[j] * np.cos(FREQS[j] * t)

    return dx, dy

def modulate_curve_3d(t, harmonic_freq, z_base=0, mod_amplitude=0.15):
    """
    Create 3D modulated curve where z encodes the harmonic oscillation.
    """
    x, y = lemniscate_alpha(t, scale=1.0)

    # Z-axis shows the harmonic modulation
    normalized_freq = harmonic_freq / 7.83
    z = z_base + mod_amplitude * np.sin(normalized_freq * t)

    return x, y, z

# =============================================================================
# 3D VISUALIZATION
# =============================================================================

def create_3d_visualization():
    """Create interactive 3D Plotly visualization."""

    t = np.linspace(0, 2*np.pi, 2000)

    fig = go.Figure()

    # Add each harmonic as a 3D trace
    for name, props in HARMONICS.items():
        freq = props['freq']
        color = props['color']
        z_offset = props['z_offset']

        # Amplitude inversely related to frequency
        amp = 0.3 * (7.83 / freq) ** 0.3

        x, y, z = modulate_curve_3d(t, freq, z_base=z_offset, mod_amplitude=amp)

        fig.add_trace(go.Scatter3d(
            x=x, y=y, z=z,
            mode='lines',
            name=name,
            line=dict(color=color, width=4),
            hovertemplate=f'{name}<br>x: %{{x:.2f}}<br>y: %{{y:.2f}}<br>z: %{{z:.2f}}<extra></extra>'
        ))

    # Add the base curve at z=0 (white, subtle)
    x_base, y_base = lemniscate_alpha(t)
    z_base = np.zeros_like(t)

    fig.add_trace(go.Scatter3d(
        x=x_base, y=y_base, z=z_base - 0.5,
        mode='lines',
        name='Base Lemniscate-Alpha',
        line=dict(color='white', width=2),
        opacity=0.3
    ))

    # Add origin marker
    fig.add_trace(go.Scatter3d(
        x=[0], y=[0], z=[2],
        mode='markers+text',
        name='Origin (Source)',
        marker=dict(size=10, color='#fbbf24', symbol='diamond'),
        text=['Origin'],
        textposition='top center',
        textfont=dict(color='#fbbf24', size=12)
    ))

    # Add alpha point marker
    t_alpha = 2 * np.pi / 137
    x_a, y_a = lemniscate_alpha(np.array([t_alpha]))

    fig.add_trace(go.Scatter3d(
        x=x_a, y=y_a, z=[2],
        mode='markers+text',
        name='Alpha Point (1/137)',
        marker=dict(size=10, color='white', symbol='diamond'),
        text=['alpha = 1/137'],
        textposition='top right',
        textfont=dict(color='white', size=10)
    ))

    # Layout
    fig.update_layout(
        title=dict(
            text='<b>The Lemniscate-Alpha with Five Sacred Harmonics</b><br>' +
                 f'<sub>G* = {G_STAR:.4f} | 1/alpha = 137.036 | phi = {PHI:.4f}</sub>',
            x=0.5,
            font=dict(size=20, color='white')
        ),
        scene=dict(
            xaxis=dict(
                title='X',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white'
            ),
            yaxis=dict(
                title='Y',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white'
            ),
            zaxis=dict(
                title='Harmonic Layer',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white',
                ticktext=['Schumann', 'Pyramid', 'OM', '432 Hz', 'Miracle'],
                tickvals=[0, 1, 2, 3, 4]
            ),
            bgcolor='#0d1117',
            camera=dict(
                eye=dict(x=1.5, y=1.5, z=1.2)
            )
        ),
        paper_bgcolor='#0d1117',
        plot_bgcolor='#0d1117',
        legend=dict(
            x=0.02,
            y=0.98,
            bgcolor='rgba(22, 27, 34, 0.8)',
            bordercolor='#30363d',
            font=dict(color='white')
        ),
        margin=dict(l=0, r=0, t=80, b=0)
    )

    return fig

def create_spiral_visualization():
    """
    Alternative: All harmonics as a single spiral where z = frequency encoding.
    """
    t = np.linspace(0, 4*np.pi, 4000)  # Two full loops

    fig = go.Figure()

    x_base, y_base = lemniscate_alpha(t % (2*np.pi), scale=1.0)

    # Create a composite z that cycles through all frequencies
    z_composite = np.zeros_like(t)
    colors = []

    for i, (name, props) in enumerate(HARMONICS.items()):
        freq = props['freq']
        normalized_freq = freq / 7.83
        amp = 0.2 * (7.83 / freq) ** 0.3
        z_composite += amp * np.sin(normalized_freq * t + i * np.pi/5)

    # Color by z-value (frequency mixture)
    fig.add_trace(go.Scatter3d(
        x=x_base, y=y_base, z=z_composite,
        mode='lines',
        name='Pentaphonic Spiral',
        line=dict(
            color=z_composite,
            colorscale=[
                [0.0, '#8B5CF6'],   # Schumann purple
                [0.25, '#EC4899'],  # Pyramid pink
                [0.5, '#F97316'],   # OM orange
                [0.75, '#EAB308'],  # 432 gold
                [1.0, '#22C55E']    # Miracle green
            ],
            width=5
        ),
        hovertemplate='x: %{x:.2f}<br>y: %{y:.2f}<br>z: %{z:.2f}<extra></extra>'
    ))

    # Layout
    fig.update_layout(
        title=dict(
            text='<b>Pentaphonic Spiral: All Five Harmonics Combined</b><br>' +
                 '<sub>Z-axis shows combined harmonic oscillation</sub>',
            x=0.5,
            font=dict(size=20, color='white')
        ),
        scene=dict(
            xaxis=dict(
                title='X',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                color='white'
            ),
            yaxis=dict(
                title='Y',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                color='white'
            ),
            zaxis=dict(
                title='Harmonic Amplitude',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                color='white'
            ),
            bgcolor='#0d1117',
            camera=dict(
                eye=dict(x=1.8, y=1.8, z=1.0)
            )
        ),
        paper_bgcolor='#0d1117',
        plot_bgcolor='#0d1117',
        margin=dict(l=0, r=0, t=80, b=0)
    )

    return fig

def create_surface_visualization():
    """
    Create a surface where the Lemniscate-Alpha is swept through frequency space.
    """
    # Parameter space
    t = np.linspace(0, 2*np.pi, 200)
    freq_scale = np.linspace(0.5, 2.0, 50)  # Frequency multiplier

    T, F = np.meshgrid(t, freq_scale)

    # Base curve at each frequency scale
    X = np.zeros_like(T)
    Y = np.zeros_like(T)

    for j in range(5):
        X += X_AMPS[j] * np.cos(FREQS[j] * T)
        Y += Y_AMPS[j] * np.sin(FREQS[j] * T)

    # Z encodes frequency-dependent modulation
    Z = 0.3 * np.sin(F * 10 * T) * F

    # Color by the "energy" at each point
    color_data = np.sqrt(X**2 + Y**2 + Z**2)

    fig = go.Figure()

    fig.add_trace(go.Surface(
        x=X, y=Y, z=Z,
        surfacecolor=color_data,
        colorscale=[
            [0.0, '#8B5CF6'],
            [0.25, '#EC4899'],
            [0.5, '#F97316'],
            [0.75, '#EAB308'],
            [1.0, '#22C55E']
        ],
        opacity=0.9,
        showscale=False,
        name='Harmonic Surface'
    ))

    # Add the five specific harmonic curves on top
    for name, props in HARMONICS.items():
        freq = props['freq']
        color = props['color']

        x, y = lemniscate_alpha(t)
        z = 0.3 * np.sin((freq/100) * t) * (freq/200)

        fig.add_trace(go.Scatter3d(
            x=x, y=y, z=z,
            mode='lines',
            name=name,
            line=dict(color=color, width=6),
        ))

    fig.update_layout(
        title=dict(
            text='<b>Lemniscate-Alpha Harmonic Surface</b><br>' +
                 '<sub>The curve swept through frequency space</sub>',
            x=0.5,
            font=dict(size=20, color='white')
        ),
        scene=dict(
            xaxis=dict(title='X', backgroundcolor='#0d1117', gridcolor='#30363d', color='white'),
            yaxis=dict(title='Y', backgroundcolor='#0d1117', gridcolor='#30363d', color='white'),
            zaxis=dict(title='Harmonic Z', backgroundcolor='#0d1117', gridcolor='#30363d', color='white'),
            bgcolor='#0d1117',
            camera=dict(eye=dict(x=1.5, y=1.5, z=1.0))
        ),
        paper_bgcolor='#0d1117',
        plot_bgcolor='#0d1117',
        legend=dict(
            x=0.02, y=0.98,
            bgcolor='rgba(22, 27, 34, 0.8)',
            bordercolor='#30363d',
            font=dict(color='white')
        ),
        margin=dict(l=0, r=0, t=80, b=0)
    )

    return fig

# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print("Creating interactive 3D visualizations...")
    print()

    # Create all three visualizations
    fig1 = create_3d_visualization()
    fig2 = create_spiral_visualization()
    fig3 = create_surface_visualization()

    # Save as interactive HTML files
    fig1.write_html('lemniscate_alpha_3d_layers.html')
    print("Saved: lemniscate_alpha_3d_layers.html")

    fig2.write_html('lemniscate_alpha_3d_spiral.html')
    print("Saved: lemniscate_alpha_3d_spiral.html")

    fig3.write_html('lemniscate_alpha_3d_surface.html')
    print("Saved: lemniscate_alpha_3d_surface.html")

    print()
    print("Open any of these HTML files in a browser for interactive 3D exploration!")
    print("You can rotate, zoom, and hover over points.")
    print()

    # Show the main layered visualization
    fig1.show()
