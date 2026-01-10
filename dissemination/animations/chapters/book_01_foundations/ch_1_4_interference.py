"""
Chapter 1.4: Interference
=========================

Wave interference patterns in the TRD flux field.
Shows how linear superposition of flux creates interference fringes.
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
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Text,
    MathTex,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS
from lib.components import (
    WavePulse,
    WaveFront,
    StandingWave,
    InterferencePattern,
    FluxWave,
)


class InterferenceIntro(TRDScene):
    """Introduction to wave interference in TRD."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.0.1", "title")
        title = self.trd_title("Interference")
        subtitle = Text(
            "Wave Superposition in the Flux Field",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Linear superposition principle
        self.add_marker("1.4.0.2", "superposition")

        principle = MathTex(
            r"J_{total} = J_1 + J_2",
            color=TRD_COLORS["highlight"],
            font_size=40,
        )
        principle_label = Text(
            "Linear Superposition",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        principle_label.next_to(principle, DOWN, buff=0.3)

        self.play(Write(principle))
        self.play(Write(principle_label))
        self.wait(2)
        self.play(FadeOut(principle), FadeOut(principle_label))

        self.export_markers()


class SingleSourceWave(TRDScene):
    """Single source wave propagation."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.1.1", "single_source")

        title = self.concept_card(
            "Single Source",
            "Circular wavefronts from a point source"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Source marker
        source = Dot(
            point=ORIGIN,
            radius=0.15,
            color=TRD_COLORS["highlight"],
        )
        source_glow = Circle(
            radius=0.25,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=2,
            stroke_opacity=0.5,
            fill_opacity=0,
        )

        self.play(FadeIn(source), FadeIn(source_glow))

        # Wave pulse
        self.add_marker("1.4.1.2", "wave_expand")

        pulse = WavePulse(
            center=ORIGIN,
            max_radius=4.0,
            color=TRD_COLORS["highlight"],
            num_rings=5,
        )
        self.add(pulse)
        self.play(pulse.expand(run_time=3.0))

        self.wait(1)
        self.play(FadeOut(source), FadeOut(source_glow), FadeOut(pulse))

        self.export_markers()


class TwoSourceInterference(TRDScene):
    """Two-source interference pattern."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.2.1", "two_sources")

        title = self.concept_card(
            "Two-Source Interference",
            "Constructive and destructive patterns"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Sources
        source1_pos = LEFT * 2
        source2_pos = RIGHT * 2

        source1 = Dot(point=source1_pos, radius=0.12, color=TRD_COLORS["matter"])
        source2 = Dot(point=source2_pos, radius=0.12, color=TRD_COLORS["antimatter"])

        source1_label = Text("Source 1", color=TRD_COLORS["matter"], font_size=16)
        source1_label.next_to(source1, DOWN, buff=0.2)
        source2_label = Text("Source 2", color=TRD_COLORS["antimatter"], font_size=16)
        source2_label.next_to(source2, DOWN, buff=0.2)

        self.play(
            FadeIn(source1), FadeIn(source2),
            Write(source1_label), Write(source2_label),
        )

        # Interference pattern
        self.add_marker("1.4.2.2", "pattern")

        pattern = InterferencePattern(
            source1=source1_pos,
            source2=source2_pos,
            wavelength=0.8,
            extent=5.0,
            resolution=40,
        )

        self.play(FadeIn(pattern, run_time=2.0))

        # Labels for constructive/destructive
        constructive = Text(
            "Bright: Constructive",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        constructive.to_edge(UP, buff=1.0)

        destructive = Text(
            "Dark: Destructive",
            color=TRD_COLORS["void_light"],
            font_size=18,
        )
        destructive.next_to(constructive, DOWN, buff=0.2)

        self.play(Write(constructive), Write(destructive))

        # Animate evolution
        self.add_marker("1.4.2.3", "evolve")
        self.play(pattern.animate_interference(run_time=4.0, cycles=2.0))

        self.wait(1)
        self.play(
            FadeOut(pattern), FadeOut(source1), FadeOut(source2),
            FadeOut(source1_label), FadeOut(source2_label),
            FadeOut(constructive), FadeOut(destructive),
        )

        self.export_markers()


class InterferenceMath(TRDScene):
    """Mathematical description of interference."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.3.1", "math")

        title = self.trd_title("Interference Mathematics")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Wave equations
        eq1 = MathTex(
            r"J_1 = A \sin(kr_1 - \omega t)",
            color=TRD_COLORS["matter"],
            font_size=28,
        )
        eq2 = MathTex(
            r"J_2 = A \sin(kr_2 - \omega t)",
            color=TRD_COLORS["antimatter"],
            font_size=28,
        )

        eq_group = VGroup(eq1, eq2)
        eq_group.arrange(DOWN, buff=0.4)
        eq_group.shift(UP * 0.5)

        self.play(Write(eq1))
        self.play(Write(eq2))

        # Superposition
        self.add_marker("1.4.3.2", "superpose")

        eq_sum = MathTex(
            r"J_{total} = 2A \cos\left(\frac{k(r_1-r_2)}{2}\right) \sin\left(k\bar{r} - \omega t\right)",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        eq_sum.next_to(eq_group, DOWN, buff=0.6)

        self.play(Write(eq_sum))

        # Conditions
        conditions = VGroup()

        cond1 = MathTex(
            r"\Delta r = n\lambda \implies \text{Constructive (max)}",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        cond2 = MathTex(
            r"\Delta r = (n+\frac{1}{2})\lambda \implies \text{Destructive (zero)}",
            color=TRD_COLORS["text"],
            font_size=22,
        )

        conditions.add(cond1, cond2)
        conditions.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        conditions.to_edge(DOWN, buff=1.0)

        self.play(Write(cond1))
        self.play(Write(cond2))

        self.wait(2)

        self.export_markers()


class LatticeInterference(TRDScene):
    """Interference on the discrete TRD lattice."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.4.1", "lattice")

        title = self.concept_card(
            "Lattice Interference",
            "Discrete wave equation on the cubic grid"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Flux wave on grid
        self.add_marker("1.4.4.2", "grid_wave")

        flux_wave = FluxWave(
            grid_size=13,
            spacing=0.5,
            wave_speed=1.0,
        )

        self.play(FadeIn(flux_wave))

        # Initial pulse
        flux_wave.set_initial_pulse((6, 6))

        # Propagate
        self.add_marker("1.4.4.3", "propagate")
        self.play(flux_wave.propagate(run_time=4.0))

        # Note about discrete Laplacian
        note = MathTex(
            r"\nabla^2 J \approx \sum_{neighbors} J_n - 6J_{center}",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        note.to_edge(DOWN, buff=0.8)
        self.play(Write(note))

        self.wait(2)
        self.play(FadeOut(flux_wave), FadeOut(note))

        self.export_markers()


class InterferenceSummary(TRDScene):
    """Summary of interference in TRD."""

    def construct(self):
        self.load_narration("1.4")

        self.add_marker("1.4.5.1", "summary")

        title = self.trd_title("Interference in TRD")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Flux vectors add linearly (superposition)",
            "Phase differences create fringes",
            "Discrete lattice preserves wave behavior",
            "Interference is pre-manifestation",
            "Explains quantum-like diffraction",
        ]

        point_mobs = VGroup()
        for i, point in enumerate(points):
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=24)
            text = Text(point, color=TRD_COLORS["text"], font_size=20)
            text.next_to(bullet, RIGHT, buff=0.2)
            group = VGroup(bullet, text)
            point_mobs.add(group)

        point_mobs.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.6)

        self.wait(2)

        # Final equation
        final = self.equation_box(
            r"J_{total}(v) = \sum_i J_i(v)",
            "All flux contributions sum at each voxel"
        )
        final.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
