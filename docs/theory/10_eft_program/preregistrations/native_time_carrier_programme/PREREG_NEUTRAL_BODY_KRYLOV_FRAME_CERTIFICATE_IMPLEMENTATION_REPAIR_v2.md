# Pre-registration — Neutral-body Krylov-frame certificate implementation repair v2

**Identifier:** `FTD-0967`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The first FTD-0966 certificate process was terminated without a verdict after
more than 150 CPU-seconds. It expanded a generic twelve-coordinate Krylov
determinant twice and then requested determinant simplification separately for
all 48 signed-cubic matrices. No mathematical or scope failure was observed;
no console verdict was emitted.

A pre-execution marker audit also found one proof-source phrase that is not
verbatim in the frozen FTD-0907 theorem: the proof requested `neutral ternary
dipole defines a polar axis`, while the source says `is therefore a native
polar axis`.

The parent protocol and proof remain unchanged.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C` |
| `proof_neutral_body_krylov_frame_handed_complex_structure.py` | `794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58` |

All three theorem-source hashes inherited from FTD-0966 remain frozen.

## 3. Permitted in-memory repairs

The wrapper may perform exactly five substitutions:

1. replace the nonexistent FTD-0907 source marker with the actual phrase
   `is therefore a native polar axis`;
2. replace the generic translated-body determinant expansion by direct exact
   centroid/dipole/covariance algebra; infer Krylov-determinant invariance only
   after both `d` and `C` are proved unchanged;
3. omit construction of the generic six-symbol determinant polynomial, which
   is not otherwise needed;
4. replace each direct transformed-determinant simplification by the stronger
   exact full-matrix identity `K'=QK`; determinant covariance then follows from
   `det(QK)=det(Q)det(K)`; and
5. evaluate the two-site minimum proof from its centered vectors directly,
   avoiding the generic body helper's unnecessary determinant factorization.

These substitutions change no source hash, definition, equation, expected
classifier, mathematical gate, or scope conclusion. They remove redundant
computer algebra only.

## 4. Integrity gates

- all three repair inputs must match their frozen hashes;
- every old anchor must occur exactly once and every replacement anchor must
  be absent before repair;
- exactly five in-memory substitutions must occur;
- the repaired inherited certificate must exit zero, report no failed checks,
  and retain Outcome B;
- the parent protocol and proof hashes must remain unchanged after execution;
- no engine or production file may be written.

Failure of any integrity gate yields Outcome D. Success licenses only the
FTD-0966 conditional snapshot theorem; moving-frame reaction, formation, and
production remain open.
