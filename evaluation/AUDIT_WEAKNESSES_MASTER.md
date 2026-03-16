# FTD Manuscript - Master Weaknesses Compilation
## Compiled from 18 Expert Agent Evaluations

---

## CRITICAL ISSUES (Cited by Multiple Agents)

### W-CRIT-1: Circularity in Integer Identification
**Cited by**: PHY-THEO, PHY-EXPT, MATH
- The integers {3, 4, 7, 13} are identified based on ability to reproduce known physics
- Constraints were designed knowing the target values
- This is fitting, not prediction
- **Location**: Chapter 1.10, throughout

### W-CRIT-2: Master Quadratic Is Imposed, Not Derived
**Cited by**: MATH, PHY-THEO
- The polynomial x² - 16(G*)²x + 16(G*)³ = 0 is chosen to produce desired roots
- "Four independent derivations" of coefficient 16 are not actually independent
- **Location**: Chapters 1.10a, 1.10b

### W-CRIT-3: Lorentz Invariance Recovery Incomplete
**Cited by**: PHY-THEO, QIS
- Cubic lattice fundamentally breaks Lorentz symmetry
- "Relational reinterpretation" is asserted not demonstrated
- No quantitative analysis of recovery at large scales
- **Location**: CLAUDE.md §14.2

### W-CRIT-4: Bell Violation Claims Not Rigorously Demonstrated
**Cited by**: QIS, PHIL
- sLoop mechanism claims S ≈ 2.83 but simulations show S ≤ 2
- Tension between local causality axiom and nonlocal correlations
- No mathematical proof that sLoop resolves Bell's theorem
- **Location**: Chapter 2.4, 14.7

### W-CRIT-5: No Alt Text on Images (WCAG Failure)
**Cited by**: ACCESS
- All 50+ examined images lack alt attributes
- Critical Level A accessibility failure
- **Location**: All _webbook/ HTML files

### W-CRIT-6: Tables Lack Accessibility Markup
**Cited by**: ACCESS
- Missing scope, caption, and proper associations
- Screen readers cannot interpret table structure
- **Location**: Throughout _webbook/

---

## SUBJECT MATTER WEAKNESSES

### Theoretical Physics (PHY-THEO)
- **W1**: Critical circularity in integer identification
- **W2**: SU(2)/SU(3) derivations weaker than U(1)
- **W3**: Lorentz invariance recovery incomplete
- **W4**: GR correspondence is superficial
- **W5**: Manifestation mechanism lacks rigorous foundation
- **W6**: Decay rate γ = α is imposed
- **W7**: Quantum gravity not addressed
- **W8**: Continuum limit asserted not proven
- **W9**: No engagement with lattice QFT literature

### Experimental Physics (PHY-EXPT)
- **W1**: Most predictions are retrodictions (ratio ~25:5)
- **W2**: No proper uncertainty quantification
- **W3**: "10^-28 probability" claim unsupported
- **W4**: Some falsification criteria operationally untestable
- **W5**: Precision formula appears post-hoc
- **W6**: Cloud-9 "confirmation" overstated
- **W7**: Proton decay range spans order of magnitude
- **W8**: Cherry-picking of "consistent" observations

### Mathematics (MATH)
- **W1**: Quadratic form chosen, not derived
- **W2**: "Four independent derivations" of 16 are not independent
- **W3**: Framework integers are imposed, not derived
- **W4**: CM selection argument is incomplete
- **W5**: Precision formula has free parameters
- **W6**: Statistical claims overstated
- **W7**: α_G derivation has circular reasoning
- **W8**: Missing proofs for [THEOREM] claims

### Philosophy (PHIL)
- **W1**: Ternary state derivation insufficiently justified
- **W2**: Circular reasoning in space emergence
- **W3**: Modal constraint reification problem
- **W4**: Noetic mass is operationally undefined
- **W5**: sLoop-Bell mechanism underspecified
- **W6**: Consciousness definition may be vacuous
- **W7**: Deflationary move on qualia too quick
- **W8**: Insufficient comparison to rival positions

### Cosmology (COSMO)
- **W1**: Inflaton identification with mean flux is ad hoc
- **W2**: Dark matter mechanism internally inconsistent
- **W3**: First-order electroweak transition assumed
- **W4**: Λ = α^57 is numerology without mechanism
- **W5**: No power spectrum or BAO predictions
- **W6**: NFW halo profile not derived
- **W7**: "Return to Void" is philosophy, not physics

### Astrophysics (ASTRO)
- **W1**: FTD claims remain qualitative mappings
- **W2**: Missing numerical predictions distinguishing FTD
- **W3**: Information paradox "resolution" is speculative
- **W4**: No specific falsifiable astrophysical predictions

### Chemistry (CHEM)
- **W1**: Fundamental scale separation problem (10^25 factor)
- **W2**: Oversimplified triad model ignores QCD
- **W3**: Unjustified binding energy formula
- **W4**: Hardcoded shell radii in simulation
- **W5**: No molecular wavefunction construction
- **W6**: Missing many-body quantum chemistry

### Materials Science (MAT-SCI)
- **W1**: Zero quantitative materials predictions
- **W2**: Missing scale-bridging mechanism
- **W3**: Terminology substitution without new physics
- **W4**: Framework integers unused for materials

### Biophysics (BIO-PHYS)
- **W1**: No quantitative biological predictions
- **W2**: Vocabulary substitution without explanation
- **W3**: Missing key biophysics topics
- **W4**: Oversimplified gate-count cognition model
- **W5**: No contact with experimental biophysics
- **W6**: Pseudoscientific societal noetics section

### Quantum Information (QIS)
- **W1**: Confusion about Bell inequalities (local vs quantum)
- **W2**: Hilbert space is constructed, not emergent
- **W3**: sLoop lacks mathematical rigor
- **W4**: Born rule derivations not truly independent
- **W5**: No treatment of quantum computing primitives
- **W6**: Consciousness extensions unfalsifiable

---

## FUNCTIONAL WEAKNESSES

### Pedagogy (PEDA)
- **W1**: Severe audience mismatch (claims accessibility, requires grad level)
- **W2**: Steep difficulty gradient in Part 0
- **W3**: Missing worked examples
- **W4**: No exercises or practice problems
- **W5**: Glossary fragmentation

### Accessibility (ACCESS)
- **W1**: CRITICAL: No alt text on images
- **W2**: CRITICAL: Tables lack accessibility markup
- **W3**: HIGH: MathJax accessibility not configured
- **W4**: MEDIUM: Insufficient focus indicators
- **W5**: MEDIUM: Marginal color contrast in some areas

### Visualization (VIS)
- **W1**: Secondary palettes not colorblind-safe
- **W2**: Some text sizes below 10pt
- **W3**: Missing alt-text metadata
- **W4**: Reproducibility gaps in import paths
- **W5**: Some figures overly dense

### Technical Writing (TECH)
- **W1**: Inconsistent parameter naming (KB vs K_B vs kappa_B)
- **W2**: Notation variations across chapters
- **W3**: Forward reference gaps
- **W4**: Table caption inconsistencies
- **W5**: Passive voice overuse
- **W6**: Code block language specification inconsistent

### Citation (CITE)
- **W1**: Inconsistent citation density across chapters
- **W2**: Missing mathematical citations (elliptic functions)
- **W3**: Philosophical attribution gaps
- **W4**: Missing negative results citations
- **W5**: Duplicate bibliography entries

### Build (BUILD)
- **W1**: Broken import paths in figure scripts
- **W2**: No single-command build orchestration
- **W3**: Missing centralized build documentation
- **W4**: External tool versions not pinned
- **W5**: Inconsistent output directories

### User Experience (UX)
- **W1**: Overwhelming sidebar navigation (91 items)
- **W2**: No reading progress indicators
- **W3**: No dark mode implementation
- **W4**: Large search index should be lazy-loaded
- **W5**: Missing in-page TOC for complex chapters

### Information Architecture (ARCH)
- **W1**: Chapter numbering gaps (2.8-2.14 missing)
- **W2**: Unbalanced part sizes (Book I: 20 chapters vs Book XV: 1)
- **W3**: Mixed appendix content (reference + technical)
- **W4**: Limited navigational metadata
- **W5**: Orphan chapter (2.15-the-alpha-ladder.qmd)

---

## TOP 10 CRITICAL WEAKNESSES

1. **Circularity**: Framework integers selected knowing targets, not derived
2. **Bell Violations**: Claims exceed what simulations demonstrate
3. **Image Accessibility**: WCAG Level A failure (no alt text)
4. **Lorentz Recovery**: Cubic lattice breaks symmetry with no rigorous recovery
5. **Retrodictions vs Predictions**: Most "derivations" are fits to known values
6. **Uncertainty Quantification**: No proper error propagation or statistics
7. **Scale Bridging**: No path from Planck to atomic/materials scales
8. **Master Quadratic**: Polynomial is chosen, not derived
9. **Audience Mismatch**: Claims accessibility but requires graduate training
10. **Build Reproducibility**: Broken import paths in figure generation

---

*Compiled: 2026-01-25*
*Source: 18 Expert Agent Findings*
