# Orchestrator Workspace - Ledger Cleanup Campaign

This directory is the dedicated coordination workspace for the Project Orchestrator leading the FTD ledger cleanup campaign.

## Campaign Goals
1. Resolve duplicate and colliding ledger IDs in `docs/theory/07_assessment/LEDGER.md`.
2. Register separate canonical ledger rows for v1 pre-registration and resolution documents.
3. Align all internal ID references in campaign and pre-registration documents.
4. Synchronize all downstream indexes (`META_INDEX.md`, `INDEX_FTD_NATIVE_EFT.md`, `TRACKER_OPEN_ITEMS.md`).
5. Rebuild the FTD math node map using `scripts/verification/build_math_node_map.py` and verify zero graph/link inconsistencies.
