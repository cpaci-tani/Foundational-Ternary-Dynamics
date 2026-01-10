"""
Chapter 2.3: The Particle Zoo
=============================

Catalog of fundamental particles in TRD.
Shows how Standard Model particles map to voxel configurations.
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
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Text,
    MathTex,
    RoundedRectangle,
    Table,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, MODE_COLORS
from lib.components import VoxelMobject, FluxArrow


class ParticleZooIntro(TRDScene):
    """Introduction to the particle catalog."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.0.1", "title")
        title = self.trd_title("The Particle Zoo")
        subtitle = Text(
            "Standard Model from Voxel Configurations",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # The mapping question
        self.add_marker("2.3.0.2", "mapping")

        question = Text(
            "How do 3 states → 17+ particles?",
            color=TRD_COLORS["highlight"],
            font_size=32,
        )
        self.play(Write(question))
        self.wait(1)

        answer = Text(
            "Configuration + internal degrees of freedom",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        answer.next_to(question, DOWN, buff=0.5)
        self.play(Write(answer))
        self.wait(2)

        self.play(FadeOut(question), FadeOut(answer))

        self.export_markers()


class QuarksScene(TRDScene):
    """Quark configurations in TRD."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.1.1", "quarks")

        title = self.concept_card(
            "Quarks",
            "Fractional charge from flux direction"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Up quark
        self.add_marker("2.3.1.2", "up")

        up_voxel = VoxelMobject(state=+1, size=1.5, show_glow=True)
        up_voxel.shift(LEFT * 3 + UP * 1)
        up_label = Text("Up quark", color=TRD_COLORS["matter"], font_size=20)
        up_label.next_to(up_voxel, UP, buff=0.3)
        up_props = VGroup(
            MathTex(r"s = +1", color=TRD_COLORS["text"], font_size=18),
            MathTex(r"q = +\frac{2}{3}", color=TRD_COLORS["highlight"], font_size=18),
        )
        up_props.arrange(DOWN, buff=0.1)
        up_props.next_to(up_voxel, DOWN, buff=0.3)

        self.play(Create(up_voxel), Write(up_label))
        self.play(Write(up_props))

        # Down quark
        self.add_marker("2.3.1.3", "down")

        down_voxel = VoxelMobject(state=+1, size=1.5, show_glow=True)
        down_voxel.shift(RIGHT * 0 + UP * 1)
        down_label = Text("Down quark", color=TRD_COLORS["matter"], font_size=20)
        down_label.next_to(down_voxel, UP, buff=0.3)
        down_props = VGroup(
            MathTex(r"s = +1", color=TRD_COLORS["text"], font_size=18),
            MathTex(r"q = -\frac{1}{3}", color=TRD_COLORS["highlight"], font_size=18),
        )
        down_props.arrange(DOWN, buff=0.1)
        down_props.next_to(down_voxel, DOWN, buff=0.3)

        self.play(Create(down_voxel), Write(down_label))
        self.play(Write(down_props))

        # Anti-quarks
        self.add_marker("2.3.1.4", "antiquarks")

        anti_up = VoxelMobject(state=-1, size=1.5, show_glow=True)
        anti_up.shift(RIGHT * 3 + UP * 1)
        anti_up_label = Text("Anti-up", color=TRD_COLORS["antimatter"], font_size=20)
        anti_up_label.next_to(anti_up, UP, buff=0.3)
        anti_up_props = VGroup(
            MathTex(r"s = -1", color=TRD_COLORS["text"], font_size=18),
            MathTex(r"q = -\frac{2}{3}", color=TRD_COLORS["highlight"], font_size=18),
        )
        anti_up_props.arrange(DOWN, buff=0.1)
        anti_up_props.next_to(anti_up, DOWN, buff=0.3)

        self.play(Create(anti_up), Write(anti_up_label))
        self.play(Write(anti_up_props))

        # Color note
        color_note = Text(
            "Color = flux axis alignment (3 axes → 3 colors)",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        color_note.to_edge(DOWN, buff=0.8)
        self.play(Write(color_note))

        self.wait(2)

        self.export_markers()


class LeptonsScene(TRDScene):
    """Lepton configurations in TRD."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.2.1", "leptons")

        title = self.concept_card(
            "Leptons",
            "Color-neutral, integer charge"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Electron
        self.add_marker("2.3.2.2", "electron")

        electron_box = RoundedRectangle(
            width=3, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_opacity=0.1,
        )
        electron_box.shift(LEFT * 3)
        electron_label = Text("Electron", color=TRD_COLORS["antimatter"], font_size=22)
        electron_label.next_to(electron_box, UP, buff=0.2)

        electron_props = VGroup(
            MathTex(r"s = -1", font_size=20),
            MathTex(r"q = -1", font_size=20),
            Text("No color", font_size=14, color=TRD_COLORS["text_dim"]),
        )
        electron_props.arrange(DOWN, buff=0.15)
        electron_props.move_to(electron_box.get_center())

        self.play(Create(electron_box), Write(electron_label))
        self.play(Write(electron_props))

        # Positron
        positron_box = RoundedRectangle(
            width=3, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_opacity=0.1,
        )
        positron_box.shift(RIGHT * 0)
        positron_label = Text("Positron", color=TRD_COLORS["matter"], font_size=22)
        positron_label.next_to(positron_box, UP, buff=0.2)

        positron_props = VGroup(
            MathTex(r"s = +1", font_size=20),
            MathTex(r"q = +1", font_size=20),
            Text("No color", font_size=14, color=TRD_COLORS["text_dim"]),
        )
        positron_props.arrange(DOWN, buff=0.15)
        positron_props.move_to(positron_box.get_center())

        self.play(Create(positron_box), Write(positron_label))
        self.play(Write(positron_props))

        # Neutrino
        self.add_marker("2.3.2.3", "neutrino")

        neutrino_box = RoundedRectangle(
            width=3, height=2.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["void_light"],
            fill_opacity=0.1,
        )
        neutrino_box.shift(RIGHT * 3)
        neutrino_label = Text("Neutrino", color=TRD_COLORS["void_light"], font_size=22)
        neutrino_label.next_to(neutrino_box, UP, buff=0.2)

        neutrino_props = VGroup(
            MathTex(r"s = 0", font_size=20),
            MathTex(r"q = 0", font_size=20),
            Text("Distinct from void!", font_size=14, color=TRD_COLORS["highlight"]),
        )
        neutrino_props.arrange(DOWN, buff=0.15)
        neutrino_props.move_to(neutrino_box.get_center())

        self.play(Create(neutrino_box), Write(neutrino_label))
        self.play(Write(neutrino_props))

        # Key note
        note = Text(
            "Neutrino: s=0 with non-zero flux, not empty void",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        note.to_edge(DOWN, buff=0.8)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class BosonsScene(TRDScene):
    """Gauge bosons as flux patterns."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.3.1", "bosons")

        title = self.concept_card(
            "Gauge Bosons",
            "Force carriers as flux waves"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Photon
        self.add_marker("2.3.3.2", "photon")

        photon_box = RoundedRectangle(
            width=5, height=2,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["glow"],
            fill_opacity=0.1,
        )
        photon_box.shift(UP * 1.5)
        photon_label = Text("Photon", color=TRD_COLORS["glow"], font_size=24, weight="BOLD")
        photon_label.next_to(photon_box, LEFT, buff=0.3)

        photon_desc = VGroup(
            Text("Transverse flux wave", color=TRD_COLORS["text"], font_size=16),
            MathTex(r"s = 0 \text{ everywhere}", font_size=18),
            Text("2 polarizations from ∇·J = 0", color=TRD_COLORS["text_dim"], font_size=14),
        )
        photon_desc.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        photon_desc.move_to(photon_box.get_center())

        self.play(Create(photon_box), Write(photon_label))
        self.play(Write(photon_desc))

        # Gluon
        self.add_marker("2.3.3.3", "gluon")

        gluon_box = RoundedRectangle(
            width=5, height=2,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_opacity=0.1,
        )
        gluon_box.shift(DOWN * 0.5)
        gluon_label = Text("Gluon", color=TRD_COLORS["matter"], font_size=24, weight="BOLD")
        gluon_label.next_to(gluon_box, LEFT, buff=0.3)

        gluon_desc = VGroup(
            Text("Color-changing flux exchange", color=TRD_COLORS["text"], font_size=16),
            Text("8 types (from 3×3 - 1)", color=TRD_COLORS["text"], font_size=16),
            Text("Confining at large distance", color=TRD_COLORS["text_dim"], font_size=14),
        )
        gluon_desc.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        gluon_desc.move_to(gluon_box.get_center())

        self.play(Create(gluon_box), Write(gluon_label))
        self.play(Write(gluon_desc))

        # W/Z
        weak_box = RoundedRectangle(
            width=5, height=2,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_opacity=0.1,
        )
        weak_box.shift(DOWN * 2.5)
        weak_label = Text("W±, Z", color=TRD_COLORS["antimatter"], font_size=24, weight="BOLD")
        weak_label.next_to(weak_box, LEFT, buff=0.3)

        weak_desc = VGroup(
            Text("Massive flux excitations", color=TRD_COLORS["text"], font_size=16),
            Text("Enable transmutation", color=TRD_COLORS["text"], font_size=16),
            Text("Short range (Yukawa)", color=TRD_COLORS["text_dim"], font_size=14),
        )
        weak_desc.arrange(DOWN, buff=0.1, aligned_edge=LEFT)
        weak_desc.move_to(weak_box.get_center())

        self.play(Create(weak_box), Write(weak_label))
        self.play(Write(weak_desc))

        self.wait(2)

        self.export_markers()


class CompositeParticles(TRDScene):
    """Composite particles: protons, neutrons."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.4.1", "composite")

        title = self.concept_card(
            "Composite Particles",
            "Triads form nucleons"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Proton
        self.add_marker("2.3.4.2", "proton")

        proton_group = VGroup()

        # Three quarks in triangle
        u1 = Circle(radius=0.3, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0)
        u1_label = MathTex(r"u", color=TRD_COLORS["background"], font_size=20)
        u1_label.move_to(u1.get_center())

        u2 = Circle(radius=0.3, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0)
        u2_label = MathTex(r"u", color=TRD_COLORS["background"], font_size=20)
        u2_label.move_to(u2.get_center())

        d = Circle(radius=0.3, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0)
        d_label = MathTex(r"d", color=TRD_COLORS["background"], font_size=20)
        d_label.move_to(d.get_center())

        u1.shift(UP * 0.5)
        u1_label.shift(UP * 0.5)
        u2.shift(DOWN * 0.25 + LEFT * 0.45)
        u2_label.shift(DOWN * 0.25 + LEFT * 0.45)
        d.shift(DOWN * 0.25 + RIGHT * 0.45)
        d_label.shift(DOWN * 0.25 + RIGHT * 0.45)

        proton_group.add(u1, u1_label, u2, u2_label, d, d_label)
        proton_group.shift(LEFT * 3)

        proton_title = Text("Proton (uud)", color=TRD_COLORS["matter"], font_size=22)
        proton_title.next_to(proton_group, UP, buff=0.4)

        proton_charge = MathTex(
            r"q = +\frac{2}{3} + \frac{2}{3} - \frac{1}{3} = +1",
            color=TRD_COLORS["text"],
            font_size=18,
        )
        proton_charge.next_to(proton_group, DOWN, buff=0.4)

        self.play(Create(proton_group), Write(proton_title))
        self.play(Write(proton_charge))

        # Neutron
        self.add_marker("2.3.4.3", "neutron")

        neutron_group = VGroup()

        u = Circle(radius=0.3, fill_color=TRD_COLORS["matter"], fill_opacity=0.8, stroke_width=0)
        u_label = MathTex(r"u", color=TRD_COLORS["background"], font_size=20)
        u_label.move_to(u.get_center())

        d1 = Circle(radius=0.3, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0)
        d1_label = MathTex(r"d", color=TRD_COLORS["background"], font_size=20)
        d1_label.move_to(d1.get_center())

        d2 = Circle(radius=0.3, fill_color=TRD_COLORS["highlight"], fill_opacity=0.8, stroke_width=0)
        d2_label = MathTex(r"d", color=TRD_COLORS["background"], font_size=20)
        d2_label.move_to(d2.get_center())

        u.shift(UP * 0.5)
        u_label.shift(UP * 0.5)
        d1.shift(DOWN * 0.25 + LEFT * 0.45)
        d1_label.shift(DOWN * 0.25 + LEFT * 0.45)
        d2.shift(DOWN * 0.25 + RIGHT * 0.45)
        d2_label.shift(DOWN * 0.25 + RIGHT * 0.45)

        neutron_group.add(u, u_label, d1, d1_label, d2, d2_label)
        neutron_group.shift(RIGHT * 3)

        neutron_title = Text("Neutron (udd)", color=TRD_COLORS["void_light"], font_size=22)
        neutron_title.next_to(neutron_group, UP, buff=0.4)

        neutron_charge = MathTex(
            r"q = +\frac{2}{3} - \frac{1}{3} - \frac{1}{3} = 0",
            color=TRD_COLORS["text"],
            font_size=18,
        )
        neutron_charge.next_to(neutron_group, DOWN, buff=0.4)

        self.play(Create(neutron_group), Write(neutron_title))
        self.play(Write(neutron_charge))

        # Key insight
        insight = Text(
            "Triads are geometrically stable configurations",
            color=TRD_COLORS["highlight"],
            font_size=20,
        )
        insight.to_edge(DOWN, buff=0.5)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class ParticleTable(TRDScene):
    """Summary table of particle mappings."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.5.1", "table")

        title = self.trd_title("TRD Particle Catalog")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Simplified table as VGroups
        self.add_marker("2.3.5.2", "catalog")

        headers = VGroup(
            Text("Particle", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
            Text("State", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
            Text("Charge", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
            Text("Config", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD"),
        )
        headers.arrange(RIGHT, buff=1.5)
        headers.shift(UP * 2)

        self.play(Write(headers))

        particles = [
            ("Up quark", "+1", "+2/3", "Single voxel"),
            ("Down quark", "+1", "-1/3", "Single voxel"),
            ("Electron", "-1", "-1", "Single voxel"),
            ("Neutrino", "0*", "0", "Flux only"),
            ("Photon", "—", "0", "Flux wave"),
            ("Proton", "uud", "+1", "Triad"),
            ("Neutron", "udd", "0", "Triad"),
        ]

        rows = VGroup()
        y_pos = 1.3
        for name, state, charge, config in particles:
            row = VGroup(
                Text(name, color=TRD_COLORS["text"], font_size=14),
                Text(state, color=TRD_COLORS["text"], font_size=14),
                Text(charge, color=TRD_COLORS["text"], font_size=14),
                Text(config, color=TRD_COLORS["text_dim"], font_size=14),
            )
            row.arrange(RIGHT, buff=1.5)
            row.shift(UP * y_pos)
            rows.add(row)
            y_pos -= 0.45

        for row in rows:
            self.play(Write(row), run_time=0.4)

        # Note
        note = Text(
            "* Neutrino s=0 is distinct from empty void (has internal flux)",
            color=TRD_COLORS["text_dim"],
            font_size=14,
        )
        note.to_edge(DOWN, buff=0.5)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class ParticleZooSummary(TRDScene):
    """Summary of particle zoo."""

    def construct(self):
        self.load_narration("2.3")

        self.add_marker("2.3.6.1", "summary")

        title = self.trd_title("The Particle Zoo")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key insights
        points = [
            "Quarks: s=±1 with fractional charge (2/3, -1/3)",
            "Leptons: s=±1 with integer charge",
            "Neutrinos: s=0 but with flux (not empty)",
            "Photons: transverse flux waves (no state)",
            "Nucleons: stable triad configurations",
            "Color: 3 flux axes → SU(3) structure",
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

        # Final equation
        final = self.equation_box(
            r"\text{Standard Model} \subset \text{TRD configurations}",
            "Particles emerge from voxel dynamics"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
