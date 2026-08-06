# FTD-0752 — Explicit-rounding E1 CUDA parity qualification v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Date:** 2026-07-30  
**Parent:** FTD-0751 `[NUMERICAL FACT — BACKEND DIVERGENCE LOCALIZED]`  
**Scope:** one finite-precision equivalence repair for the research-only CUDA
matched-field pipeline; no real stencil, matter action, threshold, tolerance,
production, ontology, or default change

## 1. Frozen repair

FTD-0751 localized the first CPU/CUDA difference to CUDA contraction of

\[
E^*=E_0+\lambda\,\mathrm{curl}(B_1)
\]

into `fma.rn.f64`, while host C++ is compiled with `-ffp-contract=off` and
therefore rounds multiply and add separately.

Compile the unchanged frozen `cuda_matched_field_pipeline.cu` into a separate
research library with NVCC `--fmad=false`. The established `ftd_cuda` target,
production backend, public field equations, current deposition, observer,
matter root, and FTD-0751 executable remain unchanged. No source expression is
rewritten and no physical gate is altered.

## 2. Frozen execution

Repeat the FTD-0751 six-arm matrix exactly:

- `L={33,65}` crossed with face `(0,0,1)`, edge `(0,1,-1)`, and body
  `(1,1,1)`;
- plus-minus polarity, separation `1.30`, inward momentum `0.0120`;
- finite-support radius 4, `dt=1/4`, live `C_SPEED`, compact-pair depth
  `0.01`, cutoff squared `3/2`, shared-anchor chart;
- gate tolerance `1e-10`, solve tolerance `2e-14`, 384 iterations;
- sparse local current/residual, deferred diagnostics;
- ticks `1..8`, the same eight serialized rows per tick, and independent CPU
  and device root caches.

The selected CUDA path remains FTD-0750 ordered deposition and deterministic
selected-radius observation.

## 3. Gates

The qualification is constructive only if:

1. all six arms execute all eight ticks and produce 64 rows each;
2. every dynamic row—initial electric, initial magnetic, magnetic prepare,
   electric prepare, matter root, ordered current, and state transfer—is
   bit-identical between CPU and CUDA;
3. read-only diagnostic maximum absolute difference is at most `2e-15`;
4. generated PTX for the repair library contains no `fma.rn.f64` in
   `prepare_electric_kernel` or `prepare_magnetic_kernel`;
5. an independent Python certificate verifies hashes, records, dynamic exact
   parity, the diagnostic bound, and six-arm conjunction without rerunning the
   dynamics.

Any dynamic mismatch closes this repair. A diagnostic-only mismatch below its
gate is recorded but does not invalidate dynamic arithmetic equivalence.

## 4. Consequence

Success closes the bounded backend arithmetic item and authorizes M2/M3
campaign design using the research-only explicit-rounding library. It does not
retroactively turn FTD-0747--0750 into passing locked campaigns and does not
establish a particle, object, charge, pole, unitarity, or Lorentz recovery.

Failure blocks long CUDA matter campaigns until the earliest new divergent
stage is understood. It does not authorize tolerance relaxation or a second
unregistered repair.

## 5. Records

Write six CSV/JSON pairs plus a frozen hash manifest under
`engine/results/ftd_0752/`. The directory must be absent before execution.
The new executable and library are research-only and are not registered as a
production default or scenario toggle.
