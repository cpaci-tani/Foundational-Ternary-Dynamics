"""
Chapter 4.3: Complex Molecules
==============================

Organic chemistry and larger molecular structures.
Shows carbon chains, functional groups, and isomers.
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


class ComplexMoleculesIntro(TRDScene):
    """Introduction to complex molecules."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.0.1", "title")
        title = self.trd_title("Complex Molecules")
        subtitle = Text(
            "Organic Chemistry and Beyond",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class CarbonBackbone(TRDScene):
    """Carbon chains as molecular backbone."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.1.1", "carbon")

        title = self.concept_card(
            "Carbon Chains",
            "The backbone of organic chemistry"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Why carbon?
        self.add_marker("4.3.1.2", "why_carbon")

        why = VGroup()
        w1 = Text("Why carbon?", color=TRD_COLORS["highlight"], font_size=20, weight="BOLD")
        w2 = Text("• 4 valence electrons → 4 bonds", color=TRD_COLORS["text"], font_size=16)
        w3 = Text("• Can bond to itself (chains, rings)", color=TRD_COLORS["text"], font_size=16)
        w4 = Text("• Stable C-C bonds", color=TRD_COLORS["text"], font_size=16)

        why.add(w1, w2, w3, w4)
        why.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        why.to_edge(UP, buff=0.8)

        for w in why:
            self.play(Write(w), run_time=0.4)

        # Carbon chain
        self.add_marker("4.3.1.3", "chain")

        chain = VGroup()
        n_carbons = 6
        spacing = 1.0

        for i in range(n_carbons):
            c = Circle(
                radius=0.25,
                fill_color=TRD_COLORS["text"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            x_pos = (i - n_carbons/2 + 0.5) * spacing
            y_pos = 0.3 * np.sin(i * PI / 3)  # Zigzag
            c.move_to([x_pos, y_pos - 1.5, 0])
            chain.add(c)

        # Bonds
        bonds = VGroup()
        for i in range(n_carbons - 1):
            bond = Line(
                chain[i].get_center(),
                chain[i+1].get_center(),
                color=TRD_COLORS["grid_bright"],
                stroke_width=3,
            )
            bonds.add(bond)

        self.play(Create(bonds), Create(chain))

        # Label
        label = MathTex(r"C_6H_{14} \text{ (hexane)}", color=TRD_COLORS["highlight"], font_size=24)
        label.to_edge(DOWN, buff=0.8)
        self.play(Write(label))

        self.wait(2)

        self.export_markers()


class FunctionalGroups(TRDScene):
    """Common functional groups."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.2.1", "functional")

        title = self.concept_card(
            "Functional Groups",
            "Chemical behavior from structure"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Functional groups
        self.add_marker("4.3.2.2", "groups")

        groups = VGroup()

        # Hydroxyl (-OH)
        oh = VGroup()
        oh_box = RoundedRectangle(width=2.5, height=1.2, corner_radius=0.1, stroke_color=TRD_COLORS["antimatter"])
        oh_label = Text("-OH", color=TRD_COLORS["antimatter"], font_size=20, weight="BOLD")
        oh_label.move_to(oh_box.get_center() + UP * 0.2)
        oh_name = Text("Hydroxyl", color=TRD_COLORS["text"], font_size=12)
        oh_name.next_to(oh_label, DOWN, buff=0.1)
        oh_ex = Text("(alcohols)", color=TRD_COLORS["text_dim"], font_size=10)
        oh_ex.next_to(oh_name, DOWN, buff=0.05)
        oh.add(oh_box, oh_label, oh_name, oh_ex)

        # Carboxyl (-COOH)
        cooh = VGroup()
        cooh_box = RoundedRectangle(width=2.5, height=1.2, corner_radius=0.1, stroke_color=TRD_COLORS["matter"])
        cooh_label = Text("-COOH", color=TRD_COLORS["matter"], font_size=20, weight="BOLD")
        cooh_label.move_to(cooh_box.get_center() + UP * 0.2)
        cooh_name = Text("Carboxyl", color=TRD_COLORS["text"], font_size=12)
        cooh_name.next_to(cooh_label, DOWN, buff=0.1)
        cooh_ex = Text("(acids)", color=TRD_COLORS["text_dim"], font_size=10)
        cooh_ex.next_to(cooh_name, DOWN, buff=0.05)
        cooh.add(cooh_box, cooh_label, cooh_name, cooh_ex)

        # Amino (-NH2)
        nh2 = VGroup()
        nh2_box = RoundedRectangle(width=2.5, height=1.2, corner_radius=0.1, stroke_color=TRD_COLORS["highlight"])
        nh2_label = Text("-NH₂", color=TRD_COLORS["highlight"], font_size=20, weight="BOLD")
        nh2_label.move_to(nh2_box.get_center() + UP * 0.2)
        nh2_name = Text("Amino", color=TRD_COLORS["text"], font_size=12)
        nh2_name.next_to(nh2_label, DOWN, buff=0.1)
        nh2_ex = Text("(amines)", color=TRD_COLORS["text_dim"], font_size=10)
        nh2_ex.next_to(nh2_name, DOWN, buff=0.05)
        nh2.add(nh2_box, nh2_label, nh2_name, nh2_ex)

        # Carbonyl (C=O)
        co = VGroup()
        co_box = RoundedRectangle(width=2.5, height=1.2, corner_radius=0.1, stroke_color=TRD_COLORS["glow"])
        co_label = Text("C=O", color=TRD_COLORS["glow"], font_size=20, weight="BOLD")
        co_label.move_to(co_box.get_center() + UP * 0.2)
        co_name = Text("Carbonyl", color=TRD_COLORS["text"], font_size=12)
        co_name.next_to(co_label, DOWN, buff=0.1)
        co_ex = Text("(aldehydes)", color=TRD_COLORS["text_dim"], font_size=10)
        co_ex.next_to(co_name, DOWN, buff=0.05)
        co.add(co_box, co_label, co_name, co_ex)

        groups.add(oh, cooh, nh2, co)
        groups.arrange_in_grid(rows=2, cols=2, buff=0.5)

        for g in groups:
            self.play(Create(g), run_time=0.5)

        # Note
        note = Text(
            "Functional groups determine chemical reactivity",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        note.to_edge(DOWN, buff=0.5)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class Isomers(TRDScene):
    """Structural isomers."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.3.1", "isomers")

        title = self.concept_card(
            "Isomers",
            "Same formula, different structure"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Example: C4H10
        self.add_marker("4.3.3.2", "example")

        formula = MathTex(r"C_4H_{10}", color=TRD_COLORS["highlight"], font_size=32)
        formula.to_edge(UP, buff=0.8)
        self.play(Write(formula))

        # n-Butane (straight chain)
        butane = VGroup()
        butane_label = Text("n-Butane", color=TRD_COLORS["matter"], font_size=16)

        # Simplified: 4 carbons in a row
        for i in range(4):
            c = Circle(radius=0.2, fill_color=TRD_COLORS["text"], fill_opacity=0.8, stroke_width=0)
            c.move_to([(i - 1.5) * 0.7, 0, 0])
            butane.add(c)
            if i > 0:
                bond = Line(
                    butane[i-1].get_center(),
                    c.get_center(),
                    color=TRD_COLORS["grid_bright"],
                    stroke_width=2,
                )
                butane.add(bond)

        butane_label.next_to(butane, DOWN, buff=0.3)
        butane.add(butane_label)
        butane.shift(LEFT * 2.5 + DOWN * 0.5)

        self.play(Create(butane))

        # Isobutane (branched)
        isobutane = VGroup()
        isobutane_label = Text("Isobutane", color=TRD_COLORS["antimatter"], font_size=16)

        # Central carbon with 3 branches
        center = Circle(radius=0.2, fill_color=TRD_COLORS["text"], fill_opacity=0.8, stroke_width=0)
        isobutane.add(center)

        positions = [LEFT * 0.6, RIGHT * 0.6, UP * 0.6]
        for pos in positions:
            c = Circle(radius=0.2, fill_color=TRD_COLORS["text"], fill_opacity=0.8, stroke_width=0)
            c.move_to(pos)
            bond = Line(ORIGIN, pos * 0.5, color=TRD_COLORS["grid_bright"], stroke_width=2)
            isobutane.add(bond, c)

        isobutane_label.next_to(isobutane, DOWN, buff=0.3)
        isobutane.add(isobutane_label)
        isobutane.shift(RIGHT * 2.5 + DOWN * 0.5)

        self.play(Create(isobutane))

        # Properties differ
        props = VGroup()
        p1 = Text("Same atoms, different arrangement", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("Different boiling points, reactivity", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2)
        props.arrange(DOWN, buff=0.15)
        props.to_edge(DOWN, buff=0.6)

        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class RingStructures(TRDScene):
    """Ring molecules (cyclic structures)."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.4.1", "rings")

        title = self.concept_card(
            "Ring Structures",
            "Cyclic molecules"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Benzene ring
        self.add_marker("4.3.4.2", "benzene")

        benzene = VGroup()
        benzene_label = Text("Benzene (C₆H₆)", color=TRD_COLORS["highlight"], font_size=18)

        # Hexagon of carbons
        n_atoms = 6
        radius = 1.0
        for i in range(n_atoms):
            angle = i * 2 * PI / n_atoms - PI / 2
            pos = radius * np.array([np.cos(angle), np.sin(angle), 0])
            c = Circle(radius=0.15, fill_color=TRD_COLORS["text"], fill_opacity=0.8, stroke_width=0)
            c.move_to(pos)
            benzene.add(c)

        # Alternating single/double bonds
        for i in range(n_atoms):
            start = benzene[i].get_center()
            end = benzene[(i+1) % n_atoms].get_center()
            if i % 2 == 0:
                # Double bond
                offset = 0.05 * np.array([np.cos(i * 2 * PI / n_atoms + PI/2),
                                          np.sin(i * 2 * PI / n_atoms + PI/2), 0])
                bond1 = Line(start + offset, end + offset, color=TRD_COLORS["highlight"], stroke_width=2)
                bond2 = Line(start - offset, end - offset, color=TRD_COLORS["highlight"], stroke_width=2)
                benzene.add(bond1, bond2)
            else:
                bond = Line(start, end, color=TRD_COLORS["grid_bright"], stroke_width=2)
                benzene.add(bond)

        benzene_label.next_to(benzene, DOWN, buff=0.3)
        benzene.add(benzene_label)
        benzene.shift(LEFT * 2.5)

        self.play(Create(benzene))

        # Cyclohexane
        cyclohexane = VGroup()
        cyclohexane_label = Text("Cyclohexane (C₆H₁₂)", color=TRD_COLORS["matter"], font_size=18)

        for i in range(6):
            angle = i * 2 * PI / 6 - PI / 2
            pos = radius * np.array([np.cos(angle), np.sin(angle), 0])
            c = Circle(radius=0.15, fill_color=TRD_COLORS["text"], fill_opacity=0.8, stroke_width=0)
            c.move_to(pos)
            cyclohexane.add(c)

        # All single bonds
        for i in range(6):
            start = cyclohexane[i].get_center()
            end = cyclohexane[(i+1) % 6].get_center()
            bond = Line(start, end, color=TRD_COLORS["grid_bright"], stroke_width=2)
            cyclohexane.add(bond)

        cyclohexane_label.next_to(cyclohexane, DOWN, buff=0.3)
        cyclohexane.add(cyclohexane_label)
        cyclohexane.shift(RIGHT * 2.5)

        self.play(Create(cyclohexane))

        # Note
        note = Text(
            "Ring strain affects stability and reactivity",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        note.to_edge(DOWN, buff=0.5)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class ComplexMoleculesSummary(TRDScene):
    """Summary of complex molecules."""

    def construct(self):
        self.load_narration("4.3")

        self.add_marker("4.3.5.1", "summary")

        title = self.trd_title("Complex Molecules")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Carbon: 4 bonds enables complex structures",
            "Functional groups determine reactivity",
            "Isomers: same formula, different structure",
            "Ring structures: cyclic arrangements",
            "TRD: All from electron flux configurations",
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

        # Final insight
        final = self.equation_box(
            r"\text{Complexity} = \text{Carbon versatility}",
            "Organic chemistry from simple bonding rules"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
