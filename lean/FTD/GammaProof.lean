/-
  FTD.GammaProof — the Γ-primitive constants as MACHINE-CHECKED ℝ identities
  =========================================================================
  Added 2026-07-24 (Lean-validation improvement, Workstream A). The sibling
  `GammaFoundation.lean` only *prints* these via `#eval` in `Float`. This file
  proves the value-INDEPENDENT identities as real theorems, from Mathlib's
  Gamma reflection formula.

  Proven:
    - reflection at 1/4: `Γ(1/4)·Γ(3/4) = π√2`  (Real.Gamma_mul_Gamma_one_sub);
    - `G* := Γ(1/4)/Γ(3/4)` is positive, and equals the pi-free form
      `Γ(1/4)²/(π√2)`;
    - the triad identity `π = 4ϖ²/G*²` with `ϖ := G*·√π/2`.

  NOT provable here: the decimal `G* ≈ 2.9587…`. Mathlib has no closed form for
  `Γ(1/4)`, so the numeric value stays a `#eval` Float illustration. Moves no
  corpus tag: G* = Γ(1/4)/Γ(3/4) is a definition; the identities are classical.
-/
import Mathlib

namespace FTD.GammaProof
open Real

/-- **Reflection at 1/4:** `Γ(1/4)·Γ(3/4) = π√2`. From Euler's reflection
    formula `Γ(s)Γ(1−s) = π/sin(πs)` at `s = 1/4` with `sin(π/4) = √2/2`. -/
theorem gamma_reflection_quarter :
    Real.Gamma (1 / 4) * Real.Gamma (3 / 4) = π * Real.sqrt 2 := by
  have h := Real.Gamma_mul_Gamma_one_sub (1 / 4 : ℝ)
  rw [show (1 : ℝ) - 1 / 4 = 3 / 4 by norm_num] at h
  rw [show (π * (1 / 4)) = π / 4 by ring, Real.sin_pi_div_four] at h
  rw [h]
  have h2 : Real.sqrt 2 * Real.sqrt 2 = 2 := Real.mul_self_sqrt (by norm_num)
  have hs2 : (0 : ℝ) < Real.sqrt 2 := by positivity
  field_simp
  nlinarith [h2]

/-- `G* = Γ(1/4)/Γ(3/4)` (the Γ-primitive form of the lemniscatic constant). -/
noncomputable def Gstar : ℝ := Real.Gamma (1 / 4) / Real.Gamma (3 / 4)

theorem gamma_quarter_pos : 0 < Real.Gamma (1 / 4) := Real.Gamma_pos_of_pos (by norm_num)
theorem gamma_three_quarter_pos : 0 < Real.Gamma (3 / 4) := Real.Gamma_pos_of_pos (by norm_num)

/-- `G* > 0`. -/
theorem gstar_pos : 0 < Gstar := by
  unfold Gstar; positivity

/-- **Pi-free form** `G* = Γ(1/4)² / (π√2)` (from reflection). -/
theorem gstar_eq_pi_free :
    Gstar = Real.Gamma (1 / 4) ^ 2 / (π * Real.sqrt 2) := by
  have hrefl := gamma_reflection_quarter
  have h34 : Real.Gamma (3 / 4) ≠ 0 := ne_of_gt gamma_three_quarter_pos
  have hden : π * Real.sqrt 2 ≠ 0 := by positivity
  unfold Gstar
  rw [div_eq_div_iff h34 hden]
  linear_combination -Real.Gamma (1 / 4) * hrefl

/-- The lemniscate constant `ϖ`, in its Γ-primitive relation to `G*`. -/
noncomputable def varpi : ℝ := Gstar * Real.sqrt π / 2

/-- **Triad identity** `π = 4ϖ²/G*²`. -/
theorem triad : π = 4 * varpi ^ 2 / Gstar ^ 2 := by
  have hg : Gstar ≠ 0 := ne_of_gt gstar_pos
  have hpi : Real.sqrt π ^ 2 = π := Real.sq_sqrt Real.pi_pos.le
  unfold varpi
  field_simp
  nlinarith [hpi]

end FTD.GammaProof
