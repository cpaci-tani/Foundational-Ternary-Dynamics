/-
  AxiomAudit.lean — build-time regression guard on the axiom footprint.

  Added 2026-07-24 after an audit found `FTD/Axioms.lean` asserting that the
  `FtdNoGo/` rendering's core theorems depend on `[propext]` only, when the
  measured footprint is `[propext, Classical.choice, Quot.sound]`. Prose
  cannot be trusted to track this; the build now checks it.

  `#assert_axioms d = [a, b, c]` fails the build unless `d`'s transitive
  axiom dependencies are exactly `{a, b, c}` (order-insensitive). A `sorry`
  anywhere upstream surfaces here as `sorryAx`, so this doubles as a
  no-`sorry` gate on the citable theorems.

  `propext`, `Classical.choice` and `Quot.sound` are Lean's three standard
  axioms — sound, and assumed by essentially all of Mathlib. The point of
  pinning them is not suspicion but drift-detection: any NEW name appearing
  in these lists is a custom axiom, a `sorry`, or a `native_decide`
  compiler-trust extension, and must be justified before it ships.
-/
import Lean
import FtdNoGo.Main
import FtdNoGo.FluxPrimary

open Lean Elab Command in
/-- Fail the build unless `decl`'s axiom footprint is exactly the given set. -/
elab "#assert_axioms " decl:ident " = " "[" exp:ident,* "]" : command => do
  let declName ← liftCoreM <| realizeGlobalConstNoOverload decl
  let actual := (← liftCoreM <| collectAxioms declName).toList
  let expected := exp.getElems.toList.map (·.getId)
  let missing := expected.filter (fun n => !actual.contains n)
  let extra := actual.filter (fun n => !expected.contains n)
  unless missing.isEmpty && extra.isEmpty do
    throwError m!"AXIOM FOOTPRINT DRIFT for {declName}"
      ++ m!"{Format.line}  expected   : {expected}"
      ++ m!"{Format.line}  actual     : {actual}"
      ++ m!"{Format.line}  missing    : {missing}"
      ++ m!"{Format.line}  UNEXPECTED : {extra}"
      ++ m!"{Format.line}An unexpected axiom means a custom `axiom`, a `sorry` (sorryAx), or a"
      ++ m!"{Format.line}`native_decide` (._native. ...) has entered the citable proof."
      ++ m!"{Format.line}Justify it or remove it — do not silently widen this list."

namespace FtdNoGo

/-! ## The bundled cores — the two theorems downstream documents cite. -/

#assert_axioms commutativity_independence_core = [propext, Classical.choice, Quot.sound]
#assert_axioms commutativity_certified_core    = [propext, Classical.choice, Quot.sound]

/-! ## The individual pillars. -/

#assert_axioms observable_commutator_zero   = [propext, Classical.choice, Quot.sound]
#assert_axioms poisson_ne_commutator        = [propext, Classical.choice, Quot.sound]
#assert_axioms poisson_witness              = [propext, Classical.choice, Quot.sound]
#assert_axioms noncommutativity_is_external = [propext, Classical.choice, Quot.sound]
#assert_axioms observable_nontrivial        = [propext, Classical.choice, Quot.sound]

/-! ## The ring-generic lemmas, which need strictly less.

    These two are stated over an abstract `CommRing`/`Ring` and so avoid
    `Classical.choice` entirely — they are the only `[propext]`-only
    declarations in the development. Anything mentioning `ℝ` picks up
    `Classical.choice` and `Quot.sound` from Mathlib's construction of the
    reals, which is why `commutator_zero_of_any_index` (below, over `ℝ`)
    carries the full standard triple despite being just as generic in its
    index type. -/

#assert_axioms commutator_eq_zero_of_comm     = [propext]
#assert_axioms noncomm_of_commutator_ne_zero  = [propext]
#assert_axioms commutator_zero_of_any_index   = [propext, Classical.choice, Quot.sound]

/-! ## The carrier / closure layer. -/

#assert_axioms observable_commutator_zero_under_update = [propext, Classical.choice, Quot.sound]
#assert_axioms observable_commutator_zero_mooreSum     = [propext, Classical.choice, Quot.sound]
#assert_axioms observable_closure_commutes             = [propext, Classical.choice, Quot.sound]

/-! ## The J-primary model (FluxPrimary.lean): Claim A′/B′ + the defect witness. -/

#assert_axioms fluxObs_factors       = [propext, Classical.choice, Quot.sound]
#assert_axioms stateFluxObs_factors  = [propext, Classical.choice, Quot.sound]
#assert_axioms stateObs_not_factors  = [propext, Classical.choice, Quot.sound]
#assert_axioms factors_add           = [propext, Classical.choice, Quot.sound]
#assert_axioms factors_mul           = [propext, Classical.choice, Quot.sound]
#assert_axioms factors_precompU      = [propext, Classical.choice, Quot.sound]
#assert_axioms factoring_commute     = [propext, Classical.choice, Quot.sound]
#assert_axioms pointwise_product_is_definitional = [propext, Classical.choice, Quot.sound]
#assert_axioms descends_nonvacuous   = [propext, Classical.choice, Quot.sound]

end FtdNoGo
