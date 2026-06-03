"""Manim scenes for structural FTD explainers.

These animations are visual aids only. They intentionally avoid numerical
searches, coincidence hunting, or promotion of parametric insertions.
"""

from __future__ import annotations

import math

import numpy as np
from manim import *


BACKGROUND = "#0D1117"
BACKGROUND_LIGHT = "#1C2128"
GRID = "#333333"
VOID = "#888888"
MATTER = "#DD4444"
ANTIMATTER = "#4488DD"
FLUX = "#FFD700"
FLUX_LOW = "#FFE066"
STRONG = "#FF6B35"
WEAK = "#9B59B6"
EM = "#3498DB"
GRAVITY = "#27AE60"
LABEL = "#CCCCCC"
RESULT = "#2ECC71"

STATE_COLORS = {
    -1: ANTIMATTER,
    0: VOID,
    1: MATTER,
}

config.background_color = BACKGROUND
config.frame_width = 16
config.frame_height = 9


def make_header(title: str, tag: str, tag_color: str = FLUX) -> VGroup:
    tag_text = Text(tag, font_size=24, weight=BOLD, color=BACKGROUND)
    tag_box = RoundedRectangle(
        corner_radius=0.1,
        width=tag_text.width + 0.36,
        height=0.46,
        stroke_width=0,
        fill_color=tag_color,
        fill_opacity=1,
    ).move_to(tag_text)
    label = Text(title, font_size=38, weight=BOLD, color=WHITE)
    header = VGroup(VGroup(tag_box, tag_text), label).arrange(RIGHT, buff=0.35)
    header.to_edge(UP, buff=0.38).to_edge(LEFT, buff=0.55)
    return header


def make_caption(text: str, width: float = 12.0) -> Text:
    caption = Text(text, font_size=25, color=LABEL, line_spacing=0.8)
    if caption.width > width:
        caption.scale_to_fit_width(width)
    return caption


def state_tile(state: int, side: float = 1.0, label: str | None = None) -> VGroup:
    fill = STATE_COLORS[state]
    square = Square(
        side_length=side,
        stroke_color=WHITE,
        stroke_width=2,
        fill_color=fill,
        fill_opacity=0.82 if state else 0.30,
    )
    if state == 0:
        square.set_stroke(color=VOID, opacity=0.8)
    text = Text(label if label is not None else f"{state:+d}", font_size=26, color=WHITE)
    if state == 0 and label is None:
        text = Text("0", font_size=26, color=WHITE)
    return VGroup(square, text.move_to(square))


def iso_point(x: float, y: float, z: float, scale: float = 0.64) -> np.ndarray:
    return np.array([(x - y) * 0.62 * scale, (x + y) * 0.34 * scale + z * 0.58 * scale, 0])


def moore_cell(coord: tuple[int, int, int], side: float = 0.38) -> VGroup:
    x, y, z = coord
    state = 1 if coord == (0, 0, 0) else 0
    tile = state_tile(state, side=side, label="" if coord != (0, 0, 0) else "v")
    tile.move_to(iso_point(x, y, z))
    depth = z * 3 + y
    tile.set_z_index(depth)
    return tile


def arrow_between(start: Mobject, end: Mobject, color: str = FLUX) -> Arrow:
    return Arrow(
        start.get_center(),
        end.get_center(),
        buff=0.18,
        stroke_width=3,
        max_tip_length_to_length_ratio=0.18,
        color=color,
    )


class TernaryVoxelLanguage(Scene):
    """Introduce the FTD primitive state language and flux/state split."""

    def construct(self) -> None:
        header = make_header("Ternary voxel language", "[AXIOM]", FLUX)
        caption = make_caption(
            "Each lattice site carries a continuous flux field J and a discrete state s.",
        )
        caption.next_to(header, DOWN, aligned_edge=LEFT, buff=0.26)

        states = VGroup(
            state_tile(0, label="0"),
            state_tile(1, label="+1"),
            state_tile(-1, label="-1"),
        ).arrange(RIGHT, buff=0.7)
        state_labels = VGroup(
            Text("void", font_size=24, color=LABEL),
            Text("positive", font_size=24, color=LABEL),
            Text("negative", font_size=24, color=LABEL),
        )
        for label, tile in zip(state_labels, states):
            label.next_to(tile, DOWN, buff=0.18)
        ternary = VGroup(states, state_labels).move_to(LEFT * 4.2 + DOWN * 0.25)

        flux_plane = NumberPlane(
            x_range=(-2.5, 2.6, 1),
            y_range=(-1.5, 1.6, 1),
            x_length=5.2,
            y_length=3.2,
            background_line_style={
                "stroke_color": GRID,
                "stroke_width": 1,
                "stroke_opacity": 0.75,
            },
            axis_config={"stroke_opacity": 0},
        )
        flux_plane.move_to(RIGHT * 3.55 + DOWN * 0.1)
        field_arrows = VGroup()
        for x in np.linspace(-2, 2, 5):
            for y in np.linspace(-1, 1, 3):
                start = flux_plane.c2p(x, y)
                angle = 0.65 * math.sin(x) + 0.45 * math.cos(y)
                direction = np.array([math.cos(angle), math.sin(angle), 0])
                field_arrows.add(
                    Arrow(
                        start,
                        start + 0.42 * direction,
                        buff=0,
                        stroke_width=3,
                        max_tip_length_to_length_ratio=0.24,
                        color=FLUX,
                    )
                )
        threshold = Circle(radius=0.78, color=FLUX_LOW, stroke_width=3).move_to(flux_plane)
        threshold_label = Text("manifestation threshold", font_size=21, color=FLUX_LOW)
        threshold_label.next_to(threshold, DOWN, buff=0.16)

        flux_title = Text("flux field J(v,t)", font_size=27, color=FLUX)
        flux_title.next_to(flux_plane, UP, buff=0.15)
        state_title = Text("state field s(v,t) in {-1,0,+1}", font_size=27, color=WHITE)
        state_title.next_to(ternary, UP, buff=0.42)

        bridge_arrow = Arrow(
            ternary.get_right() + RIGHT * 0.25,
            flux_plane.get_left() + LEFT * 0.25,
            buff=0.1,
            stroke_width=4,
            color=RESULT,
        )
        bridge_label = Text("coupled layers", font_size=22, color=RESULT)
        bridge_label.next_to(bridge_arrow, UP, buff=0.1)

        self.play(FadeIn(header, shift=DOWN * 0.15), FadeIn(caption, shift=DOWN * 0.1))
        self.play(LaggedStart(*[FadeIn(tile, scale=0.85) for tile in states], lag_ratio=0.18))
        self.play(FadeIn(state_labels), Write(state_title))
        self.play(Create(flux_plane), FadeIn(flux_title), LaggedStartMap(GrowArrow, field_arrows, lag_ratio=0.04))
        self.play(Create(threshold), FadeIn(threshold_label))
        self.play(GrowArrow(bridge_arrow), FadeIn(bridge_label))
        self.play(
            field_arrows.animate.set_color(FLUX_LOW),
            threshold.animate.scale(1.12).set_color(RESULT),
            states[1].animate.scale(1.12),
            rate_func=there_and_back,
            run_time=1.6,
        )
        self.wait(0.6)


class MooreNeighborhoodLocality(Scene):
    """Visualize the 26-neighbor Moore locality rule."""

    def construct(self) -> None:
        header = make_header("Moore-neighborhood locality", "[AXIOM]", FLUX)
        caption = make_caption(
            "The update at voxel v reads only v and its 26 adjacent sites from the previous tick.",
        )
        caption.next_to(header, DOWN, aligned_edge=LEFT, buff=0.26)

        coords = [
            (x, y, z)
            for z in (-1, 0, 1)
            for y in (-1, 0, 1)
            for x in (-1, 0, 1)
        ]
        cells = [(coord, moore_cell(coord)) for coord in coords]
        block = VGroup(*[mob for _, mob in cells]).scale(1.35)
        block.move_to(LEFT * 4.0 + DOWN * 0.2)

        neighbor_mobs = VGroup(*[mob for coord, mob in cells if coord != (0, 0, 0)])
        center = next(mob for coord, mob in cells if coord == (0, 0, 0))
        block_label = Text("3x3x3 block: 27 sites", font_size=25, color=LABEL)
        block_label.next_to(block, DOWN, buff=0.45)
        count_label = Text("center + 26 neighbors", font_size=30, color=WHITE, weight=BOLD)
        count_label.next_to(block_label, DOWN, buff=0.16)

        read_arrows = VGroup()
        for coord, mob in cells:
            if coord == (0, 0, 0):
                continue
            if sum(abs(v) for v in coord) <= 2:
                read_arrows.add(arrow_between(mob, center, color=FLUX_LOW))

        tick_panel = VGroup()
        tick_rows = []
        for tick, side in enumerate((0.5, 0.78, 1.06)):
            square = Square(side_length=side, stroke_width=3, stroke_color=[EM, WEAK, STRONG][tick])
            square.set_fill(BACKGROUND_LIGHT, opacity=0.18)
            label = Text(f"tick t+{tick}", font_size=23, color=[EM, WEAK, STRONG][tick])
            row = VGroup(square, label).arrange(RIGHT, buff=0.34)
            tick_rows.append(row)
            tick_panel.add(row)
        tick_panel.arrange(DOWN, buff=0.34, aligned_edge=LEFT)
        tick_panel.move_to(RIGHT * 3.55 + DOWN * 0.1)
        cone_title = Text("one lattice step per tick", font_size=30, color=WHITE)
        cone_title.next_to(tick_panel, UP, buff=0.5)

        rule = VGroup(
            Text("local rule:", font_size=24, color=LABEL),
            Text("next(v) = F(v, Moore_26(v))", font_size=28, color=FLUX),
        ).arrange(RIGHT, buff=0.22)
        rule.to_edge(DOWN, buff=0.55)

        self.play(FadeIn(header, shift=DOWN * 0.15), FadeIn(caption, shift=DOWN * 0.1))
        self.play(LaggedStart(*[FadeIn(mob, scale=0.65) for _, mob in cells], lag_ratio=0.025))
        self.play(FadeIn(block_label), FadeIn(count_label))
        self.play(center.animate.scale(1.25), rate_func=there_and_back, run_time=0.9)
        self.play(neighbor_mobs.animate.set_opacity(0.95), LaggedStartMap(GrowArrow, read_arrows, lag_ratio=0.025))
        self.play(FadeIn(cone_title), LaggedStart(*[FadeIn(row, shift=LEFT * 0.2) for row in tick_rows], lag_ratio=0.25))
        self.play(Write(rule))
        for row in tick_rows:
            self.play(row[0].animate.scale(1.16), rate_func=there_and_back, run_time=0.55)
        self.wait(0.6)


class EngineTickCycle(Scene):
    """Show the engine phases as an instrumented cycle."""

    def construct(self) -> None:
        header = make_header("Engine tick cycle", "[ENGINE]", RESULT)
        caption = make_caption(
            "The C++ engine advances the two-layer lattice through explicit instrumented phases.",
        )
        caption.next_to(header, DOWN, aligned_edge=LEFT, buff=0.26)

        phase_names = [
            "phase_read",
            "phase_write",
            "gauss_project",
            "phase_forces",
            "phase_movement",
        ]
        phase_colors = [EM, FLUX, RESULT, STRONG, WEAK]
        phase_nodes = VGroup()
        radius = 2.15
        for idx, (name, color) in enumerate(zip(phase_names, phase_colors)):
            angle = PI / 2 - idx * TAU / len(phase_names)
            pos = np.array([math.cos(angle) * radius, math.sin(angle) * radius, 0])
            label = Text(name, font_size=22, color=WHITE)
            box = RoundedRectangle(
                corner_radius=0.12,
                width=max(2.1, label.width + 0.4),
                height=0.58,
                stroke_color=color,
                stroke_width=2.5,
                fill_color=BACKGROUND_LIGHT,
                fill_opacity=0.92,
            )
            node = VGroup(box, label.move_to(box)).move_to(pos)
            phase_nodes.add(node)
        phase_nodes.move_to(LEFT * 3.2 + DOWN * 0.25)

        cycle_arrows = VGroup()
        for idx in range(len(phase_nodes)):
            start = phase_nodes[idx]
            end = phase_nodes[(idx + 1) % len(phase_nodes)]
            cycle_arrows.add(arrow_between(start, end, color=LABEL))

        tick_counter = VGroup(
            Text("tick", font_size=28, color=LABEL),
            Text("t -> t+1", font_size=38, color=FLUX, weight=BOLD),
        ).arrange(DOWN, buff=0.1)
        tick_counter.move_to(phase_nodes.get_center())

        lattice = VGroup()
        for y in range(4):
            for x in range(5):
                state = 0
                if (x, y) in {(1, 1), (3, 2)}:
                    state = 1
                if (x, y) == (2, 1):
                    state = -1
                tile = state_tile(state, side=0.48, label="")
                tile.move_to(np.array([x * 0.56, -y * 0.56, 0]))
                lattice.add(tile)
        lattice.move_to(RIGHT * 4.1 + DOWN * 0.15)

        flux_arrows = VGroup()
        for tile in lattice[::2]:
            start = tile.get_center() + LEFT * 0.12 + DOWN * 0.08
            flux_arrows.add(
                Arrow(
                    start,
                    start + np.array([0.34, 0.2, 0]),
                    buff=0,
                    color=FLUX,
                    stroke_width=3,
                    max_tip_length_to_length_ratio=0.25,
                )
            )
        lattice_title = Text("instrumented lattice state", font_size=29, color=WHITE)
        lattice_title.next_to(lattice, UP, buff=0.42)

        notes = VGroup(
            Text("read fields", font_size=23, color=EM),
            Text("write flux/state", font_size=23, color=FLUX),
            Text("project constraint", font_size=23, color=RESULT),
            Text("apply forces", font_size=23, color=STRONG),
            Text("move manifestations", font_size=23, color=WEAK),
        ).arrange(DOWN, buff=0.16, aligned_edge=LEFT)
        notes.next_to(lattice, DOWN, buff=0.4)

        self.play(FadeIn(header, shift=DOWN * 0.15), FadeIn(caption, shift=DOWN * 0.1))
        self.play(LaggedStart(*[FadeIn(node, scale=0.85) for node in phase_nodes], lag_ratio=0.08))
        self.play(LaggedStartMap(GrowArrow, cycle_arrows, lag_ratio=0.08), FadeIn(tick_counter))
        self.play(FadeIn(lattice_title), LaggedStart(*[FadeIn(tile, scale=0.8) for tile in lattice], lag_ratio=0.025))
        self.play(LaggedStartMap(GrowArrow, flux_arrows, lag_ratio=0.04))
        self.play(FadeIn(notes))

        for idx, node in enumerate(phase_nodes):
            note = notes[idx]
            self.play(
                node[0].animate.set_fill(phase_colors[idx], opacity=0.28),
                note.animate.scale(1.12).set_color(WHITE),
                run_time=0.38,
            )
            self.play(
                node[0].animate.set_fill(BACKGROUND_LIGHT, opacity=0.92),
                note.animate.scale(1 / 1.12).set_color(phase_colors[idx]),
                run_time=0.25,
            )
        self.play(tick_counter[1].animate.set_color(RESULT).scale(1.12), rate_func=there_and_back, run_time=1.0)
        self.wait(0.6)


class FTDCoreTrailer(Scene):
    """Compact combined trailer for quick previews."""

    def construct(self) -> None:
        title = Text("Foundational Ternary Dynamics", font_size=48, weight=BOLD, color=WHITE)
        subtitle = Text("a structural animation sketch", font_size=28, color=LABEL)
        opener = VGroup(title, subtitle).arrange(DOWN, buff=0.18)
        self.play(FadeIn(opener, shift=DOWN * 0.2))
        self.wait(0.35)
        self.play(FadeOut(opener, shift=UP * 0.15))

        states = VGroup(
            state_tile(0, side=0.85, label="0"),
            state_tile(1, side=0.85, label="+1"),
            state_tile(-1, side=0.85, label="-1"),
        ).arrange(RIGHT, buff=0.45)
        states_title = Text("ternary state field", font_size=34, color=WHITE)
        flux = VGroup()
        for i in range(7):
            x = -2.4 + i * 0.8
            flux.add(Arrow([x, -1.2, 0], [x + 0.35, -0.82 + 0.15 * math.sin(i), 0], buff=0, color=FLUX))
        flux_label = Text("continuous flux field", font_size=30, color=FLUX)
        first = VGroup(states_title, states, flux, flux_label).arrange(DOWN, buff=0.35)

        self.play(FadeIn(states_title), LaggedStart(*[FadeIn(s, scale=0.75) for s in states], lag_ratio=0.12))
        self.play(LaggedStartMap(GrowArrow, flux, lag_ratio=0.04), FadeIn(flux_label))
        self.wait(0.25)
        self.play(FadeOut(first))

        coords = [(x, y, z) for z in (-1, 0, 1) for y in (-1, 0, 1) for x in (-1, 0, 1)]
        block = VGroup(*[moore_cell(coord, side=0.34) for coord in coords]).scale(1.6)
        block.move_to(ORIGIN)
        moore_label = Text("26-neighbor Moore locality", font_size=38, color=WHITE)
        moore_label.next_to(block, DOWN, buff=0.55)
        self.play(LaggedStart(*[FadeIn(cell, scale=0.7) for cell in block], lag_ratio=0.018), FadeIn(moore_label))
        self.play(block.animate.scale(1.08), rate_func=there_and_back, run_time=0.9)
        self.wait(0.25)
        self.play(FadeOut(block), FadeOut(moore_label))

        phases = ["read", "write", "project", "forces", "move"]
        phase_mobs = VGroup()
        for name, color in zip(phases, [EM, FLUX, RESULT, STRONG, WEAK]):
            label = Text(name, font_size=26, color=WHITE)
            box = RoundedRectangle(
                corner_radius=0.12,
                width=1.55,
                height=0.72,
                stroke_color=color,
                stroke_width=2.5,
                fill_color=BACKGROUND_LIGHT,
                fill_opacity=0.95,
            )
            phase_mobs.add(VGroup(box, label.move_to(box)))
        phase_mobs.arrange(RIGHT, buff=0.32).move_to(UP * 0.15)
        arrows = VGroup(*[arrow_between(phase_mobs[i], phase_mobs[i + 1], LABEL) for i in range(len(phase_mobs) - 1)])
        tick_label = Text("tick t -> t+1", font_size=38, color=FLUX, weight=BOLD)
        tick_label.next_to(phase_mobs, DOWN, buff=0.55)

        self.play(LaggedStart(*[FadeIn(mob, scale=0.85) for mob in phase_mobs], lag_ratio=0.1))
        self.play(LaggedStartMap(GrowArrow, arrows, lag_ratio=0.08), FadeIn(tick_label))
        self.play(tick_label.animate.set_color(RESULT).scale(1.08), rate_func=there_and_back, run_time=1.0)
        self.wait(0.7)
