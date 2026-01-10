"""
Chapter 3.1: Stable Structures
==============================

Triads and binding mechanisms in TRD.
Shows how quarks form nucleons through geometric stability.
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
    Flash,
    GrowArrow,
    Rotate,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Polygon,
    RegularPolygon,
    Text,
    MathTex,
    RoundedRectangle,
    Arc,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, MODE_COLORS
from lib.components import VoxelMobject, Lattice2D


class StableStructuresIntro(TRDScene):
    """Introduction to stable structures."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.0.1", "title")
        title = self.trd_title("Stable Structures")
        subtitle = Text(
            "How Particles Bind Together",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The question
        self.add_marker("3.1.0.2", "question")

        question = Text(
            "Why don't particles just evaporate?",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        self.play(Write(question))
        self.wait(1)

        answer = Text(
            "Geometric configurations can be self-sustaining",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        answer.next_to(question, DOWN, buff=0.5)
        self.play(Write(answer))
        self.wait(2)

        self.play(FadeOut(question), FadeOut(answer))

        self.export_markers()


class TriadGeometry(TRDScene):
    """The geometry of a stable triad."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.1.1", "triad")

        title = self.concept_card(
            "The Triad",
            "Three quarks in stable equilibrium"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Three quarks in equilateral triangle
        self.add_marker("3.1.1.2", "triangle")

        # Quark positions
        angle_offset = PI / 2
        radius = 1.5
        positions = [
            np.array([radius * np.cos(angle_offset + i * 2 * PI / 3),
                      radius * np.sin(angle_offset + i * 2 * PI / 3), 0])
            for i in range(3)
        ]

        quarks = VGroup()
        labels = VGroup()
        colors = [TRD_COLORS["matter"], TRD_COLORS["matter"], TRD_COLORS["highlight"]]
        quark_names = ["u", "u", "d"]

        for i, (pos, color, name) in enumerate(zip(positions, colors, quark_names)):
            quark = Circle(
                radius=0.4,
                fill_color=color,
                fill_opacity=0.8,
                stroke_color=TRD_COLORS["glow"],
                stroke_width=2,
            )
            quark.move_to(pos)
            label = MathTex(name, color=TRD_COLORS["background"], font_size=24)
            label.move_to(pos)
            quarks.add(quark)
            labels.add(label)

        # Connecting lines (gluon exchange)
        bonds = VGroup()
        for i in range(3):
            bond = Line(
                positions[i],
                positions[(i + 1) % 3],
                color=TRD_COLORS["glow"],
                stroke_width=3,
                stroke_opacity=0.5,
            )
            bonds.add(bond)

        self.play(Create(bonds))
        self.play(
            AnimationGroup(*[Create(q) for q in quarks], lag_ratio=0.2),
            AnimationGroup(*[Write(l) for l in labels], lag_ratio=0.2),
        )

        # Distance annotation
        dist_label = MathTex(
            r"d \approx \sqrt{2} \, \ell_P",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        dist_label.next_to(bonds[0], DOWN, buff=0.2)
        self.play(Write(dist_label))

        # Stability explanation
        self.add_marker("3.1.1.3", "stability")

        explanation = VGroup()
        e1 = Text("• Equilateral = symmetric flux distribution", color=TRD_COLORS["text"], font_size=16)
        e2 = Text("• Each quark reinforces the others", color=TRD_COLORS["text"], font_size=16)
        e3 = Text("• Net decay rate → 0 when locked", color=TRD_COLORS["highlight"], font_size=16)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        explanation.to_edge(DOWN, buff=0.6)

        for e in explanation:
            self.play(Write(e), run_time=0.5)

        self.wait(2)

        self.export_markers()


class BindingEnergy(TRDScene):
    """Binding energy and the is_locked flag."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.2.1", "binding")

        title = self.concept_card(
            "Binding Energy",
            "The cost of separation"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Bound state energy
        self.add_marker("3.1.2.2", "energy")

        bound = RoundedRectangle(
            width=4, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_opacity=0.1,
        )
        bound.shift(LEFT * 2.5)
        bound_label = Text("Bound State", color=TRD_COLORS["matter"], font_size=18)
        bound_label.next_to(bound, UP, buff=0.2)

        # Mini triad inside
        mini_quarks = VGroup()
        for i in range(3):
            angle = PI / 2 + i * 2 * PI / 3
            pos = bound.get_center() + 0.5 * np.array([np.cos(angle), np.sin(angle), 0])
            q = Dot(pos, radius=0.15, color=TRD_COLORS["matter"])
            mini_quarks.add(q)

        bound_energy = MathTex(
            r"E_{bound} = 3m_q - E_B",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        bound_energy.move_to(bound.get_center() + DOWN * 0.6)

        self.play(Create(bound), Write(bound_label))
        self.play(Create(mini_quarks), Write(bound_energy))

        # Free state energy
        free = RoundedRectangle(
            width=4, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_opacity=0.1,
        )
        free.shift(RIGHT * 2.5)
        free_label = Text("Free Quarks", color=TRD_COLORS["antimatter"], font_size=18)
        free_label.next_to(free, UP, buff=0.2)

        # Separated quarks
        free_quarks = VGroup()
        for i, offset in enumerate([LEFT * 0.8, ORIGIN, RIGHT * 0.8]):
            q = Dot(free.get_center() + offset, radius=0.15, color=TRD_COLORS["antimatter"])
            free_quarks.add(q)

        free_energy = MathTex(
            r"E_{free} = 3m_q",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        free_energy.move_to(free.get_center() + DOWN * 0.6)

        self.play(Create(free), Write(free_label))
        self.play(Create(free_quarks), Write(free_energy))

        # Binding energy
        self.add_marker("3.1.2.3", "eb")

        binding = MathTex(
            r"E_B \approx K_B \cdot \phi",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        binding.to_edge(DOWN, buff=1.0)

        phi_note = Text(
            "(φ = golden ratio ≈ 1.618)",
            color=TRD_COLORS["text_dim"],
            font_size=14,
        )
        phi_note.next_to(binding, DOWN, buff=0.2)

        self.play(Write(binding))
        self.play(Write(phi_note))

        self.wait(2)

        self.export_markers()


class ColorNeutrality(TRDScene):
    """Color confinement and neutrality."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.3.1", "color")

        title = self.concept_card(
            "Color Confinement",
            "Why free quarks don't exist"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Three colors
        self.add_marker("3.1.3.2", "rgb")

        color_label = Text("Three color charges:", color=TRD_COLORS["text"], font_size=20)
        color_label.shift(UP * 2.5)
        self.play(Write(color_label))

        colors_data = [
            ("Red", "#ff4466", "x-axis flux"),
            ("Green", "#44ff66", "y-axis flux"),
            ("Blue", "#4466ff", "z-axis flux"),
        ]

        color_boxes = VGroup()
        for i, (name, hex_color, desc) in enumerate(colors_data):
            box = RoundedRectangle(
                width=2.2, height=1.5,
                corner_radius=0.1,
                stroke_color=hex_color,
                fill_color=hex_color,
                fill_opacity=0.3,
            )
            box.shift(UP * 1 + (i - 1) * RIGHT * 2.8)
            label = Text(name, color=hex_color, font_size=18)
            label.move_to(box.get_center() + UP * 0.2)
            desc_text = Text(desc, color=TRD_COLORS["text_dim"], font_size=12)
            desc_text.next_to(label, DOWN, buff=0.15)
            color_boxes.add(VGroup(box, label, desc_text))

        for cb in color_boxes:
            self.play(Create(cb), run_time=0.5)

        # Color neutral combinations
        self.add_marker("3.1.3.3", "neutral")

        neutral_label = Text("Color-neutral combinations:", color=TRD_COLORS["highlight"], font_size=18)
        neutral_label.shift(DOWN * 0.8)
        self.play(Write(neutral_label))

        combos = VGroup()

        # Baryon: R + G + B
        baryon = VGroup()
        b_label = Text("Baryon (qqq):", color=TRD_COLORS["text"], font_size=14)
        b_dots = VGroup(
            Dot(color="#ff4466", radius=0.12),
            Text("+", color=TRD_COLORS["text"], font_size=14),
            Dot(color="#44ff66", radius=0.12),
            Text("+", color=TRD_COLORS["text"], font_size=14),
            Dot(color="#4466ff", radius=0.12),
            Text("= white", color=TRD_COLORS["text"], font_size=14),
        )
        b_dots.arrange(RIGHT, buff=0.1)
        b_dots.next_to(b_label, RIGHT, buff=0.2)
        baryon.add(b_label, b_dots)

        # Meson: color + anticolor
        meson = VGroup()
        m_label = Text("Meson (q q̄):", color=TRD_COLORS["text"], font_size=14)
        m_dots = VGroup(
            Dot(color="#ff4466", radius=0.12),
            Text("+", color=TRD_COLORS["text"], font_size=14),
            Circle(radius=0.12, stroke_color="#44ffff", stroke_width=2, fill_opacity=0),
            Text("= white", color=TRD_COLORS["text"], font_size=14),
        )
        m_dots.arrange(RIGHT, buff=0.1)
        m_dots.next_to(m_label, RIGHT, buff=0.2)
        meson.add(m_label, m_dots)

        combos.add(baryon, meson)
        combos.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        combos.shift(DOWN * 2)

        for c in combos:
            self.play(Write(c), run_time=0.6)

        self.wait(2)

        self.export_markers()


class ProtonFormation(TRDScene):
    """Step-by-step proton formation."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.4.1", "proton")

        title = self.concept_card(
            "Proton Formation",
            "Two up quarks + one down quark"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Start with free quarks
        self.add_marker("3.1.4.2", "free")

        free_label = Text("Free quarks:", color=TRD_COLORS["text_dim"], font_size=18)
        free_label.to_edge(UP, buff=0.8)
        self.play(Write(free_label))

        # Three separate quarks
        u1 = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0),
            MathTex("u", color=TRD_COLORS["background"], font_size=20),
        )
        u1[1].move_to(u1[0].get_center())
        u1.shift(LEFT * 3)

        u2 = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0),
            MathTex("u", color=TRD_COLORS["background"], font_size=20),
        )
        u2[1].move_to(u2[0].get_center())

        d = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0),
            MathTex("d", color=TRD_COLORS["background"], font_size=20),
        )
        d[1].move_to(d[0].get_center())
        d.shift(RIGHT * 3)

        self.play(FadeIn(u1), FadeIn(u2), FadeIn(d))

        # Strong force attraction
        self.add_marker("3.1.4.3", "attract")

        attract_label = Text("Strong force pulls them together...", color=TRD_COLORS["highlight"], font_size=16)
        attract_label.to_edge(DOWN, buff=1.5)
        self.play(Write(attract_label))

        # Move to triangle
        final_positions = [
            UP * 0.8,
            DOWN * 0.4 + LEFT * 0.7,
            DOWN * 0.4 + RIGHT * 0.7,
        ]

        self.play(
            u1.animate.move_to(final_positions[0]),
            u2.animate.move_to(final_positions[1]),
            d.animate.move_to(final_positions[2]),
            run_time=2.0,
        )

        # Add gluon field
        self.add_marker("3.1.4.4", "gluon")

        gluon_field = Circle(
            radius=1.2,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=3,
            stroke_opacity=0.5,
            fill_opacity=0,
        )
        gluon_field.move_to(VGroup(u1, u2, d).get_center())

        self.play(Create(gluon_field))
        self.play(Flash(gluon_field, color=TRD_COLORS["glow"], flash_radius=0.5))

        # Result
        result = VGroup()
        proton_label = Text("Proton (p)", color=TRD_COLORS["matter"], font_size=24, weight="BOLD")
        charge_label = MathTex(r"q = +\frac{2}{3} + \frac{2}{3} - \frac{1}{3} = +1", font_size=20)
        mass_label = Text("m ≈ 938 MeV", color=TRD_COLORS["text_dim"], font_size=16)

        result.add(proton_label, charge_label, mass_label)
        result.arrange(DOWN, buff=0.2)
        result.to_edge(RIGHT, buff=0.8)

        self.play(
            Transform(attract_label, result),
        )

        self.wait(2)

        self.export_markers()


class NeutronFormation(TRDScene):
    """Neutron structure and beta decay."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.5.1", "neutron")

        title = self.concept_card(
            "The Neutron",
            "One up quark + two down quarks"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Neutron structure
        self.add_marker("3.1.5.2", "structure")

        # Quarks in triangle
        positions = [UP * 0.8, DOWN * 0.4 + LEFT * 0.7, DOWN * 0.4 + RIGHT * 0.7]

        u = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0),
            MathTex("u", color=TRD_COLORS["background"], font_size=20),
        )
        u[1].move_to(u[0].get_center())
        u.move_to(positions[0])

        d1 = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0),
            MathTex("d", color=TRD_COLORS["background"], font_size=20),
        )
        d1[1].move_to(d1[0].get_center())
        d1.move_to(positions[1])

        d2 = VGroup(
            Circle(radius=0.4, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0),
            MathTex("d", color=TRD_COLORS["background"], font_size=20),
        )
        d2[1].move_to(d2[0].get_center())
        d2.move_to(positions[2])

        gluon_field = Circle(
            radius=1.2,
            stroke_color=TRD_COLORS["void_light"],
            stroke_width=3,
            stroke_opacity=0.5,
        )
        gluon_field.move_to(ORIGIN + DOWN * 0.1)

        self.play(Create(gluon_field))
        self.play(FadeIn(u), FadeIn(d1), FadeIn(d2))

        # Labels
        neutron_label = Text("Neutron (n)", color=TRD_COLORS["void_light"], font_size=24, weight="BOLD")
        neutron_label.to_edge(UP, buff=0.8)

        charge = MathTex(r"q = +\frac{2}{3} - \frac{1}{3} - \frac{1}{3} = 0", font_size=22)
        charge.next_to(neutron_label, DOWN, buff=0.3)

        self.play(Write(neutron_label), Write(charge))

        # Beta decay note
        self.add_marker("3.1.5.3", "decay")

        decay_note = VGroup()
        dn1 = Text("Free neutrons are unstable:", color=TRD_COLORS["text"], font_size=16)
        dn2 = MathTex(r"n \to p + e^- + \bar{\nu}_e", color=TRD_COLORS["antimatter"], font_size=22)
        dn3 = Text("τ ≈ 15 minutes", color=TRD_COLORS["text_dim"], font_size=14)

        decay_note.add(dn1, dn2, dn3)
        decay_note.arrange(DOWN, buff=0.15)
        decay_note.to_edge(DOWN, buff=0.6)

        for dn in decay_note:
            self.play(Write(dn), run_time=0.5)

        self.wait(2)

        self.export_markers()


class StableStructuresSummary(TRDScene):
    """Summary of stable structures."""

    def construct(self):
        self.load_narration("3.1")

        self.add_marker("3.1.6.1", "summary")

        title = self.trd_title("Stable Structures")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Triads: 3 quarks in equilateral arrangement",
            "Binding energy: E_B ≈ K_B × φ",
            "Color neutrality: R + G + B = white",
            "Proton: uud (charge +1, stable)",
            "Neutron: udd (charge 0, decays when free)",
            "is_locked flag suppresses evaporation",
        ]

        point_mobs = VGroup()
        for point in points:
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=20)
            text = Text(point, color=TRD_COLORS["text"], font_size=16)
            text.next_to(bullet, RIGHT, buff=0.15)
            point_mobs.add(VGroup(bullet, text))

        point_mobs.arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        point_mobs.center()

        for point in point_mobs:
            self.play(Write(point), run_time=0.45)

        self.wait(2)

        # Final equation
        final = self.equation_box(
            r"\text{Stability} = \text{Geometry} + \text{Color neutrality}",
            "Structure determines persistence"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
