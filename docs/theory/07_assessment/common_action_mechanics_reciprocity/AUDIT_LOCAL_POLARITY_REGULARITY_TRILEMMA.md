# AUDIT — Local polarity regularity trilemma

**Date:** 2026-07-26  
**Identifier:** `FTD-0540`  
**Status:** `[THEOREM — REPRESENTATION NO-GO]` +
`[CONSTRUCTIVE — TWO ESCAPE WITNESSES]`  
**Verdict:** `LOCAL_POLARITY_REGULARITY_TRILEMMA_PROVED`  
**Derivation:**
[`THEOREM_LOCAL_POLARITY_REGULARITY_TRILEMMA.md`](../10_eft_program/derivations/THEOREM_LOCAL_POLARITY_REGULARITY_TRILEMMA.md)  
**Pre-registration:**
[`PREREG_LOCAL_POLARITY_REGULARITY_TRILEMMA_v1.md`](../10_eft_program/preregistrations/PREREG_LOCAL_POLARITY_REGULARITY_TRILEMMA_v1.md)  
**Run of record:** `engine/results/ftd_0540/windows_msvc_cpu.json`

## 1. Finding

The FTD-0478 nearest-cell shape is not one convenient interpolation among
many. Partition and first-moment reproduction uniquely force the one-
dimensional hat, and the declared multiaffine cube lift uniquely forces the
trilinear tensor basis. Its reflection-plane derivative jump is exactly two.

The stronger local theorem also closes the tempting repair “use a wider
positive smooth cardinal kernel.” Any locally finite, nonnegative, `C1`,
cardinal family has zero derivative in every off-center weight at an integer,
contradicting the unit derivative required by first-moment reproduction.

## 2. Priced exits

The executable polynomial witnesses close the two honest smooth exits:

```text
quadratic B-spline integer weights   (1/8, 3/4, 1/8)
quadratic cardinality defect         1/4
Catmull-Rom negative-lobe location   4/3
Catmull-Rom negative-lobe value     -2/27
```

Both witnesses preserve partition and first moment to machine zero in the
locked rational grid. The first keeps positivity and loses exact site-
cardinality. The second keeps cardinality and pays with signed lobes.

## 3. Scope

This result explains why the FTD-0539 edge cusp survives coefficient changes
inside the frozen FTD-0478 representation. It does not prove that either
smooth witness yields an exact-energy action, and it does not select between a
non-cardinal coat, signed coat, nonsmooth selector, or new primitive variable.
The frozen FTD-0536 action remains closed negative.

No production state, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 4. Reproducibility

- focused test: `local_polarity_regularity`, `7/7 PASS`;
- hat one-sided derivatives: `+1`, `-1`; jump `2`;
- worst tensor-basis residual: `0`;
- worst locked partition and first-moment witness residual: `0`;
- test SHA256:
  `2A4C0A60B6F5457170052C0EF697882A0A374A19A3E0AEFFAFDA98E929798E5D`;
- observer header/implementation SHA256:
  `7430D9092D510D867E2D4D492E49F365CC2DF0465F688C35243C46CD9B74B682` /
  `409FE0F0E8786CD1120C8C44D99950A4EB57FFD6D4EC5D96CFAB7117DA31456E`;
- locked preregistration SHA256:
  `55EEC9AE9B3C29407BA0F67205351DCCEBA51FFE7BB0F087BDFEC0D5BD11A0BF`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
