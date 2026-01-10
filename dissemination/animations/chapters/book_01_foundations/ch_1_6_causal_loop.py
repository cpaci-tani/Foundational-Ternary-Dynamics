"""
Chapter 1.6: The Causal Loop
============================

Animation of the 13-step TRD update cycle.
Shows the complete tick sequence from time gate to increment.
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
    Indicate,
    Flash,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Text,
    MathTex,
    RoundedRectangle,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS, PHASE_COLORS
from lib.components import (
    CAUSAL_LOOP_STEPS,
    CausalLoopDiagram,
    CausalLoopLegend,
    CausalLoopNode,
)


class CausalLoopIntro(TRDScene):
    """Introduction to the causal loop."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.0.1", "title")
        title = self.trd_title("The Causal Loop")
        subtitle = Text(
            "13 Steps of the TRD Update Cycle",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        # Tick concept
        self.add_marker("1.6.0.2", "tick")

        tick_eq = MathTex(
            r"t \to t + 1",
            color=TRD_COLORS["highlight"],
            font_size=48,
        )
        tick_label = Text(
            "One Tick = Complete Update Cycle",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        tick_label.next_to(tick_eq, DOWN, buff=0.5)

        self.play(Write(tick_eq))
        self.play(Write(tick_label))
        self.wait(2)
        self.play(FadeOut(tick_eq), FadeOut(tick_label))

        self.export_markers()


class LoopOverview(TRDScene):
    """Overview of the complete 13-step loop."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.1.1", "overview")

        # Build the loop diagram
        loop = CausalLoopDiagram(
            radius=2.8,
            node_radius=0.35,
            show_arrows=True,
            show_center=True,
        )
        loop.shift(LEFT * 0.5)

        # Legend
        legend = CausalLoopLegend(position=RIGHT * 4.5)

        # Title
        title = Text(
            "The 13-Step Update Cycle",
            color=TRD_COLORS["text"],
            font_size=28,
            weight="BOLD",
        )
        title.to_edge(UP, buff=0.5)

        self.play(Write(title))
        self.play(Create(loop, run_time=3.0))
        self.play(FadeIn(legend))

        self.wait(2)

        # Highlight each phase group
        self.add_marker("1.6.1.2", "phases")

        phase_names = ["temporal", "existence", "propagation", "forces", "motion"]
        for phase in phase_names:
            color = PHASE_COLORS[phase]
            phase_label = Text(
                phase.upper(),
                color=color,
                font_size=24,
                weight="BOLD",
            )
            phase_label.to_edge(DOWN, buff=0.8)
            self.play(Write(phase_label))

            # Find and flash all nodes of this phase
            for i, step in enumerate(CAUSAL_LOOP_STEPS):
                if step["phase"] == phase:
                    node = loop.get_node(i)
                    if node:
                        self.play(
                            Flash(node, color=color, flash_radius=0.5),
                            run_time=0.3,
                        )

            self.play(FadeOut(phase_label))

        self.wait(1)
        self.play(FadeOut(loop), FadeOut(legend), FadeOut(title))

        self.export_markers()


class StepByStep(TRDScene):
    """Detailed walkthrough of each step."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.2.1", "steps")

        # Create loop
        loop = CausalLoopDiagram(radius=2.5, node_radius=0.3)
        loop.shift(LEFT * 2)
        self.play(Create(loop, run_time=2.0))

        # Step detail panel
        detail_box = RoundedRectangle(
            width=4, height=3,
            corner_radius=0.2,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=1,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.8,
        )
        detail_box.shift(RIGHT * 3)

        self.play(Create(detail_box))

        # Walk through each step
        for i, step in enumerate(CAUSAL_LOOP_STEPS):
            self.add_marker(f"1.6.2.{i+2}", step["name"].lower().replace(" ", "_"))

            # Highlight current step
            self.play(loop.highlight_step(i, run_time=0.3))

            # Update detail panel
            color = PHASE_COLORS[step["phase"]]

            step_num = Text(
                f"Step {i+1}",
                color=TRD_COLORS["text_dim"],
                font_size=16,
            )
            step_num.move_to(detail_box.get_top() + DOWN * 0.4)

            step_name = Text(
                step["name"],
                color=color,
                font_size=24,
                weight="BOLD",
            )
            step_name.next_to(step_num, DOWN, buff=0.2)

            step_desc = Text(
                step["description"],
                color=TRD_COLORS["text"],
                font_size=18,
            )
            step_desc.next_to(step_name, DOWN, buff=0.3)

            phase_label = Text(
                f"Phase: {step['phase'].title()}",
                color=color,
                font_size=14,
            )
            phase_label.next_to(step_desc, DOWN, buff=0.3)

            detail = VGroup(step_num, step_name, step_desc, phase_label)

            if i == 0:
                self.play(Write(detail))
            else:
                self.play(
                    FadeOut(prev_detail),
                    Write(detail),
                    run_time=0.5,
                )

            prev_detail = detail
            self.wait(0.8)

        self.play(FadeOut(prev_detail))

        # Complete cycle animation
        self.add_marker("1.6.2.15", "full_cycle")

        cycle_text = Text(
            "Complete Cycle",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        cycle_text.move_to(detail_box.get_center())
        self.play(Write(cycle_text))

        self.play(loop.animate_full_cycle(duration=4.0))

        self.wait(1)
        self.play(FadeOut(loop), FadeOut(detail_box), FadeOut(cycle_text))

        self.export_markers()


class PhaseGroups(TRDScene):
    """Explain the five phase groups."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.3.1", "phase_groups")

        title = self.trd_title("The Five Phases")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Phase definitions
        phases = [
            ("TEMPORAL", "temporal", "Time gating and increment", ["TIME GATE", "INCREMENT"]),
            ("EXISTENCE", "existence", "Decay and state transitions", ["DECAY", "EXISTENCE"]),
            ("PROPAGATION", "propagation", "Wave and field dynamics", ["PROPAGATE", "SUPERPOSE", "FIELDS"]),
            ("FORCES", "forces", "Force calculation and integration", ["FORCES", "INTEGRATE"]),
            ("MOTION", "motion", "Movement and interactions", ["MOVE", "COLLIDE", "TRANSMUTE", "BIND"]),
        ]

        y_offset = 1.5
        for name, phase, description, steps in phases:
            color = PHASE_COLORS[phase]

            # Phase name
            name_text = Text(name, color=color, font_size=24, weight="BOLD")
            name_text.move_to(LEFT * 4 + UP * y_offset)

            # Description
            desc_text = Text(description, color=TRD_COLORS["text"], font_size=18)
            desc_text.next_to(name_text, RIGHT, buff=0.3)

            # Steps
            steps_text = Text(
                " → ".join(steps),
                color=TRD_COLORS["text_dim"],
                font_size=14,
            )
            steps_text.next_to(name_text, DOWN, buff=0.15, aligned_edge=LEFT)

            group = VGroup(name_text, desc_text, steps_text)
            self.play(Write(group), run_time=0.8)

            y_offset -= 0.9

        self.wait(2)

        self.export_markers()


class CausalLoopEquations(TRDScene):
    """Key equations for each phase."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.4.1", "equations")

        title = Text(
            "Phase Equations",
            color=TRD_COLORS["text"],
            font_size=28,
            weight="BOLD",
        )
        title.to_edge(UP, buff=0.5)
        self.play(Write(title))

        # Key equations
        equations = [
            (
                "Existence",
                "existence",
                r"s(v) \to \pm 1 \text{ if } |J| > K_B"
            ),
            (
                "Propagation",
                "propagation",
                r"\partial_t^2 J = c^2 \nabla^2 J"
            ),
            (
                "Forces",
                "forces",
                r"F = F_{grav} + F_{em} + F_{strong} + F_{weak}"
            ),
            (
                "Motion",
                "motion",
                r"\dot{x} = v, \quad \dot{v} = F/m"
            ),
        ]

        eq_mobs = VGroup()
        for name, phase, eq_str in equations:
            color = PHASE_COLORS[phase]

            label = Text(name + ":", color=color, font_size=20, weight="BOLD")
            eq = MathTex(eq_str, color=TRD_COLORS["text"], font_size=24)
            eq.next_to(label, RIGHT, buff=0.3)

            group = VGroup(label, eq)
            eq_mobs.add(group)

        eq_mobs.arrange(DOWN, buff=0.5, aligned_edge=LEFT)
        eq_mobs.center()

        for eq in eq_mobs:
            self.play(Write(eq), run_time=0.8)

        self.wait(2)

        self.export_markers()


class CausalLoopSummary(TRDScene):
    """Summary of the causal loop."""

    def construct(self):
        self.load_narration("1.6")

        self.add_marker("1.6.5.1", "summary")

        # Final loop animation
        loop = CausalLoopDiagram(radius=2.2, node_radius=0.28)

        self.play(Create(loop, run_time=2.0))

        # Summary text
        summary_points = [
            "13 ordered steps per tick",
            "5 functional phases",
            "Strictly local (26 neighbors)",
            "Deterministic evolution",
            "Complete in one Planck time",
        ]

        summary = VGroup()
        for point in summary_points:
            bullet = Text("•", color=TRD_COLORS["highlight"], font_size=20)
            text = Text(point, color=TRD_COLORS["text"], font_size=18)
            text.next_to(bullet, RIGHT, buff=0.1)
            summary.add(VGroup(bullet, text))

        summary.arrange(DOWN, buff=0.25, aligned_edge=LEFT)
        summary.to_edge(RIGHT, buff=0.8)

        self.play(Write(summary))

        # Animate multiple cycles
        self.play(loop.animate_full_cycle(duration=6.0))

        # Final note
        final = Text(
            "The heartbeat of reality",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        final.to_edge(DOWN, buff=0.8)
        self.play(Write(final))

        self.wait(2)

        self.export_markers()
