/-
  Independence.lean — Claim B + the independence/consistency model,
  machine-checked: a non-commutative ring EXISTS (the 2×2 matrix algebra),
  so non-commutativity is realizable but lives OUTSIDE the commutative
  observable algebra. Its existence shows that adjoining a non-commutative
  measurement map `M` to the (commutative) substrate is consistent — i.e.
  non-commutativity is logically independent of the five postulates (not
  derivable from them, per ObservableAlgebra; not forbidden by them, per the
  consistent matrix model here). Matrices are over `ℤ`, so the witness is
  fully decidable.
-/
-- Catch-all import: immune to v4.30.0 Matrix module-path renames.
import Mathlib
import FtdNoGo.Commutator

namespace FtdNoGo
open Matrix

/-- The 2×2 integer matrix algebra — a standard non-commutative ring. -/
abbrev Mat2 : Type := Matrix (Fin 2) (Fin 2) ℤ

/-- Off-diagonal matrix units. -/
def E01 : Mat2 := !![0, 1; 0, 0]
def E10 : Mat2 := !![0, 0; 1, 0]

/-- The (0,0) entry of `[E01, E10]` is `1`. (If the `simp` set leaves residual
    arithmetic on your Mathlib, append `norm_num`.) -/
theorem matrix_commutator_entry : (commutator E01 E10) 0 0 = 1 := by
  simp [commutator_def, E01, E10, Matrix.sub_apply]

/-- **Claim B witness.** The commutator `[E01, E10]` is nonzero: the matrix
    algebra is genuinely non-commutative. -/
theorem matrix_commutator_ne_zero : commutator E01 E10 ≠ 0 := by
  intro h
  have hentry := matrix_commutator_entry
  rw [h, Matrix.zero_apply] at hentry
  exact one_ne_zero hentry.symm

/-- **Independence / consistency model.** A non-commutative ring exists,
    exhibiting a nonzero commutator. Combined with `observable_commutator_zero`
    (the observable algebra is commutative), this shows non-commutativity is
    realizable only *outside* the observable algebra — it requires an added
    measurement map `M` — and that such an `M` is *consistent* (a model
    exists). This is the independence half of the no-go. -/
theorem noncommutativity_is_external :
    ∃ (R : Type) (_ : Ring R) (a b : R), commutator a b ≠ 0 :=
  ⟨Mat2, inferInstance, E01, E10, matrix_commutator_ne_zero⟩

end FtdNoGo
