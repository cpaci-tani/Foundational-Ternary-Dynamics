# FTD-0747 — Causal-horizon environmental persistence CUDA v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE HELD-OUT EXECUTION]`  
**Date:** 2026-07-29  
**Parent:** FTD-0746 `[ABORTED BEFORE SERIALIZATION — NO PHYSICS RESULT]`  
**Scope:** accelerated observer/research execution of the unchanged selected
reciprocal `(s,C,F)` action; no production rule, primitive, potential, field
equation, force, coefficient, threshold, scenario, or default change

## 1. Registered question and inherited physics

FTD-0747 asks exactly the physical question frozen by FTD-0746: whether the
FTD-0745 compact-support state retains a localized core and near field through
the independently predicted radius-48 threshold arrival, and whether the
radius-48 exterior component then remains present and outward through tick 312.

All physical inputs, ray arms, thresholds, gates, verdict ordering, discovery
data, and claim limits are inherited unchanged from
`PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md`. In particular:

- `L=321`, compact-support radius four, `T=312`, contact tick 313;
- `dt=1/4`, live `C_SPEED`, depth `0.01`, cutoff squared `3/2`;
- solve tolerance `2e-14`, common/observer gate `1e-10`;
- radii `{8,12,16,24,32,48}`;
- radius-48 threshold `1e-8`, deadline 300, post-arrival ticks `301..312`;
- exactly one `plus_minus` face, edge, and body history;
- the FTD-0745 baseline CSV remains frozen at SHA-256
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`.

The aborted CPU process produced no CSV or JSON and contributes no discovery
row, partial trajectory, stopping information, or physics verdict.

## 2. Registered CUDA implementation

Use the default-off `CudaMatchedFieldPipeline` under WSL2 Ubuntu-22.04 on the
RTX 5090. For every accepted tick:

1. keep matched face electric and edge magnetic fields resident on the device;
2. compute `B1=B0-lambda*C^T E0` and `E*=E0+lambda*C B1` on CUDA;
3. download only the prepared `B1,E*` arrays for the unchanged host-side local
   implicit matter root;
4. solve the same sparse straight-segment current and common-action residual;
5. apply `E1=E*-polarity_scale*sum(J)` on CUDA;
6. compute the six regional energy ledgers, both Gauss residuals, modified
   field energies, local translation momentum, and quadratic-spline Poynting
   momentum on CUDA;
7. complete the unchanged common-action gates from those diagnostics and
   advance the device-resident state.

The accelerated solver may rotate host buffers and omit a duplicate copy of
the returned before-field. It may not alter a constituent endpoint, current
entry, field coefficient, reduction definition, gate, or verdict.

## 3. Locked backend qualification

Before this lock, the dedicated CPU/CUDA parity target passed with:

- prepared magnetic/electric field maximum difference `2.17e-19`;
- accepted field maximum difference at most `3.47e-18`;
- regional-profile difference at most `8.68e-19`;
- Gauss/energy/local-momentum/spline-momentum difference at most `4.41e-20`;
- repeated CUDA determinism difference at most `3.47e-18`;
- full implicit transaction state difference `2.17e-19`;
- full completed transaction diagnostic difference `6.94e-18`.

The locked certification gates are respectively `2e-15`, `2e-10`, `1e-14`,
`2e-10`, and `2e-10`; every measured residual is inside its applicable gate.
An `L=321` allocation/field/observer/diagnostic smoke passed without swap.

Pre-lock causal qualifications also passed on all inequivalent rays. The face
arm ran four ticks; edge and body ran two ticks each. Their maximum common
residual was `5.33e-14`, maximum total-energy residual `7.85e-17`, and maximum
recoil defect `3.13e-17`. Qualification runs serialized no campaign artifacts
and do not count as held-out physics evidence.

## 4. Inherited physical gates and verdicts

Apply FTD-0746 gates H0--H5 without modification:

1. valid preparation and exact forward transaction, including common/regional
   residual `<=1e-10`, total-energy residual `<=1e-8`, recoil defect `<=1e-9`,
   causal excess `<=1e-12`, pair-plus-field drift `<=1e-8`, source radius
   `<=3`, and `T<T_contact`;
2. exact FTD-0745 prefix through tick 184, with discrete equality and maximum
   scalar difference `<=1e-10`;
3. at least 160 consecutive graph-inside negative-energy ticks ending at 312;
4. radius-eight late energy minimum `>=5e-4` and dynamic range `<=4`;
5. radius-48 arrival above `1e-8` no later than tick 300 with no source outside;
6. post-arrival outward increments `>=-1e-10`, final outside energy `>1e-9`,
   and every tick `301..312` outside energy `>1e-9`.

Use the same ordered tokens:

1. `CAUSAL_HORIZON_EXECUTION_INVALID`;
2. `CAUSAL_HORIZON_PREFIX_DRIFT`;
3. `CAUSAL_HORIZON_CORE_NOT_PERSISTENT`;
4. `CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE`;
5. `CAUSAL_HORIZON_R48_ARRIVAL_FAIL`;
6. `CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT`;
7. `CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE` only if every
   earlier branch passes.

## 5. Execution and records

After embedding this protocol's SHA-256 and rebuilding, freeze the source and
WSL2 executable hashes. Verify `engine/results/ftd_0747/` is absent. Invoke the
locked executable exactly once per `face`, `edge`, and `body`; do not stop any
arm because of another arm's result.

Persist exactly 939 CSV rows plus one JSON summary per arm under
`engine/results/ftd_0747/`. Every JSON records FTD-0747, this protocol hash, the
FTD-0745 baseline hash, and backend `wsl2_cuda_matched_face_edge`.

After all arms finish, an independent proof must reconstruct row counts,
prefix comparisons, every H0--H5 gate, and the ordered verdict without calling
the C++ verdict function. No result is promoted from approaching a measured
constant, and no failed gate authorizes retuning or a hidden CPU fallback.

## 6. Claim boundary

A constructive result establishes only three-ray forward core/near-field
persistence through the registered radius-48 arrival and 12-tick post-arrival
window for the selected reciprocal dynamics. It does not establish inverse
recovery, asymptotic environmental closure, M3 family identity, particle
ontology, native reduction, or production-engine CUDA equivalence outside the
qualified matched face/edge transaction.
