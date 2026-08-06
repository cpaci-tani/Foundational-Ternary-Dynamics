# ANALYSIS — The periodic triangulated sheet is `n = 4` only when clamped

**Status:** `[CLOSED NEGATIVE — SCOPED]` + `[ENGINE FACT — MEASURED]` +
`[SYNTHESIS — THE CLAMPED-QUARTIC PATTERN]`
**Verdict:** `N4_CLAMPED_ONLY`
**Protocol:** `../preregistrations/PREREG_TRIANGULATED_SHEET_N4_v1.md`, locked
at commit `0abf097b` **before the runner existed**. Executed 2026-08-04.
**Runner:** `scripts/experiments/triangulated_sheet_n4.py`
**Production impact:** none.

## 1 · Outcome

The registered outcome `N4_CLAMPED_ONLY` is what occurred, and it occurred by
the mechanism named in prereg §2 before execution.

| mode | P1 fixed box | **P2 free box** |
|---|---|---|
| corrugation `cos(q·x)` | `t⁴`, exponent **3.999** | → `4.5e-12` |
| corrugation `cos(2q·x)` | `t⁴`, exponent **3.999** | → **exactly `0.000e+00`** |
| egg-carton | `t⁴`, exponent **3.999** | → `4.9e-13` |
| random | `t⁴`, exponent **3.998** | → `6.4e-8`, exponent 2.97 |

Stable across `4×4` and `6×6`; P1 exponents lie in `[3.991, 3.999]` for every
mode at both sizes.

**Control `C-stress` passed:** `ω ≡ 1` is a self-stress, verified numerically
rather than asserted from symmetry — `max_i |Σ_j (p_i − p_j)| = 8.9e-16`
(`4×4`) and `1.8e-15` (`6×6`). The premise of the whole construction held. The
counting held too: `B = 3N`, so self-stress `≥ N+2` against `N−1` out-of-plane
flexes — the first candidate where stress *exceeds* flex.

**It still failed**, because Connelly's criterion is not the end of the story
once the cell is free.

## 2 · Why — the corrugation, as predicted

A flat sheet folds into a wave at zero stretching cost. For `z = A cos(qx)`,
the induced strain `(∂z/∂x)² = A²q²(1 − cos 2qx)/2` splits into a **mean**
part, absorbable by contracting the cell, and a **`2q` modulation**,
absorbable by an in-plane phonon `u_x = (A²q²/8q) sin(2qx)`.

With the box **fixed**, uniform contraction is not a periodic displacement, so
it is unavailable and `ω ≡ 1` blocks the flex — hence P1's clean quartic. With
the box **free**, both channels open and the flex extends exactly. `cos(2q·x)`
reaching **identically zero** is the cleanest demonstration: that mode is
exactly absorbable, so the deformation is an isometry of the discrete sheet.

**A carrier is a free body.** P2 governs, and P2 says `n = ∞`.

## 3 · An instrument failure, found and recorded

The first run reported P2 energies that were **non-monotonic by four orders**
(`6×6`, `cos(q·x)`: `9.3e-11 → 3.5e-9 → 5.4e-8 → 4.5e-12`). That is optimiser
failure, not physics: the variable-cell landscape is stiff and a single
L-BFGS start does not reach the minimum at most amplitudes.

Per prereg §6.3 this was reported **indeterminate** rather than read as a
verdict. Fixed by restarting from perturbed initial conditions and taking the
best minimum, after which P2 collapses consistently to machine zero. Recorded
rather than silently patched.

**A verdict was very nearly read off unconverged numbers**, and only the
preregistered stability requirement prevented it. The same requirement is why
the `6×6` random mode is flagged below rather than absorbed into the result.

## 4 · Loose end, stated plainly

The `6×6` **random** mode under P2 sits at `6.4e-8` with exponent `2.97` —
three orders below its P1 value but not at machine zero, and not a clean
quartic. Either a generic (non-developable) undulation carries a genuinely
small residual, or the optimiser has not converged for the hardest mode.
**Unresolved.** It does not change the verdict — `cos(2q·x)` reaching exactly
zero already exhibits an isometric finite mechanism — but it is not swept up.

## 5 · The pattern this completes

This is the **third** independent mechanism producing the same result, and the
generalization is the most useful thing in this arc:

| result | quartic appears under | vanishes when |
|---|---|---|
| FTD-0787 / 0789 trimer | a straight-line path | coordinates relax (the bend is flat) |
| FTD-0800 §4 SC shear | affine / pinned boundary | boundary released (row slides) |
| **this** | fixed cell | **cell released (corrugation)** |

> **Every quartic found under the registered compact law is a *clamped*
> quartic.** It appears whenever a constraint is imposed and vanishes when the
> body is free. Three different constraints — a held path, a pinned boundary, a
> fixed cell — and three different escape mechanisms — bending, row sliding,
> corrugation — with the same outcome each time.

That is consistent with, and sharpens, FTD-0800 §7–§8: the compact law is
**single-scale**, so nothing in it can close a flex; a free body always finds
the isometry. `n = 4` keeps appearing exactly where a constraint supplies the
second scale the law lacks.

## 6 · Where C3 now stands

Every geometry reachable by elimination has been screened and closed:
the trimer, the 16-block, 38 small clusters, SC/BCC/FCC, the stella octangula,
the hexagon+centre, and now the triangulated sheet — the best candidate the
elimination produced, with guaranteed all-positive self-stress and favourable
counting.

**C3 remains unrealized, and the obstruction now has a name rather than a
count.** The live routes are unchanged and both step outside the registered
law: a **two-scale interaction** (second minimum or second interaction range,
FTD-0800 §8), or **pre-tension** (`V' ≠ 0`), which supplies the missing
stiffness at the cost of being a stressed rather than unstressed equilibrium.

## 7 · Scope

Zero tension, central forces, single scale. `4×4` and `6×6` supercells only.
A sheet is 2-dimensional: **even a positive result would have given a
membrane, not a localized carrier**, and prereg §7 recorded in advance that a
finite patch reintroduces the boundary whose sign flip killed the
hexagon+centre. That tension between "no boundary" and "finite" is untouched
here and remains the structural question for any membrane-based carrier.
`n = 4` would satisfy **C3 only** — not C2, not the other ten constraints, and
per FTD-0784 not the FC-W surd.
