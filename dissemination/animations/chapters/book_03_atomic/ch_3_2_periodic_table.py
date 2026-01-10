"""
Chapter 3.2: The Periodic Table
===============================

TRD perspective on the periodic table.
Shows how atomic structure emerges from voxel dynamics.
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
    Indicate,
    Flash,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Square,
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


class PeriodicTableIntro(TRDScene):
    """Introduction to the periodic table in TRD."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.0.1", "title")
        title = self.trd_title("The Periodic Table")
        subtitle = Text(
            "Atomic Structure from First Principles",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The question
        self.add_marker("3.2.0.2", "question")

        question = Text(
            "Why do elements have the properties they do?",
            color=TRD_COLORS["highlight"],
            font_size=26,
        )
        self.play(Write(question))
        self.wait(1)

        answer = Text(
            "TRD: proton count + electron shell structure",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        answer.next_to(question, DOWN, buff=0.5)
        self.play(Write(answer))
        self.wait(2)

        self.play(FadeOut(question), FadeOut(answer))

        self.export_markers()


class AtomicNumber(TRDScene):
    """Atomic number = proton count."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.1.1", "z")

        title = self.concept_card(
            "Atomic Number",
            "Z = number of protons"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Build up elements
        self.add_marker("3.2.1.2", "elements")

        elements_data = [
            ("H", 1, "Hydrogen"),
            ("He", 2, "Helium"),
            ("Li", 3, "Lithium"),
            ("C", 6, "Carbon"),
            ("O", 8, "Oxygen"),
            ("Fe", 26, "Iron"),
        ]

        y_pos = 2.0
        for symbol, z, name in elements_data:
            # Element box
            box = RoundedRectangle(
                width=1.2, height=1.0,
                corner_radius=0.1,
                stroke_color=TRD_COLORS["grid_bright"],
                fill_color=TRD_COLORS["background_light"],
                fill_opacity=0.5,
            )
            box.shift(LEFT * 3 + UP * y_pos)

            z_label = Text(str(z), color=TRD_COLORS["text_dim"], font_size=12)
            z_label.move_to(box.get_corner(UP + LEFT) + RIGHT * 0.2 + DOWN * 0.15)

            sym = Text(symbol, color=TRD_COLORS["highlight"], font_size=24, weight="BOLD")
            sym.move_to(box.get_center())

            name_label = Text(name, color=TRD_COLORS["text"], font_size=14)
            name_label.next_to(box, RIGHT, buff=0.3)

            # Proton indicator
            protons = Text(f"= {z} protons in nucleus", color=TRD_COLORS["matter"], font_size=14)
            protons.next_to(name_label, RIGHT, buff=0.2)

            self.play(
                Create(box), Write(z_label), Write(sym),
                Write(name_label), Write(protons),
                run_time=0.5,
            )

            y_pos -= 0.7

        # Key insight
        insight = MathTex(
            r"Z = \text{protons} = \text{triads in nucleus}",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        insight.to_edge(DOWN, buff=0.6)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class ElectronShells(TRDScene):
    """Electron shell structure."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.2.1", "shells")

        title = self.concept_card(
            "Electron Shells",
            "Discrete radii from flux standing waves"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Shell structure diagram
        self.add_marker("3.2.2.2", "diagram")

        # Nucleus
        nucleus = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        nucleus_label = Text("Nucleus", color=TRD_COLORS["matter"], font_size=12)
        nucleus_label.next_to(nucleus, DOWN, buff=0.1)

        self.play(Create(nucleus), Write(nucleus_label))

        # Shells
        shells = []
        shell_data = [
            (1, 0.8, 2, "n=1 (K)"),
            (2, 1.5, 8, "n=2 (L)"),
            (3, 2.3, 18, "n=3 (M)"),
        ]

        for n, radius, max_e, label_text in shell_data:
            shell = Circle(
                radius=radius,
                stroke_color=TRD_COLORS["antimatter"],
                stroke_width=2,
                stroke_opacity=0.5,
                fill_opacity=0,
            )

            label = Text(label_text, color=TRD_COLORS["antimatter"], font_size=12)
            label.next_to(shell, RIGHT, buff=0.1)

            max_label = Text(f"max {max_e} e⁻", color=TRD_COLORS["text_dim"], font_size=10)
            max_label.next_to(label, DOWN, buff=0.05)

            shells.append((shell, label, max_label))
            self.play(Create(shell), Write(label), Write(max_label), run_time=0.6)

        # Shell filling rule
        self.add_marker("3.2.2.3", "rule")

        rule = MathTex(
            r"\text{Max electrons in shell } n = 2n^2",
            color=TRD_COLORS["highlight"],
            font_size=22,
        )
        rule.to_edge(DOWN, buff=0.8)
        self.play(Write(rule))

        self.wait(2)

        self.export_markers()


class ShellFillingOrder(TRDScene):
    """Aufbau principle and shell filling."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.3.1", "aufbau")

        title = self.concept_card(
            "Shell Filling Order",
            "Aufbau principle from energy minimization"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Energy level diagram
        self.add_marker("3.2.3.2", "levels")

        levels = VGroup()
        level_data = [
            ("1s", 2, 0),
            ("2s", 2, 1),
            ("2p", 6, 1),
            ("3s", 2, 2),
            ("3p", 6, 2),
            ("4s", 2, 3),
            ("3d", 10, 2),
            ("4p", 6, 3),
        ]

        x_pos = -4
        for name, capacity, row in level_data:
            y_pos = 2 - row * 0.8

            level_line = Line(
                LEFT * 0.5, RIGHT * 0.5,
                color=TRD_COLORS["grid_bright"],
                stroke_width=3,
            )
            level_line.shift(RIGHT * x_pos + UP * y_pos)

            label = Text(name, color=TRD_COLORS["highlight"], font_size=14)
            label.next_to(level_line, LEFT, buff=0.1)

            cap = Text(f"({capacity})", color=TRD_COLORS["text_dim"], font_size=10)
            cap.next_to(level_line, RIGHT, buff=0.1)

            levels.add(VGroup(level_line, label, cap))
            x_pos += 1.3

        self.play(Create(levels, run_time=2.0))

        # Filling arrows
        self.add_marker("3.2.3.3", "order")

        order_label = Text(
            "Filling order: 1s → 2s → 2p → 3s → 3p → 4s → 3d → ...",
            color=TRD_COLORS["text"],
            font_size=16,
        )
        order_label.to_edge(DOWN, buff=1.2)
        self.play(Write(order_label))

        # TRD insight
        insight = Text(
            "TRD: Lower energy = stronger flux coupling to nucleus",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        insight.to_edge(DOWN, buff=0.6)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class MiniPeriodicTable(TRDScene):
    """Simplified periodic table visualization."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.4.1", "table")

        title = self.trd_title("Periodic Table Structure")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Simplified first few elements
        self.add_marker("3.2.4.2", "first_rows")

        # Row 1
        row1 = VGroup()
        h = self._make_element("H", 1, TRD_COLORS["highlight"])
        he = self._make_element("He", 2, TRD_COLORS["matter"])
        row1.add(h, he)
        h.shift(LEFT * 4)
        he.shift(RIGHT * 4)

        # Row 2
        row2 = VGroup()
        li = self._make_element("Li", 3, TRD_COLORS["highlight"])
        be = self._make_element("Be", 4, TRD_COLORS["highlight"])
        # ... gap for simplicity
        c = self._make_element("C", 6, TRD_COLORS["text"])
        n = self._make_element("N", 7, TRD_COLORS["text"])
        o = self._make_element("O", 8, TRD_COLORS["text"])
        ne = self._make_element("Ne", 10, TRD_COLORS["matter"])

        row2.add(li, be, c, n, o, ne)
        li.shift(LEFT * 4)
        be.shift(LEFT * 3.2)
        c.shift(LEFT * 0.8)
        n.shift(ORIGIN)
        o.shift(RIGHT * 0.8)
        ne.shift(RIGHT * 4)

        row1.shift(UP * 1.5)
        row2.shift(DOWN * 0)

        self.play(Create(row1), run_time=1.0)
        self.play(Create(row2), run_time=1.5)

        # Group labels
        self.add_marker("3.2.4.3", "groups")

        alkali = Text("Alkali metals", color=TRD_COLORS["highlight"], font_size=12)
        alkali.next_to(li, DOWN, buff=0.5)

        noble = Text("Noble gases", color=TRD_COLORS["matter"], font_size=12)
        noble.next_to(ne, DOWN, buff=0.5)

        self.play(Write(alkali), Write(noble))

        # Explanation
        explanation = VGroup()
        e1 = Text("• Period = highest occupied shell", color=TRD_COLORS["text"], font_size=14)
        e2 = Text("• Group = valence electrons", color=TRD_COLORS["text"], font_size=14)
        e3 = Text("• Noble gases = full shells (stable)", color=TRD_COLORS["matter"], font_size=14)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        explanation.to_edge(DOWN, buff=0.5)

        for e in explanation:
            self.play(Write(e), run_time=0.4)

        self.wait(2)

        self.export_markers()

    def _make_element(self, symbol, z, color):
        """Create a simple element box."""
        box = Square(
            side_length=0.7,
            stroke_color=color,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        z_text = Text(str(z), color=TRD_COLORS["text_dim"], font_size=8)
        z_text.move_to(box.get_corner(UP + LEFT) + RIGHT * 0.12 + DOWN * 0.1)
        sym = Text(symbol, color=color, font_size=16, weight="BOLD")
        sym.move_to(box.get_center())
        return VGroup(box, z_text, sym)


class ChemicalProperties(TRDScene):
    """How TRD explains chemical properties."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.5.1", "properties")

        title = self.concept_card(
            "Chemical Properties",
            "Valence electrons determine reactivity"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Property examples
        self.add_marker("3.2.5.2", "examples")

        examples = VGroup()

        # Sodium
        na_box = RoundedRectangle(width=5, height=1.5, corner_radius=0.1, stroke_color=TRD_COLORS["highlight"])
        na_box.shift(UP * 1.5)
        na_text = VGroup(
            Text("Sodium (Na, Z=11):", color=TRD_COLORS["highlight"], font_size=16),
            Text("1 valence e⁻ → easily loses it → reactive metal", color=TRD_COLORS["text"], font_size=14),
        )
        na_text.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        na_text.move_to(na_box.get_center())
        examples.add(VGroup(na_box, na_text))

        # Chlorine
        cl_box = RoundedRectangle(width=5, height=1.5, corner_radius=0.1, stroke_color=TRD_COLORS["antimatter"])
        cl_box.shift(ORIGIN)
        cl_text = VGroup(
            Text("Chlorine (Cl, Z=17):", color=TRD_COLORS["antimatter"], font_size=16),
            Text("7 valence e⁻ → wants 1 more → reactive nonmetal", color=TRD_COLORS["text"], font_size=14),
        )
        cl_text.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        cl_text.move_to(cl_box.get_center())
        examples.add(VGroup(cl_box, cl_text))

        # Neon
        ne_box = RoundedRectangle(width=5, height=1.5, corner_radius=0.1, stroke_color=TRD_COLORS["matter"])
        ne_box.shift(DOWN * 1.5)
        ne_text = VGroup(
            Text("Neon (Ne, Z=10):", color=TRD_COLORS["matter"], font_size=16),
            Text("8 valence e⁻ (full) → stable → noble gas", color=TRD_COLORS["text"], font_size=14),
        )
        ne_text.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        ne_text.move_to(ne_box.get_center())
        examples.add(VGroup(ne_box, ne_text))

        for ex in examples:
            self.play(Create(ex), run_time=0.8)

        # TRD insight
        insight = Text(
            "TRD: Flux balance determines electron transfer/sharing",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        insight.to_edge(DOWN, buff=0.5)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class PeriodicTableSummary(TRDScene):
    """Summary of periodic table in TRD."""

    def construct(self):
        self.load_narration("3.2")

        self.add_marker("3.2.6.1", "summary")

        title = self.trd_title("The Periodic Table")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Atomic number Z = proton count (triads)",
            "Electron shells at radii ~ n²",
            "Shell capacity = 2n² electrons",
            "Period = highest occupied shell",
            "Group = valence electron count",
            "Chemical properties from shell filling",
        ]

        point_mobs = VGroup()
        for point in points:
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=20)
            text = Text(point, color=TRD_COLORS["text"], font_size=16)
            text.next_to(bullet, RIGHT, buff=0.15)
            point_mobs.add(VGroup(bullet, text))

        point_mobs.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.45)

        self.wait(2)

        # Final insight
        final = self.equation_box(
            r"\text{Chemistry} = \text{Electron shell dynamics}",
            "The periodic table emerges from TRD"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
