"""
Chapter 6.2: Metals and Alloys
==============================

Metallic bonding and properties.
Shows electron sea model in TRD flux terms.
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
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class MetalsIntro(TRDScene):
    """Introduction to metals."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.0.1", "title")
        title = self.trd_title("Metals and Alloys")
        subtitle = Text(
            "The Electron Sea Model",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class ElectronSea(TRDScene):
    """Electron sea model of metallic bonding."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.1.1", "electron_sea")

        title = self.concept_card(
            "Electron Sea Model",
            "Delocalized electrons in a lattice"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Metal lattice
        self.add_marker("6.2.1.2", "lattice")

        # Positive ion cores
        ions = VGroup()
        for i in range(5):
            for j in range(4):
                ion = Circle(
                    radius=0.2,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                ion.move_to([(i - 2) * 1.0, (j - 1.5) * 0.9, 0])
                ions.add(ion)

        self.play(Create(ions))

        # Delocalized electrons
        self.add_marker("6.2.1.3", "electrons")

        electrons = VGroup()
        np.random.seed(42)
        for _ in range(30):
            x = np.random.uniform(-2.5, 2.5)
            y = np.random.uniform(-1.8, 1.8)
            e = Dot(
                point=[x, y, 0],
                radius=0.05,
                color=TRD_COLORS["antimatter"],
            )
            electrons.add(e)

        self.play(FadeIn(electrons))

        # Animate electron motion
        for _ in range(3):
            new_positions = []
            for e in electrons:
                dx = np.random.uniform(-0.3, 0.3)
                dy = np.random.uniform(-0.3, 0.3)
                new_pos = e.get_center() + np.array([dx, dy, 0])
                new_pos[0] = np.clip(new_pos[0], -2.5, 2.5)
                new_pos[1] = np.clip(new_pos[1], -1.8, 1.8)
                new_positions.append(new_pos)

            self.play(
                *[e.animate.move_to(pos) for e, pos in zip(electrons, new_positions)],
                run_time=0.5,
            )

        # Labels
        ion_label = Text("Metal ions (+)", color=TRD_COLORS["matter"], font_size=14)
        ion_label.to_edge(RIGHT, buff=0.3).shift(UP)
        e_label = Text("Free electrons (-)", color=TRD_COLORS["antimatter"], font_size=14)
        e_label.next_to(ion_label, DOWN, buff=0.2)

        self.play(Write(ion_label), Write(e_label))

        # TRD note
        trd = Text(
            "TRD: Electron sea = delocalized flux density",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class MetallicProperties(TRDScene):
    """Properties of metals."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.2.1", "properties")

        title = self.concept_card(
            "Metallic Properties",
            "Consequences of the electron sea"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Property list with explanations
        self.add_marker("6.2.2.2", "list")

        properties = [
            ("Electrical conductivity", "Free electrons carry current"),
            ("Thermal conductivity", "Electrons transfer heat"),
            ("Malleability", "Layers can slide without breaking bonds"),
            ("Ductility", "Can be drawn into wires"),
            ("Luster", "Free electrons reflect light"),
        ]

        prop_mobs = VGroup()
        for prop, reason in properties:
            row = VGroup(
                Text(prop + ":", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
                Text(reason, color=TRD_COLORS["text"], font_size=14),
            )
            row.arrange(RIGHT, buff=0.2)
            prop_mobs.add(row)

        prop_mobs.arrange(DOWN, buff=0.35, aligned_edge=LEFT)
        prop_mobs.shift(LEFT * 0.5)

        for prop in prop_mobs:
            self.play(Write(prop), run_time=0.5)

        self.wait(2)

        self.export_markers()


class Conductivity(TRDScene):
    """Electrical conductivity visualization."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.3.1", "conductivity")

        title = self.concept_card(
            "Electrical Conductivity",
            "Electron flow through metal"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Wire representation
        self.add_marker("6.2.3.2", "wire")

        wire = Rectangle(
            width=6, height=1.5,
            stroke_color=TRD_COLORS["grid_bright"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.3,
        )
        self.play(Create(wire))

        # Ion lattice (simplified)
        ions = VGroup()
        for i in range(8):
            for j in range(2):
                ion = Circle(
                    radius=0.12,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.6,
                    stroke_width=0,
                )
                ion.move_to([(i - 3.5) * 0.7, (j - 0.5) * 0.5, 0])
                ions.add(ion)

        self.play(Create(ions))

        # Battery terminals
        plus = Text("+", color=TRD_COLORS["matter"], font_size=24)
        plus.move_to(LEFT * 3.5)
        minus = Text("-", color=TRD_COLORS["antimatter"], font_size=24)
        minus.move_to(RIGHT * 3.5)

        self.play(Write(plus), Write(minus))

        # Electron flow animation
        self.add_marker("6.2.3.3", "flow")

        electrons = VGroup()
        for i in range(6):
            e = Dot(
                point=LEFT * 2.5 + RIGHT * i * 0.8 + UP * np.random.uniform(-0.3, 0.3),
                radius=0.06,
                color=TRD_COLORS["antimatter"],
            )
            electrons.add(e)

        self.play(FadeIn(electrons))

        # Move electrons right to left (conventional current opposite)
        for _ in range(3):
            self.play(
                *[e.animate.shift(LEFT * 0.5) for e in electrons],
                run_time=0.5,
            )
            # Wrap around
            for e in electrons:
                if e.get_center()[0] < -2.8:
                    e.move_to([2.5, np.random.uniform(-0.3, 0.3), 0])

        # Current arrow
        current = Arrow(RIGHT * 2, LEFT * 2, color=TRD_COLORS["glow"], stroke_width=3)
        current.shift(DOWN * 1.3)
        current_label = Text("Current (I)", color=TRD_COLORS["glow"], font_size=14)
        current_label.next_to(current, DOWN, buff=0.1)

        self.play(GrowArrow(current), Write(current_label))

        self.wait(2)

        self.export_markers()


class Alloys(TRDScene):
    """Introduction to alloys."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.4.1", "alloys")

        title = self.concept_card(
            "Alloys",
            "Mixtures of metals"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Substitutional alloy
        self.add_marker("6.2.4.2", "substitutional")

        sub_label = Text("Substitutional", color=TRD_COLORS["matter"], font_size=16)
        sub_label.shift(LEFT * 3 + UP * 2)

        sub_lattice = VGroup()
        np.random.seed(123)
        for i in range(5):
            for j in range(4):
                is_substitute = np.random.random() < 0.2
                color = TRD_COLORS["highlight"] if is_substitute else TRD_COLORS["matter"]
                atom = Circle(
                    radius=0.12,
                    fill_color=color,
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(LEFT * 3 + [(i - 2) * 0.35, (j - 1.5) * 0.35, 0])
                sub_lattice.add(atom)

        self.play(Write(sub_label), Create(sub_lattice))

        sub_ex = Text("e.g., Brass (Cu + Zn)", color=TRD_COLORS["text_dim"], font_size=12)
        sub_ex.next_to(sub_lattice, DOWN, buff=0.2)
        self.play(Write(sub_ex))

        # Interstitial alloy
        self.add_marker("6.2.4.3", "interstitial")

        int_label = Text("Interstitial", color=TRD_COLORS["antimatter"], font_size=16)
        int_label.shift(RIGHT * 3 + UP * 2)

        int_lattice = VGroup()
        # Host atoms
        for i in range(5):
            for j in range(4):
                atom = Circle(
                    radius=0.15,
                    fill_color=TRD_COLORS["antimatter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(RIGHT * 3 + [(i - 2) * 0.4, (j - 1.5) * 0.4, 0])
                int_lattice.add(atom)

        # Small interstitial atoms
        np.random.seed(456)
        for _ in range(5):
            x = np.random.uniform(-0.7, 0.7)
            y = np.random.uniform(-0.5, 0.5)
            small = Circle(
                radius=0.06,
                fill_color=TRD_COLORS["glow"],
                fill_opacity=0.9,
                stroke_width=0,
            )
            small.move_to(RIGHT * 3 + [x, y, 0])
            int_lattice.add(small)

        self.play(Write(int_label), Create(int_lattice))

        int_ex = Text("e.g., Steel (Fe + C)", color=TRD_COLORS["text_dim"], font_size=12)
        int_ex.next_to(int_lattice, DOWN, buff=0.2)
        self.play(Write(int_ex))

        # Properties note
        props = Text(
            "Alloys: stronger, harder, more corrosion resistant",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        props.to_edge(DOWN, buff=0.4)
        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class MetalsSummary(TRDScene):
    """Summary of metals and alloys."""

    def construct(self):
        self.load_narration("6.2")

        self.add_marker("6.2.5.1", "summary")

        title = self.trd_title("Metals and Alloys")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Summary table
        metals = VGroup()

        data = [
            ("Property", "Origin"),
            ("Conductivity", "Free electrons"),
            ("Luster", "Electron reflection"),
            ("Malleability", "Non-directional bonds"),
            ("Alloy strength", "Lattice disruption"),
        ]

        for i, (prop, origin) in enumerate(data):
            if i == 0:
                color = TRD_COLORS["highlight"]
                weight = "BOLD"
            else:
                color = TRD_COLORS["text"]
                weight = "NORMAL"
            row = VGroup(
                Text(prop, color=color, font_size=14, weight=weight),
                Text(origin, color=color, font_size=14, weight=weight),
            )
            row.arrange(RIGHT, buff=1.5)
            metals.add(row)

        metals.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        metals.center()

        for row in metals:
            self.play(Write(row), run_time=0.4)

        self.wait(2)

        final = self.equation_box(
            r"\text{Metal} = \text{Cation Lattice} + \text{Electron Sea}",
            "Delocalized bonding enables metallic properties"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
