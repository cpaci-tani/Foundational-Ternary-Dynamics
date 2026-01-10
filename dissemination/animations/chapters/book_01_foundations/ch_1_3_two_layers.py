"""
Chapter 1.3: The Two Layers
===========================

Visualization of the dispositional flux layer vs actual manifestation layer.
Shows how TRD has a dual ontology: continuous flux and discrete states.
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
    AnimationGroup,
    Succession,
    VGroup,
    VMobject,
    Circle,
    Rectangle,
    RoundedRectangle,
    Line,
    Arrow,
    Dot,
    Text,
    MathTex,
    Brace,
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
    FluxArrow,
    Lattice2D,
)


class TwoLayersIntro(TRDScene):
    """Introduction to the two-layer ontology."""

    def construct(self):
        self.load_narration("1.3")

        # Title
        self.add_marker("1.3.0.1", "title")
        title = self.trd_title("The Two Layers")
        subtitle = Text(
            "Dispositional Flux & Actual Manifestation",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Two layer diagram
        self.add_marker("1.3.0.2", "diagram")

        # Layer boxes
        flux_box = RoundedRectangle(
            width=5, height=2.5,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=2,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        flux_box.shift(UP * 1.5)

        manifest_box = RoundedRectangle(
            width=5, height=2.5,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=2,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        manifest_box.shift(DOWN * 1.5)

        # Labels
        flux_label = Text(
            "FLUX LAYER",
            color=TRD_COLORS["highlight"],
            font_size=28,
            weight="BOLD",
        )
        flux_label.move_to(flux_box.get_top() + DOWN * 0.4)

        flux_desc = Text(
            "Continuous | Dispositional | J ∈ ℝ³",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        flux_desc.next_to(flux_label, DOWN, buff=0.3)

        manifest_label = Text(
            "MANIFESTATION LAYER",
            color=TRD_COLORS["matter"],
            font_size=28,
            weight="BOLD",
        )
        manifest_label.move_to(manifest_box.get_top() + DOWN * 0.4)

        manifest_desc = Text(
            "Discrete | Actual | s ∈ {-1, 0, +1}",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        manifest_desc.next_to(manifest_label, DOWN, buff=0.3)

        # Connecting arrow
        arrow = Arrow(
            flux_box.get_bottom() + DOWN * 0.1,
            manifest_box.get_top() + UP * 0.1,
            color=TRD_COLORS["glow"],
            stroke_width=3,
        )
        arrow_label = MathTex(
            r"|J| > K_B",
            color=TRD_COLORS["glow"],
            font_size=24,
        )
        arrow_label.next_to(arrow, RIGHT, buff=0.2)

        # Animate
        self.play(Create(flux_box), Create(manifest_box))
        self.play(
            Write(flux_label), Write(flux_desc),
            Write(manifest_label), Write(manifest_desc),
        )
        self.play(Create(arrow), Write(arrow_label))

        self.wait(2)
        self.play(
            FadeOut(flux_box), FadeOut(manifest_box),
            FadeOut(flux_label), FadeOut(flux_desc),
            FadeOut(manifest_label), FadeOut(manifest_desc),
            FadeOut(arrow), FadeOut(arrow_label),
        )

        self.export_markers()


class FluxLayerDetail(TRDScene):
    """Deep dive into the flux layer."""

    def construct(self):
        self.load_narration("1.3")

        self.add_marker("1.3.1.1", "flux_layer")

        # Title
        title = self.concept_card(
            "The Flux Layer",
            "Continuous vector field encoding potential"
        )
        self.play(FadeIn(title))
        self.wait(1.5)
        self.play(FadeOut(title))

        # Flux field visualization
        self.add_marker("1.3.1.2", "flux_field")

        flux = FluxFieldMobject(
            rows=9, cols=9,
            spacing=0.7,
            arrow_scale=0.5,
        )

        self.play(Create(flux, run_time=2.0))
        self.wait(1)

        # Properties
        props = VGroup()

        prop1 = MathTex(r"J(v) \in \mathbb{R}^3", color=TRD_COLORS["text"], font_size=24)
        prop2 = MathTex(r"\text{Continuous values}", color=TRD_COLORS["text_dim"], font_size=20)
        prop3 = MathTex(r"\text{Evolves via wave equation}", color=TRD_COLORS["text_dim"], font_size=20)

        props.add(prop1, prop2, prop3)
        props.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        props.to_edge(RIGHT, buff=0.8)
        props.shift(UP * 1)

        self.play(Write(prop1))
        self.play(Write(prop2))
        self.play(Write(prop3))

        # Wave equation
        wave_eq = MathTex(
            r"\frac{\partial^2 J}{\partial t^2} = c^2 \nabla^2 J",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        wave_eq.to_edge(DOWN, buff=1.0)
        self.play(Write(wave_eq))

        self.wait(2)

        # Animate flux evolution
        self.add_marker("1.3.1.3", "flux_evolve")
        self.play(flux.propagate(run_time=3.0))

        self.wait(1)
        self.play(FadeOut(flux), FadeOut(props), FadeOut(wave_eq))

        self.export_markers()


class ManifestationLayerDetail(TRDScene):
    """Deep dive into the manifestation layer."""

    def construct(self):
        self.load_narration("1.3")

        self.add_marker("1.3.2.1", "manifest_layer")

        # Title
        title = self.concept_card(
            "The Manifestation Layer",
            "Discrete states representing actual existence"
        )
        self.play(FadeIn(title))
        self.wait(1.5)
        self.play(FadeOut(title))

        # Voxel grid with mixed states
        self.add_marker("1.3.2.2", "voxel_states")

        grid = VoxelGrid(rows=5, cols=5, voxel_size=0.7, spacing=0.9, default_state=0)

        # Set some states
        grid.set_state(1, 2, +1)
        grid.set_state(2, 1, -1)
        grid.set_state(2, 3, +1)
        grid.set_state(3, 2, -1)

        self.play(FadeIn(grid))

        # State legend
        legend = VGroup()

        void_dot = Dot(color=TRD_COLORS["void"], radius=0.15)
        void_label = Text("s = 0 (Void)", color=TRD_COLORS["text"], font_size=18)
        void_label.next_to(void_dot, RIGHT, buff=0.2)
        legend.add(VGroup(void_dot, void_label))

        matter_dot = Dot(color=TRD_COLORS["matter"], radius=0.15)
        matter_label = Text("s = +1 (Matter)", color=TRD_COLORS["text"], font_size=18)
        matter_label.next_to(matter_dot, RIGHT, buff=0.2)
        legend.add(VGroup(matter_dot, matter_label))

        anti_dot = Dot(color=TRD_COLORS["antimatter"], radius=0.15)
        anti_label = Text("s = -1 (Antimatter)", color=TRD_COLORS["text"], font_size=18)
        anti_label.next_to(anti_dot, RIGHT, buff=0.2)
        legend.add(VGroup(anti_dot, anti_label))

        legend.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        legend.to_edge(RIGHT, buff=0.5)

        self.play(FadeIn(legend))

        # Discrete nature
        discrete_note = Text(
            "No superpositions at voxel level",
            color=TRD_COLORS["highlight"],
            font_size=22,
        )
        discrete_note.to_edge(DOWN, buff=1.0)
        self.play(Write(discrete_note))

        self.wait(2)
        self.play(FadeOut(grid), FadeOut(legend), FadeOut(discrete_note))

        self.export_markers()


class LayerInteraction(TRDScene):
    """How the two layers interact."""

    def construct(self):
        self.load_narration("1.3")

        self.add_marker("1.3.3.1", "interaction")

        title = self.trd_title("Layer Interaction")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Side by side view
        self.add_marker("1.3.3.2", "side_by_side")

        # Left: Flux
        flux_region = RoundedRectangle(
            width=4, height=4,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=1,
            fill_opacity=0,
        )
        flux_region.shift(LEFT * 3)

        flux_label = Text("Flux J", color=TRD_COLORS["highlight"], font_size=20)
        flux_label.next_to(flux_region, UP, buff=0.2)

        flux_field = FluxFieldMobject(rows=5, cols=5, spacing=0.6, arrow_scale=0.3)
        flux_field.move_to(flux_region.get_center())

        # Right: States
        state_region = RoundedRectangle(
            width=4, height=4,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=1,
            fill_opacity=0,
        )
        state_region.shift(RIGHT * 3)

        state_label = Text("State s", color=TRD_COLORS["matter"], font_size=20)
        state_label.next_to(state_region, UP, buff=0.2)

        state_grid = VoxelGrid(rows=5, cols=5, voxel_size=0.5, spacing=0.7, default_state=0)
        state_grid.move_to(state_region.get_center())

        # Animate appearance
        self.play(
            Create(flux_region), Create(state_region),
            Write(flux_label), Write(state_label),
        )
        self.play(
            FadeIn(flux_field),
            FadeIn(state_grid),
        )

        # Interaction arrows
        self.add_marker("1.3.3.3", "coupling")

        arrow_down = Arrow(
            flux_region.get_bottom() + DOWN * 0.3 + RIGHT * 1,
            state_region.get_top() + UP * 0.3 + LEFT * 1,
            color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        down_label = Text("Genesis", color=TRD_COLORS["glow"], font_size=16)
        down_label.next_to(arrow_down.get_center(), UP + RIGHT, buff=0.1)

        arrow_up = Arrow(
            state_region.get_top() + UP * 0.1 + LEFT * 2,
            flux_region.get_bottom() + DOWN * 0.1 + RIGHT * 0,
            color=TRD_COLORS["text_dim"],
            stroke_width=2,
        )
        up_label = Text("Source", color=TRD_COLORS["text_dim"], font_size=16)
        up_label.next_to(arrow_up.get_center(), DOWN + LEFT, buff=0.1)

        self.play(Create(arrow_down), Write(down_label))
        self.play(Create(arrow_up), Write(up_label))

        # Coupling equation
        coupling = MathTex(
            r"\mathcal{L}_{coupling} = -g_c \cdot s \cdot (\nabla \cdot J)",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        coupling.to_edge(DOWN, buff=0.8)
        self.play(Write(coupling))

        self.wait(2)

        self.export_markers()


class TwoLayersSummary(TRDScene):
    """Summary of the two-layer structure."""

    def construct(self):
        self.load_narration("1.3")

        self.add_marker("1.3.4.1", "summary")

        # Summary table
        title = Text("Two-Layer Ontology", color=TRD_COLORS["text"], font_size=32, weight="BOLD")
        title.to_edge(UP, buff=0.8)
        self.play(Write(title))

        # Table
        headers = ["Property", "Flux Layer", "Manifestation Layer"]
        rows = [
            ["Domain", "ℝ³ (continuous)", "{-1, 0, +1} (discrete)"],
            ["Nature", "Dispositional", "Actual"],
            ["Evolution", "Wave equation", "Threshold transitions"],
            ["Role", "Encodes potential", "Realizes existence"],
        ]

        table = VGroup()
        y_pos = 1.5
        x_positions = [-3.5, 0, 3.5]

        # Headers
        header_row = VGroup()
        for x, h in zip(x_positions, headers):
            text = Text(h, color=TRD_COLORS["highlight"], font_size=18, weight="BOLD")
            text.move_to([x, y_pos, 0])
            header_row.add(text)
        table.add(header_row)
        self.play(Write(header_row))

        # Data rows
        for row_data in rows:
            y_pos -= 0.7
            row = VGroup()
            colors = [TRD_COLORS["text"], TRD_COLORS["highlight"], TRD_COLORS["matter"]]
            for x, val, col in zip(x_positions, row_data, colors):
                text = Text(val, color=col, font_size=16)
                text.move_to([x, y_pos, 0])
                row.add(text)
            table.add(row)
            self.play(Write(row), run_time=0.5)

        self.wait(2)

        # Key insight
        insight = self.equation_box(
            r"J \text{ (disposition)} \xrightarrow{|J|>K_B} s \text{ (actuality)}",
            "The flux-to-state transition defines TRD's causal structure"
        )
        insight.to_edge(DOWN, buff=0.5)
        self.play(FadeIn(insight))

        self.wait(2)

        self.export_markers()
