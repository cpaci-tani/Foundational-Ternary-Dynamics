# ANALYSIS — Maxwell-criterion screen for a native `n = 4` mechanism (C3)

**Status:** `[CLOSED NEGATIVE — SCOPED]` + `[ENGINE FACT — MEASURED]` +
`[CORRECTION — THE SC SHEAR QUARTIC IS AFFINE, NOT RELAXED]`
**Verdict:** `NO_NATIVE_N4_IN_THE_SCREENED_SET`
**Protocol:** `../preregistrations/PREREG_MAXWELL_C3_SCREEN_v1.md`, locked at
commit `38292bf1` **before the runner existed**. Executed 2026-08-04.
**Runners:** `scripts/experiments/maxwell_c3_screen.py`,
`scripts/experiments/verify_sc_shear_quartic.py`
**Production impact:** none.

## 1 · Outcome

`NO_NATIVE_N4`. No configuration in the screened set satisfies FTD-0789's
criterion. Every candidate with a nontrivial null space classifies `n = ∞`.

**The registered expectation was wrong in the optimistic direction.** The
prereg registered `N4_SEMIDEFINITE` as the expected Tier-C outcome (quartic on
some null directions, flat on others). The measured result is *fully* flat —
plain `n = ∞`. Recorded because the prereg required it.

## 2 · Controls (Tier A) — and the bug they caught

| control | result | expected | |
|---|---|---|---|
| collinear trimer `(+1,−1,+1)` | `n = ∞`, `3N−B = 9−2 = 7`, 5 trivial (collinear, detected via the inertia tensor), 2 nontrivial, both exactly flat | `n = ∞`, 7 | **PASS** |
| 2×2×2 checkerboard, `N=8` | `B=12`, Maxwell 12, 6 nontrivial, all flat | reference | **PASS** |

The screen reproduces FTD-0789 independently. Along a trimer bend:

| `t` | straight-line path | relaxed path |
|---|---|---|
| 0.005 | 3.375e-10 | **0** |
| 0.01 | 5.399e-09 | **0** |
| 0.05 | 3.358e-06 | **0** |
| 0.1 | 5.292e-05 | **0** |

Straight-path ratios are `t⁴` to three figures. **That is FTD-0787's quartic
and FTD-0789's refutation of it, re-derived by an independent implementation.**

**The controls did their job by failing first.** The initial run walked null
directions out to amplitude 1.0, where the trimer dissociates
(`dE = +0.02 = 2ε`, both bonds broken), and the classifier scored dissociation
as curvature — **reporting the trimer as an `n = 4` candidate**, precisely the
error the screen exists to prevent. Fixed by tracking the bond set along the
walk and discarding amplitudes where it changes. Recorded per prereg §7.3
rather than silently patched.

## 3 · Tier C — the SC binding network

| block | `N` | `B` | `3N−B` | trivial | `dim N₀` | all flat? | verdict |
|---|---|---|---|---|---|---|---|
| L=2 | 8 | 12 | 12 | 6 | 6 | yes | `n = ∞` |
| L=3 | 27 | 54 | 27 | 6 | 21 | yes | `n = ∞` |
| L=4 | 64 | **144** | **48** | 6 | 42 | yes | `n = ∞` |

L=4 reproduces the repo's registered numbers exactly — **rank-144 central-bond
Hessian with 48 zero modes**, matching "all 48 axial row slides are tangent to
them and form the null space of the rank-144 central-bond Hessian."

**The preregistered decisive question is answered: the row slides are exactly
flat, to machine zero, at every block size.** They are finite mechanisms.

## 4 · Correction — the `12 ε N γ⁴` shear quartic is affine, not relaxed

The repo records the SC network as *"cohesive but not yet solid: dilation costs
`144 ε N η²`, whereas simple shear costs only `12 ε N γ⁴`, giving zero harmonic
shear modulus."* A positive quartic on a first-order-flat direction is the
`n = 4` signature, so this was the strongest single lead for C3.

Measured under three protocols, all holding the net shear exactly (verified —
the surviving shear fraction is `1.0000` in every case, so none of this is a
snap-back artifact):

| γ (L=4) | affine | pinned boundaries | **free body (projected)** |
|---|---|---|---|
| 0.01 | 5.759e-8 | 5.759e-8 | **5.8e-15** |
| 0.05 | 3.588e-5 | 3.588e-5 | **2.0e-15** |
| 0.1 | 5.683e-4 | 5.683e-4 | **2.0e-15** |

**The quartic scaling is confirmed** — `dE/γ⁴` is constant at 2.1597 (L=3) and
5.7592 (L=4) over `γ ∈ [0.01, 0.05]`.

**The coefficient is `12 ε L²(L−1)`, not `12 ε N`:** `12(0.01)(9)(2) = 2.16`
and `12(0.01)(16)(3) = 5.76`, matching to five figures. Since
`L²(L−1) = N(L−1)/L`, the registered `12 ε N` is the correct **bulk asymptote**
and carries a `(L−1)/L` surface factor at finite `L`. The count is just the
number of bonds along the shear-gradient axis. **No error in the registered
formula as a bulk statement.**

**But the relaxed cost is exactly zero.** The affine and pinned protocols agree
because pinning the extreme `y`-planes *forbids the row-slide mechanism* —
rows cannot slide when their ends are clamped. Release them and the block
reaches the identical net shear by **sliding whole rows, at machine-zero cost**.

So `12 ε N γ⁴` is the **Cauchy–Born / affine** shear cost. It is the energy of a
deformation path that forbids the mechanism the network actually uses. **The
free network has no shear stiffness at any order, not merely at harmonic
order.** This is the FTD-0787 error class — a chord across a flat valley —
arising here from an affine ansatz rather than a hand-chosen path.

**Consequence for the framing:** "cohesive but not yet solid" understates it.
In the shear channel the SC binding network is not a soft solid, it is a
**fluid to all orders**. The `γ⁴` is real only for a *clamped* block; C3 needs
a free body, where it is absent.

## 5 · Tier B

38 bound equilibria over `N = 3..6`, polarities `{−1, 0, +1}`, 24 seeds per
decoration, retained when connected (`B ≥ N−1`) and stationary
(`|∇E| < 1e-9`). **All 38 classify `n = ∞`.** No `n = 4`, and no intermediate
exponent.

Note the neutral state was included, as the prereg required: the mask returns
`1/2` whenever either site is `s = 0`, so `(+1,−1,0)` triples have all three
pairs bonded, unlike FTD-0787's `(+1,−1,+1)`. **Breaking the bipartite
structure did not produce second-order rigidity.**

## 6 · What this does and does not establish

**Does:** every configuration screened — the two registered ones, 38 small
clusters including neutral-decorated ones, and SC blocks to `L=4` — sits at
`n = 2` or `n = ∞`. **FTD-0783's bracket theorem stands, and its strongest
apparent counterexample is removed.** C3 remains unrealized.

**Does not:** this is a *screen*, not a no-go theorem. `N ≤ 6` in Tier B,
`L ≤ 4` in Tier C, zero tension, central forces only. A no-go would require
proving that no central-force network under this law can be first-order
flexible and second-order rigid — which is a real and now well-posed
mathematical question, and is *not* answered here.

## 7 · The positive control the prereg omitted — and the obstruction it found

Tier A carries only **negative** controls (a known `n = ∞` and a known `n = 2`).
A screen structurally blind to `n = 4` would pass both and return
`NO_NATIVE_N4` for the wrong reason. That hole is now closed
(`verify_n4_positive_control.py`).

Textbook first-order-flexible / second-order-rigid framework: **three collinear
points with all three bars**, natural lengths `1, 1, 2`, zero tension. Moving
the middle point perpendicular preserves every bond to first order; at second
order the two short bars must lengthen by `~d²/2` while the long bar is fixed,
and the triangle inequality blocks the motion.

| `t` | straight | **relaxed** | fitted exponent |
|---|---|---|---|
| 0.02 | 9.00e-8 | 6.00e-8 | **3.9999** |
| 0.05 | 3.51e-6 | 2.34e-6 | **3.9994** |
| 0.1 | 5.58e-5 | 3.74e-5 | **3.9973** |

Clean `t⁴`, positive coefficient, **surviving relaxation** at a ratio of `2/3`.
That ratio is itself the discriminator this whole screen turns on: **a genuine
quartic relaxes to a finite fraction of its straight-line value; a chord
relaxes to zero.** The screen detects `n = 4` when present, so `NO_NATIVE_N4`
is a real negative.

**A conjecture drafted for this section was refuted by that control and is
recorded rather than deleted.** It read: *second-order rigidity needs a
self-stress, a zero-tension central-force network at its bond minimum has none,
so C3 is unreachable in this class as a matter of structure.* **False** — the
control above is zero-tension, central-force, at its bond minimum, and is
second-order rigid. Standard rigidity theory agrees: along an infinitesimal
flex that does not extend, `δ_ij = ½|P⊥Δu|²/L`, so `E = Σ½k δ² = O(u⁴) > 0`
generically. `n = 4` is *generically available* in this class.

**So the obstruction is not the class — it is the support radius.** The
blocking bond must span the flexing unit. For a collinear triple with sub-bonds
at their minimum `r = 1`, the closing bond is at `r = 2`. The registered compact
law has support `r < √(3/2) ≈ 1.2247`. **2 > 1.2247, so the mechanism is
excluded by compact support, not by central-force character.** Equivalently:
closing a two-bond flex needs a support-to-minimum ratio `≥ 2`, and this law's
ratio is `1.2247`.

**That is the sharpest statement this screen supports, and it is falsifiable.**
It predicts `n = 4` becomes available if the support-to-minimum ratio reaches
2 — reachable by a longer-range law, or by a configuration whose closing bond
is geometrically shorter than twice the sub-bond. Whether *any* flex-closing
geometry fits inside ratio `1.2247` is **`[OPEN]`**: the `N ≤ 6` search found
none, which is suggestive and not a proof. **That question — not another
construction — is the successor, and it would convert this screen into the
no-go it currently is not.**
