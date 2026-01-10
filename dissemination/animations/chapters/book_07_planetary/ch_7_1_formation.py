"""
Chapter 7.1: Planetary Formation
================================

How planets form from cosmic material.
Shows gravitational accretion in TRD flux framework.
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
    Annulus,
    AnnularSector,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class FormationIntro(TRDScene):
    """Introduction to planetary formation."""

    def construct(self):
        self.load_narration("7.1")

        self.add_marker("7.1.0.1", "title")
        title = self.trd_title("Planetary Formation")
        subtitle = Text(
            "From Dust to Worlds",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class NebularHypothesis(TRDScene):
    """The nebular hypothesis of solar system formation."""

    def construct(self):
        self.load_narration("7.1")

        self.add_marker("7.1.1.1", "nebular")

        title = self.concept_card(
            "Nebular Hypothesis",
            "Solar system from collapsing cloud"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Stage 1: Gas cloud
        self.add_marker("7.1.1.2", "cloud")

        cloud = VGroup()
        np.random.seed(42)
        for _ in range(100):
            x = np.random.normal(0, 1.5)
            y = np.random.normal(0, 1.5)
            if abs(x) < 2.5 and abs(y) < 2.5:
                dot = Dot(
                    point=[x, y, 0],
                    radius=0.03,
                    color=TRD_COLORS["antimatter"],
                    fill_opacity=0.5,
                )
                cloud.add(dot)

        stage1_label = Text("1. Molecular Cloud", color=TRD_COLORS["text"], font_size=14)
        stage1_label.to_edge(UP, buff=0.5)

        self.play(Write(stage1_label), FadeIn(cloud))
        self.wait(1)

        # Stage 2: Collapse and rotation
        self.add_marker("7.1.1.3", "collapse")

        stage2_label = Text("2. Collapse & Rotation", color=TRD_COLORS["text"], font_size=14)
        stage2_label.to_edge(UP, buff=0.5)

        # Collapse animation
        collapsed = VGroup()
        for i, dot in enumerate(cloud):
            pos = dot.get_center()
            # Flatten and concentrate
            new_x = pos[0] * 0.6
            new_y = pos[1] * 0.3
            new_dot = Dot(
                point=[new_x, new_y, 0],
                radius=0.03,
                color=TRD_COLORS["antimatter"],
                fill_opacity=0.6,
            )
            collapsed.add(new_dot)

        self.play(
            Transform(stage1_label, stage2_label),
            Transform(cloud, collapsed),
            run_time=1.5,
        )

        # Add rotation indicator
        rotation = CurvedArrow(
            start_point=RIGHT * 1.5 + UP * 0.3,
            end_point=RIGHT * 1.5 + DOWN * 0.3,
            color=TRD_COLORS["highlight"],
            angle=-PI/2,
        )
        self.play(Create(rotation))
        self.wait(1)

        # Stage 3: Protoplanetary disk
        self.add_marker("7.1.1.4", "disk")

        stage3_label = Text("3. Protoplanetary Disk", color=TRD_COLORS["text"], font_size=14)
        stage3_label.to_edge(UP, buff=0.5)

        # Central star
        star = Circle(
            radius=0.3,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=0.9,
            stroke_width=0,
        )

        # Disk rings
        disk = VGroup()
        for r in [0.6, 0.9, 1.2, 1.5, 1.9]:
            ring = Annulus(
                inner_radius=r - 0.08,
                outer_radius=r + 0.08,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.4 - r * 0.15,
                stroke_width=0,
            )
            disk.add(ring)

        self.play(
            Transform(stage1_label, stage3_label),
            FadeOut(cloud),
            FadeOut(rotation),
            Create(star),
            Create(disk),
        )

        # TRD note
        trd = Text(
            "TRD: Gravity = flux density gradient attraction",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class Accretion(TRDScene):
    """Accretion process - dust to planetesimals."""

    def construct(self):
        self.load_narration("7.1")

        self.add_marker("7.1.2.1", "accretion")

        title = self.concept_card(
            "Accretion",
            "Growth by collision"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Dust grains
        self.add_marker("7.1.2.2", "dust")

        dust_label = Text("Dust Grains", color=TRD_COLORS["text"], font_size=14)
        dust_label.shift(LEFT * 3 + UP * 2)

        dust = VGroup()
        np.random.seed(111)
        for _ in range(30):
            x = np.random.uniform(-1.5, -0.5)
            y = np.random.uniform(-1, 1)
            d = Dot(
                point=LEFT * 3 + [x, y, 0],
                radius=0.04,
                color=TRD_COLORS["matter"],
            )
            dust.add(d)

        self.play(Write(dust_label), Create(dust))

        # Arrow
        arrow1 = Arrow(LEFT * 1.5, LEFT * 0.5, color=TRD_COLORS["highlight"])
        self.play(GrowArrow(arrow1))

        # Planetesimals
        self.add_marker("7.1.2.3", "planetesimals")

        plan_label = Text("Planetesimals", color=TRD_COLORS["text"], font_size=14)
        plan_label.shift(UP * 2)

        planetesimals = VGroup()
        for i in range(5):
            p = Circle(
                radius=0.12 + i * 0.02,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            p.move_to([np.random.uniform(-0.5, 0.5), np.random.uniform(-0.8, 0.8), 0])
            planetesimals.add(p)

        self.play(Write(plan_label), Create(planetesimals))

        # Arrow
        arrow2 = Arrow(RIGHT * 0.5, RIGHT * 1.5, color=TRD_COLORS["highlight"])
        self.play(GrowArrow(arrow2))

        # Protoplanet
        self.add_marker("7.1.2.4", "protoplanet")

        proto_label = Text("Protoplanet", color=TRD_COLORS["text"], font_size=14)
        proto_label.shift(RIGHT * 3 + UP * 2)

        protoplanet = Circle(
            radius=0.5,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        protoplanet.move_to(RIGHT * 3)

        self.play(Write(proto_label), Create(protoplanet))

        # Scale note
        scale = VGroup(
            Text("μm → km → 1000s km", color=TRD_COLORS["text_dim"], font_size=12),
        )
        scale.to_edge(DOWN, buff=0.5)
        self.play(Write(scale))

        self.wait(2)

        self.export_markers()


class DifferentPlanets(TRDScene):
    """Different types of planets."""

    def construct(self):
        self.load_narration("7.1")

        self.add_marker("7.1.3.1", "planet_types")

        title = self.concept_card(
            "Planet Types",
            "Rocky vs Gas Giants"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Rocky planets
        self.add_marker("7.1.3.2", "rocky")

        rocky_label = Text("Rocky (Terrestrial)", color=TRD_COLORS["matter"], font_size=16)
        rocky_label.shift(LEFT * 2.5 + UP * 2.2)

        rocky = VGroup()
        for i, (name, size) in enumerate([("Mercury", 0.15), ("Venus", 0.25), ("Earth", 0.25), ("Mars", 0.18)]):
            planet = Circle(
                radius=size,
                fill_color=TRD_COLORS["matter"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            planet.move_to(LEFT * 2.5 + UP * (1 - i * 0.8))
            label = Text(name, color=TRD_COLORS["text_dim"], font_size=10)
            label.next_to(planet, RIGHT, buff=0.15)
            rocky.add(VGroup(planet, label))

        self.play(Write(rocky_label), *[Create(r) for r in rocky])

        # Gas giants
        self.add_marker("7.1.3.3", "gas_giants")

        gas_label = Text("Gas Giants", color=TRD_COLORS["antimatter"], font_size=16)
        gas_label.shift(RIGHT * 2.5 + UP * 2.2)

        gas_giants = VGroup()
        for i, (name, size) in enumerate([("Jupiter", 0.6), ("Saturn", 0.5), ("Uranus", 0.3), ("Neptune", 0.3)]):
            planet = Circle(
                radius=size,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            y_pos = 1.2 - i * 0.9 if i < 2 else 0.3 - (i - 2) * 0.8
            planet.move_to(RIGHT * 2.5 + UP * y_pos)
            label = Text(name, color=TRD_COLORS["text_dim"], font_size=10)
            label.next_to(planet, RIGHT, buff=0.15)
            gas_giants.add(VGroup(planet, label))

        self.play(Write(gas_label), *[Create(g) for g in gas_giants])

        # Frost line explanation
        frost = VGroup(
            Text("Frost Line:", color=TRD_COLORS["highlight"], font_size=12, weight="BOLD"),
            Text("Beyond ~3 AU, ice can form", color=TRD_COLORS["text"], font_size=11),
        )
        frost.arrange(RIGHT, buff=0.2)
        frost.to_edge(DOWN, buff=0.4)
        self.play(Write(frost))

        self.wait(2)

        self.export_markers()


class FormationSummary(TRDScene):
    """Summary of planetary formation."""

    def construct(self):
        self.load_narration("7.1")

        self.add_marker("7.1.4.1", "summary")

        title = self.trd_title("Planetary Formation")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key steps
        steps = [
            "1. Molecular cloud collapses",
            "2. Rotation flattens into disk",
            "3. Dust grains collide and stick",
            "4. Planetesimals grow by accretion",
            "5. Protoplanets clear their orbits",
        ]

        step_mobs = VGroup()
        for step in steps:
            text = Text(step, color=TRD_COLORS["text"], font_size=16)
            step_mobs.add(text)

        step_mobs.arrange(DOWN, buff=0.35, aligned_edge=LEFT)
        step_mobs.center()

        for step in step_mobs:
            self.play(Write(step), run_time=0.5)

        self.wait(2)

        final = self.equation_box(
            r"\text{Planet} = \int \rho \cdot dV",
            "Gravity accumulates mass"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
