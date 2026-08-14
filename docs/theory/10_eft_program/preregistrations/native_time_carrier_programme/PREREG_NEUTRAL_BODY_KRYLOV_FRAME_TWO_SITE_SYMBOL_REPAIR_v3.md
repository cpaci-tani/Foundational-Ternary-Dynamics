# Pre-registration — Neutral-body Krylov-frame two-site symbol repair v3

**Identifier:** `FTD-0968`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0967 wrapper completed quickly but returned Outcome D. Its
inherited FTD-0966 certificate reported `75` checks, `72` passed, and `3`
failed. All three failures belonged to the same two-site gate G4:

- two-site dipole;
- rank-one covariance form; and
- vanishing two-site Krylov determinant.

The remaining mathematical gates G1--G3 and G5--G10 passed. FTD-0967 repair
integrity therefore reported `16/19` because the inherited exit, no-failure,
and Outcome-B checks correctly refused promotion.

Inspection identifies an implementation-only symbol-construction error in
the FTD-0967 replacement block. SymPy interprets `r10:3` and `r20:3` as range
syntax rather than as three coordinates for each endpoint. The intended
symbols are `r1_0:3` and `r2_0:3`. The FTD-0966 parent and FTD-0967 wrapper
remain unchanged as immutable provenance.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C` |
| `proof_neutral_body_krylov_frame_handed_complex_structure.py` | `794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58` |
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_CERTIFICATE_IMPLEMENTATION_REPAIR_v2.md` | `8D754A510DDF2399DF9243E0F9B8FAEDDF3EEAE8646D0EF5BC2A617DCBB7DA9F` |
| `proof_neutral_body_krylov_frame_handed_complex_structure_v2.py` | `BF416D09B3A89A6C93863D40DE5D2F8E364443673FC363EEDAA6284EF266734F` |

## 3. Permitted in-memory repairs

The v3 wrapper may perform exactly two substitutions in the FTD-0967
`NEW_G4` replacement block:

1. replace `sp.symbols("r10:3", real=True)` by
   `sp.symbols("r1_0:3", real=True)`; and
2. replace `sp.symbols("r20:3", real=True)` by
   `sp.symbols("r2_0:3", real=True)`.

The old literals in FTD-0967's `OLD_G4` anchor must remain unchanged because
they correctly identify the immutable parent text. No equation, definition,
expected result, scope boundary, or theorem source may change.

## 4. Integrity gates

- all frozen parent and repair inputs must match their recorded hashes;
- each complete old `NEW_G4` line must occur exactly once and each replacement
  line must be absent before repair;
- exactly two in-memory substitutions must occur;
- the repaired FTD-0967 wrapper must exit zero;
- its inherited certificate must report exactly `75/75`, no failures, and
  Outcome B;
- its own repair layer must report exactly `19/19`, no failures, and Outcome B;
- every frozen input hash must remain unchanged after execution; and
- no engine or production file may be written.

Failure of any gate yields Outcome D. Success licenses only the conditional
regional snapshot result preregistered by FTD-0966. It does not establish a
state-dependent canonical moving frame, reaction/work bookkeeping, a
formation law, or production implementation.
