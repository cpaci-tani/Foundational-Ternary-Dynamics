# Audit — Q10: The Finite Neutral-Lock Audit (Result)

**Tag:** [AUDIT FINDING] — this document is the **result** of executing the
hash-locked pre-registration `PREREG_FINITE_NEUTRAL_LOCK_v1.md`. It records
one of the three pre-blessed verdicts. It promotes and demotes no LEDGER
claim.
**Date:** 2026-05-22
**LEDGER row:** FTD-0190.
**Pre-registration audited:** [`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](PREREG_FINITE_NEUTRAL_LOCK_v1.md)
— SHA256 `41c3f86584270d59fd25736bfec3cee3efb6a656d34f12be44b93272e57ae346`,
git tag `preregister-finite-neutral-lock-v1`, lock commit `454b2f2`. The
pre-registration was **not edited** during the audit (pre-reg §11 step 4).
**Verification artifact:** [`../../../scripts/proofs/audit_finite_neutral_lock.py`](../../../scripts/proofs/audit_finite_neutral_lock.py)
— enumerates the frozen §4 catalog; computes every stabiliser, orbit, and
charge claimed below; all assertions pass, exit 0. No numerical search.

---

> **Update (2026-05-22) — Q10 closed negative; the chain has terminated.**
> The Q10 → Q11 → Q12 audit chain ran to completion. Q12
> ([`AUDIT_WEAK_SU2_PROVENANCE.md`](AUDIT_WEAK_SU2_PROVENANCE.md),
> FTD-0192) found the weak `SU(2)` is a **count-match**, which forces the
> colour-singlet rank to 1, so the rank-2 → rank-1 electroweak lock
> cannot assemble. Q10's UNDERDETERMINED verdict below therefore
> **resolves to CLOSED-NEGATIVE** — the pre-registered consequence (§6
> CLOSED-NEGATIVE). FTD's discrete ontology determines the electroweak
> skeleton but not the rank-2 `SU(2)×U(1)` structure; the electroweak
> sector is an honest effective continuum completion — a mapped boundary
> (goal-clause 2). The §1 verdict and the audit trace below stand as the
> correct Q10-stage record; the closure is the chain-level result.

---

## §1 — Verdict

> **UNDERDETERMINED** (pre-reg §6, clause **a**).

**[AUDIT FINDING].** FTD's frozen finite-closure catalog (pre-reg §4)
supplies *every separate ingredient* of the finite neutral-lock skeleton —
a genuine two-state internal opposition; the doublet's `±½` charge
normalisation; a rank-1 `U(1)`-shadow for the residual readout; colour-
singlet compatibility; and the hypercharge forced rather than inserted. It
does **not** supply a *forced assembly* of those ingredients into a single
rank-2 → rank-1 lock. No catalog theorem delivers the colour-singlet
rank-2 `SU(2)×U(1)` shadow as one object, and the role-assignment (which
catalog `U(1)`-shadow is `T₃`'s Cartan, which is `Y`) is an unforced
choice. The audit therefore lands on UNDERDETERMINED: not FOUND (the
construction trace is not all-`[THEOREM]`/`[DERIVED]`), not CLOSED-NEGATIVE
(no §7 falsifier provably fires against the surviving candidate).

**No falsifier fired.** UNDERDETERMINED is a clean pre-blessed landing
(contrast FTD-0186, where the v1 falsifier fired and forced a v2). The
pre-registration is **satisfied**, and its single open item — the verdict
(pre-reg §10) — is now closed.

**One-paragraph plain reading.** Spontaneous symmetry breaking, stripped
of representation theory, is one operation: pick a distinguished
configuration; the survivors are its stabiliser (little group). The
electroweak question is whether FTD's finite structure contains a
configuration whose stabiliser is rank-1 — one residual `U(1)`. The audit
finds FTD has the *parts* of such a configuration, cleanly and from the
frozen catalog, but the *rank-2 group whose breaking it would complete* is
not a derived catalog object. FTD has the pieces of the electroweak
neutral lock; it does not yet have the assembled lock. That gap is sharp,
named, and is the natural next research target (§6).

---

## §2 — What was audited, and the discipline

The audit executed the locked §9 method against the §4 frozen catalog, the
D1–D6 definitions, the §5 benchmark, and the §7 falsifier — all as fixed by
the hash. The §8 banned moves were observed throughout: no new free
integer, exponent, parameter, or finite group was introduced; no post-hoc
gauge assignment was made; no CODATA or precision claim appears; structural
resemblance was nowhere treated as derivation (GTCA F1/F10).

Where a stabiliser or orbit count is stated, it is **computed** by
`audit_finite_neutral_lock.py`, not asserted — small finite groups, signed
permutation enumeration, explicit charge arithmetic. The audit is a desk
computation on a frozen catalog; it is not, and contains no, search.

---

## §3 — The §9 method trace

### Step 1 — Catalog finite groups and natural actions

| § 4 item | Finite group | Natural action |
|---|---|---|
| 1 — ternary alphabet `{−1,0,+1}` | `Aut = ℤ₂ = ⟨τ : x ↦ −x⟩` | on the 3-state set |
| 2 — Moore point groups | `O_h ≅ S₄×ℤ₂`, order 48 | on ℝ³ / the 26 neighbours / the 27-block |
| 2 — BCC sublattice | `O_h` with the triple-cosine `ℤ₃`-triality | on the 8 BCC corner-sites → `SU(3)`-shadow |
| 3 — `ℤ[i]^×` | `ℤ₄ = ⟨i⟩` (cyclic, order 4 = `N_base`) | multiplication on `ℤ[i]`; conjugation `c:i↦−i` |
| 4 — 27-block irreps | `O_h` acting; `A_{1g}` (mult 4), `T_{1u}`, `E_g`, … | on the 27-dim permutation module |
| 5 — dual substrate | `ℤ₂ = ⟨P : J_L  J_R⟩` | on `(J_L, J_R)` |
| 6 — framework integers | *not groups* — `{3,4,7,13}` | — (invariants, not actions) |

`|O_h| = 48` is computed (signed-permutation enumeration); the conjugation
map `c` on `ℤ[i]^×` is verified to be an automorphism with `c² = id`.

### Step 2 — Distinguished configurations

Per action: the void `0` and the opposed pair `{+1,−1}` (ternary); the
centre site and the four `A_{1g}` shell-sum vectors (27-block); `0` and the
units (`ℤ[i]` under `ℤ₄`); the conjugation orbit `{i,−i}` (`ℤ[i]^×`); the
parity-even and parity-odd configurations (dual substrate).

### Step 3 — Stabilisers and rank

`Stab_G(v)` computed for each pair; the rank reading applies D5 ("the
finite shadow of **exactly one** `U(1)`").

| Configuration | `Stab_G(v)` | Single `U(1)`-shadow? |
|---|---|---|
| ternary `v = 0` | `ℤ₂` (whole group) | ambiguous — `ℤ₂` is a Weyl/`μ₂`, not a clean `U(1)` |
| ternary `v = ±1` | trivial | no (rank 0) |
| `ℤ[i]` `v = 0` | `ℤ₄` (whole group) | **yes** — `ℤ₄` is a genuine `U(1)`-shadow |
| `ℤ[i]` `v = unit` | trivial | no (rank 0) |
| 27-block `v = centre` / `A_{1g}` sums | `O_h` (whole group) | no — non-abelian |

### Step 4 — Filter to rank-1 `U(1)`-shadow stabilisers + the lock test

**[DERIVED — computed over the frozen catalog].** The only configuration
with a clean rank-1 `U(1)`-shadow stabiliser is `v = 0 ∈ ℤ[i]`, with
`Stab = ℤ[i]^× ≅ ℤ₄`. But there `Stab = G`: it is a **trivial fixed
point**, not a lock. D1's lock structure requires `G` strictly larger than
`Stab_G(v)` — some generators move `v` (the broken/massive sector) while
the stabiliser fixes it (the unbroken/massless sector). A configuration
fixed by the *whole* group breaks nothing.

**No single object in the catalog is a genuine rank-2 → rank-1 electroweak
lock.** The only genuine rank-2 shadow the catalog carries is the `SU(3)`
colour triality (the BCC triple-cosine eigenvalue,
[`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md))
— and that is the **wrong sector**: an electroweak lock must be a colour
*singlet* (D6 ii). The electroweak rank-2 structure `SU(2)_L × U(1)_Y` is
not present as a derived single object.

### Step 5 — D2–D4: two-state opposition, compensating charge, neutral readout

The catalog yields **two** genuine two-state oppositions (D2 — each is a
`τ`-orbit of an order-2 structural automorphism, neither state separately
fixed):

- **Candidate A** — the ternary pair `{+1,−1}`, swapped by the sign-flip
  `τ`. Its natural charge is `T₃ = ±1` (the ternary alphabet's own
  values).
- **Candidate B** — the pair `{i,−i} ⊂ ℤ[i]^×`, swapped by complex
  conjugation `c`. These are the grade-`(±1)` states of `ℤ₄`. Relative to
  the order-2 element `−1` carrying unit weight, `i` — a square root of
  `−1` — carries **half-unit weight**: candidate B has `T₃ = ±½`.

**[SELECTION] — the `±½` normalisation.** That candidate B carries `T₃ =
±½` is a genuine structural reading, not a fit: the half-integer weight is
intrinsic to `ℤ₄` because its generator `i` squares to the order-2 element.
The doublet normalisation is *free from the "4"* of `|ℤ[i]^×| = N_base`.
Identifying `{i,−i}` as **the** electroweak doublet is, however, a
selection — the catalog also offers candidate A.

**D3/D4.** The compensating charge is the hypercharge `Y`; D4 fixes it by
`Q⟨v⟩ = T₃ + Y = 0` on the vev component. This is the pre-reg §5
load-bearing fact: `Y` is **not** an independent quantity to derive — once
`T₃` on the vev is fixed, `Y` is forced.

### Step 6 — D6 and the §7 falsifier against `(1,2)₁/₂`

Charge arithmetic, vev on the lower (`T₃ = −t₃`) component:

| Candidate | `T₃` | `Y` (forced by D4) | `Q`-charges (lower, upper) | Falsifier F-c |
|---|---|---|---|---|
| **A** — ternary `{+1,−1}` | `±1` | `+1` | `(0, +2)` | **FIRES** — not `(neutral, unit)` |
| **B** — `ℤ[i]^×` `{i,−i}` | `±½` | `+½` | `(0, +1)` | quiet |

**[DERIVED].** Candidate A's continuum shadow is **provably not**
`(1,2)₁/₂`: its `Q`-pattern is `(0,+2)`, firing F-c. Candidate A is closed
as a sub-case.

**Candidate B fires no falsifier.** F-c is quiet (`Q`-pattern `(0,+1)`).
F-a is quiet — the `ℤ[i]^×`/conjugation structure is per-site and carries
no BCC triality, so candidate B is a colour singlet (D6 ii). F-e is quiet
— `Y = +½` is forced by D4, not inserted. F-b and F-d concern the
rank-2 assembly and are addressed in §4.

### Step 7 — Verdict

UNDERDETERMINED. The reasoning is §4.

---

## §4 — Why UNDERDETERMINED — not FOUND, not CLOSED-NEGATIVE

**Not FOUND.** Pre-reg §6 FOUND requires an explicit construction trace
exhibiting a triple `(G,S,v)` satisfying D1–D5, every step
`[THEOREM]`/`[DERIVED]` from the catalog (for the `[DERIVED]` object) or at
worst `[SELECTION]`. D1 requires the **lock**: a rank-2 `G` strictly
larger than a rank-1 `Stab_G(v)`. Exhibiting that lock requires two steps
the catalog does not force:

1. **The rank-2 assembly.** `SU(2)_L × U(1)_Y` must appear as a single
   rank-2 object. The catalog provides one clean rank-1 `U(1)`-shadow
   (`ℤ[i]^×`) and the conjugation-`ℤ₂` (the Weyl shadow of an `SU(2)`).
   Completing the conjugation-`ℤ₂` to a continuous `SU(2)_L`-shadow is
   permitted by the doctrine — SM groups *are* continuum shadows of finite
   closure (pre-reg §1) — but it is a shadow assertion, `[CONJECTURE]`-
   grade, not a catalog theorem. And no catalog theorem says the
   colour-singlet sector carries *exactly two* commuting rank-1 directions.
2. **The role-split.** Which catalog `U(1)`-shadow is `T₃`'s Cartan and
   which is `Y` is unforced. `ℤ[i]^×` can host the `SU(2)_L` doublet
   (candidate B) *or* the residual `U(1)` — the catalog does not decide.

Because D1's lock is *assembled by an unforced choice* rather than
*exhibited as a catalog object*, FOUND is not available — by the pre-reg's
own definitions.

**Not CLOSED-NEGATIVE.** Pre-reg §6 CLOSED-NEGATIVE requires one of: (i)
no object has a rank-1 stabiliser carrying a genuine two-state opposition;
(ii) every candidate requires a new postulate; (iii) the best candidate's
shadow is provably not `(1,2)₁/₂`. All three fail:

- (i) fails — candidate B's ingredients (a genuine `±½` opposition, a
  `U(1)`-shadow) are catalog facts, computed in §3.
- (ii) fails — candidate B uses only `ℤ[i]^×` and its conjugation
  automorphism, both §4-catalog. No new finite group, integer, or exponent
  is introduced (F-d quiet). The continuum completion is the doctrine's
  own move, not a new postulate.
- (iii) fails — no §7 falsifier provably fires against candidate B (§3
  step 6). Candidate A *is* closed by F-c, but A is not the best
  candidate; B is.

**The residue is exactly UNDERDETERMINED clause (a):** a rank-1-stabiliser
object can be assembled from catalog structures, but selecting/assembling
it requires an unforced choice. The audit records that choice precisely:
the rank-2 `SU(2)_L × U(1)_Y` shadow, and the `T₃`-vs-`Y` role-split within
the catalog's `U(1)`-shadows.

**Tag of the result.** The assembled candidate-B object is
`[SELECTION PRINCIPLE — open]`: the catalog contains its ingredients and B
is the natural reading, but uniqueness/forcedness is not argued. The
conditional shadow-match Q10c (B's continuum completion  `(1,2)₁/₂`) is
`[CONJECTURE]`: structural consistency is not derivation (GTCA F1/F10).

---

## §5 — Positive findings worth banking

UNDERDETERMINED is a mapped boundary, not an empty result. Three genuine
structural facts were established and survive independently of the verdict:

1. **The doublet `±½` normalisation is catalog-derivable.** `[SELECTION].`
   `ℤ[i]^× ≅ ℤ₄` carries a `{+½,−½}`-weight pair `{i,−i}` because its
   generator `i` is a square root of the order-2 element. The hardest-
   looking SM number in this sector is the *cheapest*: it falls out of the
   "4" of `N_base`, with no inserted half-integer.
2. **The hypercharge is bookkeeping, not a free input.** `[DERIVED,
   conditional on the `T₃` assignment].` Given candidate B's `T₃ = ±½` and
   a vev on the `T₃ = −½` component, D4 forces `Y = +½` — exactly the SM
   Higgs value — with nothing to tune. This confirms pre-reg §5's
   load-bearing fact on FTD's own structure.
3. **The electroweak sector is colour-blind by construction.** `[DERIVED].`
   The `ℤ[i]^×`/conjugation structure is per-site and carries no BCC
   triality; any lock built on it is automatically an `SU(3)`-shadow
   singlet (D6 ii). The colour/electroweak factorisation is not an extra
   assumption here — it is structural.

The single missing piece is therefore narrow and identified: **the
forced rank-2 electroweak shadow as one derived object.**

---

## §6 — The upgrade path

To move this result from UNDERDETERMINED to FOUND, FTD would need a
**catalog-level theorem** establishing:

> The colour-singlet sector of FTD's finite closure carries **exactly two**
> commuting rank-1 `U(1)`-shadows — no more, no fewer — one of which is the
> Cartan of an `SU(2)`-shadow whose Weyl group is the conjugation-`ℤ₂` of
> `ℤ[i]^×`, the other the residual `U(1)`.

That is a sharp, well-posed, finite question — the natural successor
("Q11"). It is *not* a search and *not* a fit: it asks whether a rank count
in the catalog's colour-singlet sector is forced to be 2. If it is, the
assembly in §4 stops being a choice and the candidate-B lock becomes a
`[DERIVED]`/`[SELECTION]` object, lifting Q10c to `[SELECTION PRINCIPLE]`.
If the colour-singlet rank is forced to some other value, a §7-style
falsifier fires and Q10 closes negative. Either way the boundary sharpens.

Until then, the electroweak sector remains, honestly, an effective/
parametric completion — its finite-closure skeleton is present in FTD, its
forced assembly is not.

---

## §7 — Epistemic discipline

- **No spine tag moved.** Per pre-reg §6/§8, this audit promotes and
  demotes no LEDGER claim. `x₊ = 1/α` and every other spine claim are
  untouched. Q10 is electroweak-structural.
- **The verdict is a middle outcome, not the favourable one.** FOUND was
  pre-blessed and available in principle; the audit did not reach for it.
  UNDERDETERMINED is what the locked criteria return (GTCA F9 — the audit
  resisted assembling a defensible FOUND reading).
- **Resemblance is not derivation.** Candidate B *resembles* the SM Higgs
  doublet closely — colour singlet, `T₃ = ±½`, `Y` forced, `Q`-pattern
  `(0,+1)`. The audit tags the resemblance `[CONJECTURE]` and the
  assembled object `[SELECTION PRINCIPLE — open]`, because the rank-2 lock
  is assembled, not derived. A correctly-applied tag does not resolve the
  question it labels (GTCA F10).
- **CLOSED-NEGATIVE was a live option throughout** and is recorded as
  *not* reached, with the three failing gates enumerated (§4).

---

## §8 — LEDGER and cross-references

- **LEDGER:** FTD-0190 — the pre-registration row; this audit is its
  execution. The row is updated to record the UNDERDETERMINED verdict
  (status only; no tag promotion).
- **Pre-registration:** [`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](PREREG_FINITE_NEUTRAL_LOCK_v1.md)
  (locked; unedited).
- **Pre-registration registry:** [`../10_eft_program/REF_PREREGISTER_MANIFEST.md`](../10_eft_program/REF_PREREGISTER_MANIFEST.md).
- **Verification script:** [`../../../scripts/proofs/audit_finite_neutral_lock.py`](../../../scripts/proofs/audit_finite_neutral_lock.py).
- **Catalog sources:** [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
  (`SU(3)` triality), [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](THEOREM_MOORE_LAYER_DECOMPOSITION.md)
  (Moore decomposition), [`../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md)
  (the 27-block `A_{1g}` multiplicity).

---

*Audit executed 2026-05-22 against the commit tagged
`preregister-finite-neutral-lock-v1`. Verdict: UNDERDETERMINED. The
pre-registration is satisfied; the open item (the verdict) is closed; the
upgrade path (§6) is a fresh, well-posed question.*
