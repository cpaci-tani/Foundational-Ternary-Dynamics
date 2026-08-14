# FTD-0916 — Production ternary-plaquette recurrence runner lock v2

**Identifier:** `FTD-0916`  
**Parent:** `FTD-0915`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — RUNNER LOCKED/PRE-RUN]`

## 1. Purpose

This document freezes the compiled observation instrument for the already
locked FTD-0915 production census. It changes no support, transition,
identity, arm, threshold, or outcome definition.

## 2. Frozen artifacts

| Artifact | SHA-256 |
|---|---|
| FTD-0915 protocol | `C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C` |
| `engine/tests/campaign_production_ternary_plaquette_recurrence_census.cpp` | `20E00A0BB988A72FEED7851A854846F3D1F18440CCA91AF9DFFC105A840F301D` |
| `engine/CMakeLists.txt` | `C895673132434DE830A15EE41676A446FCEF6D26D7C3819ED491E536D37BB745` |
| exact Release executable | `8CDCCE805C5721B1266D16B3A0B01D8857A85D42ED93EC3AFBCE8A7849147B64` |

The executable was compiled with the canonical pinned MSVC 14.44 native
build path. The runner forces the CPU backend in every arm.

## 3. Instrument audit

The locked source:

1. enumerates `3 L^3` supports per tick using only the canonical periodic
   lattice indexer;
2. recognizes exactly the four cyclic shifts of `(+1,0,-1,0)`;
3. separates raw exposures from identity-bearing exposures;
4. keys histories by fixed support and signed production particle IDs;
5. classifies all next-tick continuations into the six FTD-0915 classes;
6. requires four consecutive same-direction transitions and direct word
   closure for a full cycle;
7. checks exact dipole, bivector, successor, normal-sign, and time-reversal
   identities for every directed transition;
8. records local energy descriptively without using it as a gate;
9. hashes voxel bytes and RNG state before and after every observation;
10. uses exactly the locked volumes, seeds, ticks, families, and `6/8` cell
    gate; and
11. contains no production write, parameter sweep, `G*`, `gamma`, Born/Bell,
    context, selector, or desired-outcome read.

The output corpus is written only under `engine/results/ftd_0915/`.

## 4. Execution rule

An independent preflight must verify this document, the parent protocol,
every frozen production source, runner source, CMake file, and exact
executable before the first campaign execution. If any lock fails, FTD-0915
is invalid and no A--E result may be issued.

```text
PARENT_PROTOCOL_CHANGED=FALSE
RUNNER_LOCKED_BEFORE_EXECUTION=TRUE
PRODUCTION_TICK_MODIFIED=FALSE
OUTCOME_GATE_CHANGED=FALSE
STATUS=LOCKED_PRE_RUN
```

**LOCKED CONTENT ENDS HERE.**
