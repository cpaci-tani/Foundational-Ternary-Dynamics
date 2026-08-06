# FTD-0749 — Deterministic canonical-current CUDA replay v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FULL EXECUTION]`  
**Date:** 2026-07-29  
**Parent:** FTD-0748 `[MIXED — THREE-RAY CONJUNCTION CLOSED NEGATIVE]`  
**Scope:** deterministic backend correction for the unchanged selected
reciprocal `(s,C,F)` dynamics; no production, ontology, action, endpoint,
force, coefficient, physical threshold, tolerance, or default change

## 1. Defect being tested

FTD-0748 corrected the current-support observer but exposed trajectory-level
CUDA/CPU prefix drift in edge and body arms. Frozen FTD-0747/0748 comparisons
also differ between CUDA executions by up to `1.3945e-10` in separation.
Source inspection identifies the collision-prone deposition path: raw sparse
entries can share one canonical oriented face and are accumulated by floating
`atomicAdd`, whose addition order is not fixed.

FTD-0749 aggregates the complete ungated current on the host to unique
periodically wrapped keys `(axis,face_x,face_y,face_z)` with the already tested
canonical observer at zero filtering tolerance. Each resulting coefficient is
uploaded once and applied by one non-atomic device update. There is therefore
at most one writer per field component/index in the deposition kernel.
`apply_sparse_current()` and every production caller remain unchanged; the new
`apply_canonical_sparse_current()` API is research-only.

## 2. Disclosed pre-lock evidence

The duplicate-face device unit test uses six raw contributions collapsing to
three net faces. Independent pipelines, reversed segment/entry order, the host
canonical update, and the device update agree exactly (`0.0` maximum
difference).

Non-serializing eight-tick `L=321` qualifications pass for face, edge, and
body. Their maximum net supports are 36, 36, and 54; discarded current is zero;
maximum aggregation-moment residuals are zero, `2.7953e-19`, and `2.7444e-19`.
FTD-0747/0748 full trajectories and every qualification result are design
data. This is not a blind new-physics test.

## 3. Frozen execution

Inherit unchanged from FTD-0748:

- `L=321`, compact-support radius 4, ticks `0..312`, contact tick 313;
- `dt=1/4`, live `C_SPEED`, depth `0.01`, cutoff squared `3/2`;
- solve tolerance `2e-14`, maximum 384 iterations;
- radii `{8,12,16,24,32,48}`;
- radius-48 threshold `1e-8`, deadline 300, post-arrival `301..312`;
- one `plus_minus` face, edge, and body geometry;
- FTD-0745 baseline SHA-256
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`;
- canonical-observer tolerance `1e-10`, moment gate `1e-12`;
- every H0--H5 numerical threshold.

Invoke six independent processes serially and exactly once:
`face_a`, `face_b`, `edge_a`, `edge_b`, `body_a`, and `body_b`. Each process
must serialize one 313-row main CSV/JSON pair and one 313-row support CSV/JSON
pair. Do not stop, rebuild, retune, or change later arms after any earlier
result.

## 4. Ordered gates

For each replicate, apply FTD-0748's gates in their existing order:

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

Then apply **D0 replay identity** before any three-ray promotion:

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
the protocol, FTD-0749 runner, unique-face API header and CUDA implementation,
aggregation header/implementation, inherited FTD-0748/0747 runner dependencies,
WSL2 executable, duplicate-face unit test, and FTD-0745 baseline. Require
`engine/results/ftd_0749/` to be absent before full execution.

An independent Python certificate must verify frozen hashes, 313 ordered rows
per CSV, all A0/H0--H5 gates, D0 exact replay identity, ordered verdicts, and
the six-arm conjunction without calling the C++ verdict function or rerunning
dynamics.

## 6. Claim boundary

A constructive result qualifies deterministic unique-face CUDA deposition for
this selected campaign and repairs the specific FTD-0748 backend-reproducibility
defect. It remains disclosed protocol development, not independent evidence for
a particle. It does not establish asymptotic stability, inverse recovery,
Lorentz recovery, unitarity, production adoption, or a fundamental
electromagnetic ontology. A negative result is retained without tolerance
relaxation or post-hoc force/current amplification.

