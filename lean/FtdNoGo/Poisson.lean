/-
  Poisson.lean — the F-a landmine, machine-checked: a nonzero Poisson /
  symplectic bracket coexists with a zero commutator on the SAME commutative
  algebra. This formalizes the decisive distinction the no-go must make:
  the leapfrog/symplectic structure of the substrate is a nonzero Poisson
  bracket, but the *observable product* is commutative — so the symplectic
  structure is NOT quantum non-commutativity. Turning `{·,·}` into a nonzero
  commutator requires deformation quantization (an external ℏ / measurement
  map), which is exactly the candidate 6th postulate.
-/
-- Catch-all import: brings in `ℝ`, `MvPolynomial`, and `pderiv` regardless of
-- v4.30.0 module-path renames (the per-module import omitted `ℝ`).
import Mathlib
import FtdNoGo.Commutator

namespace FtdNoGo
open MvPolynomial

/-- Phase-space functions in one (q, p) pair: `MvPolynomial (Fin 2) ℝ`, with
    variable `0 = q` and `1 = p`. This is a *commutative* ring. -/
abbrev Phase : Type := MvPolynomial (Fin 2) ℝ

/-- The canonical symplectic Poisson bracket
    `{f, g} = ∂_q f · ∂_p g − ∂_p f · ∂_q g`. -/
noncomputable def poisson (f g : Phase) : Phase :=
  pderiv 0 f * pderiv 1 g - pderiv 1 f * pderiv 0 g

/-- `{q, p} = 1` — the Poisson bracket of the canonical pair is nonzero.

    Robust proof: `pderiv i (X j) = Pi.single i 1 j` (`pderiv_X`); `simp` with
    `Pi.single_apply` + `Fin` decidable-eq discharges the four derivatives, then
    `ring`. If a name differs in your Mathlib, `simp [poisson, pderiv_X]`
    followed by `decide`/`ring` is the fallback. -/
theorem poisson_q_p : poisson (X 0) (X 1) = (1 : Phase) := by
  simp only [poisson, pderiv_X, Pi.single_apply]
  norm_num

/-- The *associative* commutator of the same pair is zero (commutative ring). -/
theorem commutator_q_p : commutator (X 0 : Phase) (X 1) = 0 :=
  commutator_eq_zero_of_comm _ _

/-- **F-a.** On the same phase-space algebra the Poisson bracket is nonzero
    while the observable commutator vanishes: a nonzero symplectic/Poisson
    structure does not constitute quantum non-commutativity. -/
theorem poisson_ne_commutator :
    poisson (X 0 : Phase) (X 1) ≠ commutator (X 0 : Phase) (X 1) := by
  rw [poisson_q_p, commutator_q_p]
  exact one_ne_zero

/-- Packaged existential form of F-a for the bundled theorem: a commutative
    ring with a bracket that is nonzero while its commutator is zero. -/
theorem poisson_witness :
    ∃ (P : Type) (_ : CommRing P) (pb : P → P → P) (a b : P),
      pb a b ≠ 0 ∧ commutator a b = 0 :=
  ⟨Phase, inferInstance, poisson, X 0, X 1,
    by rw [poisson_q_p]; exact one_ne_zero, commutator_q_p⟩

end FtdNoGo
