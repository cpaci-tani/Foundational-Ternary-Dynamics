"""
Scale Zoom Mobject
==================

Planck to cosmic scale transitions and zoom animations.
Visualizes the multi-scale organization of TRD across 60 orders of magnitude.
"""

from __future__ import annotations

import numpy as np
from typing import Callable, Sequence

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
    Transform,
    ScaleInPlace,
    UpdateFromAlphaFunc,
    VGroup,
    VMobject,
    Circle,
    Dot,
    Line,
    Rectangle,
    RoundedRectangle,
    Square,
    Text,
    MathTex,
    DecimalNumber,
    NumberLine,
    rate_functions,
    smooth,
)

from ..colors import TRD_COLORS, GLOW_COLORS, lerp_color


# Scale levels in TRD (powers of 10 in meters)
SCALE_LEVELS = {
    "planck": {
        "power": -35,
        "name": "Planck Scale",
        "description": "Fundamental lattice spacing",
        "color": TRD_COLORS["highlight"],
        "entities": ["voxels", "flux quanta"],
    },
    "subatomic": {
        "power": -18,
        "name": "Subatomic",
        "description": "Quarks and gluons",
        "color": TRD_COLORS["matter"],
        "entities": ["quarks", "electrons"],
    },
    "nuclear": {
        "power": -15,
        "name": "Nuclear",
        "description": "Protons, neutrons",
        "color": TRD_COLORS["antimatter"],
        "entities": ["triads", "nuclei"],
    },
    "atomic": {
        "power": -10,
        "name": "Atomic",
        "description": "Atoms and molecules",
        "color": "#55ff55",  # Green
        "entities": ["atoms", "molecules"],
    },
    "molecular": {
        "power": -8,
        "name": "Molecular",
        "description": "Complex molecules",
        "color": "#cc66ff",  # Purple
        "entities": ["proteins", "DNA"],
    },
    "cellular": {
        "power": -5,
        "name": "Cellular",
        "description": "Living cells",
        "color": "#ff9955",  # Orange
        "entities": ["cells", "organelles"],
    },
    "human": {
        "power": 0,
        "name": "Human Scale",
        "description": "Observable world",
        "color": TRD_COLORS["text"],
        "entities": ["organisms", "objects"],
    },
    "planetary": {
        "power": 7,
        "name": "Planetary",
        "description": "Planets and moons",
        "color": "#5599ff",  # Blue
        "entities": ["planets", "moons"],
    },
    "stellar": {
        "power": 11,
        "name": "Stellar",
        "description": "Stars and systems",
        "color": "#ffcc55",  # Gold
        "entities": ["stars", "solar systems"],
    },
    "galactic": {
        "power": 21,
        "name": "Galactic",
        "description": "Galaxies",
        "color": "#ff55cc",  # Magenta
        "entities": ["galaxies", "clusters"],
    },
    "cosmic": {
        "power": 26,
        "name": "Cosmic",
        "description": "Observable universe",
        "color": TRD_COLORS["glow"],
        "entities": ["cosmic web", "universe"],
    },
}


class ScaleMarker(VGroup):
    """
    A marker for a specific scale level.

    Parameters
    ----------
    scale_key : str
        Key from SCALE_LEVELS dictionary
    radius : float
        Marker radius
    show_label : bool
        Whether to show the scale name
    """

    def __init__(
        self,
        scale_key: str,
        radius: float = 0.3,
        show_label: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)

        scale_data = SCALE_LEVELS.get(scale_key, SCALE_LEVELS["planck"])
        self.scale_data = scale_data
        self.color = scale_data["color"]

        # Glow layers
        for i in range(3, 0, -1):
            glow = Circle(
                radius=radius * (1 + i * 0.3),
                fill_color=self.color,
                fill_opacity=0.1 / i,
                stroke_opacity=0,
            )
            self.add(glow)

        # Main circle
        main = Circle(
            radius=radius,
            fill_color=self.color,
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["glow"],
            stroke_width=2,
        )
        self.add(main)

        # Power of 10 label
        power_text = MathTex(
            f"10^{{{scale_data['power']}}}",
            color=TRD_COLORS["background"],
            font_size=16,
        )
        power_text.move_to(main.get_center())
        self.add(power_text)

        # Name label
        if show_label:
            name_label = Text(
                scale_data["name"],
                color=self.color,
                font_size=14,
            )
            name_label.next_to(main, DOWN, buff=0.15)
            self.add(name_label)


class ScaleRuler(VGroup):
    """
    A logarithmic scale ruler showing all scales.

    Parameters
    ----------
    length : float
        Total ruler length
    show_markers : bool
        Whether to show scale markers
    """

    def __init__(
        self,
        length: float = 12.0,
        show_markers: bool = True,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.length = length

        self._build_ruler(show_markers)

    def _build_ruler(self, show_markers: bool):
        """Construct the scale ruler."""
        # Find min/max powers
        powers = [data["power"] for data in SCALE_LEVELS.values()]
        min_power = min(powers)
        max_power = max(powers)
        power_range = max_power - min_power

        # Main line
        main_line = Line(
            LEFT * self.length / 2,
            RIGHT * self.length / 2,
            color=TRD_COLORS["grid_bright"],
            stroke_width=2,
        )
        self.add(main_line)

        # Glow
        glow_line = Line(
            LEFT * self.length / 2,
            RIGHT * self.length / 2,
            color=TRD_COLORS["grid_bright"],
            stroke_width=8,
            stroke_opacity=0.2,
        )
        self.add(glow_line)

        # Tick marks and labels for each decade
        for power in range(-35, 30, 5):
            x_pos = (power - min_power) / power_range * self.length - self.length / 2

            # Tick
            tick = Line(
                np.array([x_pos, -0.1, 0]),
                np.array([x_pos, 0.1, 0]),
                color=TRD_COLORS["text_dim"],
                stroke_width=1,
            )
            self.add(tick)

            # Power label
            if power % 10 == 0:
                label = MathTex(
                    f"10^{{{power}}}",
                    color=TRD_COLORS["text_dim"],
                    font_size=12,
                )
                label.next_to(tick, DOWN, buff=0.1)
                self.add(label)

        # Scale markers
        if show_markers:
            for key, data in SCALE_LEVELS.items():
                power = data["power"]
                x_pos = (power - min_power) / power_range * self.length - self.length / 2

                marker = Dot(
                    point=np.array([x_pos, 0, 0]),
                    radius=0.12,
                    color=data["color"],
                )
                self.add(marker)

                # Glow
                glow = Circle(
                    radius=0.2,
                    stroke_color=data["color"],
                    stroke_width=2,
                    stroke_opacity=0.4,
                    fill_opacity=0,
                )
                glow.move_to(marker.get_center())
                self.add(glow)


class ZoomBox(VGroup):
    """
    A zoom box showing transition between scales.

    Shows a rectangular region that will be zoomed into.

    Parameters
    ----------
    outer_size : float
        Outer box size
    inner_size : float
        Inner (zoom target) box size
    outer_color : str
        Outer box color
    inner_color : str
        Inner box color
    """

    def __init__(
        self,
        outer_size: float = 4.0,
        inner_size: float = 0.5,
        outer_color: str = TRD_COLORS["grid"],
        inner_color: str = TRD_COLORS["highlight"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.outer_size = outer_size
        self.inner_size = inner_size

        # Outer box
        self.outer_box = Square(
            side_length=outer_size,
            stroke_color=outer_color,
            stroke_width=2,
            fill_opacity=0,
        )
        self.add(self.outer_box)

        # Inner zoom target
        self.inner_box = Square(
            side_length=inner_size,
            stroke_color=inner_color,
            stroke_width=2,
            fill_color=inner_color,
            fill_opacity=0.1,
        )
        self.add(self.inner_box)

        # Corner connectors (zoom lines)
        corners_outer = [
            self.outer_box.get_corner(UP + LEFT),
            self.outer_box.get_corner(UP + RIGHT),
            self.outer_box.get_corner(DOWN + RIGHT),
            self.outer_box.get_corner(DOWN + LEFT),
        ]
        corners_inner = [
            self.inner_box.get_corner(UP + LEFT),
            self.inner_box.get_corner(UP + RIGHT),
            self.inner_box.get_corner(DOWN + RIGHT),
            self.inner_box.get_corner(DOWN + LEFT),
        ]

        for co, ci in zip(corners_outer, corners_inner):
            line = Line(
                ci, co,
                stroke_color=inner_color,
                stroke_width=1,
                stroke_opacity=0.5,
            )
            self.add(line)

    def zoom_animation(self, scale_factor: float = 8.0, run_time: float = 2.0) -> Animation:
        """
        Animate zooming into the inner box.

        Parameters
        ----------
        scale_factor : float
            How much to zoom in
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Zoom animation
        """
        def update_zoom(mob, alpha):
            current_scale = 1 + alpha * (scale_factor - 1)
            # Scale the inner box to match outer
            target_size = self.outer_size / current_scale
            # This would need Transform to work properly
            pass

        return UpdateFromAlphaFunc(self, update_zoom, run_time=run_time)


class ScaleTransition(VGroup):
    """
    Animated transition between two scales.

    Creates a zoom effect with fade-through for content.

    Parameters
    ----------
    from_scale : str
        Starting scale key
    to_scale : str
        Target scale key
    """

    def __init__(
        self,
        from_scale: str = "planck",
        to_scale: str = "atomic",
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.from_scale = from_scale
        self.to_scale = to_scale

        from_data = SCALE_LEVELS.get(from_scale, SCALE_LEVELS["planck"])
        to_data = SCALE_LEVELS.get(to_scale, SCALE_LEVELS["atomic"])

        # Scale labels
        self.from_label = Text(
            from_data["name"],
            color=from_data["color"],
            font_size=32,
            weight="BOLD",
        )
        self.from_label.to_edge(UP, buff=1.0)
        self.add(self.from_label)

        self.to_label = Text(
            to_data["name"],
            color=to_data["color"],
            font_size=32,
            weight="BOLD",
        )
        self.to_label.to_edge(UP, buff=1.0)
        self.to_label.set_opacity(0)
        self.add(self.to_label)

        # Power display
        self.power_display = MathTex(
            f"10^{{{from_data['power']}}} \\text{{ m}}",
            color=TRD_COLORS["text"],
            font_size=24,
        )
        self.power_display.next_to(self.from_label, DOWN, buff=0.3)
        self.add(self.power_display)

        # Zoom indicator
        self.zoom_indicator = Circle(
            radius=2.0,
            stroke_color=from_data["color"],
            stroke_width=3,
            fill_opacity=0,
        )
        self.add(self.zoom_indicator)

    def transition_animation(self, run_time: float = 3.0) -> Animation:
        """
        Animate the scale transition.

        Returns
        -------
        Animation
            Transition animation
        """
        from_data = SCALE_LEVELS.get(self.from_scale)
        to_data = SCALE_LEVELS.get(self.to_scale)

        def update_transition(mob, alpha):
            # Interpolate power
            power = from_data["power"] + alpha * (to_data["power"] - from_data["power"])

            # Update power display
            self.power_display.become(
                MathTex(
                    f"10^{{{int(power)}}} \\text{{ m}}",
                    color=TRD_COLORS["text"],
                    font_size=24,
                ).next_to(self.from_label, DOWN, buff=0.3)
            )

            # Fade labels
            self.from_label.set_opacity(1 - alpha)
            self.to_label.set_opacity(alpha)

            # Color transition
            current_color = lerp_color(
                from_data["color"],
                to_data["color"],
                alpha,
            )
            self.zoom_indicator.set_stroke(color=current_color)

            # Scale zoom indicator (simulate zooming in)
            if alpha < 0.5:
                # Zoom out from current
                scale = 1 + alpha * 2
            else:
                # Zoom in to target
                scale = 2 - (alpha - 0.5) * 2
            # Note: actual scaling would need to be done differently

        return UpdateFromAlphaFunc(self, update_transition, run_time=run_time)


class ScaleJourney(VGroup):
    """
    Complete journey through all scales.

    Animates from Planck to cosmic scale with stops at each level.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Build the scale indicator
        self._current_scale_idx = 0
        self._scales = list(SCALE_LEVELS.keys())

        self._build_journey()

    def _build_journey(self):
        """Build the journey visualization."""
        # Central display area
        self.display_circle = Circle(
            radius=2.5,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=2,
            fill_color=TRD_COLORS["background"],
            fill_opacity=0.8,
        )
        self.add(self.display_circle)

        # Current scale name
        first_scale = SCALE_LEVELS[self._scales[0]]
        self.scale_name = Text(
            first_scale["name"],
            color=first_scale["color"],
            font_size=36,
            weight="BOLD",
        )
        self.add(self.scale_name)

        # Power indicator
        self.power_text = MathTex(
            f"10^{{{first_scale['power']}}} \\text{{ m}}",
            color=TRD_COLORS["text"],
            font_size=28,
        )
        self.power_text.next_to(self.scale_name, DOWN, buff=0.3)
        self.add(self.power_text)

        # Description
        self.description = Text(
            first_scale["description"],
            color=TRD_COLORS["text_dim"],
            font_size=18,
        )
        self.description.next_to(self.power_text, DOWN, buff=0.2)
        self.add(self.description)

        # Progress dots
        self.progress_dots = VGroup()
        dot_spacing = 0.4
        start_x = -(len(self._scales) - 1) * dot_spacing / 2
        for i, key in enumerate(self._scales):
            data = SCALE_LEVELS[key]
            dot = Dot(
                point=np.array([start_x + i * dot_spacing, -3.2, 0]),
                radius=0.08,
                color=data["color"] if i == 0 else TRD_COLORS["grid"],
            )
            self.progress_dots.add(dot)
        self.add(self.progress_dots)

    def _update_to_scale(self, scale_idx: int):
        """Update display to show specific scale."""
        scale_key = self._scales[scale_idx]
        scale_data = SCALE_LEVELS[scale_key]

        # Update name
        self.scale_name.become(
            Text(
                scale_data["name"],
                color=scale_data["color"],
                font_size=36,
                weight="BOLD",
            ).move_to(ORIGIN)
        )

        # Update power
        self.power_text.become(
            MathTex(
                f"10^{{{scale_data['power']}}} \\text{{ m}}",
                color=TRD_COLORS["text"],
                font_size=28,
            ).next_to(self.scale_name, DOWN, buff=0.3)
        )

        # Update description
        self.description.become(
            Text(
                scale_data["description"],
                color=TRD_COLORS["text_dim"],
                font_size=18,
            ).next_to(self.power_text, DOWN, buff=0.2)
        )

        # Update progress dots
        for i, dot in enumerate(self.progress_dots):
            if i <= scale_idx:
                dot.set_color(SCALE_LEVELS[self._scales[i]]["color"])
            else:
                dot.set_color(TRD_COLORS["grid"])

        # Update circle color
        self.display_circle.set_stroke(color=scale_data["color"])

    def journey_animation(self, run_time: float = 20.0) -> Animation:
        """
        Animate the complete journey through all scales.

        Parameters
        ----------
        run_time : float
            Total animation duration

        Returns
        -------
        Animation
            Complete journey animation
        """
        num_scales = len(self._scales)

        def update_journey(mob, alpha):
            # Determine which scale we're at
            scale_progress = alpha * (num_scales - 1)
            scale_idx = int(scale_progress)
            scale_idx = min(scale_idx, num_scales - 1)

            if scale_idx != self._current_scale_idx:
                self._current_scale_idx = scale_idx
                self._update_to_scale(scale_idx)

        return UpdateFromAlphaFunc(self, update_journey, run_time=run_time)


class ZoomPulse(VGroup):
    """
    Animated zoom pulse effect.

    Creates expanding/contracting rings to suggest scale change.
    """

    def __init__(
        self,
        center: np.ndarray = ORIGIN,
        color: str = TRD_COLORS["highlight"],
        num_rings: int = 4,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.center = center
        self.color = color
        self.num_rings = num_rings

        self._rings: list[Circle] = []
        self._build_rings()

    def _build_rings(self):
        """Create the pulse rings."""
        for i in range(self.num_rings):
            ring = Circle(
                radius=0.1,
                stroke_color=self.color,
                stroke_width=3 - i * 0.5,
                stroke_opacity=0,
                fill_opacity=0,
            )
            ring.move_to(self.center)
            self._rings.append(ring)
            self.add(ring)

    def pulse_out(self, max_radius: float = 4.0, run_time: float = 1.5) -> Animation:
        """
        Animate rings expanding outward.

        Parameters
        ----------
        max_radius : float
            Maximum ring radius
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Outward pulse animation
        """
        def update_pulse(mob, alpha):
            for i, ring in enumerate(self._rings):
                # Stagger the rings
                ring_alpha = max(0, min(1, (alpha - i * 0.1) / 0.6))

                if ring_alpha <= 0:
                    ring.set_stroke(opacity=0)
                else:
                    radius = ring_alpha * max_radius
                    opacity = (1 - ring_alpha) * 0.8

                    ring.become(
                        Circle(
                            radius=max(0.1, radius),
                            stroke_color=self.color,
                            stroke_width=3 - i * 0.5,
                            stroke_opacity=opacity,
                            fill_opacity=0,
                        ).move_to(self.center)
                    )

        return UpdateFromAlphaFunc(self, update_pulse, run_time=run_time)

    def pulse_in(self, start_radius: float = 4.0, run_time: float = 1.5) -> Animation:
        """
        Animate rings contracting inward.

        Parameters
        ----------
        start_radius : float
            Starting ring radius
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Inward pulse animation
        """
        def update_pulse(mob, alpha):
            for i, ring in enumerate(self._rings):
                # Stagger the rings (outer first)
                ring_alpha = max(0, min(1, (alpha - (self.num_rings - 1 - i) * 0.1) / 0.6))

                if ring_alpha <= 0:
                    ring.set_stroke(opacity=0)
                else:
                    radius = start_radius * (1 - ring_alpha)
                    opacity = ring_alpha * 0.8

                    ring.become(
                        Circle(
                            radius=max(0.1, radius),
                            stroke_color=self.color,
                            stroke_width=3 - i * 0.5,
                            stroke_opacity=opacity,
                            fill_opacity=0,
                        ).move_to(self.center)
                    )

        return UpdateFromAlphaFunc(self, update_pulse, run_time=run_time)
