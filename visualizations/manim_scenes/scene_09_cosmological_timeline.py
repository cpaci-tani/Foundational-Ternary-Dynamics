"""
Simulation 9: Cosmological Timeline - From Void to Stars
========================================================
A 180-second Manim animation showing FTD predictions for the history of the universe.

This is one of the most visually compelling animations - showing the entire
cosmic evolution from the primordial void through inflation, baryogenesis,
nucleosynthesis, to the present day.

Timeline:
- t = 0: The Void — pure potential, no space, no time
- t ~ 10⁻⁴³ s: Planck epoch — lattice structure emerges
- t ~ 10⁻³⁶ s: Inflation — n_s = 0.9645, r = 0.0219
- t ~ 10⁻¹² s: Baryogenesis — η ~ 6.7×10⁻¹⁰
- t ~ 380,000 yr: CMB release
- t ~ 13.8 Gyr: Present day — large-scale structure

Run with: manim -pqh scene_09_cosmological_timeline.py CosmologicalTimelineScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors
BACKGROUND = "#0D1117"
VOID_COLOR = "#888888"
FLUX_COLOR = "#FFD700"
MATTER_COLOR = "#DD4444"
ANTIMATTER_COLOR = "#4488DD"
INFLATION_COLOR = "#9B59B6"
CMB_COLOR = "#F39C12"
STAR_COLOR = "#FFEAA7"
HIGHLIGHT = "#FFD700"


class CosmologicalTimelineScene(Scene):
    """The complete cosmic history from FTD perspective."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # TITLE
        # =====================================================================

        title = Text("The Cosmic Timeline", font_size=48, color=HIGHLIGHT)
        subtitle = Text("From Void to Stars", font_size=28, color=WHITE)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=2)
        self.play(FadeIn(subtitle), run_time=1)
        self.wait(1)
        self.play(FadeOut(title_group), run_time=1)

        # =====================================================================
        # EPOCH 1: THE VOID (t = 0)
        # =====================================================================

        epoch_label = self.create_epoch_label("t = 0", "The Void")
        self.play(Write(epoch_label), run_time=1)

        # Pure black with subtle pulse
        void_text = Text(
            "Pure potential\nNo space, no time\nOnly the substrate awaits",
            font_size=28,
            color=VOID_COLOR,
            line_spacing=1.5
        )

        self.play(FadeIn(void_text, scale=0.8), run_time=2)

        # Subtle breathing animation
        for _ in range(2):
            self.play(void_text.animate.set_opacity(0.5), run_time=0.8)
            self.play(void_text.animate.set_opacity(1.0), run_time=0.8)

        self.play(FadeOut(void_text), FadeOut(epoch_label), run_time=1)

        # =====================================================================
        # EPOCH 2: PLANCK EPOCH (t ~ 10⁻⁴³ s)
        # =====================================================================

        epoch_label = self.create_epoch_label("t ~ 10⁻⁴³ s", "Planck Epoch")
        self.play(Write(epoch_label), run_time=1)

        # Lattice emerges
        lattice_text = Text("The discrete lattice crystallizes", font_size=24, color=WHITE)
        lattice_text.next_to(epoch_label, DOWN, buff=0.5)
        self.play(Write(lattice_text), run_time=1)

        # Create a grid of dots representing the lattice
        lattice = VGroup()
        for i in range(-4, 5):
            for j in range(-3, 4):
                dot = Dot(
                    point=np.array([i * 0.5, j * 0.5 - 1, 0]),
                    radius=0.05,
                    color=VOID_COLOR
                )
                lattice.add(dot)

        self.play(
            LaggedStart(*[FadeIn(d, scale=0.5) for d in lattice], lag_ratio=0.02),
            run_time=2
        )

        # Constants appear
        constants = MathTex(
            r"\ell_P = 1.6 \times 10^{-35} \text{ m}",
            font_size=28,
            color=FLUX_COLOR
        )
        constants.to_edge(DOWN, buff=1)
        self.play(Write(constants), run_time=1)

        self.wait(1)
        self.play(FadeOut(lattice), FadeOut(lattice_text), FadeOut(constants), FadeOut(epoch_label))

        # =====================================================================
        # EPOCH 3: INFLATION (t ~ 10⁻³⁶ s)
        # =====================================================================

        epoch_label = self.create_epoch_label("t ~ 10⁻³⁶ s", "Cosmic Inflation")
        self.play(Write(epoch_label), run_time=1)

        # Inflation explanation
        inflation_text = Text(
            "Sub-threshold flux drives exponential expansion",
            font_size=22,
            color=WHITE
        )
        inflation_text.next_to(epoch_label, DOWN, buff=0.3)
        self.play(Write(inflation_text), run_time=1)

        # Create expanding circle representing inflation
        inflation_circle = Circle(radius=0.3, color=INFLATION_COLOR, fill_opacity=0.3)
        inflation_circle.shift(DOWN * 1)

        self.play(FadeIn(inflation_circle), run_time=0.5)

        # Exponential expansion
        self.play(
            inflation_circle.animate.scale(10).set_opacity(0.1),
            run_time=3,
            rate_func=rate_functions.exponential_decay
        )

        # FTD predictions
        predictions = VGroup(
            MathTex(r"n_s = 0.9645", font_size=32, color=HIGHLIGHT),
            Text("(Planck: 0.9649 ± 0.0042)", font_size=20, color=GRAY_B),
            MathTex(r"r = 0.0219", font_size=32, color=HIGHLIGHT),
            Text("(Below current bounds)", font_size=20, color=GRAY_B),
        ).arrange(DOWN, buff=0.2)
        predictions.shift(DOWN * 0.5)

        self.play(FadeIn(predictions), run_time=2)

        # Highlight agreement
        check1 = MathTex(r"\checkmark", font_size=36, color=GREEN)
        check1.next_to(predictions[0], RIGHT, buff=0.3)
        self.play(Write(check1), run_time=0.5)

        self.wait(1)
        self.play(
            FadeOut(predictions), FadeOut(check1),
            FadeOut(inflation_circle), FadeOut(inflation_text),
            FadeOut(epoch_label)
        )

        # =====================================================================
        # EPOCH 4: BARYOGENESIS (t ~ 10⁻¹² s)
        # =====================================================================

        epoch_label = self.create_epoch_label("t ~ 10⁻¹² s", "Baryogenesis")
        self.play(Write(epoch_label), run_time=1)

        # Matter-antimatter asymmetry
        baryo_text = Text(
            "CP violation creates matter excess",
            font_size=24,
            color=WHITE
        )
        baryo_text.next_to(epoch_label, DOWN, buff=0.3)
        self.play(Write(baryo_text), run_time=1)

        # Create matter and antimatter particles
        matter_particles = VGroup(*[
            Dot(
                point=np.random.randn(3) * 0.8 + np.array([-2, -1, 0]),
                radius=0.1,
                color=MATTER_COLOR
            ) for _ in range(15)
        ])

        antimatter_particles = VGroup(*[
            Dot(
                point=np.random.randn(3) * 0.8 + np.array([2, -1, 0]),
                radius=0.1,
                color=ANTIMATTER_COLOR
            ) for _ in range(14)  # One fewer!
        ])

        matter_label = Text("Matter", font_size=20, color=MATTER_COLOR)
        matter_label.next_to(matter_particles, DOWN, buff=0.3)
        antimatter_label = Text("Antimatter", font_size=20, color=ANTIMATTER_COLOR)
        antimatter_label.next_to(antimatter_particles, DOWN, buff=0.3)

        self.play(
            LaggedStart(*[FadeIn(p, scale=0.5) for p in matter_particles], lag_ratio=0.05),
            LaggedStart(*[FadeIn(p, scale=0.5) for p in antimatter_particles], lag_ratio=0.05),
            Write(matter_label),
            Write(antimatter_label),
            run_time=2
        )

        # Show the asymmetry
        ratio = MathTex(
            r"\eta = \frac{n_B - n_{\bar{B}}}{n_\gamma} \sim 6.7 \times 10^{-10}",
            font_size=28,
            color=HIGHLIGHT
        )
        ratio.to_edge(DOWN, buff=0.8)
        self.play(Write(ratio), run_time=1)

        # CP phase
        cp_phase = MathTex(
            r"\delta_{CP} = \arctan(7/3) = 66.8°",
            font_size=24,
            color=FLUX_COLOR
        )
        cp_phase.next_to(ratio, UP, buff=0.3)
        self.play(Write(cp_phase), run_time=1)

        self.wait(1)
        self.play(
            FadeOut(matter_particles), FadeOut(antimatter_particles),
            FadeOut(matter_label), FadeOut(antimatter_label),
            FadeOut(ratio), FadeOut(cp_phase), FadeOut(baryo_text),
            FadeOut(epoch_label)
        )

        # =====================================================================
        # EPOCH 5: CMB RELEASE (t ~ 380,000 yr)
        # =====================================================================

        epoch_label = self.create_epoch_label("t ~ 380,000 years", "CMB Release")
        self.play(Write(epoch_label), run_time=1)

        # CMB description
        cmb_text = Text(
            "Universe becomes transparent\nPhotons decouple from matter",
            font_size=22,
            color=WHITE,
            line_spacing=1.3
        )
        cmb_text.next_to(epoch_label, DOWN, buff=0.3)
        self.play(Write(cmb_text), run_time=1)

        # Create CMB-like pattern (simplified)
        cmb_pattern = VGroup()
        np.random.seed(42)
        for i in range(200):
            x = np.random.uniform(-6, 6)
            y = np.random.uniform(-2.5, 1)
            temp_variation = np.random.uniform(-1, 1)
            color = interpolate_color(
                Color(ANTIMATTER_COLOR),
                Color(MATTER_COLOR),
                (temp_variation + 1) / 2
            )
            dot = Dot(point=np.array([x, y, 0]), radius=0.08, color=color)
            dot.set_opacity(0.6)
            cmb_pattern.add(dot)

        self.play(
            LaggedStart(*[FadeIn(d) for d in cmb_pattern], lag_ratio=0.01),
            run_time=2
        )

        # Temperature
        temp = MathTex(r"T = 2.725 \text{ K}", font_size=28, color=CMB_COLOR)
        temp.to_edge(DOWN, buff=1)
        self.play(Write(temp), run_time=1)

        self.wait(1)
        self.play(
            FadeOut(cmb_pattern), FadeOut(cmb_text),
            FadeOut(temp), FadeOut(epoch_label)
        )

        # =====================================================================
        # EPOCH 6: PRESENT DAY (t ~ 13.8 Gyr)
        # =====================================================================

        epoch_label = self.create_epoch_label("t ~ 13.8 billion years", "Present Day")
        self.play(Write(epoch_label), run_time=1)

        # Stars and galaxies
        present_text = Text(
            "Stars, galaxies, and observers",
            font_size=24,
            color=WHITE
        )
        present_text.next_to(epoch_label, DOWN, buff=0.3)
        self.play(Write(present_text), run_time=1)

        # Create starfield
        stars = VGroup()
        np.random.seed(123)
        for _ in range(100):
            x = np.random.uniform(-7, 7)
            y = np.random.uniform(-3, 2)
            size = np.random.uniform(0.02, 0.08)
            brightness = np.random.uniform(0.3, 1.0)
            star = Dot(
                point=np.array([x, y, 0]),
                radius=size,
                color=STAR_COLOR
            ).set_opacity(brightness)
            stars.add(star)

        # Add a spiral galaxy
        galaxy = self.create_spiral_galaxy()
        galaxy.scale(0.8).shift(DOWN * 0.5)

        self.play(
            LaggedStart(*[FadeIn(s, scale=0.5) for s in stars], lag_ratio=0.01),
            run_time=1.5
        )
        self.play(FadeIn(galaxy, scale=0.5), run_time=1)

        # The observer
        observer_text = Text(
            "And observers who can derive\nthe laws that created them",
            font_size=22,
            color=HIGHLIGHT,
            line_spacing=1.3
        )
        observer_text.to_edge(DOWN, buff=0.8)
        self.play(Write(observer_text), run_time=2)

        self.wait(2)

        # =====================================================================
        # FINALE: The Complete Timeline
        # =====================================================================

        self.play(
            FadeOut(stars), FadeOut(galaxy),
            FadeOut(present_text), FadeOut(observer_text),
            FadeOut(epoch_label)
        )

        # Show complete timeline
        timeline = self.create_timeline()
        self.play(FadeIn(timeline), run_time=2)

        # Final message
        final = Text(
            "From Void to Cosmos — all from {3, 4, 7, 13}",
            font_size=28,
            color=HIGHLIGHT
        )
        final.to_edge(DOWN, buff=0.5)
        self.play(Write(final), run_time=2)

        self.wait(2)

        # Fade out
        self.play(*[FadeOut(mob) for mob in self.mobjects], run_time=2)

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=32, color=HIGHLIGHT),
            Text("The Cosmic Timeline", font_size=28, color=WHITE),
            MathTex(r"n_s = 0.9645, \; \eta \sim 10^{-10}", font_size=24),
        ).arrange(DOWN, buff=0.4)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(3)

    def create_epoch_label(self, time, name):
        """Create a styled epoch label."""
        time_tex = MathTex(time, font_size=32, color=FLUX_COLOR)
        name_text = Text(name, font_size=36, color=WHITE, weight=BOLD)
        group = VGroup(time_tex, name_text).arrange(DOWN, buff=0.2)
        group.to_edge(UP, buff=0.5)
        return group

    def create_spiral_galaxy(self):
        """Create a simple spiral galaxy shape."""
        galaxy = VGroup()

        # Central bulge
        bulge = Dot(ORIGIN, radius=0.3, color=STAR_COLOR).set_opacity(0.8)
        galaxy.add(bulge)

        # Spiral arms
        for arm in range(2):
            arm_offset = arm * PI
            for i in range(50):
                t = i * 0.15
                r = 0.3 + t * 0.15
                theta = t * 0.8 + arm_offset
                x = r * np.cos(theta)
                y = r * np.sin(theta) * 0.4  # Flatten

                star = Dot(
                    point=np.array([x, y, 0]),
                    radius=0.03 * (1 - i/60),
                    color=STAR_COLOR
                ).set_opacity(0.6 * (1 - i/60))
                galaxy.add(star)

        return galaxy

    def create_timeline(self):
        """Create a visual timeline showing all epochs."""
        timeline = VGroup()

        # Main line
        line = Line(LEFT * 6, RIGHT * 6, color=WHITE, stroke_width=2)
        timeline.add(line)

        # Epochs
        epochs = [
            ("0", "Void", -5.5),
            ("10⁻⁴³s", "Planck", -3.5),
            ("10⁻³⁶s", "Inflation", -1.5),
            ("10⁻¹²s", "Baryogenesis", 0.5),
            ("380ky", "CMB", 2.5),
            ("13.8Gy", "Now", 5),
        ]

        for time, name, x in epochs:
            dot = Dot(point=np.array([x, 0, 0]), radius=0.1, color=FLUX_COLOR)
            time_label = Text(time, font_size=14, color=GRAY_B)
            time_label.next_to(dot, DOWN, buff=0.15)
            name_label = Text(name, font_size=16, color=WHITE)
            name_label.next_to(dot, UP, buff=0.15)
            timeline.add(dot, time_label, name_label)

        timeline.shift(UP * 0.5)
        return timeline


class InflationDemo(Scene):
    """A focused demo of the inflation predictions."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        title = Text("FTD Inflation Predictions", font_size=36, color=HIGHLIGHT)
        title.to_edge(UP)
        self.play(Write(title))

        # The key predictions
        predictions = VGroup(
            VGroup(
                MathTex(r"n_s = 1 - \frac{2}{N_e + 1}", font_size=32),
                MathTex(r"= 1 - \frac{2}{56} = 0.9643", font_size=28, color=HIGHLIGHT),
            ).arrange(DOWN, buff=0.2),
            VGroup(
                Text("Planck 2018:", font_size=20, color=GRAY_B),
                MathTex(r"n_s = 0.9649 \pm 0.0042", font_size=24),
            ).arrange(DOWN, buff=0.1),
        ).arrange(RIGHT, buff=2)

        self.play(Write(predictions), run_time=3)

        # Agreement indicator
        agreement = Text("Agreement: 0.10σ", font_size=24, color=GREEN)
        agreement.next_to(predictions, DOWN, buff=0.5)
        self.play(Write(agreement), Flash(agreement, color=GREEN), run_time=1)

        self.wait(2)


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pqh scene_09_cosmological_timeline.py CosmologicalTimelineScene")
    print("Or focused version:")
    print("  manim -pql scene_09_cosmological_timeline.py InflationDemo")
