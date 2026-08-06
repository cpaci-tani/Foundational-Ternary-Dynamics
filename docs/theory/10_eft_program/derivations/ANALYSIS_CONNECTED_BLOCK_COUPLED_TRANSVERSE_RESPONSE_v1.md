# Connected-block coupled transverse response

**Campaign:** FTD-0642  
**Status:** `[DERIVED — COMMON-ACTION COUPLING] + [MEASURED — REVERSIBLE
WEAK-HYBRID CLASSICAL RESPONSE]`  
**Production impact:** none

## Question

FTD-0640 qualified the 48 matter-coordinate modes while FTD-0641 qualified
the transverse face/edge field with matter fixed. FTD-0642 releases the matter
coordinates under the same registered transverse perturbations. The question
is whether the two sectors form one reversible tangent dynamics rather than
two unrelated oscillators.

## Locked result

All 18 arms pass. They cover the three canonical wavevector families
`<100>`, `<110>`, and `<111>`, two transverse polarizations, and full, half,
and sign-mirrored amplitudes. Every arm completes 256 full common-action
forward ticks and 256 state-only reverse ticks.

The verdict is
`CONNECTED_BLOCK_COUPLED_TRANSVERSE_WEAK_HYBRID_CONSTRUCTIVE`.

| diagnostic | worst value | gate |
|---|---:|---:|
| relative coupled/bare phase shift | `5.8011e-4` | `5e-2` |
| field waveform distortion | `1.8409e-2` | `2.5e-1` |
| field leakage | `3.7645e-2` | `2.5e-1` |
| full/half scaling residual | `1.1136e-7` | locked ratio window |
| sign-mirror residual | `7.7745e-7` | `1e-1` |
| common-action residual | `4.9078e-14` | `1e-10` |
| energy drift | `3.1087e-15` | `1e-12` |
| inverse recovery | `1.2105e-13` | `1e-10` |

The full-amplitude matter response ranges from `6.2643e-8` to
`1.2747e-7`, above the locked `1e-9` detection floor in every direction and
polarization. Halving the field amplitude halves this response, and reversing
the field reverses the coupled matter and field histories.

## What is derived

The matter current, field update, constituent impulse, and energy exchange are
all evaluated by the same selected common action. No legacy force, Poisson
Coulomb solve, `grad|J|` force, or post-hoc recoil is active. Consequently the
nonzero matter response is reciprocal classical coupling inside the selected
constituent ontology, rather than a visualization response or a force added
after field evolution.

## Post-result soft-subspace observation

An exploratory projection of the six full-amplitude histories finds
`0.9998313345` of total matter-coordinate power in modes `0..5`, the six soft
lattice-dressed collective modes identified after FTD-0640. Only
`1.686655e-4` lies in modes `6..47`. This statistic was not a registered gate.
It suggests that long-wavelength transverse forcing predominantly moves or
reorients the composite instead of deforming its stiff internal structure.
The individual coordinates inside degenerate soft eigenspaces are
basis-dependent; only the six-dimensional subspace statement is retained.

## Ontological consequence

For small perturbations near the exact center, the selected model now has one
reversible classical matter-field system:

- a localized constituent pattern supplies internal and collective matter
  coordinates;
- oriented face electric flux and edge magnetic field propagate independently;
- the same common action transfers energy between the propagating field and
  the localized pattern;
- the response is linear, polarity/sign sensitive, bounded, and invertible in
  the registered tangent window.

This is evidence for matter as a field-coupled dynamical pattern. It is not a
particle pole, photon, charge quantization theorem, radiation process, freely
moving object, common infrared cone, or Lorentz result.

## Next boundary

The next campaign must apply a finite collective boost to the exact-center
state. It must distinguish coherent translation from Peierls depinning,
internal heating, coat shedding, and chart aliasing. The decisive observables
are center transport, soft/deformation power, co-moving dressing, exact energy,
state-only inversion, direction covariance, and the velocity threshold.

