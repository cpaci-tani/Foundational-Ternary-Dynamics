# Lattice Spacing as Gauge Freedom

**Status:** Foundational clarification (no new derivation; no tag change)
**Tag:** [METHODOLOGICAL CLARIFICATION]
**LEDGER row:** FTD-0137
**Filed:** 2026-05-04 (post-FTD-0136 Discrete-Native Derivation Program)
**Builds on:** FTD-0041 calibration declaration, FTD-0059 / FTD-0096 calibration no-go theorems, FTD-0130 calibration architecture audit, FTD-0136 discrete-native reframe
**Related:** AUDIT_INFINITY_REFRAME (the parallel operationalist move at the *outer* end of scale)

---

## §1 The question

Why is the FTD voxel size declared `a_phys ≡ ℓ_P`? Two readings of the existing position:

- **Stated reading** (SPEC_DIMENSIONAL_MAP §1, FTD-0041): "no derivation works (FTD-0059, FTD-0096); therefore declare; ℓ_P is the natural minimum scale."
- **Sharpened reading** (this doc): "no derivation works because the FTD axioms do not specify a physical scale. The lattice spacing is *gauge*. The choice of gauge is operational, not ontological. ℓ_P is one defensible choice; it is not the only one and it is not derived."

This document defends the sharpened reading and articulates its consequences.

---

## §2 The circularity in importing `ℓ_P`

Standard Planck length:

```
ℓ_P = √(ℏ G / c³) ≈ 1.616 × 10⁻³⁵ m
```

The three constants:

- `ℏ` — action quantum, definitional in continuous quantum mechanics (Hilbert space)
- `c` — speed limit, definitional in continuous Lorentz invariance
- `G` — gravitational coupling, definitional in continuous spacetime curvature (general relativity)

All three are *continuous-physics primitives*. The argument that `ℓ_P` is a "minimum scale" — Heisenberg uncertainty + general-relativistic energy density forming an event horizon at Δx ~ ℓ_P — uses both continuous QM *and* continuous GR. Both are frameworks FTD considers emergent approximations rather than fundamental.

This is not "discrete physics says nothing can be smaller than `ℓ_P`." It is "continuous QM + continuous GR say measurements below `ℓ_P` become operationally impossible *given those theories*." Importing the result as the discrete substrate's voxel scale uses continuous-physics machinery to fix discrete-physics structure. That is circular for FTD's program.

---

## §3 What the FTD axioms say about scale

The five FTD postulates:

- **P1** (discrete space) — 3D cubic lattice with neighbor structure, no defined boundary
- **P2** (discrete time) — ticks, integer-valued
- **P3** (ternary states) — `s ∈ {-1, 0, +1}`
- **P4** (local causality) — 26-Moore neighborhood, max 1 voxel/tick propagation
- **P5** (determinism) — update is functional

**None specify a physical scale.** The lattice is a pure relational structure. P1 establishes that *neighbors exist*; it does not establish *how far apart they are in meters*. The lattice spacing is a *gauge degree of freedom* — undetermined by the axioms, fixed only by external declaration.

This is consistent with the existing position (FTD-0059 + FTD-0096 close the four mechanism candidates for *deriving* the calibration). What this document adds: the absence of a derivation is not a "limitation to be filled later"; it is a *structural feature* of the framework. The substrate genuinely does not have a preferred physical scale.

---

## §4 Four gauge candidates

Any external declaration that respects all dimensionless predictions is a valid gauge choice. Four natural candidates:

### §4.1 Planck-primary (current default per FTD-0041)

```
a_phys ≡ ℓ_P ≈ 1.616 × 10⁻³⁵ m
t_tick ≡ √3 · ℓ_P / c ≈ 9.34 × 10⁻⁴⁴ s
K_B ≡ m_e ≈ 0.511 MeV/c²
```

- **Pros**: reaches "as deep as the standard model of physics declares meaningful"; canonical; matches FTD-0015's `m_e/m_P` form
- **Cons**: borrows continuous-physics machinery (`ℏ, G, c`) to fix the substrate scale; per the calibration feasibility audit (`audit_calibration_feasibility_2026-05-04.py`), every physical scale of interest is 16-25 orders of magnitude beyond feasible engine lattices, blocking absolute-scale measurements across all of Class B/C/D

### §4.2 Cluster-primary (FTD-0130 path-(b) candidate)

```
K_B ≡ m_P (Planck mass anchor)
a_phys ≡ ℓ_P (length unchanged)
t_tick ≡ √3 · ℓ_P / c (time unchanged)
m_e then derived: m_e = m_P · √(2π) · (16/3) · α¹¹  (FTD-0015 promoted to load-bearing)
```

- **Pros**: makes FTD-0015 the load-bearing mass derivation rather than a verification check; ontologically top-down
- **Cons**: same Planck-scale circularity at the length end; depends on FTD-0015's [SMC] tag

### §4.3 Hadronic-primary

```
a_phys ≡ 1 fm = 10⁻¹⁵ m (confinement scale)
t_tick ≡ √3 · 10⁻¹⁵ / c ≈ 5.78 × 10⁻²⁴ s
K_B ≡ m_p (proton mass anchor) or m_e (status quo)
```

- **Pros**: voxel size matches the smallest direct-measurement scale of QCD; substrate adopts an experimentally-relevant unit
- **Cons**: ultra-short lifetimes (top quark ~10⁻²⁵ s) become sub-tick (infeasible from the *short* end); some absolute lengths become tractable but at the cost of others

### §4.4 Dimensionless-only (the radical option)

```
No dimensional calibration. The framework is purely dimensionless.
```

- **Predictions live entirely in dimensionless ratios**: `α`, `m_μ/m_e`, `m_τ/m_μ`, `Γ_μ/Γ_τ`, etc.
- **Comparison to data**: extract dimensionless ratios from measurement (the SM does this routinely — e.g., `α(M_Z)/α(0)`, `m_t/m_W`); compare to FTD predictions
- **Pros**: removes circularity entirely; matches the falsifiable-spine content; is computationally what the framework already does for tractable predictions (per the calibration feasibility audit)
- **Cons**: absolute predictions ("what is `m_e` in MeV/c²?") become *cross-framework translation questions*, not FTD-internal predictions. Loses the satisfying narrative of "FTD predicts the electron mass is 0.511 MeV"; gains the honest narrative of "FTD predicts the electron-to-Planck mass ratio is 4.18×10⁻²³ (matching measurement to 0.19% per FTD-0015)"
- **Status**: Most consistent with the discrete-native program (FTD-0136); most consistent with the operationalist position (§5); least convenient for SM-bridge popularization

---

## §5 The operationalist position

The position the framework *should* take, articulated:

> The minimum scale is as far as current technology and sensory abilities allow us to go. Epistemically, scale is infinitely small and infinitely big. Physically, it is difficult to say, so it is safe to say it is as small as we can meaningfully measure and will decrease with technological capacity. No need to arbitrarily set it to Planck length.

This is the **operationalist** stance (Bridgman) applied to substrate scale. Three components:

1. **Epistemic openness at both ends**: there is no fundamental upper or lower bound to scale that physics is in a position to assert. Bounds come from *measurement capability*, not from ontology.
2. **Operational definition of "minimum"**: the smallest scale meaningfully discussed is whatever current experiments can probe (currently ~10⁻¹⁹ m at the LHC; possibly ~10⁻²² m with future facilities).
3. **Rejection of arbitrary fundamentality**: declaring `a_phys ≡ ℓ_P` is dressing an operational limitation as a fundamental fact.

This position is *the same operationalist move* the framework already makes at the *outer* end of scale: per `AUDIT_INFINITY_REFRAME` (2026-04-19), FTD uses undefined-boundary lattice ontology, not completed-infinity ℤ³. The lattice has no maximum extent built in; arbitrarily large finite lattices are permitted. The position taken in this document extends the same operationalism to the *inner* end of scale: no minimum extent built in either; arbitrarily small declared physical scales are permitted, the choice is operational.

The two operationalist commitments:

| End | Operationalist position | Existing tag | Where established |
|-----|------------------------|--------------|-------------------|
| Outer (extent) | undefined boundary, not completed ℤ³ | foundational | AUDIT_INFINITY_REFRAME |
| Inner (spacing) | gauge freedom, not derived `ℓ_P` | this doc | this doc |

These are the same move at opposite ends.

---

## §6 Connection to undefined-boundary ontology

The undefined-boundary ontology (`AUDIT_INFINITY_REFRAME`) committed FTD to: *at every specified position, axis-adjacent sites exist, but no totality of all positions is asserted*. Claims of the form "in the L → ∞ limit" are not well-posed without explicit ε-L restatement.

The gauge-freedom position commits to: *one voxel exists as a discrete unit, but its physical scale is not asserted*. Claims of the form "`a_phys = ℓ_P`" are well-posed only as gauge declarations, not derivations.

The two are unified by a single underlying claim:

> **FTD asserts the existence of structure (neighborhood relations, ternary states, integer ticks) without asserting the existence of completed totalities (ℤ³ as a set, `ℓ_P` as a fundamental minimum).**

This is the **constructive** content of FTD's discreteness. The discreteness lives in the *logic* (one of three states; one of two ticks; one of N nearest-neighbors, where each is concretely specified); it does not live in any claim about the metric extent or metric resolution of the substrate.

---

## §7 Implications for existing claims

### §7.1 No tag changes

The 9-theorem algebraic spine is dimensionless throughout. Every existing [THEOREM] tag stands. Every existing [SMC]/[SELECTION]/[PARAMETRIC] tag stands. The dimensional content of any FTD prediction was already calibration-conditional per SPEC_DIMENSIONAL_MAP; this doc does not change that.

### §7.2 FTD-0041 is reframed (not retired)

`a_phys ≡ ℓ_P` and `K_B = m_e` remain valid declarations. They are now correctly understood as **gauge choices for computational and pedagogical convenience**, not as derivations or as fundamental claims about the substrate's structure. SPEC_DIMENSIONAL_MAP's "irreducible minimum" framing is preserved with the clarification that the irreducibility is *informational* (you need at least two anchors to do dimensional translation), not *ontological* (the specific anchors are not derived).

### §7.3 FTD-0130 is dissolved as an ontological decision

FTD-0130 framed path-(a) (cluster-primary) vs path-(b) (Planck-primary) as a deferred ontological decision. Under the gauge-freedom position, the decision is *not ontological*; it is *operational*. Both paths are valid; either may be preferred for specific computational or comparison purposes. The framework should adopt **either both, or pick one for default and document the gauge transformation rules to switch**.

Recommended: keep §4.1 Planck-primary as the default for backward compatibility, document §4.4 dimensionless-only as the canonical form for falsifiable predictions, and treat §4.2 / §4.3 as available transformations.

### §7.4 FTD-0136 (discrete-native program) gains a foundational anchor

The discrete-native program's calibration-feasibility audit (2026-05-04) found that all four observable classes (A/B/C/D) deliver dimensionless ratios within feasible engine runs but absolute physical-scale measurements are 16-25 orders of magnitude beyond reach. Under the gauge-freedom position, this is not a limitation — it is the **operational manifestation of the gauge structure**. The framework's content lives in dimensionless ratios because that is what the framework *has*; absolute scales are convenience and live downstream of declarations.

---

## §8 "Context-driven" and "fractal-like": what each means

### §8.1 Context-driven (defensible, EFT-like)

Different physical contexts (atomic ~Å, hadronic ~fm, gravitational ~km) are dominated by different *cluster-level emergent structures*: different cluster sizes, different cluster-cluster interactions, different stable bound states.

Under gauge freedom: the substrate is uniform and gauge-free; the *observable physics* is multi-scale because different cluster-level emergent structures dominate at different physical scales. This is the standard effective-field-theory picture, recovered discrete-natively without an action-functional construction.

This reading is **defensible** and is consistent with both FTD-0136 and the existing FTD-0110 cluster-mass identification. It also matches the user-articulated position: scale is operational, contextual, and slides with what we are measuring.

### §8.2 Fractal-like (open research question, no current support)

True fractality requires self-similarity across scales. The substrate is uniform — every voxel sees the same 26-Moore neighborhood; there is no self-similar nesting at multiple lengths. FTD-0110 cluster scaling is power-law (`N ∝ A²`), not fractal-recursive.

What *could* be fractal-like: cluster-of-cluster structure (do macroscopic clusters show self-similar internal organization at multiple scales of aggregation?). Not currently shown in FTD. Not currently disproven either. Open research question.

The honest position: **"context-driven" has substrate**; **"fractal-like" is a candidate research direction without current evidence**. Both should be distinguished in framework discussions to avoid the F1 risk of validating an aesthetically-attractive intuition before it has structural support.

---

## §9 Recommended canonical position

For FTD-internal use:

1. **Lattice spacing is gauge.** No fundamental physical scale is asserted by the framework.
2. **The falsifiable spine is dimensionless.** The 9 algebraic-spine theorems + the 4 dimensionless physical identifications + the dimensionless ratios extracted from engine measurements are the framework's content.
3. **Dimensional calibrations are convenience.** They enable cross-translation with SI-quoted measurements but are not derivations and not load-bearing for FTD's own falsifiability surface.
4. **The default gauge is Planck-primary** (§4.1) for backward compatibility with existing documentation, but the dimensionless-only gauge (§4.4) is the canonical form for falsifiable predictions.
5. **Different operational contexts may use different gauges.** Switching gauges is a documented transformation; the dimensionless content is invariant.

For external presentation:

- Lead with dimensionless predictions (`α = 1/137.036`, `m_μ/m_e ≈ 206.77`, etc.)
- Present dimensional values (`m_e ≈ 0.511 MeV/c²`) as derived from a stated calibration choice
- Do not claim the framework "predicts the Planck length" or "operates at the Planck scale" without the qualifier *under the Planck-primary calibration declaration*

---

## §10 Open questions

1. **Is there a privileged gauge after all?** Possibly via a future derivation that specifies a physical scale from FTD-internal axioms. FTD-0059 + FTD-0096 closed the four candidates known to date; new mechanisms are conceivable but not currently outlined.

2. **Does the cluster-of-cluster question (§8.2) admit a clean test in the engine?** Phase D (cluster spectrum) infrastructure when built may incidentally surface evidence for or against multi-scale recursion in cluster organization.

3. **Is "scale-invariance of the falsifiable spine" itself a derivable property?** The dimensionless-spine content (`α`, mass ratios) is currently observed to be scale-invariant by inspection; whether the framework structurally *forces* scale invariance of these quantities under arbitrary gauge choice is a candidate theorem worth investigating.

4. **Does the operationalist position interact with the algebraic spine's dependence on `Γ(1/4)`?** `G* = Γ(1/4)/Γ(3/4)` is dimensionless, so unchanged under gauge transformation. The CM-curve uniqueness scan (FTD-0001) is also dimensionless. The spine's content is *intrinsically* dimensionless, which strengthens rather than weakens the gauge-freedom reading.

---

## §11 Cross-references

- **FTD-0041** (calibration declaration; reframed as gauge choice rather than derivation)
- **FTD-0059** (length-calibration no-go theorem; closed four mechanism candidates)
- **FTD-0096** (mass-calibration no-go theorem; closed four mechanism candidates)
- **FTD-0130** (calibration architecture audit; ontological-decision framing dissolved as operational gauge choice per this doc)
- **FTD-0136** (discrete-native derivation program; gains foundational anchor)
- **FTD-0137** (LEDGER row recording this clarification)
- **AUDIT_INFINITY_REFRAME** (the parallel operationalist position at the *outer* end of scale)
- **SPEC_DIMENSIONAL_MAP** (three-layer structure: dimensionless / calibration / dimensional; this doc clarifies the calibration layer is gauge)
- **SPEC_ALGEBRAIC_SPINE** (intrinsically-dimensionless content, unchanged by this clarification)
- **CATALOG_PARAMETRIC_INSERTIONS** (every dimensional FTD prediction is calibration-conditional; this doc strengthens the case)

---

**Authoring note (per CLAUDE.md F1/F9 + GTCA F9):** the clarification is structurally clean and aligns with the framework's existing operationalist commitments at the outer end of scale. The F9 risk is treating gauge freedom as license for sloppy dimensional reasoning ("any value of `a_phys` is fine, so don't bother declaring one"). The discipline that prevents this is §7.2 (calibrations remain valid declarations needed for cross-translation) and §9.4 (gauge declarations remain documented; switching gauges is a documented transformation, not a free-for-all). The dimensionless-only canonical form (§4.4) is the most epistemically honest position; the Planck-primary default (§4.1) is retained as a backward-compatibility convenience, not as a privileged gauge.
