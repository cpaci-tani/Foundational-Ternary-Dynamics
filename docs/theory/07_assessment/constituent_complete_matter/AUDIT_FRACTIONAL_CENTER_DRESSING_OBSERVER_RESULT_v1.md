# FTD-0763 — Fractional-center dressing observer result v1

**Status:** `[CERTIFIED CONSTRUCTIVE OBSERVER EXTENSION; CO-MOVING DRESSING AND TOTAL MOMENTUM OPEN]`  
**Date:** 2026-07-31  
**Protocol SHA-256:** `FB78C2688A90E18D01071DA390BFE230FFD76CF340FD2CB56AD6D545CDD8C63A`

## Verdict

The WSL2 RTX 5090 qualification and all three registered `L=321` replays pass.
The independent frozen-artifact certificate reports:

```text
FTD-0763 artifact certificate: 175/175 checks
verdict=FRACTIONAL_CENTER_OBSERVER_CONSTRUCTED
co_moving_dressing_claimed=false
```

The result constructs the missing state-only observer chart. It does not
construct new dynamics and does not establish that the observed field is a
co-moving matter dressing.

## Qualification

The CUDA qualification covers `L=17,33`, both polarities, all six face
directions, one edge direction, and one body direction. It uses two fractional
offsets, a nonzero Gauss-free electric challenge, and a nonzero magnetic
challenge. The legacy mode rejects every genuinely fractional center. The
enabled CPU and CUDA observers and nested support ladders agree within
`1e-12`; integer translation, proper cubic rotation, and polarity conjugation
agree within `1e-11`. The legacy integer-center CUDA observer test also passes.

The measured one-sided selected-representative energy difference across the
`x=+0.5` chart seam is `6.106226635438361e-16`. This is a numerical diagnostic,
not a theorem of continuous fractional covariance.

## Untouched CUDA replay

Each ray reconstructs the frozen FTD-0761 parent through tick 160, applies the
same `q=0.015` boost, and executes 64 common-action transactions to tick 224.
No field is recentered, regenerated, or corrected. The observer consumes the
resident CUDA field and downloads only reduction scalars.

| ray | fractional center norm | displacement | matter-momentum defect | Gauss residual | energy-partition residual | boundary-identity residual |
|---|---:|---:|---:|---:|---:|---:|
| face | `0.4042465806224982` | `0.4042465806222424` | `0.007212475852178761` | `7.129930326699174e-14` | `3.469446951953614e-18` | `-3.0856393828937456e-16` |
| edge | `0.4000532620896367` | `0.4000532620896367` | `0.008833581676187037` | `1.0030691555140692e-13` | `0` | `-2.736526283353413e-16` |
| body | `0.41567332180486855` | `0.4156733218048521` | `0.006930753392601568` | `7.606458865550003e-14` | `1.734723475976807e-18` | `-7.001777629911388e-16` |

Every tick-160 and tick-224 observer, boundary ledger, and `{4,6,8}` ladder
passes. The tick-224 fractional norms reproduce FTD-0762's rejected centers
within `1e-12`, proving that the newly accepted calls inspect the same physical
histories rather than recentered controls.

The observer decompositions change during transport:

| ray | residual energy, tick 160 | residual energy, tick 224 | outgoing energy, tick 160 | outgoing energy, tick 224 |
|---|---:|---:|---:|---:|
| face | `0.045256657056961434` | `0.05310964057319937` | `0.03314026798310653` | `0.04125804650744244` |
| edge | `0.03653928976900163` | `0.04434957759622917` | `0.025842487949069085` | `0.032840936939340766` |
| body | `0.02770813578148054` | `0.03567747149665939` | `0.02170520464934039` | `0.028963548724267783` |

Those increases are not evidence for or against co-motion by themselves. They
mix translation, transient radiation, support-chart readout, and field recoil.
A time-relative morphology test is still required.

## What is closed and what remains open

Closed constructively:

- a finite-support Gauss observer can be evaluated at an arbitrary
  non-seam fractional matter center using no new ontic primitive;
- the CUDA implementation agrees with the CPU reference and preserves the
  historical integer-center default;
- FTD-0761's observer failure is fully repaired at its first failed checkpoint.

Still open:

- whether the near-field residual remains localized in a chart transported
  with the matter center;
- whether a detached outgoing component is radiation rather than ordinary
  dispersive background;
- whether matter plus field momentum balances. The recorded matter-only
  defects are `0.00693--0.00883`; no total field-momentum observable was
  included, so no momentum verdict is licensed;
- whether the same classification persists across later FTD-0761 checkpoints,
  boost amplitudes, support scales, and volumes.

The next registered test must compare successive fields in the transported
fractional chart and simultaneously measure the volume-integrated field
momentum/stress flux. Instantaneous observer validity is no longer a live gap.

## Frozen identities

| item | SHA-256 |
|---|---|
| derivation | `39D7B94ADACF168C248B2187181B16BE3D7022E01E2E1EF4F17276556388A22C` |
| protocol | `FB78C2688A90E18D01071DA390BFE230FFD76CF340FD2CB56AD6D545CDD8C63A` |
| CUDA qualification source | `42F5A573FF5AE23FA23BE87BA5BB994824F654EF16B6355136C21164F6EA2EE9` |
| CUDA runner source | `1253E17F4904FC540735C31D28E83C7027466726E4AB0A31DB69DA13D633ECBB` |
| WSL2 executable | `3DFBC0454214231F7F979569697EA8C6DFCAE3EB6B0687E46ADA825EA0B808FE` |
| certificate | `56958FCDAD249C967AF1C72209252BC8A61BA071D9C980C6D5119B00AA7C2421` |
| face JSON | `E66D8C01C6AAA73AB89EBFB2D6741F773A7DA7FC47D19345CE7C0588C316D9A8` |
| edge JSON | `83D38D419A9AB81BBE1200C7172DAE99DD9C137529B867C72BC07BEA08D4F44B` |
| body JSON | `3D446549387D98506FB1592A343E95E69F106671E60449A6DFFA8E04C19F79A5` |
| aggregate JSON | `58F9C85E7DAAFA4CFC738CB22CD1E64EA5509403A8D7FC9229928DE65B112BEA` |

The run-of-record directory is `engine/results/ftd_0763/`.

Production dynamics, defaults, ontology primitives, toggles, scenarios, and
`RenderBridge` remain unchanged.
