"""
TRD Base Scene Class
====================

Base class for all TRD animations with:
- Dark cinematic background
- Narration timing marker system
- Consistent styling for titles, equations, and concepts
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from manim import (
    DOWN,
    LEFT,
    RIGHT,
    UP,
    ORIGIN,
    config,
    FadeIn,
    FadeOut,
    Scene,
    Text,
    Tex,
    MathTex,
    VGroup,
    Rectangle,
    RoundedRectangle,
    Write,
    Create,
    Indicate,
)

from .colors import TRD_COLORS
from .config import (
    CONTENT_DIR,
    NARRATION_DIR,
    CHAPTERS_DIR,
    TIMING,
    OUTPUT_DIR,
)


@dataclass
class TimingMarker:
    """A timing marker for narration synchronization."""
    scene_id: str
    timestamp: float
    label: str | None = None


@dataclass
class NarrationData:
    """Parsed narration data from JSON."""
    chapter_id: str
    segments: dict[str, float] = field(default_factory=dict)  # scene_id -> duration
    total_duration: float = 0.0


class TRDScene(Scene):
    """
    Base class for all TRD animations.

    Features:
    - Dark cinematic background (#0a0a14)
    - Narration timing marker system
    - Consistent title/equation/concept styling
    - Automatic marker export

    Usage:
        class MyScene(TRDScene):
            def construct(self):
                self.load_narration("1.2")

                self.add_marker("1.2.0.1", "intro")
                title = self.trd_title("The First Division")
                self.play(FadeIn(title))
                self.wait_for_narration("1.2.0.1")

                self.export_markers()
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._markers: list[TimingMarker] = []
        self._narration: NarrationData | None = None
        self._chapter_id: str | None = None
        self._playhead: float = 0.0

    def setup(self):
        """Called before construct(). Sets up dark background."""
        super().setup()
        # Set dark background
        self.camera.background_color = TRD_COLORS["background"]

    # =========================================================================
    # NARRATION TIMING SYSTEM
    # =========================================================================

    def load_narration(self, chapter_id: str) -> NarrationData | None:
        """
        Load narration timing data from content/narration/{chapter_id}-narration.json.

        Parameters
        ----------
        chapter_id : str
            Chapter identifier (e.g., "1.2")

        Returns
        -------
        NarrationData or None
            Parsed narration data, or None if not found
        """
        self._chapter_id = chapter_id

        # Try standard naming
        narration_file = NARRATION_DIR / f"{chapter_id}-narration.json"
        if not narration_file.exists():
            # Try alternative naming (e.g., "1.1-the-void.json")
            matches = list(NARRATION_DIR.glob(f"{chapter_id}-*.json"))
            if matches:
                narration_file = matches[0]
            else:
                return None

        try:
            with narration_file.open("r", encoding="utf-8") as f:
                data = json.load(f)

            segments = {}
            total = 0.0

            for seg in data.get("segments", []):
                if not isinstance(seg, dict):
                    continue
                scene_id = seg.get("sceneId", "")
                if not scene_id:
                    continue
                duration = float(seg.get("duration", 0) or 0)
                pause = float(seg.get("pauseAfter", 0) or 0)
                total_seg = duration + pause
                segments[scene_id] = segments.get(scene_id, 0) + total_seg
                total += total_seg

            self._narration = NarrationData(
                chapter_id=chapter_id,
                segments=segments,
                total_duration=total,
            )
            return self._narration

        except Exception as e:
            print(f"Warning: Could not load narration for {chapter_id}: {e}")
            return None

    def add_marker(self, scene_id: str, label: str | None = None):
        """
        Add a timing marker at the current playhead position.

        Parameters
        ----------
        scene_id : str
            Scene identifier matching narration JSON (e.g., "1.2.0.1")
        label : str, optional
            Human-readable label for the marker
        """
        marker = TimingMarker(
            scene_id=scene_id,
            timestamp=self._playhead,
            label=label,
        )
        self._markers.append(marker)

    def wait_for_narration(self, scene_id: str, min_wait: float = 0.5):
        """
        Wait for the duration specified in narration JSON for this scene.

        Parameters
        ----------
        scene_id : str
            Scene identifier
        min_wait : float
            Minimum wait time if scene not found in narration
        """
        if self._narration and scene_id in self._narration.segments:
            duration = self._narration.segments[scene_id]
        else:
            duration = min_wait

        # Add buffer
        duration = max(duration, min_wait) + TIMING["marker_buffer"]
        self.wait(duration)
        self._playhead += duration

    def export_markers(self, output_path: Path | None = None):
        """
        Export timing markers to sidecar JSON for audio sync.

        Parameters
        ----------
        output_path : Path, optional
            Output file path. Defaults to output/{chapter_id}_markers.json
        """
        if not output_path:
            chapter_id = self._chapter_id or "unknown"
            output_path = OUTPUT_DIR / f"{chapter_id}_markers.json"

        output_path.parent.mkdir(parents=True, exist_ok=True)

        data = {
            "chapter_id": self._chapter_id,
            "markers": [
                {
                    "scene_id": m.scene_id,
                    "timestamp": round(m.timestamp, 3),
                    "label": m.label,
                }
                for m in self._markers
            ],
            "total_duration": round(self._playhead, 3),
        }

        with output_path.open("w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)

        print(f"Exported {len(self._markers)} markers to {output_path}")

    # =========================================================================
    # OVERRIDE WAIT TO TRACK PLAYHEAD
    # =========================================================================

    def wait(self, duration: float = 1.0, **kwargs):
        """Wait and track playhead position."""
        super().wait(duration, **kwargs)
        self._playhead += duration

    def play(self, *args, **kwargs):
        """Play animations and track playhead position."""
        # Estimate duration from run_time
        run_time = kwargs.get("run_time", 1.0)
        super().play(*args, **kwargs)
        self._playhead += run_time

    # =========================================================================
    # STYLED ELEMENTS
    # =========================================================================

    def trd_title(
        self,
        text: str,
        subtitle: str | None = None,
        position: Any = UP * 3,
    ) -> VGroup:
        """
        Create a styled title card.

        Parameters
        ----------
        text : str
            Main title text
        subtitle : str, optional
            Subtitle text
        position : np.ndarray
            Position for the title group

        Returns
        -------
        VGroup
            Title mobject group
        """
        title = Text(
            text,
            color=TRD_COLORS["text"],
            font_size=48,
            weight="BOLD",
        )

        group = VGroup(title)

        if subtitle:
            sub = Text(
                subtitle,
                color=TRD_COLORS["text_dim"],
                font_size=28,
            )
            sub.next_to(title, DOWN, buff=0.3)
            group.add(sub)

        group.move_to(position)
        return group

    def equation_box(
        self,
        latex: str,
        label: str | None = None,
        position: Any = ORIGIN,
    ) -> VGroup:
        """
        Create a styled equation display with optional label.

        Parameters
        ----------
        latex : str
            LaTeX equation string
        label : str, optional
            Label above the equation
        position : np.ndarray
            Position for the equation box

        Returns
        -------
        VGroup
            Equation box mobject group
        """
        equation = MathTex(latex, color=TRD_COLORS["text"])

        # Create background box
        box = RoundedRectangle(
            width=equation.width + 1.0,
            height=equation.height + 0.8,
            corner_radius=0.15,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.8,
            stroke_color=TRD_COLORS["highlight"],
            stroke_width=2,
        )

        group = VGroup(box, equation)

        if label:
            lbl = Text(
                label,
                color=TRD_COLORS["highlight"],
                font_size=20,
            )
            lbl.next_to(box, UP, buff=0.2)
            group.add(lbl)

        group.move_to(position)
        return group

    def concept_card(
        self,
        title: str,
        description: str,
        position: Any = ORIGIN,
    ) -> VGroup:
        """
        Create a concept explanation card.

        Parameters
        ----------
        title : str
            Concept title
        description : str
            Brief description
        position : np.ndarray
            Position for the card

        Returns
        -------
        VGroup
            Concept card mobject group
        """
        title_text = Text(
            title,
            color=TRD_COLORS["highlight"],
            font_size=32,
            weight="BOLD",
        )

        desc_text = Text(
            description,
            color=TRD_COLORS["text"],
            font_size=22,
            line_spacing=1.2,
        )
        desc_text.next_to(title_text, DOWN, buff=0.3)

        content = VGroup(title_text, desc_text)

        # Background card
        card = RoundedRectangle(
            width=content.width + 1.0,
            height=content.height + 0.8,
            corner_radius=0.2,
            fill_color=TRD_COLORS["background_light"],
            fill_opacity=0.9,
            stroke_color=TRD_COLORS["grid_bright"],
            stroke_width=1,
        )
        card.move_to(content.get_center())

        group = VGroup(card, content)
        group.move_to(position)
        return group

    def chapter_intro(
        self,
        chapter_id: str,
        title: str,
        subtitle: str | None = None,
    ):
        """
        Standard chapter introduction animation.

        Parameters
        ----------
        chapter_id : str
            Chapter number (e.g., "1.2")
        title : str
            Chapter title
        subtitle : str, optional
            Chapter subtitle
        """
        # Chapter number
        ch_num = Text(
            f"Chapter {chapter_id}",
            color=TRD_COLORS["text_dim"],
            font_size=24,
        )
        ch_num.to_edge(UP, buff=0.8)

        # Title
        title_text = Text(
            title,
            color=TRD_COLORS["text"],
            font_size=52,
            weight="BOLD",
        )
        title_text.next_to(ch_num, DOWN, buff=0.4)

        elements = [ch_num, title_text]

        if subtitle:
            sub = Text(
                subtitle,
                color=TRD_COLORS["text_dim"],
                font_size=28,
            )
            sub.next_to(title_text, DOWN, buff=0.3)
            elements.append(sub)

        # Animate
        for elem in elements:
            self.play(FadeIn(elem, shift=DOWN * 0.2), run_time=0.6)

        self.wait(TIMING["title_hold"])

        # Fade out
        self.play(*[FadeOut(e) for e in elements], run_time=0.5)

    # =========================================================================
    # UTILITY METHODS
    # =========================================================================

    def fade_all(self, run_time: float = 0.5):
        """Fade out all mobjects currently on screen."""
        self.play(
            *[FadeOut(mob) for mob in self.mobjects],
            run_time=run_time,
        )

    def transition_wipe(self, direction: Any = LEFT, run_time: float = 0.5):
        """Wipe transition in specified direction."""
        # Create wipe rectangle
        wipe = Rectangle(
            width=config.frame_width + 2,
            height=config.frame_height + 2,
            fill_color=TRD_COLORS["background"],
            fill_opacity=1,
            stroke_opacity=0,
        )

        if direction is LEFT:
            wipe.to_edge(RIGHT, buff=-config.frame_width)
        elif direction is RIGHT:
            wipe.to_edge(LEFT, buff=-config.frame_width)
        elif direction is UP:
            wipe.to_edge(DOWN, buff=-config.frame_height)
        else:
            wipe.to_edge(UP, buff=-config.frame_height)

        self.play(
            wipe.animate.move_to(ORIGIN),
            run_time=run_time,
        )
        self.remove(wipe)
        for mob in self.mobjects.copy():
            if mob is not wipe:
                self.remove(mob)
