# Pre-Registration — Readout-Structure Independence Theorem (MC-T4.3 boundary) v1

**Tag:** [PRE-REGISTRATION] — this document locks the *design* of a closure
attempt that would establish, at `[THEOREM]` grade, that the master-quadratic
**operator structure** `(Tr, Det) = (16G*², 16G*³)` is **logically independent**
of FTD's five postulates + algebraic spine + cubic (`O_h`) representation theory.
It contains **no verdict**. All three pre-blessed outcomes —
INDEPENDENT / FORCED / UNDERDETERMINED — are admissible; the verdict is genuinely
open. The prior-favoured outcome is **INDEPENDENT-as-boundary-theorem**, but
**FORCED** is kept fully live throughout (B-12) and would be the *more*
consequential result — it would re-open MC-T4.3 positive and redirect to the
Mechanism-B / ARC-D route.

**Date:** 2026-05-31
**Hash-lock target tag:** `preregister-readout-structure-independence-v1`
**LEDGER row reservation:** *to assign at hash-lock — next genuinely-free id.*
Grep the **whole `docs/` tree** (not just `LEDGER.md`) immediately before lock;
concurrent sessions have provisionally claimed through ~FTD-0243 (the 0238–0240
range is contended; see `../scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md` §7).
Do **not** register a LEDGER row until the id is confirmed free.
**Supersedes:** none — first pre-registration of the readout *operator-structure*
boundary as a forward independence theorem (distinct from the physical-boundary
ARC-A1 closure attempt, `PREREG_ALPHA_READOUT_BOUNDARY_v2.md`, despite the name).
**Companion docs:**
- `../scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md` (FTD-0240 — the scoping memo
  this operationalizes; its Obligation C "co-realizability" is this theorem's core).
- `../derivations/THEOREM_A_PHYS_NO_GO.md` (FTD-0059 — the structural template:
  ring of derivables → target property absent/unforced → external input required).
- `PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` (the **sibling** independence pre-reg
  for the QM non-commutativity wall; this one matches its 11-section format and
  must not contradict it — see §1 on the distinction).
- `../../07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`
  (FTD-0235 — the standing UNDERDETERMINED verdict + its V1–V7 falsifiers, inherited).
- `../../07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`
  (FTD-0234 — the clean odd source det_ζ ratio = G*).
- `../../09_mathematical/number_theory/EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md`
  (FTD-0237 — the even-power wall `E₆(i)=0` + no-Eisenstein-twin).
- `../../03_derivations/DERIV_BCC_COMPLEX_STRUCTURE.md` (FTD-0122 — `V_complex ≅ ℤ[i]²`).
- `../../../scripts/proofs/proof_readout_multE_zero.py` (verifies the trace-leg
  Lemma below: `mult_O(E)=0` on the 8-corner module; **machine-checked**, this commit).

> **Pre-registration discipline.** §§2–9 are committed before the closure attempt
> is run. After commit: SHA256 → `../REF_PREREGISTER_MANIFEST.md`, git tag applied.
> Any post-hoc edit to §§2–9 invalidates v1; a v2 is required. The closure
> attempt's verdict lands in a separate doc (`FOUND_*`, `AUDIT_*`, or
> `AUDIT_*_CLOSED_NEGATIVE.md`), never as edits here.

**Purpose.** Lock, *before* the proof is assembled, (a) the exact theorem
statement (an **independence** claim, not a strong-forbiddance claim), (b) what is
already discharged versus the genuinely-open obligations, (c) the falsifiers that
sink a candidate proof — including the FORCED-escape that adversarial review has
not been able to construct but has not been *proven* impossible, and (d) the
banned-moves list. This is the anti-laundering instrument for a result the project
is now inclined to believe — exactly when the temptation to assert rather than
prove is highest, and exactly the failure mode (the 2026-05-30 "scalar fixed-point"
facade, falsified) this pre-reg exists to prevent.

---

## §1 — Context and doctrine

**The wall, as currently held.** MC-T4.3 — the operational α-readout — is a
`[FOUNDATIONAL OBSTRUCTION]` (LEDGER FTD-0224, W-CRIT-2). The readout is modelled
as a `2×2` transfer operator `T` on the BCC complex plane `V_complex ≅ ℤ[i]²`
(FTD-0122), and the master quadratic is its characteristic polynomial,
`det(xI − T) = x² − (Tr T) x + (det T)` with `(Tr T, det T) = (16G*², 16G*³)`.
Five probe results map the boundary:

| # | LEDGER | What it shows | Tag |
|---|---|---|---|
| 1 | FTD-0230 (ARC-B2) | the trace `16G*²` (even term) is forward-derivable | [UNDERDETERMINED — det unforced] |
| 2 | FTD-0231 (ARC-C1) | charge quantization gives the `ℤ[i]` structure via `O_h→C₄` | [UNDERDETERMINED] |
| 3 | FTD-0233 | `G*`-degree parity (scoped) | [CLOSED NEGATIVE — scoped] |
| 4 | FTD-0234 | J-twisted det_ζ ratio `= G*` is a clean forward **odd** scalar (degree 1) | [THEOREM] |
| 5 | FTD-0235 | `det↔det_ζ` identity; `(Tr,Det)` independent for a `2×2` | [UNDERDETERMINED] |

**Why this pre-registration exists.** Those five are a *map* of the boundary
(worked instances), not a *theorem* sealing it. The standing verdict is
UNDERDETERMINED: "the scalars `{16, G*, G*²}` are all forward-computable, the
assembly `16G*³ = 16G*²·G*` is *possible*, but *why the readout operator carries
that specific `(Tr,Det)` pair rather than any other pair built from the same
scalars* is the imposed master quadratic (W-CRIT-2), unforced." The gap between
"unforced on present evidence" (a prior) and "**provably** unforced — and any
realization is a logically independent addition" (a theorem) is the single
highest-leverage item on the boundary side. Closing it discharges clause 2 of
the project's Number-One Goal ("rigorously establish what we cannot [derive]") at
theorem grade for the EM sector and converts the tracked open item W-CRIT-2 from
`[OPEN methodological]` to a stated boundary theorem.

**Distinction from the commutativity-independence sibling (do not conflate).**
`PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` seals the **QM** wall: the substrate's
observable algebra `A₅` lacks *observable* non-commutativity `[A,B]≠0`. This
theorem is **different in kind**: a single `2×2` readout operator generates a
*commutative* algebra, so the obstruction here is **not** observable
non-commutativity. It is a **co-realizability obstruction inside the (non-abelian,
substrate-native) `O_h` spatial symmetry** — whether one preparation can carry
both the trace's complex structure and the determinant's three-plane structure.
The `SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md` classification "the readout IS
the commutativity wall" is a `[SYNTHESIS]`-level interpretive grouping; this
theorem operates at the sharper, **decidable** (finite-group representation-theory)
level and is logically independent of the Bell-walled QM question. **This pre-reg
must not contradict the commutativity sibling, but it does not depend on it.**

**Doctrine.** Per CLAUDE.md scope discipline and the boundary-theorem program
(FTD-0186): a closed-negative / boundary result is a deliverable, not a failure.
This pre-reg targets a boundary theorem; its value is a rigorous verdict in
*either* direction. It promotes nothing on its own; `x₊=1/α` (FTD-0013) stays
`[STRONGLY MOTIVATED CONJECTURE]` regardless of verdict.

---

## §2 — The question (LOCKED)

> **Q-RSI.** Let `𝔉 = {P1..P5} ∪ {algebraic spine} ∪ {O_h representation theory of
> the Moore/BCC module}` define the admissible derivation base, with the
> forward-derived scalar set `S = {16 = |μ₄|², G* = det_ζ ratio (FTD-0234),
> G*² = 2π·G_BCC(0) (Watson)}`.
>
> **(Q1 — Absence/unforced.)** Does `𝔉` *force* a single substrate-native readout
> operator `T` on `V_complex ≅ ℤ[i]²` whose invariants are simultaneously
> `(Tr T, det T) = (16G*², 16G*³)` — derived forward, not asserted — from **one**
> preparation?
>
> **(Q2 — Independence.)** If not, characterize the minimal additional selection
> `W` (the operator-assembly / readout convention that fixes the `(Tr,Det)` pair),
> and exhibit that `𝔉 ∪ {W}` is **consistent** (so `W` is a genuine logically
> independent addition — as the parallel postulate is to Euclid's other four — not
> a contradiction).

The theorem to be proven (if INDEPENDENT) is the conjunction: **`𝔉` does not force
the `(Tr,Det)` pair from one preparation (Q1 = no), AND the pair is fixed only by a
consistent, logically independent selection `W` (Q2).**

**Explicitly NOT asked (scope guard, F-i):** whether *no* extension can *ever*
produce the pair (strong forbiddance) — known false (the master quadratic itself is
a consistent `W`), and claiming it would be scope creep. The theorem is
*independence*, full stop.

---

## §3 — Definitions (LOCKED)

**D1 — Substrate preparation.** A localized configuration on the lattice, with a
**stabilizer** `Stab ≤ O_h` (the subgroup of cubic symmetries fixing it). The
unbroken vacuum has `Stab = O_h`; a localized charge breaks it.

**D2 — Readout operator `T`.** A `2×2` `ℂ`-linear operator on `V_complex ≅ ℤ[i]²`
(the complex subspace of the Moore/BCC corner module, FTD-0122) whose
characteristic polynomial is the candidate master quadratic. `Tr T`, `det T` are
its two `O_h`-meaningful spectral invariants.

**D3 — Forward-derived vs imposed.** A quantity is **forward-derived** if it is an
element of (or assembled by ring operations from) the Axiom-Zero ring `R`
(`THEOREM_A_PHYS_NO_GO.md` D-`R`) realized as a structural invariant of a named
substrate object. It is **imposed** if its *value* lies in `R` but its *structural
role* (here: being `Tr`/`det` of the readout `T`) is a free choice not fixed by
`𝔉`. (This is the precise sense of W-CRIT-2: `16G*³ ∈ R`, but `det T = 16G*³` is a
role-assignment.)

**D4 — Assembly map `W`.** The map `S → (Tr, det)` that binds the forward scalars
into the readout operator's invariants. `W` is **not** a beable: it is a choice of
*what counts as the readout's operator structure*. The "6th-postulate-class" input,
if needed, is a `W`. The master quadratic is one specific `W`.

**D5 — Trace condition.** `Tr T = 16G*²` realized via a definite complex structure
`i = J` (`J² = −I`, a faithful order-4 element) on `V_complex` — the `ℤ[i]`-module
structure of FTD-0122/0231.

**D6 — Determinant condition.** `det T = 16G*³` realized as **three genuine equal**
`G*` factors (one per coordinate plane of `ℤ³`), organized by the `C₃` rotation
about a body diagonal `⟨111⟩` — the only admissible source of the **odd** power
(even-power wall, FTD-0237 / §5 H3 below), *not* by asserting `det = Tr·G*`.

---

## §4 — Admissible proof space (LOCKED)

**The proof MAY use:**
- The five postulates verbatim + the calibration declarations.
- Standard finite-group representation theory (character orthogonality,
  Frobenius reciprocity), `O ≅ S₄`, complex-structure theory (`J²=−I`).
- ζ-regularized determinant theory; the Lerch/Hurwitz `det_ζ{n+a} = √(2π)/Γ(a)`.
- The even-power wall (`E₆(i)=0`, classical) and FTD-0237's no-Eisenstein-twin.
- The `THEOREM_A_PHYS_NO_GO.md` proof anatomy (Claims A/B/C + independence half).
- FTD-0234 (det_ζ ratio = G*), FTD-0122 (`V_complex≅ℤ[i]²`), Watson (`G*²/(2π)=G_BCC(0)`)
  as `[THEOREM]`-grade imports, and the **discharged legs of §5** as established.

**The proof MAY NOT use:**
- Asserting `det = Tr·G*` or `det = 16G*³` without a forward det↔det_ζ derivation
  (the W-CRIT-2 move; B-1/F-a).
- Importing the master quadratic, its roots, `α`, or any CODATA value as a premise (B-5/F-d).
- The FQCR `M_N` matrix or any chosen-entry `2×2` as scaffold for `det` (B-2).
- Numerical near-miss / coincidence scans (CLAUDE.md).

---

## §5 — Benchmark (LOCKED): the proof obligation, with discharged legs marked

The proof, to count as INDEPENDENT, must establish the conjunction below. **Two
legs are already discharged (machine-checked / classical); they are locked here as
established, and the genuine open work is Leg 3 + the independence half.**

- **Leg 1 — trace side ✅ DISCHARGED (this commit).** The `O`-permutation module of
  the 8 cube-corner (BCC) sites decomposes `ℚ[corners] ≅ A₁ ⊕ A₂ ⊕ T₁ ⊕ T₂`, and
  the 2-dimensional irrep `E` has **`mult_O(E) = 0`**. Therefore there is **no
  `O`-symmetric 2-dimensional subspace** on which a complex structure (`J²=−I`,
  needing an even-dimensional block) can act; a definite `i` requires **breaking**
  `O` to a single `C₄` axis (FTD-0231), so `C₃(⟨111⟩) ∉ Stab`. The symmetric
  average `(J_x+J_y+J_z)/3` squares to `−I/3 ≠ −I`, confirming no symmetric `i`.
  **Verified:** `scripts/proofs/proof_readout_multE_zero.py` (group order 24;
  `mult E = 0`; constituent count 4; dims `1+1+3+3=8`).
  This upgrades the former "Lemma 3 premise" to character-theory grade.
- **Leg 2 — group core ✅ DISCHARGED.** `⟨C₄(⟨001⟩), C₃(⟨111⟩)⟩ = O` (a 4-cycle and
  a 3-cycle generate `S₄`). So any `Stab` containing both is all of `O` = unbroken
  = no localized charge = no `V_complex`. (Verified in the same script: generated
  group order 24.)
- **Leg 3 — determinant side ⛔ OPEN OBLIGATION (the crux).** Establish that the
  odd term `16G*³` *genuinely requires* `C₃(⟨111⟩) ∈ Stab` — i.e. that the only
  forward (non-asserted) source of the odd `G*` is the C₃-symmetric three-plane
  det_ζ product (Leg-1's `C₃ ∉ Stab` then collides with it) — **and** that the
  FORCED-escapes are closed:
  - **(3a) even-power-wall channel separation:** `Tr = 16·[Watson, even, no planes]`
    and `det` (odd) can only draw on the det_ζ channel (`E₆(i)=0`); show the trace
    cannot supply the odd factor.
  - **(3b) reduction-collapse:** any `C₃`-equivariant reduction of a 3-plane/rank-6
    (or infinite) det_ζ object to the rank-2 readout lands on the `C₃`-fixed
    diagonal, where `C₃` acts as identity and the cube collapses to `G*¹`.
    (Adversarial review computed this; it must be made a theorem, not an instance.)
  - **(3c) FORCED-escape closure:** prove no single substrate-native operator —
    reducible, 3-dim, or descended-from-infinite — carries **both** a definite `i`
    (Leg 1) and a `C₃`-symmetric three-plane structure (D6) without recollapsing.
    *This is the obligation whose failure yields FORCED (§6).*
- **Independence half — OPEN OBLIGATION.** Characterize `W` (the `(Tr,Det)`
  selection) and **exhibit** that `𝔉 ∪ {W}` is consistent — the master quadratic
  itself is the natural consistent witness. Consistency + Leg-3 non-derivability =
  logical independence. (Must be *exhibited*, not asserted — F-f.)

**Benchmark precision.** Legs 1–2 are exact (character arithmetic / group order).
Leg 3 + the independence half are algebra/representation-theoretic (no numerical
floor). Numerical confirmation (e.g. `16G*³ = 414.392…`) is corroboration only,
never the proof (F-g).

---

## §6 — The three pre-registered outcomes (LOCKED)

> **INDEPENDENT.** Legs 1–2 hold (they do), Leg 3 (3a–3c) all go through, and the
> independence half exhibits a consistent `W`-model; no §7 falsifier fires; no §8
> banned move. **Result:** the Readout-Structure Independence theorem stands;
> W-CRIT-2 is resolved as a boundary theorem; MC-T4.3's BCC/quantization route is
> sealed `[CLOSED NEGATIVE — boundary]`; `x₊=1/α` (FTD-0013) untouched. The
> surviving path to a *positive* MC-T4.3 closure becomes ARC-D (engine-native) or a
> new postulate supplying `W`.
>
> **FORCED.** Leg 3(3c) **fails**: a single substrate-native operator is exhibited
> that co-realizes the definite `i` and the `C₃`-symmetric three-plane determinant
> without recollapsing, yielding `(Tr,Det)=(16G*²,16G*³)` forward. This re-opens
> MC-T4.3 *positive* (ARC-3 eligibility for FTD-0013) and is the **more
> consequential** outcome. It stays genuinely hunted (B-12).
>
> **UNDERDETERMINED.** A candidate proof is admissible (no falsifier, no banned
> move) but Leg 3(3b/3c) is neither closed nor escaped (e.g. an orbit-sum
> construction is neither shown to recollapse nor shown to succeed), or the
> independence half asserts rather than exhibits a `W`-model. No tag moves; the
> standing FTD-0235 verdict is unchanged but sharpened.

---

## §7 — Falsifier rules (LOCKED) — F-a .. F-j

Inherits the FTD-0235 V1–V7 gate; restated and extended for the independence claim.
If any fires, the outcome is **not** INDEPENDENT.

- **F-a (assertion-of-det / W-CRIT-2 — decisive).** Fires if the proof anywhere
  uses `det = Tr·G*` or `det = 16G*³` as a step rather than deriving the odd factor
  forward from the det_ζ channel. (= FTD-0235 V1/V5.)
- **F-b (Tr/Det independence smuggling).** Fires if the proof treats `Tr` and `det`
  as not-independent for the finite `2×2` without justification. (= FTD-0235 V7.)
- **F-c (FORCED-escape unaddressed).** Fires if Leg 3(3c) is *claimed* without
  ruling out reducible / 3-dim / infinite-descended operators. The adversarial
  3-dim-rep and infinite-det_ζ attacks (this session) must be answered, not ignored.
- **F-d (no QM/QED/CODATA scaffold).** Fires on import of the master quadratic, its
  roots, `α`, Hilbert space, Born rule, or any CODATA value as a premise.
- **F-e (full module coverage).** Leg 1 must use the actual 8-corner module
  decomposition (`mult_O(E)=0`), not a hand-waved "2-dim needs an axis."
- **F-f (independence exhibited, not asserted).** The Q2 half must exhibit a
  consistent `W`-model; a bare "the master quadratic is consistent" without
  stating it as the model fires this.
- **F-g (probes corroborate, not prove).** Fires if any leg rests *logically* on
  FTD-0230/0231/0234/0235 as a premise rather than on the forward
  `𝔉 → T` construction. They are instances that must match, not premises.
- **F-h (even-power-wall correctness).** Fires on misuse of `E₆(i)=0` /
  FTD-0237 (e.g. claiming the odd `G*` can come from the modular/Watson channel).
- **F-i (no strong-forbiddance scope creep).** Fires on any claim that *no*
  extension can ever produce the pair. The theorem is independence only.
- **F-j (result lands in a separate doc).** Editing §§2–9 after hash-lock, or
  recording the verdict in this file, fires this and invalidates v1 (v2 required).

---

## §8 — Banned moves / anti-laundering (LOCKED) — B-1 .. B-12

- **B-1.** No `det = Tr·G*` / `det = 16G*³` assertion as a premise (it is the `W`).
- **B-2.** No chosen-entry `2×2`, no imported `M_N`/FQCR matrix as `det` scaffold.
- **B-3.** No reverse-engineering from "the master quadratic is the answer, so the
  structure must be forced." Forward direction only (`𝔉 → T → invariants`).
- **B-4.** No "MC-T4.3 must close, therefore the boundary is a theorem" (assertion).
- **B-5.** No QM/QED/CODATA import as scaffold.
- **B-6.** No conflation of the `O_h` *spatial* non-abelianness (which the substrate
  HAS) with *observable* non-commutativity `[A,B]≠0` (which it lacks) — keep this
  theorem distinct from the commutativity sibling (§1).
- **B-7.** No "orbit-sum obviously works/fails" hand-wave for Leg 3(3b/3c) — it must
  be a computation/proof (the reduction-collapse must be shown, F-c).
- **B-8.** No numerical near-miss / coincidence scan.
- **B-9.** No claim that any discharged leg (`mult_O(E)=0`, `⟨C₄,C₃⟩=O`) is weaker
  than stated, nor any over-strengthening of Leg 3 beyond what is proven.
- **B-10.** No retroactive editing of this pre-reg; v2 required if a definition or
  falsifier proves defective (FTD-0186 v1→v2 precedent).
- **B-11.** No spine tag moves (`x₊=1/α` stays `[SMC]`); any tag change happens only
  in a separate ratification doc after a verdict.
- **B-12.** **FORCED stays a live, hunted target throughout.** Engineering the proof
  toward INDEPENDENT is the process violation. The 2026-05-30 "scalar fixed-point"
  facade (a `W`-assertion dressed as a derivation, falsified) is the cautionary case.

---

## §9 — Method (LOCKED) — ordered steps

Run **only** against the hash-locked commit, in this order.

1. **Quote `𝔉`** — the five postulates + the spine imports (FTD-0122/0234/0237,
   Watson) + the scalar set `S`. Fix the readout-operator model (D2).
2. **State Legs 1–2 as established** (cite `proof_readout_multE_zero.py`); do not
   re-derive, but confirm the citation is exact (group order 24; `mult_O(E)=0`).
3. **Prove Leg 3(3a)** — even-power-wall channel separation (trace cannot supply
   the odd factor).
4. **Prove Leg 3(3b)** — reduction-collapse: make the `C₃`-equivariant rank-2
   reduction → diagonal → `G*¹` collapse a theorem (currently an adversarial
   computation).
5. **Prove Leg 3(3c)** — close the FORCED-escape: no reducible/3-dim/infinite-
   descended single operator co-realizes definite-`i` + `C₃`-symmetric determinant.
   *If this fails → FORCED (§6); record it honestly.*
6. **Characterize `W`** and **exhibit** a consistent `𝔉 ∪ {W}` model (the master
   quadratic) — the independence half (F-f).
7. **Run the F-a..F-j checklist** — record each fired / not-fired with one-line evidence.
8. **Run the B-1..B-12 checklist** — record none invoked.
9. **Write the verdict** (INDEPENDENT / FORCED / UNDERDETERMINED) in a **separate**
   result doc, with an **independent adversarial-review pass** (no project priors)
   per the FTD-0186 protocol. (This session's 3-angle adversarial pass —
   3-dim-rep, infinite-det_ζ, symmetric-trace — is the *pre-lock* refutation
   attempt; the post-lock proof must still face a fresh reviewer.)

---

## §10 — What this locks vs leaves open

**Locked (§§2–9):** the question (independence, not forbiddance); the definitions
of `T`, `W`, forward-vs-imposed; the two discharged legs; the three open
obligations (Leg 3a–3c + independence half); the three outcomes; F-a..F-j;
B-1..B-12; the 9-step method.

**Already discharged (this commit, machine-checked):** Leg 1 (`mult_O(E)=0`) and
Leg 2 (`⟨C₄,C₃⟩=O`) — see `proof_readout_multE_zero.py`.

**Left open (the genuine verdict):** Leg 3(3a/3b/3c) and the independence-half
model — i.e. whether the FORCED-escape can be closed (→ INDEPENDENT) or constructed
(→ FORCED), or neither (→ UNDERDETERMINED). Prior-favoured: INDEPENDENT, but FORCED
is fully live (B-12).

**Not in scope:** strong forbiddance (F-i); any spine retag (B-11); engine
measurement (this is a pure-math closure attempt; the ARC-D engine route is
separate); the QM commutativity wall (the sibling pre-reg; cross-checked, not re-proven).

---

## §11 — Hash-lock protocol

1. Finalise §§1–11. Compute SHA256:
   ```sh
   sha256sum docs/theory/10_eft_program/preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md
   ```
2. **Confirm the next-free LEDGER id** by grepping the whole `docs/` tree (concurrent
   sessions have claimed through ~FTD-0243). Record SHA256 + tag + the confirmed id in
   `../REF_PREREGISTER_MANIFEST.md` and add a `[PRE-REGISTRATION]` row to the LEDGER.
3. Commit + lightweight tag:
   ```sh
   git tag preregister-readout-structure-independence-v1 \
       -m "Pre-reg: readout-structure independence (MC-T4.3 boundary)"
   ```
4. The closure attempt runs ONLY against the tagged commit; verdict lands in a
   separate result doc (§9 step 9).
5. If a definition/falsifier proves defective, issue `..._v2.md` (do not edit v1).
6. Verify tag integrity: `git rev-list -n1 <tag>` and `git tag -l <tag>`.

---

## §12 — Single-line summary

A pre-registered, falsifier-gated design to prove (or refute) that the master
quadratic's **operator structure** `(Tr,Det)=(16G*²,16G*³)` is **logically
independent** of FTD's five postulates + spine + `O_h` representation theory — with
the trace-side leg (`mult_O(E)=0`, no `O`-symmetric complex structure) and the
group core (`⟨C₄,C₃⟩=O`) **already machine-checked**, the determinant-side
co-realizability obstruction (Leg 3) as the genuine open crux, and the
FORCED-escape (a single operator co-realizing both) pre-named as the decisive
falsifier — thereby converting W-CRIT-2 from `[OPEN methodological]` to a stated
boundary theorem (or, if FORCED, re-opening MC-T4.3 positive).
