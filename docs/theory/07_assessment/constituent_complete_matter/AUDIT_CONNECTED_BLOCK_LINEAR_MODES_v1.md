# Audit — FTD-0629 connected-block linear modes

**Status:** `[AUDIT — FOUR SYMMETRY-PRESERVING FINITE-AMPLITUDE CLASSICAL RESPONSES
CONSTRUCTIVE]`  
**Verdict:** `CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE`

## Findings

> **FTD-0634--0637 correction:** the reduced second-difference stencil crosses
> a `C1` coat knot. Items below audit the locked finite-amplitude campaign; the
> word `mode` is historical protocol terminology, not a qualified
> infinitesimal-mode claim.

1. The FTD-0628 static Hessian and unchanged production inertia determine four
   positive generalized eigenvalues without a new fitted coefficient.
2. The implicit-midpoint phase formula predicts four periods between `2.3577`
   and `3.7403` ticks. Every mode-selective arm agrees within the locked 2%; the
   worst frequency error is `0.1722%`.
3. Maximum cross-mode leakage is `0.1866%`, far below the locked 10% gate.
4. Both amplitudes exhibit the registered quadratic-energy and phase-stability
   behavior. The aggregate amplitude residual is `2.53e-4`.
5. Signed trajectories mirror to `1.03e-4`; cyclic trajectories agree to
   `7.28e-11`.
6. All 16 arms complete 64 forward and 64 state-only inverse ticks. Worst
   common-action residual is `1.94e-11`, energy drift `8.44e-15`, and recovery
   `5.13e-13`.
7. The independent certificate recomputes the generalized eigensystem,
   trajectory projections, recurrence phases, leakage, scaling, symmetry,
   covariance, and verdict; 167/167 checks pass.

## Audit disposition

FTD-0629 constructively isolates four classical symmetry-preserving
finite-amplitude internal responses of the selected dressed object. FTD-0637
later supplies the complete analytic matter Hessian and FTD-0638 the exact
center, so infinitesimal response frequencies must be recomputed there.
Independent field modes remain outside the campaign. The recorded frequencies
cannot be cited as physical particle masses or quantum levels.

Production remains unchanged.
