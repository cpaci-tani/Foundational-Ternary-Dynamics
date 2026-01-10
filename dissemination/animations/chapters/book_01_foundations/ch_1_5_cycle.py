"""
Chapter 1.5: The Existence Cycle
================================

Animation of the existence cycle: genesis, persistence, evaporation.
Shows the lifecycle of manifested entities in TRD.
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
    GrowFromCenter,
    ShrinkToCenter,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Arrow,
    CurvedArrow,
    Text,
    MathTex,
    RoundedRectangle,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, PHASE_COLORS
from lib.components import VoxelMobject, VoxelGrid, WavePulse


class ExistenceCycleIntro(TRDScene):
    """Introduction to the existence cycle."""

    def construct(self):
        self.load_narration("1.5")

        self.add_marker("1.5.0.1", "title")
        title = self.trd_title("The Existence Cycle")
        subtitle = Text(
            "Birth, Life, and Death of Manifestation",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Cycle diagram
        self.add_marker("1.5.0.2", "cycle_diagram")

        # Three states in a circle
        radius = 2.0

        # Void (top)
        void_pos = UP * radius
        void_box = RoundedRectangle(
            width=2, height=0.8,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["void_light"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        void_box.move_to(void_pos)
        void_label = Text("VOID", color=TRD_COLORS["void_light"], font_size=20)
        void_label.move_to(void_box.get_center())
        void_eq = MathTex("s = 0", color=TRD_COLORS["text_dim"], font_size=16)
        void_eq.next_to(void_box, UP, buff=0.1)

        # Manifest (bottom left)
        manifest_pos = DOWN * radius * 0.5 + LEFT * radius * 0.866
        manifest_box = RoundedRectangle(
            width=2.2, height=0.8,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["matter"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        manifest_box.move_to(manifest_pos)
        manifest_label = Text("MANIFEST", color=TRD_COLORS["matter"], font_size=20)
        manifest_label.move_to(manifest_box.get_center())
        manifest_eq = MathTex(r"s = \pm 1", color=TRD_COLORS["text_dim"], font_size=16)
        manifest_eq.next_to(manifest_box, DOWN, buff=0.1)

        # Persist (bottom right)
        persist_pos = DOWN * radius * 0.5 + RIGHT * radius * 0.866
        persist_box = RoundedRectangle(
            width=2, height=0.8,
            corner_radius=0.1,
            stroke_color=TRD_COLORS["highlight"],
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.5,
        )
        persist_box.move_to(persist_pos)
        persist_label = Text("PERSIST", color=TRD_COLORS["highlight"], font_size=20)
        persist_label.move_to(persist_box.get_center())
        persist_eq = MathTex(r"|J| > K_B", color=TRD_COLORS["text_dim"], font_size=16)
        persist_eq.next_to(persist_box, DOWN, buff=0.1)

        # Arrows
        arrow1 = CurvedArrow(
            void_box.get_bottom() + DOWN * 0.1 + LEFT * 0.3,
            manifest_box.get_top() + UP * 0.1,
            color=TRD_COLORS["matter"],
            angle=-PI/4,
        )
        arrow1_label = Text("Genesis", color=TRD_COLORS["matter"], font_size=14)
        arrow1_label.next_to(arrow1.get_center(), LEFT, buff=0.1)

        arrow2 = CurvedArrow(
            manifest_box.get_right() + RIGHT * 0.1,
            persist_box.get_left() + LEFT * 0.1,
            color=TRD_COLORS["highlight"],
            angle=-PI/6,
        )
        arrow2_label = Text("Sustain", color=TRD_COLORS["highlight"], font_size=14)
        arrow2_label.next_to(arrow2.get_center(), DOWN, buff=0.1)

        arrow3 = CurvedArrow(
            persist_box.get_top() + UP * 0.1 + RIGHT * 0.3,
            void_box.get_bottom() + DOWN * 0.1 + RIGHT * 0.3,
            color=TRD_COLORS["antimatter"],
            angle=-PI/4,
        )
        arrow3_label = Text("Evaporate", color=TRD_COLORS["antimatter"], font_size=14)
        arrow3_label.next_to(arrow3.get_center(), RIGHT, buff=0.1)

        # Animate
        boxes = VGroup(void_box, void_label, void_eq,
                       manifest_box, manifest_label, manifest_eq,
                       persist_box, persist_label, persist_eq)

        self.play(FadeIn(boxes))
        self.play(Create(arrow1), Write(arrow1_label))
        self.play(Create(arrow2), Write(arrow2_label))
        self.play(Create(arrow3), Write(arrow3_label))

        self.wait(2)

        self.export_markers()


class GenesisPhase(TRDScene):
    """The genesis phase: void to manifest."""

    def construct(self):
        self.load_narration("1.5")

        self.add_marker("1.5.1.1", "genesis")

        title = self.concept_card(
            "Genesis",
            "From void to manifestation"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Void voxel
        self.add_marker("1.5.1.2", "void_state")

        voxel = VoxelMobject(state=0, size=1.5, show_glow=False)
        state_label = MathTex("s = 0", color=TRD_COLORS["void_light"], font_size=28)
        state_label.to_edge(UP, buff=1.0)

        self.play(FadeIn(voxel), Write(state_label))
        self.wait(1)

        # Flux accumulation
        self.add_marker("1.5.1.3", "flux_build")

        flux_text = Text("Flux accumulates...", color=TRD_COLORS["text"], font_size=20)
        flux_text.to_edge(DOWN, buff=1.5)
        self.play(Write(flux_text))

        # Glow intensifies
        pulse = WavePulse(center=ORIGIN, max_radius=2.0, color=TRD_COLORS["highlight"], num_rings=3)
        self.add(pulse)
        self.play(pulse.expand(run_time=2.0))

        # Threshold crossed
        self.add_marker("1.5.1.4", "threshold")

        threshold = MathTex("|J| > K_B", color=TRD_COLORS["highlight"], font_size=32)
        threshold.next_to(voxel, UP, buff=0.5)
        self.play(Write(threshold), FadeOut(flux_text), FadeOut(pulse))

        # Manifest!
        self.add_marker("1.5.1.5", "manifest")

        new_voxel = VoxelMobject(state=+1, size=1.5, show_glow=True)
        new_state = MathTex("s = +1", color=TRD_COLORS["matter"], font_size=28)
        new_state.to_edge(UP, buff=1.0)

        self.play(
            FadeOut(voxel),
            GrowFromCenter(new_voxel),
            FadeOut(state_label),
            Write(new_state),
            FadeOut(threshold),
        )

        self.wait(2)

        self.export_markers()


class PersistencePhase(TRDScene):
    """The persistence phase: maintaining manifestation."""

    def construct(self):
        self.load_narration("1.5")

        self.add_marker("1.5.2.1", "persistence")

        title = self.concept_card(
            "Persistence",
            "Maintaining manifested existence"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Manifested voxel
        self.add_marker("1.5.2.2", "manifest_state")

        voxel = VoxelMobject(state=+1, size=1.2, show_glow=True)
        self.play(FadeIn(voxel))

        # Condition for persistence
        condition = MathTex(
            r"\text{Persist if: } |J(v)| \geq K_B",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        condition.to_edge(UP, buff=1.0)
        self.play(Write(condition))

        # Show stability
        self.add_marker("1.5.2.3", "stable")

        stable_text = Text("Stable configuration", color=TRD_COLORS["highlight"], font_size=20)
        stable_text.to_edge(DOWN, buff=1.5)
        self.play(Write(stable_text))

        # Pulse to show "alive"
        for _ in range(3):
            self.play(voxel.animate.scale(1.1), run_time=0.3)
            self.play(voxel.animate.scale(1/1.1), run_time=0.3)

        # Bound structures persist longer
        self.add_marker("1.5.2.4", "binding")

        binding = Text(
            "Bound structures (triads) are locked against decay",
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        binding.next_to(stable_text, DOWN, buff=0.3)
        self.play(Write(binding))

        self.wait(2)

        self.export_markers()


class EvaporationPhase(TRDScene):
    """The evaporation phase: manifest to void."""

    def construct(self):
        self.load_narration("1.5")

        self.add_marker("1.5.3.1", "evaporation")

        title = self.concept_card(
            "Evaporation",
            "Return to the void"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Start with manifested voxel
        self.add_marker("1.5.3.2", "decay")

        voxel = VoxelMobject(state=+1, size=1.2, show_glow=True)
        state_label = MathTex("s = +1", color=TRD_COLORS["matter"], font_size=28)
        state_label.to_edge(UP, buff=1.0)

        self.play(FadeIn(voxel), Write(state_label))
        self.wait(1)

        # Flux decays
        decay_text = Text("Flux decays over time...", color=TRD_COLORS["text"], font_size=20)
        decay_text.to_edge(DOWN, buff=1.5)
        self.play(Write(decay_text))

        # Show decay equation
        decay_eq = MathTex(
            r"J(t+1) = J(t) \cdot (1 - \gamma)",
            color=TRD_COLORS["antimatter"],
            font_size=24,
        )
        decay_eq.next_to(voxel, DOWN, buff=0.8)
        self.play(Write(decay_eq))

        # Voxel fades
        self.add_marker("1.5.3.3", "fade")

        self.play(
            voxel.animate.scale(0.8).set_opacity(0.7),
            run_time=1.0,
        )
        self.play(
            voxel.animate.scale(0.8).set_opacity(0.4),
            run_time=1.0,
        )

        # Threshold crossed (downward)
        threshold = MathTex("|J| < K_B", color=TRD_COLORS["antimatter"], font_size=28)
        threshold.next_to(voxel, UP, buff=0.3)
        self.play(Write(threshold), FadeOut(decay_text))

        # Evaporate
        self.add_marker("1.5.3.4", "vanish")

        void_voxel = VoxelMobject(state=0, size=1.2, show_glow=False)
        new_state = MathTex("s = 0", color=TRD_COLORS["void_light"], font_size=28)
        new_state.to_edge(UP, buff=1.0)

        self.play(
            ShrinkToCenter(voxel),
            FadeIn(void_voxel),
            FadeOut(state_label),
            Write(new_state),
            FadeOut(threshold),
            FadeOut(decay_eq),
        )

        returned = Text("Returned to void", color=TRD_COLORS["void_light"], font_size=20)
        returned.to_edge(DOWN, buff=1.5)
        self.play(Write(returned))

        self.wait(2)

        self.export_markers()


class ExistenceCycleSummary(TRDScene):
    """Summary of the existence cycle."""

    def construct(self):
        self.load_narration("1.5")

        self.add_marker("1.5.4.1", "summary")

        title = self.trd_title("The Cycle of Existence")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Complete cycle animation
        self.add_marker("1.5.4.2", "animation")

        # Start with void
        voxel = VoxelMobject(state=0, size=1.0, show_glow=False)
        self.play(FadeIn(voxel))

        phase_label = Text("VOID", color=TRD_COLORS["void_light"], font_size=24)
        phase_label.to_edge(DOWN, buff=1.0)
        self.play(Write(phase_label))
        self.wait(0.5)

        # Genesis
        new_voxel = VoxelMobject(state=+1, size=1.0, show_glow=True)
        new_label = Text("GENESIS", color=TRD_COLORS["matter"], font_size=24)
        new_label.to_edge(DOWN, buff=1.0)

        self.play(
            FadeOut(voxel),
            GrowFromCenter(new_voxel),
            FadeOut(phase_label),
            Write(new_label),
        )
        self.wait(0.5)

        # Persistence
        persist_label = Text("PERSISTENCE", color=TRD_COLORS["highlight"], font_size=24)
        persist_label.to_edge(DOWN, buff=1.0)
        self.play(FadeOut(new_label), Write(persist_label))

        # Pulse
        for _ in range(2):
            self.play(new_voxel.animate.scale(1.1), run_time=0.25)
            self.play(new_voxel.animate.scale(1/1.1), run_time=0.25)

        # Evaporation
        evap_label = Text("EVAPORATION", color=TRD_COLORS["antimatter"], font_size=24)
        evap_label.to_edge(DOWN, buff=1.0)
        self.play(FadeOut(persist_label), Write(evap_label))

        void_voxel = VoxelMobject(state=0, size=1.0, show_glow=False)
        self.play(
            ShrinkToCenter(new_voxel),
            FadeIn(void_voxel),
        )

        # Back to void
        final_label = Text("VOID (cycle complete)", color=TRD_COLORS["void_light"], font_size=24)
        final_label.to_edge(DOWN, buff=1.0)
        self.play(FadeOut(evap_label), Write(final_label))

        self.wait(2)

        # Key insight
        insight = self.equation_box(
            r"0 \xrightarrow{|J|>K_B} \pm 1 \xrightarrow{|J|<K_B} 0",
            "Existence is a temporary excitation of the void"
        )
        insight.next_to(final_label, UP, buff=0.5)
        self.play(FadeIn(insight))

        self.wait(2)

        self.export_markers()
