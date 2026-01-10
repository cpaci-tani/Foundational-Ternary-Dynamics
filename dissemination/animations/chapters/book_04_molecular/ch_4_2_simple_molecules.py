"""
Chapter 4.2: Simple Molecules
=============================

Common small molecules in TRD.
Shows water, carbon dioxide, and basic molecular geometry.
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
    Indicate,
    AnimationGroup,
    VGroup,
    Circle,
    Dot,
    Line,
    Arc,
    Text,
    MathTex,
    RoundedRectangle,
    Angle,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class SimpleMoleculesIntro(TRDScene):
    """Introduction to simple molecules."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.0.1", "title")
        title = self.trd_title("Simple Molecules")
        subtitle = Text(
            "Building Blocks of Chemistry",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class WaterMolecule(TRDScene):
    """Water molecule structure."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.1.1", "water")

        title = self.concept_card(
            "Water (H₂O)",
            "The molecule of life"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Build water molecule
        self.add_marker("4.2.1.2", "structure")

        # Oxygen atom
        oxygen = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        o_label = Text("O", color=TRD_COLORS["background"], font_size=20)
        o_label.move_to(oxygen.get_center())

        self.play(Create(oxygen), Write(o_label))

        # Hydrogen atoms at 104.5° angle
        angle = 104.5 * PI / 180
        h1_pos = oxygen.get_center() + 1.2 * np.array([np.cos(PI/2 + angle/2), np.sin(PI/2 + angle/2), 0])
        h2_pos = oxygen.get_center() + 1.2 * np.array([np.cos(PI/2 - angle/2), np.sin(PI/2 - angle/2), 0])

        h1 = Circle(
            radius=0.25,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        h1.move_to(h1_pos)
        h1_label = Text("H", color=TRD_COLORS["background"], font_size=14)
        h1_label.move_to(h1.get_center())

        h2 = Circle(
            radius=0.25,
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        h2.move_to(h2_pos)
        h2_label = Text("H", color=TRD_COLORS["background"], font_size=14)
        h2_label.move_to(h2.get_center())

        # Bonds
        bond1 = Line(oxygen.get_center(), h1_pos, color=TRD_COLORS["grid_bright"], stroke_width=4)
        bond2 = Line(oxygen.get_center(), h2_pos, color=TRD_COLORS["grid_bright"], stroke_width=4)

        self.play(Create(bond1), Create(bond2))
        self.play(Create(h1), Write(h1_label), Create(h2), Write(h2_label))

        # Angle annotation
        self.add_marker("4.2.1.3", "angle")

        angle_arc = Arc(
            radius=0.5,
            start_angle=PI/2 - angle/2,
            angle=angle,
            color=TRD_COLORS["highlight"],
        )
        angle_arc.move_to(oxygen.get_center())
        angle_label = MathTex(r"104.5°", color=TRD_COLORS["highlight"], font_size=16)
        angle_label.next_to(angle_arc, UP, buff=0.1)

        self.play(Create(angle_arc), Write(angle_label))

        # Properties
        props = VGroup()
        p1 = Text("• Bent geometry (VSEPR)", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Polar molecule (δ+ on H, δ- on O)", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Hydrogen bonding capability", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(DOWN, buff=0.6)

        for p in props:
            self.play(Write(p), run_time=0.4)

        self.wait(2)

        self.export_markers()


class CarbonDioxide(TRDScene):
    """Carbon dioxide molecule."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.2.1", "co2")

        title = self.concept_card(
            "Carbon Dioxide (CO₂)",
            "Linear molecule with double bonds"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Build CO2
        self.add_marker("4.2.2.2", "structure")

        # Carbon
        carbon = Circle(
            radius=0.35,
            fill_color=TRD_COLORS["text"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        c_label = Text("C", color=TRD_COLORS["background"], font_size=18)
        c_label.move_to(carbon.get_center())

        self.play(Create(carbon), Write(c_label))

        # Oxygen atoms
        o1 = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        o1.shift(LEFT * 1.5)
        o1_label = Text("O", color=TRD_COLORS["background"], font_size=18)
        o1_label.move_to(o1.get_center())

        o2 = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        o2.shift(RIGHT * 1.5)
        o2_label = Text("O", color=TRD_COLORS["background"], font_size=18)
        o2_label.move_to(o2.get_center())

        # Double bonds (two lines each)
        bond1a = Line(
            carbon.get_left() + UP * 0.08,
            o1.get_right() + UP * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        bond1b = Line(
            carbon.get_left() + DOWN * 0.08,
            o1.get_right() + DOWN * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        bond2a = Line(
            carbon.get_right() + UP * 0.08,
            o2.get_left() + UP * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )
        bond2b = Line(
            carbon.get_right() + DOWN * 0.08,
            o2.get_left() + DOWN * 0.08,
            color=TRD_COLORS["highlight"],
            stroke_width=3,
        )

        self.play(Create(bond1a), Create(bond1b), Create(bond2a), Create(bond2b))
        self.play(Create(o1), Write(o1_label), Create(o2), Write(o2_label))

        # Linear annotation
        self.add_marker("4.2.2.3", "linear")

        angle_label = MathTex(r"180°", color=TRD_COLORS["highlight"], font_size=20)
        angle_label.next_to(carbon, UP, buff=0.5)

        linear_arrow = Line(
            o1.get_right() + UP * 0.8,
            o2.get_left() + UP * 0.8,
            color=TRD_COLORS["text_dim"],
            stroke_width=1,
        )

        self.play(Write(angle_label), Create(linear_arrow))

        # Properties
        props = VGroup()
        p1 = Text("• Linear geometry", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Non-polar (symmetric)", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Greenhouse gas", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(DOWN, buff=0.6)

        for p in props:
            self.play(Write(p), run_time=0.4)

        self.wait(2)

        self.export_markers()


class MethaneStructure(TRDScene):
    """Methane molecule - tetrahedral geometry."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.3.1", "methane")

        title = self.concept_card(
            "Methane (CH₄)",
            "Tetrahedral geometry"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Central carbon
        self.add_marker("4.2.3.2", "structure")

        carbon = Circle(
            radius=0.35,
            fill_color=TRD_COLORS["text"],
            fill_opacity=0.8,
            stroke_width=0,
        )
        c_label = Text("C", color=TRD_COLORS["background"], font_size=18)
        c_label.move_to(carbon.get_center())

        self.play(Create(carbon), Write(c_label))

        # Hydrogens in tetrahedral arrangement (projected to 2D)
        h_positions = [
            UP * 1.0 + LEFT * 0.3,
            DOWN * 0.5 + LEFT * 0.9,
            DOWN * 0.5 + RIGHT * 0.9,
            UP * 0.2 + RIGHT * 0.8,  # "behind" in 3D
        ]

        hydrogens = VGroup()
        bonds = VGroup()

        for i, pos in enumerate(h_positions):
            h = Circle(
                radius=0.2,
                fill_color=TRD_COLORS["matter"],
                fill_opacity=0.8 if i < 3 else 0.5,  # "behind" one fainter
                stroke_width=0,
            )
            h.move_to(pos)
            h_label = Text("H", color=TRD_COLORS["background"], font_size=12)
            h_label.move_to(h.get_center())

            bond = Line(
                ORIGIN, pos * 0.6,
                color=TRD_COLORS["grid_bright"],
                stroke_width=3 if i < 3 else 2,
            )

            hydrogens.add(VGroup(h, h_label))
            bonds.add(bond)

        self.play(Create(bonds))
        self.play(Create(hydrogens))

        # Tetrahedral angle
        self.add_marker("4.2.3.3", "tetrahedral")

        angle_note = MathTex(r"109.5° \text{ (tetrahedral)}", color=TRD_COLORS["highlight"], font_size=18)
        angle_note.to_edge(RIGHT, buff=1.0)
        self.play(Write(angle_note))

        # Properties
        props = VGroup()
        p1 = Text("• sp³ hybridization", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Non-polar (symmetric)", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Simplest alkane", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(DOWN, buff=0.6)

        for p in props:
            self.play(Write(p), run_time=0.4)

        self.wait(2)

        self.export_markers()


class MolecularGeometry(TRDScene):
    """VSEPR and molecular geometry."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.4.1", "vsepr")

        title = self.concept_card(
            "Molecular Geometry",
            "VSEPR: Electron pair repulsion"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # VSEPR explanation
        self.add_marker("4.2.4.2", "explanation")

        vsepr = Text(
            "VSEPR: Valence Shell Electron Pair Repulsion",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        vsepr.to_edge(UP, buff=0.8)
        self.play(Write(vsepr))

        principle = Text(
            "Electron pairs repel → maximize separation",
            color=TRD_COLORS["text"],
            font_size=16,
        )
        principle.next_to(vsepr, DOWN, buff=0.3)
        self.play(Write(principle))

        # Examples
        self.add_marker("4.2.4.3", "examples")

        geometries = VGroup()

        # Linear (2 pairs)
        lin = VGroup()
        lin_center = Dot(ORIGIN, radius=0.15, color=TRD_COLORS["matter"])
        lin_l = Dot(LEFT * 0.6, radius=0.1, color=TRD_COLORS["antimatter"])
        lin_r = Dot(RIGHT * 0.6, radius=0.1, color=TRD_COLORS["antimatter"])
        lin_label = Text("Linear (2)", color=TRD_COLORS["text"], font_size=12)
        lin_label.next_to(VGroup(lin_l, lin_r), DOWN, buff=0.2)
        lin.add(lin_center, lin_l, lin_r, lin_label)

        # Trigonal (3 pairs)
        tri = VGroup()
        tri_center = Dot(ORIGIN, radius=0.15, color=TRD_COLORS["matter"])
        tri_positions = [np.array([0.6*np.cos(i*2*PI/3), 0.6*np.sin(i*2*PI/3), 0]) for i in range(3)]
        for pos in tri_positions:
            tri.add(Dot(pos, radius=0.1, color=TRD_COLORS["antimatter"]))
        tri_label = Text("Trigonal (3)", color=TRD_COLORS["text"], font_size=12)
        tri_label.next_to(tri, DOWN, buff=0.2)
        tri.add(tri_center, tri_label)

        # Tetrahedral (4 pairs)
        tet = VGroup()
        tet_center = Dot(ORIGIN, radius=0.15, color=TRD_COLORS["matter"])
        tet_positions = [
            UP * 0.5,
            DOWN * 0.3 + LEFT * 0.4,
            DOWN * 0.3 + RIGHT * 0.4,
            UP * 0.1 + RIGHT * 0.3,
        ]
        for pos in tet_positions:
            tet.add(Dot(pos, radius=0.1, color=TRD_COLORS["antimatter"]))
        tet_label = Text("Tetrahedral (4)", color=TRD_COLORS["text"], font_size=12)
        tet_label.next_to(tet, DOWN, buff=0.2)
        tet.add(tet_center, tet_label)

        geometries.add(lin, tri, tet)
        geometries.arrange(RIGHT, buff=1.5)
        geometries.shift(DOWN * 0.5)

        for geom in geometries:
            self.play(Create(geom), run_time=0.6)

        # TRD note
        trd_note = Text(
            "TRD: Flux distributions minimize repulsion energy",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd_note.to_edge(DOWN, buff=0.5)
        self.play(Write(trd_note))

        self.wait(2)

        self.export_markers()


class SimpleMoleculesSummary(TRDScene):
    """Summary of simple molecules."""

    def construct(self):
        self.load_narration("4.2")

        self.add_marker("4.2.5.1", "summary")

        title = self.trd_title("Simple Molecules")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key molecules
        molecules = VGroup()

        mol_data = [
            ("H₂O", "Bent (104.5°)", "Polar"),
            ("CO₂", "Linear (180°)", "Non-polar"),
            ("CH₄", "Tetrahedral", "Non-polar"),
            ("NH₃", "Trigonal pyramidal", "Polar"),
        ]

        for formula, geometry, polarity in mol_data:
            row = VGroup(
                MathTex(formula, color=TRD_COLORS["highlight"], font_size=22),
                Text(geometry, color=TRD_COLORS["text"], font_size=14),
                Text(polarity, color=TRD_COLORS["text_dim"], font_size=14),
            )
            row.arrange(RIGHT, buff=0.8)
            molecules.add(row)

        molecules.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        molecules.center()

        for mol in molecules:
            self.play(Write(mol), run_time=0.5)

        self.wait(2)

        # Final insight
        final = self.equation_box(
            r"\text{Geometry} \leftarrow \text{Electron repulsion}",
            "VSEPR determines molecular shape"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
