# EXPLR — The Sixth-Postulate Hole and the Observer Frame

**Tag:** `[EXPLORATION]` / `[OPEN]`. **This document promotes no claim, derives nothing, and
touches no canonical tag.** It records a long foundational exploration (2026-06-09) of candidate
6th postulates for FTD, including **two formalization attempts that were adversarially falsified** —
both preserved here as provenance so they are not re-attempted as zombies. Where this doc and any
canonical source (`core_ledgers/LEDGER.md`, `SPEC_ALGEBRAIC_SPINE.md`, `AUDIT_BOUNDARY_MAP.md`)
disagree, the canonical source wins.

**Companion:** `docs/theory/07_assessment/AUDIT_BOUNDARY_MAP.md` (the boundary map this arc grew out
of), `01_reference/REF_REFERENCE_FRAME_VOCABULARY.md` (the de-wooed frame vocabulary this builds on).

---

## 0 · Why this exists

The boundary map (AUDIT_BOUNDARY_MAP §2) found a **recurring pattern**: FTD's five postulates
generate structure, but every *selective* or *interpretive* feature of physics turns out to be a
structure **provably absent** from P1–P5, injectable only as an independent 6th-postulate-class
axiom. This exploration tried to name that missing primitive. It did **not** succeed in deriving
anything — but it mapped the hole precisely, which is the Number-One Goal's second clause. The
honest deliverable is a **boundary**, not a theorem.

---

## 1 · The recurring 6th-postulate pattern (five faces of one hole)

| Probe (sector) | Structure needed | Status in P1–P5 | Canonical ref |
|---|---|---|---|
| Quantum core | **non-commutativity** of observables | absent (substrate is commutative) | FTD-0225/0226/0228/0251 |
| Lorentzian metric | **reversibility** (2nd-order action) | absent (determinism ≠ reversibility) | FTD-0253 |
| The value of α | a **binding law W** realizing √(G*(4G*−1)) | absent (route-invariant) | FTD-0242/0243 |
| Actualization | **stochastic selection** among dispositions | absent (manifestation map is deterministic) | FTD-0226 |
| Meaning | the **observer's model / decoder** | absent (no such object in the corpus) | this doc, §7 |

Five probes, one shape: the postulates describe the substrate **from nowhere**; each probe needs a
*situated, selective* element the substrate does not contain.

---

## 2 · Candidate 6th postulate: **Instantiation**

The starting intuition (the "action taker"): *something that instantiates perturbations in still
fields.* Refinement: **FTD already has a deterministic, commutative action-taker** — genesis /
manifestation, the map `J → s` (dispositional flux → actual ternary state) via the manifestation
coupling `−g_c·s·∇·J` and the threshold `|J|² > K_B`. So the question is not "add an action-taker"
but **"what structure must the act carry, and should it be a postulate?"**

Naming (chosen to avoid mysticism): **Postulate of Instantiation** — one act with three moments:

- **Differentiation** — the *content*: drawing a distinction (void `0` → definite `±1`).
- **Instantiation** — the *act*: a potential becomes an actual token.
- **Identity** — the *result*: the token now has a definite, persisting identity.

(The names "Becoming" and "Will" were rejected: the first imports process-philosophy connotations,
the second smuggles agency into a postulate that should stay mechanistic.)

---

## 3 · "Stochastic will", quarantined

User ontology: **physics is deterministic; instantiation carries irreducible contingency.** This
threads cleanly:

- **P5 (determinism)** governs the *flux flow*; it was always silent on *which* dispositions
  actualize. Stochastic instantiation is a **new layer**, not a contradiction of P5. Honest
  description: *deterministic flow punctuated by stochastic actualization events* (the GRW/CSL
  shape), on the irreversible G\*/ratio face.
- **It is not blocked by FTD-0226.** That closed-negative concerned the *deterministic* manifestation
  map. A *stochastic* map is a different object — a genuinely open door, not an exhausted route.
- **Born vs. signed sampling.** A *stochastic* act needs a non-negative *realized-frequency* weight,
  which forces `|J|²` (Born) — but only at the level of realized frequencies (frequentism), **not**
  the amplitude. The amplitude `Re` may be signed; its signedness carries phase. So Born is
  **strongly motivated, not forced**; a genuinely *signed selection rule* is a logically open branch
  where any non-QM novelty would live. `[OPEN]`

---

## 4 · Two orthogonal fields, and the division of labor

"Two orthogonal fields" = the real-vector form of a complex amplitude. FTD already has it twice:
the symplectic quadrature `(q,p)` (FTD-0251) and `(Re, Im)` (EF-T5: `|x|² = E(x)² + E(ix)²`). The
apparent fork **(q,p) vs (J,s)** dissolved (savant round, 2026-06-09) into a **division of labor**:

- **(q,p)** — reversible, conservative — supplies the *phased amplitude* (the disposition that is sampled).
- **(J,s) / ReLU** — irreversible, lossy — is the *act* that samples it (genesis, the arrow, collapse).

A consequence (and a discipline note): **α⁻¹ is not "the coupling of both fields."** The coupling in
`(q,p)` is the oscillator frequency `ω(k)` (zero α content); the coupling in `(J,s)` is `g_c = √α`
(but `[PARAMETRIC]`, FTD-0031 closed-negative). The tempting reading of the budget equation
`x/16G*² + G*/x = 1` as a "two-channel normalization" with `x₊ = α⁻¹` as the balance point was
judged **numerology** by the algebraic-spine savant — it is the master quadratic rearranged, the
"two channels" are an imported `[SELECTION]`, and `x₋`'s closed-negative status (FTD-0210) kills any
"one root per field" mapping. **Do not pursue α⁻¹-as-inter-field-coupling; it would be a substitution
identity.**

---

## 5 · Self-determination vs. randomness — and frame-relativity

The ontology "inanimate = deterministic, animate = stochastic" was refined twice:

1. **"Stochastic" is the wrong word.** Art is the *least* random thing humans make. The relevant
   category is a **third** one beyond determined/random: **self-determined** — novel-but-not-arbitrary,
   generated by a self-referential loop (computational irreducibility; strange loops; autopoiesis).
   This is **not vitalism**: it is substrate-neutral (any sufficiently self-referential system would
   show it) and maps onto FTD's existing **self-referential closure** (the sLoop, "observer is
   observed"). The mapping is an **analogy**, `[CONJECTURE]`, not a derivation.

2. **Random and self-determined are frame-relative.** "Random" = *"self-determined by a loop I can't
   access"* (a pseudorandom sequence is deterministic to whoever holds the seed, random to everyone
   else — algorithmic randomness is generator-relative; meaning is mutual information). This
   **dissolves the ontic-vs-epistemic stochasticity fork**: determinism is the *inside* view of the
   loop, randomness the *outside* view — the same act seen from two frames. "Will" and "Born noise"
   are a **parallax**, both true.

---

## 6 · Two formalization attempts — BOTH FALSIFIED (provenance; do not re-attempt)

### 6a · meaning ≈ observation-depth `d` — **`[METAPHOR, NOT DERIVATION]`**
Conjecture: the Chebyshev observation-distance `d` (self-referential `d=0` / overlapping `1≤d≤2` /
external `d>2`, PI-T1) behaves like mutual information. **Killed** (adversarial savant, 2026-06-09):
- **Category error** — `d` is a lattice distance (topology); MI is a functional of a joint
  distribution (probability). Equating them is the substitution-identity move CLAUDE.md forbids.
- **Monotonicity fails** — small `d` can carry low MI (independent dynamics on shared support); large
  `d` can carry high MI (distant clusters correlated via the propagator). `d` indexes current spatial
  separation, not historical causal coupling.
- **Determinism trivializes region-region MI** to {0, maximal} by light-cone ordering — not a smooth
  function of `d`.
- **Internal correction** — in FTD, observation *depth* is **layer count** (SC/FCC/BCC activation),
  **not** the distance `d`. The metric conflated two distinct things.

### 6b · meaning ≈ `I(readout ; field)` — **`[REJECTED]` as a theory of meaning; residue `[PARAMETRIC]`/`[EXPLORATION]`**
Repair: re-anchor to the mutual information between an observer's *readout* and the field — a genuine
information functional. **Killed again** (adversarial savant, 2026-06-09):
- **Triviality** — "coarser readout preserves less MI" is just the **data-processing inequality**,
  true of every system, zero FTD-specific content.
- **Semantics ≠ correlation** — MI is symmetric and syntactic; high MI is not *aboutness* (Searle,
  Floridi; a thermometer has high MI with temperature but temperature is not "meaningful to it").
- **The 1,674× ternary-vs-boolean figure is fragile** — sourced from `REPORT_DETECTOR_INFORMATION_LOSS.md`,
  tagged `[EXPLORATION]`, **not in the LEDGER**, **not load-bearing**. The denominator
  (boolean MI ≈ 0.00027 bits) is at the histogram noise floor; the 64-bin choice is unjustified; a
  binning sweep swings the ratio 3–10×; and the setup is a **generic classical double-slit** — the
  same numbers appear in any wave sim with any quantizer. It is Shannon's machinery on a wave field,
  with FTD as *motivation*, not as the thing tested. **Do not cite it as empirical support for the
  ternary ontology.**
- **Bait-and-switch** — the original phenomenon was *two observers of one signal* differing by
  **decoder/loop** (artist vs. stranger). The repair substituted *one observer with two detector
  hardwares* (ternary vs. boolean) — **channel coarseness, not decoder mismatch.** It saved the
  formalization by abandoning the phenomenon.

**What honestly survives** (and is unremarkable): Born `|ψ|²` destroys phase `[THEOREM]`; coarse
readouts are lossy (DPI); the projection hierarchy `E → |·| → |·|² → click` is a DPI chain.

---

## 7 · The discovery — meaning is **model-conditional**, and FTD has no `M_O`

The two failures point the same way. The correct formalization of the original intuition is **not**
`I(readout; field)` but a **model-conditional** quantity:

> meaning of signal `S` to observer `O` ≈ `I(S ; source | M_O)` — information *given the observer's
> model* `M_O`; equivalently, how much `O`'s model **compresses** `S` (the pseudorandom-seed argument:
> "random" = lacking the generating program). This captures artist-vs-stranger; `I(readout; field)`
> does not.

**FTD has no object `M_O`.** Nowhere in P1–P5 or the corpus is there a formalized *observer's model /
decoder*. The lattice has observers as **regions** and readouts as **projections** — it has no
machinery for what an observer **brings**. That absence is the fifth face of the §1 hole.

---

## 8 · The synthesis

The five probes are five faces of one missing primitive: **a situated, selective element** — the
substrate is described from nowhere, and measurement, orientation, selection, and meaning all require
a *somewhere* to be measured/oriented/selected/meant **from**. The natural name for "a somewhere from
which physics is done" is a **frame of reference**. This is the forward direction (§9).

---

## 9 · Forward direction (OPEN): the **frame of reference** as a candidate 6th postulate

Einstein's move was to make the **frame of reference a first-class physical object** — physics is
written relative to it; there is no view from nowhere. The hypothesis under exploration:

> **The missing primitive across the five faces is the observer frame.** Elevate the frame from a
> bookkeeping convenience to a postulate: a primitive `F` carrying (i) an orientation/basis, (ii) a
> partial access (a self-blind-spot), and (iii) a model/decoder `M_F`. Manifestation, selection, and
> meaning are all **frame-relative**; the substrate (P1–P5) is **frame-covariant**.

**Why it is promising:**
- FTD already commits to a **frame-relative projection layer** (`REF_REFERENCE_FRAME_VOCABULARY.md`,
  the `06_reference_frames_and_measurement` cluster), with the de-wooed split *reference-frame
  structure* (structural) vs *frame dynamics* (dynamical). A frame postulate would give that layer a
  **primitive**, not just a vocabulary.
- It is **literally Einstein's move for the metric** (GR: the metric is the field of local frames),
  so it bears directly on the FTD-0253 open question (causal cone forced `[THEOREM]`, metric posited).
- It supplies the missing `M_O` of §7 (the frame *is* the observer's model/decoder), and the
  self-blind-spot of a frame is exactly where the apparent stochasticity of §5 came from.

**Why it is hard (the honest tensions, to be attacked, not wished away):**
- **Covariance vs. projection.** Einstein's frames express *objectivity* (laws invariant across
  frames). FTD's projection layer expresses *situatedness* (appearances vary by frame). A frame
  postulate must hold **both** — invariant substrate, variant manifestation — and show the frame is
  the *bridge* (proper time invariant, coordinate time frame-relative). Conflating the two senses of
  "frame" would be a category error of exactly the kind §6 was killed for.
- **It does not obviously absorb all five faces.** It fits the observer-model/meaning face and the
  measurement/self-blind-spot face well, and the metric face (GR). It is only *necessary scaffolding*
  — not sufficient — for **non-commutativity** (incompatible frames need a non-trivial transform
  between them, which the frame alone does not supply), and it is *orthogonal* to **reversibility**
  (FTD-0253's missing ingredient is a separate axis). **Claiming the frame unifies all five would be
  over-reach.** The disciplined claim is: the frame is a strong candidate primitive for the
  measurement/meaning/metric cluster, with non-commutativity and reversibility as residue.
- **Anti-vitalism / anti-woo guard.** The frame must be **substrate-neutral and observer-as-region**,
  not a special consciousness-bearing entity. `REF_REFERENCE_FRAME_VOCABULARY`'s dropping of qualia
  commitments is the model to follow.

---

## 10 · What a testable version would need (before any tag above `[CONJECTURE]`)

1. A formal definition of a frame `F` as a lattice object: which sites, which basis, what "partial
   access" means operationally (a coarse-graining? a causal horizon? a sub-algebra?).
2. A precise statement of **frame-covariance** of the substrate dynamics (which transformations leave
   P1–P5 invariant) — and whether it is the lattice's discrete symmetry group (O_h ⋉ translations) or
   something larger in the continuum limit.
3. A precise statement of **frame-relative manifestation** (how the readout/Existence-Filter depends
   on `F`), and a proof that two frames on one substrate state can disagree on appearances while
   agreeing on invariants.
4. A check against the §6 killers: is "frame" being used in **one** rigorous sense throughout, or
   sliding between coordinate-covariance and decoder-mismatch? (This is the most likely failure mode.)
5. Whether the frame postulate **bears on FTD-0253** (does an emergent metric = the structure relating
   frames?) and on **FTD-0252** (proper-time vs coordinate-time as frame-invariant vs frame-relative).

---

## 11 · Round 3 (2026-06-09): the kinematic Frame Postulate — adversarially scoped down

The §9 kinematic face was run through a three-lens savant round (constructive / logical-relation /
falsifier). Verdict: **the naive form fails; a heavily-scoped form survives as
`[STRONGLY MOTIVATED CONJECTURE]`.** Results preserved here.

### 11a · The falsifier's kills (do not re-attempt the naive form)
1. **Discreteness obstruction, sharpened:** rotations have finite subgroups (O_h survives on the
   lattice and can plausibly restore to SO(3) in the IR); **boosts have no discrete subgroup**
   (the boost group is non-compact). "Boost invariance emerges in the IR" is therefore a
   *categorically harder* claim than rotation-restoration, and nothing in the corpus currently
   addresses it.
2. **The free-sector gap (CPSUV):** FTD-0252's L⁻² convergence was measured on **free wave clocks
   only**. In interacting lattice theories, UV Lorentz violation generically **percolates to the IR
   via loop corrections** (Collins–Perez–Sudarsky–Urrutia–Vucetich) absent fine-tuning. FTD's
   interacting sector (genesis + Gauss + Langevin) is **untested** on this point — the single
   biggest evidence gap.
3. **Tautology charge:** as naively stated, the postulate *asserts* the hard content (boost
   invariance + isotropy restoration + reversibility) and relabels the result "Einstein." The cone
   (forced) was the easy part; any local update rule has one.
4. **Type-check failure:** "the lattice rest frame is unobservable in the IR" is an L→∞ claim —
   not well-posed under the undefined-boundary ontology (`AUDIT_INFINITY_REFRAME`). Must be ε-L
   restated: *for every ε there is L such that frame-detection at resolution ε fails* — and even
   that is currently an assertion, with FTD-0252's diagonals (⟨111⟩, L≤193) still non-converged.

### 11b · The structural discovery: the two candidate axioms live at DIFFERENT LEVELS
The logical-relation lens resolved the §9 "reversibility vs relativity-principle" question, with
the **telegrapher/Cattaneo equation** as the decisive wedge (hyperbolic + dissipative + Lorentz-
covariant):
- The **relativity principle** constrains the **kinematics** (the transformation group between
  frames; group closure = reversible *transformations*) — it forces a hyperbolic, clock-bearing
  sector to exist but **permits dissipative overlays** (telegrapher).
- **Reversibility** (FTD-0253) constrains the **dynamics** — it forces the hyperbolic sector to be
  the *whole* dynamics (no dissipation) but **permits preferred frames** (FTD's own engine is the
  counterexample: reversible wave dynamics on a frame-preferring lattice).
- **Neither subsumes the other** (hypotheses (b),(d) falsified; (a),(c) each partial). They overlap
  on hyperbolicity and diverge on dissipation-tolerance vs frame-preference. Together they force
  the unique dissipation-free Lorentzian structure.
- Resolution of the apparent tension with the constructive lens's finding (von Ignatowsky's group
  closure "smuggles reversibility"): group closure is reversibility **of the transformations**
  (kinematic level), not of the dynamics — consistent with the telegrapher counterexample.

This sharpens §9's caution into a result-shaped statement: **"the 6th postulate" is not one axiom —
the metric needs a kinematic axiom (frame-invariance) AND a dynamical axiom (reversibility), and
FTD-0253 named only the second.** Whether a single frame-primitive can carry both is the live
question — asserting it would repeat the §6 conflation error.

### 11c · The minimal defensible claim (current honest form)
> Free massless-wave dynamics on the ⟨100⟩ axis at v ≲ 0.85 approach the relativistic dilation form
> with residual ∝ L⁻² (FTD-0252, `[MEASURED — scoped]`). This is *consistent with* — but very far
> from establishing — an emergent frame-invariance. Unestablished: diagonal axes (L ≳ 257 needed),
> boost-group emergence (no discrete subgroup exists), the interacting sector (CPSUV risk), and an
> ε-L-well-posed statement of frame-unobservability. Tag: `[STRONGLY MOTIVATED CONJECTURE]`,
> scoped exactly as above.

### 11d · The decisive pre-registerable experiment (queued, not run)
**Three-axis isotropy-restoration sweep:** inject the same soft mode along ⟨100⟩/⟨110⟩/⟨111⟩ at
L ∈ {65, 129, 193, 257}; measure the dilation residual power law per axis. Prediction if
frame-invariance emerges: all three axes converge with the same exponent p ≈ 2. Falsifier: diagonal
exponents differ by >0.5 or saturate. This *completes* FTD-0252's own open item (diagonals need
L ≳ 257) and is the cheapest decisive advance. A follow-up campaign on an **interacting** clock
(genesis/Langevin on) would address the CPSUV gap — the genuinely novel attack surface this round
surfaced. **Pre-registration required before any run** (per FTD-0203 discipline).

*Round-3 status: naive kinematic Frame Postulate `[FALSIFIED as stated]`; scoped form `[SMC]`;
kinematic/dynamical two-axiom split = the round's load-bearing structural finding; engine
experiment queued `[OPEN]`.*

---

## 12 · Round 4 (2026-06-09): the grand unification — **object-level CLOSED-NEGATIVE**, schema survives as navigation only

The highest-risk question was put directly: can a **single frame primitive** carry both axiom-halves —
and, the grand prize, subsume all five §1/§8 holes as sections of one "redundancy bundle"? Three lenses
(constructive / logical / falsifier), falsifier from the start. **Verdict: object-level unification is
`[CLOSED NEGATIVE within this exploration]`; only a navigational schema survives, at `[OBSERVATION]`.**

### 12a · The three kills
1. **Type-heterogeneity (the kill-shot).** The five holes are not one type. **Reversibility and
   non-commutativity are *properties*** — of the dynamics and of the observable algebra respectively —
   **not sections/choices/frames at all.** The other three (measurement basis, observer/system cut,
   decoder `M_O`) *are* choices, but sections of **three different bundles over incompatible base
   spaces** (Hilbert space / partition lattice / model-space). Calling all five "frames" is the same
   property↔choice metaphor-laundering that killed §6b's `I(readout;field)` (which conflated correlation
   with semantics). A schema that miscategorises two of its five members is already broken.
2. **Round-3 pre-falsification confirmed.** Even within the *metric* face, kinematic reversibility
   (frame-change invertibility / group closure) is **logically independent** of dynamical reversibility
   (dissipation-freedom) — the telegrapher equation (hyperbolic + dissipative + Lorentz-covariant) is
   the standing counterexample: an invertible *family of frames* observing an *irreversible* evolution.
   So "frame" cannot carry both axiom-halves for the two most closely-related holes; it will not do so
   across five.
3. **The Stone's-theorem rescue is relabeling, not derivation.** Defining "frame" to *include
   time-translation as a group* does force a self-adjoint generator (Stone ⟹ unitary ⟹ reversible) —
   but the **semigroup-vs-group distinction IS the reversibility question** (FTD-0253: determinism ≠
   reversibility, P5 gives only a semigroup). Requiring the group structure assumes what it claims to
   derive; it smuggles reversibility into the frame *definition*. Legitimate as an axiom choice,
   worthless as a unification.

### 12b · No canonical redundancy bundle in FTD (the referent is missing)
"Frame = section of *the* substrate redundancy" presupposes one redundancy. FTD has **several unrelated
ones**: the **Moore-layer gauge** `U(1)×SU(2)×SU(3)` is **topologically forced** (counts which J-components
a sublattice excites — *not* chosen); the **Domain-A/B context-selection** bifurcation (from the master-
quadratic discriminant) *is* a genuine chosen redundancy; the **observer/system cut** lives in the
partition lattice. Different categories, disjoint bases. "One frame primitive" has **no referent** in the
actual infrastructure.

### 12c · Vacuity
Even granting the schema, "everything the substrate doesn't fix is a 'frame'" **predicts nothing and
forbids nothing** — it cannot be used to *derive* that (say) reversibility is missing; the derivation
had to be done first and the label applied after. This is the data-processing-inequality-triviality
pattern again (§6b): true-by-construction, content-free.

### 12d · What survives — and a refinement to the §1/§8 framing
- **Survivor (`[OBSERVATION]`/`[SCHEMA]`, navigational only):** the five gaps share a *recurring shape* —
  *the substrate is frame-free (frame-covariant); selective/interpretive physics is frame-relative.*
  Useful as a map label for where the boundaries cluster. **It is not an explanation, not a primitive,
  and not a unification.**
- **Refinement of the "five holes" table (§1/§8):** the holes are **heterogeneous in type** — two are
  *missing properties* (non-commutativity of the algebra; reversibility of the dynamics) and three are
  *missing choice-structures* (measurement basis; observer/system cut; decoder `M_O`); the binding-law
  `W` (α) is a fourth, distinct, *operator-assembly* gap. They are five different things that share only
  the meta-fact of being unforced by P1–P5. The honest §9c/§11c position — "frames are a candidate for
  the measurement/meaning/metric *cluster*, with non-commutativity and reversibility as residue" — was
  already correct and is now sharpened: **the cluster is itself not one object.**
- **Corpus-hygiene flag (for a later, separate pass):** FTD's "frame"/"reference-frame" vocabulary
  quietly spans the *chosen* context-selection redundancy and the *topologically-forced* Moore-layer
  gauge. These are different kinds of object; a future cleanup should not let the shared word imply a
  shared structure. (Flag only — no edit made here.)

*Round-4 status: **object-level frame-unification `[CLOSED NEGATIVE within this exploration]`** (type-
heterogeneity + round-3 pre-falsification + no-referent + vacuity); **schema-level `[OBSERVATION]`,
navigational only**; the kinematic/dynamical two-axiom split (§11b) and the type-heterogeneity of the
five holes (§12a/§12d) are the durable structural findings. No canonical claim promoted or demoted.*

---

*Arc summary (rounds 1–4): four conjectures attacked, four scoped-or-killed, zero canon touched. The
durable yield is **boundaries, not theorems** — meaning is model-conditional (§7); the metric needs two
different-level axioms not one (§11b); the five 6th-postulate holes are genuinely heterogeneous in type
and do not unify under a single frame primitive (§12). The one queued positive probe is the three-axis
L≤257 isotropy sweep (§11d), `[OPEN]`, pre-registration required.*

---

## 13 · Round 5 (2026-06-09): the consolidation bet — Instantiation → non-commutativity — **theorem-level CLOSED-NEGATIVE**

The last live consolidation ("go big"): can a single **stochastic-Instantiation** postulate purchase
*both* the actualization face *and* the quantum-non-commutativity face — the latter emergent via
sequential Born-sampling with Gauss back-reaction (A-then-B ≠ B-then-A)? Three lenses, falsifier armed.
**Verdict: the non-commutativity half is `[CLOSED NEGATIVE within this exploration]` at theorem
strength; the actualization half survives, but separately and unfreely.** This *confirms and sharpens*
the existing commutativity boundary (FTD-0226/0228/0251; `THEOREM_COMMUTATIVITY_INDEPENDENCE`;
SYNTHESIS FTD-0238) — it is not a new boundary, it closes the one new route (stochasticity).

### 13a · The theorem chain (this is what makes it definitive)
1. FTD's substrate observable algebra is **commutative** — every beable (`s`, `J_a`, `v`, `L`) is a real
   function on one configuration space Ω; pointwise real multiplication commutes (`THEOREM_COMMUTATIVITY_INDEPENDENCE`, FTD-0243, `[THEOREM]`).
2. **Birkhoff–von Neumann (1936):** a commutative algebra ⟹ a **distributive (Boolean)** event lattice.
3. **Fine's theorem (1982):** a Boolean lattice ⟹ a **global joint distribution** over all contexts ⟹
   **no Bell/KS violation** ⟹ classical.
4. **Stochasticity is orthogonal to this wall.** Born-sampling `|J|²` draws from a joint distribution
   that *exists* (over the definite hidden flux λ = J); it cannot make that distribution *nonexistent*,
   which is what non-commutativity requires. The commutativity wall is at the *algebra* level, not the
   *determinism* level — so the one new ingredient (stochasticity) buys nothing against it.

### 13b · Why the back-reaction is not the rescue (the sharp discriminator)
The Gauss back-reaction (`div J = ρ`) does make sequential instantiation **order-dependent** — but both
composites remain **functions of the commuting J**, so a joint distribution over J persists.
**Order-dependence ≠ complementarity.** Concretely, this is the **Leggett-Garg "clumsiness loophole"**:
a classical system whose measurement physically disturbs it (billiard balls) violates LG *without* any
quantum coherence. FTD's back-reaction is exactly classical measurement-disturbance, not incompatible
observables.

### 13c · The phase escape is blocked
The only rescue would be a genuinely *inaccessible* phase. But the substrate's symplectic phase is
**co-measurable**: `[q,p] = 0` to machine zero (~10⁻¹⁶, FTD-0251 `[MEASURED]`), while `{q,p} ≠ 0` is
only the Poisson bracket (a structure on beables, not the observable product). Born-sampling a
*classical* phase yields classical interference-of-probabilities (Spekkens), not quantum amplitude
interference.

### 13d · The positive yield — FTD is a **Spekkens-type epistemic model** (sharpened characterization)
The clean placement this round buys: **commutative ontic substrate (the flux λ = J) + an observer
epistemic restriction (the self-blind-spot / sLoop partial access) = QM *phenomenology* without
genuine non-commutativity** — structurally a Spekkens toy-model regime. This sharpens FTD-0238's
"commutative substrate derives the period algebra; QM/relativity are the imported non-commutative
layer" into a named model class, and it explains *why* FTD's existing `S = 2√2` (`DERIV_SINGLET_FROM_VOID_EVENT`)
needs its two imports — the **complexification** `J → ψ` (`[SELECTION]`) and **superdeterminism**
(measurement-independence dropped, the 't Hooft route, a 6th-postulate-class input) — to clear the Bell
wall. Both are honestly imports, not derivations.

### 13e · The honest split (do not over-credit the survivor)
The conjecture conflated **three independent things**; separated:
- **Stochastic actualization / the arrow** (void → definite): genuinely `[OPEN]` and viable as a
  6th-postulate-class input — FTD-0226 closed only the *deterministic* map, so this door is still open.
- **Born `|·|²` statistics**: **already separately `[CLOSED NEGATIVE]`** (FTD-0200, threshold-crossing
  ↛ Born) — *not* a free residue of stochastic instantiation; must not be claimed as one.
- **Emergent non-commutativity via order-dependence**: `[CLOSED NEGATIVE]` (this round, theorem-level).

This *confirms round 4's type-heterogeneity finding from the other side*: stochasticity (Type-2 choice)
and non-commutativity (Type-1 property) are independent — one cannot purchase the other.

*Round-5 status: **Instantiation → non-commutativity `[CLOSED NEGATIVE within this exploration]`**
(Birkhoff–von Neumann + Fine + FTD-0226/0251); the LG back-reaction route = the clumsiness loophole; the
phase route blocked by `[q,p]=0`. **Positive yield:** FTD characterized as a Spekkens-type epistemic
model (sharpens FTD-0238). **Survivor:** stochastic actualization/the arrow remains `[OPEN]`, but neither
Born statistics (FTD-0200) nor non-commutativity comes free with it. LEDGER row for the non-commutativity
closure (suggested FTD-0254 `[CLOSED NEGATIVE]`) flagged for owner ID-assignment. No canon promoted.*

---

## 14 · Arc terminus (rounds 1–5)

Five conjectures, five adversarial verdicts (kill / kill / scope / kill / theorem-kill), **zero canon
promoted, one canon boundary *refined* (FTD-0253 §7, the two-axiom result)**. The "find the unifying
6th postulate" program is **exhausted**, and the exhaustion is itself the result: the 6th-postulate holes
are **irreducibly plural** — type-heterogeneous (§12), and for the QM hole, theorem-level-separate from
the stochastic-actualization hole (§13). Durable yield, all `[BOUNDARY]`/`[OBSERVATION]`-grade:
1. meaning is **model-conditional** (`I(S;source|M_O)`); FTD has no observer-model object (§7);
2. the Lorentzian metric needs **two** independent axioms — kinematic (relativity principle) + dynamical
   (reversibility) — not one (§11b; now FTD-0253 §7);
3. the five holes are **heterogeneous in type** and do not unify under one frame primitive (§12);
4. FTD is a **Spekkens-type epistemic model** — commutative ontic + epistemic restriction; stochasticity
   does not escape commutativity (§13; sharpens FTD-0238).

The only `[OPEN]` positive probes left are *measurements*, not philosophy: the three-axis isotropy sweep
(§11d) and, independently, the stochastic-actualization/arrow postulate (§13e) — each pre-registerable,
neither claimed.

