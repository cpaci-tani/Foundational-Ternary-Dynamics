/-
  FTD/AxiomAudit.lean — build-time regression guard on the LEGACY tree.

  Added 2026-07-24 (Workstream C). Companion to `FtdNoGo/AxiomAudit.lean`; this
  one guards the `FTD/` legacy tree. Its purpose is to keep the
  2026-07-24 `native_decide → decide` sweep from silently regressing: every
  `native_decide` emits a per-theorem compiler-trust axiom
  (`…._native.native_decide.ax_1_1`), so if one creeps back in, the pins below
  fail the build instead of the drift living unnoticed.

  `#assert_axioms_ftd d = [a, b, …]` fails elaboration unless `d`'s transitive
  axiom dependencies are exactly `{a, b, …}` (order-insensitive; `= []` means
  axiom-free). A `sorry` surfaces as `sorryAx`, so this is also a no-`sorry`
  gate on the pinned theorems. The macro is named distinctly from
  `FtdNoGo/AxiomAudit.lean`'s `#assert_axioms` so the two can coexist if ever
  imported together.

  These proofs are pure `decide`/`omega`/`rfl` on ℕ, so they are either
  axiom-free or `[propext, Quot.sound]`-only (the latter when a `Fintype`
  `Decidable` instance is unfolded). No Mathlib, no custom axioms.
-/
import Lean
import FTD.Constants
import FTD.NumberTheory
import FTD.DimensionalUniqueness
import FTD.FineStructure
import FTD.GaussianIntegers
import FTD.Emergence

open Lean Elab Command in
/-- Fail the build unless `decl`'s axiom footprint is exactly the given set. -/
elab "#assert_axioms_ftd " decl:ident " = " "[" exp:ident,* "]" : command => do
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
      ++ m!"{Format.line}An unexpected axiom means a `sorry` (sorryAx) or a `native_decide`"
      ++ m!"{Format.line}(._native. …) compiler-trust axiom has re-entered the legacy tree."
      ++ m!"{Format.line}Convert it back to `decide`, or justify the pin change."

/-! ## Kernel-checked arithmetic — must stay axiom-free. -/

#assert_axioms_ftd FTD.coefficient_is_16 = []
#assert_axioms_ftd FTD.NumberTheory.fib_7 = []
#assert_axioms_ftd FTD.FineStructure.c1_den_47 = []
#assert_axioms_ftd FTD.GaussianIntegers.split_13_norm = []

/-! ## Fintype/`match`-based facts — `[propext, Quot.sound]` only. -/

#assert_axioms_ftd FTD.NumberTheory.three_is_moat = [propext, Quot.sound]
#assert_axioms_ftd FTD.DimensionalUniqueness.f_eq_16_iff = [propext, Quot.sound]
#assert_axioms_ftd FTD.DimensionalUniqueness.f_gt_16_of_ge_5 = [propext, Quot.sound]
#assert_axioms_ftd FTD.Emergence.Nbase_plus_n_eq_b3_forall = [propext, Quot.sound]
