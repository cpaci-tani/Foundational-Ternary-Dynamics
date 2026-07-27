# FTD Lean Formalizations

This is the Lean 4 workspace for FTD's formal artifacts.

- `FtdNoGo/`, `FtdNoGo.lean`, and `Standalone.lean` formalize the Commutativity Independence No-Go. This is the current citable machine-checked proof artifact.
- `FTD/`, `FTD.lean`, and `FTDAlpha.lean` contain the older FTD algebra / master-quadratic material from the former separate Lean proof tree. These files are legacy, axiom-bearing numerical/algebraic checks; they are not a formal alpha derivation.

Use `cd lean && lake build` for the default citable workspace (`FtdNoGo`). Build the legacy tree explicitly with `lake build FTD FTDAlpha` when auditing or replaying old alpha-era checks.

## FtdNoGo — Lean 4 formalization of the Commutativity Independence No-Go

> **VERIFICATION STATUS (2026-06-14, toolchain `leanprover/lean4:v4.30.0`):**
> - **`Standalone.lean` —  MACHINE-CHECKED.** `lean Standalone.lean` →
>   **exit 0, stderr 0 bytes**, the `#check`s print the real signatures
>   (`ocommutator A B = fun x => 0`, etc. — no `sorry`), and
>   **`#print axioms standalone_core` → `[propext, Quot.sound]`** (standard
>   Lean axioms only; no `sorryAx`, no
>   custom axioms). This Mathlib-free, Lean-core-only, `Int`-based file is the
>   **citable verified artifact**. Claim A/C is a *real* theorem (Int
>   multiplication genuinely commutes — proved via `Int.mul_comm` + `omega`),
>   not a hypothesis. *(History: the first draft erred by using Mathlib-only
>   typeclasses; rewritten core-only and re-verified.)*
> - **`FtdNoGo/` (Mathlib version) —  MACHINE-CHECKED.** `lake build` →
>   **`Build completed successfully`, exit 0**, all six
>   `FtdNoGo/*` modules built, no errors (after patching v4.30.0 issues:
>   catch-all `import Mathlib`; `Poisson.lean` `ℝ`/`pderiv_X`/`norm_num`;
>   trimmed two unused-`simp`-arg warnings in `Independence.lean`). This is the
>   canonical-Mathlib-API rendering, using standard `CommRing`/`Matrix`/
>   `MvPolynomial`.
>
> **Bottom line:** BOTH the Mathlib-free core (`Standalone.lean`, axiom-clean)
> and the canonical Mathlib development (`FtdNoGo/`, `lake build` green) are
> machine-checked. They prove the same algebraic core two ways.

> **RE-VALIDATED 2026-07-24** (independent audit, same toolchain). All targets
> re-elaborated from source with the `.olean`s deleted first, so this is not a
> cache no-op. Findings and fixes:
> - `FtdNoGo/` core footprint is **`[propext, Classical.choice, Quot.sound]`**,
>   not `[propext]` as an earlier note in `FTD/Axioms.lean` claimed. Standard,
>   sound, and now **pinned at build time** by `FtdNoGo/AxiomAudit.lean` — any
>   drift (custom axiom, `sorry`, `native_decide`) fails the build.
> - The `FtdNoGo/` **postulate encoding is not consumed by any proof.** Every
>   theorem is an instance of `commutator_zero_of_any_index`, which holds for
>   an arbitrary index type. `IsLocal` is never used as a hypothesis. See the
>   sharpened modeling-bridge note in `FtdNoGo/Postulates.lean`.
> - **Non-degeneracy recorded** (`observable_nontrivial`): the commutativity
>   result is not the vacuous "everything holds in the trivial ring" reading.
> - Legacy `FTD/`: **141 `native_decide` proofs converted to `decide`**,
>   eliminating 141 custom compiler-trust axioms; the tree is now axiom-free
>   or standard-axioms-only, and builds in ~5 s.
> - Two false prose claims in `FTD/` corrected and replaced with real theorems
>   (`Emergence.lean` non-uniqueness, `DimensionalUniqueness.lean` uniqueness
>   over all naturals). Neither moves any corpus tag.
>
> **IMPROVEMENTS 2026-07-24** (follow-on to the audit; all green, no tag moved):
> - **Spine formalised over ℝ.** The master-quadratic Vieta relations,
>   `sum = product`, and **harmonic-mean = 2**, plus the Γ-reflection
>   `Γ(1/4)Γ(3/4) = π√2`, the pi-free form of `G*`, and the triad identity, are
>   now genuine machine-checked ℝ theorems in `FTD/MasterQuadraticProof.lean`
>   and `FTD/GammaProof.lean` (via Mathlib). Previously these lived only as
>   `#eval` `Float` printouts (`MasterQuadratic.lean` / `GammaFoundation.lean`),
>   which are now explicitly relabelled "numeric illustration, NOT
>   machine-checked". The numeric values `x₊ = 137.036…`, `x₋ = 3.024…`,
>   `G* = 2.9587…` remain un-formalizable — Mathlib has no closed form for
>   `Γ(1/4)` — and that boundary is now stated in-file rather than hidden behind
>   a printed `PASS`.
> - **Legacy tree guarded too.** `FTD/AxiomAudit.lean` pins representative swept
>   theorems (via `#assert_axioms_ftd`), so a `native_decide` regression or a
>   `sorry` fails `lake build FTD`. Together with `FtdNoGo/AxiomAudit.lean`, the
>   two build targets — `lake build` (default) and `lake build FTD` —
>   machine-check both trees **including** their axiom-footprint pins.

> **Legacy boundary:** `FTD/`, `FTD.lean`, and `FTDAlpha.lean` are retained for
> provenance and optional replay. They contain explicit custom axioms and `#eval`
> numerical comparisons, including the physical `x₊ = 1/α` bridge as an axiom.
> They must not be cited as a Lean proof that alpha is derived.

This is the Lean 4 (Mathlib) formalization of the **algebraic core** of
[`docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md`](../docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md)
(FTD pre-registration of the QM+GR "commutativity wall" as a forward
theorem). It turns the prose Claims A / C, the F-a landmine, and the
independence model into machine-checkable statements.

## What is proven (the algebra) vs. what is *not* (the bridge)

This is the load-bearing honesty boundary, also stated in
`FtdNoGo/Postulates.lean`:

**PROVEN (theorems, once compiled):**
- **Claim A + C** — the observable algebra `A₅ := Config → ℝ` generated by
  the substrate fields is a **commutative ring**, so *every* commutator
  vanishes — including products, Moore-neighbourhood sums, and
  update-composites of the field generators (`observable_commutator_zero`).
- **F-a (the decisive landmine)** — on the *same* phase-space algebra a
  **nonzero Poisson/symplectic bracket coexists with a zero commutator**
  (`poisson_ne_commutator`): a nonzero symplectic structure is *not* quantum
  non-commutativity. The map taking `{·,·}` to a nonzero commutator
  (deformation quantization / ℏ) is therefore external.
- **Claim B + independence** — a non-commutative ring **exists** (the 2×2
  matrix algebra, `noncommutativity_is_external`), so non-commutativity is
  *consistent* but lives **outside** the commutative observable algebra: it
  requires an added measurement map `M` (the candidate 6th postulate).

**NOT PROVEN — the modeling bridge (a `[DEFINITION]`, not a theorem):**
That `Observable := Config → ℝ` (and the `Voxel / Fields / Update`
encoding in `Postulates.lean`) is the *correct* or *unique* formalization of
FTD's five postulates. The Lean development proves the algebra is commutative
**given this model**; the faithfulness of the model to the physics is a
modelling choice, tagged `[DEFINITION]` / `[SELECTION]` in the corpus, and is
exactly the pre-reg's §3 definitions and F-g discipline ("the six probes
corroborate; they are not the proof"). **A green `lake build` certifies the
mathematics, not the physical faithfulness of the encoding.**

**Sharper, and stated in Lean rather than prose (2026-07-24 audit).** The
encoding is not merely unproven-faithful — it is *never used*. The entire
mathematical content is `Pi.commRing`: real-valued functions on **any** type
commute. `ObservableAlgebra.lean` records this as
`commutator_zero_of_any_index`, together with
`observable_commutator_zero_is_generic_instance` proving (by `rfl`) that the
FTD-specific theorem is literally that generic theorem at `X := Config`.
Consequences:

- No theorem here distinguishes FTD's lattice from any other configuration
  space; swapping `moore` for the empty or total neighbourhood changes nothing.
- `IsLocal` is documentation — it is never a hypothesis anywhere.
- **Do not cite this development as machine-checking anything that "quantifies
  over the five postulates."** It quantifies over an abstract configuration
  space. (`SPEC_FTD_FRAMEWORK_V1.md` §2 said otherwise; corrected 2026-07-24.)

This is still the correct *shape* for an independence result — a commutative
carrier plus a consistent non-commutative witness, on a ring proven
non-trivial (`observable_nontrivial`) — but the load-bearing physics argument
is the prose case that `Config → ℝ` is the right carrier, which is what
`PREREG_COMMUTATIVITY_DERIVATION_v1` targets and what Lean does not supply.

## Map to the pre-registration

| Pre-reg item | Lean artifact | File |
|---|---|---|
| Claim A (A₅ commutative) | `observable_commutator_zero` | `ObservableAlgebra.lean` |
| Claim C (comm ⟹ [A,B]=0) | `commutator_eq_zero_of_comm` | `Commutator.lean` |
| Claim B (non-comm needs ≠0 commutator) | `noncomm_of_commutator_ne_zero` | `Commutator.lean` |
| F-a (Poisson ≠ commutator) | `poisson_ne_commutator` | `Poisson.lean` |
| Independence / consistency model | `noncommutativity_is_external` | `Independence.lean` |
| Postulate encoding (P1–P5) | `Voxel, Fields, Config, Update, IsLocal, moore` | `Postulates.lean` |
| Bundled core theorem | `commutativity_independence_core` | `Main.lean` |
| ** verified Mathlib-free mirror** (axiom-clean) | `standalone_core` + the 4 pillars | `Standalone.lean` |
| Scope guard — encoding is not load-bearing | `commutator_zero_of_any_index`, `observable_commutator_zero_is_generic_instance` | `ObservableAlgebra.lean` |
| Non-degeneracy — result is not vacuous | `observable_nontrivial` | `Postulates.lean` |
| Build-time axiom-footprint pin | `#assert_axioms` (13 pins) | `AxiomAudit.lean` |

The independence half is consistency-by-model: the matrix algebra witnesses
that adding a non-commutative measurement structure `M` to the (commutative)
substrate is consistent — i.e. non-commutativity is *independent* of the five
postulates, not derivable from and not forbidden by them. The pre-reg's
**strong-forbiddance** outcome is deliberately **not** formalized (it is false
— `M` is always consistently addable, which is exactly what the matrix model
shows).

## Build / verify

### Fast path — the Mathlib-free core ( verified this session, seconds)

```sh
cd lean
lean Standalone.lean      # exit 0, empty stderr, #check + #print axioms ⟹ checked
```

Needs only the toolchain — no Mathlib, no `lake`, no network. Verified in
authoring (2026-06-14, v4.30.0): exit 0, stderr 0 bytes, the `#check`s print
the real signatures, and `#print axioms standalone_core` →
`[propext, Quot.sound]` only (no `sorryAx`).

### Full path — the Mathlib rendering (verified this session, canonical API)

Requires [`elan`](https://github.com/leanprover/elan) and Mathlib cache access.

```sh
cd lean
elan toolchain install $(cat lean-toolchain)   # if not already present
lake exe cache get                              # download prebuilt Mathlib (large, ~minutes)
lake build                                      # machine-check FtdNoGo/
```

A successful default `lake build` with no `sorry` and no errors verifies `FtdNoGo/`.
(`grep -rn "sorry" FtdNoGo Standalone.lean` returns nothing.) This was run in
authoring (2026-06-14): cache fetched 8459 oleans and `lake build` printed
`Build completed successfully`. If you cannot fetch the cache,
the `Standalone.lean` fast path proves the same algebraic core with no
dependencies.

### Legacy replay path — alpha/master-quadratic material (not citable as a proof)

```sh
cd lean
lake build FTD FTDAlpha
```

This optional target replays the older algebra, integer identities, and numerical
checks. It is useful for provenance but carries explicit custom axioms
(`FTD/LFunction.lean`, `FTD/SelfDuality.lean`, `FTD/Axioms.lean`,
`FTD/Emergence.lean`) and therefore does not upgrade corpus tags. In particular,
the physical alpha identification remains `[STRONGLY MOTIVATED CONJECTURE]` /
axiom-class input in the current corpus.

**2026-07-24 audit changes to this tree** (all provenance-preserving; no tag moved):

- **`native_decide` → `decide` (141 sites).** Each `native_decide` emitted a
  per-theorem custom axiom (`…._native.native_decide.ax_1_1`) that moves trust
  from the kernel to the compiler. Every fact involved is small arithmetic that
  `decide` discharges kernel-only. The tree is now axiom-free or
  `[propext, Quot.sound]`-only, and builds in ~5 s.
- **`Emergence.lean`** — a comment claimed `N_base + N_c = b_3` holds "ONLY for
  n = 3". It is an identity for **every** `n ≥ 2` and selects nothing. Comment
  corrected; the general statement is now the theorem
  `Nbase_plus_n_eq_b3_forall` so the non-uniqueness is on the record.
- **`DimensionalUniqueness.lean`** — the header claimed a uniqueness proof the
  file did not contain (only `D ∈ {1..6}` was checked; `D ≥ 7` was asserted in
  a comment). Now genuinely proven for all naturals via `f_gt_16_of_ge_5`
  (`f D ≥ 2^D ≥ 32 > 16`), giving `f_eq_16_iff : f D = 16 ↔ D = 3`.
  **Arithmetic only** — D = 3 remains `[SELECTION — declared]` (FTD-0355).
- **Caveat that still stands:** the 17 `#eval` blocks print `PASS`/`FAIL` during
  the build but are *not* checked — a `FAIL` still builds green, and they run in
  64-bit `Float` with no error control. They are printouts, not verification.
  Do not cite `#eval` output as machine-checked.

## Notes (as built & verified, v4.30.0)

- All `FtdNoGo/*` modules use a catch-all `import Mathlib` (immune to the
  v4.30.0 module-path renames that broke the original per-module imports, e.g.
  `Algebra.BigOperators.Basic`, `Data.Matrix.Notation`).
- `Poisson.lean` proves `{q,p}=1` via `simp [poisson, pderiv_X, Pi.single_apply]`
  then `norm_num` (the original `pderiv_X_self`/`pderiv_X_of_ne` route is
  superseded; this is what compiled).
- `Independence.lean` evaluates the 2×2 commutator entry with
  `simp [commutator_def, E01, E10, Matrix.sub_apply]`; matrices are over `ℤ`.
- The `FtdNoGo/` development depends only on stable Mathlib (`Pi.commRing`,
  `MvPolynomial`, `Matrix`). No custom `axiom`s are introduced there;
  `Standalone.lean`'s axiom footprint is exactly `[propext, Quot.sound]`.
  The only non-theorem content in the no-go proof is the
  *definitional* model of the substrate, by design (see the bridge section).
