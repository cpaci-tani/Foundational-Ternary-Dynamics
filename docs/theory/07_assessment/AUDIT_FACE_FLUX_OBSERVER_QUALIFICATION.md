# FTD-0480 — Face-flux observer qualification

**Date:** 2026-07-25  
**Status:** `[CLOSED NEGATIVE — FROZEN E/B-ONLY FORCE GATHER]`  
**Verdict:** `OBSERVER_QUALIFICATION_IDENTITIES_FAIL`

The broad campaign passes 58 of 70 arms and fails 12. Translation and proper
cubic-rotation residuals are `3.06e-16` and zero. The failing arms are the two
static-dressing controls and both polarities of the `+z` axis arm in each of
the five moving field families. Their implicit residuals remain between
`9.27e-4` and `1.195e-3`, far above the locked `1e-12` gate.

The failure exposes an algebraic underdetermination, not a tolerance problem.
For a component with nonzero displacement, face-current work fixes

```text
Ebar_i = <K_i,E_i> / (q Delta x_i).
```

For `Delta x_i=0`, both numerator and work contribution vanish, so continuity
and energy give no value for the transverse force component. The implemented
midpoint interpolation is a selected completion. Edge-B collocation is also
selected rather than obtained from the current-generating interaction. Thus
the frozen variables do not determine the full three-vector impulse required
by the plan.

FTD-0481 is not licensed. No `common_action_face_dynamics` toggle or dashboard
scenario was added, and FTD-0482/0483 remain unexecuted behind that gate.
