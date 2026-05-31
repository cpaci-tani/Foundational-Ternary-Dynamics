/-
  Commutator.lean — the additive commutator (the "quantum bracket") and its
  basic theory. This is the algebraic heart of the no-go: in a commutative
  ring every commutator vanishes, so non-commutativity is precisely a nonzero
  commutator, which a commutative algebra cannot host.

  Maps to pre-reg Claim C (`commutator_eq_zero_of_comm`) and Claim B
  (`noncomm_of_commutator_ne_zero`).
-/
import Mathlib.Algebra.Ring.Basic

namespace FtdNoGo

variable {R : Type*}

/-- The additive commutator `[a, b] = a*b - b*a`. In any associative ring this
    is the standard Lie/commutator bracket; it vanishes iff `a` and `b`
    commute. -/
def commutator [Mul R] [Sub R] (a b : R) : R := a * b - b * a

@[simp] theorem commutator_def [Mul R] [Sub R] (a b : R) :
    commutator a b = a * b - b * a := rfl

/-- **Claim C.** In a commutative ring every commutator vanishes. -/
theorem commutator_eq_zero_of_comm [CommRing R] (a b : R) :
    commutator a b = 0 := by
  simp only [commutator_def, mul_comm a b, sub_self]

/-- **Claim B (contrapositive form).** A nonzero commutator is exactly the
    witness of non-commutativity. -/
theorem noncomm_of_commutator_ne_zero [Ring R] {a b : R}
    (h : commutator a b ≠ 0) : a * b ≠ b * a := by
  intro hab
  apply h
  simp only [commutator_def, hab, sub_self]

end FtdNoGo
