# No-Go Theorem — Mass-Unit μ is Not Derivable from Axiom Zero

**Tag:** [THEOREM] / [CLOSED NEGATIVE for FTD-0096]
**Date:** 2026-04-28
**LEDGER row:** FTD-0096 (extension of FTD-0059)
**Dependencies:** FTD-0059 (a_phys no-go theorem; `THEOREM_A_PHYS_NO_GO.md`).
**Supersedes status of:** FTD-0096 ([OPEN] → [CLOSED THEOREM-NEGATIVE]); confirms terminal [PARAMETRIC] for FTD-0094 (the L₂ identity).

---

## 1 · Statement

> **No-Go Theorem (μ).** No quantity with SI dimension of mass is derivable from Axiom Zero alone, including from the threshold parameters of the FTD update rules (`K_GENESIS`, `K_EVAP`, `K_drain`, etc.). Consequently, the lattice-to-physical mass conversion `μ` (the FTD mass unit) must be supplied as an external calibration input, on equal footing with `a_phys`.

This theorem extends FTD-0059's Corollary 4.1 ("No mass, time, energy, temperature, or charge from Axiom Zero") to address the specific FTD-0096 hypothesis that **the dynamical threshold parameters (`K_GENESIS` and friends) might supply a hidden mass-dimensional generator**. They do not.

---

## 2 · Background — what FTD-0059 already covers

FTD-0059's Corollary 4.1 explicitly extends the length no-go to mass:

> *Proof.* Replace "L¹" in Claim B with `M¹`, `T¹`, `M L² T⁻²`, `Θ¹`, or `I T`. Claims A and C apply verbatim. `∎`

This says: by the same ring-algebra argument as for `a_phys`, no element of the Axiom-Zero ring `R` has SI mass dimension `M¹`. Therefore no function of Axiom-Zero invariants produces a quantity with mass units.

**The FTD-0096 [OPEN] caveat (LEDGER row, 2026-04-26):** the ring-algebra proof technique applied to kinematic dimensions assumes Axiom Zero defines only *kinematic* primitives (lattice geometry, time-stepping, ternary states). The 2026-04-26 LEDGER row noted: "*mass enters through dynamical threshold (manifestation rule, K_GENESIS), not the dimensional ring*", suggesting that the threshold parameters might supply a hidden mass-dimensional generator that escapes the kinematic ring-algebra argument.

This document closes that hypothesis: **`K_GENESIS` and the other threshold parameters are dimensionless in the same sense as `a_phys` — they are pure numbers in the abstract update rules — and therefore Corollary 4.1 applies to them too.**

---

## 3 · The K_GENESIS-as-dimensionless argument

### 3.1 Definitions

- **`K_GENESIS`**: the threshold value such that genesis manifestation triggers when `|φ_v|² > K_GENESIS²` at voxel `v`. In the engine implementation, `K_GENESIS = 1` (in engine units).
- **`K_EVAP`**: the threshold below which evaporation triggers (`|φ_v|² < K_EVAP²` → state cleared).
- **`K_drain`**: the fraction of flux removed when genesis manifests at a voxel.

Each of these is a parameter of the manifestation rule (Postulate 2 in `SPEC_FTD.md`).

### 3.2 The dimensional status of `K_GENESIS`

The flux field `φ : V_lattice → R³` has its values in `R³`. Under Axiom Zero, this is a dimensionless `R³`-valued field on a dimensionless lattice. The condition `|φ_v|² > K_GENESIS²` compares two dimensionless real numbers.

**Therefore `K_GENESIS` is dimensionless in SI units.**

There is no hidden "mass per voxel" dimension lurking inside `K_GENESIS`. The framework is specified abstractly in dimensionless terms; the threshold value `1` is a pure number, not a kg or a MeV.

### 3.3 What might have been thought (and why it fails)

A plausible-but-incorrect reading: `K_GENESIS` *should* have units of mass-density (or energy-density, or similar) because the manifestation rule converts flux energy into a manifested state, which intuitively has "mass." Surely the threshold for that manifestation has units of mass-density?

**Why this fails:** the manifestation rule is specified at the abstract-Axiom-Zero level as

```
if |φ_v|² > K_GENESIS²:
    s_v ← sign(...)
```

This rule operates on dimensionless quantities (`|φ|²` is a dimensionless number; `K_GENESIS` is a dimensionless number). The interpretation of the resulting `s_v ∈ {-1, 0, +1}` as a "manifested particle" is a *physical interpretation step* — it converts the abstract framework to a physical reading. That conversion requires the calibration `K_B = m_e` (the mass-unit calibration declared in `SPEC_FTD.md`).

The mass-unit `μ_FTD := K_B · m_e^{phys}` is the bridge between the abstract Axiom-Zero `s_v ∈ {-1, 0, +1}` and the physical mass interpretation. **`μ_FTD` is the calibration; `K_GENESIS` is the dimensionless threshold within Axiom Zero.**

### 3.4 The same logic applies to all threshold parameters

`K_EVAP`, `K_drain`, `K_LANGEVIN_T`, and all other dynamical-rule parameters in `SPEC_FTD.md` are pure numerical constants in the abstract rule specification. None supplies a dimensional generator outside `R`.

---

## 4 · The proof

### 4.1 Statement

> **Theorem (μ no-go, formal).** Let `R'` be the ring obtained from `R` (the Axiom-Zero ring of FTD-0059) by adjoining all numerical constants appearing in the FTD update rules (`K_GENESIS`, `K_EVAP`, `K_drain`, `K_LANGEVIN_T`, etc.) and closing under `+, −, ×, ÷, exp, log, Γ, θ`, Watson integrals. Then every element of `R'` has SI dimension `1` (the zero element of the SI dimension module).

### 4.2 Proof

By construction, the threshold parameters `{K_GENESIS, K_EVAP, K_drain, K_LANGEVIN_T, ...}` are specified in the abstract update rules of `SPEC_FTD.md` as numerical constants (e.g., `K_GENESIS = 1`, `K_drain = 1/2`). They are real numbers, not labelled with SI units.

The ring `R'` is therefore the smallest sub-ring of `R` containing all Axiom-Zero invariants AND all rule-parameter constants. Each generator of `R'` has SI dimension `1`. Ring operations and the standard transcendental closures preserve SI dimension `1` (per FTD-0059 Claim A's argument).

Therefore every element of `R'` has SI dimension `1`. By the same logic as FTD-0059 Claim C (replacing `L¹` with `M¹`), no element of `R'` has SI mass dimension `M¹`. ∎

### 4.3 Corollary

The mass-unit `μ_FTD` (the SI mass corresponding to one FTD-internal mass unit, e.g., `K_B · m_e^{phys}` in the canonical calibration) cannot be expressed as a function of elements of `R'` alone. Any expression yielding `μ_FTD` in kg must contain at least one external dimensional generator. That generator is the calibration. ∎

---

## 5 · Consequences

### 5.1 The L₂ identity is terminally [PARAMETRIC]

The L₂ identity `2 m_e/α = 16 G*²` (FTD-0094) involves `m_e` expressed in MeV (or any other physical mass unit). To evaluate it, one needs the calibration `μ_FTD := K_B · m_e^{phys}`. By the no-go theorem, `μ_FTD` is calibration, not derivation.

The L₂ identity is therefore terminally [PARAMETRIC]:
- The dimensionless ratio `m_e/α · 2/(16 G*²)` is in `R'` and can be evaluated to any precision.
- The numerical match to `1` at 68.77 ppm is calibration-dependent: under the canonical `K_B = m_e` calibration, the match is exact by construction; under any other calibration, the match would be different.
- No further structural promotion is possible without an external mass anchor — which would be a calibration choice, not a derivation.

This **independently confirms** the terminal [PARAMETRIC] tag that the LEDGER (2026-04-27) assigned to FTD-0094 based on the methodological FTD-0097 look-elsewhere null-rejected-upward verdict. **The structural side (this document) and the methodological side (FTD-0097) now jointly close FTD-0094 as [PARAMETRIC] from two independent directions.**

### 5.2 The dimensional-ring of FTD has exactly two calibrations

Per FTD-0059 §4.2, dimensional FTD predictions require exactly one calibration per independent SI dimension. The base set is `{L, T, M, E}`; with `c_lat  c_phys` linking length-time and `c² = E/M` linking mass-energy, the irreducible calibration count is **two**: `a_phys ≡ ℓ_P` (length) and `K_B = m_e` (mass).

This document closes FTD-0096 by confirming that **mass cannot be reduced further** — the second calibration is structurally required.

### 5.3 Cluster-mass identification (FTD-0110) and mass-unit calibration

The cluster-mass identification of FTD-0110 (`N(A = 2√R) = R · m_e^{phys}/m_e^{phys} = R` voxels for SM particle with mass ratio `R = m_X/m_e`) is calibration-CONSISTENT: both sides of the identification are dimensionless ratios (cluster size in voxel count; mass ratio in `m_e` units). The empirical 5%-precision match across 5 SM particles holds independently of the absolute calibration `μ_FTD`.

**This means FTD-0110 main claim is structurally untouched by the μ no-go theorem.** The cluster-mass identification is a dimensionless ratio statement, not a dimensional mass-derivation. The no-go closes FTD-0096 (calibration is calibration); it does not close FTD-0110 (which is about ratio-of-masses, calibration-independent).

### 5.4 Updated calibration narrative for SPEC_FTD.md

The current calibration declaration in `SPEC_FTD.md` ("LATTICE  PHYSICAL CALIBRATION") declares two calibrations as design choices. With this theorem, the declaration becomes a **theorem-enforced minimum**: exactly two calibrations are required by the no-go, and any third calibration would over-determine the system.

Suggested SPEC_FTD update text:

> "Per FTD-0059 (length no-go) and FTD-0096 (mass no-go), exactly two SI-dimensional calibrations are theorem-enforced as the irreducible minimum: `a_phys ≡ ℓ_P` (length) and `K_B = m_e` (mass). Adding a third calibration (e.g., separately fixing `ℏ` or `c_phys`) would over-determine the dimensional system and must be checked for consistency with the two primary anchors."

---

## 6 · What this theorem does **not** claim

- It does not claim that the L₂ identity is wrong, or that the structural relation `2 m_e/α = 16 G*²` is uninteresting. It claims that the identity, if assigned a numerical value, depends on the calibration choice and therefore cannot be promoted past [PARAMETRIC].
- It does not claim that all FTD mass-related predictions are calibration-dependent. The dimensionless ratios (`m_p/m_e`, `m_μ/m_e`, etc.) are calibration-independent and remain falsifiable spine elements.
- It does not foreclose a future Axiom-Zero extension that introduces a mass-dimensional generator. Such an extension would change `R'` and require re-examination of the theorem. The theorem is conditional on the current Axiom Zero + current update-rule parameter set.

---

## 7 · Connection to FTD-0094 chain

FTD-0094 (the L₂ identity `2m_e/α = 16G*²`) had pre-registered demotion conditions:

1. FTD-0093 (Mechanism C structural derivation of `g_c`) closes negative.
2. FTD-0096 (μ-from-ℓ_P missing arrow) remains [OPEN] or closes negative.

Both conditions are now met:

- **2026-04-27**: FTD-0093 [CLOSED NEGATIVE] at L ∈ {24, 32, 48} (LEDGER FTD-0093).
- **2026-04-28** (this document): FTD-0096 [CLOSED THEOREM-NEGATIVE] via mass-dimensional ring-algebra extension.

**FTD-0094 is therefore terminally [PARAMETRIC] under three independent closures:**

1. Methodological (FTD-0097 look-elsewhere scan, 2026-04-27): the L₂ identity appears among chance-level monomial fits at exactly its claimed 68.77 ppm precision.
2. Structural-mechanism (FTD-0093 closed negative, 2026-04-27): the proposed structural mechanism for `g_c` (which would have anchored the L₂ identity) closes.
3. Dimensional (FTD-0096 closed theorem-negative, 2026-04-28; this document): the mass unit required to evaluate the L₂ identity numerically is calibration, not derivation.

Three independent closures from different directions all agree: the L₂ identity is terminally [PARAMETRIC] and cannot be promoted further within the current Axiom-Zero framework.

---

## 8 · LEDGER tag movement

**FTD-0096 (μ-from-ℓ_P missing arrow):**
- Was: [OPEN] (2026-04-26)
- Becomes: **[CLOSED THEOREM-NEGATIVE]** (2026-04-28). Mass-dimensional ring-algebra argument (this document) extends FTD-0059 to cover threshold parameters; mass-unit μ is calibration, not derivation.

**FTD-0094 (L₂ identity 2m_e/α = 16G*²):**
- Was: [PARAMETRIC] terminal (2026-04-27, conditional on FTD-0096 staying [OPEN])
- Becomes: **[PARAMETRIC] terminal under three independent closures** (2026-04-28). Tag unchanged but no longer conditional; closure dimensions expanded.

**FTD-0059 (a_phys no-go theorem):**
- Tag unchanged (still [THEOREM]).
- Cross-reference added: this document is the dual closure of Corollary 4.1's mass extension.

---

## 9 · Cross-references

- FTD-0059 a_phys no-go (parent theorem): [`THEOREM_A_PHYS_NO_GO.md`](THEOREM_A_PHYS_NO_GO.md)
- LEDGER row FTD-0096: `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- LEDGER row FTD-0094 (L₂ identity): `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- LEDGER row FTD-0093 (Mechanism C closure): `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- LEDGER row FTD-0097 (look-elsewhere scan): `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- Open-item tracker: [`archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md`](archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md) — marked CLOSED RESOLVED-BY-THEOREM
- SPEC_FTD calibration declaration: `docs/SPEC_FTD.md` (LATTICE  PHYSICAL CALIBRATION section)
- Sympathetic audit identifying this as closeable: `docs/theory/07_assessment/AUDIT_PAPER_SYMPATHETIC_2026-04-28.md`

---

## 10 · Single-line summary

**FTD-0096 (μ-from-ℓ_P missing arrow) closes [THEOREM-NEGATIVE] via extension of FTD-0059's ring-algebra argument: the manifestation-rule threshold parameters (`K_GENESIS`, `K_EVAP`, `K_drain`, `K_LANGEVIN_T`) are dimensionless real numbers in the abstract Axiom-Zero specification, hence belong to the dimensionless ring `R'`; therefore no function of Axiom-Zero invariants and rule parameters produces a quantity with SI mass dimension. Mass-unit `μ_FTD` is calibration, not derivation. Consequence: the L₂ identity `2 m_e/α = 16 G*²` (FTD-0094) is terminally [PARAMETRIC] under three independent closures (methodological FTD-0097, structural FTD-0093, dimensional FTD-0096). The dimensional-calibration interface of FTD is theorem-enforced at exactly two parameters: `a_phys ≡ ℓ_P` and `K_B = m_e`. Cluster-mass identification (FTD-0110) is unaffected — it's a dimensionless ratio statement, not a dimensional mass derivation.**
