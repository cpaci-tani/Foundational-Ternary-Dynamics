"""
TRD Manuscript Figure Generation Utilities
==========================================
Shared utilities for generating manuscript figures.
"""

from .style import (
    COLORS,
    FORCE_COLORS,
    MODE_COLORS,
    FONTS,
    FIGURE_SIZES,
    DPI,
    apply_trd_style,
    create_figure,
)

from .physics_constants import (
    # Framework integers
    B3, N_C, N_EFF, N_BASE,
    # Derived constants
    ALPHA, ALPHA_INV, G_STAR, KB, PHI,
    GRAVITY_BIAS, DECAY_RATE,
    SIN2_THETA_W, ALPHA_S,
    # Curve parameters
    FREQUENCIES, X_AMPLITUDES, Y_AMPLITUDES,
    # Particle data
    PARTICLE_MASSES,
    # Helper functions
    triangular,
)

__all__ = [
    # Style
    'COLORS', 'FORCE_COLORS', 'MODE_COLORS', 'FONTS', 'FIGURE_SIZES', 'DPI',
    'apply_trd_style', 'create_figure',
    # Physics
    'B3', 'N_C', 'N_EFF', 'N_BASE',
    'ALPHA', 'ALPHA_INV', 'G_STAR', 'KB', 'PHI',
    'GRAVITY_BIAS', 'DECAY_RATE', 'SIN2_THETA_W', 'ALPHA_S',
    'FREQUENCIES', 'X_AMPLITUDES', 'Y_AMPLITUDES',
    'PARTICLE_MASSES', 'triangular',
]
