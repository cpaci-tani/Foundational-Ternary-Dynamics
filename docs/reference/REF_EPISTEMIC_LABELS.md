# EPISTEMIC LABELS REFERENCE
## Foundational Ternary Dynamics v5.27-bell

**Document Status:** AUTHORITATIVE REFERENCE
**Last Updated:** 2026-02-26

---

## Purpose

This document defines the epistemic labels used throughout FTD documentation and code to clearly distinguish different levels of certainty and derivation status.

---

## Label Definitions

### [AXIOM]
**Meaning:** Structural postulate that defines the model. Not derivable from anything more fundamental within FTD.

**Reviewer expectation:** Accept as model definition.

**Examples:**
- Space is a 3D cubic lattice
- States are ternary {-1, 0, +1}
- Time advances in discrete ticks
- Local causality (26-neighbor updates)

---

### [THEOREM]
**Meaning:** Rigorously proven from axioms via mathematical derivation.

**Reviewer expectation:** Check the proof.

**Examples:**
- Update rules from Euler-Lagrange equations (given action S[s,J])
- Hilbert space construction from complexified flux
- Conservation laws from closed system + deterministic update

---

### [DERIVED]
**Meaning:** Computed from framework integers via pure arithmetic or algebra. No fitting to experiment.

**Reviewer expectation:** Verify the calculation.

**Examples:**
- Muon/electron ratio: 3 × 7 × 10 - 3 = 207 [DERIVED]
- Tau/electron ratio: 17 × 207 - 42 = 3477 [DERIVED]
- α from master quadratic x₊ = 137.036 [DERIVED once G* is established]

**Important:** [DERIVED] does not mean "proven from first principles in physics." It means "follows arithmetically from the integers within FTD assumptions."

---

### [SELECTION]
**Meaning:** Argued from consistency constraints, but not uniquely proven. Other choices might work.

**Reviewer expectation:** Critique the selection argument.

**Examples:**
- CM selection j = 1728 for lemniscatic curve [SELECTION]
- Quark mass formulas (other integer combinations could fit) [SELECTION]
- Boson mass formulas [SELECTION]
- CP phase δ = arctan(7/3) [SELECTION]

---

### [FITTED]
**Meaning:** Parameter chosen or formula tuned to match experimental values.

**Reviewer expectation:** Note as input, not output.

**Examples:**
- ~~Quark masses~~ (now [SELECTION] with larger errors)
- DECAY_RATE = α (phenomenological targeting) [FITTED/IMPOSED]
- KB = 0.511 MeV (matched to electron mass) [IMPOSED]

---

### [IMPOSED]
**Meaning:** Parameter choice or model calibration that could have been otherwise.

**Reviewer expectation:** Note as input, not output.

**Examples:**
- 1 voxel = Planck length (scale identification) [IMPOSED]
- 1 tick = Planck time [IMPOSED]
- Dissipation rate γ = α [IMPOSED]
- 26-neighbor Moore neighborhood [IMPOSED]

---

### [CONJECTURE]
**Meaning:** Proposed interpretation or correspondence that requires validation.

**Reviewer expectation:** Demand evidence.

**Examples:**
- U(1) gauge symmetry emerges from Helmholtz [CONJECTURE]
- SU(2) from ternary structure [CONJECTURE]
- SU(3) from 3 spatial dimensions [CONJECTURE]
- CKM matrix from integer fractions [CONJECTURE/NUMEROLOGY]

---

### [EMERGENT]
**Meaning:** Behavior arising from dynamics without being explicitly designed in.

**Reviewer expectation:** Verify in simulation.

**Examples:**
- Bound structures (triads) [EMERGENT]
- Interference patterns [EMERGENT]
- Hierarchical organization [EMERGENT]
- 2 photon polarizations from constraint counting [EMERGENT]

---

### [NUMEROLOGY]
**Meaning:** Pattern matching between numbers without dynamical derivation.

**Reviewer expectation:** Skepticism appropriate.

**Examples:**
- CKM angles from α-dependent formulas [NUMEROLOGY]
- Some mass relations with unexplained numerical factors

---

### [OPEN]
**Meaning:** Unresolved question requiring further research.

**Reviewer expectation:** Research opportunity.

**Examples:**
- Planck-scale Lorentz departures
- Experimental validation of Bell predictions
- Full QFT correspondence

---

## Code Comment Conventions

In Python/code files, use comments like:
```python
# [DERIVED] Pure integer arithmetic
muon_ratio = 3 * b_3 * (b_3 + N_c) - N_c  # = 207

# [SELECTION] Formula chosen for numerical match
up_ratio = N_base + sin2_theta_w

# [FITTED] Tuned to experimental value
decay_rate = alpha  # = 0.00729
```

---

## Consistency Requirements

1. **Code comments** must match **manuscript epistemic tags**
2. **CLAUDE.md** is the authoritative source for claim status
3. **Chapter 21 of SPEC_CLAUDE.md** (Assumption Ledger) tracks all assumptions
4. When in doubt, use the more conservative label

---

## Label Hierarchy (Strength of Claim)

From strongest to weakest:

1. **[AXIOM]** - Definitional, cannot be wrong within model
2. **[THEOREM]** - Mathematically proven
3. **[DERIVED]** - Arithmetically computed
4. **[SELECTION]** - Argued but not unique
5. **[EMERGENT]** - Observed in simulation
6. **[CONJECTURE]** - Proposed, needs validation
7. **[NUMEROLOGY]** - Pattern without derivation
8. **[FITTED]/[IMPOSED]** - Input, not output
9. **[OPEN]** - Unresolved

---

## Summary Table: Key FTD Claims

| Claim | Label | Notes |
|-------|-------|-------|
| α = 1/137.036 | [DERIVED] (from G* via CM) | Once CM selection accepted |
| N_c = 3 | [SELECTION] | From x₋ ≈ 3.024 |
| m_e formula | [DERIVED] | m_P √(2π) (16/3) α¹¹ |
| Lepton ratios | [DERIVED] | Pure integer arithmetic |
| Quark masses | [SELECTION] | Formulas not unique |
| CKM angles | [NUMEROLOGY] | 5-20% errors |
| PMNS angles | [SELECTION] | Integer fractions, 1-13% errors |
| CP phase δ | [SELECTION] | arctan(7/3) = 66.8° |
| U(1) gauge | [CONJECTURE] | Helmholtz argument only |
| SU(2)/SU(3) gauge | [CONJECTURE] | Geometric motivation only |
| GR recovery | [CONJECTURE] | Linearized only, diffeomorphism violated |
| Bell violations | [SELECTION] | Three-level observer hierarchy: substrate S=2, observer S=2√2. See DERIV_OBSERVER_BELL_MECHANISM.md |
| Born rule | [SELECTION] | Emerges under imposed sampling rule |

---

*Document Classification: AUTHORITATIVE REFERENCE*
*Created: 2026-01-24*
*Updated: 2026-02-26*
