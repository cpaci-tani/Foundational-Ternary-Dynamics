"""
Chapter 6.3: Polymers
=====================

Long-chain molecules and their properties.
Shows polymer structures in TRD flux framework.
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
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS


class PolymersIntro(TRDScene):
    """Introduction to polymers."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.0.1", "title")
        title = self.trd_title("Polymers")
        subtitle = Text(
            "Long-Chain Molecules",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class Polymerization(TRDScene):
    """How polymers form."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.1.1", "polymerization")

        title = self.concept_card(
            "Polymerization",
            "Building long chains"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Monomers
        self.add_marker("6.3.1.2", "monomers")

        mono_label = Text("Monomers", color=TRD_COLORS["text"], font_size=18)
        mono_label.shift(UP * 2.5)
        self.play(Write(mono_label))

        monomers = VGroup()
        for i in range(5):
            mono = Circle(
                radius=0.25,
                fill_color=TRD_COLORS["matter"],
                fill_opacity=0.8,
                stroke_color=TRD_COLORS["highlight"],
                stroke_width=2,
            )
            mono.move_to([(i - 2) * 1.2, 1, 0])
            monomers.add(mono)

        self.play(Create(monomers))

        # Show linking
        self.add_marker("6.3.1.3", "linking")

        arrow = Text("→", color=TRD_COLORS["highlight"], font_size=36)
        self.play(Write(arrow))

        # Polymer chain
        poly_label = Text("Polymer", color=TRD_COLORS["text"], font_size=18)
        poly_label.shift(DOWN * 0.5)
        self.play(Write(poly_label))

        # Linked chain
        chain = VGroup()
        for i in range(5):
            mono = Circle(
                radius=0.25,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            mono.move_to([(i - 2) * 0.6, -1.5, 0])
            chain.add(mono)

        # Links between monomers
        links = VGroup()
        for i in range(4):
            link = Line(
                chain[i].get_right(),
                chain[i + 1].get_left(),
                color=TRD_COLORS["highlight"],
                stroke_width=3,
            )
            links.add(link)

        self.play(
            Transform(monomers.copy(), chain),
            Create(links),
        )

        # Equation
        eq = MathTex(
            r"n \cdot \text{monomer} \rightarrow (-\text{monomer}-)_n",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        eq.to_edge(DOWN, buff=0.5)
        self.play(Write(eq))

        self.wait(2)

        self.export_markers()


class PolymerTypes(TRDScene):
    """Types of polymers."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.2.1", "types")

        title = self.concept_card(
            "Polymer Types",
            "Linear, branched, cross-linked"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Linear polymer
        self.add_marker("6.3.2.2", "linear")

        linear_label = Text("Linear", color=TRD_COLORS["matter"], font_size=16)
        linear_label.shift(LEFT * 3.5 + UP * 2)

        linear = VGroup()
        for i in range(8):
            node = Dot(
                point=LEFT * 3.5 + RIGHT * (i - 3.5) * 0.35 + UP * 0.5,
                radius=0.08,
                color=TRD_COLORS["matter"],
            )
            linear.add(node)
        for i in range(7):
            link = Line(
                linear[i].get_center(),
                linear[i + 1].get_center(),
                color=TRD_COLORS["matter"],
                stroke_width=2,
            )
            linear.add(link)

        self.play(Write(linear_label), Create(linear))

        # Branched polymer
        self.add_marker("6.3.2.3", "branched")

        branched_label = Text("Branched", color=TRD_COLORS["antimatter"], font_size=16)
        branched_label.shift(UP * 2)

        branched = VGroup()
        # Main chain
        main_positions = [
            [-1.2, 0.5, 0], [-0.8, 0.5, 0], [-0.4, 0.5, 0],
            [0, 0.5, 0], [0.4, 0.5, 0], [0.8, 0.5, 0], [1.2, 0.5, 0]
        ]
        for pos in main_positions:
            node = Dot(point=pos, radius=0.08, color=TRD_COLORS["antimatter"])
            branched.add(node)

        # Connect main chain
        for i in range(6):
            link = Line(
                main_positions[i],
                main_positions[i + 1],
                color=TRD_COLORS["antimatter"],
                stroke_width=2,
            )
            branched.add(link)

        # Branches
        branch_positions = [
            ([-0.4, 0.5, 0], [-0.4, 0.9, 0], [-0.4, 1.2, 0]),
            ([0.4, 0.5, 0], [0.4, 0.1, 0], [0.4, -0.2, 0]),
        ]
        for base, mid, end in branch_positions:
            for pos in [mid, end]:
                node = Dot(point=pos, radius=0.08, color=TRD_COLORS["highlight"])
                branched.add(node)
            link1 = Line(base, mid, color=TRD_COLORS["highlight"], stroke_width=2)
            link2 = Line(mid, end, color=TRD_COLORS["highlight"], stroke_width=2)
            branched.add(link1, link2)

        self.play(Write(branched_label), Create(branched))

        # Cross-linked polymer
        self.add_marker("6.3.2.4", "crosslinked")

        cross_label = Text("Cross-linked", color=TRD_COLORS["highlight"], font_size=16)
        cross_label.shift(RIGHT * 3.5 + UP * 2)

        cross = VGroup()
        # Two parallel chains
        for j, y in enumerate([0.8, 0.2]):
            for i in range(6):
                node = Dot(
                    point=RIGHT * 3.5 + [(i - 2.5) * 0.35, y, 0],
                    radius=0.08,
                    color=TRD_COLORS["highlight"],
                )
                cross.add(node)

        # Horizontal links
        for row in range(2):
            for i in range(5):
                idx = row * 6 + i
                link = Line(
                    cross[idx].get_center(),
                    cross[idx + 1].get_center(),
                    color=TRD_COLORS["highlight"],
                    stroke_width=2,
                )
                cross.add(link)

        # Vertical cross-links
        cross_links_pos = [1, 3]
        for i in cross_links_pos:
            link = Line(
                cross[i].get_center(),
                cross[i + 6].get_center(),
                color=TRD_COLORS["glow"],
                stroke_width=2,
            )
            cross.add(link)

        self.play(Write(cross_label), Create(cross))

        # Properties note
        props = VGroup(
            Text("Flexible", color=TRD_COLORS["matter"], font_size=12),
            Text("Intermediate", color=TRD_COLORS["antimatter"], font_size=12),
            Text("Rigid", color=TRD_COLORS["highlight"], font_size=12),
        )
        props[0].move_to(LEFT * 3.5 + DOWN * 0.5)
        props[1].move_to(DOWN * 0.5)
        props[2].move_to(RIGHT * 3.5 + DOWN * 0.5)

        self.play(Write(props))

        self.wait(2)

        self.export_markers()


class PolymerProperties(TRDScene):
    """Properties of polymers."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.3.1", "properties")

        title = self.concept_card(
            "Polymer Properties",
            "Structure determines behavior"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Property boxes
        self.add_marker("6.3.3.2", "props_list")

        properties = [
            ("Thermoplastics", "Soften when heated\nCan be remolded", TRD_COLORS["matter"]),
            ("Thermosets", "Harden permanently\nCross-linked", TRD_COLORS["antimatter"]),
            ("Elastomers", "Stretchy, flexible\nRubber-like", TRD_COLORS["highlight"]),
        ]

        boxes = VGroup()
        for name, desc, color in properties:
            box = RoundedRectangle(
                width=3.2, height=1.5,
                corner_radius=0.1,
                stroke_color=color,
                fill_opacity=0.1,
            )
            name_text = Text(name, color=color, font_size=16, weight="BOLD")
            name_text.move_to(box.get_top() + DOWN * 0.3)
            desc_text = Text(desc, color=TRD_COLORS["text"], font_size=11)
            desc_text.move_to(box.get_center() + DOWN * 0.15)
            boxes.add(VGroup(box, name_text, desc_text))

        boxes.arrange(RIGHT, buff=0.3)

        for box in boxes:
            self.play(Create(box), run_time=0.6)

        # Examples
        examples = VGroup(
            Text("PE, PP, PVC", color=TRD_COLORS["text_dim"], font_size=10),
            Text("Epoxy, Bakelite", color=TRD_COLORS["text_dim"], font_size=10),
            Text("Rubber, Silicone", color=TRD_COLORS["text_dim"], font_size=10),
        )
        for i, ex in enumerate(examples):
            ex.next_to(boxes[i], DOWN, buff=0.15)

        self.play(Write(examples))

        self.wait(2)

        self.export_markers()


class CommonPolymers(TRDScene):
    """Common polymer examples."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.4.1", "common")

        title = self.concept_card(
            "Common Polymers",
            "Everyday materials"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Table of common polymers
        self.add_marker("6.3.4.2", "table")

        polymers = [
            ("Polyethylene (PE)", "Bags, bottles"),
            ("Polypropylene (PP)", "Containers, rope"),
            ("PVC", "Pipes, vinyl"),
            ("Polystyrene (PS)", "Foam, packaging"),
            ("Nylon", "Fabrics, rope"),
            ("Polyester", "Clothing, bottles"),
        ]

        table = VGroup()
        for name, uses in polymers:
            row = VGroup(
                Text(name, color=TRD_COLORS["highlight"], font_size=14),
                Text(uses, color=TRD_COLORS["text"], font_size=12),
            )
            row.arrange(RIGHT, buff=0.5)
            table.add(row)

        table.arrange(DOWN, buff=0.25, aligned_edge=LEFT)
        table.shift(LEFT * 0.5)

        for row in table:
            self.play(Write(row), run_time=0.4)

        # TRD note
        trd = Text(
            "TRD: Polymer chains = extended flux coupling networks",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.4)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class PolymersSummary(TRDScene):
    """Summary of polymers."""

    def construct(self):
        self.load_narration("6.3")

        self.add_marker("6.3.5.1", "summary")

        title = self.trd_title("Polymers")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Polymers: long chains of repeating units",
            "Formed by linking monomers together",
            "Structure types: linear, branched, cross-linked",
            "Properties depend on structure and bonding",
            "TRD: Chains = extended flux configurations",
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
            r"(-\text{CH}_2-\text{CH}_2-)_n",
            "Polyethylene: simplest synthetic polymer"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
