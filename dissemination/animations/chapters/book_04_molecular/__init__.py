"""
Book 4: Molecular Realm
=======================

Chapter animations for the molecular realm:
- 4.1: Chemical Bonds
- 4.2: Simple Molecules
- 4.3: Complex Molecules
- 4.4: Macromolecules
"""

from .ch_4_1_chemical_bonds import (
    ChemicalBondsIntro,
    IonicBonds,
    CovalentBonds,
    BondTypes,
    BondEnergy,
    TRDBondPicture,
    ChemicalBondsSummary,
)

from .ch_4_2_simple_molecules import (
    SimpleMoleculesIntro,
    WaterMolecule,
    CarbonDioxide,
    MethaneStructure,
    MolecularGeometry,
    SimpleMoleculesSummary,
)

from .ch_4_3_complex_molecules import (
    ComplexMoleculesIntro,
    CarbonBackbone,
    FunctionalGroups,
    Isomers,
    RingStructures,
    ComplexMoleculesSummary,
)

from .ch_4_4_macromolecules import (
    MacromoleculesIntro,
    ProteinStructure,
    DNAStructure,
    Polymers,
    BiologicalFunction,
    MacromoleculesSummary,
)

__all__ = [
    # Chapter 4.1: Chemical Bonds
    "ChemicalBondsIntro",
    "IonicBonds",
    "CovalentBonds",
    "BondTypes",
    "BondEnergy",
    "TRDBondPicture",
    "ChemicalBondsSummary",
    # Chapter 4.2: Simple Molecules
    "SimpleMoleculesIntro",
    "WaterMolecule",
    "CarbonDioxide",
    "MethaneStructure",
    "MolecularGeometry",
    "SimpleMoleculesSummary",
    # Chapter 4.3: Complex Molecules
    "ComplexMoleculesIntro",
    "CarbonBackbone",
    "FunctionalGroups",
    "Isomers",
    "RingStructures",
    "ComplexMoleculesSummary",
    # Chapter 4.4: Macromolecules
    "MacromoleculesIntro",
    "ProteinStructure",
    "DNAStructure",
    "Polymers",
    "BiologicalFunction",
    "MacromoleculesSummary",
]
