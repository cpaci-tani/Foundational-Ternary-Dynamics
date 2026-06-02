# Pre-Registration — Q12: The Weak-SU(2) Provenance Audit (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the question,
definitions, frozen audit targets, success criterion, falsifier, and
method for a **desk audit of an existing derivation's provenance** (no
engine, no numerics). It contains **no result**. All three outcomes —
GENUINE / COUNT-MATCH / PARTIAL — are pre-blessed; the verdict is
genuinely open.
**Date:** 2026-05-22
**LEDGER row:** FTD-0192 (provisional — confirm next-free id against
`../07_assessment/core_ledgers/LEDGER.md` at hash-lock).
**Hash-lock:** to be SHA256-locked and git-tagged
`preregister-weak-su2-provenance-v1` before the audit runs — see §11.

**Purpose.** Q11 ([`AUDIT_COLOUR_SINGLET_RANK.md`](AUDIT_COLOUR_SINGLET_RANK.md),
FTD-0191) reduced the entire electroweak-rank question to a single named
claim: *is the weak `SU(2)` on the dual-substrate chirality mode
`φ = J_L − J_R` a genuine derivation, or a count-match?* Q12 audits exactly
that claim. It locks, *before* any critical reading of the target
derivation, (a) what would count as a **genuine derivation** of the weak
`SU(2)` and (b) what would mark it a **count-match**. This is the
anti-laundering instrument for Q12 and the **terminating step** of the
Q10 → Q11 → Q12 chain.

---

## §1 — Context

**The chain.** Q10 ([`AUDIT_FINITE_NEUTRAL_LOCK.md`](AUDIT_FINITE_NEUTRAL_LOCK.md),
FTD-0190) found FTD has the electroweak neutral-lock *ingredients* but not
a *forced* rank-2 → rank-1 assembly — UNDERDETERMINED. Q11
([`AUDIT_COLOUR_SINGLET_RANK.md`](AUDIT_COLOUR_SINGLET_RANK.md), FTD-0191)
localised the single unforced bit: the catalog forces exactly **one**
colour-singlet internal rank-1 `U(1)`-shadow (`ℤ[i]^×`); a **second** —
the Cartan of the weak `SU(2)` — exists only as a `[SELECTION]`/
`[CONJECTURE]` riding on the chirality mode `φ = J_L − J_R`
([`../02_foundations/FOUND_FORCE_STRUCTURE.md`](../02_foundations/FOUND_FORCE_STRUCTURE.md)
FST-1 `[SELECTION]`, FST-6 `[CONJECTURE]`). Q12 decides that `[SELECTION]`.

**Why Q12 terminates the chain.** The verdict is decisive both ways and
spawns no successor:
- **GENUINE** → the second rank-1 is forced → Q11's rank is forced to 2 →
  (Q11c already shows the rank-2 structure is `SU(2)×U(1)`) → **Q10 and
  Q11 both lift to FOUND.**
- **COUNT-MATCH** → the second rank-1 stays selection-grade → Q11's rank
  is forced to 1 → **Q10 closes negative** — the electroweak sector is an
  honest effective/parametric continuum completion (a mapped boundary, per
  goal-clause 2).
- **PARTIAL** → the audit names the exact load-bearing step that is not
  yet derived; Q10/Q11 stay UNDERDETERMINED, with the gap pinned to one
  step. No "Q13" — a PARTIAL verdict *is* the boundary map.

**Doctrine.** Unchanged from Q10/Q11: FTD recovers a finite-closure
parallel whose continuum shadow benchmarks the SM. **[Program statement —
derives nothing on its own; F10 discipline applies.]**

**This audit reads, before any verdict, an existing derivation.** Q12 is a
provenance audit — the same genre as the FTD-0186 boundary-theorem
discriminator and the FTD-0189 graviton-provenance audit. It does not
re-derive the weak `SU(2)`; it traces and grades the derivation FTD
already has.

---

## §2 — The question (LOCKED)

> **Q12.** Is the weak `SU(2)` carried by the dual-substrate chirality
> mode `φ = J_L − J_R` (the `SU(2)_L` of FTD's electroweak sector) a
> **genuine derivation** (D1) — an explicit chain from the FTD postulates
> and/or the §4 catalog in which every **load-bearing step** (D4) is
> `[THEOREM]`/`[DERIVED]` — or is it a **count-match** (D2): the `SU(2)`
> asserted because an FTD count matches `dim SU(2) = 3` (or a doublet's 2
> states, or `rank SU(2) = 1`) without the group's non-abelian structure
> being constructed?

The audit grades the derivation FTD has; it does not supply a new one.

---

## §3 — Definitions (LOCKED)

**D1 — Genuine derivation of `SU(2)`.** An explicit chain from the FTD
postulates and/or the §4 catalog (D5-frozen) that **constructs `SU(2)` as
a group**: it exhibits three generators `T₁, T₂, T₃` as structures/
operators traceable to the catalog, establishes the non-abelian Lie
bracket `[Tₐ, T_b] = i ε_{abc} T_c` (or the equivalent group law), and
exhibits a genuine 2-dimensional irrep (the doublet) on which they act —
with **every load-bearing step (D4) `[THEOREM]`/`[DERIVED]`**.

**D2 — Count-match.** The `SU(2)` is inferred because some FTD count —
the number of components of `φ`, the number of chirality degrees of
freedom, a generator count, etc. — equals `dim SU(2) = 3`, or a two-state
count matches a doublet, **without** the commutation relations / group
law being constructed. A count-match is `[SELECTION]` at best; it is
**not** a derivation (GTCA F1/F10).

**D3 — The chirality mode `φ = J_L − J_R`.** The difference mode of the
dual substrate `(J_L, J_R)`; per `FOUND_FORCE_STRUCTURE.md` Parts III/VII,
a real 3-component pseudovector. Its **forced** internal symmetry, as
established by the Q11 audit, is the parity `ℤ₂` (`J_L ↔ J_R`).

**D4 — Load-bearing step.** A step of the claimed derivation whose removal
collapses the `SU(2)` conclusion. The audit identifies the load-bearing
steps explicitly and grades each.

**D5 — Frozen audit targets.** The primary target is
[`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md).
The audit also reads `FOUND_FORCE_STRUCTURE.md` (FST-1, FST-6) and **any
document either cites as load-bearing** for the `SU(2)` construction — all
as they exist **at the Q12 hash-lock commit**. A later edit to any target
does not retroactively change the audit; a materially changed target
requires a v2 (§11 step 5).

---

## §4 — What a genuine derivation must exhibit (LOCKED benchmark)

`SU(2)` is a 3-dimensional non-abelian Lie group; its algebra `su(2)` has
generators `T₁, T₂, T₃` with `[Tₐ, T_b] = i ε_{abc} T_c`. **[Standard Lie
theory.]**

**Load-bearing distinction for Q12.** The Q10 audit already established
that the §4 catalog supplies the `SU(2)` **skeleton** — the Weyl-`ℤ₂` (the
conjugation of `ℤ[i]^×`) and the `±½` doublet weights (the Cartan `T₃`
direction). A **genuine** derivation must go beyond the skeleton: it must
construct the **off-diagonal generators `T₁, T₂`** (the
raising/lowering structure) and their bracket `[T₁, T₂] = i T₃` from the
catalog. Exhibiting only `T₃` and the Weyl reflection is the skeleton, not
the group — and the skeleton is exactly what Q10 already had when it
returned UNDERDETERMINED.

The benchmark is a **falsification benchmark**: the audit author must be
able — and willing — to write the COUNT-MATCH report (§6).

---

## §5 — (reserved)

*This pre-registration uses §4 for the benchmark; §5 is intentionally
empty so the section numbering of the locked clauses (§6 outcomes, §7
falsifier, §8 banned moves, §9 method) aligns with the Q10/Q11
pre-registrations for cross-comparison.*

---

## §6 — The three pre-registered outcomes (LOCKED)

The audit returns exactly one verdict. Each carries an explicit
consequence for Q10 (FTD-0190) and Q11 (FTD-0191).

**GENUINE.** Every load-bearing step of the weak-`SU(2)`-on-`φ`
derivation is `[THEOREM]`/`[DERIVED]`; the off-diagonal generators and the
bracket are constructed from the catalog, not asserted. **Consequence:**
Q11's second rank-1 becomes forced → the colour-singlet rank is forced to
2 → **Q10 and Q11 both lift to FOUND.** Tag: the audit *reports* the
finding; any resulting promotion of the weak-`SU(2)` claim's LEDGER /
source-doc tag is a **separate canonical-change action**, owner-approved,
executed per `META_STRUCTURE.md` — **not** performed by this audit (§8).

**COUNT-MATCH.** A load-bearing step is a count-match (D2) or a
`[SELECTION]`/`[CONJECTURE]`/`[IMPOSED]` that is structurally a
count-match. The weak `SU(2)` is not a genuine derivation. **Consequence:**
the second rank-1 stays selection-grade → Q11's rank is forced to 1 →
**Q10 closes negative.** **Per goal-clause 2 ("rigorously establish what
we cannot derive"), a COUNT-MATCH verdict is a deliverable** — it maps the
precise boundary: FTD's discrete ontology does not determine the
electroweak `SU(2)`, which is then an honest effective completion.

**PARTIAL.** The derivation is genuine in part — some steps
`[THEOREM]`/`[DERIVED]` — but at least one load-bearing step is
`[SELECTION]` and the audit can neither promote it (no construction
exists) nor mark it a definite count-match (it is not a bare count).
**Consequence:** Q10/Q11 stay UNDERDETERMINED; the audit names the exact
step and what a construction of it would require. A PARTIAL verdict is the
boundary map; it spawns no successor pre-registration.

No outcome promotes `x₊ = 1/α` or any spine claim. Q12 is
electroweak-structural.

---

## §7 — The falsifier (LOCKED)

"The weak `SU(2)` on `φ` is a genuine derivation" is **falsified** if any
load-bearing step (D4) is found to be:

- **F-a.** A **count-match** — an FTD count equated with `dim SU(2) = 3`,
  `rank SU(2) = 1`, or a doublet's 2 states, with no commutation relations
  or group law constructed.
- **F-b.** Tagged `[SELECTION]`/`[CONJECTURE]`/`[IMPOSED]` in its own
  source document, with no `[THEOREM]`/`[DERIVED]` replacement available.
- **F-c.** Asserted by **analogy to the Standard Model** ("the weak force
  is `SU(2)`, therefore `φ` carries `SU(2)`") rather than constructed from
  the catalog — a circular import of the benchmark.
- **F-d.** Dependent on a structure outside the §4 catalog and the five
  FTD postulates (a new postulate).
- **F-e.** A construction of **only** the Cartan `T₃` and/or the
  Weyl-`ℤ₂` — the `SU(2)` skeleton — with the off-diagonal generators
  `T₁, T₂` and the bracket `[T₁, T₂] = i T₃` left unconstructed (§4).

Any single firing on a load-bearing step → the GENUINE verdict is rejected
(the audit returns COUNT-MATCH or PARTIAL per §6).

---

## §8 — Banned moves / anti-laundering (LOCKED)

- **No engineering toward GENUINE.** The COUNT-MATCH report must be a live
  option throughout. The §4 benchmark is a falsification benchmark.
- **A generator count is not a derivation.** "`φ` has 3 components, `SU(2)`
  has 3 generators" is a count-match, not a construction (F-a, GTCA
  F1/F10).
- **No SM read-back.** The SM's `SU(2)` may not be assumed onto `φ` (F-c).
- **Honour the source documents' own tags.** Where a target document tags
  a step `[SELECTION]`/`[CONJECTURE]`, the audit reports that tag; it does
  **not** silently upgrade it.
- **The audit changes no tags.** Q12 *reports* a provenance finding. A
  GENUINE verdict *recommends* a tag promotion to be executed separately
  under the `META_STRUCTURE.md` canonical-change protocol with owner
  approval; the audit itself promotes, demotes, and re-tags nothing.
- **No numerical-precision claims.** Q12 is a structural provenance audit.
- **No spine tag moves.** Q12 must not promote, demote, or re-tag any
  spine claim. Its only LEDGER consequence is the Q10/Q11 status update
  specified in §6.

---

## §9 — Method (LOCKED)

The audit executes exactly these steps and reports each:

1. **Read** the frozen target documents (D5): `DERIV_LATTICE_SU2_WEAK.md`,
   `FOUND_FORCE_STRUCTURE.md` FST-1/FST-6, and every document they cite as
   load-bearing for the `SU(2)` construction — at the Q12 lock commit.
2. **Extract** the claimed derivation of "`φ = J_L − J_R` carries
   `SU(2)_L`" as an explicit, ordered list of steps.
3. **Classify** each step: `[THEOREM]` / `[DERIVED]` / `[SELECTION]` /
   count-match (D2) / `[CONJECTURE]` / `[IMPOSED]`. Use each step's own
   source-document tag where one exists; where none exists, assign the
   classification the step's content warrants and say so.
4. **Identify the load-bearing steps** (D4) — those whose removal
   collapses the `SU(2)` conclusion.
5. **Apply the falsifier** F-a…F-e to each load-bearing step. In
   particular, test F-e: are the off-diagonal generators `T₁, T₂` and the
   bracket constructed, or only the Cartan/Weyl skeleton?
6. **Verdict** (§6): GENUINE iff every load-bearing step is
   `[THEOREM]`/`[DERIVED]`; COUNT-MATCH iff a load-bearing step fires F-a
   or is a structural count-match; PARTIAL otherwise.
7. **Report** the verdict with the full step ledger and the explicit
   consequence for Q10 (FTD-0190) and Q11 (FTD-0191).

The audit is a desk classification of an existing derivation's
provenance; it involves no engine run and no numerical search. Where a
step ledger or step count is tabulated, it may be recorded by a small
enumeration script (as for Q10/Q11), but the classification itself is a
reading judgement, stated with its reasons.

---

## §10 — What this pre-registration locks vs leaves open

**Locked by the hash** (§11): the question (§2), definitions D1–D5 (§3),
the genuine-derivation benchmark (§4), the three outcomes and their
Q10/Q11 consequences (§6), the falsifier F-a…F-e (§7), the banned moves
(§8), the method (§9).

**Open** — and only this: the **verdict**. Whether FTD's weak `SU(2)` on
`φ` is a genuine derivation, a count-match, or partial is exactly what the
audit will determine. GENUINE, COUNT-MATCH, and PARTIAL are all permitted;
none is favoured. A COUNT-MATCH that closes Q10 negative is as valuable a
deliverable as a GENUINE that lifts it.

---

## §11 — Hash-lock protocol

To lock this pre-registration before the audit runs:

1. Finalise this document. Compute
   `sha256sum docs/theory/08_structural/PREREG_WEAK_SU2_PROVENANCE_v1.md`.
2. Record the SHA256 in
   [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md)
   (new row) and add a [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md)
   row (FTD-0192 or next-free) tagged `[PRE-REGISTRATION]`, citing this file.
3. `git commit` the pre-registration; create a lightweight tag
   `git tag preregister-weak-su2-provenance-v1`.
4. The audit (executing §9) runs only against the tagged commit, reading
   the target documents as they exist at that commit. Its result lands in
   a separate `AUDIT_WEAK_SU2_PROVENANCE.md` — never by editing this file.
5. If a definition here proves defective once the audit starts, the
   correct response is a **v2 pre-registration**, not an edit to v1 (cf.
   the FTD-0186 boundary-theorem v1→v2 precedent).

---

*Pre-registration authored 2026-05-22. No result. The audit (§9) is the
next step, and runs only after hash-lock. Q12 is the terminating step of
the Q10 → Q11 → Q12 chain; its verdict decides whether FTD-0190 and
FTD-0191 lift to FOUND, close negative, or stay UNDERDETERMINED with the
gap pinned to one named step.*
