"""
Simulation 6: Gauge Symmetry Emergence - U(1) × SU(2) × SU(3)
=============================================================
A 150-second Manim animation showing how the Standard Model gauge group
emerges from lattice constraints.

Storyboard:
1. (0-30s)   Helmholtz decomposition: J = J_T + J_L visualized
2. (30-60s)  Gauss constraint freezes longitudinal → 2 transverse modes (photon)
3. (60-90s)  Ternary states as SU(2) triangle rotations
4. (90-120s) Three spatial axes → SU(3) color (R, G, B alignment)
5. (120-150s) Product structure tree: U(1) × SU(2) × SU(3)

Run with: manim -pql scene_06_gauge_emergence.py GaugeEmergenceScene
For high quality: manim -pqh scene_06_gauge_emergence.py GaugeEmergenceScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors
BACKGROUND = "#0D1117"
U1_COLOR = "#3498DB"      # Blue for U(1) / EM
SU2_COLOR = "#9B59B6"     # Purple for SU(2) / Weak
SU3_COLOR = "#E67E22"     # Orange for SU(3) / Strong
FLUX_COLOR = "#FFD700"    # Gold for flux
HIGHLIGHT = "#F39C12"

# Color charge colors
RED_CHARGE = "#E74C3C"
GREEN_CHARGE = "#2ECC71"
BLUE_CHARGE = "#3498DB"


class GaugeEmergenceScene(Scene):
    """How the Standard Model gauge group emerges from FTD."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # INTRO
        # =====================================================================

        title = Text("Gauge Symmetry Emergence", font_size=42, color=HIGHLIGHT)
        subtitle = MathTex(r"U(1) \times SU(2) \times SU(3)", font_size=36)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=2)
        self.play(Write(subtitle), run_time=1)
        self.wait(1)
        self.play(FadeOut(title_group), run_time=1)

        # =====================================================================
        # SCENE 1: Helmholtz Decomposition (0-30s)
        # =====================================================================

        scene_title = Text("1. Helmholtz Decomposition", font_size=28, color=U1_COLOR)
        scene_title.to_corner(UL, buff=0.5)
        self.play(Write(scene_title), run_time=1)

        # The decomposition equation
        decomp_eq = MathTex(
            r"\mathbf{J} = \mathbf{J}_T + \mathbf{J}_L",
            font_size=42
        )
        decomp_eq.to_edge(UP, buff=1)
        self.play(Write(decomp_eq), run_time=1)

        # Explanation
        trans_def = MathTex(
            r"\mathbf{J}_T: \nabla \cdot \mathbf{J}_T = 0",
            font_size=28,
            color=U1_COLOR
        )
        long_def = MathTex(
            r"\mathbf{J}_L: \nabla \times \mathbf{J}_L = 0 \;\Rightarrow\; \mathbf{J}_L = \nabla\phi",
            font_size=28,
            color=GRAY_B
        )
        defs = VGroup(trans_def, long_def).arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        defs.next_to(decomp_eq, DOWN, buff=0.5)
        self.play(Write(defs), run_time=2)

        # Visual representation
        # Create a central point with two types of arrows
        center = Dot(ORIGIN, color=WHITE)

        # Transverse component - circular/rotational
        trans_arrows = VGroup()
        for angle in np.linspace(0, TAU, 8, endpoint=False):
            # Arrows tangent to circle
            start = 1.5 * np.array([np.cos(angle), np.sin(angle), 0])
            tangent = np.array([-np.sin(angle), np.cos(angle), 0])
            arrow = Arrow(
                start, start + 0.5 * tangent,
                color=U1_COLOR, stroke_width=3, buff=0
            )
            trans_arrows.add(arrow)

        trans_label = Text("Transverse J_T", font_size=20, color=U1_COLOR)
        trans_label.next_to(trans_arrows, DOWN, buff=0.3)

        # Longitudinal component - radial
        long_arrows = VGroup()
        for angle in np.linspace(0, TAU, 8, endpoint=False):
            start = 0.5 * np.array([np.cos(angle), np.sin(angle), 0])
            end = 1.2 * np.array([np.cos(angle), np.sin(angle), 0])
            arrow = Arrow(start, end, color=GRAY_B, stroke_width=2, buff=0)
            long_arrows.add(arrow)

        long_label = Text("Longitudinal J_L", font_size=20, color=GRAY_B)
        long_label.next_to(long_arrows, UP, buff=0.3)

        # Position the visual
        visual = VGroup(center, trans_arrows, long_arrows, trans_label, long_label)
        visual.shift(DOWN * 1.5)

        self.play(
            FadeIn(center),
            *[GrowArrow(a) for a in trans_arrows],
            *[GrowArrow(a) for a in long_arrows],
            run_time=2
        )
        self.play(Write(trans_label), Write(long_label), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 2: Gauss Constraint → U(1) (30-60s)
        # =====================================================================

        self.play(
            FadeOut(scene_title),
            FadeOut(decomp_eq),
            FadeOut(defs),
            FadeOut(visual)
        )

        scene_title = Text("2. U(1) from Gauss Constraint", font_size=28, color=U1_COLOR)
        scene_title.to_corner(UL, buff=0.5)
        self.play(Write(scene_title), run_time=1)

        # Gauss's law
        gauss = MathTex(
            r"\nabla \cdot \mathbf{J}_L = \rho_{\text{charge}}",
            font_size=36
        )
        gauss.to_edge(UP, buff=1)
        self.play(Write(gauss), run_time=1)

        # Explanation
        constraint_text = Text(
            "The longitudinal component is constrained by charge distribution",
            font_size=22, color=WHITE
        )
        constraint_text.next_to(gauss, DOWN, buff=0.3)
        self.play(Write(constraint_text), run_time=1)

        # Degree of freedom counting
        dof_box = VGroup(
            Text("Degree of Freedom Counting:", font_size=24, color=HIGHLIGHT),
            MathTex(r"\mathbf{J} \text{ has 3 components}", font_size=28),
            MathTex(r"- 1 \text{ constraint (Gauss law)}", font_size=28),
            MathTex(r"= 2 \text{ physical (transverse) modes}", font_size=28, color=U1_COLOR),
        ).arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        dof_box.shift(DOWN * 0.5)

        for item in dof_box:
            self.play(Write(item), run_time=0.8)

        # Result
        result = MathTex(
            r"\Rightarrow \text{2 photon polarizations!}",
            font_size=32,
            color=U1_COLOR
        )
        result.next_to(dof_box, DOWN, buff=0.5)
        self.play(Write(result), run_time=1)

        # U(1) label
        u1_label = MathTex(r"U(1)_{\text{EM}}", font_size=48, color=U1_COLOR)
        u1_label.to_corner(DR, buff=1)
        self.play(Write(u1_label), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 3: SU(2) from Ternary States (60-90s)
        # =====================================================================

        self.play(
            FadeOut(scene_title),
            FadeOut(gauss),
            FadeOut(constraint_text),
            FadeOut(dof_box),
            FadeOut(result),
            u1_label.animate.scale(0.7).to_edge(LEFT, buff=0.5).shift(DOWN * 2)
        )

        scene_title = Text("3. SU(2) from Ternary States", font_size=28, color=SU2_COLOR)
        scene_title.to_corner(UL, buff=0.5)
        self.play(Write(scene_title), run_time=1)

        # Ternary states
        states_eq = MathTex(
            r"s \in \{-1, 0, +1\}",
            font_size=36
        )
        states_eq.to_edge(UP, buff=1)
        self.play(Write(states_eq), run_time=1)

        # Triangle representation
        triangle = RegularPolygon(n=3, radius=2, color=SU2_COLOR, stroke_width=3)
        triangle.shift(DOWN * 0.5)

        # Vertices
        vertices = triangle.get_vertices()
        state_labels = VGroup(
            MathTex("+1", font_size=28, color=MATTER_COLOR if 'MATTER_COLOR' in dir() else "#DD4444"),
            MathTex("0", font_size=28, color=GRAY_B),
            MathTex("-1", font_size=28, color=ANTIMATTER_COLOR if 'ANTIMATTER_COLOR' in dir() else "#4488DD"),
        )

        for label, vertex in zip(state_labels, vertices):
            label.next_to(vertex, vertex - triangle.get_center(), buff=0.3)

        self.play(Create(triangle), run_time=1)
        self.play(*[Write(l) for l in state_labels], run_time=1)

        # Rotation animation
        rotation_text = Text("SU(2) rotations in state space", font_size=22, color=SU2_COLOR)
        rotation_text.next_to(triangle, DOWN, buff=0.8)
        self.play(Write(rotation_text), run_time=1)

        # Animate rotation
        for _ in range(2):
            self.play(
                Rotate(triangle, angle=TAU/3, about_point=triangle.get_center()),
                run_time=1
            )

        # Weak isospin connection
        weak_text = MathTex(
            r"\text{Weak Isospin: } (e^-, \nu_e), (u, d), ...",
            font_size=24,
            color=WHITE
        )
        weak_text.next_to(rotation_text, DOWN, buff=0.3)
        self.play(Write(weak_text), run_time=1)

        # SU(2) label
        su2_label = MathTex(r"SU(2)_L", font_size=48, color=SU2_COLOR)
        su2_label.next_to(u1_label, RIGHT, buff=1)
        self.play(Write(su2_label), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 4: SU(3) from Spatial Dimensions (90-120s)
        # =====================================================================

        self.play(
            FadeOut(scene_title),
            FadeOut(states_eq),
            FadeOut(triangle),
            FadeOut(state_labels),
            FadeOut(rotation_text),
            FadeOut(weak_text),
        )

        scene_title = Text("4. SU(3) from Spatial Dimensions", font_size=28, color=SU3_COLOR)
        scene_title.to_corner(UL, buff=0.5)
        self.play(Write(scene_title), run_time=1)

        # 3D axes
        axes_text = Text("3 spatial dimensions of the lattice", font_size=24)
        axes_text.to_edge(UP, buff=1)
        self.play(Write(axes_text), run_time=1)

        # Create 3D-like axes
        origin = ORIGIN + DOWN * 0.5
        x_axis = Arrow(origin, origin + RIGHT * 2.5, color=RED_CHARGE, stroke_width=4)
        y_axis = Arrow(origin, origin + UP * 2.5, color=GREEN_CHARGE, stroke_width=4)
        z_axis = Arrow(origin, origin + (UP + LEFT) * 1.5, color=BLUE_CHARGE, stroke_width=4)

        x_label = Text("X (Red)", font_size=20, color=RED_CHARGE).next_to(x_axis, RIGHT)
        y_label = Text("Y (Green)", font_size=20, color=GREEN_CHARGE).next_to(y_axis, UP)
        z_label = Text("Z (Blue)", font_size=20, color=BLUE_CHARGE).next_to(z_axis, LEFT)

        self.play(
            GrowArrow(x_axis), Write(x_label),
            GrowArrow(y_axis), Write(y_label),
            GrowArrow(z_axis), Write(z_label),
            run_time=2
        )

        # Color charge explanation
        color_text = VGroup(
            Text("Quark 'color' = primary flux axis alignment", font_size=22, color=WHITE),
            MathTex(r"\text{Red: } J_x \gg J_y, J_z", font_size=24, color=RED_CHARGE),
            MathTex(r"\text{Green: } J_y \gg J_x, J_z", font_size=24, color=GREEN_CHARGE),
            MathTex(r"\text{Blue: } J_z \gg J_x, J_y", font_size=24, color=BLUE_CHARGE),
        ).arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        color_text.to_edge(RIGHT, buff=0.5)

        self.play(Write(color_text), run_time=2)

        # Color neutrality
        neutral = MathTex(
            r"R + G + B = \text{White (neutral)}",
            font_size=24
        )
        neutral.next_to(color_text, DOWN, buff=0.5)
        self.play(Write(neutral), run_time=1)

        # Confinement
        confine = Text("Confinement: Only color-neutral states observed", font_size=20, color=GRAY_B)
        confine.to_edge(DOWN, buff=0.5)
        self.play(Write(confine), run_time=1)

        # SU(3) label
        su3_label = MathTex(r"SU(3)_C", font_size=48, color=SU3_COLOR)
        su3_label.next_to(su2_label, RIGHT, buff=1)
        self.play(Write(su3_label), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 5: The Complete Gauge Group (120-150s)
        # =====================================================================

        self.play(
            FadeOut(scene_title),
            FadeOut(axes_text),
            FadeOut(x_axis), FadeOut(y_axis), FadeOut(z_axis),
            FadeOut(x_label), FadeOut(y_label), FadeOut(z_label),
            FadeOut(color_text),
            FadeOut(neutral),
            FadeOut(confine),
        )

        # Move gauge labels to center
        self.play(
            u1_label.animate.move_to(UP * 2 + LEFT * 3),
            su2_label.animate.move_to(UP * 2),
            su3_label.animate.move_to(UP * 2 + RIGHT * 3),
            run_time=1
        )

        # Add multiplication signs
        times1 = MathTex(r"\times", font_size=36)
        times1.move_to((u1_label.get_center() + su2_label.get_center()) / 2)
        times2 = MathTex(r"\times", font_size=36)
        times2.move_to((su2_label.get_center() + su3_label.get_center()) / 2)

        self.play(Write(times1), Write(times2), run_time=0.5)

        # Title
        final_title = Text("The Standard Model Gauge Group", font_size=32, color=HIGHLIGHT)
        final_title.to_edge(UP, buff=0.5)
        self.play(Write(final_title), run_time=1)

        # Origin tree
        tree_items = VGroup(
            self.create_origin_box("U(1)", "Gauss constraint", "2 photon modes", U1_COLOR),
            self.create_origin_box("SU(2)", "Ternary states", "Weak isospin", SU2_COLOR),
            self.create_origin_box("SU(3)", "3D lattice", "Color charge", SU3_COLOR),
        ).arrange(RIGHT, buff=0.5)
        tree_items.shift(DOWN * 0.5)

        self.play(FadeIn(tree_items), run_time=2)

        # Final equation
        final_eq = MathTex(
            r"G_{\text{SM}} = U(1)_Y \times SU(2)_L \times SU(3)_C",
            font_size=36,
            color=HIGHLIGHT
        )
        final_eq.to_edge(DOWN, buff=1)
        self.play(Write(final_eq), run_time=2)

        # Highlight
        self.play(
            Flash(final_eq, color=HIGHLIGHT, flash_radius=0.5),
            run_time=1
        )

        self.wait(2)

        # Fade out
        self.play(*[FadeOut(mob) for mob in self.mobjects], run_time=2)

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=32, color=HIGHLIGHT),
            MathTex(r"U(1) \times SU(2) \times SU(3)", font_size=42),
            Text("Emergent from lattice geometry", font_size=24, color=GRAY_B)
        ).arrange(DOWN, buff=0.5)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(3)

    def create_origin_box(self, gauge, origin, meaning, color):
        """Create a box showing the origin of a gauge symmetry."""
        box = VGroup(
            MathTex(gauge, font_size=28, color=color),
            Text(f"From: {origin}", font_size=16, color=WHITE),
            Text(f"→ {meaning}", font_size=16, color=GRAY_B),
        ).arrange(DOWN, buff=0.2)

        rect = SurroundingRectangle(box, color=color, buff=0.2, stroke_width=2)
        return VGroup(rect, box)


class DoFCountingDemo(Scene):
    """A simpler demo showing degree of freedom counting."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        title = Text("Degree of Freedom Counting", font_size=36, color=HIGHLIGHT)
        title.to_edge(UP)
        self.play(Write(title))

        # Vector field has 3 components
        line1 = MathTex(r"\mathbf{J} = (J_x, J_y, J_z)", font_size=32)
        line2 = Text("3 components", font_size=24, color=WHITE)
        step1 = VGroup(line1, line2).arrange(DOWN, buff=0.2)
        step1.shift(UP * 1.5)

        self.play(Write(step1), run_time=1)

        # Minus constraint
        line3 = MathTex(r"\nabla \cdot \mathbf{J} = \rho", font_size=32)
        line4 = Text("- 1 constraint (Gauss law)", font_size=24, color=GRAY_B)
        step2 = VGroup(line3, line4).arrange(DOWN, buff=0.2)

        self.play(Write(step2), run_time=1)

        # Equals
        line5 = MathTex(r"3 - 1 = 2", font_size=48, color=U1_COLOR)
        line6 = Text("2 physical degrees of freedom", font_size=24, color=U1_COLOR)
        step3 = VGroup(line5, line6).arrange(DOWN, buff=0.2)
        step3.shift(DOWN * 1.5)

        self.play(Write(step3), run_time=1)

        # Photon polarizations
        photon = Text("= 2 photon polarization states!", font_size=28, color=HIGHLIGHT)
        photon.to_edge(DOWN, buff=1)
        self.play(Write(photon), Flash(photon, color=HIGHLIGHT), run_time=1)

        self.wait(2)


# Add missing color constants
MATTER_COLOR = "#DD4444"
ANTIMATTER_COLOR = "#4488DD"


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pql scene_06_gauge_emergence.py GaugeEmergenceScene")
    print("Or simpler version:")
    print("  manim -pql scene_06_gauge_emergence.py DoFCountingDemo")
