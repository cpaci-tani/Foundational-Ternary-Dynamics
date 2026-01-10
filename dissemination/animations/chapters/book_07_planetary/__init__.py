"""
Book 7: Planetary Realm
=======================

Chapter animations for planetary science:
- 7.1: Planetary Formation
- 7.2: Planetary Structure
- 7.3: Planetary Atmospheres
- 7.4: Planetary Geology
"""

from .ch_7_1_formation import (
    FormationIntro,
    NebularHypothesis,
    Accretion,
    DifferentPlanets,
    FormationSummary,
)

from .ch_7_2_structure import (
    StructureIntro,
    Differentiation,
    EarthStructure,
    PlanetaryComparison,
    StructureSummary,
)

from .ch_7_3_atmospheres import (
    AtmospheresIntro,
    AtmosphericLayers,
    Composition,
    GreenhouseEffect,
    EscapeVelocity,
    AtmospheresSummary,
)

from .ch_7_4_geology import (
    GeologyIntro,
    PlateTectonics,
    Volcanism,
    ImpactCraters,
    ComparativeGeology,
    GeologySummary,
)

__all__ = [
    # Chapter 7.1: Planetary Formation
    "FormationIntro",
    "NebularHypothesis",
    "Accretion",
    "DifferentPlanets",
    "FormationSummary",
    # Chapter 7.2: Planetary Structure
    "StructureIntro",
    "Differentiation",
    "EarthStructure",
    "PlanetaryComparison",
    "StructureSummary",
    # Chapter 7.3: Planetary Atmospheres
    "AtmospheresIntro",
    "AtmosphericLayers",
    "Composition",
    "GreenhouseEffect",
    "EscapeVelocity",
    "AtmospheresSummary",
    # Chapter 7.4: Planetary Geology
    "GeologyIntro",
    "PlateTectonics",
    "Volcanism",
    "ImpactCraters",
    "ComparativeGeology",
    "GeologySummary",
]
