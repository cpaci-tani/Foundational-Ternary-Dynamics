# FTD Fusion: Usage Guide

## Installation

```bash
# Clone the repository
git clone https://github.com/ftd/ftd-fusion.git
cd ftd-fusion

# Install dependencies
pip install numpy scipy matplotlib
```

---

## Quick Start

### Calculate Binding Energy for Any Nucleus

```python
from derivations.binding_energy import binding_energy, binding_energy_per_nucleon

# Helium-4 (alpha particle)
A, Z = 4, 2
B = binding_energy(A, Z)
B_per_A = binding_energy_per_nucleon(A, Z)

print(f"He-4 binding energy: {B:.2f} MeV")
print(f"He-4 B/A: {B_per_A:.2f} MeV/nucleon")
# Output: He-4 binding energy: 28.30 MeV
# Output: He-4 B/A: 7.07 MeV/nucleon
```

### Calculate Q-Value for a Reaction

```python
from derivations.mass_defect import calculate_q_value, NuclearReaction

# D + T -> He-4 + n
reaction = NuclearReaction(
    name='D-T Fusion',
    reactants=[(2, 1), (3, 1)],   # Deuterium + Tritium
    products=[(4, 2), (1, 0)],    # He-4 + neutron
)

Q = calculate_q_value(reaction)
print(f"D-T Q-value: {Q:.2f} MeV")
# Output: D-T Q-value: 17.59 MeV
```

### Find the Most Stable Nucleus

```python
from derivations.binding_energy import find_maximum_stability

A_max, Z_max, B_per_A_max = find_maximum_stability()
print(f"Most stable: A={A_max}, Z={Z_max}, B/A={B_per_A_max:.2f} MeV")
# Output: Most stable: A=52, Z=24, B/A=8.83 MeV
```

---

## Module Reference

### `derivations/binding_energy.py`

#### Framework Constants

```python
from derivations.binding_energy import N_C, N_BASE, B_3, N_EFF, K_B, ALPHA

print(f"N_c = {N_C}")      # 3 (colors)
print(f"N_base = {N_BASE}") # 4 (dimensions)
print(f"b_3 = {B_3}")      # 7 (QCD beta)
print(f"N_eff = {N_EFF}")  # 13 (effective DoF)
```

#### SEMF Coefficients

```python
from derivations.binding_energy import get_semf_coefficients

coef = get_semf_coefficients(refined=True)
print(coef)
# {'a_V': 15.75, 'a_S': 17.81, 'a_C': 0.72, 'a_A': 28.27, 'a_P': 15.09}
```

#### Key Functions

| Function | Description |
|----------|-------------|
| `binding_energy(A, Z)` | Total binding energy in MeV |
| `binding_energy_per_nucleon(A, Z)` | B/A in MeV |
| `find_optimal_Z(A)` | Most stable Z for given A |
| `generate_binding_curve(A_max)` | Arrays of A, Z, B/A |
| `find_maximum_stability()` | (A, Z, B/A) for iron peak |
| `validate_binding_energies()` | Compare to experimental data |

### `derivations/mass_defect.py`

#### Nuclear Masses

```python
from derivations.mass_defect import nuclear_mass, mass_defect, M_PROTON, M_NEUTRON

# Proton mass: 938.27 MeV
# Neutron mass: 939.57 MeV

# Calculate nuclear mass (with binding)
M_He4 = nuclear_mass(4, 2)  # ~3727.4 MeV

# Calculate mass defect
dm = mass_defect(4, 2)  # 28.30 MeV
```

#### Pre-defined Reactions

```python
from derivations.mass_defect import FUSION_REACTIONS, calculate_q_value

# Available reactions:
# 'D-T', 'D-D-1', 'D-D-2', 'p-p', 'He3-He3', 'triple-alpha', 'CNO-net'

for name, reaction in FUSION_REACTIONS.items():
    Q = calculate_q_value(reaction)
    print(f"{name}: {Q:.2f} MeV")
```

#### Experimental Comparison

```python
from derivations.mass_defect import EXPERIMENTAL_Q_VALUES

for name, Q_exp in EXPERIMENTAL_Q_VALUES.items():
    Q_calc = calculate_q_value(FUSION_REACTIONS[name])
    error = abs(Q_calc - Q_exp) / Q_exp * 100
    print(f"{name}: calc={Q_calc:.2f}, exp={Q_exp:.2f}, error={error:.1f}%")
```

### `derivations/fusion_fission.py`

#### Analyze Binding Curve

```python
from derivations.fusion_fission import analyze_binding_curve

analysis = analyze_binding_curve()

print(f"Iron peak at A = {analysis['A_peak']}")
print(f"Maximum B/A = {analysis['B_per_A_peak']:.2f} MeV")
print(f"Fusion regime: A < {analysis['A_peak']}")
print(f"Fission regime: A > {analysis['A_peak']}")
```

#### Calculate Fusion Energy

```python
from derivations.fusion_fission import energy_released_fusion

# Two deuterons fusing
Q, explanation = energy_released_fusion(2, 1, 2, 1)
print(explanation)
# "Fusion releases 3.27 MeV (product more tightly bound)"
```

#### Calculate Fission Energy

```python
from derivations.fusion_fission import energy_released_fission

# U-235 splitting 60/40
Q, explanation = energy_released_fission(235, 92, split_ratio=0.6)
print(explanation)
# "Fission releases 167.5 MeV (products more stable)"
```

#### Generate Binding Curve Plot

```python
from derivations.fusion_fission import plot_binding_curve

plot_binding_curve('my_binding_curve.png')
```

---

## Running Verifications

### Full Binding Energy Verification

```bash
python -m derivations.binding_energy
```

Output:
```
======================================================================
NUCLEAR BINDING ENERGY FROM FTD FIRST PRINCIPLES
======================================================================

[1] SEMF COEFFICIENTS FROM FTD INTEGERS
    Framework: N_c=3, N_base=4, b_3=7, N_eff=13
--------------------------------------------------
Coefficient  | FTD Derived  | Experimental | Error %
--------------------------------------------------
a_V          | 15.750       | 15.750       | 0.0
a_S          | 17.813       | 17.800       | 0.1
a_C          | 0.720        | 0.711        | 1.3
...

[2] MAXIMUM STABILITY (IRON PEAK)
--------------------------------------------------
    Most stable nucleus: A=52, Z=24
    Maximum B/A = 8.829 MeV/nucleon
    [PASS] Iron peak emerges from FTD integers!

[3] BINDING ENERGY VALIDATION
...
```

### Full Q-Value Verification

```bash
python -m derivations.mass_defect
```

### Full Fusion/Fission Analysis

```bash
python -m derivations.fusion_fission
```

---

## Custom Calculations

### Arbitrary Nucleus

```python
from derivations.binding_energy import binding_energy

# Plutonium-239
A, Z = 239, 94
B = binding_energy(A, Z)
print(f"Pu-239 binding: {B:.1f} MeV")
```

### Custom Reaction

```python
from derivations.mass_defect import NuclearReaction, calculate_q_value

# Custom reaction: Li-7 + p -> 2 He-4
reaction = NuclearReaction(
    name='Li-7 + p -> 2 He-4',
    reactants=[(7, 3), (1, 1)],   # Li-7 + proton
    products=[(4, 2), (4, 2)],    # 2 alpha particles
)

Q = calculate_q_value(reaction)
print(f"Q = {Q:.2f} MeV")
# This reaction releases ~17.3 MeV
```

### Stellar Nucleosynthesis

```python
from derivations.binding_energy import binding_energy_per_nucleon
import numpy as np

# Plot B/A for elements up to iron
elements = [
    ('H', 1, 1), ('He', 4, 2), ('C', 12, 6), ('N', 14, 7),
    ('O', 16, 8), ('Ne', 20, 10), ('Mg', 24, 12), ('Si', 28, 14),
    ('S', 32, 16), ('Ar', 40, 18), ('Ca', 40, 20), ('Fe', 56, 26)
]

for name, A, Z in elements:
    B_A = binding_energy_per_nucleon(A, Z)
    print(f"{name:2s}-{A:2d}: B/A = {B_A:.2f} MeV")
```

---

## API Reference

### binding_energy.py

```python
def binding_energy(A: int, Z: int,
                   coefficients: Optional[dict] = None,
                   use_shell_corrections: bool = True) -> float:
    """
    Calculate total binding energy B(A,Z) in MeV.

    Parameters:
        A: Mass number (protons + neutrons)
        Z: Atomic number (protons)
        coefficients: SEMF coefficients (uses FTD-derived if None)
        use_shell_corrections: Apply corrections for light/magic nuclei

    Returns:
        Binding energy in MeV
    """

def binding_energy_per_nucleon(A: int, Z: int,
                                coefficients: Optional[dict] = None) -> float:
    """
    Calculate binding energy per nucleon B/A in MeV.
    """

def find_optimal_Z(A: int, coefficients: Optional[dict] = None) -> int:
    """
    Find the most stable Z for a given A (valley of stability).
    """

def generate_binding_curve(A_max: int = 250) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Generate the binding energy per nucleon curve.
    Returns: (A_values, Z_values, B_per_A_values)
    """

def find_maximum_stability() -> Tuple[int, int, float]:
    """
    Find the nucleus with maximum B/A (iron peak).
    Returns: (A, Z, B/A)
    """
```

### mass_defect.py

```python
@dataclass
class NuclearReaction:
    name: str
    reactants: List[Tuple[int, int]]  # List of (A, Z)
    products: List[Tuple[int, int]]
    description: str = ""

def calculate_q_value(reaction: NuclearReaction) -> float:
    """
    Calculate Q-value (energy released) for a nuclear reaction.
    Q = B(products) - B(reactants)
    """

def nuclear_mass(A: int, Z: int, use_binding: bool = True) -> float:
    """
    Calculate nuclear mass in MeV/c^2.
    M(A,Z) = Z*m_p + N*m_n - B(A,Z)
    """
```

---

## Troubleshooting

### Import Errors

Make sure you're in the `ftd-fusion` directory:
```bash
cd ftd-fusion
python -c "from derivations.binding_energy import binding_energy; print('OK')"
```

### Negative Binding Energy

The SEMF can give negative values for exotic nuclei far from stability. The code automatically returns 0 for these cases:
```python
return max(0, B)
```

### Light Nuclei Accuracy

For A <= 4, the code uses empirical shell model values instead of SEMF:
```python
binding_energy(2, 1)  # Returns 2.224 MeV (empirical)
```

---

## Contributing

Contributions welcome! Areas of interest:
- Additional nuclear reactions (CNO cycle, r-process)
- Stellar nucleosynthesis pathways
- Coulomb barrier and tunneling calculations
- Improved SEMF coefficient derivations

Submit issues and PRs to: https://github.com/ftd/ftd-fusion
