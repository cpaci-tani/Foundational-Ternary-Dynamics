"""
Simulation 7: CKM Matrix Visualization
======================================
A 90-second Manim animation showing the CKM quark mixing matrix
with unitarity triangles and FTD predictions.

Key achievement: FTD derives all CKM elements to 3-6% accuracy
using CP phase δ = arctan(7/3) = 66.8°

Storyboard:
1. (0-20s) Introduction: What quarks mix?
2. (20-45s) The CKM matrix as 3x3 heatmap
3. (45-70s) Unitarity triangle in complex plane
4. (70-90s) FTD predictions vs experimental values

Run with: manim -pql scene_07_ckm_matrix.py CKMMatrixScene
For high quality: manim -pqh scene_07_ckm_matrix.py CKMMatrixScene

Author: FTD Visualization Suite
Date: January 2026
"""

from manim import *
import numpy as np

# Colors
BACKGROUND = "#0D1117"
UP_QUARK = "#E74C3C"      # Red
DOWN_QUARK = "#F39C12"    # Orange
CHARM_QUARK = "#9B59B6"   # Purple
STRANGE_QUARK = "#27AE60" # Green
TOP_QUARK = "#3498DB"     # Blue
BOTTOM_QUARK = "#E67E22"  # Dark orange
HIGHLIGHT = "#FFD700"     # Gold
FTD_COLOR = "#00CED1"     # Cyan for FTD predictions


class CKMMatrixScene(Scene):
    """The CKM matrix and unitarity triangles."""

    def construct(self):
        self.camera.background_color = BACKGROUND

        # =====================================================================
        # SCENE 1: Introduction (0-20s)
        # =====================================================================

        title = Text("The CKM Matrix", font_size=42, color=HIGHLIGHT)
        subtitle = Text("Quark Flavor Mixing", font_size=28, color=WHITE)
        title_group = VGroup(title, subtitle).arrange(DOWN, buff=0.3)

        self.play(Write(title), run_time=1.5)
        self.play(FadeIn(subtitle), run_time=1)
        self.wait(0.5)
        self.play(title_group.animate.to_edge(UP, buff=0.3).scale(0.8), run_time=1)

        # Show quark generations
        up_type = VGroup(
            Text("Up-type:", font_size=22, color=WHITE),
            MathTex(r"u", font_size=32, color=UP_QUARK),
            MathTex(r"c", font_size=32, color=CHARM_QUARK),
            MathTex(r"t", font_size=32, color=TOP_QUARK),
        ).arrange(RIGHT, buff=0.5)
        up_type.shift(UP * 1.5 + LEFT * 2)

        down_type = VGroup(
            Text("Down-type:", font_size=22, color=WHITE),
            MathTex(r"d", font_size=32, color=DOWN_QUARK),
            MathTex(r"s", font_size=32, color=STRANGE_QUARK),
            MathTex(r"b", font_size=32, color=BOTTOM_QUARK),
        ).arrange(RIGHT, buff=0.5)
        down_type.shift(UP * 0.5 + LEFT * 2)

        self.play(FadeIn(up_type), FadeIn(down_type), run_time=1.5)

        # Explanation
        explanation = Text(
            "Weak interactions mix quark flavors",
            font_size=22, color=WHITE
        )
        explanation.shift(DOWN * 0.5)
        self.play(Write(explanation), run_time=1)

        # Mixing arrows
        mixing_arrow = Arrow(
            up_type[1].get_bottom(),
            down_type[1].get_top(),
            color=HIGHLIGHT, stroke_width=2
        )
        self.play(Create(mixing_arrow), run_time=0.5)

        self.wait(1)

        # Clear for matrix
        self.play(
            FadeOut(VGroup(up_type, down_type, explanation, mixing_arrow)),
            run_time=0.5
        )

        # =====================================================================
        # SCENE 2: The CKM Matrix (20-45s)
        # =====================================================================

        # CKM matrix values (experimental)
        ckm_exp = np.array([
            [0.97373, 0.2243, 0.00382],
            [0.221, 0.975, 0.0408],
            [0.0086, 0.0415, 0.9991]
        ])

        # FTD predictions
        ckm_ftd = np.array([
            [0.974, 0.226, 0.0036],
            [0.226, 0.973, 0.042],
            [0.0088, 0.041, 0.999]
        ])

        # Create matrix display
        matrix_label = MathTex(r"V_{\text{CKM}} = ", font_size=36)
        matrix_label.shift(LEFT * 4)

        # Matrix as colored grid
        cell_size = 0.8
        matrix_grid = VGroup()
        value_labels = VGroup()

        row_labels = ["d", "s", "b"]
        col_labels = ["u", "c", "t"]

        for i in range(3):
            for j in range(3):
                # Color based on magnitude
                val = ckm_exp[i, j]
                if val > 0.9:
                    color = "#2ECC71"  # Green - diagonal dominant
                elif val > 0.1:
                    color = "#F39C12"  # Orange - medium mixing
                else:
                    color = "#E74C3C"  # Red - small mixing

                cell = Square(
                    side_length=cell_size,
                    fill_color=color,
                    fill_opacity=val,  # Opacity proportional to value
                    stroke_color=WHITE,
                    stroke_width=1
                )
                cell.move_to(RIGHT * (j - 1) * cell_size + DOWN * (i - 1) * cell_size)
                matrix_grid.add(cell)

                # Value label
                val_text = MathTex(f"{val:.3f}", font_size=18, color=WHITE)
                val_text.move_to(cell.get_center())
                value_labels.add(val_text)

        matrix_grid.shift(RIGHT * 0.5)
        value_labels.shift(RIGHT * 0.5)

        # Row and column labels
        row_label_group = VGroup(
            *[MathTex(r"V_{" + l + "}", font_size=18, color=WHITE) for l in row_labels]
        ).arrange(DOWN, buff=cell_size - 0.25)
        row_label_group.next_to(matrix_grid, LEFT, buff=0.3)

        col_label_group = VGroup(
            *[MathTex(l, font_size=24, color=WHITE) for l in col_labels]
        ).arrange(RIGHT, buff=cell_size - 0.25)
        col_label_group.next_to(matrix_grid, UP, buff=0.3)

        self.play(Write(matrix_label), run_time=0.5)
        self.play(
            Create(matrix_grid),
            FadeIn(value_labels),
            FadeIn(row_label_group),
            FadeIn(col_label_group),
            run_time=2
        )

        # Legend
        legend = VGroup(
            VGroup(Square(0.2, fill_color="#2ECC71", fill_opacity=1, stroke_width=0),
                   Text("Diagonal (large)", font_size=14, color=WHITE)).arrange(RIGHT, buff=0.2),
            VGroup(Square(0.2, fill_color="#F39C12", fill_opacity=1, stroke_width=0),
                   Text("Off-diagonal (medium)", font_size=14, color=WHITE)).arrange(RIGHT, buff=0.2),
            VGroup(Square(0.2, fill_color="#E74C3C", fill_opacity=1, stroke_width=0),
                   Text("Suppressed (small)", font_size=14, color=WHITE)).arrange(RIGHT, buff=0.2),
        ).arrange(DOWN, buff=0.2, aligned_edge=LEFT)
        legend.to_edge(RIGHT, buff=0.5).shift(UP * 1)

        self.play(FadeIn(legend), run_time=1)

        # Key insight
        insight = Text(
            "Near-diagonal: flavor mostly conserved",
            font_size=20, color=HIGHLIGHT
        )
        insight.to_edge(DOWN, buff=1)
        self.play(Write(insight), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 3: Unitarity Triangle (45-70s)
        # =====================================================================

        # Clear matrix
        self.play(
            FadeOut(VGroup(
                matrix_label, matrix_grid, value_labels,
                row_label_group, col_label_group, legend, insight
            )),
            run_time=0.5
        )

        # Unitarity triangle
        tri_title = Text("Unitarity Triangle", font_size=32, color=HIGHLIGHT)
        tri_title.next_to(title_group, DOWN, buff=0.3)
        self.play(Write(tri_title), run_time=0.5)

        # Complex plane axes
        plane = NumberPlane(
            x_range=[-0.5, 1.2, 0.5],
            y_range=[-0.2, 0.6, 0.2],
            x_length=6,
            y_length=3,
            background_line_style={"stroke_color": GRAY, "stroke_opacity": 0.3}
        )
        plane.shift(DOWN * 0.5)

        self.play(Create(plane), run_time=1)

        # Triangle vertices (normalized)
        # (0,0), (1,0), (ρ̄, η̄) where ρ̄ ≈ 0.13, η̄ ≈ 0.35
        rho_bar = 0.13
        eta_bar = 0.35

        v0 = plane.c2p(0, 0)
        v1 = plane.c2p(1, 0)
        v2 = plane.c2p(rho_bar, eta_bar)

        triangle = Polygon(v0, v1, v2, color=HIGHLIGHT, stroke_width=3)
        self.play(Create(triangle), run_time=1)

        # Label vertices
        vertex_labels = VGroup(
            MathTex(r"(0, 0)", font_size=18, color=WHITE).next_to(v0, DL, buff=0.1),
            MathTex(r"(1, 0)", font_size=18, color=WHITE).next_to(v1, DR, buff=0.1),
            MathTex(r"(\bar{\rho}, \bar{\eta})", font_size=18, color=HIGHLIGHT).next_to(v2, UP, buff=0.1),
        )
        self.play(FadeIn(vertex_labels), run_time=0.5)

        # Angles
        alpha_angle = Angle(
            Line(v2, v1), Line(v2, v0),
            radius=0.3, color=UP_QUARK
        )
        beta_angle = Angle(
            Line(v0, v2), Line(v0, v1),
            radius=0.3, color=CHARM_QUARK
        )
        gamma_angle = Angle(
            Line(v1, v0), Line(v1, v2),
            radius=0.3, color=TOP_QUARK
        )

        alpha_label = MathTex(r"\alpha", font_size=20, color=UP_QUARK)
        alpha_label.next_to(alpha_angle, LEFT, buff=0.1)
        beta_label = MathTex(r"\beta", font_size=20, color=CHARM_QUARK)
        beta_label.next_to(beta_angle, UR, buff=0.1)
        gamma_label = MathTex(r"\gamma", font_size=20, color=TOP_QUARK)
        gamma_label.next_to(gamma_angle, UP, buff=0.1)

        self.play(
            Create(alpha_angle), Write(alpha_label),
            Create(beta_angle), Write(beta_label),
            Create(gamma_angle), Write(gamma_label),
            run_time=1.5
        )

        # Unitarity constraint
        unitarity = MathTex(
            r"\alpha + \beta + \gamma = 180°",
            font_size=28, color=WHITE
        )
        unitarity.to_edge(DOWN, buff=0.8)
        self.play(Write(unitarity), run_time=1)

        self.wait(2)

        # =====================================================================
        # SCENE 4: FTD Predictions (70-90s)
        # =====================================================================

        # Clear triangle
        self.play(
            FadeOut(VGroup(
                tri_title, plane, triangle, vertex_labels,
                alpha_angle, beta_angle, gamma_angle,
                alpha_label, beta_label, gamma_label, unitarity
            )),
            run_time=0.5
        )

        # FTD prediction title
        ftd_title = Text("FTD Predictions", font_size=32, color=FTD_COLOR)
        ftd_title.next_to(title_group, DOWN, buff=0.3)
        self.play(Write(ftd_title), run_time=0.5)

        # Key prediction: CP phase
        cp_phase = VGroup(
            Text("CP-violating phase:", font_size=24, color=WHITE),
            MathTex(r"\delta = \arctan\left(\frac{7}{3}\right) = 66.8°", font_size=32, color=FTD_COLOR),
            Text("(Experimental: 68.3° ± 2.0°)", font_size=20, color=GRAY_B),
        ).arrange(DOWN, buff=0.3)
        cp_phase.shift(UP * 0.5)

        self.play(FadeIn(cp_phase), run_time=1.5)

        # Highlight the integers
        integers_note = MathTex(
            r"7 = b_3, \quad 3 = N_c",
            font_size=24, color=HIGHLIGHT
        )
        integers_note.next_to(cp_phase, DOWN, buff=0.5)

        self.play(Write(integers_note), run_time=1)

        # Accuracy table
        table_data = [
            ("|V_{ud}|", "0.974", "0.974", "0.03%"),
            ("|V_{us}|", "0.226", "0.224", "0.9%"),
            ("|V_{cb}|", "0.042", "0.041", "2.4%"),
            ("|V_{ub}|", "0.0036", "0.0038", "5.3%"),
        ]

        table = VGroup()
        headers = VGroup(
            Text("Element", font_size=16, color=WHITE),
            Text("FTD", font_size=16, color=FTD_COLOR),
            Text("Exp", font_size=16, color=WHITE),
            Text("Error", font_size=16, color=HIGHLIGHT),
        ).arrange(RIGHT, buff=0.8)
        headers.shift(DOWN * 1)
        table.add(headers)

        for i, (elem, ftd, exp, err) in enumerate(table_data):
            row = VGroup(
                MathTex(elem, font_size=18, color=WHITE),
                Text(ftd, font_size=16, color=FTD_COLOR),
                Text(exp, font_size=16, color=WHITE),
                Text(err, font_size=16, color=HIGHLIGHT),
            ).arrange(RIGHT, buff=0.8)
            row.next_to(headers, DOWN, buff=0.2 + i * 0.4)
            table.add(row)

        self.play(FadeIn(table), run_time=2)

        # Summary
        summary = Text(
            "All elements predicted to 3-6% accuracy!",
            font_size=22, color=HIGHLIGHT
        )
        summary.to_edge(DOWN, buff=0.5)

        self.play(
            Write(summary),
            Flash(summary, color=HIGHLIGHT, flash_radius=0.6),
            run_time=1
        )

        self.wait(3)

        # Fade out
        self.play(
            FadeOut(VGroup(
                ftd_title, cp_phase, integers_note, table, summary, title_group
            )),
            run_time=1
        )

        # End card
        end_card = VGroup(
            Text("FOUNDATIONAL TERNARY DYNAMICS", font_size=28, color=HIGHLIGHT),
            MathTex(r"\delta_{\text{CKM}} = \arctan(7/3) = 66.8°", font_size=32),
            Text("Flavor physics from framework integers", font_size=20, color=WHITE),
        ).arrange(DOWN, buff=0.4)

        self.play(FadeIn(end_card), run_time=2)
        self.wait(2)


if __name__ == "__main__":
    print("Run with:")
    print("  manim -pql scene_07_ckm_matrix.py CKMMatrixScene")
