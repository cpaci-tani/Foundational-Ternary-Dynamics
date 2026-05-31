/-
  Standalone.lean — Mathlib-FREE core of the FTD Commutativity Independence
  No-Go. Compiles with the BARE Lean 4 toolchain (no Mathlib, no `lake`):

      lean Standalone.lean        # exit 0, only #check / #print output ⟹ checked

  It uses ONLY Lean core (`Int`, `Int.mul_comm`, `omega`, `decide`, `funext`,
  `congrArg`) — no algebra typeclasses, which live in Mathlib. The observable
  carrier is `Config → Int` (Int is genuinely commutative, so Claim A/C is a
  REAL theorem, not a hypothesis); the non-commutativity witness is a 2×2
  integer matrix. The Mathlib development (FtdNoGo/) is the canonical-API
  rendering; if both compile they agree.

  Epistemic scope: this verifies the ALGEBRA. That `Config → Int` / `Config → ℝ`
  faithfully encodes FTD's five postulates is the [DEFINITION] modeling bridge,
  NOT proven here.

  (Restored 2026-05-30 after a concurrent-session placeholder overwrite; this
  is the full verified content + the §5 carrier/closure mirror of Closure.lean.)
-/

namespace FtdStandalone

/-! ## 1. Observables = `Config → Int`; Claim A + C as a real core theorem. -/

abbrev Config : Type := Nat                 -- stand-in index set for configs
abbrev Observable : Type := Config → Int    -- pointwise Int-valued functions

/-- Pointwise observable commutator `[A,B](c) = A c * B c − B c * A c`. -/
def ocommutator (A B : Observable) : Observable := fun c => A c * B c - B c * A c

/-- **Claim A + C.** Every observable commutator is the zero observable.
    A real theorem (Int multiplication commutes), no hypothesis. Covers the
    whole algebra: any `Config → Int` (generators, products, sums,
    update-composites) commutes pointwise. -/
theorem observable_commutator_zero (A B : Observable) :
    ocommutator A B = fun _ => 0 := by
  funext c
  show A c * B c - B c * A c = 0
  rw [Int.mul_comm (A c) (B c)]
  omega   -- goal `B c * A c - B c * A c = 0`; omega treats `B c * A c` as atomic

/-! ## 2. Claim B + Independence — a non-commutative structure EXISTS. -/

structure M2 where
  a : Int
  b : Int
  c : Int
  d : Int

def M2.mul (x y : M2) : M2 :=
  { a := x.a * y.a + x.b * y.c, b := x.a * y.b + x.b * y.d,
    c := x.c * y.a + x.d * y.c, d := x.c * y.b + x.d * y.d }

def M2.sub (x y : M2) : M2 :=
  { a := x.a - y.a, b := x.b - y.b, c := x.c - y.c, d := x.d - y.d }

def M2.commutator (x y : M2) : M2 := M2.sub (M2.mul x y) (M2.mul y x)

def E01 : M2 := { a := 0, b := 1, c := 0, d := 0 }
def E10 : M2 := { a := 0, b := 0, c := 1, d := 0 }
def M2.zero : M2 := { a := 0, b := 0, c := 0, d := 0 }

/-- The `.a` entry of the witnessing commutator `[E01,E10]` is `1`. -/
theorem matrix_commutator_a : (M2.commutator E01 E10).a = 1 := by decide

/-- **Claim B witness / independence model.** The matrix algebra is genuinely
    non-commutative: `[E01,E10] ≠ 0`. With `observable_commutator_zero`, this
    shows non-commutativity is realizable only OUTSIDE the commutative
    observable algebra — it needs an added measurement map `M` — and that such
    an `M` is consistent (this matrix is the model): the independence half. -/
theorem matrix_noncommutative : M2.commutator E01 E10 ≠ M2.zero := by
  intro h
  have h1 : (M2.commutator E01 E10).a = (M2.zero).a := congrArg M2.a h
  rw [matrix_commutator_a] at h1
  exact absurd h1 (by decide)

/-! ## 3. F-a — Poisson bracket ≠ commutator (the decisive landmine).

Phase-space linear forms `α·q + β·p` as coordinates `(α,β) : Int × Int`. The
canonical Poisson bracket is the determinant `α_f β_g − β_f α_g`. A nonzero
Poisson bracket (`{q,p}=1`) coexists with the commutative observable product
(commutator 0): a symplectic structure is NOT the quantum commutator, so
promoting `{·,·}` to a nonzero `[·,·]` (deformation quantization / ℏ) is an
external addition — the candidate 6th postulate. -/

abbrev Lin : Type := Int × Int   -- (coeff of q, coeff of p)

def qLin : Lin := (1, 0)
def pLin : Lin := (0, 1)

/-- Poisson bracket of two linear forms `= α_f β_g − β_f α_g`. -/
def poisson (f g : Lin) : Int := f.1 * g.2 - f.2 * g.1

/-- **F-a.** `{q,p} = 1 ≠ 0`: the Poisson bracket is nonzero while the
    observable commutator (above) vanishes. -/
theorem poisson_is_not_commutator : poisson qLin pLin ≠ 0 := by decide

/-! ## 4. The bundled standalone core. -/

/-- All three pillars (Mathlib-free analogue of `commutativity_independence_core`):
    (A/C) every observable commutator vanishes — a real `Int` theorem;
    (F-a) a nonzero Poisson bracket coexists with that;
    (B/independence) a genuinely non-commutative ring exists (matrix witness). -/
theorem standalone_core :
    (∀ A B : Observable, ocommutator A B = fun _ => 0)
  ∧ (poisson qLin pLin ≠ 0)
  ∧ (M2.commutator E01 E10 ≠ M2.zero) :=
  ⟨observable_commutator_zero, poisson_is_not_commutator, matrix_noncommutative⟩

/-- Structure-disambiguation check: these projections compile only if
    `standalone_core` is the intended 3-way conjunction (the `∀` binds the
    FIRST conjunct, not the whole bundle). -/
example : (∀ A B : Observable, ocommutator A B = fun _ => 0) := standalone_core.1
example : poisson qLin pLin ≠ 0 := standalone_core.2.1
example : M2.commutator E01 E10 ≠ M2.zero := standalone_core.2.2

/-! ## 5. Carrier + completeness mirror (the Closure.lean facts, Int-only).

These mirror `FtdNoGo/Closure.lean`: the carrier fact (commutativity survives
deterministic evolution `∘U`) and a generated-algebra closure fact, here over
the Mathlib-free `Config → Int` carrier. Honest scope: like the rest of this
file, they certify the ALGEBRA, not the [DEFINITION] bridge. They do NOT close
sub-claim 2 (that the pointwise product is forced by the dynamics). -/

/-- Composition with a deterministic update `U : Config → Config`. -/
def precompU (U : Config → Config) (A : Observable) : Observable := fun c => A (U c)

/-- **Carrier (sub-claim 1), Int-only.** Observables precomposed with the
    deterministic update still commute: time evolution does not escape the
    commutative carrier. -/
theorem observable_commutator_zero_under_update
    (U : Config → Config) (A B : Observable) :
    ocommutator (precompU U A) (precompU U B) = fun _ => 0 :=
  observable_commutator_zero _ _

/-- Iterated deterministic evolution (n ticks) still commutes. -/
theorem observable_commutator_zero_iterated
    (U : Config → Config) (n : Nat) (A B : Observable) :
    ocommutator (precompU (fun c => Nat.rec c (fun _ c' => U c') n) A)
                (precompU (fun c => Nat.rec c (fun _ c' => U c') n) B) = fun _ => 0 :=
  observable_commutator_zero _ _

/-- Bundled carrier core (Int-only mirror of `commutativity_certified_core`). -/
theorem standalone_certified_core :
    (∀ (U : Config → Config) (A B : Observable),
        ocommutator (precompU U A) (precompU U B) = fun _ => 0) :=
  observable_commutator_zero_under_update

#check @standalone_core
#check @standalone_certified_core
#check @observable_commutator_zero
#check @observable_commutator_zero_under_update
#check @matrix_noncommutative
#check @poisson_is_not_commutator

#print axioms standalone_core
#print axioms standalone_certified_core

end FtdStandalone
