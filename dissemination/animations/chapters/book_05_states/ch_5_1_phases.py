"""
Chapter 5.1: Phases of Matter
=============================

Solid, liquid, gas states in TRD.
Shows how flux configurations determine phase.
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
    Square,
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


class PhasesIntro(TRDScene):
    """Introduction to phases of matter."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.0.1", "title")
        title = self.trd_title("Phases of Matter")
        subtitle = Text(
            "Solid, Liquid, Gas, and Beyond",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class SolidState(TRDScene):
    """Solid state - fixed structure."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.1.1", "solid")

        title = self.concept_card(
            "Solid State",
            "Fixed positions, strong bonds"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Crystal lattice
        self.add_marker("5.1.1.2", "lattice")

        lattice = VGroup()
        rows, cols = 5, 7
        spacing = 0.6

        for i in range(rows):
            for j in range(cols):
                atom = Circle(
                    radius=0.15,
                    fill_color=TRD_COLORS["matter"],
                    fill_opacity=0.8,
                    stroke_width=0,
                )
                x = (j - cols/2 + 0.5) * spacing
                y = (i - rows/2 + 0.5) * spacing
                atom.move_to([x, y, 0])
                lattice.add(atom)

        # Bonds
        bonds = VGroup()
        for i in range(rows):
            for j in range(cols - 1):
                idx = i * cols + j
                bond = Line(
                    lattice[idx].get_center(),
                    lattice[idx + 1].get_center(),
                    color=TRD_COLORS["grid_bright"],
                    stroke_width=1,
                    stroke_opacity=0.5,
                )
                bonds.add(bond)
        for i in range(rows - 1):
            for j in range(cols):
                idx = i * cols + j
                bond = Line(
                    lattice[idx].get_center(),
                    lattice[idx + cols].get_center(),
                    color=TRD_COLORS["grid_bright"],
                    stroke_width=1,
                    stroke_opacity=0.5,
                )
                bonds.add(bond)

        self.play(Create(bonds), Create(lattice))

        # Properties
        props = VGroup()
        p1 = Text("• Fixed shape and volume", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Strong intermolecular forces", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Atoms vibrate in place", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(RIGHT, buff=0.5)

        for p in props:
            self.play(Write(p), run_time=0.4)

        # TRD note
        trd = Text(
            "TRD: Strong flux coupling locks positions",
            color=TRD_COLORS["highlight"],
            font_size=14,
        )
        trd.to_edge(DOWN, buff=0.5)
        self.play(Write(trd))

        self.wait(2)

        self.export_markers()


class LiquidState(TRDScene):
    """Liquid state - flowing structure."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.2.1", "liquid")

        title = self.concept_card(
            "Liquid State",
            "Flowing, takes container shape"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Random-ish arrangement
        self.add_marker("5.1.2.2", "arrangement")

        container = Rectangle(
            width=4, height=3,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=2,
            fill_opacity=0,
        )
        self.play(Create(container))

        # Particles in semi-random positions
        particles = VGroup()
        np.random.seed(42)
        for _ in range(30):
            x = np.random.uniform(-1.8, 1.8)
            y = np.random.uniform(-1.3, 1.3)
            p = Circle(
                radius=0.12,
                fill_color=TRD_COLORS["antimatter"],
                fill_opacity=0.7,
                stroke_width=0,
            )
            p.move_to([x, y, 0])
            particles.add(p)

        self.play(FadeIn(particles))

        # Show slight movement
        self.add_marker("5.1.2.3", "flow")

        # Animate random motion
        for _ in range(2):
            new_positions = []
            for p in particles:
                dx = np.random.uniform(-0.2, 0.2)
                dy = np.random.uniform(-0.2, 0.2)
                new_pos = p.get_center() + np.array([dx, dy, 0])
                # Keep in bounds
                new_pos[0] = np.clip(new_pos[0], -1.8, 1.8)
                new_pos[1] = np.clip(new_pos[1], -1.3, 1.3)
                new_positions.append(new_pos)

            self.play(
                *[p.animate.move_to(pos) for p, pos in zip(particles, new_positions)],
                run_time=0.8,
            )

        # Properties
        props = VGroup()
        p1 = Text("• Fixed volume, variable shape", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Moderate intermolecular forces", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Particles slide past each other", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(RIGHT, buff=0.3)

        for p in props:
            self.play(Write(p), run_time=0.4)

        self.wait(2)

        self.export_markers()


class GasState(TRDScene):
    """Gas state - free particles."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.3.1", "gas")

        title = self.concept_card(
            "Gas State",
            "Free particles, fills container"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Container
        self.add_marker("5.1.3.2", "particles")

        container = Rectangle(
            width=5, height=3.5,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=2,
            fill_opacity=0,
        )
        self.play(Create(container))

        # Sparse particles
        particles = VGroup()
        np.random.seed(123)
        for _ in range(15):
            x = np.random.uniform(-2.3, 2.3)
            y = np.random.uniform(-1.5, 1.5)
            p = Circle(
                radius=0.1,
                fill_color=TRD_COLORS["highlight"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            p.move_to([x, y, 0])
            particles.add(p)

        self.play(FadeIn(particles))

        # Rapid motion
        self.add_marker("5.1.3.3", "motion")

        for _ in range(3):
            new_positions = []
            for p in particles:
                # Larger displacements for gas
                dx = np.random.uniform(-0.5, 0.5)
                dy = np.random.uniform(-0.5, 0.5)
                new_pos = p.get_center() + np.array([dx, dy, 0])
                new_pos[0] = np.clip(new_pos[0], -2.3, 2.3)
                new_pos[1] = np.clip(new_pos[1], -1.5, 1.5)
                new_positions.append(new_pos)

            self.play(
                *[p.animate.move_to(pos) for p, pos in zip(particles, new_positions)],
                run_time=0.5,
            )

        # Properties
        props = VGroup()
        p1 = Text("• Variable shape and volume", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Weak/no intermolecular forces", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• Rapid random motion", color=TRD_COLORS["text"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(RIGHT, buff=0.3)

        for p in props:
            self.play(Write(p), run_time=0.4)

        self.wait(2)

        self.export_markers()


class PlasmaState(TRDScene):
    """Plasma - ionized gas."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.4.1", "plasma")

        title = self.concept_card(
            "Plasma State",
            "Ionized gas - the fourth state"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Plasma visualization
        self.add_marker("5.1.4.2", "ionized")

        # Mixed positive and negative particles
        particles = VGroup()
        np.random.seed(456)
        for i in range(20):
            x = np.random.uniform(-2.5, 2.5)
            y = np.random.uniform(-1.5, 1.5)
            is_positive = i % 2 == 0
            p = Circle(
                radius=0.12 if is_positive else 0.08,
                fill_color=TRD_COLORS["matter"] if is_positive else TRD_COLORS["antimatter"],
                fill_opacity=0.8,
                stroke_width=0,
            )
            p.move_to([x, y, 0])
            particles.add(p)

        self.play(FadeIn(particles))

        # Chaotic motion with glow effect
        for _ in range(2):
            new_positions = []
            for p in particles:
                dx = np.random.uniform(-0.6, 0.6)
                dy = np.random.uniform(-0.6, 0.6)
                new_pos = p.get_center() + np.array([dx, dy, 0])
                new_pos[0] = np.clip(new_pos[0], -2.5, 2.5)
                new_pos[1] = np.clip(new_pos[1], -1.5, 1.5)
                new_positions.append(new_pos)

            self.play(
                *[p.animate.move_to(pos) for p, pos in zip(particles, new_positions)],
                run_time=0.4,
            )

        # Properties
        props = VGroup()
        p1 = Text("• Ionized (electrons stripped)", color=TRD_COLORS["text"], font_size=14)
        p2 = Text("• Conducts electricity", color=TRD_COLORS["text"], font_size=14)
        p3 = Text("• 99% of visible universe", color=TRD_COLORS["highlight"], font_size=14)

        props.add(p1, p2, p3)
        props.arrange(DOWN, buff=0.15, aligned_edge=LEFT)
        props.to_edge(DOWN, buff=0.6)

        for p in props:
            self.play(Write(p), run_time=0.4)

        # Examples
        examples = Text(
            "Examples: Sun, lightning, neon signs",
            color=TRD_COLORS["glow"],
            font_size=14,
        )
        examples.to_edge(UP, buff=0.8)
        self.play(Write(examples))

        self.wait(2)

        self.export_markers()


class PhaseComparison(TRDScene):
    """Comparison of all phases."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.5.1", "comparison")

        title = self.trd_title("Phase Comparison")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.4).scale(0.7))

        # Four phase boxes
        phases = VGroup()

        phase_data = [
            ("Solid", TRD_COLORS["matter"], "Fixed", "Fixed", "Strong"),
            ("Liquid", TRD_COLORS["antimatter"], "Fixed", "Variable", "Moderate"),
            ("Gas", TRD_COLORS["highlight"], "Variable", "Variable", "Weak"),
            ("Plasma", TRD_COLORS["glow"], "Variable", "Variable", "Ionized"),
        ]

        for name, color, vol, shape, forces in phase_data:
            box = RoundedRectangle(
                width=2.8, height=1.8,
                corner_radius=0.1,
                stroke_color=color,
                fill_opacity=0.1,
            )
            name_text = Text(name, color=color, font_size=16, weight="BOLD")
            name_text.move_to(box.get_top() + DOWN * 0.3)

            details = VGroup(
                Text(f"Vol: {vol}", color=TRD_COLORS["text"], font_size=10),
                Text(f"Shape: {shape}", color=TRD_COLORS["text"], font_size=10),
                Text(f"Forces: {forces}", color=TRD_COLORS["text"], font_size=10),
            )
            details.arrange(DOWN, buff=0.08, aligned_edge=LEFT)
            details.move_to(box.get_center() + DOWN * 0.2)

            phases.add(VGroup(box, name_text, details))

        phases.arrange_in_grid(rows=2, cols=2, buff=0.3)
        phases.shift(DOWN * 0.3)

        for phase in phases:
            self.play(Create(phase), run_time=0.5)

        self.wait(2)

        self.export_markers()


class PhasesSummary(TRDScene):
    """Summary of phases."""

    def construct(self):
        self.load_narration("5.1")

        self.add_marker("5.1.6.1", "summary")

        title = self.trd_title("Phases of Matter")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Solid: fixed structure, strong bonds",
            "Liquid: flows, moderate bonds",
            "Gas: free particles, weak bonds",
            "Plasma: ionized, 99% of universe",
            "TRD: Phase = flux coupling strength",
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
            r"\text{Phase} \leftarrow \text{Temperature} + \text{Pressure}",
            "Kinetic energy vs intermolecular forces"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
