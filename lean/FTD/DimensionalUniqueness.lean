/-
  FTD.DimensionalUniqueness -- D = 3 from the Automorphism Group
  ===============================================================
  The equation |Aut(E_i)|^2 = 2^D * (D-1)! has unique natural-number
  solution D = 3 (`f_eq_16_iff`).

  This is a purely arithmetic result. Scope, added 2026-07-24 after an audit
  found the header claiming a uniqueness proof the file did not contain:
  the earlier version checked only D in {1,...,6} by evaluation and asserted
  the rest ("f is increasing for D >= 3") in a comment. The unbounded half is
  now PROVEN — `f_gt_16_of_ge_5` bounds f below by 2^D, so every D >= 5 is
  excluded outright, and the remaining D in {0,1,2,4} fall to `decide`.

  EPISTEMIC SCOPE. This is arithmetic uniqueness for the equation as posed.
  It does NOT show that this equation is the correct or forced constraint on
  spatial dimension; D = 3 remains [SELECTION -- declared] per FTD-0355 (see
  CLAUDE.md and SPEC_FTD_FRAMEWORK_V1.md 1.4/3.2). Nothing here promotes it.

  Reference: DERIV_D3_FROM_AUTOMORPHISM.md (April 2026)
-/

import FTD.Constants

namespace FTD.DimensionalUniqueness
open FTD

/-! ## The function f(D) = 2^D * (D-1)!

  We inline factorial since Nat.factorial requires Mathlib. -/

/-- Simple factorial for small naturals -/
def fact : Nat → Nat
  | 0 => 1
  | n + 1 => (n + 1) * fact n

/-- f(D) = 2^D * (D-1)! for the dimensional uniqueness equation -/
def f (D : Nat) : Nat := 2^D * fact (D - 1)

/-! ## Explicit evaluations -/

theorem f_1 : f 1 = 2 := by decide
theorem f_2 : f 2 = 4 := by decide
theorem f_3 : f 3 = 16 := by decide
theorem f_4 : f 4 = 96 := by decide
theorem f_5 : f 5 = 768 := by decide
theorem f_6 : f 6 = 7680 := by decide

/-! ## The target: |Aut(E_i)|^2 = 16 -/

/-- f(3) equals the squared automorphism order -/
theorem f_3_eq_aut_sq : f 3 = Aut_E_order * Aut_E_order := by decide

/-! ## Uniqueness: no other D in {1,...,6} gives 16 -/

theorem f_1_ne_16 : f 1 ≠ 16 := by decide
theorem f_2_ne_16 : f 2 ≠ 16 := by decide
theorem f_4_ne_16 : f 4 ≠ 16 := by decide
theorem f_5_ne_16 : f 5 ≠ 16 := by decide
theorem f_6_ne_16 : f 6 ≠ 16 := by decide

/-- f(4) > 16. -/
theorem f_4_exceeds : f 4 > 16 := by decide

/-! ## Uniqueness over ALL naturals (not just the {1,...,6} window).

    The finite checks above cannot by themselves exclude D >= 7. They are
    completed here by an unbounded lower bound: `fact` is positive, so
    `f D >= 2^D`, and `2^D >= 32 > 16` for every `D >= 5`. -/

/-- `fact n` is never zero. -/
theorem fact_pos : ∀ n : Nat, 0 < fact n
  | 0 => Nat.zero_lt_one
  | n + 1 => Nat.mul_pos (Nat.succ_pos n) (fact_pos n)

/-- `f` dominates `2^D`, since `fact (D-1) >= 1`. -/
theorem two_pow_le_f (D : Nat) : 2 ^ D ≤ f D :=
  Nat.le_mul_of_pos_right _ (fact_pos (D - 1))

/-- Every `D >= 5` overshoots the target: `f D >= 32 > 16`. This is the step
    the old "f is increasing" comment asserted without proof. -/
theorem f_gt_16_of_ge_5 {D : Nat} (h : 5 ≤ D) : 16 < f D := by
  have h32 : 32 ≤ 2 ^ D := by
    calc (32 : Nat) = 2 ^ 5 := by decide
      _ ≤ 2 ^ D := Nat.pow_le_pow_right (by decide) h
  have := two_pow_le_f D
  omega

/-- **Uniqueness [THEOREM].** `2^D * (D-1)! = 16` holds for exactly one
    natural number, `D = 3`. Arithmetic only — see the header's epistemic
    scope note; this does not make the physical dimension forced. -/
theorem f_eq_16_iff (D : Nat) : f D = 16 ↔ D = 3 := by
  constructor
  · intro hD
    match D with
    | 0 => exact absurd hD (by decide)
    | 1 => exact absurd hD (by decide)
    | 2 => exact absurd hD (by decide)
    | 3 => rfl
    | 4 => exact absurd hD (by decide)
    | (n + 5) =>
        have hgt := f_gt_16_of_ge_5 (D := n + 5) (by omega)
        omega
  · intro h; subst h; decide

/-- The same uniqueness stated against the framework's `|Aut(E)|^2`. -/
theorem f_eq_aut_sq_iff (D : Nat) : f D = Aut_E_order * Aut_E_order ↔ D = 3 :=
  f_eq_16_iff D

/-! ## Connection to framework -/

/-- 2^3 * 2! = |Aut(E_i)|^2 — the bridge between dimension and CM theory -/
theorem dim_aut_bridge : 2^3 * fact 2 = Aut_E_order * Aut_E_order := by decide

/-- The octahedral group connection: |O_h| = 3 * |Aut|^2 -/
theorem Oh_from_aut : 3 * (Aut_E_order * Aut_E_order) = 48 := by decide

/-! ## Verification -/

#eval do
  IO.println "═══════════════════════════════════════════════════════"
  IO.println "  D = 3 UNIQUENESS FROM |Aut(E_i)|^2 = 2^D * (D-1)!"
  IO.println "═══════════════════════════════════════════════════════"
  IO.println ""
  IO.println "  Target: |Aut(E_i)|^2 = 4^2 = 16"
  IO.println ""
  IO.println "  | D | 2^D * (D-1)! | = 16? |"
  IO.println "  |---|--------------|-------|"
  for D in [1, 2, 3, 4, 5, 6] do
    let val := 2^D * fact (D - 1)
    let mark := if val == 16 then "  YES ✓" else "  no"
    IO.println s!"  | {D} | {val} |{mark} |"
  IO.println ""
  IO.println "  D = 3 is the UNIQUE natural-number solution."
  IO.println "  Proven for ALL D by `f_eq_16_iff` (not just this window):"
  IO.println "    D >= 5 excluded by f D >= 2^D >= 32 > 16 (f_gt_16_of_ge_5);"
  IO.println "    D in {0,1,2,4} excluded by evaluation."
  IO.println "  Arithmetic only: D = 3 stays [SELECTION -- declared], FTD-0355."
  IO.println "═══════════════════════════════════════════════════════"

end FTD.DimensionalUniqueness
