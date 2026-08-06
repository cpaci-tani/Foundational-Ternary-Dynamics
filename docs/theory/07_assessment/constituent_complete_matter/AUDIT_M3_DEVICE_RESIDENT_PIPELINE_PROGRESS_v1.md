# FTD-0759 — M3 device-resident pipeline implementation progress v1

**Status:** `[INFRASTRUCTURE PARTIAL — LOCKED PARITY MATRIX PASSES; UTILIZATION SMOKE FAILS]`  
**Date:** 2026-07-30  
**Frozen protocol:**
`../10_eft_program/preregistrations/PREREG_M3_DEVICE_RESIDENT_PIPELINE_PARITY_v1.md`  
**Frozen protocol SHA-256:**
`204A737092AA1F42D16C795EBEEC5A5C8F2A4A6EBADF303E1D135BF3DE8FBC42`

## 1. Result

A partial CUDA implementation now keeps the matched face-electric and
edge-magnetic fields resident while the host root solver requests only local
quadratic-coat orbit gathers. The accepted current is applied in the existing
per-face order. Common-action volume diagnostics, the state-only matter/field
observer, and the `{4,6,8}` support ladder reduce on the GPU without a complete
field download.

The complete locked small-volume parity matrix passes:

```text
volumes:       L = 33, 65
rays:          face, edge, body
states:        tick-0 prepared pair; tick-160 parent; remote-plaquette parent
steps/state:   2 forward transactions
total:         18 cases; 36 transactions
```

On each transaction it checks:

- reference/resident accepted endpoint and root-scalar parity;
- byte-identical ordered sparse-current entries;
- common-action diagnostic parity;
- state-only field-observer and support-ladder parity;
- zero complete-field downloads inside the resident root and observers;
- a deliberate post-transaction full-field checkpoint with complete-state
  maximum difference at most `1e-12`; and
- device-only field advance before the second transaction.

Each two-step result is then inverted from the final complete state alone.
Forward/reverse recovery remains within `1e-10` without stored route history.

The matrix executable returns:

```text
FTD-0759 resident parity matrix: PASS cases=18 transactions=36
```

It does not execute the `L={321,385}` performance matrix.

## 2. Implemented boundary

The implementation adds read-only borrowed device-field views, an exact
piecewise orbit-gather kernel, a matter-only resident root entry point, and
resident overloads for the two state-only observers. Trial constituent
endpoints and sparse segments remain host-side exactly as frozen. Only local
gather records and reduction scalars cross from device to host during a
resident transaction.

The CPU-constructed compact bound representatives are now transferred
sparsely: target device arrays are zeroed and only their exact nonzero indexed
coefficients are uploaded. The primitive boundary ledger still reads
individual device faces rather than using one compact local gather. That is a
remaining performance/infrastructure gap, not a change to the reference
arithmetic.

## 3. Verification

WSL2 CUDA focused regression:

```text
ctest --test-dir engine/build_wsl \
  -R '^(cuda_matched_field_pipeline|cuda_state_only_support_ladder|cuda_quadratic_coat_orbit_gather|quadratic_coat_orbit_gather|state_only_matter_field_observer|state_only_matter_field_observer_covariance)$' \
  --output-on-failure -j 6

6/6 passed; real time 6.37 s

campaign_m3_device_resident_pipeline_parity_cuda

18/18 cases and 36/36 transactions passed; real time 16.9 s
```

The test result is infrastructure qualification only. It produces no matter,
particle, charge, pole, Lorentz, mass, or ontology evidence.

## 4. Remaining locked gates

Large-volume result-free smoke measurements after the parity pass are:

| Volume | Reference, 2 ticks | Resident, 2 ticks | Smoke speedup | Reference D2H/tick | Resident D2H/tick |
|---:|---:|---:|---:|---:|---:|
| 321 | 25.5362 s | 2.49816 s | 10.22x | 1.588 GB | 2.626 MB |
| 385 | 45.2020 s | 4.30060 s | 10.51x | 2.739 GB | 2.626 MB |

Both resident arms report zero complete-field downloads. A separate 10-tick
`L=385` resident stretch completes in 23.409 s and transfers 4.844 MB H2D and
26.260 MB D2H in total. One-second `nvidia-smi dmon` samples over its active
window show median SM utilization `0%`, peak `48%`, and peak framebuffer use
about `16.2 GB`. Thus speed and memory smokes clear their thresholds, but the
registered `>=50%` median-utilization gate does not. These smoke measurements
are not the frozen three-repetition performance matrix and do not constitute
an FTD-0759 performance verdict.

The low utilization localizes the next infrastructure problem to host-driven
root iterations, synchronization gaps, and/or fine-grained boundary-ledger
reads between otherwise successful GPU kernels.

FTD-0759 remains open until all of the following pass without changing the
frozen physics or tolerances:

1. a compact local-cube/boundary gather replacing individual device-face
   reads in the boundary ledger;
2. enough device-side batching or root/controller residency to clear the
   utilization gate without idle work; and
3. the complete registered three-repetition performance matrix at
   `L={321,385}`.

Production defaults, `RenderBridge`, scenario status, the selected action,
the M3 predicate, and FTD-0758's consumed evidential status are unchanged.
