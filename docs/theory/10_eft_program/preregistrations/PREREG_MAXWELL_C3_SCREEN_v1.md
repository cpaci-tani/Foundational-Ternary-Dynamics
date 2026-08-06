# PREREG — Maxwell-criterion screen for a native `n = 4` mechanism (C3)

**Status:** `[PREREGISTRATION — LOCKED BEFORE EXECUTION]`
**Question:** Does any native configuration under the registered compact law
satisfy the FTD-0789 criterion for `n = 4` — **first-order flexible with
second-order rigid** — i.e. `null(H)` nontrivial with the quartic form
**positive definite** on it?
**Parents:** FTD-0783 (bracket theorem), FTD-0787 (the refuted claim),
FTD-0789 (the refutation *and* the decidable criterion), FTD-0786 (C3 is the
mechanism by which C2 could be satisfied), `SPEC_CARRIER_CONSTRAINTS_v1.md`
**Production impact:** none — standalone analysis, no engine change.

## 1 · Why this is worth running

C3 is the wall. Every registered carrier candidate fails it, and FTD-0789
converted it from an open-ended search into a **decidable** criterion. Both
registered configurations fail from opposite sides: the connected
16-constituent block is rigid (`n = 2`, positive-definite 48-coordinate
Hessian, FTD-0637/0638) and the isolated collinear trimer is a free mechanism
(`n = ∞`, FTD-0789). Nothing has systematically searched *between* them.

## 2 · The registered model — verbatim, no substitutions

Pair potential in the squared distance `q = |r_i − r_j|²`
(`verify_flexural_refutation.py`, FTD-0789):

```text
V(q) = 0                                    if q >= 3/2
V(q) = -16*eps*(q - 3/2)^2 * (q - 3/4)      otherwise
```

Polarity mask `A(s_i, s_j) = (1 - s_i*s_j)/2`, total energy
`E = sum_{i<j} A(s_i,s_j) * V(q_ij)`, with `eps = 0.01`.

Consequences fixed in advance, none of them free parameters:
minimum at `q = 1` (`r = 1`) with depth `−eps` per bond; radial stiffness
`d²E/dr² = 96*eps`; compact support `r < sqrt(3/2) ≈ 1.2247`.

**Declared in advance — the neutral state is included.** The mask gives
`A = 1` for opposite polarity, `A = 0` for like polarity, and **`A = 1/2`
whenever either site is `s = 0`**. A `(+1,−1,0)` triple therefore has all
three pairs bonded, whereas FTD-0787's `(+1,−1,+1)` trimer had `A_AC = 0`
identically. Like-polarity-only bonding is bipartite and triangle-free;
admitting `s = 0` breaks that. This region has never been screened and is the
main reason to expect this screen can return something the previous attempts
could not.

**Positions are continuous.** The lattice sets the law; constituent positions
are real-valued (the registered `(q,X,p)` picture). On-lattice triangles do
not exist under compact support — sites at `(0,0,0)`, `(1,0,0)`, `(0,1,0)`
have `q_13 = 2 > 3/2` — but off-lattice equilateral triples at `r = 1` do.

## 3 · The criterion, stated formally

For a configuration `x0` with `∇E(x0) = 0` and `H = ∇²E(x0)`:

1. `T` := the trivial rigid-body null space — **6** generically (3 translations
   + 3 rotations), **5** for a collinear configuration. Collinearity is
   detected, not assumed.
2. `N0` := `null(H) ⊖ T`.
3. **`dim N0 = 0` → `n = 2` (rigid) → REJECT.**
4. **`dim N0 > 0`** → classify each direction:
   - energy stays flat to numerical zero at finite amplitude → **finite
     mechanism → `n = ∞` → REJECT**
   - energy grows as `c·t⁴`, `c > 0` → second-order rigid in that direction
5. **`n = 4` requires `c > 0` on *all* of `N0`** (positive definite, not merely
   positive semi-definite).

### 3.1 The FTD-0787 error, and the mandatory guard against it

FTD-0787 measured a quartic that was **the curvature of a rectilinear chord
across an exactly flat valley**. It parametrised a path by intuition
(transverse displacement) instead of along the actual null space, and that
path stretched both bonds while the system could reach the same offset by
bending, for free.

**Mandatory, and the screen is invalid without it:** energy along a null
direction is *never* evaluated on a straight line. At every amplitude `t` the
remaining coordinates are **relaxed** (minimised over the orthogonal
complement of the path) before the energy is read. A quartic that survives
relaxation is real; a quartic that vanishes under relaxation was a chord.

Any candidate returning `n = 4` must additionally pass an **O_h orientation
sweep** (all 48 frames, `geometry_probe.orient_sweep`) to exclude a
frame-choice artifact.

## 4 · Candidate set — declared before execution

**Tier A — controls with known answers.** These calibrate the screen.
- collinear trimer `(+1,−1,+1)`, bonds A–B, B–C — must return `n = ∞`,
  7 zero modes, `3N − B = 9 − 2 = 7`, bend flat to all orders
- the connected 16-constituent block — must return a positive-definite
  48-coordinate Hessian, `n = 2`

**Tier B — exhaustive small clusters.** All polarity-decorated configurations
with `N = 3..7` constituents, polarities drawn from `{−1, 0, +1}`, relaxed
from both lattice-aligned and randomised starts (≥ 20 seeds each), classified
by the §3 criterion. Enumerated up to `O_h` symmetry, translation, and global
polarity inversion.

**Tier C — the SC binding network, the sharpest single question.** The repo
already records that this network has **zero harmonic shear modulus**, with
dilation costing `144*eps*N*eta^2` but simple shear only `12*eps*N*gamma^4` —
*a positive quartic on a first-order-flat direction*, which is the `n = 4`
signature. It also records that **all 48 axial row slides are tangent to the
capacity rows and span the null space of the rank-144 central-bond Hessian.**

> **The decisive question, fixed here before it is computed:** are the
> row-slide directions **exactly flat** (finite mechanisms) or do they cost
> quartic? If exactly flat, the quartic form is positive *semi*-definite, the
> `n = ∞` branch wins, and the shear quartic is irrelevant. If they cost
> quartic, the SC network satisfies the criterion.

Tested on checkerboard-polarity blocks `L = 2, 3, 4`.

## 5 · Observables (declared before execution)

Per candidate: `N`, bond count `B`, Maxwell count `3N − B`, full Hessian
spectrum, `dim T`, `dim N0`, per-null-direction relaxed energy profile
`E(t) − E(0)` at `t ∈ {0.01, 0.03, 0.1, 0.3, 1.0}`, the fitted leading
exponent and coefficient, and the classification `{2, 4, ∞}`.

## 6 · Preregistered outcomes

- **`NO_NATIVE_N4`** — every candidate classifies `n = 2` or `n = ∞`. C3
  stays unrealized and the bracket theorem stands unchallenged.
- **`N4_FOUND`** — at least one candidate has `dim N0 > 0` with the quartic
  form positive definite on all of `N0`, surviving relaxation and the `O_h`
  sweep. **C3 realized natively.**
- **`N4_SEMIDEFINITE`** — quartic positive on some null directions and exactly
  flat on others. `n = ∞` wins, but the flat directions are then named
  explicitly and become the object to suppress. *(This is the outcome I expect
  for Tier C, recorded here so that expecting it cannot later be presented as
  predicting it.)*
- **`SCREEN_INVALID`** — the Tier-A controls fail to reproduce their known
  verdicts.

## 7 · Kill conditions

1. If the collinear trimer does **not** return `n = ∞` with 7 zero modes and a
   flat bend, the implementation is wrong: **`SCREEN_INVALID`, not evidence.**
2. If the 16-constituent block does **not** return a positive-definite
   48-coordinate Hessian, same.
3. Any `n = 4` report that does **not** survive coordinate relaxation (§3.1) is
   discarded as an FTD-0787 repeat, and its discovery is reported as a
   near-miss rather than suppressed.
4. Any `n = 4` report whose `O_h` spread exceeds `1e-9` is a frame artifact.
5. Numerical: null-space tolerance `1e-7` on Hessian eigenvalues, with the
   classification re-run at `1e-6` and `1e-8`. **A verdict that changes across
   that range is reported as indeterminate, not resolved by choosing.**

## 8 · What this cannot show

- **Zero tension only.** The trichotomy assumes `rank(H) <= B` at zero
  tension. Under tension the counting changes and this screen says nothing.
- **Central forces only.** The registered law is a pair potential in `|r|`.
  Any future angular or three-body term voids the criterion's applicability.
- **`n = 4` would satisfy C3 alone.** It does not deliver C2 — the resulting
  frequency must still clear the band top `2*arcsin(1/sqrt3) = 1.230959` — nor
  any of the other ten constraints, nor the three Gates.
- **`N <= 7` in Tier B**, `L <= 4` in Tier C. Larger configurations unscanned;
  a negative result is scoped to the enumerated set and is not a no-go theorem.
- Per **FTD-0784**, even total success cannot deliver the FC-W surd
  `sqrt(G*(4G* − 1))`; W stays external regardless of this outcome.
