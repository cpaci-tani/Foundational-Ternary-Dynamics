# FTD-1001a — Preregistration: zero-action canonical-shear certificate relock v3

**Identifier:** `FTD-1001` (batch relock, part a)
**Date locked:** 2026-08-13
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY RELOCK]`
**Parents:** `FTD-0993` (first execution `95/96`), `FTD-0994` (repaired `96/96`, Outcome B)

## 1. Immutable parent record

- Parent protocol
  `PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md`
  SHA-256 `9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753`.
- Immutable parent proof
  `proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary.py`
  SHA-256 `4F158B7A8847852D1DEF98E29E30999634FF769B27C56C04E1E39C2048029831`.
- Prior repair protocol (FTD-0994)
  `PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_REPAIR_v2.md`
  SHA-256 `0504086A3D106D3A04B90B20467394D6E2F0F3206E126525F29A89B1345851D9`.
- Prior repair wrapper (FTD-0994)
  `proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary_v2.py`
  SHA-256 `19CB58C25C408A56F50D1BB05A99EE9825311269F4FB336C419AAA49F2147CD1`.

The parent protocol and proof remain byte-preserved. The FTD-0993/0994
mathematics, scope, and Outcome B verdict are inherited unchanged.

## 2. Why a relock is required

On 2026-08-13 an owner-authorized documentation-only edit changed the bytes of
one source document the frozen parent proof pins by SHA-256:

- `THEOREM_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_MINIMUM_ACTIVE_APERTURE_v1.md`
  moved from `E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F`
  to `C1AFBB93596DC60AC9C5EDB600843EA0650D1A78ECCC39339A7EAC3ABF75B142`.

The edit was a certificate-count transparency amendment (the disposition line
now reports `94/94 computational, 2 disclosure` instead of a blended headline)
plus a provenance re-pin of the FTD-0992 repair wrapper's own hash citation.
No equation, tolerance, check logic, or physical claim in that document
changed. Because the parent proof is immutable, its stale pinned hash literal
is updated in memory only, alongside the prior repair's already-authorized
substitution.

## 3. Authorized in-memory repairs

The wrapper may apply exactly these two substitutions, each once:

1. **(inherited from FTD-0994, hash-literal typo fix)** replace the truncated
   C18 hash literal
   `2A93D9CFF23DFFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8`
   by the complete
   `2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8`; and
2. **(new, relock)** replace the stale pinned hash literal
   `E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F`
   by the current
   `C1AFBB93596DC60AC9C5EDB600843EA0650D1A78ECCC39339A7EAC3ABF75B142`.

No expression being tested, expected value, classifier, physical claim, or
scope statement may change. The repaired source exists only in memory during
the wrapper execution.

## 4. Integrity gates

The wrapper must verify:

- both parent hashes and both prior-repair hashes before execution;
- this relock protocol exists;
- every old fragment occurs exactly once and every replacement is absent;
- exactly two authorized substitutions occur;
- the repaired inherited certificate exits zero with `96/96` and Outcome B;
- the parent protocol and proof bytes remain unchanged; and
- the wrapper reports its own integrity count and fails closed.

## 5. Classifier

- **Outcome B:** all integrity gates pass and the inherited FTD-0993/0994
  result stands unchanged under the refreshed source pin.
- **Outcome D:** any hash, occurrence, inherited check, byte-preservation, or
  scope gate fails.

No engine mutation, numerical search, parameter scan, fit, or formula
substitution is authorized.
