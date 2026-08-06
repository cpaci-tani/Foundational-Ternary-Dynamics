# FTD-0725 — Lower-energy covariance conditioning v1

**Status:** `[SELECTED NUMERICAL REALIZATION + MEASURED — CONDITIONING
DEFECT CONFIRMED]`  
**Verdict:** `COVARIANCE_DEFECT_NUMERICAL_CONDITIONING_CONFIRMED`  
**Production status:** unchanged

## Result

The FTD-0724 translation-covariance failure is attributable to nonlinear
root-solver termination accuracy in the registered long-interaction sector.
Replaying the unchanged physical problem with a ten-times tighter solve
tolerance reduces both the scalar and complete-state covariance defects below
the locked `1e-9` gate while preserving every graph and energy-sign class.

The baseline diagnostic exactly reproduces the parent maximum scalar spread
`1.0680766715509549e-8`. At solve tolerance `2e-12`, the maximum is
`8.9040064210621495e-10`. The translated complete-state defect falls from
`3.4504217538700033e-9` to `3.5991636233856372e-10`.

```text
paired physical problems per condition       78
forward histories per condition             156
per-tick records total                     7,644
executed/gate-pass histories            312/312
translation class-agreement pairs       156/156
raw negative unbound histories          104/130 per condition
bound controls retained                  26/26 per condition

scalar covariance       1.0681e-8 -> 8.9040e-10  ratio 0.08336
complete-state covariance 3.4504e-9 -> 3.5992e-10 ratio 0.10431
electric covariance     2.3579e-9 -> 2.6265e-10
magnetic covariance     2.0772e-9 -> 2.2642e-10
matter covariance       3.4504e-9 -> 3.5992e-10
root residual           1.9998e-11 -> 1.9914e-12
```

## Localization

At baseline precision the worst scalar defect is constituent separation for
the raw trapped `p=0.0095`, direction `1_-1_1` family at tick 36. The worst
complete-state defect is matter position/momentum in the same family at tick
39. Electric and magnetic differences peak later but remain smaller.

At tight precision the worst scalar and complete defects remain in the
`p=0.0095` raw trapped family, now on direction `1_1_0` at ticks 42 and 39.
The dominant complete-state component remains matter phase space. This
localizes the sensitivity near the raw formation boundary; it does not expose
an independent face/edge field asymmetry.

## What the result proves and does not prove

The campaign demonstrates numerical convergence of the selected finite-volume
common-action solution under lattice translation for the frozen matrix. It
also shows that the FTD-0724 raw sign pattern does not disappear when the root
is solved more accurately: all four lower families remain raw-negative and
the `p=0.0120` family remains positive in both translations.

It does not retroactively validate FTD-0724, prove global root uniqueness, or
establish exact translation covariance as a theorem. The tight root tolerance
is a selected numerical realization of the same mathematical action. A full
fresh campaign must still repeat both polarity orders, state-only inversion,
energy exchange, and the capture morphology classifier at that tolerance.

## Ontological consequence

No new primitive is priced by the FTD-0724 covariance miss. The complete
constituent plus face/edge state converges to the same translated history as
the numerical root is tightened. The remaining matter question is physical:
does the lower-energy negative basin persist and behave as one covariant
complete-state object, or is it temporary near-field trapping that later
returns its energy?

The next admissible gate is therefore a fresh full tighter-root formation
campaign. Only after it passes may a longer-horizon stability/recurrence test
be run.

## Scope

This is a solver-conditioning result, not matter formation. Production
defaults retain solve tolerance `2e-11`; no action coefficient, equation,
physical initial state, toggle, scenario, or production tick changed.
