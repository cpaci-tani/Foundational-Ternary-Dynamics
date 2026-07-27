/-
  FTD.MasterQuadraticProof — the master quadratic as MACHINE-CHECKED ℝ algebra
  ============================================================================
  Added 2026-07-24 (Lean-validation improvement, Workstream A). The sibling
  `MasterQuadratic.lean` only *prints* PASS/FAIL for these facts via `#eval` in
  64-bit `Float` — a `FAIL` still builds green. This file proves the
  value-INDEPENDENT content as genuine theorems over ℝ, using Mathlib. Unlike
  most of `FTD/`, this module imports Mathlib on purpose (the reals and the
  `discrim` API live there).

  What is proven here (for an ABSTRACT real `g`, so the theorems hold for the
  framework value `g = G*` and for every other `g`):
    - the discriminant factorisation `64 g³ (4g − 1)` and its positivity for
      g > 1/4 (two real roots) / vanishing at g = 1/4 (the unification point);
    - both roots satisfy the quadratic;
    - Vieta: sum = 16 g², product = 16 g³;
    - the normalised roots satisfy sum = product;
    - the harmonic mean of the normalised roots is EXACTLY 2 = [ℚ(i):ℚ].

  What is NOT and CANNOT be proven here: the numeric values x₊ = 137.036…,
  x₋ = 3.024…, floor(x₋) = 3. Those need G*'s decimal value, and Mathlib has no
  closed form for Γ(1/4); they remain `#eval` Float illustrations in
  `MasterQuadratic.lean`, honestly labelled as such. This file moves NO corpus
  tag: x₊ = 1/α stays [SMC]; the harmonic-mean = 2 fact is pure algebra.
-/
import Mathlib

namespace FTD.MasterQuadraticProof
open Real

/-- The master quadratic polynomial `x² − 16g²x + 16g³` as a function of the
    root variable, parametrised by an abstract real `g` (the framework instance
    is `g = G*`). -/
def masterQuad (g x : ℝ) : ℝ := x ^ 2 - 16 * g ^ 2 * x + 16 * g ^ 3

/-- The discriminant factors as `64 g³ (4g − 1)`. -/
theorem discrim_masterQuad (g : ℝ) :
    discrim 1 (-16 * g ^ 2) (16 * g ^ 3) = 64 * g ^ 3 * (4 * g - 1) := by
  unfold discrim; ring

/-- For `g > 1/4` the discriminant is strictly positive: two distinct real
    roots. (`g > 1/4` already gives `g > 0`, so no separate positivity
    hypothesis is needed.) -/
theorem discrim_pos {g : ℝ} (hg : 1 / 4 < g) :
    0 < discrim 1 (-16 * g ^ 2) (16 * g ^ 3) := by
  rw [discrim_masterQuad]
  have hg0 : 0 < g := by linarith
  have h1 : 0 < 4 * g - 1 := by linarith
  positivity

/-- At the unification point `g = 1/4` the discriminant vanishes: a double
    root (both normalised roots equal 2). -/
theorem discrim_zero_at_quarter :
    discrim 1 (-16 * (1 / 4 : ℝ) ^ 2) (16 * (1 / 4 : ℝ) ^ 3) = 0 := by
  rw [discrim_masterQuad]; norm_num

/-- The larger root `x₊ = 8g² + √(64g⁴ − 16g³)`. -/
noncomputable def xplus (g : ℝ) : ℝ := 8 * g ^ 2 + Real.sqrt (64 * g ^ 4 - 16 * g ^ 3)

/-- The smaller root `x₋ = 8g² − √(64g⁴ − 16g³)`. -/
noncomputable def xminus (g : ℝ) : ℝ := 8 * g ^ 2 - Real.sqrt (64 * g ^ 4 - 16 * g ^ 3)

variable {g : ℝ}

/-- The radicand `64g⁴ − 16g³ = 16g³(4g − 1)` is nonnegative for `g ≥ 1/4`. -/
theorem radicand_nonneg (hg : 1 / 4 ≤ g) : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3 := by
  have hg0 : 0 ≤ g := by linarith
  nlinarith [sq_nonneg g, sq_nonneg (g ^ 2)]

/-- `x₊` is a root. -/
theorem masterQuad_xplus (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    masterQuad g (xplus g) = 0 := by
  have hs : Real.sqrt (64 * g ^ 4 - 16 * g ^ 3) ^ 2 = 64 * g ^ 4 - 16 * g ^ 3 :=
    Real.sq_sqrt hd
  simp only [masterQuad, xplus]; nlinarith [hs]

/-- `x₋` is a root. -/
theorem masterQuad_xminus (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    masterQuad g (xminus g) = 0 := by
  have hs : Real.sqrt (64 * g ^ 4 - 16 * g ^ 3) ^ 2 = 64 * g ^ 4 - 16 * g ^ 3 :=
    Real.sq_sqrt hd
  simp only [masterQuad, xminus]; nlinarith [hs]

/-- **Vieta — sum of roots** `x₊ + x₋ = 16g²`. -/
theorem root_sum : xplus g + xminus g = 16 * g ^ 2 := by
  simp only [xplus, xminus]; ring

/-- **Vieta — product of roots** `x₊ · x₋ = 16g³`. -/
theorem root_prod (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    xplus g * xminus g = 16 * g ^ 3 := by
  have hs : Real.sqrt (64 * g ^ 4 - 16 * g ^ 3) * Real.sqrt (64 * g ^ 4 - 16 * g ^ 3)
      = 64 * g ^ 4 - 16 * g ^ 3 := Real.mul_self_sqrt hd
  simp only [xplus, xminus]; nlinarith [hs]

/-- Normalised larger root `u₊ = x₊ / G*`. -/
noncomputable def uplus (g : ℝ) : ℝ := xplus g / g

/-- Normalised smaller root `u₋ = x₋ / G*`. -/
noncomputable def uminus (g : ℝ) : ℝ := xminus g / g

/-- The normalised roots sum to `16g`. -/
theorem norm_sum (hg0 : 0 < g) : uplus g + uminus g = 16 * g := by
  have hgne : g ≠ 0 := ne_of_gt hg0
  simp only [uplus, uminus]; field_simp; nlinarith [root_sum (g := g)]

/-- The normalised roots multiply to `16g`. -/
theorem norm_prod (hg0 : 0 < g) (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    uplus g * uminus g = 16 * g := by
  have hgne : g ≠ 0 := ne_of_gt hg0
  simp only [uplus, uminus]; field_simp; nlinarith [root_prod (g := g) hd]

/-- **Normalised sum = product.** Both equal `16g`, so the normalised roots
    obey `u₊ + u₋ = u₊ · u₋` (the "sum = product" fact the `#eval` prints). -/
theorem norm_sum_eq_prod (hg0 : 0 < g) (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    uplus g + uminus g = uplus g * uminus g :=
  (norm_sum hg0).trans (norm_prod hg0 hd).symm

/-- **The harmonic mean of the normalised roots is exactly 2** ( = [ℚ(i):ℚ]).
    This is the actual content `MasterQuadratic.lean`'s `#eval` only printed. -/
theorem harmonic_mean (hg0 : 0 < g) (hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3) :
    2 * uplus g * uminus g / (uplus g + uminus g) = 2 := by
  have h16 : (16 * g) ≠ 0 := by positivity
  rw [mul_assoc, norm_prod hg0 hd, norm_sum hg0]
  field_simp

/-- Convenience: for the framework regime `g > 1/4`, both Vieta relations and
    the harmonic-mean identity hold together. -/
theorem master_quadratic_facts (hg : 1 / 4 < g) :
    xplus g + xminus g = 16 * g ^ 2
    ∧ xplus g * xminus g = 16 * g ^ 3
    ∧ 2 * uplus g * uminus g / (uplus g + uminus g) = 2 := by
  have hg0 : 0 < g := by linarith
  have hd : (0 : ℝ) ≤ 64 * g ^ 4 - 16 * g ^ 3 := radicand_nonneg (le_of_lt hg)
  exact ⟨root_sum, root_prod hd, harmonic_mean hg0 hd⟩

end FTD.MasterQuadraticProof
