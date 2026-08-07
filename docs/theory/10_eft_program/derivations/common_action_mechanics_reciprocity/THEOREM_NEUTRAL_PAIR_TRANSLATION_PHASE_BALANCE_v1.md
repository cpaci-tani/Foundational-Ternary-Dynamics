# FTD-0603 — Neutral-pair translation-phase balance v1

**Status:** `[MEASURED — COMPACT-CARRIER FORCE SIGN IS PHASE DEPENDENT] +
[NUMERICAL FACT — MOMENTUM PHASE MEAN UNRESOLVED] + [THEOREM — EXACT INTEGER
TRANSLATION COVARIANCE]`  
**Protocol:**
[`PREREG_NEUTRAL_PAIR_TRANSLATION_PHASE_BALANCE_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_NEUTRAL_PAIR_TRANSLATION_PHASE_BALANCE_v1.md),
SHA-256 `9C88B2B593C2E31EA08999010E71EF85204ECB3F8C63AA248B7A86A937E16595`  
**Verdict:** `TRANSLATION_PHASE_ATTRACTION_NOT_ROBUST`

## Campaign

The FTD-0602 pair was rigidly shifted through one full subcell period along
each axis at `N=8,16,32`. Every one of the 168 placements received a fresh
minimum-energy Gauss solve and one unchanged common-action step. All
initializer and common-action gates pass; exact one-site translation closes to
`1.78e-15`.

At `N=32`, force-sign counts are:

| shift axis | attractive phases | non-attractive phases | inward range |
|---|---:|---:|---:|
| `x` (principal separation direction) | 18 | 14 | `-4.159e-4 .. +3.289e-4` |
| `y` | 32 | 0 | `+3.274e-4 .. +5.470e-4` |
| `z` | 32 | 0 | `+1.150e-4 .. +3.289e-4` |

The worst outward response occurs near `x` phase `0.46875`; separation grows
by `2.713e-4` in that step. Consequently the force between these compact
selected composites is not independent of absolute subcell placement.

## Momentum phase mean

The maximum axis mean of both matter impulse and the registered total
pseudomomentum defect is

| phase resolution | maximum absolute mean |
|---:|---:|
| 8 | `3.124e-6` |
| 16 | `2.471e-7` |
| 32 | `2.574e-8` |

The trend is toward zero but misses the locked `1e-8` absolute gate. It is
therefore `UNRESOLVED`, not evidence for either a secular defect or an exact
momentum theorem.

## Consequence

FTD-0553 already proves that every nonzero localized rigid quadratic-coat
source has a positive Peierls coefficient on at least one axis. FTD-0603 is
the direct dynamical manifestation for the selected six-constituent pair.
Adding a connection variable would not by itself cure this compact-carrier
phase dependence. The live constructive branches are a dynamically deforming
constituent/dressing core, or a native smooth low-momentum extended carrier
whose Peierls index is suppressed as in FTD-0555. Neither is yet constructed.

