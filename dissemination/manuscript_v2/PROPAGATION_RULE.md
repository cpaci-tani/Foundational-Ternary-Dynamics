# Manuscript Propagation Rule

**Status:** Authoritative as of 2026-04-19. Read this before editing any manuscript chapter.
**Maintainers:** anyone editing `dissemination/manuscript/`, `dissemination/manuscript_v2/`, or any of the volume splits.

---

## The structure (as it actually is)

The manuscript portfolio has **four chapter locations** carrying overlapping content:

| Location | Chapter count | Naming convention | Role |
|---|---|---|---|
| `dissemination/manuscript/src/chapters/` | 92 | Decimal (`0.0-formal-logic.qmd`, `1.10b-master-quadratic-derivation.qmd`) | **manuscript_v1** — original Quarto book; broad audience |
| `dissemination/manuscript_v2/src/chapters/` | 83 | Two-digit (`01-five-postulates.qmd`, `10.1-large-scale-structure.qmd`) | **manuscript_v2 consolidated** — physicist-targeted rewrite |
| `dissemination/manuscript_v2/vol1/src/chapters/` | 35 | Same as v2 consolidated | **vol1 publication snapshot** (foundational chapters 01–10ish) |
| `dissemination/manuscript_v2/vol2/src/chapters/` | 45 | Same as v2 consolidated | **vol2 publication snapshot** (extensions 10.1+ onwards) |

Total: 92 + 83 + 35 + 45 = **255 chapter files** across four locations.

### Verified facts

- `vol1/src/chapters/` and `src/chapters/` share **35 filenames** (every vol1 file has a same-name counterpart in `src/chapters/`).
- `vol2/src/chapters/` and `src/chapters/` share **45 filenames** (every vol2 file has a same-name counterpart in `src/chapters/`).
- 35 + 45 = 80, but `src/chapters/` has 83 files → **3 files exist in `src/chapters/` that are not in either volume.** These are likely consolidation-only chapters (preface material, bridging chapters) that have not yet been routed into either volume.
- `vol1/src/chapters/01-five-postulates.qmd` **differs** from `src/chapters/01-five-postulates.qmd` (verified with `diff`). The volumes are NOT symlinks; they are independent files that have diverged.

### Implication

**Edits made in one location do not propagate to the other.** Any change to `src/chapters/01-five-postulates.qmd` made today is invisible to `vol1/src/chapters/01-five-postulates.qmd` unless explicitly copied.

---

## The rule (canonical, effective immediately)

### Authoritative location

> **`dissemination/manuscript_v2/src/chapters/`** is the **single source of truth** for all manuscript_v2 chapter content.

The `vol1/src/chapters/` and `vol2/src/chapters/` directories are **publication snapshots** — they exist to support volume-specific PDF builds with adjusted preambles, but their content must be a copy of the corresponding `src/chapters/` file.

### Propagation workflow (mandatory)

When editing **any** file in `manuscript_v2/src/chapters/`:

1. Make the edit in `src/chapters/`.
2. Check whether the same filename exists in `vol1/src/chapters/`. If yes, copy the edited file over (`cp src/chapters/<name>.qmd vol1/src/chapters/<name>.qmd`).
3. Check whether the same filename exists in `vol2/src/chapters/`. If yes, same.
4. Rebuild any volume PDFs that were affected.

When editing a file in **`vol1/`** or **`vol2/`** directly:

1. **Don't.** Edit `src/chapters/` first, then propagate. This prevents accidental divergence.

### manuscript_v1 ↔ manuscript_v2

The two manuscript versions are **different products** with overlapping but not identical content:

- `manuscript/` (v1) — original 92-chapter Quarto book; broader audience; numbered `0.x`, `1.x`, etc.
- `manuscript_v2/` — 83-chapter physicist-targeted rewrite; numbered `01`, `02`, ..., `10.1`, `10.2`, etc.

There is **no chapter-level identity** between the two. v2 is a partial rewrite, not a renaming. **Edits to v1 do not propagate to v2 and vice versa.** Any reframe-related substantive change should be applied to **both** v1 and v2 if the topic is covered in both.

The inventory agent's earlier "share ~57 chapters verbatim" finding was a heuristic by title-similarity, not by file-equality. **Do not assume v1 and v2 share verbatim content.** Diff before propagating.

### Identification mapping (v1 ↔ v2)

A canonical mapping table from v1 chapter numbers to v2 chapter numbers does not currently exist. Until it does, the safe rule is:

> **For any reframe edit, identify all chapters in v1 and v2 that cover the same topic, and apply the edit to all of them, plus the corresponding `src/chapters/` files in v2 plus the volume snapshots in v2/vol1 and v2/vol2.**

A future task: build `MANUSCRIPT_CHAPTER_MAP.md` cross-walking v1 chapter numbers to v2 chapter numbers (likely ~40–60 cross-walked entries; the rest are v2-only or v1-only).

---

## Quick-check commands

When you edit a file in `manuscript_v2/`, run these immediately after:

```bash
# Verify vol1 propagation needed
ls dissemination/manuscript_v2/vol1/src/chapters/<file>.qmd 2>/dev/null && \
  diff -q dissemination/manuscript_v2/src/chapters/<file>.qmd \
          dissemination/manuscript_v2/vol1/src/chapters/<file>.qmd

# Verify vol2 propagation needed
ls dissemination/manuscript_v2/vol2/src/chapters/<file>.qmd 2>/dev/null && \
  diff -q dissemination/manuscript_v2/src/chapters/<file>.qmd \
          dissemination/manuscript_v2/vol2/src/chapters/<file>.qmd
```

If `diff` returns "differ" but you intended to keep them synchronised, propagate.
If `diff` returns nothing (files are identical), no propagation needed.

For batch propagation of all v2 chapters:

```bash
# Sync vol1 chapters from src/chapters (overwrites vol1)
for f in dissemination/manuscript_v2/vol1/src/chapters/*.qmd; do
  src="dissemination/manuscript_v2/src/chapters/$(basename "$f")"
  [ -f "$src" ] && cp "$src" "$f"
done

# Sync vol2 chapters from src/chapters (overwrites vol2)
for f in dissemination/manuscript_v2/vol2/src/chapters/*.qmd; do
  src="dissemination/manuscript_v2/src/chapters/$(basename "$f")"
  [ -f "$src" ] && cp "$src" "$f"
done
```

**Caution:** these commands overwrite `vol1` and `vol2` entirely. Only run if you are confident `src/chapters/` is more recent.

---

## Known divergences (as of 2026-04-19)

A full audit of which chapters have diverged between `src/chapters/` and `vol1/`/`vol2/` has not been done. The single spot-check (`01-five-postulates.qmd`) showed divergence; the actual divergence count is unknown.

**Recommended near-term action:** run a full diff sweep:

```bash
for f in dissemination/manuscript_v2/vol1/src/chapters/*.qmd; do
  src="dissemination/manuscript_v2/src/chapters/$(basename "$f")"
  if [ -f "$src" ]; then
    diff -q "$src" "$f" 2>/dev/null
  fi
done
```

The output is the list of currently-diverged files. Decide for each: which version is canonical, then propagate.

---

## Reframe-deployment-specific addendum

For any chapter edit made under the undefined-boundary reframe (per `docs/theory/07_assessment/reframe_deployment/CANONICAL_REFRAME.md`):

- Find every location of the chapter (manuscript v1 + manuscript_v2 src/chapters + vol1/vol2 if present).
- Apply the same restatement to every location.
- Log the edit set in `docs/theory/07_assessment/CHANGELOG_REFRAME.md` so subsequent sessions can verify propagation.
- Consider tagging the LEDGER row (`docs/theory/07_assessment/LEDGER.md`) with the manuscript citation locations as part of the `citations:` field.

This is mandatory under the reframe — silently editing one location and leaving the others stale produces a divergent portfolio that contradicts itself across documents.
