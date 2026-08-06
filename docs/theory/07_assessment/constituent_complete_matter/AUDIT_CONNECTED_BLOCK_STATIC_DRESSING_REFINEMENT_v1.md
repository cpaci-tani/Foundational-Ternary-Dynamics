# Audit — FTD-0628 connected-block static dressing refinement

**Status:** `[AUDIT — ENGINE-RESOLUTION STATIC DRESSED FIXED POINT
CONSTRUCTIVE]`  
**Verdict:** `CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE`

## Findings

1. The preregistered four-coordinate Newton refinement converges from the sole
   allowed rigid start in two accepted full steps for both cyclic arms.
2. Static energy decreases by `1.3778846684e-5`; the final gradient is below
   `2.36e-10` and every independently recomputed Hessian eigenvalue is positive,
   with minimum `21.0199`.
3. The unmodified common-action step passes the full 48-coordinate stationarity
   test. Maximum constituent impulse is `6.36e-10` against `1e-9`.
4. Both refined states complete 64 forward and 64 state-only inverse ticks.
   Complete-state motion remains below `1.58e-9`, centre motion below
   `1.17e-14`, energy drift below `1.78e-15`, and recovery below `3.17e-14`.
5. The complete rotated state agrees to `6.54e-14`; the locked aggregate cubic
   covariance gate passes at `2.69e-10`.
6. The existing multiplicity-two chart remains sufficient. Shared-anchor
   constituents stay `0.998934` cell apart, so no reaction interpretation is
   implicated.
7. The independent record proof passes 64/64 checks, including its own Jacobi
   diagonalization of the recorded Hessians.

## Audit disposition

The rigid FTD-0626/0627 centre-rest initialization was an excited geometry, not
a stationary internal configuration. FTD-0628 constructively establishes a
nearby engine-resolution dressed fixed point without a new primitive or a
change to the selected action.

The result is numerical, finite-volume, finite-horizon, and margin-limited at
the `1e-9` full-space gate. It does not prove an exact fixed point, global
minimum, particle, mass, clock, quantum state, native formation channel, or
infrared pole. Production remains unchanged.
