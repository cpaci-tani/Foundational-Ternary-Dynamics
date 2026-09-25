# Repository Audit Snapshot (2026-09-24)

This is a tracked-scope inventory and targeted hygiene/architecture audit, not a claim that every source file or physics result has been semantically certified. Recompute counts with `git ls-files`; use `REPOSITORY_MAP.md` to find the owning subsystem and the canonical ledger for claim status. The observations below are the pre-remediation baseline; see the follow-up at the end for current verification and remaining debt.

## Scope

| Area | Tracked paths in audit index | Boundary |
|---|---:|---|
| `engine/` | 4,184 | Native/CUDA/WASM engine, browser dashboard, tests, third-party code, recorded results |
| `docs/` | 1,948 | Theory, references, specifications, papers |
| `scripts/` | 1,680 | Proofs, verification, exploration, experiments, tests |
| `dissemination/` | 487 | Manuscripts, notebooks, interactive outputs |
| `lean/`, `evaluation/`, and remaining root/tooling files | 86 | Formalization, assessment, CI, project policy |
| **Total** | **8,385** | Git-index snapshot; local untracked work is excluded |

The non-web engine contains 841 tracked test files, 271 result files, 135 evidence files, and 1,102 third-party files. `scripts/` includes 812 proof files, 268 exploration files, 167 experiment files, 136 test files, and 75 verification files. These are file counts, not test or proof pass counts.

## Public Checkout Boundary

- The two main theory indexes have 1,104 and 1,009 local file links. The on-disk check passes, but `python scripts/verification/verify_index_links.py --tracked` currently fails on 260 and 211 links respectively: their targets are not in the Git index. Many are `AUDIT_*.md` documents intentionally ignored by root policy. A clean clone cannot follow those links.
- 425 on-disk theory audit Markdown files are ignored; none were bulk-published. Of these, 390 are linked from tracked Markdown and 35 have no direct tracked link. Screening found four with absolute user paths and seven with private agent-plan paths. Four dependency-clean, status-explicit candidates were identified for a separately approved first batch: `AUDIT_POSTULATE_ACTION_SUFFICIENCY_AND_OCCAM_BOUNDARY_v1.md`, `AUDIT_INFINITY_REFRAME.md`, `AUDIT_RATIONAL_FIT_CLAIMS.md`, and `AUDIT_MASTER_QUADRATIC.md`. This screening is not editorial certification. Tracked README navigation was redirected to tracked sources; the vetted contributor guide and engine spec were made trackable deliberately.
- The working tree has about 37,231 ignored paths (roughly 12.8 GiB at audit time), mostly build/cache material under `engine/`, but also local research archives, models, and audit evidence. No recursive deletion was performed. Do not treat all ignored paths as disposable.
- A bounded current-tree and Git-history scan found no common token/private-key fingerprints or tracked credential/key filenames. Tracked raw execution evidence and hash-locked preregistrations still contain historical absolute workstation paths; older commits also retain them. This is not a full secret-scanner certification, and rewriting historical evidence or Git history requires a separate provenance decision.

## Verification Boundary

- `ctest --test-dir engine/build -N -C Release` registered 650 targets, one disabled; at least one executable was missing from the current build. No full native suite was run for this audit.
- `python -m pytest scripts/tests --collect-only -q` collected 8,784 cases with 493 deselected by the default retained-evidence gate (9,277 total). Collection is not a pass result.
- Focused checks passed: 5 link-checker pytest cases; 230 assistant Node cases; assistant browser full profile 16 passed and 7 optional local-model/hardware cases skipped; CSS lint and dependency-cruiser (500 modules, 1,139 dependencies); local index links 0 broken; generated ledger/dimensional-map checks; `git diff --check`.
- The strict tracked-checkout link mode fails by design until the publication boundary above is reconciled. It is not yet a CI gate.

## Cleanup Completed In This Pass

- Preserved three FTD-0032-dependent retracted arguments under `docs/theory/archive/` with `git mv`, archive notices, index/ledger/reference updates, and narrow ignore exceptions. The current FTD-0032/0081 summaries now distinguish surviving algebra from the withdrawn physics route.
- Made the contributor guide, engine spec, dependency-cruiser config, and Stylelint config available to a tracked checkout. Redirected README and project guidance from ignored navigation files to `REPOSITORY_MAP.md` and other tracked authorities.
- The index-link verifier now has a nonzero exit on failures and an opt-in tracked-checkout mode. The default browser suite excludes the long hardware campaign, whose explicit profile has a separate port. Assistant browser smoke/full profiles and dedicated server ports are explicit.
- The open-items tracker was partitioned without dropping any of its 86 original item bodies: 44 working or pending-verification headings remain active and 42 terminal headings moved to `TRACKER_RESOLVED_ITEMS.md`. The generated index now uses file location rather than a `CLOSED` keyword guess. New Scale-0 hardware runs write run-identified, unpromoted scratch reports under ignored `engine/build/scale0-hardware/`; the 14 tracked historical reports and their measurement gates remain untouched.

## Remaining Decisions And Debt

1. Classify ignored theory audits as publishable canonical records or local-only notes. Then either track reviewed targets or repair/remove their links in public indexes. Do not mass-add the directory.
2. The `validate-physics` CI job currently installs dependencies but runs no validation; its prior numerical comparison was deliberately retired. Check required status checks before removing or replacing this no-op job. ADR-0015 deliberately limits the Windows merge gate to Pages-triggering changes; broader source-triggered CI requires an explicit runner-budget/policy change, not a silent deploy-filter expansion.
3. Hardware reports in scratch require a separate, deliberate review and promotion protocol before becoming canonical evidence; do not overwrite or silently replace the 14 historical records.
4. Continue sector-by-sector review of stale active claims and index summaries, including mixed-status tracker headings; do not promote tags during cleanup.
5. Inventory rebuildable caches by exact resolved path and dated backup before any recursive removal. Keep `archive/`, `models/`, ignored audits, and recorded evidence intact unless separately classified.

## Architecture Follow-Up (2026-09-24)

This section describes the current working tree; new workflows and tools do not exist in a remote checkout until deliberately staged, committed, and pushed.

- The README now names the active finite-record v3 constitution, identifies continuous `J` as a recovery target, and labels the current engine a distinct probe law. `docs/WHERE_WE_LEFT_OFF.md` is a short ledger-checked resume; its former 4,457-line content is preserved in the tracked `docs/reference/REF_RESEARCH_PROGRESS_LOG.md`.
- The three principal theory indexes label ignored audit references as local-only paths instead of broken public links. `python scripts/verification/verify_index_links.py --tracked` now covers those indexes plus README, CLAUDE, REPOSITORY_MAP, and the resume: **0 broken links across those seven entry points**. This is not corpus-wide certification. A broader lexical inventory still finds 912 nontracked Markdown-link occurrences in 121 other tracked files, including 80 targets missing even locally; the ledger contains 583 occurrences. Publication or link conversion needs sector-by-sector provenance review.
- CI now has meaningful repository-integrity checks and separate scoped CPU-engine and web source gates. The CPU merge gate built and passed 8/8 locally; the web Vite build and combined shell/asset Chromium smoke passed 25/25 locally. The CPU gate does not compile CUDA, WASM, or desktop-native targets; hosted Actions execution remains unverified. The misleading rigorous-verification runner and its stale classification test are preserved as retired text, not current gates. Ruff covers maintained assistant staging with narrow exemptions for existing one-line-statement style debt.
- The web shell now shares panel-mount preference behavior across first paint and runtime, and the built Vite page carries its required local static assets. Scale 2/3 AtomEngine helper direction is narrower, with dependency rules checking it. These are software-architecture repairs, not a v3 cross-scale recovery result.
- The scale ownership register distinguishes independent effective dashboard engines from the v3 recovery contract. It records required state ownership, finite-history readout, error/stability, causal support, and accounting evidence; it does **not** claim those recoveries have been achieved.
- `python scripts/verification/check_manuscript_v2_propagation.py` now reports chapter-copy parity without overwriting publication snapshots. Its current result is 26 divergent Volume 1 copies out of 35, and 0 divergent Volume 2 copies out of 45. Strict mode is opt-in pending editorial reconciliation.
