# FTD-0923 — Ternary-dipole-core evanescent `C4` certificate repair v2

**Identifier:** `FTD-0923`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED]`  
**Parent:** FTD-0922  
**Scope:** correct only the lattice domain used by the two compact-source
support/norm checks; inherit every physical equation, source, outcome, and
scope ceiling

## 1. Invalid parent execution

The first immutable FTD-0922 certificate run returned `71/74`. Every
resolvent, tail, C18 band, exact periodic field, rotation, kick--drift,
circulation, source-work, continuity-failure, production-marker, and scope
gate passed. Two checks failed, and the combined discriminator failed
dependently:

```text
FAIL  central dipole gradient has exactly eleven vector-support sites
FAIL  central dipole gradient norm squared is seven halves
FAIL  combined Outcome A discriminator
```

The locked parent protocol states these two values for the **uncontained**
dipole. The invalid verifier evaluated them on the separate `L=4` periodic
consistency witness. Periodic identification merges the `x=+2` and `x=-2`
source lobes, giving ten vector-support sites and norm squared four. The
infinite compact source has the locked values eleven and `7/2` exactly.

No theorem is booked from FTD-0922's invalid run.

## 2. Frozen parent artifacts

| Artifact | SHA-256 |
|---|---|
| FTD-0922 parent protocol | `59B061102D498727E8099F6109464A0B8A9439FD014BC8176888524D40AD9BC7` |
| FTD-0922 invalid certificate | `2FEC105772F6396E49C3E2C47ADA2F2792438C7ADACF64D68AC4BE38C73CECEE` |

All seven parent theory/production source hashes remain inherited and must
still pass inside the repaired execution.

## 3. Sole permitted repair

The repair wrapper may replace only the parent block that computes
`source_support` and `source_norms` from the periodic `gradients` array.

The replacement must:

1. construct the uncontained source

   \[
   s_0=\delta_{(1,0,0)}-\delta_{(-1,0,0)};
   \]

2. enumerate exactly the finite candidate set one central-difference step
   from those two sites;
3. evaluate all three central-gradient components with exact SymPy rationals;
4. count nonzero vector sites and sum component squares; and
5. feed those exact values into the two existing check labels.

The repaired verifier must retain the periodic `L=4` gradients for all
rotation, resolvent, recurrence, circulation, work, and continuity arms.

No expected physics value, operator, field profile, outcome rule, source
hash, or scope statement may change. The total check count remains `74`.

## 4. Outcome rule

- If the exact one-block repair yields inherited `74/74`, register FTD-0922
  Outcome A through this repaired execution and book the scoped reference
  theorem.
- If any other check fails, the execution remains invalid and no theorem is
  booked.

The repair does not license source autonomy, continuity, reaction, switching
work, formation, storage, `G*`, gamma, Born/Bell, context, or production
integration.
