# FTD-1001b — Preregistration: crossing-matched clock-growth certificate relock v3

**Identifier:** `FTD-1001` (batch relock, part b)
**Date locked:** 2026-08-13
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY RELOCK]`
**Parents:** `FTD-0995` (first execution `84/88`, Outcome D), `FTD-0996`
(repaired `88/88` plus integrity `17/17`, Outcome B)

## 1. Immutable parent record

- Parent protocol
  `PREREG_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md`
  SHA-256 `B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F`.
- Immutable parent proof
  `proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth.py`
  SHA-256 `17DE90F5BBEFD1BDEFC22AACB236C024FBE8446BD5DE765AA7F95B79EDD87574`.
- Prior repair wrapper (FTD-0996)
  `proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth_v2.py`
  SHA-256 `9104D6F3FD842C8BF09C7F35BC080BCCCA96EFCC0F4022CAAAF9DF3846B130E2`.

The parent protocol and proof remain byte-preserved. The FTD-0995/0996
mathematics, scope, and Outcome B verdict are inherited unchanged.

## 2. Why a relock is required

Two owner-authorized documentation-only edits of 2026-08-13 changed the bytes
of files this chain pins by SHA-256:

1. The FTD-0996 repair protocol itself,
   `PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md`, was
   amended to correct its own scope self-description: its four authorized
   substitutions are now categorized as two representational-only items (1
   and 4) and two domain-completion items (2 and 3, the `sigma^2 -> 1`
   substitutions, which do change the tested expression by supplying the
   orientation-sign domain restriction the frozen predicate omitted). The
   amendment corrects the prior blanket claim that "no expression being
   tested may change" — a description found self-contradictory on audit. The
   **authorized substitution list itself is unchanged**; only its
   characterization was corrected. The document moved from
   `854C1EDA934DA8CDFA1B0C2649EF9CE2A20C4D6A30731D28A2C285BDB7379554` to
   `36B7E3F0C3645E28605633BD2A46E0F8EB64C9D852BEDAF513F883C3DDE8B12D`.
2. The parent proof's pinned source document
   `THEOREM_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_MINIMUM_ACTIVE_APERTURE_v1.md`
   received a certificate-count transparency amendment (disposition line now
   `94/94 computational, 2 disclosure`) plus a provenance re-pin, moving from
   `E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F` to
   `C1AFBB93596DC60AC9C5EDB600843EA0650D1A78ECCC39339A7EAC3ABF75B142`.

No equation, tolerance, check logic, or physical claim changed in either
document.

## 3. Authorized in-memory repairs

The wrapper pins the amended repair protocol at its current hash
(`36B7E3F0...B12D`) and may apply exactly these five substitutions to the
parent proof, each once:

1–4. **(inherited from FTD-0996, unchanged)** the four substitutions declared
   in `PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md` §2 —
   the frozen-marker spelling fix, the two `sigma_symbol**2 -> 1`
   domain-completion substitutions, and the elementwise `sp.simplify` of
   `K * uniform_q`.
5. **(new, relock)** replace the stale pinned hash literal
   `E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F`
   by the current
   `C1AFBB93596DC60AC9C5EDB600843EA0650D1A78ECCC39339A7EAC3ABF75B142`.

No expression being tested (beyond the already-authorized items 2–3 as
recharacterized above), expected value, classifier, physical claim, or scope
statement may change. The repaired source exists only in memory during the
wrapper execution.

## 4. Integrity gates

The wrapper must verify:

- both parent hashes, the amended repair-protocol hash, and the prior
  wrapper hash before execution;
- this relock protocol exists;
- every old fragment occurs exactly once and every replacement is absent;
- exactly five authorized substitutions occur;
- the repaired inherited certificate exits zero with `88/88` and Outcome B;
- the parent protocol and proof bytes remain unchanged; and
- the wrapper reports its own integrity count and fails closed.

## 5. Classifier

- **Outcome B:** all integrity gates pass and the inherited FTD-0995/0996
  result stands unchanged under the refreshed pins.
- **Outcome D:** any hash, occurrence, inherited check, byte-preservation, or
  scope gate fails.

No engine mutation, numerical search, parameter scan, fit, or formula
substitution is authorized.
