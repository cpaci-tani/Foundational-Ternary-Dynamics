/-
  FluxPrimary.lean — the J-primary model: the formal substrate for the
  hash-locked PREREG_COMMUTATIVITY_DERIVATION_v1 closure attempt.

  Added 2026-07-24 (Workstream B). The audit of this session found that the
  `FtdNoGo/` no-go's postulate encoding is NOT load-bearing: every commutativity
  theorem is an instance of `commutator_zero_of_any_index` (`Pi.commRing` on an
  arbitrary index type), and — worse — the `Fields` struct in `Postulates.lean`
  makes `vel`/`lat` INDEPENDENT data, contradicting P3's "J primary". This file
  builds the faithful J-primary model the derivation prereg asks for, and
  discharges its Claim A′ / B′ obligations as real theorems.

  ── WHAT IS PROVEN (real, non-generic theorems) ─────────────────────────────
  * Claim A′ — the flux generators and the genesis-threshold state (read as a
    function of |J| vs K_B) FACTOR THROUGH THE FLUX (`FactorsThroughFlux`):
    `fluxObs_factors`, `stateFluxObs_factors`.
  * The DEFECT, made concrete — the ORIGINAL `stateObs` (threshold on the
    independent `vel`) provably does NOT factor through the flux
    (`stateObs_not_factors`): two configs with identical J but different vel
    disagree on s. This is why the current model is not J-faithful.
  * Claim B′ — the closure operations preserve factoring: `factors_add`,
    `factors_mul`, `factors_neg`, and the load-bearing `∘U` step
    `factors_precompU`, which holds PRECISELY WHEN the update descends to the
    flux (`Descends`). This is the prereg's genuinely-new obligation, discharged
    conditional on descent.

  ── THE HONEST VERDICT: NOT FOUND (adversarially reviewed 2026-07-24) ────────
  The decisive reason this substrate cannot be a FOUND for the derivation is
  that it POSITS D3(i). In `Observable := Config → ℝ` the product is Mathlib's
  `Pi.commRing` pointwise product, so `(A*B) c = A c * B c` holds by `rfl`
  (`pointwise_product_is_definitional` below). The prereg's F-circular falsifier
  — "the proof must NOT posit D3(i) (pointwise product) anywhere", declared
  DECISIVE — is therefore FIRED by the encoding itself. A genuine derivation
  needs a model that does NOT type observables as functions (deriving that
  beables ARE functions with a pointwise product), which the prereg §1 already
  says "more of the same proof" cannot do. Corollary: `factoring_commute`'s
  proof ignores its factoring hypotheses — factoring buys FAITHFULNESS, not
  commutativity — but that is a symptom of the posited product, not the root.

  `Descends` is a HYPOTHESIS, not a theorem (`descends_nonvacuous` exhibits a
  non-descending update, so it is a real condition). Whether the actual FTD
  update descends to the flux is the physics question the prereg flags as
  CLOSED-NEGATIVE-live; this file makes the dependency explicit. Note this also
  corrects the prereg's D8, which claims `A∘U` factors "because U is a function"
  — a non-sequitur: descent to the flux is required, and can fail.

  ⚠ SINGLE-TICK CAVEAT. `stateObs_not_factors` is TRUE but partly an artifact of
  single-tick modeling: `vel` stands in for `v_wave = |Δ_t J|/K_B` (D1), a
  TWO-tick quantity that a single flux cannot compute. Faithful J-primacy needs
  a flux-HISTORY primitive; `stateFlux` (threshold on single-tick `|J|²`) is a
  simplification with different physics, not the genesis threshold itself.

  So the deliverable is: A′/B′ as real theorems + the ∘U step correctly
  conditioned on descent (a fix to the prereg's D8) + the machine-checked
  not-FOUND witnesses. Verdict toward the prereg: this encoding cannot derive
  D3(i) (F-circular fires); it sharpens faithfulness within the assumption. It
  moves NO corpus tag: D3(i) stays `[DEFINITION]`, x₊=1/α stays `[SMC]`, the
  Level-4 ceiling is untouched. Full §9 verdict + independent review in
  `docs/theory/10_eft_program/AUDIT_COMMUTATIVITY_DERIVATION_LEAN_SUBSTRATE.md`.
-/
import Mathlib
import FtdNoGo.Postulates
import FtdNoGo.ObservableAlgebra

namespace FtdNoGo
open scoped BigOperators

/-- Flux configuration: the flux `J` at every voxel — the primitive datum P3
    calls primary. -/
abbrev Flux : Type := Voxel → (Fin 3 → ℝ)

/-- Projection forgetting the non-primary fields `vel`, `lat`. -/
def toFlux (c : Config) : Flux := fun v => (c v).J

/-- A beable *factors through the flux* iff it depends on the configuration only
    via `J` (the prereg's D7 "function-of-J" property). -/
def FactorsThroughFlux (A : Observable) : Prop :=
  ∃ f : Flux → ℝ, A = fun c => f (toFlux c)

/-! ## Claim A′ — the generators factor through the flux. -/

/-- Each flux component reads only `J`. -/
theorem fluxObs_factors (a : Fin 3) (v : Voxel) : FactorsThroughFlux (fluxObs a v) :=
  ⟨fun F => F v a, rfl⟩

/-- The genesis threshold as a function of the FLUX (magnitude vs `K_B`) — the
    faithful "J primary" reading of `s`. -/
noncomputable def stateFlux (F : Flux) (v : Voxel) : ℝ :=
  if (∑ a, (F v a) ^ 2) > KB ^ 2 then (if F v 0 ≥ 0 then 1 else -1) else 0

/-- The J-faithful state observable. -/
noncomputable def stateFluxObs (v : Voxel) : Observable := fun c => stateFlux (toFlux c) v

theorem stateFluxObs_factors (v : Voxel) : FactorsThroughFlux (stateFluxObs v) :=
  ⟨fun F => stateFlux F v, rfl⟩

/-- **The defect, made concrete.** The ORIGINAL `stateObs` (`Postulates.lean`),
    whose threshold reads `vel` — independent data — does NOT factor through the
    flux: two configs with identical `J` but different `vel` disagree on `s`.
    This is a real (non-vacuous) negative theorem, and the reason the current
    model is not J-faithful. -/
theorem stateObs_not_factors : ¬ FactorsThroughFlux (stateObs (0 : Voxel)) := by
  rintro ⟨f, hf⟩
  set c0 : Config := fun _ => { J := fun _ => 0, vel := 0, lat := 0 }
  set c1 : Config := fun _ => { J := fun _ => 0, vel := 2 * KB, lat := 0 }
  have hflux : toFlux c0 = toFlux c1 := rfl
  have h0 : stateObs (0 : Voxel) c0 = 0 := by
    simp only [stateObs, state, c0]; norm_num [KB]
  have h1 : stateObs (0 : Voxel) c1 = 1 := by
    simp only [stateObs, state, c1]; norm_num [KB]
  simp only [hf] at h0 h1
  rw [show toFlux c1 = toFlux c0 from hflux.symm] at h1
  rw [h0] at h1
  norm_num at h1

/-! ## Claim B′ — the closure operations preserve factoring. -/

theorem factors_add {A B : Observable}
    (hA : FactorsThroughFlux A) (hB : FactorsThroughFlux B) :
    FactorsThroughFlux (A + B) := by
  obtain ⟨f, rfl⟩ := hA; obtain ⟨g, rfl⟩ := hB
  exact ⟨fun F => f F + g F, rfl⟩

theorem factors_mul {A B : Observable}
    (hA : FactorsThroughFlux A) (hB : FactorsThroughFlux B) :
    FactorsThroughFlux (A * B) := by
  obtain ⟨f, rfl⟩ := hA; obtain ⟨g, rfl⟩ := hB
  exact ⟨fun F => f F * g F, rfl⟩

theorem factors_neg {A : Observable} (hA : FactorsThroughFlux A) :
    FactorsThroughFlux (-A) := by
  obtain ⟨f, rfl⟩ := hA
  exact ⟨fun F => -f F, rfl⟩

/-- The update **descends to the flux**: its effect on `J` depends only on `J`.
    This is exactly the physics input the prereg's `∘U` step needs; whether the
    real dynamics satisfy it is the open (CLOSED-NEGATIVE-live) question, made
    explicit here as a hypothesis rather than assumed. -/
def Descends (U : Update) : Prop :=
  ∃ Ubar : Flux → Flux, ∀ c, toFlux (U c) = Ubar (toFlux c)

/-- **The `∘U` step — the genuinely-new obligation.** If `U` descends to the
    flux, composing a flux-factoring beable with `U` still factors. Discharged
    conditional on `Descends U`; the descent itself is the modeling bridge. -/
theorem factors_precompU {U : Update} (hU : Descends U)
    {A : Observable} (hA : FactorsThroughFlux A) :
    FactorsThroughFlux (precompU U A) := by
  obtain ⟨Ubar, hUbar⟩ := hU
  obtain ⟨f, rfl⟩ := hA
  refine ⟨fun F => f (Ubar F), ?_⟩
  funext c
  simp only [precompU]
  rw [hUbar c]

/-! ## Claim C′ — commutativity (the honest form). -/

/-- Flux-factoring beables commute — but the proof does NOT use the factoring
    hypotheses. Real-valued beables commute unconditionally (`Pi.commRing`), so
    factoring buys FAITHFULNESS, not commutativity. A symptom of the NOT-FOUND
    verdict, whose root cause is `pointwise_product_is_definitional` below: the
    encoding posits the product, so commutativity cannot depend on factoring. -/
theorem factoring_commute {A B : Observable}
    (_hA : FactorsThroughFlux A) (_hB : FactorsThroughFlux B) :
    commutator A B = 0 :=
  observable_commutator_zero A B

/-! ## Honest limits — machine-checked (added after adversarial self-review).

These two witnesses pin down WHY this substrate cannot be a FOUND for the
derivation prereg, and are checked by the kernel rather than asserted in prose. -/

/-- **The decisive limit (prereg falsifier F-circular FIRES).** The pointwise
    product — the prereg's D3(i), the very thing a derivation must NOT posit — is
    DEFINITIONAL in `Observable := Config → ℝ` (`Pi.commRing`): `(A*B) c` reduces
    to `A c * B c` by `rfl`. So this encoding *assumes* D3(i) at the type level;
    it cannot derive it. A genuine derivation would need an encoding that does
    NOT type observables as functions (deriving that beables ARE functions with
    a pointwise product), which is exactly what the prereg §1 says "more of the
    same proof" cannot do. This is the real reason the verdict is not FOUND. -/
theorem pointwise_product_is_definitional (A B : Observable) (c : Config) :
    (A * B) c = A c * B c := rfl

/-- **`Descends` is non-vacuous**, so `factors_precompU`'s hypothesis is a real
    condition — and the prereg's D8 (which claims `A∘U` factors "because U is a
    function") is a non-sequitur: descent to the flux is required and can fail.
    Witness: an update whose new `J` reads the non-flux field `vel` does not
    descend. -/
theorem descends_nonvacuous :
    ∃ U : Update, ¬ Descends U := by
  refine ⟨fun c _ => { J := fun _ => (c 0).vel, vel := 0, lat := 0 }, ?_⟩
  rintro ⟨Ubar, h⟩
  set c0 : Config := fun _ => { J := fun _ => 0, vel := 0, lat := 0 } with hc0
  set c1 : Config := fun _ => { J := fun _ => 0, vel := 1, lat := 0 } with hc1
  have hf : toFlux c0 = toFlux c1 := rfl
  -- same flux ⇒ same Ubar image; but the update's flux reads `vel`, which differs.
  have e0 := h c0
  have e1 := h c1
  rw [hf] at e0
  have key := e0.trans e1.symm
  have contra := congrFun (congrFun key 0) 0
  simp only [toFlux, hc1] at contra
  norm_num at contra

end FtdNoGo
