# FTD Papers

Published outputs and paper source trees for Foundational Ternary Dynamics.

## Working Rule

- Active PDFs live directly in `docs/papers/`.
- Archived PDFs live under `docs/papers/archive/`.
- TeX source trees live under `docs/papers/src/` and `docs/papers/speculative/`.
- Figure source assets live under `docs/papers/src/figures/`.
- New active PDFs should not be left behind in `src/` or `speculative/`.

## Directory Structure

```
papers/
├── *.pdf                # All active paper PDFs and exported figure PDFs
├── *.tex, *.md          # Root-level companion files and paper indexes
├── speculative/         # Speculative paper TeX sources; PDFs publish to root
├── src/                 # Main paper TeX sources; PDFs publish to root
│   └── figures/         # Figure sources and source-only support assets
└── archive/             # Historical versions, superseded papers, unused figures
```

## Active PDF Library

This directory is the single active PDF shelf for the project. It includes:

- core/published paper PDFs
- PDFs compiled from `src/`
- PDFs compiled from `speculative/`
- exported figure PDFs that are still active

## Root-Level Companion Files

Root `*.tex` and `*.md` files are companion sources or indexes for papers that are managed directly from `docs/papers/`.

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

### Active Exported Figure PDFs

| File | Used By |
|------|---------|
| FTD_KMS_Thermal_Time.pdf | Active exported figure |
| FTD_Modular_Structure.pdf | Active exported figure |
| FTD_Spatial_Correlations.pdf | Active exported figure |
| FTD_Thermodynamic_Limit.pdf | Active exported figure |

## speculative/

Speculative extensions and applications of FTD to open mathematical problems.

- `speculative/` keeps the TeX source files.
- Their active compiled PDFs now live in `docs/papers/`.

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

Main paper source tree.

- `src/` keeps the TeX source files.
- Their active compiled PDFs now live in `docs/papers/`.
- `src/figures/` is for source figures and figure assets, not for the active PDF shelf.

### TeX Source Papers

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

### PDF Outputs Published To Root

Compiled PDFs from this source tree are published directly into `docs/papers/` so active PDFs stay in one place.

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
| figures/ | Referenced figures and source assets |

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

Compile from the paper's source directory, then move or copy the active PDF into `docs/papers/` if your build tool does not already emit it there.
