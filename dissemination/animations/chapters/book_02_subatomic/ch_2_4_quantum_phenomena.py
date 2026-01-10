"""
Chapter 2.4: Quantum Phenomena
==============================

Hilbert space construction and the sLoop mechanism.
Shows how quantum mechanics emerges from TRD.
"""

from __future__ import annotations

import numpy as np

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    OUT,
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
    CurvedArrow,
    Arc,
    Text,
    MathTex,
    RoundedRectangle,
    Axes,
    ParametricFunction,
    ComplexPlane,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS
from lib.components import VoxelMobject, FluxArrow, WavePulse


class QuantumIntro(TRDScene):
    """Introduction to quantum phenomena in TRD."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.0.1", "title")
        title = self.trd_title("Quantum Phenomena")
        subtitle = Text(
            "From Flux to Wave Functions",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The puzzle
        self.add_marker("2.4.0.2", "puzzle")

        puzzle = Text(
            "How does quantum mechanics emerge from discrete updates?",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        self.play(Write(puzzle))
        self.wait(2)
        self.play(FadeOut(puzzle))

        self.export_markers()


class HilbertSpaceConstruction(TRDScene):
    """Building Hilbert space from flux."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.1.1", "hilbert")

        title = self.concept_card(
            "Hilbert Space Construction",
            "Complexifying the flux field"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Flux to wave function
        self.add_marker("2.4.1.2", "complexify")

        # Real flux
        flux_label = Text("Real flux:", color=TRD_COLORS["text"], font_size=20)
        flux_label.shift(UP * 2 + LEFT * 3)

        flux_eq = MathTex(
            r"\mathbf{J}(v) = (J_x, J_y, J_z) \in \mathbb{R}^3",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        flux_eq.next_to(flux_label, RIGHT, buff=0.3)

        self.play(Write(flux_label), Write(flux_eq))

        # Arrow down
        arrow1 = Arrow(
            start=flux_eq.get_bottom() + DOWN * 0.2,
            end=flux_eq.get_bottom() + DOWN * 1.0,
            color=TRD_COLORS["highlight"],
        )
        complexify_label = Text("Complexify", color=TRD_COLORS["highlight"], font_size=16)
        complexify_label.next_to(arrow1, RIGHT, buff=0.1)

        self.play(GrowArrow(arrow1), Write(complexify_label))

        # Wave function
        self.add_marker("2.4.1.3", "wavefunction")

        psi_label = Text("Wave function:", color=TRD_COLORS["text"], font_size=20)
        psi_label.shift(DOWN * 0.5 + LEFT * 3)

        psi_eq = MathTex(
            r"\psi(v) = J_x + i J_y \in \mathbb{C}",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        psi_eq.next_to(psi_label, RIGHT, buff=0.3)

        self.play(Write(psi_label), Write(psi_eq))

        # Hilbert space
        hilbert_eq = MathTex(
            r"\mathcal{H}_{TRD} = L^2(\text{Lattice}, \mathbb{C})",
            color=TRD_COLORS["matter"],
            font_size=26,
        )
        hilbert_eq.shift(DOWN * 2)

        self.play(Write(hilbert_eq))

        # Norm
        norm_eq = MathTex(
            r"\|\psi\|^2 = \sum_v |\psi(v)|^2",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        norm_eq.next_to(hilbert_eq, DOWN, buff=0.4)

        self.play(Write(norm_eq))

        self.wait(2)

        self.export_markers()


class BornRuleEmergence(TRDScene):
    """How the Born rule emerges."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.2.1", "born")

        title = self.concept_card(
            "The Born Rule",
            "Probability from manifestation"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Born rule equation
        self.add_marker("2.4.2.2", "probability")

        born = MathTex(
            r"P(v) = \frac{|\psi(v)|^2}{\|\psi\|^2}",
            color=TRD_COLORS["highlight"],
            font_size=40,
        )
        born.shift(UP * 1.5)

        self.play(Write(born))

        # Connection to TRD
        self.add_marker("2.4.2.3", "connection")

        connection = VGroup()

        c1 = Text("In TRD:", color=TRD_COLORS["text"], font_size=18, weight="BOLD")
        c2 = Text("• Manifestation occurs when |J|² > K_B", color=TRD_COLORS["text"], font_size=16)
        c3 = Text("• Higher flux density → higher probability", color=TRD_COLORS["text"], font_size=16)
        c4 = Text("• Conservation requires normalization", color=TRD_COLORS["text"], font_size=16)

        connection.add(c1, c2, c3, c4)
        connection.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        connection.shift(DOWN * 0.5)

        for c in connection:
            self.play(Write(c), run_time=0.5)

        # Key insight
        insight = MathTex(
            r"|\psi|^2 \propto \text{flux density} \propto P(\text{manifest})",
            color=TRD_COLORS["highlight"],
            font_size=22,
        )
        insight.to_edge(DOWN, buff=0.8)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class SuperpositionMeaning(TRDScene):
    """What superposition means in TRD."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.3.1", "superposition")

        title = self.concept_card(
            "Superposition",
            "Not in state, but in flux"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Key distinction
        self.add_marker("2.4.3.2", "distinction")

        standard = VGroup()
        std_label = Text("Standard QM:", color=TRD_COLORS["text_dim"], font_size=18)
        std_eq = MathTex(
            r"|\psi\rangle = \alpha|0\rangle + \beta|1\rangle",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        std_eq.next_to(std_label, RIGHT, buff=0.2)
        std_note = Text("(State IS superposition)", color=TRD_COLORS["text_dim"], font_size=14)
        std_note.next_to(std_eq, DOWN, buff=0.1)
        standard.add(std_label, std_eq, std_note)
        standard.shift(UP * 1.5)

        self.play(Write(standard))

        trd = VGroup()
        trd_label = Text("TRD:", color=TRD_COLORS["highlight"], font_size=18)
        trd_state = MathTex(
            r"s \in \{-1, 0, +1\}",
            color=TRD_COLORS["matter"],
            font_size=24,
        )
        trd_state.next_to(trd_label, RIGHT, buff=0.2)
        trd_note = Text("(State is DEFINITE)", color=TRD_COLORS["matter"], font_size=14)
        trd_note.next_to(trd_state, DOWN, buff=0.1)
        trd.add(trd_label, trd_state, trd_note)
        trd.shift(DOWN * 0.5)

        self.play(Write(trd))

        # Flux is what superposes
        self.add_marker("2.4.3.3", "flux_superpose")

        flux_super = VGroup()
        fs_label = Text("Superposition in flux:", color=TRD_COLORS["antimatter"], font_size=18)
        fs_eq = MathTex(
            r"\psi = \psi_1 + \psi_2",
            color=TRD_COLORS["antimatter"],
            font_size=28,
        )
        fs_eq.next_to(fs_label, RIGHT, buff=0.2)
        fs_note = Text("(Before manifestation)", color=TRD_COLORS["antimatter"], font_size=14)
        fs_note.next_to(fs_eq, DOWN, buff=0.1)
        flux_super.add(fs_label, fs_eq, fs_note)
        flux_super.shift(DOWN * 2.2)

        self.play(Write(flux_super))

        self.wait(2)

        self.export_markers()


class EntanglementScene(TRDScene):
    """Entanglement as shared origin."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.4.1", "entanglement")

        title = self.concept_card(
            "Entanglement",
            "Shared origin, correlated fates"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Pair production
        self.add_marker("2.4.4.2", "pair")

        # Central void
        void = Circle(
            radius=0.5,
            fill_color=TRD_COLORS["void"],
            fill_opacity=0.5,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        void_label = Text("High-density void", color=TRD_COLORS["glow"], font_size=14)
        void_label.next_to(void, UP, buff=0.2)

        self.play(Create(void), Write(void_label))

        # Pair creation
        self.add_marker("2.4.4.3", "correlate")

        particle1 = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        label1 = MathTex(r"+1", color=TRD_COLORS["background"], font_size=20)
        label1.move_to(particle1.get_center())

        particle2 = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        label2 = MathTex(r"-1", color=TRD_COLORS["background"], font_size=20)
        label2.move_to(particle2.get_center())

        # Animation: particles emerge and separate
        self.play(
            void.animate.set_opacity(0.2),
            FadeIn(particle1), FadeIn(label1),
            FadeIn(particle2), FadeIn(label2),
        )

        self.play(
            particle1.animate.shift(LEFT * 2.5),
            label1.animate.shift(LEFT * 2.5),
            particle2.animate.shift(RIGHT * 2.5),
            label2.animate.shift(RIGHT * 2.5),
        )

        # Correlation line
        correlation = Line(
            particle1.get_center(),
            particle2.get_center(),
            color=TRD_COLORS["glow"],
            stroke_width=2,
            stroke_opacity=0.5,
        )
        corr_label = Text("Shared partner_uuid", color=TRD_COLORS["glow"], font_size=14)
        corr_label.next_to(correlation, UP, buff=0.1)

        self.play(Create(correlation), Write(corr_label))

        # Explanation
        explanation = VGroup()
        e1 = Text("• Created simultaneously from same void", color=TRD_COLORS["text"], font_size=16)
        e2 = Text("• Correlated properties established at creation", color=TRD_COLORS["text"], font_size=16)
        e3 = Text("• No faster-than-light signaling", color=TRD_COLORS["text"], font_size=16)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        explanation.to_edge(DOWN, buff=0.6)

        for e in explanation:
            self.play(Write(e), run_time=0.5)

        self.wait(2)

        self.export_markers()


class SLoopIntroduction(TRDScene):
    """Introduction to the sLoop concept."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.5.1", "sloop")

        title = self.concept_card(
            "The sLoop",
            "Self-referential observer coupling"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Standard observation
        self.add_marker("2.4.5.2", "standard")

        std_label = Text("Standard view:", color=TRD_COLORS["text_dim"], font_size=18)
        std_label.shift(UP * 2 + LEFT * 3)

        observer = RoundedRectangle(
            width=1.8, height=0.8,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["text_dim"],
        )
        observer.shift(UP * 2 + LEFT * 0.5)
        obs_label = Text("Observer", color=TRD_COLORS["text_dim"], font_size=14)
        obs_label.move_to(observer.get_center())

        system = RoundedRectangle(
            width=1.8, height=0.8,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["text_dim"],
        )
        system.shift(UP * 2 + RIGHT * 2)
        sys_label = Text("System", color=TRD_COLORS["text_dim"], font_size=14)
        sys_label.move_to(system.get_center())

        arrow = Arrow(
            observer.get_right(),
            system.get_left(),
            color=TRD_COLORS["text_dim"],
            buff=0.1,
        )

        self.play(
            Write(std_label),
            Create(observer), Write(obs_label),
            Create(system), Write(sys_label),
            GrowArrow(arrow),
        )

        # sLoop view
        self.add_marker("2.4.5.3", "sloop_view")

        sloop_label = Text("sLoop (TRD):", color=TRD_COLORS["highlight"], font_size=18)
        sloop_label.shift(DOWN * 0.5 + LEFT * 3)

        substrate = RoundedRectangle(
            width=6, height=2.5,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["highlight"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.3,
        )
        substrate.shift(DOWN * 1.5)
        sub_label = Text("Flux Substrate", color=TRD_COLORS["highlight"], font_size=16)
        sub_label.next_to(substrate, UP, buff=0.1)

        obs2 = Circle(
            radius=0.5,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.3,
            stroke_color=TRD_COLORS["matter"],
        )
        obs2.shift(DOWN * 1.5 + LEFT * 1.5)
        obs2_label = Text("Observer", color=TRD_COLORS["matter"], font_size=12)
        obs2_label.move_to(obs2.get_center())

        sys2 = Circle(
            radius=0.5,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.3,
            stroke_color=TRD_COLORS["antimatter"],
        )
        sys2.shift(DOWN * 1.5 + RIGHT * 1.5)
        sys2_label = Text("System", color=TRD_COLORS["antimatter"], font_size=12)
        sys2_label.move_to(sys2.get_center())

        # Bidirectional coupling
        loop = CurvedArrow(
            obs2.get_right(),
            sys2.get_left(),
            color=TRD_COLORS["glow"],
            angle=-PI/4,
        )
        loop2 = CurvedArrow(
            sys2.get_left() + DOWN * 0.2,
            obs2.get_right() + DOWN * 0.2,
            color=TRD_COLORS["glow"],
            angle=PI/4,
        )

        self.play(
            Write(sloop_label),
            Create(substrate), Write(sub_label),
        )
        self.play(
            Create(obs2), Write(obs2_label),
            Create(sys2), Write(sys2_label),
        )
        self.play(Create(loop), Create(loop2))

        # Key insight
        insight = Text(
            "Observer and system share the same flux substrate",
            color=TRD_COLORS["glow"],
            font_size=18,
        )
        insight.to_edge(DOWN, buff=0.3)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class BellViolation(TRDScene):
    """Bell inequality violations in TRD."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.6.1", "bell")

        title = self.concept_card(
            "Bell Violations",
            "Substrate overlap → quantum correlations"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Classical bound
        self.add_marker("2.4.6.2", "bounds")

        classical = MathTex(
            r"S \leq 2 \text{ (classical bound)}",
            color=TRD_COLORS["text_dim"],
            font_size=28,
        )
        classical.shift(UP * 2)

        quantum = MathTex(
            r"S \leq 2\sqrt{2} \approx 2.83 \text{ (quantum bound)}",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        quantum.next_to(classical, DOWN, buff=0.4)

        self.play(Write(classical))
        self.play(Write(quantum))

        # TRD result
        self.add_marker("2.4.6.3", "result")

        result = MathTex(
            r"S_{TRD} \approx 2.85",
            color=TRD_COLORS["matter"],
            font_size=36,
        )
        result.next_to(quantum, DOWN, buff=0.6)

        self.play(Write(result))
        self.play(Flash(result, color=TRD_COLORS["glow"], flash_radius=0.8))

        # Mechanism
        mechanism = VGroup()
        m1 = Text("How TRD achieves this:", color=TRD_COLORS["text"], font_size=18, weight="BOLD")
        m2 = Text("• Hilbert space tensor product structure", color=TRD_COLORS["text"], font_size=16)
        m3 = Text("• Entangled pair shares flux substrate", color=TRD_COLORS["text"], font_size=16)
        m4 = Text("• Measurement couples via same substrate", color=TRD_COLORS["text"], font_size=16)

        mechanism.add(m1, m2, m3, m4)
        mechanism.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        mechanism.to_edge(DOWN, buff=0.5)

        for m in mechanism:
            self.play(Write(m), run_time=0.5)

        self.wait(2)

        self.export_markers()


class QuantumSummary(TRDScene):
    """Summary of quantum phenomena in TRD."""

    def construct(self):
        self.load_narration("2.4")

        self.add_marker("2.4.7.1", "summary")

        title = self.trd_title("Quantum Phenomena")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key achievements
        points = [
            ("Hilbert space", r"\mathcal{H} = L^2(\text{Lattice}, \mathbb{C})"),
            ("Wave function", r"\psi = J_x + i J_y"),
            ("Born rule", r"P(v) = |\psi(v)|^2 / \|\psi\|^2"),
            ("Superposition", r"\text{In flux, not in state}"),
            ("Entanglement", r"\text{Shared origin + substrate}"),
            ("Bell violations", r"S \approx 2.85 \leq 2\sqrt{2}"),
        ]

        point_mobs = VGroup()
        for label, math in points:
            label_mob = Text(label + ":", color=TRD_COLORS["highlight"], font_size=18)
            math_mob = MathTex(math, color=TRD_COLORS["text"], font_size=20)
            math_mob.next_to(label_mob, RIGHT, buff=0.2)
            point_mobs.add(VGroup(label_mob, math_mob))

        point_mobs.arrange(DOWN, buff=0.35, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.5)

        self.wait(2)

        # Final insight
        final = self.equation_box(
            r"\text{QM} \subset \text{TRD}",
            "Quantum mechanics emerges from discrete dynamics"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
