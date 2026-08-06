# FTD-0729 — Late-reentry covariance convergence v1

**Status:** `[SELECTED NUMERICAL REALIZATION + MEASURED — TARGETED ROOT
CONDITIONING CONFIRMED]`  
**Verdict:** `LATE_REENTRY_ROOT_CONDITIONING_CONFIRMED`  
**Production status:** unchanged

## Result

The exact FTD-0728 worst history was replayed in translated pairs for both
polarity orders at three root tolerances. The middle condition reproduces the
parent scalar maximum exactly. One further tolerance decade collapses scalar
and complete-state covariance by more than seventy-fivefold:

```text
condition       scalar maximum        complete-state maximum
2e-12           4.5307579910e-10       2.2210677741e-10
2e-13           5.6798055148e-10       2.2662695158e-10
2e-14           6.2501115394e-12       2.9611868513e-12

ultra/tight scalar ratio               0.0110041
ultra/tight complete ratio             0.0130663
```

Electric, magnetic, and matter components all converge. The ultra worst scalar
remains separation at tick 92; the complete-state maximum is matter phase
space at tick 92. All four histories retain three graph transitions and a
positive final-eight-tick class under every condition.

## Interpretation and scope

The localized late-reentry covariance defect is numerical root conditioning,
not a persistent translated-state discrepancy. Its nonmonotonic parent-to-
tight step shows that a single tolerance ratio is not a reliable asymptotic
error estimate near this root, but the additional decade supplies decisive
componentwise convergence below `1e-11`.

This does not retroactively satisfy FTD-0728's failed full-matrix fivefold
gate. It licenses the already planned volume discriminator using the `2e-14`
selected numerical realization. No state-completeness defect or new primitive
is exposed.

## Verification anchors

- protocol `96751A97…D384`;
- runner `98322821…6A16`;
- JSON `C9EF34EB…D6E9`;
- CSV `204724D2…2F3D`;
- independent certificate `E2DDCB97…721C`, `70/70 PASS`;
- focused CTest `1/1 PASS` in `28.56 s`.

