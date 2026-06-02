# Pre-Registration — Commutativity Derivation (whole-A₅ generalization of FTD-0226) (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the *design* of a closure
attempt that would close the one assumption the companion
`PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` leaves open: its **D3(i)** "the
observable product is pointwise (P5)." It contains **no result**. All three
pre-blessed outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are
admissible; the verdict is genuinely open. The prior-favoured outcome is
**FOUND-for-substrate-commutativity** (FTD-0226 already established the
manifestation-map case), but CLOSED-NEGATIVE is kept genuinely live (it would
mean the D3 closure operations secretly break the function-of-J property).

**Date:** 2026-05-30
**Hash-lock target tag:** `preregister-commutativity-derivation-v1`
**LEDGER row reservation:** **FTD-0244** (verified first-free across the whole
`docs/` tree at writing; re-verify at lock time — the tree is shared by
concurrent sessions). The companion `PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md`
is to be registered at FTD-0246 in the same pass — it currently has no LEDGER
row (FTD-0245 is taken by a concurrent session's Mechanism-B doc).
**Companion docs:**
`PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` (the no-go whose D3(i) assumption
this closes; its §3 definitions D1–D6 are imported verbatim below);
`../archive/closed_negative/AUDIT_MANIFESTATION_NONCOMMUTATIVITY_CLOSED_NEGATIVE.md`
(FTD-0226, the worked base case — manifestation map is a function of J →
Boolean → classical, Python-verified 5/5 via
`../../../../scripts/proofs/proof_manifestation_noncommutativity.py`);
`../../01_reference/SPEC_FTD_LAGRANGIAN.md` §3.4/§3.6/§4.2 (the canonical
genesis-threshold + Gauss-constraint dynamics this derives commutativity
*from*);
`../../../../lean/FtdNoGo/Closure.lean` + `../../../../lean/Standalone.lean`
(Part-A machine-checked carrier + closure theorems — the formal substrate);
`engine/tests/test_observable_commutativity.cpp` (Part-C engine-level
M-localization — empirical corroboration).

> **Pre-registration discipline.** Sections §§2–9 are committed before the
> closure attempt is run. After commit: SHA256 → `REF_PREREGISTER_MANIFEST.md`,
> git tag applied. Any post-hoc edit to §§2–9 invalidates v1; a v2 is
> required. The closure verdict lands in a separate doc
> (`FOUND_*`/`AUDIT_*`), never as edits to this file. **Design-lock only** —
> no closure attempt is run in the same session as the lock.

**Purpose.** Lock, before any proof construction, what would count as a
*derivation* (rather than a positing) of the pointwise-product structure on
the **entire** observable algebra A₅ — generalizing FTD-0226 from the single
manifestation map to all of A₅ — and what would falsify any candidate.

---

## §1 — Context and doctrine

**The gap this closes.** `PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` proves
A₅ is commutative *given* D3(i): "products are pointwise (P5)." That is an
assumption, not a derivation — and the FTD-lead audit (this session) noted
positing it is logically equivalent to the commutativity conclusion, so it
cannot be closed by more of the *same* proof. It can only be closed by
*deriving* the pointwise structure from the substrate dynamics.

**What is already done (not re-derived here).** FTD-0226
(`AUDIT_MANIFESTATION_NONCOMMUTATIVITY_CLOSED_NEGATIVE.md`, Python 5/5) already
derived, for the **manifestation map** specifically: it is a deterministic
function of the commuting flux configuration J → its observables are functions
of J → Boolean lattice → joint distribution exists → classical. Its Step 1
("function of J") = the carrier; Step 2 ("functions of a common variable
commute") = pointwise-product commutativity. This pre-reg does **not** redo
that; FTD-0226 is the **cited worked base case** (a worked instance per the
independence pre-reg's F-g, not a premise the proof rests on).

**What is new (the generalization).** FTD-0226 covers one map. The
independence pre-reg's A₅ (D3) is generated from *all* substrate-field
generators closed under pointwise +/×, Moore-neighbourhood sums (P4), and
composition with the deterministic update U (P2+P5). The open question is
whether the *whole generated algebra* — not just the manifestation map —
inherits the function-of-J property, hence commutativity. The genuinely-new
content is the **`∘U`-closure step**: that composition with the deterministic
evolution preserves function-of-J-hood across arbitrary time.

**Doctrine.** Per CLAUDE.md scope discipline (Constraint 9) and the
boundary-theorem program (FTD-0186): a closed-negative / boundary result is a
deliverable. This pre-reg promotes nothing on its own; promotion (if FOUND)
happens only in a downstream ratification doc.

---

## §2 — The question (LOCKED)

> **Q-CD.** Import the five postulates and the field inventory of
> `PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` §2, and its A₅ definition (§3 D3).
> Given that FTD-0226 established the function-of-J property for the
> manifestation map:
>
> **(Q1 — Closure of D3(i))** Do the D3 closure operations — pointwise
> real-linear combination + product (i), Moore-neighbourhood sums (ii), and
> composition with the deterministic update `U` (iii) — *preserve* the
> "function of the single configuration J" property, so that **every** element
> of A₅ (not just the manifestation map) is a function of J on the common
> sample space Ω_J, hence the pointwise product on A₅ is *forced* (not posited)
> and A₅ is commutative?
>
> **(Q2 — M-localization)** Is the genesis/Gauss **state-mutation** the *sole*
> locus at which sequential operations fail to commute — so that any
> non-commutativity must enter through a measurement map `M ∉ A₅` (D4), exactly
> the candidate 6th postulate the independence no-go isolates?

The theorem to be proven (if FOUND) is the conjunction: **the D3(i) pointwise
product is forced by the function-of-J property under all D3 closures (Q1),
and M is localized to the genesis/Gauss state-mutation (Q2)** — closing the
independence pre-reg's D3(i) assumption by derivation.

**Explicitly NOT asked (scope guard):** whether nature is commutative beneath
measurement (Level 4 — empirically undecidable; the imported-M reading
reproduces QM by construction). FOUND certifies the *substrate* algebra, not
the world.

---

## §3 — Definitions (LOCKED)

Definitions **D1–D6 are imported verbatim** from
`PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` §3 (substrate configuration with the
genesis threshold `s = sign(div J)·[‖J‖>K_B]`; beable; A₅ via the D3 closure
operations; measurement map M; non-commutativity via Birkhoff–von Neumann;
the D6 Poisson-bracket-vs-commutator distinction). Two derivation-specific
definitions are added:

**D7 — Function-of-J property.** A beable `A` *has the function-of-J property*
iff there is a function `f_A : Ω_J → ℝ` on the flux-configuration sample space
Ω_J with `A = f_A ∘ (config ↦ J)`. (FTD-0226 established this for the
manifestation observables; the genesis threshold makes `s` itself a function
of J, so all generators have it.)

**D8 — Closure-preservation.** A D3 operation *preserves* the function-of-J
property iff, applied to operands with the property, its result has the
property: pointwise `f+g`, `f·g`, finite Moore sums, and `f ∘ U` (where
`U : Ω→Ω` is the deterministic update, P2+P5, hence itself a function) are all
functions on Ω_J. Q1 is exactly: every D3 operation preserves D7.

---

## §4 — Admissible proof space (LOCKED)

**The proof MAY use:**
- D1–D8; the five postulates verbatim; the calibration declarations.
- The canonical dynamics as the *source* commutativity is derived from:
  `SPEC_FTD_LAGRANGIAN.md` §3.4 (genesis threshold / K_B), §3.6 (state-flux
  coupling term, Gauss-constraint term), §4.2 (Poisson equation from the
  action). The dynamics are an INPUT (what we derive *from*), not a premise to
  be smuggled past F-circular.
- FTD-0226 as the cited base case (worked instance, not a load-bearing premise
  — F-0226-consistency).
- Standard mathematics: commutative-algebra/Gelfand correspondence,
  Birkhoff–von Neumann (1936), Kochen–Specker (1967).
- The Part-A Lean theorems (`observable_commutator_zero_under_update`,
  `observable_closure_commutes`) as the formal carrier/closure substrate, and
  the Part-C engine result as empirical corroboration.

**The proof MAY NOT use:**
- D3(i) pointwise product as a *premise* (that is what must be derived;
  F-circular).
- Any imported measurement basis / complex structure / quantization map as a
  premise (that is M, the conclusion).
- Any QM/QED formula, Hilbert space, Born rule, or CCR `[q,p]=iℏ` as scaffold.
- The Poisson/symplectic bracket as if it were the observable commutator
  (F-a / D6).

---

## §5 — Benchmark (LOCKED): the proof obligation

Generalize FTD-0226's Step 1–2 from `{manifestation observables}` to
`{A₅ generated by D3}`:

- **Claim A′ (all generators have D7).** Each generator `{s, J_a, v_wave, ℒ}`
  is a function of J: J_a trivially; `s` via the genesis threshold (D1); v_wave
  and ℒ via their definitions as functions of J/its differences. (FTD-0226
  established this for the manifestation observables.)
- **Claim B′ (D3 ops preserve D7 — the new content).** Each D3 closure
  operation preserves the function-of-J property (D8): pointwise +/× and finite
  Moore sums of functions on Ω_J are functions on Ω_J (immediate); and `A ∘ U`
  is a function on Ω_J because `U : Ω→Ω` is itself a deterministic function
  (P2+P5) — **this `∘U` step is the genuinely-new obligation beyond FTD-0226**.
- **Claim C′ (forced commutativity).** By A′ + B′, every element of A₅ is a
  function on the common Ω_J; functions on a common space commute under
  pointwise product (FTD-0226 Step 2, now quantified over all of A₅); hence the
  pointwise product is *forced*, not posited, and A₅ is commutative. This
  *derives* the independence pre-reg's D3(i).
- **M-localization (Q2).** Exhibit that the genesis/Gauss operations are
  *state-mutating* (they change J, hence change subsequent reads — confirmed
  empirically by Part C), so they are the sole locus of ordering-dependence;
  any non-commutativity is therefore an M ∉ A₅.

No numerical floor (algebra-theoretic). The M-localization half must be
non-vacuous (the genesis/Gauss mutation must be exhibited as genuine, not
assumed).

---

## §6 — The three pre-registered outcomes (LOCKED)

> **FOUND.** Claims A′, B′, C′ all go through (with the D6 Poisson distinction
> explicit, so F-a does not fire; the `∘U`-closure step explicit, so F-vacuity
> does not fire); the M-localization half exhibits the genesis/Gauss mutation
> as the sole ordering-dependence locus; no §7 falsifier fires; no §8 banned
> move is invoked; and FTD-0226 is generalized, not contradicted (F-0226).
> **Result:** the independence pre-reg's D3(i) is *derived*, not posited — the
> substrate algebra A₅ is commutative as a consequence of the function-of-J
> dynamics. Downstream (separate ratification doc only): the
> commutativity-derivation leg is eligible to upgrade the D3(i) `[DEFINITION]`
> to `[DERIVED]`; `x₊=1/α` (FTD-0013) is untouched; Level 4 stays undecidable.
>
> **UNDERDETERMINED.** A candidate proof is admissible (no falsifier fires, no
> banned move) but at least one of: the `∘U`-closure (Claim B′) has an
> unhandled case (e.g. a composite whose function-of-J-hood is unclear); or the
> F-a Poisson/commutator distinction is not cleanly resolved; or the
> M-localization asserts rather than exhibits the genesis/Gauss mutation as the
> sole locus; or the function-of-J property holds only up to an unforced choice.
> No tag moves.
>
> **CLOSED-NEGATIVE.** Either (a) a D3 closure operation is exhibited that
> **breaks** the function-of-J property — producing an A₅ element that is *not*
> a function of J, hence a candidate non-commuting pair from A₅ alone (D3(i)
> would then be false, and the independence no-go's commutativity claim would
> itself be threatened); or (b) the M-localization fails — non-commutativity is
> shown to enter at a locus *inside* A₅ rather than only through an external M.
> Either sub-outcome is a genuine deliverable and the more consequential
> verdict (it would mean the substrate is subtler than FTD-0226 suggests); it
> does not move the spine.

---

## §7 — Falsifier rules (LOCKED) — F-a..F-f

- **F-circular (decisive).** The proof must NOT posit D3(i) (pointwise
  product) anywhere; it must *derive* the pointwise structure from the
  function-of-J property (D7) + closure-preservation (D8). Asserting D3(i)
  fires the falsifier — this is the entire point of the derivation.
- **F-a (carry over from the independence pre-reg).** The proof must keep the
  D6 Poisson-bracket ≠ observable-commutator distinction explicit; treating
  the nonzero symplectic bracket as observable non-commutativity fires it
  (deformation/ℏ is an external M).
- **F-0226-consistency.** The proof must *generalize* FTD-0226, not contradict
  it, and must use it as a cited base case (worked instance) — not rest the
  whole derivation logically on it (that would make the generalization vacuous;
  F-g of the independence pre-reg).
- **F-vacuity.** The `∘U`-closure step (Claim B′, the genuinely-new content)
  must be discharged explicitly. A proof that only restates the
  manifestation-map case (FTD-0226) without the whole-A₅ closure is vacuous and
  fires the falsifier.
- **F-Mloc.** The M-localization half (Q2) must *exhibit* the genesis/Gauss
  state-mutation as genuine (Part C is the empirical witness), not assume it.
  Asserting M's locus without exhibiting the mutation fires it.
- **F-level4.** Any claim that the derivation certifies nature as commutative
  beneath measurement (Level 4) fires the falsifier; FOUND certifies the
  substrate algebra only.

---

## §8 — Banned moves / anti-laundering (LOCKED) — B-1..B-8

- **B-1.** No D3(i) pointwise product as a premise (it is the conclusion).
- **B-2.** No imported measurement basis / complex structure / quantization
  map / 't Hooft template as a premise (that is M).
- **B-3.** No QM/QED scaffold (Hilbert space, Born, CCR, beta function).
- **B-4.** No conflation of the Poisson bracket with the observable commutator
  (also F-a).
- **B-5.** No "QM is the goal, therefore D3(i) must hold" (assertion).
- **B-6.** No numerical near-miss / coincidence scan (CLAUDE.md).
- **B-7.** No retroactive edit of this pre-reg; v2 required if a
  definition/falsifier proves defective (FTD-0186 v1→v2 precedent).
- **B-8.** CLOSED-NEGATIVE stays a live option throughout; engineering toward
  FOUND is a process violation. No spine tag moves in the result doc; the D3(i)
  `[DEFINITION]→[DERIVED]` upgrade happens only in a downstream ratification
  doc after FOUND.

---

## §9 — Method (LOCKED) — ordered steps

Run **only** against the hash-locked commit, in order. Do not reorder (run the
F-checklist at step 6, before any verdict at step 8).

1. Import D1–D6 (independence pre-reg §3) + D7–D8; quote the five postulates
   and the canonical dynamics (`SPEC_FTD_LAGRANGIAN.md` §3.4/§3.6/§4.2).
2. **Claim A′:** show each generator has the function-of-J property (D7);
   cite FTD-0226 for the manifestation observables.
3. **Claim B′:** show each D3 operation preserves D7 (D8) — pointwise +/×,
   Moore sums (immediate), and `∘U` (the new content: U is a function, P2+P5).
4. **Claim C′:** conclude the pointwise product is forced and A₅ commutes
   (FTD-0226 Step 2 over all of A₅). This derives D3(i).
5. **M-localization:** exhibit the genesis/Gauss state-mutation as the sole
   ordering-dependence locus (Part C engine result is the witness); conclude
   any non-commutativity is an M ∉ A₅.
6. Run the F-circular..F-level4 checklist — record each fired/not-fired.
7. Run the B-1..B-8 banned-moves checklist — record none invoked.
8. Write the verdict (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE) in a
   **separate** result doc, with an independent adversarial-review pass (no
   project priors), per the FTD-0186 precedent.

---

## §10 — What this pre-registration locks vs leaves open

**Locked (§§2–9):** the question (derive D3(i), localize M); D7/D8; the
A′/B′/C′ benchmark; the three outcomes; the F-circular..F-level4 falsifiers
(with the `∘U`-closure and M-localization as the load-bearing new obligations);
the B-1..B-8 banned moves; the method.

**Left open (the genuine verdict):** whether the `∘U`-closure (Claim B′) holds
without an unhandled case; whether the M-localization is cleanly exhibitable;
hence which of the three outcomes lands. Prior-favoured: FOUND (FTD-0226 +
Part A + Part C all point that way), but CLOSED-NEGATIVE is kept fully live.

**Honest ceiling (LOCKED).** Even FOUND certifies that the *substrate* algebra
A₅ is commutative as a derived consequence of the function-of-J dynamics, and
that M is the sole non-commutativity locus. It does **NOT** certify that nature
is commutative beneath measurement (Level 4, empirically undecidable — the
imported-M reading reproduces QM by construction). The Part-A Lean theorems +
Part-C engine result + FTD-0226 are the proof substrate this pre-reg locks
criteria for; the closure verdict is a downstream artifact.

---

## §11 — Hash-lock protocol

1. Finalise §§1–11. Compute SHA256:
   ```sh
   sha256sum docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_DERIVATION_v1.md
   ```
2. Record SHA256 + tag in `REF_PREREGISTER_MANIFEST.md`; add a
   `[PRE-REGISTRATION]` row to `../../07_assessment/core_ledgers/LEDGER.md` at
   the next-free FTD-ID (FTD-0244 at writing; re-grep the whole `docs/` tree
   first — the tree is shared). Register the companion
   `PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` at the next ID (FTD-0245) in the
   same pass.
3. Commit + lightweight tag:
   ```sh
   git commit docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_DERIVATION_v1.md
   git tag preregister-commutativity-derivation-v1 \
       -m "Pre-reg: derive D3(i) commutativity over all of A5 (generalizes FTD-0226)"
   ```
4. The closure attempt runs ONLY against the tagged commit; verdict lands in a
   separate result doc (§9 step 8), never as edits here.
5. If a definition/falsifier proves defective, issue
   `PREREG_COMMUTATIVITY_DERIVATION_v2.md` (do not edit v1) — FTD-0186 v1→v2
   precedent.
6. Verify tag integrity:
   ```sh
   git rev-list -n1 preregister-commutativity-derivation-v1
   git tag -l preregister-commutativity-derivation-v1
   ```

---

## §12 — Single-line summary

A pre-registered, falsifier-gated design to **derive** (not posit) the
pointwise-product commutativity of the whole observable algebra A₅ — closing
`PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md`'s D3(i) assumption by generalizing
FTD-0226 from the manifestation map to all of A₅ via the deterministic-update
`∘U`-closure, and localizing the measurement map M to the genesis/Gauss
state-mutation — with the circularity, Poisson, FTD-0226-consistency, vacuity,
and Level-4 objections pre-named as falsifiers. Design-lock only; the substrate
algebra, not Level-4 nature, is what FOUND would certify.
