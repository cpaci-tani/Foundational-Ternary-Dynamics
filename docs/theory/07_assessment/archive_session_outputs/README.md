# Archived Session Outputs (2026-04-19)

**Status:** historical record of session-specific outputs from the April 19 reframe deployment (Sessions 1-4). These are no longer live trackers / open work queues — every action they describe has been resolved or absorbed into the canonical reference docs in the parent directory.

**Maintenance rule:** read-only. Do not edit. If new content needs to be added that relates to one of these audits, add it to a new audit doc in the parent `07_assessment/` directory and cross-reference here.

---

## Contents

### Deleted 2026-05-03 (resolved trackers — git history preserves them)

| File | Original purpose | Resolution |
|---|---|---|
| ~~`TRACKER_REFRAME_FLAGS.md`~~ | 5 inline `[FLAG: re-derivation needed]` markers in EFT-program docs | All 5 RESOLVED 2026-04-19 (Session 2). File deleted 2026-05-03 in tracker consolidation. |
| ~~`TRACKER_PDF_ONLY_PAPERS.md`~~ | Triage of 13 PDF-only papers | All 13 ARCHIVED 2026-04-19 (Session 3). File deleted 2026-05-03; live PDF-only status now in `dissemination/papers/INVENTORY.json`. |

### Session-specific audits (point-in-time deliverables)

| File | Generated | Purpose |
|---|---|---|
| `INVENTORY_PORTFOLIO.md` | Session 1 | Phase 1 inventory of 280 artifacts outside `docs/theory/` |
| `FLAGGED_PASSAGES_PAPERS.md` | Session 1 | Phase 2 classification of 34 TeX papers in `docs/papers/` |
| `REDERIVE_REPORT_YM_NS.md` | Session 2 | RE-DERIVE assessment for Yang-Mills + Navier-Stokes (informed Session 3 retraction decisions) |
| `DEVILS_ADVOCATE_REPORT.md` | Session 2 | Phase 6.1 falsification pass on the 6 substantive rewrites; 3 BLOCKING bugs found and fixed same-day |
| `ENGINE_AUDIT_REFRAME.md` | Session 1 | C++ engine + JS frontend completed-infinity audit; 3 HIGH risk findings (2 fixed Session 2; 1 fixed Session 3 via α_inf rename) |

---

## Where the live state now lives

For current status of any claim or process previously tracked here:

- **Single source of truth for claim status:** `../LEDGER.md` (parent directory)
- **Append-only record of every change:** `../CHANGELOG_REFRAME.md`
- **Current open work:** `../PARKING_LOT.md`
- **Final epistemic / engine / constants audits (Session 4):**
  - `../AUDIT_EPISTEMIC_FINAL_2026_04_19.md` (note: returned inline by agent, summarized in CHANGELOG_REFRAME Session 4)
  - `../ENGINE_AUDIT_FINAL_2026_04_19.md`
  - `../AUDIT_CONSTANTS_FINAL_2026_04_19.md`
  - `../AUDIT_MANUSCRIPT_REFRAME.md`
  - `../AUDIT_SPECULATIVE_BOOK_2026_04_19.md`
  - `../REFACTORING_RECOMMENDATIONS_2026_04_19.md`
  - `../PHYSICIST_REVIEW_2026_04_19.md`

---

## Why these are archived (not deleted)

- They contain the **full reasoning trail** for decisions that the LEDGER and CHANGELOG only summarize.
- A future reviewer (or the owner six months from now) may want to verify that a retraction or restatement was supported by evidence at the time it was made.
- Per CHANGELOG_REFRAME maintenance rule, history is preserved.

If you need to action anything based on what's in these files, do it via a new entry in CHANGELOG_REFRAME.md. Do not edit the archived files in place.
