"""
Book 3: Atomic Realm
====================

Chapter animations for the atomic realm:
- 3.1: Stable Structures
- 3.2: The Periodic Table
- 3.3: Electron Dynamics
- 3.4: Nuclear Physics
"""

from .ch_3_1_stable_structures import (
    StableStructuresIntro,
    TriadGeometry,
    BindingEnergy,
    ColorNeutrality,
    ProtonFormation,
    NeutronFormation,
    StableStructuresSummary,
)

from .ch_3_2_periodic_table import (
    PeriodicTableIntro,
    AtomicNumber,
    ElectronShells,
    ShellFillingOrder,
    MiniPeriodicTable,
    ChemicalProperties,
    PeriodicTableSummary,
)

from .ch_3_3_electron_dynamics import (
    ElectronDynamicsIntro,
    OrbitalConcept,
    ShellRadii,
    ElectronTransitions,
    SpectralLines,
    TRDOrbitalPicture,
    ElectronDynamicsSummary,
)

from .ch_3_4_nuclear_physics import (
    NuclearPhysicsIntro,
    StrongForce,
    NuclearBinding,
    BindingEnergyCurve,
    FusionFission,
    NuclearDecay,
    NuclearPhysicsSummary,
)

__all__ = [
    # Chapter 3.1: Stable Structures
    "StableStructuresIntro",
    "TriadGeometry",
    "BindingEnergy",
    "ColorNeutrality",
    "ProtonFormation",
    "NeutronFormation",
    "StableStructuresSummary",
    # Chapter 3.2: The Periodic Table
    "PeriodicTableIntro",
    "AtomicNumber",
    "ElectronShells",
    "ShellFillingOrder",
    "MiniPeriodicTable",
    "ChemicalProperties",
    "PeriodicTableSummary",
    # Chapter 3.3: Electron Dynamics
    "ElectronDynamicsIntro",
    "OrbitalConcept",
    "ShellRadii",
    "ElectronTransitions",
    "SpectralLines",
    "TRDOrbitalPicture",
    "ElectronDynamicsSummary",
    # Chapter 3.4: Nuclear Physics
    "NuclearPhysicsIntro",
    "StrongForce",
    "NuclearBinding",
    "BindingEnergyCurve",
    "FusionFission",
    "NuclearDecay",
    "NuclearPhysicsSummary",
]
