"""
Chapter 2.1: The Planck Scale
=============================

Zoom into the fundamental scale of TRD.
Shows the discrete lattice structure at Planck length.
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
    ScaleInPlace,
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
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS
from lib.components import (
    ScaleJourney,
    ScaleMarker,
    ScaleRuler,
    ZoomBox,
    Lattice2D,
    Lattice3D,
    VoxelMobject,
)


class PlanckIntro(TRDScene):
    """Introduction to the Planck scale."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.0.1", "title")
        title = self.trd_title("The Planck Scale")
        subtitle = Text(
            "The Smallest Possible Length",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The numbers
        self.add_marker("2.1.0.2", "numbers")

        planck_length = MathTex(
            r"\ell_P = \sqrt{\frac{\hbar G}{c^3}} \approx 1.6 \times 10^{-35} \text{ m}",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        planck_time = MathTex(
            r"t_P = \frac{\ell_P}{c} \approx 5.4 \times 10^{-44} \text{ s}",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        planck_mass = MathTex(
            r"m_P = \sqrt{\frac{\hbar c}{G}} \approx 2.2 \times 10^{-8} \text{ kg}",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )

        planck_group = VGroup(planck_length, planck_time, planck_mass)
        planck_group.arrange(DOWN, buff=0.5)

        for eq in planck_group:
            self.play(Write(eq), run_time=0.8)

        self.wait(2)
        self.play(FadeOut(planck_group))

        self.export_markers()


class ScaleZoomDown(TRDScene):
    """Zoom from human scale to Planck scale."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.1.1", "zoom_start")

        title = self.concept_card(
            "Zooming In",
            "From meters to Planck lengths"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Scale journey
        self.add_marker("2.1.1.2", "journey")

        journey = ScaleJourney(
            start_scale="human",
            end_scale="planck",
            show_labels=True,
        )

        self.play(Create(journey, run_time=2.0))
        self.play(journey.animate_journey(run_time=8.0))

        self.wait(2)
        self.play(FadeOut(journey))

        self.export_markers()


class LatticeReveal(TRDScene):
    """Reveal the discrete lattice at Planck scale."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.2.1", "reveal")

        title = self.concept_card(
            "The Discrete Substrate",
            "Space is quantized at the Planck scale"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Start with continuous-looking space
        self.add_marker("2.1.2.2", "continuous")

        continuous = Text(
            "Continuous space?",
            color=TRD_COLORS["text_dim"],
            font_size=28,
        )
        continuous.to_edge(UP, buff=1.0)
        self.play(Write(continuous))

        # Smooth gradient background
        smooth_bg = Square(
            side_length=5.0,
            fill_opacity=0.3,
            fill_color=TRD_COLORS["void_light"],
            stroke_width=0,
        )
        self.play(FadeIn(smooth_bg))

        # Transition to discrete
        self.add_marker("2.1.2.3", "discrete")

        self.play(
            continuous.animate.set_opacity(0.3),
            smooth_bg.animate.set_opacity(0.1),
        )

        discrete = Text(
            "No - Discrete lattice!",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        discrete.to_edge(UP, buff=1.0)
        self.play(FadeIn(discrete))

        # 2D lattice
        lattice = Lattice2D(rows=9, cols=9, spacing=0.55, show_glow=True)
        self.play(
            FadeOut(smooth_bg),
            FadeIn(lattice, run_time=2.0),
        )

        # Label
        label = MathTex(
            r"\text{1 voxel} = \ell_P \approx 10^{-35} \text{ m}",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        label.to_edge(DOWN, buff=0.8)
        self.play(Write(label))

        self.wait(2)

        self.export_markers()


class ThreeDimensionalLattice(TRDScene):
    """Show the 3D cubic lattice structure."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.3.1", "3d_lattice")

        title = self.trd_title("The Cubic Lattice")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # 3D lattice
        self.add_marker("2.1.3.2", "cube")

        lattice3d = Lattice3D(
            size=4,
            spacing=0.7,
            show_glow=True,
            glow_intensity=0.5,
        )
        self.play(Create(lattice3d, run_time=3.0))

        # Rotate to show 3D nature
        self.add_marker("2.1.3.3", "rotate")
        self.play(lattice3d.rotate_view(angle=PI/6, run_time=2.0))

        # Properties
        props = VGroup()
        p1 = Text("• Each point: one voxel", color=TRD_COLORS["text"], font_size=18)
        p2 = Text("• Three states: {-1, 0, +1}", color=TRD_COLORS["text"], font_size=18)
        p3 = Text("• 26 neighbors (Moore)", color=TRD_COLORS["text"], font_size=18)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        props.to_edge(RIGHT, buff=0.5)

        for p in props:
            self.play(Write(p), run_time=0.5)

        self.wait(2)

        self.export_markers()


class VoxelAtPlanck(TRDScene):
    """Single voxel at Planck scale."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.4.1", "single_voxel")

        title = self.concept_card(
            "The Fundamental Unit",
            "One voxel = one Planck volume"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Single large voxel
        self.add_marker("2.1.4.2", "voxel")

        voxel = VoxelMobject(
            state=0,
            size=2.5,
            show_flux=False,
            show_glow=True,
        )
        self.play(Create(voxel, run_time=2.0))

        # State label
        state_label = Text(
            "State: 0 (Void)",
            color=TRD_COLORS["void_light"],
            font_size=24,
        )
        state_label.next_to(voxel, DOWN, buff=0.5)
        self.play(Write(state_label))

        # Show state transitions
        self.add_marker("2.1.4.3", "transitions")

        # Transition to +1
        self.play(voxel.set_state(+1))
        new_label = Text(
            "State: +1 (Matter)",
            color=TRD_COLORS["matter"],
            font_size=24,
        )
        new_label.next_to(voxel, DOWN, buff=0.5)
        self.play(Transform(state_label, new_label))

        self.wait(1)

        # Transition to -1
        self.play(voxel.set_state(-1))
        new_label2 = Text(
            "State: -1 (Antimatter)",
            color=TRD_COLORS["antimatter"],
            font_size=24,
        )
        new_label2.next_to(voxel, DOWN, buff=0.5)
        self.play(Transform(state_label, new_label2))

        self.wait(1)

        # Back to void
        self.play(voxel.set_state(0))
        new_label3 = Text(
            "State: 0 (Void)",
            color=TRD_COLORS["void_light"],
            font_size=24,
        )
        new_label3.next_to(voxel, DOWN, buff=0.5)
        self.play(Transform(state_label, new_label3))

        self.wait(2)

        self.export_markers()


class PlanckSummary(TRDScene):
    """Summary of Planck scale properties."""

    def construct(self):
        self.load_narration("2.1")

        self.add_marker("2.1.5.1", "summary")

        title = self.trd_title("The Planck Scale")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            ("Smallest length", r"\ell_P \approx 10^{-35} \text{ m}"),
            ("Smallest time", r"t_P \approx 10^{-44} \text{ s}"),
            ("Discrete structure", r"\text{3D cubic lattice}"),
            ("Speed limit", r"C = 1 \text{ voxel/tick}"),
            ("TRD foundation", r"\text{All physics from here}"),
        ]

        point_mobs = VGroup()
        for title_text, math_text in points:
            title_mob = Text(title_text, color=TRD_COLORS["highlight"], font_size=20)
            math_mob = MathTex(math_text, color=TRD_COLORS["text"], font_size=22)
            math_mob.next_to(title_mob, RIGHT, buff=0.3)
            group = VGroup(title_mob, math_mob)
            point_mobs.add(group)

        point_mobs.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.6)

        self.wait(2)

        # Final insight
        final = self.equation_box(
            r"\text{Reality} = \sum_{\text{voxels}} \text{local updates}",
            "Everything emerges from the lattice"
        )
        final.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
