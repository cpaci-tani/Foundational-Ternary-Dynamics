# FTD-0762 — M4 moving-dressing observer forensics result v1

**Status:** `[CERTIFIED NUMERICAL FACT — INTEGER-CENTER OBSERVER CHART OBSTRUCTION; PHYSICAL DRESSING UNTESTED]`  
**Date:** 2026-07-31  
**Protocol SHA-256:** `880293A2DC1F129637D1D1C28D8C0D9AE5FA3AC29D76042348CFE09ABB9E5B46`

## Result

The frozen WSL2 CUDA face, edge, and body replays each reached the first
FTD-0761 failed checkpoint at tick 224. The independent certificate passes:

```text
FTD-0762 artifact certificate: 179/179 checks
verdict=OBSERVER_INTEGER_CENTER_CHART_OBSTRUCTION
```

All three evolved states remain valid common-action histories. Their centroids
are fractional:

| ray | centroid | distance to integer chart |
|---|---|---:|
| face | `(160,160,160.4042465806225)` | `0.40424658062249819` |
| edge | `(160,160.2828803744593,159.71711962554053)` | `0.40005326208963671` |
| body | `(160.23998910423907,160.23998910423902,160.23998910423887)` | `0.41567332180486855` |

For every ray, CPU and CUDA observer and support-ladder calls return invalid.
The CUDA errors are exactly `compact observer preparation failed` and
`compact bound preparation failed`. A fresh finite-support preparation using
the unchanged evolved constituent geometry also returns invalid before any
field comparison is made.

The diagnostic rigid recenter changes only the common centroid. Relative
geometry and both momenta are preserved with residual exactly zero. The
recentered finite-support preparation passes on every ray, as do the CUDA
matter/field observer, boundary-energy ledger, and `{4,6,8}` support ladder.
The largest ladder projection residual is
`2.2064869462795622e-16`; every registered `1e-12` residual gate passes.

## Cause

`prepare_finite_support_derived_compact_pair` rejects any centroid more than
`1e-12` from its nearest integer site. The FTD-0754 observer and ladder call
that preparation before examining the evolved dressing. FTD-0761 therefore
fed every genuinely translated checkpoint outside the observer's declared
domain. This repeats the continuous-midpoint/integer-chart defect already
localized by FTD-0756/0757.

## Correction to FTD-0761

FTD-0761's frozen artifact and certificate remain historical facts, but its
`M4_BOOSTED_RELATIONAL_COHERENCE_CLOSED_AT_REGISTERED_SCALE` physical
interpretation is not valid. The 0/24 observer and 0/24 ladder bits do not
measure field incoherence; they record an inapplicable observer chart.

FTD-0761 still does **not** certify complete mobile matter. Its moving cores
are not field-momentum balanced, and no observer defined on fractional centers
has yet classified their evolved dressing. The corrected status is:

`[MOVING-CORE WITNESS; COMPLETE DRESSING AND MOMENTUM BALANCE OPEN]`.

The matter-only boost is therefore unqualified, not physically falsified by
the FTD-0761 observer bits.

## Frozen identities

| item | SHA-256 |
|---|---|
| protocol | `880293A2DC1F129637D1D1C28D8C0D9AE5FA3AC29D76042348CFE09ABB9E5B46` |
| CUDA runner | `77FEDD160A6FBC12DD72EBA2DD025B1837146937D7A5CD45CEF250DC84778004` |
| WSL2 executable | `3BBED134F84664F81880D8D99E28F8FEB541B6FD7D4A07301912415891C228D2` |
| certificate | `06871BDB07970B5289A92629EDE13CB09C1FFF6A77BDF62AAE9C875E3C8C0810` |
| face JSON | `B803F7152E7FD76C612B739640CF8593E09A9C9BE66BBF6BE025C22ED494B5F4` |
| edge JSON | `A376C7991A8DFB40075FC029831B26018E7D401860A84BA5885EB7CA4EA6F295` |
| body JSON | `02FD4577A70F000B075E41DB77B0A6FF9C0FB6535601B40F8EFD96B7DCAE2A34` |
| aggregate JSON | `279A1567C7973E10B58864EDEA8529FB20DFF83A03018CA41E8961788BFBFF83` |

The ignored run-of-record directory is `engine/results/ftd_0762/`.

## Next gate

Define one state-only finite-support observer on arbitrary fractional
centroids, with integer translation and cubic covariance tests, then replay
the untouched FTD-0761 evolved field. Only that observer can decide whether
the field dressing co-moves. If it exposes a physical mismatch, construct a
joint matter/field boost from the common action. Momentum balance remains an
independent mandatory gate in either case.

Production dynamics, defaults, ontology primitives, toggles, scenarios, and
`RenderBridge` were unchanged.

