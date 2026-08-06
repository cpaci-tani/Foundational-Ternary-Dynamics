# AUDIT — Tangent-mode mass-metric correction

**Date:** 2026-07-28  
**Identifier:** `FTD-0675`  
**Status:** `[THEOREM — DIAGNOSTIC CORRECTION]`  
**Verdict:** `LEGACY_MODAL_DISPLACEMENT_DIAGNOSTIC_RETRACTED`

## Defect

The legacy paired-mode observer used

```text
q_wrong = v^T delta x,
P       = v^T delta p
```

for eigenvectors normalized by `v^T M v=1`. The canonical coordinate is
`q=v^T M delta x`. Because the selected constituent mass is not one, the
legacy observer mixes two incompatible normalizations in the same quadratic
energy.

The exact theorem and rational certificate show that the potential part is
overweighted by `1/M_INERTIAL^2`. The executable harmonic witness keeps the
canonical energy constant while the legacy diagnostic oscillates by that
factor and therefore manufactures trough/recovery sequences.

## Empirical trigger

FTD-0674 was the first fresh campaign to apply the corrected FTD-0673 energy
coordinate. At maximum constituent momentum `1e-6`, its canonical target-mode
ratio declines from `0.6310796` at tick 72 to `0.6058984` at tick 78 and to
`0.6015652` at tick 80. The locked donor campaign is invalid because the
preregistered recovery never occurs. Both signs agree on the negative
tick-72--78 change within `2e-9`.

This does not establish monotone decay at every amplitude or infinite time.
It does establish that the old unweighted observable cannot support the
published recovery claim.

## Required status changes

- FTD-0670 is `[RETRACTED — MODAL MASS-METRIC ERROR]`.
- FTD-0672 remains `[SELECTED DYNAMICS — MIXED]` for its exact field-flow
  classifier, but its recovery/donor interpretation is retracted.
- FTD-0674 is `[EXECUTION INVALID — NO LOCKED RECOVERY]`; its donor class is
  not reportable.
- FTD-0664/0665/0668 modal-energy interpretations are under correction review.

No production state, tick, force, toggle, scenario, or field result changed.

## Qualification

```text
M_INERTIAL                       0.51100000000000001
canonical harmonic variation    8.1315162936412833e-20
legacy max/min ratio             3.8296421965295782
expected 1/M_INERTIAL^2          3.8296421965295782
CTest                            1/1 pass
exact-rational certificate       pass
```

- C++ witness SHA256:
  `F92E998448FD2309A584B44A0D52C737AAE0EB6BC665683E73DDAD3BF131A751`;
- rational certificate SHA256:
  `AAC7624A5BF474390B6637049F158413937DF1F6135C7C1764F4E3A3DC28B48D`;
- theorem SHA256:
  `299BFE84AD2075F13493E19B263FB437F5E9892367E9E383BED292F3DC0F26A9`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU.
