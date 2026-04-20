# Refactoring Recommendations — Reframe Deployment Cleanup (2026-04-19)

**Scope:** consolidation tickets for the documents Sessions 1–3 added to `docs/theory/07_assessment/`. Engine/script duplication scan included.
**Method:** read every reframe-deployment doc, cross-reference inbound citations, sized each consolidation candidate against actual content overlap.
**Audience:** owner deciding what to merge/move/keep before more reframe work resumes from `PARKING_LOT.md`.

---

## Summary table (sorted by priority)

| #  | Title                                              | Files affected                                          | Priority | Effort | Risk |
|----|----------------------------------------------------|---------------------------------------------------------|----------|--------|------|
| 1  | Collapse 3 SESSION_WRAPUP docs into 1 + appendix   | 3 → 1 (-271 LOC net)                                    | P1       | S      | L    |
| 2  | Move 2 RESOLVED trackers to `archive/`             | 2 trackers (-155 LOC active surface)                    | P1       | S      | L    |
| 3  | Demote 5 audit reports to `archive_session_outputs/`| ENGINE/DEVILS/INVENTORY/FLAGGED/REDERIVE (-1820 LOC)   | P2       | S      | M    |
| 4  | Make CANONICAL the agent-facing source; demote AUDIT to per-claim appendix | 2 docs, ~80 LOC overlap | P2       | M      | M    |
| 5  | Standardise LEDGER citation format across 15 sites | 15 files                                                | P2       | S      | L    |
| 6  | Promote AUDIT_INFINITY's "Permitted/Proscribed" table to canonical, delete from CANONICAL | 2 docs (-30 LOC duplication) | P3 | S | L |
| 7  | Add `INDEX.md` to `07_assessment/` (reading map)   | 1 new file                                              | P2       | S      | L    |
| 8  | Merge `TRACKER_PDF_ONLY_PAPERS` into `RETRACTION_NOTES` since action is complete | 2 → 1 (-95 LOC) | P3 | S | L |
| 9  | Extract `α_largeL` rename pattern as a comment macro | 3 source files                                        | P3       | S      | L    |
| 10 | `reframe_deployment/` → `_archive/reframe_deployment_v1/` once Session-4 work resumes | 1 directory rename | P3 | S | M |

**Net impact if all P1+P2 executed:** ~1900 LOC out of active reading surface; `07_assessment/` shrinks from 28 docs to ~17; reading-order ambiguity collapses from 3 wrapups + 2 canonical-overlap docs to 1 wrapup + 1 canonical doc.

---

## RF-1: Consolidate 3 SESSION_WRAPUP docs into FINAL_WRAPUP + per-session appendix

- **Category:** consolidation
- **Severity:** HIGH (active reading bottleneck — every resumer reads three files)
- **Location:**
  - `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19.md` (106 LOC)
  - `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19_evening.md` (130 LOC)
  - `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19_session3.md` (185 LOC)
- **Current State:** Three sequential wrapups, each opening with "read this in addition to / in sequence with the previous." Headline-metrics tables are cumulative-rewritten in S2 and S3 (S3's table includes columns for all three sessions). "Reading order" sections appear in all three with three different orderings. Session 1's "what you need to decide" list has been overtaken (every item is now RESOLVED, RETRACTED, or in PARKING_LOT). Session 2's "open work queue" is similarly stale. Cumulative metrics in S3 supersede both prior tables. Concrete duplication: ~60 LOC of "where to find everything" + headline-metrics scaffolding repeated three times.
- **Proposed Change:** Create `SESSION_WRAPUP_REFRAME_FINAL.md` (~150 LOC) holding (a) cumulative S1+S2+S3 headline metrics table (already in S3), (b) consolidated "what shipped" list, (c) one canonical reading-order pointing at LEDGER + CHANGELOG + PARKING_LOT, (d) the single still-pending decision (force-push approval). Move the three originals to `archive/session_diaries/` as historical record (renaming inbound CHANGELOG references to the archive path).
- **LOC / file count impact:** 3 files (421 LOC) → 1 file (~150 LOC) + 3 archived. Net active-surface −271 LOC.
- **Effort:** S (~30 min).
- **Risk:** LOW. CHANGELOG_REFRAME already records every change; the wrapups are summary-of-summary. Only inbound citations are from each other.
- **Dependencies:** none.

---

## RF-2: Move RESOLVED trackers to `archive/`

- **Category:** consolidation
- **Severity:** HIGH
- **Location:**
  - `docs/theory/07_assessment/archive_session_outputs/TRACKER_REFRAME_FLAGS.md` (60 LOC) — all 5 rows now RESOLVED 2026-04-19 per S2 wrapup
  - `docs/theory/07_assessment/archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` (95 LOC) — all 13 papers archived in S3
- **Current State:** Both files have a `**Status:** open work queue` header in the prose, but every entry is now in a terminal state. The trackers' own §4 ("Recommended near-term action") is now history, not a queue. They linger as live trackers and any reader landing on them initially believes the work is open.
- **Proposed Change:** Move both to `docs/theory/07_assessment/archive/resolved_trackers/`. Add a one-line header to each: "ALL ITEMS RESOLVED — see CHANGELOG_REFRAME `2026-04-19 Session 2/3` for disposition." Update the 4 inbound references (CHANGELOG_REFRAME, SESSION_WRAPUP×3) to point at the archive paths. Optionally collapse both into a single `archive/resolved_trackers/REFRAME_TRACKERS_RESOLVED.md` since they share structure.
- **LOC / file count impact:** 2 files (155 LOC) leave active surface; either kept as-is in archive or merged (-30 LOC of repeated prose if merged).
- **Effort:** S (~15 min).
- **Risk:** LOW. The trackers' content is preserved in CHANGELOG and LEDGER row resolutions.
- **Dependencies:** none.

---

## RF-3: Demote 5 session-output audit reports to `archive_session_outputs/`

- **Category:** consolidation
- **Severity:** MEDIUM (clutters the assessment top level)
- **Location:**
  - `ENGINE_AUDIT_REFRAME.md` (255 LOC) — HIGH-1 resolved, MED-1..3 still flagged but already cataloged in LEDGER
  - `DEVILS_ADVOCATE_REPORT.md` (215 LOC) — all PASS-WITH-NOTES items resolved S2
  - `INVENTORY_PORTFOLIO.md` (332 LOC) — Phase 1 census; superseded by per-paper actions S2/S3
  - `FLAGGED_PASSAGES_PAPERS.md` (419 LOC) — Phase 2 classifier output; 5 tractable papers restated S2; YM/NS retracted S3
  - `REDERIVE_REPORT_YM_NS.md` (599 LOC) — assessment that drove the S3 retraction; now historical
- **Current State:** Five generated artifacts produced during the deployment as decision-support inputs. Each was actionable when written; the actions have all landed (in CHANGELOG_REFRAME) or have been parked (in PARKING_LOT). They sit alongside reference-grade docs (`AUDIT_EPISTEMIC_AUDIT`, `LEDGER`, `REF_CLAIMS_MATRIX`) that readers expect to be live.
- **Proposed Change:** Create `docs/theory/07_assessment/archive_session_outputs/` and `git mv` all five there. The directory README lists what each was used for and where its action landed. Keep `AUDIT_INFINITY_REFRAME.md` at top level — that one is a per-claim disposition reference, not a session output.
- **LOC / file count impact:** 5 files (1820 LOC) leave the top-level reading surface.
- **Effort:** S (~20 min). Update inbound links in CHANGELOG_REFRAME, SESSION_WRAPUP files (or those go away under RF-1), CLAUDE.md if any.
- **Risk:** MEDIUM. ENGINE_AUDIT_REFRAME's MED-1..3 + LOW-* findings are still actionable and would be less discoverable. **Mitigation:** before archiving, cherry-pick the unresolved findings into LEDGER as new rows (FTD-0050+) or into PARKING_LOT.
- **Dependencies:** RF-1 (so wrapup citations are updated only once).

---

## RF-4: Make CANONICAL_REFRAME the source; demote AUDIT_INFINITY_REFRAME to per-claim appendix

- **Category:** consolidation / hierarchy
- **Severity:** MEDIUM (two docs claim canonical-status on overlapping content)
- **Location:**
  - `docs/theory/07_assessment/reframe_deployment/CANONICAL_REFRAME.md` (211 LOC) — agent-facing, frozen v1.0, Q1–Q4 decision procedure, proscribed/permitted lists
  - `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` (264 LOC) — per-claim SURVIVES/RESTATE/RE-DERIVE/RETRACT triage
- **Current State:** Both opened with definitions of completed-infinity vs undefined-boundary. AUDIT §2 "permitted/proscribed" is a subset of CANONICAL "Proscribed Moves" (§§Proscribed/Permitted). CANONICAL §"Four Triage Actions" overlaps AUDIT §2 disposition headers verbatim. Cited from 33 files combined; unclear precedence.
- **Proposed Change:** Reorganise as:
  - **CANONICAL_REFRAME.md** = the *only* source of "what the reframe means" — definitions, proscribed/permitted moves, Q1–Q4 procedure, four-tag legend, four-triage definitions. Move it up one level (out of `reframe_deployment/`) to `docs/theory/07_assessment/CANONICAL_REFRAME.md` so it sits next to LEDGER and CHANGELOG. (The `reframe_deployment/` directory becomes templates+agents only.)
  - **AUDIT_INFINITY_REFRAME.md** = the per-claim disposition list only. Strip §1–§2 (definitions) and replace with a one-paragraph pointer: "for definitions see `CANONICAL_REFRAME.md`." Keep §2.1 (SURVIVES table), §2.2 (RESTATE table), §2.3 (RE-DERIVE list), §2.4 (RETRACT list) — these are the unique value-add.
- **LOC / file count impact:** AUDIT_INFINITY_REFRAME drops from 264 → ~180 LOC; CANONICAL gains ~10 LOC of cross-pointer text. Net −80 LOC of duplicated definitions.
- **Effort:** M (~1 hour). Citation updates in 33 files (mostly path-only changes via sed or Edit).
- **Risk:** MEDIUM. Reframe-deployment agents read `CANONICAL_REFRAME.md` from a hardcoded path (`reframe_deployment/CANONICAL_REFRAME.md`) — moving it requires updating agent prompts in `reframe_deployment/agents/`.
- **Dependencies:** RF-10.

---

## RF-5: Standardise LEDGER citation format across 15 inbound sites

- **Category:** cross-reference hygiene
- **Severity:** MEDIUM
- **Location:** 15 files cite `LEDGER.md` (47 occurrences total)
- **Current State:** Three styles in active use:
  - Plain text: `LEDGER.md`
  - Path-relative: `docs/theory/07_assessment/LEDGER.md`
  - Markdown link: `[LEDGER.md](LEDGER.md)` or `[LEDGER](../07_assessment/LEDGER.md)`
  - Some cite specific rows: `LEDGER FTD-0030`, others `LEDGER row FTD-0030`, others `LEDGER.md (FTD-0030)`
- **Proposed Change:** Establish convention in `LEDGER.md` header §"Citation rule":
  - Cross-doc: `[LEDGER FTD-XXXX](relative/path/LEDGER.md#ftd-xxxx)` with anchor.
  - Add per-row HTML anchors `<a id="ftd-0001"></a>` to each row in the quick index table.
  - Single file pass to normalise the 47 occurrences.
- **LOC / file count impact:** +50 anchor lines in LEDGER.md, citation normalisation in 15 files (no LOC change).
- **Effort:** S (~30 min).
- **Risk:** LOW.
- **Dependencies:** none.

---

## RF-6: Move proscribed/permitted-moves list to single home

- **Category:** duplication
- **Severity:** LOW
- **Location:**
  - `CANONICAL_REFRAME.md` §"Proscribed Moves" (10 items, ~25 LOC) + §"Permitted Moves" (10 items, ~25 LOC)
  - `AUDIT_INFINITY_REFRAME.md` §1 "What changed" — bullet lists that recapitulate the same proscribed/permitted distinction (~15 LOC each)
- **Current State:** Both files maintain near-isomorphic lists. CANONICAL is more authoritative (frozen v1.0). AUDIT's version was written first and was the source CANONICAL canonicalised from.
- **Proposed Change:** In AUDIT, replace the two bullet lists with: "See `CANONICAL_REFRAME.md` §Proscribed/Permitted Moves." Saves ~30 LOC and removes the risk of the two lists drifting.
- **LOC / file count impact:** −30 LOC.
- **Effort:** S (~10 min).
- **Risk:** LOW.
- **Dependencies:** RF-4 (do RF-4 first; RF-6 falls out as part of stripping AUDIT §1–§2).

---

## RF-7: Add `INDEX.md` to `07_assessment/`

- **Category:** navigation
- **Severity:** MEDIUM
- **Location:** `docs/theory/07_assessment/` has 28 files; no per-directory map. The closest is `META_INDEX.md` at `docs/theory/META_INDEX.md` which now has 7.20–7.34 rows for the new docs but mixes them with the 100+ other theory files.
- **Current State:** Each new entrant adds another row to META_INDEX. Readers landing on `07_assessment/` see a `ls` of mixed audit/tracker/session/ledger/changelog/devil's-advocate/etc. with no orientation.
- **Proposed Change:** Create `docs/theory/07_assessment/INDEX.md` with three sections:
  - **Live (read first):** LEDGER, CHANGELOG_REFRAME, CANONICAL_REFRAME (after RF-4 move), AUDIT_INFINITY_REFRAME (the per-claim trimmed version), AUDIT_EPISTEMIC_AUDIT, REF_CLAIMS_MATRIX, TRACKER_OPEN_ITEMS, TRACKER_DOCUMENT_STATUS, PARKING_LOT.
  - **Reference (browse as needed):** AUDIT_BELL_ANALYSIS, AUDIT_ENGINE_CALLSTACK, AUDIT_HIDDEN_SELECTIONS, AUDIT_MASTER_QUADRATIC, AUDIT_RATIONAL_FIT_CLAIMS, AUDIT_SELF_CONSISTENCY, AUDIT_WHAT_IS_GENUINELY_NEW, CATALOG_PARAMETRIC_INSERTIONS, REPORT_DETECTOR_INFORMATION_LOSS, DERIV_INTEGER_UNIQUENESS.
  - **Archive (historical record):** archived session outputs (RF-3), resolved trackers (RF-2), session diaries (RF-1).
- **LOC / file count impact:** +1 file (~80 LOC).
- **Effort:** S (~20 min).
- **Risk:** LOW.
- **Dependencies:** Best done after RF-1, RF-2, RF-3 land.

---

## RF-8: Merge TRACKER_PDF_ONLY_PAPERS into RETRACTION_NOTES

- **Category:** consolidation
- **Severity:** LOW (also covered by RF-2, kept as a separate ticket because the merge target lives in `docs/papers/` not `07_assessment/`)
- **Location:**
  - `docs/theory/07_assessment/archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` (95 LOC) — triaged 13 papers
  - `docs/papers/archive/retracted_under_reframe/RETRACTION_NOTES.md` — 4 retracted papers' rationale
  - `docs/papers/archive/pdf_only_no_source/README.md` — 11 archived papers' triage
- **Current State:** TRACKER_PDF_ONLY's content is now operationalised in the two `archive/` README files. Three docs collectively describe the same 13 papers' fate.
- **Proposed Change:** If RF-2 archives TRACKER_PDF_ONLY in place, no further action. Optionally fold its §1 table (per-paper recommended action) into the two archive READMEs as a "what was decided and why" column. The original tracker becomes the "history of how we got from 13 PDFs to 11+2 archived" record.
- **LOC / file count impact:** Subsumed under RF-2 if executed; standalone savings −95 LOC.
- **Effort:** S (~10 min beyond RF-2).
- **Risk:** LOW.
- **Dependencies:** RF-2.

---

## RF-9: No code-level reframe duplication needs fixing

- **Category:** code (negative finding)
- **Severity:** LOW (acknowledgment ticket — no action needed)
- **Location:** Sessions 1–3 touched `engine/include/ftd/lagrangian.h`, `engine/tests/test_einstein_equations.cpp`, `engine/tests/benchmark_dynamical_sm.cpp`, `scripts/benchmarks/analyze_convergence.py`, `scripts/benchmarks/continuum_extrapolate.py`, `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`.
- **Current State:** Reframe-related code edits were the `alpha_inf` → `alpha_largeL` rename across 4 files (3 `.cpp/.py` + 1 `.tex`) plus 2 comment fixes. No new helper, no new pattern, no candidate for a shared utility — each call-site is a literal rename of a struct member or variable. The "undefined-boundary" phrasing appears in exactly one comment in `test_einstein_equations.cpp`.
- **Proposed Change:** None. The rename is naturally site-local; abstracting it would obscure rather than DRY. Note for future: if multiple benchmarks accumulate `alpha_largeL`-style extrapolated quantities, consider a shared `ExtrapolatedFit { value, residual_band, L_range }` struct in `engine/include/ftd/eft_fit.h` — but only when the second benchmark needing it appears.
- **LOC / file count impact:** 0.
- **Effort:** none.
- **Risk:** none.
- **Dependencies:** none.

---

## RF-10: Move `reframe_deployment/` to `_archive/` once Session-4 dispatch is decided

- **Category:** directory hygiene
- **Severity:** LOW
- **Location:** `docs/theory/07_assessment/reframe_deployment/` — 3 top-level files + `agents/` (9 prompts) + `templates/` + `checklists/`
- **Current State:** The package was imported as a turnkey deployment; Sessions 1–3 used the agent prompts directly. PARKING_LOT items 5 (manuscript reframe) and 6 (whitepaper reframe) would re-use them. While those are parked, the directory sits in the assessment area implying live work.
- **Proposed Change:** Two options:
  - **(a)** Leave as-is (correct if PARKING_LOT items 5–6 will be picked up within ~weeks).
  - **(b)** If parked indefinitely, rename to `docs/theory/07_assessment/_archive/reframe_deployment_v1/`. When work resumes, copy back as `reframe_deployment_v2/` with any updates.
- **LOC / file count impact:** 0.
- **Effort:** S (~10 min if (b)).
- **Risk:** MEDIUM if (b) and work resumes — agents would be re-pointed.
- **Dependencies:** RF-4 if CANONICAL_REFRAME is hoisted out first.

---

## Recommendations by execution order

**Quick wins (do first, ~1 hour total):**
1. RF-1 (consolidate wrapups)
2. RF-2 (archive resolved trackers)
3. RF-5 (standardise LEDGER citations)

**Middle pass (~1 hour, after quick wins):**
4. RF-3 (archive session outputs) — first cherry-pick unresolved ENGINE_AUDIT findings into LEDGER/PARKING_LOT
5. RF-4 (CANONICAL hoist + AUDIT trim)
6. RF-6 (drop duplicated proscribed/permitted lists, falls out of RF-4)

**Final pass (~30 min):**
7. RF-7 (INDEX.md for 07_assessment/)
8. RF-8 (subsumed by RF-2)
9. RF-10 (defer until Session-4 status known)

**No-op / acknowledgment:**
- RF-9 (no code-level duplication to fix)

---

## What was checked but not flagged

- **`reframe_deployment/agents/*.md`** (9 stateless prompts) — not duplicative; each agent has a distinct phase responsibility. Some boilerplate in headers but well within tolerance.
- **`reframe_deployment/templates/*.md`** — single-purpose templates; no redundancy.
- **`reframe_deployment/checklists/{pre,per,post}_flight.md`** — sequenced gates, no overlap.
- **LEDGER and CHANGELOG_REFRAME** — both append-only by policy; do not consolidate.
- **AUDIT_EPISTEMIC_AUDIT** vs new audit reports — different scope (project-wide epistemic state vs reframe-specific session outputs); keep separate.
- **`CATALOG_PARAMETRIC_INSERTIONS.md`** — orthogonal axis (per-insertion enumeration); no overlap with reframe docs.

---

## What's well-designed (acknowledged)

- **LEDGER** as single source of truth + append-only CHANGELOG is the right shape. The 49-row table with per-row YAML detail blocks is a good pattern.
- **CANONICAL_REFRAME's Q1–Q4 decision procedure** is genuinely reusable across agents and worth keeping frozen at v1.0.
- **PARKING_LOT** is explicit about the difference between "deferred" and "forgotten" and includes the reasoning for each item — exactly what a future session needs.
- The **`a_phys ≡ ℓ_P` declaration in SPEC_FTD** is a calibration discipline that resolves a real ambiguity in the framework, not bureaucratic.
- **Commit-attribution policy** addition to CLAUDE.md is the right level (project-wide, one paragraph, enforced by reviewer).
