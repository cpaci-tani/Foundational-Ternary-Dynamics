# ANALYSIS — Causal regional field flow v1

**Campaign:** `FTD-0672`  
**Status:** `[SELECTED DYNAMICS — MIXED LOCKED CLASSIFICATION]`  
**Mechanistic result:** `[EXACT OBSERVER — NO INWARD FLOW AT TICKS 68--80]`  
**Verdict:** `CAUSAL_REGIONAL_FIELD_FLOW_MIXED`

> **FTD-0675 correction:** the paired-mode displacement diagnostic used to
> call ticks 68--80 a “recovery window” omitted the inertial-mass metric. The
> canonical target energy does not support that recovery claim. Every exact
> regional field-transport and current-work number below remains valid, but
> the claim that another matter reservoir simultaneously funds a doublet rise
> is retracted. “Recovery window” below is only the historical preregistered
> name for the fixed ticks 68--80.

## 1. Question

Does field energy return inward during the historically registered tick-68--80
window, or does the dynamic field continue moving outward?

FTD-0672 applies the exact FTD-0671 regional ledger to fresh `2e-6` sign-paired
data on `L=97`, through tick 80, before conservative source self-contact at
tick 81. The fixed component-aware radii are `8`, `16`, and `24`.

## 2. Exact execution

All execution gates pass:

```text
initial excited/control fields       bitwise equal
maximum source support radius        4
maximum complete-energy drift        1.0658141036401503e-14
maximum common-action residual       5.321853450206636e-13
maximum regional update residual     4.7510395648848926e-17
maximum partition residual           1.5339030484030207e-22
maximum regional ledger residual     0
state-only inverse recovery           5.5422333389287814e-13
polarity control                      pass
```

The legacy unweighted diagnostic gives a primary trough at tick 72 and an
apparent second-post-trough rise of `~0.0796856`. FTD-0675 retracts its
interpretation as canonical mode-energy recovery.

## 3. Locked regional result

Normalized cumulative values are:

| sign | quantity | R=8 | R=16 | R=24 |
|---:|---|---:|---:|---:|
| - | outward, ticks 1--67 | 0.2463537332 | 0.1137430378 | 0.0096636115 |
| + | outward, ticks 1--67 | 0.2463537336 | 0.1137430382 | 0.0096636117 |
| - | inward, ticks 68--80 | 0 | 0 | 0 |
| + | inward, ticks 68--80 | 0 | 0 | 0 |
| - | net outward, ticks 1--80 | 0.3060860213 | 0.1907281793 | 0.0567667600 |
| + | net outward, ticks 1--80 | 0.3060860201 | 0.1907281800 | 0.0567667595 |
| - | current-to-field, ticks 68--80 | 0.0483563490 | 0.0483563490 | 0.0483563490 |
| + | current-to-field, ticks 68--80 | 0.0483563464 | 0.0483563464 | 0.0483563464 |

The radius-24 outward integral before tick 68 is only `~0.00966`, below the
locked `0.05` prerequisite for both bidirectional and one-pass constructive
classes. The tick-80 positive-norm near fraction is `~0.235603`, below the
`0.50` near-bound gate. The preregistered transport class is therefore
`REGIONAL_FLOW_MIXED`.

This mixed label cannot be repaired on the same data. It records that the
fixed pre-68 timing gate was not crossed at the outermost radius.

## 4. What the exact ledger nevertheless determines

There is no positive inward transport step at any registered radius during
ticks 68--80. Over the same interval the deposited difference current gives
energy to the dynamic field, not the reverse. At radius 24,

```text
late outward transport = net outward - pre-68 outward
                        = 0.04710314845  (- sign)
                        = 0.04710314781  (+ sign),

current-to-field exchange = 0.04835634903  (- sign)
                          = 0.04835634638  (+ sign).
```

Hence

```text
late outward / current-to-field = 0.97408405...,
Delta U_inside(R=24)             = 0.00125320...
```

by the exact regional identity. During ticks 68--80, approximately `97.4%` of
the current-to-field exchange is matched by outward transport through radius
24, while the dynamic field retained inside radius 24 grows only slightly.

There is no field energy returning through the registered boundaries and no
negative field-to-current work during this interval. FTD-0672 does not
establish a simultaneous canonical doublet increase and therefore does not
identify or require a donor reservoir for such an increase.

## 5. Ontological consequence

The tested source is not surrounded by a field that leaves and then returns
through the registered boundaries over ticks 68--80. The exact surviving
account is:

```text
selected matter/current  --> dynamic matched field --> outward flow
```

This resembles an internally exchanging localized source coupled to an
outgoing field channel. It is not yet legitimate to call that channel
radiation: no asymptotic, positive-flux, pole-residue, or open-boundary limit
has closed. It is also not a pure near-bound dressing in this run: only
`~23.6%` of positive difference-field norm remains inside radius eight at
tick 80, and net outward energy has crossed every registered boundary.

## 6. Corrected next discriminator

FTD-0673 supplies the exact complete-reservoir decomposition and FTD-0674
finds no canonical recovery at its fresh amplitude. The next gate is therefore
not donor identification. It is a canonical-mode replay over a longer
causal/open-boundary window, followed by pole/width extraction. Only then
should the program choose between:

- a metastable emitting resonance and a genuinely bound hybrid pole;
- two-object separability/scattering tests; or
- an additional internal phase/connectivity type.

No production change, toggle, scenario, particle label, or ontology promotion
follows from this campaign.
