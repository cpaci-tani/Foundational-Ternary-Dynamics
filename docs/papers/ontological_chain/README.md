# The FTD Ontological Chain — A Canonical Map and Intuition Driver

**Tag:** `[SYNTHESIS]` — re-states canonical FTD claims at their canonical LEDGER tags; **introduces no new mathematics**.

**Precedence rule:** `LEDGER > constitution > this document`. If this document disagrees with
`docs/theory/07_assessment/core_ledgers/LEDGER.md`, `docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md`,
or `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` on any tag or value, **those documents are correct
and this one has a bug** — please file it.

## What this is

A single organized PDF mapping FTD's complete six-layer ontological chain —
**Ontology → Logic → Mathematics → Philosophy → Physics → Science** — written
map-first / intuition-second / provenance-always for an external reader. Every
load-bearing claim carries its epistemic tag and its `FTD-NNNN` provenance id.
The Master Chain DAG (Figure 0.1) is the centerpiece intuition device; each
chapter opens with a mini-map of its band.

## Build

`latexmk` is unavailable on this MiKTeX (no Perl). Build with the explicit pass sequence:

```powershell
pwsh -File build.ps1
```

or

```bat
build.bat
```

which runs `pdflatex → biber → pdflatex → pdflatex`. Output: `ontological_chain.pdf`.

## Source of truth for constants

**All numeric constants come from `ftd-math-macros.sty` / `chain-macros.sty`**, whose values
are sourced from `scripts/constants.py` (the canonical Python triple). Body text must **never**
hand-key a constant (e.g. never type `2.9586`, `137.036`, `3.024` directly) — use the macro.
The epistemic tag macro `\etag{...}` (`ftd-epistemic.sty`) rejects an unknown tag key as a hard
compile error (the typo guard).

## Files

| File | Role |
|------|------|
| `ontological_chain.tex` | master: KOMA `scrreprt` preamble + `\input` structure + shared TikZ styles |
| `ftd-epistemic.sty` | epistemic tag macro + compile-time typo guard (ported from the construction monograph) |
| `ftd-math-macros.sty` | canonical constants/equations defined once (ported) |
| `chain-macros.sty` | chain-specific constants/equations |
| `frontmatter.tex` | scope, precedence, three-register table, rule-key legend |
| `master_dag.tex` | the Master Chain DAG (Figure 0.1) |
| `ch1_ontology.tex` … `ch6_science.tex` | the six layer-chapters |
| `appendices.tex` | App A (epistemic accounting), B (provenance registry), C (glossary) |
| `references.bib` | classical literature |

## Relationship to the construction monograph

`docs/papers/monograph_construction/` tells the *mathematical construction story*
(`i → ℤ[i] → G* → master quadratic → the α boundary`). This document *contains* that story
as its Chapter 3 (Mathematics) but is broader — it is the whole six-layer chain — and cites
the monograph for the deep math-layer treatment.
