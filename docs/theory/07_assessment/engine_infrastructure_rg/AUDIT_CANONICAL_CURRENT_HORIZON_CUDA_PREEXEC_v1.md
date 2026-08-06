# FTD-0748 canonical net-current horizon CUDA pre-execution audit v1

**Status:** `[FROZEN — HELD-OUT FULL EXECUTION NOT YET STARTED]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46`

## Scope

FTD-0748 is the observer-corrected successor to FTD-0747. It retains the
unchanged selected reciprocal `(s,C,F)` evolution, WSL2 CUDA field pipeline,
host implicit matter solve, physical thresholds, and three registered ray
arms. It replaces exact comparison of raw sparse-current container length with
the canonical net support of periodically wrapped, coefficient-aggregated
oriented faces. The complete ungated raw current remains the source applied to
the field.

This is a disclosed protocol correction, not a blind independent physics
replication. FTD-0747 outcomes and the pre-lock current-support probes were
available when the corrected observer and gates were defined.

## Pre-lock evidence

The canonical aggregation observer passed unit checks for coefficient
splitting, periodic-image equivalence, exact opposite cancellation, and
explicit tolerance accounting. The three pre-lock eight-tick qualification
arms did not serialize campaign artifacts and gave:

| arm | maximum net support | discarded L1 | maximum moment residual | runtime |
|---|---:|---:|---:|---:|
| face | 36 | 0 | 0 | 55.86 s |
| edge | 36 | 0 | `2.795e-19` | 64.67 s |
| body | 54 | 0 | `2.744e-19` | 52.70 s |

The locked static protocol-conformance certificate passes `48/48`. After the
single authorized post-lock rebuild, the focused regression set passes `3/3`:
`quadratic_coat_face_current`, `cuda_matched_field_pipeline`, and
`causal_horizon_csv_loader`. Invoking the successor without an arm performs a
non-serializing usage smoke check.

## Frozen dependency hashes

- protocol:
  `D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46`
- FTD-0748 runner:
  `59E33BAB8FAB4DEBAABC9DDE87EE7D214C5B2196D0D60A8D9F3BD87B990C4446`
- aggregation header:
  `77E67E4EBC4B27F7A70B8289195EA2D3A398A862C04DA29041C4ED33B8DA7409`
- aggregation implementation:
  `DD39B5776D74F9D942F0F5BA7518ED2D4B97E42927E361D314E5C7D6F1D0F1D0`
- inherited FTD-0747 runner dependency:
  `85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14`
- frozen WSL2 FTD-0748 executable:
  `FF2004DEC314FB4167ADBC26EBD1C8C8673393C40201C659CCB0B211085CEE8B`
- unchanged frozen WSL2 FTD-0747 executable:
  `907B873ABF89F352FD340BBD874AC0CB94282F0BDEABCE81798F85B026E9A01B`
- FTD-0745 baseline CSV:
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`

## Artifact absence and execution rule

Immediately before held-out execution,
`engine/results/ftd_0748/` did not exist. The frozen executable must now be
invoked exactly once for each full `face`, `edge`, and `body` arm, serially and
without rebuild, retuning, early stopping, or arm-specific modification. Each
arm must serialize one 313-row main CSV/JSON pair and one 313-row support
CSV/JSON pair. An independent certificate will adjudicate the frozen records.

