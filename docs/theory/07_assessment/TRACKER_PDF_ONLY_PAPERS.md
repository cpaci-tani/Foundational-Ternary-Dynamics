# TRACKER — PDF-Only Papers (Source-Recovery Status)

**Status:** open work queue.
**Trigger:** the inventory pass (`INVENTORY_PORTFOLIO.md`) found 13 PDF-only papers in `docs/papers/` with no recoverable TeX/MD source. Reframe action requires source. This file enumerates them, categorises by reframe-relevance, and recommends an action per paper.

---

## 1 · The 13 PDF-only papers

Verified via exhaustive search across `docs/papers/`, `docs/papers/src/`, `docs/papers/speculative/`, `docs/papers/archive/`, and `dissemination/papers/`:

| # | Paper | Reframe-relevance (heuristic by title) | Recommended action |
|---|---|---|---|
| 1 | `DERIV_ALPHA_INVERSE_LATTICE_GAUGE.pdf` | Likely (lattice gauge → α) | Recover source if possible; otherwise demote to "historical reference" |
| 2 | `DERIV_EMERGENT_GRAVITY.pdf` | Possible (gravity emergence may invoke continuum limit) | Recover source if possible; otherwise demote |
| 3 | `DERIV_FUNDAMENTAL_CONSTANTS.pdf` | Possible (constants derivation may invoke L → ∞) | Recover source if possible; otherwise demote |
| 4 | `DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME.pdf` | Likely (title flags discrete spacetime — central reframe topic) | **High priority — recover source.** Likely subsumed by `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` (already restated in this session). Verify and archive the PDF if redundant. |
| 5 | `DERIV_QUANTUM_INFERENCE.pdf` | Possible | Recover source; otherwise demote |
| 6 | `DERIV_SELF_REFERENCE_FOUR_INTEGERS.pdf` | Likely (algebraic; may survive the reframe unchanged) | Recover source for re-tagging |
| 7 | `DERIV_THERMODYNAMIC_REFLEXION.pdf` | **High** (title contains "Thermodynamic" — likely uses thermodynamic-limit framing) | Recover source if possible; if not, the PDF should be moved to `archive/` with a note that the framework no longer endorses thermodynamic-limit reasoning |
| 8 | `FTD_KMS_Thermal_Time.pdf` | **High** (KMS state condition is classically formulated in inductive-limit von Neumann algebras — directly reframe-relevant) | **Top priority — recover source.** Likely needs Type III₁ scaffold-language treatment per `DERIV_VON_NEUMANN_CONSTRUCTION.md` |
| 9 | `FTD_Modular_Structure.pdf` | **High** (modular structure is the algebraic spine of Type III₁; same issue as #8) | **Top priority — recover source.** Same treatment as #8 |
| 10 | `FTD_Spatial_Correlations.pdf` | Possible (correlations may involve thermodynamic limit) | Recover source; otherwise demote |
| 11 | `FTD_Thermodynamic_Limit.pdf` | **HIGHEST** (title literally is the proscribed concept) | **Highest priority — recover source.** If the paper's premise IS the thermodynamic limit, it does not survive the reframe in its current form and either (a) needs full re-derivation in finitary terms or (b) should be moved to `archive/` with a clear retraction note |
| 12 | `SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME.pdf` | Likely subsumed by current sources | Verify it is superseded by `PAPER_2A_MASTER_QUADRATIC.tex` + `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` (rewritten); if so, archive |
| 13 | `SPEC_MASTER_QUADRATIC_PAPER.pdf` | Same as #12 | Same as #12 |

---

## 2 · Triage summary

- **2 papers (#11, #7):** title-level evidence of thermodynamic-limit framing. **Cannot survive reframe in current form.** Source recovery is required to decide between full re-derivation and retraction.
- **2 papers (#8, #9):** KMS / modular structure — directly affected by Type III₁ demotion. Treatment in `DERIV_VON_NEUMANN_CONSTRUCTION.md` is the template.
- **2 papers (#12, #13):** likely already-superseded by existing TeX sources. Verify and archive.
- **1 paper (#4):** likely already-superseded by `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` which has been restated in this session. Verify.
- **6 remaining papers:** unknown impact without source. Recover or demote case-by-case.

---

## 3 · Recovery options

**A — Locate original sources.** Check:
- `~/Documents/`, `~/Downloads/`, `~/.claude/projects/`, any external drives, any cloud-sync folders.
- Git history of the project: `git log --all --diff-filter=D --name-only` (lists deleted files; the source may have been committed at some point and later removed).
- Any author's local editor caches (Overleaf, VS Code workspaces).

**B — Re-extract from PDF.** Use `pdftotext` + manual cleanup. This recovers prose but loses LaTeX semantic markup (math is approximated, citations are flat, figures are missing). Acceptable as a starting point for re-derivation but not as canonical source.

**C — Cite as historical PDF only.** Move the PDF to `docs/papers/archive/pdf_only/` with a clear note that no editable source exists; subsequent reframe action on that paper is impossible without re-authoring.

**D — Retract / archive without re-authoring.** For papers whose premise is unambiguously incompatible with the reframe (e.g., #11 `FTD_Thermodynamic_Limit`), move to archive without attempting re-derivation. Replace any portfolio citation with a note: "Previously published; superseded by undefined-boundary reframe; not reframed."

---

## 4 · Recommended near-term action sequence

1. **Run `git log --all --diff-filter=D --name-only -- 'docs/papers/*.tex' 'docs/papers/src/*.tex'`** to find any TeX source that was deleted from the repository at any point.
2. **For any source recovered**, place it next to the PDF and run the standard classifier + restatement pipeline.
3. **For sources not recoverable**, decide per-paper using the table above:
   - If reframe-relevant → archive the PDF with a retraction note (acknowledge incompatible with current ontology).
   - If reframe-irrelevant or already-subsumed → archive as historical reference, with cross-reference to the surviving source.
4. **Delete superseded PDF entries** from the manuscript / dissemination indices so future readers do not encounter them as live documents.

---

## 5 · Reproducibility

```bash
# List all PDFs in docs/papers/
ls docs/papers/*.pdf | sort

# For each PDF, check whether a same-name TeX or MD exists anywhere
for pdf in docs/papers/*.pdf; do
  base=$(basename "$pdf" .pdf)
  found=""
  for ext in tex md; do
    for dir in docs/papers docs/papers/src docs/papers/speculative docs/papers/archive dissemination/papers; do
      [ -f "$dir/$base.$ext" ] && found="$dir/$base.$ext" && break 2
    done
  done
  [ -z "$found" ] && echo "PDF-only: $base"
done

# Recover deleted TeX from git history
git log --all --diff-filter=D --name-only -- 'docs/papers/*.tex' 'docs/papers/src/*.tex' | sort -u
```

---

## 6 · Cross-references

- `INVENTORY_PORTFOLIO.md` — Phase 1 inventory that surfaced these papers
- `FLAGGED_PASSAGES_PAPERS.md` — Phase 2 classification of papers WITH source
- `CANONICAL_REFRAME.md` — what the reframe means
- `LEDGER.md` — claim-status ledger; per-paper claims that depend on these PDFs need to be flagged
