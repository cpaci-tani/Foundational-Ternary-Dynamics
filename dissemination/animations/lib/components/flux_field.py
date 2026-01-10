"""
Flux Field Mobject
==================

Vector field visualization with glowing arrows for TRD flux representation.
The flux field J(v,t) is a vector field carrying potential energy density.
"""

from __future__ import annotations

import numpy as np
from typing import Callable, Sequence

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    OUT,
    ORIGIN,
    PI,
    TAU,
    Animation,
    AnimationGroup,
    Create,
    FadeIn,
    FadeOut,
    Transform,
    UpdateFromFunc,
    VGroup,
    VMobject,
    Arrow,
    Line,
    Dot,
    Circle,
    rate_functions,
    linear,
    smooth,
    interpolate_color,
    normalize,
)

from ..colors import TRD_COLORS, GLOW_COLORS, lerp_color


class FluxArrow(VGroup):
    """
    A single flux vector arrow with glow effect.

    Represents the flux J at a single lattice point.

    Parameters
    ----------
    start : np.ndarray
        Start position of arrow
    direction : np.ndarray
        Direction vector (magnitude determines arrow length scaling)
    max_length : float
        Maximum arrow length
    color : str
        Base color of the arrow
    show_glow : bool
        Whether to show glow effect
    """

    def __init__(
        self,
        start: np.ndarray = ORIGIN,
        direction: np.ndarray = RIGHT,
        max_length: float = 1.0,
        color: str = TRD_COLORS["highlight"],
        show_glow: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self._start = np.array(start)
        self._direction = np.array(direction)
        self._max_length = max_length
        self._color = color
        self._show_glow = show_glow

        self._build_arrow()

    def _build_arrow(self):
        """Construct the arrow visualization."""
        self.submobjects.clear()

        # Calculate arrow endpoint
        magnitude = np.linalg.norm(self._direction)
        if magnitude < 1e-6:
            # Zero vector: just show a dot
            dot = Dot(
                self._start,
                color=TRD_COLORS["void"],
                radius=0.05,
            )
            self.add(dot)
            return

        # Normalize and scale
        unit_dir = self._direction / magnitude
        scaled_length = min(magnitude, 1.0) * self._max_length
        end = self._start + unit_dir * scaled_length

        # Intensity based on magnitude (for color/opacity)
        intensity = min(magnitude, 1.0)

        # Glow layers
        if self._show_glow:
            for i in range(3, 0, -1):
                glow_arrow = Arrow(
                    self._start,
                    end,
                    buff=0,
                    stroke_width=8 * i,
                    stroke_opacity=0.1 / i * intensity,
                    color=self._color,
                    tip_length=0.15 * i,
                )
                self.add(glow_arrow)

        # Main arrow
        arrow = Arrow(
            self._start,
            end,
            buff=0,
            stroke_width=3,
            stroke_opacity=0.7 + 0.3 * intensity,
            color=self._color,
            tip_length=0.15,
        )
        self.add(arrow)

        # Bright tip
        if intensity > 0.3:
            tip_dot = Dot(
                end,
                color=TRD_COLORS["glow"],
                radius=0.05 * intensity,
            )
            self.add(tip_dot)

    def set_direction(self, new_direction: np.ndarray):
        """Update the flux direction."""
        self._direction = np.array(new_direction)
        self._build_arrow()

    def set_position(self, new_start: np.ndarray):
        """Update the starting position."""
        self._start = np.array(new_start)
        self._build_arrow()


class FluxFieldMobject(VGroup):
    """
    A grid of flux vectors representing the flux field J.

    Parameters
    ----------
    rows : int
        Number of rows in the grid
    cols : int
        Number of columns in the grid
    spacing : float
        Spacing between grid points
    flux_func : Callable
        Function (x, y) -> (jx, jy) returning flux at each point
    max_arrow_length : float
        Maximum arrow length
    color : str
        Base color for arrows
    show_glow : bool
        Whether to show glow effects
    """

    def __init__(
        self,
        rows: int = 10,
        cols: int = 10,
        spacing: float = 0.8,
        flux_func: Callable[[float, float], tuple[float, float]] | None = None,
        max_arrow_length: float = 0.6,
        color: str = TRD_COLORS["highlight"],
        show_glow: bool = True,
        arrow_scale: float = 1.0,
        **kwargs,
    ):
        # Remove any unexpected kwargs before passing to parent
        kwargs.pop('arrow_scale', None)
        super().__init__(**kwargs)
        self.arrow_scale = arrow_scale
        self.rows = rows
        self.cols = cols
        self.spacing = spacing
        self.max_arrow_length = max_arrow_length
        self._color = color
        self._show_glow = show_glow

        # Default flux function: radial outward
        self._flux_func = flux_func or self._default_flux

        self._arrows: dict[tuple[int, int], FluxArrow] = {}
        self._build_field()

    def _default_flux(self, x: float, y: float) -> tuple[float, float]:
        """Default flux function: radial outward from origin."""
        r = np.sqrt(x**2 + y**2)
        if r < 1e-6:
            return (0.0, 0.0)
        return (x / r * 0.5, y / r * 0.5)

    def _build_field(self):
        """Construct the vector field."""
        self.submobjects.clear()
        self._arrows.clear()

        for i in range(self.rows):
            for j in range(self.cols):
                # Calculate position
                x = (j - self.cols / 2 + 0.5) * self.spacing
                y = (i - self.rows / 2 + 0.5) * self.spacing
                pos = np.array([x, y, 0])

                # Get flux at this point
                jx, jy = self._flux_func(x, y)
                direction = np.array([jx, jy, 0])

                arrow = FluxArrow(
                    start=pos,
                    direction=direction,
                    max_length=self.max_arrow_length,
                    color=self._color,
                    show_glow=self._show_glow,
                )
                self._arrows[(i, j)] = arrow
                self.add(arrow)

    def set_flux_func(self, flux_func: Callable[[float, float], tuple[float, float]]):
        """Update the flux function and rebuild field."""
        self._flux_func = flux_func
        self._build_field()

    def update_field(self) -> Animation:
        """
        Create an animation that updates the field based on current flux_func.

        Returns
        -------
        Animation
            Update animation
        """
        def updater(mob, dt):
            for (i, j), arrow in self._arrows.items():
                x = (j - self.cols / 2 + 0.5) * self.spacing
                y = (i - self.rows / 2 + 0.5) * self.spacing
                jx, jy = self._flux_func(x, y)
                arrow.set_direction(np.array([jx, jy, 0]))

        return UpdateFromFunc(self, lambda m: None)

    def propagate(
        self,
        target_func: Callable[[float, float], tuple[float, float]],
        run_time: float = 2.0,
    ) -> Animation:
        """
        Animate transition to a new flux configuration.

        Parameters
        ----------
        target_func : Callable
            New flux function to transition to
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Propagation animation
        """
        # Create target field
        target_field = FluxFieldMobject(
            rows=self.rows,
            cols=self.cols,
            spacing=self.spacing,
            flux_func=target_func,
            max_arrow_length=self.max_arrow_length,
            color=self._color,
            show_glow=self._show_glow,
        )

        return Transform(self, target_field, run_time=run_time)

    def accumulate(
        self,
        center: np.ndarray = ORIGIN,
        strength: float = 1.0,
        run_time: float = 2.0,
    ) -> Animation:
        """
        Animate flux accumulating toward a center point.

        Parameters
        ----------
        center : np.ndarray
            Point where flux accumulates
        strength : float
            Accumulation strength
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Accumulation animation
        """
        cx, cy = center[0], center[1]

        def accumulating_flux(x: float, y: float) -> tuple[float, float]:
            dx, dy = cx - x, cy - y
            r = np.sqrt(dx**2 + dy**2)
            if r < 1e-6:
                return (0.0, 0.0)
            # Stronger toward center
            mag = strength / (1 + r)
            return (dx / r * mag, dy / r * mag)

        return self.propagate(accumulating_flux, run_time=run_time)

    def disperse(self, run_time: float = 2.0) -> Animation:
        """
        Animate flux dispersing outward from center.

        Parameters
        ----------
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Dispersion animation
        """
        def dispersing_flux(x: float, y: float) -> tuple[float, float]:
            r = np.sqrt(x**2 + y**2)
            if r < 1e-6:
                return (0.0, 0.0)
            # Stronger away from center
            mag = 0.5 * (1 - np.exp(-r))
            return (x / r * mag, y / r * mag)

        return self.propagate(dispersing_flux, run_time=run_time)

    def wave_pulse(
        self,
        origin: np.ndarray = ORIGIN,
        speed: float = 2.0,
        run_time: float = 3.0,
    ) -> Animation:
        """
        Animate a wave pulse propagating from origin.

        Parameters
        ----------
        origin : np.ndarray
            Origin of the wave
        speed : float
            Propagation speed
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Wave animation
        """
        ox, oy = origin[0], origin[1]
        start_time = [0.0]  # Mutable for closure

        def wave_flux(x: float, y: float) -> tuple[float, float]:
            dx, dy = x - ox, y - oy
            r = np.sqrt(dx**2 + dy**2)
            if r < 1e-6:
                return (0.0, 0.0)

            # Wave front position
            wave_pos = start_time[0] * speed
            # Gaussian-ish wave profile
            wave_strength = np.exp(-((r - wave_pos) ** 2) / 0.5)
            mag = wave_strength * 0.5

            return (dx / r * mag, dy / r * mag)

        def update_wave(mob, alpha):
            start_time[0] = alpha * run_time
            self.set_flux_func(wave_flux)

        return UpdateFromFunc(self, update_wave, run_time=run_time)


def create_flux_spiral(
    turns: float = 2.0,
    strength: float = 0.5,
) -> Callable[[float, float], tuple[float, float]]:
    """
    Create a spiral flux function.

    Parameters
    ----------
    turns : float
        Number of spiral turns
    strength : float
        Flux strength

    Returns
    -------
    Callable
        Flux function (x, y) -> (jx, jy)
    """
    def spiral_flux(x: float, y: float) -> tuple[float, float]:
        r = np.sqrt(x**2 + y**2)
        if r < 1e-6:
            return (0.0, 0.0)

        theta = np.arctan2(y, x)
        # Spiral: outward with rotation
        jx = np.cos(theta + turns * r) * strength
        jy = np.sin(theta + turns * r) * strength
        return (jx, jy)

    return spiral_flux


def create_flux_dipole(
    strength: float = 0.5,
    separation: float = 2.0,
) -> Callable[[float, float], tuple[float, float]]:
    """
    Create a dipole flux function (+1 and -1 source).

    Parameters
    ----------
    strength : float
        Flux strength
    separation : float
        Distance between poles

    Returns
    -------
    Callable
        Flux function (x, y) -> (jx, jy)
    """
    def dipole_flux(x: float, y: float) -> tuple[float, float]:
        # Positive pole at (-sep/2, 0), negative at (+sep/2, 0)
        x1, y1 = -separation / 2, 0
        x2, y2 = separation / 2, 0

        # Distance to each pole
        r1 = np.sqrt((x - x1)**2 + (y - y1)**2)
        r2 = np.sqrt((x - x2)**2 + (y - y2)**2)

        if r1 < 0.3 or r2 < 0.3:
            return (0.0, 0.0)

        # Outward from positive, inward to negative
        jx = strength * ((x - x1) / r1**2 - (x - x2) / r2**2)
        jy = strength * ((y - y1) / r1**2 - (y - y2) / r2**2)

        # Clamp magnitude
        mag = np.sqrt(jx**2 + jy**2)
        if mag > 1.0:
            jx, jy = jx / mag, jy / mag

        return (jx, jy)

    return dipole_flux
