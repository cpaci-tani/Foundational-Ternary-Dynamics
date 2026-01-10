"""
Chapter 1.2: The First Division
===============================

Genesis animation showing pair production from the void.
Flux accumulates until threshold, then matter/antimatter emerge.
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
    Wait,
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
from lib.colors import TRD_COLORS, GLOW_COLORS
from lib.components import (
    VoxelMobject,
    VoxelGrid,
    FluxFieldMobject,
    WavePulse,
    ZoomPulse,
)


class GenesisIntro(TRDScene):
    """Introduction to the genesis concept."""

    def construct(self):
        # Load narration timing
        self.load_narration("1.2")

        # Title
        self.add_marker("1.2.0.1", "title")
        title = self.trd_title("The First Division")
        subtitle = Text(
            "Genesis: From Void to Manifestation",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The void
        self.add_marker("1.2.0.2", "void_intro")
        void_text = Text(
            "The Void: s = 0",
            color=TRD_COLORS["void_light"],
            font_size=32,
        )
        void_text.to_edge(UP, buff=1.0)

        # Empty voxel grid
        grid = VoxelGrid(rows=5, cols=5, voxel_size=0.6, spacing=0.8, default_state=0)
        grid.shift(DOWN * 0.5)

        self.play(Write(void_text))
        self.play(FadeIn(grid, run_time=2.0))
        self.wait(1)

        # Explanation
        void_desc = Text(
            "A substrate awaiting activation",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        void_desc.next_to(grid, DOWN, buff=0.5)
        self.play(Write(void_desc))
        self.wait(2)

        self.play(FadeOut(void_text), FadeOut(grid), FadeOut(void_desc))

        # Export timing markers
        self.export_markers()


class FluxAccumulation(TRDScene):
    """Shows flux building up until threshold."""

    def construct(self):
        self.load_narration("1.2")

        # Title card
        self.add_marker("1.2.1.1", "flux_title")
        title = self.concept_card(
            "Flux Accumulation",
            "Energy density builds in the void substrate"
        )
        self.play(FadeIn(title))
        self.wait(1.5)
        self.play(FadeOut(title))

        # Central void voxel
        self.add_marker("1.2.1.2", "central_voxel")
        center_voxel = VoxelMobject(state=0, size=1.5, show_glow=False)
        self.play(FadeIn(center_voxel))

        # Flux arrows converging
        self.add_marker("1.2.1.3", "flux_converge")
        flux_field = FluxFieldMobject(
            rows=7, cols=7,
            spacing=0.8,
            arrow_scale=0.4,
        )
        flux_field.shift(DOWN * 0.3)

        self.play(FadeIn(flux_field))
        self.wait(0.5)

        # Animate flux accumulating toward center
        self.play(flux_field.accumulate(center=ORIGIN, run_time=3.0))

        # Threshold indicator
        self.add_marker("1.2.1.4", "threshold")
        threshold_text = MathTex(
            r"|J| > K_B",
            color=TRD_COLORS["highlight"],
            font_size=36,
        )
        threshold_text.to_edge(UP, buff=1.0)
        self.play(Write(threshold_text))

        # Glow intensifies
        glow_pulse = ZoomPulse(
            center=ORIGIN,
            color=TRD_COLORS["highlight"],
            num_rings=4,
        )
        self.add(glow_pulse)
        self.play(glow_pulse.pulse_in(start_radius=3.0, run_time=1.5))

        self.wait(1)
        self.play(
            FadeOut(flux_field),
            FadeOut(center_voxel),
            FadeOut(threshold_text),
            FadeOut(glow_pulse),
        )

        self.export_markers()


class PairProduction(TRDScene):
    """The climactic moment: pair production from void."""

    def construct(self):
        self.load_narration("1.2")

        # Setup
        self.add_marker("1.2.2.1", "pair_setup")

        # Central high-density region
        center_glow = Circle(
            radius=0.8,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.3,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=3,
        )

        # Outer glow layers
        glows = VGroup()
        for i in range(4, 0, -1):
            glow = Circle(
                radius=0.8 + i * 0.3,
                stroke_color=TRD_COLORS["highlight"],
                stroke_width=2,
                stroke_opacity=0.2 / i,
                fill_opacity=0,
            )
            glows.add(glow)

        self.play(FadeIn(glows), FadeIn(center_glow))

        # Building intensity
        self.add_marker("1.2.2.2", "intensity_build")
        intensity_text = Text(
            "Flux density exceeds threshold...",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        intensity_text.to_edge(UP, buff=1.0)
        self.play(Write(intensity_text))

        # Pulse effect
        for _ in range(3):
            self.play(
                center_glow.animate.scale(1.2),
                rate_func=lambda t: np.sin(t * PI),
                run_time=0.4,
            )

        self.play(FadeOut(intensity_text))

        # THE SPLIT - pair production
        self.add_marker("1.2.2.3", "pair_creation")

        # Flash
        flash = Circle(
            radius=0.1,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=1.0,
            stroke_opacity=0,
        )
        self.play(
            flash.animate.scale(30).set_opacity(0),
            run_time=0.5,
        )

        # Remove center, create pair
        self.remove(center_glow, glows, flash)

        # Matter particle (+1)
        matter = VoxelMobject(state=+1, size=1.0, show_glow=True)
        matter.shift(LEFT * 2)
        matter.set_opacity(0)

        # Antimatter particle (-1)
        antimatter = VoxelMobject(state=-1, size=1.0, show_glow=True)
        antimatter.shift(RIGHT * 2)
        antimatter.set_opacity(0)

        # Labels
        matter_label = MathTex(
            r"s = +1",
            color=TRD_COLORS["matter"],
            font_size=28,
        )
        matter_label.next_to(matter, DOWN, buff=0.3)

        antimatter_label = MathTex(
            r"s = -1",
            color=TRD_COLORS["antimatter"],
            font_size=28,
        )
        antimatter_label.next_to(antimatter, DOWN, buff=0.3)

        # Dramatic emergence
        self.add(matter, antimatter)
        self.play(
            matter.animate.set_opacity(1),
            antimatter.animate.set_opacity(1),
            GrowFromCenter(matter),
            GrowFromCenter(antimatter),
            run_time=1.5,
        )

        self.play(
            Write(matter_label),
            Write(antimatter_label),
        )

        # Title
        genesis_title = Text(
            "GENESIS",
            color=TRD_COLORS["highlight"],
            font_size=48,
            weight="BOLD",
        )
        genesis_title.to_edge(UP, buff=0.8)
        self.play(Write(genesis_title))

        self.wait(2)

        # Conservation note
        self.add_marker("1.2.2.4", "conservation")
        conservation = MathTex(
            r"\sum s = (+1) + (-1) = 0",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        conservation.to_edge(DOWN, buff=1.0)
        self.play(Write(conservation))

        conservation_note = Text(
            "Total charge conserved",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        conservation_note.next_to(conservation, DOWN, buff=0.2)
        self.play(Write(conservation_note))

        self.wait(2)

        self.export_markers()


class GenesisEquations(TRDScene):
    """Mathematical formulation of genesis."""

    def construct(self):
        self.load_narration("1.2")

        self.add_marker("1.2.3.1", "equations")

        # Title
        title = self.trd_title("Genesis Dynamics")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Equation 1: Threshold condition
        eq1_label = Text("Threshold Condition:", color=TRD_COLORS["text"], font_size=20)
        eq1 = MathTex(
            r"|J(v)| > K_B",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        eq1_group = VGroup(eq1_label, eq1).arrange(DOWN, buff=0.2)
        eq1_group.shift(UP * 1.5 + LEFT * 3)

        # Equation 2: Probability
        eq2_label = Text("Manifestation Probability:", color=TRD_COLORS["text"], font_size=20)
        eq2 = MathTex(
            r"P = 1 - e^{-(|J| - K_B)/K_B}",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        eq2_group = VGroup(eq2_label, eq2).arrange(DOWN, buff=0.2)
        eq2_group.shift(UP * 1.5 + RIGHT * 3)

        # Equation 3: Polarity selection
        eq3_label = Text("Polarity Selection:", color=TRD_COLORS["text"], font_size=20)
        eq3 = MathTex(
            r"s = \text{sign}(\nabla \cdot J)",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        eq3_group = VGroup(eq3_label, eq3).arrange(DOWN, buff=0.2)
        eq3_group.shift(DOWN * 0.5)

        # Animate equations
        self.play(Write(eq1_label), Write(eq1))
        self.wait(1)
        self.play(Write(eq2_label), Write(eq2))
        self.wait(1)
        self.play(Write(eq3_label), Write(eq3))
        self.wait(2)

        # Summary box
        summary = self.equation_box(
            r"0 \xrightarrow{|J|>K_B} \pm 1",
            "The void becomes manifest"
        )
        summary.to_edge(DOWN, buff=0.8)
        self.play(FadeIn(summary))

        self.wait(2)

        self.export_markers()


class FirstDivisionSummary(TRDScene):
    """Chapter summary with key takeaways."""

    def construct(self):
        self.load_narration("1.2")

        self.add_marker("1.2.4.1", "summary")

        # Title
        title = self.trd_title("The First Division")
        subtitle = Text("Key Insights", color=TRD_COLORS["text_dim"], font_size=24)
        subtitle.next_to(title, DOWN, buff=0.2)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(1)
        self.play(
            title.animate.to_edge(UP, buff=0.5).scale(0.8),
            FadeOut(subtitle),
        )

        # Key points
        points = [
            ("1.", "Void is substrate, not empty space"),
            ("2.", "Flux accumulates dispositional potential"),
            ("3.", "Threshold crossing triggers manifestation"),
            ("4.", "Pairs emerge conserving total charge"),
            ("5.", "Genesis is the birth of duality"),
        ]

        point_mobs = VGroup()
        for num, text in points:
            num_mob = Text(num, color=TRD_COLORS["highlight"], font_size=24)
            text_mob = Text(text, color=TRD_COLORS["text"], font_size=22)
            text_mob.next_to(num_mob, RIGHT, buff=0.2)
            point_group = VGroup(num_mob, text_mob)
            point_mobs.add(point_group)

        point_mobs.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.8)
            self.wait(0.3)

        self.wait(2)

        # Final visual
        self.play(FadeOut(point_mobs), FadeOut(title))

        # Iconic pair
        matter = VoxelMobject(state=+1, size=0.8, show_glow=True)
        matter.shift(LEFT * 1.5)
        antimatter = VoxelMobject(state=-1, size=0.8, show_glow=True)
        antimatter.shift(RIGHT * 1.5)

        pair = VGroup(matter, antimatter)
        self.play(GrowFromCenter(pair))

        final_text = Text(
            "From One, Two",
            color=TRD_COLORS["text"],
            font_size=32,
        )
        final_text.to_edge(DOWN, buff=1.5)
        self.play(Write(final_text))

        self.wait(2)

        self.export_markers()
