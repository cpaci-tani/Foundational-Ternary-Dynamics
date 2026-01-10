"""
Chapter 5.3: Thermodynamics
===========================

Energy, entropy, and the laws of thermodynamics.
Shows how TRD grounds thermal behavior.
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
    CurvedArrow,
    Text,
    MathTex,
    RoundedRectangle,
    Rectangle,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class ThermodynamicsIntro(TRDScene):
    """Introduction to thermodynamics."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.0.1", "title")
        title = self.trd_title("Thermodynamics")
        subtitle = Text(
            "Energy, Entropy, and the Laws",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class FirstLaw(TRDScene):
    """First law - energy conservation."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.1.1", "first_law")

        title = self.concept_card(
            "First Law",
            "Conservation of Energy"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Equation
        self.add_marker("5.3.1.2", "equation")

        law = MathTex(
            r"\Delta U = Q - W",
            color=TRD_COLORS["highlight"],
            font_size=48,
        )
        law.shift(UP * 1)
        self.play(Write(law))

        # Definitions
        defs = VGroup()
        d1 = MathTex(r"\Delta U = \text{change in internal energy}", color=TRD_COLORS["text"], font_size=22)
        d2 = MathTex(r"Q = \text{heat added to system}", color=TRD_COLORS["matter"], font_size=22)
        d3 = MathTex(r"W = \text{work done by system}", color=TRD_COLORS["antimatter"], font_size=22)

        defs.add(d1, d2, d3)
        defs.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        defs.next_to(law, DOWN, buff=0.6)

        for d in defs:
            self.play(Write(d), run_time=0.5)

        # TRD interpretation
        trd = VGroup()
        t1 = Text("TRD:", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD")
        t2 = Text("Internal energy = total flux magnitude", color=TRD_COLORS["text"], font_size=14)
        t2.next_to(t1, RIGHT, buff=0.2)
        trd.add(t1, t2)
        trd.to_edge(DOWN, buff=0.6)

        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class SecondLaw(TRDScene):
    """Second law - entropy increases."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.2.1", "second_law")

        title = self.concept_card(
            "Second Law",
            "Entropy always increases"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Entropy equation
        self.add_marker("5.3.2.2", "entropy")

        law = MathTex(
            r"\Delta S_{universe} \geq 0",
            color=TRD_COLORS["highlight"],
            font_size=48,
        )
        law.shift(UP * 1.5)
        self.play(Write(law))

        # Visualization: ordered → disordered
        self.add_marker("5.3.2.3", "visual")

        # Ordered state
        ordered = VGroup()
        for i in range(4):
            for j in range(4):
                dot = Dot(
                    point=LEFT * 3 + [(j - 1.5) * 0.4, (i - 1.5) * 0.4 - 1, 0],
                    radius=0.1,
                    color=TRD_COLORS["matter"],
                )
                ordered.add(dot)

        ordered_label = Text("Low entropy", color=TRD_COLORS["matter"], font_size=14)
        ordered_label.next_to(ordered, DOWN, buff=0.2)

        self.play(Create(ordered), Write(ordered_label))

        # Arrow
        arrow = Arrow(LEFT * 1, RIGHT * 1, color=TRD_COLORS["text_dim"])
        time_label = Text("Time", color=TRD_COLORS["text_dim"], font_size=12)
        time_label.next_to(arrow, UP, buff=0.1)
        self.play(GrowArrow(arrow), Write(time_label))

        # Disordered state
        disordered = VGroup()
        np.random.seed(999)
        for _ in range(16):
            x = np.random.uniform(-0.8, 0.8)
            y = np.random.uniform(-0.8, 0.8)
            dot = Dot(
                point=RIGHT * 3 + [x, y - 1, 0],
                radius=0.1,
                color=TRD_COLORS["antimatter"],
            )
            disordered.add(dot)

        disordered_label = Text("High entropy", color=TRD_COLORS["antimatter"], font_size=14)
        disordered_label.next_to(disordered, DOWN, buff=0.2)

        self.play(Create(disordered), Write(disordered_label))

        # TRD note
        trd = Text(
            "TRD: Entropy = number of equivalent flux microstates",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class ThirdLaw(TRDScene):
    """Third law - absolute zero."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.3.1", "third_law")

        title = self.concept_card(
            "Third Law",
            "Absolute zero is unattainable"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Statement
        self.add_marker("5.3.3.2", "statement")

        law = MathTex(
            r"S \to 0 \text{ as } T \to 0",
            color=TRD_COLORS["highlight"],
            font_size=40,
        )
        law.shift(UP * 1)
        self.play(Write(law))

        temp = MathTex(
            r"T = 0 \text{ K} = -273.15°\text{C}",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        temp.next_to(law, DOWN, buff=0.5)
        self.play(Write(temp))

        # Implications
        implications = VGroup()
        i1 = Text("• Perfect crystal has zero entropy at 0 K", color=TRD_COLORS["text"], font_size=16)
        i2 = Text("• Cannot reach absolute zero in finite steps", color=TRD_COLORS["text"], font_size=16)
        i3 = Text("• Quantum ground state = minimum entropy", color=TRD_COLORS["text"], font_size=16)

        implications.add(i1, i2, i3)
        implications.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        implications.to_edge(DOWN, buff=0.6)

        for imp in implications:
            self.play(Write(imp), run_time=0.5)

        self.wait(2)

        self.export_markers()


class HeatEngines(TRDScene):
    """Heat engines and efficiency."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.4.1", "engines")

        title = self.concept_card(
            "Heat Engines",
            "Converting heat to work"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Engine diagram
        self.add_marker("5.3.4.2", "diagram")

        # Hot reservoir
        hot = RoundedRectangle(
            width=2.5, height=1,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.3,
        )
        hot.shift(UP * 2)
        hot_label = Text("Hot (T_H)", color=TRD_COLORS["matter"], font_size=14)
        hot_label.move_to(hot.get_center())

        # Engine
        engine = Circle(
            radius=0.8,
            stroke_color=TRD_COLORS["highlight"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        engine_label = Text("Engine", color=TRD_COLORS["highlight"], font_size=14)
        engine_label.move_to(engine.get_center())

        # Cold reservoir
        cold = RoundedRectangle(
            width=2.5, height=1,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.3,
        )
        cold.shift(DOWN * 2)
        cold_label = Text("Cold (T_C)", color=TRD_COLORS["antimatter"], font_size=14)
        cold_label.move_to(cold.get_center())

        self.play(
            Create(hot), Write(hot_label),
            Create(engine), Write(engine_label),
            Create(cold), Write(cold_label),
        )

        # Heat flows
        q_h = Arrow(hot.get_bottom(), engine.get_top(), color=TRD_COLORS["matter"], buff=0.1)
        q_h_label = MathTex(r"Q_H", color=TRD_COLORS["matter"], font_size=18)
        q_h_label.next_to(q_h, LEFT, buff=0.1)

        q_c = Arrow(engine.get_bottom(), cold.get_top(), color=TRD_COLORS["antimatter"], buff=0.1)
        q_c_label = MathTex(r"Q_C", color=TRD_COLORS["antimatter"], font_size=18)
        q_c_label.next_to(q_c, LEFT, buff=0.1)

        w = Arrow(engine.get_right(), engine.get_right() + RIGHT * 1.5, color=TRD_COLORS["glow"], buff=0.1)
        w_label = MathTex(r"W", color=TRD_COLORS["glow"], font_size=18)
        w_label.next_to(w, UP, buff=0.1)

        self.play(GrowArrow(q_h), Write(q_h_label))
        self.play(GrowArrow(q_c), Write(q_c_label))
        self.play(GrowArrow(w), Write(w_label))

        # Efficiency
        efficiency = MathTex(
            r"\eta = \frac{W}{Q_H} = 1 - \frac{T_C}{T_H}",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        efficiency.to_edge(RIGHT, buff=0.5)
        self.play(Write(efficiency))

        self.wait(2)

        self.export_markers()


class BoltzmannEntropy(TRDScene):
    """Boltzmann entropy formula."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.5.1", "boltzmann")

        title = self.concept_card(
            "Boltzmann Entropy",
            "Microstates and macrostates"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Famous equation
        self.add_marker("5.3.5.2", "equation")

        boltzmann = MathTex(
            r"S = k_B \ln \Omega",
            color=TRD_COLORS["highlight"],
            font_size=56,
        )
        boltzmann.shift(UP * 1)
        self.play(Write(boltzmann))

        # Definitions
        defs = VGroup()
        d1 = MathTex(r"S = \text{entropy}", color=TRD_COLORS["text"], font_size=20)
        d2 = MathTex(r"k_B = \text{Boltzmann constant}", color=TRD_COLORS["text"], font_size=20)
        d3 = MathTex(r"\Omega = \text{number of microstates}", color=TRD_COLORS["matter"], font_size=20)

        defs.add(d1, d2, d3)
        defs.arrange(DOWN, buff=0.25, aligned_edge=LEFT)
        defs.next_to(boltzmann, DOWN, buff=0.5)

        for d in defs:
            self.play(Write(d), run_time=0.4)

        # TRD connection
        trd = VGroup()
        t1 = Text("TRD:", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD")
        t2 = Text("Ω = distinct flux configurations", color=TRD_COLORS["text"], font_size=14)
        t2.next_to(t1, RIGHT, buff=0.2)
        trd.add(t1, t2)
        trd.to_edge(DOWN, buff=0.5)

        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class ThermodynamicsSummary(TRDScene):
    """Summary of thermodynamics."""

    def construct(self):
        self.load_narration("5.3")

        self.add_marker("5.3.6.1", "summary")

        title = self.trd_title("Thermodynamics")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # The laws
        laws = VGroup()

        law_data = [
            ("0th Law", "Thermal equilibrium is transitive"),
            ("1st Law", "Energy is conserved: ΔU = Q - W"),
            ("2nd Law", "Entropy increases: ΔS ≥ 0"),
            ("3rd Law", "S → 0 as T → 0"),
        ]

        for name, statement in law_data:
            row = VGroup(
                Text(name + ":", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
                Text(statement, color=TRD_COLORS["text"], font_size=14),
            )
            row[1].next_to(row[0], RIGHT, buff=0.2)
            laws.add(row)

        laws.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        laws.center()

        for law in laws:
            self.play(Write(law), run_time=0.5)

        self.wait(2)

        final = self.equation_box(
            r"S = k_B \ln \Omega",
            "Entropy counts TRD flux microstates"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
