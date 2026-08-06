# FTD-0705 — Moving dressed-matter transverse-field growth v1

**Status:** `[SELECTED DYNAMICS — MIXED TRANSVERSE FIELD RESPONSE]`  
**Verdict:** `MOVING_DRESSED_MATTER_DYNAMIC_TRANSVERSE_NO_THRESHOLD_SEPARATION`  
**Production status:** unchanged

## Result

Three `L=65` complete dressed-matter histories ran for 24 forward and 24
reverse ticks at target speeds `0.35`, `0.45`, and `0.50`. All execution,
coherence, source-quality, observer, deposited-current coupling, collinear
control, energy, and inversion gates pass.

The registered response ratios are:

| channel | complex `R^2` | amplitude ratio | source-normalized response | below-threshold response | contrast |
|---|---:|---:|---:|---:|---:|
| `R45@v=0.45` | `0.0931` | `0.6699` | `0.42946` | `0.25204` | `1.704` |
| `R50@v=0.50` | `0.9859` | `2.5197` | `0.57599` | `0.23204` | `2.482` |

Collinear transverse field-slope controls normalized to the paired currents are
`5.93e-12` and `3.66e-14`; their deposited transverse-current fractions are
below `1e-20`. Thus the non-collinear transverse signal is real. Neither
channel reaches the locked `5x` below-threshold contrast. The `v=0.45`
registered channel additionally fails linearity and growth.

Magnetic norm outside radius 6 grows in every arm by tick 24: `0.399` at
`v=0.35`, `0.379` at `v=0.45`, and `0.301` at `v=0.50`. The below-threshold
arm has the largest far fraction, so this morphology cannot be identified as
threshold radiation; launch and ordinary driven transients remain sufficient.

## Ontological consequence

The selected moving relational object necessarily sources a dynamic
transverse field through the common action. The present evidence does not show
a sharp lattice-Cherenkov onset or a radiation-free dressed fast branch.
“Flux as a literal wake” remains unearned: the observed field contains a
co-moving/dynamic component plus a spreading launch response, with no locked
separation into dressing and detached radiation.

The next admissible discriminator must change the preparation, not retune this
run: prepare a self-consistent moving dressing or compare multiple launch
ramps/horizons so the finite launch transient can be separated from sustained
velocity-dependent emission.

## Record

- protocol SHA256 `A60CF2A5...18DE`;
- JSON SHA256 `7FA8699D...AD4`;
- arms/fits/ticks SHA256 `7553E2DF...CBEC`, `8EB2578A...99C`,
  `AE754364...920`;
- runner SHA256 `83504AC8...E67`;
- proof SHA256 `321FA74A...D9D`.

