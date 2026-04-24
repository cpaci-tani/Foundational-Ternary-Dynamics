# FTD Documentation Cleanup Ledger

**Status:** Active audit and synchronization ledger  
**Date:** 2026-04-11  
**Intent:** Improve documentation trustworthiness, consistency, and navigability without deleting content

This ledger is the working audit record for project documentation cleanup. It covers public and internal documentation surfaces, records where the repo currently drifts from its own maps, and separates what was synchronized in this pass from what should be deferred or preserved as historical.

---

## Cleanup Status Model

Use these labels when classifying documents during cleanup:

| Status | Meaning |
|--------|---------|
| `current` | Matches current repo truth and is safe to use as a live reference |
| `stale-metadata` | Version, counts, dates, or summary metadata lag the live repo |
| `stale-links` | Links resolve poorly, point to superseded docs, or use the wrong entry point |
| `superseded-but-live` | Still present outside archive and should not be treated as primary |
| `internal-only` | Local working guidance, not part of the public documentation spine |
| `needs-epistemic-tightening` | Claim language is stronger than the project’s own audit/evaluation layer supports |
| `needs-structure-review` | File placement, naming, or cataloging should be revisited in a later pass |

---

## Inventory Summary

The counts below were gathered from the live filesystem during this cleanup pass.

| Surface | Live count / note |
|---------|-------------------|
| Root Markdown docs | 9 before this ledger; 10 after adding this file |
| `docs/` Markdown files | 244 |
| `docs/theory/` raw active-category Markdown files | 136 |
| `docs/theory/archive/` Markdown files | 76 |
| `docs/reference/` Markdown files | 5 |
| `docs/papers/` doc-like files (`.md`, `.tex`, `.pdf`) | 119 |
| `docs/internal/` Markdown files | 12 |
| `evaluation/` root Markdown files | 3 |
| `engine/tests/` files | 184 |
| `scripts/tests/` files | 40 |
| `dissemination/manuscript/src/chapters/` `.qmd` files | 92 |
| `dissemination/book/chapters/` `.qmd` files | 46 |
| `dissemination/notebooks/` `.ipynb` files | 12 |
| `dissemination/interactive/` files | 17 |

Important distinction:

- `docs/theory/META_INDEX.md` is currently a **curated theory catalog**, not a raw directory listing.
- The curated index now reports `129` indexed core entries and `76` archived entries.
- The raw active theory directories currently contain `136` Markdown files, which means some live-in-place, superseded, or not-yet-normalized documents still remain outside `archive/`.
- Theory sweep on `2026-04-11`: promoted `FOUND_GSTAR_SCALE.md` and `THEOREM_MOORE_LAYER_DECOMPOSITION.md` into the curated catalog; archived `FOUND_ENDGAME_SYNTHESIS.md`, `FOUND_FROM_PCIR_TO_FTD.md`, and `REPORT_FIVE_MINDS.md` as historical/noncanonical documents; archived `EXPLR_CUBOCTAHEDRAL_GEOMETRY.md`, `EXPLR_VACUUM_DRAG_DERIVATION.md`, and `EXPLR_PARTITION_PRIME_DETECTION.md` as superseded or low-signal exploratory documents; added `FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` as an active foundations note to formalize the new Potential Core / Generative Interior vocabulary; added `FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md` as the canonical live replacement for the missing consciousness source file and the lattice-facing formalization of `Activate_C`.

---

## Verified Drift Findings

### Public metadata drift

- `META_DOCUMENTATION_MAP.md` and `META_PROJECT_ATLAS.md` still identified the framework as `v5.28` and the engine as `v2.11`, while `docs/SPEC_FTD.md` is `v5.29` and `engine/SPEC_ENGINE.md` is `v2.12`.
- `META_DOCUMENTATION_MAP.md` and `META_PROJECT_ATLAS.md` repeated stale hard-coded counts for theory docs, engine tests, and dissemination interactives.
- `README.md` also contained stale structure counts and an outdated theory subdirectory name (`06_measurement` instead of `06_consciousness` in the structure block).

### Catalog-versus-filesystem drift

- `docs/theory/META_INDEX.md` presents a curated catalog of `129` indexed core entries and `76` archived entries.
- The raw theory tree currently contains `136` Markdown files in active categories plus `76` archive files.
- This is not necessarily a bug, but it must be stated clearly so users do not treat indexed counts as raw filesystem totals.

### Engine/test-count drift

- `engine/SPEC_ENGINE.md` reports `182` test files and `177` registered in CMake.
- The raw `engine/tests/` directory currently contains `184` files.
- Older public navigation docs reported `168`.
- Cleanup implication: public docs should either cite the engine spec explicitly or use durable wording instead of repeating brittle file totals.

### Internal-versus-public boundary drift

- Older public navigation pointed people toward `docs/internal/META_WALKTHROUGH.md`.
- `docs/internal/` is gitignored local-only working material and should not compete with the public documentation spine.
- Public docs should point to root docs, `docs/SPEC_FTD.md`, `engine/SPEC_ENGINE.md`, `META_CONTRIBUTOR_ONBOARDING.md`, and this ledger before sending readers into `docs/internal/`.

### Epistemic posture drift

- Public-facing summaries, especially in `README.md` and flagship docs, are stronger in tone than the audit/evaluation layer.
- This pass does not rewrite the theory corpus, but it does ensure navigation docs steer readers to:
  - `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md`
  - `docs/reference/REF_SCOPE_LIMITATIONS.md`
  - `evaluation/AUDIT_UNRESOLVED_ISSUES.md`

---

## This Pass: Synchronized Now

These items were updated in the current cleanup pass:

- Added this repo-wide cleanup ledger as the explicit audit record.
- Updated `README.md` to point to current contributor navigation and cleanup references, remove brittle structure counts, and correct the theory subdirectory naming in the structure sketch.
- Updated `META_DOCUMENTATION_MAP.md` to use current framework/engine versions, add this ledger as a first-class cleanup reference, remove or soften brittle hard-coded counts, and stop treating `docs/internal/` as a public onboarding dependency.
- Updated `META_PROJECT_ATLAS.md` to use current versions, add this ledger as a technical guide, and replace stale hard-coded counts with safer wording.
- Updated `META_CONTRIBUTOR_ONBOARDING.md` to reference this ledger for current documentation drift and cleanup status.
- Updated `docs/theory/META_INDEX.md` to explicitly present itself as a curated catalog rather than a raw directory count.
- Updated `docs/reference/REF_NAMING_CONVENTIONS.md` with a maintenance rule tying naming changes to catalog/status updates.

---

## Deferred But Tracked

### P0

- Audit public quick-start and structure sections beyond the root navigation docs for statements that imply stronger claim status than the audit/evaluation layer supports.
- Reconcile the engine test story across `engine/SPEC_ENGINE.md`, `engine/CMakeLists.txt`, and the raw `engine/tests/` directory.

### P1

- Normalize theory indexing so `docs/theory/META_INDEX.md`, `TRACKER_DOCUMENT_STATUS.md`, and raw active-category directory counts can coexist without ambiguity.
- Review stale numeric badges and numerically precise public summaries in `README.md`.
- Audit `docs/papers/` metadata and naming consistency against `REF_NAMING_CONVENTIONS.md`.

### P2

- Strengthen cross-linking between public navigation docs, the contributor onboarding guide, and the theory status tracker.
- Add clearer status banners to public docs that are still live but partially superseded.
- Audit dissemination-facing README/config docs so publication structure is easier to navigate without local lore.

### P3

- Optional structural cleanup later:
  - retitle or move superseded-but-live docs
  - move more historical-in-place theory docs into `archive/`
  - split oversized navigation docs if they become harder to maintain

No item in this queue implies deletion.

---

## Internal Docs: Second-Wave Classification

The following `docs/internal/` Markdown files were inventoried during this pass. They remain in place and are treated as local working material unless explicitly promoted later.

| File | Classification | Status | Recommended treatment |
|------|----------------|--------|-----------------------|
| `META_WALKTHROUGH.md` | Internal navigation note | `internal-only` | Keep local-only; do not use as the public first stop |
| `ONBOARDING_FTD_THEORY.md` | Internal theory onboarding | `internal-only` | Preserve; compare against public onboarding before any promotion |
| `SPEC_CLAUDE.md` | Internal AI/manual spec | `internal-only` | Preserve as local working guidance |
| `REF_PUBLICATION_EDITOR_INSTRUCTIONS.md` | Editorial workflow doc | `internal-only` | Preserve; do not treat as public documentation |
| `REF_IMAGE_INVENTORY.md` | Asset inventory | `internal-only` | Preserve |
| `META_BULLETPROOFING_STRATEGY.md` | Working strategy note | `superseded-but-live` | Preserve, but not authoritative |
| `META_IMPLEMENTATION_PLAN.md` | Working implementation plan | `superseded-but-live` | Preserve; consider labeling more explicitly later |
| `PHASE_2.4_SIMULATION_ENGINE_PLAN.md` | Phase-specific plan | `superseded-but-live` | Preserve as historical working note |
| `PLAN_ENGINE_PROOF_OUT.md` | Completion snapshot | `superseded-but-live` | Preserve as historical working note |
| `META_GEMINI_CONTEXT_TRANSFER.md` | Agent handoff doc | `internal-only` | Preserve local-only |
| `DIRECTIONS_UNEXPLORED.md` | Research ideas note | `internal-only` | Preserve |
| `RESEARCH_QM_SM_Postulates_and_Assumptions.md` | Internal research critique | `internal-only` | Preserve; do not mix with canonical public references |

---

## Preserve As Historical

These categories should remain available, but clearly non-authoritative unless a current doc points to them:

- `archive/`
- archived theory references explicitly marked in `docs/theory/META_INDEX.md`
- internal working plans and context-transfer material in `docs/internal/`
- older public summaries that still have historical value but should not be used as the first navigation layer

---

## Ongoing Maintenance Rule

For any future documentation change:

1. If you add or rename a public-facing theory, navigation, onboarding, or reference doc, update the relevant catalog in the same change.
2. If a change affects public entry points, update at least:
   - `META_DOCUMENTATION_MAP.md`
   - `META_PROJECT_ATLAS.md`
   - `META_CONTRIBUTOR_ONBOARDING.md` when contributor flow changes
3. If a change materially affects theory status or categorization, update:
   - `docs/theory/META_INDEX.md`
   - `docs/theory/07_assessment/TRACKER_DOCUMENT_STATUS.md` when status expectations change
4. Prefer removing brittle hard-coded counts over guessing when the repo has multiple non-identical counting surfaces.
5. Never delete documentation as part of routine cleanup; preserve and classify first, then archive or relabel explicitly later.

---

## Current Outcome

After this pass, the public documentation spine is more trustworthy:

- contributor entry points are clearer
- version and count drift is called out instead of silently repeated
- internal-only docs are less likely to be mistaken for canonical guidance
- there is now a durable ledger for future cleanup rounds

This ledger should be updated whenever the project performs another significant documentation synchronization pass.

---

## 2026-04-19 Addendum — Web-dashboard UX pass

Second synchronization pass, narrower scope than the initial April 11
sweep. The preceding sections (Inventory Summary, Drift Findings,
deferred P0-P3 queues) are preserved unchanged; this addendum records
only what moved.

### Updated counts

The April 11 figures have drifted. Live counts as of 2026-04-19:

| Surface | April 11 | April 19 | Delta |
|---------|----------|----------|-------|
| Root Markdown docs | 10 | 11 | +1 (`MAINTAINABILITY.md`) |
| `docs/theory/` total `.md` | 212 | 230 | +18 (April 13 EFT-program additions + recent derivations) |
| `docs/theory/` archive `.md` | 76 | 76 | — |
| `docs/theory/` active categories | 136 | 154 | +18 |
| `engine/web/docs/` SPEC_*.md | 10 | 10 | — (one audited for archive candidacy this pass) |

### Docs added this pass

- `MAINTAINABILITY.md` (repo root) — 582-line field manual: 8
  project-level hazards, 15 step-by-step recipes, tech-debt ledger.
  Complements this cleanup ledger on the code/UX side.
- `engine/web/docs/TELEMETRY_CATALOG_SCALE0.md` — 297 lines
  documenting every Scale 0 telemetry surface (ring buffers,
  diagnostics rows, charts, Lagrangian panel). Sibling catalogs for
  other scales planned as follow-ups.

### Docs rewritten this pass

- `engine/web/docs/SPEC_VERIFICATION_LAB.md` — rewrote to match the
  new three-tier evidence-scoreboard design (replaces the
  21-experiment Monte-Carlo v1 spec).

### P0-P3 status

None of the April 11 deferred items (theory indexing normalization,
`README.md` numeric-badge review, `docs/papers/` naming audit) were
addressed in this pass. Still open, same priority.

### New items flagged for later

- **`engine/web/docs/SPEC_*.md` audit** — `[COMPLETED]` `SPEC_REFACTOR_LARGE_FILES.md` and `SPEC_S0_OVERLAY_COMPLEXITY.md` archived to `docs/theory/archive/` (April 23, 2026).
- **Cross-reference `MAINTAINABILITY.md` from navigation spine** — `[COMPLETED]` Added to `META_PROJECT_ATLAS.md` (April 23, 2026).
