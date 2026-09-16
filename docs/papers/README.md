# FTD Papers

Published outputs and paper source trees for Foundational Ternary Dynamics.

## Working Rule

- Active PDFs live directly in `docs/papers/`.
- Archived PDFs live under `docs/papers/archive/`.
- TeX source trees live under `docs/papers/src/`; the retained finitude source remains in `speculative/`.
- Figure source assets live under `docs/papers/src/figures/`.
- New active PDFs should not be left behind in `src/` or `speculative/`.

## Directory Structure

```
papers/
├── *.pdf                # All active paper PDFs and exported figure PDFs
├── *.tex, *.md          # Root-level companion files and paper indexes
├── speculative/         # Retained canonical finitude source
├── src/                 # Main paper TeX sources; PDFs publish to root
│   └── figures/         # Figure sources and source-only support assets
└── archive/             # Historical versions, superseded papers, unused figures
```

## Active PDF Library

This directory is the single active PDF shelf for the project. It includes:

- core/published paper PDFs
- PDFs compiled from `src/`
- the retained finitude paper when compiled
- exported figure PDFs that are still active

## Root-Level Companion Files

Root `*.tex` and `*.md` files are companion sources or indexes for papers that are managed directly from `docs/papers/`.

### With TeX Sources

| Paper | Description |
|-------|-------------|
| DERIV_CLOSURE_RENORMALIZATION | Closure under renormalization |
| PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY | Gauge coupling constants from lattice coordination geometry (.md + .tex + .pdf) |
| PAPER_LIFECYCLE_SOFTPLUS | Lifecycle of the softplus function |

### PDF-Only Papers (all archived or retracted)

The following PDFs lacked recoverable TeX source (verified via `git log --all --diff-filter=AD` archaeology — only figure files were ever committed):

**Archived to `archive/pdf_only_no_source/`** (status pending owner re-authoring or accept-as-historical; `pdftotext` extractions provided):

- `DERIV_ALPHA_INVERSE_LATTICE_GAUGE`
- `DERIV_EMERGENT_GRAVITY`
- `DERIV_FUNDAMENTAL_CONSTANTS`
- `DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME` (likely-superseded by `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex`)
- `DERIV_QUANTUM_INFERENCE`
- `DERIV_SELF_REFERENCE_FOUR_INTEGERS`
- `FTD_KMS_Thermal_Time` (Type-III₁/KMS — would need scaffold-framing if re-authored)
- `FTD_Modular_Structure` (same as above)
- `FTD_Spatial_Correlations`
- `SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME` (likely-superseded)
- `SPEC_MASTER_QUADRATIC_PAPER` (likely-superseded)

See `docs/papers/archive/pdf_only_no_source/README.md` for per-paper triage details.

**Retracted to `archive/retracted_under_reframe/`** (reframe-incompatible; pdftotext extractions provided):

- `FTD_Thermodynamic_Limit` — title is the proscribed concept
- `DERIV_THERMODYNAMIC_REFLEXION` — title-level evidence

See `docs/papers/archive/retracted_under_reframe/RETRACTION_NOTES.md`.

## Retained finitude source and external research

`speculative/FTD_Finitude_Theorem.tex` remains because its existing preamble identifies it as canonical framework material.

The RH paper, Casimir ratchet, geometric biophysics, grand unified mass,
sonoluminescence, and Hermitian letter sources were moved outside FTD at the
owner's request on 2026-09-15. The already-retracted Navier–Stokes and
Yang–Mills paper sources/PDFs were moved with them. Their prior scientific
statuses are unchanged; they are not active publications in this tree.

See the [external archive receipt](../audits/RESEARCH_EXTERNALIZATION_2026-09-15.md) for original paths,
SHA-256 digests and recovery instructions. Retraction notes remain in
`archive/retracted_under_reframe/RETRACTION_NOTES.md`. The per-voxel mass-gap
proof remains in `scripts/proofs/proof_per_voxel_mass_gap.py`.

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
| ARCH_REFLEXIVE_DYNAMICS.pdf | Frame-relative dynamics |
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
