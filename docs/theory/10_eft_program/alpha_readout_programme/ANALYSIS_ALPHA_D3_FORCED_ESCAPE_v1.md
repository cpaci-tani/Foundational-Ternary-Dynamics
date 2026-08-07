# ANALYSIS — The D=3 FORCED-escape: the complex-structure branch is closed; the residual is W-CRIT-2 (FTD-0284)

**Status:** `[DERIVED — branch (A) closure]` + `[UNDERDETERMINED — branch (B)]`
= **FORCED-ESCAPE NARROWED.**
**Pre-registration:** [`PREREG_ALPHA_D3_FORCED_ESCAPE_v1.md`](../preregistrations/alpha_readout_programme/PREREG_ALPHA_D3_FORCED_ESCAPE_v1.md),
tag `preregister-alpha-d3-forced-escape-v1`; artifact SHA `b320e0eb…`.
**What this is:** a genuine, rigorous swing at the α obstruction — the single surviving exit
of MC-T4.3 (the RSI Leg-3 FORCED-escape). **It does not derive α** and does not promote
`x₊ = 1/α` (stays `[STRONGLY MOTIVATED CONJECTURE]`). It closes one branch of the escape with
a clean theorem and pins the entire remaining surface to one residual.

---

## 0 · Result in one line

The trace's `16 = |ℤ[i]^×|²` and the master quadratic's **real-distinct** roots are
**provably incompatible on the same 2D readout**: a complex structure is *elliptic*
(`Det ≥ Tr²/4`), the master quadratic is *hyperbolic* (`Det = 16G*³ < 64G*⁴ = Tr²/4`). So the
"complex-structure-on-the-readout" version of the FORCED-escape is **closed**. The only
surviving way to force `α` is a **real** operator whose readout determinant is constrained to
`16·(three-plane product)` *without* a module structure on the readout — which is exactly the
unresolved W-CRIT-2 question (plus the infinite-descended transfer-operator sub-branch).

## 1 · The two branches and why the dichotomy is exhaustive

The FORCED-escape needs a 2D readout `R` whose operator `A = M|R` has char poly
`x² − 16G*²x + 16G*³`. The factor `16 = |ℤ[i]^×|²` in the trace can enter `R` in exactly two
ways:

**Branch (A) — as a complex structure J (J² = −I) acting on R** (the V_complex = ℤ[i]² module
reading the corpus's wall assumed). The commutant of J in `M₂(ℝ)` is exactly
`{aI + bJ} ≅ ℂ`, with eigenvalues `a ± bi`. Imposing `Tr = 16G*²` gives `a = 8G*²`; then
`b² = Det − a² = 16G*³ − 64G*⁴ = −16G*³(4G* − 1) < 0` for every `G* > 1/4`. **`b` is not
real** — no real J-commuting operator on `R` has the master determinant. The only reproduction
is the non-real witness `M_w = 8G*²·I + i√(16G*³(4G*−1))·(K/…)`, which invokes the ambient
scalar `i` ⇒ a single C₄ axis ⇒ `⟨C₄, C₃⟩ = O` ⇒ unbroken O_h ⇒ no localized charge ⇒ no
readout (the machine-checked Legs 1–2 / 3b wall). **Branch (A): CLOSED-NEGATIVE.**

> This is a clean strengthening of the corpus's 3b-scope theorem. 3b ruled out *C₃-equivariant
> rank-2 restrictions* via Schur. The argument here is **general**: it closes **any**
> J-commuting 2D readout, by the one-line elliptic/hyperbolic incompatibility
> (`commutant(J) ≅ ℂ ⇒ conjugate-pair eigenvalues ⇒ never real-distinct`). The master quadratic
> is hyperbolic; complex structures are elliptic; they cannot meet on one real 2D readout.

**Branch (B) — as a bare integer coefficient** (16 counted, not acting; `R` a plain real 2D
space). A real `A` with `Tr = 16G*²`, `Det = 16G*³`, real-distinct, **exists** (companion
form) — reality does not close (B). **But** with no module structure on `R`, the determinant
`Det = ad − bc` is a **free** invariant (W-CRIT-2): nothing in the native generators forces it
to equal `16·e₃ = 16G*³` rather than `16·e₂·G* = 48G*³` or any other symmetric functional of
the three planes; the companion form's `−16G*³` entry is **hand-placed** (the banned move
F-HP). **Branch (B): UNDERDETERMINED** — it *is* the W-CRIT-2 question.

**Reducibility adds no third branch.** A direct sum `M = M_tr ⊕ M_det` has whole-operator
`Tr = Tr(M_tr) + Tr(M_det)` and `Det = Det(M_tr)·Det(M_det)` — not the invariants of one 2D
readout. Any single 2D readout still falls under (A)/(B). So the "reducible / 3-dimensional"
FORCED-escape, *for a 2D readout*, reduces to the dichotomy above.

## 2 · What is genuinely new vs. what was known

- **New `[DERIVED]`:** the elliptic/hyperbolic closure of branch (A) — a general, one-line
  reason the complex-structure escape is dead (`commutant(J) ≅ ℂ` is elliptic; the master
  quadratic is hyperbolic). This subsumes and generalizes the C₃-specific 3b-scope theorem.
- **Sharpened boundary:** the entire surviving FORCED-escape is now pinned to **branch (B)**
  — can the three-plane structure force a *real* operator's 2D-readout determinant to
  `16·(three-plane product)` without a module structure on the readout? — together with the
  one sub-branch this analysis does **not** cover: an **infinite-descended** transfer/monodromy
  operator whose finite truncations leave the generator set (the detdet_ζ identity, the
  `PREREG_READOUT_STRUCTURE_INDEPENDENCE` Leg-3 crux). These two are the complete remaining
  surface.

## 3 · Honest scope and limits

- **No α derivation.** `x₊ = 1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`.
- **MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`** — but its surviving surface is now provably
  just branch (B) + the infinite-descended sub-branch, not the whole complex-structure escape.
- **Scope of the closure:** branch (A) is closed for 2D real readouts of operators built from
  the frozen native generator set 𝒢 (§3 of the pre-reg). The closure is relative to that
  axiomatization (as is FTD-0244).
- **Adversarial gate (§7) not triggered:** no FORCED claim is made, so the gate (which would
  guard a positive result) did not fire. The branch-(A) closure is a *negative* result proved
  directly.

## 4 · The next honest move (queued, not done)

The residual is sharp and decidable: **(B)** does any FTD-native *real* operator-construction
constrain a 2D-readout determinant to `16·(g_xy·g_yz·g_zx)` — i.e., make the determinant a
forced functional of the three plane-sources rather than a free entry? Equivalently, is there
a real (not complex-structured) substrate object whose determinant *factorizes through the
three coordinate planes*? A v2 should target this directly, and separately the
infinite-descended transfer-operator sub-branch (the detdet_ζ identity). The prior remains
that both close — but they are now the *entire* surface, which is the value of this swing.

## 5 · Epistemic accounting

**Nothing promoted.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FC-class
commitments, and the algebraic spine are untouched. The deliverable is (i) a new
`[DERIVED]` elliptic/hyperbolic closure of the complex-structure branch of the FORCED-escape,
and (ii) the precise reduction of the entire remaining α-forcing question to the W-CRIT-2
real-operator residual + the infinite-descended sub-branch. This is the Number-One-Goal's
second clause at its sharpest: a real swing that mapped the boundary more precisely than it
was mapped before, without overclaiming a derivation. Next free LEDGER id: **FTD-0302**.
