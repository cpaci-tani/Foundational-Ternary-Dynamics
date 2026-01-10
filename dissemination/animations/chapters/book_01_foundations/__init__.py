"""
Book 1: Foundations
===================

Animation scenes for the foundational TRD concepts.

Chapters:
    1.1 - The Void (already exists: ch_1_1_void.py)
    1.2 - The First Division (Genesis)
    1.3 - The Two Layers
    1.4 - Interference
    1.5 - The Existence Cycle
    1.6 - The Causal Loop
    1.7 - Time and Causality
    1.8 - The Four Forces
    1.9 - The Constants (G* → α, N_c)
"""

# Import all scenes for easy access
from .ch_1_1_void import VoidIntro, VoidGenesisPreview
from .ch_1_2_first_division import (
    GenesisIntro,
    FluxAccumulation,
    PairProduction,
    GenesisEquations,
    FirstDivisionSummary,
)
from .ch_1_3_two_layers import (
    TwoLayersIntro,
    FluxLayerDetail,
    ManifestationLayerDetail,
    LayerInteraction,
    TwoLayersSummary,
)
from .ch_1_4_interference import (
    InterferenceIntro,
    SingleSourceWave,
    TwoSourceInterference,
    InterferenceMath,
    LatticeInterference,
    InterferenceSummary,
)
from .ch_1_6_causal_loop import (
    CausalLoopIntro,
    LoopOverview,
    StepByStep,
    PhaseGroups,
    CausalLoopEquations,
    CausalLoopSummary,
)
from .ch_1_8_four_forces import (
    ForcesIntro,
    GravityForce,
    EMForce,
    StrongForce,
    WeakForce,
    ForceComparison,
)
from .ch_1_9_constants import (
    ConstantsIntro,
    LemniscateScene,
    GStarDerivation,
    MasterQuadraticScene,
    TwoRoots,
    ConstantsDerivationChain,
    ConstantsSummary,
)

__all__ = [
    # 1.1
    "VoidIntro",
    "VoidGenesisPreview",
    # 1.2
    "GenesisIntro",
    "FluxAccumulation",
    "PairProduction",
    "GenesisEquations",
    "FirstDivisionSummary",
    # 1.3
    "TwoLayersIntro",
    "FluxLayerDetail",
    "ManifestationLayerDetail",
    "LayerInteraction",
    "TwoLayersSummary",
    # 1.4
    "InterferenceIntro",
    "SingleSourceWave",
    "TwoSourceInterference",
    "InterferenceMath",
    "LatticeInterference",
    "InterferenceSummary",
    # 1.6
    "CausalLoopIntro",
    "LoopOverview",
    "StepByStep",
    "PhaseGroups",
    "CausalLoopEquations",
    "CausalLoopSummary",
    # 1.8
    "ForcesIntro",
    "GravityForce",
    "EMForce",
    "StrongForce",
    "WeakForce",
    "ForceComparison",
    # 1.9
    "ConstantsIntro",
    "LemniscateScene",
    "GStarDerivation",
    "MasterQuadraticScene",
    "TwoRoots",
    "ConstantsDerivationChain",
    "ConstantsSummary",
]
