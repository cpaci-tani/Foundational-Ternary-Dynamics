"""
Simulation 24: THE GRAND SYNTHESIS
==================================
A 5-minute (300-second) epic Manim animation that weaves together
the entire FTD story - from the Void through all of physics to
conscious observers and the sLoop closure.

This is the capstone visualization - the complete Theory of Everything
in one breathtaking animation.

Storyboard:
1. (0-30s) THE VOID - Before manifestation
2. (30-60s) FIRST DIVISION - Three states emerge
3. (60-90s) THE FOUR INTEGERS - The foundation
4. (90-120s) FORCES EMERGE - From geometry
5. (120-150s) PARTICLES MANIFEST - The Standard Model
6. (150-180s) ATOMS AND MOLECULES - Chemistry emerges
7. (180-210s) STARS AND GALAXIES - Cosmic structure
8. (210-240s) OBSERVERS ARISE - Consciousness
9. (240-270s) THE SLOOP CLOSES - Self-reference
10. (270-300s) FINALE - Full circle to Void

Run with: manim -pql scene_24_grand_synthesis.py GrandSynthesisScene
For high quality (recommended): manim -pqh scene_24_grand_synthesis.py GrandSynthesisScene
For 4K: manim -qk scene_24_grand_synthesis.py GrandSynthesisScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors - the complete FTD palette
BACKGROUND = "#0D1117"
VOID = "#888888"
MATTER = "#DD4444"
ANTIMATTER = "#4488DD"
FLUX = "#FFD700"
STRONG = "#FF6B35"
WEAK = "#9B59B6"
EM = "#3498DB"
GRAVITY = "#27AE60"
CONSCIOUSNESS = "#FF69B4"
SLOOP = "#00CED1"


class GrandSynthesisScene(Scene):
    """The complete FTD story in one epic animation."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # PROLOGUE: Title Sequence (0-10s)
        # =====================================================================

        main_title = Text(
            "FOUNDATIONAL TERNARY DYNAMICS",
            font_size=48,
            color=FLUX,
            weight=BOLD
        )
        subtitle = Text(
            "A Theory of Everything",
            font_size=32,
            color=WHITE
        )
        title_group = VGroup(main_title, subtitle).arrange(DOWN, buff=0.5)

        self.play(Write(main_title), run_time=3)
        self.play(FadeIn(subtitle), run_time=2)
        self.wait(2)
        self.play(FadeOut(title_group), run_time=2)

        # =====================================================================
        # CHAPTER 1: THE VOID (10-30s)
        # =====================================================================

        chapter_1 = self.create_chapter_title("I", "THE VOID", "Before manifestation, there is potential")
        self.play(FadeIn(chapter_1), run_time=1)
        self.wait(1)
        self.play(chapter_1.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Empty void - just darkness with subtle noise
        void_text = Text("Nothing... yet everything possible", font_size=24, color=VOID)
        self.play(FadeIn(void_text), run_time=2)

        # Subtle pulse
        void_pulse = Circle(radius=0.5, color=VOID, fill_opacity=0.1)
        self.play(
            void_pulse.animate.scale(3).set_opacity(0),
            run_time=3
        )

        self.play(FadeOut(void_text), FadeOut(chapter_1), run_time=1)

        # =====================================================================
        # CHAPTER 2: FIRST DIVISION (30-60s)
        # =====================================================================

        chapter_2 = self.create_chapter_title("II", "THE FIRST DIVISION", "From unity, three states emerge")
        self.play(FadeIn(chapter_2), run_time=1)
        self.wait(1)
        self.play(chapter_2.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Single voxel
        voxel = Square(side_length=1, color=VOID, fill_opacity=0.5)
        state_label = MathTex(r"s = 0", font_size=24, color=VOID)
        state_label.next_to(voxel, DOWN, buff=0.3)

        self.play(FadeIn(voxel), Write(state_label), run_time=1)

        # Flux builds up
        flux_arrow = Arrow(ORIGIN, UP * 0.8, color=FLUX, stroke_width=4)
        flux_label = MathTex(r"|\mathbf{J}| > K_B", font_size=20, color=FLUX)
        flux_label.next_to(flux_arrow, RIGHT, buff=0.2)

        self.play(Create(flux_arrow), Write(flux_label), run_time=1.5)

        # SPLIT!
        self.play(
            Flash(voxel, color=FLUX, flash_radius=1.5),
            run_time=0.5
        )

        # Two voxels emerge
        matter_voxel = Square(side_length=0.8, color=MATTER, fill_opacity=0.8)
        antimatter_voxel = Square(side_length=0.8, color=ANTIMATTER, fill_opacity=0.8)

        matter_label = MathTex(r"s = +1", font_size=20, color=MATTER)
        antimatter_label = MathTex(r"s = -1", font_size=20, color=ANTIMATTER)

        matter_voxel.shift(LEFT * 2)
        antimatter_voxel.shift(RIGHT * 2)
        matter_label.next_to(matter_voxel, DOWN, buff=0.2)
        antimatter_label.next_to(antimatter_voxel, DOWN, buff=0.2)

        self.play(
            ReplacementTransform(voxel, VGroup(matter_voxel, antimatter_voxel)),
            FadeOut(state_label),
            FadeOut(flux_arrow),
            FadeOut(flux_label),
            run_time=1
        )
        self.play(
            Write(matter_label), Write(antimatter_label),
            run_time=1
        )

        # Three states summary
        three_states = MathTex(
            r"s(v,t) \in \{-1, 0, +1\}",
            font_size=36, color=FLUX
        )
        three_states.to_edge(DOWN, buff=1)

        self.play(Write(three_states), run_time=1)
        self.wait(2)

        self.play(
            FadeOut(VGroup(matter_voxel, antimatter_voxel,
                          matter_label, antimatter_label, three_states, chapter_2)),
            run_time=1
        )

        # =====================================================================
        # CHAPTER 3: THE FOUR INTEGERS (60-90s)
        # =====================================================================

        chapter_3 = self.create_chapter_title("III", "THE FOUR INTEGERS", "The foundation of all physics")
        self.play(FadeIn(chapter_3), run_time=1)
        self.wait(1)
        self.play(chapter_3.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Four integers orbiting
        integers_data = [
            (3, "#E74C3C", "N_c", "Color charges"),
            (4, "#F39C12", "N_{base}", "Fermat boundary"),
            (7, "#9B59B6", "b_3", "QCD beta"),
            (13, "#3498DB", "n_{eff}", "Effective DoF"),
        ]

        integer_mobjects = VGroup()
        for i, (val, color, symbol, meaning) in enumerate(integers_data):
            angle = i * TAU / 4 + TAU / 8
            pos = 2 * np.array([np.cos(angle), np.sin(angle), 0])

            circle = Circle(radius=0.6, color=color, fill_opacity=0.3, stroke_width=3)
            number = Text(str(val), font_size=48, color=color, weight=BOLD)
            label = MathTex(symbol, font_size=20, color=WHITE)
            label.next_to(circle, DOWN, buff=0.1)

            group = VGroup(circle, number, label)
            group.move_to(pos)
            integer_mobjects.add(group)

        self.play(
            *[FadeIn(mob, scale=0.5) for mob in integer_mobjects],
            run_time=2
        )

        # Connect them
        connections = VGroup()
        for i in range(4):
            for j in range(i+1, 4):
                line = Line(
                    integer_mobjects[i].get_center(),
                    integer_mobjects[j].get_center(),
                    color=FLUX, stroke_opacity=0.3, stroke_width=2
                )
                connections.add(line)

        self.play(Create(connections), run_time=1)

        # The constraint equation
        constraint = MathTex(
            r"n_{eff} = b_3 + 2N_c",
            font_size=36, color=FLUX
        )
        constraint.to_edge(DOWN, buff=1)

        self.play(Write(constraint), run_time=1)

        # Verify
        verify = MathTex(
            r"13 = 7 + 2 \times 3 \; \checkmark",
            font_size=28, color=GRAVITY
        )
        verify.next_to(constraint, DOWN, buff=0.3)

        self.play(
            Write(verify),
            Flash(verify, color=GRAVITY),
            run_time=1
        )

        self.wait(2)
        self.play(
            FadeOut(VGroup(integer_mobjects, connections, constraint, verify, chapter_3)),
            run_time=1
        )

        # =====================================================================
        # CHAPTER 4: FORCES EMERGE (90-120s)
        # =====================================================================

        chapter_4 = self.create_chapter_title("IV", "FORCES EMERGE", "From geometry alone")
        self.play(FadeIn(chapter_4), run_time=1)
        self.wait(1)
        self.play(chapter_4.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Four forces appearing
        forces_data = [
            ("Strong", STRONG, r"SU(3)", "3 dimensions"),
            ("Electromagnetic", EM, r"U(1)", "Gauss constraint"),
            ("Weak", WEAK, r"SU(2)", "Ternary states"),
            ("Gravity", GRAVITY, r"g_{\mu\nu}", "Flux curvature"),
        ]

        forces_vgroup = VGroup()
        for i, (name, color, sym, origin) in enumerate(forces_data):
            y_pos = 1.5 - i * 1

            force_name = Text(name, font_size=24, color=color, weight=BOLD)
            force_sym = MathTex(sym, font_size=28, color=color)
            force_origin = Text(f"← {origin}", font_size=18, color=WHITE)

            force_name.move_to(LEFT * 3 + UP * y_pos)
            force_sym.move_to(ORIGIN + UP * y_pos)
            force_origin.move_to(RIGHT * 2.5 + UP * y_pos)

            row = VGroup(force_name, force_sym, force_origin)
            forces_vgroup.add(row)

        for row in forces_vgroup:
            self.play(FadeIn(row, shift=RIGHT), run_time=0.7)

        # The hierarchy
        hierarchy = MathTex(
            r"\alpha_G = 2\pi\left(\frac{16}{3}\right)^2\left(13 + \frac{3}{7}\right)^2\alpha^{20}",
            font_size=28, color=FLUX
        )
        hierarchy.to_edge(DOWN, buff=0.8)

        self.play(Write(hierarchy), run_time=1.5)

        self.wait(2)
        self.play(FadeOut(VGroup(forces_vgroup, hierarchy, chapter_4)), run_time=1)

        # =====================================================================
        # CHAPTER 5: PARTICLES MANIFEST (120-150s)
        # =====================================================================

        chapter_5 = self.create_chapter_title("V", "PARTICLES MANIFEST", "The Standard Model from 4 integers")
        self.play(FadeIn(chapter_5), run_time=1)
        self.wait(1)
        self.play(chapter_5.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Particle grid
        particles = [
            ("e", MATTER, -3), ("μ", MATTER, -1), ("τ", MATTER, 0.25),
            ("u", "#E74C3C", -3), ("c", "#E74C3C", 0.1), ("t", "#E74C3C", 2.24),
            ("d", "#F39C12", -2.3), ("s", "#F39C12", -1), ("b", "#F39C12", 0.62),
        ]

        particle_dots = VGroup()
        for i, (name, color, log_mass) in enumerate(particles):
            x = (i % 3 - 1) * 2
            y = (i // 3 - 1) * 1.5

            dot = Dot(radius=0.2 + abs(log_mass) * 0.05, color=color)
            dot.move_to([x, y, 0])
            label = MathTex(name, font_size=20, color=WHITE)
            label.next_to(dot, DOWN, buff=0.1)

            particle_dots.add(VGroup(dot, label))

        self.play(
            *[FadeIn(p, scale=0.5) for p in particle_dots],
            run_time=2
        )

        # Key mass prediction
        mass_pred = VGroup(
            MathTex(r"m_e = m_P \sqrt{2\pi} \cdot \frac{16}{3} \cdot \alpha^{11}", font_size=26),
            Text("Accuracy: 0.27%", font_size=18, color=GRAVITY),
        ).arrange(DOWN, buff=0.2)
        mass_pred.to_edge(DOWN, buff=0.8)

        self.play(Write(mass_pred[0]), run_time=1)
        self.play(FadeIn(mass_pred[1]), run_time=0.5)

        self.wait(2)
        self.play(FadeOut(VGroup(particle_dots, mass_pred, chapter_5)), run_time=1)

        # =====================================================================
        # CHAPTER 6: ATOMS AND MOLECULES (150-180s)
        # =====================================================================

        chapter_6 = self.create_chapter_title("VI", "ATOMS AND MOLECULES", "Chemistry from quantum binding")
        self.play(FadeIn(chapter_6), run_time=1)
        self.wait(1)
        self.play(chapter_6.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Simple atom
        nucleus = Dot(radius=0.3, color=MATTER)
        electron_orbit = Circle(radius=1.5, color=EM, stroke_opacity=0.5)

        electrons = VGroup()
        for i in range(2):
            e = Dot(radius=0.1, color=ANTIMATTER)
            angle = i * PI
            e.move_to(1.5 * np.array([np.cos(angle), np.sin(angle), 0]))
            electrons.add(e)

        atom = VGroup(nucleus, electron_orbit, electrons)
        atom.shift(LEFT * 3)

        self.play(FadeIn(atom), run_time=1)

        # Rotate electrons
        self.play(
            Rotate(electrons, angle=TAU, about_point=atom[0].get_center()),
            run_time=2
        )

        # Molecule
        atom2 = atom.copy()
        atom2.shift(RIGHT * 6)

        self.play(FadeIn(atom2), run_time=0.5)

        # Bond
        bond = Line(
            atom.get_center() + RIGHT * 0.5,
            atom2.get_center() + LEFT * 0.5,
            color=FLUX, stroke_width=4
        )

        self.play(Create(bond), run_time=1)

        # DNA hint
        dna_text = Text("→ Proteins → DNA → Life", font_size=24, color=FLUX)
        dna_text.to_edge(DOWN, buff=1)

        self.play(Write(dna_text), run_time=1.5)

        self.wait(2)
        self.play(FadeOut(VGroup(atom, atom2, bond, dna_text, chapter_6)), run_time=1)

        # =====================================================================
        # CHAPTER 7: STARS AND GALAXIES (180-210s)
        # =====================================================================

        chapter_7 = self.create_chapter_title("VII", "STARS AND GALAXIES", "Cosmic structure emerges")
        self.play(FadeIn(chapter_7), run_time=1)
        self.wait(1)
        self.play(chapter_7.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Spiral galaxy
        galaxy = self.create_spiral_galaxy()
        galaxy.scale(0.7)

        self.play(FadeIn(galaxy), run_time=2)

        # Rotate
        self.play(
            Rotate(galaxy, angle=TAU/4, run_time=3),
        )

        # Cosmic web hint
        cosmic_text = VGroup(
            Text("Galaxies form cosmic web", font_size=22, color=WHITE),
            Text("Dark matter halos guide structure", font_size=18, color=VOID),
        ).arrange(DOWN, buff=0.2)
        cosmic_text.to_edge(DOWN, buff=0.8)

        self.play(FadeIn(cosmic_text), run_time=1)

        self.wait(2)
        self.play(FadeOut(VGroup(galaxy, cosmic_text, chapter_7)), run_time=1)

        # =====================================================================
        # CHAPTER 8: OBSERVERS ARISE (210-240s)
        # =====================================================================

        chapter_8 = self.create_chapter_title("VIII", "OBSERVERS ARISE", "Consciousness from complexity")
        self.play(FadeIn(chapter_8), run_time=1)
        self.wait(1)
        self.play(chapter_8.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Brain/observer representation
        observer = VGroup()

        # Central node (consciousness)
        center = Dot(radius=0.3, color=CONSCIOUSNESS)
        observer.add(center)

        # Neural-like connections
        for i in range(8):
            angle = i * TAU / 8
            outer = Dot(radius=0.1, color=SLOOP)
            outer.move_to(1.5 * np.array([np.cos(angle), np.sin(angle), 0]))
            line = Line(center.get_center(), outer.get_center(),
                       color=CONSCIOUSNESS, stroke_opacity=0.5)
            observer.add(line, outer)

        self.play(FadeIn(observer), run_time=1.5)

        # Pulsing consciousness
        pulse = Circle(radius=0.3, color=CONSCIOUSNESS, fill_opacity=0.3)
        for _ in range(2):
            self.play(
                pulse.animate.scale(5).set_opacity(0),
                rate_func=linear,
                run_time=1
            )
            pulse.scale(1/5).set_opacity(0.3)

        # Consciousness quadratic
        consciousness_eq = MathTex(
            r"y = 2.19 \pm 1.30i",
            font_size=28, color=CONSCIOUSNESS
        )
        consciousness_eq.to_edge(DOWN, buff=0.8)

        self.play(Write(consciousness_eq), run_time=1)

        # Complex roots
        complex_note = Text(
            "Complex roots: subjective experience",
            font_size=18, color=WHITE
        )
        complex_note.next_to(consciousness_eq, DOWN, buff=0.2)

        self.play(Write(complex_note), run_time=1)

        self.wait(2)
        self.play(FadeOut(VGroup(observer, pulse, consciousness_eq, complex_note, chapter_8)), run_time=1)

        # =====================================================================
        # CHAPTER 9: THE SLOOP CLOSES (240-270s)
        # =====================================================================

        chapter_9 = self.create_chapter_title("IX", "THE SLOOP CLOSES", "The observer observes itself")
        self.play(FadeIn(chapter_9), run_time=1)
        self.wait(1)
        self.play(chapter_9.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # The sLoop diagram
        sloop_circle = Circle(radius=2, color=SLOOP, stroke_width=4)

        # Observer and System as part of the same loop
        observer_node = Dot(radius=0.3, color=CONSCIOUSNESS)
        observer_node.move_to(sloop_circle.point_from_proportion(0.25))
        observer_label = Text("Observer", font_size=18, color=CONSCIOUSNESS)
        observer_label.next_to(observer_node, UP, buff=0.2)

        system_node = Dot(radius=0.3, color=FLUX)
        system_node.move_to(sloop_circle.point_from_proportion(0.75))
        system_label = Text("System", font_size=18, color=FLUX)
        system_label.next_to(system_node, DOWN, buff=0.2)

        # Arrows along the loop
        arrow1 = Arrow(
            sloop_circle.point_from_proportion(0.35),
            sloop_circle.point_from_proportion(0.65),
            color=SLOOP, stroke_width=3
        )
        arrow2 = Arrow(
            sloop_circle.point_from_proportion(0.85),
            sloop_circle.point_from_proportion(0.15),
            color=SLOOP, stroke_width=3
        )

        sloop_group = VGroup(
            sloop_circle, observer_node, observer_label,
            system_node, system_label
        )

        self.play(Create(sloop_circle), run_time=1)
        self.play(
            FadeIn(observer_node), Write(observer_label),
            FadeIn(system_node), Write(system_label),
            run_time=1
        )

        # Self-reference text
        self_ref = Text(
            "Observer ⊂ System being observed",
            font_size=24, color=SLOOP
        )
        self_ref.to_edge(DOWN, buff=1.2)

        self.play(Write(self_ref), run_time=1)

        # Bell correlation
        bell = MathTex(
            r"S \to 2\sqrt{2} \text{ (quantum correlations)}",
            font_size=24, color=FLUX
        )
        bell.next_to(self_ref, DOWN, buff=0.3)

        self.play(Write(bell), run_time=1)

        self.wait(2)
        self.play(FadeOut(VGroup(sloop_group, self_ref, bell, chapter_9)), run_time=1)

        # =====================================================================
        # CHAPTER 10: FINALE - RETURN TO VOID (270-300s)
        # =====================================================================

        chapter_10 = self.create_chapter_title("X", "RETURN TO VOID", "The circle completes")
        self.play(FadeIn(chapter_10), run_time=1)
        self.wait(1)
        self.play(chapter_10.animate.to_edge(UP, buff=0.3).scale(0.6), run_time=1)

        # Everything emerged from void
        cycle_text = VGroup(
            Text("Void → Three States → Four Integers", font_size=20, color=WHITE),
            Text("→ Forces → Particles → Atoms → Stars", font_size=20, color=WHITE),
            Text("→ Observers → sLoop → Back to Void", font_size=20, color=SLOOP),
        ).arrange(DOWN, buff=0.3)

        self.play(FadeIn(cycle_text), run_time=2)

        # The final equation
        final_eq = MathTex(
            r"\{3, 4, 7, 13\} \to \text{Everything}",
            font_size=48, color=FLUX
        )
        final_eq.shift(DOWN * 1)

        self.play(Write(final_eq), run_time=2)

        # Flash
        self.play(
            Flash(final_eq, color=FLUX, flash_radius=2),
            run_time=1
        )

        self.wait(2)

        # Fade everything
        self.play(FadeOut(VGroup(cycle_text, final_eq, chapter_10)), run_time=2)

        # =====================================================================
        # END CARD
        # =====================================================================

        end_title = Text(
            "FOUNDATIONAL TERNARY DYNAMICS",
            font_size=36,
            color=FLUX,
            weight=BOLD
        )

        end_subtitle = VGroup(
            Text("A complete Theory of Everything", font_size=24, color=WHITE),
            Text("derived from four integers", font_size=20, color=VOID),
            MathTex(r"\{3, 4, 7, 13\}", font_size=36, color=FLUX),
        ).arrange(DOWN, buff=0.3)

        end_group = VGroup(end_title, end_subtitle).arrange(DOWN, buff=0.8)

        self.play(FadeIn(end_group), run_time=3)
        self.wait(5)

    def create_chapter_title(self, number, title, subtitle):
        """Create a chapter title card."""
        chapter_num = Text(f"Chapter {number}", font_size=20, color=VOID)
        chapter_title = Text(title, font_size=36, color=FLUX, weight=BOLD)
        chapter_sub = Text(subtitle, font_size=22, color=WHITE)

        return VGroup(chapter_num, chapter_title, chapter_sub).arrange(DOWN, buff=0.2)

    def create_spiral_galaxy(self):
        """Create a simple spiral galaxy representation."""
        galaxy = VGroup()

        # Central bulge
        bulge = Dot(radius=0.3, color=FLUX)
        galaxy.add(bulge)

        # Spiral arms
        for arm_offset in [0, PI]:
            for r in np.linspace(0.5, 2.5, 30):
                angle = arm_offset + r * 1.5  # Spiral
                x = r * np.cos(angle)
                y = r * np.sin(angle)
                star = Dot(
                    radius=0.03 * (3 - r),
                    color=interpolate_color(FLUX, WHITE, r/2.5)
                )
                star.move_to([x, y, 0])
                galaxy.add(star)

        return galaxy


class GrandSynthesisShort(Scene):
    """A shorter 2-minute version hitting the key points."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # Title
        title = Text("FTD: The Complete Story", font_size=42, color=FLUX)
        self.play(Write(title), run_time=2)
        self.play(FadeOut(title), run_time=1)

        # Quick montage
        scenes = [
            ("VOID", r"s = 0", VOID),
            ("THREE STATES", r"s \in \{-1, 0, +1\}", MATTER),
            ("FOUR INTEGERS", r"\{3, 4, 7, 13\}", FLUX),
            ("FOUR FORCES", r"U(1) \times SU(2) \times SU(3)", EM),
            ("PARTICLES", r"31+ \text{ masses derived}", STRONG),
            ("ATOMS TO LIFE", r"\text{Chemistry emerges}", GRAVITY),
            ("OBSERVERS", r"\text{Consciousness}", CONSCIOUSNESS),
            ("SLOOP", r"\text{Self-reference}", SLOOP),
        ]

        for name, formula, color in scenes:
            title = Text(name, font_size=48, color=color, weight=BOLD)
            eq = MathTex(formula, font_size=32, color=WHITE)
            group = VGroup(title, eq).arrange(DOWN, buff=0.3)

            self.play(FadeIn(group), run_time=0.5)
            self.wait(1)
            self.play(FadeOut(group), run_time=0.3)

        # Final
        final = VGroup(
            Text("From Void to Observer and back", font_size=28, color=FLUX),
            MathTex(r"\{3, 4, 7, 13\} \to \text{Everything}", font_size=36, color=FLUX),
        ).arrange(DOWN, buff=0.5)

        self.play(FadeIn(final), run_time=2)
        self.wait(3)


if __name__ == "__main__":
    print("Run the epic 5-minute version with:")
    print("  manim -pqh scene_24_grand_synthesis.py GrandSynthesisScene")
    print("")
    print("Or the shorter 2-minute version:")
    print("  manim -pql scene_24_grand_synthesis.py GrandSynthesisShort")
    print("")
    print("For 4K rendering:")
    print("  manim -qk scene_24_grand_synthesis.py GrandSynthesisScene")
