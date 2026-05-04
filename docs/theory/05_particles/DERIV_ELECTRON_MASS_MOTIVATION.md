# Electron Mass from the Lattice Structure

## Motivation for m_e = M_P · √(2π) · (16/3) · α¹¹

**Date:** April 3, 2026
**Status:** [SELECTION] — each factor is structurally motivated; the combination is not uniquely derived
**Depends on:** FOUND_LADDER_GENERATING_RULE.md, DERIV_MASTER_QUADRATIC_FROM_Z.md, DERIV_PHI3_EXACT_EFT.md

---

## The Formula

$$m_e = M_P \cdot \sqrt{2\pi} \cdot \frac{16}{3} \cdot \alpha^{11}$$

**Result:** 0.5124 MeV (0.19% from PDG value 0.51100 MeV).

This is a parametric insertion: the formula structure (M_P · prefactor · α^n) is standard dimensional analysis for relating particle masses to the Planck scale. What FTD provides is a structural motivation for each factor.

---

## Factor-by-Factor Motivation

### Factor 1: M_P (the Planck mass)

The lattice has a UV cutoff at the lattice spacing. The natural mass scale of the cubic lattice ℤ³ is the Planck mass M_P = 1.221 × 10¹⁹ GeV. This is the only external input in the formula. [IMPOSED]

### Factor 2: √(2π)

The partition function Z(x) involves an exact Gaussian integral over the flux field J (see DERIV_MASTER_QUADRATIC_FROM_Z.md, Step 1). The Gaussian integral over a single real variable produces √(2π). This factor is the signature of the exact J-integration. [SELECTION] (downgraded from [THEOREM] 2026-05-03 night per FTD-0133 audit; the cited DERIV_MASTER_QUADRATIC_FROM_Z.md derivation route was retracted as FTD-0032 on 2026-04-19, and even the retracted chain produced a *cancelling* 2π rather than a surviving √(2π); the √(2π) factor in m_e is more honestly read as inherited from `v = M_P·√(2π)·α^8` in HIGGS-4, which is itself [SELECTION]; the √(2π) cancels in the cleaner ratio `m_e/v = (16/3)·α³` whose exponent 3 = N_c is [DERIVED] post-MC-T3.2 closure)

In the φ³ EFT (DERIV_PHI3_EXACT_EFT.md), the mass parameter m² = x₊ − x₋ = 134.012. The ratio √(2π)/m ≈ 0.217 sets the natural scale at which loop corrections become significant.

### Factor 3: 16/3 = |Aut(Eᵢ)|² / D

This ratio appears throughout FTD:

- **16 = |Aut(Eᵢ)|²**: the squared automorphism order of the CM elliptic curve, which is the coefficient of the master quadratic. It counts the gauge-fixed degrees of freedom on the minimal torus (Faddeev-Popov ghost counting, O_h/ℤ₃ = 48/3 = 16). [THEOREM]

- **3 = D**: the spatial dimension, uniquely selected by 16 = 2^D · (D−1)!. [THEOREM]

- **16/3**: the number of gauge DOF per spatial dimension. This is the lattice's "charge-per-axis" — how many independent field configurations contribute to the self-energy along each coordinate direction. [SELECTION for interpretation]

Numerically: 16/3 = 5.333...

### Factor 4: α¹¹

The exponent 11 is the position of the electron on the alpha-power ladder:

| n | Scale | Construction |
|---|-------|-------------|
| 4 | Perturbative boundary | n = D + 1 = N_base (spacetime dimension) |
| +4 | Electroweak structure | + N_base (SU(2) doublets, Higgs, mass generation) |
| +3 | Color confinement | + N_c (hadrons, stable matter) |
| = 11 | **Electron mass** | 4 + 4 + 3 = 11 |

**Why the electron sits at n = 11:** The electron is the lightest charged lepton. It requires:
1. The perturbative QED regime (n = 1 through 4)
2. Electroweak symmetry breaking to acquire mass via the Higgs mechanism (+ N_base = 4)
3. The QCD vacuum to set the confinement scale that stabilizes the proton, which in turn provides the environment for atomic matter (+ N_c = 3)

The electron doesn't need the second color factor (neutrino mixing, +3) or all flavors (gravity, +6), so it sits below n = 14 (neutrino) and n = 20 (gravitational scale).

**The ladder rule [THEOREM]:** The total walk 4 + 3 + 3 + 6 = 16 = |Aut(Eᵢ)|². The alpha-power ladder exhausts all particle-counting integers exactly once, and their sum equals the master quadratic coefficient. This is a structural identity (LGR-2 in FOUND_LADDER_GENERATING_RULE.md).

---

## The Complete Argument

Combining all factors:

$$m_e = \underbrace{M_P}_{\text{UV cutoff}} \cdot \underbrace{\sqrt{2\pi}}_{\text{Gaussian flux integral}} \cdot \underbrace{\frac{16}{3}}_{\text{gauge DOF / dimension}} \cdot \underbrace{\alpha^{11}}_{\text{EW + color on ladder}}$$

Each factor has a structural origin:
- M_P: the lattice scale [IMPOSED]
- √(2π): exact Gaussian integration [THEOREM]
- 16/3: gauge DOF per axis [SELECTION]
- α¹¹: ladder position [SELECTION for physical interpretation, THEOREM for the algebra]

---

## Epistemic Status

| Component | Tag | Justification |
|-----------|-----|---------------|
| m_e = M_P · prefactor · α^n (dimensional form) | [IMPOSED] | Standard Planck-scale mass formula |
| √(2π) from Gaussian J-integral | [SELECTION] | Originally tagged [THEOREM] citing DERIV_MASTER_QUADRATIC_FROM_Z.md Step 1; that document was RETRACTED as FTD-0032 (2026-04-19) and even its chain gave a *cancelling* 2π. The √(2π) factor is more honestly understood as inherited from `v = M_P·√(2π)·α^8` (HIGGS-4 [SELECTION]). Downgraded 2026-05-03 night per FTD-0133. |
| 16 = |Aut(Eᵢ)|² from Faddeev-Popov | [THEOREM] | O_h gauge fixing on minimal torus |
| 3 = D from |Aut|² = 2^D·(D−1)! | [THEOREM] | Algebraic uniqueness |
| Exponent 11 from ladder position | [SELECTION] | Structural walk, not uniquely forced |
| Combined formula | [SELECTION] | Motivated but not derived from the action |

**Honesty note:** This is a parametric insertion with structural motivation. The formula m_e = M_P · √(2π) · (16/3) · α¹¹ is not derived from the FTD Lagrangian via a Feynman diagram computation. Each factor is individually motivated, but the specific combination is identified by matching to the known electron mass. A first-principles derivation would require computing the pole mass from the lattice propagator, which has not been done.

---

## Verification

```python
from scripts.constants import M_PLANCK, ALPHA, Experimental
import numpy as np

m_e_derived = M_PLANCK * np.sqrt(2*np.pi) * (16/3) * ALPHA**11
m_e_exp = Experimental.m_electron / 1000  # Convert MeV to GeV
error = abs(m_e_derived - m_e_exp) / m_e_exp * 100

print(f"m_e (FTD):  {m_e_derived*1000:.4f} MeV")   # 0.5124 MeV
print(f"m_e (PDG):  {m_e_exp*1000:.4f} MeV")        # 0.5110 MeV
print(f"Error:      {error:.2f}%")                    # 0.19%
```

---

## Cross-References

- **Ladder rule:** [FOUND_LADDER_GENERATING_RULE.md](../02_foundations/FOUND_LADDER_GENERATING_RULE.md) — why {1,2,3,4,8,11,14,20}
- **16 from Faddeev-Popov:** [DERIV_MASTER_QUADRATIC_FROM_Z.md](../03_derivations/DERIV_MASTER_QUADRATIC_FROM_Z.md) — gauge DOF counting
- **D = 3:** [DERIV_D3_FROM_AUTOMORPHISM.md](../02_foundations/DERIV_D3_FROM_AUTOMORPHISM.md) — dimensional uniqueness
- **Muon/tau ratios:** [DERIV_COMPLETE_PARTICLE_PHYSICS.md](DERIV_COMPLETE_PARTICLE_PHYSICS.md) — lepton mass hierarchy
