"""
TRD Animation Components
========================

Reusable Manim mobjects for TRD visualizations.
"""

# Core components (Phase 1)
from .voxel import VoxelMobject, VoxelGrid
from .flux_field import FluxFieldMobject, FluxArrow

# Lattice components (Phase 2)
from .lattice import (
    LatticeEdge,
    LatticeNode,
    Lattice2D,
    Lattice3D,
    MooreNeighborhood,
)

# Wave components (Phase 2)
from .wave import (
    WaveFront,
    WavePulse,
    StandingWave,
    InterferencePattern,
    FluxWave,
)

# Causal loop components (Phase 2)
from .causal_loop import (
    CAUSAL_LOOP_STEPS,
    CausalLoopNode,
    CausalLoopDiagram,
    CausalLoopLegend,
)

# Lemniscate components (Phase 2)
from .lemniscate import (
    G_STAR,
    VARPI,
    LemniscateCurve,
    LemniscateWithGlow,
    LemniscateHarmonic,
    LemniscateDecomposition,
    ArcLengthTracer,
    GStarReveal,
    LemniscateAlphaConnection,
    RotatingLemniscate,
)

# Master quadratic components (Phase 2)
from .quadratic import (
    X_PLUS,
    X_MINUS,
    master_quadratic,
    QuadraticCurve,
    MasterQuadraticDiagram,
    QuadraticRootExplorer,
    QuadraticDerivation,
    AlphaHighlight,
    NcHighlight,
)

# Scale zoom components (Phase 2)
from .scale_zoom import (
    SCALE_LEVELS,
    ScaleMarker,
    ScaleRuler,
    ZoomBox,
    ScaleTransition,
    ScaleJourney,
    ZoomPulse,
)

__all__ = [
    # Phase 1
    "VoxelMobject",
    "VoxelGrid",
    "FluxFieldMobject",
    "FluxArrow",
    # Lattice
    "LatticeEdge",
    "LatticeNode",
    "Lattice2D",
    "Lattice3D",
    "MooreNeighborhood",
    # Wave
    "WaveFront",
    "WavePulse",
    "StandingWave",
    "InterferencePattern",
    "FluxWave",
    # Causal Loop
    "CAUSAL_LOOP_STEPS",
    "CausalLoopNode",
    "CausalLoopDiagram",
    "CausalLoopLegend",
    # Lemniscate
    "G_STAR",
    "VARPI",
    "LemniscateCurve",
    "LemniscateWithGlow",
    "LemniscateHarmonic",
    "LemniscateDecomposition",
    "ArcLengthTracer",
    "GStarReveal",
    "LemniscateAlphaConnection",
    "RotatingLemniscate",
    # Quadratic
    "X_PLUS",
    "X_MINUS",
    "master_quadratic",
    "QuadraticCurve",
    "MasterQuadraticDiagram",
    "QuadraticRootExplorer",
    "QuadraticDerivation",
    "AlphaHighlight",
    "NcHighlight",
    # Scale Zoom
    "SCALE_LEVELS",
    "ScaleMarker",
    "ScaleRuler",
    "ZoomBox",
    "ScaleTransition",
    "ScaleJourney",
    "ZoomPulse",
]
