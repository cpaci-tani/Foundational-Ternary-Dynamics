# Audit — Q11: The Colour-Singlet Rank Audit (Result)

**Tag:** [AUDIT FINDING] — this document is the **result** of executing the
hash-locked pre-registration `PREREG_COLOUR_SINGLET_RANK_v1.md`. It records
one of the three pre-blessed verdicts. It promotes and demotes no LEDGER
claim.
**Date:** 2026-05-22
**LEDGER row:** FTD-0191.
**Pre-registration audited:** [`PREREG_COLOUR_SINGLET_RANK_v1.md`](PREREG_COLOUR_SINGLET_RANK_v1.md)
— SHA256 `08c55b8e060332a2311be7ae6dedf5d48cbf1af861db627195d1dd2f8a886dbe`,
git tag `preregister-colour-singlet-rank-v1`, lock commit `4f3140a`. The
pre-registration was **not edited** during the audit (pre-reg §11 step 4).
**Verification artifact:** [`../../../scripts/proofs/audit_colour_singlet_rank.py`](../../../scripts/proofs/audit_colour_singlet_rank.py)
— enumerates the frozen §4 catalog; computes the rank tally under each
reading; all assertions pass, exit 0. No numerical search.

---

> **Update (2026-05-22) — Q11 resolved to CLOSED-NEGATIVE; the chain has
> terminated.** Q11 below deferred one question to Q12: is the weak
> `SU(2)` on `φ` a genuine `[SELECTION]` or a count-match? Q12
> ([`AUDIT_WEAK_SU2_PROVENANCE.md`](AUDIT_WEAK_SU2_PROVENANCE.md),
> FTD-0192) found it is a **count-match** — not a genuine rank-1. The
> rank-2 reading therefore collapses: the colour-singlet internal abelian
> rank is **forced to 1**, and Q11's UNDERDETERMINED verdict below
> **resolves to CLOSED-NEGATIVE** (rank forced ≠ 2). Q10 (FTD-0190)
> closes negative with it. The §1 verdict and the audit trace below stand
> as the correct Q11-stage record; the closure is the chain-level result.

---

## §1 — Verdict

> **UNDERDETERMINED** (pre-reg §6).
> **Consequence for Q10 (FTD-0190): stays UNDERDETERMINED.**

**[AUDIT FINDING].** FTD's frozen catalog **forces exactly one**
colour-singlet, internal, rank-1 `U(1)`-shadow: `ℤ[i]^× ≅ ℤ₄`. A **second**
rank-1 — the one needed to make the electroweak rank 2 — exists only under
FTD's `[SELECTION]`/`[CONJECTURE]`-grade assignment of the weak force to
the dual-substrate chirality mode `φ = J_L − J_R`. The colour-singlet
internal abelian rank is therefore **1 under the catalog's forced content
and 2 under that `[SELECTION]` reading** — not reading-invariant, hence
**not forced** (D6). The audit lands on UNDERDETERMINED: not FOUND (the
rank is not forced to 2), not CLOSED-NEGATIVE (a rank-2 reading is
admissible — the weak-`SU(2)` reading is in a catalog-cited document, not a
new postulate).

**Q11 sharpens Q10 rather than resolving it.** Q10 found the electroweak
rank-2 → rank-1 lock "assembled by an unforced choice." Q11 has now
**located that unforced choice precisely**: it is the single pre-existing
`[SELECTION]` that the weak `SU(2)` lives on the chirality mode
`φ = J_L − J_R`. The entire electroweak-rank question reduces to one
named, auditable claim (§6).

**No falsifier fired.** UNDERDETERMINED is a clean pre-blessed landing.

**One-paragraph plain reading.** The electroweak gauge group has rank 2:
two independent "dial" directions (`SU(2)_L`'s Cartan and `U(1)_Y`),
breaking to one (`U(1)_EM`). FTD's frozen catalog supplies one such dial
without ambiguity — the `ℤ[i]^×` Gaussian-unit structure. The second dial
is the weak `SU(2)`. FTD *does* place a weak `SU(2)` in its catalog — on
the left-minus-right chirality mode of the dual substrate — but only as a
selection, not a derivation: structurally that mode is a plain real
pseudovector whose only forced internal symmetry is a parity flip, and a
parity flip is not a `U(1)`. So FTD has one-and-a-half of the two dials it
needs: one forced, one conjectured. That is exactly enough to keep the
electroweak sector open and not enough to close it either way.

---

## §2 — What was audited, and the discipline

The audit executed the locked §9 method against the §4 frozen catalog
(identical to Q10's), the D1–D6 definitions, the §5 benchmark, and the §7
falsifier — all as fixed by the hash. The §8 banned moves were observed:
no new free integer, exponent, cyclic structure, or finite group was
introduced; no spacetime symmetry was counted as internal; no `ℤ₂` was
counted as a `U(1)`; no spine tag was moved.

Catalog item 5 (the dual substrate) is **defined** by the §4-cited
document [`../02_foundations/FOUND_FORCE_STRUCTURE.md`](../02_foundations/FOUND_FORCE_STRUCTURE.md).
The audit reads that document for the dual substrate's locked structure;
it is part of the frozen catalog, not an external import.

Where a group order or a rank is stated, it is **computed** by
`audit_colour_singlet_rank.py`, not asserted.

---

## §3 — The §9 method trace

### Step 1 — Catalog, marked colour-singlet (D2) and internal (D3)

| §4 item | Structure | Colour-singlet? | Internal? | Carried? |
|---|---|---|---|---|
| 1 | ternary alphabet, `Aut = ℤ₂` | yes | yes | **carry** |
| 2 | `O_h` spatial point group | yes | **no — spacetime** | drop (D3) |
| 2 | BCC triality `ℤ₃ → SU(3)` | **no — this is colour** | yes | drop (D2) |
| 3 | `ℤ[i]^× ≅ ℤ₄` | yes | yes | **carry** |
| 4 | 27-block `O_h` irreps | yes | **no — spatial `O_h`** | drop (D3) |
| 5 | dual substrate `(J_L, J_R)` | yes | yes | **carry** |
| 6 | framework integers `{3,4,7,13}` | — | — | drop (not groups) |

Three structures carry forward. The exclusion of `O_h` (items 2, 4) under
D3 is load-bearing: `O_h` is the *spatial* Moore-neighbourhood point
group; counting a spatial rotation as an internal `U(1)` is the
Coleman–Mandula-type error the pre-registration explicitly forbids.

**Dual-substrate decomposition** (from `FOUND_FORCE_STRUCTURE.md` Parts
III, VII — the §4-cited defining document). A dual-substrate vector field
`(J_L, J_R)` in `D = 3` has four mode types:

| Mode | Sector | Colour-singlet? | Forced internal symmetry |
|---|---|---|---|
| `J = J_L + J_R` | EM transverse vector | yes | `U(1)_EM` — the residual readout (lock *output*) |
| `φ = J_L − J_R` | weak / chirality pseudovector | yes | **parity `ℤ₂`** (`J_L  J_R`) only |
| `\|J\|` | gravity scalar | yes | none — a scalar has no internal `U(1)` |
| orientation of `J_R` | colour / `SU(3)` | **no — coloured** | BCC triality — excluded by D2 |

The colour sector is precisely the **internal orientation of `J_R`**; it
is removed by D2. The colour-singlet remainder is `{J, φ, |J|}`.

### Step 2–3 — Abelian shadows and the rank-1 classification

D1: a clean rank-1 `U(1)`-shadow is a cyclic group of order ≥ 3; `ℤ₂` is
excluded.

| Colour-singlet internal shadow | Order | Rank-1 `U(1)`-shadow? | Grade |
|---|---|---|---|
| ternary alphabet `Aut = ℤ₂` | 2 | **no** — `ℤ₂`, D1 excludes | — |
| `ℤ[i]^× = ℤ₄` | 4 | **yes** — cyclic, 4 ≥ 3 | **forced** |
| dual substrate `\|J\|` | — | no — scalar | — |
| dual substrate `J`: `U(1)_EM` | — | residual readout — the lock *output*, not a pre-breaking input rank-1 | — |
| dual substrate `φ`: parity `ℤ₂` | 2 | **no** — `ℤ₂`, D1 excludes | — |
| dual substrate `φ`: weak `SU(2)` Cartan | — | yes — one rank-1 Cartan | **`[SELECTION]`** |

`[DERIVED — computed].` The catalog's *forced* colour-singlet internal
rank-1 `U(1)`-shadows are exactly `{ℤ[i]^×}`. The two `ℤ₂`'s — the ternary
sign automorphism and the dual-substrate parity — are correctly rejected
by D1. The mode `φ = J_L − J_R` is structurally a real 3-component
pseudovector; its only *forced* internal symmetry is the parity `ℤ₂`.

`[SELECTION].` `FOUND_FORCE_STRUCTURE.md` additionally assigns a weak
`SU(2)` to `φ` (FST-1 `[SELECTION]`, FST-6 `[CONJECTURE]`,
[`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md)).
That `SU(2)` would contribute a second rank-1 — its Cartan. But the
assignment is selection-grade: nothing in the catalog *forces* `φ` to
carry an internal `SU(2)` rather than only its parity `ℤ₂`.

### Step 3–5 — Rank tally by reading

| Reading | Rank-1 `U(1)`-shadows counted | Rank |
|---|---|---|
| **Forced** (catalog-forced content only) | `{ℤ[i]^×}` | **1** |
| **`[SELECTION]`** (also the weak `SU(2)` Cartan on `φ`) | `{ℤ[i]^×, SU(2)_φ}` | **2** |

`[DERIVED — computed].` The rank is **not reading-invariant**: it is 1
under the forced reading and 2 under the `[SELECTION]` reading. By D6 it
is therefore **not forced**.

### Step 5–6 — Benchmark, falsifier, Q11c

No §7 falsifier fires:
- **F-a** (rank forced ≠ 2) — does not fire: the rank is not forced *at
  all*, neither to 1 nor to 2.
- **F-b** (rank 2 needs a new postulate) — does not fire: the weak-`SU(2)`
  reading sits in `FOUND_FORCE_STRUCTURE.md`, a §4-cited catalog document,
  as a `[SELECTION]` — not a structure invented for Q11.
- **F-d** (`ℤ₂` counted as a `U(1)`) — does not fire: both `ℤ₂`'s were
  rejected by D1.
- **F-e** (spacetime as internal) — does not fire: `O_h` was excluded by D3.
- **F-f / Q11c** — under the rank-2 `[SELECTION]` reading the second
  factor *is* a non-abelian `SU(2)`, so the structure is `SU(2)×U(1)`, not
  `U(1)×U(1)`; F-f would not fire. But the rank is not forced, so Q11c is
  moot for the verdict.

### Step 7 — Verdict

UNDERDETERMINED. The reasoning is §4.

---

## §4 — Why UNDERDETERMINED — not FOUND, not CLOSED-NEGATIVE

**Not FOUND.** Pre-reg §6 FOUND requires the rank to be **forced to
exactly 2**. It is not forced: the forced reading gives 1, the
`[SELECTION]` reading gives 2 (§3 step 3–5). A rank that depends on a
selection is not a forced rank.

**Not CLOSED-NEGATIVE.** Pre-reg §6 CLOSED-NEGATIVE requires the rank to
be **forced to ≠ 2**, or rank 2 to require a new postulate, or the rank-2
sector to be forced to `U(1)×U(1)`. None holds:
- The rank is not forced to 1 either — the rank-2 reading is admissible.
- The rank-2 reading uses the weak `SU(2)` on `φ`, which is in the
  catalog-cited `FOUND_FORCE_STRUCTURE.md` as a `[SELECTION]` — admissible,
  not a new postulate (F-b quiet).
- Under the rank-2 reading the structure is `SU(2)×U(1)`, not `U(1)×U(1)`
  (F-f quiet).

**The residue is exactly UNDERDETERMINED:** "a rank-2 reading exists but
the rank is not forced — some admissible reading gives a different rank"
(pre-reg §6). The audit records the unforced choice precisely: **whether
the dual-substrate chirality mode `φ = J_L − J_R` carries an internal weak
`SU(2)`, or only its forced parity `ℤ₂`.**

**Tag of the result.** The rank-2 reading is `[SELECTION PRINCIPLE —
open]`: the catalog contains the ingredients and the rank-2 reading is the
natural one, but it is not forced. The electroweak-rank match is
`[CONJECTURE]` until the §6 upgrade lands.

---

## §5 — The sharpened finding

UNDERDETERMINED here is **more informative than Q10's UNDERDETERMINED** —
it converts a diffuse gap into a single named claim.

- Q10 ([`AUDIT_FINITE_NEUTRAL_LOCK.md`](AUDIT_FINITE_NEUTRAL_LOCK.md))
  found the electroweak lock "assembled by an unforced choice" and named
  the choice loosely as "the rank-2 assembly and the `T₃`-vs-`Y`
  role-split."
- Q11 has now **localised that choice to one structure and one tag**: the
  missing second rank-1 is the **Cartan of the weak `SU(2)`**, and the
  weak `SU(2)` is present in FTD only as a `[SELECTION]`/`[CONJECTURE]`
  riding on the chirality mode `φ = J_L − J_R`
  (`FOUND_FORCE_STRUCTURE.md` FST-1, FST-6).

`[AUDIT FINDING].` The entire electroweak-rank question — and therefore
the FOUND-vs-CLOSED-NEGATIVE fate of Q10 — reduces to a **single,
pre-existing, auditable claim**: *is the weak `SU(2)` on `φ = J_L − J_R` a
genuine derivation, or a count-match?* This is consistent with the
state-of-the-theory roundtable's finding that FTD's gauge-group
identifications are largely selection-grade; Q11 confirms it from the rank
side and pins it to one document.

What the catalog **does** force, banked: `[DERIVED — computed]` exactly one
colour-singlet internal rank-1 `U(1)`-shadow, `ℤ[i]^× ≅ ℤ₄`. FTD has one
of the two electroweak dials without ambiguity.

---

## §6 — The upgrade path

To move this result from UNDERDETERMINED to a definite verdict, the
successor question — "Q12" — is sharp and already has a home document:

> **Q12.** Is the weak `SU(2)` on the dual-substrate chirality mode
> `φ = J_L − J_R` a genuine `[DERIVED]` consequence of the catalog, or a
> selection-grade count-match? Audit
> [`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md)
> and `FOUND_FORCE_STRUCTURE.md` FST-1/FST-6 against the boundary-theorem
> discriminator.

The two outcomes are both decisive:

- **If the weak `SU(2)` on `φ` is promoted `[SELECTION] → [DERIVED]`:** the
  second rank-1 becomes forced, the colour-singlet rank is forced to 2,
  and — since Q11c already shows the rank-2 structure is `SU(2)×U(1)` —
  **both Q11 and Q10 (FTD-0190) lift to FOUND.**
- **If the weak `SU(2)` is shown to be a count-match (not derivable):** the
  rank is forced to 1, and **Q10 closes negative** — the electroweak
  sector is, honestly, an effective continuum completion. Per goal-clause
  2, that is a mapped boundary.

Either way Q12 is finite and well-posed — an audit of one existing
derivation document, not a search. Until it runs, the electroweak rank
stays `[CONJECTURE]`-grade.

---

## §7 — Epistemic discipline

- **No spine tag moved.** Per pre-reg §6/§8, this audit promotes and
  demotes no LEDGER claim. It re-tags nothing in
  `FOUND_FORCE_STRUCTURE.md` or `DERIV_LATTICE_SU2_WEAK.md`; it *reads*
  their existing `[SELECTION]`/`[CONJECTURE]` tags and reports the
  consequence.
- **The verdict is the middle outcome.** FOUND was pre-blessed and
  available; the audit did not reach for it. The weak `SU(2)` on `φ` is a
  close *resemblance* to the electroweak structure — but `FOUND_FORCE_STRUCTURE.md`
  itself tags that assignment `[SELECTION]`/`[CONJECTURE]`, and the audit
  honours those tags rather than silently upgrading them (GTCA F9/F10).
- **Resemblance is not derivation.** Counting the weak `SU(2)` Cartan
  would give the "right" rank of 2 — but a tag does not resolve the
  question it labels. The rank-2 reading is `[SELECTION PRINCIPLE — open]`,
  not `[DERIVED]`.
- **CLOSED-NEGATIVE was a live option** and is recorded as not reached,
  with the failing gates enumerated (§4).

---

## §8 — LEDGER and cross-references

- **LEDGER:** FTD-0191 — the pre-registration row; this audit is its
  execution. The row is updated to record the UNDERDETERMINED verdict
  (status only; no tag promotion).
- **Pre-registration:** [`PREREG_COLOUR_SINGLET_RANK_v1.md`](PREREG_COLOUR_SINGLET_RANK_v1.md)
  (locked; unedited).
- **Predecessor:** [`AUDIT_FINITE_NEUTRAL_LOCK.md`](AUDIT_FINITE_NEUTRAL_LOCK.md)
  (Q10 / FTD-0190 — UNDERDETERMINED). Q11 is its successor; this audit
  leaves FTD-0190 UNDERDETERMINED and sharpens its open item.
- **Pre-registration registry:** [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md).
- **Verification script:** [`../../../scripts/proofs/audit_colour_singlet_rank.py`](../../../scripts/proofs/audit_colour_singlet_rank.py).
- **Catalog-defining sources:** [`../02_foundations/FOUND_FORCE_STRUCTURE.md`](../02_foundations/FOUND_FORCE_STRUCTURE.md)
  (the dual-substrate four-mode decomposition; FST-1/FST-6),
  [`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md)
  (the weak `SU(2)` — the Q12 audit target),
  [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
  (the `SU(3)` colour triality).

---

*Audit executed 2026-05-22 against the commit tagged
`preregister-colour-singlet-rank-v1`. Verdict: UNDERDETERMINED; Q10
(FTD-0190) stays UNDERDETERMINED. The electroweak-rank question is reduced
to one named `[SELECTION]` — the Q12 target (§6).*
