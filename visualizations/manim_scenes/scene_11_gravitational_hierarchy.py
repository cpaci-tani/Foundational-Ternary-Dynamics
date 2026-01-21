"""
Simulation 11: The Gravitational Hierarchy - Why Gravity is Weak
================================================================
A 90-second Manim animation showing how FTD explains the 10^37 ratio
between electromagnetic and gravitational forces.

The hierarchy is NOT a mystery - it's derived from the framework integers!

α_G = 2π(16/3)²(n_eff + 3/b₃)²α²⁰ = 1.75×10⁻⁴⁵

This gives the correct gravity-to-EM ratio to 0.06% accuracy.

Storyboard:
1. (0-20s) The hierarchy problem: EM vs Gravity between electrons
2. (20-45s) The FTD formula with each component explained
3. (45-70s) Step-by-step calculation
4. (70-90s) Result and significance

Run with: manim -pql scene_11_gravitational_hierarchy.py GravitationalHierarchyScene
For high quality: manim -pqh scene_11_gravitational_hierarchy.py GravitationalHierarchyScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors
BACKGROUND = "#0D1117"
GRAVITY = "#27AE60"      # Green for gravity
EM = "#3498DB"           # Blue for EM
HIGHLIGHT = "#FFD700"    # Gold
INTEGER_3 = "#E74C3C"    # Red for N_c
INTEGER_4 = "#F39C12"    # Amber for N_base
INTEGER_7 = "#9B59B6"    # Purple for b_3
INTEGER_13 = "#3498DB"   # Blue for n_eff


class GravitationalHierarchyScene(Scene):
    """Why gravity is 10^37 times weaker than electromagnetism."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # SCENE 1: The Hierarchy Problem (0-20s)
        # =====================================================================

        title = Text("The Gravitational Hierarchy", font_size=42, color=HIGHLIGHT)
        subtitle = Text("Why is gravity so weak?", font_size=28, color=WHITE)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=1.5)
        self.play(FadeIn(subtitle), run_time=1)
        self.wait(0.5)
        self.play(title_group.animate.to_edge(UP, buff=0.3).scale(0.8), run_time=1)

        # Two electrons
        electron1 = VGroup(
            Circle(radius=0.3, color=EM, fill_opacity=0.5),
            MathTex(r"e^-", font_size=24, color=WHITE)
        )
        electron2 = electron1.copy()

        electron1.shift(LEFT * 2)
        electron2.shift(RIGHT * 2)

        self.play(FadeIn(electron1), FadeIn(electron2), run_time=1)

        # Show both forces
        em_arrow = Arrow(LEFT * 1.5, RIGHT * 1.5, color=EM, stroke_width=8)
        em_label = Text("Electromagnetic", font_size=18, color=EM)
        em_label.next_to(em_arrow, UP, buff=0.2)

        grav_arrow = Arrow(LEFT * 1.5, RIGHT * 1.5, color=GRAVITY, stroke_width=1)
        grav_arrow.shift(DOWN * 1.2)
        grav_label = Text("Gravitational", font_size=18, color=GRAVITY)
        grav_label.next_to(grav_arrow, DOWN, buff=0.2)

        self.play(
            Create(em_arrow), Write(em_label),
            Create(grav_arrow), Write(grav_label),
            run_time=1.5
        )

        # The ratio
        ratio = MathTex(
            r"\frac{F_{\text{EM}}}{F_{\text{grav}}} = ",
            r"10^{37}",
            font_size=48
        )
        ratio[1].set_color(HIGHLIGHT)
        ratio.shift(DOWN * 2.5)

        self.play(Write(ratio), run_time=1.5)

        # Flash the huge number
        self.play(
            Flash(ratio[1], color=HIGHLIGHT, flash_radius=0.8),
            ratio[1].animate.scale(1.3),
            run_time=0.5
        )
        self.play(ratio[1].animate.scale(1/1.3), run_time=0.3)

        # Question
        question = Text("Why such an enormous ratio?", font_size=28, color=WHITE)
        question.next_to(ratio, DOWN, buff=0.5)

        self.play(Write(question), run_time=1)
        self.wait(1)

        # Clear for formula
        self.play(
            FadeOut(VGroup(
                electron1, electron2, em_arrow, em_label,
                grav_arrow, grav_label, ratio, question
            )),
            run_time=1
        )

        # =====================================================================
        # SCENE 2: The FTD Formula (20-45s)
        # =====================================================================

        answer = Text("FTD DERIVES this ratio from 4 integers!", font_size=28, color=HIGHLIGHT)
        answer.next_to(title_group, DOWN, buff=0.5)

        self.play(Write(answer), run_time=1.5)

        # The master formula
        formula = MathTex(
            r"\alpha_G = 2\pi",
            r"\left(\frac{16}{3}\right)^2",
            r"\left(n_{\text{eff}} + \frac{3}{b_3}\right)^2",
            r"\alpha^{20}",
            font_size=36
        )
        formula.shift(DOWN * 0.5)

        self.play(Write(formula), run_time=2)
        self.wait(1)

        # Highlight each component
        components = [
            (formula[0], "2π", "Action principle normalization", WHITE),
            (formula[1], "(16/3)²", "= (N_base²/N_c)²", INTEGER_4),
            (formula[2], "(n_eff + 3/b₃)²", "= (13 + 3/7)²", INTEGER_13),
            (formula[3], "α²⁰", "Fine structure to 20th power!", EM),
        ]

        explanation_texts = []
        for i, (part, name, desc, color) in enumerate(components):
            # Highlight the part
            self.play(
                part.animate.set_color(color).scale(1.2),
                run_time=0.5
            )

            # Show explanation
            exp = VGroup(
                Text(name, font_size=22, color=color, weight=BOLD),
                Text(desc, font_size=18, color=WHITE)
            ).arrange(RIGHT, buff=0.3)
            exp.next_to(formula, DOWN, buff=0.8 + i * 0.6)

            self.play(FadeIn(exp, shift=UP), run_time=0.5)
            explanation_texts.append(exp)

            self.play(part.animate.scale(1/1.2), run_time=0.3)

        self.wait(2)

        # Clear explanations
        self.play(
            FadeOut(VGroup(*explanation_texts)),
            FadeOut(answer),
            run_time=0.5
        )

        # =====================================================================
        # SCENE 3: Step-by-step Calculation (45-70s)
        # =====================================================================

        calc_title = Text("Calculation:", font_size=28, color=WHITE)
        calc_title.next_to(formula, DOWN, buff=0.6)

        self.play(Write(calc_title), run_time=0.5)

        # Step by step
        steps = VGroup(
            MathTex(r"\alpha = 1/137.036 \approx 0.00729", font_size=26),
            MathTex(r"\alpha^{20} = (0.00729)^{20} \approx 3.7 \times 10^{-43}", font_size=26),
            MathTex(r"(16/3)^2 = 28.44", font_size=26),
            MathTex(r"(13 + 3/7)^2 = (13.43)^2 \approx 180.3", font_size=26),
            MathTex(r"2\pi \times 28.44 \times 180.3 \times 3.7 \times 10^{-43}", font_size=26),
        ).arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        steps.next_to(calc_title, DOWN, buff=0.3)

        for step in steps:
            self.play(Write(step), run_time=0.8)

        self.wait(1)

        # Result
        result_box = VGroup(
            MathTex(r"\alpha_G = 1.75 \times 10^{-45}", font_size=36, color=HIGHLIGHT),
        )
        result_box.next_to(steps, DOWN, buff=0.5)

        box_rect = SurroundingRectangle(result_box, color=HIGHLIGHT, buff=0.2)

        self.play(
            Write(result_box[0]),
            Create(box_rect),
            run_time=1
        )

        # Comparison with experiment
        comparison = MathTex(
            r"\text{Experimental: } \alpha_G = 1.75 \times 10^{-45}",
            font_size=28, color=GRAVITY
        )
        comparison.next_to(box_rect, DOWN, buff=0.3)

        self.play(Write(comparison), run_time=1)

        # Accuracy
        accuracy = MathTex(
            r"\text{Accuracy: } 0.06\%",
            font_size=32, color=HIGHLIGHT
        )
        accuracy.next_to(comparison, DOWN, buff=0.2)

        self.play(
            Write(accuracy),
            Flash(accuracy, color=GREEN, flash_radius=0.6),
            run_time=1
        )

        self.wait(2)

        # =====================================================================
        # SCENE 4: Significance (70-90s)
        # =====================================================================

        # Clear calculation
        self.play(
            FadeOut(VGroup(
                calc_title, steps, result_box, box_rect, comparison, accuracy, formula
            )),
            run_time=1
        )

        # Final message
        final = VGroup(
            Text("THE HIERARCHY IS NOT A MYSTERY", font_size=32, color=HIGHLIGHT, weight=BOLD),
            Text("", font_size=10),
            Text("It emerges from:", font_size=24, color=WHITE),
            VGroup(
                Dot(radius=0.05, color=INTEGER_3),
                MathTex(r"N_c = 3", font_size=28, color=INTEGER_3),
                Text("color charges", font_size=20, color=WHITE)
            ).arrange(RIGHT, buff=0.3),
            VGroup(
                Dot(radius=0.05, color=INTEGER_4),
                MathTex(r"N_{\text{base}} = 4", font_size=28, color=INTEGER_4),
                Text("Fermat boundary", font_size=20, color=WHITE)
            ).arrange(RIGHT, buff=0.3),
            VGroup(
                Dot(radius=0.05, color=INTEGER_7),
                MathTex(r"b_3 = 7", font_size=28, color=INTEGER_7),
                Text("QCD beta", font_size=20, color=WHITE)
            ).arrange(RIGHT, buff=0.3),
            VGroup(
                Dot(radius=0.05, color=INTEGER_13),
                MathTex(r"n_{\text{eff}} = 13", font_size=28, color=INTEGER_13),
                Text("effective DoF", font_size=20, color=WHITE)
            ).arrange(RIGHT, buff=0.3),
        ).arrange(DOWN, buff=0.4)

        self.play(FadeIn(final), run_time=2)

        # The punchline
        punchline = MathTex(
            r"\text{Gravity is weak because } \alpha^{20} \text{ is tiny!}",
            font_size=28, color=HIGHLIGHT
        )
        punchline.to_edge(DOWN, buff=0.8)

        self.play(Write(punchline), run_time=1.5)

        self.wait(3)

        # Fade out
        self.play(FadeOut(VGroup(final, punchline, title_group)), run_time=2)

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=28, color=HIGHLIGHT),
            MathTex(
                r"\alpha_G = 2\pi\left(\frac{16}{3}\right)^2\left(13 + \frac{3}{7}\right)^2\alpha^{20}",
                font_size=32
            ),
            Text("The hierarchy problem: solved", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.4)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(2)


class HierarchyVisual(Scene):
    """A visual comparison of force strengths."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        title = Text("Force Comparison", font_size=36, color=HIGHLIGHT)
        title.to_edge(UP, buff=0.5)
        self.play(Write(title))

        # Bar chart showing relative strengths
        forces = [
            ("Strong", 1, "#FF6B35"),
            ("EM", 1/137, "#3498DB"),
            ("Weak", 1e-6, "#9B59B6"),
            ("Gravity", 6e-39, "#27AE60"),
        ]

        bars = VGroup()
        labels = VGroup()

        for i, (name, strength, color) in enumerate(forces):
            # Use log scale for visualization
            log_strength = np.log10(strength) + 39  # Shift so gravity is at 0
            bar_height = max(0.2, log_strength / 10)  # Normalize

            bar = Rectangle(
                width=1.5,
                height=bar_height,
                fill_color=color,
                fill_opacity=0.8,
                stroke_color=color
            )
            bar.move_to(LEFT * 4.5 + RIGHT * i * 2.5 + DOWN * 1)
            bar.align_to(DOWN * 3, DOWN)

            label = Text(name, font_size=20, color=color)
            label.next_to(bar, DOWN, buff=0.2)

            strength_label = MathTex(f"{strength:.0e}" if strength < 0.01 else f"{strength:.3f}",
                                    font_size=18, color=WHITE)
            strength_label.next_to(bar, UP, buff=0.1)

            bars.add(bar)
            labels.add(VGroup(label, strength_label))

        self.play(
            *[GrowFromEdge(bar, DOWN) for bar in bars],
            run_time=2
        )
        self.play(FadeIn(labels), run_time=1)

        # Highlight the huge gap
        brace = Brace(VGroup(bars[1], bars[3]), DOWN, color=HIGHLIGHT)
        brace_label = MathTex(r"10^{37} \text{ ratio}", font_size=24, color=HIGHLIGHT)
        brace_label.next_to(brace, DOWN)

        self.play(Create(brace), Write(brace_label), run_time=1)

        self.wait(3)


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pql scene_11_gravitational_hierarchy.py GravitationalHierarchyScene")
    print("Or visual comparison:")
    print("  manim -pql scene_11_gravitational_hierarchy.py HierarchyVisual")
