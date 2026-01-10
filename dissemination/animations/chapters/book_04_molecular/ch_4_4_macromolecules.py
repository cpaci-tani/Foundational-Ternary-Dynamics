"""
Chapter 4.4: Macromolecules
===========================

Large biological molecules.
Shows proteins, nucleic acids (DNA/RNA), and polymers.
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
    CubicBezier,
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


class MacromoleculesIntro(TRDScene):
    """Introduction to macromolecules."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.0.1", "title")
        title = self.trd_title("Macromolecules")
        subtitle = Text(
            "The Molecules of Life",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Four types
        self.add_marker("4.4.0.2", "types")

        types = VGroup()
        type_data = [
            ("Proteins", TRD_COLORS["matter"]),
            ("Nucleic Acids", TRD_COLORS["antimatter"]),
            ("Carbohydrates", TRD_COLORS["highlight"]),
            ("Lipids", TRD_COLORS["glow"]),
        ]

        for name, color in type_data:
            box = RoundedRectangle(
                width=2.5, height=1.0,
                corner_radius=0.1,
                stroke_color=color,
                fill_color=color,
                fill_opacity=0.2,
            )
            label = Text(name, color=color, font_size=18)
            label.move_to(box.get_center())
            types.add(VGroup(box, label))

        types.arrange_in_grid(rows=2, cols=2, buff=0.5)

        for t in types:
            self.play(Create(t), run_time=0.5)

        self.wait(2)
        self.play(FadeOut(types))

        self.export_markers()


class ProteinStructure(TRDScene):
    """Protein structure and folding."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.1.1", "proteins")

        title = self.concept_card(
            "Proteins",
            "Chains of amino acids"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Amino acid chain
        self.add_marker("4.4.1.2", "chain")

        chain_label = Text("Polypeptide chain:", color=TRD_COLORS["text"], font_size=16)
        chain_label.to_edge(UP, buff=0.8)
        self.play(Write(chain_label))

        # Amino acids as beads
        amino_acids = VGroup()
        colors = [TRD_COLORS["matter"], TRD_COLORS["antimatter"], TRD_COLORS["highlight"],
                  TRD_COLORS["glow"], TRD_COLORS["matter"], TRD_COLORS["antimatter"]]
        labels = ["Ala", "Gly", "Ser", "Val", "Leu", "Pro"]

        for i in range(6):
            aa = Circle(
                radius=0.35,
                fill_color=colors[i],
                fill_opacity=0.7,
                stroke_width=0,
            )
            aa.move_to([(i - 2.5) * 1.0, 0, 0])
            label = Text(labels[i], color=TRD_COLORS["background"], font_size=10)
            label.move_to(aa.get_center())
            amino_acids.add(VGroup(aa, label))

        # Peptide bonds
        bonds = VGroup()
        for i in range(5):
            bond = Line(
                amino_acids[i][0].get_right(),
                amino_acids[i+1][0].get_left(),
                color=TRD_COLORS["grid_bright"],
                stroke_width=3,
            )
            bonds.add(bond)

        self.play(Create(bonds), Create(amino_acids))

        # Structure levels
        self.add_marker("4.4.1.3", "levels")

        levels = VGroup()
        level_data = [
            ("Primary", "Sequence of amino acids"),
            ("Secondary", "α-helices, β-sheets"),
            ("Tertiary", "3D folded shape"),
            ("Quaternary", "Multiple subunits"),
        ]

        for name, desc in level_data:
            level = VGroup(
                Text(name + ":", color=TRD_COLORS["highlight"], font_size=14, weight="BOLD"),
                Text(desc, color=TRD_COLORS["text"], font_size=12),
            )
            level[1].next_to(level[0], RIGHT, buff=0.1)
            levels.add(level)

        levels.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        levels.to_edge(DOWN, buff=0.5)

        for level in levels:
            self.play(Write(level), run_time=0.4)

        self.wait(2)

        self.export_markers()


class DNAStructure(TRDScene):
    """DNA double helix structure."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.2.1", "dna")

        title = self.concept_card(
            "DNA",
            "The genetic code"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Double helix representation (simplified)
        self.add_marker("4.4.2.2", "helix")

        # Two backbone strands
        strand1_points = []
        strand2_points = []
        n_turns = 3
        points_per_turn = 10

        for i in range(n_turns * points_per_turn):
            t = i / points_per_turn * 2 * PI
            x = 2.0 * np.sin(t)
            y = i * 0.15 - 2
            strand1_points.append([x - 0.3, y, 0])
            strand2_points.append([-x + 0.3, y, 0])

        # Draw as connected dots
        strand1 = VGroup()
        strand2 = VGroup()

        for i in range(len(strand1_points) - 1):
            line1 = Line(
                strand1_points[i],
                strand1_points[i+1],
                color=TRD_COLORS["matter"],
                stroke_width=4,
            )
            line2 = Line(
                strand2_points[i],
                strand2_points[i+1],
                color=TRD_COLORS["antimatter"],
                stroke_width=4,
            )
            strand1.add(line1)
            strand2.add(line2)

        self.play(Create(strand1), Create(strand2), run_time=2.0)

        # Base pairs (rungs)
        self.add_marker("4.4.2.3", "bases")

        bases = VGroup()
        base_pairs = [("A", "T"), ("T", "A"), ("G", "C"), ("C", "G"), ("A", "T")]
        colors_left = {"A": "#ff6666", "T": "#66ff66", "G": "#6666ff", "C": "#ffff66"}
        colors_right = {"A": "#ff6666", "T": "#66ff66", "G": "#6666ff", "C": "#ffff66"}

        for i, (b1, b2) in enumerate(base_pairs):
            idx = i * 6 + 3
            if idx < len(strand1_points):
                rung = Line(
                    strand1_points[idx],
                    strand2_points[idx],
                    color=TRD_COLORS["highlight"],
                    stroke_width=2,
                )
                bases.add(rung)

        self.play(Create(bases))

        # Base pair legend
        legend = VGroup()
        pairs = [("A-T", TRD_COLORS["matter"]), ("G-C", TRD_COLORS["antimatter"])]
        for pair, color in pairs:
            dot = Dot(radius=0.1, color=color)
            label = Text(pair, color=color, font_size=12)
            label.next_to(dot, RIGHT, buff=0.1)
            legend.add(VGroup(dot, label))

        legend.arrange(RIGHT, buff=0.5)
        legend.to_edge(DOWN, buff=0.5)
        self.play(Create(legend))

        self.wait(2)

        self.export_markers()


class Polymers(TRDScene):
    """Polymer chains."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.3.1", "polymers")

        title = self.concept_card(
            "Polymers",
            "Repeating monomer units"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Monomer → Polymer
        self.add_marker("4.4.3.2", "polymerization")

        # Single monomer
        monomer = Circle(
            radius=0.4,
            fill_color=TRD_COLORS["highlight"],
            fill_opacity=0.7,
            stroke_width=0,
        )
        monomer.shift(UP * 1.5 + LEFT * 2)
        monomer_label = Text("Monomer", color=TRD_COLORS["text"], font_size=14)
        monomer_label.next_to(monomer, DOWN, buff=0.2)

        self.play(Create(monomer), Write(monomer_label))

        # Arrow
        arrow = Text("→ polymerize →", color=TRD_COLORS["text_dim"], font_size=14)
        arrow.next_to(monomer, RIGHT, buff=0.5)
        self.play(Write(arrow))

        # Polymer chain
        polymer = VGroup()
        for i in range(8):
            m = Circle(
                radius=0.25,
                fill_color=TRD_COLORS["highlight"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            m.move_to([i * 0.5 + 1.5, 1.5, 0])
            polymer.add(m)
            if i > 0:
                bond = Line(
                    polymer[i-1].get_right(),
                    m.get_left(),
                    color=TRD_COLORS["grid_bright"],
                    stroke_width=2,
                )
                polymer.add(bond)

        polymer_label = Text("Polymer", color=TRD_COLORS["text"], font_size=14)
        polymer_label.next_to(polymer, DOWN, buff=0.2)

        self.play(Create(polymer), Write(polymer_label))

        # Examples
        self.add_marker("4.4.3.3", "examples")

        examples = VGroup()
        ex_data = [
            ("Polyethylene", "Plastic bags"),
            ("Polystyrene", "Foam cups"),
            ("Nylon", "Fibers"),
            ("Cellulose", "Plant cell walls"),
        ]

        for name, use in ex_data:
            ex = VGroup(
                Text(name + ":", color=TRD_COLORS["highlight"], font_size=14),
                Text(use, color=TRD_COLORS["text_dim"], font_size=12),
            )
            ex[1].next_to(ex[0], RIGHT, buff=0.1)
            examples.add(ex)

        examples.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        examples.to_edge(DOWN, buff=0.5)

        for ex in examples:
            self.play(Write(ex), run_time=0.4)

        self.wait(2)

        self.export_markers()


class BiologicalFunction(TRDScene):
    """Macromolecule functions in biology."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.4.1", "function")

        title = self.concept_card(
            "Biological Function",
            "Structure determines function"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Function table
        self.add_marker("4.4.4.2", "table")

        functions = VGroup()

        data = [
            ("Proteins", "Enzymes, structure, transport", TRD_COLORS["matter"]),
            ("DNA", "Genetic information storage", TRD_COLORS["antimatter"]),
            ("RNA", "Protein synthesis, regulation", TRD_COLORS["highlight"]),
            ("Carbohydrates", "Energy, structure", TRD_COLORS["glow"]),
        ]

        for name, func, color in data:
            row = VGroup()
            name_box = RoundedRectangle(
                width=2.5, height=0.8,
                corner_radius=0.1,
                stroke_color=color,
                fill_opacity=0.1,
            )
            name_text = Text(name, color=color, font_size=16)
            name_text.move_to(name_box.get_center())

            func_text = Text(func, color=TRD_COLORS["text"], font_size=14)
            func_text.next_to(name_box, RIGHT, buff=0.3)

            row.add(VGroup(name_box, name_text), func_text)
            functions.add(row)

        functions.arrange(DOWN, buff=0.3, aligned_edge=LEFT)

        for func in functions:
            self.play(Create(func), run_time=0.5)

        # Key insight
        insight = Text(
            "Life = information encoded in molecular structure",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        insight.to_edge(DOWN, buff=0.5)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class MacromoleculesSummary(TRDScene):
    """Summary of macromolecules."""

    def construct(self):
        self.load_narration("4.4")

        self.add_marker("4.4.5.1", "summary")

        title = self.trd_title("Macromolecules")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Proteins: amino acid chains → enzymes",
            "DNA: double helix → genetic code",
            "RNA: single strand → protein synthesis",
            "Polymers: repeating units → materials",
            "Structure determines function",
            "TRD: Complexity from simple bonding",
        ]

        point_mobs = VGroup()
        for point in points:
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=20)
            text = Text(point, color=TRD_COLORS["text"], font_size=15)
            text.next_to(bullet, RIGHT, buff=0.15)
            point_mobs.add(VGroup(bullet, text))

        point_mobs.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.45)

        self.wait(2)

        # Final insight
        final = self.equation_box(
            r"\text{Life} = \text{Self-organizing flux patterns}",
            "Biology emerges from chemistry emerges from TRD"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
