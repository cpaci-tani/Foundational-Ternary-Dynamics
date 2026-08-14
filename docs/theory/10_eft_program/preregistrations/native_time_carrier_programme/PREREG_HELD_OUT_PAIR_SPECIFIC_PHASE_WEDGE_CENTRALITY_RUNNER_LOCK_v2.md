# FTD-0912 — Held-out pair-specific phase-wedge/centrality runner lock v2

**Identifier:** `FTD-0912`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — RUNNER/ADJUDICATOR LOCKED; PRE-RUN]`  
**Parent protocol:** FTD-0911, SHA-256
`D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE`

## 1. Purpose

This companion freezes the implemented FTD-0911 observer and independent
adjudicator after compile-only qualification and before any held-out data.
It changes no source lock, arm, qualification rule, derangement, midpoint
identity, seed gate, outcome, or promotion ceiling in FTD-0911.

## 2. Frozen instrument chain

| Artifact | SHA-256 |
|---|---|
| `engine/tests/campaign_held_out_pair_specific_phase_wedge_centrality.cpp` | `092954834F568DF2CCCB0F4908CE3E6E0212C45CAE2CFAEF568518C27ED7CE5D` |
| `engine/CMakeLists.txt` | `DFB9E52B9BA43B10344C806BCD8B2B0936F71BF5F1A1632428939EF58F1D544D` |
| `engine/build/Release/campaign_held_out_pair_specific_phase_wedge_centrality.exe` | `D2DEFE4F4D540EDC044CFD8C7E0802CD40CE60BD153D4473347E99C65042AD60` |
| `scripts/proofs/proof_held_out_pair_specific_phase_wedge_and_centrality_result.py` | `7FB9F3575E3965108B3A35E05C6799D5CC24555A250611ED3FF0E2A1CACF5CEA` |
| `scripts/proofs/proof_held_out_pair_specific_phase_wedge_centrality_runner_lock_preflight.py` | `F13B93189587CF5BFD7622730DE9D1CE5A29876B39B786E14B4C75D129C3FB2C` |

The exact executable identifies the first local binary of record; portable
scientific identity is the C++ source plus the parent production-source locks.

## 3. Compile-only qualification

`engine/build_native.bat` completed under pinned MSVC 14.44.35207. The new
translation unit compiled and linked. No CTest or campaign was executed and
`engine/results/ftd_0911/` remained absent. The lock preflight passed `23/23`.

## 4. Independent reconstruction contract

The frozen adjudicator cannot call or modify the engine. It must reconstruct:

1. the exact 64-arm by 128-tick matrix and all state/RNG nonmutation checks;
2. every pair axis, endpoint projection, wedge, chirality, ID, and tick count;
3. the retained identities and longest common consecutive interval;
4. actual lag-one same-sign count and every nonzero fixed cyclic derangement
   on identical sample support;
5. all four chronology controls;
6. every midpoint `Delta ell`, central-torque, kinetic-alignment, identity
   residual, and exact centrality decision; and
7. all arm summaries, six-of-eight cell gates, 12-of-16 no-bath
   qualification, and invalid/A/B/C/D/U outcome.

Any disagreement invalidates the campaign. Any post-data parser or runner
repair requires preservation of the invalid corpus and a new lock.

## 5. Scope

This lock creates no evidence. Only a valid FTD-0911 Outcome A permits a
later perturbation/work protocol. No outcome here proves protected recursive
memory, G* synchronization, Born recovery, or contextual actualization.

```text
PARENT_PROTOCOL_CHANGED=FALSE
PRODUCTION_SOURCES_CHANGED=FALSE
RUNNER_COMPILED=TRUE
CAMPAIGN_EXECUTED=FALSE
RESULT_CORPUS_EXISTS=FALSE
RUNNER_AND_ADJUDICATOR_FROZEN=TRUE
RUNNER_LOCK_PREFLIGHT=23/23
PRODUCTION_TICK_MODIFIED=FALSE
PERTURBATION_APPLIED=FALSE
GSTAR_READ=FALSE
CONTEXT_OUTCOME_BORN_READ=FALSE
PAIR_CENTRALITY_VERDICT=NOT_YET_AVAILABLE
STATUS=LOCKED_PRE_RUN
```
