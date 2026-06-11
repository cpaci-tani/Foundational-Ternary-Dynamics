# SCOPE LIMITATIONS
## Foundational Ternary Dynamics v5.27-bell

**Document Status:** REQUIRED FOR ALL DISTRIBUTIONS
**Last Updated:** 2026-02-26

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
- **Full non-linear Einstein equations**: Linearized equations derived (v5.0); full non-linear regime remains approximate
- **Kerr metrics**: Not derived from FTD axioms (Schwarzschild and Reissner-Nordstrom are derived; see DERIV_LATTICE_SCHWARZSCHILD.md, DERIV_LATTICE_REISSNER_NORDSTROM.md)
- **Gravitational waves**: Qualitative correspondence established

### Required Acknowledgment
> FTD operates on a fixed cubic lattice, which fundamentally violates diffeomorphism invariance. The framework can only claim approximate GR recovery at scales >> lattice spacing (ℓ_P). Einstein equations with correct 8πG coefficient are derived (v5.0), but full non-linear GR recovery is not achievable within the current axiom structure.

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

## 5. Simulation Engine **[MINOR]**

### Current Status
The C++ simulation engine (`engine/`) is **feature-complete** with all 12 update phases implemented:

| Phase | Status |
|-------|--------|
| 1. Time Gating |  Implemented |
| 2. Entropy |  Implemented |
| 3. Existence Transitions |  Implemented |
| 4. Wave Propagation |  Implemented |
| 5. Field Computation |  Implemented |
| 6. Force Accumulation |  Implemented |
| 7. Integration |  Implemented |
| 8. Movement |  Implemented |
| 9. Collisions |  Implemented |
| 10. Transmutation |  Implemented |
| 11. Binding |  Implemented |
| 12. Increment |  Implemented |

- **GUI:** Qt6 native interface (`engine/qt_gui/`) with 9 panels and OpenGL viewport
- **Tests:** 61 CTests (variational proof, forces, SM sectors)
- **Build:** CMake build system

**Note:** The former Python prototype (`ternary_matrix/`) has been archived. The C++ engine is the sole active simulation platform.

### Remaining Limitation
> The engine is feature-complete but operates on small lattice sizes. Large-scale physics validation (multi-particle bound states, thermodynamic limits) remains computationally constrained.

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

> **Foundational Ternary Dynamics v5.27-bell** is a discrete computational framework for particle physics and cosmology with the following acknowledged limitations:
>
> 1. **Gravity:** Einstein equations derived with 8πG coefficient (v5.0); diffeomorphism invariance fundamentally violated by discrete lattice
> 2. **Gauge theory:** U(1) emergence argued; SU(2) × SU(3) conjectured, not proven
> 3. **Intermediate scales:** Chapters IV-VII provide pedagogical context only; no FTD-derived predictions
> 4. **Renormalization:** Not addressed
> 5. **Simulation:** C++ engine feature-complete (12/12 phases); large-scale validation computationally constrained
> 6. **Falsifiability:** Limited near-term testable predictions
>
> These limitations do not invalidate the framework's achievements in particle physics numerical predictions (30+ at sub-1% accuracy) but define the current scope of rigorous claims.

---

*Document Classification: REQUIRED FOR v5.27-bell CERTIFICATION*
*Created: 2026-01-24*
*Updated: 2026-02-26*
