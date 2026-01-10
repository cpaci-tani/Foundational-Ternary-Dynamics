"""
Chapter 6.4: Composite Materials
================================

Combining materials for enhanced properties.
Shows how TRD flux fields combine in composites.
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
    Rectangle,
    Text,
    MathTex,
    RoundedRectangle,
    Polygon,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class CompositesIntro(TRDScene):
    """Introduction to composite materials."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.0.1", "title")
        title = self.trd_title("Composite Materials")
        subtitle = Text(
            "Combining Materials for Better Properties",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class CompositeBasics(TRDScene):
    """Basic structure of composites."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.1.1", "basics")

        title = self.concept_card(
            "Composite Structure",
            "Matrix + Reinforcement"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Composite visualization
        self.add_marker("6.4.1.2", "structure")

        # Matrix (background)
        matrix = Rectangle(
            width=5, height=3,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.3,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=2,
        )
        self.play(Create(matrix))

        matrix_label = Text("Matrix", color=TRD_COLORS["antimatter"], font_size=14)
        matrix_label.next_to(matrix, UP, buff=0.2)
        self.play(Write(matrix_label))

        # Reinforcement (fibers)
        self.add_marker("6.4.1.3", "reinforcement")

        fibers = VGroup()
        for i in range(6):
            y = (i - 2.5) * 0.45
            fiber = Line(
                LEFT * 2.2 + UP * y,
                RIGHT * 2.2 + UP * y,
                color=TRD_COLORS["matter"],
                stroke_width=4,
            )
            fibers.add(fiber)

        self.play(Create(fibers))

        fiber_label = Text("Reinforcement (fibers)", color=TRD_COLORS["matter"], font_size=14)
        fiber_label.next_to(matrix, DOWN, buff=0.3)
        self.play(Write(fiber_label))

        # Properties equation
        props = VGroup(
            Text("Matrix:", color=TRD_COLORS["antimatter"], font_size=12),
            Text("binds, transfers load", color=TRD_COLORS["text"], font_size=12),
        )
        props.arrange(RIGHT, buff=0.2)
        props.shift(DOWN * 2.2 + LEFT * 1.5)

        props2 = VGroup(
            Text("Reinforcement:", color=TRD_COLORS["matter"], font_size=12),
            Text("provides strength", color=TRD_COLORS["text"], font_size=12),
        )
        props2.arrange(RIGHT, buff=0.2)
        props2.next_to(props, DOWN, buff=0.15, aligned_edge=LEFT)

        self.play(Write(props), Write(props2))

        self.wait(2)

        self.export_markers()


class FiberComposites(TRDScene):
    """Fiber-reinforced composites."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.2.1", "fiber_composites")

        title = self.concept_card(
            "Fiber Composites",
            "Strong fibers in polymer matrix"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Different fiber types
        self.add_marker("6.4.2.2", "fiber_types")

        fiber_types = [
            ("Glass Fiber", "Fiberglass boats, tanks", TRD_COLORS["matter"]),
            ("Carbon Fiber", "Aircraft, sports gear", TRD_COLORS["antimatter"]),
            ("Aramid (Kevlar)", "Body armor, tires", TRD_COLORS["highlight"]),
        ]

        boxes = VGroup()
        for name, uses, color in fiber_types:
            box = RoundedRectangle(
                width=3.2, height=1.6,
                corner_radius=0.1,
                stroke_color=color,
                fill_opacity=0.1,
            )

            name_text = Text(name, color=color, font_size=16, weight="BOLD")
            name_text.move_to(box.get_top() + DOWN * 0.35)

            uses_text = Text(uses, color=TRD_COLORS["text"], font_size=12)
            uses_text.move_to(box.get_center() + DOWN * 0.15)

            # Mini fiber representation
            mini_fibers = VGroup()
            for i in range(3):
                f = Line(
                    box.get_center() + LEFT * 0.8 + DOWN * 0.5 + UP * i * 0.15,
                    box.get_center() + RIGHT * 0.8 + DOWN * 0.5 + UP * i * 0.15,
                    color=color,
                    stroke_width=2,
                )
                mini_fibers.add(f)

            boxes.add(VGroup(box, name_text, uses_text, mini_fibers))

        boxes.arrange(RIGHT, buff=0.3)

        for box in boxes:
            self.play(Create(box), run_time=0.6)

        # Properties comparison
        props = Text(
            "High strength-to-weight ratio",
            color=TRD_COLORS["glow"],
            font_size=14,
        )
        props.to_edge(DOWN, buff=0.4)
        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class ParticulateComposites(TRDScene):
    """Particle-reinforced composites."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.3.1", "particulate")

        title = self.concept_card(
            "Particulate Composites",
            "Particles in a matrix"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Particulate composite visualization
        self.add_marker("6.4.3.2", "particles")

        # Matrix
        matrix = Rectangle(
            width=4, height=3,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.2,
            stroke_color=TRD_COLORS["antimatter"],
            stroke_width=2,
        )
        self.play(Create(matrix))

        # Random particles
        particles = VGroup()
        np.random.seed(789)
        for _ in range(25):
            x = np.random.uniform(-1.7, 1.7)
            y = np.random.uniform(-1.2, 1.2)
            r = np.random.uniform(0.08, 0.15)
            p = Circle(
                radius=r,
                fill_color=TRD_COLORS["matter"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            p.move_to([x, y, 0])
            particles.add(p)

        self.play(Create(particles))

        # Examples
        self.add_marker("6.4.3.3", "examples")

        examples = VGroup(
            Text("Concrete:", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
            Text("gravel + sand + cement", color=TRD_COLORS["text"], font_size=12),
        )
        examples.arrange(RIGHT, buff=0.2)
        examples.shift(RIGHT * 3)

        examples2 = VGroup(
            Text("Rubber:", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
            Text("carbon black + polymer", color=TRD_COLORS["text"], font_size=12),
        )
        examples2.arrange(RIGHT, buff=0.2)
        examples2.next_to(examples, DOWN, buff=0.2, aligned_edge=LEFT)

        self.play(Write(examples), Write(examples2))

        # Properties
        props = Text(
            "Improved hardness, wear resistance, stiffness",
            color=TRD_COLORS["glow"],
            font_size=14,
        )
        props.to_edge(DOWN, buff=0.4)
        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class LaminateComposites(TRDScene):
    """Laminate composites - layered structures."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.4.1", "laminates")

        title = self.concept_card(
            "Laminate Composites",
            "Layered materials"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Exploded layer view
        self.add_marker("6.4.4.2", "layers")

        layers = VGroup()
        colors = [
            TRD_COLORS["matter"],
            TRD_COLORS["antimatter"],
            TRD_COLORS["matter"],
            TRD_COLORS["highlight"],
            TRD_COLORS["matter"],
        ]
        labels = ["0°", "90°", "0°", "45°", "0°"]

        for i, (color, label) in enumerate(zip(colors, labels)):
            # Layer rectangle
            layer = Rectangle(
                width=4, height=0.3,
                fill_color=color,
                fill_opacity=0.6,
                stroke_color=color,
                stroke_width=1,
            )
            layer.shift(UP * (2 - i * 0.8))

            # Fiber direction lines
            if label == "0°":
                for j in range(-4, 5):
                    line = Line(
                        layer.get_left() + RIGHT * 0.1,
                        layer.get_right() + LEFT * 0.1,
                        color=TRD_COLORS["glow"],
                        stroke_width=0.5,
                        stroke_opacity=0.5,
                    )
                    line.shift(UP * (2 - i * 0.8) + DOWN * 0.05)
            elif label == "90°":
                for j in range(-8, 9, 2):
                    line = Line(
                        layer.get_center() + UP * 0.12 + RIGHT * j * 0.2,
                        layer.get_center() + DOWN * 0.12 + RIGHT * j * 0.2,
                        color=TRD_COLORS["glow"],
                        stroke_width=0.5,
                        stroke_opacity=0.5,
                    )

            # Label
            lbl = Text(label, color=color, font_size=12)
            lbl.next_to(layer, RIGHT, buff=0.3)

            layers.add(VGroup(layer, lbl))

        for layer in layers:
            self.play(Create(layer), run_time=0.4)

        # Properties
        props = VGroup(
            Text("Orientation affects strength direction", color=TRD_COLORS["text"], font_size=14),
            Text("Plywood, CFRP, bulletproof glass", color=TRD_COLORS["text_dim"], font_size=12),
        )
        props.arrange(DOWN, buff=0.15)
        props.to_edge(DOWN, buff=0.5)
        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class NaturalComposites(TRDScene):
    """Natural composite materials."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.5.1", "natural")

        title = self.concept_card(
            "Natural Composites",
            "Nature's engineered materials"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Natural examples
        self.add_marker("6.4.5.2", "examples")

        natural = [
            ("Bone", "Collagen + hydroxyapatite", "Flexible + Hard"),
            ("Wood", "Cellulose + lignin", "Strong + Stiff"),
            ("Shell", "Nacre layers", "Tough + Hard"),
            ("Tendon", "Collagen fibers", "Strong + Flexible"),
        ]

        table = VGroup()
        # Header
        header = VGroup(
            Text("Material", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
            Text("Composition", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
            Text("Properties", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
        )
        header.arrange(RIGHT, buff=1.0)
        table.add(header)

        for material, comp, props in natural:
            row = VGroup(
                Text(material, color=TRD_COLORS["matter"], font_size=12),
                Text(comp, color=TRD_COLORS["text"], font_size=11),
                Text(props, color=TRD_COLORS["text_dim"], font_size=11),
            )
            row.arrange(RIGHT, buff=1.0)
            table.add(row)

        table.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        table.shift(LEFT * 0.5)

        for row in table:
            self.play(Write(row), run_time=0.4)

        # TRD note
        trd = Text(
            "TRD: Nature optimizes flux coupling for function",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class CompositesSummary(TRDScene):
    """Summary of composite materials."""

    def construct(self):
        self.load_narration("6.4")

        self.add_marker("6.4.6.1", "summary")

        title = self.trd_title("Composite Materials")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Composites: matrix + reinforcement",
            "Types: fiber, particulate, laminate",
            "Combine best properties of components",
            "Nature uses composite principles",
            "TRD: Optimized flux coupling patterns",
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
            r"\text{Composite} = \text{Matrix} + \text{Reinforcement}",
            "Whole greater than sum of parts"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
