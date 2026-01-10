"""
Chapter 1.8: The Four Forces
============================

Visualization of how TRD generates force-like behaviors.
Compares gravitational, electromagnetic, strong, and weak forces.
"""

from __future__ import annotations

import numpy as np

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    PI,
    TAU,
    Create,
    FadeIn,
    FadeOut,
    Write,
    AnimationGroup,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Text,
    MathTex,
    RoundedRectangle,
    Axes,
    FunctionGraph,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, FORCE_COLORS


class ForcesIntro(TRDScene):
    """Introduction to forces in TRD."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.0.1", "title")
        title = self.trd_title("The Four Forces")
        subtitle = Text(
            "Emergent Interactions from Flux Dynamics",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Force preview
        self.add_marker("1.8.0.2", "preview")

        forces = [
            ("Gravity", FORCE_COLORS["gravity"], "∇ρ"),
            ("Electromagnetic", FORCE_COLORS["electromagnetic"], "∇q, ∇×J"),
            ("Strong", FORCE_COLORS["strong"], "Yukawa"),
            ("Weak", FORCE_COLORS["weak"], "Transmutation"),
        ]

        force_mobs = VGroup()
        for name, color, mechanism in forces:
            box = RoundedRectangle(
                width=2.5, height=1.2,
                corner_radius=0.1,
                stroke_color=color,
                stroke_width=2,
                fill_color=TRD_COLORS["background_light"],
                fill_opacity=0.5,
            )
            label = Text(name, color=color, font_size=20, weight="BOLD")
            mech = Text(mechanism, color=TRD_COLORS["text_dim"], font_size=14)
            label.move_to(box.get_center() + UP * 0.2)
            mech.move_to(box.get_center() + DOWN * 0.25)
            force_mobs.add(VGroup(box, label, mech))

        force_mobs.arrange(RIGHT, buff=0.3)
        force_mobs.scale(0.9)

        for f in force_mobs:
            self.play(FadeIn(f), run_time=0.5)

        self.wait(2)
        self.play(FadeOut(force_mobs))

        self.export_markers()


class GravityForce(TRDScene):
    """Gravity-like behavior from density gradients."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.1.1", "gravity")

        title = self.concept_card(
            "Gravity",
            "Attraction from density gradients"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Equation
        eq = MathTex(
            r"F_{grav} = G_N \cdot \nabla \bar{\rho}",
            color=FORCE_COLORS["gravity"],
            font_size=36,
        )
        eq.to_edge(UP, buff=1.0)
        self.play(Write(eq))

        # Visualization: two masses attracting
        self.add_marker("1.8.1.2", "attraction")

        mass1 = Circle(
            radius=0.4,
            fill_color=FORCE_COLORS["gravity"],
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        mass1.shift(LEFT * 2)

        mass2 = Circle(
            radius=0.6,
            fill_color=FORCE_COLORS["gravity"],
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        mass2.shift(RIGHT * 2)

        # Arrows showing attraction
        arrow1 = Arrow(
            mass1.get_right() + RIGHT * 0.2,
            mass1.get_right() + RIGHT * 1.0,
            color=FORCE_COLORS["gravity"],
            stroke_width=3,
        )
        arrow2 = Arrow(
            mass2.get_left() + LEFT * 0.2,
            mass2.get_left() + LEFT * 1.0,
            color=FORCE_COLORS["gravity"],
            stroke_width=3,
        )

        self.play(FadeIn(mass1), FadeIn(mass2))
        self.play(Create(arrow1), Create(arrow2))

        # Description
        desc = Text(
            "Density gradients pull toward higher flux concentration",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        desc.to_edge(DOWN, buff=1.0)
        self.play(Write(desc))

        # Range note
        range_note = Text(
            "Long-range: 1/r² from 3D geometry",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        range_note.next_to(desc, DOWN, buff=0.2)
        self.play(Write(range_note))

        self.wait(2)
        self.play(
            FadeOut(eq), FadeOut(mass1), FadeOut(mass2),
            FadeOut(arrow1), FadeOut(arrow2),
            FadeOut(desc), FadeOut(range_note),
        )

        self.export_markers()


class EMForce(TRDScene):
    """Electromagnetic-like behavior."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.2.1", "em")

        title = self.concept_card(
            "Electromagnetic",
            "Electric and magnetic from charge and curl"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Equations
        eq_elec = MathTex(
            r"F_{elec} = -q \cdot \nabla \bar{q}",
            color=FORCE_COLORS["electromagnetic"],
            font_size=28,
        )
        eq_mag = MathTex(
            r"F_{mag} = \beta (\nabla \times J) \times \hat{J}",
            color=FORCE_COLORS["electromagnetic"],
            font_size=28,
        )

        eqs = VGroup(eq_elec, eq_mag)
        eqs.arrange(DOWN, buff=0.3)
        eqs.to_edge(UP, buff=1.0)

        self.play(Write(eq_elec))
        self.play(Write(eq_mag))

        # Visualization: opposite charges
        self.add_marker("1.8.2.2", "charges")

        pos_charge = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        pos_label = Text("+", color=TRD_COLORS["background"], font_size=24, weight="BOLD")
        pos_label.move_to(pos_charge.get_center())
        pos_group = VGroup(pos_charge, pos_label)
        pos_group.shift(LEFT * 2)

        neg_charge = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        neg_label = Text("-", color=TRD_COLORS["background"], font_size=24, weight="BOLD")
        neg_label.move_to(neg_charge.get_center())
        neg_group = VGroup(neg_charge, neg_label)
        neg_group.shift(RIGHT * 2)

        # Attraction arrows
        attract_arrow = Arrow(
            pos_group.get_right() + RIGHT * 0.2,
            neg_group.get_left() + LEFT * 0.2,
            color=FORCE_COLORS["electromagnetic"],
            stroke_width=2,
        )
        attract_label = Text("Attract", color=TRD_COLORS["text"], font_size=16)
        attract_label.next_to(attract_arrow, UP, buff=0.1)

        self.play(FadeIn(pos_group), FadeIn(neg_group))
        self.play(Create(attract_arrow), Write(attract_label))

        # Like charges repel
        self.add_marker("1.8.2.3", "repel")

        pos2 = pos_group.copy()
        pos2.shift(DOWN * 2 + RIGHT * 4)

        pos3 = pos_group.copy()
        pos3.shift(DOWN * 2)

        repel_arrows = VGroup(
            Arrow(
                pos3.get_right() + RIGHT * 0.2,
                pos3.get_right() + RIGHT * 1.0,
                color=FORCE_COLORS["electromagnetic"],
                stroke_width=2,
            ),
            Arrow(
                pos2.get_left() + LEFT * 0.2,
                pos2.get_left() + LEFT * 1.0,
                color=FORCE_COLORS["electromagnetic"],
                stroke_width=2,
            ),
        )
        repel_label = Text("Repel", color=TRD_COLORS["text"], font_size=16)
        repel_label.move_to((pos3.get_center() + pos2.get_center()) / 2 + DOWN * 0.5)

        self.play(FadeIn(pos2), FadeIn(pos3))
        self.play(Create(repel_arrows), Write(repel_label))

        self.wait(2)

        self.export_markers()


class StrongForce(TRDScene):
    """Strong force with Yukawa form."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.3.1", "strong")

        title = self.concept_card(
            "Strong Force",
            "Short-range binding via Yukawa potential"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Yukawa equation
        eq = MathTex(
            r"F_{strong}(r) = g_s^2 \frac{e^{-m_\pi r}}{r^2}(1 + m_\pi r)",
            color=FORCE_COLORS["strong"],
            font_size=28,
        )
        eq.to_edge(UP, buff=1.0)
        self.play(Write(eq))

        # Plot the Yukawa potential
        self.add_marker("1.8.3.2", "yukawa_plot")

        axes = Axes(
            x_range=[0.5, 4, 1],
            y_range=[0, 1, 0.2],
            x_length=6,
            y_length=3,
            axis_config={"color": TRD_COLORS["grid_bright"]},
        )
        axes.shift(DOWN * 0.5)

        x_label = Text("r (fm)", color=TRD_COLORS["text"], font_size=16)
        x_label.next_to(axes.x_axis, RIGHT)
        y_label = Text("F", color=TRD_COLORS["text"], font_size=16)
        y_label.next_to(axes.y_axis, UP)

        def yukawa(r):
            m_pi = 1.0
            if r < 0.5:
                return 1.0
            return np.exp(-m_pi * r) / r**2 * (1 + m_pi * r)

        curve = axes.plot(
            yukawa,
            x_range=[0.5, 4],
            color=FORCE_COLORS["strong"],
        )

        self.play(Create(axes), Write(x_label), Write(y_label))
        self.play(Create(curve))

        # Annotation
        short_range = Text(
            "Rapid exponential decay",
            color=FORCE_COLORS["strong"],
            font_size=18,
        )
        short_range.to_edge(DOWN, buff=0.8)
        self.play(Write(short_range))

        # Confinement note
        confine = Text(
            "Binds quarks into triads (nucleons)",
            color=TRD_COLORS["text_dim"],
            font_size=16,
        )
        confine.next_to(short_range, DOWN, buff=0.2)
        self.play(Write(confine))

        self.wait(2)

        self.export_markers()


class WeakForce(TRDScene):
    """Weak force via transmutation."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.4.1", "weak")

        title = self.concept_card(
            "Weak Force",
            "Polarity flips under high stress"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Stress equation
        eq = MathTex(
            r"\text{stress} = |\nabla \cdot J| + |\nabla \times J| + |\nabla \rho|",
            color=FORCE_COLORS["weak"],
            font_size=28,
        )
        eq.to_edge(UP, buff=1.0)
        self.play(Write(eq))

        # Threshold condition
        threshold = MathTex(
            r"\text{stress} > \text{WEAK\_THRESHOLD} \implies \text{transmutation}",
            color=FORCE_COLORS["weak"],
            font_size=24,
        )
        threshold.next_to(eq, DOWN, buff=0.3)
        self.play(Write(threshold))

        # Visualization: transmutation
        self.add_marker("1.8.4.2", "transmutation")

        before = VGroup()
        plus = Circle(radius=0.3, fill_color=TRD_COLORS["matter"], fill_opacity=0.9, stroke_width=0)
        plus_label = Text("+1", color=TRD_COLORS["background"], font_size=16)
        plus_label.move_to(plus.get_center())
        before.add(plus, plus_label)
        before.shift(LEFT * 2)

        arrow = Arrow(LEFT * 0.8, RIGHT * 0.8, color=FORCE_COLORS["weak"], stroke_width=3)
        stress_label = Text("High stress", color=FORCE_COLORS["weak"], font_size=14)
        stress_label.next_to(arrow, UP, buff=0.1)

        after = VGroup()
        minus = Circle(radius=0.3, fill_color=TRD_COLORS["antimatter"], fill_opacity=0.9, stroke_width=0)
        minus_label = Text("-1", color=TRD_COLORS["background"], font_size=16)
        minus_label.move_to(minus.get_center())
        after.add(minus, minus_label)
        after.shift(RIGHT * 2)

        transmute_group = VGroup(before, arrow, stress_label, after)
        transmute_group.shift(DOWN * 0.5)

        self.play(FadeIn(before))
        self.play(Create(arrow), Write(stress_label))
        self.play(FadeIn(after))

        # Beta decay analog
        beta = Text(
            "Analog of beta decay: n → p + e⁻ + ν̄",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        beta.to_edge(DOWN, buff=0.8)
        self.play(Write(beta))

        self.wait(2)

        self.export_markers()


class ForceComparison(TRDScene):
    """Compare all four forces."""

    def construct(self):
        self.load_narration("1.8")

        self.add_marker("1.8.5.1", "comparison")

        title = self.trd_title("Force Comparison")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Comparison table
        headers = ["Force", "Range", "Strength", "Mechanism"]
        data = [
            ("Gravity", "∞", "10⁻³⁹", "∇ρ"),
            ("EM", "∞", "α ≈ 1/137", "∇q, ∇×J"),
            ("Strong", "~1 fm", "~1", "Yukawa"),
            ("Weak", "~0.001 fm", "10⁻⁵", "Stress threshold"),
        ]

        colors = [
            FORCE_COLORS["gravity"],
            FORCE_COLORS["electromagnetic"],
            FORCE_COLORS["strong"],
            FORCE_COLORS["weak"],
        ]

        # Create table
        y_pos = 1.2
        x_positions = [-4, -1.5, 0.5, 3]

        # Headers
        for x, h in zip(x_positions, headers):
            text = Text(h, color=TRD_COLORS["highlight"], font_size=18, weight="BOLD")
            text.move_to([x, y_pos, 0])
            self.play(Write(text), run_time=0.3)

        # Data rows
        for (name, range_val, strength, mech), color in zip(data, colors):
            y_pos -= 0.65
            row_data = [name, range_val, strength, mech]
            for x, val in zip(x_positions, row_data):
                col = color if val == name else TRD_COLORS["text"]
                text = Text(val, color=col, font_size=16)
                text.move_to([x, y_pos, 0])
                self.play(Write(text), run_time=0.15)

        self.wait(2)

        # Key insight
        insight = self.equation_box(
            r"F = F_{grav} + F_{em} + F_{strong} + F_{weak}",
            "All forces from flux field operations"
        )
        insight.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(insight))

        self.wait(2)

        self.export_markers()
