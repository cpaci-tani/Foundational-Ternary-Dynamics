# Audit — External-Drive Radiation Functional

**Record:** FTD-0559
**Date:** 2026-07-26
**Verdict:** `EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_DERIVED`
**Production changes:** none

## Result

The corrected production field operator now has an exact external source-work
ledger and finite-time retarded solution.  On a finite lossless lattice,
off-resonant harmonic energy is bounded and exact-resonance energy grows as
`|F|²N²/2`.  The constant-rate object is the Fejer/continuum limit, whose
resonance-surface Jacobian is the source/group-velocity mismatch.

## Locked CPU campaign

| Diagnostic | Result |
|---|---:|
| exact work arms | 12/12 |
| harmonic response arms | 24/24 |
| Fejer normalization arms | 4/4 |
| maximum one-step work residual | 7.28583859910e-17 |
| maximum direct/closed response residual | 5.12379653438e-13 |
| maximum cumulative work residual | 1.81898940355e-12 |
| maximum Fejer residual | 1.55431223448e-15 |
| maximum resonant coefficient error | 0.00189107114801 |
| minimum registered resonant bound margin | 0.0520630269221 |
| minimum off-resonant bound margin | 110.873684701 |
| failures | 0 |

The cumulative work residual is below the preregistered `1e-10` response/work
gate.  Every asymptotic coefficient lies inside its analytic conservative
bound; every off-resonant trajectory remains below its analytic uniform-energy
bound.

## Status consequence

- The field-side external-drive work and radiation functional are
  theorem-grade for the frozen production operator.
- The retracted FTD-0120 physical power claim is not reinstated: it lacked a
  native source/current, reciprocal recoil, and the correct discrete-time
  pole/Jacobian.
- A smooth prescribed source below `2/(3pi)` has zero principal-branch
  external-drive radiation rate.
- A periodic integer-hop source can carry radiating Floquet harmonics; their
  weights and resonance surfaces depend on the hop schedule.

## Remaining gate

Physical radiation requires the same exact work to appear with opposite sign
in a dynamically selected manifested carrier.  Until a stable mobile carrier
and common current-field transaction exist, `P_ext` cannot be called particle
energy loss, Larmor power, or a phenomenological prediction.
