"""
Chapter 1.9: The Constants
==========================

The climactic derivation of fundamental constants from TRD.
Shows the lemniscatic constant G* leading to α and N_c.
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
    Transform,
    Flash,
    Indicate,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Text,
    MathTex,
    RoundedRectangle,
    Axes,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, MODE_COLORS
from lib.components import (
    LemniscateWithGlow,
    LemniscateDecomposition,
    ArcLengthTracer,
    GStarReveal,
    LemniscateAlphaConnection,
    MasterQuadraticDiagram,
    QuadraticDerivation,
    AlphaHighlight,
    NcHighlight,
    G_STAR,
    X_PLUS,
    X_MINUS,
)


class ConstantsIntro(TRDScene):
    """Introduction to derived constants."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.0.1", "title")
        title = self.trd_title("The Constants")
        subtitle = Text(
            "From Geometry to Physics",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The big question
        self.add_marker("1.9.0.2", "question")

        question = Text(
            "Why is α ≈ 1/137?",
            color=TRD_COLORS["highlight"],
            font_size=48,
        )
        question2 = Text(
            "Why are there 3 color charges?",
            color=TRD_COLORS["antimatter"],
            font_size=36,
        )
        question2.next_to(question, DOWN, buff=0.5)

        self.play(Write(question))
        self.play(Write(question2))
        self.wait(2)
        self.play(FadeOut(question), FadeOut(question2))

        # Promise
        promise = Text(
            "TRD derives these from geometry",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        self.play(Write(promise))
        self.wait(2)
        self.play(FadeOut(promise))

        self.export_markers()


class LemniscateScene(TRDScene):
    """The lemniscate curve and its properties."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.1.1", "lemniscate")

        title = self.concept_card(
            "The Lemniscate",
            "An ancient curve with modern significance"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Draw the lemniscate
        self.add_marker("1.9.1.2", "curve")

        lemniscate = LemniscateWithGlow(
            scale=2.5,
            color=TRD_COLORS["highlight"],
        )

        self.play(Create(lemniscate, run_time=2.0))

        # Equation
        eq = MathTex(
            r"r^2 = a^2 \cos(2\theta)",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        eq.to_edge(DOWN, buff=1.5)
        self.play(Write(eq))

        # Name
        name = Text(
            "Lemniscate of Bernoulli",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        name.next_to(eq, UP, buff=0.3)
        self.play(Write(name))

        self.wait(2)

        # Why this curve?
        self.add_marker("1.9.1.3", "why")

        why = Text(
            "Why this curve?",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        why.to_edge(UP, buff=1.0)
        self.play(Write(why))

        reasons = VGroup()
        r1 = Text("• Appears in lattice regularization", color=TRD_COLORS["text"], font_size=18)
        r2 = Text("• Connected to elliptic integrals", color=TRD_COLORS["text"], font_size=18)
        r3 = Text("• Self-similar structure", color=TRD_COLORS["text"], font_size=18)

        reasons.add(r1, r2, r3)
        reasons.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        reasons.next_to(why, DOWN, buff=0.3)

        self.play(Write(reasons))

        self.wait(2)
        self.play(
            FadeOut(lemniscate), FadeOut(eq), FadeOut(name),
            FadeOut(why), FadeOut(reasons),
        )

        self.export_markers()


class GStarDerivation(TRDScene):
    """Derivation of the lemniscatic constant G*."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.2.1", "g_star")

        title = self.trd_title("The Lemniscatic Constant")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Arc length tracer
        self.add_marker("1.9.2.2", "arc_length")

        tracer = ArcLengthTracer(scale=2.0)
        tracer.shift(LEFT * 2)
        self.play(FadeIn(tracer))
        self.play(tracer.trace_animation(run_time=4.0))

        # G* formula
        self.add_marker("1.9.2.3", "formula")

        formula = MathTex(
            r"G^* = \frac{\sqrt{2} \, \Gamma(1/4)^2}{2\pi}",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        formula.shift(RIGHT * 2.5 + UP * 1)

        value = MathTex(
            r"\approx 2.6221",
            color=TRD_COLORS["highlight"],
            font_size=36,
        )
        value.next_to(formula, DOWN, buff=0.3)

        self.play(Write(formula))
        self.play(Write(value))

        # Components explanation
        components = VGroup()
        c1 = MathTex(r"\sqrt{2}", color=TRD_COLORS["text"], font_size=20)
        c1_label = Text(" ← Critical coupling", color=TRD_COLORS["text_dim"], font_size=16)
        c1_label.next_to(c1, RIGHT, buff=0.1)

        c2 = MathTex(r"\Gamma(1/4)^2", color=TRD_COLORS["text"], font_size=20)
        c2_label = Text(" ← Lattice regularization", color=TRD_COLORS["text_dim"], font_size=16)
        c2_label.next_to(c2, RIGHT, buff=0.1)

        components.add(VGroup(c1, c1_label), VGroup(c2, c2_label))
        components.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        components.next_to(value, DOWN, buff=0.5)

        self.play(Write(components))

        self.wait(2)
        self.play(FadeOut(tracer), FadeOut(formula), FadeOut(value), FadeOut(components))

        self.export_markers()


class MasterQuadraticScene(TRDScene):
    """The master quadratic equation."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.3.1", "quadratic")

        title = self.concept_card(
            "The Master Quadratic",
            "A single equation yields two constants"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Equation
        self.add_marker("1.9.3.2", "equation")

        eq = MathTex(
            r"x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0",
            color=TRD_COLORS["highlight"],
            font_size=40,
        )
        eq.to_edge(UP, buff=1.5)

        self.play(Write(eq))

        # Where this comes from
        source = Text(
            "From: 16 DoF × Gauss constraint × elliptic structure",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        source.next_to(eq, DOWN, buff=0.3)
        self.play(Write(source))

        # The quadratic diagram
        self.add_marker("1.9.3.3", "diagram")

        diagram = MasterQuadraticDiagram(show_labels=True, show_equation=False)
        diagram.scale(0.7)
        diagram.shift(DOWN * 0.5)

        self.play(Create(diagram, run_time=2.0))

        self.wait(2)
        self.play(FadeOut(eq), FadeOut(source), FadeOut(diagram))

        self.export_markers()


class TwoRoots(TRDScene):
    """Reveal the two roots."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.4.1", "roots")

        title = self.trd_title("The Two Roots")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Root x+
        self.add_marker("1.9.4.2", "x_plus")

        alpha_highlight = AlphaHighlight()
        alpha_highlight.shift(LEFT * 3)
        alpha_highlight.scale(0.8)

        self.play(FadeIn(alpha_highlight))

        # Comparison to experiment
        exp_value = MathTex(
            r"\text{Experiment: } \frac{1}{\alpha} = 137.035999...",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        exp_value.next_to(alpha_highlight, DOWN, buff=0.5)
        self.play(Write(exp_value))

        accuracy1 = Text(
            "1.26 ppm accuracy",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        accuracy1.next_to(exp_value, DOWN, buff=0.2)
        self.play(Write(accuracy1))

        # Root x-
        self.add_marker("1.9.4.3", "x_minus")

        nc_highlight = NcHighlight()
        nc_highlight.shift(RIGHT * 3)
        nc_highlight.scale(0.8)

        self.play(FadeIn(nc_highlight))

        # Note about discreteness
        nc_note = Text(
            "Truncates to exactly 3",
            color=TRD_COLORS["antimatter"],
            font_size=18,
        )
        nc_note.next_to(nc_highlight, DOWN, buff=0.5)
        self.play(Write(nc_note))

        self.wait(2)

        # Both from same equation
        self.add_marker("1.9.4.4", "unified")

        unified = Text(
            "Both from ONE equation",
            color=TRD_COLORS["glow"],
            font_size=28,
            weight="BOLD",
        )
        unified.to_edge(DOWN, buff=0.8)
        self.play(Write(unified))
        self.play(Flash(unified, color=TRD_COLORS["glow"], flash_radius=0.8))

        self.wait(2)

        self.export_markers()


class ConstantsDerivationChain(TRDScene):
    """Complete derivation chain visualization."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.5.1", "chain")

        title = self.trd_title("The Derivation Chain")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Chain of derivation
        chain = VGroup()

        steps = [
            ("3D Cubic Lattice", TRD_COLORS["text"]),
            ("↓", TRD_COLORS["grid_bright"]),
            ("Gauss Constraint", TRD_COLORS["text"]),
            ("↓", TRD_COLORS["grid_bright"]),
            ("16 Degrees of Freedom", TRD_COLORS["text"]),
            ("↓", TRD_COLORS["grid_bright"]),
            ("Elliptic Structure", TRD_COLORS["text"]),
            ("↓", TRD_COLORS["grid_bright"]),
            (r"G* = 2.6221...", TRD_COLORS["highlight"]),
            ("↓", TRD_COLORS["grid_bright"]),
            ("Master Quadratic", TRD_COLORS["text"]),
            ("↓", TRD_COLORS["grid_bright"]),
        ]

        for text, color in steps:
            if text == "↓":
                mob = Text(text, color=color, font_size=20)
            else:
                mob = Text(text, color=color, font_size=18)
            chain.add(mob)

        chain.arrange(DOWN, buff=0.12)
        chain.shift(LEFT * 0.5)

        for item in chain:
            self.play(Write(item), run_time=0.3)

        # Final results (side by side)
        results = VGroup()

        alpha_box = RoundedRectangle(
            width=3.5, height=1.2,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        alpha_text = MathTex(r"\frac{1}{\alpha} = 137.036", color=TRD_COLORS["matter"], font_size=24)
        alpha_text.move_to(alpha_box.get_center())
        alpha_group = VGroup(alpha_box, alpha_text)

        nc_box = RoundedRectangle(
            width=3.5, height=1.2,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        nc_text = MathTex(r"N_c = 3", color=TRD_COLORS["antimatter"], font_size=24)
        nc_text.move_to(nc_box.get_center())
        nc_group = VGroup(nc_box, nc_text)

        results.add(alpha_group, nc_group)
        results.arrange(RIGHT, buff=0.5)
        results.next_to(chain, DOWN, buff=0.3)

        self.play(
            Create(alpha_box), Write(alpha_text),
            Create(nc_box), Write(nc_text),
        )

        self.wait(2)

        self.export_markers()


class ConstantsSummary(TRDScene):
    """Final summary of constants chapter."""

    def construct(self):
        self.load_narration("1.9")

        self.add_marker("1.9.6.1", "summary")

        # Full connection diagram
        connection = LemniscateAlphaConnection()
        connection.scale(0.85)

        self.play(connection.animate_connection(run_time=5.0))

        self.wait(1)

        # Key insight
        self.add_marker("1.9.6.2", "insight")

        insight_box = self.equation_box(
            r"\text{Geometry} \to G^* \to \{\alpha, N_c\}",
            "Fundamental constants are not free parameters"
        )
        insight_box.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(insight_box))

        self.wait(2)

        # Final message
        self.play(FadeOut(connection), FadeOut(insight_box))

        final = Text(
            "The universe is not arbitrary",
            color=TRD_COLORS["highlight"],
            font_size=36,
        )
        self.play(Write(final))
        self.wait(1)

        final2 = Text(
            "It is geometrically necessary",
            color=TRD_COLORS["glow"],
            font_size=32,
        )
        final2.next_to(final, DOWN, buff=0.4)
        self.play(Write(final2))

        self.wait(2)

        self.export_markers()
