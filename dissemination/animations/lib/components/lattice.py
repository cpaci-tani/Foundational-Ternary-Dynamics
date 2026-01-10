"""
Lattice Mobject
===============

3D cubic lattice visualization with glowing edges for TRD space representation.
Shows the discrete structure of spacetime with Moore neighborhood highlighting.
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
    ShowPassingFlash,
    Indicate,
    VGroup,
    VMobject,
    Line,
    Dot,
    Square,
    Cube,
    Circle,
    Sphere,
    ThreeDScene,
    rate_functions,
)

from ..colors import TRD_COLORS, GLOW_COLORS


class LatticeEdge(VGroup):
    """
    A single lattice edge with glow effect.

    Parameters
    ----------
    start : np.ndarray
        Start point
    end : np.ndarray
        End point
    color : str
        Edge color
    glow : bool
        Whether to show glow effect
    """

    def __init__(
        self,
        start: np.ndarray = ORIGIN,
        end: np.ndarray = RIGHT,
        color: str = TRD_COLORS["grid"],
        glow: bool = True,
        stroke_width: float = 1.5,
        **kwargs,
    ):
        super().__init__(**kwargs)

        # Glow layers (if enabled)
        if glow:
            for i in range(3, 0, -1):
                glow_line = Line(
                    start, end,
                    stroke_color=color,
                    stroke_width=stroke_width * (1 + i * 0.5),
                    stroke_opacity=0.1 / i,
                )
                self.add(glow_line)

        # Main edge
        main_line = Line(
            start, end,
            stroke_color=color,
            stroke_width=stroke_width,
            stroke_opacity=0.7,
        )
        self.add(main_line)


class LatticeNode(VGroup):
    """
    A lattice node (vertex) with optional highlighting.

    Parameters
    ----------
    position : np.ndarray
        Node position
    radius : float
        Node radius
    color : str
        Node color
    glow : bool
        Whether to show glow
    """

    def __init__(
        self,
        position: np.ndarray = ORIGIN,
        radius: float = 0.08,
        color: str = TRD_COLORS["grid_bright"],
        glow: bool = False,
        **kwargs,
    ):
        super().__init__(**kwargs)

        if glow:
            # Glow layers
            for i in range(3, 0, -1):
                glow_dot = Dot(
                    position,
                    radius=radius * (1 + i * 0.5),
                    color=color,
                    fill_opacity=0.15 / i,
                )
                self.add(glow_dot)

        # Main dot
        dot = Dot(
            position,
            radius=radius,
            color=color,
            fill_opacity=0.8,
        )
        self.add(dot)


class Lattice2D(VGroup):
    """
    2D lattice grid visualization.

    Parameters
    ----------
    rows : int
        Number of rows
    cols : int
        Number of columns
    spacing : float
        Grid spacing
    show_nodes : bool
        Whether to show lattice nodes
    show_glow : bool
        Whether to show glow effects
    """

    def __init__(
        self,
        rows: int = 5,
        cols: int = 5,
        spacing: float = 1.0,
        show_nodes: bool = True,
        show_glow: bool = True,
        edge_color: str = TRD_COLORS["grid"],
        node_color: str = TRD_COLORS["grid_bright"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.rows = rows
        self.cols = cols
        self.spacing = spacing

        self._edges: list[LatticeEdge] = []
        self._nodes: dict[tuple[int, int], LatticeNode] = {}

        self._build_lattice(show_nodes, show_glow, edge_color, node_color)

    def _build_lattice(
        self,
        show_nodes: bool,
        show_glow: bool,
        edge_color: str,
        node_color: str,
    ):
        """Construct the lattice."""
        # Calculate offset to center the grid
        x_offset = -(self.cols - 1) * self.spacing / 2
        y_offset = -(self.rows - 1) * self.spacing / 2

        # Create horizontal edges
        for i in range(self.rows):
            for j in range(self.cols - 1):
                x1 = x_offset + j * self.spacing
                x2 = x_offset + (j + 1) * self.spacing
                y = y_offset + i * self.spacing

                edge = LatticeEdge(
                    start=np.array([x1, y, 0]),
                    end=np.array([x2, y, 0]),
                    color=edge_color,
                    glow=show_glow,
                )
                self._edges.append(edge)
                self.add(edge)

        # Create vertical edges
        for i in range(self.rows - 1):
            for j in range(self.cols):
                x = x_offset + j * self.spacing
                y1 = y_offset + i * self.spacing
                y2 = y_offset + (i + 1) * self.spacing

                edge = LatticeEdge(
                    start=np.array([x, y1, 0]),
                    end=np.array([x, y2, 0]),
                    color=edge_color,
                    glow=show_glow,
                )
                self._edges.append(edge)
                self.add(edge)

        # Create nodes
        if show_nodes:
            for i in range(self.rows):
                for j in range(self.cols):
                    x = x_offset + j * self.spacing
                    y = y_offset + i * self.spacing
                    pos = np.array([x, y, 0])

                    node = LatticeNode(
                        position=pos,
                        color=node_color,
                        glow=False,
                    )
                    self._nodes[(i, j)] = node
                    self.add(node)

    def get_node(self, row: int, col: int) -> LatticeNode | None:
        """Get node at specified position."""
        return self._nodes.get((row, col))

    def get_node_position(self, row: int, col: int) -> np.ndarray:
        """Get position of node at specified grid coordinates."""
        x_offset = -(self.cols - 1) * self.spacing / 2
        y_offset = -(self.rows - 1) * self.spacing / 2
        x = x_offset + col * self.spacing
        y = y_offset + row * self.spacing
        return np.array([x, y, 0])

    def highlight_moore_neighborhood(
        self,
        center_row: int,
        center_col: int,
        run_time: float = 1.5,
    ) -> Animation:
        """
        Animate highlighting the Moore neighborhood (8-connected).

        Parameters
        ----------
        center_row : int
            Row of center node
        center_col : int
            Column of center node
        run_time : float
            Animation duration

        Returns
        -------
        Animation
            Highlighting animation
        """
        anims = []

        # Highlight center
        center = self.get_node(center_row, center_col)
        if center:
            center_highlight = LatticeNode(
                position=self.get_node_position(center_row, center_col),
                radius=0.15,
                color=TRD_COLORS["highlight"],
                glow=True,
            )
            anims.append(FadeIn(center_highlight))

        # Highlight neighbors
        for di in [-1, 0, 1]:
            for dj in [-1, 0, 1]:
                if di == 0 and dj == 0:
                    continue
                ni, nj = center_row + di, center_col + dj
                if 0 <= ni < self.rows and 0 <= nj < self.cols:
                    # Determine neighbor type
                    dist = abs(di) + abs(dj)
                    if dist == 1:
                        color = TRD_COLORS["matter"]  # Face neighbor
                    else:
                        color = TRD_COLORS["antimatter"]  # Diagonal neighbor

                    neighbor_highlight = LatticeNode(
                        position=self.get_node_position(ni, nj),
                        radius=0.12,
                        color=color,
                        glow=True,
                    )
                    anims.append(FadeIn(neighbor_highlight, run_time=run_time * 0.5))

        return AnimationGroup(*anims, lag_ratio=0.1)


class Lattice3D(VGroup):
    """
    3D cubic lattice visualization (isometric projection for 2D rendering).

    Parameters
    ----------
    size : int
        Grid size in each dimension
    spacing : float
        Grid spacing
    show_glow : bool
        Whether to show glow effects
    """

    def __init__(
        self,
        size: int = 3,
        spacing: float = 1.0,
        show_glow: bool = True,
        edge_color: str = TRD_COLORS["grid"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.size = size
        self.spacing = spacing

        self._build_lattice_3d(show_glow, edge_color)

    def _project_3d_to_2d(self, x: float, y: float, z: float) -> np.ndarray:
        """
        Isometric projection from 3D to 2D.

        Uses standard isometric angles for visual clarity.
        """
        # Isometric projection matrix
        iso_x = (x - z) * np.cos(PI / 6)
        iso_y = y + (x + z) * np.sin(PI / 6) * 0.5
        return np.array([iso_x, iso_y, 0])

    def _build_lattice_3d(self, show_glow: bool, edge_color: str):
        """Construct the 3D lattice with isometric projection."""
        offset = -(self.size - 1) * self.spacing / 2

        # Create all edges
        for i in range(self.size):
            for j in range(self.size):
                for k in range(self.size):
                    x = offset + i * self.spacing
                    y = offset + j * self.spacing
                    z = offset + k * self.spacing

                    # X-direction edges
                    if i < self.size - 1:
                        start = self._project_3d_to_2d(x, y, z)
                        end = self._project_3d_to_2d(x + self.spacing, y, z)
                        edge = LatticeEdge(
                            start=start, end=end,
                            color=edge_color,
                            glow=show_glow,
                            stroke_width=1.0,
                        )
                        self.add(edge)

                    # Y-direction edges
                    if j < self.size - 1:
                        start = self._project_3d_to_2d(x, y, z)
                        end = self._project_3d_to_2d(x, y + self.spacing, z)
                        edge = LatticeEdge(
                            start=start, end=end,
                            color=edge_color,
                            glow=show_glow,
                            stroke_width=1.0,
                        )
                        self.add(edge)

                    # Z-direction edges
                    if k < self.size - 1:
                        start = self._project_3d_to_2d(x, y, z)
                        end = self._project_3d_to_2d(x, y, z + self.spacing)
                        edge = LatticeEdge(
                            start=start, end=end,
                            color=edge_color,
                            glow=show_glow,
                            stroke_width=1.0,
                        )
                        self.add(edge)

        # Add corner nodes for visibility
        for i in [0, self.size - 1]:
            for j in [0, self.size - 1]:
                for k in [0, self.size - 1]:
                    x = offset + i * self.spacing
                    y = offset + j * self.spacing
                    z = offset + k * self.spacing
                    pos = self._project_3d_to_2d(x, y, z)

                    node = LatticeNode(
                        position=pos,
                        radius=0.06,
                        color=TRD_COLORS["grid_bright"],
                        glow=False,
                    )
                    self.add(node)


class MooreNeighborhood(VGroup):
    """
    Visualization of the 26-connected Moore neighborhood in 3D.

    Shows a central voxel and its 26 neighbors with distance-based coloring.
    """

    def __init__(
        self,
        spacing: float = 1.2,
        center_color: str = TRD_COLORS["highlight"],
        face_color: str = TRD_COLORS["matter"],
        edge_color: str = TRD_COLORS["antimatter"],
        corner_color: str = TRD_COLORS["void_light"],
        **kwargs,
    ):
        super().__init__(**kwargs)
        self.spacing = spacing

        self._build_neighborhood(center_color, face_color, edge_color, corner_color)

    def _project(self, x: float, y: float, z: float) -> np.ndarray:
        """Isometric projection."""
        iso_x = (x - z) * np.cos(PI / 6)
        iso_y = y + (x + z) * np.sin(PI / 6) * 0.5
        return np.array([iso_x, iso_y, 0])

    def _build_neighborhood(
        self,
        center_color: str,
        face_color: str,
        edge_color: str,
        corner_color: str,
    ):
        """Build the Moore neighborhood visualization."""
        s = self.spacing

        # Center voxel (largest)
        center_pos = self._project(0, 0, 0)
        center = LatticeNode(
            position=center_pos,
            radius=0.25,
            color=center_color,
            glow=True,
        )
        self.add(center)

        # All 26 neighbors
        for di in [-1, 0, 1]:
            for dj in [-1, 0, 1]:
                for dk in [-1, 0, 1]:
                    if di == 0 and dj == 0 and dk == 0:
                        continue  # Skip center

                    pos = self._project(di * s, dj * s, dk * s)

                    # Classify by distance
                    dist = abs(di) + abs(dj) + abs(dk)
                    if dist == 1:
                        # Face neighbor (6 total)
                        color = face_color
                        radius = 0.18
                    elif dist == 2:
                        # Edge neighbor (12 total)
                        color = edge_color
                        radius = 0.14
                    else:
                        # Corner neighbor (8 total)
                        color = corner_color
                        radius = 0.10

                    node = LatticeNode(
                        position=pos,
                        radius=radius,
                        color=color,
                        glow=True,
                    )
                    self.add(node)

                    # Connection line to center
                    line = Line(
                        center_pos, pos,
                        stroke_color=TRD_COLORS["grid"],
                        stroke_width=0.5,
                        stroke_opacity=0.3,
                    )
                    self.add(line)

    def animate_build(self, run_time: float = 3.0) -> Animation:
        """
        Animate building the neighborhood layer by layer.

        Returns
        -------
        Animation
            Build animation showing center, then face, edge, corner neighbors
        """
        # This would need to track submobjects by type
        # For now, just fade in the whole thing
        return FadeIn(self, run_time=run_time)
