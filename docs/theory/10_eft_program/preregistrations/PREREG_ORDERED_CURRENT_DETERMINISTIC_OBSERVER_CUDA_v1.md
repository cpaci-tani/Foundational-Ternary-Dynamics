# FTD-0750 — Ordered-current deterministic-observer CUDA replay v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FULL EXECUTION]`  
**Date:** 2026-07-29  
**Parent:** FTD-0749 `[MIXED — STRICT CONJUNCTION CLOSED NEGATIVE]`  
**Scope:** two research-only finite-precision repairs for the unchanged selected
reciprocal `(s,C,F)` campaign; no production, ontology, action, endpoint,
force, coefficient, baseline, physical threshold, tolerance, or default change

## 1. Frozen defects and repairs

FTD-0749 established exact paired trajectory replay after removing
collision-prone current atomics, but its registered conjunction failed in two
independent places.

First, strict D0 failed by at most `2.775557561563e-17` only in read-only
regional records. The cause is the two-level floating `atomicAdd` histogram in
`regional_profile_kernel`. FTD-0750 adds a selected-radius observer with a
fixed 128-thread block reduction and fixed host block-order reduction. It
computes the six registered radii directly and accepts at most six radii.
`observe()` remains unchanged.

Second, the face CPU prefix failed at `1.386979420204e-10`. The FTD-0749
aggregate-once map is mathematically equivalent to, but not the same
finite-precision map as, CPU's sequential raw additions. FTD-0750 groups raw
entries by canonical periodic oriented face while preserving the original
segment/entry order within each face. One device thread per face then applies
each multiplication and addition with explicit round-to-nearest intrinsics.
`apply_sparse_current()` and `apply_canonical_sparse_current()` remain
unchanged.

## 2. Disclosed pre-lock evidence

The adversarial CUDA unit passes. It checks exact device/CPU equality for
duplicate periodic faces; exact identity of two independent device pipelines;
bit-identical repeated selected-radius observations; and a cancellation case
whose result changes when the same-face entry order changes. Each reordered
device result equals its corresponding sequential CPU result exactly.

Six non-serializing four-tick `L=321` qualifications pass. Face and edge have
maximum canonical net support 36; body has 54. Discarded current is zero.
Maximum aggregation-moment residual is zero for face,
`1.8888834723770898e-19` for edge, and `7.496241583200558e-20` for body.
All FTD-0745--0749 records and these qualifications are disclosed design data.
This is a locked repair test, not blind new-physics evidence.

## 3. Frozen execution

Inherit unchanged from FTD-0749:

- `L=321`, compact-support radius 4, ticks `0..312`, contact tick 313;
- `dt=1/4`, live `C_SPEED`, depth `0.01`, cutoff squared `3/2`;
- solve tolerance `2e-14`, maximum 384 iterations;
- radii `{8,12,16,24,32,48}`;
- radius-48 threshold `1e-8`, deadline 300, post-arrival `301..312`;
- one `plus_minus` face, edge, and body geometry;
- FTD-0745 baseline SHA-256
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`;
- observer tolerance `1e-10`, aggregation-moment gate `1e-12`;
- every H0--H5 numerical threshold.

Invoke six independent processes serially and exactly once: `face_a`,
`face_b`, `edge_a`, `edge_b`, `body_a`, and `body_b`. Each process must
serialize one 313-row main CSV/JSON pair and one 313-row support CSV/JSON pair.
Do not stop, rebuild, retune, or change later arms after any earlier result.

## 4. Ordered gates

For each replicate, apply these gates in order:

1. **H0 execution:** preparation and every forward transaction valid; common
   and regional residual `<=1e-10`, energy residual `<=1e-8`, recoil defect
   `<=1e-9`, causal excess `<=1e-12`, pair-plus-field drift `<=1e-8`, source
   radius `<=3`, and `T<T_contact`.
2. **A0 canonical aggregation:** every observer valid; moment residual
   `<=1e-12`, discarded L1 `<=1e-10`, net source radius `<=3`.
3. **H1 corrected CPU prefix:** through tick 184, exact equality of `valid`,
   `common`, `regional_valid`, canonical net `source_radius`, and
   `graph_inside`; maximum difference across the original 39 scalar
   observables `<=1e-10`. Raw container length remains excluded.
4. **H2 persistent core:** at least 160 consecutive terminal ticks with
   `graph_inside` and pair energy `<-1e-6`.
5. **H3 stable near field:** tick-281--312 radius-eight minimum `>=5e-4` and
   maximum/minimum `<=4`.
6. **H4 causal arrival:** radius-48 outside energy starts `<=1e-12`, exceeds
   `1e-8` by tick 300, and outside-source residual remains `<=1e-10`.
7. **H5 persistence:** post-arrival outward increments `>=-1e-10`, final
   radius-48 outside energy `>1e-9`, and ticks 301--312 remain `>1e-9`.

Then apply **D0 strict replay identity** before any promotion:

- for each ray, the `a` and `b` support CSV rows must be byte-identical;
- every main CSV cell except the registered replicate-specific `arm` token
  must be byte-identical;
- every JSON physical/gate value except `arm` must be identical;
- the independently reconstructed ordered verdict must match within each pair.

The registered conjunction is constructive only if D0 and H0--H5 pass for all
six records. Any mismatch closes deterministic CUDA replay for this path. Any
H1 failure keeps CPU-prefix equivalence closed. Later gates cannot override an
earlier failure.

## 5. Freeze and records

After embedding this document's SHA-256, rebuild once and freeze hashes for
the protocol, FTD-0750 runner, CUDA header and implementation, adversarial
unit, inherited FTD-0748/0747 runner dependencies, WSL2 executable, and
FTD-0745 baseline. Require `engine/results/ftd_0750/` to be absent immediately
before full execution.

An independent Python certificate must verify frozen hashes, 313 ordered rows
per CSV, A0/H0--H5, exact D0 replay identity, ordered verdicts, and the six-arm
conjunction without calling the C++ verdict function or rerunning dynamics.

## 6. Claim boundary

A constructive result qualifies exact CUDA replay and CPU-prefix equivalence
for this selected pre-contact campaign. It repairs backend arithmetic; it is
not independent evidence for a particle. It does not establish asymptotic
stability, inverse recovery, Lorentz recovery, unitarity, production adoption,
or a fundamental electromagnetic ontology. A negative result is retained
without tolerance relaxation, baseline replacement, or post-hoc force/current
amplification.
