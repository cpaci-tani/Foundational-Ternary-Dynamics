"""
Simulation 10: Bell Inequality and the sLoop Mechanism
=======================================================
A 150-second Manim animation explaining how FTD achieves Bell violations
without superluminal signaling through the sLoop (self-referential loop).

Key insight: The measurement apparatus is embedded in the same flux substrate
as the measured particles. Correlations are inherited, not transmitted.

Storyboard:
1. (0-25s) Classical setup: Two particles, two detectors, angle choices
2. (25-50s) Classical bound: CHSH inequality S ≤ 2 derived
3. (50-80s) Quantum prediction: S = 2√2 ≈ 2.828 shown
4. (80-110s) The sLoop insight: Observer embedded in substrate
5. (110-140s) FTD simulation results: S scales with substrate overlap
6. (140-150s) Conclusion: Correlations inherited, not transmitted

Run with: manim -pql scene_10_bell_sloop.py BellSLoopScene
For high quality: manim -pqh scene_10_bell_sloop.py BellSLoopScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Import FTD color scheme
import sys
sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.ftd_colors import *

# Colors
BACKGROUND = "#0D1117"
PARTICLE_A = "#DD4444"   # Red (Alice's particle)
PARTICLE_B = "#4488DD"   # Blue (Bob's particle)
DETECTOR = "#27AE60"     # Green
CLASSICAL = "#888888"    # Gray for classical
QUANTUM = "#FFD700"      # Gold for quantum
SLOOP = "#9B59B6"        # Purple for sLoop


class BellSLoopScene(Scene):
    """The Bell inequality and sLoop mechanism animation."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # SCENE 1: Classical Setup (0-25s)
        # =====================================================================

        title = Text("Bell Inequality & the sLoop", font_size=42, color=QUANTUM)
        subtitle = Text("Why quantum correlations exceed classical bounds",
                       font_size=24, color=WHITE)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=2)
        self.play(FadeIn(subtitle), run_time=1)
        self.wait(1)
        self.play(title_group.animate.to_edge(UP, buff=0.3).scale(0.7), run_time=1)

        # Create the Bell test setup
        source = Dot(ORIGIN, radius=0.2, color=QUANTUM)
        source_label = Text("Source", font_size=18, color=WHITE)
        source_label.next_to(source, DOWN, buff=0.2)

        # Two particles flying apart
        particle_a = Dot(LEFT * 3, radius=0.15, color=PARTICLE_A)
        particle_b = Dot(RIGHT * 3, radius=0.15, color=PARTICLE_B)

        # Detectors
        detector_a = self.create_detector("Alice", LEFT * 4.5)
        detector_b = self.create_detector("Bob", RIGHT * 4.5)

        # Angle indicators
        angle_a = Arc(radius=0.5, angle=PI/3, arc_center=LEFT * 4.5, color=PARTICLE_A)
        angle_b = Arc(radius=0.5, angle=-PI/4, arc_center=RIGHT * 4.5, color=PARTICLE_B)

        self.play(
            FadeIn(source),
            Write(source_label),
            run_time=1
        )

        # Particles emerge and fly apart
        self.play(
            particle_a.animate.move_to(LEFT * 3),
            particle_b.animate.move_to(RIGHT * 3),
            FadeIn(detector_a),
            FadeIn(detector_b),
            run_time=2
        )

        # Show angle choices
        angle_a_label = MathTex(r"\theta_A", font_size=24, color=PARTICLE_A)
        angle_b_label = MathTex(r"\theta_B", font_size=24, color=PARTICLE_B)
        angle_a_label.next_to(angle_a, UP, buff=0.1)
        angle_b_label.next_to(angle_b, UP, buff=0.1)

        self.play(
            Create(angle_a),
            Create(angle_b),
            Write(angle_a_label),
            Write(angle_b_label),
            run_time=1.5
        )

        # Measurement outcomes
        outcome_a = MathTex(r"A = \pm 1", font_size=28, color=PARTICLE_A)
        outcome_b = MathTex(r"B = \pm 1", font_size=28, color=PARTICLE_B)
        outcome_a.next_to(detector_a, DOWN, buff=0.5)
        outcome_b.next_to(detector_b, DOWN, buff=0.5)

        self.play(
            Write(outcome_a),
            Write(outcome_b),
            run_time=1
        )

        self.wait(2)

        # =====================================================================
        # SCENE 2: Classical Bound S ≤ 2 (25-50s)
        # =====================================================================

        # Clear setup, keep title
        setup_group = VGroup(
            source, source_label, particle_a, particle_b,
            detector_a, detector_b, angle_a, angle_b,
            angle_a_label, angle_b_label, outcome_a, outcome_b
        )

        self.play(
            setup_group.animate.scale(0.6).to_corner(UL, buff=0.5),
            run_time=1
        )

        # CHSH inequality
        chsh_title = Text("CHSH Inequality", font_size=32, color=CLASSICAL)
        chsh_title.to_edge(LEFT, buff=1).shift(UP * 1)

        self.play(Write(chsh_title), run_time=0.5)

        # Define S parameter
        s_def = MathTex(
            r"S = E(a,b) - E(a,b') + E(a',b) + E(a',b')",
            font_size=28
        )
        s_def.next_to(chsh_title, DOWN, buff=0.5, aligned_edge=LEFT)

        self.play(Write(s_def), run_time=2)

        # Classical bound
        classical_bound = MathTex(
            r"\text{Classical: } S \leq 2",
            font_size=36,
            color=CLASSICAL
        )
        classical_bound.next_to(s_def, DOWN, buff=0.5, aligned_edge=LEFT)

        self.play(Write(classical_bound), run_time=1)

        # Explain why
        explanation = VGroup(
            Text("Each particle carries definite values", font_size=20, color=WHITE),
            Text("Outcomes are pre-determined", font_size=20, color=WHITE),
            Text("No communication between detectors", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        explanation.next_to(classical_bound, DOWN, buff=0.5, aligned_edge=LEFT)

        for line in explanation:
            self.play(FadeIn(line, shift=RIGHT), run_time=0.6)

        # Visual: classical correlation graph
        ax = Axes(
            x_range=[0, 4, 1],
            y_range=[0, 3, 1],
            x_length=4,
            y_length=2.5,
            tips=False,
            axis_config={"color": WHITE}
        ).to_edge(RIGHT, buff=1).shift(DOWN * 0.5)

        ax_labels = ax.get_axis_labels(
            x_label=Text("Angle diff", font_size=16),
            y_label=Text("S", font_size=16)
        )

        # Classical bound line at S = 2
        classical_line = ax.plot(lambda x: 2, color=CLASSICAL, stroke_width=3)
        classical_label = MathTex(r"S = 2", font_size=20, color=CLASSICAL)
        classical_label.next_to(classical_line, RIGHT, buff=0.1)

        self.play(
            Create(ax),
            Write(ax_labels),
            run_time=1
        )
        self.play(
            Create(classical_line),
            Write(classical_label),
            run_time=1
        )

        self.wait(2)

        # =====================================================================
        # SCENE 3: Quantum Prediction S = 2√2 (50-80s)
        # =====================================================================

        # Quantum bound
        quantum_bound = MathTex(
            r"\text{Quantum: } S = 2\sqrt{2} \approx 2.828",
            font_size=36,
            color=QUANTUM
        )
        quantum_bound.next_to(explanation, DOWN, buff=0.8, aligned_edge=LEFT)

        self.play(Write(quantum_bound), run_time=1.5)

        # Quantum line on graph
        quantum_line = ax.plot(lambda x: 2.828, color=QUANTUM, stroke_width=3)
        quantum_label = MathTex(r"S = 2\sqrt{2}", font_size=20, color=QUANTUM)
        quantum_label.next_to(quantum_line, RIGHT, buff=0.1)

        self.play(
            Create(quantum_line),
            Write(quantum_label),
            run_time=1
        )

        # Highlight the gap
        gap_arrow = DoubleArrow(
            ax.c2p(2, 2), ax.c2p(2, 2.828),
            color=YELLOW,
            buff=0
        )
        gap_label = Text("Violation!", font_size=18, color=YELLOW)
        gap_label.next_to(gap_arrow, LEFT, buff=0.1)

        self.play(
            Create(gap_arrow),
            Write(gap_label),
            Flash(gap_arrow, color=YELLOW),
            run_time=1
        )

        # The paradox
        paradox = VGroup(
            Text("The Paradox:", font_size=24, color=YELLOW, weight=BOLD),
            Text("Experiments confirm S ≈ 2.83", font_size=20, color=WHITE),
            Text("But particles are space-like separated!", font_size=20, color=WHITE),
            Text("No signal can explain the correlation", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        paradox.next_to(quantum_bound, DOWN, buff=0.5, aligned_edge=LEFT)

        self.play(FadeIn(paradox), run_time=2)

        self.wait(3)

        # =====================================================================
        # SCENE 4: The sLoop Insight (80-110s)
        # =====================================================================

        # Clear previous
        self.play(
            FadeOut(VGroup(
                chsh_title, s_def, classical_bound, explanation,
                quantum_bound, paradox, ax, ax_labels,
                classical_line, classical_label, quantum_line, quantum_label,
                gap_arrow, gap_label
            )),
            setup_group.animate.scale(1/0.6).move_to(ORIGIN),
            run_time=1
        )

        # sLoop title
        sloop_title = Text("The sLoop Resolution", font_size=36, color=SLOOP)
        sloop_title.to_edge(UP, buff=1.5)
        self.play(Write(sloop_title), run_time=1)

        # Draw the flux substrate
        substrate = Rectangle(
            width=12, height=4,
            fill_color=SLOOP, fill_opacity=0.1,
            stroke_color=SLOOP, stroke_width=2
        )
        substrate_label = Text("FLUX SUBSTRATE", font_size=20, color=SLOOP)
        substrate_label.next_to(substrate, UP, buff=0.1)

        self.play(
            FadeIn(substrate),
            Write(substrate_label),
            run_time=1
        )

        # Show particles AND detectors embedded in substrate
        embedded_text = VGroup(
            Text("Particles", font_size=18, color=PARTICLE_A),
            Text("AND", font_size=18, color=WHITE),
            Text("Detectors", font_size=18, color=DETECTOR),
            Text("share the same substrate", font_size=18, color=WHITE),
        ).arrange(RIGHT, buff=0.2)
        embedded_text.next_to(substrate, DOWN, buff=0.3)

        self.play(FadeIn(embedded_text), run_time=1)

        # Animate flux connections
        flux_lines = VGroup()
        for _ in range(5):
            start = LEFT * 3 + UP * np.random.uniform(-1, 1)
            end = RIGHT * 3 + UP * np.random.uniform(-1, 1)
            line = Line(start, end, color=QUANTUM, stroke_opacity=0.3, stroke_width=1)
            flux_lines.add(line)

        self.play(
            *[Create(line) for line in flux_lines],
            run_time=1
        )

        # Key insight box
        insight_box = VGroup(
            Text("Key Insight:", font_size=24, color=QUANTUM, weight=BOLD),
            Text("Correlations are INHERITED", font_size=20, color=WHITE),
            Text("from shared substrate origin", font_size=20, color=WHITE),
            Text("NOT transmitted between detectors", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        insight_box.to_edge(LEFT, buff=0.5).shift(DOWN * 1)

        box_rect = SurroundingRectangle(insight_box, color=QUANTUM, buff=0.2)

        self.play(
            FadeIn(insight_box),
            Create(box_rect),
            run_time=2
        )

        # Contrast with standard QM
        contrast = VGroup(
            Text("Standard QM: Nonlocal collapse", font_size=18, color=CLASSICAL),
            Text("sLoop: Local inheritance", font_size=18, color=SLOOP),
        ).arrange(DOWN, buff=0.3, aligned_edge=LEFT)
        contrast.to_edge(RIGHT, buff=0.5).shift(DOWN * 1)

        self.play(FadeIn(contrast), run_time=1)

        self.wait(3)

        # =====================================================================
        # SCENE 5: FTD Simulation Results (110-140s)
        # =====================================================================

        # Clear and show results
        self.play(
            FadeOut(VGroup(
                setup_group, substrate, substrate_label, embedded_text,
                flux_lines, insight_box, box_rect, contrast, sloop_title
            )),
            run_time=1
        )

        results_title = Text("FTD Simulation Results", font_size=36, color=QUANTUM)
        results_title.to_edge(UP, buff=0.5)
        self.play(Write(results_title), run_time=1)

        # Create results graph
        ax2 = Axes(
            x_range=[0, 1, 0.2],
            y_range=[1.5, 3, 0.5],
            x_length=8,
            y_length=4,
            tips=False,
            axis_config={"color": WHITE}
        )
        ax2_labels = ax2.get_axis_labels(
            x_label=Text("Substrate Overlap f", font_size=20),
            y_label=Text("S parameter", font_size=20)
        )

        self.play(Create(ax2), Write(ax2_labels), run_time=1)

        # Classical and quantum bounds
        classical_ref = ax2.plot(lambda x: 2, color=CLASSICAL, stroke_width=2)
        quantum_ref = ax2.plot(lambda x: 2.828, color=QUANTUM, stroke_width=2)

        cl_label = Text("Classical S=2", font_size=16, color=CLASSICAL)
        qu_label = Text("Quantum S=2√2", font_size=16, color=QUANTUM)
        cl_label.next_to(ax2.c2p(0.9, 2), UP, buff=0.1)
        qu_label.next_to(ax2.c2p(0.9, 2.828), UP, buff=0.1)

        self.play(
            Create(classical_ref), Write(cl_label),
            Create(quantum_ref), Write(qu_label),
            run_time=1
        )

        # FTD simulation curve: S scales from ~1.95 to ~2.85 with overlap
        def s_curve(f):
            return 1.95 + 0.9 * f  # Linear approximation of simulation results

        ftd_curve = ax2.plot(s_curve, color=SLOOP, stroke_width=4)
        ftd_label = Text("FTD sLoop", font_size=18, color=SLOOP, weight=BOLD)
        ftd_label.next_to(ax2.c2p(0.5, s_curve(0.5)), UR, buff=0.1)

        self.play(Create(ftd_curve), Write(ftd_label), run_time=2)

        # Data points
        points = [
            (0, 1.95), (0.2, 2.13), (0.4, 2.31),
            (0.6, 2.49), (0.8, 2.67), (1.0, 2.85)
        ]

        dots = VGroup()
        for f, s in points:
            dot = Dot(ax2.c2p(f, s), radius=0.08, color=SLOOP)
            dots.add(dot)

        self.play(*[FadeIn(d, scale=0.5) for d in dots], run_time=1)

        # Key result
        key_result = VGroup(
            MathTex(r"f = 0 \text{ (no overlap): } S \approx 1.95", font_size=24),
            MathTex(r"f = 1 \text{ (full overlap): } S \approx 2.85", font_size=24),
            MathTex(r"\text{Matches } 2\sqrt{2} = 2.828!", font_size=28, color=QUANTUM),
        ).arrange(DOWN, buff=0.3)
        key_result.to_edge(DOWN, buff=0.5)

        self.play(FadeIn(key_result), run_time=2)

        # Flash the match
        self.play(
            Flash(key_result[2], color=QUANTUM, flash_radius=0.8),
            run_time=0.5
        )

        self.wait(3)

        # =====================================================================
        # SCENE 6: Conclusion (140-150s)
        # =====================================================================

        self.play(
            FadeOut(VGroup(
                results_title, ax2, ax2_labels, classical_ref, quantum_ref,
                cl_label, qu_label, ftd_curve, ftd_label, dots, key_result
            )),
            run_time=1
        )

        # Final message
        conclusion = VGroup(
            Text("THE SLOOP MECHANISM", font_size=36, color=SLOOP, weight=BOLD),
            Text("", font_size=10),
            Text("Bell violations arise when:", font_size=24, color=WHITE),
            Text("• Particles AND apparatus share flux substrate", font_size=20, color=WHITE),
            Text("• Correlations are inherited from creation", font_size=20, color=WHITE),
            Text("• No superluminal signaling required", font_size=20, color=WHITE),
            Text("", font_size=10),
            MathTex(r"S \to 2\sqrt{2} \text{ as substrate overlap } \to 1",
                   font_size=28, color=QUANTUM),
        ).arrange(DOWN, buff=0.3)

        self.play(FadeIn(conclusion), run_time=3)
        self.wait(3)

        # Fade out
        self.play(FadeOut(conclusion), FadeOut(title_group), run_time=2)

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=28, color=QUANTUM),
            Text("The sLoop: Quantum correlations without nonlocality",
                 font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.3)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(2)

    def create_detector(self, name, position):
        """Create a detector with label."""
        box = Rectangle(width=0.8, height=1.2, color=DETECTOR, fill_opacity=0.3)
        box.move_to(position)
        label = Text(name, font_size=18, color=DETECTOR)
        label.next_to(box, UP, buff=0.1)
        return VGroup(box, label)


class BellInequalitySimple(Scene):
    """A simpler version showing just the key concepts."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # Title
        title = Text("Bell Inequality: The Key Insight", font_size=36, color=QUANTUM)
        title.to_edge(UP, buff=0.5)
        self.play(Write(title))

        # Classical vs Quantum comparison
        classical = VGroup(
            Text("CLASSICAL", font_size=28, color=CLASSICAL, weight=BOLD),
            MathTex(r"S \leq 2", font_size=48, color=CLASSICAL),
            Text("Pre-determined outcomes", font_size=20, color=WHITE),
            Text("Local hidden variables", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.3)
        classical.shift(LEFT * 3)

        quantum = VGroup(
            Text("QUANTUM", font_size=28, color=QUANTUM, weight=BOLD),
            MathTex(r"S = 2\sqrt{2}", font_size=48, color=QUANTUM),
            Text("Entangled correlations", font_size=20, color=WHITE),
            Text("Experimentally confirmed", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.3)
        quantum.shift(RIGHT * 3)

        self.play(FadeIn(classical), FadeIn(quantum), run_time=2)

        # Arrow between them
        arrow = Arrow(classical.get_right(), quantum.get_left(), color=YELLOW)
        violation = Text("VIOLATION", font_size=24, color=YELLOW)
        violation.next_to(arrow, UP, buff=0.1)

        self.play(Create(arrow), Write(violation), run_time=1)
        self.wait(1)

        # sLoop resolution
        sloop_box = VGroup(
            Text("FTD Resolution: The sLoop", font_size=28, color=SLOOP, weight=BOLD),
            Text("Observer embedded in same substrate as particles", font_size=20, color=WHITE),
            Text("Correlations inherited, not transmitted", font_size=20, color=WHITE),
            MathTex(r"S \to 2\sqrt{2} \text{ (reproduced!)", font_size=24, color=SLOOP),
        ).arrange(DOWN, buff=0.3)
        sloop_box.to_edge(DOWN, buff=1)

        box_rect = SurroundingRectangle(sloop_box, color=SLOOP, buff=0.3)

        self.play(
            FadeIn(sloop_box),
            Create(box_rect),
            run_time=2
        )

        self.wait(3)


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pql scene_10_bell_sloop.py BellSLoopScene")
    print("Or simpler version:")
    print("  manim -pql scene_10_bell_sloop.py BellInequalitySimple")
