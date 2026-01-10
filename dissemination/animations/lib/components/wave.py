"""
Wave Mobject
============

Luminous flux wave propagation visualization.
Animates the discrete wave equation on the TRD lattice.
"""

from __future__ import annotations

import numpy as np
from typing import Callable

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    PI,
    TAU,
    Animation,
    AnimationGroup,
    Create,
    FadeIn,
    FadeOut,
    UpdateFromAlphaFunc,
    UpdateFromFunc,
    VGroup,
    VMobject,
    Circle,
    Annulus,
    Line,
    Dot,
    ParametricFunction,
    FunctionGraph,
    rate_functions,
    linear,
    smooth,
    there_and_back,
)

from ..colors import TRD_COLORS, GLOW_COLORS, lerp_color


class WaveFront(VGroup):
    """
    A single expanding wave front (ring) with glow.

    Parameters
    ----------
    center : np.ndarray
        Center of the wave
    radius : float
        Current radius
    color : str
        Wave color
    width : float
        Ring width
    """

    def __init__(
        self,
        center: np.ndarray = ORIGIN,
        radius: float = 1.0,
        color: str = TRD_COLORS["highlight"],
        width: float = 0.1,
        opacity: float = 0.8,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self._center = center
        self._radius = radius
        self._color = color
        self._width = width
        self._opacity = opacity

        self._build_wavefront()

    def _build_wavefront(self):
        """Construct the wave front visualization."""
        self.submobjects.clear()

        # Outer glow
        for i in range(3, 0, -1):
            glow = Circle(
                radius=self._radius,
                stroke_color=self._color,
                stroke_width=self._width * 20 * i,
                stroke_opacity=self._opacity * 0.1 / i,
                fill_opacity=0,
            )
            glow.move_to(self._center)
            self.add(glow)

        # Main ring
        ring = Circle(
            radius=self._radius,
            stroke_color=self._color,
            stroke_width=self._width * 20,
            stroke_opacity=self._opacity,
            fill_opacity=0,
        )
        ring.move_to(self._center)
        self.add(ring)

        # Inner bright edge
        inner = Circle(
            radius=self._radius - self._width * 0.3,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=self._width * 5,
            stroke_opacity=self._opacity * 0.5,
            fill_opacity=0,
        )
        inner.move_to(self._center)
        self.add(inner)

    def set_radius(self, new_radius: float):
        """Update the wave radius."""
        self._radius = new_radius
        self._build_wavefront()


class WavePulse(VGroup):
    """
    An animated wave pulse that expands from a point.

    Parameters
    ----------
    center : np.ndarray
        Origin of the pulse
    max_radius : float
        Maximum expansion radius
    color : str
        Wave color
    num_rings : int
        Number of concentric rings
    """

    def __init__(
        self,
        center: np.ndarray = ORIGIN,
        max_radius: float = 5.0,
        color: str = TRD_COLORS["highlight"],
        num_rings: int = 3,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.center = center
        self.max_radius = max_radius
        self.color = color
        self.num_rings = num_rings

        self._rings: list[WaveFront] = []
        self._build_pulse()

    def _build_pulse(self):
        """Create initial pulse structure."""
        for i in range(self.num_rings):
            ring = WaveFront(
                center=self.center,
                radius=0.1,
                color=self.color,
                width=0.05,
                opacity=0.0,
            )
            self._rings.append(ring)
            self.add(ring)

    def expand(self, run_time: float = 2.0) -> Animation:
        """
        Animate the pulse expanding outward.

        Parameters
        ----------
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Expansion animation
        """
        def update_pulse(mob, alpha):
            for i, ring in enumerate(self._rings):
                # Stagger the rings
                ring_alpha = max(0, min(1, (alpha - i * 0.15) / 0.7))

                if ring_alpha <= 0:
                    ring.set_opacity(0)
                else:
                    # Expand
                    radius = ring_alpha * self.max_radius
                    # Fade out as it expands
                    opacity = (1 - ring_alpha) * 0.8

                    ring._radius = max(0.1, radius)
                    ring._opacity = opacity
                    ring._build_wavefront()

        return UpdateFromAlphaFunc(self, update_pulse, run_time=run_time)


class StandingWave(VGroup):
    """
    A standing wave pattern visualization.

    Shows nodes and antinodes of a 1D standing wave.

    Parameters
    ----------
    length : float
        Total length of the wave
    wavelength : float
        Wavelength
    amplitude : float
        Wave amplitude
    color : str
        Wave color
    """

    def __init__(
        self,
        length: float = 10.0,
        wavelength: float = 2.0,
        amplitude: float = 1.0,
        color: str = TRD_COLORS["highlight"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.length = length
        self.wavelength = wavelength
        self.amplitude = amplitude
        self.color = color

        self._time = 0.0
        self._build_wave()

    def _wave_func(self, x: float) -> float:
        """Standing wave function."""
        k = TAU / self.wavelength
        omega = TAU  # Frequency = 1
        # Standing wave: sin(kx) * cos(omega*t)
        return self.amplitude * np.sin(k * x) * np.cos(omega * self._time)

    def _build_wave(self):
        """Construct the wave visualization."""
        self.submobjects.clear()

        # Create the wave curve
        wave_curve = FunctionGraph(
            self._wave_func,
            x_range=[-self.length / 2, self.length / 2, 0.05],
            color=self.color,
            stroke_width=3,
        )

        # Add glow
        for i in range(3, 0, -1):
            glow_curve = FunctionGraph(
                self._wave_func,
                x_range=[-self.length / 2, self.length / 2, 0.05],
                color=self.color,
                stroke_width=3 + i * 4,
                stroke_opacity=0.1 / i,
            )
            self.add(glow_curve)

        self.add(wave_curve)

        # Mark nodes (zero crossings at quarter wavelengths)
        num_nodes = int(self.length / (self.wavelength / 2)) + 1
        for i in range(num_nodes):
            x = -self.length / 2 + i * (self.wavelength / 2)
            if abs(x) <= self.length / 2:
                node = Dot(
                    point=np.array([x, 0, 0]),
                    radius=0.08,
                    color=TRD_COLORS["void_light"],
                )
                self.add(node)

    def oscillate(self, run_time: float = 2.0, cycles: float = 1.0) -> Animation:
        """
        Animate the standing wave oscillation.

        Parameters
        ----------
        run_time : float
            Animation duration
        cycles : float
            Number of oscillation cycles

        Returns
        -------
        Animation
            Oscillation animation
        """
        def update_wave(mob, alpha):
            self._time = alpha * cycles
            self._build_wave()

        return UpdateFromAlphaFunc(self, update_wave, run_time=run_time)


class InterferencePattern(VGroup):
    """
    Two-source interference pattern visualization.

    Shows constructive and destructive interference fringes.

    Parameters
    ----------
    source1 : np.ndarray
        Position of first source
    source2 : np.ndarray
        Position of second source
    wavelength : float
        Wave wavelength
    extent : float
        Visualization extent
    resolution : int
        Grid resolution
    """

    def __init__(
        self,
        source1: np.ndarray = LEFT * 2,
        source2: np.ndarray = RIGHT * 2,
        wavelength: float = 1.0,
        extent: float = 6.0,
        resolution: int = 50,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.source1 = source1
        self.source2 = source2
        self.wavelength = wavelength
        self.extent = extent
        self.resolution = resolution

        self._time = 0.0
        self._build_pattern()

    def _wave_amplitude(self, pos: np.ndarray) -> float:
        """Calculate combined wave amplitude at a position."""
        k = TAU / self.wavelength
        omega = TAU

        r1 = np.linalg.norm(pos - self.source1)
        r2 = np.linalg.norm(pos - self.source2)

        # Two waves with same frequency, different phases due to distance
        wave1 = np.sin(k * r1 - omega * self._time) / max(0.5, np.sqrt(r1))
        wave2 = np.sin(k * r2 - omega * self._time) / max(0.5, np.sqrt(r2))

        return wave1 + wave2

    def _build_pattern(self):
        """Construct the interference pattern."""
        self.submobjects.clear()

        # Create grid of points showing interference
        step = self.extent / self.resolution
        for i in range(self.resolution):
            for j in range(self.resolution):
                x = -self.extent / 2 + i * step
                y = -self.extent / 2 + j * step
                pos = np.array([x, y, 0])

                amp = self._wave_amplitude(pos)

                # Map amplitude to color and opacity
                intensity = (amp + 2) / 4  # Normalize to 0-1
                intensity = max(0, min(1, intensity))

                if intensity > 0.5:
                    # Constructive: bright
                    color = lerp_color(TRD_COLORS["void"], TRD_COLORS["highlight"], (intensity - 0.5) * 2)
                    opacity = 0.3 + intensity * 0.5
                else:
                    # Destructive: dim
                    color = TRD_COLORS["background_light"]
                    opacity = 0.1 + intensity * 0.2

                dot = Dot(
                    point=pos,
                    radius=step * 0.4,
                    color=color,
                    fill_opacity=opacity,
                )
                self.add(dot)

        # Mark sources
        for src, color in [(self.source1, TRD_COLORS["matter"]),
                          (self.source2, TRD_COLORS["antimatter"])]:
            source_marker = Dot(
                point=src,
                radius=0.15,
                color=color,
            )
            # Glow
            glow = Circle(
                radius=0.3,
                stroke_color=color,
                stroke_width=3,
                stroke_opacity=0.5,
                fill_opacity=0,
            )
            glow.move_to(src)
            self.add(glow)
            self.add(source_marker)

    def animate_interference(self, run_time: float = 3.0, cycles: float = 1.0) -> Animation:
        """
        Animate the interference pattern evolution.

        Parameters
        ----------
        run_time : float
            Animation duration
        cycles : float
            Number of wave cycles

        Returns
        -------
        Animation
            Interference animation
        """
        def update_pattern(mob, alpha):
            self._time = alpha * cycles
            self._build_pattern()

        return UpdateFromAlphaFunc(self, update_pattern, run_time=run_time)


class FluxWave(VGroup):
    """
    Discrete flux wave propagation on a lattice.

    Shows the wave equation solution on a grid.

    Parameters
    ----------
    grid_size : int
        Grid size
    spacing : float
        Grid spacing
    wave_speed : float
        Wave propagation speed (in lattice units per time)
    """

    def __init__(
        self,
        grid_size: int = 15,
        spacing: float = 0.6,
        wave_speed: float = 1.0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.grid_size = grid_size
        self.spacing = spacing
        self.wave_speed = wave_speed

        self._amplitudes = np.zeros((grid_size, grid_size))
        self._velocities = np.zeros((grid_size, grid_size))
        self._time = 0.0

        self._dots: dict[tuple[int, int], Dot] = {}
        self._build_grid()

    def _build_grid(self):
        """Create the visualization grid."""
        self.submobjects.clear()
        self._dots.clear()

        offset = -(self.grid_size - 1) * self.spacing / 2

        for i in range(self.grid_size):
            for j in range(self.grid_size):
                x = offset + j * self.spacing
                y = offset + i * self.spacing

                # Initial state: flat
                dot = Dot(
                    point=np.array([x, y, 0]),
                    radius=self.spacing * 0.3,
                    color=TRD_COLORS["void"],
                    fill_opacity=0.5,
                )
                self._dots[(i, j)] = dot
                self.add(dot)

    def _update_visuals(self):
        """Update dot colors/sizes based on amplitudes."""
        for i in range(self.grid_size):
            for j in range(self.grid_size):
                amp = self._amplitudes[i, j]
                dot = self._dots.get((i, j))
                if dot:
                    # Map amplitude to visual properties
                    if amp > 0.1:
                        color = lerp_color(TRD_COLORS["void"], TRD_COLORS["matter"], min(1, amp))
                        radius = self.spacing * 0.3 * (1 + amp * 0.5)
                    elif amp < -0.1:
                        color = lerp_color(TRD_COLORS["void"], TRD_COLORS["antimatter"], min(1, -amp))
                        radius = self.spacing * 0.3 * (1 - amp * 0.5)
                    else:
                        color = TRD_COLORS["void"]
                        radius = self.spacing * 0.3

                    dot.set_color(color)
                    dot.set_radius(radius)

    def set_initial_pulse(self, center: tuple[int, int], amplitude: float = 1.0):
        """Set an initial pulse at a grid position."""
        ci, cj = center
        for di in range(-2, 3):
            for dj in range(-2, 3):
                ni, nj = ci + di, cj + dj
                if 0 <= ni < self.grid_size and 0 <= nj < self.grid_size:
                    dist = np.sqrt(di**2 + dj**2)
                    self._amplitudes[ni, nj] = amplitude * np.exp(-dist**2 / 2)

        self._update_visuals()

    def propagate(self, run_time: float = 3.0, dt: float = 0.1) -> Animation:
        """
        Animate wave propagation using discrete wave equation.

        Parameters
        ----------
        run_time : float
            Animation duration
        dt : float
            Time step for simulation

        Returns
        -------
        Animation
            Propagation animation
        """
        c2 = self.wave_speed ** 2

        def update_wave(mob, alpha):
            # Number of simulation steps
            steps = int(alpha * run_time / dt)

            # Reset and replay
            self._amplitudes = np.zeros((self.grid_size, self.grid_size))
            self._velocities = np.zeros((self.grid_size, self.grid_size))

            # Set initial pulse
            center = self.grid_size // 2
            for di in range(-2, 3):
                for dj in range(-2, 3):
                    ni, nj = center + di, center + dj
                    if 0 <= ni < self.grid_size and 0 <= nj < self.grid_size:
                        dist = np.sqrt(di**2 + dj**2)
                        self._amplitudes[ni, nj] = np.exp(-dist**2 / 2)

            # Simulate
            for _ in range(steps):
                # Discrete Laplacian
                laplacian = np.zeros_like(self._amplitudes)
                for i in range(1, self.grid_size - 1):
                    for j in range(1, self.grid_size - 1):
                        laplacian[i, j] = (
                            self._amplitudes[i+1, j] +
                            self._amplitudes[i-1, j] +
                            self._amplitudes[i, j+1] +
                            self._amplitudes[i, j-1] -
                            4 * self._amplitudes[i, j]
                        )

                # Update velocities and positions
                self._velocities += c2 * laplacian * dt
                self._amplitudes += self._velocities * dt

                # Damping at boundaries
                self._amplitudes[0, :] *= 0.9
                self._amplitudes[-1, :] *= 0.9
                self._amplitudes[:, 0] *= 0.9
                self._amplitudes[:, -1] *= 0.9

            self._update_visuals()

        return UpdateFromAlphaFunc(self, update_wave, run_time=run_time)
