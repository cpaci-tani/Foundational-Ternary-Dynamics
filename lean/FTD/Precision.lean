/-
  FTD.Precision -- Epsilon Parameter and Precision Coefficients
  =============================================================
  Numerical verification of the precision formula:
    1/α ≈ x₊ - c₁ε + c₂ε² - c₃ε³ - c₄ε⁴
  where ε = e^π - π - 20 and all coefficients are rationals
  built from framework integers {3, 4, 7, 13, 47}.

  Reference: SPEC_FTD.md §4.5, MATH_MASTER_QUADRATIC.md
-/

import FTD.Constants

namespace FTD.Precision
open FTD

/-! ## Framework Integer Algebra for Precision Coefficients

  c₁ = 9/47        (3²/D_constraint)
  c₂ = 5/64        (5/N_base³)
  c₃ = 4/141       (N_base/(3·D_constraint))
  c₄ = 141/11      (3·D_constraint/11)

  All denominators and numerators come from {3, 4, 7, 13, 47}.
-/

/-- 9 = 3² -/
theorem nine_is_3sq : N_c * N_c = 9 := by native_decide

/-- 47 = D_constraint -/
theorem fortyseven : D_constraint = 47 := rfl

/-- 64 = 4³ = N_base³ -/
theorem sixtyfour_is_Nbase_cubed : N_base * N_base * N_base = 64 := by native_decide

/-- 141 = 3 × 47 = N_c × D_constraint -/
theorem onefouryone : N_c * D_constraint = 141 := by native_decide

/-- 11 = N_c + N_base + N_base = 3 + 4 + 4 -/
theorem eleven_from_framework : N_c + N_base + N_base = 11 := by native_decide

/-- Alternative: 11 = β₀ = 11*N_c/N_c -/
theorem eleven_is_beta0 : 11 * N_c / N_c = 11 := by native_decide

/-- c₁ numerator: 9 = N_c² -/
theorem c1_num : N_c * N_c = 9 := nine_is_3sq

/-- c₁ denominator: 47 = D_constraint -/
theorem c1_den : D_constraint = 47 := fortyseven

/-- c₂ numerator: 5 = N_c + CM_field_degree = 3 + 2 -/
theorem c2_num : N_c + CM_field_degree = 5 := by native_decide

/-- c₂ denominator: 64 = N_base³ -/
theorem c2_den : N_base * N_base * N_base = 64 := sixtyfour_is_Nbase_cubed

/-- c₃ numerator: 4 = N_base -/
theorem c3_num : N_base = 4 := rfl

/-- c₃ denominator: 141 = 3 × 47 = N_c × D_constraint -/
theorem c3_den : N_c * D_constraint = 141 := onefouryone

/-- c₄ numerator: 141 = N_c × D_constraint -/
theorem c4_num : N_c * D_constraint = 141 := onefouryone

/-- c₄ denominator: 11 = β₀ -/
theorem c4_den : N_c + N_base + N_base = 11 := eleven_from_framework

/-! ## Numerical Verification via Float -/

#eval do
  let g := Gstar_f
  let pi_val := gamma2_f * gamma2_f
  let e_pi := Float.exp pi_val
  let epsilon := e_pi - pi_val - 20.0
  let ae := Float.abs epsilon

  -- Master quadratic roots
  let B := 16.0 * g * g
  let C := 16.0 * g * g * g
  let disc := B * B - 4.0 * C
  let xPlus := (B + Float.sqrt disc) / 2.0

  -- Precision coefficients
  let c1 := 9.0 / 47.0
  let c2 := 5.0 / 64.0
  let c3 := 4.0 / 141.0
  let c4 := 141.0 / 11.0

  -- Series evaluation
  let term1 := c1 * ae
  let term2 := c2 * ae * ae
  let term3 := c3 * ae * ae * ae
  let term4 := c4 * ae * ae * ae * ae

  let prec_1 := xPlus - term1
  let prec_2 := prec_1 + term2
  let prec_3 := prec_2 - term3
  let prec_4 := prec_3 - term4

  let codata := 137.035999177

  IO.println "--- PRECISION FORMULA VERIFICATION ---"
  IO.println s!"  e^pi             = {e_pi}"
  IO.println s!"  pi               = {pi_val}"
  IO.println s!"  epsilon          = {epsilon}"
  IO.println s!"  |epsilon|        = {ae}"
  IO.println ""
  IO.println s!"  x+ (leading)     = {xPlus}"
  IO.println s!"  1-term (x+-c1e)  = {prec_1}"
  IO.println s!"  2-term           = {prec_2}"
  IO.println s!"  3-term           = {prec_3}"
  IO.println s!"  4-term           = {prec_4}"
  IO.println s!"  CODATA           = {codata}"
  IO.println ""
  let dev0 := Float.abs (xPlus - codata) / codata * 1e6
  let dev4 := Float.abs (prec_4 - codata) / codata
  IO.println s!"  Leading dev:  {dev0} ppm"
  IO.println s!"  4-term dev:   {dev4}"
  IO.println s!"  Leading < 2ppm? {if dev0 < 2.0 then "PASS" else "FAIL"}"

end FTD.Precision
