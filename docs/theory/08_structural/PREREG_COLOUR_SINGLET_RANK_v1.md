# Pre-Registration — Q11: The Colour-Singlet Rank Audit (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the question, definitions,
admissible search space, success criterion, falsifier, and method for a
**desk audit** (finite group theory; no engine, no numerics). It contains
**no result**. All three outcomes — FOUND / UNDERDETERMINED /
CLOSED-NEGATIVE — are pre-blessed; the audit's verdict is genuinely open.
**Date:** 2026-05-22
**LEDGER row:** FTD-0191 (provisional — confirm next-free id against
`../07_assessment/LEDGER.md` at hash-lock).
**Hash-lock:** to be SHA256-locked and git-tagged
`preregister-colour-singlet-rank-v1` before the audit runs — see §11.

**Purpose.** Q10 ([`AUDIT_FINITE_NEUTRAL_LOCK.md`](AUDIT_FINITE_NEUTRAL_LOCK.md),
FTD-0190) returned **UNDERDETERMINED**: FTD's frozen catalog supplies every
*ingredient* of the electroweak neutral lock but not a *forced* rank-2 →
rank-1 assembly. Q11 isolates the single step that decides Q10. It locks,
*before* any counting, (a) what would count as FTD's colour-singlet sector
having abelian **rank exactly 2** — the electroweak rank — and (b) what
would **falsify** it. This pre-registration is the anti-laundering
instrument for Q11.

---

## §1 — Context and doctrine

**The doctrine is unchanged from Q10** (pre-reg
[`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](PREREG_FINITE_NEUTRAL_LOCK_v1.md) §1):
FTD does not recover the Standard Model as primitive ontology; the SM is a
**benchmark shadow**, the effective continuum completion of finite closure.
`SU(2)×U(1)` is treated as the continuum shadow of a finite colour-singlet
sector, not as a primitive object. **[Program statement — derives nothing
on its own; F10 discipline applies.]**

**What Q10 left open.** The Q10 audit found exactly one clean internal
colour-singlet rank-1 `U(1)`-shadow — `ℤ[i]^× ≅ ℤ₄` — together with a
two-state opposition `{i,−i}` carrying the correct `±½` doublet weights,
and a hypercharge forced by `Q⟨v⟩=0`. The electroweak lock needs a
**rank-2** structure (`SU(2)_L × U(1)_Y`); the catalog hands over a
rank-2 object only in the *coloured* sector (the `SU(3)` triality of
[`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md))
— the wrong sector. Q10's verdict therefore hinged on whether the catalog
supplies a **second independent internal colour-singlet `U(1)`-shadow**
beyond `ℤ[i]^×`. That is exactly Q11. **[Statement of the open question;
no result asserted.]**

**The decisive structure (flagged, not pre-judged).** The catalog's
remaining internal colour-singlet structures are the ternary alphabet
(automorphism `ℤ₂`) and the dual substrate `(J_L, J_R)`. Whether the dual
substrate carries a genuine independent `U(1)`-shadow (an `L`/`R` relative
phase) or only a parity `ℤ₂` is the crux the audit must settle. The
pre-registration names it as the crux; it does not decide it.

**Stabiliser framing carries over.** Q10 §1's reframing — symmetry breaking
= a distinguished configuration and its stabiliser — stands. Q11 is the
*rank* sub-question of that framing: a rank-1 stabiliser (one residual EM
`U(1)`) inside a rank-2 group requires the rank-2 group to exist first.

---

## §2 — The question (LOCKED)

> **Q11.** Within FTD's frozen finite-closure catalog (§4, identical to the
> Q10 §4 catalog), restricted to its **colour-singlet sector** (D2) and
> counting only **internal** symmetry (D3): is the **abelian rank** (D4) —
> the maximal number of pairwise-commuting, pairwise-independent rank-1
> `U(1)`-shadows (D1, D5) — **forced** (D6) to be exactly **2**?
>
> Conditional follow-up (Q11c): *if* the rank is forced to 2, is exactly
> **one** of the two rank-1 directions the Cartan of a non-abelian
> `SU(2)`-shadow (whose Weyl group is the conjugation-`ℤ₂` of `ℤ[i]^×`),
> the other a residual abelian `U(1)` — i.e. is the colour-singlet sector
> `SU(2)×U(1)`, and not `U(1)×U(1)`?

The audit reports the rank it finds and whether that value is forced;
neither is assumed. Rank 0, 1, 2, and ≥3 are all admissible findings.

---

## §3 — Definitions (LOCKED)

**D1 — Rank-1 `U(1)`-shadow.** A cyclic group `C` — a subgroup or quotient
of a catalog structure's symmetry group, or a catalog structure that is
itself cyclic — with `|C| ≥ 3`, arising as the finite shadow of a
continuous `U(1)`. A group of order 2 (`ℤ₂`) is **explicitly not** a clean
`U(1)`-shadow: it is a `μ₂` / Weyl element (consistent with Q10 D5).

**D2 — Colour-singlet sector.** The part of a catalog structure that is
**trivial** under the BCC `ℤ₃`-triality (the triple-cosine colour
structure of `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`). A structure that
transforms non-trivially under that `ℤ₃` is *coloured* and is excluded
from the Q11 count.

**D3 — Internal symmetry.** A symmetry that acts on FTD's **state / flux /
arithmetic** structure (the ternary alphabet, the flux field, `ℤ[i]`
arithmetic, the dual substrate), **not** on spatial lattice position. The
`O_h` *spatial* point group — rotations/reflections of the Moore
neighbourhood — is a **spacetime** symmetry and is excluded from the Q11
count. (Conflating spacetime and internal symmetry is the classic
Coleman–Mandula-type error; this exclusion is a primary anti-laundering
control.)

**D4 — Abelian rank.** The maximal number of pairwise-commuting,
pairwise-independent (D5) rank-1 `U(1)`-shadows (D1) drawn from the
catalog's colour-singlet (D2) internal (D3) structures.

**D5 — Independence.** Two rank-1 `U(1)`-shadows are independent iff
neither is contained in the other and they commute, so that together they
generate a rank-2 torus (`U(1)×U(1)`). Two shadows that are the same
`U(1)` read two ways count once.

**D6 — Forced.** The rank is *forced* iff it takes the **same value across
every admissible reading** of the frozen catalog — no interpretive choice
(e.g. whether a `ℤ₂` promotes, whether the dual substrate supplies one
`U(1)` or none) changes it. If two admissible readings give different
ranks, the rank is **not** forced (→ UNDERDETERMINED). This is the same
forced-vs-unforced axis on which Q10 turned.

---

## §4 — Admissible search space (LOCKED)

**The catalog is identical to Q10's, and frozen.** The audit may draw
**only** on the finite structures already established in the FTD corpus, as
fixed by `PREREG_FINITE_NEUTRAL_LOCK_v1.md` §4:

1. The ternary state alphabet `{−1,0,+1}` and its automorphism group.
2. The Moore-neighbourhood point groups — `O_h` (`|O_h|=48`), the
   cuboctahedral group, the BCC sublattice structure
   (`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`,
   [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](THEOREM_MOORE_LAYER_DECOMPOSITION.md)).
3. `ℤ[i]` and its unit group `ℤ[i]^×` (cyclic of order 4 = `N_base`).
4. The 27-block representation structure and its `O_h` irreps.
5. The dual substrate `(J_L, J_R)` and its parity / orientation structure.
6. The four framework integers `{N_c=3, N_base=4, b_3=7, N_eff=13}`.

Any finite group, cyclic structure, or action **not reducible to the
above** is a **new postulate**. It may appear in the audit, but a rank
count that depends on it is tagged `[CONJECTURE — new postulate]` and
**does not** support a FOUND verdict (§6).

---

## §5 — Benchmark: the electroweak rank (LOCKED)

The SM electroweak gauge group `SU(2)_L × U(1)_Y` has **rank 2**: one
Cartan generator from `SU(2)_L` (the `T₃` direction) and one from
`U(1)_Y`. After electroweak symmetry breaking the residual `U(1)_EM` has
rank 1. **[Standard electroweak theory.]**

**Load-bearing structural fact for Q11.** Rank 2 alone is necessary but
not sufficient. The structure must be `SU(2)×U(1)` — exactly **one**
non-abelian factor — because an irreducible weak doublet requires the full
`SU(2)`, not merely its Cartan. A rank-2 sector of the form `U(1)×U(1)`
(two abelian factors) **cannot** host the doublet of Q10's candidate B.
Q11c is therefore not optional polish: a rank-2 `U(1)×U(1)` finding falsifies
the electroweak reading (F-f).

`rank = 2` with structure `SU(2)×U(1)` is a **falsification benchmark**,
never a target to engineer toward. The audit author must be able — and
willing — to write the CLOSED-NEGATIVE report (§6).

---

## §6 — The three pre-registered outcomes (LOCKED)

The audit returns exactly one verdict. Each carries an explicit
consequence for Q10 (FTD-0190).

**FOUND.** The colour-singlet internal abelian rank is **forced to exactly
2**, with a construction trace exhibiting both rank-1 `U(1)`-shadows from
the §4 catalog (no new postulate), and Q11c confirms exactly one factor is
a non-abelian `SU(2)`-shadow. Tag consequences:
- The rank-2 result is `[SELECTION PRINCIPLE]` iff the rank-2 reading is
  unique in the catalog; `[STRONGLY MOTIVATED CONJECTURE]` if it is one of
  a small rigid set; `[CONJECTURE]` otherwise.
- **Consequence for Q10:** FTD-0190 is lifted UNDERDETERMINED → FOUND; the
  candidate-B neutral lock becomes a `[DERIVED]`/`[SELECTION]` object and
  the `(1,2)₁/₂` shadow-match Q10c rises to `[SELECTION PRINCIPLE]`.
- No outcome promotes `x₊ = 1/α` or any spine claim. Q11 is
  electroweak-structural.

**UNDERDETERMINED.** A rank-2 reading exists but the rank is **not
forced** (D6) — some admissible reading of the catalog gives a different
rank (e.g. the dual substrate's second `U(1)` is present under one reading,
absent under another). Tag: `[SELECTION PRINCIPLE — open]` or
`[CONJECTURE]`. **Consequence for Q10:** FTD-0190 stays UNDERDETERMINED;
the audit records exactly which reading is unforced.

**CLOSED-NEGATIVE.** The colour-singlet internal abelian rank is **forced
to ≠ 2** (0, 1, or ≥3); or reaching rank 2 requires a new postulate (§4);
or the rank-2 sector is forced to be `U(1)×U(1)` with no `SU(2)` (Q11c
fails by F-f). Tag: `[CLOSED NEGATIVE]`. **Consequence for Q10:** FTD-0190
**closes negative** — FTD's finite catalog does not contain the
electroweak rank-2 invariant, and the electroweak sector is, honestly, an
effective/parametric continuum completion. **Per the project's stated
goal-clause 2 ("rigorously establish what we cannot derive"), a
CLOSED-NEGATIVE here is a deliverable** — it maps the precise boundary of
what the discrete ontology determines in the electroweak sector. That is a
mapped boundary, not a failure.

---

## §7 — The falsifier (LOCKED)

"The colour-singlet internal abelian rank is exactly 2, with structure
`SU(2)×U(1)`" is **falsified** if any of:

- **F-a.** The rank is forced to a value ≠ 2 (0, 1, or ≥3).
- **F-b.** Reaching rank 2 requires a finite group, cyclic structure, or
  action **not** in the §4 catalog (a new postulate / a new `U(1)`).
- **F-c.** The two rank-1 `U(1)`-shadows do not commute, or are not
  independent (one is contained in the other — D5).
- **F-d.** A `ℤ₂` — the ternary sign automorphism, the dual-substrate
  parity, or any order-2 group — is counted as a clean rank-1
  `U(1)`-shadow in order to reach 2 (violates D1).
- **F-e.** A **spacetime** symmetry (an `O_h` spatial rotation or
  reflection) is counted as an **internal** `U(1)`-shadow (violates D3).
- **F-f.** (Q11c) The rank-2 sector is forced to be `U(1)×U(1)` — no
  factor is a non-abelian `SU(2)` — so it cannot host an irreducible
  doublet.

Any single firing → the electroweak-rank reading is rejected; the audit
proceeds to UNDERDETERMINED or CLOSED-NEGATIVE per §6.

---

## §8 — Banned moves / anti-laundering (LOCKED)

- **No new free integers, exponents, cyclic structures, or finite groups**
  introduced to reach rank 2. The §4 catalog is frozen.
- **No spacetime-as-internal conflation.** `O_h` spatial rotations are not
  internal `U(1)`s (D3, F-e).
- **A `ℤ₂` is not a `U(1)`.** Order-2 groups do not count toward the rank
  (D1, F-d).
- **No post-hoc role assignment.** Which catalog structure is `T₃`'s
  Cartan and which is `U(1)_Y` must be motivated independently of the
  rank-2 target.
- **No CODATA, no numerical-precision claims.** Q11 is structural finite
  group theory; "closes to N digits" statements are inadmissible.
- **A structural resemblance is not a derivation** (GTCA F1/F10). The rank
  is `[forced]` only with an explicit reading-invariance argument (D6);
  absent that, `[CONJECTURE]` is the ceiling.
- **No spine tag moves.** Q11 must not promote, demote, or re-tag any
  existing LEDGER claim. Its only LEDGER consequence is the Q10 status
  update specified in §6.
- **CLOSED-NEGATIVE stays a live option throughout.** The audit author
  must be willing to write it.

---

## §9 — Method (LOCKED)

The audit executes exactly these steps and reports each:

1. **Restate** the §4 catalog. Mark each structure colour-singlet vs
   coloured (D2) and internal vs spacetime (D3). Carry forward only the
   colour-singlet internal structures.
2. For each surviving structure, **enumerate its abelian symmetry
   shadows** and classify each as: rank-1 `U(1)`-shadow (cyclic, order ≥3,
   D1) / `ℤ₂`-only / non-abelian.
3. **Determine the maximal set** of pairwise-commuting, pairwise-
   independent (D5) rank-1 `U(1)`-shadows; record the **rank** (D4).
4. **Test forced-ness** (D6): enumerate the admissible readings of the
   catalog (in particular the dual-substrate reading — does `(J_L,J_R)`
   carry an independent `U(1)` or only a parity `ℤ₂`?) and check whether
   the rank is reading-invariant.
5. **Compare** the rank to the benchmark 2; apply falsifiers F-a…F-e.
6. **If rank = 2:** apply Q11c — is exactly one factor a non-abelian
   `SU(2)`-shadow (Weyl group = the conjugation-`ℤ₂` of `ℤ[i]^×`)? Apply
   F-f.
7. **Report** the verdict (§6) with either the full construction trace
   (FOUND / UNDERDETERMINED) or the specific obstruction / firing
   falsifier (CLOSED-NEGATIVE), and the explicit consequence for Q10
   (FTD-0190).

The audit is a desk computation: small finite groups, cyclic-subgroup
enumeration, commutativity and containment checks. Where a group order,
subgroup, or rank is claimed, it is **computed** (sympy / explicit
enumeration), not asserted (GTCA tool-use discipline).

---

## §10 — What this pre-registration locks vs leaves open

**Locked by the hash** (§11): the question (§2), definitions D1–D6 (§3),
the admissible catalog (§4, identical to Q10's), the benchmark (§5), the
three outcomes and their Q10 consequences (§6), the falsifier F-a…F-f
(§7), the banned moves (§8), the method (§9).

**Open** — and only this: the **verdict**. The colour-singlet internal
abelian rank, and whether it is forced, is exactly what the audit will
determine. FOUND, UNDERDETERMINED, and CLOSED-NEGATIVE are all permitted;
none is favoured. A CLOSED-NEGATIVE that closes Q10 is as valuable a
deliverable as a FOUND that lifts it.

---

## §11 — Hash-lock protocol

To lock this pre-registration before the audit runs:

1. Finalise this document. Compute
   `sha256sum docs/theory/08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md`.
2. Record the SHA256 in
   [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md)
   (new row) and add a [`../07_assessment/LEDGER.md`](../07_assessment/LEDGER.md)
   row (FTD-0191 or next-free) tagged `[PRE-REGISTRATION]`, citing this file.
3. `git commit` the pre-registration; create a lightweight tag
   `git tag preregister-colour-singlet-rank-v1`.
4. The audit (executing §9) runs only against the tagged commit. Its
   result lands in a separate `AUDIT_COLOUR_SINGLET_RANK.md` (or a
   `FOUND_…` / `…_CLOSED_NEGATIVE.md` per the verdict) — never by editing
   this file.
5. If a definition here proves defective once the audit starts, the
   correct response is a **v2 pre-registration**, not an edit to v1 (cf.
   the FTD-0186 boundary-theorem v1→v2 precedent).

---

*Pre-registration authored 2026-05-22. No result. The audit (§9) is the
next step, and runs only after hash-lock. Q11 is the successor to Q10
(FTD-0190); its verdict decides whether FTD-0190 lifts to FOUND, stays
UNDERDETERMINED, or closes negative.*
