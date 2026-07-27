# AUDIT — Quadratic-coat discrete-gradient transaction

**Date:** 2026-07-26  
**Identifier:** `FTD-0551`  
**Status:** `[SELECTED DYNAMICS] + [THEOREM — CONDITIONAL EXACT ENERGY/GAUSS IDENTITIES] + [NUMERICAL FACT — NONLINEAR ROOT CAMPAIGN]`  
**Verdict:** `QUADRATIC_COAT_DG_TRANSACTION_CONSTRUCTIVE`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION_v1.md)  
**Derivation:**
[`DERIV_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION.md`](../10_eft_program/derivations/DERIV_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION.md)  
**Run of record:** `engine/results/ftd_0551/windows_msvc_cpu.json`

## Result

```text
registered arms                 72
converged roots                 72
nontrivially moved              72
worst identity residual         9.224148138017863e-15
worst solve residual            5.637851296924623e-18
worst continuity residual       1.075528555105620e-16
worst Gauss residual            4.440892098500626e-16
worst work residual             9.224148138017863e-15
worst total-energy residual     9.214851104388799e-15
worst inverse residual          2.220446049250313e-16
translation residual            0
rotation residual               1.355252715606881e-20
failures                        0
```

All endpoint roots converged without a correction term. The same quadratic
orbit produces the current, electric impulse, magnetic impulse, displacement,
and field work. Continuity, both Gauss endpoints, exact production-dispersion
work, total energy, causality, translation, cubic rotation, and recorded
inverse all pass the locked gates.

## Verdict and scope

The observer-level reciprocal transaction is constructive. This repairs both
representation defects explicitly recorded by FTD-0479 and closes the
one-step algebraic part of the face-flux mobile-matter plan.

It remains selected discrete-gradient dynamics. It is not the unique
fixed-time gauge action, does not reconstruct the FTD-0548 spacetime source
split, and has not passed multi-tick controls. No production toggle or
scenario is licensed yet.

## Reproducibility

- test: `test_quadratic_coat_discrete_gradient_transaction`, 72 arms;
- preregistration SHA256:
  `570B9FB5533203F3635BBB237A004DAF36D16A57DF555A80FEC16432F49D67BB`;
- header SHA256:
  `BCB070B318190D6AB17895C95003D2F1A53CD649184004ABA42254851E7C1871`;
- source SHA256:
  `03BC386F3AFC2819AFE2816A762029D56FFBCCB5C052AD7720749F37124BEE2F`;
- test SHA256:
  `C7FBC171D975ABFBBA75CB071A700A9DD702F580BE61593E76BC4F36688966DB`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.

No production state, force, phase, toggle, scenario, or default changed.
