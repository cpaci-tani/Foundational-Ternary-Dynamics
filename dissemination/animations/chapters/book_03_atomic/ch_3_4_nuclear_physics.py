"""
Chapter 3.4: Nuclear Physics
============================

Nuclear forces and structure in TRD.
Shows strong force binding and nuclear stability.
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


class NuclearPhysicsIntro(TRDScene):
    """Introduction to nuclear physics."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.0.1", "title")
        title = self.trd_title("Nuclear Physics")
        subtitle = Text(
            "The Strong Force and Nuclear Structure",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The question
        self.add_marker("3.4.0.2", "question")

        question = Text(
            "What holds the nucleus together?",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        self.play(Write(question))
        self.wait(1)

        # Protons repel
        sub = Text(
            "Protons repel electrically... so why don't nuclei explode?",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        sub.next_to(question, DOWN, buff=0.5)
        self.play(Write(sub))
        self.wait(2)

        self.play(FadeOut(question), FadeOut(sub))

        self.export_markers()


class StrongForce(TRDScene):
    """The strong nuclear force."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.1.1", "strong")

        title = self.concept_card(
            "The Strong Force",
            "Color charge attraction"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Comparison of forces
        self.add_marker("3.4.1.2", "comparison")

        forces = VGroup()

        # EM force
        em_label = Text("Electromagnetic:", color=TRD_COLORS["antimatter"], font_size=18)
        em_range = Text("Range: infinite (1/r²)", color=TRD_COLORS["text"], font_size=14)
        em_range.next_to(em_label, RIGHT, buff=0.2)
        em_note = Text("Protons repel", color=TRD_COLORS["text_dim"], font_size=12)
        em_note.next_to(em_label, DOWN, buff=0.1, aligned_edge=LEFT)
        em = VGroup(em_label, em_range, em_note)

        # Strong force
        strong_label = Text("Strong (nuclear):", color=TRD_COLORS["matter"], font_size=18)
        strong_range = Text("Range: ~1 fm (Yukawa)", color=TRD_COLORS["text"], font_size=14)
        strong_range.next_to(strong_label, RIGHT, buff=0.2)
        strong_note = Text("100× stronger at short range!", color=TRD_COLORS["highlight"], font_size=12)
        strong_note.next_to(strong_label, DOWN, buff=0.1, aligned_edge=LEFT)
        strong = VGroup(strong_label, strong_range, strong_note)

        forces.add(em, strong)
        forces.arrange(DOWN, buff=0.6, aligned_edge=LEFT)
        forces.shift(UP * 1)

        self.play(Write(em))
        self.play(Write(strong))

        # Yukawa potential
        self.add_marker("3.4.1.3", "yukawa")

        yukawa = MathTex(
            r"V(r) = -g^2 \frac{e^{-m_\pi r}}{r}",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        yukawa.to_edge(DOWN, buff=1.2)

        yukawa_note = Text(
            "Exponential falloff → short range",
            color=TRD_COLORS["text_dim"],
            font_size=14,
        )
        yukawa_note.next_to(yukawa, DOWN, buff=0.2)

        self.play(Write(yukawa), Write(yukawa_note))

        self.wait(2)

        self.export_markers()


class NuclearBinding(TRDScene):
    """Nuclear binding energy."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.2.1", "binding")

        title = self.concept_card(
            "Nuclear Binding Energy",
            "Mass deficit from binding"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Mass equation
        self.add_marker("3.4.2.2", "mass")

        mass_eq = MathTex(
            r"M_{nucleus} < Z \cdot m_p + N \cdot m_n",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        mass_eq.shift(UP * 1.5)
        self.play(Write(mass_eq))

        # Binding energy
        binding_eq = MathTex(
            r"E_B = (Z m_p + N m_n - M_{nucleus}) c^2",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        binding_eq.next_to(mass_eq, DOWN, buff=0.5)
        self.play(Write(binding_eq))

        # Example
        self.add_marker("3.4.2.3", "example")

        example = VGroup()
        ex_label = Text("Example: Helium-4 (α particle)", color=TRD_COLORS["matter"], font_size=18)
        ex_calc = MathTex(
            r"E_B = 28.3 \text{ MeV} \approx 7.1 \text{ MeV/nucleon}",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        ex_calc.next_to(ex_label, DOWN, buff=0.2)
        example.add(ex_label, ex_calc)
        example.shift(DOWN * 0.8)

        self.play(Write(example))

        # TRD note
        trd_note = Text(
            "TRD: Binding energy = flux reduction in bound configuration",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        trd_note.to_edge(DOWN, buff=0.5)
        self.play(Write(trd_note))

        self.wait(2)

        self.export_markers()


class BindingEnergyCurve(TRDScene):
    """Binding energy per nucleon curve."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.3.1", "curve")

        title = self.concept_card(
            "Binding Energy Curve",
            "Why iron is the most stable"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Axes
        self.add_marker("3.4.3.2", "axes")

        axes = Axes(
            x_range=[0, 250, 50],
            y_range=[0, 10, 2],
            x_length=8,
            y_length=4,
            axis_config={"color": TRD_COLORS["grid_bright"]},
        )
        axes.shift(DOWN * 0.5)

        x_label = Text("Mass number A", color=TRD_COLORS["text"], font_size=14)
        x_label.next_to(axes, DOWN, buff=0.3)

        y_label = Text("E_B / A (MeV)", color=TRD_COLORS["text"], font_size=14)
        y_label.next_to(axes, LEFT, buff=0.3)
        y_label.rotate(PI / 2)

        self.play(Create(axes), Write(x_label), Write(y_label))

        # Binding energy curve (simplified)
        self.add_marker("3.4.3.3", "plot")

        # Key points
        points = [
            (4, 7.1, "He"),
            (12, 7.7, "C"),
            (56, 8.8, "Fe"),
            (120, 8.2, ""),
            (238, 7.6, "U"),
        ]

        dots = VGroup()
        for a, e, label_text in points:
            x = axes.c2p(a, e)[0]
            y = axes.c2p(a, e)[1]
            dot = Dot(point=[x, y, 0], radius=0.08, color=TRD_COLORS["highlight"])
            dots.add(dot)
            if label_text:
                label = Text(label_text, color=TRD_COLORS["text"], font_size=12)
                label.next_to(dot, UP, buff=0.1)
                dots.add(label)

        # Curve through points (simplified line segments)
        curve_points = [axes.c2p(a, e) for a, e, _ in points]
        curve = VGroup()
        for i in range(len(curve_points) - 1):
            segment = Line(
                curve_points[i],
                curve_points[i + 1],
                color=TRD_COLORS["matter"],
                stroke_width=3,
            )
            curve.add(segment)

        self.play(Create(curve), Create(dots))

        # Iron peak annotation
        iron_arrow = Arrow(
            axes.c2p(56, 8.8) + UP * 0.8,
            axes.c2p(56, 8.8) + UP * 0.2,
            color=TRD_COLORS["glow"],
        )
        iron_label = Text("Fe-56: most stable!", color=TRD_COLORS["glow"], font_size=14)
        iron_label.next_to(iron_arrow, UP, buff=0.1)

        self.play(GrowArrow(iron_arrow), Write(iron_label))

        self.wait(2)

        self.export_markers()


class FusionFission(TRDScene):
    """Fusion and fission processes."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.4.1", "processes")

        title = self.concept_card(
            "Fusion and Fission",
            "Energy release from binding curve"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Fusion
        self.add_marker("3.4.4.2", "fusion")

        fusion_box = RoundedRectangle(
            width=5, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["glow"],
            fill_opacity=0.1,
        )
        fusion_box.shift(UP * 1.3 + LEFT * 2)

        fusion_title = Text("FUSION", color=TRD_COLORS["glow"], font_size=20, weight="BOLD")
        fusion_title.next_to(fusion_box, UP, buff=0.1)

        fusion_desc = VGroup()
        fd1 = Text("Light nuclei combine", color=TRD_COLORS["text"], font_size=14)
        fd2 = MathTex(r"^2H + ^3H \to ^4He + n + E", color=TRD_COLORS["text"], font_size=18)
        fd3 = Text("Powers the Sun", color=TRD_COLORS["text_dim"], font_size=12)
        fusion_desc.add(fd1, fd2, fd3)
        fusion_desc.arrange(DOWN, buff=0.15)
        fusion_desc.move_to(fusion_box.get_center())

        self.play(Create(fusion_box), Write(fusion_title))
        self.play(Write(fusion_desc))

        # Fission
        self.add_marker("3.4.4.3", "fission")

        fission_box = RoundedRectangle(
            width=5, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_opacity=0.1,
        )
        fission_box.shift(DOWN * 1.3 + LEFT * 2)

        fission_title = Text("FISSION", color=TRD_COLORS["antimatter"], font_size=20, weight="BOLD")
        fission_title.next_to(fission_box, UP, buff=0.1)

        fission_desc = VGroup()
        fsd1 = Text("Heavy nuclei split", color=TRD_COLORS["text"], font_size=14)
        fsd2 = MathTex(r"^{235}U + n \to \text{fragments} + n + E", color=TRD_COLORS["text"], font_size=18)
        fsd3 = Text("Nuclear reactors", color=TRD_COLORS["text_dim"], font_size=12)
        fission_desc.add(fsd1, fsd2, fsd3)
        fission_desc.arrange(DOWN, buff=0.15)
        fission_desc.move_to(fission_box.get_center())

        self.play(Create(fission_box), Write(fission_title))
        self.play(Write(fission_desc))

        # Both release energy
        arrow = Arrow(
            RIGHT * 2 + UP * 0.5,
            RIGHT * 2 + DOWN * 0.5,
            color=TRD_COLORS["highlight"],
            stroke_width=4,
        )
        arrow_label = Text("Both → Fe\n(release E)", color=TRD_COLORS["highlight"], font_size=14)
        arrow_label.next_to(arrow, RIGHT, buff=0.2)

        self.play(GrowArrow(arrow), Write(arrow_label))

        self.wait(2)

        self.export_markers()


class NuclearDecay(TRDScene):
    """Radioactive decay in TRD."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.5.1", "decay")

        title = self.concept_card(
            "Radioactive Decay",
            "Unstable configurations seeking stability"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Types of decay
        self.add_marker("3.4.5.2", "types")

        decays = VGroup()

        # Alpha decay
        alpha = VGroup()
        alpha_label = Text("α decay:", color=TRD_COLORS["matter"], font_size=18, weight="BOLD")
        alpha_eq = MathTex(r"^A_Z X \to ^{A-4}_{Z-2}Y + ^4_2He", color=TRD_COLORS["text"], font_size=20)
        alpha_eq.next_to(alpha_label, RIGHT, buff=0.2)
        alpha_note = Text("Emits He-4 nucleus", color=TRD_COLORS["text_dim"], font_size=12)
        alpha_note.next_to(alpha_label, DOWN, buff=0.1, aligned_edge=LEFT)
        alpha.add(alpha_label, alpha_eq, alpha_note)

        # Beta decay
        beta = VGroup()
        beta_label = Text("β decay:", color=TRD_COLORS["antimatter"], font_size=18, weight="BOLD")
        beta_eq = MathTex(r"n \to p + e^- + \bar{\nu}_e", color=TRD_COLORS["text"], font_size=20)
        beta_eq.next_to(beta_label, RIGHT, buff=0.2)
        beta_note = Text("Weak force: d → u", color=TRD_COLORS["text_dim"], font_size=12)
        beta_note.next_to(beta_label, DOWN, buff=0.1, aligned_edge=LEFT)
        beta.add(beta_label, beta_eq, beta_note)

        # Gamma decay
        gamma = VGroup()
        gamma_label = Text("γ decay:", color=TRD_COLORS["glow"], font_size=18, weight="BOLD")
        gamma_eq = MathTex(r"X^* \to X + \gamma", color=TRD_COLORS["text"], font_size=20)
        gamma_eq.next_to(gamma_label, RIGHT, buff=0.2)
        gamma_note = Text("Excited state → photon", color=TRD_COLORS["text_dim"], font_size=12)
        gamma_note.next_to(gamma_label, DOWN, buff=0.1, aligned_edge=LEFT)
        gamma.add(gamma_label, gamma_eq, gamma_note)

        decays.add(alpha, beta, gamma)
        decays.arrange(DOWN, buff=0.5, aligned_edge=LEFT)
        decays.shift(LEFT * 0.5)

        for decay in decays:
            self.play(Write(decay), run_time=0.8)

        # TRD interpretation
        trd_note = Text(
            "TRD: Decay = flux configuration seeking lower energy state",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        trd_note.to_edge(DOWN, buff=0.5)
        self.play(Write(trd_note))

        self.wait(2)

        self.export_markers()


class NuclearPhysicsSummary(TRDScene):
    """Summary of nuclear physics."""

    def construct(self):
        self.load_narration("3.4")

        self.add_marker("3.4.6.1", "summary")

        title = self.trd_title("Nuclear Physics")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Strong force: short-range, 100× EM",
            "Yukawa potential: V ~ exp(-mr)/r",
            "Binding energy: mass deficit × c²",
            "Iron-56: maximum stability",
            "Fusion: light → heavy (releases E)",
            "Fission: heavy → light (releases E)",
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
            r"E = \Delta m \cdot c^2",
            "Mass-energy equivalence from TRD flux binding"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
