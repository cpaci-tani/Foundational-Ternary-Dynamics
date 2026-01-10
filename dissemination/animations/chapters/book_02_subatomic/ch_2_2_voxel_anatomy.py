"""
Chapter 2.2: Voxel Anatomy
==========================

Exploded view of voxel internal structure.
Shows all data fields and their physical meaning.
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
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Square,
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
from lib.components import VoxelMobject, FluxArrow


class VoxelAnatomyIntro(TRDScene):
    """Introduction to voxel structure."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.0.1", "title")
        title = self.trd_title("Voxel Anatomy")
        subtitle = Text(
            "Inside the Fundamental Unit",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Teaser
        self.add_marker("2.2.0.2", "teaser")

        voxel = VoxelMobject(state=0, size=3.0, show_glow=True)
        self.play(Create(voxel, run_time=2.0))

        question = Text(
            "What's inside a voxel?",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        question.to_edge(DOWN, buff=1.0)
        self.play(Write(question))

        self.wait(2)
        self.play(FadeOut(voxel), FadeOut(question))

        self.export_markers()


class IdentityFields(TRDScene):
    """Voxel identity: position, UUID, partner."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.1.1", "identity")

        title = self.concept_card(
            "Identity Fields",
            "How a voxel knows itself"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Voxel box
        voxel_box = RoundedRectangle(
            width=4, height=5,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["grid_bright"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.8,
        )
        voxel_box.shift(LEFT * 2)

        box_label = Text("VOXEL", color=TRD_COLORS["highlight"], font_size=20)
        box_label.next_to(voxel_box, UP, buff=0.2)

        self.play(Create(voxel_box), Write(box_label))

        # Identity fields
        self.add_marker("2.2.1.2", "position")

        fields = VGroup()

        # Position
        pos_label = Text("position:", color=TRD_COLORS["text"], font_size=16)
        pos_value = MathTex(r"(x, y, z) \in \mathbb{Z}^3", font_size=18, color=TRD_COLORS["highlight"])
        pos_value.next_to(pos_label, RIGHT, buff=0.2)
        pos_group = VGroup(pos_label, pos_value)

        # UUID
        uuid_label = Text("uuid:", color=TRD_COLORS["text"], font_size=16)
        uuid_value = Text("unique identifier", font_size=14, color=TRD_COLORS["text_dim"])
        uuid_value.next_to(uuid_label, RIGHT, buff=0.2)
        uuid_group = VGroup(uuid_label, uuid_value)

        # Partner UUID
        partner_label = Text("partner_uuid:", color=TRD_COLORS["text"], font_size=16)
        partner_value = Text("entanglement link", font_size=14, color=TRD_COLORS["antimatter"])
        partner_value.next_to(partner_label, RIGHT, buff=0.2)
        partner_group = VGroup(partner_label, partner_value)

        fields.add(pos_group, uuid_group, partner_group)
        fields.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        fields.move_to(voxel_box.get_center())

        for field in fields:
            self.play(Write(field), run_time=0.6)

        # Explanation
        explanation = VGroup()
        e1 = Text("• Position in the lattice", color=TRD_COLORS["text"], font_size=16)
        e2 = Text("• Unique tracking ID", color=TRD_COLORS["text"], font_size=16)
        e3 = Text("• Link to entangled partner", color=TRD_COLORS["text"], font_size=16)

        explanation.add(e1, e2, e3)
        explanation.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        explanation.shift(RIGHT * 3)

        for e in explanation:
            self.play(Write(e), run_time=0.4)

        self.wait(2)
        self.play(FadeOut(voxel_box), FadeOut(box_label), FadeOut(fields), FadeOut(explanation))

        self.export_markers()


class StateFields(TRDScene):
    """Ontological state: the ternary value."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.2.1", "state")

        title = self.concept_card(
            "Ontological State",
            "The ternary reality"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # State display
        self.add_marker("2.2.2.2", "ternary")

        states = VGroup()

        # State -1
        minus_box = RoundedRectangle(
            width=2.5, height=2.0,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.2,
        )
        minus_label = MathTex(r"s = -1", color=TRD_COLORS["antimatter"], font_size=32)
        minus_label.move_to(minus_box.get_center() + UP * 0.3)
        minus_name = Text("Antimatter", color=TRD_COLORS["antimatter"], font_size=16)
        minus_name.next_to(minus_label, DOWN, buff=0.2)
        minus_group = VGroup(minus_box, minus_label, minus_name)

        # State 0
        zero_box = RoundedRectangle(
            width=2.5, height=2.0,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["void_light"],
            fill_color=TRD_COLORS["void"],
            fill_opacity=0.3,
        )
        zero_label = MathTex(r"s = 0", color=TRD_COLORS["void_light"], font_size=32)
        zero_label.move_to(zero_box.get_center() + UP * 0.3)
        zero_name = Text("Void", color=TRD_COLORS["void_light"], font_size=16)
        zero_name.next_to(zero_label, DOWN, buff=0.2)
        zero_group = VGroup(zero_box, zero_label, zero_name)

        # State +1
        plus_box = RoundedRectangle(
            width=2.5, height=2.0,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_color=TRD_COLORS["matter"],
            fill_opacity=0.2,
        )
        plus_label = MathTex(r"s = +1", color=TRD_COLORS["matter"], font_size=32)
        plus_label.move_to(plus_box.get_center() + UP * 0.3)
        plus_name = Text("Matter", color=TRD_COLORS["matter"], font_size=16)
        plus_name.next_to(plus_label, DOWN, buff=0.2)
        plus_group = VGroup(plus_box, plus_label, plus_name)

        states.add(minus_group, zero_group, plus_group)
        states.arrange(RIGHT, buff=0.5)

        for state in states:
            self.play(Create(state), run_time=0.6)

        # Key insight
        insight = Text(
            "Every voxel is exactly one of these three states",
            color=TRD_COLORS["text"],
            font_size=20,
        )
        insight.to_edge(DOWN, buff=1.0)
        self.play(Write(insight))

        # Charge field
        self.add_marker("2.2.2.3", "charge")

        charge_note = MathTex(
            r"\text{charge} \in \{-1, -\tfrac{1}{3}, +\tfrac{2}{3}, +1\}",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        charge_note.to_edge(UP, buff=1.0)
        self.play(Write(charge_note))

        self.wait(2)

        self.export_markers()


class FluxFields(TRDScene):
    """The flux vector: continuous energy field."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.3.1", "flux")

        title = self.concept_card(
            "The Flux Vector",
            "Continuous potential within discrete structure"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Voxel with flux
        self.add_marker("2.2.3.2", "vector")

        voxel = VoxelMobject(state=0, size=2.5, show_glow=True)
        voxel.shift(LEFT * 2)
        self.play(Create(voxel))

        # Flux arrow
        flux_arrow = Arrow(
            start=voxel.get_center(),
            end=voxel.get_center() + RIGHT * 1.5 + UP * 0.8,
            color=TRD_COLORS["highlight"],
            buff=0.2,
            stroke_width=4,
        )
        flux_label = MathTex(
            r"\mathbf{J} \in \mathbb{R}^3",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        flux_label.next_to(flux_arrow.get_end(), RIGHT, buff=0.2)

        self.play(GrowArrow(flux_arrow), Write(flux_label))

        # Derived quantities
        self.add_marker("2.2.3.3", "derived")

        derived = VGroup()

        d1 = MathTex(
            r"\rho = |\mathbf{J}|",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        d1_label = Text("density (energy)", color=TRD_COLORS["text_dim"], font_size=14)
        d1_label.next_to(d1, RIGHT, buff=0.2)

        d2 = MathTex(
            r"\omega = \text{frequency}",
            color=TRD_COLORS["text"],
            font_size=22,
        )
        d2_label = Text("oscillation rate", color=TRD_COLORS["text_dim"], font_size=14)
        d2_label.next_to(d2, RIGHT, buff=0.2)

        derived.add(VGroup(d1, d1_label), VGroup(d2, d2_label))
        derived.arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        derived.shift(RIGHT * 2.5)

        for d in derived:
            self.play(Write(d), run_time=0.5)

        # The key insight
        insight = Text(
            "Flux = dispositional potential (what COULD manifest)",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        insight.to_edge(DOWN, buff=0.8)
        self.play(Write(insight))

        self.wait(2)

        self.export_markers()


class MechanicalFields(TRDScene):
    """Force and motion fields."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.4.1", "mechanical")

        title = self.concept_card(
            "Mechanical State",
            "Forces, motion, and position"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Field list
        self.add_marker("2.2.4.2", "fields")

        fields = VGroup()

        f1 = VGroup(
            Text("force_accumulator:", color=TRD_COLORS["highlight"], font_size=18),
            MathTex(r"\mathbf{F} \in \mathbb{R}^3", color=TRD_COLORS["text"], font_size=20),
        )
        f1[1].next_to(f1[0], RIGHT, buff=0.2)

        f2 = VGroup(
            Text("position_remainder:", color=TRD_COLORS["highlight"], font_size=18),
            MathTex(r"\delta\mathbf{x} \in [0,1)^3", color=TRD_COLORS["text"], font_size=20),
        )
        f2[1].next_to(f2[0], RIGHT, buff=0.2)

        f3 = VGroup(
            Text("wave_velocity:", color=TRD_COLORS["highlight"], font_size=18),
            MathTex(r"\mathbf{v}_{wave} \in \mathbb{R}^3", color=TRD_COLORS["text"], font_size=20),
        )
        f3[1].next_to(f3[0], RIGHT, buff=0.2)

        fields.add(f1, f2, f3)
        fields.arrange(DOWN, buff=0.5, aligned_edge=LEFT)
        fields.shift(LEFT * 1)

        for f in fields:
            self.play(Write(f), run_time=0.6)

        # Explanations
        explanations = VGroup()
        e1 = Text("Sum of all forces acting", color=TRD_COLORS["text_dim"], font_size=14)
        e2 = Text("Sub-lattice position offset", color=TRD_COLORS["text_dim"], font_size=14)
        e3 = Text("Flux wave propagation rate", color=TRD_COLORS["text_dim"], font_size=14)

        explanations.add(e1, e2, e3)
        for i, e in enumerate(explanations):
            e.next_to(fields[i], DOWN, buff=0.1, aligned_edge=LEFT)
            self.play(Write(e), run_time=0.4)

        self.wait(2)

        self.export_markers()


class FlagFields(TRDScene):
    """Boolean flags for special states."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.5.1", "flags")

        title = self.concept_card(
            "State Flags",
            "Boolean markers for special conditions"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Flags
        self.add_marker("2.2.5.2", "boolean")

        flags = VGroup()

        # is_locked
        lock_box = RoundedRectangle(
            width=4, height=1.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_opacity=0.1,
        )
        lock_label = Text("is_locked", color=TRD_COLORS["matter"], font_size=20)
        lock_desc = Text(
            "Part of bound structure (triad, atom)",
            color=TRD_COLORS["text"],
            font_size=14,
        )
        lock_label.move_to(lock_box.get_center() + UP * 0.2)
        lock_desc.next_to(lock_label, DOWN, buff=0.15)
        lock_group = VGroup(lock_box, lock_label, lock_desc)

        # is_active
        active_box = RoundedRectangle(
            width=4, height=1.5,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["antimatter"],
            fill_opacity=0.1,
        )
        active_label = Text("is_active", color=TRD_COLORS["antimatter"], font_size=20)
        active_desc = Text(
            "Passed phase gate this tick",
            color=TRD_COLORS["text"],
            font_size=14,
        )
        active_label.move_to(active_box.get_center() + UP * 0.2)
        active_desc.next_to(active_label, DOWN, buff=0.15)
        active_group = VGroup(active_box, active_label, active_desc)

        flags.add(lock_group, active_group)
        flags.arrange(DOWN, buff=0.5)

        for flag in flags:
            self.play(Create(flag), run_time=0.8)

        # Importance
        importance = Text(
            "Flags control update behavior during the causal loop",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        importance.to_edge(DOWN, buff=0.8)
        self.play(Write(importance))

        self.wait(2)

        self.export_markers()


class VoxelAnatomySummary(TRDScene):
    """Complete voxel data structure summary."""

    def construct(self):
        self.load_narration("2.2")

        self.add_marker("2.2.6.1", "summary")

        title = self.trd_title("Complete Voxel Structure")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Complete structure diagram
        self.add_marker("2.2.6.2", "diagram")

        box = RoundedRectangle(
            width=8, height=5,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["grid_bright"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.9,
        )

        sections = VGroup()

        # Identity section
        id_header = Text("IDENTITY", color=TRD_COLORS["highlight"], font_size=16, weight="BOLD")
        id_content = Text("position, uuid, partner_uuid", color=TRD_COLORS["text"], font_size=12)
        id_content.next_to(id_header, DOWN, buff=0.1)
        id_section = VGroup(id_header, id_content)

        # State section
        state_header = Text("STATE", color=TRD_COLORS["matter"], font_size=16, weight="BOLD")
        state_content = Text("s ∈ {-1, 0, +1}, charge", color=TRD_COLORS["text"], font_size=12)
        state_content.next_to(state_header, DOWN, buff=0.1)
        state_section = VGroup(state_header, state_content)

        # Flux section
        flux_header = Text("FLUX", color=TRD_COLORS["antimatter"], font_size=16, weight="BOLD")
        flux_content = Text("J ∈ ℝ³, density, frequency", color=TRD_COLORS["text"], font_size=12)
        flux_content.next_to(flux_header, DOWN, buff=0.1)
        flux_section = VGroup(flux_header, flux_content)

        # Mechanical section
        mech_header = Text("MECHANICAL", color=TRD_COLORS["glow"], font_size=16, weight="BOLD")
        mech_content = Text("force, position_rem, wave_vel", color=TRD_COLORS["text"], font_size=12)
        mech_content.next_to(mech_header, DOWN, buff=0.1)
        mech_section = VGroup(mech_header, mech_content)

        # Flags section
        flag_header = Text("FLAGS", color=TRD_COLORS["text_dim"], font_size=16, weight="BOLD")
        flag_content = Text("is_locked, is_active", color=TRD_COLORS["text"], font_size=12)
        flag_content.next_to(flag_header, DOWN, buff=0.1)
        flag_section = VGroup(flag_header, flag_content)

        sections.add(id_section, state_section, flux_section, mech_section, flag_section)
        sections.arrange(DOWN, buff=0.35, aligned_edge=LEFT)
        sections.move_to(box.get_center())

        self.play(Create(box))
        for section in sections:
            self.play(Write(section), run_time=0.5)

        # Final insight
        final = Text(
            "All physics emerges from local updates of these fields",
            color=TRD_COLORS["highlight"],
            font_size=18,
        )
        final.to_edge(DOWN, buff=0.5)
        self.play(Write(final))

        self.wait(2)

        self.export_markers()
