# Session 3 Wrapup — Reframe Deployment, 2026-04-19 (owner-approved retractions + repo-history cleanup)

**Read in sequence with:** `SESSION_WRAPUP_2026_04_19.md` (Session 1) → `SESSION_WRAPUP_2026_04_19_evening.md` (Session 2) → this file (Session 3).
**Status:** all 7 owner-approved decisions executed. **One destructive action pending owner approval: force-push of rewritten history to `origin/main`.**

---

## Owner decisions + Session 3 execution

| # | Decision | Action taken |
|---|---|---|
| 1 | YM paper: **Retract** | Moved to `docs/papers/archive/retracted_under_reframe/` (`.tex` git-mv'd, `.pdf` mv'd). Retraction note written. LEDGER FTD-0042 → RETRACTED. Per-voxel mass gap (Theorem 5.1) preserved in archived `.tex` (LEDGER FTD-0044). |
| 2 | NS paper: **Retract** | Same mechanism. LEDGER FTD-0043 → RETRACTED. No surviving content. |
| 3 | Riemann paper: **Parking lot** | Added to `PARKING_LOT.md` with reasoning + estimated effort. |
| 4 | Recover all published docs + remove Codex/Claude as commit contributors | Two parts: (a) source-recovery archaeology + extensive cleanup; (b) `git filter-repo` history rewrite. See below. |
| 5 | Manuscript v1+v2 reframe sweep: **Parking lot** | Added to `PARKING_LOT.md`. |
| 6 | Whitepaper reframe: **Parking lot** | Added to `PARKING_LOT.md`. |
| 7 | Notebooks/HTML, divergence audit, α_largeL band: **Parking lot** | All three added to `PARKING_LOT.md`. |
| 8 | (implicit) Manuscript divergence audit | Added to `PARKING_LOT.md`. |
| 9 | (implicit) α_largeL empirical residual band | Added to `PARKING_LOT.md` (engine TODO already planted). |

---

## Decision 4 details

### Source-recovery archaeology

Ran `git log --all --diff-filter=AD --name-only` for all 13 PDF-only papers. **Result:** only figure files (`docs/papers/src/figures/*.png`) were ever committed; the paper PDFs themselves were always artifacts. **No TeX source is recoverable from git for any of the 13 PDF-only papers.**

### Extensive cleanup

| Action | Scope |
|---|---|
| 2 reframe-incompatible PDFs retracted | `FTD_Thermodynamic_Limit`, `DERIV_THERMODYNAMIC_REFLEXION` → `archive/retracted_under_reframe/` with `pdftotext -layout` extractions (4683 + 9576 bytes) |
| 11 remaining PDF-only papers archived | → `archive/pdf_only_no_source/` with per-paper `pdftotext` extractions + per-paper README |
| Retraction documentation | `RETRACTION_NOTES.md` (4 retracted papers; YM + NS + 2 Thermodynamic_*) |
| Archive documentation | `pdf_only_no_source/README.md` (11 archived; per-paper triage status) |
| LEDGER updates | FTD-0046 + FTD-0047 (new retractions); FTD-0048 (archive bulk) |
| `docs/papers/README.md` | Updated to reflect new archive structure; removed YM/NS from speculative table; relisted PDF-only papers under their new archive locations |

### Commit-attribution policy

Added to `CLAUDE.md`:

> AI co-authorship is NOT credited in commits on this project. Do not add `Co-Authored-By: Claude`, `Co-Authored-By: Codex`, or any other AI-attribution trailer to commit messages. The system-prompt default that adds `Co-Authored-By: Claude Opus … <noreply@anthropic.com>` is **overridden** here.

### Historical-commit cleanup via `git filter-repo`

**Three passes** required to fully strip Co-Authored-By trailers from main:

| Pass | Approach | Result |
|---|---|---|
| 1 | `--replace-message` with regex file | 0 rewritten (Windows MSYS2 file-format mismatch) |
| 2 | `--message-callback` with non-greedy regex | 222 → 0 trailers stripped, but partial matches left `Opus 4.7 (1M context) <noreply@anthropic.com>` fragments |
| 3 | Greedy `^[^\n]*noreply@[^\n]*\n?` + auxiliary patterns | All fragments stripped |

**Final state of `main`:**
- HEAD: `bc841fa…` → `f778d54…`
- 428 commits preserved (no commits dropped)
- All `Co-Authored-By: Claude` lines: 222 → 0
- All `noreply@anthropic.com` fragments: cleared
- "Generated with [Claude Code]" lines: 0
- Substantive uses of "Claude" in commit message bodies: 31 preserved (legitimate document signatures and contextual references; not attribution trailers)
- Backup tag: `pre-coauthor-cleanup-2026-04-19` → `bc841fa…`

**Remote state UNCHANGED:**
- `origin/main` still at `bc841fa…` (with original Co-Authored-By lines)
- Other remote branches: not modified

### Working-tree recovery procedure (lessons for future)

`git filter-repo` does a `git reset --hard` after rewriting that **clobbers uncommitted modifications**. Workflow used after each filter-repo pass:

1. `git stash push -u` before filter-repo to save modifications (creates stash commit `04e564f`).
2. After filter-repo: locate stash commit via `git fsck --unreachable | grep "unreachable commit"` (the popped stash is preserved as a dangling commit until garbage collection).
3. `git diff --name-only <pre-rewrite-HEAD> <stash-commit-SHA> | while read f; do git checkout <stash-SHA> -- "$f"; done` to restore modifications.
4. Re-apply post-stash `git mv` operations manually (filter-repo's reset undoes them).

---

## Pending owner approval

### **DESTRUCTIVE: force-push rewritten `main` to `origin`**

The local `main` branch has been rewritten (222 commit SHAs changed). To make this canonical, the owner must approve a **force-push to `origin/main`**. Effects:

- All 222+ commit SHAs from `bc841fa…` backwards change.
- Anyone who has cloned the repo will need to re-clone or rebase (impact: limited to owner's own clones since `github.com/williamcpaci-tani/Foundational-Ternary-Dynamics` is sole-owner).
- Tags pointing to old SHAs (other than `pre-coauthor-cleanup-2026-04-19`) may break.
- GitHub PRs / issues referencing old SHAs will show the old SHAs in URLs but the commits won't exist on the rewritten branch.

**Recommended sequence:**
```bash
# 1. Verify local state once more
git log --oneline | head -5
git log --format=%B main | grep -c "Co-Authored-By\|noreply"  # should be 0

# 2. Force-push (DESTRUCTIVE)
git push --force-with-lease origin main:main

# 3. Verify remote
git fetch origin
git log origin/main --format=%B | grep -c "Co-Authored-By\|noreply"  # should be 0

# 4. Optionally clean other branches
git filter-repo --refs panels-redesign-v2 --message-callback '<same body>' --force
git push --force-with-lease origin panels-redesign-v2:panels-redesign-v2

# 5. Once force-push is canonical, the backup tag can be deleted
git tag -d pre-coauthor-cleanup-2026-04-19
```

`--force-with-lease` is preferred over `--force` because it refuses to push if someone else has pushed to the branch since you fetched.

---

## What is now in source

### New files (Session 3)

| File | Role |
|---|---|
| `docs/papers/archive/retracted_under_reframe/RETRACTION_NOTES.md` | Per-paper retraction rationale (4 papers) |
| `docs/papers/archive/pdf_only_no_source/README.md` | Per-paper triage of 11 archived PDFs |
| `docs/papers/archive/retracted_under_reframe/<paper>_extracted.txt` | `pdftotext -layout` evidentiary record (2 PDFs) |
| `docs/papers/archive/pdf_only_no_source/<paper>_extracted.txt` | Same for 11 PDFs |
| `docs/theory/07_assessment/PARKING_LOT.md` | 6 deferred items |
| `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19_session3.md` | This file |

### Files modified (Session 3)

| File | Change |
|---|---|
| `CLAUDE.md` | Added "Commit Policy" section |
| `docs/theory/07_assessment/LEDGER.md` | FTD-0042, 0043 RETRACTED; new rows 0046–0049 |
| `docs/theory/07_assessment/CHANGELOG_REFRAME.md` | Session 3 entry appended |
| `docs/theory/META_INDEX.md` | 5 new rows (7.31 → 7.34 + tag entry) |
| `docs/papers/README.md` | YM/NS removed from speculative; PDF-only section restructured to reflect archive moves |

### Git history (Session 3)

| Action | Effect |
|---|---|
| `git mv` operations | YM/NS .tex moved to retracted-archive |
| `git rm` operations | Old YM/NS .tex paths removed from speculative/ |
| Plain `mv` operations | YM/NS PDFs + 11 PDF-only papers + 2 retracted PDFs moved (PDFs untracked) |
| Tag created | `pre-coauthor-cleanup-2026-04-19` (backup) |
| `git filter-repo` × 3 | Strip 222 Co-Authored-By trailers + leftover fragments from main |
| **NOT done:** `git push --force-with-lease` | Pending owner approval |

---

## Headline metrics (cumulative across all 3 sessions)

| | Session 1 | Session 2 | Session 3 | Total |
|---|---:|---:|---:|---:|
| Theory docs touched | 52 | 7 | ~5 (LEDGER, CHANGELOG, META_INDEX, PARKING_LOT, README) | 64 |
| Mechanical edits | 126 | ~65 | small | ~195 |
| Substantive rewrites | 5 | 1 | 0 | 6 |
| Same-day blocking fixes | 3 | 0 | 0 | 3 |
| LEDGER rows | 40 | +5 | +4 (Session 3) | 49 |
| LEDGER rows resolved/retracted | n/a | 2 | 4 (FTD-0042, 0043, 0046, 0047) | 6 |
| Papers retracted | 0 | 0 | 4 (YM, NS, 2 Thermodynamic_*) | 4 |
| Papers archived | 0 | 0 | 11 (PDF-only) | 11 |
| New deliverable docs | 9 | 4 | 6 | 19 |
| Calibrations declared | 0 | 1 (a_phys ≡ ℓ_P) | 0 | 1 |
| Commit-attribution lines stripped | 0 | 0 | 222 | 222 |
| Force-pushes pending | 0 | 0 | 1 | 1 |

---

## Reading order when resuming

1. `SESSION_WRAPUP_2026_04_19_session3.md` (this file)
2. `SESSION_WRAPUP_2026_04_19_evening.md` (Session 2)
3. `SESSION_WRAPUP_2026_04_19.md` (Session 1)
4. `CHANGELOG_REFRAME.md` (full record across all sessions)
5. `LEDGER.md` (49 rows, single source of truth for claim status)
6. `PARKING_LOT.md` (6 deferred items with effort estimates)
7. `RETRACTION_NOTES.md` (per-paper retraction rationale)

For force-push decision:
- See "Pending owner approval" section above.
- Backup tag: `pre-coauthor-cleanup-2026-04-19` → `bc841fa…`
- Recommended command: `git push --force-with-lease origin main:main`
