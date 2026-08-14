# PRE-REGISTRATION — Relative action/orientation transducer certificate repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0860`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 36/36]`  
**Parent:** `FTD-0859`, frozen first execution `31/36 FAIL`; no theorem  
**Inherited physics protocol:**
[`PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_v1.md`](PREREG_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_v1.md)  
**Parent pre-run protocol SHA256:**
`FC558B798D556812A12FE239264B99855A7753EDD8DCAA66891B906BBB4DD351`  
**Frozen invalid verifier SHA256:**
`E9B2C1D33730DA6A72EE4F1446FCCA7C0467E33D5A5E6456375CA76BDDE825D8`

## 1. Failure diagnosis

The parent passed all seven frozen source hashes and `31/36` checks. Independent
read-only evaluation established that every failed gate was verifier-only:

1. **C13:** `solve` returned the algebraically equal expanded rational
   `(2B+p^2+q^2)/(p^2+q^2)`; the verifier used structural list equality.
2. **C15:** the exact Poisson bracket simplified to `s^2`; the substitution was
   applied before SymPy formed that product. Both registered branches `s=+1`
   and `s=-1` give one.
3. **C27:** the inverse-image check used `+z` where its stated equality requires
   subtraction of `z`.
4. **C29:** the protocol contains the complete circle-image proof, but the
   source markers omitted `the`, used lower-case `continuous`, and ignored a
   line wrap.
5. **C36:** the scope text is present exactly, but the phrase `biological
   hemispheres` crosses a Markdown line boundary.

No equation, hypothesis, source hash, map, topology claim, production fact,
outcome, denominator, or scope ceiling failed.

## 2. Frozen repair scope

The wrapper may perform exactly five in-memory transformations on the frozen
parent verifier:

1. compare C13's sole solution after symbolic subtraction and simplification;
2. evaluate C15 on the two registered sign branches;
3. replace the erroneous `+z` by `-z` in C27;
4. normalize protocol whitespace and use the exact C29 phrases; and
5. use the same normalized protocol text for the unchanged C36 phrases.

The wrapper must hash-check the invalid parent, require every old fragment
exactly once, apply every transformation exactly once in memory, and execute
the resulting certificate without editing the parent.

## 3. Acceptance and firewalls

- **Repaired Outcome B:** all inherited `36/36` gates pass.
- **Still invalid:** wrapper/source mismatch, a transformation outside the five
  registered repairs, or any inherited gate failure.

No production change is authorized. No quarter-turn uniqueness beyond the
registered selection, nonzero-vacuum claim, local-mode claim, reversibility
claim, Born/Bell claim, biological claim, CM/substrate identity, `G*` cadence,
or completeness promotion is permitted.

## 4. Locked executable

- Wrapper:
  `scripts/proofs/proof_relative_action_orientation_transducer_v2.py`
- Wrapper SHA256:
  `E174EDE70863EC68FA27765EFAF61C8C849DD66EC7B91D21294ED4E23D6BC53B`
- Required command:
  `python scripts/proofs/proof_relative_action_orientation_transducer_v2.py`
- Required denominator: exactly `36/36`.

## 5. Recorded outcome

- Pre-run repair-protocol SHA256:
  `9971EEE4B0E532F7732AAE35151DF284C55EF95DBD1C26D8B42CE7F7A9BEC11F`.
- First execution: `36/36 PASS` after exactly five in-memory verifier repairs.
- Frozen outcome: **repaired Outcome B — exact nonzero-carrier action pump
  plus one-pair faithfulness boundary; production incomplete**.
- Theorem of record:
  [`THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md).
