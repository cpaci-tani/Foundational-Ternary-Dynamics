"""
Book 2: Subatomic Realm
=======================

Chapter animations for the subatomic realm:
- 2.1: The Planck Scale
- 2.2: Voxel Anatomy
- 2.3: The Particle Zoo
- 2.4: Quantum Phenomena
"""

from .ch_2_1_planck_scale import (
    PlanckIntro,
    ScaleZoomDown,
    LatticeReveal,
    ThreeDimensionalLattice,
    VoxelAtPlanck,
    PlanckSummary,
)

from .ch_2_2_voxel_anatomy import (
    VoxelAnatomyIntro,
    IdentityFields,
    StateFields,
    FluxFields,
    MechanicalFields,
    FlagFields,
    VoxelAnatomySummary,
)

from .ch_2_3_particle_zoo import (
    ParticleZooIntro,
    QuarksScene,
    LeptonsScene,
    BosonsScene,
    CompositeParticles,
    ParticleTable,
    ParticleZooSummary,
)

from .ch_2_4_quantum_phenomena import (
    QuantumIntro,
    HilbertSpaceConstruction,
    BornRuleEmergence,
    SuperpositionMeaning,
    EntanglementScene,
    SLoopIntroduction,
    BellViolation,
    QuantumSummary,
)

__all__ = [
    # Chapter 2.1: The Planck Scale
    "PlanckIntro",
    "ScaleZoomDown",
    "LatticeReveal",
    "ThreeDimensionalLattice",
    "VoxelAtPlanck",
    "PlanckSummary",
    # Chapter 2.2: Voxel Anatomy
    "VoxelAnatomyIntro",
    "IdentityFields",
    "StateFields",
    "FluxFields",
    "MechanicalFields",
    "FlagFields",
    "VoxelAnatomySummary",
    # Chapter 2.3: The Particle Zoo
    "ParticleZooIntro",
    "QuarksScene",
    "LeptonsScene",
    "BosonsScene",
    "CompositeParticles",
    "ParticleTable",
    "ParticleZooSummary",
    # Chapter 2.4: Quantum Phenomena
    "QuantumIntro",
    "HilbertSpaceConstruction",
    "BornRuleEmergence",
    "SuperpositionMeaning",
    "EntanglementScene",
    "SLoopIntroduction",
    "BellViolation",
    "QuantumSummary",
]
