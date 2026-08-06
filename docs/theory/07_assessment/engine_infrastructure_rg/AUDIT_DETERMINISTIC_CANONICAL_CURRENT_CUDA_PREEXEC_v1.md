# FTD-0749 deterministic canonical-current CUDA pre-execution audit v1

**Status:** `[FROZEN — SIX HELD-OUT EXECUTIONS NOT YET STARTED]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83`

## Frozen correction

FTD-0749 uses a research-only CUDA deposition path. It aggregates the complete
ungated sparse current to unique periodic oriented faces with zero filtering
tolerance, uploads one coefficient per face, and performs one non-atomic
device update per component/index. The legacy atomic API and every production
caller remain unchanged.

The static protocol-conformance certificate passes `50/50`. The exact
duplicate-face CUDA unit test and focused neighboring regressions pass `4/4`:
`cuda_canonical_current_deposition`, `cuda_matched_field_pipeline`,
`quadratic_coat_face_current`, and `causal_horizon_csv_loader`.

## Disclosed qualifications

Non-serializing eight-tick `L=321` qualifications completed before lock:

| arm | rows | aggregation | maximum support | discarded L1 | moment residual | runtime |
|---|---:|---:|---:|---:|---:|---:|
| face_a | 9 | pass | 36 | 0 | 0 | 45.12 s |
| edge_a | 9 | pass | 36 | 0 | `2.7953e-19` | 41.89 s |
| body_a | 9 | pass | 54 | 0 | `2.7444e-19` | 43.64 s |

No qualification serialized an FTD-0749 result artifact.

## Frozen dependency hashes

- protocol:
  `6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83`
- FTD-0749 runner:
  `A1D8E0FA9DCFCF07E99DE87FB1CCDC0653A373BE23C34ED28C404A80B76C83B3`
- CUDA pipeline header:
  `FB14626C32BC1F8EA4667E9FFE3982455E319F469C3F418831EF38A55A8DE312`
- CUDA pipeline implementation:
  `86C1E5C9A4F12F761258706026CA8A8EDB2B061BA6319636980F954A7C046D9D`
- canonical aggregation header:
  `77E67E4EBC4B27F7A70B8289195EA2D3A398A862C04DA29041C4ED33B8DA7409`
- canonical aggregation implementation:
  `DD39B5776D74F9D942F0F5BA7518ED2D4B97E42927E361D314E5C7D6F1D0F1D0`
- inherited guarded FTD-0748 runner:
  `70948B76A359DC01A92DC2BD46289DDA1D318009B51DB63889D95413DDC2EED8`
- inherited FTD-0747 runner:
  `85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14`
- duplicate-face unit source:
  `75DA11585CBA008C44A639BBE09F754394327B399D3BC88C55782CF2E4715039`
- WSL2 FTD-0749 executable:
  `13ECEF1C337BF1E50DD783D936F9263AB5F77DAD0697120B2CB7FB2C2280053D`
- FTD-0745 baseline CSV:
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`

## Artifact absence and execution rule

Immediately before held-out execution, `engine/results/ftd_0749/` did not
exist. The frozen executable must be invoked exactly once, serially, for
`face a`, `face b`, `edge a`, `edge b`, `body a`, and `body b`, without
rebuild, retuning, or early stopping. Each invocation must serialize four
records. The independent certificate must adjudicate D0 replay identity before
the six-record physics conjunction can be constructive.

