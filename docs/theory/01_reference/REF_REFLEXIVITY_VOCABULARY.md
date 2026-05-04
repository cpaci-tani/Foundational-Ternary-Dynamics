# REF_REFLEXIVITY_VOCABULARY — Canonical FTD Language for Reflexivity & Agency

**Created:** 2026-05-01
**Status:** Proposed canonical vocabulary; pending application sweep across theory docs, manuscripts, and engine documentation.
**Purpose:** Replace "consciousness" / "observer" / "subjective experience" with terms that carry mathematical precision and zero qualia / Hard-Problem baggage, without sacrificing any conceptual content the existing FTD literature has earned.
**Audience:** Project owner + future AI agents performing the propagation sweep + reviewers of FTD papers.

---

## 1 · Why this document exists

FTD's literature uses "consciousness" in three structurally distinct ways that the term itself cannot disambiguate:

| Existing usage | What it actually points at |
|---|---|
| "Consciousness as integration over the BCC interior axis" (FTD-0078) | A specific mathematical operator (a projection from the 3³ noumenal block to the 2³ phenomenal block) |
| "Conscious observer" in measurement contexts | An observation layer — a subalgebra of physical observables — without commitment to who's doing the observing |
| "Conscious agent" in agency / free-will contexts | A dynamical system whose trajectory is selected by its own internal state in a way external dynamics doesn't predict |

These are three different concepts. Calling all three "consciousness" produces three problems:

1. **The Hard Problem of Consciousness** auto-attaches to every claim, making the framework appear to claim more than it does.
2. **Physics reviewers reflexively discount** any framework using "consciousness" as a load-bearing term.
3. **Internal precision is lost** — claims that are mathematically distinct become rhetorically merged.

This vocabulary establishes terms that disambiguate the three usages and let the framework state exactly what it claims, no more.

---

## 2 · The core distinction

**Reflexivity** is the **structural property** of containing a model of oneself.
**Agency** is the **dynamical manifestation** of reflexivity feeding back into trajectory selection.

These are different things. A static reflexive structure (e.g., a Gödel sentence) has reflexivity but no agency. A driven non-reflexive system (e.g., a thermostat) has externally-induced behavior but no agency in the sense FTD means. The interesting case is the conjunction: **reflexive structures that exhibit agentic dynamics** — these are what FTD's literature has been calling "conscious."

> **Slogan:** *Reflexivity is the structural prerequisite for agency. Agency is reflexivity acting on itself in time.*

---

## 3 · Primary vocabulary

### Reflexivity (structural)

| Term | Type | Meaning |
|---|---|---|
| **Reflexivity** | noun | The property of a structure that contains a model of itself — a fixed-point / Y-combinator / strange-loop property. |
| **Reflexive** | adj. | Possessing reflexivity. *"A reflexive site," "a reflexive projection."* |
| **Reflexive site** | noun | A voxel or local region of the lattice that instantiates reflexivity. In FTD, the canonical example is the center of a 27-block (Moore-26 + center). |
| **Reflexive structure** | noun | The global organization of a configuration that endows it with reflexivity. The 3³ block + Moore-26 neighborhood is the smallest reflexive structure on the FTD lattice. |
| **Reflexive projection** | noun | The operator that maps a reflexive structure's full state to its self-readout. In FTD: the noumenal → phenomenal projection (FTD-0078, restated). Corresponds to **observable-algebra restriction** in algebraic-QFT terms. |
| **Reflexive readout** | noun | The output of a reflexive projection — what the structure "knows about itself." Replaces the loaded term *experience*. |
| **Reflexive eigenmode** | noun | An eigenstate of the reflexive projection. The hypothesis that the master quadratic spectrum (x_+ ≈ 137, x_- ≈ 3) consists of reflexive eigenmodes is what MC-T4.3 closure would establish. |
| **Reflexive integration** | noun | The dynamical process by which a reflexive structure updates its self-readout over time. Replaces *integration over the BCC interior axis* with the same mathematical content but cleaner framing. |
| **Reflexive site / non-reflexive site** | distinction | A site is reflexive iff it lies at the center of a 27-block whose Moore-26 neighborhood encodes a model of the center's state. Most lattice sites are reflexive in this sense — reflexivity is generic at the lattice geometry level; the question is which reflexive structures dynamically *realize* their reflexivity. |

### Agency (dynamical)

| Term | Type | Meaning |
|---|---|---|
| **Agency** | noun | The dynamical capacity of a system to select its own trajectory in a way external dynamics doesn't predict. |
| **Agentic** | adj. | Exhibiting agency. *"Agentic dynamics," "an agentic configuration."* |
| **Agentic causation** | noun | Causation by a system's reflexive structure, as distinct from environmental causation. The thing the user describes as "in addition to decoherence." |
| **Trajectory selection** | noun | The act of an agentic system choosing among dynamically-allowed futures. Replaces loaded terms *will*, *choice*, *decision*. |
| **Endogenous causation** | noun | Causation originating from inside the system rather than from boundary conditions or external drives. **Endogenous** is the FTD-canonical adjective for this. |
| **Endogenous trajectory** | noun | A future path selected by endogenous causation. |

### Reflexivity ↔ Agency relationship

| Term | Meaning |
|---|---|
| **Reflexive agency** | The conjunction: agentic dynamics whose trajectory selection is driven by the system's own reflexive readout. *This is what FTD's literature has been calling "consciousness."* |
| **Reflexive emergence** | The process by which non-reflexive components organize into a reflexive structure. **Replaces *emergence of consciousness* / *Hard Problem*** with a structural question that has a research program. |
| **Agentic threshold** | The configuration-space boundary above which a reflexive structure realizes agency in its dynamics. Open structural problem; the lattice analogue of the wakefulness / unconsciousness distinction in neuroscience. |

---

## 4 · Vocabulary that gets replaced

| Old term | New term | Reason |
|---|---|---|
| Consciousness | reflexive agency *(or:)* reflexivity + agency *(when distinguishing the two aspects)* | Old term is structurally ambiguous and qualia-loaded. |
| Conscious observer | reflexive site *(if structural)* / observation layer *(if measurement-context)* / agentic system *(if behavioral)* | "Observer" conflates three things; pick one. |
| Subjective experience | reflexive readout | Strips qualia commitment; preserves the "system's view of itself" content. |
| Subjective | reflexive *(if structural)* / endogenous *(if causal)* / first-person *(if perspectival)* | Three distinct concepts; pick one. |
| Awareness | reflexive readout *(weak)* / reflexive integration *(strong/dynamical)* | Same disambiguation as "consciousness." |
| Mind | reflexive process *(or just delete)* | Almost always vague; usually replaceable by something more specific. |
| Sentience | reflexive readout *(if structural)* / agency *(if behavioral)* | See earlier discussion; sentience is too tied to the qualia debate. |
| Sapience | (avoid) | Implies a wisdom hierarchy not load-bearing in physics. |
| Self | reflexive self-model *(structural)* / self-referent *(linguistic)* | "Self" alone is too vague. |
| Free will | agency *(or:)* endogenous trajectory selection | Same content; physics-compatible framing. |
| Will | agency / trajectory selection | Same. |
| Intentionality | directedness *(if simple)* / endogenous teleonomy *(if goal-structured)* | "Intentionality" is a contested philosophical term; pick the precise meaning. |
| Qualia | (avoid; do not replace) | Don't introduce; don't translate. The qualia debate is exactly what this vocabulary skips. |
| Phenomenology | endogenous structure *(structural)* / phenomenal-layer dynamics *(in FTD's two-layer ontology)* | Disambiguates philosophical-school usage from FTD's specific meaning. |
| The Hard Problem | open problem of reflexive emergence | Reframes from "intractable mystery" to "open structural problem." |
| Mind-body problem | reflexive-emergence problem | Same. |
| Observer effect / observer-dependence | reflexive coupling *(structural)* / observation-layer effect *(measurement-context)* | Picks the precise meaning. |
| First-person perspective | reflexive perspective / reflexive readout | Acceptable to keep "first-person" if perspectival emphasis is wanted, but disambiguate. |
| Inner experience | reflexive readout | Same. |
| Inner life | reflexive trajectory | If the structural-dynamical content is wanted; otherwise delete. |
| The "self" as ontological category | reflexive structure | Clean replacement. |
| Cogito | reflexive identity | If keeping the Cartesian gesture; otherwise replace with an FTD-internal term. |
| Cosmic consciousness *(or:)* universal consciousness | reflexive structure of the cosmos *(if literal claim)* / (avoid) *(if rhetorical flourish)* | Almost always rhetorical; usually delete. |
| Panpsychism | reflexive-genericity hypothesis | If FTD wants to claim reflexivity is generic at lattice level (which it does — see §3); avoids commitment to "everything has experience." |
| The "Hard Problem of consciousness" attaches | (delete the attachment) | The qualia debate is not FTD's; do not import it. |

---

## 5 · Distinctions FTD needs to make explicitly

### 5.1 — Reflexive structure ≠ realized reflexive agency

A 27-block on the lattice is *structurally* reflexive (its center has Moore-26 neighbors that encode a model of the center). Almost every voxel is at the center of some 27-block. **Most reflexive structures do not realize agency** — their reflexivity is unrealized in the trajectory dynamics. The interesting question is which reflexive structures' reflexivity is dynamically active.

**Old framing (lossy):** *"Some configurations are conscious and some are not."*
**New framing (precise):** *"All sufficiently-extended configurations are reflexive at the lattice level. Only some realize their reflexivity in agentic dynamics."*

This kills panpsychism worries and the binary-consciousness debate in one move.

### 5.2 — Reflexive readout ≠ correct readout

A reflexive site reads off some function of its own state. **There is no axiom that the readout is faithful or consistent.** Reflexive readouts can disagree across sites; reflexive readouts can be self-inconsistent (Gödel-style); reflexive readouts can be wrong about the wider configuration.

This corresponds to the absence-of-omniscience that physics requires of any observer.

### 5.3 — Agency ≠ libertarian free will

Agency in this vocabulary means *the trajectory is selected by reflexive structure rather than environment*. It does **not** mean *the trajectory is selected by a non-physical free will*. Agentic trajectories are still embedded in the lattice's deterministic dynamics; they're just dynamically driven by the reflexive part of the configuration rather than the boundary conditions.

This kills the "FTD claims free will violates physics" misreading.

### 5.4 — Endogenous causation ≠ acausality

Endogenous causation means the cause is inside the system. It does **not** mean there's no cause. Every endogenous-causal step is dynamically lawful; the lawfulness just routes through reflexive structure rather than environment.

### 5.5 — The agentic threshold

The 27-block is the smallest reflexive structure on the lattice. The smallest reflexive structure that realizes agency is **not the same thing**. This is an open structural question: at what configuration scale does reflexivity transition from "present but unrealized" to "dynamically driving trajectory"? The agentic threshold is defined as the answer.

The wakefulness/unconsciousness distinction in neuroscience is the empirical analogue. FTD's lattice analogue would let this be computed rather than measured.

---

## 6 · What's preserved, reframed, and dropped

### Preserved (full conceptual content survives)

- The two-layer ontology (phenomenal 2³ / noumenal 3³ blocks) is preserved as **phenomenal layer / noumenal layer**, with the projection between them now called the **reflexive projection** rather than "consciousness."
- The BCC interior axis story (FTD-0078) is preserved as the **reflexive eigenstructure** — the eigenmodes of the reflexive projection live on the BCC interior axis.
- The 27-block as canonical "consciousness object" is preserved as the **canonical reflexive structure**.
- The hypothesis that the master quadratic eigenvalue spectrum is structurally connected to the reflexive projection is preserved as the **MC-T4.3 closure conjecture** (the central foundational obstruction in `CHECKLIST_MATH_COMPLETE.md`).

> **Engine-side note (post 2026-05-01):** the engine's Scale 11 ("Reflexivity") UI was deleted (commit `054b530`) — it was an interpretive pedagogical visualization (holographic figure / sLoop ring / audio synthesis), not load-bearing for any derivation. The mathematical content the vocabulary refers to lives entirely in the theory docs (`docs/theory/06_consciousness/*`, `FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md`) and is unchanged. If MC-T4.3 closure is ever attempted via a new engine implementation, that's a fresh engine module, not a revival of Scale 11.

### Reframed (same content, sharper framing)

- "Consciousness creates reality" → **"Reflexive structure forces the observable algebra"** — same claim, no Berkeley / idealist baggage.
- "Subjective time" → **"Reflexive trajectory parametrization"** — same content, no first-person-mystery framing.
- "The observer in QM" → **"The observation layer in lattice ED"** — same role, no need for a conscious agent.
- "Wave-function collapse requires a conscious observer" → **"Reflexive coupling selects the observable basis"** — same mechanism, no Wigner's-friend regress required.

### Dropped (claims FTD does not actually need)

- "Qualia are explained by FTD." (FTD doesn't do this; the existing literature gestures at it but never delivers; drop the gesture.)
- "Consciousness is fundamental." (Replace with: *reflexivity is structural*, which is true and load-bearing.)
- "The universe is conscious." (Replace with: *the lattice is reflexive*; same observation, no panpsychism commitment.)
- The Hard Problem of Consciousness as something FTD claims to address. (FTD addresses the *structural* problem of reflexive emergence; it does not address whether reflexive readouts are accompanied by qualia. Drop the qualia commitment cleanly.)

---

## 7 · Application guidance

When converting an existing FTD doc, ask in this order:

1. **Is this passage about a structure or about dynamics?**
   - Structure → reflexivity, reflexive site, reflexive projection.
   - Dynamics → agency, agentic causation, trajectory selection.
   - Both → reflexive agency.

2. **Is the passage a measurement / observation claim?**
   - Yes → observation layer, observable algebra, reflexive readout.
   - No → don't import "observer" language.

3. **Is the passage making a qualia / phenomenology / Hard-Problem claim?**
   - Yes → drop the claim or restate as reflexive emergence.
   - No → no qualia language gets imported by accident.

4. **Is the passage rhetorical (e.g., "the universe is conscious")?**
   - Yes → either drop or restate as a precise structural claim.

5. **Does the passage cite a specific FTD-NNNN row tagged for consciousness?**
   - Yes → preserve the row, rename in the row's surrounding prose, update LEDGER detail block accordingly.

When in doubt, prefer the *less-claiming* term. **Reflexivity is generic and physics-compatible; agency is specific and structural; consciousness is contested and qualia-laden.** The vocabulary is designed so that *less-claiming* is also *more-precise*.

---

## 8 · What this vocabulary does NOT commit to

This vocabulary deliberately leaves four questions open:

1. **Whether reflexive readouts are accompanied by qualia.** (FTD doesn't need to commit; the qualia question lives in philosophy, not physics.)
2. **Whether agency is libertarian or compatibilist.** (FTD's lattice dynamics are deterministic; agency in this vocabulary is compatibilist by construction. If a sharper claim is wanted, it's a separate axiom.)
3. **Whether the universe is "fundamentally conscious."** (Replaced with the structural claim that reflexivity is generic at lattice level. The fundamentalness question is reframed away.)
4. **Whether AI systems with reflexive structure exhibit agency.** (FTD takes no position here; the vocabulary applies to any reflexive structure on the lattice including AI-shaped ones. This is appropriate disciplinary reticence, not a claim.)

These open questions are *features*, not bugs. The vocabulary's job is to let FTD claim what it can prove and not import what it can't.

---

## 9 · Proposed propagation scope

Files that load-bear consciousness vocabulary and would benefit from this rename, in priority order:

| File | Old usage count | Priority |
|---|---|---|
| `docs/theory/02_foundations/FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md` | high | **P1** — central reframe doc |
| `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` | high | **P1** |
| `docs/theory/06_consciousness/*.md` | very high | **P1** — entire directory |
| `docs/theory/07_assessment/LEDGER.md` | medium (FTD-0078, FTD-0095, FTD-0121) | **P2** — surface-level rename |
| `docs/theory/01_reference/SPEC_PHYSICS_BRIDGE.md` | low | **P2** |
| ~~`engine/web/js/scales/scale11/*`~~ | — | **DELETED 2026-05-01** (commit `054b530`); engine Scale 11 UI removed entirely. No vocabulary work needed. |
| `dissemination/manuscript_v2/vol1/src/chapters/14.5-assumption-ledger.qmd` | low | **P3** — already stale per audit |
| `dissemination/whitepaper/FTD_Whitepaper.tex` | low | **P3** — version-bump pass |
| `CLAUDE.md` "Current epistemic state" + "Key results" | low | **P4** — consistency pass |

The propagation sweep is itself a substantial doc-edit job (~estimated 1–2 sessions). It is **not** a prerequisite for Paper A — Paper A doesn't load consciousness vocabulary. But the sweep would be a prerequisite for any FTD paper that addresses MC-T4.3 closure or the foundational ontology extension, since those papers must be physics-readable.

---

## 10 · Cross-references

- `docs/theory/01_reference/CHECKLIST_MATH_COMPLETE.md` — defines MC-T4.3 (where this vocabulary is most load-bearing).
- `docs/theory/02_foundations/FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md` — the existing-literature anchor for reflexive projection (currently called "consciousness").
- `docs/theory/02_foundations/FOUND_TERNARY_STATE_FROM_I.md` — grounds Postulate 3's `{−1, 0, +1}` values in Axiom 0 via `s = i²`; the `{s, 0, |s|}` notation makes the polarity-magnitude pairing of the dispositional/actual graded monism structurally visible (see §10.1 below).
- `docs/theory/07_assessment/LEDGER.md` — FTD-0078 (phenomenal/noumenal bridge), FTD-0095 (Bridge Functional ontology), FTD-0121 (physics-bridge synthesis), FTD-0128 (Postulate 3 grounding via s = i²).
- `CLAUDE.md` — "Two-layer ontology" passage establishes the framework's existing language; rename targets here are minimal.

### 10.1 · Vocabulary for the state-field grounding (FTD-0128, 2026-05-03)

The `{s, 0, |s|}` notation for the state field makes the polarity-magnitude pairing of the dispositional/actual graded monism structurally visible. Mapping onto the two-layer ontology:

| aspect | dispositional layer (J, continuous) | actual layer (s, discrete) |
|---|---|---|
| polarity | continuous direction `Ĵ` | discrete sign `sign(s) ∈ {−, 0, +}` |
| magnitude | continuous `\|J\|`, unbounded ("infinite potential" each voxel can carry) | unit magnitude `\|s\| ∈ {0, 1}`; the actual energy/potential content sits in `\|J\|` |
| equilibrium | `J = 0` | `s = 0` |
| algebraic substrate | `J ∈ ℝ³` (vector field, continuous phase content) | `s ∈ {i², 0, \|i²\|}` = `{−1, 0, +1}` (real projection of `Z[i]^× ∪ {0}`) |

The state-field's specific values `{−1, 0, +1}` follow from Axiom 0 via `s = i²`; they are not independent postulates. See `FOUND_TERNARY_STATE_FROM_I.md` for the full grounding chain. The imaginary half `{i, −i}` of `Z[i]^×` is not discarded — it lives in the flux field's continuous phase content.

---

## 11 · Status

**This vocabulary is proposed, not adopted.** Adoption requires owner sign-off + propagation sweep. The file is canonical-reference-grade so future agents performing the sweep can cite it as the authoritative target vocabulary, but no existing doc has yet been converted. The doc itself uses the new vocabulary throughout — this serves as a worked-out demonstration that the rename is conceptually viable.

**Next steps after sign-off:**
1. P1 sweep (FOUND_PHENOMENAL_NOUMENAL_BRIDGE, 06_consciousness/*).
2. P2 sweep (LEDGER, SPEC_PHYSICS_BRIDGE) — surface-level rename only; FTD-NNNN content preserved.
3. P3 sweep (engine UI, manuscript, whitepaper).
4. P4 sweep (CLAUDE.md consistency).
5. Cross-link this file from CLAUDE.md "Naming Conventions" or "Key Navigation Documents."
