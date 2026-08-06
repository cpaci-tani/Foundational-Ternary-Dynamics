# FTD-0696 — Matched symmetry-ray spectrum observer v1

**Status:** `[THEOREM — QUALIFIED SELECTED OBSERVER]`  
**Production status:** unchanged

## Result

The carrier-aware direct Fourier observer registered in
`PREREG_MATCHED_SYMMETRY_RAY_SPECTRUM_OBSERVER_v1.md` passes every locked
qualification arm on `L=31`.

| diagnostic | worst value | gate |
|---|---:|---:|
| transverse-mode longitudinal fraction | `8.773814248583778e-32` | `1e-24` |
| longitudinal-mode transverse fraction | `1.836749187819971e-31` | `1e-24` |
| matched-curl longitudinal fraction | `1.681495730898508e-32` | `1e-24` |
| unoccupied-mode power fraction | `1.798027623892018e-28` | `1e-24` |
| cubic-copy power residual | `4.741399271152821e-13` | `1e-12` |

Zero and nonfinite controls, both transverse polarizations, sign reversal,
quadratic amplitude scaling, integer-translation phase, two-mode linearity,
and projection reconstruction all pass.

## What is now measurable

For any connected matter/control pair, the observer can measure the complex
difference-field coefficient at a registered wavevector using the actual face
and edge carrier positions. It separates lattice-longitudinal and
lattice-transverse morphology with
`khat_a=2 sin(k_a/2)` and preserves the phase needed for subsequent temporal
analysis.

This removes one ambiguity in FTD-0694: the next campaign can directly test
whether occupied symmetry-ray modes cluster around the internal-resonant
surface derived in FTD-0695.

## Boundary

The scalar `P_T=|E_T|^2+c^2|B_T|^2` is not the exact modified leapfrog energy,
and a finite set of symmetry rays is not a complete Brillouin-zone spectrum.
This result qualifies an instrument; it supplies no new matter, resonance,
photon, pole, or Lorentz claim by itself.
