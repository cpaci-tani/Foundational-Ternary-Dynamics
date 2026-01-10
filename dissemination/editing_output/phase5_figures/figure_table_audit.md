# Figure and Table Audit Report

**Document:** FTD Manuscript Figure Audit
**Date:** 2026-01-10
**Auditor:** Automated Analysis

---

## Executive Summary

This audit examines all figures and tables across the FTD manuscript, comparing planned figures against actual implementations and identifying gaps in the current publication workflow.

### Key Findings

| Metric | Count |
|--------|-------|
| Total Figure Files (PNG) | 150+ |
| Total Figure Files (SVG) | 150+ |
| Chapters with ASCII Diagrams | 66 |
| Chapters with Embedded Image References | 0 |
| Planned Figures (per FIGURE_TRACKER.md) | 65 |
| Evidence Figures Planned | 22 |

### Critical Issue

**Figures are NOT embedded in chapter source files.** While extensive figure files exist in the `figures/` directory structure, the `.qmd` chapter files contain **zero image embed syntax** (`![](...)` or Quarto figure references). The FIGURE_PLAN.md states figures are "implemented" but this refers to figure *generation*, not *embedding*.

---

## 1. Figure Files Inventory

### 1.1 Directory Structure

```
manuscript/figures/
├── ch00/  (Front matter figures)
├── ch01/  (Book I: Foundations)
├── ch02/  (Book II: Subatomic)
├── ch03/  (Book III: Atomic)
├── ch04/  (Book IV: Molecular)
├── ch05/  (Book V: States of Matter)
├── ch06/  (Book VI: Structures)
├── ch07/  (Book VII: Planetary)
├── ch08/  (Book VIII: Stellar)
├── ch09/  (Book IX: Galactic)
├── ch10/  (Book X: Cosmic)
├── ch11/  (Book XI: Extreme)
├── ch12/  (Book XII: Emergent)
├── ch13/  (Book XIII: The End)
├── ch14/  (Book XIV: Appendices)
└── individual/  (Figure generation scripts)
```

### 1.2 Figure Types by Chapter

#### ch00 (Front Matter)
- `fig-ascii-index-*.png/svg` (ASCII converted)
- `fig-intro-hierarchy-of-being.png/svg`
- `fig-epistemic-claim-types.png/svg`
- `fig-discrete-operators.png/svg`
- `fig-ontological-levels.png/svg`

#### ch01 (Foundations) - Most Extensive
- `fig-void-three-states.png/svg`
- `fig-genesis-pair-production.png/svg`
- `fig-two-layers-split.png/svg`
- `fig-interference-constructive.png/svg`
- `fig-interference-destructive.png/svg`
- `fig-existence-cycle.png/svg`
- `fig-annihilation-burst.png/svg`
- `fig-universal-tick.png/svg`
- `fig-time-gate-and-lightcone.png/svg`
- `fig-force-unification.png/svg`
- `fig-constants-dependency.png/svg`
- `fig-lemniscate-alpha-curve.png/svg`
- `fig-master-quadratic-roots.png/svg`
- `fig-derivation-chain-alpha.png/svg`
- `fig-action-terms.png/svg`
- Multiple `fig_*` numbered figures (Tier 1-3)

#### ch02 (Subatomic)
- `fig-scale-zoom.png/svg`
- `fig-voxel-structure.png/svg`
- `fig-wavefunction-collapse.png/svg`
- `fig-quantum-phenomena-panels.png/svg`
- `fig-standard-model-overview.png/svg`
- `fig-mexican-hat-and-mass.png/svg`

#### ch03-ch05 (Atomic through States)
- Various chemistry figures (bonds, orbitals, phase diagrams)
- ASCII-converted figures with hash suffixes

#### ch06 (Structures) - Professional Vector Graphics
- `fig-simple-cubic.png/svg`
- `fig-bcc.png/svg`
- `fig-fcc.png/svg`
- `fig-hcp.png/svg`
- `fig-diamond-cubic.png/svg`
- `fig-electron-sea.png/svg`
- `fig-metal-band-structure.png/svg`
- `fig-band-structure.png/svg`
- `fig-n-type-doping.png/svg`
- `fig-p-type-doping.png/svg`
- `fig-pn-junction.png/svg`
- `fig-mosfet.png/svg`
- `fig-lipid-bilayer.png/svg`
- `fig-dna-helix.png/svg`
- `fig-alpha-helix.png/svg`
- `fig-beta-sheet.png/svg`

#### ch07-ch14 (Planetary through Appendices)
- Primarily ASCII-converted figures (with hash suffixes)
- Some generated scientific figures

### 1.3 File Format Analysis

| Format | Count | Purpose |
|--------|-------|---------|
| PNG | 150+ | Primary format for PDF/print |
| SVG | 150+ | Vector format for HTML/scaling |
| TXT | 70+ | ASCII diagram source files |

All figures exist in **dual format** (PNG + SVG), following best practices for multi-format publishing.

---

## 2. Figure References in Chapter Files

### 2.1 Search Results

**Pattern Searched:** `![`, `@fig-`, `{{< figure`

**Result:** Zero matches in any `.qmd` chapter file.

### 2.2 Implications

The chapter files contain:
- 430+ code block markers (```)
- 1900+ table rows (pipe-delimited)
- Extensive ASCII art diagrams
- **No embedded image references**

This means the generated figures are **orphaned** - they exist but are not referenced by the manuscript source.

---

## 3. Planned vs Implemented Figures

### 3.1 From FIGURE_TRACKER.md

| Tier | Planned | Status per Tracker |
|------|---------|-------------------|
| Tier 1 (Essential) | 15 | All marked complete |
| Tier 2 (Highly Important) | 20 | All marked complete |
| Tier 3 (Supporting) | 30 | All marked complete |
| **Total** | **65** | **65 "complete"** |

### 3.2 From FIGURE_PLAN.md

The plan states:
- "116 images across 66/68 chapter files" - **Not verified in source**
- "0 ASCII placeholder diagrams" - **False, many remain**
- Figures "implemented" for all major chapters - **Generated, not embedded**

### 3.3 Reality Check

| Claim | Actual Status |
|-------|---------------|
| Figures generated | TRUE |
| Figures embedded in chapters | FALSE |
| ASCII diagrams replaced | PARTIALLY (files exist, but chapters still have ASCII) |
| Cross-references working | NOT APPLICABLE (no refs) |

---

## 4. ASCII Diagrams Still in Chapters

### 4.1 Chapters with Code Block ASCII Art

Based on code block counts, chapters with significant ASCII diagrams:

| Chapter | Code Blocks | Likely ASCII Diagrams |
|---------|-------------|----------------------|
| 4.1-chemical-bonds.qmd | 16 | Bond diagrams |
| 6.4-biological-structures.qmd | 16 | Structure diagrams |
| 4.2-simple-molecules.qmd | 14 | Molecule diagrams |
| 4.3-complex-molecules.qmd | 14 | Complex molecules |
| 13.3-return-to-void.qmd | 14 | Conceptual diagrams |
| 6.3-semiconductors.qmd | 12 | Device diagrams |
| 4.4-macromolecules.qmd | 12 | Polymer structures |
| 8.2-main-sequence.qmd | 12 | Star diagrams |
| 11.1-black-holes.qmd | 12 | Black hole diagrams |
| 6.1-crystal-lattices.qmd | 10 | Crystal structure ASCII |

### 4.2 Sample ASCII Diagrams Found

From `6.1-crystal-lattices.qmd`:
```
  ○───○
 /|  /|
○───○ |
| ○─|─○
|/  |/
○───○
```

From `13.3-return-to-void.qmd`:
Multiple conceptual diagrams showing cosmic evolution cycles.

---

## 5. Tables Inventory

### 5.1 Table Usage

All 71 chapter files contain pipe-delimited tables. Total occurrences: 1,905.

### 5.2 Heavy Table Users

| Chapter | Table Rows |
|---------|------------|
| 14.4-particle-catalog.qmd | 204 |
| 14.5-assumption-ledger.qmd | 172 |
| 14.1-constants-reference.qmd | 103 |
| 14.9-experimental-predictions.qmd | 90 |
| 1.10-lemniscate-alpha.qmd | 80 |
| 2.3-the-particle-zoo.qmd | 75 |
| 14.7-sloop-formalization.qmd | 68 |

### 5.3 Table Quality Assessment

Tables appear to be:
- Consistently formatted with pipe delimiters
- Properly aligned (based on structure)
- Using Quarto's standard Markdown table syntax

---

## 6. Evidence Visuals (Additional Figures)

### 6.1 Planned Evidence Figures

From `EVIDENCE_VISUALS.md`, 22 additional figures are planned:

| ID | Purpose |
|----|---------|
| E01 | CODATA vs FTD alpha match |
| E02 | Master quadratic residuals |
| E03 | Coefficient sensitivity sweep |
| E04-E10 | Various precision/convergence plots |
| E11-E15 | Integer scan robustness |
| E16-E18 | Mass/coupling comparisons |
| E19-E22 | Look-elsewhere analysis |

### 6.2 Status

These evidence figures are **planned but implementation status unclear** (not found in figure directories).

---

## 7. Recommendations

### 7.1 Critical Actions Required

1. **Embed Figures in Chapters**
   - Run `replace_ascii_diagrams.py` or manually add image references
   - Use standard Quarto syntax: `![Caption](../figures/chXX/fig-name.png){#fig-id width="80%"}`

2. **Add Cross-References**
   - Add `@fig-*` references in text to enable Quarto's figure numbering

3. **Replace Remaining ASCII Diagrams**
   - Chapters 4.x, 6.x have extensive ASCII that should use generated figures
   - Crystal lattice ASCII can be replaced with ch06 vector graphics

4. **Generate Evidence Figures**
   - Run `generate_evidence_figures.py` for publication-quality validation graphics

### 7.2 Verification Steps

After embedding:
```bash
cd dissemination/manuscript
quarto render
# Check _book/ output for figure presence
```

### 7.3 Numbering Consistency

Recommended figure ID format:
- `#fig-<chapter>-<number>-<description>`
- Example: `#fig-01-01-three-states`

---

## 8. File Paths Reference

### 8.1 Key Directories

| Path | Contents |
|------|----------|
| `C:\Users\cpaci\Desktop\pbr_pedagogy\dissemination\manuscript\figures\` | All figure files |
| `C:\Users\cpaci\Desktop\pbr_pedagogy\dissemination\manuscript\chapters\` | Chapter .qmd files |
| `C:\Users\cpaci\Desktop\pbr_pedagogy\dissemination\manuscript\figures\individual\` | Figure generation scripts |

### 8.2 Key Documentation

| File | Purpose |
|------|---------|
| `FIGURE_PLAN.md` | Master figure plan |
| `FIGURE_TRACKER.md` | Implementation tracking |
| `EVIDENCE_VISUALS.md` | Evidence figure specs |
| `generate_figures.py` | Main generation script |
| `replace_ascii_diagrams.py` | ASCII replacement tool |

---

## Appendix A: Complete Figure File List

### A.1 ch00 Figures
- fig-ascii-index-2148ef3490.png/svg
- fig-intro-hierarchy-of-being.png/svg
- fig-epistemic-claim-types.png/svg
- fig-discrete-operators.png/svg
- fig-ontological-levels.png/svg

### A.2 ch01 Figures
- fig-void-three-states.png/svg
- fig-genesis-pair-production.png/svg
- fig-two-layers-split.png/svg
- fig-interference-constructive.png/svg
- fig-interference-destructive.png/svg
- fig-existence-cycle.png/svg
- fig-annihilation-burst.png/svg
- fig-universal-tick.png/svg
- fig-time-gate-and-lightcone.png/svg
- fig-force-unification.png/svg
- fig-constants-dependency.png/svg
- fig-lemniscate-alpha-curve.png/svg
- fig-master-quadratic-roots.png/svg
- fig-derivation-chain-alpha.png/svg
- fig-action-terms.png/svg
- fig-ascii-1-2-the-first-division-*.png/svg
- fig-ascii-1-5-the-cycle-of-existence-*.png/svg
- fig-ascii-1-10-lemniscate-alpha-*.png/svg
- fig_1_1_lemniscate_alpha.png
- fig_1_2_master_quadratic.png
- fig_1_3_three_state_transitions.png
- fig_1_4_causal_loop.png
- fig_1_6_interference.png
- fig_1_7_double_slit.png
- fig_1_11_force_comparison.png
- fig_1_12_yukawa_potential.png
- fig_2_1_helmholtz.png
- fig_2_2_discrete_continuous.png
- fig_2_3_ontological_levels.png
- fig_2_4_force_gradients.png
- fig_2_5_arc_length_alpha.png
- fig_3_1_tick_sequence.png

### A.3 ch06 Figures (High Quality Vector)
- fig-simple-cubic.png/svg
- fig-bcc.png/svg
- fig-fcc.png/svg
- fig-hcp.png/svg
- fig-diamond-cubic.png/svg
- fig-electron-sea.png/svg
- fig-metal-band-structure.png/svg
- fig-band-structure.png/svg
- fig-n-type-doping.png/svg
- fig-p-type-doping.png/svg
- fig-pn-junction.png/svg
- fig-mosfet.png/svg
- fig-lipid-bilayer.png/svg
- fig-dna-helix.png/svg
- fig-alpha-helix.png/svg
- fig-beta-sheet.png/svg

---

## Summary

The FTD manuscript has an extensive library of generated figures (150+ PNG, 150+ SVG) organized by chapter, but these figures are **not yet embedded in the chapter source files**. The FIGURE_PLAN.md and FIGURE_TRACKER.md indicate completion of figure *generation*, but the critical step of *embedding* has not been performed.

**Priority Action:** Run the embedding workflow to insert figure references into all chapter `.qmd` files before publication.

---

*Report generated: 2026-01-10*
*Audit scope: C:\Users\cpaci\Desktop\pbr_pedagogy\dissemination\manuscript\*
