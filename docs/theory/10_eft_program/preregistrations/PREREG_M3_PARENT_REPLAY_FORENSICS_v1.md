# FTD-0756 — M3 parent-replay infrastructure forensics v1

**Status:** `[PRE-REGISTRATION — FROZEN AFTER FTD-0755 INFRASTRUCTURE VERDICT; NOT RUN]`  
**Date:** 2026-07-30  
**Parent:** FTD-0755 `M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED`  
**Scope:** observer-only localization of the parent replay failure; no repair,
new dynamics, predicate change, or matter claim

## 1. Locked question

Why did every FTD-0755 candidate and causal-fibre arm report an uninitialized
tick-160 parent even though the published FTD-0753 `L=321` scalar records are
valid, common-action, graph-inside, and negative-energy at tick 160?

## 2. Frozen matrix

Replay exactly the FTD-0755 parent construction for

```text
L = {321,385}
ray = {face,edge,body}
ticks = 0,...,160
```

Use the locked FTD-0755 executable source by textual inclusion with its `main`
renamed. Do not edit that source or the FTD-0755 artifacts. Use the same
explicit-rounding ordered CUDA library, options, initial geometry, radius-four
finite-support preparation, field normalization, cache, and `M3CudaStepper`.

## 3. Frozen observations

Record before dynamics:

- preparation validity, density containment, compact support, zero boundary
  crossing, Poisson residual, Gauss residual, outside maximum, and boundary
  crossing maximum;
- the state-only sector, membership, graph margin, energy margin, separation,
  and pair energy.

At every attempted transaction record:

- wrapper validity and failure stage;
- nonlinear solve attempted/converged, iteration count, and solve residual;
- final step validity/common-action bit and maximum residual;
- energy, recoil, causal-speed, and pipeline-liveness diagnostics;
- the state-only sector, membership, graph margin, energy margin, separation,
  and pair energy after the attempted step.

Stop a history at the first invalid wrapper/step/common-action transaction.
Do not continue from a failed state.

## 4. Frozen independent comparison

For `L=321`, compare every successfully reached tick against the exact
FTD-0753 CSV for the same ray:

```text
valid
common
separation
pair_energy
maximum common-action residual
energy residual
recoil diagnostic
causal speed excess
```

The FTD-0753 strings are the source of record. Report bit/string equality and
also maximum numerical difference; no tolerance is used to call a differing
string exact. `L=385` has no historical scalar baseline and is not fitted to
`L=321`.

## 5. Frozen verdict map

Apply the first matching outcome:

1. preparation fails: `M3_PARENT_FINITE_SUPPORT_PREPARATION_FAILURE`;
2. replay differs from FTD-0753 before its first wrapper failure:
   `M3_PARENT_REPLAY_DIVERGENCE`;
3. replay matches FTD-0753 through the last successful row and the wrapper
   then fails: `M3_PARENT_WRAPPER_TRANSACTION_FAILURE`;
4. tick 160 is reached but the state-only predicate rejects the published
   graph-inside negative-energy state: `M3_PARENT_PREDICATE_RECONCILIATION_FAILURE`;
5. all `L=321` parents pass but an `L=385` parent fails:
   `M3_PARENT_LARGE_VOLUME_FAILURE`;
6. all six parents pass:
   `M3_PARENT_FORENSICS_PASS_FTD0755_DISPOSITION_INCONSISTENT`.

FTD-0756 diagnoses FTD-0755. It cannot retroactively reopen, rerun, or change
the FTD-0755 verdict. Any repair requires FTD-0757 or later.

## 6. Execution firewall

Before execution, freeze the protocol, diagnostic source, independent
certificate, WSL2 executable hashes, output schema, and absence of
`engine/results/ftd_0756/`. Qualification may run for at most one transaction
and writes no registered artifact. Production, established CUDA, scenarios,
and ontology remain unchanged.
