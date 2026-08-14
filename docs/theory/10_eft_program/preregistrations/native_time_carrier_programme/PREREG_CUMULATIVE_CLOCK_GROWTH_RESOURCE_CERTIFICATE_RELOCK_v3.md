# FTD-1001c — Preregistration: cumulative clock-growth resource certificate relock v3

**Identifier:** `FTD-1001` (batch relock, part c)
**Date locked:** 2026-08-13
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY RELOCK]`
**Parents:** `FTD-0998` (first execution `89/91`), `FTD-0999` (repaired
`91/91` plus integrity `11/11`, Outcome B)

## 1. Immutable parent record

- Parent protocol
  `PREREG_CUMULATIVE_CLOCK_GROWTH_ENERGY_RESERVE_AND_BACKPRESSURE_v1.md`
  SHA-256 `6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052`.
- Immutable parent proof
  `proof_cumulative_clock_growth_energy_reserve_and_backpressure.py`
  SHA-256 `E8257678700C732214D1A44E69FF5FCBEB31696BB86E6A2F5DB8F611534CD6F0`.
- Prior repair protocol (FTD-0999)
  `PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_REPAIR_v2.md`
  SHA-256 `28525592D68887E4795B9E4F9664565C72969DD7828EE22F68970D1C2173EB70`
  (unchanged).
- Prior repair wrapper (FTD-0999)
  `proof_cumulative_clock_growth_energy_reserve_and_backpressure_v2.py`
  SHA-256 `3FC2DA55EF1DAC8C48CE65AF5B76870981B75F15C191C45D6B62BCD6306961E3`.

The parent protocol and proof remain byte-preserved. The FTD-0998/0999
mathematics, scope, and Outcome B verdict are inherited unchanged.

## 2. Why a relock is required

On 2026-08-13 an owner-authorized documentation-only edit changed the bytes
of one source document the frozen parent proof pins by SHA-256:

- `THEOREM_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_AND_QUIESCENT_SEAM_BOUNDARY_v1.md`
  moved from `9418AA0841B3122A65B3276525A7B9DEDE89C31FEA563AC4055B8F50EF262110`
  to `95357F142A94FBA2B4A3441429C6C4B81818D19342268092E51622AB34ED2B00`.

The edit was a certificate-count transparency amendment (the disposition line
now reports `64/64 computational, 35/35 disclosure` instead of the blended
`99/99` headline) plus a provenance re-pin of the FTD-0997 certificate
script's own hash citation. No equation, tolerance, check logic, or physical
claim in that document changed. Because the parent proof is immutable, its
stale pinned hash literal is updated in memory only, alongside the prior
repair's already-authorized marker substitution.

## 3. Authorized in-memory repairs

The wrapper may apply exactly these substitutions:

1. **(inherited from FTD-0999, unchanged)** replace both occurrences of the
   source-census marker `"rest-offset-free accounted channels"` by
   `"rest-offset-free accounted // channels"` (the normalized-C++-comment
   representation fix); and
2. **(new, relock, once)** replace the stale pinned hash literal
   `9418AA0841B3122A65B3276525A7B9DEDE89C31FEA563AC4055B8F50EF262110`
   by the current
   `95357F142A94FBA2B4A3441429C6C4B81818D19342268092E51622AB34ED2B00`.

No expression being tested, expected value, classifier, physical claim, or
scope statement may change. The repaired source exists only in memory during
the wrapper execution.

## 4. Integrity gates

The wrapper must verify:

- both parent hashes and both prior-repair hashes before execution;
- this relock protocol exists;
- the marker occurs exactly twice, the hash fragment exactly once, and every
  replacement is absent beforehand;
- the repaired inherited certificate exits zero with `91/91` and Outcome B;
- the parent protocol and proof bytes remain unchanged; and
- the wrapper reports its own integrity count and fails closed, carrying
  forward the FTD-0999 computational/disclosure breakdown report
  (`74/74` computational, `17` disclosure).

## 5. Classifier

- **Outcome B:** all integrity gates pass and the inherited FTD-0998/0999
  result stands unchanged under the refreshed source pin.
- **Outcome D:** any hash, occurrence, inherited check, byte-preservation, or
  scope gate fails.

No engine mutation, numerical search, parameter scan, fit, or formula
substitution is authorized.
