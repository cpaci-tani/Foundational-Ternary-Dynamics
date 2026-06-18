# Project Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Professionally organize the FTD repository while preserving scientific provenance and keeping public GitHub surfaces clear, current, and honest.

**Architecture:** Treat cleanup as a sequence of small, auditable passes. Public-facing documentation gets stabilized first; theory corpus movement happens only after classification; generated outputs are removed or ignored only when provenance and reproduction paths are preserved.

**Tech Stack:** Markdown, GitHub Actions, Python verification scripts, C++/CTest, Quarto documentation builds, Playwright where web dashboard behavior is touched.

---

## Scope And Non-Goals

This plan covers repository organization, README/GitHub metadata, documentation navigation, archive hygiene, generated-output policy, and verification surfaces.

This plan does not promote any FTD claim, derive new physics, run numerical coincidence searches, delete historical theory provenance, or rewrite git history.

## File Map

- Modify: `README.md` - public project overview and first-run guide.
- Modify: `CONTRIBUTING.md` - contributor expectations, verification, and epistemic discipline.
- Modify: `.github/copilot-instructions.md` - GitHub assistant orientation.
- Modify: `.github/PULL_REQUEST_TEMPLATE.md` - review checklist for epistemic and verification impact.
- Modify: `.github/workflows/ci.yml` - CI language and non-overclaiming validation checks.
- Modify: `docs/audits/INDEX.md` - audit plan discoverability.
- Move: root/ad-hoc visualization helpers into `scripts/visualization/legacy_root/`.
- Move: root maintenance and audit runners into `scripts/maintenance/` and the relevant `docs/audits/active/...` folder.
- Modify later: `META_DOCUMENTATION_MAP.md`, `META_PROJECT_ATLAS.md`, `docs/theory/META_INDEX.md`, and local `INDEX_*.md` files when deeper cleanup moves files.
- Create later as needed: `docs/audits/active/<slug>/AUDIT.md` for each structured cleanup sweep.

## Phase 1: Public-Facing GitHub Cleanup

- [x] **Step 1: Replace stale GitHub assistant instructions**

  Update `.github/copilot-instructions.md` so it describes this FTD repository, not an unrelated `packages/` monorepo.

- [x] **Step 2: Refresh the pull request template**

  Replace obsolete tag language with the current canonical tag set and require authors to state epistemic impact.

- [x] **Step 3: Make CI wording epistemically safe**

  Keep the core constant sanity check, but describe `x_+` as a structural match and verify the master-quadratic residuals. Do not call the alpha match a derivation or identify `x_-` with `N_c`.

- [x] **Step 4: Rewrite the root README for public orientation**

  Keep the README concise. Point readers to `WHERE_WE_LEFT_OFF`, canonical ledgers, the curated theory index, build commands, and contribution rules. Remove brittle corpus counts from the README body.

- [x] **Step 5: Refresh contributor guidance**

  Update `CONTRIBUTING.md` with current verification commands, canonical claim-status sources, and cleanup rules.

## Phase 2: Navigation And Metadata Sweep

- [ ] **Step 1: Audit root navigation docs for stale counts and stale dates**

  Search `README.md`, `META_DOCUMENTATION_MAP.md`, `META_PROJECT_ATLAS.md`, `CONTRACTS.md`, and `MAINTAINABILITY.md` for hard-coded file counts, stale engine versions, stale golden hashes, and outdated dates.

  Run:

  ```bash
  rg "586|118 archived|FTD-0267|FTD-0269|June 10|v5\\.40|0x56fa28acb5b9fe88|0xc13713f0e11a96da" README.md META_DOCUMENTATION_MAP.md META_PROJECT_ATLAS.md CONTRACTS.md MAINTAINABILITY.md
  ```

- [ ] **Step 2: Replace brittle counts with authority links**

  Where counts are decorative, remove them. Where counts are useful, point to a canonical source or label them as a dated snapshot.

- [ ] **Step 3: Reconcile root navigation entry points**

  Ensure all public paths agree on the first-read sequence:

  ```text
  README.md -> docs/WHERE_WE_LEFT_OFF.md -> META_PROJECT_ATLAS.md -> docs/theory/META_INDEX.md
  ```

- [ ] **Step 4: Verify navigation links**

  Run documentation-safe link checks only. Do not run physics searches or numerical scans as part of cleanup.

## Phase 3: Theory Corpus Classification

- [ ] **Step 1: Create an active sweep ledger**

  Create `docs/audits/active/theory-corpus-cleanup/AUDIT.md` with the `[x]`, `[~]`, `[d]`, `[n]` legend from `docs/audits/INDEX.md`.

- [ ] **Step 2: Classify active theory documents**

  For each active `docs/theory/*/*.md` document, classify it as one of:

  ```text
  current
  stale-metadata
  stale-links
  superseded-but-live
  internal-only
  needs-epistemic-tightening
  needs-structure-review
  ```

  Use the status model from `docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md`.

- [ ] **Step 3: Move only clear archive candidates**

  Use `git mv` for documents that are superseded, retracted, or historical-only. Place them in the nearest appropriate `archive/` directory and update `.gitignore` exceptions only if needed.

- [ ] **Step 4: Update navigation in the same change**

  For every moved or relabeled file, update `docs/theory/META_INDEX.md`, the local cluster `INDEX_*.md`, and any direct reading-path references.

- [ ] **Step 5: Check for stale active-path references**

  Run:

  ```bash
  rg "old/path/or/document-name" docs README.md META_DOCUMENTATION_MAP.md META_PROJECT_ATLAS.md
  git diff --check
  ```

## Phase 4: Generated Output And Git Hygiene

- [x] **Step 1: Review untracked generated files**

  Separate source/docs from generated data, `__pycache__`, temporary runners, and campaign outputs.

- [x] **Step 2: Delete only regenerable local debris**

  Remove Python bytecode caches and temporary files only when they are clearly generated and not intentionally tracked.

- [x] **Step 3: Preserve intentional campaign data policy**

  Do not remove tracked campaign outputs unless the project owner explicitly approves. Update `engine/results/README.md` if policy clarity is needed.

- [x] **Step 4: Strengthen ignore rules only when patterns recur**

  Prefer targeted `.gitignore` rules. Do not hide files that should be reviewed, such as new theory docs or source files.

  2026-06-18 pass: root/ad-hoc visualization helpers were consolidated into
  `scripts/visualization/legacy_root/`; the root `.gitignore` helper moved to
  `scripts/maintenance/clean_gitignore.py`; the engine audit runner moved to
  `docs/audits/active/ftd_engine_audit_team_review/run_audit.bat`. Existing
  scoped output directories such as `scripts/exploration/outputs/`,
  `scripts/verification/results/`, and `scripts/visualization/results/` were
  left in place because they are already grouped by producing subsystem.

## Phase 5: Verification And Release Readiness

- [ ] **Step 1: Run documentation-safe checks**

  ```bash
  git diff --check
  python scripts/theory/sync_theory_briefing.py --since 7d
  ```

- [ ] **Step 2: Run relevant automated tests**

  For documentation-only changes, run a narrow check such as:

  ```bash
  python scripts/proofs/proof_master_verification.py
  ```

  For engine changes, run the relevant CTest target first, then broaden to:

  ```bash
  ctest --test-dir engine/build -j 24 --output-on-failure -C Release
  ```

- [ ] **Step 3: Record deferred checks**

  If WSL2 GPU, Quarto, Playwright, or full CTest checks are not run, record why in the PR body.

- [ ] **Step 4: Prepare small reviewable commits**

  Suggested commit grouping:

  ```text
  docs: refresh public project orientation
  docs: align GitHub contribution metadata
  docs: classify theory cleanup candidates
  docs: archive superseded theory provenance
  chore: remove generated local debris
  ```

## Acceptance Criteria

- [ ] `README.md` is concise, accurate, and points to canonical status authorities.
- [ ] `.github/` files describe the actual FTD repo and current contribution rules.
- [ ] No public file calls `x_+ = 1/alpha` a derivation.
- [ ] No public file uses obsolete tag vocabulary for new work.
- [ ] Superseded docs are archived with provenance, not deleted.
- [ ] Navigation indexes are updated with every move.
- [ ] `git diff --check` passes.
- [ ] Any skipped verification is explicitly documented.
