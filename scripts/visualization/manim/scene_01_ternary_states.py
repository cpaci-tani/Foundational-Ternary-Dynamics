"""
Simulation 1: The Ternary State System - Birth of Existence
============================================================
A 90-second Manim animation showing how manifestation emerges from the void.

Storyboard:
1. (0-15s)  Black screen → "Before manifestation, there is potential"
2. (15-35s) Single gray voxel pulses; equations: s(v,t) ∈ {-1, 0, +1}
3. (35-55s) Golden flux vectors J(v,t) ∈ ℝ³ emerge, voxel glows as |J| grows
4. (55-75s) Threshold KB crossed → voxel splits: red (+1) and blue (-1) pair
5. (75-90s) Pull back to 3D lattice, text: "From three states, a universe"

Run with: manim -pql scene_01_ternary_states.py TernaryStateScene
For high quality: manim -pqh scene_01_ternary_states.py TernaryStateScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np
import sys
import os

# Add utils to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'utils'))

# Color constants (inline to avoid import issues)
VOID_COLOR = "#888888"
MATTER_COLOR = "#DD4444"
ANTIMATTER_COLOR = "#4488DD"
FLUX_COLOR = "#FFD700"
BACKGROUND_COLOR = "#0D1117"


class TernaryStateScene(ThreeDScene):
    """The foundational animation: from void to manifestation."""

    def construct(self):
        # Set dark background
        self.camera.background_color = BACKGROUND_COLOR

        # =====================================================================
        # SCENE 1: The Potential (0-15s)
        # =====================================================================
        title = Text(
            "Before manifestation,\nthere is potential.",
            font_size=48,
            color=WHITE
        ).set_opacity(0)

        self.play(title.animate.set_opacity(1), run_time=3)
        self.wait(2)
        self.play(title.animate.set_opacity(0), run_time=2)
        self.wait(1)

        # Subtitle
        subtitle = Text(
            "The Ternary State System",
            font_size=36,
            color=GRAY_B
        )
        self.play(Write(subtitle), run_time=2)
        self.wait(1)
        self.play(FadeOut(subtitle), run_time=1)

        # =====================================================================
        # SCENE 2: The Void Voxel (15-35s)
        # =====================================================================

        # Create a single voxel as a cube
        voxel = Cube(side_length=1.5, fill_opacity=0.7, stroke_width=2)
        voxel.set_fill(VOID_COLOR)
        voxel.set_stroke(WHITE, width=1)

        # Label
        void_label = Text("void", font_size=24, color=VOID_COLOR)
        void_label.next_to(voxel, DOWN, buff=0.5)

        # State equation
        state_eq = MathTex(
            r"s(v,t) \in \{-1, 0, +1\}",
            font_size=36
        )
        state_eq.to_corner(UL, buff=0.5)

        # Set up 3D camera
        self.set_camera_orientation(phi=70 * DEGREES, theta=30 * DEGREES)

        # Animate voxel appearance
        self.play(
            FadeIn(voxel, scale=0.5),
            Write(void_label),
            run_time=2
        )

        # Add equation
        self.add_fixed_in_frame_mobjects(state_eq)
        self.play(Write(state_eq), run_time=2)

        # Pulse the voxel to show it's not truly empty
        for _ in range(3):
            self.play(
                voxel.animate.scale(1.1).set_opacity(0.9),
                run_time=0.5
            )
            self.play(
                voxel.animate.scale(1/1.1).set_opacity(0.7),
                run_time=0.5
            )

        # State table
        state_table = VGroup(
            MathTex(r"-1", color=ANTIMATTER_COLOR),
            Text("antimatter", font_size=20, color=ANTIMATTER_COLOR),
            MathTex(r"0", color=VOID_COLOR),
            Text("void", font_size=20, color=VOID_COLOR),
            MathTex(r"+1", color=MATTER_COLOR),
            Text("matter", font_size=20, color=MATTER_COLOR),
        ).arrange_in_grid(rows=3, cols=2, buff=0.3)
        state_table.to_corner(UR, buff=0.5)

        self.add_fixed_in_frame_mobjects(state_table)
        self.play(FadeIn(state_table), run_time=2)
        self.wait(2)

        # =====================================================================
        # SCENE 3: Flux Field Emergence (35-55s)
        # =====================================================================

        # Remove state table, keep equation
        self.play(FadeOut(state_table), FadeOut(void_label))

        # Flux equation
        flux_eq = MathTex(
            r"\mathbf{J}(v,t) \in \mathbb{R}^3",
            font_size=36
        )
        flux_eq.next_to(state_eq, DOWN, buff=0.3, aligned_edge=LEFT)
        self.add_fixed_in_frame_mobjects(flux_eq)
        self.play(Write(flux_eq), run_time=2)

        # Create flux arrows emanating from voxel center
        flux_arrows = VGroup()
        n_arrows = 6
        arrow_directions = [
            RIGHT, LEFT, UP, DOWN, OUT, IN
        ]

        for direction in arrow_directions:
            arrow = Arrow3D(
                start=ORIGIN,
                end=direction * 0.8,
                color=FLUX_COLOR,
                thickness=0.02,
                height=0.15,
                base_radius=0.05
            )
            flux_arrows.add(arrow)

        # Animate flux emergence
        self.play(
            *[GrowArrow(arrow) for arrow in flux_arrows],
            run_time=2
        )

        # Flux magnitude label
        flux_mag = MathTex(r"|J| = 0.1", font_size=28, color=FLUX_COLOR)
        flux_mag.next_to(voxel, RIGHT, buff=1)
        self.add_fixed_in_frame_mobjects(flux_mag)
        self.play(Write(flux_mag), run_time=1)

        # Grow flux and make voxel glow
        threshold_eq = MathTex(
            r"K_B = 0.511 \text{ (threshold)}",
            font_size=28,
            color=YELLOW
        )
        threshold_eq.next_to(flux_eq, DOWN, buff=0.3, aligned_edge=LEFT)
        self.add_fixed_in_frame_mobjects(threshold_eq)
        self.play(Write(threshold_eq), run_time=1)

        # Animate flux growing
        for i, mag in enumerate([0.2, 0.35, 0.45, 0.55, 0.65]):
            new_mag = MathTex(f"|J| = {mag:.2f}", font_size=28, color=FLUX_COLOR)
            new_mag.move_to(flux_mag)
            self.add_fixed_in_frame_mobjects(new_mag)

            scale_factor = 1 + mag
            glow_color = interpolate_color(
                Color(VOID_COLOR), Color(FLUX_COLOR), mag
            )

            self.play(
                *[arrow.animate.scale(scale_factor / (1 + (mag - 0.1))) for arrow in flux_arrows],
                voxel.animate.set_fill(glow_color, opacity=0.5 + 0.3 * mag),
                Transform(flux_mag, new_mag),
                run_time=0.8
            )

        self.wait(1)

        # =====================================================================
        # SCENE 4: Genesis - Pair Production (55-75s)
        # =====================================================================

        # Genesis equation
        genesis_eq = MathTex(
            r"P_{\text{manifest}} = 1 - e^{-(|J| - K_B)/K_B}",
            font_size=28
        )
        genesis_eq.next_to(threshold_eq, DOWN, buff=0.3, aligned_edge=LEFT)
        self.add_fixed_in_frame_mobjects(genesis_eq)
        self.play(Write(genesis_eq), run_time=2)

        # Show threshold crossed
        crossed = Text("THRESHOLD CROSSED!", font_size=32, color=YELLOW)
        crossed.to_edge(DOWN, buff=1)
        self.add_fixed_in_frame_mobjects(crossed)
        self.play(
            Write(crossed),
            Flash(voxel, color=YELLOW, flash_radius=2),
            run_time=1
        )
        self.wait(1)

        # Create matter and antimatter particles
        matter_particle = Sphere(radius=0.5, resolution=(20, 20))
        matter_particle.set_color(MATTER_COLOR)
        matter_particle.set_opacity(0.9)

        antimatter_particle = Sphere(radius=0.5, resolution=(20, 20))
        antimatter_particle.set_color(ANTIMATTER_COLOR)
        antimatter_particle.set_opacity(0.9)

        # Labels
        matter_label = MathTex(r"+1", font_size=36, color=MATTER_COLOR)
        antimatter_label = MathTex(r"-1", font_size=36, color=ANTIMATTER_COLOR)

        # Position for separation
        matter_particle.move_to(RIGHT * 2)
        antimatter_particle.move_to(LEFT * 2)

        # Polarity rule
        polarity_eq = MathTex(
            r"\text{sign}(s) = \text{sign}(\nabla \cdot J)",
            font_size=28
        )
        polarity_eq.next_to(genesis_eq, DOWN, buff=0.3, aligned_edge=LEFT)
        self.add_fixed_in_frame_mobjects(polarity_eq)

        # Remove flux arrows and old voxel
        self.play(
            FadeOut(flux_arrows),
            FadeOut(crossed),
            FadeOut(flux_mag),
            voxel.animate.set_opacity(0),
            run_time=1
        )

        # Birth animation - particles emerge from center
        matter_particle.move_to(ORIGIN)
        antimatter_particle.move_to(ORIGIN)

        self.play(
            FadeIn(matter_particle, scale=0.1),
            FadeIn(antimatter_particle, scale=0.1),
            run_time=0.5
        )

        self.play(
            matter_particle.animate.move_to(RIGHT * 2),
            antimatter_particle.animate.move_to(LEFT * 2),
            Write(polarity_eq),
            run_time=2
        )

        # Add labels
        matter_label.next_to(matter_particle, DOWN, buff=0.3)
        antimatter_label.next_to(antimatter_particle, DOWN, buff=0.3)

        self.add_fixed_in_frame_mobjects(matter_label, antimatter_label)
        self.play(
            Write(matter_label),
            Write(antimatter_label),
            run_time=1
        )

        # Pair production annotation
        pair_text = Text("Pair Production", font_size=28, color=WHITE)
        pair_text.to_edge(DOWN, buff=0.5)
        self.add_fixed_in_frame_mobjects(pair_text)
        self.play(Write(pair_text), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 5: The Lattice Universe (75-90s)
        # =====================================================================

        # Clear scene
        self.play(
            *[FadeOut(mob) for mob in [
                matter_particle, antimatter_particle,
                matter_label, antimatter_label, pair_text,
                state_eq, flux_eq, threshold_eq, genesis_eq, polarity_eq, voxel
            ]],
            run_time=1
        )

        # Create a 5x5x5 lattice of voxels
        lattice = VGroup()
        size = 3
        spacing = 1.2

        np.random.seed(42)  # For reproducibility

        for i in range(-size, size + 1):
            for j in range(-size, size + 1):
                for k in range(-size, size + 1):
                    # Random state assignment
                    state = np.random.choice([-1, 0, 0, 0, 1], p=[0.1, 0.3, 0.3, 0.2, 0.1])

                    if state == 0:
                        # Void - small transparent cube
                        cube = Cube(side_length=0.3, fill_opacity=0.1)
                        cube.set_fill(VOID_COLOR)
                        cube.set_stroke(GRAY, width=0.5)
                    elif state == 1:
                        # Matter - red sphere
                        cube = Sphere(radius=0.25, resolution=(10, 10))
                        cube.set_color(MATTER_COLOR)
                        cube.set_opacity(0.9)
                    else:
                        # Antimatter - blue sphere
                        cube = Sphere(radius=0.25, resolution=(10, 10))
                        cube.set_color(ANTIMATTER_COLOR)
                        cube.set_opacity(0.9)

                    cube.move_to(np.array([i, j, k]) * spacing * 0.5)
                    lattice.add(cube)

        # Scale down and zoom out
        lattice.scale(0.5)

        self.play(
            FadeIn(lattice, scale=0.5),
            self.camera.animate.set_euler_angles(
                phi=60 * DEGREES,
                theta=45 * DEGREES
            ),
            run_time=3
        )

        # Rotate camera slowly
        self.begin_ambient_camera_rotation(rate=0.1)

        # Final text
        final_text = Text(
            "From three states, a universe.",
            font_size=40,
            color=WHITE
        )
        final_text.to_edge(DOWN, buff=1)
        self.add_fixed_in_frame_mobjects(final_text)
        self.play(Write(final_text), run_time=2)

        self.wait(3)

        # Stop rotation and fade
        self.stop_ambient_camera_rotation()
        self.play(
            FadeOut(lattice),
            FadeOut(final_text),
            run_time=2
        )

        # End card
        end_text = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=32, color=FLUX_COLOR),
            Text("The Ternary State System", font_size=24, color=WHITE),
        ).arrange(DOWN, buff=0.5)

        self.add_fixed_in_frame_mobjects(end_text)
        self.play(FadeIn(end_text), run_time=2)
        self.wait(2)


class VoxelPulseDemo(Scene):
    """A simpler 2D version for testing."""

    def construct(self):
        self.camera.background_color = BACKGROUND_COLOR

        # Title
        title = Text("Ternary States", font_size=48)
        self.play(Write(title))
        self.wait(1)
        self.play(title.animate.to_edge(UP))

        # Three squares representing states
        void_sq = Square(side_length=2, fill_opacity=0.7)
        void_sq.set_fill(VOID_COLOR)
        void_sq.set_stroke(WHITE)

        matter_sq = Square(side_length=2, fill_opacity=0.7)
        matter_sq.set_fill(MATTER_COLOR)
        matter_sq.set_stroke(WHITE)

        antimatter_sq = Square(side_length=2, fill_opacity=0.7)
        antimatter_sq.set_fill(ANTIMATTER_COLOR)
        antimatter_sq.set_stroke(WHITE)

        # Labels
        void_label = MathTex("0", font_size=48).move_to(void_sq)
        matter_label = MathTex("+1", font_size=48).move_to(matter_sq)
        antimatter_label = MathTex("-1", font_size=48).move_to(antimatter_sq)

        # Arrange horizontally
        squares = VGroup(antimatter_sq, void_sq, matter_sq).arrange(RIGHT, buff=1)
        labels = VGroup(antimatter_label, void_label, matter_label)

        antimatter_label.move_to(antimatter_sq)
        void_label.move_to(void_sq)
        matter_label.move_to(matter_sq)

        # State names
        names = VGroup(
            Text("Antimatter", font_size=24, color=ANTIMATTER_COLOR),
            Text("Void", font_size=24, color=VOID_COLOR),
            Text("Matter", font_size=24, color=MATTER_COLOR),
        )
        names[0].next_to(antimatter_sq, DOWN)
        names[1].next_to(void_sq, DOWN)
        names[2].next_to(matter_sq, DOWN)

        # Animate
        self.play(
            FadeIn(void_sq),
            Write(void_label),
            Write(names[1]),
            run_time=1
        )
        self.play(
            FadeIn(matter_sq),
            Write(matter_label),
            Write(names[2]),
            FadeIn(antimatter_sq),
            Write(antimatter_label),
            Write(names[0]),
            run_time=1
        )

        # Equation
        eq = MathTex(r"s(v,t) \in \{-1, 0, +1\}", font_size=36)
        eq.to_edge(DOWN, buff=1)
        self.play(Write(eq))

        self.wait(2)


if __name__ == "__main__":
    # For testing, render the simple version
    print("Run with: manim -pql scene_01_ternary_states.py TernaryStateScene")
    print("Or 2D version: manim -pql scene_01_ternary_states.py VoxelPulseDemo")
