# FTD Papers

Published and working papers for Foundational Ternary Dynamics.

## Directory Structure

```
papers/
├── *.tex, *.pdf         # Core papers (TeX+PDF pairs or PDF-only)
├── speculative/          # Speculative extensions (Millennium Prize problems, etc.)
├── src/                  # Source papers with complete TeX+PDF pairs
│   └── figures/          # Figures referenced by src/ and speculative/ papers
└── archive/              # Historical versions, superseded papers, unused figures
```

## Core Papers (this directory)

### With TeX Sources

| Paper | Description |
|-------|-------------|
| DERIV_CLOSURE_RENORMALIZATION | Closure under renormalization |
| PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY | Gauge coupling constants from lattice coordination geometry (.md + .tex + .pdf) |
| PAPER_LIFECYCLE_SOFTPLUS | Lifecycle of the softplus function |

### PDF-Only (no TeX source available)

| Paper | Description |
|-------|-------------|
| DERIV_ALPHA_INVERSE_LATTICE_GAUGE | Alpha inverse from lattice gauge theory |
| DERIV_EMERGENT_GRAVITY | Emergent gravity from lattice dynamics |
| DERIV_FUNDAMENTAL_CONSTANTS | Fundamental constants derivation |
| DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME | Gauge couplings in discrete spacetime |
| DERIV_QUANTUM_INFERENCE | Quantum inference framework |
| DERIV_SELF_REFERENCE_FOUR_INTEGERS | Self-referential four integers {3, 4, 7, 13} |
| DERIV_THERMODYNAMIC_REFLEXION | Thermodynamic reflexion |
| SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME | Master quadratic in discrete spacetime |
| SPEC_MASTER_QUADRATIC_PAPER | Master quadratic paper |

### Figures

| File | Used By |
|------|---------|
| fig1_cuboctahedron.png | PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY |

## speculative/

Speculative extensions and applications of FTD to open mathematical problems. All papers have complete TeX+PDF pairs.

| Paper | Description |
|-------|-------------|
| DERIV_CASIMIR_RATCHET | Casimir ratchet mechanism |
| DERIV_GEOMETRIC_BIOPHYSICS | Geometric biophysics applications |
| DERIV_GRAND_UNIFIED_MASS | Grand unified mass formula |
| DERIV_SONOLUMINESCENCE | Sonoluminescence from lattice dynamics |
| FTD_Finitude_Theorem | Why infinity cannot be physical |
| FTD_Navier_Stokes | Navier-Stokes regularity via FTD |
| FTD_Riemann_Hypothesis | Riemann Hypothesis connection |
| FTD_Yang_Mills_Mass_Gap | Yang-Mills mass gap via lattice confinement |
| LETTER_HERMITIAN_COPE | Open letter on the Hermitian inner product |

## src/

Source papers with complete TeX+PDF pairs (the canonical source of truth for these papers).

### Complete Papers (TeX + PDF)

| Paper | Description |
|-------|-------------|
| DERIV_ALPHA_PRECISION | Fine structure constant precision formula |
| DERIV_GSTAR_ALGEBRAIC_DESCENT | G* algebraic descent |
| DERIV_GSTAR_PERIOD_WEIGHT | G* period-weight connection |
| DERIV_GSTAR_THETA_IDENTITY | G* theta function identity |
| DERIV_SELF_ORGANIZED_CRITICALITY | Self-organized criticality |
| DERIV_SOFTPLUS_RELU_DUALITY | Softplus-ReLU duality |
| FOUND_ONTIC_CONSTANT_CHAIN | Ontic constant derivation chain |
| FOUND_ONTIC_INCOMPLETENESS | On the ontic incompleteness |
| FOUND_ONTOLOGICAL_INVERSION | Ontological inversion |
| FOUND_PRINCIPIA_ONTOLOGICA | Principia Ontologica |
| FTD_Constants_Reference | Complete constants reference card |
| FTD_Discrete_Continuous_Bridge | Discrete-continuous bridge |
| FTD_Instantiating_Formula | The instantiating formula |
| FTD_Lattice_Engine | Lattice engine technical paper |
| FTD_One_Unit_Final | One unit of existence (final version) |
| FTD_One_Unit_Narrative | One unit of existence (narrative) |
| FTD_One_Unit_of_Existence | One unit of existence (original) |
| ontic_derivation_chain | Ontic derivation chain |

### PDF-Only

| Paper | Notes |
|-------|-------|
| DERIV_GSTAR_CONTINUOUS_DISCRETE | No TeX source available |
| FTD_One_Unit_of_Existence_v2 | Version 2, no TeX source |
| One_Unit_of_Existence | Alternate format of FTD_One_Unit_of_Existence |

### TeX Fragments (no PDF)

| File | Notes |
|------|-------|
| gstar_final.tex | G* derivation fragment |
| gstar_holographic_resonance.tex | Holographic resonance fragment |
| appendix_proofs.tex | Shared proof appendix |

### Support Files

| File | Purpose |
|------|---------|
| gen_alpha_figures.py | Figure generation script |
| figures/ | Referenced figures (6 files) |

## archive/

Historical and superseded content.

| Item | Description |
|------|-------------|
| GSM/ | Geometric Standard Model (original fine-structure paper + TeX) |
| master_quadratic/ | White paper in 3 formats (.md, .tex, .qmd) |
| figures/ | 24 unreferenced figures from development |
| ARCH_GEOMETRIC_STANDARD_MODEL.pdf | Archived geometric standard model |
| ARCH_MASTER_QUADRATIC_DISCRETE_SPACETIME_V1.pdf | Master quadratic v1 |
| ARCH_REFLEXIVE_DYNAMICS.pdf | Reflexive dynamics |
| ON_THE_ONTIC_INCOMPLETENESS.pdf | Earlier version of FOUND_ONTIC_INCOMPLETENESS |
| SPEC_FTD_FINE_STRUCTURE_CONSTANT.docx | Fine structure constant (Word format) |
| SPEC_FTD_FINE_STRUCTURE_CONSTANT_V2.docx | Fine structure constant v2 (Word format) |

## Compilation

All TeX papers can be compiled with:

```bash
pdflatex <paper>.tex
pdflatex <paper>.tex  # Run twice for TOC/references
```

If `pdflatex` is not on your PATH (common on Windows), use the full MiKTeX path or add the MiKTeX `bin` directory to PATH.

Papers in `speculative/` and some in `src/` reference `figures/` relative to `src/`. Compile from within the appropriate directory.
