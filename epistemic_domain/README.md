# FTD Epistemic Domain

A mathematical framework for deriving physics from ternary foundations.

## Philosophy

Instead of simulating a "universe" with numerical instabilities, we build an **epistemic domain**: a hierarchy of mathematical objects with precisely tracked derivations.

Each level derives from the previous, with explicit epistemic status:
- **AXIOM**: Foundational postulate (not derivable)
- **DERIVED**: Follows mathematically from axioms
- **IMPOSED**: Parameter choice (calibration)
- **EMERGENT**: Behavior arising from structure

---

## Level Hierarchy

### Level 0: Planck Scale (`level_0_planck.py`)

The irreducible foundation.

**Axioms:**
- Space is a 3D cubic lattice (Z^3)
- Time is discrete ticks (N)
- Each site has state in {-1, 0, +1}
- Causality is local (26-neighbor Moore)
- Maximum speed is 1 site/tick

**Derived Constants:**
| Constant | Value | Source |
|----------|-------|--------|
| G* (lemniscatic) | 2.958675 | Gamma(1/4)^2 * sqrt(2) / (2*pi) |
| 1/alpha | 137.036171 | Master quadratic root x_+ |
| N_c | 3.024 -> 3 | Master quadratic root x_- |

**Accuracy:** 1/alpha matches CODATA to **1.26 ppm**

---

### Level 1: Single Voxel (`level_1_voxel.py`)

The minimal physical system.

**Derived Quantities:**
| Quantity | Formula | Value |
|----------|---------|-------|
| KB (threshold) | sqrt(2*pi) * (16/3) * alpha^11 | 4.18e-23 (Planck units) |
| m_e (electron mass) | KB * m_P | 0.510 MeV |

**Accuracy:** Electron mass predicted to **0.19%**

**State Transitions:**
```
0 -> +1  (Genesis positive)
0 -> -1  (Genesis negative)
+1 -> 0  (Evaporation)
-1 -> 0  (Evaporation)
+1 <-> +1, -1 <-> -1, 0 <-> 0  (Persistence)
```

**Genesis Probability:**
```
p = 1 - exp(-(density - KB) / KB)  for density > KB
p = 0                               for density <= KB
```

---

### Level 2: Two Voxels (TO DO)

The minimal interacting system.

**Will derive:**
- Annihilation dynamics (+1 adjacent to -1)
- Force-like behaviors from flux gradients
- Binding conditions

---

### Level 3: Triads (TO DO)

The minimal stable structure.

**Will derive:**
- Triad geometry (3 particles)
- Binding energy
- Stability conditions

---

### Level 4: Heptad (TO DO)

The first "nucleon-like" structure.

**Will derive:**
- Heptad geometry (1 + 6 = 7 particles)
- Mass from binding
- Charge distribution

---

## Running the Verification

```bash
# Level 0: Planck scale
python -m epistemic_domain.level_0_planck

# Level 1: Single voxel
python -m epistemic_domain.level_1_voxel
```

---

## Key Results Summary

### From Pure Mathematics:

1. **Fine Structure Constant**
   - Predicted: 1/137.036171
   - Measured: 1/137.035999177
   - Error: 1.26 ppm

2. **Color Charges**
   - Predicted: 3.024
   - Measured: 3
   - Error: 0.8%

3. **Electron Mass**
   - Predicted: 0.510 MeV
   - Measured: 0.511 MeV
   - Error: 0.19%

### The Master Quadratic

The single equation that produces both electromagnetic and strong coupling:

```
x^2 - 16(G*)^2 * x + 16(G*)^3 = 0
```

Where G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = 2.9587

Roots:
- x_+ = 137.036 (= 1/alpha)
- x_- = 3.024 (= N_c)

This is the central mathematical result of the framework.

---

## Epistemic Status

This is a **mathematical framework**, not a confirmed physical theory.

The remarkable numerical agreements (ppm-level for alpha, sub-percent for masses) may be:
1. Deep truth about reality
2. Mathematical coincidence
3. A mixture of both

Independent experimental validation remains the ultimate test.
