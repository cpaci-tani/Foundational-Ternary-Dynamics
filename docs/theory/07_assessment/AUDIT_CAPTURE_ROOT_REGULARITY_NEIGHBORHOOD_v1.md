# Audit — FTD-0735 capture-root regularity and finite-time neighborhood

**Status:** `[AUDIT — PASS WITH FINITE-TIME SCOPE]`  
**Verdict checked:**
`CAPTURE_FINITE_TIME_OPEN_NEIGHBORHOOD_NUMERICALLY_SUPPORTED`

## Audit finding

The run supports a finite-time open-neighborhood statement on the admissible
selected-dynamics state manifold.  It does not support an invariant basin,
attractor, or asymptotic particle claim.

The distinction is material.  FTD-0734 sampled a finite mixed set with strict
capture margins.  FTD-0735 measures the missing local uniqueness condition:
all 9,216 accepted forward/reverse roots have minimum singular value at least
`0.938619877`, maximum condition number `1.087957209`, and two-scale minimum-
singular-value discrepancy at most `6.36490e-8`.  The selected trajectories are
therefore not numerically close to an implicit branch fold.

The independent certificate verifies hashes, the exact 18-history matrix, all
9,216 root rows, both 256-tick phase ranges, every root/action/regularity gate,
the locked hostile selector names, forward capture margins, and all aggregate
extrema: `92236/92236 PASS`.

## Epistemic limit

The general implicit-function argument is a theorem.  The claim that its
nonsingularity hypothesis holds for these floating-point roots is a numerical
fact, not an exact determinant proof.  The correct combined status is
`[CONDITIONAL THEOREM + NUMERICAL FACT]`.

An open set for a fixed finite horizon is also weaker than an invariant open
set.  Its radius may contract with horizon and may approach zero.  No language
of attraction, asymptotic stability, particlehood, or generic formation is
licensed.

## Infrastructure boundary

The final-root observer is default-off.  It evaluates the unchanged residual
at `h` and `h/2`, diagonalizes `J^T J`, and does not feed the measured Jacobian
to the nonlinear solver.  The observer-on/off regression reports endpoint
difference zero.  Production defaults and the FTD-0734 run-of-record are
unchanged.
