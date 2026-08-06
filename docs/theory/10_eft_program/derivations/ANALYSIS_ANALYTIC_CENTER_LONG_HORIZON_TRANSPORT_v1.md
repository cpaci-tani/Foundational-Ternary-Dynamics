# Analytic-center long-horizon transport

**Campaign:** FTD-0646  
**Status:** `[MEASURED — COHERENT LONG-HORIZON DYNAMICS; MIXED TRANSPORT /
PINNED COLLECTIVE OSCILLATION]`  
**Production impact:** none

## Result

All 23 arms execute, remain coherent, preserve the selected field-coupled
object, and invert after 256 forward ticks. The verdict is
`ANALYTIC_CENTER_LONG_HORIZON_MIXED`.

Only two of 12 canonical arms pass the locked secular-transport discriminator:

- `<100>, p=0.015`: displacement `6.4410`, mobility `0.8582`,
  `R^2=0.999883`;
- `<110>, p=0.015`: displacement `5.6272`, mobility `0.7498`,
  `R^2=0.998962`.

Four arms are explicitly classified bounded reversal. The remainder are mixed
or stalled. In particular, the `p=0.001875` centers end with negative projected
displacements in all three direction families after moving forward during the
FTD-0645 16-tick window. `<111>, p=0.015` reaches displacement `3.7014` but
misses the locked gates with mobility `0.4932` and `R^2=0.976895`.

| diagnostic | worst value | gate |
|---|---:|---:|
| common-action residual | `1.9998e-11` | `1e-10` |
| energy drift | `1.5530e-12` | `1e-9` |
| inverse recovery | `3.1315e-9` | `1e-8` |
| shape RMS | `5.2866e-3` | `5e-2` |
| edge strain | `2.4395e-3` | `5e-2` |
| dressing residual | `4.0740e-2` | `5e-1` |
| mirror residual | `2.2179e-9` | `1e-6` |
| cubic residual | `1.8093e-9` | `1e-6` |

All nonzero arms keep integrated soft fraction above `0.99999956`. Failure to
translate persistently is therefore not disintegration or conversion into
stiff internal modes. It is center-of-pattern motion in a periodic substrate
potential.

## Ontological consequence

The selected object behaves as a classical lattice quasiparticle:

- it has a localized, reversible internal identity;
- it has soft collective translations and rotations;
- it couples reciprocally to a propagating face/edge field;
- it can translate coherently above a direction-dependent finite excitation;
- below that excitation, the same soft coordinate undergoes bounded Peierls
  motion rather than free drift.

The early anchor crossings in FTD-0645 were real constituent
re-coordinatizations, but they did not prove secular center transport. A short
forward trajectory can therefore look mobile while belonging to a long-period
pinned orbit.

## Damage to a particle interpretation

For this fixed selected composite at `L=17`, low-momentum translation is not a
qualified gapless mode. The inferred registered onset lies between `0.0075`
and `0.015` for `<100>` and `<110>`; `<111>` remains unresolved at `0.015`.
The response is direction-dependent. This blocks an infrared free-particle
pole and Lorentz-like common cone for this fixed object unless a controlled
limit drives the Peierls scale and directional splitting to zero.

FTD-0621 supplies a separate extensive-width trend for exact block bipoles,
but its mass/content grow with width. It does not yet provide the required
fixed-mass limit.

## Next boundary

Do not extract a physical matter pole from the current fixed object. The next
research gate is a size/refinement family with an explicit mass and binding
normalization. It must show that the depinning momentum, directional splitting,
and translation reaction scale toward zero while rest energy and internal gap
remain finite. If no such scaling family exists, the selected composite is a
pinned lattice excitation rather than an infrared particle.

