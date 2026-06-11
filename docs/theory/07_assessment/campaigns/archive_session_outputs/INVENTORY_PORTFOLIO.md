# Portfolio Inventory — Reframe Deployment Phase 1

**Date:** 2026-04-19
**Scope:** all editable artifacts outside `docs/theory/` (already reframed) — papers, manuscripts, whitepaper, narrative book, notebooks, interactive HTML simulations.
**Heuristic for "Likely affected":** based on filename / topic. **LIKELY** = explicit invocation of limits, infinity, completion, continuum, RG flow to fixed points, thermodynamic limit, infinite lattice; **LIKELY-NOT** = parametric, geometric, or pedagogical content with no obvious limit-claim; **UNKNOWN** = needs file inspection.

---

## Summary
- **Total artifacts enumerated:** 280 (excluding generated `.aux/.log/.out/.toc`, image files, archive subdirs)
- **Editable sources** (`.tex/.md/.qmd/.ipynb/.html`): 267
- **PDF-only** (defer to source recovery): 13
- **Likely affected by reframe** (heuristic): ~85
- **Likely-not affected** (heuristic): ~155
- **Unknown / needs inspection:** ~40

**Cross-reference note:** `dissemination/manuscript_v2/src/chapters/` is the consolidated authoritative chapter set (83 files). `vol1/src/chapters` and `vol2/src/chapters` mirror subsets — likely build/preview splits, not authoritative source. `dissemination/manuscript/src/chapters/` (92 files) is the older v1 manuscript; v2 retains many of its chapters (numbered 3.x–13.x) verbatim, so reframe edits to v1 chapters MUST be propagated to v2.

---

## 1. docs/papers/ — formal papers (TeX sources)

### 1a. Top-level papers (with .tex source)
| File | Lines | Status | Likely affected | Cited |
|------|-------|--------|-----------------|-------|
| `docs/papers/DERIV_CLOSURE_RENORMALIZATION.tex` | 797 | current | **LIKELY** (renormalization, RG limits) | UNKNOWN |
| `docs/papers/PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` | 500 | current | **LIKELY** (lattice geometry → gauge limits) | UNKNOWN |
| `docs/papers/PAPER_GSTAR_BRIDGE_CONSTANT.tex` | 658 | current | **LIKELY** (G* limits, completed identities) | YES (memory) |
| `docs/papers/PAPER_GSTAR_IDENTITIES.tex` | 675 | current | LIKELY-NOT (algebraic identities, finite) | UNKNOWN |
| `docs/papers/PAPER_LIFECYCLE_SOFTPLUS.tex` | 858 | current | LIKELY-NOT | UNKNOWN |
| `docs/papers/PAPER_MISSING_RATIO.tex` | 373 | current | UNKNOWN | UNKNOWN |
| `docs/papers/PAPER_RATIO_AND_PRODUCT.tex` | 310 | current | LIKELY-NOT (algebraic) | UNKNOWN |
| `docs/papers/PAPER_RATIO_AND_THE_ARROW.tex` | 292 | current | LIKELY-NOT (already-published, time-asymmetry argument) | YES (CLAUDE.md) |
| `docs/papers/PAPER_TWO_RACES.tex` | 464 | current | UNKNOWN | UNKNOWN |
| `docs/papers/ratio_and_the_arrow.tex` | 176 | **draft / superseded by uppercase variant** | LIKELY-NOT | NO |
| `docs/papers/PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.md` | 449 | current (markdown twin) | **LIKELY** | UNKNOWN |
| `docs/papers/PAPER_PATH.md` | 69 | meta / navigation | LIKELY-NOT | YES (likely) |
| `docs/papers/README.md` | 165 | meta | LIKELY-NOT | YES |

### 1b. Papers in `docs/papers/src/` (production sources)
| File | Lines | Status | Likely affected | Cited |
|------|-------|--------|-----------------|-------|
| `docs/papers/src/DERIV_ALPHA_PRECISION.tex` | 579 | current | **LIKELY** (precision-cascade fit, post-hoc-series concerns) | YES |
| `docs/papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex` | 646 | current | LIKELY-NOT | UNKNOWN |
| `docs/papers/src/FOUND_ONTIC_INCOMPLETENESS.tex` | 618 | current | **LIKELY** (incompleteness  infinity ontology) | UNKNOWN |
| `docs/papers/src/FTD_Discrete_Continuous_Bridge.tex` | 843 | current | **LIKELY** (continuum-limit framing) | UNKNOWN |
| `docs/papers/src/FTD_One_Unit_Final.tex` | 543 | current | UNKNOWN | UNKNOWN |
| `docs/papers/src/PAPER_0A_PERIOD_DESCENT.tex` | 353 | current | UNKNOWN | UNKNOWN |
| `docs/papers/src/PAPER_0B_THREE_CONSTANTS.tex` | 303 | current | LIKELY-NOT | UNKNOWN |
| `docs/papers/src/PAPER_0E_ARITHMETIC_GEOMETRIC_EQUIVALENCE.tex` | 326 | current | UNKNOWN | UNKNOWN |
| `docs/papers/src/PAPER_1A_WATSON_LATTICE_BRIDGE.tex` | 301 | current | **LIKELY** (Watson identity, BCC infinite-lattice integral) | YES |
| `docs/papers/src/PAPER_2A_MASTER_QUADRATIC.tex` | 491 | current | LIKELY-NOT (algebraic quadratic) | YES (high) |
| `docs/papers/src/PAPER_3A_PHYSICAL_IDENTIFICATION.tex` | 293 | current | LIKELY-NOT (identifications) | YES |
| `docs/papers/src/ontic_derivation_chain.tex` | 596 | current | UNKNOWN | YES |

### 1c. Papers in `docs/papers/speculative/` — flagged as exploratory
| File | Lines | Status | Likely affected | Cited |
|------|-------|--------|-----------------|-------|
| `docs/papers/speculative/DERIV_CASIMIR_RATCHET.tex` | 214 | speculative | LIKELY-NOT | NO |
| `docs/papers/speculative/DERIV_GEOMETRIC_BIOPHYSICS.tex` | 293 | speculative | LIKELY-NOT | NO |
| `docs/papers/speculative/DERIV_GRAND_UNIFIED_MASS.tex` | 314 | speculative | UNKNOWN | NO |
| `docs/papers/speculative/DERIV_SONOLUMINESCENCE.tex` | 222 | speculative | LIKELY-NOT | NO |
| `docs/papers/speculative/FTD_Finitude_Theorem.tex` | 714 | speculative | **LIKELY (HIGH PRIORITY)** — title is exactly the reframe topic | UNKNOWN |
| `docs/papers/speculative/FTD_Navier_Stokes.tex` | 543 | speculative | **LIKELY** (continuum PDE + lattice) | NO |
| `docs/papers/speculative/FTD_Riemann_Hypothesis.tex` | 633 | speculative | **LIKELY** (zeros at infinity, completed product) | NO |
| `docs/papers/speculative/FTD_Yang_Mills_Mass_Gap.tex` | 648 | speculative | **LIKELY** (continuum YM, infinite-volume mass gap) | NO |
| `docs/papers/speculative/LETTER_HERMITIAN_COPE.tex` | 134 | speculative letter | UNKNOWN | NO |

---

## 2. dissemination/papers/ — newer papers
| File | Lines | Status | Likely affected | Cited |
|------|-------|--------|-----------------|-------|
| `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` | 999 | **CURRENT — flagship EFT paper** | **LIKELY (HIGH PRIORITY)** — Wilson EFT requires explicit ε-L treatment, plateau claims, retracted L→∞ claim noted in CLAUDE.md | YES (top-level) |

---

## 3. dissemination/whitepaper/
| File | Lines | Status | Likely affected | Cited |
|------|-------|--------|-----------------|-------|
| `dissemination/whitepaper/FTD_Whitepaper.tex` | 512 | current public-facing | **LIKELY** (overview will mention infinite lattice / continuum limits) | YES (entry point) |

---

## 4. dissemination/manuscript/ — v1 (96 chapters, Quarto)

**Total:** 92 chapter `.qmd` files + index/preface/glossary/about/symbols-glossary (5 ancillary). Total 26,712 chapter lines.

### 4a. Chapters likely affected (heuristic — limits, completion, continuum, infinity)
| File | Lines | Reason |
|------|-------|--------|
| `0.0-formal-logic.qmd` | 457 | Formal logic — likely discusses ω-completeness |
| `0.2-mathematics.qmd` | 265 | Math foundations |
| `0.4-event-constraint-ontology.qmd` | 292 | Ontology — needs ε-L review |
| `0.5-computational-ontology.qmd` | 297 | Computational ontology — finitude central |
| `1.0-before-the-void.qmd` | 368 | Void / pre-existence framing |
| `1.10-lemniscate-alpha.qmd` | 907 | Convergent series / completed identities |
| `1.10b-master-quadratic-derivation.qmd` | 730 | Derivation chain |
| `1.11-the-action-principle.qmd` | 266 | Action integral over space |
| `1.12-gravity-from-integers.qmd` | 247 | Likely [DERIVED] claims |
| `1.13-grand-unification.qmd` | 361 | RG-flow language likely |
| `1.15-vacuum-energy.qmd` | 265 | Vacuum integrals (UV/IR limits) |
| `2.1-the-planck-scale.qmd` | 133 | Scale limits |
| `2.15-the-alpha-ladder.qmd` | 69 | Convergence ladder |
| `2.4-quantum-phenomena.qmd` | 416 | Continuum QM emergence |
| `10.3-dark-energy.qmd` | 266 | Cosmological-constant integrals |
| `10.4-cosmological-epochs.qmd` | 397 | Cosmic history at infinite time |
| `11.1-black-holes.qmd` | 276 | Horizons / infinities |
| `13.1-heat-death.qmd` | 265 | t→∞ |
| `13.2-alternative-endings.qmd` | 259 | t→∞ |
| `13.3-return-to-void.qmd` | 284 | t→∞ |
| `14.5-assumption-ledger.qmd` | 602 | **MUST UPDATE** — reframe is a foundational change to assumption set |
| `14.6-self-consistency.qmd` | 405 | Will reference completed-infinity claims |
| `14.7-sloop-formalization.qmd` | 761 | Formal structure |
| `14.8-information-quantification.qmd` | 634 | Info content of infinite lattice |
| `14.9-experimental-predictions.qmd` | 447 | Predictions framing |
| `15.1-observational-confirmations.qmd` | 571 | Cosmological observations |

### 4b. Chapters likely-not affected (concrete physics, no limit-claims)
Compactly: chapters in series **3.x** (atoms), **4.x** (molecules), **5.x** (matter states), **6.x** (solids/biology), **7.x** (planetary), **8.x** (stellar), **9.x** (galactic) — total 30 chapters, 5,406 lines. These are descriptive applications, not foundational claims. Spot-check recommended but unlikely to need edits.

### 4c. Other v1 chapters needing brief inspection (UNKNOWN)
`0.1`, `0.3`, `0.6`, `1.1`, `1.2`, `1.2a`, `1.3`–`1.9`, `1.8a`, `1.10a`, `1.14`, `2.2`, `2.3`, `2.5`–`2.7`, `10.1`, `10.2`, `11.2`–`11.4`, `12.0`–`12.5`, `14.1`–`14.4`, `14.10` — ~30 chapters.

### 4d. Manuscript ancillary
| File | Lines | Affected | Cited |
|------|-------|----------|-------|
| `manuscript/src/index.qmd` | 206 | UNKNOWN | YES |
| `manuscript/src/preface.qmd` | 218 | LIKELY (sets framing) | YES |
| `manuscript/src/symbols-glossary.qmd` | 219 | LIKELY-NOT | YES |
| `manuscript/src/about.qmd` | 66 | LIKELY-NOT | YES |

---

## 5. dissemination/manuscript_v2/ — v2 physicist-targeted (83 chapters)

**Authoritative location:** `manuscript_v2/src/chapters/` (consolidated, 83 files, 18,489 lines).
**Mirror:** `vol1/src/chapters/` (35 files including 14.7 and a P1/P2 prelude pair) + `vol2/src/chapters/` (45 files). These appear to be a vol1/vol2 split — confirm before editing.

### 5a. v2-NEW chapters (numbered 01–24 + P1/P2 — physicist-targeted rewrites)
All ~26 new chapters are **LIKELY affected** because they are the rigorous re-presentation:
| File | Lines | Heuristic |
|------|-------|-----------|
| `P1-epistemic-framework.qmd` | 81 | **HIGH** — establishes epistemic ground rules |
| `P2-mathematical-prerequisites.qmd` | 137 | **HIGH** |
| `01-five-postulates.qmd` | 88 | **HIGH** — postulate set must reflect undefined-boundary lattice |
| `02-two-layer-ontology.qmd` | 81 | **HIGH** |
| `03-why-d-equals-3.qmd` | 87 | LIKELY |
| `04-moore-decomposition.qmd` | 108 | LIKELY |
| `05-gauge-groups.qmd` | 79 | LIKELY |
| `06-bcc-eigenvalue-watson.qmd` | 121 | **LIKELY** (Watson is infinite-lattice integral) |
| `07-bridge-constant-gstar.qmd` | 85 | LIKELY-NOT (algebraic) |
| `08-master-quadratic.qmd` | 107 | LIKELY-NOT |
| `09-roots-alpha-and-nc.qmd` | 88 | LIKELY-NOT |
| `10-framework-integers.qmd` | 94 | LIKELY-NOT |
| `11-precision-formula.qmd` | 71 | UNKNOWN |
| `12-mass-spectrum.qmd` | 97 | LIKELY-NOT |
| `13-complete-standard-model.qmd` | 90 | UNKNOWN |
| `14-action-principle-forces.qmd` | 121 | LIKELY (action over space) |
| `15-gravity-from-lattice.qmd` | 82 | **LIKELY** (continuum GR emergence) |
| `16-qm-from-flux.qmd` | 101 | **LIKELY** (continuum QM emergence) |
| `17-bell-tsirelson.qmd` | 87 | UNKNOWN |
| `18-matter-antimatter-confinement.qmd` | 99 | UNKNOWN |
| `19-observer-formalism.qmd` | 102 | UNKNOWN |
| `20-measurement.qmd` | 67 | UNKNOWN |
| `21-reference frame context.qmd` | 74 | LIKELY-NOT |
| `22-dark-matter-energy.qmd` | 57 | LIKELY |
| `23-vacuum-energy.qmd` | 73 | **LIKELY** |
| `24-predictions-status.qmd` | 76 | LIKELY (must reflect reframe-affected predictions) |

### 5b. v2-INHERITED chapters (numbered 3.x–13.x + 14.x + 15.1)
~57 chapters carried over verbatim from v1. **Coordinate edits with v1 to avoid drift.** See 4a/4b/4c above for breakdown.

### 5c. v2 ancillary
| File | Lines | Affected |
|------|-------|----------|
| `manuscript_v2/src/index.qmd` | 47 | UNKNOWN |
| `manuscript_v2/src/preface.qmd` | 17 | LIKELY (framing) |
| `manuscript_v2/CHECKLIST.md` | 158 | **MUST UPDATE** — track reframe progress |

---

## 6. dissemination/book/ — "The Golden Thread" narrative (53 .qmd)

**Total:** 46 chapter qmd + 5 appendices + index + preface = 53 files, 8,771 lines.

### 6a. Narrative chapters 00–35 (history of mysticism, 36 chapters)
**LIKELY-NOT affected** — historical/cultural narrative, not formal claims. Files: `00_prologue` through `35_the_search`. ~5,500 lines.

### 6b. Narrative chapters 36–45 (FTD presentation in narrative form, 10 chapters)
**LIKELY affected** — these chapters ARE the popular presentation of FTD's foundational claims:
| File | Lines | Reason |
|------|-------|--------|
| `36_the_integers.qmd` | 227 | Framework-integer story |
| `37_the_verification.qmd` | 242 | Verification claims |
| `38_the_unification.qmd` | 220 | Unification framing |
| `39_the_implications.qmd` | 165 | Implications of foundational claims |
| `40_the_bridge.qmd` | 207 | Bridge constants |
| `41_the_eternal_equation.qmd` | 232 | Master equation framing |
| `42_what_we_learned.qmd` | 199 | Conclusions |
| `43_the_unfinished_temple.qmd` | 165 | LIKELY (open questions) |
| `44_letter_to_future.qmd` | 173 | Forward-looking claims |
| `45_epilogue.qmd` | 152 | Final framing |

### 6c. Appendices
| File | Lines | Affected |
|------|-------|----------|
| `appendix_a_mathematical.qmd` | 169 | LIKELY |
| `appendix_b_timeline.qmd` | 155 | LIKELY-NOT |
| `appendix_c_sacred_numbers.qmd` | 216 | LIKELY-NOT |
| `appendix_d_glossary.qmd` | 244 | LIKELY (must mention undefined-boundary terminology) |
| `appendix_e_further_reading.qmd` | 183 | LIKELY-NOT |

(Appendix `.md` twins exist — 5 files — likely auto-generated from `.qmd`, treat `.qmd` as source of truth.)

| `book/index.qmd` | 67 | LIKELY-NOT |
| `book/preface.qmd` | 49 | LIKELY (sets framing) |
| `book/BOOK_OUTLINE.md` | 517 | **LIKELY** — restructure may need outline update |

---

## 7. dissemination/notebooks/ — Jupyter pedagogy (12 notebooks)

| File | Lines (raw JSON) | Affected | Cited |
|------|------------------|----------|-------|
| `00_introduction.ipynb` | 693 | LIKELY (intro framing) | UNKNOWN |
| `01_void_and_flux.ipynb` | 760 | LIKELY-NOT | UNKNOWN |
| `02_manifestation.ipynb` | 1,241 | LIKELY-NOT | UNKNOWN |
| `03_forces_and_fields.ipynb` | 863 | LIKELY-NOT | UNKNOWN |
| `04_binding_structures.ipynb` | 2,011 | LIKELY-NOT | UNKNOWN |
| `05_quantum_phenomena.ipynb` | 964 | **LIKELY** (continuum QM) | UNKNOWN |
| `06_constants_derivation.ipynb` | 737 | **LIKELY** (derivation chain) | UNKNOWN |
| `07_verification_suite.ipynb` | 955 | LIKELY (claims being verified) | UNKNOWN |
| `09_comprehensive_verification_executed.ipynb` | 1,044 | LIKELY | UNKNOWN |
| `10_genesis_to_atoms.ipynb` | 329 | LIKELY-NOT | UNKNOWN |
| `11_genesis_to_chemistry.ipynb` | 1,044 | LIKELY-NOT | UNKNOWN |
| `12_interactive_universe.ipynb` | 416 | LIKELY-NOT | UNKNOWN |

---

## 8. dissemination/interactive/ — HTML simulations (17 files)

All standalone — **LIKELY-NOT affected** because they're parameter-explorers and animations, not claim documents. Listed for completeness; spot-check labels/tooltips for "in the limit" wording.

| File | Lines |
|------|-------|
| `convergence_races.html` | 235 |
| `dual_convergence.html` | 422 |
| `electromagnetic_simulation.html` | 856 |
| `fermat_coil.html` | 289 |
| `fermat_dual_source.html` | 1,194 |
| `gauss_circle_explorer.html` | 1,143 |
| `hamiltonian_bridge_explorer.html` | 720 |
| `master_quadratic_explorer.html` | 408 |
| `octant_prime_explorer.html` | 884 |
| `potential_core_explorer.html` | 740 |
| `precision_cascade.html` | 229 (LIKELY — name suggests convergent series UI) |
| `prime_music.html` | 327 |
| `single_photon_source.html` | 1,005 |
| `strong_force_simulation.html` | 841 |
| `ternary_cube_27.html` | 643 |
| `unified_forces_simulation.html` | 1,093 |
| `weak_force_simulation.html` | 967 |

Plus `dissemination/FTD_Symbol_Cheatsheet.html` (1,161 lines, top-level) — **LIKELY** (will reference G* / α infinite-product identities).

---

## 9. PDF-only artifacts (defer until source recovery)

These 13 PDFs in `docs/papers/` have no `.tex` or `.md` companion. All require source recovery before reframe action; titles suggest several are central:

| PDF | Likely affected (by title) |
|-----|----------------------------|
| `DERIV_ALPHA_INVERSE_LATTICE_GAUGE.pdf` | **LIKELY** |
| `DERIV_EMERGENT_GRAVITY.pdf` | **LIKELY** |
| `DERIV_FUNDAMENTAL_CONSTANTS.pdf` | LIKELY |
| `DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME.pdf` | **LIKELY** |
| `DERIV_QUANTUM_INFERENCE.pdf` | UNKNOWN |
| `DERIV_SELF_REFERENCE_FOUR_INTEGERS.pdf` | UNKNOWN |
| `DERIV_THERMODYNAMIC_REFLEXION.pdf` | LIKELY |
| `FTD_KMS_Thermal_Time.pdf` | **LIKELY** (KMS at infinite volume) |
| `FTD_Modular_Structure.pdf` | UNKNOWN |
| `FTD_Spatial_Correlations.pdf` | LIKELY (correlation functions, IR cutoff) |
| `FTD_Thermodynamic_Limit.pdf` | **HIGHEST PRIORITY** — title is exactly the reframe topic |
| `SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME.pdf` | LIKELY (likely superseded by `PAPER_2A` source) |
| `SPEC_MASTER_QUADRATIC_PAPER.pdf` | LIKELY (likely superseded by `PAPER_2A` source) |

**Action:** before editing any of these, find or regenerate the `.tex` source (check git history, archive/, or treat as published-immutable and add an addendum/erratum instead).

---

## 10. Generated / temporary files (skipped from inventory)

In `docs/papers/`: 27 files matching `*.aux | *.log | *.out | *.toc` from LaTeX builds. Plus image artifacts (`.png`, `figures/`). Skipped per spec.

---

## 11. Recommended priority order for classification (top 20)

Ordered by combined likelihood-of-affected × downstream-citation-impact:

1. **`docs/papers/speculative/FTD_Finitude_Theorem.tex`** (714 L) — title literally about finitude; central to reframe
2. **`docs/papers/PDF: FTD_Thermodynamic_Limit.pdf`** — title literally the reframe topic; recover source first
3. **`dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`** (999 L) — flagship; CLAUDE.md notes retracted L→∞ claim
4. **`dissemination/whitepaper/FTD_Whitepaper.tex`** (512 L) — public entry point
5. **`dissemination/manuscript_v2/src/chapters/01-five-postulates.qmd`** (88 L) — postulate set is the foundation
6. **`dissemination/manuscript_v2/src/chapters/P1-epistemic-framework.qmd`** (81 L) — sets epistemic ground rules
7. **`docs/papers/src/PAPER_1A_WATSON_LATTICE_BRIDGE.tex`** (301 L) — Watson identity is infinite-lattice integral
8. **`dissemination/manuscript_v2/src/chapters/06-bcc-eigenvalue-watson.qmd`** (121 L) — same as above, in manuscript
9. **`docs/papers/src/FOUND_ONTIC_INCOMPLETENESS.tex`** (618 L) — incompleteness  infinity
10. **`docs/papers/src/FTD_Discrete_Continuous_Bridge.tex`** (843 L) — continuum-limit framing
11. **`docs/papers/DERIV_CLOSURE_RENORMALIZATION.tex`** (797 L) — RG / closure
12. **`docs/papers/PAPER_GSTAR_BRIDGE_CONSTANT.tex`** (658 L) — G* identities
13. **`docs/papers/src/DERIV_ALPHA_PRECISION.tex`** (579 L) — convergence claims
14. **`dissemination/manuscript/src/chapters/14.5-assumption-ledger.qmd`** (602 L) — assumption ledger MUST list reframe
15. **`dissemination/manuscript/src/chapters/14.7-sloop-formalization.qmd`** (761 L) — formal structure
16. **`dissemination/manuscript/src/chapters/14.8-information-quantification.qmd`** (634 L) — info content of lattice
17. **`docs/papers/speculative/FTD_Yang_Mills_Mass_Gap.tex`** (648 L) — YM continuum claim
18. **`docs/papers/speculative/FTD_Riemann_Hypothesis.tex`** (633 L) — completed-product claim
19. **`docs/papers/speculative/FTD_Navier_Stokes.tex`** (543 L) — continuum PDE
20. **`dissemination/manuscript_v2/CHECKLIST.md`** (158 L) — track reframe progress in v2 build

---

## 12. Workflow recommendations

1. **Phase 2 — Triage:** open the 20 priority artifacts, classify each statement against `AUDIT_INFINITY_REFRAME.md` triage (survives / needs ε-L restatement / needs re-derivation / fails).
2. **Phase 3 — Coordinated edits:** edit in v2 first (newer, physicist-targeted), then propagate to v1 inherited chapters and to whitepaper. Notebooks last (pedagogical lag is acceptable).
3. **Phase 4 — PDF-only papers:** check `archive/` for `.tex` sources before treating as published-immutable. If source recoverable, edit in place; if not, attach an addendum citing `AUDIT_INFINITY_REFRAME.md`.
4. **Phase 5 — Cross-reference checks:** ensure `manuscript/src/chapters/14.5-assumption-ledger.qmd` (and v2 equivalent if present) explicitly lists the undefined-boundary commitment, and ensure `book/BOOK_OUTLINE.md` is updated if narrative chapters 36–45 require restructuring.
5. **Sanity:** speculative-folder papers (`docs/papers/speculative/`) are already flagged exploratory — they need reframe edits but may be lower priority for reader-facing release.
