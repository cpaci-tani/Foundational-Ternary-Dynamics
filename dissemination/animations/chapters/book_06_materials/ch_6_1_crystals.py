"""
Chapter 6.1: Crystal Structures
===============================

Crystalline arrangements and lattice types.
Shows how TRD flux configurations create crystal order.
"""

from __future__ import annotations

import numpy as np

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    OUT,
    IN,
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
    Square,
    Cube,
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


class CrystalsIntro(TRDScene):
    """Introduction to crystal structures."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.0.1", "title")
        title = self.trd_title("Crystal Structures")
        subtitle = Text(
            "Order from Flux Configurations",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class UnitCells(TRDScene):
    """Unit cells - the building blocks."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.1.1", "unit_cells")

        title = self.concept_card(
            "Unit Cells",
            "The repeating building block"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Simple cubic unit cell
        self.add_marker("6.1.1.2", "simple_cubic")

        sc_label = Text("Simple Cubic", color=TRD_COLORS["matter"], font_size=18)
        sc_label.shift(LEFT * 3.5 + UP * 2)

        # Draw 2D representation of unit cell
        sc_atoms = VGroup()
        for i in range(2):
            for j in range(2):
                atom = Circle(
                    radius=0.15,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(LEFT * 3.5 + [i * 0.8 - 0.4, j * 0.8 - 0.4, 0])
                sc_atoms.add(atom)

        sc_box = Square(side_length=0.8, color=TRD_COLORS["grid_bright"], stroke_width=1)
        sc_box.move_to(LEFT * 3.5)

        self.play(Write(sc_label), Create(sc_box), Create(sc_atoms))

        # BCC
        self.add_marker("6.1.1.3", "bcc")

        bcc_label = Text("Body-Centered Cubic", color=TRD_COLORS["antimatter"], font_size=18)
        bcc_label.shift(UP * 2)

        bcc_atoms = VGroup()
        # Corner atoms
        for i in range(2):
            for j in range(2):
                atom = Circle(
                    radius=0.15,
                    fill_color=TRD_COLORS["antimatter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to([i * 0.8 - 0.4, j * 0.8 - 0.4, 0])
                bcc_atoms.add(atom)
        # Center atom
        center = Circle(
            radius=0.15,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.9,
            stroke_width=0,
        )
        bcc_atoms.add(center)

        bcc_box = Square(side_length=0.8, color=TRD_COLORS["grid_bright"], stroke_width=1)

        self.play(Write(bcc_label), Create(bcc_box), Create(bcc_atoms))

        # FCC
        self.add_marker("6.1.1.4", "fcc")

        fcc_label = Text("Face-Centered Cubic", color=TRD_COLORS["highlight"], font_size=18)
        fcc_label.shift(RIGHT * 3.5 + UP * 2)

        fcc_atoms = VGroup()
        # Corner atoms
        for i in range(2):
            for j in range(2):
                atom = Circle(
                    radius=0.12,
                    fill_color=TRD_COLORS["highlight"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(RIGHT * 3.5 + [i * 0.8 - 0.4, j * 0.8 - 0.4, 0])
                fcc_atoms.add(atom)
        # Face centers
        for pos in [(0.4, 0, 0), (-0.4, 0, 0), (0, 0.4, 0), (0, -0.4, 0)]:
            atom = Circle(
                radius=0.12,
                fill_color=TRD_COLORS["glow"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            atom.move_to(RIGHT * 3.5 + np.array([pos[0], pos[1], 0]))
            fcc_atoms.add(atom)

        fcc_box = Square(side_length=0.8, color=TRD_COLORS["grid_bright"], stroke_width=1)
        fcc_box.move_to(RIGHT * 3.5)

        self.play(Write(fcc_label), Create(fcc_box), Create(fcc_atoms))

        # Coordination numbers
        coords = VGroup(
            Text("CN = 6", color=TRD_COLORS["matter"], font_size=14),
            Text("CN = 8", color=TRD_COLORS["antimatter"], font_size=14),
            Text("CN = 12", color=TRD_COLORS["highlight"], font_size=14),
        )
        coords[0].move_to(LEFT * 3.5 + DOWN * 1.2)
        coords[1].move_to(DOWN * 1.2)
        coords[2].move_to(RIGHT * 3.5 + DOWN * 1.2)

        self.play(Write(coords))

        self.wait(2)

        self.export_markers()


class CrystalSystems(TRDScene):
    """The seven crystal systems."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.2.1", "systems")

        title = self.concept_card(
            "Crystal Systems",
            "Seven fundamental symmetries"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # List the seven systems
        self.add_marker("6.1.2.2", "list")

        systems = [
            ("Cubic", "a = b = c, α = β = γ = 90°"),
            ("Tetragonal", "a = b ≠ c, α = β = γ = 90°"),
            ("Orthorhombic", "a ≠ b ≠ c, α = β = γ = 90°"),
            ("Hexagonal", "a = b ≠ c, α = β = 90°, γ = 120°"),
            ("Trigonal", "a = b = c, α = β = γ ≠ 90°"),
            ("Monoclinic", "a ≠ b ≠ c, α = γ = 90° ≠ β"),
            ("Triclinic", "a ≠ b ≠ c, α ≠ β ≠ γ"),
        ]

        system_mobs = VGroup()
        for i, (name, params) in enumerate(systems):
            row = VGroup(
                Text(name, color=TRD_COLORS["highlight"], font_size=14),
                Text(params, color=TRD_COLORS["text_dim"], font_size=11),
            )
            row.arrange(RIGHT, buff=0.3)
            system_mobs.add(row)

        system_mobs.arrange(DOWN, buff=0.25, aligned_edge=LEFT)
        system_mobs.shift(LEFT * 1)

        for sys in system_mobs:
            self.play(Write(sys), run_time=0.4)

        # TRD note
        trd = Text(
            "TRD: Crystal symmetry = flux field periodicity",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.5)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class Defects(TRDScene):
    """Crystal defects."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.3.1", "defects")

        title = self.concept_card(
            "Crystal Defects",
            "Imperfections in order"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Perfect lattice
        self.add_marker("6.1.3.2", "perfect")

        perfect_label = Text("Perfect Crystal", color=TRD_COLORS["matter"], font_size=16)
        perfect_label.shift(LEFT * 3 + UP * 2.2)

        perfect = VGroup()
        for i in range(5):
            for j in range(4):
                atom = Circle(
                    radius=0.1,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(LEFT * 3 + [(i - 2) * 0.4, (j - 1.5) * 0.4, 0])
                perfect.add(atom)

        self.play(Write(perfect_label), Create(perfect))

        # Vacancy defect
        self.add_marker("6.1.3.3", "vacancy")

        vacancy_label = Text("Vacancy", color=TRD_COLORS["antimatter"], font_size=16)
        vacancy_label.shift(UP * 2.2)

        vacancy = VGroup()
        for i in range(5):
            for j in range(4):
                if i == 2 and j == 2:  # Skip one atom
                    continue
                atom = Circle(
                    radius=0.1,
                    fill_color=TRD_COLORS["antimatter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to([(i - 2) * 0.4, (j - 1.5) * 0.4, 0])
                vacancy.add(atom)

        # Show empty spot
        empty = Circle(
            radius=0.1,
            stroke_color=TRD_COLORS["text_dim"],
            stroke_width=1,
            fill_opacity=0,
        )
        empty.move_to([0, 0.2, 0])

        self.play(Write(vacancy_label), Create(vacancy), Create(empty))

        # Interstitial
        self.add_marker("6.1.3.4", "interstitial")

        inter_label = Text("Interstitial", color=TRD_COLORS["highlight"], font_size=16)
        inter_label.shift(RIGHT * 3 + UP * 2.2)

        interstitial = VGroup()
        for i in range(5):
            for j in range(4):
                atom = Circle(
                    radius=0.1,
                    fill_color=TRD_COLORS["highlight"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                atom.move_to(RIGHT * 3 + [(i - 2) * 0.4, (j - 1.5) * 0.4, 0])
                interstitial.add(atom)

        # Extra atom
        extra = Circle(
            radius=0.1,
            fill_color=TRD_COLORS["glow"],
            fill_opacity=1.0,
            stroke_width=0,
        )
        extra.move_to(RIGHT * 3 + [0.2, 0.4, 0])
        interstitial.add(extra)

        self.play(Write(inter_label), Create(interstitial))

        # Defect properties
        props = Text(
            "Defects affect: conductivity, strength, reactivity",
            color=TRD_COLORS["text"],
            font_size=14,
        )
        props.to_edge(DOWN, buff=0.5)
        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class Bonding(TRDScene):
    """Bonding in crystals."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.4.1", "bonding")

        title = self.concept_card(
            "Crystal Bonding",
            "What holds crystals together"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Four bonding types
        self.add_marker("6.1.4.2", "types")

        bonding_types = [
            ("Ionic", "NaCl, MgO", TRD_COLORS["matter"]),
            ("Covalent", "Diamond, Si", TRD_COLORS["antimatter"]),
            ("Metallic", "Cu, Fe, Au", TRD_COLORS["highlight"]),
            ("van der Waals", "Ice, Ar(s)", TRD_COLORS["glow"]),
        ]

        boxes = VGroup()
        for name, examples, color in bonding_types:
            box = RoundedRectangle(
                width=2.5, height=1.2,
                corner_radius=0.1,
                stroke_color=color,
                fill_opacity=0.1,
            )
            name_text = Text(name, color=color, font_size=16, weight="BOLD")
            name_text.move_to(box.get_top() + DOWN * 0.3)
            ex_text = Text(examples, color=TRD_COLORS["text_dim"], font_size=12)
            ex_text.move_to(box.get_center() + DOWN * 0.2)
            boxes.add(VGroup(box, name_text, ex_text))

        boxes.arrange_in_grid(rows=2, cols=2, buff=0.3)

        for box in boxes:
            self.play(Create(box), run_time=0.5)

        # TRD interpretation
        trd = Text(
            "TRD: All bonding = flux field coupling patterns",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class CrystalsSummary(TRDScene):
    """Summary of crystal structures."""

    def construct(self):
        self.load_narration("6.1")

        self.add_marker("6.1.5.1", "summary")

        title = self.trd_title("Crystal Structures")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Unit cells: repeating structural units",
            "7 crystal systems based on symmetry",
            "Defects affect material properties",
            "Bonding type determines crystal behavior",
            "TRD: Crystals = periodic flux configurations",
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
            r"\text{Crystal} = \text{Unit Cell} \times \text{Translation}",
            "Periodicity creates long-range order"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
