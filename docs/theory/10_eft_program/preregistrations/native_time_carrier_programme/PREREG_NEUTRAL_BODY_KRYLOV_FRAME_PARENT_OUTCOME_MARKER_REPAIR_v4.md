# Pre-registration — Neutral-body Krylov-frame parent outcome-marker repair v4

**Identifier:** `FTD-0969`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0968 wrapper repaired the two coordinate-symbol literals
and executed the complete chain. The inherited FTD-0966 mathematical
certificate passed `75/75`, and FTD-0967 implementation integrity passed
`19/19`. FTD-0968 nevertheless returned Outcome D at `19/20` because its
verifier searched for `FTD-0966 OUTCOME B`, while the immutable parent prints
`OUTCOME B - exact conditional regional frame; moving production open`.

This is a verifier-marker mismatch only. No mathematical check, replacement,
definition, or conclusion failed. FTD-0966 through FTD-0968 remain unchanged.

## 2. Frozen chain

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C` |
| `proof_neutral_body_krylov_frame_handed_complex_structure.py` | `794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58` |
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_CERTIFICATE_IMPLEMENTATION_REPAIR_v2.md` | `8D754A510DDF2399DF9243E0F9B8FAEDDF3EEAE8646D0EF5BC2A617DCBB7DA9F` |
| `proof_neutral_body_krylov_frame_handed_complex_structure_v2.py` | `BF416D09B3A89A6C93863D40DE5D2F8E364443673FC363EEDAA6284EF266734F` |
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_TWO_SITE_SYMBOL_REPAIR_v3.md` | `55DB0E19370B743199E40ADF863DC4E9B90DB93A5FDC5196DB6BDCCC5B061122` |
| `proof_neutral_body_krylov_frame_handed_complex_structure_v3.py` | `555FB4C627D585E01D3F7BB9E5E4F4F5A13E4FA95E4EB309217746F4BF08D4CF` |

## 3. Sole permitted repair

The v4 wrapper may replace exactly one verifier predicate in the FTD-0968
wrapper:

```text
"FTD-0966 OUTCOME B" in inherited
```

with the actual stable parent marker:

```text
"OUTCOME B - exact conditional regional frame" in inherited
```

The label and all executable mathematical or symbolic content remain
unchanged.

## 4. Integrity gates

- every frozen input must match its recorded hash;
- the old predicate must occur exactly once and the replacement predicate
  must be absent before repair;
- exactly one in-memory substitution must occur;
- the repaired FTD-0968 wrapper must exit zero and report exactly `20/20` with
  Outcome B;
- nested reports must retain FTD-0966 `75/75` and FTD-0967 `19/19`;
- all frozen hashes must remain unchanged after execution; and
- no engine or production file may be written.

Failure of any gate yields Outcome D. Success licenses only the conditional
regional snapshot theorem. Moving-frame canonical reaction, autonomous
formation, persistence across degeneracy, and production integration remain
open.
