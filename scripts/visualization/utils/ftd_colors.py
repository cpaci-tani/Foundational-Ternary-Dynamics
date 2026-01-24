"""
FTD Color Scheme
================
Unified color palette for all FTD visualizations.
Designed for pedagogical clarity and visual consistency.

Author: FTD Visualization Suite
Date: January 2026
"""

# =============================================================================
# PRIMARY TERNARY STATE COLORS
# =============================================================================

# The fundamental ontological states
VOID = '#888888'           # Gray - unmanifested substrate (state 0)
MATTER = '#DD4444'         # Red - positive manifestation (state +1)
ANTIMATTER = '#4488DD'     # Blue - negative manifestation (state -1)

# =============================================================================
# FLUX FIELD COLORS
# =============================================================================

FLUX = '#FFD700'           # Gold - the flux field J(v,t)
FLUX_LOW = '#FFE066'       # Light gold - low flux magnitude
FLUX_HIGH = '#CC9900'      # Dark gold - high flux magnitude
FLUX_GRADIENT = ['#FFE066', '#FFD700', '#CC9900']

# =============================================================================
# FORCE COLORS (Standard Model correspondence)
# =============================================================================

STRONG = '#FF6B35'         # Orange - strong force / SU(3)
WEAK = '#9B59B6'           # Purple - weak force / SU(2)
ELECTROMAGNETIC = '#3498DB' # Electric blue - EM / U(1)
GRAVITY = '#27AE60'        # Green - gravitational interaction

# Shorthand aliases
EM = ELECTROMAGNETIC
GRAV = GRAVITY

# =============================================================================
# PARTICLE TYPE COLORS
# =============================================================================

# Quarks (by generation)
QUARK_GEN1 = '#E74C3C'     # First generation (u, d)
QUARK_GEN2 = '#E67E22'     # Second generation (c, s)
QUARK_GEN3 = '#9B59B6'     # Third generation (t, b)

# Leptons (by generation)
LEPTON_GEN1 = '#3498DB'    # First generation (e, νe)
LEPTON_GEN2 = '#1ABC9C'    # Second generation (μ, νμ)
LEPTON_GEN3 = '#2ECC71'    # Third generation (τ, ντ)

# Bosons
PHOTON = '#F1C40F'         # Yellow - photon
W_BOSON = '#9B59B6'        # Purple - W±
Z_BOSON = '#8E44AD'        # Dark purple - Z⁰
GLUON = '#E67E22'          # Orange - gluon
HIGGS = '#1ABC9C'          # Teal - Higgs

# =============================================================================
# MATHEMATICAL ELEMENTS
# =============================================================================

EQUATION = '#FFFFFF'       # White - equations
LABEL = '#CCCCCC'          # Light gray - labels
HIGHLIGHT = '#F39C12'      # Amber - highlighted elements
DERIVATION = '#3498DB'     # Blue - derivation steps
RESULT = '#2ECC71'         # Green - final results

# =============================================================================
# BACKGROUND AND UI
# =============================================================================

BACKGROUND = '#0D1117'     # Deep space black
BACKGROUND_LIGHT = '#1C2128'  # Slightly lighter for contrast
GRID = '#333333'           # Grid lines
AXES = '#555555'           # Axis lines

# =============================================================================
# SPECIAL ELEMENTS
# =============================================================================

# The Four Integers
INTEGER_3 = '#E74C3C'      # N_c = 3 (color charges)
INTEGER_4 = '#F39C12'      # N_base = 4 (Fermat boundary)
INTEGER_7 = '#9B59B6'      # b_3 = 7 (QCD beta)
INTEGER_13 = '#3498DB'     # n_eff = 13 (effective DoF)

# Lemniscate visualization
LEMNISCATE = '#FFD700'     # Gold curve
LEMNISCATE_MODES = [
    '#FF6B6B',             # Mode 1 - red
    '#4ECDC4',             # Mode 2 - teal
    '#45B7D1',             # Mode 3 - blue
    '#96CEB4',             # Mode 4 - green
    '#FFEAA7',             # Mode 5 - yellow
]

# =============================================================================
# COLOR DICTIONARIES FOR PROGRAMMATIC ACCESS
# =============================================================================

TRD_COLORS = {
    'void': VOID,
    'matter': MATTER,
    'antimatter': ANTIMATTER,
    'flux': FLUX,
    'strong': STRONG,
    'weak': WEAK,
    'em': ELECTROMAGNETIC,
    'gravity': GRAVITY,
    'highlight': HIGHLIGHT,
    'background': BACKGROUND,
}

STATE_COLORS = {
    -1: ANTIMATTER,
    0: VOID,
    1: MATTER,
}

FORCE_COLORS = {
    'strong': STRONG,
    'weak': WEAK,
    'electromagnetic': ELECTROMAGNETIC,
    'gravity': GRAVITY,
}

INTEGER_COLORS = {
    3: INTEGER_3,
    4: INTEGER_4,
    7: INTEGER_7,
    13: INTEGER_13,
}

# =============================================================================
# MANIM-SPECIFIC COLOR OBJECTS
# =============================================================================

def get_manim_colors():
    """Return colors as Manim Color objects (import manim first)."""
    try:
        from manim import ManimColor
        return {
            'void': ManimColor(VOID),
            'matter': ManimColor(MATTER),
            'antimatter': ManimColor(ANTIMATTER),
            'flux': ManimColor(FLUX),
            'strong': ManimColor(STRONG),
            'weak': ManimColor(WEAK),
            'em': ManimColor(ELECTROMAGNETIC),
            'gravity': ManimColor(GRAVITY),
        }
    except ImportError:
        return TRD_COLORS

# =============================================================================
# PLOTLY-SPECIFIC COLOR SCALES
# =============================================================================

PLOTLY_FLUX_SCALE = [
    [0.0, VOID],
    [0.5, FLUX_LOW],
    [1.0, FLUX],
]

PLOTLY_STATE_SCALE = [
    [0.0, ANTIMATTER],
    [0.5, VOID],
    [1.0, MATTER],
]

PLOTLY_MASS_SCALE = [
    [0.0, '#3498DB'],      # Low mass (blue)
    [0.5, '#F39C12'],      # Mid mass (amber)
    [1.0, '#E74C3C'],      # High mass (red)
]

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

def hex_to_rgb(hex_color):
    """Convert hex color to RGB tuple (0-1 range)."""
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) / 255 for i in (0, 2, 4))

def hex_to_rgba(hex_color, alpha=1.0):
    """Convert hex color to RGBA tuple (0-1 range)."""
    r, g, b = hex_to_rgb(hex_color)
    return (r, g, b, alpha)

def state_to_color(state):
    """Return color for a given ternary state (-1, 0, +1)."""
    return STATE_COLORS.get(state, VOID)

def interpolate_color(color1, color2, t):
    """Linear interpolation between two hex colors."""
    r1, g1, b1 = hex_to_rgb(color1)
    r2, g2, b2 = hex_to_rgb(color2)
    r = r1 + (r2 - r1) * t
    g = g1 + (g2 - g1) * t
    b = b1 + (b2 - b1) * t
    return f'#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}'

def flux_magnitude_color(magnitude, threshold=0.511):
    """Return color based on flux magnitude relative to threshold KB."""
    if magnitude < threshold * 0.5:
        return VOID
    elif magnitude < threshold:
        t = magnitude / threshold
        return interpolate_color(VOID, FLUX_LOW, t)
    else:
        t = min((magnitude - threshold) / threshold, 1.0)
        return interpolate_color(FLUX_LOW, FLUX, t)
