# The SR / Gravity / GR Trichotomy

## Why Special Relativity, Gravity, and General Relativity Are Three Distinct Concepts in FTD

**Document Version:** 2.0
**Framework Version:** FTD v5.28
**Date:** March 17, 2026 (updated from February 20, 2026)
**Standard:** Semantic disambiguation with epistemic classification

**Depends on:**
- [DERIV_RELATIVITY_DERIVATION.md](../03_derivations/DERIV_RELATIVITY_DERIVATION.md) — SR derivation (Part I) and g₀₀ from flux saturation (Part II, Theorem 11.1)
- [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) — Combined proper time formula, velocity cost amplification
- [DERIV_FORCE_EMERGENCE.md](../03_derivations/DERIV_FORCE_EMERGENCE.md) — Newtonian gravity as weak-field limit
- [DERIV_QFT_GRT_BRIDGE.md](../03_derivations/DERIV_QFT_GRT_BRIDGE.md) — Linearized GR (propagators, wave equations)
- [SPEC_QFT_GRT_BRIDGE_ROADMAP.md](../01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md) — Gap analysis (GAP-G2, GAP-G4, GAP-G5)

---

## Abstract

Standard physics treats Special Relativity and General Relativity as a two-level hierarchy: SR describes flat spacetime; GR extends SR by identifying gravity with spacetime curvature. Einstein's core insight was that gravity **is** curvature — they are the same thing.

FTD inverts this identification. In FTD, gravity is **not** curvature — it is computational budget saturation. Curvature is an emergent mathematical description of saturation patterns. This produces a **three-level hierarchy** where standard physics has two:

```
Standard:  SR (flat spacetime)  →  GR (curved spacetime = gravity)
FTD:       SR (C=1 kinematics)  →  Gravity (saturation)  →  GR (emergent geometry)
```

The middle layer — **gravity without curvature** — is FTD's novel structural contribution. This document makes the trichotomy explicit, defines precise terminology, and identifies where existing FTD documents conflate these three distinct concepts.

---

## Preface: Epistemic Framework

| Tag | Meaning | Standard |
|-----|---------|----------|
| **[AXIOM]** | Primitive FTD postulate | Cannot be derived; foundational |
| **[DEFINITION]** | Formal naming | No truth claim; establishes notation |
| **[THEOREM]** | Rigorously proven | Complete derivation from prior results |
| **[SELECTION]** | Argued choice | Not unique; justified by criteria |
| **[CONJECTURE]** | Unproven claim | Evidence but no proof |
| **[GAP]** | Missing derivation | Acknowledged; future work |

---

## §1. The Conflation Problem

### 1.1 Two Concepts vs Three

In standard physics, Einstein unified gravity and geometry:

| Standard Physics | What it means |
|-----------------|---------------|
| **Special Relativity** | Physics in flat spacetime (no gravity) |
| **General Relativity** | Physics in curved spacetime **= gravity** |

Einstein's equivalence principle makes this a **binary**: either spacetime is flat (no gravity, SR applies) or curved (gravity present, GR applies). There is no "gravity without curvature."

FTD breaks this binary. The framework produces three ontologically distinct concepts:

| FTD Concept | What it is | What it is NOT |
|-------------|-----------|----------------|
| **Special Relativity** | Kinematics from C = 1 speed limit | Not about gravity or mass |
| **Gravity** | Computational budget saturation from mass | Not curvature; not a force law |
| **General Relativity** | Mathematical encoding of the saturation pattern as a metric tensor | Not the source of gravity; emergent, not fundamental |

### 1.2 Why This Matters

The distinction is not merely terminological. It determines:

1. **What FTD has actually derived** — SR and gravity are [THEOREM]; full GR is largely [GAP]
2. **What is novel** — "gravity without curvature" is FTD's unique structural claim
3. **What the equivalence principle means** — emergent approximation, not fundamental postulate
4. **Where the open problems are** — full nonlinear GR, diffeomorphism invariance, background independence

---

## §2. Special Relativity in FTD

### 2.1 Origin [AXIOM → THEOREM]

SR derives entirely from **POSTULATE 4** (Local Causality): updates to voxel $v$ at tick $t$ depend only on $v$ and its 26 neighbors at tick $t-1$.

**Consequence:** $C = 1$ voxel/tick — maximum propagation speed.

From this single axiom, the following are **complete theorems** (proven in [DERIV_RELATIVITY_DERIVATION.md](../03_derivations/DERIV_RELATIVITY_DERIVATION.md) Part I):

| Result | Formula | Status |
|--------|---------|--------|
| Time dilation | $d\tau/dT = \sqrt{1 - v^2}$ | [THEOREM] |
| Length contraction | $L = L_0 \sqrt{1 - v^2}$ | [THEOREM] |
| Relativity of simultaneity | $\Delta t' = -\gamma v \Delta x / c^2$ | [THEOREM] |
| Lorentz transformations | $x' = \gamma(x - vt)$, $t' = \gamma(t - vx/c^2)$ | [THEOREM] |
| Minkowski metric | $ds^2 = dt^2 - dx^2 - dy^2 - dz^2$ | [THEOREM] |
| Energy-momentum relation | $E^2 = p^2 + m^2$ | [THEOREM] |

### 2.2 What SR Does NOT Require

SR in FTD is **self-contained**. It requires:

- No mass
- No gravity
- No curvature
- No metric tensor beyond Minkowski (which is just notation for C = 1)
- No computational budget metaphor (beyond the raw speed limit)

The Minkowski metric $\eta_{\mu\nu} = \text{diag}(+1, -1, -1, -1)$ is not "geometry" in any deep sense — it is the **invariant structure** of the wave equation $\partial_t^2 J = c^2 \nabla^2 J$, which follows directly from the axiom.

### 2.3 Computational Budget Interpretation [SELECTION]

From [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) §1-2: each lattice node receives one unit of computational budget per Universal Tick. This budget is distributed between:

- **Spatial translation**: moving information across lattice boundaries (costs $v^2$)
- **Internal state update**: evolving local degrees of freedom (costs $1 - v^2$; this is proper time)

The Pythagorean structure $(\text{temporal cost})^2 + (\text{spatial cost})^2 = 1$ is a restatement of $ds^2 = dt^2 - dx^2$. This interpretation adds no new physics — it provides computational language for the same theorem.

---

## §3. Gravity in FTD

### 3.1 Origin [THEOREM + SELECTION]

Gravity arises from a second, independent mechanism: **mass creates information density that saturates lattice nodes**.

Near a mass $M$, lattice nodes carry more information (gravitational field data, flux density). This consumes part of each node's computational budget before any spatial or temporal processes begin.

**The availability factor** (from Theorem 11.1 in [DERIV_RELATIVITY_DERIVATION.md](../03_derivations/DERIV_RELATIVITY_DERIVATION.md)):

$$f(r) = 1 - \frac{\rho_{\text{info}}}{\rho_{\text{max}}} = 1 - \frac{r_s}{r}$$

where $r_s = 2GM/c^2$ is the Schwarzschild radius.

### 3.2 Gravity Is a Scalar [SELECTION]

The availability factor $f$ is a **scalar field** — a single number at each point. You do not need a metric tensor, a Riemann curvature tensor, or any geometric object to state:

> "The fraction of computational capacity remaining at position $r$ from mass $M$ is $f(r) = 1 - r_s/r$."

This is the novel claim: **gravity exists as a resource constraint before any geometric language is invoked.** Objects "fall" because they follow the saturation gradient — they drift toward regions where proper time runs slower, driven by the inhomogeneous computational budget.

### 3.3 Static Gravitational Time Dilation [THEOREM]

For a static observer ($v = 0$) at position $r$:

$$\frac{d\tau}{dT_U} = \sqrt{f} = \sqrt{1 - \frac{r_s}{r}}$$

This is gravitational time dilation — derived from flux saturation, not from spacetime curvature.

### 3.4 What Gravity Is NOT in FTD

| Common identification | FTD status |
|----------------------|------------|
| Gravity = curvature | **No.** Curvature is emergent description, not source |
| Gravity = force ($F = GMm/r^2$) | **Partially.** Newtonian force is the weak-field *limit*, not the fundamental mechanism |
| Gravity = geometry ($g_{\mu\nu}$) | **No.** The metric *encodes* the saturation pattern but is not its cause |
| Gravity = saturation | **Yes.** This is FTD's identification |

---

## §4. General Relativity as Emergent Language

### 4.1 When Does GR Enter? [THEOREM]

GR enters FTD **only** when you ask what happens when SR and gravity interact — when an observer is both *moving* and *in a gravitational field*.

If SR and gravity were independent, the proper time formula would be:

$$\frac{d\tau}{dT_U} \stackrel{?}{=} \sqrt{f - v^2} \qquad \textbf{[NAIVE — WRONG]}$$

The actual formula (Theorem 6.1 from [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md)) is:

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v^2}{f}} \qquad \textbf{[CORRECT]}$$

The difference — $v^2/f$ instead of $v^2$ — is the **fingerprint of curved geometry**. Moving through gravitationally saturated nodes costs *more* per unit displacement: the velocity cost is amplified by $1/f$. This non-trivial coupling between motion and gravity **forces** the introduction of a metric tensor to track how spatial and temporal costs relate at each point.

### 4.2 The Schwarzschild Metric [THEOREM]

The combined formula is equivalent to the Schwarzschild line element:

$$ds^2 = f \, dt^2 - \frac{dr^2}{f} - r^2 \, d\Omega^2$$

| Component | Value | Origin | Tag |
|-----------|-------|--------|-----|
| $g_{tt} = f$ | $1 - r_s/r$ | Flux saturation (gravity) | [THEOREM] |
| $g_{rr} = -1/f$ | $-1/(1 - r_s/r)$ | Velocity cost amplification | [THEOREM + SELECTION] |
| $g_{\theta\theta} = -r^2$ | Area-radius relation | Coordinate choice | [DEFINITION] |

The metric is GR's mathematical language for encoding the saturation pattern. The physics (gravity = saturation) comes first; the mathematics (metric tensor) comes second.

### 4.3 What FTD Has Derived vs What Remains Open

| Level | Status | Source |
|-------|--------|--------|
| Schwarzschild metric | **[THEOREM]** | DERIV_LATTICE_SCHWARZSCHILD |
| Weak-field geodesics | **[THEOREM]** | DERIV_RELATIVITY_DERIVATION §12 |
| Linearized Einstein equations | **[THEOREM]** | DERIV_QFT_GRT_BRIDGE |
| Gravitational waves (linearized) | **[THEOREM]** | DERIV_RELATIVITY_DERIVATION §15 |
| Full nonlinear Einstein equations | **[GAP-G2]** | Not derived |
| Diffeomorphism invariance | **[GAP-G4]** | Broken by fixed lattice |
| Background independence | **[GAP-G5]** | Broken by fixed lattice |

### 4.4 Structural Limits of GR in FTD [SELECTION]

Full GR (nonlinear Einstein equations, diffeomorphism invariance, background independence) may be **structurally unachievable** on a fixed cubic lattice. The lattice defines a preferred frame — it is background-*dependent* by construction.

This is not necessarily a defect. FTD proposes that the lattice is more fundamental than the geometry it produces. Diffeomorphism invariance would be an emergent symmetry at arbitrarily fine spacing, not a property of the substrate. Whether this emergence actually occurs is **[OPEN]**.

---

## §5. The Equivalence Principle

### 5.1 What the EP Says

Einstein's Equivalence Principle: in a small enough region of spacetime, the effects of gravity are indistinguishable from acceleration. Locally, a freely falling observer experiences no gravitational effects.

### 5.2 Why the EP Works in FTD [SELECTION]

Both SR time dilation and gravitational time dilation reduce the **same computational budget**:

| Mechanism | What consumes the budget | Formula |
|-----------|------------------------|---------|
| Motion (SR) | Spatial translation across lattice boundaries | $v^2$ |
| Gravity | Local information density from mass | $1 - f = r_s/r$ |

An embedded observer cannot distinguish "my budget is reduced because I'm moving fast" from "my budget is reduced because I'm in a gravity well" — because in both cases, the *observable consequence* is the same: proper time runs slower.

### 5.3 Why the EP Is Approximate, Not Fundamental [SELECTION]

At the substrate level, the mechanisms are **ontologically distinct**:

- SR time dilation: budget consumed by *spatial translation* across lattice boundaries — a **kinematic** effect
- Gravitational time dilation: budget consumed by *local information density* at each node — a **thermodynamic/informational** effect

The EP is exact in the *weak-field, small-region limit* where $f \approx 1$ and the velocity-gravity coupling ($v^2/f \approx v^2$) is negligible. It becomes approximate in strong fields where the $1/f$ amplification distinguishes the two mechanisms.

### 5.4 Analogy

A computer running slowly because it's doing heavy I/O (data transfer = motion) vs. running slowly because its CPU is thermally throttled (local heat = gravity). Same symptom (slow clock), different cause. Locally indistinguishable, but the underlying mechanisms are distinct — and in extreme conditions (CPU near meltdown), the distinction matters.

---

## §6. The Combined Formula Decoded

### 6.1 Three Components, Not Two [THEOREM]

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v^2}{f}}$$

| Term | Physical meaning | Source concept |
|------|-----------------|----------------|
| $f = 1 - r_s/r$ | Budget reduced by mass | **Gravity** (saturation) |
| $v^2$ | Budget consumed by motion | **SR** (kinematics) |
| $1/f$ amplification | Motion costs more in saturated regions | **GR** (geometry) |

The three terms correspond to the three distinct concepts. If any one is absent:

| Condition | Formula | What you need |
|-----------|---------|---------------|
| No gravity ($f = 1$) | $\sqrt{1 - v^2}$ | SR only (Lorentz factor) |
| No motion ($v = 0$) | $\sqrt{f}$ | Gravity only (scalar field) |
| Both present | $\sqrt{f - v^2/f}$ | Full metric (GR language required) |

### 6.2 The Naive Formula and Its Domain [THEOREM]

The naive formula $\sqrt{f - v^2}$ (SR and gravity independently subtracting from the budget) is valid in the weak-field limit. Expanding for $f = 1 - \epsilon$ with $\epsilon \ll 1$:

$$f - \frac{v^2}{f} \approx f - v^2(1 + \epsilon) \approx f - v^2 + O(\epsilon \cdot v^2)$$

The correction is $O(\epsilon \cdot v^2)$ — negligible for GPS satellites ($\epsilon \sim 10^{-10}$), solar system physics ($\epsilon \sim 10^{-6}$), and essentially all practical applications. The full formula matters only near compact objects (neutron stars, black holes) where $f$ departs significantly from 1.

### 6.3 Budget Conservation [THEOREM]

The relationship $g_{tt} \cdot g_{rr} = f \cdot (-1/f) = -1$ expresses a conservation law: **gravity cannot create or destroy computational budget — only redistribute it between temporal and spatial channels.** Where time runs slow ($g_{tt}$ small), space is expensive to traverse ($|g_{rr}|$ large). The total "difficulty" of spacetime is conserved.

---

## §7. The Seven-Level Hierarchy

### 7.1 Complete Hierarchy [DEFINITION]

```
Level 0:  C = 1 (axiom)                                               [AXIOM]
            ↓
Level 1:  SPECIAL RELATIVITY                                          [THEOREM]
            dτ/dT = √(1 - v²) — kinematics in flat lattice
            ↓
Level 2:  GRAVITY                                                      [THEOREM + SELECTION]
            f(r) = 1 - r_s/r — computational saturation from mass
            ↓
Level 3:  COMBINED PROPER TIME                                        [THEOREM]
            dτ/dT = √(f - v²/f) — non-trivial SR-gravity coupling
            ↓
Level 4:  METRIC DESCRIPTION                                          [THEOREM]
            g_μν = diag(f, -1/f, -r², -r²sin²θ) — Schwarzschild
            ↓
Level 5:  LINEARIZED GR                                               [THEOREM]
            □h_μν = -16πG T_μν — wave equations, propagators
            ↓
Level 6:  FULL NONLINEAR GR                                           [GAP-G2]
            R_μν - ½g_μν R = 8πG T_μν — not derived
            ↓
Level 7:  GRT (background independent, diffeomorphism invariant)      [GAP-G4, GAP-G5]
            NOT achievable on fixed cubic lattice
```

### 7.2 The Boundary

FTD has complete results through **Level 5**. Levels 6-7 represent genuine open problems, not temporary gaps — the fixed cubic lattice may be structurally incompatible with background independence.

### 7.3 Key Observation

**Levels 1 and 2 are independent physical inputs.** SR requires only C = 1; gravity requires only mass-induced saturation. Neither implies the other.

**Level 3 onward are mathematical consequences** of combining these two independent inputs. The metric tensor, Einstein equations, and geometric language are tools for describing how SR and gravity *interact* — they are not independent physical phenomena.

---

## §8. Terminology Recommendations

### 8.1 Precise Definitions for FTD Usage

| Term | Precise FTD meaning | Scope |
|------|---------------------|-------|
| **SR** / **Special Relativity** | Consequences of C = 1 in flat (unloaded) lattice | Kinematics only; no mass or gravity |
| **Gravity** | Computational budget saturation from mass-induced information density | Physical phenomenon; independent of any mathematical description |
| **Newtonian gravity** | Weak-field, low-velocity limit: $F = G_N \cdot \nabla\bar\rho$ | Mathematical approximation, not fundamental mechanism |
| **GR** / **General Relativity** | Einstein field equations as effective description of saturation patterns | Mathematical framework; linearized = [THEOREM], nonlinear = [GAP] |
| **GRT** | Full General Relativity Theory including diffeomorphism invariance and background independence | NOT achievable on fixed lattice |

### 8.2 Conflation Sites in Existing Documents

| Document | Issue |
|----------|-------|
| CLAUDE.md §6.2 | "Gravity-Like Behavior" uses Newtonian force form — conflates gravity-as-force with gravity-as-saturation |
| CLAUDE.md §14.1 | "No general relativistic curvature (fixed flat lattice)" contradicts later claims that GR is derived |
| DERIV_RELATIVITY_DERIVATION.md Part II | Labeled "General Relativity" but §9-11 derive **gravity** (saturation → g₀₀), not full GR |
| DERIV_FORCE_EMERGENCE.md | Treats gravity as $F = G_N \cdot \nabla\bar\rho$ — this is Newtonian gravity (weak-field limit), not gravity-as-saturation |

### 8.3 Documents Using Correct Terminology

| Document | What it gets right |
|----------|-------------------|
| DERIV_LATTICE_SCHWARZSCHILD.md | Separates SR (Part I) from gravitational extension (Part II); uses "computational budget" language consistently |
| SPEC_QFT_GRT_BRIDGE_ROADMAP.md | Distinguishes "GRT" (full theory) from partial results; identifies GAP-G2, GAP-G4, GAP-G5 |
| EXPLR_COLLAPSE_GRAVITY_BRIDGE.md | Uses "gravity" to mean computational saturation throughout |
| DERIV_QFT_GRT_BRIDGE.md | Uses "GRT" precisely for the full geometric theory |

---

## §9. Claims Table

### 9.1 Theorems

| ID | Claim | Epistemic Tag | Depends On |
|----|-------|---------------|------------|
| RGD-T1 | SR derives completely from C = 1 (POSTULATE 4), independent of gravity | [THEOREM] | POSTULATE 4 |
| RGD-T2 | Gravity derives from flux saturation as scalar field $f(r) = 1 - r_s/r$, independent of any metric tensor | [THEOREM] | Theorem 11.1 (DERIV_RELATIVITY_DERIVATION) |
| RGD-T3 | The combined formula $d\tau/dT = \sqrt{f - v^2/f}$ encodes the Schwarzschild metric | [THEOREM] | Theorem 6.1 (DERIV_LATTICE_SCHWARZSCHILD) |
| RGD-T4 | The naive formula $\sqrt{f - v^2}$ agrees with the correct formula to $O(\epsilon \cdot v^2)$ in weak fields | [THEOREM] | Algebraic expansion |

### 9.2 Selections

| ID | Claim | Epistemic Tag | Depends On |
|----|-------|---------------|------------|
| RGD-S1 | Gravity is ontologically a resource constraint (computational saturation), not spacetime curvature | [SELECTION] | Lattice computational budget interpretation |
| RGD-S2 | The equivalence principle is emergent from shared budget, not a fundamental postulate | [SELECTION] | Budget interpretation of SR and gravity |
| RGD-S3 | The metric tensor is a mathematical encoding of saturation patterns, not the source of gravity | [SELECTION] | Ontological ordering: saturation → metric |

### 9.3 Conjectures

| ID | Claim | Epistemic Tag | Depends On |
|----|-------|---------------|------------|
| RGD-C1 | Full nonlinear GR may be structurally unachievable on a fixed cubic lattice | [CONJECTURE] | Background dependence of lattice |
| RGD-C2 | Diffeomorphism invariance, if it emerges, is a property at arbitrarily fine spacing, not the substrate | [CONJECTURE] | Substrate vs aggregate distinction |

### 9.4 Open Questions

| ID | Question | Status |
|----|----------|--------|
| RGD-O1 | Can full nonlinear Einstein equations be recovered from lattice dynamics? | [GAP-G2] |
| RGD-O2 | Does diffeomorphism invariance emerge at arbitrarily fine spacing? | [GAP-G4] |
| RGD-O3 | Is background independence achievable in any formulation of FTD? | [GAP-G5] |
| RGD-O4 | Can the equivalence principle be tested for deviations in the strong-field regime? | [OPEN] |

---

## §10. The Epistemic Time Dimension

### 10.1 Three Concepts, Three Temporal Relations [SELECTION]

The SR / Gravity / GR trichotomy is not merely about spatial relations — each concept embodies a fundamentally different relationship to **epistemic time**: how an observer knows and experiences temporal progression.

| Concept | Spatial Relation | Epistemic Time Relation | What the Observer Knows |
|---------|-----------------|------------------------|------------------------|
| **SR** | Relative motion between lattice regions | **Kinematic time**: proper time $d\tau = \sqrt{1 - v^2}\,dT_U$ depends on motion alone | The observer's clock rate is determined by how fast they traverse the lattice |
| **Gravity** | Local information density at a position | **Thermodynamic time**: proper time $d\tau = \sqrt{f}\,dT_U$ depends on ambient energy density | The observer's clock rate is determined by how much the local lattice is saturated |
| **GR** | Both motion and density simultaneously | **Geometric time**: proper time $d\tau = \sqrt{f - v^2/f}\,dT_U$ emerges from the non-trivial coupling | The observer's experience is shaped by the interplay of motion and environment — neither alone suffices |

### 10.2 Epistemic vs Ontological Time [SELECTION]

FTD distinguishes two fundamentally different time concepts:

**Ontological time** (the Universal Tick $T_U$): Absolute, discrete, the same everywhere. Every voxel updates once per tick. This is the lattice's internal clock — it never varies, never bends, never slows down. It is not observable.

**Epistemic time** (proper time $\tau$): What an observer embedded in the lattice actually experiences. This depends on the observer's spatial relation (motion) and informational environment (gravity). Epistemic time is the only time any observer can measure.

The three concepts of the trichotomy correspond to three **sources of epistemic time deviation** from the ontological tick:

| Source | Mechanism | Formula |
|--------|-----------|---------|
| **SR** | Spatial traversal consumes tick budget | $\Delta\tau/\Delta T_U = \sqrt{1 - v^2}$ |
| **Gravity** | Information density pre-consumes tick budget | $\Delta\tau/\Delta T_U = \sqrt{1 - r_s/r}$ |
| **GR** | Both effects amplify each other non-linearly | $\Delta\tau/\Delta T_U = \sqrt{f - v^2/f}$ |

### 10.3 Why Time Is Central to the Distinction [SELECTION]

Standard physics unifies SR and gravity into GR by treating them as aspects of spacetime geometry. In this view, time and space are on equal footing (Minkowski signature).

FTD inverts this: **time is more fundamental than geometry**. The Universal Tick exists at Level 0; spatial relations exist at Level 1; gravity at Level 2; geometry at Level 4. Time *precedes* space in the ontological hierarchy.

The three concepts are therefore three different **perturbations of the observer's temporal experience**:

1. **SR**: A *kinematic* perturbation — moving through space costs time
2. **Gravity**: A *thermodynamic* perturbation — being near mass costs time (the lattice is "busy" processing gravitational information)
3. **GR**: An *emergent geometric* encoding of how these two perturbations couple

This is why they are "separate but similar" — all three affect the same observable (proper time), but through ontologically distinct mechanisms that happen to produce the same phenomenological signature (clock slowdown). The equivalence principle is the statement that locally, the observer cannot distinguish which mechanism is responsible.

### 10.4 Connection to Consciousness and Measurement [CONJECTURE]

The discriminant trichotomy ($\Delta > 0$, $\Delta = 0$, $\Delta < 0$) may extend this temporal hierarchy further:

| Domain | $\Delta$ | Temporal Character |
|--------|----------|-------------------|
| Physics (real roots) | $\Delta > 0$ | Oscillatory — reversible dynamics |
| Measurement (degenerate) | $\Delta = 0$ | Critical — the "now" of collapse |
| Consciousness (complex roots) | $\Delta < 0$ | Exponential — irreversible experience |

If correct, the three temporal modes (kinematic, thermodynamic, geometric) of the gravity trichotomy would nest inside the three temporal domains (oscillatory, critical, experiential) of the discriminant trichotomy. This remains [CONJECTURE] — the connection between gravitational time dilation and the measurement-consciousness boundary has not been rigorously established.

---

## Cross-References

| Document | Relationship |
|----------|-------------|
| [DERIV_RELATIVITY_DERIVATION.md](../03_derivations/DERIV_RELATIVITY_DERIVATION.md) | SR derivation (Part I); gravity/g₀₀ (Part II §9-11) |
| [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) | Combined proper time formula; velocity cost amplification; budget conservation |
| [DERIV_FORCE_EMERGENCE.md](../03_derivations/DERIV_FORCE_EMERGENCE.md) | Newtonian gravity as Green's function (weak-field limit) |
| [DERIV_QFT_GRT_BRIDGE.md](../03_derivations/DERIV_QFT_GRT_BRIDGE.md) | Linearized GR; graviton propagator; gravitational wave equations |
| [SPEC_QFT_GRT_BRIDGE_ROADMAP.md](../01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md) | Full GRT gap analysis (GAP-G2, GAP-G4, GAP-G5) |
| [EXPLR_COLLAPSE_GRAVITY_BRIDGE.md](../09_mathematical/EXPLR_COLLAPSE_GRAVITY_BRIDGE.md) | Gravity as spatial crystallization; Hawking-KMS bridge |

---

*Document created: February 20, 2026 | Updated: March 17, 2026 (v2.0 — added §10 epistemic time dimension)*
*Framework: Foundational Ternary Dynamics v5.28*
*Classification: Foundational (FOUND_)*
