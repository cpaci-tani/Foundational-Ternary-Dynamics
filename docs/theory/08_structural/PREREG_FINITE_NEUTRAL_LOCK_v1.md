# Pre-Registration — Q10: The Finite Neutral-Lock Audit (v1)

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-22
**Status:** [PRE-REGISTRATION] — this document locks the question, definitions, admissible search space, success criterion, falsifier, and method for a **desk audit** (finite group theory; no engine, no numerics). It contains **no result**. All three outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are pre-blessed; the audit's verdict is genuinely open.
**LEDGER row:** FTD-0190 (provisional — confirm next-free id against `../07_assessment/core_ledgers/LEDGER.md` at hash-lock).
**Hash-lock:** to be SHA256-locked and git-tagged `preregister-finite-neutral-lock-v1` before the audit runs — see §11.

**Purpose.** Lock, *before* any matching to Standard-Model representation theory, (a) what would count as FTD possessing a **finite neutral-lock** object, and (b) what would **falsify** the claim that such an object's continuum shadow is the SM Higgs representation `(1,2)₁/₂`. This pre-registration is the anti-laundering instrument for Q10: it exists so the subsequent audit cannot become a post-hoc construct-to-match.

---

## §1 — Context and doctrine

**The doctrine (2026-05-22 owner direction; corroborated by `../07_assessment/ROUNDTABLE_STATE_OF_FTD_2026-05-22.md` §5).** FTD does not aim to recover the Standard Model as primitive ontology. The SM is a **benchmark shadow**. FTD aims to recover the **finite-closure invariants** whose continuum completion parallels SM observables where empirically tested. `SU(3)×SU(2)×U(1)` is treated as the *effective continuous completion* of finite centre/transport closure, not as a primitive object. **[Program statement — derives nothing on its own; F10 discipline applies.]**

**What Q10 targets.** The electroweak sector's defining function: a mechanism that breaks an internal symmetry while leaving exactly one massless electromagnetic readout. The SM realises this with the Higgs `(1,2)₁/₂` and `Q = T₃ + Y`, with the vacuum expectation value satisfying `Q⟨Φ⟩ = 0`. **[Standard electroweak theory.]**

**Cross-domain anchor — the stabiliser framing.** Stripped of representation theory, spontaneous symmetry breaking in *any* domain (particle physics, crystallography's residual point groups, the general theory of group actions) is one operation: **select a distinguished configuration; the unbroken symmetries are exactly its stabiliser (little group).** The Higgs mechanism's entire invariant content is therefore: *a distinguished configuration whose stabiliser is rank-1*. Q10 asks whether FTD's finite structure contains such a configuration. This reframing converts "derive the Higgs" — representation-theory-laden, the doctrine forbids it — into a finite, decidable group-theory question. **[Reframing; standard group theory.]**

---

## §2 — The question (LOCKED)

> **Q10.** Within FTD's established finite-closure structure (the frozen catalog of §4), does there exist a *minimal* object — a finite group action `G ↻ S` together with a distinguished configuration `v ∈ S` — such that:
> 1. `S` carries a genuine **two-state internal opposition** (D2);
> 2. `v` is paired with a **compensating ratio charge** (D3) so that exactly one **neutral readout** survives (D4);
> 3. the **stabiliser** `Stab_G(v)` is **rank-1** — a single residual `U(1)`-shadow (D5);
>
> and is that object **[DERIVED]** from the §4 catalog rather than constructed ad hoc?
>
> Conditional follow-up (Q10c): *if* such an object is found, is its continuum completion representation-consistent with the SM Higgs `(1,2)₁/₂` (§5)?

"Minimal" means: no proper sub-structure of the object also satisfies (1)–(3). If several catalog objects qualify, all are reported; minimality and uniqueness are recorded, not assumed.

---

## §3 — Definitions (LOCKED)

**D1 — Finite neutral lock.** A triple `(G, S, v)`: `G` a finite group, `S` a finite `G`-set or finite `G`-module, `v ∈ S` a distinguished configuration, satisfying D2–D5. The "lock" is the constraint that the non-stabilising generators of `G` move `v` (the broken/massive sector) while `Stab_G(v)` fixes it (the unbroken/massless sector).

**D2 — Two-state internal opposition.** A `ℤ₂`-graded or sign-opposed pair of states within `S`, exchanged by an automorphism `τ ∈ Aut(S)` with `τ² = id` (the `T₃`-analog generator). The opposition is **genuine** only if `τ` is an automorphism of the *structure*, not a relabelling — i.e. the two states are not separately `G`-fixed.

**D3 — Compensating ratio charge.** An abelian charge `Y : S → (1/k)ℤ` (a rational charge, "ratio" in the FTD sense — cf. the charge-quartic `e² = 1/x`) such that on `v` the combination `Q := T₃ + Y` vanishes (D4). `Y` must be **exhibited from §4 catalog structure**, not posited.

**D4 — Neutral readout.** The distinguished `v` with `Q·v = 0`, where `Q` is the unbroken generator. "Neutral" = `Q` annihilates `v`. The audit must identify which generator is `Q`.

**D5 — Rank-1 stabiliser / single residual U(1)-shadow.** `Stab_G(v)` must be the finite/compact shadow of a **rank-1** abelian group — exactly one `U(1)`. Rank 0 (no unbroken generator → no EM) and rank ≥ 2 (extra unbroken `U(1)` → wrong low-energy spectrum) both fail.

**D6 — Continuum completion / shadow (the matching criterion).** A finite object `O` *shadows* a continuum representation `R` of `H₃ × H₂ × H₁` iff **all** hold: (i) `O`'s automorphism structure has a continuous/compact-Lie completion containing the `SU(2)`-shadow under which `O`'s two opposed states form a doublet; (ii) `O` is a **singlet** under FTD's `SU(3)`-shadow (the BCC triple-cosine / `ℤ₃`-triality structure of `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`); (iii) the abelian charges on `O`'s two states are `(T₃ + Y) = (−½+Y, +½+Y)` with `Y` fixed by D4, yielding surviving `Q`-charges `(0, +1)` or `(−1, 0)` — exactly one neutral, one unit-charged.

---

## §4 — Admissible search space (LOCKED)

The audit may draw **only** on finite structures already established in the FTD corpus. This freeze is the primary anti-laundering control: a finite group invented to order does not count as "FTD has the lock."

The frozen catalog:

1. The ternary state alphabet `{−1, 0, +1}` and its automorphism group.
2. The Moore-neighbourhood point groups — `O_h` (`|O_h| = 48`), the cuboctahedral group, the BCC sublattice structure (`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`, `THEOREM_MOORE_LAYER_DECOMPOSITION.md`).
3. `ℤ[i]` and its unit group `ℤ[i]^×` (cyclic of order 4 = `N_base`).
4. The 27-block representation structure and its `O_h` irreps (`A_{1g}`, `T_{1u}`, `E_g`, `T_{2g}`, …; `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`).
5. The dual substrate `(J_L, J_R)` and its parity / orientation structure (`FOUND_FORCE_STRUCTURE.md`, the parity-twist of `EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`).
6. The four framework integers `{N_c = 3, N_base = 4, b_3 = 7, N_eff = 13}`.

Any finite group or action **not reducible to the above** is a **new postulate**. It may appear in the audit, but the object built on it is tagged `[CONJECTURE — new postulate]` and **does not** support a FOUND verdict (§6).

---

## §5 — Benchmark: the SM Higgs `(1,2)₁/₂` (LOCKED)

Notation: `(d₃, d₂)_Y` = `(SU(3)_c` dim, `SU(2)_L` dim`)` at weak hypercharge `Y`, convention `Q = T₃ + Y`. The SM Higgs `Φ` is `(1,2)₁/₂`: colour singlet, weak doublet, `Y = +½`. Components: upper `T₃ = +½ → Q = +1`; lower `T₃ = −½ → Q = 0`. The vev occupies the **lower (neutral)** component; `Q⟨Φ⟩ = −½ + ½ = 0`. **[Standard electroweak theory.]**

**Load-bearing structural fact for Q10.** `Y_Φ = +½` is **not a free SM input** — it is forced by the requirement `Q⟨Φ⟩ = 0` given `T₃ = −½` on the vev component. Consequence for the audit: if Q10b delivers a two-state opposition with a *derived* `±½`-analog normalisation, the hypercharge of `(1,2)₁/₂` is **not a separate quantity to derive** — it falls out of D4. The benchmark's hardest-looking number is the cheapest part, conditional entirely on Q10b. **[Standard EW theory; consequence noted.]**

`(1,2)₁/₂` is a **falsification benchmark**, never a target to engineer toward. The audit author must be able — and willing — to write the CLOSED-NEGATIVE report (§6).

---

## §6 — The three pre-registered outcomes (LOCKED)

The audit returns exactly one verdict.

**FOUND.** A triple `(G, S, v)` satisfying D1–D5 is exhibited with an explicit construction trace from the §4 catalog (no new postulate). Tag consequences:
- The **object** is tagged `[DERIVED]` only if every step of the construction trace is itself `[THEOREM]`/`[DERIVED]` from the catalog; otherwise `[SELECTION]`.
- The **shadow-match** to `(1,2)₁/₂` (Q10c via D6) is tagged `[SELECTION PRINCIPLE]` **iff** the rank-1-stabiliser object is *unique* in the §4 catalog; `[STRONGLY MOTIVATED CONJECTURE]` if it is one of a small rigid set; `[CONJECTURE]` otherwise.
- No outcome promotes `x₊ = 1/α` or any LEDGER claim. Q10 is electroweak-structural; it touches no spine tag.

**UNDERDETERMINED.** A rank-1-stabiliser object exists, but either (a) selecting it requires an unforced choice among catalog structures, or (b) its shadow is consistent with `(1,2)₁/₂` *and* with other representations. Tag: `[SELECTION PRINCIPLE — open]` or `[CONJECTURE]`. The audit records exactly which choice is unforced.

**CLOSED-NEGATIVE.** No object in the §4 catalog has a rank-1 stabiliser carrying a genuine two-state opposition (D2–D5); or every candidate requires a new postulate (§4); or the best candidate's shadow is provably **not** `(1,2)₁/₂` (a falsifier of §7 fires). Tag: `[CLOSED NEGATIVE]`. **Per the project's stated goal-clause 2 ("rigorously establish what we cannot derive"), a CLOSED-NEGATIVE here is itself a deliverable** — it would establish that FTD's finite structure does *not* contain the electroweak neutral-lock invariant, and the EW sector therefore remains an honest parametric/effective completion. That is a mapped boundary, not a failure.

---

## §7 — The falsifier (LOCKED)

"The finite neutral lock's shadow is `(1,2)₁/₂`" is **falsified** if any of:

- **F-a.** The minimal rank-1-stabiliser object is **not** an `SU(3)`-shadow singlet (it transforms non-trivially under the BCC/triality colour structure) — then it is a coloured representation, not `(1,2)₁/₂`.
- **F-b.** `Stab_G(v)` has rank ≠ 1.
- **F-c.** The two opposed states' surviving `Q`-charges are not `(neutral, unit)` — e.g. both charged, or a fractional pattern inconsistent with a lepton-type doublet.
- **F-d.** Producing the object requires introducing a free integer, exponent, parameter, or finite group **not** in the §4 catalog.
- **F-e.** The hypercharge required is **not** the value forced by `Q⟨v⟩ = 0` given the object's derived `T₃` normalisation — i.e. `Y` would have to be inserted by hand.

Any single firing → the shadow-match is rejected (the audit proceeds to UNDERDETERMINED or CLOSED-NEGATIVE).

---

## §8 — Banned moves / anti-laundering (LOCKED)

- **No new free integers, exponents, or finite groups** introduced to achieve a match. The §4 catalog is frozen.
- **No post-hoc gauge assignment.** Which FTD structure plays the role of which gauge factor must be motivated *independently* of the `(1,2)₁/₂` target.
- **Hypercharge is derived, not inserted** — see F-e. Any claimed `Y` value cites D4.
- **`(1,2)₁/₂` is checked, never engineered toward.** The CLOSED-NEGATIVE report must be a live option throughout.
- **No CODATA, no numerical-precision claims.** Q10 is structural (finite group theory). No "closes to N digits" statement is admissible.
- **A structural resemblance is not a derivation** (GTCA F1/F10). The object is `[DERIVED]` only with an explicit construction trace; absent that, `[CONJECTURE]` is the ceiling. A correctly-applied tag does not resolve the question it labels.
- **No spine tag moves.** Q10 must not promote, demote, or re-tag any existing LEDGER claim.

---

## §9 — Method (LOCKED)

The audit executes exactly these steps and reports each:

1. **Enumerate** the finite group structures of the §4 catalog and their natural actions.
2. **Enumerate distinguished configurations** for each action — fixed points and special vectors (e.g. the `A_{1g}` singlet in the 27-block; the void state `0` in the ternary alphabet; the unit configurations of `ℤ[i]^×`; orientation-distinguished states of the dual substrate).
3. For each `(structure, configuration)` pair, **compute `Stab_G(v)` and record its rank.**
4. **Filter** to rank-1-stabiliser candidates (D5).
5. For each survivor, **test D2–D4**: genuine two-state opposition, exhibitable compensating charge, `Q·v = 0`.
6. For survivors of step 5, **apply D6 and the §7 falsifier** against `(1,2)₁/₂`.
7. **Report** the verdict (§6) with either the full construction trace (FOUND/UNDERDETERMINED) or the specific obstruction / firing falsifier (CLOSED-NEGATIVE).

The audit is a desk computation: small finite groups, character tables, stabiliser calculations. Where a stabiliser or orbit count is claimed, it is computed (sympy / explicit enumeration), not asserted (GTCA tool-use discipline).

---

## §10 — What this pre-registration locks vs leaves open

**Locked by the hash** (§11): the question (§2), definitions D1–D6 (§3), the admissible catalog (§4), the benchmark decomposition (§5), the three outcomes and their tag consequences (§6), the falsifier F-a…F-e (§7), the banned moves (§8), the method (§9).

**Open** — and only this: the **verdict**. Whether FTD's finite structure contains the neutral lock, and if so what its shadow is, is exactly what the audit will determine. FOUND, UNDERDETERMINED, and CLOSED-NEGATIVE are all permitted; none is favoured.

---

## §11 — Hash-lock protocol

To lock this pre-registration before the audit runs:

1. Finalise this document. Compute `sha256sum docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md`.
2. Record the SHA256 in `../10_eft_program/REF_PREREGISTER_MANIFEST.md` (new row) and add a `07_assessment/core_ledgers/LEDGER.md` row (FTD-0190 or next-free) tagged `[PRE-REGISTRATION]`, citing this file.
3. `git commit` the pre-registration; create a lightweight tag `git tag preregister-finite-neutral-lock-v1 -m "Pre-reg for Q10: finite neutral-lock audit"`.
4. The audit (executing §9) runs only against the tagged commit. Its result lands in a separate `FOUND_FINITE_NEUTRAL_LOCK.md` (or `AUDIT_…` / `…_CLOSED_NEGATIVE.md` per the verdict) — never by editing this file.
5. If a definition here proves defective once the audit starts, the correct response is a **v2 pre-registration**, not an edit to v1 (cf. the FTD-0186 boundary-theorem v1→v2 precedent).

---

*Pre-registration authored 2026-05-22. No result. The audit (§9) is the next step, and runs only after hash-lock.*
