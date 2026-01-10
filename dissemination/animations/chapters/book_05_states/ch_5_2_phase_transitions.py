"""
Chapter 5.2: Phase Transitions
==============================

Melting, boiling, and phase changes in TRD.
Shows energy changes during transitions.
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
    GrowArrow,
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
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class PhaseTransitionsIntro(TRDScene):
    """Introduction to phase transitions."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.0.1", "title")
        title = self.trd_title("Phase Transitions")
        subtitle = Text(
            "Crossing the Boundaries",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class MeltingProcess(TRDScene):
    """Melting: solid to liquid."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.1.1", "melting")

        title = self.concept_card(
            "Melting",
            "Solid → Liquid"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Solid lattice
        self.add_marker("5.2.1.2", "solid")

        solid_label = Text("Solid", color=TRD_COLORS["matter"], font_size=18)
        solid_label.shift(LEFT * 3 + UP * 2)

        solid = VGroup()
        for i in range(4):
            for j in range(4):
                atom = Circle(
                    radius=0.12,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(LEFT * 3 + [(j - 1.5) * 0.4, (i - 1.5) * 0.4, 0])
                solid.add(atom)

        self.play(Write(solid_label), Create(solid))

        # Arrow
        arrow = Arrow(LEFT * 1, RIGHT * 1, color=TRD_COLORS["highlight"])
        heat = Text("+ Heat", color=TRD_COLORS["glow"], font_size=14)
        heat.next_to(arrow, UP, buff=0.1)
        self.play(GrowArrow(arrow), Write(heat))

        # Liquid
        self.add_marker("5.2.1.3", "liquid")

        liquid_label = Text("Liquid", color=TRD_COLORS["antimatter"], font_size=18)
        liquid_label.shift(RIGHT * 3 + UP * 2)

        liquid = VGroup()
        np.random.seed(111)
        for _ in range(16):
            x = np.random.uniform(-0.8, 0.8)
            y = np.random.uniform(-0.8, 0.8)
            atom = Circle(
                radius=0.12,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            atom.move_to(RIGHT * 3 + [x, y, 0])
            liquid.add(atom)

        self.play(Write(liquid_label), Create(liquid))

        # Explanation
        explanation = VGroup()
        e1 = Text("• Bonds break, structure loosens", color=TRD_COLORS["text"], font_size=14)
        e2 = Text("• Requires latent heat of fusion", color=TRD_COLORS["text"], font_size=14)
        e3 = MathTex(r"\Delta H_{fus} > 0", color=TRD_COLORS["highlight"], font_size=20)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        explanation.to_edge(DOWN, buff=0.5)

        for e in explanation:
            self.play(Write(e), run_time=0.4)

        self.wait(2)

        self.export_markers()


class BoilingProcess(TRDScene):
    """Boiling: liquid to gas."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.2.1", "boiling")

        title = self.concept_card(
            "Boiling",
            "Liquid → Gas"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Liquid
        self.add_marker("5.2.2.2", "liquid")

        liquid_label = Text("Liquid", color=TRD_COLORS["antimatter"], font_size=18)
        liquid_label.shift(LEFT * 3 + UP * 2)

        liquid = VGroup()
        np.random.seed(222)
        for _ in range(20):
            x = np.random.uniform(-0.8, 0.8)
            y = np.random.uniform(-0.8, 0.8)
            atom = Circle(
                radius=0.1,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            atom.move_to(LEFT * 3 + [x, y, 0])
            liquid.add(atom)

        self.play(Write(liquid_label), Create(liquid))

        # Arrow
        arrow = Arrow(LEFT * 1, RIGHT * 1, color=TRD_COLORS["highlight"])
        heat = Text("+ More Heat", color=TRD_COLORS["glow"], font_size=14)
        heat.next_to(arrow, UP, buff=0.1)
        self.play(GrowArrow(arrow), Write(heat))

        # Gas
        self.add_marker("5.2.2.3", "gas")

        gas_label = Text("Gas", color=TRD_COLORS["highlight"], font_size=18)
        gas_label.shift(RIGHT * 3 + UP * 2)

        gas = VGroup()
        np.random.seed(333)
        for _ in range(12):
            x = np.random.uniform(-1.5, 1.5)
            y = np.random.uniform(-1.2, 1.2)
            atom = Circle(
                radius=0.08,
                fill_color=TRD_COLORS["highlight"],
                fill_opacity=0.6,
                stroke_width=0,
            )
            atom.move_to(RIGHT * 3 + [x, y, 0])
            gas.add(atom)

        self.play(Write(gas_label), Create(gas))

        # Explanation
        explanation = VGroup()
        e1 = Text("• All bonds break", color=TRD_COLORS["text"], font_size=14)
        e2 = Text("• Requires latent heat of vaporization", color=TRD_COLORS["text"], font_size=14)
        e3 = MathTex(r"\Delta H_{vap} > \Delta H_{fus}", color=TRD_COLORS["highlight"], font_size=20)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        explanation.to_edge(DOWN, buff=0.5)

        for e in explanation:
            self.play(Write(e), run_time=0.4)

        self.wait(2)

        self.export_markers()


class HeatingCurve(TRDScene):
    """Heating curve diagram."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.3.1", "curve")

        title = self.concept_card(
            "Heating Curve",
            "Temperature vs energy added"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Axes
        self.add_marker("5.2.3.2", "axes")

        axes = Axes(
            x_range=[0, 10, 2],
            y_range=[0, 6, 1],
            x_length=8,
            y_length=4,
            axis_config={"color": TRD_COLORS["grid_bright"]},
        )
        axes.shift(DOWN * 0.3)

        x_label = Text("Heat Added", color=TRD_COLORS["text"], font_size=14)
        x_label.next_to(axes, DOWN, buff=0.3)

        y_label = Text("Temperature", color=TRD_COLORS["text"], font_size=14)
        y_label.next_to(axes, LEFT, buff=0.3)
        y_label.rotate(PI / 2)

        self.play(Create(axes), Write(x_label), Write(y_label))

        # Heating curve (with plateaus)
        self.add_marker("5.2.3.3", "plot")

        # Solid heating (0,1) to (2,2)
        seg1 = Line(axes.c2p(0, 1), axes.c2p(2, 2), color=TRD_COLORS["matter"], stroke_width=3)
        # Melting plateau (2,2) to (3,2)
        seg2 = Line(axes.c2p(2, 2), axes.c2p(3, 2), color=TRD_COLORS["matter"], stroke_width=3)
        # Liquid heating (3,2) to (6,4)
        seg3 = Line(axes.c2p(3, 2), axes.c2p(6, 4), color=TRD_COLORS["antimatter"], stroke_width=3)
        # Boiling plateau (6,4) to (8,4)
        seg4 = Line(axes.c2p(6, 4), axes.c2p(8, 4), color=TRD_COLORS["antimatter"], stroke_width=3)
        # Gas heating (8,4) to (10,5.5)
        seg5 = Line(axes.c2p(8, 4), axes.c2p(10, 5.5), color=TRD_COLORS["highlight"], stroke_width=3)

        self.play(Create(seg1))
        self.play(Create(seg2))
        self.play(Create(seg3))
        self.play(Create(seg4))
        self.play(Create(seg5))

        # Labels
        solid_label = Text("Solid", color=TRD_COLORS["matter"], font_size=12)
        solid_label.next_to(seg1, UP, buff=0.1)

        melt_label = Text("Melting", color=TRD_COLORS["text_dim"], font_size=10)
        melt_label.next_to(seg2, DOWN, buff=0.1)

        liquid_label = Text("Liquid", color=TRD_COLORS["antimatter"], font_size=12)
        liquid_label.next_to(seg3.get_center(), UP, buff=0.1)

        boil_label = Text("Boiling", color=TRD_COLORS["text_dim"], font_size=10)
        boil_label.next_to(seg4, DOWN, buff=0.1)

        gas_label = Text("Gas", color=TRD_COLORS["highlight"], font_size=12)
        gas_label.next_to(seg5, UP, buff=0.1)

        self.play(
            Write(solid_label), Write(melt_label), Write(liquid_label),
            Write(boil_label), Write(gas_label),
        )

        self.wait(2)

        self.export_markers()


class PhaseDiagram(TRDScene):
    """Phase diagram (P-T)."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.4.1", "diagram")

        title = self.concept_card(
            "Phase Diagram",
            "Pressure vs Temperature"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Axes
        self.add_marker("5.2.4.2", "axes")

        axes = Axes(
            x_range=[0, 10, 2],
            y_range=[0, 10, 2],
            x_length=6,
            y_length=5,
            axis_config={"color": TRD_COLORS["grid_bright"]},
        )

        x_label = Text("Temperature", color=TRD_COLORS["text"], font_size=14)
        x_label.next_to(axes, DOWN, buff=0.3)

        y_label = Text("Pressure", color=TRD_COLORS["text"], font_size=14)
        y_label.next_to(axes, LEFT, buff=0.3)
        y_label.rotate(PI / 2)

        self.play(Create(axes), Write(x_label), Write(y_label))

        # Phase boundaries
        self.add_marker("5.2.4.3", "boundaries")

        # Triple point at (3, 2)
        # Solid-liquid line (steep)
        sl_line = Line(axes.c2p(3, 2), axes.c2p(4, 10), color=TRD_COLORS["grid_bright"], stroke_width=2)
        # Solid-gas line
        sg_line = Line(axes.c2p(0, 0), axes.c2p(3, 2), color=TRD_COLORS["grid_bright"], stroke_width=2)
        # Liquid-gas line (ends at critical point)
        lg_line = Line(axes.c2p(3, 2), axes.c2p(7, 6), color=TRD_COLORS["grid_bright"], stroke_width=2)

        self.play(Create(sl_line), Create(sg_line), Create(lg_line))

        # Triple point
        triple = Dot(axes.c2p(3, 2), radius=0.1, color=TRD_COLORS["highlight"])
        triple_label = Text("Triple Point", color=TRD_COLORS["highlight"], font_size=12)
        triple_label.next_to(triple, DOWN + LEFT, buff=0.1)

        # Critical point
        critical = Dot(axes.c2p(7, 6), radius=0.1, color=TRD_COLORS["glow"])
        critical_label = Text("Critical Point", color=TRD_COLORS["glow"], font_size=12)
        critical_label.next_to(critical, UP, buff=0.1)

        self.play(Create(triple), Write(triple_label))
        self.play(Create(critical), Write(critical_label))

        # Phase labels
        solid = Text("SOLID", color=TRD_COLORS["matter"], font_size=14)
        solid.move_to(axes.c2p(1.5, 6))

        liquid = Text("LIQUID", color=TRD_COLORS["antimatter"], font_size=14)
        liquid.move_to(axes.c2p(5, 7))

        gas = Text("GAS", color=TRD_COLORS["highlight"], font_size=14)
        gas.move_to(axes.c2p(6, 2))

        self.play(Write(solid), Write(liquid), Write(gas))

        self.wait(2)

        self.export_markers()


class PhaseTransitionsSummary(TRDScene):
    """Summary of phase transitions."""

    def construct(self):
        self.load_narration("5.2")

        self.add_marker("5.2.5.1", "summary")

        title = self.trd_title("Phase Transitions")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Transition table
        transitions = VGroup()

        trans_data = [
            ("Melting", "Solid → Liquid", "ΔH_fus"),
            ("Boiling", "Liquid → Gas", "ΔH_vap"),
            ("Sublimation", "Solid → Gas", "ΔH_sub"),
            ("Condensation", "Gas → Liquid", "-ΔH_vap"),
        ]

        for name, change, energy in trans_data:
            row = VGroup(
                Text(name, color=TRD_COLORS["highlight"], font_size=16),
                Text(change, color=TRD_COLORS["text"], font_size=14),
                MathTex(energy, color=TRD_COLORS["text_dim"], font_size=18),
            )
            row.arrange(RIGHT, buff=0.8)
            transitions.add(row)

        transitions.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        transitions.center()

        for trans in transitions:
            self.play(Write(trans), run_time=0.5)

        self.wait(2)

        final = self.equation_box(
            r"\text{Transition} = \text{Energy} + \text{Reorganization}",
            "Phase changes require latent heat"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
