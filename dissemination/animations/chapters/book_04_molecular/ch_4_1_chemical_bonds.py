"""
Chapter 4.1: Chemical Bonds
===========================

How atoms bond in TRD.
Shows electron sharing and transfer mechanisms.
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
    GrowArrow,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    DoubleArrow,
    Text,
    MathTex,
    RoundedRectangle,
    Ellipse,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class ChemicalBondsIntro(TRDScene):
    """Introduction to chemical bonds."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.0.1", "title")
        title = self.trd_title("Chemical Bonds")
        subtitle = Text(
            "How Atoms Connect",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The question
        self.add_marker("4.1.0.2", "question")

        question = Text(
            "Why do atoms form molecules?",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        self.play(Write(question))
        self.wait(1)

        answer = Text(
            "To achieve stable electron configurations",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        answer.next_to(question, DOWN, buff=0.5)
        self.play(Write(answer))
        self.wait(2)

        self.play(FadeOut(question), FadeOut(answer))

        self.export_markers()


class IonicBonds(TRDScene):
    """Ionic bonding: electron transfer."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.1.1", "ionic")

        title = self.concept_card(
            "Ionic Bonds",
            "Electron transfer between atoms"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Sodium atom
        self.add_marker("4.1.1.2", "transfer")

        na_nucleus = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        na_nucleus.shift(LEFT * 3)
        na_label = Text("Na", color=TRD_COLORS["matter"], font_size=16)
        na_label.next_to(na_nucleus, UP, buff=0.2)

        na_shell = Circle(
            radius=0.8,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=1,
            stroke_opacity=0.5,
        )
        na_shell.move_to(na_nucleus.get_center())

        na_electron = Dot(
            na_nucleus.get_center() + RIGHT * 0.8,
            radius=0.1,
            color=TRD_COLORS["antimatter"],
        )

        self.play(Create(na_nucleus), Write(na_label), Create(na_shell), Create(na_electron))

        # Chlorine atom
        cl_nucleus = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        cl_nucleus.shift(RIGHT * 3)
        cl_label = Text("Cl", color=TRD_COLORS["antimatter"], font_size=16)
        cl_label.next_to(cl_nucleus, UP, buff=0.2)

        cl_shell = Circle(
            radius=0.8,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=1,
            stroke_opacity=0.5,
        )
        cl_shell.move_to(cl_nucleus.get_center())

        # 7 electrons shown as dots around
        cl_electrons = VGroup()
        for i in range(7):
            angle = i * 2 * PI / 7
            e = Dot(
                cl_nucleus.get_center() + 0.8 * np.array([np.cos(angle), np.sin(angle), 0]),
                radius=0.08,
                color=TRD_COLORS["antimatter"],
            )
            cl_electrons.add(e)

        self.play(Create(cl_nucleus), Write(cl_label), Create(cl_shell), Create(cl_electrons))

        # Electron transfer
        self.add_marker("4.1.1.3", "move")

        transfer_arrow = Arrow(
            na_electron.get_center(),
            cl_nucleus.get_center() + LEFT * 0.8,
            color=TRD_COLORS["glow"],
            stroke_width=3,
        )
        self.play(GrowArrow(transfer_arrow))
        self.play(
            na_electron.animate.move_to(cl_nucleus.get_center() + LEFT * 0.8),
            FadeOut(transfer_arrow),
        )

        # Result labels
        na_ion = MathTex(r"Na^+", color=TRD_COLORS["matter"], font_size=24)
        na_ion.next_to(na_nucleus, DOWN, buff=0.5)

        cl_ion = MathTex(r"Cl^-", color=TRD_COLORS["antimatter"], font_size=24)
        cl_ion.next_to(cl_nucleus, DOWN, buff=0.5)

        self.play(Write(na_ion), Write(cl_ion))

        # Electrostatic attraction
        attraction = DoubleArrow(
            na_nucleus.get_center() + RIGHT * 0.4,
            cl_nucleus.get_center() + LEFT * 0.4,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        attr_label = Text("Electrostatic attraction", color=TRD_COLORS["highlight"], font_size=14)
        attr_label.next_to(attraction, DOWN, buff=0.2)

        self.play(Create(attraction), Write(attr_label))

        self.wait(2)

        self.export_markers()


class CovalentBonds(TRDScene):
    """Covalent bonding: electron sharing."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.2.1", "covalent")

        title = self.concept_card(
            "Covalent Bonds",
            "Electron sharing between atoms"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Two hydrogen atoms
        self.add_marker("4.1.2.2", "hydrogen")

        h1_nucleus = Circle(
            radius=0.2,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        h1_nucleus.shift(LEFT * 2)
        h1_label = Text("H", color=TRD_COLORS["text"], font_size=14)
        h1_label.next_to(h1_nucleus, UP, buff=0.3)

        h1_electron = Dot(
            h1_nucleus.get_center() + RIGHT * 0.5,
            radius=0.1,
            color=TRD_COLORS["antimatter"],
        )

        h2_nucleus = Circle(
            radius=0.2,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        h2_nucleus.shift(RIGHT * 2)
        h2_label = Text("H", color=TRD_COLORS["text"], font_size=14)
        h2_label.next_to(h2_nucleus, UP, buff=0.3)

        h2_electron = Dot(
            h2_nucleus.get_center() + LEFT * 0.5,
            radius=0.1,
            color=TRD_COLORS["antimatter"],
        )

        self.play(
            Create(h1_nucleus), Write(h1_label), Create(h1_electron),
            Create(h2_nucleus), Write(h2_label), Create(h2_electron),
        )

        # Atoms approach
        self.add_marker("4.1.2.3", "approach")

        self.play(
            h1_nucleus.animate.shift(RIGHT * 1),
            h1_label.animate.shift(RIGHT * 1),
            h1_electron.animate.shift(RIGHT * 0.7),
            h2_nucleus.animate.shift(LEFT * 1),
            h2_label.animate.shift(LEFT * 1),
            h2_electron.animate.shift(LEFT * 0.7),
        )

        # Shared electron cloud
        self.add_marker("4.1.2.4", "share")

        shared_cloud = Ellipse(
            width=1.5, height=0.8,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.3,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=1,
        )
        shared_cloud.move_to(ORIGIN)

        self.play(
            FadeOut(h1_electron), FadeOut(h2_electron),
            FadeIn(shared_cloud),
        )

        # H2 molecule label
        h2_mol = MathTex(r"H_2", color=TRD_COLORS["highlight"], font_size=32)
        h2_mol.to_edge(DOWN, buff=1.0)
        self.play(Write(h2_mol))

        # Explanation
        explanation = Text(
            "Both electrons shared equally → covalent bond",
            color=TRD_COLORS["text"],
            font_size=16,
        )
        explanation.next_to(h2_mol, UP, buff=0.3)
        self.play(Write(explanation))

        self.wait(2)

        self.export_markers()


class BondTypes(TRDScene):
    """Different types of covalent bonds."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.3.1", "types")

        title = self.concept_card(
            "Bond Types",
            "Single, double, and triple bonds"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Single bond
        self.add_marker("4.1.3.2", "single")

        single = VGroup()
        s_left = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        s_left.shift(LEFT * 0.6)
        s_right = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        s_right.shift(RIGHT * 0.6)
        s_bond = Line(s_left.get_right(), s_right.get_left(), color=TRD_COLORS["highlight"], stroke_width=3)
        s_label = Text("Single bond (2 e⁻)", color=TRD_COLORS["text"], font_size=14)
        s_label.next_to(VGroup(s_left, s_right), DOWN, buff=0.2)
        s_example = Text("H-H, C-C", color=TRD_COLORS["text_dim"], font_size=12)
        s_example.next_to(s_label, DOWN, buff=0.1)
        single.add(s_left, s_right, s_bond, s_label, s_example)
        single.shift(UP * 1.5 + LEFT * 3)

        self.play(Create(single))

        # Double bond
        double = VGroup()
        d_left = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        d_left.shift(LEFT * 0.6)
        d_right = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        d_right.shift(RIGHT * 0.6)
        d_bond1 = Line(
            s_left.get_right() + UP * 0.08,
            s_right.get_left() + UP * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        d_bond2 = Line(
            s_left.get_right() + DOWN * 0.08,
            s_right.get_left() + DOWN * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        d_label = Text("Double bond (4 e⁻)", color=TRD_COLORS["text"], font_size=14)
        d_label.next_to(VGroup(d_left, d_right), DOWN, buff=0.2)
        d_example = Text("O=O, C=C", color=TRD_COLORS["text_dim"], font_size=12)
        d_example.next_to(d_label, DOWN, buff=0.1)
        double.add(d_left, d_right, d_bond1, d_bond2, d_label, d_example)
        double.shift(UP * 1.5 + RIGHT * 0)

        self.play(Create(double))

        # Triple bond
        triple = VGroup()
        t_left = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        t_left.shift(LEFT * 0.6)
        t_right = Circle(radius=0.3, stroke_color=TRD_COLORS["text"], stroke_width=2)
        t_right.shift(RIGHT * 0.6)
        t_bond1 = Line(
            s_left.get_right() + UP * 0.12,
            s_right.get_left() + UP * 0.12,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        t_bond2 = Line(
            s_left.get_right(),
            s_right.get_left(),
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        t_bond3 = Line(
            s_left.get_right() + DOWN * 0.12,
            s_right.get_left() + DOWN * 0.12,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        t_label = Text("Triple bond (6 e⁻)", color=TRD_COLORS["text"], font_size=14)
        t_label.next_to(VGroup(t_left, t_right), DOWN, buff=0.2)
        t_example = Text("N≡N, C≡C", color=TRD_COLORS["text_dim"], font_size=12)
        t_example.next_to(t_label, DOWN, buff=0.1)
        triple.add(t_left, t_right, t_bond1, t_bond2, t_bond3, t_label, t_example)
        triple.shift(UP * 1.5 + RIGHT * 3)

        self.play(Create(triple))

        # TRD interpretation
        trd_note = Text(
            "TRD: More shared electrons = stronger flux overlap",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        trd_note.to_edge(DOWN, buff=0.6)
        self.play(Write(trd_note))

        self.wait(2)

        self.export_markers()


class BondEnergy(TRDScene):
    """Bond energy and stability."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.4.1", "energy")

        title = self.concept_card(
            "Bond Energy",
            "Energy required to break a bond"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Energy comparison
        self.add_marker("4.1.4.2", "comparison")

        bonds = [
            ("C-C", 347, TRD_COLORS["text"]),
            ("C=C", 614, TRD_COLORS["highlight"]),
            ("C≡C", 839, TRD_COLORS["matter"]),
            ("C-H", 413, TRD_COLORS["text_dim"]),
            ("O-H", 463, TRD_COLORS["antimatter"]),
        ]

        bars = VGroup()
        max_e = 900
        bar_width = 0.4

        for i, (name, energy, color) in enumerate(bonds):
            bar_height = energy / max_e * 3
            bar = RoundedRectangle(
                width=bar_width,
                height=bar_height,
                corner_radius=0.05,
                fill_color=color,
                fill_opacity=0.7,
                stroke_width=0,
            )
            bar.move_to(LEFT * 3 + RIGHT * i * 1.2 + UP * (bar_height / 2 - 1))

            label = Text(name, color=color, font_size=14)
            label.next_to(bar, DOWN, buff=0.1)

            value = Text(f"{energy}", color=TRD_COLORS["text"], font_size=10)
            value.next_to(bar, UP, buff=0.1)

            bars.add(VGroup(bar, label, value))

        for bar in bars:
            self.play(Create(bar), run_time=0.4)

        # Axis label
        axis_label = Text("Bond Energy (kJ/mol)", color=TRD_COLORS["text"], font_size=14)
        axis_label.to_edge(LEFT, buff=0.5)
        axis_label.rotate(PI / 2)
        self.play(Write(axis_label))

        # Key insight
        insight = Text(
            "Stronger bonds = more shared electrons = lower energy state",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        insight.to_edge(DOWN, buff=0.5)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class TRDBondPicture(TRDScene):
    """TRD interpretation of chemical bonds."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.5.1", "trd_bonds")

        title = self.concept_card(
            "TRD Bond Picture",
            "Flux overlap and electron sharing"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # TRD description
        self.add_marker("4.1.5.2", "description")

        description = VGroup()
        d1 = Text("In TRD, chemical bonds are:", color=TRD_COLORS["text"], font_size=18)
        d2 = Text("• Overlapping electron flux clouds", color=TRD_COLORS["highlight"], font_size=16)
        d3 = Text("• Shared flux reduces total energy", color=TRD_COLORS["highlight"], font_size=16)
        d4 = Text("• Stability from flux minimization", color=TRD_COLORS["highlight"], font_size=16)

        description.add(d1, d2, d3, d4)
        description.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        description.shift(UP * 1)

        for d in description:
            self.play(Write(d), run_time=0.5)

        # Visual: overlapping flux
        self.add_marker("4.1.5.3", "visual")

        atom1 = Circle(
            radius=1.0,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=1,
        )
        atom1.shift(DOWN * 1.5 + LEFT * 0.6)

        atom2 = Circle(
            radius=1.0,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=1,
        )
        atom2.shift(DOWN * 1.5 + RIGHT * 0.6)

        # Overlap region
        overlap = Ellipse(
            width=0.8, height=1.5,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.4,
            stroke_width=0,
        )
        overlap.shift(DOWN * 1.5)

        self.play(Create(atom1), Create(atom2))
        self.play(FadeIn(overlap))

        overlap_label = Text("Shared flux region", color=TRD_COLORS["highlight"], font_size=14)
        overlap_label.next_to(overlap, DOWN, buff=0.2)
        self.play(Write(overlap_label))

        self.wait(2)

        self.export_markers()


class ChemicalBondsSummary(TRDScene):
    """Summary of chemical bonds."""

    def construct(self):
        self.load_narration("4.1")

        self.add_marker("4.1.6.1", "summary")

        title = self.trd_title("Chemical Bonds")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Ionic: electron transfer (Na⁺Cl⁻)",
            "Covalent: electron sharing (H₂)",
            "Single/double/triple bonds",
            "Bond strength ∝ electrons shared",
            "TRD: flux overlap minimizes energy",
            "Molecules = stable flux configurations",
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
            r"\text{Bond} = \text{Shared electron flux}",
            "Chemistry emerges from TRD electron dynamics"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
