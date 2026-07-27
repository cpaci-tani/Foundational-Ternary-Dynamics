# AUDIT — Quadratic-coat self-force

**Date:** 2026-07-26  
**Identifier:** `FTD-0552`  
**Status:** `[DERIVED — SELF-ENERGY MECHANISM] + [NUMERICAL FACT — GENERIC SELF-FORCE] + [CLOSED NEGATIVE — UNSUBTRACTED ISOLATED MOBILE LAW]`  
**Verdict:** `UNSUBTRACTED_QUADRATIC_SELF_FORCE_PRESENT`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_SELF_FORCE_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_SELF_FORCE_v1.md)  
**Derivation:**
[`DERIV_QUADRATIC_COAT_SELF_FORCE.md`](../10_eft_program/derivations/DERIV_QUADRATIC_COAT_SELF_FORCE.md)  
**Run of record:** `engine/results/ftd_0552/windows_msvc_cpu.json`

## Result

```text
registered 64-tick arms              12
static arms                           8
worst Poisson residual                9.420849950346530e-14
worst curl-adjoint residual           1.040834085586084e-17
worst transaction identity residual  9.421802598633296e-14
worst accumulated energy residual     2.775557561562891e-17
largest displacement                  0.8464862214540756
largest momentum                      0.010170940522624974
polarity residual                     0
```

Both polarities at integer and half-cell symmetry positions remain static.
Both polarities at the generic registered subcell position move at `L=17`
and `L=33`. The effect is polarity-even, volume-stable in classification, and
occurs while every algebraic conservation gate remains closed.

## Verdict and consequence

The result is a conservative lattice self-force, not energy drift. The
unsubtracted quadratic transaction is closed negative as the isolated mobile
matter law demanded by FTD-0481. Under the frozen rules, the default-off
`common_action_face_dynamics` toggle, packet/hop promotion campaign, dressing
scenario, and infrared pole campaign are not licensed.

Self-field subtraction was explicitly prohibited by the preregistration and
is not added. A neutral composite branch could be researched separately, but
it would change the carrier being qualified from a bare polarity to a
resolved multi-polarity object.

No production state, force, phase, toggle, scenario, or default changed.

## Reproducibility

- test: `test_quadratic_coat_self_force`, 12 arms x 64 ticks;
- preregistration SHA256:
  `B428FE2D3895F468EDEC2A77B8DF6724202293357213543B7C2F45C19D759C93`;
- test SHA256:
  `7666DC3289C57A6A32ADD1EE4CFBA07D85B57DEA144BE7D59B811BA2BACED9C7`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
