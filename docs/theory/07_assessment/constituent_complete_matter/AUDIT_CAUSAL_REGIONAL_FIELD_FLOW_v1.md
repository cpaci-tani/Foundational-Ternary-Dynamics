# AUDIT — Causal regional field flow v1

**Date:** 2026-07-28  
**Identifier:** `FTD-0672`  
**Status:** `[SELECTED DYNAMICS — MIXED]`  
**Verdict:** `CAUSAL_REGIONAL_FIELD_FLOW_MIXED`  
**Pre-registration:**
[`PREREG_CAUSAL_REGIONAL_FIELD_FLOW_v1.md`](../../10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_REGIONAL_FIELD_FLOW_v1.md)  
**Analysis:**
[`ANALYSIS_CAUSAL_REGIONAL_FIELD_FLOW_v1.md`](../../10_eft_program/derivations/constituent_complete_matter/ANALYSIS_CAUSAL_REGIONAL_FIELD_FLOW_v1.md)

> **FTD-0675 correction:** the exact regional field ledger survives, but the
> paired-mode observable used to label ticks 68--80 a recovery window omitted
> the mass metric. The doublet-recovery and donor-reservoir interpretations
> are retracted.

## Result

The locked constructive transport classes do not close because the pre-tick-68
outward integral at radius 24 is `~0.00966`, below `0.05`. Near-bound dressing
also fails: the tick-80 positive-norm near fraction is `~0.235603`.

The exact mechanistic result is stronger than the mixed classifier:

- both signs reproduce the historical unweighted tick-72 diagnostic trough;
- no registered radius has any positive inward transport during ticks 68--80;
- the recovery-window current transfers `~0.04835635` into the dynamic field;
- `~0.04710315` crosses outward through radius 24 during the same interval;
- the ratio is `~0.974084`, leaving `~0.00125320` added inside radius 24.

Thus ticks 68--80 contain continued current-to-field exchange and outward
flow, with no inward return through radii 8, 16, or 24. FTD-0672 does not
establish canonical internal-doublet recovery, so it cannot infer a donor
reservoir.

## Qualification

```text
rows                              486
observer-valid rows               486
maximum regional update residual  4.7510395648848926e-17
maximum partition residual        1.5339030484030207e-22
maximum regional ledger residual  0
maximum complete-energy drift     1.0658141036401503e-14
maximum common-action residual    5.321853450206636e-13
maximum source support radius     4
state-only inverse recovery       5.5422333389287814e-13
```

No production state, force, tick, toggle, scenario, tolerance, or field
normalization changed.

## Reproducibility

- preregistration SHA256:
  `F0A2F895C07ADD99FC0BF4E39B95CD2FCEEE4BEBC10A4EE16CE4E47324B9C971`;
- runner SHA256:
  `77C4DBAB010A3ED599AAB0109E9D6AB4EAB590D63560A90900A8BC4446944843`;
- JSON SHA256:
  `E3EFB78EC36F32FEFE7627A3EE368E2A5A700BCE0890FBEF1E27D2D8E9B414D3`;
- CSV SHA256:
  `C4339D5985F4EB36DFE2F0DDF28A4151C805D4EC5913ED08EAF1A601F84F5C8E`;
- independent certificate SHA256:
  `90962E9897886A42D8B76218D56952A2261B76F747942ABF0006C73F4111F657`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.

The independent certificate validates all artifact hashes, recomputes the
trough classifier and every regional integral from the 486 CSV rows, confirms
zero recovery-window inward transport, and reproduces the mixed verdict.
