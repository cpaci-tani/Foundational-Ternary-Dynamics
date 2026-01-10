"""
Chapter 7.3: Planetary Atmospheres
==================================

Atmospheric composition and dynamics.
Shows gas behavior in TRD flux framework.
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
    GrowArrow,
    AnimationGroup,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Text,
    MathTex,
    RoundedRectangle,
    Rectangle,
    Annulus,
    Axes,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class AtmospheresIntro(TRDScene):
    """Introduction to planetary atmospheres."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.0.1", "title")
        title = self.trd_title("Planetary Atmospheres")
        subtitle = Text(
            "The Gaseous Envelope",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class AtmosphericLayers(TRDScene):
    """Layers of Earth's atmosphere."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.1.1", "layers")

        title = self.concept_card(
            "Atmospheric Layers",
            "Earth's layered atmosphere"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Earth surface
        self.add_marker("7.3.1.2", "earth_layers")

        surface = Line(LEFT * 3.5, RIGHT * 3.5, color=TRD_COLORS["matter"], stroke_width=3)
        surface.shift(DOWN * 2)
        surface_label = Text("Surface", color=TRD_COLORS["matter"], font_size=12)
        surface_label.next_to(surface, DOWN, buff=0.1)

        self.play(Create(surface), Write(surface_label))

        # Layers with heights
        layers_data = [
            ("Troposphere", 0, 12, TRD_COLORS["antimatter"], "Weather, clouds"),
            ("Stratosphere", 12, 50, TRD_COLORS["highlight"], "Ozone layer"),
            ("Mesosphere", 50, 80, TRD_COLORS["matter"], "Meteors burn"),
            ("Thermosphere", 80, 150, TRD_COLORS["glow"], "Aurora, ISS"),
        ]

        # Scale: 150 km → 3 units
        scale = 3 / 150

        layers = VGroup()
        for name, h_low, h_high, color, note in layers_data:
            y_low = -2 + h_low * scale
            y_high = -2 + h_high * scale
            height = y_high - y_low

            layer = Rectangle(
                width=7,
                height=height,
                fill_color=color,
                fill_opacity=0.3,
                stroke_color=color,
                stroke_width=1,
            )
            layer.move_to([0, (y_low + y_high) / 2, 0])

            # Labels
            name_lbl = Text(name, color=color, font_size=12, weight="BOLD")
            name_lbl.move_to(layer.get_left() + RIGHT * 0.8)

            height_lbl = Text(f"{h_low}-{h_high} km", color=TRD_COLORS["text_dim"], font_size=10)
            height_lbl.next_to(name_lbl, DOWN, buff=0.05)

            note_lbl = Text(note, color=TRD_COLORS["text_dim"], font_size=9)
            note_lbl.move_to(layer.get_right() + LEFT * 1)

            layers.add(VGroup(layer, name_lbl, height_lbl, note_lbl))

        for layer in layers:
            self.play(Create(layer), run_time=0.5)

        self.wait(2)

        self.export_markers()


class Composition(TRDScene):
    """Atmospheric composition."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.2.1", "composition")

        title = self.concept_card(
            "Atmospheric Composition",
            "What's in the air"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Earth's atmosphere
        self.add_marker("7.3.2.2", "earth_comp")

        earth_label = Text("Earth", color=TRD_COLORS["highlight"], font_size=18)
        earth_label.shift(LEFT * 2.5 + UP * 2.2)

        earth_comp = VGroup(
            Text("N₂: 78%", color=TRD_COLORS["antimatter"], font_size=14),
            Text("O₂: 21%", color=TRD_COLORS["matter"], font_size=14),
            Text("Ar: 0.9%", color=TRD_COLORS["text_dim"], font_size=14),
            Text("CO₂: 0.04%", color=TRD_COLORS["glow"], font_size=14),
        )
        earth_comp.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        earth_comp.move_to(LEFT * 2.5)

        self.play(Write(earth_label), Write(earth_comp))

        # Venus
        self.add_marker("7.3.2.3", "venus_comp")

        venus_label = Text("Venus", color=TRD_COLORS["matter"], font_size=18)
        venus_label.shift(UP * 2.2)

        venus_comp = VGroup(
            Text("CO₂: 96%", color=TRD_COLORS["glow"], font_size=14),
            Text("N₂: 3.5%", color=TRD_COLORS["antimatter"], font_size=14),
            Text("SO₂: trace", color=TRD_COLORS["text_dim"], font_size=14),
        )
        venus_comp.arrange(DOWN, buff=0.2, aligned_edge=LEFT)

        self.play(Write(venus_label), Write(venus_comp))

        # Mars
        self.add_marker("7.3.2.4", "mars_comp")

        mars_label = Text("Mars", color=TRD_COLORS["antimatter"], font_size=18)
        mars_label.shift(RIGHT * 2.5 + UP * 2.2)

        mars_comp = VGroup(
            Text("CO₂: 95%", color=TRD_COLORS["glow"], font_size=14),
            Text("N₂: 2.7%", color=TRD_COLORS["antimatter"], font_size=14),
            Text("Ar: 1.6%", color=TRD_COLORS["text_dim"], font_size=14),
        )
        mars_comp.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        mars_comp.move_to(RIGHT * 2.5)

        self.play(Write(mars_label), Write(mars_comp))

        # Note about greenhouse effect
        greenhouse = Text(
            "CO₂ → Greenhouse effect → Surface temperature",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        greenhouse.to_edge(DOWN, buff=0.4)
        self.play(Write(greenhouse))

        self.wait(2)

        self.export_markers()


class GreenhouseEffect(TRDScene):
    """The greenhouse effect."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.3.1", "greenhouse")

        title = self.concept_card(
            "Greenhouse Effect",
            "Trapping heat in the atmosphere"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Diagram
        self.add_marker("7.3.3.2", "diagram")

        # Sun
        sun = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        sun.shift(UP * 2.5 + LEFT * 2.5)
        sun_label = Text("Sun", color=TRD_COLORS["glow"], font_size=12)
        sun_label.next_to(sun, UP, buff=0.1)

        self.play(Create(sun), Write(sun_label))

        # Earth surface
        surface = Line(LEFT * 2, RIGHT * 2, color=TRD_COLORS["matter"], stroke_width=4)
        surface.shift(DOWN * 1.5)

        # Atmosphere layer
        atm = Rectangle(
            width=4.5, height=2,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=1,
        )
        atm.shift(DOWN * 0.3)
        atm_label = Text("Atmosphere", color=TRD_COLORS["antimatter"], font_size=12)
        atm_label.next_to(atm, RIGHT, buff=0.2)

        self.play(Create(surface), Create(atm), Write(atm_label))

        # Incoming solar radiation
        self.add_marker("7.3.3.3", "radiation")

        incoming = Arrow(
            sun.get_center() + DOWN * 0.4 + RIGHT * 0.2,
            DOWN * 0.5,
            color=TRD_COLORS["glow"],
            stroke_width=3,
        )
        in_label = Text("Solar", color=TRD_COLORS["glow"], font_size=10)
        in_label.next_to(incoming, LEFT, buff=0.1)

        self.play(GrowArrow(incoming), Write(in_label))

        # Absorbed and re-emitted
        absorbed = Arrow(
            DOWN * 0.5,
            surface.get_center() + UP * 0.1,
            color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        self.play(GrowArrow(absorbed))

        # Infrared radiation up
        ir_up = Arrow(
            surface.get_center() + UP * 0.1 + RIGHT * 0.5,
            UP * 0.5 + RIGHT * 0.5,
            color=TRD_COLORS["matter"],
            stroke_width=2,
        )
        ir_label = Text("IR", color=TRD_COLORS["matter"], font_size=10)
        ir_label.next_to(ir_up, RIGHT, buff=0.05)

        self.play(GrowArrow(ir_up), Write(ir_label))

        # Some escapes, some trapped
        escape = Arrow(
            UP * 0.5 + RIGHT * 0.3,
            UP * 2 + RIGHT * 0.3,
            color=TRD_COLORS["matter"],
            stroke_width=2,
            stroke_opacity=0.5,
        )
        trapped = Arrow(
            UP * 0.3 + LEFT * 0.3,
            DOWN * 1 + LEFT * 0.3,
            color=TRD_COLORS["matter"],
            stroke_width=2,
        )
        trapped_label = Text("Trapped", color=TRD_COLORS["matter"], font_size=10)
        trapped_label.next_to(trapped, LEFT, buff=0.1)

        self.play(GrowArrow(escape), GrowArrow(trapped), Write(trapped_label))

        # Note
        note = Text(
            "Greenhouse gases trap heat → warmer surface",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        note.to_edge(DOWN, buff=0.3)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class EscapeVelocity(TRDScene):
    """Why some planets keep atmospheres and others don't."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.4.1", "escape")

        title = self.concept_card(
            "Escape Velocity",
            "Holding onto an atmosphere"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Escape velocity equation
        self.add_marker("7.3.4.2", "equation")

        eq = MathTex(
            r"v_{escape} = \sqrt{\frac{2GM}{R}}",
            color=TRD_COLORS["highlight"],
            font_size=36,
        )
        eq.shift(UP * 2)
        self.play(Write(eq))

        # Comparison
        self.add_marker("7.3.4.3", "comparison")

        bodies = [
            ("Earth", "11.2 km/s", "Keeps N₂, O₂", TRD_COLORS["highlight"]),
            ("Moon", "2.4 km/s", "Lost atmosphere", TRD_COLORS["text_dim"]),
            ("Mars", "5.0 km/s", "Thin CO₂", TRD_COLORS["matter"]),
            ("Jupiter", "59.5 km/s", "Keeps H₂, He", TRD_COLORS["antimatter"]),
        ]

        table = VGroup()
        for body, v_esc, result, color in bodies:
            row = VGroup(
                Text(body, color=color, font_size=14),
                Text(v_esc, color=TRD_COLORS["text"], font_size=14),
                Text(result, color=TRD_COLORS["text_dim"], font_size=12),
            )
            row.arrange(RIGHT, buff=0.8)
            table.add(row)

        table.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        table.shift(DOWN * 0.5)

        for row in table:
            self.play(Write(row), run_time=0.4)

        # TRD note
        trd = Text(
            "TRD: Escape = particle flux exceeds binding flux",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class AtmospheresSummary(TRDScene):
    """Summary of planetary atmospheres."""

    def construct(self):
        self.load_narration("7.3")

        self.add_marker("7.3.5.1", "summary")

        title = self.trd_title("Planetary Atmospheres")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Atmospheres layered by temperature",
            "Composition varies by planet",
            "Greenhouse effect traps heat",
            "Escape velocity determines retention",
            "TRD: Atmosphere = bound gas flux",
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
            r"P = \rho g h",
            "Pressure from gas column weight"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
