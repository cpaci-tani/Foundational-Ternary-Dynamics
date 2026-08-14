# Pre-registration — C4 field-cocycle certificate source-marker repair v2

**Identifier:** `FTD-0974`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0973 certificate completed `64` checks and returned Outcome
D at `63/64`. Every exact cocycle, gauge, symmetry, time-reversal,
underdetermination, symplectic-minimum, Hamiltonian, closed-flow, energy,
reaction, and scope gate passed. The sole failure was a source marker:

- requested: `positive complete-square phase connection`;
- frozen FTD-0963 theorem: `exact positive autonomous connection Hamiltonian`.

No mathematical identity, source hash, definition, classifier, or scope
conclusion failed. The FTD-0973 protocol and proof remain unchanged.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md` | `6328CD0FCA455BB135F1642D9A85C4BADFB63C3A9DA070B3BC8765434E4F1E87` |
| `proof_c4_field_cocycle_minimum_canonical_suspension.py` | `B83F616681E1E27D2F9AE6F2F935403032E5FB536E8B6942D7157DB909C2A3B8` |

All four theorem-source hashes inherited from FTD-0973 remain frozen.

## 3. Sole permitted repair

Replace exactly one in-memory string literal:

```text
"positive complete-square phase connection"
```

with

```text
"exact positive autonomous connection Hamiltonian"
```

No executable mathematical expression or firewall may change.

## 4. Integrity gates

- the parent protocol, parent proof, and this repair protocol match their
  frozen hashes;
- the old marker occurs exactly once and the replacement is absent before
  repair;
- exactly one in-memory substitution occurs;
- the inherited certificate exits zero, reports `64/64`, and retains Outcome
  B;
- all frozen hashes remain unchanged; and
- no engine or production file is written.

Failure of any gate yields Outcome D. Success licenses only the exact cocycle
classification and selected minimum canonical suspension. Physical identity,
formation, switching, `G*`, Born/Bell, hiding, and production remain open.
