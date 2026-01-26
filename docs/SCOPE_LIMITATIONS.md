# SCOPE LIMITATIONS
## Foundational Ternary Dynamics v1.0

**Document Status:** REQUIRED FOR ALL DISTRIBUTIONS
**Last Updated:** 2026-01-24

---

## Executive Summary

Foundational Ternary Dynamics (FTD) v1.0 is certified as a discrete computational framework for particle physics and cosmology. The framework achieves exceptional numerical accuracy (30+ predictions <1% error) at fundamental scales but operates with acknowledged limitations documented below.

---

## 1. Gravity Limitations **[CRITICAL]**

### What Is Addressed
- Inverse-square law from 3D geometry + flux conservation
- Newtonian gravity as weak-field limit of flux gradients
- Effective metric g_μν from flux density (correspondence argument)
- Linearized Einstein equations from flux wave equation
- Gravitational hierarchy α_G derived to 0.06% accuracy

### What Is NOT Addressed
- **Diffeomorphism invariance**: A cubic lattice fundamentally violates diffeomorphism invariance. This is a structural feature of discrete spacetime models, not a documentation issue.
- **Full non-linear Einstein equations**: Only linearized equations are addressed
- **Schwarzschild/Kerr metrics**: Not derived from FTD axioms
- **Gravitational waves**: Only qualitative correspondence, not rigorous derivation

### Required Acknowledgment
> FTD operates on a fixed cubic lattice, which fundamentally violates diffeomorphism invariance. The framework can only claim approximate GR recovery at scales >> lattice spacing (ℓ_P). Full GR derivation is not achievable within the current axiom structure.

---

## 2. Gauge Theory Limitations **[CRITICAL]**

### U(1) Electromagnetic Gauge Symmetry
**Status:** ARGUED (Helmholtz decomposition), NOT PROVEN

**What is argued:**
- 2 transverse modes matching photon polarizations
- Longitudinal mode constrained by Gauss law
- Gauge transformation J → J + ∇λ leaves physics invariant

**What is missing:**
- Ward identity derivation
- Anomaly analysis
- Rigorous proof of gauge invariance of the action

### SU(2) Weak Gauge Symmetry
**Status:** CONJECTURE (geometric motivation only)

**What is claimed:**
- Ternary state structure {-1, 0, +1} provides SU(2)-like structure

**What is missing:**
- Gauge-covariant derivatives
- W/Z boson field derivation
- Electroweak unification mechanism
- Weak mixing angle derivation from first principles

### SU(3) Strong Gauge Symmetry
**Status:** CONJECTURE (geometric motivation only)

**What is claimed:**
- Three spatial dimensions provide color structure
- N_c ≈ 3.024 from master quadratic
- Color neutrality observed in simulation

**What is missing:**
- Gauge-covariant derivatives
- Gluon self-interactions
- Asymptotic freedom mechanism
- Confinement proof from first principles

### Required Acknowledgment
> SU(2) and SU(3) gauge symmetries are proposed as geometric interpretations, not derived theorems. Rigorous derivation from FTD axioms remains an open research question.

---

## 3. Intermediate Scale Coverage **[MAJOR]**

### Chapters Affected
- Chapter IV: Molecular Physics (4.1-4.4)
- Chapter V: States of Matter (5.1-5.3)
- Chapter VI: Materials Science (6.1-6.4)
- Chapter VII: Planetary/Geological Scales (7.1-7.4)

### Limitation
These chapters provide **pedagogical context only**. The content is standard textbook physics presented with FTD terminology. **Zero quantitative predictions** are made at these scales from FTD axioms.

### Why This Gap Exists
- Many-body dynamics not implemented in current simulations
- Emergent chemistry requires quantum chemistry calculations
- Materials science requires condensed matter approximations
- These scales are ~10²⁰ lattice spacings from Planck scale

### Required Acknowledgment
> Chapters IV-VII provide pedagogical context only. These scales require many-body dynamics not implemented in current simulations. Zero quantitative predictions are made at molecular, materials, or geological scales.

---

## 4. Renormalization Group **[MAJOR]**

### What Is NOT Addressed
- β-function calculations
- Running coupling constants
- UV completion
- Renormalization of divergences

### Required Acknowledgment
> FTD does not address the renormalization group. Coupling constants are fixed at the Planck scale without running behavior. This represents a fundamental incompleteness for comparison with experimental QFT.

---

## 5. Simulation Engine **[MAJOR]**

### Current Status
The `ternary_matrix/` simulation engine is **incomplete** (6/12 phases implemented):

| Phase | Status |
|-------|--------|
| 1. Time Gating | ⬜ NOT IMPLEMENTED |
| 2. Entropy | ✅ Implemented |
| 3. Existence Transitions | ✅ Implemented |
| 4. Wave Propagation | ✅ Implemented |
| 5. Field Computation | ✅ Implemented |
| 6. Force Accumulation | ⬜ NOT IMPLEMENTED |
| 7. Integration | ⬜ NOT IMPLEMENTED |
| 8. Movement | ⬜ NOT IMPLEMENTED |
| 9. Collisions | ✅ Implemented |
| 10. Transmutation | ⬜ NOT IMPLEMENTED |
| 11. Binding | ✅ Implemented |
| 12. Increment | ⬜ NOT IMPLEMENTED |

### Required Acknowledgment
> Physics validation requires full simulation implementation. The current 6/12 completion limits empirical testing of framework predictions.

---

## 6. Falsifiability **[MINOR]**

### Near-Term Testable Predictions
Limited. Most predictions are:
- **Retrodictions** (matching known values)
- **Technologically impractical** (Planck-scale measurements)

### What Would Falsify FTD
1. Discovery of 4th generation with standard gauge couplings
2. Precision α measurement incompatible with x₊ = 137.036... at >10 ppm
3. Observable Lorentz violation with wrong sign
4. Bell violations exceeding 2√2

---

## 7. Summary Statement

**For All Distributions:**

> **Foundational Ternary Dynamics v1.0** is a discrete computational framework for particle physics and cosmology with the following acknowledged limitations:
>
> 1. **Gravity:** Approximate recovery only; diffeomorphism invariance fundamentally violated by discrete lattice
> 2. **Gauge theory:** U(1) emergence argued; SU(2) × SU(3) conjectured, not proven
> 3. **Intermediate scales:** Chapters IV-VII provide pedagogical context only; no FTD-derived predictions
> 4. **Renormalization:** Not addressed
> 5. **Simulation:** Incomplete; physics validation pending full implementation
> 6. **Falsifiability:** Limited near-term testable predictions
>
> These limitations do not invalidate the framework's achievements in particle physics numerical predictions (30+ at sub-1% accuracy) but define the current scope of rigorous claims.

---

*Document Classification: REQUIRED FOR v1.0 CERTIFICATION*
*Created: 2026-01-24*
