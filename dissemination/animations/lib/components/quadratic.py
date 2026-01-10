"""
Master Quadratic Mobject
========================

Visualization of the TRD master quadratic equation.
Shows the parabola with roots at x₊ = 137.036 (1/α) and x₋ = 3.024 (N_c).
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
    Succession,
    Create,
    FadeIn,
    FadeOut,
    Write,
    GrowFromPoint,
    UpdateFromAlphaFunc,
    VGroup,
    VMobject,
    FunctionGraph,
    NumberPlane,
    Axes,
    Line,
    DashedLine,
    Dot,
    Circle,
    Text,
    MathTex,
    DecimalNumber,
    Arrow,
    rate_functions,
)

from ..colors import TRD_COLORS, GLOW_COLORS, lerp_color


# The lemniscatic constant
G_STAR = 2.6220575542921198

# Master quadratic coefficients
# x² - 16c²x + 16c³ = 0 where c = G*
A_COEFF = 1.0
B_COEFF = -16 * G_STAR ** 2  # ≈ -109.86
C_COEFF = 16 * G_STAR ** 3   # ≈ 287.86

# Roots
X_PLUS = 137.0360  # ≈ 1/α
X_MINUS = 3.024    # ≈ N_c


def master_quadratic(x: float) -> float:
    """Evaluate the master quadratic at x."""
    return x**2 + B_COEFF * x + C_COEFF


def find_vertex() -> tuple[float, float]:
    """Find the vertex of the parabola."""
    x_vertex = -B_COEFF / (2 * A_COEFF)
    y_vertex = master_quadratic(x_vertex)
    return x_vertex, y_vertex


class QuadraticCurve(VGroup):
    """
    The master quadratic curve with glow effects.

    Parameters
    ----------
    x_range : tuple
        (x_min, x_max) for plotting
    color : str
        Curve color
    show_glow : bool
        Whether to add glow effect
    """

    def __init__(
        self,
        x_range: tuple[float, float] = (-10, 150),
        color: str = TRD_COLORS["highlight"],
        show_glow: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.x_min, self.x_max = x_range
        self.color = color

        self._build_curve(show_glow)

    def _build_curve(self, show_glow: bool):
        """Construct the quadratic curve."""
        # Create axes-appropriate scaling
        x_vertex, y_vertex = find_vertex()

        # Scale function to fit visualization
        def scaled_quadratic(x: float) -> float:
            # Map from display x to actual x
            actual_x = x * 15 + 70  # Center around vertex
            y = master_quadratic(actual_x)
            # Scale y for display
            return y / 500

        # Glow layers
        if show_glow:
            for i in range(3, 0, -1):
                glow = FunctionGraph(
                    scaled_quadratic,
                    x_range=[-5, 5, 0.05],
                    color=self.color,
                    stroke_width=3 + i * 3,
                    stroke_opacity=0.1 / i,
                )
                self.add(glow)

        # Main curve
        curve = FunctionGraph(
            scaled_quadratic,
            x_range=[-5, 5, 0.05],
            color=self.color,
            stroke_width=3,
        )
        self.add(curve)


class MasterQuadraticDiagram(VGroup):
    """
    Complete visualization of the master quadratic equation.

    Shows:
    - The parabola
    - Both roots with labels
    - Vertex
    - Equation

    Parameters
    ----------
    show_labels : bool
        Whether to show root labels
    show_equation : bool
        Whether to show the equation
    """

    def __init__(
        self,
        show_labels: bool = True,
        show_equation: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)

        self._build_diagram(show_labels, show_equation)

    def _build_diagram(self, show_labels: bool, show_equation: bool):
        """Construct the complete diagram."""
        # Create custom axes
        self.axes = Axes(
            x_range=[-10, 150, 20],
            y_range=[-1500, 500, 500],
            x_length=10,
            y_length=5,
            axis_config={
                "color": TRD_COLORS["grid_bright"],
                "stroke_width": 1,
                "include_tip": True,
                "tip_length": 0.2,
            },
            x_axis_config={
                "numbers_to_include": [0, 50, 100, 137],
            },
            y_axis_config={
                "numbers_to_include": [-1000, 0],
            },
        )
        self.axes.shift(DOWN * 0.5)
        self.add(self.axes)

        # Plot the quadratic
        curve = self.axes.plot(
            master_quadratic,
            x_range=[0, 145, 0.5],
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        self.curve = curve

        # Glow for curve
        for i in range(3, 0, -1):
            glow = self.axes.plot(
                master_quadratic,
                x_range=[0, 145, 0.5],
                color=TRD_COLORS["highlight"],
                stroke_width=3 + i * 3,
                stroke_opacity=0.1 / i,
            )
            self.add(glow)
        self.add(curve)

        # Root markers
        # x₋ ≈ 3.024
        root_minus_point = self.axes.c2p(X_MINUS, 0)
        self.root_minus_dot = Dot(
            root_minus_point,
            radius=0.12,
            color=TRD_COLORS["antimatter"],
        )
        self.root_minus_glow = Circle(
            radius=0.25,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=2,
            stroke_opacity=0.5,
            fill_opacity=0,
        ).move_to(root_minus_point)
        self.add(self.root_minus_glow)
        self.add(self.root_minus_dot)

        # x₊ ≈ 137.036
        root_plus_point = self.axes.c2p(X_PLUS, 0)
        self.root_plus_dot = Dot(
            root_plus_point,
            radius=0.12,
            color=TRD_COLORS["matter"],
        )
        self.root_plus_glow = Circle(
            radius=0.25,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=2,
            stroke_opacity=0.5,
            fill_opacity=0,
        ).move_to(root_plus_point)
        self.add(self.root_plus_glow)
        self.add(self.root_plus_dot)

        # Vertex
        x_v, y_v = find_vertex()
        vertex_point = self.axes.c2p(x_v, y_v)
        self.vertex_dot = Dot(
            vertex_point,
            radius=0.08,
            color=TRD_COLORS["text_dim"],
        )
        self.add(self.vertex_dot)

        # Vertical lines to x-axis
        self.root_minus_line = DashedLine(
            self.axes.c2p(X_MINUS, -1400),
            root_minus_point,
            color=TRD_COLORS["antimatter"],
            stroke_width=1,
            stroke_opacity=0.5,
        )
        self.root_plus_line = DashedLine(
            self.axes.c2p(X_PLUS, -1400),
            root_plus_point,
            color=TRD_COLORS["matter"],
            stroke_width=1,
            stroke_opacity=0.5,
        )
        self.add(self.root_minus_line)
        self.add(self.root_plus_line)

        # Labels
        if show_labels:
            self.root_minus_label = MathTex(
                r"x_- \approx 3.02",
                color=TRD_COLORS["antimatter"],
                font_size=20,
            )
            self.root_minus_label.next_to(self.root_minus_dot, DOWN + LEFT, buff=0.2)

            self.nc_label = MathTex(
                r"\rightarrow N_c",
                color=TRD_COLORS["antimatter"],
                font_size=18,
            )
            self.nc_label.next_to(self.root_minus_label, DOWN, buff=0.1)

            self.root_plus_label = MathTex(
                r"x_+ \approx 137.04",
                color=TRD_COLORS["matter"],
                font_size=20,
            )
            self.root_plus_label.next_to(self.root_plus_dot, UP + RIGHT, buff=0.2)

            self.alpha_label = MathTex(
                r"\rightarrow \frac{1}{\alpha}",
                color=TRD_COLORS["matter"],
                font_size=18,
            )
            self.alpha_label.next_to(self.root_plus_label, DOWN, buff=0.1)

            self.add(self.root_minus_label, self.nc_label)
            self.add(self.root_plus_label, self.alpha_label)

        # Equation
        if show_equation:
            self.equation = MathTex(
                r"x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0",
                color=TRD_COLORS["text"],
                font_size=28,
            )
            self.equation.to_edge(UP, buff=0.5)
            self.add(self.equation)

            self.g_star_note = MathTex(
                r"G^* \approx 2.6221",
                color=TRD_COLORS["highlight"],
                font_size=20,
            )
            self.g_star_note.next_to(self.equation, DOWN, buff=0.2)
            self.add(self.g_star_note)


class QuadraticRootExplorer(VGroup):
    """
    Interactive exploration of the quadratic roots.

    Allows animating through different coefficient values
    to show how the roots change.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._c_value = G_STAR
        self._build_explorer()

    def _build_explorer(self):
        """Build the explorer interface."""
        # Title
        self.title = Text(
            "Master Quadratic Explorer",
            color=TRD_COLORS["text"],
            font_size=24,
            weight="BOLD",
        )
        self.title.to_edge(UP, buff=0.3)
        self.add(self.title)

        # Equation template
        self.equation = MathTex(
            r"x^2 - 16c^2 x + 16c^3 = 0",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        self.equation.next_to(self.title, DOWN, buff=0.3)
        self.add(self.equation)

        # c value display
        self.c_label = Text("c = ", color=TRD_COLORS["text"], font_size=20)
        self.c_value = DecimalNumber(
            G_STAR,
            num_decimal_places=4,
            color=TRD_COLORS["highlight"],
            font_size=20,
        )
        self.c_label.next_to(self.equation, DOWN, buff=0.3)
        self.c_value.next_to(self.c_label, RIGHT, buff=0.1)
        self.add(self.c_label, self.c_value)

        # Root displays
        self.root_plus_display = VGroup()
        rp_label = MathTex(r"x_+ = ", color=TRD_COLORS["matter"], font_size=20)
        self.rp_value = DecimalNumber(
            X_PLUS,
            num_decimal_places=4,
            color=TRD_COLORS["matter"],
            font_size=20,
        )
        rp_label.next_to(self.c_value, DOWN + LEFT, buff=0.4)
        self.rp_value.next_to(rp_label, RIGHT, buff=0.1)
        self.root_plus_display.add(rp_label, self.rp_value)
        self.add(self.root_plus_display)

        self.root_minus_display = VGroup()
        rm_label = MathTex(r"x_- = ", color=TRD_COLORS["antimatter"], font_size=20)
        self.rm_value = DecimalNumber(
            X_MINUS,
            num_decimal_places=4,
            color=TRD_COLORS["antimatter"],
            font_size=20,
        )
        rm_label.next_to(rp_label, DOWN, buff=0.2)
        self.rm_value.next_to(rm_label, RIGHT, buff=0.1)
        self.root_minus_display.add(rm_label, self.rm_value)
        self.add(self.root_minus_display)

    def _compute_roots(self, c: float) -> tuple[float, float]:
        """Compute roots for given c value."""
        b = -16 * c**2
        cc = 16 * c**3
        discriminant = b**2 - 4 * cc
        if discriminant < 0:
            return 0.0, 0.0
        sqrt_d = np.sqrt(discriminant)
        x_plus = (-b + sqrt_d) / 2
        x_minus = (-b - sqrt_d) / 2
        return x_plus, x_minus

    def vary_c_animation(
        self,
        c_start: float = 2.0,
        c_end: float = 3.0,
        run_time: float = 4.0,
    ) -> Animation:
        """
        Animate varying the c parameter.

        Parameters
        ----------
        c_start : float
            Starting c value
        c_end : float
            Ending c value
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Parameter variation animation
        """
        def update_values(mob, alpha):
            c = c_start + alpha * (c_end - c_start)
            self.c_value.set_value(c)
            x_plus, x_minus = self._compute_roots(c)
            self.rp_value.set_value(x_plus)
            self.rm_value.set_value(x_minus)

        return UpdateFromAlphaFunc(self, update_values, run_time=run_time)


class QuadraticDerivation(VGroup):
    """
    Step-by-step derivation of the master quadratic.

    Shows how the equation arises from TRD constraints.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._build_derivation()

    def _build_derivation(self):
        """Build the derivation steps."""
        # Step 1: Gauss constraint
        self.step1_title = Text(
            "1. Gauss Constraint",
            color=TRD_COLORS["text"],
            font_size=20,
            weight="BOLD",
        )
        self.step1_eq = MathTex(
            r"\nabla \cdot J = \rho \implies 16 \text{ DoF}",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )

        # Step 2: Elliptic structure
        self.step2_title = Text(
            "2. Elliptic Integral",
            color=TRD_COLORS["text"],
            font_size=20,
            weight="BOLD",
        )
        self.step2_eq = MathTex(
            r"K(1/\sqrt{2}) = \frac{\Gamma(1/4)^2}{4\sqrt{\pi}}",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )

        # Step 3: Lemniscatic constant
        self.step3_title = Text(
            "3. Lemniscatic Constant",
            color=TRD_COLORS["text"],
            font_size=20,
            weight="BOLD",
        )
        self.step3_eq = MathTex(
            r"G^* = \frac{\sqrt{2} \Gamma(1/4)^2}{2\pi} \approx 2.6221",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )

        # Step 4: Master equation
        self.step4_title = Text(
            "4. Master Quadratic",
            color=TRD_COLORS["text"],
            font_size=20,
            weight="BOLD",
        )
        self.step4_eq = MathTex(
            r"x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0",
            color=TRD_COLORS["highlight"],
            font_size=22,
        )

        # Step 5: Physical roots
        self.step5_title = Text(
            "5. Physical Constants",
            color=TRD_COLORS["text"],
            font_size=20,
            weight="BOLD",
        )
        self.step5_eq = MathTex(
            r"x_+ = 137.036 = \frac{1}{\alpha}, \quad x_- = 3.02 \approx N_c",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )

        # Arrange vertically
        steps = [
            (self.step1_title, self.step1_eq),
            (self.step2_title, self.step2_eq),
            (self.step3_title, self.step3_eq),
            (self.step4_title, self.step4_eq),
            (self.step5_title, self.step5_eq),
        ]

        y_offset = 2.5
        for title, eq in steps:
            title.move_to(UP * y_offset + LEFT * 2)
            eq.next_to(title, RIGHT, buff=0.3)
            self.add(title, eq)
            y_offset -= 1.0

    def reveal_animation(self, run_time: float = 10.0) -> Animation:
        """
        Animate revealing the derivation step by step.

        Returns
        -------
        Animation
            Step-by-step reveal animation
        """
        steps = [
            self.step1_title, self.step1_eq,
            self.step2_title, self.step2_eq,
            self.step3_title, self.step3_eq,
            self.step4_title, self.step4_eq,
            self.step5_title, self.step5_eq,
        ]

        anims = [Write(step, run_time=run_time / len(steps)) for step in steps]
        return Succession(*anims)


class AlphaHighlight(VGroup):
    """
    Highlight visualization for the fine structure constant.

    Shows α = 1/137.036 with visual emphasis.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Main value
        self.alpha_eq = MathTex(
            r"\alpha = \frac{1}{137.036}",
            color=TRD_COLORS["matter"],
            font_size=48,
        )

        # Glow circle
        self.glow = Circle(
            radius=1.5,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=3,
            stroke_opacity=0.3,
            fill_opacity=0,
        )
        self.glow.move_to(self.alpha_eq.get_center())

        # Outer glow layers
        for i in range(3, 0, -1):
            outer = Circle(
                radius=1.5 + i * 0.3,
                stroke_color=TRD_COLORS["matter"],
                stroke_width=2,
                stroke_opacity=0.1 / i,
                fill_opacity=0,
            )
            outer.move_to(self.alpha_eq.get_center())
            self.add(outer)

        self.add(self.glow)
        self.add(self.alpha_eq)

        # Subtitle
        self.subtitle = Text(
            "Fine Structure Constant",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        self.subtitle.next_to(self.alpha_eq, DOWN, buff=0.5)
        self.add(self.subtitle)

        # Accuracy note
        self.accuracy = Text(
            "1.26 ppm from experiment",
            color=TRD_COLORS["text_dim"],
            font_size=16,
        )
        self.accuracy.next_to(self.subtitle, DOWN, buff=0.2)
        self.add(self.accuracy)


class NcHighlight(VGroup):
    """
    Highlight visualization for the color charge number.

    Shows N_c ≈ 3 with visual emphasis.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Main value
        self.nc_eq = MathTex(
            r"N_c \approx 3",
            color=TRD_COLORS["antimatter"],
            font_size=48,
        )

        # Glow circle
        self.glow = Circle(
            radius=1.2,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=3,
            stroke_opacity=0.3,
            fill_opacity=0,
        )
        self.glow.move_to(self.nc_eq.get_center())

        # Outer glow layers
        for i in range(3, 0, -1):
            outer = Circle(
                radius=1.2 + i * 0.3,
                stroke_color=TRD_COLORS["antimatter"],
                stroke_width=2,
                stroke_opacity=0.1 / i,
                fill_opacity=0,
            )
            outer.move_to(self.nc_eq.get_center())
            self.add(outer)

        self.add(self.glow)
        self.add(self.nc_eq)

        # Subtitle
        self.subtitle = Text(
            "Color Charges (QCD)",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        self.subtitle.next_to(self.nc_eq, DOWN, buff=0.5)
        self.add(self.subtitle)

        # Note
        self.note = Text(
            "x₋ = 3.024 → truncates to 3",
            color=TRD_COLORS["text_dim"],
            font_size=16,
        )
        self.note.next_to(self.subtitle, DOWN, buff=0.2)
        self.add(self.note)
