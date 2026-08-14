# FTD-0917 — Production plaquette-recurrence raw-telemetry repair v3

**Identifier:** `FTD-0917`  
**Parents:** `FTD-0915`, `FTD-0916`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — TELEMETRY REPAIR LOCKED/PRE-RUN]`

## 1. Failed execution audit

The first execution of the FTD-0916 runner completed its 64 arms and emitted
a processed corpus, but that corpus is **execution-invalid for FTD-0915
adjudication**. It stored word labels, identity-key labels, transition labels,
and local energies without the four raw per-site states, particle IDs, fluxes,
and wave velocities needed for an independent reconstruction of those
quantities. This violates FTD-0915 validity gate 6.

No A--E result is issued from that execution. Its artifacts remain preserved
under `engine/results/ftd_0915/` with hashes:

| Invalid v2 artifact | SHA-256 |
|---|---|
| `ftd_0915_summary_v1.json` | `53CC7D0C78BB5EB050B1D0F45F1CAD0F6118C48C1092CA6CAACFC3A6915D204E` |
| `ftd_0915_tick_census_v1.csv` | `F006ADACDABFEF970F4DE4914ADDBE3DCE2B812E49993596CAB23ED1AA80AA47` |
| `ftd_0915_transition_census_v1.csv` | `27291B4A36F82ED3C0168DBD514ED63510A99AD849343A1411682326FE60B49C` |

The descriptive provisional string printed by that invalid runner is not a
registered result.

## 2. Repair scope

The v3 repair changes telemetry only:

1. every identity-bearing exposure now stores all four site indices, actual
   states, particle IDs, flux vectors, and wave-velocity vectors;
2. every transition attempt stores the same raw fields at both the before and
   after endpoints;
3. word, signed identity, support geometry, relation, dipoles, bivector,
   local energies, and closure can therefore be reconstructed independently;
4. the repaired corpus is written to the new preserved subdirectory
   `engine/results/ftd_0915/v3/`.

The following are byte-for-byte or semantically unchanged from FTD-0915:

- all frozen production sources and the FTD-0914 analyzer;
- plaquette enumeration and vertex order;
- identity key and six relation classes;
- four-transition direct-closure definition;
- volumes, seeds, ticks, families, and production configuration;
- `6/8` replication threshold and Outcomes A--E;
- exact controls and all promotion/stop boundaries.

The prior corpus was not inspected to select a parameter, threshold, support,
or outcome. The repair adds fields required by the already-locked validity
gate and nothing else.

## 3. Repaired runner locks

| Artifact | SHA-256 |
|---|---|
| FTD-0915 protocol | `C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C` |
| repaired runner source | `D24970F34346167197D53681F1E6231A68C5E81F0515E6CA85B7335FBED83F21` |
| unchanged `engine/CMakeLists.txt` | `C895673132434DE830A15EE41676A446FCEF6D26D7C3819ED491E536D37BB745` |
| repaired exact Release executable | `E02B56E25F8FD38C0E12815A30D342378E7E9CC072DD0A7011CB71A80548249D` |

The executable was built through the canonical pinned MSVC 14.44 path.

## 4. Execution rule

An independent v3 preflight must verify the parent protocol, this repair,
every frozen production source, the repaired runner, CMake file, and exact
executable before execution. Any failure returns
`PROTOCOL_INVALID_NO_RECURRENCE_VERDICT`.

```text
PARENT_OUTCOME_MAP_CHANGED=FALSE
PARENT_ARMS_CHANGED=FALSE
PARENT_THRESHOLDS_CHANGED=FALSE
PHYSICS_CHANGED=FALSE
TELEMETRY_ONLY_REPAIR=TRUE
INVALID_V2_CORPUS_PRESERVED=TRUE
RUNNER_V3_LOCKED_BEFORE_EXECUTION=TRUE
STATUS=LOCKED_PRE_RUN
```

**LOCKED CONTENT ENDS HERE.**
