# FTD-0747 CUDA pre-execution audit v2

**Status:** `[AUTHORIZED — HELD-OUT ARMS NOT YET EXECUTED]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`1FB4A49897D8FEC333C686A54D44A90EA6E51D799EDBD9168F8D313287F4FD5F`

## Frozen artifacts

- Protocol:
  `docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CUDA_v2.md`
- Runner source:
  `engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp`
- Runner source SHA-256:
  `85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14`
- WSL2 Release executable SHA-256:
  `907B873ABF89F352FD340BBD874AC0CB94282F0BDEABCE81798F85B026E9A01B`
- FTD-0745 baseline SHA-256:
  `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C`

The static protocol certificate passed 48/48 checks. The locked parity target
passed field, regional observer, Gauss, modified energy, local momentum,
quadratic-spline momentum, repeated-CUDA determinism, and complete implicit
transaction equivalence gates. Its largest complete-transaction diagnostic
difference was `6.94e-18` against the registered `2e-10` limit.

## Large-volume qualification

The `L=321` CUDA allocation and field/observer/diagnostic smoke completed with
no swap. Pre-lock short causal histories completed on face, edge, and body
rays. Across those histories:

- maximum common residual: `5.33e-14`;
- maximum total-energy residual: `7.85e-17`;
- maximum recoil defect: `3.13e-17`.

These qualifications created no `engine/results/ftd_0747/` artifacts and are
not admissible held-out physics evidence.

## Authorization check

Immediately after the locked rebuild, `engine/results/ftd_0747/` was absent.
Exactly one invocation each of `face`, `edge`, and `body` is authorized. An arm
must serialize its own CSV/JSON only after reaching the runner's serialization
point. A process interruption before serialization is an administrative abort,
not a physics result.
