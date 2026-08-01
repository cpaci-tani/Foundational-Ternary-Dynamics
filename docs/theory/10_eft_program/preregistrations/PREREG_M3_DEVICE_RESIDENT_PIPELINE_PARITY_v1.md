# FTD-0759 — M3 device-resident pipeline parity v1

**Status:** `[PRE-REGISTRATION — FROZEN BEFORE IMPLEMENTATION; INFRASTRUCTURE ONLY]`  
**Date:** 2026-07-30  
**Parents:** FTD-0757 fixed-chart parent qualification; FTD-0758 held-out
validation protocol  
**Scope:** CUDA residency, CPU parity, and performance qualification only; no
new matter-family, particle, charge, mass, pole, Lorentz, or ontology claim

## 1. Locked question

Can the selected reciprocal-pair matter transaction and the complete M3
qualification observer execute with the volume fields resident on the GPU,
while reproducing the existing CPU/reference arithmetic to the locked gates
and without changing any physical state, predicate, perturbation, tolerance,
or verdict rule?

The current FTD-0758 executable is CUDA-linked but is not a device-resident M3
pipeline.  It downloads six full `L^3` field components before every matter
transaction, and its every-tick state-only field observer and `{4,6,8}` support
ladder perform host `L^3` reductions.  FTD-0759 treats this as an infrastructure
boundary, not a physics failure.

## 2. Frozen semantic source of truth

The following existing routines define the result to reproduce:

- `solve_connected_moore_block_forward_prepared` in
  `engine/src/eft/connected_moore_block_action.cpp`;
- `CudaMatchedFieldPipeline::{prepare_forward,
  apply_ordered_sparse_current,observe_deterministic,
  diagnose_common_action,advance}`;
- `observe_state_only_matter_field` and
  `observe_state_only_support_ladder`;
- the FTD-0758 fixed integer chart `C_L`, support ladder `{4,6,8}`, observer
  support `4`, shell radii, local radius `24`, and all algebraic gates.

No formula, operation order, reduction quantity, field normalization, root
seed, finite-difference scale, support, radius, tolerance, or acceptance rule
may change under this item.

## 3. Required device-resident boundary

Implement a default-off research path with these properties:

1. Upload the complete earlier matched face/edge fields once.  Keep `E_0`,
   `B_0`, prepared `E_pre`, `B_1`, and post-current `E_1` resident across
   ticks.  No complete `L^3` field download is allowed inside a tick.
2. Construct trial constituent endpoints and sparse ordered face-current
   segments on the host exactly as the reference solver does.
3. Evaluate each implicit-root trial by a GPU gather over only the quadratic
   coat/orbit support required by those segments.  Return only the gathered
   impulses and scalar residual data.  The dense field must not be
   materialized on the host for Newton/Broyden probes.
4. Apply the accepted sparse ordered current on the GPU using the existing
   per-face order.  A route regrouping or non-deterministic atomic sum is not
   permitted.
5. Evaluate common-action diagnostics, the fixed-chart regional observer,
   the state-only field observer, and the `{4,6,8}` support ladder on the GPU.
   The finite-support bound fields may be generated on the CPU from the same
   compact Poisson construction and uploaded sparsely; their support and
   values must be byte-identical to the reference construction.
6. Download only constituent data, root/ledger scalars, support-ladder
   scalars, shell scalars, and the selected radius-24 local field cube.  A
   deliberate parity checkpoint may download a full field, but the timed
   device-resident path may not.
7. Preserve a state-only inversion interface: the device field buffers plus
   host constituent metadata are implementation state, not an ontological
   primitive and not visible to production defaults or `RenderBridge`.

## 4. Locked parity matrix

Before any new physics campaign, compare the reference and device-resident
paths on fresh, non-result-producing qualification arms:

```text
volumes:       L = 33, 65
rays:          face, edge, body
states:        tick-0 prepared pair; tick-160 parent; remote-plaquette parent
steps/state:   2 forward transactions
supports:      4, 6, 8
local cube:    radius 24 (clipped only where L requires the reference rule)
```

For every transaction require:

- identical accepted anchors, remainders, charges, site-hop counts, sparse
  segments, and ordered current entries;
- constituent position/momentum maximum difference `<= 1e-13`;
- root, continuity, Gauss, work, recoil, speed, and energy scalar difference
  `<= 1e-12` and identical Boolean gates;
- state-only observer and support-ladder scalar difference `<= 1e-12`,
  identical membership, and identical valid/invalid classifications;
- full-field maximum difference `<= 1e-12` at the parity checkpoint;
- forward/reverse state recovery at the existing reference tolerances.

Any mismatch is an infrastructure defect.  It cannot be repaired by changing
a physical tolerance, operation order, observer quantity, or matter rule.

## 5. Locked performance matrix

Only after the parity matrix passes, run a result-free timing comparison:

```text
volumes:       L = 321, 385
ray:           face
history:       one qualified parent plus 8 continuation transactions
branches:      baseline and remote fibre
warm-up:       2 unmeasured transactions
repetitions:   3
```

Record median wall time, complete-field bytes transferred per tick, kernel
time, transfer time, observer time, peak device memory, and one-second GPU
utilization samples.  Acceptance requires:

- zero complete-field downloads in the timed device-resident tick;
- at least `10x` median wall-time speedup over the current FTD-0758 path;
- median GPU utilization at least `50%` during measured transactions;
- no increase in peak device memory above `30 GiB`;
- identical parity verdicts after timing.

Failure leaves the existing CUDA-linked/host-observer path as the reference
and records device-resident acceleration as unresolved.  It has no effect on
the FTD-0758 physics verdict.

## 6. Execution firewall

- FTD-0758 continues under its already-frozen executable and is neither
  interrupted nor reinterpreted by FTD-0759.
- FTD-0759 may not read FTD-0758 result values during implementation.
- Production defaults, `RenderBridge`, scenarios, and the frozen ontology are
  unchanged.
- The parity and timing runner writes only infrastructure JSON/CSV under a new
  FTD-0759 result directory.  Those files cannot count as matter evidence.
- A future held-out physics campaign may use the device-resident path only
  after an independent certificate accepts every parity and performance gate.

