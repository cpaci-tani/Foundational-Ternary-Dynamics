"""
Chapter 3.3: Electron Dynamics
==============================

Electron behavior in TRD atoms.
Shows orbital structure, transitions, and emission.
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
    Rotate,
    AnimationGroup,
    Succession,
    VGroup,
    Circle,
    Dot,
    Line,
    Arrow,
    Arc,
    Text,
    MathTex,
    RoundedRectangle,
    Annulus,
)

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from lib.trd_scene import TRDScene
from lib.colors import TRD_COLORS
from lib.components import WavePulse


class ElectronDynamicsIntro(TRDScene):
    """Introduction to electron dynamics."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.0.1", "title")
        title = self.trd_title("Electron Dynamics")
        subtitle = Text(
            "Orbital Structure and Transitions",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        subtitle.next_to(title, DOWN, buff=0.3)

        self.play(FadeIn(title), FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

        self.export_markers()


class OrbitalConcept(TRDScene):
    """What orbitals mean in TRD."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.1.1", "orbital")

        title = self.concept_card(
            "Electron Orbitals",
            "Standing waves in the flux field"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Classical vs TRD view
        self.add_marker("3.3.1.2", "comparison")

        # Classical view
        classical = VGroup()
        cl_label = Text("Classical view:", color=TRD_COLORS["text_dim"], font_size=16)
        cl_label.shift(UP * 2 + LEFT * 3)

        nucleus_cl = Dot(ORIGIN + LEFT * 3 + UP * 0.5, radius=0.15, color=TRD_COLORS["matter"])
        electron_cl = Dot(ORIGIN + LEFT * 2 + UP * 0.5, radius=0.1, color=TRD_COLORS["antimatter"])

        orbit_cl = Circle(radius=1.0, stroke_color=TRD_COLORS["text_dim"], stroke_width=1)
        orbit_cl.move_to(nucleus_cl.get_center())

        classical.add(cl_label, nucleus_cl, orbit_cl, electron_cl)

        self.play(Write(cl_label), Create(nucleus_cl), Create(orbit_cl), Create(electron_cl))
        self.play(Rotate(electron_cl, angle=TAU, about_point=nucleus_cl.get_center()), run_time=2)

        # TRD view
        trd = VGroup()
        trd_label = Text("TRD view:", color=TRD_COLORS["highlight"], font_size=16)
        trd_label.shift(UP * 2 + RIGHT * 2)

        nucleus_trd = Dot(ORIGIN + RIGHT * 2 + UP * 0.5, radius=0.15, color=TRD_COLORS["matter"])

        # Probability cloud
        cloud = Annulus(
            inner_radius=0.3,
            outer_radius=1.2,
            fill_color=TRD_COLORS["antimatter"],
            fill_opacity=0.3,
            stroke_width=0,
        )
        cloud.move_to(nucleus_trd.get_center())

        trd.add(trd_label, nucleus_trd, cloud)

        self.play(Write(trd_label), Create(nucleus_trd), FadeIn(cloud))

        # Explanation
        explanation = VGroup()
        e1 = Text("Not a point orbiting", color=TRD_COLORS["text_dim"], font_size=14)
        e2 = Text("but flux distributed in space", color=TRD_COLORS["highlight"], font_size=14)

        explanation.add(e1, e2)
        explanation.arrange(DOWN, buff=0.1)
        explanation.to_edge(DOWN, buff=1.0)

        self.play(Write(e1), Write(e2))

        self.wait(2)

        self.export_markers()


class ShellRadii(TRDScene):
    """Discrete shell radii from standing waves."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.2.1", "radii")

        title = self.concept_card(
            "Shell Radii",
            "Quantized from standing wave condition"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Nucleus
        nucleus = Dot(ORIGIN, radius=0.2, color=TRD_COLORS["matter"])
        self.play(Create(nucleus))

        # Shells with radii proportional to n²
        self.add_marker("3.3.2.2", "shells")

        shells = VGroup()
        shell_info = [
            (1, 0.6, "n=1"),
            (2, 1.2, "n=2"),
            (3, 2.0, "n=3"),
        ]

        for n, radius, label_text in shell_info:
            shell = Circle(
                radius=radius,
                stroke_color=TRD_COLORS["antimatter"],
                stroke_width=2,
                stroke_opacity=0.7,
            )

            label = MathTex(label_text, color=TRD_COLORS["antimatter"], font_size=16)
            label.next_to(shell, RIGHT, buff=0.1)

            shells.add(VGroup(shell, label))
            self.play(Create(shell), Write(label), run_time=0.6)

        # Radius formula
        formula = MathTex(
            r"r_n = n^2 \cdot a_0",
            color=TRD_COLORS["highlight"],
            font_size=28,
        )
        formula.to_edge(DOWN, buff=1.5)
        self.play(Write(formula))

        bohr_note = MathTex(
            r"a_0 = \frac{4\pi\epsilon_0\hbar^2}{m_e e^2} \approx 0.53 \text{ Å}",
            color=TRD_COLORS["text_dim"],
            font_size=20,
        )
        bohr_note.next_to(formula, DOWN, buff=0.3)
        self.play(Write(bohr_note))

        self.wait(2)

        self.export_markers()


class ElectronTransitions(TRDScene):
    """Electron transitions between shells."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.3.1", "transitions")

        title = self.concept_card(
            "Electron Transitions",
            "Jumping between energy levels"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Energy levels
        self.add_marker("3.3.3.2", "levels")

        nucleus = Dot(LEFT * 2, radius=0.15, color=TRD_COLORS["matter"])
        self.play(Create(nucleus))

        shells = VGroup()
        radii = [0.5, 1.0, 1.6]
        for r in radii:
            shell = Circle(radius=r, stroke_color=TRD_COLORS["grid_bright"], stroke_width=1)
            shell.move_to(nucleus.get_center())
            shells.add(shell)

        self.play(Create(shells))

        # Electron in outer shell
        electron = Dot(
            nucleus.get_center() + RIGHT * 1.6,
            radius=0.12,
            color=TRD_COLORS["antimatter"],
        )
        self.play(Create(electron))

        # Transition down
        self.add_marker("3.3.3.3", "emission")

        down_label = Text("Emission: electron drops down", color=TRD_COLORS["highlight"], font_size=16)
        down_label.to_edge(UP, buff=0.8)
        self.play(Write(down_label))

        # Move electron to inner shell
        self.play(electron.animate.move_to(nucleus.get_center() + RIGHT * 0.5))

        # Emit photon
        photon = WavePulse(
            center=electron.get_center(),
            max_radius=2.0,
            color=TRD_COLORS["glow"],
            num_rings=3,
        )
        self.add(photon)
        self.play(photon.expand(run_time=1.5))

        # Energy equation
        energy_eq = MathTex(
            r"\Delta E = E_{high} - E_{low} = h\nu",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        energy_eq.to_edge(DOWN, buff=1.0)
        self.play(Write(energy_eq))

        self.wait(1)

        # Absorption (reverse)
        self.add_marker("3.3.3.4", "absorption")

        self.play(FadeOut(down_label), FadeOut(photon))

        up_label = Text("Absorption: electron jumps up", color=TRD_COLORS["antimatter"], font_size=16)
        up_label.to_edge(UP, buff=0.8)
        self.play(Write(up_label))

        # Incoming photon
        incoming = Arrow(
            start=RIGHT * 3,
            end=electron.get_center() + RIGHT * 0.3,
            color=TRD_COLORS["glow"],
            stroke_width=3,
        )
        self.play(Create(incoming))
        self.play(FadeOut(incoming))

        # Electron jumps up
        self.play(electron.animate.move_to(nucleus.get_center() + RIGHT * 1.6))

        self.wait(2)

        self.export_markers()


class SpectralLines(TRDScene):
    """Discrete spectral lines from transitions."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.4.1", "spectrum")

        title = self.concept_card(
            "Spectral Lines",
            "Fingerprints of atomic structure"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # Energy level diagram
        self.add_marker("3.3.4.2", "diagram")

        levels = VGroup()
        level_data = [
            (1, "n=1 (ground)", -3),
            (2, "n=2", -1.5),
            (3, "n=3", -0.8),
            (4, "n=∞", 0),
        ]

        for _, label_text, y_pos in level_data:
            line = Line(LEFT * 2, RIGHT * 2, color=TRD_COLORS["grid_bright"], stroke_width=2)
            line.shift(UP * y_pos + LEFT * 2)
            label = Text(label_text, color=TRD_COLORS["text"], font_size=12)
            label.next_to(line, LEFT, buff=0.1)
            levels.add(VGroup(line, label))

        self.play(Create(levels))

        # Transitions as arrows
        self.add_marker("3.3.4.3", "transitions")

        transitions = [
            ((-3, -1.5), "#ff4444", "Lyman α"),
            ((-3, -0.8), "#ff8844", "Lyman β"),
            ((-1.5, -0.8), "#44ff44", "Balmer α"),
        ]

        arrows = VGroup()
        for (y1, y2), color, name in transitions:
            arrow = Arrow(
                start=LEFT * 2 + UP * y2 + RIGHT * 0.5,
                end=LEFT * 2 + UP * y1 + RIGHT * 0.5,
                color=color,
                stroke_width=3,
                buff=0.1,
            )
            label = Text(name, color=color, font_size=10)
            label.next_to(arrow, RIGHT, buff=0.1)
            arrows.add(VGroup(arrow, label))

        for arr in arrows:
            self.play(Create(arr), run_time=0.5)

        # Spectrum bar
        self.add_marker("3.3.4.4", "bar")

        spectrum_bar = Rectangle(
            width=6, height=0.8,
            fill_color=TRD_COLORS["background"],
            fill_opacity=1,
            stroke_color=TRD_COLORS["grid_bright"],
        )
        spectrum_bar.shift(RIGHT * 2.5)

        self.play(Create(spectrum_bar))

        # Spectral lines
        line_positions = [-2.5, -1.5, 0, 1.5]
        line_colors = ["#ff4444", "#ff8844", "#44ff44", "#4444ff"]

        for pos, color in zip(line_positions, line_colors):
            spec_line = Line(
                spectrum_bar.get_center() + UP * 0.35 + RIGHT * pos,
                spectrum_bar.get_center() + DOWN * 0.35 + RIGHT * pos,
                color=color,
                stroke_width=4,
            )
            self.play(Create(spec_line), run_time=0.3)

        # Note
        note = Text(
            "Each element has unique spectral signature",
            color=TRD_COLORS["highlight"],
            font_size=16,
        )
        note.to_edge(DOWN, buff=0.5)
        self.play(Write(note))

        self.wait(2)

        self.export_markers()


class TRDOrbitalPicture(TRDScene):
    """TRD interpretation of orbitals."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.5.1", "trd_orbitals")

        title = self.concept_card(
            "TRD Orbital Picture",
            "Flux standing waves around nucleus"
        )
        self.play(FadeIn(title))
        self.wait(1)
        self.play(FadeOut(title))

        # TRD description
        self.add_marker("3.3.5.2", "flux")

        description = VGroup()
        d1 = Text("In TRD, electron 'orbitals' are:", color=TRD_COLORS["text"], font_size=18)
        d2 = Text("• Standing waves in the flux field", color=TRD_COLORS["highlight"], font_size=16)
        d3 = Text("• Concentrated around nucleus", color=TRD_COLORS["highlight"], font_size=16)
        d4 = Text("• Quantized by boundary conditions", color=TRD_COLORS["highlight"], font_size=16)

        description.add(d1, d2, d3, d4)
        description.arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        description.shift(UP * 1.5 + LEFT * 1)

        for d in description:
            self.play(Write(d), run_time=0.5)

        # Visualization
        self.add_marker("3.3.5.3", "visual")

        nucleus = Dot(DOWN * 1.5, radius=0.2, color=TRD_COLORS["matter"])
        self.play(Create(nucleus))

        # Flux distribution (multiple rings with varying opacity)
        rings = VGroup()
        for i in range(8):
            r = 0.4 + i * 0.25
            opacity = 0.4 * np.exp(-0.5 * i)  # Decreasing opacity
            ring = Circle(
                radius=r,
                stroke_color=TRD_COLORS["antimatter"],
                stroke_width=2,
                stroke_opacity=opacity,
            )
            ring.move_to(nucleus.get_center())
            rings.add(ring)

        self.play(Create(rings, run_time=2.0))

        # Wave function connection
        connection = MathTex(
            r"\psi(r) = J_x(r) + i J_y(r)",
            color=TRD_COLORS["highlight"],
            font_size=24,
        )
        connection.to_edge(DOWN, buff=0.5)
        self.play(Write(connection))

        self.wait(2)

        self.export_markers()


class ElectronDynamicsSummary(TRDScene):
    """Summary of electron dynamics."""

    def construct(self):
        self.load_narration("3.3")

        self.add_marker("3.3.6.1", "summary")

        title = self.trd_title("Electron Dynamics")
        self.play(FadeIn(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP, buff=0.5).scale(0.7))

        # Key points
        points = [
            "Orbitals = flux standing waves",
            "Shell radii: r_n = n² × a₀",
            "Transitions emit/absorb photons",
            "ΔE = hν (quantized energy)",
            "Spectral lines = atomic fingerprint",
            "Wave function from complexified flux",
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

        # Final insight
        final = self.equation_box(
            r"|\psi|^2 \propto \text{electron probability density}",
            "Born rule from TRD manifestation statistics"
        )
        final.to_edge(DOWN, buff=0.4)
        self.play(FadeIn(final))

        self.wait(2)

        self.export_markers()
