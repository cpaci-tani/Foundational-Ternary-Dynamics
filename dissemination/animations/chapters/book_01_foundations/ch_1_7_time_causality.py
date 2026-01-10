"""
Chapter 1.7: Time and Causality
===============================

Animation showing discrete time in TRD and the causal structure.
Demonstrates the speed of causality and light cones.
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
    GrowFromCenter,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Polygon,
    Text,
    MathTex,
    NumberLine,
    RoundedRectangle,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS
from lib.components import Lattice2D, WavePulse


class TimeIntro(TRDScene):
    """Introduction to discrete time."""

    def construct(self):
        self.load_narration("1.7")

        self.add_marker("1.7.0.1", "title")
        title = self.trd_title("Time and Causality")
        subtitle = Text(
            "Discrete Ticks and the Speed Limit",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Discrete vs continuous
        self.add_marker("1.7.0.2", "discrete")

        # Continuous line
        cont_label = Text("Continuous time:", color=TRD_COLORS["text_dim"], font_size=20)
        cont_label.shift(UP * 1.5 + LEFT * 3)

        cont_line = Line(LEFT * 2, RIGHT * 2, color=TRD_COLORS["text_dim"])
        cont_line.next_to(cont_label, RIGHT, buff=0.3)

        # Discrete ticks
        disc_label = Text("TRD time:", color=TRD_COLORS["highlight"], font_size=20)
        disc_label.shift(DOWN * 0.5 + LEFT * 3)

        disc_line = Line(LEFT * 2, RIGHT * 2, color=TRD_COLORS["highlight"])
        disc_line.next_to(disc_label, RIGHT, buff=0.3)

        # Tick marks
        ticks = VGroup()
        for i in range(-4, 5):
            tick = Line(
                disc_line.get_center() + RIGHT * i * 0.5 + DOWN * 0.1,
                disc_line.get_center() + RIGHT * i * 0.5 + UP * 0.1,
                color=TRD_COLORS["highlight"],
                stroke_width=2,
            )
            ticks.add(tick)

        tick_labels = VGroup()
        for i, label in enumerate(["t-2", "t-1", "t", "t+1", "t+2"]):
            t = Text(label, color=TRD_COLORS["text"], font_size=14)
            t.next_to(ticks[i * 2 + 2], DOWN, buff=0.15)
            tick_labels.add(t)

        self.play(Write(cont_label), Create(cont_line))
        self.play(Write(disc_label), Create(disc_line))
        self.play(Create(ticks), Write(tick_labels))

        # Planck time
        planck = MathTex(
            r"\Delta t = t_P \approx 5.4 \times 10^{-44} \text{ s}",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        planck.to_edge(DOWN, buff=1.0)
        self.play(Write(planck))

        self.wait(2)

        self.export_markers()


class CausalitySpeed(TRDScene):
    """The speed of causality C = 1."""

    def construct(self):
        self.load_narration("1.7")

        self.add_marker("1.7.1.1", "speed")

        title = self.concept_card(
            "Speed of Causality",
            "Maximum information propagation rate"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # The rule
        self.add_marker("1.7.1.2", "rule")

        rule = MathTex(
            r"C = 1 \text{ voxel/tick}",
            color=TRD_COLORS["highlight"],
            font_size=40,
        )
        rule.to_edge(UP, buff=1.0)
        self.play(Write(rule))

        # Visualization: information spreading
        self.add_marker("1.7.1.3", "spread")

        lattice = Lattice2D(rows=9, cols=9, spacing=0.6, show_glow=False)
        lattice.shift(DOWN * 0.5)
        self.play(FadeIn(lattice))

        # Center event
        center = Dot(ORIGIN + DOWN * 0.5, radius=0.15, color=TRD_COLORS["highlight"])
        self.play(FadeIn(center))

        # Spreading influence
        pulse = WavePulse(
            center=ORIGIN + DOWN * 0.5,
            max_radius=2.4,
            color=TRD_COLORS["highlight"],
            num_rings=4,
        )
        self.add(pulse)

        tick_counter = Text("t = 0", color=TRD_COLORS["text"], font_size=20)
        tick_counter.to_edge(DOWN, buff=0.5)
        self.play(Write(tick_counter))

        # Animate spread with tick counter
        for t in range(1, 5):
            new_counter = Text(f"t = {t}", color=TRD_COLORS["text"], font_size=20)
            new_counter.to_edge(DOWN, buff=0.5)
            self.play(
                pulse.pulse_out(max_radius=t * 0.6, run_time=0.5),
                FadeOut(tick_counter),
                FadeIn(new_counter),
            )
            tick_counter = new_counter

        explanation = Text(
            "After t ticks, influence reaches distance t",
            color=TRD_COLORS["text"],
            font_size=18,
        )
        explanation.next_to(rule, DOWN, buff=0.3)
        self.play(Write(explanation))

        self.wait(2)

        self.export_markers()


class LightCone(TRDScene):
    """The discrete light cone."""

    def construct(self):
        self.load_narration("1.7")

        self.add_marker("1.7.2.1", "light_cone")

        title = self.concept_card(
            "The Light Cone",
            "Boundary of causal influence"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # 2D light cone diagram
        self.add_marker("1.7.2.2", "diagram")

        # Axes
        t_axis = Arrow(DOWN * 2, UP * 2.5, color=TRD_COLORS["text"], stroke_width=2)
        x_axis = Arrow(LEFT * 3, RIGHT * 3, color=TRD_COLORS["text"], stroke_width=2)

        t_label = Text("t", color=TRD_COLORS["text"], font_size=20)
        t_label.next_to(t_axis, UP)
        x_label = Text("x", color=TRD_COLORS["text"], font_size=20)
        x_label.next_to(x_axis, RIGHT)

        self.play(Create(t_axis), Create(x_axis))
        self.play(Write(t_label), Write(x_label))

        # Light cone
        future_cone = Polygon(
            ORIGIN,
            UP * 2 + LEFT * 2,
            UP * 2 + RIGHT * 2,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=2,
        )

        past_cone = Polygon(
            ORIGIN,
            DOWN * 1.5 + LEFT * 1.5,
            DOWN * 1.5 + RIGHT * 1.5,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=2,
        )

        self.play(Create(future_cone))
        self.play(Create(past_cone))

        # Labels
        future_label = Text("Future", color=TRD_COLORS["highlight"], font_size=16)
        future_label.move_to(UP * 1.5)

        past_label = Text("Past", color=TRD_COLORS["antimatter"], font_size=16)
        past_label.move_to(DOWN * 1)

        elsewhere_label = Text("Spacelike", color=TRD_COLORS["text_dim"], font_size=14)
        elsewhere_label.move_to(RIGHT * 2 + UP * 0.5)

        self.play(Write(future_label), Write(past_label), Write(elsewhere_label))

        # Slope note
        slope = MathTex(
            r"\text{slope} = \pm C = \pm 1",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        slope.to_edge(DOWN, buff=0.8)
        self.play(Write(slope))

        self.wait(2)

        self.export_markers()


class LocalCausality(TRDScene):
    """Local causality and the Moore neighborhood."""

    def construct(self):
        self.load_narration("1.7")

        self.add_marker("1.7.3.1", "local")

        title = self.concept_card(
            "Local Causality",
            "Updates depend only on neighbors"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Moore neighborhood
        self.add_marker("1.7.3.2", "moore")

        lattice = Lattice2D(rows=7, cols=7, spacing=0.8, show_glow=True)
        self.play(FadeIn(lattice))

        # Highlight center
        center = lattice.get_node_position(3, 3)
        center_highlight = Circle(
            radius=0.2,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        center_highlight.move_to(center)
        self.play(FadeIn(center_highlight))

        # Highlight neighbors
        self.add_marker("1.7.3.3", "neighbors")
        self.play(lattice.highlight_moore_neighborhood(3, 3))

        # Explanation
        explanation = VGroup()
        e1 = Text("Center voxel sees only 26 neighbors", color=TRD_COLORS["text"], font_size=18)
        e2 = Text("No action at a distance", color=TRD_COLORS["highlight"], font_size=18)
        e3 = Text("Causality is strictly local", color=TRD_COLORS["text"], font_size=18)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        explanation.to_edge(RIGHT, buff=0.5)

        for e in explanation:
            self.play(Write(e), run_time=0.6)

        self.wait(2)

        self.export_markers()


class TimeSummary(TRDScene):
    """Summary of time and causality."""

    def construct(self):
        self.load_narration("1.7")

        self.add_marker("1.7.4.1", "summary")

        title = self.trd_title("Time and Causality")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            ("Time is discrete", "t ∈ ℕ (integer ticks)"),
            ("Speed limit exists", "C = 1 voxel/tick"),
            ("Causality is local", "26-neighbor Moore neighborhood"),
            ("Light cones emerge", "Boundary of causal influence"),
            ("No FTL signaling", "Strict locality constraint"),
        ]

        point_mobs = VGroup()
        for title_text, detail in points:
            title_mob = Text(title_text, color=TRD_COLORS["highlight"], font_size=20)
            detail_mob = Text(detail, color=TRD_COLORS["text_dim"], font_size=16)
            detail_mob.next_to(title_mob, RIGHT, buff=0.3)
            group = VGroup(title_mob, detail_mob)
            point_mobs.add(group)

        point_mobs.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.6)

        self.wait(2)

        # Final equation
        final = self.equation_box(
            r"s(v, t+1) = f(s(N_{26}(v), t), J(N_{26}(v), t))",
            "State depends only on local past"
        )
        final.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
