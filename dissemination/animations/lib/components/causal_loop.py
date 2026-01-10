"""
Causal Loop Diagram
===================

Visualization of the 13-step TRD update cycle.
Circular flowchart with phase-based coloring and step highlighting.
"""

from __future__ import annotations

import numpy as np
from typing import Sequence

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    PI,
    TAU,
    Animation,
    AnimationGroup,
    Succession,
    Create,
    FadeIn,
    FadeOut,
    Indicate,
    Flash,
    Circumscribe,
    VGroup,
    VMobject,
    Circle,
    Arc,
    ArcBetweenPoints,
    Arrow,
    CurvedArrow,
    Line,
    Dot,
    Text,
    Rectangle,
    RoundedRectangle,
    Polygon,
    rate_functions,
)

from ..colors import TRD_COLORS, PHASE_COLORS


# The 13 steps of the TRD causal loop
CAUSAL_LOOP_STEPS = [
    {"name": "TIME GATE", "phase": "temporal", "description": "Phase accumulator check"},
    {"name": "DECAY", "phase": "existence", "description": "Entropy to unlocked voxels"},
    {"name": "EXISTENCE", "phase": "existence", "description": "Evaporate or Genesis"},
    {"name": "PROPAGATE", "phase": "propagation", "description": "Flux waves advance"},
    {"name": "SUPERPOSE", "phase": "propagation", "description": "Vector field summation"},
    {"name": "FIELDS", "phase": "propagation", "description": "Compute gradients, curl, div"},
    {"name": "FORCES", "phase": "forces", "description": "Calculate all force types"},
    {"name": "INTEGRATE", "phase": "forces", "description": "Forces to velocity updates"},
    {"name": "MOVE", "phase": "motion", "description": "Particle position updates"},
    {"name": "COLLIDE", "phase": "motion", "description": "Handle interactions"},
    {"name": "TRANSMUTE", "phase": "motion", "description": "Polarity flip if stressed"},
    {"name": "BIND", "phase": "motion", "description": "Lock stable structures"},
    {"name": "INCREMENT", "phase": "temporal", "description": "t ← t + 1"},
]


class CausalLoopNode(VGroup):
    """
    A single node in the causal loop diagram.

    Parameters
    ----------
    step_data : dict
        Step information (name, phase, description)
    position : np.ndarray
        Node position
    radius : float
        Node radius
    highlighted : bool
        Whether this node is currently active
    """

    def __init__(
        self,
        step_data: dict,
        position: np.ndarray = ORIGIN,
        radius: float = 0.4,
        highlighted: bool = False,
        show_description: bool = False,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.step_data = step_data
        self._radius = radius
        self._highlighted = highlighted
        self._show_description = show_description

        self._build_node()
        self.move_to(position)

    def _build_node(self):
        """Construct the node visualization."""
        self.submobjects.clear()

        phase = self.step_data.get("phase", "temporal")
        base_color = PHASE_COLORS.get(phase, TRD_COLORS["text"])

        if self._highlighted:
            # Highlighted: full glow
            for i in range(4, 0, -1):
                glow = Circle(
                    radius=self._radius * (1 + i * 0.2),
                    fill_color=base_color,
                    fill_opacity=0.15 / i,
                    stroke_opacity=0,
                )
                self.add(glow)

            # Main circle
            circle = Circle(
                radius=self._radius,
                fill_color=base_color,
                fill_opacity=0.9,
                stroke_color=TRD_COLORS["glow"],
                stroke_width=3,
            )
            text_color = TRD_COLORS["background"]
        else:
            # Not highlighted: dim
            circle = Circle(
                radius=self._radius,
                fill_color=TRD_COLORS["background_light"],
                fill_opacity=0.7,
                stroke_color=base_color,
                stroke_width=2,
                stroke_opacity=0.6,
            )
            text_color = base_color

        self.add(circle)

        # Step name (abbreviated)
        name = self.step_data.get("name", "")
        # Abbreviate long names
        if len(name) > 8:
            abbrev = name[:7] + "."
        else:
            abbrev = name

        name_text = Text(
            abbrev,
            color=text_color,
            font_size=14 if len(abbrev) > 6 else 16,
            weight="BOLD" if self._highlighted else "NORMAL",
        )
        name_text.move_to(circle.get_center())
        self.add(name_text)

        # Description (if enabled)
        if self._show_description:
            desc = self.step_data.get("description", "")
            desc_text = Text(
                desc,
                color=TRD_COLORS["text_dim"],
                font_size=12,
            )
            desc_text.next_to(circle, DOWN, buff=0.15)
            self.add(desc_text)

    def highlight(self) -> Animation:
        """Return animation to highlight this node."""
        self._highlighted = True
        target = CausalLoopNode(
            self.step_data,
            position=self.get_center(),
            radius=self._radius,
            highlighted=True,
            show_description=self._show_description,
        )
        return AnimationGroup(
            FadeIn(target),
            Flash(self, color=PHASE_COLORS.get(self.step_data.get("phase"), TRD_COLORS["highlight"])),
        )

    def unhighlight(self) -> Animation:
        """Return animation to unhighlight this node."""
        self._highlighted = False
        target = CausalLoopNode(
            self.step_data,
            position=self.get_center(),
            radius=self._radius,
            highlighted=False,
            show_description=self._show_description,
        )
        return FadeIn(target, run_time=0.3)


class CausalLoopDiagram(VGroup):
    """
    Complete 13-step causal loop diagram in circular layout.

    Parameters
    ----------
    radius : float
        Radius of the circular layout
    node_radius : float
        Radius of each node
    show_arrows : bool
        Whether to show connecting arrows
    show_center : bool
        Whether to show center element
    """

    def __init__(
        self,
        radius: float = 3.0,
        node_radius: float = 0.4,
        show_arrows: bool = True,
        show_center: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.layout_radius = radius
        self.node_radius = node_radius

        self._nodes: list[CausalLoopNode] = []
        self._arrows: list[VMobject] = []
        self._current_step: int = -1

        self._build_diagram(show_arrows, show_center)

    def _build_diagram(self, show_arrows: bool, show_center: bool):
        """Construct the complete diagram."""
        num_steps = len(CAUSAL_LOOP_STEPS)

        # Create nodes in circular arrangement
        for i, step in enumerate(CAUSAL_LOOP_STEPS):
            # Angle: start from top, go clockwise
            angle = PI / 2 - (i / num_steps) * TAU
            x = self.layout_radius * np.cos(angle)
            y = self.layout_radius * np.sin(angle)
            pos = np.array([x, y, 0])

            node = CausalLoopNode(
                step_data=step,
                position=pos,
                radius=self.node_radius,
                highlighted=False,
            )
            self._nodes.append(node)
            self.add(node)

        # Create connecting arrows
        if show_arrows:
            for i in range(num_steps):
                j = (i + 1) % num_steps
                start = self._nodes[i].get_center()
                end = self._nodes[j].get_center()

                # Direction vector
                direction = end - start
                dist = np.linalg.norm(direction)
                unit = direction / dist

                # Adjust start/end to be on circle edges
                start_adj = start + unit * self.node_radius * 1.1
                end_adj = end - unit * self.node_radius * 1.1

                # Get color from current node's phase
                phase = CAUSAL_LOOP_STEPS[i].get("phase", "temporal")
                color = PHASE_COLORS.get(phase, TRD_COLORS["text"])

                # Curved arrow along the circle
                arrow = CurvedArrow(
                    start_adj, end_adj,
                    angle=-TAU / (num_steps * 2),  # Slight curve
                    color=color,
                    stroke_width=2,
                    stroke_opacity=0.5,
                    tip_length=0.15,
                )
                self._arrows.append(arrow)
                self.add(arrow)

        # Center label
        if show_center:
            center_circle = Circle(
                radius=self.layout_radius * 0.35,
                fill_color=TRD_COLORS["background"],
                fill_opacity=0.9,
                stroke_color=TRD_COLORS["grid_bright"],
                stroke_width=1,
            )
            self.add(center_circle)

            center_text = Text(
                "TICK\nt → t+1",
                color=TRD_COLORS["text"],
                font_size=20,
                line_spacing=1.2,
            )
            self.add(center_text)

    def get_node(self, index: int) -> CausalLoopNode | None:
        """Get node by index."""
        if 0 <= index < len(self._nodes):
            return self._nodes[index]
        return None

    def highlight_step(self, step_index: int, run_time: float = 0.5) -> Animation:
        """
        Highlight a specific step in the loop.

        Parameters
        ----------
        step_index : int
            Index of step to highlight (0-12)
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Highlighting animation
        """
        anims = []

        # Unhighlight previous
        if self._current_step >= 0 and self._current_step != step_index:
            prev_node = self._nodes[self._current_step]
            prev_node._highlighted = False
            prev_node._build_node()

        # Highlight current
        node = self._nodes[step_index]
        phase = CAUSAL_LOOP_STEPS[step_index].get("phase", "temporal")
        color = PHASE_COLORS.get(phase, TRD_COLORS["highlight"])

        self._current_step = step_index

        # Flash effect
        flash = Flash(
            node,
            color=color,
            flash_radius=self.node_radius * 2,
            line_length=self.node_radius,
            run_time=run_time,
        )

        # Update node
        node._highlighted = True
        node._build_node()

        return flash

    def animate_full_cycle(
        self,
        duration: float = 6.0,
        pause_per_step: float = 0.3,
    ) -> Animation:
        """
        Animate a complete cycle through all 13 steps.

        Parameters
        ----------
        duration : float
            Total animation duration
        pause_per_step : float
            Pause time at each step

        Returns
        -------
        Animation
            Full cycle animation
        """
        anims = []
        step_time = duration / len(CAUSAL_LOOP_STEPS)

        for i in range(len(CAUSAL_LOOP_STEPS)):
            anims.append(self.highlight_step(i, run_time=step_time * 0.7))

        return Succession(*anims)

    def show_phase_groups(self, run_time: float = 2.0) -> Animation:
        """
        Animate highlighting steps by phase group.

        Returns
        -------
        Animation
            Phase grouping animation
        """
        phases = ["temporal", "existence", "propagation", "forces", "motion"]
        anims = []

        for phase in phases:
            phase_anims = []
            for i, step in enumerate(CAUSAL_LOOP_STEPS):
                if step.get("phase") == phase:
                    node = self._nodes[i]
                    phase_anims.append(
                        Indicate(node, color=PHASE_COLORS.get(phase), scale_factor=1.2)
                    )
            if phase_anims:
                anims.append(AnimationGroup(*phase_anims))

        return Succession(*anims, lag_ratio=0.3)


class CausalLoopLegend(VGroup):
    """
    Legend showing the five phases of the causal loop.

    Parameters
    ----------
    position : np.ndarray
        Legend position
    """

    def __init__(
        self,
        position: np.ndarray = RIGHT * 5,
        **kwargs,
    ):
        super().__init__(**kwargs)

        phases = [
            ("Temporal", "temporal"),
            ("Existence", "existence"),
            ("Propagation", "propagation"),
            ("Forces", "forces"),
            ("Motion", "motion"),
        ]

        y_offset = 0
        for name, phase in phases:
            color = PHASE_COLORS.get(phase, TRD_COLORS["text"])

            # Color box
            box = RoundedRectangle(
                width=0.4,
                height=0.3,
                corner_radius=0.05,
                fill_color=color,
                fill_opacity=0.8,
                stroke_opacity=0,
            )
            box.shift(UP * y_offset)

            # Label
            label = Text(
                name,
                color=TRD_COLORS["text"],
                font_size=16,
            )
            label.next_to(box, RIGHT, buff=0.2)

            self.add(box, label)
            y_offset -= 0.5

        self.move_to(position)
