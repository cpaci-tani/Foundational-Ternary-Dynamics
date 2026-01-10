"""
TRD Animation Library
=====================

Manim-based animation components for Ternary Realization Dynamics pedagogy.

This library provides:
- TRDScene: Base scene class with dark theme and timing markers
- Reusable components: voxels, flux fields, lattices, waves
- Physics-specific visualizations: lemniscate curve, causal loop, etc.
"""

from .config import RENDER_CONFIG, OUTPUT_DIR, CONTENT_DIR
from .colors import TRD_COLORS, GLOW_COLORS, MODE_COLORS, PHASE_COLORS
from .trd_scene import TRDScene

__all__ = [
    "RENDER_CONFIG",
    "OUTPUT_DIR",
    "CONTENT_DIR",
    "TRD_COLORS",
    "GLOW_COLORS",
    "MODE_COLORS",
    "PHASE_COLORS",
    "TRDScene",
]

__version__ = "0.1.0"
