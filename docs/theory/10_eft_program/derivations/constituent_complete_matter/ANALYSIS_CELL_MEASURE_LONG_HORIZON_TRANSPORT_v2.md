# Cell-measure long-horizon transport v2

**Campaign:** FTD-0652  
**Status:** `[SELECTED DYNAMICS — EXACT/COHERENT TRANSPORT, MIXED RESOLUTION TREND]`  
**Verdict:** `CELL_MEASURE_LONG_HORIZON_MIXED`  
**Production impact:** none

## Result

All 30 locked histories completed. Every action, graph, chart, strain,
causality, zero, sign-mirror, cubic, and state-only inverse gate passes.
Every high-speed canonical arm is persistent at widths two through four.

| exact/coherence diagnostic | result | gate |
|---|---:|---:|
| worst action residual | `1.99747e-11` | `1e-9` |
| worst state-only recovery | `2.02757e-10` | `1e-7` |
| worst relative bond strain | `0.00218681` | `0.10` |
| worst zero displacement | `9.30258e-14` | `1e-6` |
| sign-mirror residual | `3.72495e-11` | `1e-6` |
| cubic residual | `1.22902e-10` | `1e-6` |
| high-speed persistent arms | `9/9` | `9/9` |
| low-speed persistent arms | `6/9` | classifier only |

The resolution conjunction fails only its registered minimum-mobility trend:

| width | minimum high-speed mobility | direction span | maximum spline defect |
|---:|---:|---:|---:|
| 2 | `1.0322442343` | `0.0571992479` | `0.0701017185` |
| 3 | `1.0063738410` | `0.0089735884` | `0.0548213497` |
| 4 | `0.9948037380` | `0.0040461332` | `0.0455103292` |

The locked gate required the minimum mobility to be nondecreasing. It instead
decreases, so the verdict is `MIXED`. Anisotropy and translation-defect trends
pass.

## What is now supported

Within the selected action, matter can be represented as an extended,
fixed-total-measure polarity configuration whose constituent, binding, and
face/edge field degrees of freedom translate together for a finite physical
horizon. This is stronger than a one-step or static result: the object moves,
remains coherent, exchanges energy reciprocally, and reconstructs its prior
state without a saved history or post-step dressing.

The low-speed classifier also improves with refinement. Coarse diagonal arms
fail only the transverse-drift threshold; all width-four low-speed arms are
persistent. This is measured evidence for smoother collective mobility, not a
proof of zero depinning threshold.

## Locked boundary and exploratory observation

The `MIXED` verdict cannot be relabelled. However, the failed gate is not a
valid general convergence criterion for a target mobility of one: a sequence
approaching one from above must decrease. Post hoc, the mean absolute
high-speed mobility error relative to one is

`0.0595380 -> 0.0101207 -> 0.00324519`,

and the maximum absolute error is

`0.0894435 -> 0.0153474 -> 0.00519626`.

These target-centred statistics were not registered and are exploratory only.
A fresh out-of-sample campaign must lock the correct target-centred criterion
before execution. Until then, FTD-0652 does not license an interacting pole or
an infrared particle claim.

## Solver record

The checkpointed cache path completed in about 68 minutes wall time. Across
all arms it used `36,322` exact residual evaluations, `60` Jacobian refreshes,
and `9,622` reuses. Summed concurrent solver time was `21,873.19` seconds.
Every arm produced an atomic JSON and full tick CSV checkpoint.

No native formation, reaction-complete charge, gauge redundancy, quantum
statistics, particle mass, Lorentz recovery, unitarity, or production adoption
follows.
