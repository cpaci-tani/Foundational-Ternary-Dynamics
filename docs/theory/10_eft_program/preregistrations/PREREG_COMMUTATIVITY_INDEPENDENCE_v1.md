# Pre-Registration — Commutativity Independence No-Go Theorem (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the *design* of a closure
attempt that would upgrade the "commutativity wall" from `[SYNTHESIS]`
(`../../07_assessment/SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md`) to a
forward-proven `[THEOREM]`. It contains **no result**. All three
pre-blessed outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are
admissible; the closure attempt's verdict is genuinely open. The
prior-favoured outcome is **FOUND-for-independence**, but CLOSED-NEGATIVE
is kept genuinely live throughout (and would be the *more* consequential
result — it would mean postulates 1–5 secretly already permit
non-commutativity).

**Date:** 2026-05-30
**Hash-lock target tag:** `preregister-commutativity-independence-v1`
**LEDGER row reservation:** next-free at hash-lock — confirm by grepping
the whole `docs/` tree, not just `LEDGER.md`, immediately before lock. As of
writing, the max LEDGER *row* is FTD-0240; FTD-0241/0242 appear only as text
references (concurrent-session notes), so the first genuinely-free ID is
**FTD-0243**. Re-verify at lock time (the tree is shared by concurrent
sessions).
**Supersedes:** none — first pre-registration of the commutativity
boundary as a forward theorem.
**Companion docs:**
`../../07_assessment/SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md` (the
[SYNTHESIS] this would promote);
`../derivations/THEOREM_A_PHYS_NO_GO.md` (FTD-0059 — the structural
template: ring of derivables → target property absent → external input
required);
`../../07_assessment/core_ledgers/LEDGER.md` (FTD-0199/0200/0208/0225/0226/0227/0228
— the six probes, here corroborating instances, NOT the proof);
the five postulates verbatim in
`../../../../dissemination/manuscript_v2/vol1/src/chapters/01-five-postulates.qmd`
and `../../01_reference/SPEC_FTD_LAGRANGIAN.md`;
`../../03_derivations/quantum_mechanics/DERIV_BELL_COSINE_FROM_GAUSS.md` +
`DERIV_SINGLET_FROM_VOID_EVENT.md` (the emergent S=2√2 result, tagged
`[THEOREM] + [SELECTION]`, the no-go must NOT contradict — F-b);
`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` and
`PREREG_SPIN2_BOUNDARY_THEOREM_v1.md` (the 11-section format this matches);
`PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` (the v1→v2 cautionary
precedent for §11);
`../../../../lean/` (Lean 4 formalization of the **algebraic core** — Claims
A/C, the F-a Poisson≠commutator distinction, and the independence consistency
model; see `lean/README.md`. **Status: ✅ MACHINE-CHECKED (2026-05-30,
toolchain v4.30.0)** — both renderings verify: the Mathlib-free core
`lean/Standalone.lean` (`lean Standalone.lean` → exit 0, axioms
`[propext, Quot.sound]`, no `sorry`) and the canonical Mathlib development
`lean/FtdNoGo/` (`lake build` → "Build completed successfully (8482 jobs)").
It machine-checks the **algebra** — Claim A/C `observable_commutator_zero`,
F-a `poisson_is_not_commutator`, independence `matrix_noncommutative` — NOT
the §3 modeling bridge: that `Config → ℝ` faithfully encodes the five
postulates stays a `[DEFINITION]`).

> **Pre-registration discipline.** Sections §§2–9 are committed before the
> closure attempt is run. After commit: SHA256 → `REF_PREREGISTER_MANIFEST.md`,
> git tag applied. Any post-hoc edit to §§2–9 invalidates v1; a v2 is
> required before the closure attempt is run or re-run. The closure
> attempt's result lands in a separate doc (`FOUND_*`, `AUDIT_*`, or
> `AUDIT_*_CLOSED_NEGATIVE.md`), never as edits to this file.

**Purpose.** Lock, *before* any proof construction, (a) the exact theorem
statement (an **independence** claim, not a strong-forbiddance claim), (b)
the admissible proof inputs, (c) the falsifiers that would sink a candidate
proof — including the two adversarial landmines (the Poisson/symplectic
objection and the Bell S=2√2 consistency obligation), and (d) the
banned-moves list. This is the anti-laundering instrument for a proof whose
conclusion the project already believes — which is exactly when the
temptation to assert rather than prove is highest.

---

## §1 — Context and doctrine

**The wall, as currently held.** The FTD program has closed six separate
results that all point at one absence — the substrate cannot supply the
**non-commutative** structure that quantum mechanics and relativistic time
require:

| # | LEDGER | What it shows | Tag |
|---|---|---|---|
| 1 | FTD-0208 | substrate update is L¹/L∞, not the L² (Pythagorean) metric | [CLOSED NEGATIVE, AXIOM-LEVEL] |
| 2 | FTD-0225 | commutative → type I → trivial Tomita–Takesaki modular flow | [CLOSED NEGATIVE] |
| 3 | FTD-0226 | manifestation map → distributive Boolean event lattice | [CLOSED NEGATIVE] |
| 4 | FTD-0227 | binding epistemic horizon DERIVED; sharpness not | [PARTIAL] |
| 5 | FTD-0228 | N_c=3→ℤ/3 budget symmetry is apophenia (commuting ≠ complementary) | [CLOSED NEGATIVE] |
| 6 | FTD-0199/0200 | substrate gives Rice/Gaussian, not Born | [CLOSED NEGATIVE] |

The synthesis doc recognizes these as **one wall** via the established
equivalence chain: non-commutativity `[q,p]=i` ⟺ L² inner product ⟺
non-distributive (orthomodular) lattice ⟺ SU(2) on complementary
observables ⟺ type III₁ modular algebra ⟹ Bell violation. FTD's substrate
has none of them.

**Why this pre-registration exists.** That recognition is tagged
`[SYNTHESIS]` — it proves "the substrate **as built** is commutative" (six
worked instances), not "the five postulates **do not fix** non-commutativity
**and** any realization is a logically independent addition." The former is
a *map* of the boundary; the latter is a *theorem* sealing it. The gap
between them is the single highest-value open item on the boundary side, and
closing it discharges the second clause of the project's Number-One Goal
("rigorously establish what we cannot [derive]") at theorem grade for the
QM+GR frontier.

**Doctrine.** Per CLAUDE.md scope discipline (Constraint 9) and the
boundary-theorem program (FTD-0186): a closed-negative / boundary result is
a deliverable, not a failure. This pre-reg targets a boundary theorem; its
value is a rigorous verdict in *either* direction. It promotes nothing on
its own — promotion (if FOUND) happens only in a downstream ratification
doc, never here.

---

## §2 — The question (LOCKED)

> **Q-CI.** Let the five postulates (P1 discrete space, P2 discrete time,
> P3 ternary states with **J** primary and *s* the Genesis-threshold
> manifestation, P4 Moore-local causality, P5 determinism) define the
> substrate, with canonical field inventory `(s ∈ {−1,0,+1}, J ∈ ℝ³,
> v, ℒ)`. Let `A₅` be the observable algebra these postulates generate
> (defined in §3).
>
> **(Q1 — Absence)** Does `A₅` contain a non-commuting pair — observables
> `A, B ∈ A₅` with `[A,B] = AB − BA ≠ 0` (equivalently, a non-distributive
> event sub-lattice, Birkhoff–von Neumann)?
>
> **(Q2 — Independence)** If `A₅` is commutative, what is the *minimal*
> additional postulate `M` (a non-commutative measurement / quantization
> map from beables to lab observables) that supplies non-commutativity; and
> is `{P1..P5} ∪ {M}` **consistent** (so that `M` is a genuine logically
> independent postulate, not a contradiction)?

The theorem to be proven (if FOUND) is the conjunction: **`A₅` is
commutative (Q1 = no), AND non-commutativity is supplied only by a
consistent, logically independent postulate `M` (Q2)** — i.e.
non-commutativity is *independent* of the five postulates in the precise
sense the parallel postulate is independent of Euclid's other four.

**Explicitly NOT asked (scope guard, see F-i):** whether *no* extension can
ever produce non-commutativity (strong forbiddance). That is known false
('t Hooft CA: a quantization map can always be bolted on); claiming it
would be scope creep and self-closes negative.

---

## §3 — Definitions (LOCKED)

**D1 — Substrate configuration.** A configuration is the assignment, at a
fixed tick, of `(s(v), J(v), v_wave(v), ℒ(v))` to every voxel `v ∈ Z³`,
with `s ∈ {−1,0,+1}`, `J ∈ ℝ³`, `v_wave = |Δ_t J|/K_B ∈ ℝ≥0`, `ℒ ∈ [0,1)`.
Per P3, `s` is not independent: `s(v) = sign(div J(v))·[‖J(v)‖ > K_B]`
(Genesis threshold). The configuration space is `Ω`.

**D2 — Beable.** A *beable* is a real-valued functional `A: Ω → ℝ` — a
function of the configuration. Examples: `J_x(v)`, `‖J(v)‖²`, `s(v)`, a
Moore-neighborhood sum `Σ_{u∈N^M(v)} J(u)`, a two-tick difference. ('t
Hooft's term; adopted because P5 determinism makes every substrate quantity
a beable.)

**D3 — Observable algebra `A₅`.** `A₅` is the smallest set of beables
containing the generator set `G = {s(v), J_a(v), v_wave(v), ℒ(v) : v ∈ Z³,
a ∈ {x,y,z}}` and closed under the operations the postulates license:
(i) pointwise real-linear combination and pointwise product (P5: beables
are functions on the same `Ω`, so products are pointwise);
(ii) Moore-neighborhood sums/stencils (P4: the only spatial coupling
allowed);
(iii) composition with the one-tick deterministic update map `U: Ω → Ω`
(P2 + P5: `A ∘ U` is a beable whenever `A` is).
`A₅` is a commutative-or-not **associative real algebra under the pointwise
product**; whether it is commutative is exactly Q1.

**D4 — Measurement map `M`.** A *measurement map* is a transform from
beables to a (possibly non-commutative) operator algebra of "lab
observables" — e.g. a choice of complex structure `J_x + iJ_y ↦ ψ` plus a
projection-valued measure, or a 't Hooft template-state basis. `M` is **not**
a beable: it is a choice of *what counts as a measurement*, not fixed by the
configuration or the update rule. The 6th postulate, if needed, is an `M`.

**D5 — Non-commutativity (operational).** Two observables are
*non-commuting* iff `[A,B] ≠ 0`. By Birkhoff–von Neumann (1936), this is
equivalent to: their joint event lattice is **non-distributive**
(orthomodular), equivalently no joint probability distribution over their
simultaneous values exists. A *commutative* algebra ⟺ a **distributive
(Boolean)** event lattice ⟺ a joint distribution always exists ⟺ a
non-contextual hidden-variable model exists. This bridge lets the proof use
the FTD-0226 lattice machinery and connects directly to Kochen–Specker
contextuality.

**D6 — Poisson bracket vs commutator (the F-a distinction, defined here so
the proof cannot blur it).** The leapfrog/symplectic time-update endows the
`(J, Δ_t J)` phase space with a **Poisson bracket** `{·,·}` — an
antisymmetric bilinear derivation on phase-space *functions*. This is a
structure on the space of beables, **not** the algebra's multiplication.
The **commutator** `[A,B] = A·B − B·A` is built from the **pointwise
product** of D3. `A₅` being commutative (Q1) is a statement about the
*pointwise product* — `(A·B)(ω) = A(ω)B(ω) = B(ω)A(ω) = (B·A)(ω)` for all
`ω`, trivially — and is logically independent of `{A,B}` being nonzero. A
candidate proof MUST keep D3-product-commutativity and D6-Poisson-structure
distinct; conflating them fires F-a.

---

## §4 — Admissible proof space (LOCKED)

**The proof MAY use:**
- The five postulates **verbatim** (quoted in §9 step 1 from
  `01-five-postulates.qmd` / `SPEC_FTD_LAGRANGIAN.md`), and the calibration
  declarations (`a_phys ≡ ℓ_P`, `K_B = m_e`).
- Standard mathematics with citation: commutative/von Neumann algebra
  theory, the Gelfand correspondence (commutative C\*-algebra ⟺ functions on
  a space), Birkhoff–von Neumann (1936), Tomita–Takesaki type
  classification, Poisson/symplectic geometry, Kochen–Specker (1967).
- The `THEOREM_A_PHYS_NO_GO.md` proof anatomy (ring of derivables → forbidden
  property → external input) as the structural template.
- The six probe results **as corroborating worked instances only** — to
  show the forward theorem's prediction matches what was found in each case.
  They are NOT load-bearing premises (see F-g).

**The proof MAY NOT use:**
- Any imported measurement basis, complex structure, or quantization map as
  a *premise* (those are the `M` being characterized — the conclusion, not
  an input; B-1, F-c).
- Any QM/QED formula, Hilbert-space postulate, Born rule, or CCR `[q,p]=iℏ`
  as scaffold (B-5, F-d).
- 't Hooft's template transform as a proof step (it is the paradigm `M`; it
  may be *cited* in §9 step 6 as the exemplar 6th postulate, not used to
  prove Q1).
- Numerical near-miss / coincidence scans (CLAUDE.md epistemic discipline).

---

## §5 — Benchmark (LOCKED): the three-claim proof obligation

The proof, to count as FOUND, must establish all of the following, mirroring
`THEOREM_A_PHYS_NO_GO.md` §3 Claims A/B/C plus an independence half:

- **Claim A (commutativity).** Every generator in `G` (D3) is a real-valued
  function on the single configuration space `Ω`; the pointwise product is
  commutative; the closure operations (i)–(iii) preserve
  pointwise-function-hood (a Moore sum of functions is a function; `A∘U` is
  a function). Therefore `A₅` is a **commutative** real algebra. **The F-a
  Poisson/commutator distinction (D6) must be made explicit here** — the
  claim is about the pointwise product, and the nonzero Poisson bracket of
  the symplectic update does not make the product non-commutative.
- **Claim B (target property).** Non-commutativity (the structure QM
  requires) means `∃ A,B: [A,B] ≠ 0` in the observable algebra — equivalently
  (D5) a non-distributive event lattice.
- **Claim C (absence).** No pair in a commutative algebra has `[A,B] ≠ 0`.
  Hence `A₅` cannot host non-commutativity; any non-commuting observable
  requires a generator outside `A₅` — by D4, a measurement map `M`. `∎`(Q1)
- **Independence half (Q2).** Characterize the minimal such `M` precisely
  (e.g. the complex-structure-plus-PVM that the Bell construction's
  [SELECTION] step already uses — see F-b), and **exhibit a model** of
  `{P1..P5} ∪ {M}` that is consistent (e.g. the emergent-QM layer the
  corpus already constructs *is* such a model). Consistency + non-derivability
  (Claim C) = logical independence.

**Benchmark precision.** Claims A–C are algebra-theoretic (no numerical
floor). The independence half requires an *exhibited consistent model*, not
an assertion (F-f).

---

## §6 — The three pre-registered outcomes (LOCKED)

> **FOUND.** Claims A, B, C all go through (with the D6 Poisson distinction
> explicit, so F-a does not fire); the independence half exhibits a
> consistent model of `{P1..P5} ∪ {M}` with `M` characterized precisely; no
> §7 falsifier fires; no §8 banned move is invoked; and the Bell
> consistency cross-check (F-b) confirms the no-go *sharpens* rather than
> contradicts the emergent S=2√2 theorem. **Result:** the commutativity
> independence theorem stands. Downstream (separate ratification doc only):
> the commutativity-wall leg is eligible for `[SYNTHESIS] → [THEOREM]`, and
> the six probes become corollaries; `x₊=1/α` (FTD-0013) is untouched.
>
> **UNDERDETERMINED.** A candidate proof is admissible (no falsifier fires,
> no banned move) but at least one of: the F-a Poisson/commutator distinction
> is not cleanly resolved (the critic's "non-commutativity is one
> deformation-quantization away" is neither refuted nor conceded); or Claim
> A has an unhandled generator/closure case (e.g. an `A∘U` composite whose
> function-hood is unclear); or the independence half asserts rather than
> exhibits a consistent model; or `M` is characterized only up to an unforced
> choice. No tag moves.
>
> **CLOSED-NEGATIVE.** Either (a) a non-commuting pair `[A,B] ≠ 0` **is**
> exhibited from `A₅` (generators + licensed closures) alone — independence
> fails, the postulates already permit non-commutativity, and the
> "commutativity wall" was an artifact of the specific maps probed, not a
> property of the postulates; or (b) the independence consistency half fails
> — every candidate `M` that supplies non-commutativity is *contradictory*
> with P1–P5 (so `M` is not a clean independent postulate but an
> inconsistency), which would itself be a strong and surprising structural
> result. Either sub-outcome is a genuine deliverable and the more
> informative verdict; it does not move the spine.

---

## §7 — Falsifier rules (LOCKED) — F-a..F-j

Each is mechanically checkable against the closure-attempt result doc. If
any fires, the outcome is **not** FOUND (it is UNDERDETERMINED or
CLOSED-NEGATIVE per §6).

- **F-a (Poisson/symplectic objection — the decisive falsifier).** The
  proof MUST keep the D6 distinction explicit: `A₅`-commutativity is a
  property of the **pointwise product** (`A·B − B·A`), while the
  leapfrog/symplectic update carries a **nonzero Poisson bracket**
  `{q,p}=1` on phase-space functions. A Dirac/Moyal-style critic holds that
  non-commutativity is *latent* — deformation quantization turns `{·,·}`
  into `[·,·]/iℏ`, so "the substrate is one ℏ away from non-commuting." The
  falsifier **fires** if the proof (i) conflates the Poisson bracket with
  the observable commutator, OR (ii) proves `A₅` commutative without
  addressing why the nonzero Poisson structure does not already constitute
  the non-commutativity QM needs. The proof must show the deformation
  parameter (ℏ) is itself an external import — i.e. the map
  `{·,·} ↦ [·,·]/iℏ` is an `M` (D4), not a beable — so that latent Poisson
  structure does not satisfy Q1 without `M`. (The corpus states the bare
  distinction in `scripts/proofs/proof_modular_time_algebra_type.py`
  lines 13–18: "classical symplectic/Poisson J, but the observable product
  is commutative — not a CCR algebra." The proof must *formalize* this, not
  merely cite it.)
- **F-b (Bell S=2√2 consistency obligation).** The corpus carries an
  **emergent** Tsirelson result S=2√2
  (`DERIV_BELL_COSINE_FROM_GAUSS.md`, `DERIV_SINGLET_FROM_VOID_EVENT.md`),
  tagged **`[THEOREM] + [SELECTION]`** — the Gauss→cosine link is [THEOREM],
  the identification with quantum measurement is [SELECTION]; substrate-level
  is S≤2. **That [SELECTION] half is precisely the measurement-map import `M`
  this no-go characterizes** — so F-b is not just a no-collision check, it is
  the positive bridge: the no-go must land the emergent layer's complexification
  on the [SELECTION] side, where the Bell doc already places it. The no-go MUST be **consistent** with both: the
  two-level reading is "S≤2 at the substrate (commutative, this theorem's
  Q1=no), S=2√2 at the emergent layer (via the complexification
  `J_x+iJ_y=ψ` + Gauss-constraint coarse-graining, tagged [SELECTION])."
  The falsifier **fires** if the proof (i) would force S≤2 at the *emergent*
  level (contradicting the theorem-tagged result), OR (ii) fails to identify
  the emergent layer's [SELECTION] complexification step as *exactly* an
  instance of the measurement map `M` (D4). The no-go must *sharpen* the
  S=2√2 theorem (locating its import), not collide with it.
- **F-c (no measurement basis as premise).** Any candidate `M` (complex
  structure, PVM, lab basis, template transform) appearing as a *premise* of
  Claim A/B/C fires the falsifier. `M` may appear only in §9 step 6–7 as the
  object being characterized/exhibited, never as a proof input.
- **F-d (no QM/QED scaffold).** Use of a Hilbert space, Born rule, CCR
  `[q,p]=iℏ`, Schrödinger/Dirac equation, or any QED formula as a load-bearing
  step fires the falsifier.
- **F-e (full generator coverage).** Claim A must cover **all** generators
  `{s, J_a, v_wave, ℒ}` and their licensed closures (pointwise products,
  Moore sums, `∘U` composites), not just `J`. A proof that establishes
  commutativity only for the flux sub-algebra fires the falsifier (the
  FTD-0226 instance covered the manifestation map; this theorem must cover
  the whole `A₅`).
- **F-f (independence is exhibited, not asserted).** The Q2 independence
  half must **exhibit a consistent model** of `{P1..P5} ∪ {M}` (e.g. the
  emergent-QM construction the corpus already builds). A bare assertion
  that "`M` is consistent" without a model fires the falsifier.
- **F-g (probes corroborate, they do not prove).** If any of Claim A/B/C
  rests *logically* on FTD-0225/0226/0228/0199/0200/0208 (rather than on the
  forward postulate→`A₅` construction), the falsifier fires. The probes are
  worked instances that must *match* the theorem's prediction, not premises
  of it.
- **F-h (Birkhoff–von Neumann bridge correctness).** The D5 equivalence
  (commutative ⟺ distributive Boolean ⟺ joint distribution exists ⟺
  non-contextual) must be cited and applied correctly. Misuse (e.g.
  claiming distributivity implies a *unique* hidden-variable model, or
  conflating contextuality with non-locality) fires the falsifier.
- **F-i (no strong-forbiddance scope creep).** Any claim, anywhere in the
  result doc, that *no extension can ever* produce non-commutativity (rather
  than the independence claim that the five postulates do not *fix* it and
  `M` is a consistent independent addition) fires the falsifier. The theorem
  is independence, full stop.
- **F-j (result lands in a separate doc).** Any edit of §§2–9 of this
  pre-reg after hash-lock, or recording the verdict inside this file rather
  than in a separate `FOUND_*`/`AUDIT_*` doc, fires the falsifier and
  invalidates v1 (a v2 is then required).

---

## §8 — Banned moves / anti-laundering (LOCKED) — B-1..B-12

- **B-1.** No imported measurement basis / complex structure / quantization
  map / 't Hooft template as a premise (it is the conclusion `M`).
- **B-2.** No new free integer, exponent, finite group, or coefficient
  introduced to make the proof go through.
- **B-3.** No reverse-engineering from "QM is non-commutative, therefore the
  postulates must permit it" — forward direction only (postulates → `A₅` →
  commutativity → `M` required).
- **B-4.** No appeal to "QM/GR is the goal, therefore the wall must be a
  theorem" (assertion, not proof).
- **B-5.** No QM/QED formula import as scaffold (Hilbert space, Born, CCR,
  beta function, scattering normalization).
- **B-6.** No conflation of the Poisson bracket with the observable
  commutator (this is B-grade as well as F-a, because it is the single most
  likely laundering route).
- **B-7.** No superdeterminism / Bell-locality hand-wave standing in for the
  Q2 consistency *model* (a model must be exhibited, F-f).
- **B-8.** No numerical near-miss / coincidence scan anywhere (CLAUDE.md).
- **B-9.** No claim that the emergent S=2√2 result is *wrong* or *demoted*;
  it must be preserved and sharpened (F-b).
- **B-10.** No retroactive editing of this pre-reg; v2 required if a
  definition/falsifier proves defective (precedent: FTD-0186 v1→v2).
- **B-11.** No spine tag moves (`[THEOREM]` spine, `x₊=1/α` [SMC]) in the
  result doc; any promotion of the wall leg happens only in a separate
  ratification doc after FOUND.
- **B-12.** CLOSED-NEGATIVE stays a live option throughout; engineering the
  proof toward FOUND is a process violation. (CLOSED-NEGATIVE here is the
  *more* interesting outcome — treat it as the genuine target, not the
  failure mode.)

---

## §9 — Method (LOCKED) — 11 ordered steps

Run **only** against the hash-locked commit, in this order. Do not reorder
(in particular: run the F-a..F-j falsifier checklist at step 8, before
declaring any verdict at step 11).

1. **Quote the five postulates + field inventory** verbatim from
   `01-five-postulates.qmd` / `SPEC_FTD_LAGRANGIAN.md`. Fix `Ω`, `G`, `U`.
2. **Construct `A₅`** explicitly: list the generators `G`; state the three
   closure operations (D3 i–iii); confirm closure is well-defined.
3. **Prove Claim A (commutativity)** — pointwise product on functions over
   `Ω` is commutative; closures preserve function-hood. **Discharge the F-a
   Poisson/commutator distinction (D6) explicitly here**: show the symplectic
   update's nonzero Poisson bracket is a structure on beables, not the
   algebra product, and that turning it into a commutator requires the
   external deformation map `M`.
4. **State Claim B (target property)** — non-commutativity = `∃[A,B]≠0` =
   non-distributive lattice (D5).
5. **Prove Claim C (absence)** — abelian algebra ⟹ no `[A,B]≠0` ⟹ `A₅`
   cannot host it ⟹ non-commuting observable requires an `M ∉ A₅`. `∎`(Q1)
6. **Characterize the minimal `M`** — the smallest non-commutative
   measurement map (D4) supplying `[A,B]≠0`; identify it with the emergent
   layer's [SELECTION] complexification (F-b) and note 't Hooft's template
   transform as the paradigm exemplar.
7. **Exhibit consistency of `{P1..P5} ∪ {M}`** — present a concrete model
   (the emergent-QM construction the corpus already builds is the natural
   candidate). Consistency + Claim C = independence. (F-f.)
8. **Run the F-a..F-j falsifier checklist** — record each as fired / not
   fired with one-line evidence.
9. **Run the B-1..B-12 banned-moves checklist** — record none invoked.
10. **Bell-consistency cross-check (F-b)** — confirm S≤2 substrate / S=2√2
    emergent is preserved and that the emergent [SELECTION] step is the `M`.
11. **Write the verdict** (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE) in a
    **separate** result doc (`FOUND_COMMUTATIVITY_INDEPENDENCE.md` or
    `AUDIT_COMMUTATIVITY_INDEPENDENCE_*.md`), with an adversarial-review pass
    by an independent agent (no project priors) per the FTD-0186 precedent.

---

## §10 — What this pre-registration locks vs leaves open

**Locked (§§2–9):** the question (independence, not forbiddance); the
definitions of `A₅`, beable, `M`, the Poisson/commutator distinction; the
three-claim benchmark; the three outcomes; the F-a..F-j falsifiers (incl.
the Poisson and Bell landmines); the B-1..B-12 banned moves; the 11-step
method.

**Left open (the genuine verdict):** whether Claim A holds for *all* of
`A₅` (the `∘U` composites are the least-trivial case); whether the F-a
Poisson objection is cleanly dischargeable (this is the crux — if not, the
outcome is UNDERDETERMINED); whether a consistent `M`-model can be exhibited
(F-f); and therefore which of the three outcomes lands. Prior-favoured is
FOUND-for-independence, but the pre-reg is indifferent — a CLOSED-NEGATIVE
(the postulates secretly permit non-commutativity) would be the more
consequential result and is kept fully live (B-12).

**Not in scope:** strong forbiddance (F-i); any spine retag (B-11); any
engine measurement (this is a pure-math closure attempt); the GR-side
spin-2 boundary theorem (FTD-0209, a separate already-FOUND result this
must not contradict — cross-checked, not re-proven).

---

## §11 — Hash-lock protocol

1. Finalise §§1–11. Compute SHA256:
   ```sh
   sha256sum docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md
   ```
2. Record the SHA256 + tag in `REF_PREREGISTER_MANIFEST.md` (following the
   existing FTD-0186/0190/0191/0192 entries) and add a `[PRE-REGISTRATION]`
   row to `../../07_assessment/core_ledgers/LEDGER.md` at the next-free
   FTD-ID (grep the whole `docs/` tree first — concurrent sessions have
   claimed through FTD-0241).
3. Commit + lightweight tag:
   ```sh
   git commit docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md
   git tag preregister-commutativity-independence-v1 \
       -m "Pre-reg: commutativity independence no-go (QM+GR boundary)"
   ```
4. The closure attempt runs ONLY against the tagged commit; its verdict
   lands in a separate result doc (§9 step 11), never as edits here.
5. If a definition or falsifier proves defective during the attempt, issue
   `PREREG_COMMUTATIVITY_INDEPENDENCE_v2.md` (do not edit v1) — FTD-0186
   v1→v2 precedent.
6. Verify tag integrity:
   ```sh
   git rev-list -n1 preregister-commutativity-independence-v1
   git tag -l preregister-commutativity-independence-v1
   ```

---

## §12 — Single-line summary

A pre-registered, falsifier-gated design to prove (or refute) that
**non-commutativity is logically independent of FTD's five postulates** —
the substrate's observable algebra `A₅` is commutative, and quantum/relativistic
non-commutativity enters only through a consistent, separately-characterized
6th postulate — thereby sealing the QM+GR "commutativity wall" from
`[SYNTHESIS]` to `[THEOREM]`, with the Poisson-bracket and Bell S=2√2
objections pre-named as the decisive falsifiers.
