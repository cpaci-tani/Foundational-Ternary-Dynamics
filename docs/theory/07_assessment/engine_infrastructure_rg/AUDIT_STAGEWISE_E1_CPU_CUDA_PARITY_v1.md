# FTD-0751 — Stagewise E1 CPU/CUDA parity audit v1

**Status:** `[NUMERICAL FACT — BACKEND DIVERGENCE LOCALIZED]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_STAGEWISE_E1_CPU_CUDA_PARITY_v1.md`  
**Certificate:** `scripts/proofs/proof_stagewise_e1_cpu_cuda_parity.py`

## Verdict

The six-arm bounded instrument executed and independently certified. Every arm
has the same earliest classification:

> `SOURCE_FREE_ELECTRIC_PREPARE_DIVERGENCE`

The selected E1 matter root, ordered current deposition, state transfer, and
diagnostic observer are not the first source of the CPU/CUDA split. The first
split occurs in the source-free field map

\[
E^*=E_0+\lambda\,\mathrm{curl}(B_1)
\]

before the matter root is evaluated.

This is a backend arithmetic result. It is neither constructive nor negative
evidence for a physical matter object.

## Registered matrix

All `L={33,65}` face, edge, and body arms executed ticks `1..8` and wrote 64
ordered stage rows. Tick-one initial electric and magnetic fields and the
magnetic prepare stage are bit-identical in all arms. The tick-one electric
prepare differs in every arm:

| Arm | first location | unequal scalars | maximum absolute | maximum ULP |
|---|---:|---:|---:|---:|
| L33 face | `x:14705` | 15 | `4.336808689942018e-19` | 1 |
| L33 edge | `x:13608` | 12 | `4.336808689942018e-19` | 1 |
| L33 body | `x:21297` | 9 | `1.084202172485504e-19` | 1 |
| L65 face | `x:128866` | 15 | `2.168404344971009e-19` | 1 |
| L65 edge | `x:120408` | 12 | `4.336808689942018e-19` | 1 |
| L65 body | `y:124641` | 9 | `2.168404344971009e-19` | 1 |

The first discrepancy is therefore universally one ULP, independent of the
two tested volumes and three cubic direction classes.

## Root response

The implicit matter root remains bit-identical at tick one in every arm even
though the prepared electric volumes differ. Face and edge roots first respond
at tick two. Their largest root-stage component difference through tick eight
is `9.7044e-19` (L33 face), `1.7347e-17` (L33 edge), `9.9119e-22` (L65 face),
and `5.8980e-20` (L65 edge). Both body-diagonal roots remain bit-identical
through all eight ticks.

Thus the field-rounding discrepancy can reach the matter root, but it is not a
distinct root equation or deposition algorithm. Which compact-support entries
the local gather samples determines whether the discrepancy is dynamically
visible over this horizon.

## Arithmetic localization

The host C++ target is explicitly compiled with `-ffp-contract=off` in
`engine/CMakeLists.txt`. The CPU map therefore rounds the multiplication and
addition separately. The CUDA command contains no corresponding contraction
disable. PTX generated from the frozen CUDA source emits, in
`prepare_electric_kernel`, three instructions of the form

```text
fma.rn.f64 destination, lambda, curl_component, electric_before
```

for the source lines

```cpp
electric_before.{x,y,z}[i] + lambda * c{x,y,z}
```

The observed one-ULP first differences are exactly the signature permitted by
that fused-versus-unfused evaluation. The discrepancy is a finite-precision
implementation difference between two evaluations of the same real-valued
stencil, not evidence that CPU and CUDA implement different continuum
operators.

This audit does not claim that every later difference is caused only by one
instruction. It establishes that the first difference is generated there and
then propagated by the deterministic maps.

## Consequence for the matter program

The bounded backend question is understood well enough to stop spending the
main research budget on raw whole-trajectory CPU/GPU equality. One separately
preregistered arithmetic-equivalence patch may replace fused preparation with
explicit round-to-nearest multiply then add/subtract in this research pipeline
and rerun the classifier. That patch cannot change the real stencil, physics
thresholds, action, or ontology.

The physical mainline may now return to M2/M3 only after that short repair
qualification: test whether the frozen selected `(s,C,F)` candidate has an
uncontained metastable object basin and autonomous motion. Charge naming,
Lorentz recovery, pole extraction, and new constant formulae remain deferred.

## Reproducibility

- independent certificate: `109/109` checks pass;
- results: `engine/results/ftd_0751/`;
- six CSV and six JSON arm records plus frozen hash manifest;
- production tick, defaults, legacy force paths, and ontology unchanged.
