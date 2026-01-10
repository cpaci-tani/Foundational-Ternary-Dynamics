# FTD Paper: Derivation of the Fine Structure Constant

**Epistemic note:** This README is a convenience summary. The authoritative statements, assumptions, and caveats are those in the paper source (`trd_fine_structure.tex`).

This directory contains the arXiv submission materials for the paper: **Derivation of the Fine Structure Constant from Discrete Lattice Dynamics**

## Files

- `trd_fine_structure.tex` - Main paper (LaTeX)
- `appendix_proofs.tex` - Detailed proofs appendix
- `README.md` - This file

## Compilation

```bash
pdflatex trd_fine_structure.tex
pdflatex trd_fine_structure.tex  # Run twice for TOC
```

If `pdflatex` is not on your PATH (common on Windows), invoke it via the full MiKTeX path (e.g., `...\MiKTeX\miktex\bin\x64\pdflatex.exe`) or add the MiKTeX `bin` directory to PATH.

Or use your preferred LaTeX distribution.

## arXiv Submission

**Suggested categories:**

- Primary: `hep-th` (High Energy Physics - Theory)
- Cross-list: `gr-qc` (General Relativity and Quantum Cosmology)
- Cross-list: `math-ph` (Mathematical Physics)

**Suggested title:**
"Derivation of the Fine Structure Constant from Discrete Lattice Dynamics"

**Suggested abstract:**
See the paper abstract in `trd_fine_structure.tex`.

## Supplementary Materials

The computational verification scripts are in `../simulations/`:

| Script | Verifies |
| ------ | -------- |
| `cm_selection_proof.py` | CM curve selection by cubic symmetry |
| `elliptic_fibration_proof.py` | Moduli space fibration structure |
| `tau_equals_i_proof.py` | τ = i at critical point |
| `sw_curve_from_trd.py` | Seiberg-Witten curve emergence |
| `critical_coupling_selection.py` | λ = 1 from Gauss constraint |
| `coefficient_16_from_lattice.py` | 16 DoF on minimal lattice |
| `agm_from_laplacian.py` | Γ(1/4)² from lattice regularization |
| `g_star_from_trd.py` | Full G* derivation chain |

## Key Results

```text
G* = √2 × Γ(1/4)² / (2π) = 2.9586751192

Master quadratic: x² - 16(G*)²x + 16(G*)³ = 0

Roots:
  x₊ = 137.036171  →  1/α (relative agreement: 1.26 ppm)
  x₋ = 3.023964    →  Nc (suggests a small deviation from 3 within the stated framework)
```

## Contact

[Author information to be added]

## License

[To be determined - likely CC-BY or similar for academic work]
