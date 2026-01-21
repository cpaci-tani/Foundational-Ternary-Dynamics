"""
Simulation 4: The Four Integers - Constraint Cascade
====================================================
A 120-second Manim animation showing how {3, 4, 7, 13} uniquely determine all physics.

Design: Four glowing integers orbit, each expands into branches showing their role:
- N_c = 3: color charges, dimensions, generations
- N_base = 4: Fermat boundary, 16 = 4² DoF, 2⁴ binary
- b₃ = 7: QCD β₀, Fibonacci F₇
- n_eff = 13: DoF count, closure: n_eff = b₃ + 2N_c

Run with: manim -pql scene_04_four_integers.py FourIntegersScene
For high quality: manim -pqh scene_04_four_integers.py FourIntegersScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors
BACKGROUND = "#0D1117"
INTEGER_3 = "#E74C3C"   # Red for N_c
INTEGER_4 = "#F39C12"   # Amber for N_base
INTEGER_7 = "#9B59B6"   # Purple for b_3
INTEGER_13 = "#3498DB"  # Blue for n_eff
HIGHLIGHT = "#FFD700"   # Gold for emphasis


class FourIntegersScene(Scene):
    """The four framework integers and their physical meanings."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # INTRO: Title and Setup
        # =====================================================================

        title = Text("The Four Framework Integers", font_size=42, color=HIGHLIGHT)
        subtitle = Text("From which all physics flows", font_size=24, color=WHITE)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=2)
        self.play(FadeIn(subtitle), run_time=1)
        self.wait(1)
        self.play(FadeOut(title_group), run_time=1)

        # =====================================================================
        # Create the four integer objects
        # =====================================================================

        def create_integer_mobject(value, color, label):
            """Create a styled integer display."""
            circle = Circle(radius=0.8, color=color, fill_opacity=0.3, stroke_width=4)
            number = Text(str(value), font_size=72, color=color, weight=BOLD)
            name = Text(label, font_size=18, color=WHITE)
            name.next_to(circle, DOWN, buff=0.2)
            return VGroup(circle, number, name)

        int_3 = create_integer_mobject(3, INTEGER_3, "N_c")
        int_4 = create_integer_mobject(4, INTEGER_4, "N_base")
        int_7 = create_integer_mobject(7, INTEGER_7, "b₃")
        int_13 = create_integer_mobject(13, INTEGER_13, "n_eff")

        # Position in a diamond
        int_3.move_to(UP * 2)
        int_4.move_to(RIGHT * 3)
        int_7.move_to(DOWN * 2)
        int_13.move_to(LEFT * 3)

        integers = VGroup(int_3, int_4, int_7, int_13)

        # Animate entrance
        self.play(
            *[FadeIn(i, scale=0.5) for i in integers],
            run_time=2
        )

        # Create connecting golden threads
        connections = VGroup()
        positions = [int_3.get_center(), int_4.get_center(), int_7.get_center(), int_13.get_center()]

        for i in range(4):
            for j in range(i+1, 4):
                line = Line(positions[i], positions[j], color=HIGHLIGHT, stroke_opacity=0.3, stroke_width=2)
                connections.add(line)

        self.play(Create(connections), run_time=1)

        # Orbit animation
        self.play(
            Rotate(integers, angle=TAU/4, about_point=ORIGIN),
            run_time=3
        )

        self.wait(1)

        # =====================================================================
        # SCENE 2: N_c = 3 (Color Charges)
        # =====================================================================

        # Move others aside, highlight N_c
        self.play(
            int_3.animate.move_to(LEFT * 3 + UP * 1.5).scale(1.2),
            int_4.animate.move_to(RIGHT * 4 + UP * 1).scale(0.6).set_opacity(0.3),
            int_7.animate.move_to(RIGHT * 4).scale(0.6).set_opacity(0.3),
            int_13.animate.move_to(RIGHT * 4 + DOWN * 1).scale(0.6).set_opacity(0.3),
            FadeOut(connections),
            run_time=1.5
        )

        # N_c = 3 explanation
        n_c_title = Text("N_c = 3", font_size=36, color=INTEGER_3)
        n_c_title.to_corner(UL, buff=0.5)
        self.play(Write(n_c_title), run_time=0.5)

        # Branches
        branches_3 = VGroup(
            self.create_branch("Color Charges", "Red, Green, Blue quarks", INTEGER_3),
            self.create_branch("Spatial Dimensions", "3D cubic lattice", INTEGER_3),
            self.create_branch("Generations", "3 families of fermions", INTEGER_3),
        ).arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        branches_3.next_to(int_3, RIGHT, buff=1)

        for branch in branches_3:
            self.play(FadeIn(branch, shift=RIGHT), run_time=0.8)

        # Pulse the integer
        self.play(
            int_3[0].animate.scale(1.2),
            run_time=0.3
        )
        self.play(
            int_3[0].animate.scale(1/1.2),
            run_time=0.3
        )

        self.wait(1.5)

        # Clear branches
        self.play(FadeOut(branches_3), FadeOut(n_c_title))

        # =====================================================================
        # SCENE 3: N_base = 4 (Fermat Boundary)
        # =====================================================================

        self.play(
            int_3.animate.move_to(LEFT * 4 + UP * 1).scale(1/1.2).scale(0.6).set_opacity(0.3),
            int_4.animate.move_to(LEFT * 3 + UP * 1.5).scale(1/0.6).scale(1.2).set_opacity(1),
            run_time=1
        )

        n_base_title = Text("N_base = 4", font_size=36, color=INTEGER_4)
        n_base_title.to_corner(UL, buff=0.5)
        self.play(Write(n_base_title), run_time=0.5)

        branches_4 = VGroup(
            self.create_branch("Fermat Boundary", "n=4 first forbidden FLT exponent", INTEGER_4),
            self.create_branch("Lattice DoF", "16 = 4² physical modes", INTEGER_4),
            self.create_branch("Binary Power", "2⁴ = 16 (information encoding)", INTEGER_4),
            self.create_branch("Coefficient", "Master quadratic: 16G*²", INTEGER_4),
        ).arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        branches_4.next_to(int_4, RIGHT, buff=1)

        for branch in branches_4:
            self.play(FadeIn(branch, shift=RIGHT), run_time=0.7)

        # Show 4² = 16
        eq_16 = MathTex(r"4^2 = 16", font_size=32, color=INTEGER_4)
        eq_16.next_to(branches_4, DOWN, buff=0.5)
        self.play(Write(eq_16), run_time=0.5)

        self.wait(1.5)
        self.play(FadeOut(branches_4), FadeOut(n_base_title), FadeOut(eq_16))

        # =====================================================================
        # SCENE 4: b₃ = 7 (QCD Beta)
        # =====================================================================

        self.play(
            int_4.animate.move_to(LEFT * 4).scale(1/1.2).scale(0.6).set_opacity(0.3),
            int_7.animate.move_to(LEFT * 3 + UP * 1.5).scale(1/0.6).scale(1.2).set_opacity(1),
            run_time=1
        )

        b3_title = Text("b₃ = 7", font_size=36, color=INTEGER_7)
        b3_title.to_corner(UL, buff=0.5)
        self.play(Write(b3_title), run_time=0.5)

        branches_7 = VGroup(
            self.create_branch("QCD Beta", "β₀ = 11 - 2n_f/3 = 7", INTEGER_7),
            self.create_branch("Fibonacci", "F₇ = 13 (closure)", INTEGER_7),
            self.create_branch("Flavor Ratios", "Mass hierarchies", INTEGER_7),
        ).arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        branches_7.next_to(int_7, RIGHT, buff=1)

        for branch in branches_7:
            self.play(FadeIn(branch, shift=RIGHT), run_time=0.8)

        # Fibonacci sequence highlight
        fib = MathTex(r"1, 1, 2, 3, 5, 8, \mathbf{13}, 21, ...", font_size=28)
        fib[0][12:14].set_color(INTEGER_13)  # Highlight 13
        fib.next_to(branches_7, DOWN, buff=0.5)
        self.play(Write(fib), run_time=1)

        self.wait(1.5)
        self.play(FadeOut(branches_7), FadeOut(b3_title), FadeOut(fib))

        # =====================================================================
        # SCENE 5: n_eff = 13 (Effective DoF)
        # =====================================================================

        self.play(
            int_7.animate.move_to(LEFT * 4 + DOWN * 1).scale(1/1.2).scale(0.6).set_opacity(0.3),
            int_13.animate.move_to(LEFT * 3 + UP * 1.5).scale(1/0.6).scale(1.2).set_opacity(1),
            run_time=1
        )

        n_eff_title = Text("n_eff = 13", font_size=36, color=INTEGER_13)
        n_eff_title.to_corner(UL, buff=0.5)
        self.play(Write(n_eff_title), run_time=0.5)

        branches_13 = VGroup(
            self.create_branch("Effective DoF", "Total physical modes", INTEGER_13),
            self.create_branch("Fibonacci F₇", "7th Fibonacci number", INTEGER_13),
            self.create_branch("Self-Consistency", "Framework closure", INTEGER_13),
        ).arrange(DOWN, buff=0.4, aligned_edge=LEFT)
        branches_13.next_to(int_13, RIGHT, buff=1)

        for branch in branches_13:
            self.play(FadeIn(branch, shift=RIGHT), run_time=0.8)

        # THE CONSTRAINT EQUATION
        constraint = MathTex(
            r"n_{\text{eff}} = b_3 + 2N_c",
            font_size=36,
            color=HIGHLIGHT
        )
        constraint.next_to(branches_13, DOWN, buff=0.8)

        self.play(Write(constraint), run_time=1)

        # Show the calculation
        calc = MathTex(
            r"13 = 7 + 2 \times 3 = 7 + 6 = 13 \; \checkmark",
            font_size=32
        )
        calc[0][0:2].set_color(INTEGER_13)
        calc[0][3].set_color(INTEGER_7)
        calc[0][5:8].set_color(INTEGER_3)
        calc.next_to(constraint, DOWN, buff=0.3)

        self.play(Write(calc), run_time=1.5)

        # Flash the checkmark
        self.play(
            Flash(calc, color=GREEN, flash_radius=0.5),
            run_time=0.5
        )

        self.wait(2)

        # =====================================================================
        # FINALE: All Four Together
        # =====================================================================

        self.play(
            FadeOut(branches_13),
            FadeOut(n_eff_title),
            FadeOut(constraint),
            FadeOut(calc)
        )

        # Bring all four back to center
        self.play(
            int_3.animate.move_to(UP * 2 + LEFT * 2).scale(1/0.6).set_opacity(1),
            int_4.animate.move_to(UP * 2 + RIGHT * 2).scale(1).set_opacity(1),
            int_7.animate.move_to(DOWN * 1 + LEFT * 2).scale(1).set_opacity(1),
            int_13.animate.move_to(DOWN * 1 + RIGHT * 2).scale(1/1.2).set_opacity(1),
            run_time=1.5
        )

        # Scale to uniform
        integers_new = VGroup(int_3, int_4, int_7, int_13)
        self.play(
            integers_new.animate.arrange_in_grid(rows=2, cols=2, buff=1.5).scale(0.8),
            run_time=1
        )

        # Final message
        final_box = VGroup(
            Text("From these four integers alone:", font_size=28, color=WHITE),
            MathTex(r"\{3, 4, 7, 13\}", font_size=48, color=HIGHLIGHT),
            Text("All Standard Model physics emerges", font_size=24, color=GRAY_B),
        ).arrange(DOWN, buff=0.3)
        final_box.to_edge(DOWN, buff=0.8)

        self.play(FadeIn(final_box), run_time=2)

        # Draw connecting lines with equations
        self.wait(3)

        # Fade out
        self.play(*[FadeOut(mob) for mob in self.mobjects], run_time=2)

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=32, color=HIGHLIGHT),
            MathTex(r"N_c = 3, \; N_{\text{base}} = 4, \; b_3 = 7, \; n_{\text{eff}} = 13", font_size=36),
            Text("The integers of existence", font_size=24, color=GRAY_B)
        ).arrange(DOWN, buff=0.5)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(3)

    def create_branch(self, title, description, color):
        """Create a branch item with title and description."""
        bullet = Dot(radius=0.08, color=color)
        title_text = Text(title, font_size=22, color=color, weight=BOLD)
        desc_text = Text(description, font_size=16, color=WHITE)

        title_text.next_to(bullet, RIGHT, buff=0.2)
        desc_text.next_to(title_text, DOWN, buff=0.1, aligned_edge=LEFT)

        return VGroup(bullet, title_text, desc_text)


class IntegerConstraintDemo(Scene):
    """A simpler demonstration of the constraint equation."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # Title
        title = Text("The Framework Constraint", font_size=36, color=HIGHLIGHT)
        title.to_edge(UP, buff=0.5)
        self.play(Write(title))

        # The four integers with their values
        integers = VGroup(
            MathTex(r"N_c = 3", font_size=42, color=INTEGER_3),
            MathTex(r"N_{\text{base}} = 4", font_size=42, color=INTEGER_4),
            MathTex(r"b_3 = 7", font_size=42, color=INTEGER_7),
            MathTex(r"n_{\text{eff}} = 13", font_size=42, color=INTEGER_13),
        ).arrange_in_grid(rows=2, cols=2, buff=1)

        self.play(
            *[Write(i) for i in integers],
            run_time=2
        )
        self.wait(1)

        # Move up and show constraint
        self.play(integers.animate.shift(UP * 1), run_time=1)

        constraint = MathTex(
            r"n_{\text{eff}} = b_3 + 2N_c",
            font_size=48,
            color=WHITE
        )
        constraint.shift(DOWN * 1.5)
        self.play(Write(constraint), run_time=1)

        # Substitution
        substitution = MathTex(
            r"13 = 7 + 2(3) = 7 + 6 = 13",
            font_size=42
        )
        substitution.next_to(constraint, DOWN, buff=0.5)

        self.play(Write(substitution), run_time=2)

        # Checkmark
        check = MathTex(r"\checkmark", font_size=72, color=GREEN)
        check.next_to(substitution, RIGHT, buff=0.5)
        self.play(Write(check), Flash(check, color=GREEN), run_time=1)

        self.wait(2)


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pql scene_04_four_integers.py FourIntegersScene")
    print("Or simpler version:")
    print("  manim -pql scene_04_four_integers.py IntegerConstraintDemo")
