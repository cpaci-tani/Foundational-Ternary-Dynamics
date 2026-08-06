# FTD-0724 — Lower-energy formation crossover v1

**Status:** `[EXECUTION UNRESOLVED — GLOBAL COVARIANCE GATE FAILED; RAW
ENERGETIC TRAPPING NON-PROMOTABLE]`  
**Verdict:** `LOWER_ENERGY_TRANSACTION_UNRESOLVED`  
**Production status:** unchanged

## Result

The 48-tick lower-energy campaign executes all 312 complete histories and
passes every rowwise common-action, energy, recoil, and state-only inverse
gate. Its global translation/polarity scalar-history spread is
`1.0680766715509549e-8`, however, above the preregistered `1e-9` gate.
Consequently the registered scientific verdict is unresolved.

The raw energy-sign pattern is strong but non-promotable. All 208 arms at
`p=0.0060`, `0.0075`, `0.0085`, and `0.0095` enter the compact graph once,
remain inside for the final eight ticks, and finish with negative pair energy.
All 52 `p=0.0120` arms enter and leave, finishing just above zero. All 52
already-bound controls remain negative and connected.

## Locked campaign and raw record

The action, pair well, field normalization, volume, timestep, geometry,
currents, and classifiers remained unchanged from FTD-0722/0723. The horizon
alone was preregistered at 48 forward and 48 reverse steps to complete the
slower encounters.

| `p` | initial pair energy | field-energy export | final pair energy | graph changes | raw negative | qualified capture |
|---:|---:|---:|---:|---:|---:|---:|
| 0.0060 | 0.000070443 | 0.000544442--0.000673112 | -0.000602669---0.000473999 | 1 | 52/52 | 0/52 |
| 0.0075 | 0.000110060 | 0.000577157--0.000684191 | -0.000574130---0.000467097 | 1 | 52/52 | 0/52 |
| 0.0085 | 0.000141360 | 0.000581173--0.000684361 | -0.000543001---0.000439813 | 1 | 52/52 | 0/52 |
| 0.0095 | 0.000176569 | 0.000568607--0.000682498 | -0.000505929---0.000392038 | 1 | 52/52 | 0/52 |
| 0.0120 | 0.000281684 | 0.000272287--0.000280273 | 0.000001411--0.000009397 | 2 | 0/52 | 0/52 |

```text
complete histories                         312 / 312
rowwise common-action identity arms        312 / 312
state-only inverse arms                    312 / 312
symmetric recoil arms                      312 / 312
raw negative-sector arms                   208 / 260
qualified captured arms                      0 / 260
already-bound controls retained             52 / 52
maximum common residual                    2.000e-11
maximum pair/field energy-balance residual 1.345e-11
maximum recoil defect                      1.164e-11
maximum 48-step inverse recovery           3.341e-9
translation/polarity scalar-history spread 1.068e-8  FAIL
```

## Why the locked prediction is not confirmed

Even without the covariance failure, the preregistered directional envelope
is not confirmed. It predicted positive escape in all `p=0.0095` arms, but the
raw record leaves all 52 negative. The apparent crossover is shifted upward,
between `0.0095` and `0.0120`, rather than lying in the extrapolated
`0.00776--0.00919` interval.

The 48-tick export is also much larger than the FTD-0723 linear continuation
for the four raw trapped families. The narrow 24-tick approximately linear fit
therefore is not a long-horizon energy-loss law. Interaction duration and
field feedback matter.

No arm qualifies detached-field capture. Raw trapped families have
dynamic-field median doubled radius three, below the locked threshold four.
The escaping `p=0.0120` family has radius five. Thus the run never combines a
negative pair basin with a separated receiver field. The raw trapping could be
near-field energy storage followed by a later return; 48 ticks do not prove a
stable formed object.

## Localized defect and next gate

Polarity mirrors agree in the emitted final records. Translation-normalized
final pair and field energies differ only at about `1e-12`, while the maximum
intermediate scalar-history spread reaches `1.07e-8`. This is consistent with
accumulated root-conditioning or branch sensitivity, but the run record does
not identify which history coordinate and tick sets the maximum.

The next admissible step is a fresh conditioning diagnostic, not threshold
refinement. It must record the worst translated scalar and complete-state
defect by tick while tightening the numerical root tolerance under the same
mathematical action. If the defect converges below `1e-9` without changing the
energy-sign classification, a separately locked full rerun may test the
apparent crossover. If it does not converge, translation covariance or root
uniqueness fails in this long-interaction sector.

## Ontological consequence

FTD-0724 does not price a new primitive. The current variables execute and
invert every individual encounter and exhibit a coherent raw energy basin.
But it also does not establish formation. The candidate now has two separate
obligations:

1. establish translation-covariant selection of the same long-interaction
   history; and
2. distinguish a stable localized matter-plus-near-field state from temporary
   trapping that later re-emits its energy.

This sharpens matter-as-pattern: negative pair energy alone is insufficient.
A formed object must be a covariantly selected, persistent complete-state
orbit, including its field dressing.

## Scope

The raw `208/260` negative-sector pattern is diagnostic only. It is not a
validated capture window, physical particle, radiation process, quantum bound
state, or production law. The locked FTD-0724 status remains unresolved until
the failed `1e-9` covariance gate is addressed in a new protocol.
