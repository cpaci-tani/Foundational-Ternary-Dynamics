"""
TRD Animation Color Scheme
==========================

Dark cinematic theme with glowing particles for TRD visualizations.
All colors optimized for dark background (#0a0a14).
"""

# =============================================================================
# CORE COLORS (Dark Cinematic Theme)
# =============================================================================

TRD_COLORS = {
    # Background and base tones
    "background": "#0a0a14",      # Deep space blue-black
    "background_light": "#12121e", # Slightly lighter for layering
    "void": "#2a2a3a",            # Subtle gray for void substrate
    "void_light": "#3a3a4a",      # Lighter void for highlights

    # Manifestation states
    "matter": "#ff4466",          # Glowing red-pink for +1 state
    "matter_bright": "#ff6688",   # Brighter matter highlight
    "antimatter": "#44aaff",      # Glowing cyan-blue for -1 state
    "antimatter_bright": "#66ccff", # Brighter antimatter highlight

    # UI and text
    "text": "#e0e0e0",            # Light gray text
    "text_dim": "#888899",        # Dimmed text
    "highlight": "#ffcc00",       # Golden emphasis
    "highlight_dim": "#cc9900",   # Dimmed gold

    # Structural elements
    "grid": "#1a1a2a",            # Subtle grid lines
    "grid_bright": "#2a2a4a",     # Brighter grid for emphasis
    "edge": "#3a3a5a",            # Edge highlights

    # Core glow
    "glow": "#ffffff",            # White glow core
    "glow_dim": "#aaaacc",        # Dimmed glow
}

# =============================================================================
# GLOW EFFECT COLORS (Multi-layer particle glow)
# =============================================================================

GLOW_COLORS = {
    # Matter (+1) glow layers: white core -> pink mid -> red outer
    "matter_core": "#ffffff",     # White hot center
    "matter_mid": "#ff6688",      # Pink mid-range
    "matter_outer": "#ff4466",    # Red outer glow
    "matter_far": "#aa2244",      # Distant red haze

    # Antimatter (-1) glow layers: white core -> light blue mid -> cyan outer
    "antimatter_core": "#ffffff", # White hot center
    "antimatter_mid": "#66ccff",  # Light blue mid-range
    "antimatter_outer": "#44aaff", # Cyan outer glow
    "antimatter_far": "#2266aa",  # Distant blue haze

    # Neutral/void glow
    "void_core": "#666688",       # Dim gray center
    "void_mid": "#444466",        # Darker mid
    "void_outer": "#2a2a3a",      # Subtle outer
}

# =============================================================================
# LEMNISCATE HARMONIC MODE COLORS
# =============================================================================

MODE_COLORS = {
    1: "#ff5555",     # Bright red - fundamental (ω=1)
    2: "#5599ff",     # Bright blue - first harmonic (ω=2)
    4: "#55ff55",     # Bright green - second harmonic (ω=4)
    8: "#cc66ff",     # Bright purple - third harmonic (ω=8)
    16: "#ffaa33",    # Bright orange - fourth harmonic (ω=16)
}

# Matching dim versions for trails/echoes
MODE_COLORS_DIM = {
    1: "#993333",
    2: "#336699",
    4: "#339933",
    8: "#773399",
    16: "#996622",
}

# =============================================================================
# CAUSAL LOOP PHASE COLORS
# =============================================================================

PHASE_COLORS = {
    "temporal": "#ffbb55",        # Warm orange (TIME GATE, INCREMENT)
    "existence": "#55ccff",       # Sky cyan (DECAY, EXISTENCE)
    "propagation": "#55ff99",     # Mint green (PROPAGATE, SUPERPOSE, FIELDS)
    "forces": "#ff88cc",          # Pink (FORCES, INTEGRATE)
    "motion": "#ffee55",          # Yellow (MOVE, COLLIDE, TRANSMUTE, BIND)
}

PHASE_COLORS_DIM = {
    "temporal": "#996622",
    "existence": "#226688",
    "propagation": "#228844",
    "forces": "#883355",
    "motion": "#888822",
}

# =============================================================================
# FORCE COLORS
# =============================================================================

FORCE_COLORS = {
    "gravity": "#cc8844",         # Warm brown-orange
    "electromagnetic": "#ffdd44", # Golden yellow
    "strong": "#ff6644",          # Orange-red
    "weak": "#aa77dd",            # Purple
}

# =============================================================================
# SCALE COLORS (For Planck -> Cosmic progression)
# =============================================================================

SCALE_COLORS = {
    "planck": "#ff55ff",          # Magenta - quantum scale
    "subatomic": "#ff5555",       # Red - particle scale
    "atomic": "#ffaa55",          # Orange - atomic scale
    "molecular": "#ffff55",       # Yellow - molecular scale
    "macroscopic": "#55ff55",     # Green - everyday scale
    "planetary": "#55ffff",       # Cyan - planetary scale
    "stellar": "#5555ff",         # Blue - stellar scale
    "galactic": "#aa55ff",        # Purple - galactic scale
    "cosmic": "#ff55aa",          # Pink - cosmic scale
}

# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def hex_to_rgb(hex_color: str) -> tuple[float, float, float]:
    """Convert hex color to RGB tuple (0-1 range)."""
    hex_color = hex_color.lstrip("#")
    return tuple(int(hex_color[i:i+2], 16) / 255 for i in (0, 2, 4))


def rgb_to_hex(r: float, g: float, b: float) -> str:
    """Convert RGB tuple (0-1 range) to hex color."""
    return f"#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}"


def lerp_color(color1: str, color2: str, t: float) -> str:
    """Linearly interpolate between two hex colors."""
    r1, g1, b1 = hex_to_rgb(color1)
    r2, g2, b2 = hex_to_rgb(color2)
    r = r1 + (r2 - r1) * t
    g = g1 + (g2 - g1) * t
    b = b1 + (b2 - b1) * t
    return rgb_to_hex(r, g, b)


def get_state_color(state: int, bright: bool = False) -> str:
    """Get color for a voxel state (+1, 0, -1)."""
    if state > 0:
        return TRD_COLORS["matter_bright"] if bright else TRD_COLORS["matter"]
    elif state < 0:
        return TRD_COLORS["antimatter_bright"] if bright else TRD_COLORS["antimatter"]
    else:
        return TRD_COLORS["void_light"] if bright else TRD_COLORS["void"]


def get_glow_gradient(state: int) -> list[str]:
    """Get glow gradient colors for a state (core to outer)."""
    if state > 0:
        return [
            GLOW_COLORS["matter_core"],
            GLOW_COLORS["matter_mid"],
            GLOW_COLORS["matter_outer"],
            GLOW_COLORS["matter_far"],
        ]
    elif state < 0:
        return [
            GLOW_COLORS["antimatter_core"],
            GLOW_COLORS["antimatter_mid"],
            GLOW_COLORS["antimatter_outer"],
            GLOW_COLORS["antimatter_far"],
        ]
    else:
        return [
            GLOW_COLORS["void_core"],
            GLOW_COLORS["void_mid"],
            GLOW_COLORS["void_outer"],
            TRD_COLORS["background"],
        ]
