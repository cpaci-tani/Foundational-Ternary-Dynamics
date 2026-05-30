# Manuscript v2 Rewrite Checklist

**Target:** Working physicists (QFT/GR/lattice gauge theory assumed)
**Style:** Theorem-proof-remark, every claim epistemically tagged
**Started:** April 12, 2026

## Status Key
- `[ ]` Not started
- `[D]` Draft complete
- `[R]` Under review
- `[X]` Final

---

## Prolegomena (write LAST — Phase 5)

- [ ] P.1 Epistemic Framework and Conventions
- [ ] P.2 Mathematical Prerequisites

## Book I: The Lattice and Its Algebra (Phase 2)

- [D] Ch 1 The Five Postulates — DRAFTED April 12
- [D] Ch 2 The Two-Layer Ontology — DRAFTED April 12
- [D] Ch 3 Why D = 3 — DRAFTED April 12
- [D] Ch 4 The Moore Neighborhood Decomposition — DRAFTED April 12
- [D] Ch 5 Gauge Groups from Geometry — DRAFTED April 12
- [D] Ch 6 The BCC Eigenvalue and Watson Identity — DRAFTED April 12
- [D] Ch 7 The Bridge Constant G* — DRAFTED April 12
- [D] Ch 8 The Master Quadratic — DRAFTED April 12

## Book II: Physical Content (Phase 3)

- [D] Ch 9 The Roots: alpha and N_c — DRAFTED April 12
- [D] Ch 10 The Framework Integers — DRAFTED April 12
- [D] Ch 11 The Precision Formula — DRAFTED April 12
- [D] Ch 12 The Mass Spectrum — DRAFTED April 12
- [D] Ch 13 The Complete Standard Model — DRAFTED April 12
- [D] Ch 14 The Action Principle and Force Laws — DRAFTED April 12
- [D] Ch 15 Gravity from the Lattice — DRAFTED April 12
- [D] Ch 16 Quantum Mechanics from Flux — DRAFTED April 12
- [D] Ch 17 Bell's Theorem and Tsirelson's Bound — DRAFTED April 12
- [D] Ch 18 Matter-Antimatter and Confinement — DRAFTED April 12

## Book III: The Observer and the Cosmos (Phase 4)

- [D] Ch 19 The Observer Formalism — DRAFTED April 12
- [D] Ch 20 Measurement: Type III_1 to Type I — DRAFTED April 12
- [D] Ch 21 Reference frame context and Self-Reference — DRAFTED April 12
- [D] Ch 22 Dark Matter and Dark Energy — DRAFTED April 12
- [D] Ch 23 The Vacuum Energy Resolution — DRAFTED April 12
- [D] Ch 24 Experimental Predictions and Status — DRAFTED April 12

## Books IV-XV: Editorial Pass (Phase 6)

### Editorial Checklist (apply to each chapter)
1. [ ] Cross-references updated (old ch numbers -> new)
2. [ ] Epistemic tags on every claim
3. [ ] Bell terminology updated (emergent, three-level)
4. [ ] BCC/Moore terminology updated
5. [ ] Observer terminology updated (O-operation, Activate_C)
6. [ ] Images verified (paths resolve)
7. [ ] Notation matches P.2
8. [ ] Scope limitations noted where needed
9. [ ] April 2026 results incorporated
10. [ ] Master quadratic refs point to Ch 8

### Chapters (copy from v1, apply checklist)
- [ ] 3.1 Stable Structures (-> Book IV opening)
- [ ] 3.2 The Periodic Table
- [ ] 3.3 Electron Dynamics
- [ ] 3.4 Nuclear Physics
- [ ] 4.1 Chemical Bonds
- [ ] 4.2 Simple Molecules
- [ ] 4.3 Complex Molecules
- [ ] 4.4 Macromolecules
- [ ] 5.1 States of Matter
- [ ] 5.2 Phase Transitions
- [ ] 5.3 Exotic States
- [ ] 6.1 Crystal Lattices
- [ ] 6.2 Metals and Conductors
- [ ] 6.3 Semiconductors
- [ ] 6.4 Biological Structures
- [ ] 7.1 Gravity Wells
- [ ] 7.2 Atmospheres
- [ ] 7.3 Geology
- [ ] 7.4 Magnetospheres
- [ ] 8.1 Stellar Formation
- [ ] 8.2 Main Sequence
- [ ] 8.3 Stellar Nucleosynthesis
- [ ] 8.4 Stellar Death
- [ ] 8.5 Compact Objects
- [ ] 9.1 Galaxy Formation
- [ ] 9.2 Galaxy Types
- [ ] 9.3 The Milky Way
- [ ] 9.4 Galaxy Interactions
- [ ] 10.1 Large-Scale Structure
- [ ] 10.2 Dark Matter (heavy edit -> ref Ch 22)
- [ ] 10.3 Dark Energy (heavy edit -> ref Ch 22)
- [ ] 10.4 Cosmological Epochs
- [ ] 11.1 Black Holes
- [ ] 11.2 Gravitational Waves
- [ ] 11.3 Cosmic Rays
- [ ] 11.4 Vacuum Fluctuations
- [ ] 12.0 Definition of Life
- [ ] 12.1 Self-Organization
- [ ] 12.1a Hierarchy of Sentience
- [ ] 12.2 Information and Entropy
- [ ] 12.3 Complexity
- [ ] 12.4 The Anthropic Window
- [ ] 13.1 Heat Death
- [ ] 13.2 Alternative Endings
- [ ] 13.3 Return to Void
- [ ] 14.1 Constants Reference
- [ ] 14.2 Equations Reference
- [ ] 14.3 Glossary
- [ ] 14.4 Particle Catalog
- [ ] 14.5 Assumption Ledger (heavy edit)
- [ ] 14.6 Self-Consistency
- [ ] 14.8 Information Quantification
- [ ] 14.9 Experimental Predictions (-> ref Ch 24)
- [ ] 14.10 Number Theory
- [ ] 15.1 Observational Confirmations (-> ref Ch 24)

## Infrastructure

- [X] Directory structure created
- [X] styles.css copied from v1
- [X] media/ symlinked
- [X] references.bib copied
- [X] _quarto.yml configured
- [X] CHECKLIST.md created
- [X] Skeleton chapter files created
- [X] index.qmd written
- [X] preface.qmd written
- [ ] Build test (quarto render)

## Session Summary (April 12, 2026)

### Phase 1-5: Core manuscript
- 26 new chapters drafted (P.1-P.2, Ch 1-24, index, preface)
- 57 chapters copied from v1 (Books IV-XV)
- 83 total chapter files in manuscript_v2/src/chapters/
- _quarto.yml fully configured with 15 parts
- Media symlinked to v1 (365 images shared)
- All infrastructure files in place (CSS, bib, TeX preambles)

### Phase 6: Editorial pass (4 parallel agents)
- 54 chapters reviewed against 10-item checklist
- 41 chapters: CLEAN (no edits needed)
- 13 chapters: EDITED (cross-refs, forward refs, Bell updates, ledger notice)
- Edits by type:
  - Old chapter number updates: 4 files
  - Dark matter forward references (-> Ch 22): 2 files
  - Reference frame context forward references (-> Ch 21): 4 files
  - Assumption ledger v2 notice: 1 file
  - Predictions cross-ref (-> Ch 24): 2 files
  - Bell resolution note: 1 file
  - Dedup fix: 1 file
