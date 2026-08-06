# Audit — finite-support outgoing-tail formation pre-execution conformance v1

**Identifier:** FTD-0739  
**Status:** `[PRE-EXECUTION CONFORMANCE PASS — NO PHYSICS RESULT]`  
**Date:** 2026-07-29  
**Locked protocol SHA-256:**
`9AA9B806877F07F9567291E73B58E6157CFBDAE425DE843B85D3753CECA7868E`

## 1. Scope

The owner explicitly resumed FTD-0739 after the FTD-0740--0744 planning pause.
Before any new physics execution, the implemented runner was compared with the
unchanged locked protocol. This record contains no trajectory or physical
verdict.

## 2. Defects found before execution

Four protocol-conformance defects were corrected before a result existed:

1. Three C++ verdict strings differed from the exact locked tokens. They now
   use `FINITE_SUPPORT_FORMATION_EXECUTION_INVALID`,
   `FINITE_SUPPORT_NO_DURABLE_NEGATIVE_CORE_ALL_RAYS`, and
   `FINITE_SUPPORT_CAPTURE_ENERGY_LEDGER_MISMATCH` verbatim.
2. The polarity metric compared only separation, pair energy, total field
   energy, and radius-12 outside energy. It now compares every persisted scalar
   and discrete row field apart from the deliberately conjugated polarity
   label.
3. The bound-control negative/inside gate was evaluated only on tick zero and
   forward states. It now also checks every persisted reverse-root state.
4. Per-step total-energy residual was already included in the common maximum
   residual, but the locked `1e-8` energy gate was not separately named in the
   infrastructure conjunction. The explicit gate is now present.

These changes strengthen protocol observation and serialize the registered
verdicts. They change no initial state, field preparation, force, potential,
field equation, common-action residual, root, iteration count, tolerance,
volume, horizon, current, event order, or verdict ordering.

## 3. Static certificate

[`proof_finite_support_outgoing_tail_protocol_conformance.py`](../../../scripts/proofs/proof_finite_support_outgoing_tail_protocol_conformance.py)
checks the protocol hash, embedded hash, volume, horizon, support, shells,
options, matrix, forward/reverse counts, physical gates, serialization, exact
verdicts, obsolete-verdict absence, and CTest registration.

Result: **`55/55 PASS`**.

Proof SHA-256:
`AE31AEE88619E554B0A8E36C143AB29C854D6BE3DA5BA8B772417D35AD78F9BE`.

## 4. Frozen implementation hashes

| artifact | SHA-256 |
|---|---|
| campaign source | `F08AD44732B5A51AE3C5ACABD540033224DEC56BF77FEFF5589DE27C8CF62DCC` |
| selected action implementation | `DC9E8FF56E2972FCEA9DDC9BE20C609F2597F1089BB39B6779E96CC565F445FF` |
| compact-preparation qualification | `139E98824A2214FF63D99AC022E99545B20BEB0F77E475A018509D6A20C4CF42` |
| Release executable | `C0E158D171B0BDF822FF2EC173E3810ACFB3B6F47947873C350F7C6E27B263B5` |

The Release executable was built with pinned MSVC toolset `14.44.35207`.

## 5. Focused preconditions

The two required focused CTests were rebuilt and run after the conformance
changes:

```text
matched_regional_energy_transport  PASS
finite_support_pair_preparation    PASS
2/2 passed
```

The preparation test covers compact support, zero crossing flux, Gauss and
Poisson residuals, energy minimality against both orientations of a nonzero
cycle, translation, proper-cubic rotation, polarity conjugation, three
symmetry rays, and failure closure.

## 6. Execution authorization

The runner is conformant and the focused preconditions pass. Exactly one fresh
execution of the five locked histories is authorized. No checkpoint or restart
amendment is used. Any interruption before complete CSV/JSON serialization is
execution-incomplete and supplies no physical evidence.
