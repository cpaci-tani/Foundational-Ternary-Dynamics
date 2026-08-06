# FTD-0748 — Canonical net-current horizon CUDA v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FULL EXECUTION]`  
**Date:** 2026-07-29  
**Parent:** FTD-0747 `[MIXED — THREE-RAY CONJUNCTION CLOSED NEGATIVE]`  
**Scope:** observer-corrected replication of the unchanged selected reciprocal
`(s,C,F)` dynamics; no production, ontology, action, field-update, endpoint,
current-deposition, force, coefficient, physical threshold, or default change

## 1. Correction being tested

FTD-0747 compared `sparse_current.size()` exactly across CPU and CUDA. That
quantity counts unaggregated container entries. It is not invariant under
splitting one coefficient into two, periodic-image choice, or cancellation of
multiple constituent contributions on the same oriented face. FTD-0747 found:

- face tick 25: CPU raw count 72, CUDA raw count 36, CUDA net support 36;
- edge tick 100: CPU raw count 108, CUDA raw count 72, CUDA net support 36;
- body tick 8: CPU and CUDA raw count 108, CUDA net support 54.

All FTD-0747 prefix scalars remained within `1e-10`; its H2--H5 gates passed
in every arm. These disclosed results are design data. FTD-0748 is therefore
not blind new evidence for environmental persistence. It tests whether a
representation-invariant current observer repairs only the invalid H1
bookkeeping gate while reproducing the frozen physical trajectory.

## 2. Canonical current observer

For every tick, wrap every sparse entry to a unique periodic key
`(axis,face_x,face_y,face_z)`, multiply by the existing `polarity_scale`, and
sum all entries sharing that key in long-double host accumulation. Define net
support as coefficients with `abs(value)>1e-10`; this reuses the existing H1
backend-equivalence gate and is not fitted to a physical target.

Record raw contribution count, net support, raw and net L1 norms, cancelled L1,
discarded L1, current-moment aggregation residual, and net source radius.
Before lock, the observer must pass unit tests for entry splitting, exact
opposite cancellation, periodic-image equivalence, and explicit tolerance
accounting. The live aggregation gate requires on every tick:

- observer validity;
- current-moment aggregation residual `<=1e-12`;
- discarded net-current L1 `<=1e-10`;
- net source radius `<=3`.

The observer is read-only. CUDA continues to apply the complete ungated raw
current to the field, so no deposited coefficient is removed from dynamics.

## 3. Frozen execution

Inherit unchanged from FTD-0747:

- `L=321`, compact-support radius 4, ticks `0..312`, contact tick 313;
- `dt=1/4`, live `C_SPEED`, depth `0.01`, cutoff squared `3/2`;
- solve tolerance `2e-14`, maximum 384 iterations;
- radii `{8,12,16,24,32,48}`;
- radius-48 threshold `1e-8`, deadline 300, post-arrival `301..312`;
- exactly one `plus_minus` face, edge, and body arm;
- FTD-0745 baseline SHA-256
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`.

The WSL2 CUDA pipeline and host implicit matter root are unchanged. The FTD-0747
CSV/JSON artifacts remain frozen and are not inputs to numerical evolution.
Pre-lock qualification is capped at eight ticks and cannot serialize results.

## 4. Ordered gates

Apply these gates in order:

1. **H0 execution:** valid preparation and every forward transaction; common
   and regional residual `<=1e-10`, energy residual `<=1e-8`, recoil defect
   `<=1e-9`, causal excess `<=1e-12`, pair-plus-field drift `<=1e-8`, net
   source radius `<=3`, and `T<T_contact`.
2. **A0 canonical aggregation:** every observer row satisfies Section 2.
3. **H1 corrected prefix:** through tick 184, exact equality of `valid`,
   `common`, `regional_valid`, net `source_radius`, and `graph_inside`, plus
   maximum difference `<=1e-10` across the original 39 scalar observables.
   The FTD-0745 raw `source_entries` container length is explicitly excluded.
   FTD-0748's CSV `source_entries` field stores net support instead.
4. **H2 persistent core:** at least 160 consecutive graph-inside ticks with
   pair energy `<-1e-6`, ending at tick 312.
5. **H3 stable near field:** tick-281--312 radius-eight energy minimum
   `>=5e-4` and maximum/minimum `<=4`.
6. **H4 causal arrival:** radius-48 outside energy starts `<=1e-12`, exceeds
   `1e-8` by tick 300, and outside-source residual remains `<=1e-10`.
7. **H5 persistence:** outward increments after arrival `>=-1e-10`, final
   radius-48 outside energy `>1e-9`, and every tick 301--312 remains `>1e-9`.

Use the ordered verdict tokens implemented in the runner, beginning with
`CANONICAL_HORIZON_EXECUTION_INVALID`, then
`CANONICAL_HORIZON_CURRENT_AGGREGATION_INVALID`, corrected prefix, H2--H5,
and ending with `CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE`
only if every earlier branch passes.

## 5. Freeze and records

After embedding this document's SHA-256, rebuild once and freeze hashes for:

- this protocol;
- the FTD-0748 runner;
- the aggregation header and implementation;
- the inherited FTD-0747 runner dependency;
- the WSL2 executable;
- the FTD-0745 baseline.

Require `engine/results/ftd_0748/` to be absent before execution. Invoke the
locked executable exactly once for each full arm, serially. Each arm writes a
313-row main CSV/JSON and a 313-row support CSV/JSON. Do not stop or retune an
arm based on another arm's result.

An independent Python certificate must verify all frozen hashes, row counts,
support aggregation gates, corrected H0--H5 gates, and ordered verdicts without
calling the C++ verdict function or rerunning dynamics.

## 6. Claim boundary

A constructive conjunction corrects the FTD-0747 representation-dependent
H1 failure and establishes the registered three-ray, finite-time persistence
statement for the selected reciprocal research dynamics. Because the physical
trajectory and later gate outcomes were disclosed by FTD-0747, it is a
protocol/observer correction, not an independent replication. It does not
establish asymptotic stability, inverse recovery, particle identity, Lorentz
recovery, a fundamental electromagnetic ontology, or production adoption.
