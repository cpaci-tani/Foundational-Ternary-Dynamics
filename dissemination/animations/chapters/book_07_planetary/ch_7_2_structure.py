"""
Chapter 7.2: Planetary Structure
================================

Internal layers and composition of planets.
Shows differentiation in TRD flux framework.
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
    VGroup,
    Circle,
    Dot,
    Line,
    Arc,
    ArcBetweenPoints,
    Text,
    MathTex,
    Annulus,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class StructureIntro(TRDScene):
    """Introduction to planetary structure."""

    def construct(self):
        self.load_narration("7.2")

        self.add_marker("7.2.0.1", "title")
        title = self.trd_title("Planetary Structure")
        subtitle = Text(
            "Layers Within Worlds",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class Differentiation(TRDScene):
    """Planetary differentiation - heavy sinks, light rises."""

    def construct(self):
        self.load_narration("7.2")

        self.add_marker("7.2.1.1", "differentiation")

        title = self.concept_card(
            "Differentiation",
            "Heavy material sinks to center"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Before differentiation
        self.add_marker("7.2.1.2", "before")

        before_label = Text("Homogeneous", color=TRD_COLORS["text"], font_size=16)
        before_label.shift(LEFT * 2.5 + UP * 2.2)

        before = Circle(
            radius=1.2,
            fill_color=TRD_COLORS["text_dim"],
            fill_opacity=0.5,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=2,
        )
        before.move_to(LEFT * 2.5)

        # Mixed particles
        mixed = VGroup()
        np.random.seed(42)
        for _ in range(30):
            r = np.random.uniform(0, 1.0)
            theta = np.random.uniform(0, TAU)
            x = r * np.cos(theta)
            y = r * np.sin(theta)
            is_heavy = np.random.random() < 0.4
            color = TRD_COLORS["matter"] if is_heavy else TRD_COLORS["antimatter"]
            p = Dot(
                point=LEFT * 2.5 + [x, y, 0],
                radius=0.06 if is_heavy else 0.04,
                color=color,
            )
            mixed.add(p)

        self.play(Write(before_label), Create(before), Create(mixed))

        # Arrow
        arrow = Text("→ Heat →", color=TRD_COLORS["highlight"], font_size=18)
        self.play(Write(arrow))

        # After differentiation
        self.add_marker("7.2.1.3", "after")

        after_label = Text("Differentiated", color=TRD_COLORS["text"], font_size=16)
        after_label.shift(RIGHT * 2.5 + UP * 2.2)

        # Core
        core = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        core.move_to(RIGHT * 2.5)

        # Mantle
        mantle = Annulus(
            inner_radius=0.4,
            outer_radius=0.9,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.6,
            stroke_width=0,
        )
        mantle.move_to(RIGHT * 2.5)

        # Crust
        crust = Annulus(
            inner_radius=0.9,
            outer_radius=1.2,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.4,
            stroke_width=0,
        )
        crust.move_to(RIGHT * 2.5)

        self.play(
            Write(after_label),
            Create(crust),
            Create(mantle),
            Create(core),
        )

        # Labels
        core_lbl = Text("Core (Fe, Ni)", color=TRD_COLORS["matter"], font_size=10)
        core_lbl.next_to(core, DOWN, buff=1.5)
        mantle_lbl = Text("Mantle (silicates)", color=TRD_COLORS["antimatter"], font_size=10)
        mantle_lbl.next_to(core_lbl, DOWN, buff=0.15)
        crust_lbl = Text("Crust (light rock)", color=TRD_COLORS["highlight"], font_size=10)
        crust_lbl.next_to(mantle_lbl, DOWN, buff=0.15)

        self.play(Write(core_lbl), Write(mantle_lbl), Write(crust_lbl))

        self.wait(2)

        self.export_markers()


class EarthStructure(TRDScene):
    """Earth's internal structure."""

    def construct(self):
        self.load_narration("7.2")

        self.add_marker("7.2.2.1", "earth")

        title = self.concept_card(
            "Earth's Structure",
            "Layers of our planet"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Earth cross-section
        self.add_marker("7.2.2.2", "cross_section")

        # Inner core
        inner_core = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.9,
            stroke_width=0,
        )

        # Outer core
        outer_core = Annulus(
            inner_radius=0.4,
            outer_radius=0.85,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )

        # Lower mantle
        lower_mantle = Annulus(
            inner_radius=0.85,
            outer_radius=1.4,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.6,
            stroke_width=0,
        )

        # Upper mantle
        upper_mantle = Annulus(
            inner_radius=1.4,
            outer_radius=1.8,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.4,
            stroke_width=0,
        )

        # Crust (thin)
        crust = Annulus(
            inner_radius=1.8,
            outer_radius=1.85,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.8,
            stroke_width=0,
        )

        earth = VGroup(crust, upper_mantle, lower_mantle, outer_core, inner_core)
        earth.shift(LEFT * 1)

        self.play(Create(earth))

        # Labels with lines
        labels_data = [
            ("Inner Core", 0.2, TRD_COLORS["glow"]),
            ("Outer Core", 0.65, TRD_COLORS["matter"]),
            ("Mantle", 1.2, TRD_COLORS["antimatter"]),
            ("Crust", 1.82, TRD_COLORS["highlight"]),
        ]

        labels = VGroup()
        for name, r, color in labels_data:
            lbl = Text(name, color=color, font_size=12)
            labels.add(lbl)

        labels.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        labels.move_to(RIGHT * 2.5)

        # Properties
        props_data = [
            "Solid iron (5000°C)",
            "Liquid iron",
            "Hot rock (convecting)",
            "5-70 km thick",
        ]

        props = VGroup()
        for prop in props_data:
            p = Text(prop, color=TRD_COLORS["text_dim"], font_size=10)
            props.add(p)

        props.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        props.next_to(labels, RIGHT, buff=0.3)

        for lbl, prop in zip(labels, props):
            self.play(Write(lbl), Write(prop), run_time=0.4)

        self.wait(2)

        self.export_markers()


class PlanetaryComparison(TRDScene):
    """Comparing structures of different planets."""

    def construct(self):
        self.load_narration("7.2")

        self.add_marker("7.2.3.1", "comparison")

        title = self.concept_card(
            "Planetary Comparison",
            "Different worlds, different structures"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Three planet types
        self.add_marker("7.2.3.2", "three_types")

        # Mars (rocky, small core)
        mars_label = Text("Mars", color=TRD_COLORS["matter"], font_size=14)
        mars_label.shift(LEFT * 3.5 + UP * 2)

        mars_core = Circle(radius=0.25, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0)
        mars_mantle = Annulus(inner_radius=0.25, outer_radius=0.7, fill_color=TRD_COLORS["antimatter"], fill_opacity=0.5, stroke_width=0)
        mars_crust = Annulus(inner_radius=0.7, outer_radius=0.75, fill_color=TRD_COLORS["highlight"], fill_opacity=0.6, stroke_width=0)
        mars = VGroup(mars_crust, mars_mantle, mars_core)
        mars.move_to(LEFT * 3.5)

        self.play(Write(mars_label), Create(mars))

        # Earth
        earth_label = Text("Earth", color=TRD_COLORS["highlight"], font_size=14)
        earth_label.shift(UP * 2)

        earth_core = Circle(radius=0.35, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0)
        earth_mantle = Annulus(inner_radius=0.35, outer_radius=0.85, fill_color=TRD_COLORS["antimatter"], fill_opacity=0.5, stroke_width=0)
        earth_crust = Annulus(inner_radius=0.85, outer_radius=0.9, fill_color=TRD_COLORS["highlight"], fill_opacity=0.6, stroke_width=0)
        earth = VGroup(earth_crust, earth_mantle, earth_core)

        self.play(Write(earth_label), Create(earth))

        # Jupiter (gas giant)
        jupiter_label = Text("Jupiter", color=TRD_COLORS["glow"], font_size=14)
        jupiter_label.shift(RIGHT * 3.5 + UP * 2)

        jup_core = Circle(radius=0.15, fill_color=TRD_COLORS["matter"], fill_opacity=0.9, stroke_width=0)
        jup_metallic = Annulus(inner_radius=0.15, outer_radius=0.5, fill_color=TRD_COLORS["antimatter"], fill_opacity=0.7, stroke_width=0)
        jup_molecular = Annulus(inner_radius=0.5, outer_radius=0.9, fill_color=TRD_COLORS["highlight"], fill_opacity=0.4, stroke_width=0)
        jup_atm = Annulus(inner_radius=0.9, outer_radius=1.1, fill_color=TRD_COLORS["glow"], fill_opacity=0.3, stroke_width=0)
        jupiter = VGroup(jup_atm, jup_molecular, jup_metallic, jup_core)
        jupiter.move_to(RIGHT * 3.5)

        self.play(Write(jupiter_label), Create(jupiter))

        # Legend
        legend = VGroup(
            Text("Rocky: thin crust, large core", color=TRD_COLORS["matter"], font_size=11),
            Text("Gas: small core, thick atmosphere", color=TRD_COLORS["glow"], font_size=11),
        )
        legend.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        legend.to_edge(DOWN, buff=0.4)
        self.play(Write(legend))

        self.wait(2)

        self.export_markers()


class StructureSummary(TRDScene):
    """Summary of planetary structure."""

    def construct(self):
        self.load_narration("7.2")

        self.add_marker("7.2.4.1", "summary")

        title = self.trd_title("Planetary Structure")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Differentiation: heavy → center, light → surface",
            "Rocky planets: core, mantle, crust",
            "Gas giants: small core, vast atmosphere",
            "Internal heat drives geological activity",
            "TRD: Gravity gradient organizes flux density",
        ]

        point_mobs = VGroup()
        for point in points:
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=20)
            text = Text(point, color=TRD_COLORS["text"], font_size=16)
            text.next_to(bullet, RIGHT, buff=0.15)
            point_mobs.add(VGroup(bullet, text))

        point_mobs.arrange(DOWN, buff=0.35, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.5)

        self.wait(2)

        final = self.equation_box(
            r"\rho(r) \propto \text{density increases with depth}",
            "Gravity sorts by density"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
