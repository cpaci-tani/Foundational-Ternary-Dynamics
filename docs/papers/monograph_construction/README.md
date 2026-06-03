# The Construction of the FTD Mathematics — LaTeX monograph

A platinum-standard LaTeX formalization of
[`docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md`](../../theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md)
(tag `[SYNTHESIS]`). It is a *faithful port*: it introduces no new mathematics
and promotes no epistemic tag. Where this document and a canonical source
disagree on a tag, the source is correct.

## Build

`latexmk` is unavailable on the development MiKTeX (no Perl), so the build is a
manual `pdflatex → biber → pdflatex ×2`:

```powershell
pwsh -File build.ps1      # or: build.bat
```

Engine: **pdflatex** (newtx fonts are Type1-native). Bibliography: **biblatex + biber**.
Output: `monograph.pdf` (≈49 pp).

## Architecture

| File | Role |
|------|------|
| `ftdmonograph.cls` | bespoke memoir-based class; monochrome, print-safe; wide tag margin |
| `ftd-epistemic.sty` | the monochrome epistemic-tag system (`\etag`, `\margetag`, `\ctag`, marquee boxes) |
| `ftd-math-macros.sty` | every canonical constant/equation defined **once** (anti-drift) |
| `monograph.tex` | master file |
| `frontmatter.tex` | title page, scope/authorities, how-to-read, tag rule-key, ToC |
| `part0_seed.tex` … `coda.tex` | the five movements (Parts 0–III + Coda) |
| `dag.tex` | the hand-authored native-TikZ construction DAG |
| `references.bib` | external classical literature (Watson, Chowla–Selberg, Chudnovsky, …) |

## The monochrome tag system

Epistemic *tier* is carried by **rule style, never colour**, so the PDF reads
identically in grayscale:

| Treatment | Rule | Tags |
|-----------|------|------|
| **FORCED** | heavy solid | `THEOREM DERIVED AXIOM NUMERICAL FACT MEASURED EMERGENT` |
| **CONDITIONAL** | thin solid | `SELECTION IMPOSED PARAMETRIC SYNTHESIS` |
| **CONJECTURAL** | dashed | `STRONGLY MOTIVATED CONJECTURE  CONJECTURE` |
| **BOUNDARY** | double (a "wall") | `OPEN  CLOSED NEGATIVE  FOUNDATIONAL OBSTRUCTION` |

An unknown tag key is a hard compile error (a built-in typo guard).

## Owner actions

- **Author line** is deliberately omitted on the title page (the source carries
  none; the project's grade-A paper leaves `\author{}` empty). Fill at submission
  time in `frontmatter.tex`. Do **not** fabricate a name.
- Numeric anchors in `ftd-math-macros.sty` are cross-checked against
  `scripts/constants.py` (`G_STAR`, `VARPI_CLASSICAL`, `X_PLUS`, `X_MINUS`).
