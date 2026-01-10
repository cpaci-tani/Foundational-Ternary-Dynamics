"""
Voxel Mobject
=============

Animated 3D voxel cube with state visualization and glow effects.
Represents the fundamental unit of TRD space.
"""

from __future__ import annotations

import numpy as np
from typing import Sequence

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    OUT,
    IN,
    ORIGIN,
    PI,
    TAU,
    Animation,
    AnimationGroup,
    Create,
    FadeIn,
    FadeOut,
    GrowFromCenter,
    ShrinkToCenter,
    Transform,
    VGroup,
    VMobject,
    Square,
    Circle,
    Polygon,
    Line,
    Dot,
    rate_functions,
)

from ..colors import TRD_COLORS, GLOW_COLORS, get_state_color, get_glow_gradient


class VoxelMobject(VGroup):
    """
    A single voxel with state visualization.

    Represents one lattice site in TRD with state in {-1, 0, +1}.
    Includes glow effects for cinematic appearance.

    Parameters
    ----------
    state : int
        Initial state: -1 (antimatter), 0 (void), +1 (matter)
    size : float
        Side length of the voxel cube
    position : np.ndarray
        3D position (will be projected to 2D for display)
    show_glow : bool
        Whether to show glow effect
    glow_layers : int
        Number of glow layers for the effect
    """

    def __init__(
        self,
        state: int = 0,
        size: float = 1.0,
        position: np.ndarray = ORIGIN,
        show_glow: bool = True,
        glow_layers: int = 3,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.state = state
        self.size = size
        self._show_glow = show_glow
        self._glow_layers = glow_layers

        # Build the voxel visualization
        self._build_voxel()
        self.move_to(position)

    def _build_voxel(self):
        """Construct the voxel visualization."""
        self.submobjects.clear()

        # Get colors for current state
        fill_color = get_state_color(self.state)
        glow_gradient = get_glow_gradient(self.state)

        # Create glow layers (outermost first)
        if self._show_glow and self.state != 0:
            for i in range(self._glow_layers, 0, -1):
                scale = 1.0 + (i * 0.3)
                opacity = 0.15 / i

                glow = self._create_cube_face(
                    size=self.size * scale,
                    fill_color=glow_gradient[min(i, len(glow_gradient) - 1)],
                    fill_opacity=opacity,
                    stroke_opacity=0,
                )
                self.add(glow)

        # Create main cube (isometric-ish projection)
        if self.state == 0:
            # Void state: just an outline
            cube = self._create_cube_outline(
                size=self.size,
                stroke_color=TRD_COLORS["void"],
                stroke_opacity=0.4,
                stroke_width=1,
            )
        else:
            # Manifested state: filled cube with glow
            cube = self._create_cube_face(
                size=self.size,
                fill_color=fill_color,
                fill_opacity=0.9,
                stroke_color=glow_gradient[0],  # White/bright stroke
                stroke_width=2,
                stroke_opacity=0.8,
            )
        self.add(cube)

        # Add inner glow for manifested states
        if self.state != 0 and self._show_glow:
            inner_glow = Circle(
                radius=self.size * 0.3,
                fill_color=glow_gradient[0],  # Core color (white)
                fill_opacity=0.6,
                stroke_opacity=0,
            )
            self.add(inner_glow)

    def _create_cube_face(
        self,
        size: float,
        fill_color: str,
        fill_opacity: float = 1.0,
        stroke_color: str | None = None,
        stroke_width: float = 2,
        stroke_opacity: float = 1.0,
    ) -> VMobject:
        """Create a square face representing the front of a cube."""
        face = Square(
            side_length=size,
            fill_color=fill_color,
            fill_opacity=fill_opacity,
            stroke_color=stroke_color or fill_color,
            stroke_width=stroke_width,
            stroke_opacity=stroke_opacity,
        )
        return face

    def _create_cube_outline(
        self,
        size: float,
        stroke_color: str,
        stroke_opacity: float = 1.0,
        stroke_width: float = 1,
    ) -> VGroup:
        """Create a wireframe cube outline (simplified 2D projection)."""
        # Front face
        front = Square(
            side_length=size,
            stroke_color=stroke_color,
            stroke_width=stroke_width,
            stroke_opacity=stroke_opacity,
            fill_opacity=0,
        )

        # Offset for pseudo-3D effect
        offset = size * 0.25

        # Back face (shifted)
        back = Square(
            side_length=size,
            stroke_color=stroke_color,
            stroke_width=stroke_width * 0.5,
            stroke_opacity=stroke_opacity * 0.5,
            fill_opacity=0,
        )
        back.shift(UP * offset + RIGHT * offset)

        # Connecting lines
        corners_front = [
            front.get_corner(d) for d in [UP + LEFT, UP + RIGHT, DOWN + RIGHT, DOWN + LEFT]
        ]
        corners_back = [
            back.get_corner(d) for d in [UP + LEFT, UP + RIGHT, DOWN + RIGHT, DOWN + LEFT]
        ]

        lines = VGroup()
        for cf, cb in zip(corners_front, corners_back):
            line = Line(
                cf, cb,
                stroke_color=stroke_color,
                stroke_width=stroke_width * 0.5,
                stroke_opacity=stroke_opacity * 0.3,
            )
            lines.add(line)

        return VGroup(back, lines, front)

    # =========================================================================
    # STATE TRANSITIONS
    # =========================================================================

    def set_state(self, new_state: int):
        """
        Immediately set state (no animation).

        Parameters
        ----------
        new_state : int
            New state: -1, 0, or +1
        """
        self.state = new_state
        self._build_voxel()

    def genesis(self, target_state: int = 1, run_time: float = 1.0) -> Animation:
        """
        Animate genesis: transition from void (0) to manifested state (±1).

        Parameters
        ----------
        target_state : int
            Target state: +1 (matter) or -1 (antimatter)
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            The genesis animation
        """
        if self.state != 0:
            raise ValueError("Genesis requires void state (0)")

        # Create the target voxel
        target = VoxelMobject(
            state=target_state,
            size=self.size,
            position=self.get_center(),
            show_glow=self._show_glow,
            glow_layers=self._glow_layers,
        )

        # Store state for after animation
        def update_state(mob, alpha):
            if alpha >= 1.0:
                mob.state = target_state

        return AnimationGroup(
            GrowFromCenter(target, rate_func=rate_functions.ease_out_back),
            FadeOut(self, rate_func=rate_functions.ease_in),
            run_time=run_time,
        )

    def evaporate(self, run_time: float = 1.0) -> Animation:
        """
        Animate evaporation: transition from manifested state to void.

        Parameters
        ----------
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            The evaporation animation
        """
        if self.state == 0:
            raise ValueError("Evaporation requires manifested state (±1)")

        # Create void voxel to transition to
        target = VoxelMobject(
            state=0,
            size=self.size,
            position=self.get_center(),
            show_glow=False,
        )

        return AnimationGroup(
            ShrinkToCenter(self.copy(), rate_func=rate_functions.ease_in_back),
            FadeIn(target, rate_func=rate_functions.ease_out),
            run_time=run_time,
        )

    def pulse(self, scale_factor: float = 1.2, run_time: float = 0.5) -> Animation:
        """
        Animate a pulse effect (grow and shrink).

        Parameters
        ----------
        scale_factor : float
            Maximum scale during pulse
        run_time : float
            Total animation duration

        Returns
        -------
        Animation
            The pulse animation
        """
        return AnimationGroup(
            self.animate.scale(scale_factor),
            self.animate.scale(1 / scale_factor),
            lag_ratio=0.5,
            run_time=run_time,
        )


class VoxelGrid(VGroup):
    """
    A grid of voxels representing a portion of the TRD lattice.

    Parameters
    ----------
    rows : int
        Number of rows
    cols : int
        Number of columns
    voxel_size : float
        Size of each voxel
    spacing : float
        Spacing between voxels
    default_state : int
        Default state for all voxels
    """

    def __init__(
        self,
        rows: int = 5,
        cols: int = 5,
        voxel_size: float = 0.8,
        spacing: float = 1.0,
        default_state: int = 0,
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.rows = rows
        self.cols = cols
        self.voxel_size = voxel_size
        self.spacing = spacing

        self._voxels: dict[tuple[int, int], VoxelMobject] = {}
        self._build_grid(default_state)

    def _build_grid(self, default_state: int):
        """Construct the voxel grid."""
        for i in range(self.rows):
            for j in range(self.cols):
                # Calculate position
                x = (j - self.cols / 2 + 0.5) * self.spacing
                y = (i - self.rows / 2 + 0.5) * self.spacing
                pos = np.array([x, y, 0])

                voxel = VoxelMobject(
                    state=default_state,
                    size=self.voxel_size,
                    position=pos,
                    show_glow=default_state != 0,
                )
                self._voxels[(i, j)] = voxel
                self.add(voxel)

    def get_voxel(self, row: int, col: int) -> VoxelMobject | None:
        """Get voxel at specified grid position."""
        return self._voxels.get((row, col))

    def set_state(self, row: int, col: int, state: int):
        """Set state of voxel at specified position."""
        voxel = self.get_voxel(row, col)
        if voxel:
            voxel.set_state(state)

    def get_neighbors(self, row: int, col: int) -> list[VoxelMobject]:
        """Get Moore neighborhood (8-connected) voxels."""
        neighbors = []
        for di in [-1, 0, 1]:
            for dj in [-1, 0, 1]:
                if di == 0 and dj == 0:
                    continue
                ni, nj = row + di, col + dj
                if 0 <= ni < self.rows and 0 <= nj < self.cols:
                    voxel = self.get_voxel(ni, nj)
                    if voxel:
                        neighbors.append(voxel)
        return neighbors

    def highlight_neighborhood(
        self,
        center_row: int,
        center_col: int,
        run_time: float = 1.0,
    ) -> Animation:
        """
        Animate highlighting the Moore neighborhood of a voxel.

        Parameters
        ----------
        center_row : int
            Row of center voxel
        center_col : int
            Column of center voxel
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            The highlighting animation
        """
        center = self.get_voxel(center_row, center_col)
        neighbors = self.get_neighbors(center_row, center_col)

        anims = []
        if center:
            anims.append(center.animate.set_color(TRD_COLORS["highlight"]))
        for n in neighbors:
            anims.append(n.animate.set_color(TRD_COLORS["highlight_dim"]))

        return AnimationGroup(*anims, run_time=run_time)
