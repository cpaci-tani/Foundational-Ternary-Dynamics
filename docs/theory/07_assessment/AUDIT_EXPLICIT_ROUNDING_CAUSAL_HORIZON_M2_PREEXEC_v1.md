# FTD-0753 — Explicit-rounding causal-horizon M2 pre-execution audit v1

**Status:** `[PRE-EXECUTION AUDIT — REGISTERED ARMS NOT YET RUN]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_EXPLICIT_ROUNDING_CAUSAL_HORIZON_M2_v1.md`

## Audit verdict

The FTD-0753 physical successor is locked and executable before result
creation. It uses the FTD-0750 ordered deposition/deterministic observer path
and links only the FTD-0752 explicit-rounding CUDA research library. The new
runner's verdict omits the obsolete CPU-prefix gate but changes no physical
gate inherited from the causal-horizon construction.

The registered result directory was absent after all qualifications. No
312-tick arm had executed when the hashes below were recorded.

## Frozen identities

- protocol SHA-256:
  `66D64B1A09AAB3243C5BA06991B9979C10C03EA8B8B4A01BA3803260BF3822A4`;
- runner SHA-256:
  `B8AC5DED34953F8F59D9036EED9F72266DAF218842DA21CDA226666357986562`;
- WSL2 executable SHA-256:
  `878D752B4C4422A865B5C08EC1DC55C50610ECB2F743AFA6793A29303606F4D6`;
- explicit-rounding CUDA library SHA-256:
  `EE50D5C9C1746A063661658FD816D9CA09B3625EC043495D8F311034CFC409D0`;
- CUDA source SHA-256:
  `62080A7CC52560DDCB0F0F6F69CB6CF41C18C02A930F5C037540C40875246022`;
- CUDA CMake SHA-256:
  `D2CE82260C37B95956FA79DF045E1A4E776442AA7E64808DBDA809060605D5AC`.

`engine/CMakeLists.txt` is intentionally not a frozen campaign input because
other research targets share it; the executable and linked research library
are the binary identities of record.

## Qualification

Face, edge, and body each completed a four-tick `L=321` qualification without
writing results:

| arm | rows | aggregation | max net support | discarded L1 | moment residual |
|---|---:|---:|---:|---:|---:|
| face | 5 | pass | 36 | 0 | 0 |
| edge | 5 | pass | 36 | 0 | `1.889e-19` |
| body | 5 | pass | 54 | 0 | `7.496e-20` |

## Authorization and scope

Exactly one face, edge, and body arm may now execute serially. The result is a
finite causal-horizon environmental-persistence witness only. Even a
constructive conjunction leaves the state-only field separator, perturbation
measure, open-neighborhood radius, volume-stability statement, autonomous
motion, and particle interpretation unresolved.
