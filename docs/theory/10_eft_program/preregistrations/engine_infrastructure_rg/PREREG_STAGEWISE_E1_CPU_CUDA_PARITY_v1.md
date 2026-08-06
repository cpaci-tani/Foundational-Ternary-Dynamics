# FTD-0751 — Stagewise E1 CPU/CUDA parity classifier v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Date:** 2026-07-30  
**Parent:** FTD-0750 `[MIXED — CUDA REPLAY QUALIFIED; CPU PREFIX NOT UNIVERSAL]`  
**Scope:** bounded backend audit of the unchanged selected reciprocal
`(s,C,F)` matter candidate; no production, ontology, action, coefficient,
threshold, tolerance, or default change

## 1. Purpose and claim boundary

FTD-0750 showed that ordered current deposition can reproduce the host's
sequential additions exactly, while the complete host/device trajectory still
misses the registered `1e-10` CPU-prefix gate on the edge and body rays. That
result did not identify the first operation at which the maps separate.

FTD-0751 is a localization instrument, not a repair. It compares the CPU and
WSL2-CUDA maps after each causally ordered stage and records the first
bitwise-unequal scalar. It must not change CUDA contraction settings, replace
arithmetic, relax a tolerance, or use the result as evidence for a particle.

The selected E1 candidate is frozen for this audit as:

- ternary site polarity represented by the existing two opposite constituent
  records and their deterministic subcell charts;
- matched oriented face electric flux `F` and edge magnetic field `C`;
- the derived compact-pair binding law with no stored primitive edge;
- the existing finite-support radius-four Gauss preparation;
- the unchanged host implicit common-action root;
- the FTD-0750 ordered sparse-current device deposition;
- no legacy force, Poisson-Coulomb, production-tick, reaction, collision,
  pair-production, GPU-root, or post-hoc correction branch.

## 2. Frozen arms and controls

Run exactly six arms: volumes `L={33,65}` crossed with the registered face,
edge, and body directions `(0,0,1)`, `(0,1,-1)`, and `(1,1,1)`. Each arm uses:

- plus-minus polarity ordering;
- separation `1.30` and inward momentum magnitude `0.0120`;
- `dt=1/4`, live `C_SPEED`, compact-pair depth `0.01`, and cutoff squared
  `3/2`;
- shared-anchor chart enabled;
- gate tolerance `1e-10`, solve tolerance `2e-14`, and at most 384 root
  iterations;
- sparse local current and local residual evaluation;
- deferred volume diagnostics;
- finite-support preparation tolerance `1e-13` and at most 4096 iterations;
- ticks `1..8`, with complete records at every tick.

The CPU and CUDA arms begin from independently copied, bit-identical prepared
states. They use independent host root caches. No state from one arm may be
substituted into the other after tick zero.

## 3. Ordered stage records

For every arm and tick, record these stages in order:

1. `initial_electric` and `initial_magnetic` before source-free evolution;
2. `magnetic_prepare`, after `B1=B0-lambda*C^T E0`;
3. `electric_prepare`, after `E*=E0+lambda*C B1`;
4. `matter_root`, including validity, solve status, iterations, residual,
   endpoint positions and momenta, deposited sparse-current entries, and
   interaction metadata;
5. `ordered_current`, after `E1=E*-polarity_scale*sum(j)`;
6. `state_transfer`, after both host and device arms accept their own later
   state and the CUDA pipeline advances its resident fields;
7. `diagnostics`, using the deterministic selected-radius CUDA observer and
   the corresponding CPU profile at radii `{2,4,8}` clipped to the periodic
   half-volume.

Every field comparison records exact-equality, unequal scalar count, maximum
absolute difference, maximum ULP distance where finite, and the first unequal
axis/index with both values. Matter-root comparisons record the analogous
maximum component difference and first differing constituent component.
Sparse current is compared in original segment/entry order, including face,
axis, and value.

## 4. Frozen classification

Classification uses the earliest unequal dynamic stage across all arms and
ticks:

- `EXACT_STAGE_PARITY`: every recorded dynamic scalar is bit-identical;
- `SOURCE_FREE_MAGNETIC_PREPARE_DIVERGENCE`: the first inequality is in
  `magnetic_prepare`;
- `SOURCE_FREE_ELECTRIC_PREPARE_DIVERGENCE`: magnetic preparation is exact and
  the first inequality is in `electric_prepare`;
- `MATTER_ROOT_DIVERGENCE`: both prepared fields are exact and the first
  inequality is in root outputs or deposited-current records;
- `ORDERED_CURRENT_DIVERGENCE`: preparation and root outputs are exact and the
  first inequality appears only after ordered deposition;
- `STATE_TRANSFER_DIVERGENCE`: the transaction agrees before transfer but not
  after each backend advances its own state;
- `DIAGNOSTIC_ONLY_DIVERGENCE`: dynamic stages are exact and only read-only
  diagnostics differ;
- `EXECUTION_INVALID`: preparation, root, deposition, transfer, or observation
  fails to execute.

Later amplification cannot change the classification. The raw record may also
report downstream differences, but the earliest stage is the verdict.

## 5. Acceptance and consequence

This audit passes as an instrument only if all six arms execute eight ticks,
the initial copies are exact, the classifier is internally consistent, and an
independent certificate reproduces the earliest-stage verdict from the CSV
without calling the C++ classifier.

`EXACT_STAGE_PARITY` permits the bounded backend question to close. A prepare
divergence permits one separately preregistered arithmetic-equivalence repair
only if the differing operation is demonstrated to implement the same real
map. A root, deposition, or transfer divergence blocks long CUDA matter
campaigns until that stage is understood. No outcome promotes E1 to ontology.

After certification, the research mainline returns to the physical M2/M3
questions: whether this frozen E1 candidate has an uncontained metastable
object basin and autonomous matter motion. Lorentz recovery, charge
identification, pole extraction, and new constant formulae remain deferred.

## 6. Records and reproducibility

Write one CSV and one JSON summary per arm under
`engine/results/ftd_0751/`. Each record stores the protocol SHA-256, source and
executable hashes, volume, direction, tick, stage, exact flag, unequal count,
maximum absolute difference, maximum ULP distance, first-difference location,
both values, and arm classification.

The WSL2 executable is built once after the protocol hash is embedded. The
result directory must be absent before the registered execution. A Python
certificate verifies frozen hashes, the six-arm/eight-tick matrix, stage
ordering, independent earliest-divergence classification, and JSON/CSV
agreement without rerunning the dynamics.
