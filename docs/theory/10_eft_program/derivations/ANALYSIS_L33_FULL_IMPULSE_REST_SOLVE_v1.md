# FTD-0708 — L=33 full-impulse rest solve v1

**Status:** `[SELECTED DYNAMICS — CONSTRUCTIVE REST FIXED POINT]`  
**Verdict:** `L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE`  
**Production status:** unchanged

The full 48-component one-tick impulse residual is a valid local rest
equation. One undamped Newton step with minimum pivot `0.0143249` and maximum
coordinate displacement `9.46581e-6` reduces its infinity norm from
`1.06261e-5` to `3.25973e-11`.

The corrected complete state passes the actual dynamics: one-step state change
`3.25913e-11`, total momentum `4.05e-17`, zero hops over eight forward ticks,
maximum state excursion `5.83564e-11`, center drift `3.55e-15`, energy drift
`4.44e-16`, common residual `6.70e-14`, reverse recovery `2.51e-14`, and
integer-translation covariance `3.55e-15`.

This establishes a nearby selected `L=33` complete rest fixed point using only
the existing constituent coordinates, binding graph, and matched fields. The
FTD-0706 failure was a preparation error at rest, not evidence for a missing
primitive. This is not a production-emergence, particle-pole, or global-
stability theorem.

Record: protocol `D978E892...7D007`; JSON `C6CDA862...B51B6`; runner
`F18ED45C...7C372`; proof `ACAB4010...60001`.

