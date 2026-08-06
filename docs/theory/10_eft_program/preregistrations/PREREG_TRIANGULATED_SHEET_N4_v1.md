# PREREG — Is a periodic triangulated sheet an `n = 4` mechanism? (C3)

**Status:** `[PREREGISTRATION — LOCKED BEFORE EXECUTION]`
**Question:** Does a periodic triangular-lattice sheet of neutral (`s = 0`)
sites satisfy the FTD-0789 criterion — `null(H)` nontrivial with the quartic
form positive definite on it?
**Parents:** FTD-0800 (the screen, and the SC/BCC/FCC separation), FTD-0789
(the criterion), FTD-0783 (bracket theorem), `SPEC_CARRIER_CONSTRAINTS_v1.md`
**Production impact:** none.

## 1 · Why this configuration, by elimination

FTD-0800 killed every prior candidate through one of two failures:

- **no self-stress at all** — SC bulk, the cube graph / stella octangula, the
  collinear trimer. Connelly requires a self-stress, so `n = ∞` is forced.
- **stress outnumbered by flexes** — the hexagon+centre had `stress = 1`
  against `flex = 4`, and its stress was *mixed-sign*
  (`ω_spoke = −ω_ring`, solved by hand), so the centre-piston mode went
  negative while ring modes went positive. Indefinite.

Mixed signs there were structural, not unlucky: by Maxwell–Cremona a planar
self-stress corresponds to a polyhedral lift, and **boundary edges carry the
opposite sign to interior ones.** The hexagon was almost all boundary.

The periodic triangulated sheet fixes both, and is the first configuration
where the counting favours us:

| property | value | why it matters |
|---|---|---|
| bonds | `B = 3N` (6-coordinated) | over-constrained in-plane |
| in-plane rank | `≤ 2N − 2` | triangulated ⇒ in-plane rigid |
| **self-stress** | **`≥ N + 2`** | guaranteed, not hoped for |
| out-of-plane flex | `N − 1` | planar ⇒ every `z` mode is a flex |
| **stress vs flex** | **`N+2 > N−1`** | first time stress *exceeds* flex |

**`ω ≡ 1` is a self-stress, and it is all-positive.** The six neighbour
vectors of a triangular lattice sum to zero, so `Σ_j ω_ij(p_i − p_j) = 0`
holds identically at every site. Then on out-of-plane modes the Connelly form
`Σ ω_ij(z_i − z_j)²` is the **graph Laplacian**, positive definite on every
non-constant `z`. **Periodicity removes the boundary, hence the sign flip that
killed the hexagon, and also removes the tilt modes (`z = αx + βy` is not
periodic).**

Geometry fits the law exactly: NN at `q = 1` (the minimum), second neighbours
at `√3 = 1.732`, safely beyond support `1.2247`. No unwanted bonds, and no
second length scale is required — which matters because FTD-0800 §7/§8 showed
the compact law is single-scale.

Polarity: the triangular lattice has odd cycles, so it cannot be `±`
2-coloured. **All sites `s = 0`**, where the mask gives `1/2` for every pair.
This is forced, not chosen.

## 2 · The corrugation problem — named before execution

**This is the way I expect it to fail, and it is recorded here so that it
cannot later be presented as a prediction.**

A flat sheet folds into a wave at zero stretching cost. For `z = A cos(qx)`
the induced strain is `(∂z/∂x)² = A²q²(1 − cos 2qx)/2`. Its **mean** part is
absorbable by contracting the cell; its **`2q` modulation** is absorbable by
an in-plane phonon `u_x = (A²q²/8q) sin(2qx)`. If both are available the flex
extends exactly and the verdict is `n = ∞`.

Whether the mean part is absorbable depends entirely on the cell:

- **Fixed box** — uniform contraction is *not* a periodic displacement, so it
  is unavailable, `ω ≡ 1` blocks the flex, and the prediction is `n = 4`.
- **Free box** — it is available, and the flex may extend exactly.

**A carrier is a free body, so the free-box protocol is the physically
relevant one for C3.** Both are run and reported. If they disagree, the free
box governs and the fixed-box quartic is recorded as a *clamped* result —
exactly the distinction that reclassified the SC shear quartic in FTD-0800 §4.

## 3 · Protocols

Both use the FTD-0800 relaxation guard: energy along a null direction is never
evaluated on a straight line; all other coordinates are relaxed at every
amplitude, and amplitudes that change the bond set are discarded.

- **P1 fixed box.** Relax in-plane and out-of-plane site coordinates at fixed
  lattice vectors.
- **P2 free box.** Additionally relax the two in-plane lattice vectors
  (variable-cell). The out-of-plane mode amplitude is held in both.

Sheets: `m × n` supercells for `(m,n) ∈ {(4,4), (6,6), (8,8)}`, to check the
verdict is not a finite-size artifact.

## 4 · Controls (can invalidate the run)

- **C-pos (positive):** the FTD-0800 collinear-triple control (bars `1,1,2`)
  must still return a clean `t⁴` with positive coefficient surviving
  relaxation, **re-run under the periodic-boundary code path**, to prove the
  probe is not blinded by PBC.
- **C-neg (negative):** an SC sheet (square lattice, 4-coordinated, `B = 2N`)
  must return `n = ∞`. It is under-constrained and untriangulated, so it has
  no positive self-stress.
- **C-stress:** `ω ≡ 1` must be verified numerically to be a self-stress
  (residual `< 1e-12` at every site), not merely asserted from symmetry.

## 5 · Preregistered outcomes

- **`N4_CONFIRMED`** — quartic positive definite on the whole out-of-plane
  flex space under **P2 (free box)**, surviving relaxation, at all three sizes.
  **C3 realized natively.**
- **`N4_CLAMPED_ONLY`** — positive under P1 but exactly flat under P2. The
  corrugation of §2 wins; `n = ∞` for a free body. **Expected.**
- **`N4_PARTIAL`** — positive on some out-of-plane modes and flat on others
  under P2 ⇒ positive semi-definite ⇒ fails the criterion, but the surviving
  directions are named.
- **`SHEET_INVALID`** — any control in §4 fails.

## 6 · Kill conditions

1. Any control in §4 failing ⇒ `SHEET_INVALID`, not evidence.
2. A quartic that vanishes under relaxation is an FTD-0787 repeat and is
   reported as a near-miss, never as a result.
3. Verdicts must be stable across null tolerances `1e-6 / 1e-7 / 1e-8`;
   a verdict that changes is reported **indeterminate**, not resolved by
   choosing a tolerance.
4. A verdict that changes with supercell size is reported as a finite-size
   artifact, not resolved by picking the favourable size.

## 7 · What this cannot show

- **A sheet is 2-dimensional.** Even `N4_CONFIRMED` yields a *membrane*, not
  a localized carrier. C3 wants a bounded object, and a finite patch
  reintroduces the boundary whose sign flip killed the hexagon. **There may be
  a genuine tension between "no boundary" and "finite", and this run does not
  address it.** That is the next question either way.
- Zero tension, central forces, single scale — the FTD-0800 scope caveats
  carry over unchanged.
- `n = 4` satisfies **C3 only**. It delivers neither C2 (the frequency must
  still clear `2 arcsin(1/√3) = 1.230959`) nor the other ten constraints.
- Per FTD-0784, even total success cannot deliver the FC-W surd.
