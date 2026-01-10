"""
Book 5: States of Matter
========================

Chapter animations for states of matter:
- 5.1: Phases of Matter
- 5.2: Phase Transitions
- 5.3: Thermodynamics
"""

from .ch_5_1_phases import (
    PhasesIntro,
    SolidState,
    LiquidState,
    GasState,
    PlasmaState,
    PhaseComparison,
    PhasesSummary,
)

from .ch_5_2_phase_transitions import (
    PhaseTransitionsIntro,
    MeltingProcess,
    BoilingProcess,
    HeatingCurve,
    PhaseDiagram,
    PhaseTransitionsSummary,
)

from .ch_5_3_thermodynamics import (
    ThermodynamicsIntro,
    FirstLaw,
    SecondLaw,
    ThirdLaw,
    HeatEngines,
    BoltzmannEntropy,
    ThermodynamicsSummary,
)

__all__ = [
    # Chapter 5.1: Phases of Matter
    "PhasesIntro",
    "SolidState",
    "LiquidState",
    "GasState",
    "PlasmaState",
    "PhaseComparison",
    "PhasesSummary",
    # Chapter 5.2: Phase Transitions
    "PhaseTransitionsIntro",
    "MeltingProcess",
    "BoilingProcess",
    "HeatingCurve",
    "PhaseDiagram",
    "PhaseTransitionsSummary",
    # Chapter 5.3: Thermodynamics
    "ThermodynamicsIntro",
    "FirstLaw",
    "SecondLaw",
    "ThirdLaw",
    "HeatEngines",
    "BoltzmannEntropy",
    "ThermodynamicsSummary",
]
