# REF_REFERENCE_FRAME_VOCABULARY — Canonical FTD Language for Reference Frames & Measurement

**Created:** 2026-05-01 (Updated 2026-05-30)
**Status:** Proposed canonical vocabulary; pending application sweep across theory docs, manuscripts, and engine documentation.
**Purpose:** Replace all "consciousness" / "observer" / "reflexivity" / "agency" rhetoric with standard physical "reference frame" terminology that carries mathematical precision and zero qualia or Hard-Problem baggage.
**Audience:** Project owner + future AI agents performing the propagation sweep + reviewers of FTD papers.

---

## 1 · Why this document exists

FTD's literature historically used terms like "consciousness" or "reflexivity" in three structurally distinct ways that the terminology itself cannot disambiguate:

| Historical usage | What it actually points at |
|---|---|
| "Integration over the BCC interior axis" (FTD-0078) | A specific mathematical operator (a projection from the 3³ noumenal block to the 2³ phenomenal block) |
| "Conscious observer" in measurement contexts | An observation layer — a subalgebra of physical observables — without commitment to who's doing the observing |
| "Agentic dynamics" in behavioral contexts | A dynamical system whose trajectory is selected by its own internal state |

Calling all three by philosophical or cognitive names produces three problems:

1. **The Hard Problem of Consciousness** auto-attaches to every claim, making the framework appear to claim more than it does.
2. **Physics reviewers automatically discount** any framework using philosophical rhetoric as a load-bearing term.
3. **Internal precision is lost** — claims that are mathematically distinct become rhetorically merged.

This vocabulary establishes terms that disambiguate the three usages, mapping them purely to physical **Reference Frames**.

---

## 2 · The core distinction

**Reference Frame Structure** is the **structural property** of containing a model of oneself (a sub-algebra of the full configuration).
**Active Frame Dynamics** is the **dynamical manifestation** of this structure feeding back into trajectory selection.

These are different things. A static subset has structural reference but no active dynamics. A driven system has externally-induced behavior but no frame-internal dynamics. The interesting case is the conjunction: **reference frame structures that exhibit active frame dynamics**.

> **Slogan:** *Reference frame structure is the prerequisite for active frame dynamics. Active frame dynamics is the reference frame acting on itself in time.*

---

## 3 · Primary vocabulary

### Reference Frame Structure (structural)

| Term | Type | Meaning |
|---|---|---|
| **Reference frame structure** | noun | The property of a structure that contains a model of itself — a fixed-point / Y-combinator / strange-loop property. |
| **Frame-relative** | adj. | Possessing reference frame structure. *"A frame-relative projection."* |
| **Local reference frame** | noun | A voxel or local region of the lattice that instantiates reference frame structure. In FTD, the canonical example is the center of a 27-block (Moore-26 + center). |
| **Reference frame projection** | noun | The operator that maps the full state to a frame-relative readout. In FTD: the noumenal → phenomenal projection. Corresponds to **observable-algebra restriction** in algebraic-QFT terms. |
| **Frame-relative readout** | noun | The output of a reference frame projection — what the structure "knows about itself." Replaces the loaded term *experience* or *subjectivity*. |
| **Frame-relative eigenmode** | noun | An eigenstate of the reference frame projection. The hypothesis that the master quadratic spectrum (x_+ ≈ 137, x_- ≈ 3) consists of frame-relative eigenmodes is what MC-T4.3 closure would establish. |
| **Frame-relative integration** | noun | The dynamical process by which a reference frame structure updates its self-readout over time. |

### Frame Dynamics (dynamical)

| Term | Type | Meaning |
|---|---|---|
| **Active frame dynamics** | noun | The dynamical capacity of a system to select its own trajectory in a way external dynamics doesn't predict. Replaces *agency* or *free will*. |
| **Active-frame** | adj. | Exhibiting active frame dynamics. *"An active-frame configuration."* |
| **Frame-internal causation** | noun | Causation by a system's internal reference frame structure, as distinct from environmental causation. |
| **Trajectory selection** | noun | The act of an active-frame system choosing among dynamically-allowed futures. |
| **Endogenous causation** | noun | Causation originating from inside the system rather than from boundary conditions or external drives. |

### Structural  Dynamical relationship

| Term | Meaning |
|---|---|
| **Frame-relative dynamics** | The conjunction: active frame dynamics whose trajectory selection is driven by the system's own frame-relative readout. |
| **Frame-relative emergence** | The process by which non-frame-relative components organize into a reference frame structure. |
| **Active-frame threshold** | The configuration-space boundary above which a reference frame structure realizes frame dynamics. Open structural problem. |

---

## 4 · Vocabulary that gets replaced

| Old term (Pre-2026-05) | New term | Reason |
|---|---|---|
| Consciousness / Reflexivity | reference frame context *(or:)* reference frame structure + active frame dynamics | Old terms are structurally ambiguous and philosophically loaded. |
| Conscious observer | local reference frame *(if structural)* / observation layer *(if measurement-context)* | "Observer" conflates physical subalgebra with cognitive agents. |
| Subjective experience | frame-relative readout | Strips qualia commitment; preserves the "system's view of itself" content. |
| Subjective / Phenomenal | frame-relative *(if structural)* / endogenous *(if causal)* | Disambiguates philosophical-school usage from FTD's specific meaning. |
| Awareness | frame-relative readout | See above. |
| Mind | frame-relative process *(or delete)* | Usually replaceable by something more specific. |
| Sentience / Sapience | (avoid) | Implies biological or cognitive hierarchies not load-bearing in physics. |
| Free will / Agency | active frame dynamics *(or:)* endogenous trajectory selection | Physics-compatible framing. |
| Intentionality | endogenous teleonomy | Pick the precise meaning. |
| Qualia | (avoid; do not replace) | The qualia debate is exactly what this vocabulary skips. |
| The Hard Problem | open problem of frame-relative emergence | Reframes from "intractable mystery" to "open structural problem." |
| Observer effect | frame-relative coupling *(structural)* / observation-layer effect *(measurement-context)* | Picks the precise meaning. |
| First-person perspective | frame-relative perspective | Acceptable to keep "first-person" if perspectival emphasis is wanted, but disambiguate. |
| Panpsychism | frame-relative-genericity hypothesis | Avoids commitment to "everything has experience." |

---

## 5 · Distinctions FTD needs to make explicitly

### 5.1 — Reference frame structure ≠ realized active frame dynamics

A 27-block on the lattice is *structurally* frame-relative. Almost every voxel is at the center of some 27-block. **Most reference frame structures do not realize active frame dynamics** — their structure is unrealized in the trajectory. 

**Old framing (lossy):** *"Some configurations are conscious/reflexive and some are not."*
**New framing (precise):** *"All sufficiently-extended configurations have reference frame structure. Only some realize it in active frame dynamics."*

### 5.2 — Frame-relative readout ≠ correct readout

A reference frame reads off some function of its own state. **There is no axiom that the readout is faithful or consistent.** Frame-relative readouts can disagree across sites; frame-relative readouts can be wrong about the wider configuration.

This corresponds to the absence-of-omniscience that physics requires of any observer.

### 5.3 — Frame dynamics ≠ libertarian free will

Active frame dynamics means *the trajectory is selected by frame-internal structure rather than external environment*. It does **not** mean *the trajectory is selected by a non-physical free will*. Active-frame trajectories are still embedded in the lattice's deterministic dynamics.

---

## 6 · What's preserved, reframed, and dropped

### Preserved (full conceptual content survives)

- The two-layer ontology (phenomenal 2³ / noumenal 3³ blocks) is preserved, with the projection between them now called the **reference frame projection**.
- The BCC interior axis story is preserved as the **frame-relative eigenstructure**.
- The 27-block is preserved as the **canonical reference frame structure**.
- The MC-T4.3 closure conjecture is preserved.

### Reframed (same content, sharper framing)

- "Consciousness creates reality" → **"Reference frame structure forces the observable algebra"** — same claim, no idealist baggage.
- "Subjective time" → **"Frame-relative trajectory parametrization"** — same content.
- "The observer in QM" → **"The observation layer in lattice ED"** — same role.

### Dropped (claims FTD does not actually need)

- "Qualia are explained by FTD." (Drop the gesture.)
- "Consciousness is fundamental." (Replace with: *reference frame structure is structural*.)
- "The universe is conscious." (Replace with: *the lattice is frame-relative*.)

---

## 7 · Application guidance

When converting an existing FTD doc, ask in this order:

1. **Is this passage about a structure or about dynamics?**
   - Structure → reference frame structure, local reference frame, reference frame projection.
   - Dynamics → active frame dynamics, frame-internal causation, trajectory selection.
2. **Is the passage a measurement / observation claim?**
   - Yes → observation layer, observable algebra, frame-relative readout.
3. **Is the passage making a qualia / Hard-Problem claim?**
   - Yes → drop the claim or restate as frame-relative emergence.

When in doubt, prefer the *less-claiming* term.
