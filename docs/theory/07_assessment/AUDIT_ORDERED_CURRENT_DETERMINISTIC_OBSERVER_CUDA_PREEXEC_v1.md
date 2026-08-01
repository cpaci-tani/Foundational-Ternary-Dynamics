# FTD-0750 ordered-current deterministic-observer CUDA pre-execution audit v1

**Status:** `[FROZEN — SIX HELD-OUT EXECUTIONS NOT YET STARTED]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`C3A3E787A201F1E429E1ED8D8D81B9F06B508A413B41A6B5E2584ED1BFD13385`

## Frozen corrections

FTD-0750 adds two research-only CUDA paths. Ordered deposition groups entries
by canonical periodic face, preserves raw per-face order, and assigns one
writer to reproduce CPU's sequential double arithmetic. Deterministic regional
observation replaces histogram atomics with fixed 128-thread reduction trees
for the six selected radii and a fixed host block-order sum. The legacy
deposition and observer APIs and every production caller remain unchanged.

The focused CUDA regressions pass `3/3`: `cuda_ordered_current_observer`,
`cuda_canonical_current_deposition`, and `cuda_matched_field_pipeline`. The
adversarial ordered-current unit establishes exact CPU/device identity and
bit-identical repeated observer records.

## Disclosed qualifications

All six non-serializing four-tick `L=321` qualifications completed before
lock:

| arm | rows | aggregation | maximum support | discarded L1 | moment residual | runtime |
|---|---:|---:|---:|---:|---:|---:|
| face_a | 5 | pass | 36 | 0 | 0 | 43.04 s |
| face_b | 5 | pass | 36 | 0 | 0 | 37.71 s |
| edge_a | 5 | pass | 36 | 0 | `1.8889e-19` | 35.99 s |
| edge_b | 5 | pass | 36 | 0 | `1.8889e-19` | 38.10 s |
| body_a | 5 | pass | 54 | 0 | `7.4962e-20` | 38.88 s |
| body_b | 5 | pass | 54 | 0 | `7.4962e-20` | 37.24 s |

No qualification serialized an FTD-0750 result artifact.

## Frozen dependency hashes

- protocol:
  `C3A3E787A201F1E429E1ED8D8D81B9F06B508A413B41A6B5E2584ED1BFD13385`
- FTD-0750 runner:
  `D7ABFE3E6E8D255F17920CC2510CA9B150389FBC2092C4DE8113E354E9A15963`
- CUDA pipeline header:
  `B7EBCF382BEDED20921267FD30BC3B7AF501BF4DDD933E272D66CC799B79B5C5`
- CUDA pipeline implementation:
  `62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022`
- adversarial ordered-current/observer unit:
  `000D91CEF745F3490968E2965A9AA205BEC9137C492CDEE710FB2ED9A7921EDA`
- inherited guarded FTD-0748 runner:
  `70948B76A359DC01A92DC2BD46289DDA1D318009B51DB63889D95413DDC2EED8`
- inherited FTD-0747 runner:
  `85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14`
- WSL2 FTD-0750 executable:
  `F5E423093D8AB69BCBAAB936F25EABA4EB38E3E5223DE9E4EE3A01A2347EDAD2`
- FTD-0745 baseline CSV:
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`

## Artifact absence and execution rule

Immediately before held-out execution, `engine/results/ftd_0750/` did not
exist. The frozen executable must be invoked exactly once, serially, for
`face a`, `face b`, `edge a`, `edge b`, `body a`, and `body b`, without
rebuild, retuning, or early stopping. Each invocation must serialize four
records. The independent certificate must adjudicate D0 exact replay identity
before the six-record physics conjunction can be constructive.
