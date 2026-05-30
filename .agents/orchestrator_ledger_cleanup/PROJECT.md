# Project: Ledger Numbering Cleanup and Reconciliation

## Architecture
This project is an administrative and mathematical indexing synchronization effort. The Foundational Ternary Dynamics (FTD) campaign documents, ledger rows, index lists, and math node maps have grown in size and encountered ID collisions and duplicate IDs. The goal is to clean up these ID collisions, register late-May 2026 documents with correct canonical ledger IDs and honest statuses, synchronize downstream index documents, and rebuild the math node map.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | Exploration & Mapping | Audit current files to confirm duplicate and colliding IDs, identifying the exact lines and patterns. | None | DONE |
| 2 | Ledger & Doc Renumbering | Renumber colliding rows in LEDGER.md and update internal headers/references within the campaign files. | M1 | DONE |
| 3 | Index Sync & Math Map | Update META_INDEX.md, INDEX_FTD_NATIVE_EFT.md, TRACKER_OPEN_ITEMS.md, and rebuild the math node map using the verification script. | M2 | DONE |
| 4 | Audit & Review | Verify 100% correctness of renumbered files, math node map consistency, and run checks to ensure zero duplicate IDs or broken links. | M3 | DONE |

## Interface Contracts
- **LEDGER.md Schema Integrity**: All FTD IDs must be unique strings `FTD-NNNN` in the ID column.
- **Reference Integrity**: FTD-NNNN IDs mentioned in other files must exactly match their canonical rows in LEDGER.md.
- **Math Node Map Integration**: `scripts/verification/build_math_node_map.py` must run successfully on the updated documents and generate valid JSON/HTML with 100% graph consistency.
