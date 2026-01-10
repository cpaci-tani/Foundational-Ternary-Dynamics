"""
TRD Animation Configuration
===========================

Render settings, paths, and global configuration for 1080p60 animations.
"""

from pathlib import Path

# =============================================================================
# PATHS
# =============================================================================

# Root of the animation package
ANIMATIONS_DIR = Path(__file__).resolve().parent.parent

# Output directories
OUTPUT_DIR = ANIMATIONS_DIR / "output"
OUTPUT_1080P60 = OUTPUT_DIR / "1080p60"
OUTPUT_PREVIEW = OUTPUT_DIR / "preview"

# Content directories (relative to project root)
PROJECT_ROOT = ANIMATIONS_DIR.parent.parent.parent
CONTENT_DIR = PROJECT_ROOT / "content"
CHAPTERS_DIR = CONTENT_DIR / "chapters"
NARRATION_DIR = CONTENT_DIR / "narration"
CONCEPTS_DIR = CONTENT_DIR / "concepts"

# Reference files
STYLE_FILE = PROJECT_ROOT / "dissemination" / "manuscript" / "figures" / "utils" / "style.py"
CONSTANTS_FILE = PROJECT_ROOT / "dissemination" / "manuscript" / "figures" / "utils" / "physics_constants.py"

# =============================================================================
# RENDER CONFIGURATION
# =============================================================================

RENDER_CONFIG = {
    "production": {
        "pixel_width": 1920,
        "pixel_height": 1080,
        "frame_rate": 60,
        "format": "mp4",
        "codec": "libx264",
        "quality": "high_quality",
        "preview": False,
    },
    "preview": {
        "pixel_width": 854,
        "pixel_height": 480,
        "frame_rate": 30,
        "format": "mp4",
        "quality": "low_quality",
        "preview": True,
    },
    "gif": {
        "pixel_width": 640,
        "pixel_height": 360,
        "frame_rate": 24,
        "format": "gif",
        "quality": "medium_quality",
        "preview": False,
    },
}

# =============================================================================
# MANIM CONFIG OVERRIDES
# =============================================================================

def get_manim_config(quality: str = "production") -> dict:
    """
    Get Manim configuration dictionary for the specified quality.

    Parameters
    ----------
    quality : str
        One of: "production", "preview", "gif"

    Returns
    -------
    dict
        Configuration dictionary for manim.config
    """
    config = RENDER_CONFIG.get(quality, RENDER_CONFIG["production"]).copy()

    return {
        "pixel_width": config["pixel_width"],
        "pixel_height": config["pixel_height"],
        "frame_rate": config["frame_rate"],
        "background_color": "#0a0a14",  # Dark cinematic theme
        "output_file": None,  # Set per-scene
        "media_dir": str(OUTPUT_DIR / quality),
    }

# =============================================================================
# TIMING DEFAULTS
# =============================================================================

# Default durations (in seconds)
TIMING = {
    "title_hold": 2.0,          # How long to hold title cards
    "transition": 0.5,          # Fade transition duration
    "equation_reveal": 1.5,     # Time to reveal an equation
    "concept_hold": 3.0,        # Hold for concept explanation
    "marker_buffer": 0.25,      # Buffer after timing marker
}

# =============================================================================
# SCENE METADATA
# =============================================================================

# Standard aspect ratio
ASPECT_RATIO = 16 / 9

# Safe area margins (percentage from edge)
SAFE_MARGIN = 0.05

# Frame dimensions (Manim default is [-7.1, 7.1] x [-4, 4])
FRAME_WIDTH = 14.2
FRAME_HEIGHT = 8.0
