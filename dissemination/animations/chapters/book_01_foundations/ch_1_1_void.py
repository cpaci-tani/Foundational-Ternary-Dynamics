"""
Chapter 1.1: The Void
=====================

The void substrate - the foundational layer of TRD reality.
Dark cinematic visualization with subtle flux shimmer.

This animation introduces:
- The void as dispositional substrate (state 0)
- The flux field as latent potential
- The contrast between emptiness and possibility
"""

from __future__ import annotations

import numpy as np
import sys
from pathlib import Path

# Add lib to path for imports
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    PI,
    TAU,
    config,
    FadeIn,
    FadeOut,
    Write,
    Create,
    GrowFromCenter,
    AnimationGroup,
    Succession,
    VGroup,
    Text,
    Circle,
    Dot,
    rate_functions,
)

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, GLOW_COLORS
from lib.components.voxel import VoxelMobject, VoxelGrid
from lib.components.flux_field import FluxFieldMobject


class VoidIntro(TRDScene):
    """Opening scene: introduce the concept of the void."""

    def construct(self):
        # Load narration timing
        self.load_narration("1.1")

        # === SCENE 1: Title Card ===
        self.add_marker("1.1.0.1", "title")
        self.chapter_intro(
            chapter_id="1.1",
            title="The Void",
            subtitle="Where existence begins",
        )

        # === SCENE 2: Empty Space ===
        self.add_marker("1.1.0.2", "empty_space")

        # Just darkness with a few subtle particles
        quote = Text(
            '"In the beginning, there was nothing."',
            color=TRD_COLORS["text_dim"],
            font_size=32,
            slant="ITALIC",
        )
        self.play(FadeIn(quote, run_time=2.0))
        self.wait(2.0)
        self.play(FadeOut(quote, run_time=1.5))

        # === SCENE 3: But Not Empty ===
        self.add_marker("1.1.0.3", "not_empty")

        quote2 = Text(
            '"But nothing is not empty."',
            color=TRD_COLORS["text"],
            font_size=36,
        )
        self.play(FadeIn(quote2, run_time=1.5))
        self.wait(2.0)

        # Fade and reveal void grid
        self.play(FadeOut(quote2), run_time=1.0)

        # === SCENE 4: The Void Grid ===
        self.add_marker("1.1.0.4", "void_grid")

        # Create a grid of void voxels
        grid = VoxelGrid(
            rows=7,
            cols=9,
            voxel_size=0.6,
            spacing=0.9,
            default_state=0,  # All void
        )
        grid.set_opacity(0)

        self.add(grid)
        self.play(
            grid.animate.set_opacity(1),
            run_time=2.0,
            rate_func=rate_functions.ease_out_cubic,
        )

        # Label
        label = Text(
            "The Void Substrate",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        label.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(label), run_time=0.8)
        self.wait(2.0)

        # === SCENE 5: Dispositional Nature ===
        self.add_marker("1.1.0.5", "dispositional")

        # Highlight center voxel
        center_voxel = grid.get_voxel(3, 4)
        if center_voxel:
            # Create glow around it
            glow = Circle(
                radius=0.5,
                color=TRD_COLORS["highlight"],
                stroke_width=2,
                stroke_opacity=0.8,
                fill_opacity=0,
            )
            glow.move_to(center_voxel.get_center())

            self.play(Create(glow), run_time=1.0)

            # Pulse effect
            self.play(
                glow.animate.scale(1.3).set_stroke(opacity=0.4),
                run_time=0.8,
            )
            self.play(
                glow.animate.scale(1 / 1.3).set_stroke(opacity=0.8),
                run_time=0.8,
            )

            # Text
            potential_text = Text(
                "Awaiting activation...",
                color=TRD_COLORS["text_dim"],
                font_size=24,
            )
            potential_text.next_to(glow, UP, buff=0.3)
            self.play(FadeIn(potential_text), run_time=0.6)
            self.wait(1.5)

            self.play(
                FadeOut(glow),
                FadeOut(potential_text),
                run_time=0.8,
            )

        # === SCENE 6: Flux Field Overlay ===
        self.add_marker("1.1.0.6", "flux_field")

        self.play(FadeOut(label), run_time=0.5)

        # Create subtle flux field
        def subtle_flux(x: float, y: float) -> tuple[float, float]:
            # Very subtle random-ish flow
            t = 0.3  # Static for now
            jx = 0.1 * np.sin(x + y + t)
            jy = 0.1 * np.cos(x - y + t)
            return (jx, jy)

        flux = FluxFieldMobject(
            rows=7,
            cols=9,
            spacing=0.9,
            flux_func=subtle_flux,
            max_arrow_length=0.3,
            color=TRD_COLORS["void_light"],
            show_glow=False,
        )
        flux.set_opacity(0)

        self.add(flux)
        self.play(flux.animate.set_opacity(0.6), run_time=1.5)

        # New label
        flux_label = Text(
            "The Flux Field: Latent Potential",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        flux_label.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(flux_label), run_time=0.8)
        self.wait(2.0)

        # === SCENE 7: Key Revelation ===
        self.add_marker("1.1.0.7", "revelation")

        self.play(
            FadeOut(grid),
            FadeOut(flux),
            FadeOut(flux_label),
            run_time=1.0,
        )

        # Key revelation text
        revelation = Text(
            "The void is not empty space.\n"
            "It is the substrate of possibility.",
            color=TRD_COLORS["highlight"],
            font_size=32,
            line_spacing=1.4,
        )
        self.play(FadeIn(revelation, shift=UP * 0.3), run_time=1.5)
        self.wait(3.0)
        self.play(FadeOut(revelation), run_time=1.0)

        # Export markers
        self.export_markers()


class VoidGenesisPreview(TRDScene):
    """Preview of genesis: what the void can become."""

    def construct(self):
        self.load_narration("1.1")
        self.add_marker("1.1.1.1", "genesis_preview")

        # Start with void voxel
        voxel = VoxelMobject(
            state=0,
            size=1.5,
            position=ORIGIN,
            show_glow=False,
        )
        self.play(FadeIn(voxel), run_time=1.0)

        # Label
        void_label = Text(
            "Void (state = 0)",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        void_label.next_to(voxel, DOWN, buff=0.5)
        self.play(FadeIn(void_label), run_time=0.5)
        self.wait(1.5)

        # === Transition to matter ===
        self.add_marker("1.1.1.2", "to_matter")

        # Create matter voxel
        matter_voxel = VoxelMobject(
            state=+1,
            size=1.5,
            position=ORIGIN,
            show_glow=True,
            glow_layers=4,
        )

        # Animate genesis
        self.play(
            GrowFromCenter(matter_voxel, rate_func=rate_functions.ease_out_back),
            FadeOut(voxel),
            FadeOut(void_label),
            run_time=1.5,
        )

        matter_label = Text(
            "Matter (state = +1)",
            color=TRD_COLORS["matter"],
            font_size=24,
        )
        matter_label.next_to(matter_voxel, DOWN, buff=0.5)
        self.play(FadeIn(matter_label), run_time=0.5)
        self.wait(1.5)

        # === Transition to antimatter ===
        self.add_marker("1.1.1.3", "to_antimatter")

        antimatter_voxel = VoxelMobject(
            state=-1,
            size=1.5,
            position=RIGHT * 3,
            show_glow=True,
            glow_layers=4,
        )

        # Show both
        self.play(
            matter_voxel.animate.shift(LEFT * 1.5),
            matter_label.animate.shift(LEFT * 1.5),
            run_time=0.8,
        )
        self.play(
            GrowFromCenter(antimatter_voxel.shift(LEFT * 1.5), rate_func=rate_functions.ease_out_back),
            run_time=1.0,
        )

        antimatter_label = Text(
            "Antimatter (state = -1)",
            color=TRD_COLORS["antimatter"],
            font_size=24,
        )
        antimatter_label.next_to(antimatter_voxel, DOWN, buff=0.5)
        self.play(FadeIn(antimatter_label), run_time=0.5)
        self.wait(2.0)

        # === Summary ===
        self.add_marker("1.1.1.4", "summary")

        summary = Text(
            "Three states. One substrate. Infinite possibility.",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        summary.to_edge(UP, buff=0.8)
        self.play(FadeIn(summary), run_time=1.0)
        self.wait(2.0)

        # Fade all
        self.fade_all()

        self.export_markers()


# For direct execution
if __name__ == "__main__":
    # Set up config for preview
    config.pixel_width = 1920
    config.pixel_height = 1080
    config.frame_rate = 60
    config.background_color = TRD_COLORS["background"]

    # Run VoidIntro scene
    scene = VoidIntro()
    scene.render()
