# Audit — Q12: The Weak-SU(2) Provenance Audit (Result)

**Tag:** [AUDIT FINDING] — this document is the **result** of executing
the hash-locked pre-registration `PREREG_WEAK_SU2_PROVENANCE_v1.md`. It
records one of the three pre-blessed verdicts. It promotes and demotes no
LEDGER claim; where it finds a mistag it *recommends* a correction (§8).
**Date:** 2026-05-22
**LEDGER row:** FTD-0192.
**Pre-registration audited:** [`PREREG_WEAK_SU2_PROVENANCE_v1.md`](PREREG_WEAK_SU2_PROVENANCE_v1.md)
— SHA256 `25ee75f4cf472841bf79a2c14495728731b2b2c27f5395ab28f3b30ea2c61784`,
git tag `preregister-weak-su2-provenance-v1`, lock commit `af49d80`. The
pre-registration was **not edited** during the audit (pre-reg §11 step 4).
**Frozen targets read:** [`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md)
(v1.0, primary) and [`../02_foundations/FOUND_FORCE_STRUCTURE.md`](../02_foundations/FOUND_FORCE_STRUCTURE.md)
(FST-1, FST-6), as they exist at the lock commit.

---

## §1 — Verdict

> **COUNT-MATCH** (pre-reg §6).
> **Consequence: Q10 (FTD-0190) closes negative; Q11 (FTD-0191) resolves
> to CLOSED-NEGATIVE. The Q10 → Q11 → Q12 chain terminates.**

**[AUDIT FINDING].** The weak `SU(2)` of FTD's electroweak sector is **not
a genuine derivation**. The load-bearing step of its construction — the
claim that FTD's ternary doublet `{|+⟩, |−⟩}` "carries the fundamental
representation of `SU(2)`" (`DERIV_LATTICE_SU2_WEAK.md` Theorem 1.1) —
rests entirely on the **count** `dim{+1,−1} = 2 = dim(SU(2) fundamental)`,
together with the standard fact that *any* 2-dimensional complex space
admits an `su(2)` action. The off-diagonal generators `T₁, T₂` are the
imported Pauli matrices, given FTD-flavoured names but no FTD
construction. Falsifier **F-a** (count-match) and **F-e** (skeleton-only)
both fire on load-bearing steps.

**What FTD genuinely has** `[DERIVED]`: the `SU(2)` **skeleton** — the
Cartan `T₃ = ŝ/2` (the ternary state-charge operator) and the Weyl-`ℤ₂`
(the transmutation flip `+1  −1`). This is exactly the skeleton Q10
already credited. The skeleton is the skeleton of *any* rank-1 structure;
it does not single out `SU(2)`.

**What is count-matched:** the non-abelian group itself — the off-diagonal
generators and the continuous `SU(2)`.

**The terminating consequence.** Q11 (FTD-0191) returned UNDERDETERMINED
because it could not tell whether the weak `SU(2)` — the candidate second
rank-1 — was a genuine structure or a count-match, and explicitly deferred
that to Q12. Q12 finds **count-match**. A count-match is not a genuine
rank-1 `U(1)`-shadow (Q11 D1). The colour-singlet internal abelian rank is
therefore **forced to 1**, not 2. Q11's open question resolves to
**CLOSED-NEGATIVE** (rank forced ≠ 2), and Q10 (FTD-0190) — whose FOUND
required the rank-2 lock — **closes negative**.

**The mapped boundary.** FTD's discrete ontology determines the
electroweak **skeleton** (a rank-1 Cartan, a Weyl-`ℤ₂`, the `±½` doublet
weights of `ℤ[i]^×`, established across Q10/Q11) but **not** the rank-2
non-abelian `SU(2)×U(1)` structure. The electroweak sector is, honestly,
an **effective / parametric continuum completion**. Per the project's #1
goal, clause 2 ("rigorously establish what we *cannot* derive"), this is a
**deliverable** — a boundary mapped precisely, not a failure.

---

## §2 — What was audited, and the discipline

The audit executed the locked §9 method against the frozen targets, the
D1–D5 definitions, the §4 benchmark, and the §7 falsifier — all as fixed
by the hash. The pre-registration was locked (commit `af49d80`) **before**
`DERIV_LATTICE_SU2_WEAK.md` was read critically; the trace below is the
first critical reading.

The §8 banned moves were observed: no engineering toward GENUINE; no
generator-count accepted as a construction; no SM read-back; the source
document's own tags are reported, not silently changed; **the audit
re-tags nothing** — §8 records a recommendation only.

Per §9, the audit is a documentary classification, not a computation; it
permits but does not require a script. None was written — a script here
would only restate a reading judgement. The artifact is the step ledger
(§3): each step cites the exact location in the frozen target for
independent check.

---

## §3 — The §9 method trace: the step ledger

`DERIV_LATTICE_SU2_WEAK.md` §1 constructs the weak `SU(2)`. Its steps:

| # | Step (location) | Content | Classification |
|---|---|---|---|
| S1 | §1.1 | ternary state space `{−1,0,+1}`; the void `0` distinguished from `{+1,−1}` | `[AXIOM]` + `[DERIVED]` — **genuine** (the void *is* structurally distinguished) |
| S2 | §1.1 | `{+1,−1}` labelled "the weak isospin doublet" | **count-match** — the label rests on `\|{+1,−1}\| = 2 = dim(SU(2) fundamental)`; §1.1's "it follows from the structure of the state-flux coupling" is asserted, not shown |
| S3 | §1.2, Thm 1.1 | "the Pauli matrices restricted to `{+1,−1}` generate `su(2)`" | **count-match** — the proof is verbatim "the Pauli matrices satisfy `[σ_i,σ_j]=2iε_{ijk}σ_k`": a *generic* fact about any `ℂ²`, not an FTD derivation. **Load-bearing.** |
| S4 | §2.2 | `T₃ = ŝ/2`, `ŝ` the ternary state-charge operator | `[DERIVED]` — **genuine** FTD Cartan |
| S5 | §1.2, §2.1 | transmutation `+1  −1` identified with `T₊, T₋` | the flip is `[DERIVED]` (CLAUDE.md Ch 6.5 stress-threshold rule) — but it is the **Weyl-`ℤ₂`**, a discrete reflection, not a continuous `su(2)` ladder operator; the identification conflates skeleton with group |
| S6 | §1.2 | off-diagonal generators `T₁ = ½σ₁`, `T₂ = ½σ₂` | `[IMPORTED]` — written as Pauli matrices; **no FTD construction**. Fires **F-e**. |

**Load-bearing steps** (D4 — removal collapses the `SU(2)` conclusion):
S2, S3 (the `SU(2)` claim itself) and S6 (the off-diagonal generators).
S2 and S3 are count-matches; S6 fires F-e.

**Genuine FTD content:** S1 (the ternary space + the distinguished void),
S4 (the Cartan `ŝ/2`), and the flip of S5 (the Weyl-`ℤ₂`). Together these
are the `SU(2)` **skeleton** — a Cartan and a Weyl reflection — and
nothing more.

---

## §4 — A second finding: the home discrepancy

The two frozen targets place the weak `SU(2)` on **two different
structures**:

- `DERIV_LATTICE_SU2_WEAK.md` (the primary derivation) builds it on the
  **ternary state doublet** `{+1,−1} ⊂ {−1,0,+1}`.
- `FOUND_FORCE_STRUCTURE.md` (Parts III/VII, FST-1) builds it on the
  **dual-substrate chirality mode** `φ = J_L − J_R`.

`[AUDIT FINDING].` These are not the same structure. A genuinely *derived*
group would have one structural home; a claim with two incompatible homes
is a symptom of the group being recognised (by its expected role) rather
than constructed. The Q12 pre-registration inherited Q11's "`φ`"
localisation in its question wording; the audit finds the primary
derivation document uses the ternary doublet instead. This does not make
Q12 undecidable — the verdict is **robust across both homes**:

- The ternary-doublet home is a **count-match** (§3, S2/S3).
- The `φ`-home is, by `FOUND_FORCE_STRUCTURE.md`'s own tags, `[SELECTION]`
  (FST-1) / `[CONJECTURE]` (FST-6); Q11
  ([`AUDIT_COLOUR_SINGLET_RANK.md`](AUDIT_COLOUR_SINGLET_RANK.md))
  established `φ`'s only *forced* internal symmetry is the parity `ℤ₂`.

Neither home yields a genuine derivation. No v2 is required (pre-reg §11
step 5): the question — "is FTD's weak `SU(2)` genuinely derived?" —
remains decidable, and is decided.

---

## §5 — Why COUNT-MATCH — not GENUINE, not PARTIAL

**Not GENUINE.** Pre-reg §6 GENUINE requires every load-bearing step to be
`[THEOREM]`/`[DERIVED]`, with the off-diagonal generators and the bracket
constructed from the catalog. They are not: S6 imports `T₁, T₂` as Pauli
matrices with no FTD construction (F-e), and S3's proof is the generic
`ℂ² ↦ su(2)` fact (F-a). The §4 benchmark — construct `T₁, T₂` and
`[T₁,T₂]=iT₃` from the catalog — is not met. Only the Cartan/Weyl skeleton
is FTD-grounded, and the skeleton is exactly what Q10 already had when it
returned UNDERDETERMINED.

**Not PARTIAL.** Pre-reg §6 PARTIAL requires a load-bearing step that is a
genuine `[SELECTION]` the audit "can neither promote nor mark a definite
count-match." The load-bearing step S3 **is** a definite count-match: its
proof, verbatim, invokes only the dimensionality of the doublet and the
standard Pauli relations. It is not a vague selection — it is a specific,
identifiable count-match (`dim = 2`). PARTIAL is therefore excluded by the
pre-registration's own wording.

**COUNT-MATCH.** A load-bearing step (S2, S3) is a count-match (D2); F-a
fires. The verdict is COUNT-MATCH.

This verdict is consistent with `FOUND_FORCE_STRUCTURE.md`'s own honesty
(it tags the weak-`SU(2)` assignment `[SELECTION]`/`[CONJECTURE]`) and
with the state-of-the-theory roundtable's finding that FTD's gauge-group
identifications are largely count-matches. Q12 confirms it for the
electroweak `SU(2)` with an explicit step-by-step trace.

---

## §6 — What is genuine, banked

A COUNT-MATCH verdict is not "the weak sector is empty." Three results
survive and are worth stating precisely:

1. **The electroweak skeleton is genuine** `[DERIVED]`. FTD has a Cartan
   (`T₃ = ŝ/2`, the ternary state-charge) and a Weyl-`ℤ₂` (the
   transmutation flip). Across Q10/Q11 it also has the `±½` doublet
   weights and the `ℤ[i]^×` rank-1 `U(1)`-shadow. The skeleton is real.
2. **The void is a genuine `SU(2)` singlet** `[DERIVED]` (Theorem 1.2):
   the distinguished void state `0` is annihilated by the generators —
   this follows from the genuine `{+1,−1}/{0}` partition (S1), independent
   of whether the `SU(2)` itself is derived.
3. **What is missing is sharply named:** the non-abelian step. FTD has the
   rank-1 skeleton; the rank-2 non-abelian `SU(2)` is count-matched. The
   gap is not diffuse — it is exactly the construction of `T₁, T₂` and
   `[T₁,T₂]=iT₃` from substrate dynamics (the §4 benchmark).

---

## §7 — The terminating consequence

Q12 ends the Q10 → Q11 → Q12 chain. The propagation:

- **Q12:** the weak `SU(2)` is a **count-match** — not a genuine rank-1.
- **Q11 (FTD-0191):** its rank-2 reading rested on the weak `SU(2)` being
  an admissible second rank-1. A count-match is not a rank-1 `U(1)`-shadow
  (Q11 D1). The rank-2 reading collapses → the colour-singlet internal
  abelian rank is **forced to 1** → Q11's open question resolves to
  **CLOSED-NEGATIVE** (rank forced ≠ 2; Q11 pre-reg §6).
- **Q10 (FTD-0190):** FOUND required the rank-2 → rank-1 electroweak lock.
  With the rank forced to 1, the lock cannot assemble → Q10
  **closes negative** (Q10 pre-reg §6 CLOSED-NEGATIVE; Q12 pre-reg §6).

`[AUDIT FINDING].` **FTD's discrete ontology does not determine the
electroweak `SU(2)×U(1)` rank-2 structure.** It determines the skeleton
and stops. The electroweak sector — the `SU(2)` group, and downstream of
it the `W/Z` content — is an honest effective / parametric continuum
completion. This terminates the chain: COUNT-MATCH is a definite verdict,
not a deferral; there is no "Q13".

This is a **closed negative of project-goal-clause-2 type**: it maps,
rigorously and with a step-by-step trace, a boundary of what the discrete
ontology reaches. It removes a standing ambiguity. It is a result.

---

## §8 — Recommendation (not performed by this audit)

Per pre-reg §8, the audit re-tags nothing; it records one recommendation
for a separate, owner-approved canonical-change action
(`META_STRUCTURE.md` protocol):

- **`DERIV_LATTICE_SU2_WEAK.md` claim SU2-1** ("SU(2) generators from
  ternary doublet") is tagged `[THEOREM]`. The genuine theorem-content of
  §1.2 is the *generic* fact that `ℂ²` carries `su(2)`; the *FTD-derivation*
  content ("FTD's ternary doublet carries `su(2)` as a forced structure")
  is a count-match. **Recommendation:** correct SU2-1 from `[THEOREM]` to
  `[SELECTION]` (count-match), with a one-line note that the Cartan/Weyl
  skeleton is `[DERIVED]` and the non-abelian group is not. This is a
  recommendation; the audit does not perform it.

Out of scope, explicitly: this audit does **not** re-litigate
`sin²θ_W = 3/13` (already `[PARAMETRIC]`, FTD-0018), `G_F = 1/(√2v²)`, the
`W/Z` masses, or the ~50 weak decay rates. Q12's scope is the `SU(2)`
*group*; those quantities are separately tagged and downstream.

---

## §9 — Epistemic discipline

- **No spine tag moved.** Q12 promotes and demotes no LEDGER claim. The
  FTD-0190 and FTD-0191 verdict updates (§7) are **pre-registered
  consequences** — both pre-registrations explicitly pre-blessed "Q12
  COUNT-MATCH → Q10 closes negative" — not new claims or tag moves.
- **The negative verdict was held to the same bar as a positive one.**
  COUNT-MATCH is a strong claim; it is supported by a verbatim trace of
  the load-bearing proof (§3, S3), not by impression. GTCA F1 (overreach)
  applies to negative findings too — the audit confirms the skeleton *is*
  genuine (§6) and keeps the verdict scoped to the `SU(2)` group.
- **The target document's partial honesty is acknowledged.**
  `DERIV_LATTICE_SU2_WEAK.md` tags `W±`, `Z⁰`, and `V−A` `[SELECTION]`,
  and its §8.4 lists `V−A`, anomaly cancellation, and radiative
  corrections as not rigorous. The single overclaim is SU2-1's
  `[THEOREM]` (§8). The verdict is not "the document is dishonest"; it is
  "the `SU(2)` group is count-matched."
- **GENUINE was pre-blessed and available** — the audit did not reach for
  it. A 2-state structure *resembling* an `SU(2)` doublet is not a
  derivation of `SU(2)` (GTCA F1/F10).

---

## §10 — LEDGER and cross-references

- **LEDGER:** FTD-0192 — the pre-registration row; this audit is its
  execution. The row is updated to record the COUNT-MATCH verdict.
  FTD-0190 (Q10) and FTD-0191 (Q11) rows are updated to record the
  pre-registered consequence (Q10 closes negative; Q11 → CLOSED-NEGATIVE).
- **Pre-registration:** [`PREREG_WEAK_SU2_PROVENANCE_v1.md`](PREREG_WEAK_SU2_PROVENANCE_v1.md)
  (locked; unedited).
- **The Q-chain:** [`AUDIT_FINITE_NEUTRAL_LOCK.md`](AUDIT_FINITE_NEUTRAL_LOCK.md)
  (Q10, FTD-0190) → [`AUDIT_COLOUR_SINGLET_RANK.md`](AUDIT_COLOUR_SINGLET_RANK.md)
  (Q11, FTD-0191) → this audit (Q12, FTD-0192, terminating).
- **Pre-registration registry:** [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md).
- **Frozen audit targets:** [`../03_derivations/DERIV_LATTICE_SU2_WEAK.md`](../03_derivations/DERIV_LATTICE_SU2_WEAK.md)
  (the primary derivation; SU2-1 is the §8 recommendation target),
  [`../02_foundations/FOUND_FORCE_STRUCTURE.md`](../02_foundations/FOUND_FORCE_STRUCTURE.md)
  (the `φ`-home; FST-1/FST-6).

---

*Audit executed 2026-05-22 against the commit tagged
`preregister-weak-su2-provenance-v1`. Verdict: COUNT-MATCH. The
Q10 → Q11 → Q12 chain terminates: FTD's discrete ontology determines the
electroweak skeleton but not the rank-2 `SU(2)×U(1)` structure; the
electroweak sector is an honest effective continuum completion — a mapped
boundary (goal-clause 2).*
