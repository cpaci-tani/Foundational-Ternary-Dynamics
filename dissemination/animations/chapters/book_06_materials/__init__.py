"""
Book 6: Structures and Materials
================================

Chapter animations for structures and materials:
- 6.1: Crystal Structures
- 6.2: Metals and Alloys
- 6.3: Polymers
- 6.4: Composite Materials
"""

from .ch_6_1_crystals import (
    CrystalsIntro,
    UnitCells,
    CrystalSystems,
    Defects,
    Bonding,
    CrystalsSummary,
)

from .ch_6_2_metals import (
    MetalsIntro,
    ElectronSea,
    MetallicProperties,
    Conductivity,
    Alloys,
    MetalsSummary,
)

from .ch_6_3_polymers import (
    PolymersIntro,
    Polymerization,
    PolymerTypes,
    PolymerProperties,
    CommonPolymers,
    PolymersSummary,
)

from .ch_6_4_composites import (
    CompositesIntro,
    CompositeBasics,
    FiberComposites,
    ParticulateComposites,
    LaminateComposites,
    NaturalComposites,
    CompositesSummary,
)

__all__ = [
    # Chapter 6.1: Crystal Structures
    "CrystalsIntro",
    "UnitCells",
    "CrystalSystems",
    "Defects",
    "Bonding",
    "CrystalsSummary",
    # Chapter 6.2: Metals and Alloys
    "MetalsIntro",
    "ElectronSea",
    "MetallicProperties",
    "Conductivity",
    "Alloys",
    "MetalsSummary",
    # Chapter 6.3: Polymers
    "PolymersIntro",
    "Polymerization",
    "PolymerTypes",
    "PolymerProperties",
    "CommonPolymers",
    "PolymersSummary",
    # Chapter 6.4: Composite Materials
    "CompositesIntro",
    "CompositeBasics",
    "FiberComposites",
    "ParticulateComposites",
    "LaminateComposites",
    "NaturalComposites",
    "CompositesSummary",
]
