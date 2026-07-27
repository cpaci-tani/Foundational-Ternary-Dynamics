# AUDIT — Accelerated-worldline energy escape

**Date:** 2026-07-26  
**Identifier:** `FTD-0547`  
**Status:** `[DERIVED — EXACT UNIFORM-FORCE SUBSECTOR]`  
**Verdict:** `UNIFORM_ACCELERATED_WORLDLINE_REPAIRS_WORK_EXACTLY`  
**Pre-registration:**
[`PREREG_ACCELERATED_WORLDLINE_ENERGY_ESCAPE_v1.md`](../10_eft_program/preregistrations/PREREG_ACCELERATED_WORLDLINE_ENERGY_ESCAPE_v1.md)  
**Derivation:**
[`DERIV_ACCELERATED_WORLDLINE_ENERGY_ESCAPE.md`](../10_eft_program/derivations/DERIV_ACCELERATED_WORLDLINE_ENERGY_ESCAPE.md)  
**Run of record:** `engine/results/ftd_0547/windows_msvc_cpu.json`

## Result

```text
registered arms                    144
worst exact-work residual          2.7105054312137611e-20
worst defect-identity residual     1.2468324983583301e-18
worst endpoint residual            1.5959455978986625e-15
worst derivative residual          1.3877787807814457e-17
worst causal excess                0
worst reversal residual            1.3877787807814457e-17
worst cubic residual               0
largest old midpoint defect        4.1017724139693484e-05
largest schedule deviation         0.0055368684341502161
failures                           0
```

The production dispersion and the same total impulse are compatible with
exact work when the within-tick worldline is integrated under the force. The
previous nonzero matter-work defect is exactly the difference between a
relativistic energy secant and the frozen midpoint tangent.

## Scope

This reopens only an accelerated within-tick research branch. It does not
reverse FTD-0546's verdict on the frozen linear-time quadratic-coat step, and
does not establish a self-consistent field force history. No production
state, force, phase, toggle, scenario, or default changed.

## Reproducibility

- test: `test_accelerated_worldline_energy`, 144 arms, failures `0`;
- preregistration SHA256:
  `2F9550EC2D30440D21E872D9BC9A83C1FAD5FC76A16D78615760372D855B99C0`;
- header SHA256:
  `EDE784E8F20A82FFE8E7597523AE48960A865C4623834D3BA47D050F25528C54`;
- source SHA256:
  `A07713EBCC05B9E8CB34A2BA15D599252C40945AFD7AFD8E1C3C4A99F3CC5FC0`;
- test SHA256:
  `46673DDEFAE379BFA6F8C77788D439ACB07098A96200C76E8D89DB0E3B0C678B`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
