/-
  Postulates.lean — a Lean encoding of FTD's five postulates and the
  observable algebra A₅ they generate.

  ── THE MODELING BRIDGE (read this) ──────────────────────────────────────
  The definitions below (`Voxel`, `Fields`, `Config`, `Update`, `Observable`)
  are a *model* of the five postulates. That this model is the CORRECT or
  UNIQUE formalization of the physics is **not** a theorem — it is a
  modelling choice, tagged [DEFINITION] / [SELECTION] in the FTD corpus
  (pre-reg §3). Everything proven downstream ("the observable algebra is
  commutative") holds *of this model*. A green `lake build` certifies the
  mathematics, not the physical faithfulness of the encoding.
  ─────────────────────────────────────────────────────────────────────────
-/
-- Catch-all import: immune to Mathlib module-path renames across versions
-- (v4.30.0 moved BigOperators.Basic / others). Cache makes this cheap.
import Mathlib
import FtdNoGo.Commutator

open scoped Classical BigOperators

namespace FtdNoGo

/-! ## Postulate 1 — Discrete space: the voxel lattice ℤ³. -/
abbrev Voxel : Type := ℤ × ℤ × ℤ

/-! ## Postulate 4 — Local causality: the Moore neighbourhood (Chebyshev ≤ 1). -/

/-- The 27 offsets `{-1,0,1}³`. -/
def offsets : Finset Voxel :=
  ({-1, 0, 1} : Finset ℤ) ×ˢ ({-1, 0, 1} : Finset ℤ) ×ˢ ({-1, 0, 1} : Finset ℤ)

/-- The Moore neighbourhood of a voxel: the 27 sites within Chebyshev
    distance 1 (Postulate 4). -/
def moore (v : Voxel) : Finset Voxel :=
  offsets.image (fun o => (v.1 + o.1, v.2.1 + o.2.1, v.2.2 + o.2.2))

/-! ## Postulate 3 — Ternary states (J primary; `s` as manifestation).

    Per-voxel substrate data: flux `J ∈ ℝ³` (dispositional, primary), the
    wave-velocity `vel` and latency `lat` (derived kinematic scalars). The
    ternary state `s ∈ {-1,0,+1}` is NOT independent data — it is the Genesis
    projection of `J` (see `state`), so it is omitted from the primitive
    record, honouring "J primary, s = action of J via the threshold rule". -/
structure Fields where
  J : Fin 3 → ℝ
  vel : ℝ
  lat : ℝ

/-- A configuration assigns fields to every voxel. -/
abbrev Config : Type := Voxel → Fields

/-- Manifestation threshold `K_B` (value irrelevant to the algebra). -/
def KB : ℝ := 1

/-- The ternary state field as the Genesis projection of `J`: a *function of
    the configuration*, not independent data (Postulate 3). -/
noncomputable def state (c : Config) (v : Voxel) : ℝ :=
  if (c v).vel > KB then (if (c v).J 0 ≥ 0 then 1 else -1) else 0

/-! ## Postulate 5 — Determinism: the one-tick update is a *function*. -/
abbrev Update : Type := Config → Config

/-- Postulate 4 as a property of an update: `U c v` depends only on `c`
    restricted to the Moore neighbourhood of `v`. (Documentation of locality;
    not load-bearing for the commutativity result.) -/
def IsLocal (U : Update) : Prop :=
  ∀ (c c' : Config) (v : Voxel),
    (∀ u ∈ moore v, c u = c' u) → U c v = U c' v

/-! ## Observables (beables): real functionals of the configuration.

    Postulate 5 (determinism) makes every substrate quantity a function of the
    configuration, i.e. a beable in 't Hooft's sense. -/
abbrev Observable : Type := Config → ℝ

/-- The observable algebra is commutative — supplied by `Pi.commRing`
    (pointwise operations on real-valued functions). This single instance is
    what makes every commutator vanish. -/
example : CommRing Observable := inferInstance

/-! ### Generators of A₅ (the substrate fields, Postulate 3). -/
def fluxObs (a : Fin 3) (v : Voxel) : Observable := fun c => (c v).J a
def velObs (v : Voxel) : Observable := fun c => (c v).vel
def latObs (v : Voxel) : Observable := fun c => (c v).lat
noncomputable def stateObs (v : Voxel) : Observable := fun c => state c v

/-! ### Closure operations the postulates license — all land back in
    `Observable`, the commutative ring, so commutativity is preserved. -/

/-- Composition with the deterministic update (Postulate 5): `A ∘ U`. -/
def precompU (U : Update) (A : Observable) : Observable := fun c => A (U c)

/-- A Moore-neighbourhood sum of observables (Postulate 4 locality): the only
    spatial coupling the postulates license. Still an observable. -/
noncomputable def mooreSum (v : Voxel) (g : Voxel → Observable) : Observable :=
  ∑ u ∈ moore v, g u

end FtdNoGo
