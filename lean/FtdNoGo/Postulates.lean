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

  ── AND, SHARPER (audit finding, 2026-07-24) ─────────────────────────────
  The encoding is not merely unproven-faithful; it is **not consumed by any
  proof in this development**. `observable_commutator_zero` and every
  carrier/closure theorem downstream reduce to `Pi.commRing` on `Config → ℝ`,
  which holds for an ARBITRARY index type. `ObservableAlgebra.lean` now
  records this explicitly as `commutator_zero_of_any_index`, of which the
  FTD-specific statement is a direct instance. Consequences a reader must
  hold onto:

    * No theorem here distinguishes FTD's lattice from any other
      configuration space. Replacing `moore` with the empty or the total
      neighbourhood changes nothing.
    * `IsLocal` (below) is stated for documentation and is deliberately
      never used as a hypothesis anywhere.
    * Therefore this development must NOT be cited as machine-checking
      anything that "quantifies over the five postulates". It quantifies
      over an abstract configuration space. The physics content lives in the
      prose argument that `Config → ℝ` is the right carrier — which is
      exactly the [DEFINITION] bridge, and exactly what
      PREREG_COMMUTATIVITY_DERIVATION_v1 targets.

  What the development DOES establish is the right shape for an independence
  result: a commutative carrier (Claim A/C) plus a consistent
  non-commutative witness (Claim B), on a NON-DEGENERATE ring — see
  `observable_nontrivial` below, which rules out the vacuous reading in
  which commutators vanish only because the ring is trivial.
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

/-! ### Non-degeneracy: the observable ring is not the zero ring.

    Without this, "every commutator vanishes" would admit a vacuous reading:
    in the trivial ring every equation holds. `Config` is inhabited (a voxel
    field can be identically zero), so `Observable = Config → ℝ` inherits
    `ℝ`'s nontriviality and the commutativity result has real content. -/

instance : Inhabited Fields := ⟨{ J := fun _ => 0, vel := 0, lat := 0 }⟩
instance : Inhabited Config := ⟨fun _ => default⟩

/-- **The observable algebra is nontrivial** (`0 ≠ 1`), so the vanishing of
    every commutator is not an artifact of a degenerate carrier. -/
theorem observable_nontrivial : (0 : Observable) ≠ 1 := by
  intro h
  have := congrFun h (default : Config)
  simp at this

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
