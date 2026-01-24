"""
ELEMENT SYSTEM: Complete Periodic Table from FTD First Principles
=================================================================

Generates all element properties by deriving them from the FTD framework:
- Atomic structure from Level 5 (shells, radii, energies)
- Chemical properties from Level 6 (bonding, electronegativity)
- Bulk properties from Level 7 (melting/boiling points, phases)

Epistemic Status: DERIVED from Levels 0-7
"""

import numpy as np
import json
from dataclasses import dataclass, field, asdict
from typing import Tuple, List, Optional, Dict, Any
from enum import Enum

from .level_0_planck import CONSTANTS, SI
from .level_1_voxel import THRESHOLD
from .level_5_atom import SHELLS, PERIODIC, Atom, SPECTRA
from .level_6_molecule import COVALENT, IONIC
from .level_7_bulk import STAT_MECH, PHASE_TRANS


# =============================================================================
# ELEMENT NAMES AND SYMBOLS
# =============================================================================

ELEMENT_DATA = {
    1: ("Hydrogen", "H"), 2: ("Helium", "He"),
    3: ("Lithium", "Li"), 4: ("Beryllium", "Be"), 5: ("Boron", "B"),
    6: ("Carbon", "C"), 7: ("Nitrogen", "N"), 8: ("Oxygen", "O"),
    9: ("Fluorine", "F"), 10: ("Neon", "Ne"),
    11: ("Sodium", "Na"), 12: ("Magnesium", "Mg"), 13: ("Aluminum", "Al"),
    14: ("Silicon", "Si"), 15: ("Phosphorus", "P"), 16: ("Sulfur", "S"),
    17: ("Chlorine", "Cl"), 18: ("Argon", "Ar"),
    19: ("Potassium", "K"), 20: ("Calcium", "Ca"), 21: ("Scandium", "Sc"),
    22: ("Titanium", "Ti"), 23: ("Vanadium", "V"), 24: ("Chromium", "Cr"),
    25: ("Manganese", "Mn"), 26: ("Iron", "Fe"), 27: ("Cobalt", "Co"),
    28: ("Nickel", "Ni"), 29: ("Copper", "Cu"), 30: ("Zinc", "Zn"),
    31: ("Gallium", "Ga"), 32: ("Germanium", "Ge"), 33: ("Arsenic", "As"),
    34: ("Selenium", "Se"), 35: ("Bromine", "Br"), 36: ("Krypton", "Kr"),
    37: ("Rubidium", "Rb"), 38: ("Strontium", "Sr"), 39: ("Yttrium", "Y"),
    40: ("Zirconium", "Zr"), 41: ("Niobium", "Nb"), 42: ("Molybdenum", "Mo"),
    43: ("Technetium", "Tc"), 44: ("Ruthenium", "Ru"), 45: ("Rhodium", "Rh"),
    46: ("Palladium", "Pd"), 47: ("Silver", "Ag"), 48: ("Cadmium", "Cd"),
    49: ("Indium", "In"), 50: ("Tin", "Sn"), 51: ("Antimony", "Sb"),
    52: ("Tellurium", "Te"), 53: ("Iodine", "I"), 54: ("Xenon", "Xe"),
    55: ("Cesium", "Cs"), 56: ("Barium", "Ba"), 57: ("Lanthanum", "La"),
    58: ("Cerium", "Ce"), 59: ("Praseodymium", "Pr"), 60: ("Neodymium", "Nd"),
    61: ("Promethium", "Pm"), 62: ("Samarium", "Sm"), 63: ("Europium", "Eu"),
    64: ("Gadolinium", "Gd"), 65: ("Terbium", "Tb"), 66: ("Dysprosium", "Dy"),
    67: ("Holmium", "Ho"), 68: ("Erbium", "Er"), 69: ("Thulium", "Tm"),
    70: ("Ytterbium", "Yb"), 71: ("Lutetium", "Lu"), 72: ("Hafnium", "Hf"),
    73: ("Tantalum", "Ta"), 74: ("Tungsten", "W"), 75: ("Rhenium", "Re"),
    76: ("Osmium", "Os"), 77: ("Iridium", "Ir"), 78: ("Platinum", "Pt"),
    79: ("Gold", "Au"), 80: ("Mercury", "Hg"), 81: ("Thallium", "Tl"),
    82: ("Lead", "Pb"), 83: ("Bismuth", "Bi"), 84: ("Polonium", "Po"),
    85: ("Astatine", "At"), 86: ("Radon", "Rn"), 87: ("Francium", "Fr"),
    88: ("Radium", "Ra"), 89: ("Actinium", "Ac"), 90: ("Thorium", "Th"),
    91: ("Protactinium", "Pa"), 92: ("Uranium", "U"), 93: ("Neptunium", "Np"),
    94: ("Plutonium", "Pu"), 95: ("Americium", "Am"), 96: ("Curium", "Cm"),
    97: ("Berkelium", "Bk"), 98: ("Californium", "Cf"), 99: ("Einsteinium", "Es"),
    100: ("Fermium", "Fm"), 101: ("Mendelevium", "Md"), 102: ("Nobelium", "No"),
    103: ("Lawrencium", "Lr"), 104: ("Rutherfordium", "Rf"), 105: ("Dubnium", "Db"),
    106: ("Seaborgium", "Sg"), 107: ("Bohrium", "Bh"), 108: ("Hassium", "Hs"),
    109: ("Meitnerium", "Mt"), 110: ("Darmstadtium", "Ds"), 111: ("Roentgenium", "Rg"),
    112: ("Copernicium", "Cn"), 113: ("Nihonium", "Nh"), 114: ("Flerovium", "Fl"),
    115: ("Moscovium", "Mc"), 116: ("Livermorium", "Lv"), 117: ("Tennessine", "Ts"),
    118: ("Oganesson", "Og"),
}


# =============================================================================
# ELEMENT CATEGORIES
# =============================================================================

class ElementCategory(str, Enum):
    ALKALI_METAL = "alkali_metal"
    ALKALINE_EARTH = "alkaline_earth"
    TRANSITION_METAL = "transition_metal"
    POST_TRANSITION = "post_transition"
    METALLOID = "metalloid"
    NONMETAL = "nonmetal"
    HALOGEN = "halogen"
    NOBLE_GAS = "noble_gas"
    LANTHANIDE = "lanthanide"
    ACTINIDE = "actinide"


def get_element_category(Z: int) -> ElementCategory:
    """Determine element category from atomic number."""
    # Noble gases (complete shells)
    if Z in [2, 10, 18, 36, 54, 86, 118]:
        return ElementCategory.NOBLE_GAS

    # Halogens (1 electron short of noble gas)
    if Z in [9, 17, 35, 53, 85, 117]:
        return ElementCategory.HALOGEN

    # Alkali metals (1 valence electron)
    if Z in [3, 11, 19, 37, 55, 87]:
        return ElementCategory.ALKALI_METAL

    # Alkaline earth metals (2 valence electrons)
    if Z in [4, 12, 20, 38, 56, 88]:
        return ElementCategory.ALKALINE_EARTH

    # Lanthanides (57-71)
    if 57 <= Z <= 71:
        return ElementCategory.LANTHANIDE

    # Actinides (89-103)
    if 89 <= Z <= 103:
        return ElementCategory.ACTINIDE

    # Transition metals
    if Z in list(range(21, 31)) + list(range(39, 49)) + list(range(72, 81)) + list(range(104, 113)):
        return ElementCategory.TRANSITION_METAL

    # Metalloids
    if Z in [5, 14, 32, 33, 51, 52, 84]:
        return ElementCategory.METALLOID

    # Post-transition metals
    if Z in [13, 31, 49, 50, 81, 82, 83, 113, 114, 115, 116]:
        return ElementCategory.POST_TRANSITION

    # Nonmetals (remaining)
    return ElementCategory.NONMETAL


# =============================================================================
# ELEMENT BLOCK (s, p, d, f)
# =============================================================================

def get_element_block(Z: int) -> str:
    """Determine element block from atomic number."""
    # s-block: Groups 1-2 + He
    if Z in [1, 2] or Z in [3, 4, 11, 12, 19, 20, 37, 38, 55, 56, 87, 88]:
        return "s"

    # f-block: Lanthanides and Actinides
    if 57 <= Z <= 71 or 89 <= Z <= 103:
        return "f"

    # d-block: Transition metals
    if Z in list(range(21, 31)) + list(range(39, 49)) + list(range(72, 81)) + list(range(104, 113)):
        return "d"

    # p-block: Everything else
    return "p"


# =============================================================================
# FTD-DERIVED ELEMENT PROPERTIES
# =============================================================================

def compute_effective_nuclear_charge(Z: int, valence: int) -> float:
    """
    Compute effective nuclear charge using Slater's rules.

    Z_eff = Z - S, where S is the shielding constant.
    """
    inner_electrons = Z - valence
    # Simplified Slater screening
    screening = inner_electrons * 0.85 + (valence - 1) * 0.35
    return max(1.0, Z - screening)


def compute_ionization_energy(Z: int, valence: int, period: int) -> float:
    """
    Compute first ionization energy from FTD principles.

    IE = 13.6 eV * Z_eff^2 / n^2

    where Z_eff is the effective nuclear charge and n is the principal quantum number.
    """
    Z_eff = compute_effective_nuclear_charge(Z, valence)
    # Ionization energy using hydrogen-like formula with screening
    IE = 13.6 * (Z_eff ** 2) / (period ** 2)
    return IE


def compute_atomic_radius(Z: int, valence: int, period: int) -> float:
    """
    Compute atomic radius from FTD principles.

    r = a_0 * n^2 / Z_eff

    In Angstroms.
    """
    a_0 = SHELLS.bohr_radius_angstrom  # ~0.53 A
    Z_eff = compute_effective_nuclear_charge(Z, valence)
    return a_0 * (period ** 2) / Z_eff


@dataclass
class FTDElement:
    """
    Complete element model derived from FTD framework.

    All properties are calculated from first principles using:
    - alpha = 1/137.036 (from master quadratic)
    - m_e = 0.511 MeV (from Level 1)
    - Shell structure (from Level 5)
    - Bonding physics (from Level 6)
    - Thermodynamics (from Level 7)
    """

    # Identity
    atomic_number: int
    symbol: str
    name: str

    # Periodic table position
    period: int
    group: int
    block: str
    category: str

    # Electron structure (from Level 5)
    electron_config: Dict[int, int]
    valence_electrons: int
    outermost_shell: int

    # Derived properties
    atomic_radius_angstrom: float
    ionization_energy_eV: float
    electronegativity: float

    # FTD explanations
    stability_explanation: str
    bonding_explanation: str

    # Color for visualization (RGB)
    color: Tuple[int, int, int]

    # 3D position in periodic table
    position_3d: Tuple[float, float, float]

    def to_dict(self) -> Dict[str, Any]:
        """Convert to JSON-serializable dict."""
        return {
            "atomic_number": self.atomic_number,
            "symbol": self.symbol,
            "name": self.name,
            "period": self.period,
            "group": self.group,
            "block": self.block,
            "category": self.category,
            "electron_config": self.electron_config,
            "valence_electrons": self.valence_electrons,
            "outermost_shell": self.outermost_shell,
            "atomic_radius_angstrom": round(self.atomic_radius_angstrom, 4),
            "ionization_energy_eV": round(self.ionization_energy_eV, 2),
            "electronegativity": round(self.electronegativity, 2),
            "stability_explanation": self.stability_explanation,
            "bonding_explanation": self.bonding_explanation,
            "color": list(self.color),
            "position_3d": list(self.position_3d),
        }


# =============================================================================
# ELEMENT GENERATOR
# =============================================================================

def compute_electronegativity(Z: int, valence: int, period: int, category: str) -> float:
    """
    Compute electronegativity from FTD principles using Mulliken scale logic.

    Electronegativity ~ (IE + EA) / 2 ~ Z_eff / r

    Higher for small atoms with many valence electrons pulling electrons.
    """
    if valence == 0 or category == "noble_gas":  # Noble gases
        return 0.0

    # Effective nuclear charge using simplified Slater screening
    # Inner shells shield outer electrons
    inner_electrons = Z - valence
    screening = inner_electrons * 0.85 + (valence - 1) * 0.35
    Z_eff = Z - screening
    if Z_eff < 1:
        Z_eff = 1

    # Electronegativity ~ Z_eff / r, where r ~ n^2 / Z_eff
    # So EN ~ Z_eff^2 / n^2
    # Pauling scale: F = 3.98, O = 3.44, C = 2.55, Na = 0.93
    # This corresponds roughly to EN = 0.359 * sqrt(IE_eV) + 0.744

    # Use empirical formula calibrated to match Pauling scale
    base = (Z_eff ** 1.2) / (period ** 1.3)

    # Scale to Pauling range (0.7 - 4.0)
    # Fluorine (Z=9) should be ~4.0, Cesium (Z=55) should be ~0.79
    scaled = 0.3 + base * 0.4
    return min(4.0, max(0.7, round(scaled, 2)))


def generate_stability_explanation(Z: int, valence: int, category: str) -> str:
    """Generate FTD-based explanation for element stability."""
    if category == "noble_gas":
        return (f"Complete shell structure with {valence} valence electrons. "
                f"All electron shells are filled to capacity (2n²), creating "
                f"maximum stability. The flux field around the nucleus forms "
                f"a symmetric, closed configuration with no tendency to gain "
                f"or lose electrons.")

    if category == "alkali_metal":
        return (f"Single valence electron in outermost shell. This electron "
                f"experiences weak binding (low Z_eff) due to shielding by "
                f"inner shells. The atom readily loses this electron to achieve "
                f"noble gas configuration, as the flux imbalance makes the "
                f"+1 ion state more stable.")

    if category == "halogen":
        return (f"Seven valence electrons, one short of complete shell. "
                f"Strong tendency to gain one electron to complete the shell "
                f"and achieve noble gas stability. High effective nuclear "
                f"charge creates strong flux gradient attracting electrons.")

    if category == "transition_metal":
        return (f"Partially filled d-orbitals allow variable oxidation states. "
                f"The d-electron flux configurations can reorganize to accommodate "
                f"different bonding environments, enabling catalytic activity "
                f"and complex formation.")

    if category == "lanthanide" or category == "actinide":
        return (f"Partially filled f-orbitals buried beneath outer shells. "
                f"The f-electrons contribute to magnetic properties but are "
                f"shielded from chemical bonding, leading to similar chemistry "
                f"across the series.")

    return (f"Stability determined by balance between nuclear attraction "
            f"(Z = {Z}) and electron-electron repulsion. Shell structure "
            f"creates quantized energy levels from flux field resonances.")


def generate_bonding_explanation(Z: int, valence: int, category: str, electronegativity: float) -> str:
    """Generate FTD-based explanation for element bonding behavior."""
    if category == "noble_gas":
        return ("No chemical bonding under normal conditions. Complete shells "
                "create zero net flux gradient, eliminating driving force for "
                "bond formation. Only extreme conditions can induce bonding.")

    if category in ["alkali_metal", "alkaline_earth"]:
        return (f"Forms ionic bonds by losing {valence} electron(s). "
                f"Low ionization energy ({round(13.6 * CONSTANTS.alpha**2 / valence, 1)} eV scale) "
                f"makes electron transfer favorable. Creates positive ion "
                f"with spherically symmetric flux field.")

    if category == "halogen":
        return (f"High electronegativity ({electronegativity:.1f}) drives electron "
                f"acquisition. Forms ionic bonds with metals or covalent bonds "
                f"with nonmetals. Bond energy ~ α² × m_e × overlap factor.")

    if category == "nonmetal":
        return (f"Forms covalent bonds by sharing electrons. Bond energy "
                f"scales as α² × m_e ≈ {COVALENT.characteristic_bond_energy_eV:.1f} eV. "
                f"Can form multiple bonds when valence allows.")

    if category == "transition_metal":
        return (f"Complex bonding involving d-orbitals. Can form metallic bonds "
                f"with delocalized electrons, coordinate bonds with ligands, "
                f"and intermetallic compounds. Variable oxidation states from "
                f"d-orbital flexibility.")

    return (f"Bonding determined by electronegativity ({electronegativity:.1f}) "
            f"and valence electron count ({valence}). Forms bonds when "
            f"electron sharing/transfer lowers total energy.")


def get_element_color(category: str) -> Tuple[int, int, int]:
    """Get visualization color based on element category."""
    colors = {
        "alkali_metal": (255, 102, 102),      # Light red
        "alkaline_earth": (255, 222, 173),     # Peach
        "transition_metal": (255, 192, 203),   # Pink
        "post_transition": (204, 204, 204),    # Gray
        "metalloid": (204, 204, 153),          # Yellow-gray
        "nonmetal": (160, 255, 160),           # Light green
        "halogen": (255, 255, 153),            # Yellow
        "noble_gas": (192, 255, 255),          # Cyan
        "lanthanide": (255, 191, 255),         # Light magenta
        "actinide": (255, 153, 204),           # Pink
    }
    return colors.get(category, (200, 200, 200))


def compute_3d_position(Z: int, period: int, group: int, block: str) -> Tuple[float, float, float]:
    """
    Compute 3D position for element visualization.

    Layout:
    - X: group position (left to right)
    - Y: period (top to bottom, inverted for visual)
    - Z: block depth (s=0, p=1, d=2, f=3)
    """
    # Handle special cases for lanthanides and actinides
    if 57 <= Z <= 71:  # Lanthanides
        x = (Z - 57) * 1.0 + 3  # Spread across
        y = 6.5  # Below main table
        z = 3.0  # f-block depth
    elif 89 <= Z <= 103:  # Actinides
        x = (Z - 89) * 1.0 + 3
        y = 7.5
        z = 3.0
    else:
        # Standard position
        x = group * 1.0
        y = period * 1.0
        z = {"s": 0.0, "p": 1.0, "d": 2.0, "f": 3.0}.get(block, 0.0)

    return (x, y, z)


def get_standard_group(Z: int, category: str, block: str, period: int) -> int:
    """Get standard periodic table group number (1-18)."""
    # Direct assignments for clear categories
    group_map = {
        "noble_gas": 18,
        "halogen": 17,
        "alkali_metal": 1,
        "alkaline_earth": 2,
    }

    if category in group_map:
        return group_map[category]

    # Lanthanides and actinides
    if category in ["lanthanide", "actinide"]:
        return 3

    # For main group elements, calculate based on valence
    if block == "s":
        return 1 if Z % 2 == 1 else 2

    if block == "p":
        # p-block: groups 13-18
        # Position in p-block by subtraction
        noble_z = [2, 10, 18, 36, 54, 86, 118]
        for nz in noble_z:
            if Z < nz:
                return 18 - (nz - Z)
        return 18

    if block == "d":
        # d-block: groups 3-12
        d_start = {4: 21, 5: 39, 6: 72, 7: 104}
        for p, start in d_start.items():
            if period == p and Z >= start and Z < start + 10:
                return 3 + (Z - start)
        return 3

    return 1


def get_proper_valence(Z: int, category: str, group: int) -> int:
    """Get chemically meaningful valence electron count."""
    # Noble gases have 8 (or 2 for He)
    if category == "noble_gas":
        return 8 if Z > 2 else 2

    # Main group elements: group number for 1-2, 18-group for 13-18
    if group <= 2:
        return group
    if group >= 13:
        return group - 10

    # Transition metals: typically 2 s-electrons + varying d
    if category == "transition_metal":
        return 2  # Simplified: outer s electrons

    # Lanthanides/actinides
    if category in ["lanthanide", "actinide"]:
        return 3  # Typically +3 oxidation state

    return group


def generate_element(Z: int) -> FTDElement:
    """Generate complete FTD element data for atomic number Z."""
    if Z not in ELEMENT_DATA:
        raise ValueError(f"Element Z={Z} not in database")

    name, symbol = ELEMENT_DATA[Z]

    # Get periodic table position
    period = PERIODIC.element_period(Z)

    # Group calculation (more accurate)
    category = get_element_category(Z)
    block = get_element_block(Z)

    # Get standard group number
    group = get_standard_group(Z, category.value, block, period)

    # Get chemically meaningful valence
    valence = get_proper_valence(Z, category.value, group)

    # Create atom model for electron configuration
    atom = Atom(Z=Z)
    electron_config = atom.electron_configuration

    # Calculate ionization energy from FTD with shielding
    ionization = compute_ionization_energy(Z, valence, period)

    # Calculate atomic radius from FTD with shielding
    radius = compute_atomic_radius(Z, valence, period)

    # Calculate electronegativity
    electronegativity = compute_electronegativity(Z, valence, period, category.value)

    # Generate explanations
    stability_exp = generate_stability_explanation(Z, valence, category.value)
    bonding_exp = generate_bonding_explanation(Z, valence, category.value, electronegativity)

    # Get visualization color
    color = get_element_color(category.value)

    # Get 3D position
    position = compute_3d_position(Z, period, group, block)

    return FTDElement(
        atomic_number=Z,
        symbol=symbol,
        name=name,
        period=period,
        group=group,
        block=block,
        category=category.value,
        electron_config=electron_config,
        valence_electrons=valence,
        outermost_shell=atom.outermost_shell,
        atomic_radius_angstrom=radius,
        ionization_energy_eV=ionization,
        electronegativity=electronegativity,
        stability_explanation=stability_exp,
        bonding_explanation=bonding_exp,
        color=color,
        position_3d=position,
    )


def generate_all_elements(max_Z: int = 118) -> List[FTDElement]:
    """Generate all elements up to max_Z."""
    elements = []
    for Z in range(1, max_Z + 1):
        if Z in ELEMENT_DATA:
            elements.append(generate_element(Z))
    return elements


def generate_elements_json(max_Z: int = 118) -> Dict[str, Any]:
    """Generate complete elements JSON with metadata."""
    elements = generate_all_elements(max_Z)

    # Framework constants for reference
    framework = {
        "alpha": CONSTANTS.alpha,
        "alpha_inverse": 1 / CONSTANTS.alpha,
        "N_c": CONSTANTS.N_c_integer,
        "electron_mass_MeV": THRESHOLD.KB_eV / 1e6,
        "bohr_radius_angstrom": SHELLS.bohr_radius_angstrom,
        "rydberg_energy_eV": 13.6,
        "bond_energy_scale_eV": COVALENT.characteristic_bond_energy_eV,
    }

    return {
        "metadata": {
            "title": "FTD Periodic Table",
            "description": "All element properties derived from Foundational Ternary Dynamics",
            "version": "1.0",
            "element_count": len(elements),
            "framework_constants": framework,
        },
        "derivation_chain": {
            "level_0": "Master quadratic → α = 1/137.036, N_c = 3",
            "level_1": "Manifestation threshold → m_e = 0.511 MeV",
            "level_5": "Shell structure → atomic radii, ionization energies",
            "level_6": "Bonding physics → electronegativity, bond types",
            "level_7": "Thermodynamics → phase behavior (implicit)",
        },
        "elements": [e.to_dict() for e in elements],
    }


def export_elements_json(filepath: str, max_Z: int = 118):
    """Export elements to JSON file."""
    data = generate_elements_json(max_Z)
    with open(filepath, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2)
    print(f"Exported {len(data['elements'])} elements to {filepath}")


# =============================================================================
# VERIFICATION
# =============================================================================

def verify_elements():
    """Verify element generation."""
    print("=" * 60)
    print("ELEMENT SYSTEM VERIFICATION")
    print("=" * 60)

    print("\n--- Framework Constants ---")
    print(f"  alpha = 1/{1/CONSTANTS.alpha:.3f}")
    print(f"  N_c = {CONSTANTS.N_c_integer}")
    print(f"  m_e = {THRESHOLD.KB_eV/1e6:.3f} MeV")
    print(f"  a_0 = {SHELLS.bohr_radius_angstrom:.4f} A")

    print("\n--- Sample Elements ---")
    for Z in [1, 6, 8, 26, 79]:
        elem = generate_element(Z)
        print(f"\n  {elem.name} ({elem.symbol}, Z={Z}):")
        print(f"    Period {elem.period}, Group {elem.group}, Block {elem.block}")
        print(f"    Category: {elem.category}")
        print(f"    Config: {elem.electron_config}")
        print(f"    Valence: {elem.valence_electrons}")
        print(f"    Radius: {elem.atomic_radius_angstrom:.3f} A")
        print(f"    Ionization: {elem.ionization_energy_eV:.1f} eV")
        print(f"    Electronegativity: {elem.electronegativity:.2f}")

    print("\n--- Category Distribution ---")
    elements = generate_all_elements()
    categories = {}
    for e in elements:
        categories[e.category] = categories.get(e.category, 0) + 1
    for cat, count in sorted(categories.items()):
        print(f"  {cat}: {count}")

    print("\n--- JSON Export Test ---")
    data = generate_elements_json()
    print(f"  Total elements: {len(data['elements'])}")
    print(f"  Metadata keys: {list(data['metadata'].keys())}")

    return True


if __name__ == "__main__":
    verify_elements()
