"""
Lemniscate Mobject
==================

Lemniscate-Alpha curve visualization with harmonic decomposition.
Shows the connection between elliptic integrals and the fine structure constant.
"""

from __future__ import annotations

import numpy as np
from typing import Callable, Sequence

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
    Succession,
    Create,
    FadeIn,
    FadeOut,
    ShowPassingFlash,
    Write,
    UpdateFromAlphaFunc,
    VGroup,
    VMobject,
    ParametricFunction,
    FunctionGraph,
    Line,
    Dot,
    Circle,
    Arc,
    Text,
    MathTex,
    DecimalNumber,
    rate_functions,
)

from ..colors import TRD_COLORS, MODE_COLORS, GLOW_COLORS, lerp_color


# Lemniscatic constant G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
# This is the arc length of the lemniscate r^2 = cos(2*theta) from 0 to 2*pi
G_STAR = 2.6220575542921198  # Actual lemniscatic constant (often denoted varpi)
VARPI = 2.622057554292119810  # More precise value


class LemniscateCurve(ParametricFunction):
    """
    The Lemniscate of Bernoulli: r² = a² cos(2θ)

    In Cartesian: (x² + y²)² = a²(x² - y²)

    Parametric form uses t ∈ [0, 2π]:
        x = a * cos(t) / (1 + sin²(t))
        y = a * sin(t) * cos(t) / (1 + sin²(t))

    Parameters
    ----------
    scale : float
        Scale factor (a in the equation)
    color : str
        Curve color
    stroke_width : float
        Line width
    """

    def __init__(
        self,
        scale: float = 2.0,
        color: str = TRD_COLORS["highlight"],
        stroke_width: float = 3,
        **kwargs,
    ):
        self.scale = scale

        def lemniscate_func(t: float) -> np.ndarray:
            # Parametric form of lemniscate
            denom = 1 + np.sin(t) ** 2
            x = self.scale * np.cos(t) / denom
            y = self.scale * np.sin(t) * np.cos(t) / denom
            return np.array([x, y, 0])

        super().__init__(
            lemniscate_func,
            t_range=[0, TAU, 0.01],
            color=color,
            stroke_width=stroke_width,
            **kwargs,
        )


class LemniscateWithGlow(VGroup):
    """
    Lemniscate curve with glow effect layers.

    Parameters
    ----------
    scale : float
        Scale factor
    color : str
        Main curve color
    glow_layers : int
        Number of glow layers
    """

    def __init__(
        self,
        scale: float = 2.0,
        color: str = TRD_COLORS["highlight"],
        glow_layers: int = 3,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.scale = scale
        self.color = color

        # Add glow layers (outermost first)
        for i in range(glow_layers, 0, -1):
            glow = LemniscateCurve(
                scale=scale,
                color=color,
                stroke_width=3 + i * 4,
                stroke_opacity=0.15 / i,
            )
            self.add(glow)

        # Main curve
        main = LemniscateCurve(
            scale=scale,
            color=color,
            stroke_width=3,
        )
        self.add(main)

        # Bright core
        core = LemniscateCurve(
            scale=scale,
            color=TRD_COLORS["glow"],
            stroke_width=1.5,
            stroke_opacity=0.8,
        )
        self.add(core)


class LemniscateHarmonic(VGroup):
    """
    A single harmonic mode of the lemniscate.

    The lemniscate can be decomposed into harmonic modes.
    This shows one mode with its characteristic frequency.

    Parameters
    ----------
    mode : int
        Harmonic mode number (1, 2, 4, 8, 16)
    scale : float
        Scale factor
    amplitude : float
        Mode amplitude
    """

    def __init__(
        self,
        mode: int = 1,
        scale: float = 2.0,
        amplitude: float = 1.0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.mode = mode
        self.scale = scale
        self.amplitude = amplitude

        color = MODE_COLORS.get(mode, TRD_COLORS["text"])

        def harmonic_func(t: float) -> np.ndarray:
            # Harmonic modulation of lemniscate
            denom = 1 + np.sin(t) ** 2
            base_x = self.scale * np.cos(t) / denom
            base_y = self.scale * np.sin(t) * np.cos(t) / denom

            # Apply harmonic modulation
            mod = self.amplitude * np.cos(mode * t) * 0.3
            x = base_x * (1 + mod)
            y = base_y * (1 + mod)
            return np.array([x, y, 0])

        # Glow
        for i in range(2, 0, -1):
            glow = ParametricFunction(
                harmonic_func,
                t_range=[0, TAU, 0.01],
                color=color,
                stroke_width=3 + i * 3,
                stroke_opacity=0.1 / i,
            )
            self.add(glow)

        # Main curve
        curve = ParametricFunction(
            harmonic_func,
            t_range=[0, TAU, 0.01],
            color=color,
            stroke_width=2,
        )
        self.add(curve)


class LemniscateDecomposition(VGroup):
    """
    Shows the lemniscate decomposed into multiple harmonics.

    Parameters
    ----------
    modes : list[int]
        Which harmonic modes to show
    scale : float
        Base scale
    """

    def __init__(
        self,
        modes: list[int] = [1, 2, 4, 8, 16],
        scale: float = 2.0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.modes = modes
        self.scale = scale

        self._harmonics: list[LemniscateHarmonic] = []
        self._build_decomposition()

    def _build_decomposition(self):
        """Build the harmonic components."""
        # Base lemniscate (dim)
        base = LemniscateCurve(
            scale=self.scale,
            color=TRD_COLORS["grid"],
            stroke_width=1,
            stroke_opacity=0.3,
        )
        self.add(base)

        # Harmonic modes
        for i, mode in enumerate(self.modes):
            # Offset each harmonic vertically for visibility
            harmonic = LemniscateHarmonic(
                mode=mode,
                scale=self.scale * 0.8,
                amplitude=0.5 / (i + 1),
            )
            self._harmonics.append(harmonic)
            self.add(harmonic)

    def build_up_animation(self, run_time: float = 5.0) -> Animation:
        """
        Animate building up the lemniscate from its harmonics.

        Parameters
        ----------
        run_time : float
            Total animation duration

        Returns
        -------
        Animation
            Build-up animation
        """
        anims = []
        time_per_mode = run_time / len(self._harmonics)

        for harmonic in self._harmonics:
            anims.append(Create(harmonic, run_time=time_per_mode))

        return Succession(*anims)


class ArcLengthTracer(VGroup):
    """
    Traces the arc length of the lemniscate.

    Shows how the total arc length relates to G*.

    Parameters
    ----------
    scale : float
        Lemniscate scale
    """

    def __init__(
        self,
        scale: float = 2.0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.scale = scale
        self._arc_length = 0.0

        # The lemniscate curve
        self.curve = LemniscateWithGlow(scale=scale)
        self.add(self.curve)

        # Tracing dot
        self.tracer = Dot(
            point=self._get_point(0),
            radius=0.12,
            color=TRD_COLORS["glow"],
        )
        # Glow around tracer
        self.tracer_glow = Circle(
            radius=0.25,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=2,
            stroke_opacity=0.5,
            fill_opacity=0,
        )
        self.tracer_glow.move_to(self.tracer.get_center())
        self.add(self.tracer_glow)
        self.add(self.tracer)

        # Arc length display
        self.arc_label = Text(
            "Arc Length: ",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        self.arc_value = DecimalNumber(
            0,
            num_decimal_places=4,
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        self.arc_label.to_corner(UP + LEFT, buff=0.5)
        self.arc_value.next_to(self.arc_label, RIGHT, buff=0.1)
        self.add(self.arc_label)
        self.add(self.arc_value)

    def _get_point(self, t: float) -> np.ndarray:
        """Get point on lemniscate at parameter t."""
        denom = 1 + np.sin(t) ** 2
        x = self.scale * np.cos(t) / denom
        y = self.scale * np.sin(t) * np.cos(t) / denom
        return np.array([x, y, 0])

    def _compute_arc_length(self, t: float, num_segments: int = 1000) -> float:
        """Numerically compute arc length from 0 to t."""
        if t <= 0:
            return 0.0

        ts = np.linspace(0, t, num_segments)
        points = np.array([self._get_point(ti) for ti in ts])

        # Sum of segment lengths
        diffs = np.diff(points, axis=0)
        lengths = np.linalg.norm(diffs, axis=1)
        return np.sum(lengths)

    def trace_animation(self, run_time: float = 4.0) -> Animation:
        """
        Animate tracing the full lemniscate and computing arc length.

        Parameters
        ----------
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Tracing animation
        """
        def update_tracer(mob, alpha):
            t = alpha * TAU
            point = self._get_point(t)
            self.tracer.move_to(point)
            self.tracer_glow.move_to(point)

            # Update arc length
            arc = self._compute_arc_length(t)
            self.arc_value.set_value(arc)

        return UpdateFromAlphaFunc(self, update_tracer, run_time=run_time)


class GStarReveal(VGroup):
    """
    Animation revealing the lemniscatic constant G* and its connection to alpha.

    Shows:
    1. The lemniscate curve
    2. Arc length computation
    3. G* = varpi = 2.622...
    4. Connection to fine structure constant

    Parameters
    ----------
    scale : float
        Lemniscate scale
    """

    def __init__(
        self,
        scale: float = 2.0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.scale = scale

        # Main lemniscate
        self.lemniscate = LemniscateWithGlow(scale=scale)
        self.add(self.lemniscate)

        # G* label
        self.g_star_label = MathTex(
            r"G^* = \varpi = \frac{\sqrt{2} \, \Gamma(1/4)^2}{2\pi}",
            color=TRD_COLORS["text"],
            font_size=32,
        )
        self.g_star_label.to_edge(DOWN, buff=1.5)

        # G* value
        self.g_star_value = MathTex(
            r"\approx 2.6221",
            color=TRD_COLORS["highlight"],
            font_size=36,
        )
        self.g_star_value.next_to(self.g_star_label, DOWN, buff=0.3)

        # Master quadratic
        self.quadratic = MathTex(
            r"x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        self.quadratic.to_edge(UP, buff=1.0)

        # Roots
        self.root_plus = MathTex(
            r"x_+ = 137.036 \approx \frac{1}{\alpha}",
            color=TRD_COLORS["matter"],
            font_size=24,
        )
        self.root_minus = MathTex(
            r"x_- = 3.024 \approx N_c",
            color=TRD_COLORS["antimatter"],
            font_size=24,
        )
        self.root_plus.next_to(self.quadratic, DOWN, buff=0.4)
        self.root_minus.next_to(self.root_plus, DOWN, buff=0.2)

    def reveal_sequence(self, run_time: float = 8.0) -> Animation:
        """
        Full reveal animation sequence.

        Returns
        -------
        Animation
            Complete reveal animation
        """
        return Succession(
            Create(self.lemniscate, run_time=run_time * 0.25),
            Write(self.g_star_label, run_time=run_time * 0.15),
            Write(self.g_star_value, run_time=run_time * 0.1),
            Write(self.quadratic, run_time=run_time * 0.15),
            Write(self.root_plus, run_time=run_time * 0.15),
            Write(self.root_minus, run_time=run_time * 0.2),
        )


class LemniscateAlphaConnection(VGroup):
    """
    Comprehensive visualization showing how the lemniscate
    connects to the fine structure constant.

    This is the key animation for Chapter 1.9 (Constants).
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.scale = 1.8

        # Build components
        self._build_visualization()

    def _build_visualization(self):
        """Construct the complete visualization."""
        # Main lemniscate (center-left)
        self.lemniscate = LemniscateWithGlow(
            scale=self.scale,
            color=TRD_COLORS["highlight"],
        )
        self.lemniscate.shift(LEFT * 2.5)
        self.add(self.lemniscate)

        # Equation panel (right side)
        self.equations = VGroup()

        # Title
        title = Text(
            "The Lemniscatic Constant",
            color=TRD_COLORS["text"],
            font_size=28,
            weight="BOLD",
        )
        title.to_edge(UP, buff=0.5)
        self.equations.add(title)

        # G* definition
        g_def = MathTex(
            r"G^* = \frac{\sqrt{2} \, \Gamma(1/4)^2}{2\pi} \approx 2.6221",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        g_def.next_to(title, DOWN, buff=0.5)
        self.equations.add(g_def)

        # Master quadratic
        quad = MathTex(
            r"x^2 - 16c^2 x + 16c^3 = 0",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        quad.next_to(g_def, DOWN, buff=0.4)
        self.equations.add(quad)

        # Where c = G*
        where = MathTex(
            r"\text{where } c = G^*",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        where.next_to(quad, DOWN, buff=0.2)
        self.equations.add(where)

        # Roots box
        roots_box = VGroup()

        root1 = MathTex(
            r"x_+ = 137.0360",
            color=TRD_COLORS["matter"],
            font_size=22,
        )
        root1_label = MathTex(
            r"\rightarrow \frac{1}{\alpha}",
            color=TRD_COLORS["matter"],
            font_size=18,
        )
        root1_label.next_to(root1, RIGHT, buff=0.2)

        root2 = MathTex(
            r"x_- = 3.024",
            color=TRD_COLORS["antimatter"],
            font_size=22,
        )
        root2_label = MathTex(
            r"\rightarrow N_c",
            color=TRD_COLORS["antimatter"],
            font_size=18,
        )
        root2_label.next_to(root2, RIGHT, buff=0.2)

        roots_box.add(root1, root1_label, root2, root2_label)
        root2.next_to(root1, DOWN, buff=0.3, aligned_edge=LEFT)
        root2_label.next_to(root2, RIGHT, buff=0.2)

        roots_box.next_to(where, DOWN, buff=0.5)
        self.equations.add(roots_box)

        # Position equations panel
        self.equations.shift(RIGHT * 2)
        self.add(self.equations)

        # Connecting arrow
        arrow = Line(
            self.lemniscate.get_right() + RIGHT * 0.3,
            g_def.get_left() + LEFT * 0.3,
            color=TRD_COLORS["grid_bright"],
            stroke_width=2,
        )
        self.add(arrow)

    def animate_connection(self, run_time: float = 6.0) -> Animation:
        """
        Animate revealing the connection.

        Returns
        -------
        Animation
            Connection reveal animation
        """
        return Succession(
            Create(self.lemniscate, run_time=run_time * 0.4),
            AnimationGroup(
                *[Write(eq, run_time=run_time * 0.1) for eq in self.equations],
                lag_ratio=0.3,
            ),
        )


class RotatingLemniscate(VGroup):
    """
    A lemniscate that can rotate to show 3D structure.

    Used for visualizing how the lemniscate relates to
    the elliptic curve structure in TRD.
    """

    def __init__(
        self,
        scale: float = 2.0,
        color: str = TRD_COLORS["highlight"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.scale = scale
        self.color = color
        self._angle = 0.0

        self._build_lemniscate()

    def _build_lemniscate(self):
        """Build the rotatable lemniscate."""
        self.submobjects.clear()

        def rotated_lemniscate(t: float) -> np.ndarray:
            # Base lemniscate
            denom = 1 + np.sin(t) ** 2
            x = self.scale * np.cos(t) / denom
            y = self.scale * np.sin(t) * np.cos(t) / denom

            # Apply rotation around Y axis
            cos_a = np.cos(self._angle)
            sin_a = np.sin(self._angle)

            x_rot = x * cos_a
            z = x * sin_a  # This becomes depth

            # Simple depth cue: slight vertical offset based on z
            y_offset = z * 0.1

            return np.array([x_rot, y + y_offset, 0])

        # Glow layers
        for i in range(3, 0, -1):
            glow = ParametricFunction(
                rotated_lemniscate,
                t_range=[0, TAU, 0.01],
                color=self.color,
                stroke_width=3 + i * 3,
                stroke_opacity=0.1 / i,
            )
            self.add(glow)

        # Main curve
        main = ParametricFunction(
            rotated_lemniscate,
            t_range=[0, TAU, 0.01],
            color=self.color,
            stroke_width=3,
        )
        self.add(main)

    def rotate_animation(
        self,
        angle: float = TAU,
        run_time: float = 4.0,
    ) -> Animation:
        """
        Animate rotation around the Y axis.

        Parameters
        ----------
        angle : float
            Total rotation angle
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Rotation animation
        """
        def update_rotation(mob, alpha):
            self._angle = alpha * angle
            self._build_lemniscate()

        return UpdateFromAlphaFunc(self, update_rotation, run_time=run_time)
