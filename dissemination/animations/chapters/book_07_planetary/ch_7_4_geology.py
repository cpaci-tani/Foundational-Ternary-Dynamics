"""
Chapter 7.4: Planetary Geology
==============================

Geological processes on planets.
Shows tectonics and volcanism in TRD flux framework.
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
    CurvedArrow,
    Text,
    MathTex,
    RoundedRectangle,
    Rectangle,
    Polygon,
    Arc,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class GeologyIntro(TRDScene):
    """Introduction to planetary geology."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.0.1", "title")
        title = self.trd_title("Planetary Geology")
        subtitle = Text(
            "Forces Shaping Worlds",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class PlateTectonics(TRDScene):
    """Plate tectonics on Earth."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.1.1", "tectonics")

        title = self.concept_card(
            "Plate Tectonics",
            "Moving crustal plates"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Cross-section view
        self.add_marker("7.4.1.2", "cross_section")

        # Mantle
        mantle = Rectangle(
            width=8, height=2,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.4,
            stroke_width=0,
        )
        mantle.shift(DOWN * 1)
        mantle_label = Text("Mantle", color=TRD_COLORS["antimatter"], font_size=12)
        mantle_label.move_to(mantle.get_center())

        self.play(Create(mantle), Write(mantle_label))

        # Crustal plates
        plate1 = Rectangle(
            width=3.5, height=0.3,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=1,
        )
        plate1.move_to(LEFT * 2 + UP * 0.15)

        plate2 = Rectangle(
            width=3.5, height=0.3,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=1,
        )
        plate2.move_to(RIGHT * 2 + UP * 0.15)

        self.play(Create(plate1), Create(plate2))

        # Convection currents
        self.add_marker("7.4.1.3", "convection")

        conv1 = CurvedArrow(
            start_point=LEFT * 2 + DOWN * 1.5,
            end_point=LEFT * 2 + DOWN * 0.3,
            color=TRD_COLORS["glow"],
            angle=-PI/2,
        )
        conv2 = CurvedArrow(
            start_point=RIGHT * 2 + DOWN * 0.3,
            end_point=RIGHT * 2 + DOWN * 1.5,
            color=TRD_COLORS["glow"],
            angle=-PI/2,
        )

        self.play(Create(conv1), Create(conv2))

        conv_label = Text("Convection", color=TRD_COLORS["glow"], font_size=12)
        conv_label.shift(DOWN * 1.8)
        self.play(Write(conv_label))

        # Plate motion arrows
        motion1 = Arrow(plate1.get_right() + LEFT * 0.5, plate1.get_right() + RIGHT * 0.3, color=TRD_COLORS["text"], stroke_width=3)
        motion2 = Arrow(plate2.get_left() + RIGHT * 0.5, plate2.get_left() + LEFT * 0.3, color=TRD_COLORS["text"], stroke_width=3)

        self.play(GrowArrow(motion1), GrowArrow(motion2))

        # Boundary types
        boundary = Text("Convergent Boundary", color=TRD_COLORS["highlight"], font_size=14)
        boundary.shift(UP * 1.5)
        self.play(Write(boundary))

        self.wait(2)

        self.export_markers()


class Volcanism(TRDScene):
    """Volcanic activity."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.2.1", "volcanism")

        title = self.concept_card(
            "Volcanism",
            "Magma reaching the surface"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Simple volcano cross-section
        self.add_marker("7.4.2.2", "volcano")

        # Ground
        ground = Line(LEFT * 4, RIGHT * 4, color=TRD_COLORS["matter"], stroke_width=3)
        ground.shift(DOWN * 1)
        self.play(Create(ground))

        # Volcano shape
        volcano = Polygon(
            LEFT * 2 + DOWN * 1,
            LEFT * 0.3 + UP * 1.5,
            RIGHT * 0.3 + UP * 1.5,
            RIGHT * 2 + DOWN * 1,
            fill_color=TRD_COLORS["text_dim"],
            fill_opacity=0.6,
            stroke_color=TRD_COLORS["matter"],
            stroke_width=2,
        )
        self.play(Create(volcano))

        # Magma chamber
        chamber = Circle(
            radius=0.8,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.7,
            stroke_width=0,
        )
        chamber.shift(DOWN * 2.3)
        chamber_label = Text("Magma Chamber", color=TRD_COLORS["glow"], font_size=10)
        chamber_label.next_to(chamber, DOWN, buff=0.1)

        self.play(Create(chamber), Write(chamber_label))

        # Conduit
        conduit = Rectangle(
            width=0.3, height=2.5,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.5,
            stroke_width=0,
        )
        conduit.shift(DOWN * 0.5)
        self.play(Create(conduit))

        # Eruption
        self.add_marker("7.4.2.3", "eruption")

        eruption = VGroup()
        np.random.seed(42)
        for _ in range(15):
            x = np.random.uniform(-0.5, 0.5)
            y = np.random.uniform(1.6, 2.5)
            p = Dot(
                point=[x, y, 0],
                radius=0.08,
                color=TRD_COLORS["glow"],
            )
            eruption.add(p)

        self.play(FadeIn(eruption))

        # Types
        types = VGroup(
            Text("Shield volcano (gentle)", color=TRD_COLORS["antimatter"], font_size=12),
            Text("Stratovolcano (explosive)", color=TRD_COLORS["matter"], font_size=12),
            Text("Cinder cone (small)", color=TRD_COLORS["highlight"], font_size=12),
        )
        types.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        types.to_edge(RIGHT, buff=0.3)
        self.play(Write(types))

        self.wait(2)

        self.export_markers()


class ImpactCraters(TRDScene):
    """Impact cratering."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.3.1", "impacts")

        title = self.concept_card(
            "Impact Craters",
            "Scars from cosmic collisions"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Impact sequence
        self.add_marker("7.4.3.2", "sequence")

        # Stage 1: Incoming
        stage1_label = Text("1. Incoming", color=TRD_COLORS["text"], font_size=14)
        stage1_label.shift(LEFT * 3 + UP * 2.2)

        surface1 = Line(LEFT * 4 + DOWN * 0.5, LEFT * 2 + DOWN * 0.5, color=TRD_COLORS["matter"], stroke_width=2)
        impactor = Circle(
            radius=0.15,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        impactor.move_to(LEFT * 3 + UP * 1)
        arrow1 = Arrow(impactor.get_center(), surface1.get_center() + UP * 0.2, color=TRD_COLORS["highlight"])

        self.play(Write(stage1_label), Create(surface1), Create(impactor), GrowArrow(arrow1))

        # Stage 2: Impact
        stage2_label = Text("2. Impact", color=TRD_COLORS["text"], font_size=14)
        stage2_label.shift(UP * 2.2)

        surface2 = Line(LEFT * 1 + DOWN * 0.5, RIGHT * 1 + DOWN * 0.5, color=TRD_COLORS["matter"], stroke_width=2)

        # Ejecta
        ejecta = VGroup()
        for angle in np.linspace(PI/6, 5*PI/6, 7):
            e = Arrow(
                ORIGIN + DOWN * 0.3,
                [np.cos(angle) * 0.8, np.sin(angle) * 0.8 - 0.3, 0],
                color=TRD_COLORS["glow"],
                stroke_width=2,
                buff=0,
            )
            ejecta.add(e)

        self.play(Write(stage2_label), Create(surface2), Create(ejecta))

        # Stage 3: Crater
        stage3_label = Text("3. Crater", color=TRD_COLORS["text"], font_size=14)
        stage3_label.shift(RIGHT * 3 + UP * 2.2)

        # Crater shape
        crater_left = Line(RIGHT * 2 + DOWN * 0.5, RIGHT * 2.5 + DOWN * 0.8, color=TRD_COLORS["matter"], stroke_width=2)
        crater_bottom = Arc(radius=0.5, start_angle=PI, angle=PI, color=TRD_COLORS["matter"], stroke_width=2)
        crater_bottom.move_to(RIGHT * 3 + DOWN * 0.8)
        crater_right = Line(RIGHT * 3.5 + DOWN * 0.8, RIGHT * 4 + DOWN * 0.5, color=TRD_COLORS["matter"], stroke_width=2)

        # Rim
        rim_left = Line(RIGHT * 2 + DOWN * 0.5, RIGHT * 2.3 + DOWN * 0.3, color=TRD_COLORS["matter"], stroke_width=2)
        rim_right = Line(RIGHT * 3.7 + DOWN * 0.3, RIGHT * 4 + DOWN * 0.5, color=TRD_COLORS["matter"], stroke_width=2)

        crater = VGroup(crater_left, crater_bottom, crater_right, rim_left, rim_right)
        self.play(Write(stage3_label), Create(crater))

        # Examples
        examples = VGroup(
            Text("Moon: heavily cratered", color=TRD_COLORS["antimatter"], font_size=12),
            Text("Earth: eroded/hidden", color=TRD_COLORS["highlight"], font_size=12),
            Text("Mars: mix of old/new", color=TRD_COLORS["matter"], font_size=12),
        )
        examples.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        examples.to_edge(DOWN, buff=0.4)
        self.play(Write(examples))

        self.wait(2)

        self.export_markers()


class ComparativeGeology(TRDScene):
    """Comparing geology across planets."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.4.1", "comparative")

        title = self.concept_card(
            "Comparative Geology",
            "Geology across the solar system"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Comparison table
        self.add_marker("7.4.4.2", "table")

        data = [
            ("Body", "Tectonics", "Volcanism", "Craters"),
            ("Earth", "Active", "Active", "Few"),
            ("Moon", "Dead", "Dead", "Many"),
            ("Mars", "Dead", "Dormant", "Many"),
            ("Venus", "Unknown", "Recent", "Some"),
            ("Io", "None", "Extreme", "None"),
        ]

        colors = [
            TRD_COLORS["highlight"],
            TRD_COLORS["highlight"],
            TRD_COLORS["text_dim"],
            TRD_COLORS["matter"],
            TRD_COLORS["antimatter"],
            TRD_COLORS["glow"],
        ]

        table = VGroup()
        for i, (body, tect, volc, crat) in enumerate(data):
            color = colors[i]
            weight = "BOLD" if i == 0 else "NORMAL"
            row = VGroup(
                Text(body, color=color, font_size=12, weight=weight),
                Text(tect, color=TRD_COLORS["text"] if i > 0 else color, font_size=12, weight=weight),
                Text(volc, color=TRD_COLORS["text"] if i > 0 else color, font_size=12, weight=weight),
                Text(crat, color=TRD_COLORS["text"] if i > 0 else color, font_size=12, weight=weight),
            )
            row.arrange(RIGHT, buff=0.8)
            table.add(row)

        table.arrange(DOWN, buff=0.25, aligned_edge=LEFT)
        table.shift(LEFT * 0.5)

        for row in table:
            self.play(Write(row), run_time=0.4)

        # Key insight
        insight = Text(
            "Internal heat drives geological activity",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        insight.to_edge(DOWN, buff=0.4)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class GeologySummary(TRDScene):
    """Summary of planetary geology."""

    def construct(self):
        self.load_narration("7.4")

        self.add_marker("7.4.5.1", "summary")

        title = self.trd_title("Planetary Geology")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Plate tectonics: crustal plates in motion",
            "Volcanism: magma reaching surface",
            "Impact craters: records of bombardment",
            "Internal heat drives activity",
            "TRD: Geology = flux redistribution processes",
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
            r"\text{Heat} \rightarrow \text{Convection} \rightarrow \text{Geology}",
            "Internal energy reshapes planetary surfaces"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
