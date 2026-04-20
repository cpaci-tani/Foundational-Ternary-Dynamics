# Documentation Build Report — Session 4 (2026-04-19)

## Tool availability
| Tool | Available | Version |
|---|---|---|
| quarto | yes | 1.8.26 |
| pdflatex | yes | MiKTeX 4.18 (24.1) |
| jupyter | NO | not installed (`py -m pip install jupyter` per Quarto's hint) |

Absence of Jupyter blocks any `.qmd` that contains `{python}` cells.

## Build Results
| Asset | Status | Notes |
|---|---|---|
| Whitepaper (`dissemination/whitepaper/FTD_Whitepaper.tex`) | OK | 12-page PDF rebuilt, 658 KB. 14 minor warnings (`fancyhdr \headheight too small`, `hyperref` Unicode tokens), 1 overfull hbox at lines 86-99 (74.77pt). All 5 figures resolve. No undefined refs/citations after pass 2. |
| EFT paper (`dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`) | **FAIL** | Fatal: `! Undefined control sequence … \l_siunitx_quantity_prefix_mode_str :n` at l.67 on `\SI{0.4}{\percent}`. ~10 occurrences of `\SI{...}{\percent}` throughout (lines 67, 68, 71, 87, 227, 228, 229, 230, 231, 271, 272, 431, 522). Likely cause: paper missing `\sisetup{}` config or MiKTeX siunitx version mismatch. No PDF produced. |
| Manuscript v2 (`dissemination/manuscript_v2/src/`) | OK with warnings | 82/82 chapters rendered to HTML (`_book/index.html` exists). 19 warnings: 17 unresolved `@sec-*` crossrefs (see "Broken cross-references"), 2 path/cleanup warnings (`Refusing to remove _book/site_libs`, `Quarto did not expect path configuration`). |
| Manuscript v1 (`dissemination/manuscript/src/`) | **FAIL** | `ERROR: Error executing 'py': The pipe is being closed.` after chapter 24. Root cause: Jupyter kernel not installed (`Jupyter is not available in this Python installation`). Followed by `PermissionDenied: Access is denied` cleanup error. Build aborts mid-render. |
| Book (`dissemination/book/`) | OK | 53/53 chapters rendered to HTML (`_book/index.html`). No warnings. Includes Session 3's edit to `chapters/38_the_unification.qmd`. |
| Finitude Theorem (`docs/papers/speculative/FTD_Finitude_Theorem.tex`) | **FAIL** | Fatal: `! Undefined control sequence \C` at l.124 (`the complex numbers $\C$`). File defines `\R, \Z, \N, \Q` (lines 56-59) but **not `\C`**. PDF on disk dates from before the Session 3 canonical-status preamble was added — current `.tex` cannot rebuild. |
| Other `docs/papers/*.tex` (10 files) | not rebuilt | Skipped per time budget (PDFs exist, no Session-3-4 edits). Spot-check recommended for `PAPER_RATIO_AND_THE_ARROW.tex`, `PAPER_GSTAR_BRIDGE_CONSTANT.tex`. |
| Notebooks (`dissemination/notebooks/*.ipynb`, 12 files) | OK (JSON only) | All 12 parse as valid JSON. Execute-render not attempted (no jupyter). |
| Interactive HTML (`dissemination/interactive/*.html`, 17 files) | OK | All 17 parse as well-formed HTML via Python `html.parser`. Includes the 2 surgical edits from Session 3. |

## Failures (full detail)

### EFT paper — siunitx `\percent` failure
```
! Undefined control sequence.
<argument> ...\l_siunitx_quantity_prefix_mode_str :n
l.67 ...ariance residuals reach \SI{0.4}{\percent} at $r \le 4$ lattice
```
The siunitx package loads (line 20: `\usepackage{siunitx}`) but `\percent` is not recognized. In siunitx v3, `\percent` exists as a unit; the error suggests an internal expl3 token is missing — most often a stale siunitx or expl3 in MiKTeX. Manual fix would be either `\sisetup{...}` early in preamble, replacing `\SI{x}{\percent}` with `x\,\%`, or updating MiKTeX. **Not fixed per instructions.**

### Manuscript v1 — Jupyter required
```
[24/94] chapters\1.10b-master-quadratic-derivation.qmd
Starting python3 kernel...
ERROR: Error executing 'py': The pipe is being closed. (os error 232)
Jupyter is not available in this Python installation.
Install with py -m pip install jupyter
```
Chapter 1.10b (or one of its predecessors) embeds an executable `{python}` cell. v1 cannot render until `jupyter` is installed in `Python313`.

### Finitude Theorem — undefined `\C`
```
! Undefined control sequence.
<recently read> \C
l.124 ...eal numbers $\R$, the complex numbers $\C$, infinite-dimensional Hi...
```
Lines 56-59 define `\R, \Z, \N, \Q` but never `\C`. Single missing macro. The on-disk PDF predates the Session 3 preamble edit, so the published artifact and the current source disagree.

## Broken cross-references
**Manuscript v2** — 17 unresolved Quarto crossrefs (all label-style, not file-path). These are intra-document `@sec-*` references where the target label was renamed or removed:

| File | Crossref |
|---|---|
| `chapters/14.1-constants-reference.qmd` | `@sec-constants` |
| `chapters/14.1-constants-reference.qmd` | `@sec-lemniscate-alpha` |
| `chapters/14.1-constants-reference.qmd` | `@sec-mandelbrot-bridge` |
| `chapters/14.1-constants-reference.qmd` | `@sec-minimum-distance` |
| `chapters/14.1-constants-reference.qmd` | `@sec-consciousness-quadratic` |
| `chapters/14.3-glossary.qmd` | `@sec-formal-logic` (×4) |
| `chapters/14.5-assumption-ledger.qmd` | `@sec-formal-logic` (×2), `@sec-master-quadratic-derivation`, `@sec-four-forces`, `@sec-born-rule`, `@sec-uniqueness`, `@sec-curve-uniqueness` |
| `chapters/14.6-self-consistency.qmd` | `@sec-master-quadratic-derivation` |

These are pre-existing — not caused by Session-4 work — but worth flagging since the v2 reference chapters (14.x) are in the navigation and currently render with broken anchors.

**No broken cross-references** to retracted/moved papers were found. `AUDIT_MANUSCRIPT_REFRAME.md` line 27 already confirmed: "Chapters citing retracted papers: 0".

## Missing figures
**None.** All 5 whitepaper figures (`figure1_lemniscate_gstar.pdf` … `figure5_alpha_precision.pdf`) resolve. EFT paper has no `\includegraphics` directives. Book chapter 38 has no image references. Notebook outputs were not regenerated (no jupyter), so any embedded image data is from previous executions.

## Stale citations to retracted/moved papers
**None outside of intentional reframe documentation.** Sweep across `*.{tex,qmd,md}` shows every reference to `FTD_Yang_Mills_Mass_Gap`, `FTD_Navier_Stokes`, `FTD_Thermodynamic_Limit`, `DERIV_THERMODYNAMIC_REFLEXION`, and the 11 PDF-only papers either:
- Points to the new `docs/papers/archive/retracted_under_reframe/` or `docs/papers/archive/pdf_only_no_source/` location, or
- Lives inside `docs/theory/07_assessment/` reframe/ledger/audit documents that intentionally name the retracted titles.

`docs/papers/README.md` (Session-3 restructured) correctly describes the PDF-only / retracted partitioning.

## Accessibility findings (changed sections only)
- **Book chapter 38** (`38_the_unification.qmd`): Session 3 edit. No `![](...)` image macros present in the file → no alt-text concern introduced.
- **EFT paper** (`PAPER_FTD_AS_WILSONIAN_EFT.tex`): no figures, no tables added in Session 3 edits (10 tag/overclaim fixes).
- **Whitepaper** (`FTD_Whitepaper.tex`): no Session-3 figure changes; existing figures use LaTeX `\caption{}` (acceptable for print).
- **Interactive HTML** (`dissemination/interactive/*.html`, 2 edited): not re-audited at element level — out of scope for "changed sections only" beyond verifying parse-clean.

No new accessibility regressions introduced by Session-3 edits.

## Verdict
**YELLOW**

Reasoning:
- 5/8 build targets succeed (whitepaper, manuscript v2, book, notebooks JSON, interactive HTML).
- 3 hard failures: EFT paper (siunitx `\percent`), manuscript v1 (jupyter not installed), Finitude Theorem (`\C` undefined).
- 0 broken cross-references to moved/retracted papers — Session 3's archive moves were correctly propagated.
- 0 missing figures.
- The whitepaper — the highest-priority target with the most Session-3 changes — builds clean.
- The EFT paper failure is the most significant unblocked item: it was the focus of Session-3 reframe edits but cannot be regenerated to PDF until the `\percent` issue is resolved.

Not RED because the dissemination tree's primary HTML/PDF surfaces (whitepaper, book, manuscript v2, interactive simulations) all build. Not GREEN because the EFT paper, manuscript v1, and Finitude Theorem each fail with a single distinct root cause.
