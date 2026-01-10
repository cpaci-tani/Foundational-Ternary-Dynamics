"""
TRD Manuscript Figure Style Guide
=================================
Centralized style definitions for consistent figure appearance.
"""

import matplotlib.pyplot as plt
import matplotlib as mpl

# =============================================================================
# COLOR PALETTES
# =============================================================================

# Primary TRD Colors (from FIGURE_TRACKER.md)
COLORS = {
    'void': '#888888',          # Gray - unmanifested substrate
    'matter': '#DD4444',        # Red - positive manifestation (+1)
    'antimatter': '#4488DD',    # Blue - negative manifestation (-1)
    'background': '#FFFFFF',    # White
    'text': '#222222',          # Near black
    'accent1': '#44BB44',       # Green (for positive/success)
    'accent2': '#9944BB',       # Purple (for special states)
    'grid': '#CCCCCC',          # Light gray
    'highlight': '#FFAA00',     # Orange/gold (for emphasis)
}

# Force-specific colors
FORCE_COLORS = {
    'gravity': '#8B4513',       # Saddle brown
    'electromagnetic': '#FFD700', # Gold
    'strong': '#FF4500',        # Orange red
    'weak': '#9370DB',          # Medium purple
}

# Harmonic mode colors (for Lemniscate-Alpha curve)
MODE_COLORS = {
    1: '#E41A1C',   # Red - fundamental
    2: '#377EB8',   # Blue - first harmonic
    4: '#4DAF4A',   # Green - second harmonic
    8: '#984EA3',   # Purple - third harmonic
    16: '#FF7F00',  # Orange - fourth harmonic
}

# Causal loop phase colors
PHASE_COLORS = {
    'temporal': '#FFB347',      # Orange (TIME GATE, INCREMENT)
    'existence': '#87CEEB',     # Sky blue (DECAY, EXISTENCE)
    'propagation': '#98FB98',   # Pale green (PROPAGATE, SUPERPOSE, FIELDS)
    'forces': '#DDA0DD',        # Plum (FORCES, INTEGRATE)
    'motion': '#F0E68C',        # Khaki (MOVE, COLLIDE, TRANSMUTE, BIND)
}

# Voxel category colors
VOXEL_CATEGORY_COLORS = {
    'identity': '#FF6B6B',      # Coral red
    'state': '#4ECDC4',         # Teal
    'energy': '#FFE66D',        # Yellow
    'mechanics': '#95E1D3',     # Mint
    'temporal': '#A8E6CF',      # Light green
    'stability': '#DCD6F7',     # Lavender
}

# Neighbor distance colors (for Moore neighborhood)
NEIGHBOR_COLORS = {
    'center': '#FFAA00',        # Gold - the central voxel
    'face': '#DD4444',          # Red (distance 1)
    'edge': '#4488DD',          # Blue (distance sqrt(2))
    'corner': '#44BB44',        # Green (distance sqrt(3))
}

# Standard Model particle type colors
SM_COLORS = {
    'quark': '#FF6B6B',         # Red tones
    'lepton': '#4ECDC4',        # Teal tones
    'gauge_boson': '#FFE66D',   # Yellow tones
    'higgs': '#DDA0DD',         # Purple
}

# =============================================================================
# TYPOGRAPHY
# =============================================================================

FONTS = {
    'title': {'family': 'sans-serif', 'size': 14, 'weight': 'bold'},
    'subtitle': {'family': 'sans-serif', 'size': 12, 'weight': 'normal'},
    'label': {'family': 'sans-serif', 'size': 11},
    'tick': {'family': 'sans-serif', 'size': 9},
    'annotation': {'family': 'sans-serif', 'size': 10},
    'math': {'family': 'serif', 'size': 11},
    'legend': {'family': 'sans-serif', 'size': 9},
}

# =============================================================================
# FIGURE DIMENSIONS
# =============================================================================

# Figure sizes in inches
FIGURE_SIZES = {
    'standard': (8, 6),         # Default single panel
    'wide': (10, 6),            # Wide aspect
    'square': (7, 7),           # Square
    'tall': (6, 8),             # Portrait orientation
    'dual_panel': (12, 5),      # Side-by-side panels
    'multi_panel': (12, 8),     # Multiple panels
    'flowchart': (10, 12),      # Vertical flowchart
    'small': (6, 4),            # Compact
}

# Resolution settings
DPI = {
    'web': 150,                 # Web/screen display
    'print': 300,               # Print quality
    'preview': 72,              # Quick preview
}

# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def apply_trd_style(ax, title=None, xlabel=None, ylabel=None, grid=True):
    """
    Apply TRD styling to a matplotlib axis.

    Parameters
    ----------
    ax : matplotlib.axes.Axes
        The axis to style
    title : str, optional
        Title for the plot
    xlabel : str, optional
        Label for x-axis
    ylabel : str, optional
        Label for y-axis
    grid : bool, default True
        Whether to show grid
    """
    # Set title
    if title:
        ax.set_title(title, fontdict=FONTS['title'], pad=10)

    # Set axis labels
    if xlabel:
        ax.set_xlabel(xlabel, fontdict=FONTS['label'])
    if ylabel:
        ax.set_ylabel(ylabel, fontdict=FONTS['label'])

    # Style tick labels
    ax.tick_params(labelsize=FONTS['tick']['size'])

    # Grid
    if grid:
        ax.grid(True, alpha=0.3, color=COLORS['grid'], linestyle='-', linewidth=0.5)
        ax.set_axisbelow(True)

    # Spine styling
    for spine in ax.spines.values():
        spine.set_color(COLORS['text'])
        spine.set_linewidth(0.5)

    # Background
    ax.set_facecolor(COLORS['background'])


def create_figure(figtype='standard', dpi='web', nrows=1, ncols=1):
    """
    Create a figure with TRD styling.

    Parameters
    ----------
    figtype : str
        One of: 'standard', 'wide', 'square', 'tall', 'dual_panel',
                'multi_panel', 'flowchart', 'small'
    dpi : str
        One of: 'web' (150), 'print' (300), 'preview' (72)
    nrows, ncols : int
        Number of subplot rows/columns

    Returns
    -------
    fig, ax : matplotlib figure and axis/axes
    """
    size = FIGURE_SIZES.get(figtype, FIGURE_SIZES['standard'])
    resolution = DPI.get(dpi, DPI['web'])

    fig, ax = plt.subplots(nrows, ncols, figsize=size, dpi=resolution)
    fig.patch.set_facecolor(COLORS['background'])

    return fig, ax


def save_figure(fig, filepath, dpi='web', tight=True):
    """
    Save a figure with standard settings.

    Parameters
    ----------
    fig : matplotlib.figure.Figure
        The figure to save
    filepath : str or Path
        Output path
    dpi : str
        Resolution setting
    tight : bool
        Use tight bounding box
    """
    resolution = DPI.get(dpi, DPI['web'])
    bbox = 'tight' if tight else None

    fig.savefig(
        filepath,
        dpi=resolution,
        bbox_inches=bbox,
        facecolor=COLORS['background'],
        edgecolor='none',
        pad_inches=0.1
    )
    plt.close(fig)


def setup_matplotlib_defaults():
    """Configure matplotlib with TRD defaults."""
    mpl.rcParams['font.family'] = 'sans-serif'
    mpl.rcParams['font.size'] = 10
    mpl.rcParams['axes.titlesize'] = 14
    mpl.rcParams['axes.labelsize'] = 11
    mpl.rcParams['xtick.labelsize'] = 9
    mpl.rcParams['ytick.labelsize'] = 9
    mpl.rcParams['legend.fontsize'] = 9
    mpl.rcParams['figure.facecolor'] = COLORS['background']
    mpl.rcParams['axes.facecolor'] = COLORS['background']
    mpl.rcParams['savefig.facecolor'] = COLORS['background']
    mpl.rcParams['savefig.edgecolor'] = 'none'
