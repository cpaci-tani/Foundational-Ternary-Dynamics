# Lean formal substrate for the commutativity-derivation closure attempt — analysis note

**Status:** `[ANALYSIS — Lean substrate + honest reading; adversarially reviewed]`.
**This is not the prereg's §9 verdict.** It records what the machine-checked Lean
artifact (`lean/FtdNoGo/FluxPrimary.lean`) establishes toward
`PREREG_COMMUTATIVITY_DERIVATION_v1.md` (git tag
`preregister-commutativity-derivation-v1`), and the honest reading that follows.
An adversarial self-review (2026-07-24, Lean experiments; the dedicated
red-team subagent was unavailable) corrected two overclaims in an earlier draft
of this note — see §3/§4 — and added two machine-checked witnesses to the
artifact (`pointwise_product_is_definitional`, `descends_nonvacuous`). The formal
§9 verdict — with a further **independent (non-self) review** and a LEDGER row at
a freshly-grepped next-free ID — is deliberately deferred (the working tree is
shared by concurrent sessions and already carries IDs past FTD-0430; minting a
row here would risk collision).

**Date:** 2026-07-24.
**Artifact:** `lean/FtdNoGo/FluxPrimary.lean` (builds green under the default
`lake build`; every theorem `[propext, Classical.choice, Quot.sound]`, pinned in
`lean/FtdNoGo/AxiomAudit.lean`).
**Moves no tag:** D3(i) stays `[DEFINITION]`; x₊=1/α stays `[SMC]`; MC-T4.3
untouched; the Level-4 ceiling untouched.

---

## §1 — What prompted this

The Lean-validation audit (2026-07-24) found the `FtdNoGo/` no-go's postulate
encoding **not load-bearing**: every commutativity theorem is a `rfl`-instance
of `commutator_zero_of_any_index` (`Pi.commRing` on an arbitrary index type),
and the `Fields` struct in `Postulates.lean` makes `vel`/`lat` **independent
data**, contradicting P3's "J primary" and the derivation prereg's D7. This note
reports a faithful J-primary Lean model that discharges the prereg's Claim A′/B′
obligations and makes the honest verdict explicit.

## §2 — What the Lean substrate proves (Claims A′, B′)

Definitions: `Flux := Voxel → (Fin 3 → ℝ)`; `toFlux : Config → Flux` (project to
J); `FactorsThroughFlux A := ∃ f, A = f ∘ toFlux` (the prereg's D7).

- **Claim A′ (generators factor through J):** `fluxObs_factors`,
  `stateFluxObs_factors` — the flux components and the genesis-threshold state,
  read as a function of `|J|` vs `K_B`, are functions of J.
- **The defect, machine-checked (with a caveat):** `stateObs_not_factors` — the
  ORIGINAL `stateObs` (threshold on the independent `vel`) provably does **not**
  factor through J (two configs with identical J, different `vel`, disagree on
  `s`). ⚠ This is TRUE but partly a **single-tick artifact**: `vel` stands in for
  `v_wave = |Δ_t J|/K_B` (independence-prereg D1), a two-tick quantity that a
  single flux configuration cannot compute. Faithful J-primacy needs a
  flux-HISTORY primitive; the theorem correctly shows the single-tick model is
  not J-faithful, but the fix is a two-tick model, not merely redefining `s` on
  `|J|²` (which is different physics — `stateFlux` is a simplification).
- **Claim B′ (closure preserves factoring):** `factors_add`, `factors_mul`,
  `factors_neg`, and the load-bearing **`∘U` step** `factors_precompU`, which
  holds **precisely when the update descends to the flux** (`Descends U :=
  ∃ Ū, toFlux ∘ U = Ū ∘ toFlux`). The `∘U` obligation — the prereg's
  genuinely-new content — is discharged *conditional on descent*.

## §3 — The honest verdict: NOT FOUND (F-circular fires)

**Correction (adversarial review).** An earlier draft said "UNDERDETERMINED
because commutativity doesn't use factoring." That points the right way (not
FOUND, no tag moves) but names the wrong root cause. The decisive reason is
sharper, and is now machine-checked:

1. **The encoding POSITS D3(i) at the type level.** `Observable := Config → ℝ`
   carries Mathlib's `Pi.commRing`, whose product is pointwise:
   `(A*B) c = A c * B c` holds by `rfl` (`pointwise_product_is_definitional`).
   D3(i) — the pointwise product a derivation must NOT posit — is baked into the
   *type*. The prereg's **F-circular** falsifier (§7, declared **decisive**) is
   **FIRED** by the encoding. This substrate therefore cannot be a FOUND for the
   D3(i) derivation, full stop — consistent with the prereg §1's own observation
   that positing beables-are-functions is equivalent to the commutativity
   conclusion and "cannot be closed by more of the same proof."
2. **The factoring-is-not-used point is a *symptom*, not the cause.**
   `factoring_commute` ignores its factoring hypotheses because commutativity is
   automatic in a function algebra — which is exactly what point 1 says. Do not
   read it (as the earlier draft did) as the primary reason.
3. **`Descends` is a hypothesis, not a theorem** (`descends_nonvacuous` exhibits
   a non-descending update, so it is a genuine condition). Whether the actual
   FTD update descends is the CLOSED-NEGATIVE-live physics question; the model
   makes the dependency explicit.

So a genuine FOUND is unreachable in ANY encoding that types observables as
real-valued functions — the pointwise product comes for free with the type. The
deliverable here is not a derivation but a **sharper, J-faithful substrate**:
A′/B′ as real theorems, the ∘U step correctly conditioned on descent (a
correction to the prereg's D8, §4), and the machine-checked not-FOUND witnesses.

## §4 — Falsifier / banned-move checklist (prereg §7–§8)

- **F-circular** — **FIRED (corrected from an earlier "not fired").** The
  pointwise product IS posited: it is the definitional `Pi.commRing` product on
  `Observable := Config → ℝ` (`pointwise_product_is_definitional`, `rfl`). Since
  F-circular is the prereg's decisive falsifier, this alone bars a FOUND. The
  factoring theorems (A′/B′) are real, but they refine faithfulness *within* the
  posited product; they do not derive it.
- **D8 correction (finding against the prereg, not a falsifier).** The prereg's
  D8 asserts `A∘U` is a function of J "because `U : Ω→Ω` is a function". That is
  a non-sequitur: `A∘U` factors through J only if `U` **descends** to the flux.
  `descends_nonvacuous` exhibits a `U` that does not descend, so `factors_precompU`
  genuinely needs `Descends U`. The Lean model is stricter than D8 here.
- **F-a (Poisson ≠ commutator)** — not applicable here; no symplectic bracket is
  invoked. (Handled in `Poisson.lean`.)
- **F-0226-consistency** — the model generalizes FTD-0226's "function of J →
  classical" reading (the flux projection is the common Ω_J), does not
  contradict it.
- **F-vacuity** — the `∘U` step is discharged explicitly (`factors_precompU`),
  not skipped; its non-triviality is witnessed by `descends_nonvacuous` and by
  `stateObs_not_factors`. (This is distinct from the *broader* vacuity of §3: the
  factoring apparatus does not feed the commutativity conclusion — that is the
  F-circular problem, not F-vacuity.)
- **F-Mloc** — the M-localization half is **not** claimed here; this note scopes
  to the substrate factoring, not to exhibiting the genesis/Gauss mutation as the
  sole ordering-dependence locus. That remains open.
- **F-level4** — no claim about nature beneath measurement.
- **Banned moves B-1…B-8** — the pointwise product enters at the *type* level
  (`Pi.commRing`), which is the F-circular problem above, not an explicit
  premise in a proof step (B-1). Otherwise none invoked: no imported measurement
  basis / complex structure, no QM scaffold, no Poisson≡commutator, no numerical
  scan, no tag promotion.

## §5 — What remains for the full §9 verdict

- The **M-localization half (Q2)** is not addressed by this substrate.
- **Independent (non-self) review** of this NOT-FOUND reading, per the prereg's
  §9 step 8. This note's own review was a Lean-based adversarial self-review
  (the dedicated red-team subagent was unavailable); it should be confirmed by an
  outside reader before ratification.
- Only after those: a LEDGER row at a freshly-grepped next-free ID recording the
  NOT-FOUND / F-circular verdict, with the D3(i) `[DEFINITION]` **unchanged**.

## §6 — One-line summary

A J-faithful Lean model discharges the commutativity-derivation prereg's Claim
A′/B′ (with the `∘U` step correctly conditioned on flux-descent, fixing the
prereg's D8) and machine-checks the current model's non-faithfulness defect —
but the encoding types observables as functions, so it POSITS the pointwise
product (`rfl`) and the prereg's decisive F-circular falsifier fires. Honest
verdict: **NOT FOUND** — the substrate is sharper, D3(i) is not derived, no tag
moves.
