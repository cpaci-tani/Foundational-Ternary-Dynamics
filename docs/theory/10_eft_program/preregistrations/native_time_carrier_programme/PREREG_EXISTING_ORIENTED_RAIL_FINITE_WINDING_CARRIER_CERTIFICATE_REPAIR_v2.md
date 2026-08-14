# Pre-registration — Existing oriented-rail finite winding carrier certificate repair v2

**Identifier:** `FTD-0961`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Expected classifier:** inherited `Outcome B`

## 1. Immutable parent record

The FTD-0960 protocol and first certificate are preserved byte for byte:

| Artifact | SHA-256 |
|---|---|
| `PREREG_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_BOUNDARY_v1.md` | `B8BDCCCDEB5ECFE4FE2B9CAAD1C00AAF69C5E5F6CD0E4266866FBDF79A6ADDBA` |
| `proof_existing_oriented_rail_finite_winding_carrier_boundary.py` | `EAF1890622606B584EB3473FB6D5444C52CAB79B38AA8E70A808AE28CA6A28C8` |

The first immutable execution reported `55/60`, Outcome D. Every encoding,
inverse, winding, locality, covariance, energy, capacity, parity-permutation,
compact-counter, target-leakage, and scope gate passed. Five source-marker
checks failed:

1. `No completed infinite rail is assumed` crossed one Markdown line break.
2. `Loading an event port is a separate transaction` crossed one Markdown
   line break.
3. The fresh-front marker contained a literal verifier-only `+` after `\n`.
4. The support/energy marker contained a literal verifier-only `+` after `\n`.
5. The compact-carry marker contained a literal verifier-only `+` after `\n`.

## 2. Authorized repair

The wrapper may make exactly five in-memory source substitutions:

- replace the first marker by the exact two-line source form;
- replace the second marker by the exact two-line source form; and
- remove the verifier-only `+` after `\n` in the three named multiline
  markers.

The wrapper may not alter either parent file on disk. It may not change the
candidate encoding, update, inverse, winding readout, energy, capacity, source
hashes, number of inherited checks, outcome table, or scope.

## 3. Repair integrity gates

The repair certificate must verify:

1. both parent hashes;
2. this repair-protocol hash;
3. every old anchor occurs exactly once;
4. every replacement is absent before substitution;
5. exactly five substitutions occur in memory;
6. both parent files are unchanged after execution;
7. the inherited certificate reports all `60/60` gates passing; and
8. the inherited classifier remains Outcome B.

Any other edit or any inherited failure is Outcome D. This repair licenses no
new theorem beyond the FTD-0960 protocol.
