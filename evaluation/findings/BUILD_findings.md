# BUILD Evaluation Report

## Agent Profile
- **Domain**: Build Systems and Reproducibility
- **Credentials**: Expert in Reproducible Research, Build Systems, Computational Reproducibility
- **Scope**: Build pipeline, figure generation, environment setup, and output validation

## Executive Summary

The FTD manuscript build system demonstrates **strong foundations** with a well-structured Quarto project, comprehensive requirements files with version pinning, functioning CI/CD pipeline, and a modular figure generation system. However, several gaps compromise full reproducibility: the figure scripts reference utility modules via relative imports that assume a specific directory structure, there is no single-command build script for generating all figures, and build documentation is scattered rather than centralized. The HTML and PDF outputs exist and appear functional, indicating the build process works but may require implicit knowledge to execute correctly.

**Overall Grade: B (77/100)**

The system achieves good reproducibility for experienced users but falls short of turnkey reproducibility for newcomers.

---

## Strengths (S1-S5)

### S1: Well-Structured Quarto Configuration
The `manuscript/src/_quarto.yml` is comprehensive and well-organized:
- Clear project type specification (`type: book`)
- Extensive chapter organization (82 chapters across 15 books)
- Proper cross-reference configuration
- Freeze enabled for computational content (`execute: freeze: true`)
- Bibliography integration

### S2: Version-Pinned Dependencies
Two requirements files provide clear dependency management:

**requirements.txt** (runtime):
```
numpy>=1.24.0,<2.0.0
scipy>=1.11.0,<2.0.0
matplotlib>=3.7.0,<4.0.0
sympy>=1.12,<2.0
mpmath>=1.3.0,<2.0.0
```

**requirements-dev.txt** (development):
- Includes runtime requirements
- Adds pytest, pytest-cov, ruff, black
- Clear separation of concerns

Version bounds prevent silent breakage from upstream changes.

### S3: Comprehensive CI/CD Pipeline
The `.github/workflows/ci.yml` includes:
- Multi-Python version testing (3.10, 3.11, 3.12)
- Linting with ruff and black
- Coverage reporting to Codecov
- Manuscript build verification with Quarto
- Physics validation tests
- Artifact upload for built outputs

This ensures changes do not break the build or physics calculations.

### S4: Modular Figure Generation System
The figure scripts in `manuscript/media/images/` are:
- Well-documented with docstrings explaining what each figure shows
- Organized with consistent naming conventions (`fig_XX_description.py`)
- Use shared utility modules for styling and physics constants
- Each script has a standalone execution block

The `__init__.py` properly exports all generation functions for programmatic access.

### S5: Colorblind-Safe Styling Infrastructure
The `media/utils/style.py` implements:
- Okabe-Ito colorblind-safe palette (WCAG 2.1 compliant)
- Consistent typography and font sizing
- Reusable styling functions (`apply_trd_style`, `create_figure`)
- Comprehensive color definitions for all figure types

This is professional-grade visualization infrastructure.

---

## Weaknesses (W1-W6)

### W1: No Centralized Build Documentation
The README provides quick-start commands but lacks:
- Step-by-step setup guide for different operating systems
- Troubleshooting section for common build errors
- Explanation of the build profiles mentioned in `_quarto.yml`
- Documentation of the figure generation workflow

Users must infer the build process from multiple files.

### W2: Figure Scripts Have Broken Import Paths
Figure scripts reference utilities via relative imports that require the parent directory:
```python
sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.style import COLORS, MODE_COLORS, apply_trd_style
```

However, the utils module path `manuscript/media/images/utils/` does not exist. The actual utilities are at `media/utils/`. This inconsistency would cause import failures when running scripts directly.

### W3: No Single-Command Figure Generation
There is no `Makefile`, `build.py`, or equivalent that:
- Regenerates all figures in sequence
- Validates figure output exists
- Checks figure dimensions/resolution
- Reports generation status

Users must run each of 80+ scripts individually or write their own orchestration.

### W4: Missing Environment Specification Beyond Python
The build depends on:
- **Quarto** (mentioned in README as "optional", version not specified)
- **TeX Live 2024+** (mentioned, not enforced)
- System fonts (for matplotlib)

No `environment.yml` (conda), Docker container, or similar exists for full environment reproducibility.

### W5: Inconsistent Output Directories
The build creates multiple output directories:
- `_webbook/` - HTML output (exists, populated)
- `_book/` - PDF output (contains only clipboard.min.js and the PDF)
- `_site/` - referenced in CI but not found

The CI workflow uploads `manuscript/_site/` which differs from the actual output paths.

### W6: No Build Verification Suite
No automated checks exist to verify:
- All expected figures were generated
- PDF page count matches chapter count
- Cross-references resolve correctly
- All images are embedded in outputs

---

## Detailed Analysis

### Build Documentation

| Aspect | Status | Location |
|--------|--------|----------|
| Quick start commands | Present | README.md |
| Full setup instructions | Missing | - |
| OS-specific guidance | Missing | - |
| Troubleshooting guide | Missing | - |
| Profile documentation | Missing | _quarto.yml comments only |
| Figure regeneration guide | Missing | - |

**Assessment**: Basic commands documented; detailed workflow undocumented.

### Dependency Management

| File | Purpose | Quality |
|------|---------|---------|
| requirements.txt | Runtime deps | Good (versioned, minimal) |
| requirements-dev.txt | Dev deps | Good (includes -r reference) |
| pyproject.toml | Project config | Missing |
| environment.yml | Full env spec | Missing |
| Dockerfile | Containerization | Missing |

**Assessment**: Python dependencies well-managed; full environment not captured.

### Figure Reproducibility

| Aspect | Status | Notes |
|--------|--------|-------|
| Figure scripts exist | Yes | 80+ scripts in images/ |
| Scripts documented | Yes | Docstrings present |
| Utility modules exist | Yes | style.py, physics_constants.py |
| Import paths correct | No | Scripts reference wrong utils path |
| Batch generation script | No | Must run individually |
| Output validation | No | No checks for success |

**Assessment**: Strong foundation with broken import paths preventing easy execution.

### Environment Setup

| Component | Documented | Version Specified |
|-----------|------------|-------------------|
| Python | Yes | 3.10+ |
| numpy/scipy/etc | Yes | Range-pinned |
| Quarto | Partial | "1.4" mentioned |
| TeX Live | Partial | "2024+" mentioned |
| System fonts | No | - |

**Assessment**: Core Python environment reproducible; external tools under-specified.

### Output Validation

| Check | Implemented | Location |
|-------|-------------|----------|
| Tests pass | Yes | CI pipeline |
| Physics validation | Yes | CI inline Python |
| Manuscript renders | Yes | CI (partial) |
| Figures generated | No | - |
| PDF complete | No | - |
| Links work | No | - |

**Assessment**: Basic validation present; comprehensive checks missing.

---

## Build Test Results

### Attempted Build Trace

Based on examination of outputs and configuration:

1. **HTML Build** (`_webbook/`):
   - Status: SUCCESS
   - Evidence: 100+ HTML files present, search.json populated
   - All major chapters rendered

2. **PDF Build** (`_book/`):
   - Status: SUCCESS
   - Evidence: `Foundational-Ternary-Dynamics.pdf` exists
   - Size: Present and non-trivial

3. **Figure Generation**:
   - Status: UNCERTAIN
   - Evidence: 100+ PNG files exist in `media/images/`
   - Concern: Import path issues would prevent regeneration

4. **Executable Code Cells**:
   - Status: FROZEN
   - Evidence: `.quarto/_freeze/` contains cached execution results
   - Python code blocks in chapters would execute on fresh build

### Reproducibility Assessment

| Scenario | Success Probability | Reason |
|----------|---------------------|--------|
| Clone and read PDF | 100% | PDF committed to repo |
| Clone and build HTML | 80% | Requires Quarto installation |
| Clone and build PDF | 60% | Requires Quarto + TeX Live |
| Clone and regenerate figures | 30% | Import paths broken |
| Full clean rebuild | 40% | Missing orchestration |

---

## Scores

| Criterion | Score | Justification |
|-----------|-------|---------------|
| **Clarity** | 65/100 | Quick-start present; detailed documentation missing |
| **Accessibility** | 70/100 | Standard Python tools; external deps under-specified |
| **Usability** | 75/100 | Builds work but require implicit knowledge |
| **Consistency** | 80/100 | Output directories somewhat scattered |
| **Reproducibility** | 75/100 | Python deps good; full env not captured |
| **Modernity** | 95/100 | Quarto, GitHub Actions, modern tooling throughout |

**Overall: 77/100 (B)**

---

## Overall Grade: B

The FTD manuscript has a professionally designed build system that successfully produces outputs. The Quarto configuration is comprehensive, dependencies are properly pinned, and CI/CD provides continuous validation. However, reproducibility is compromised by broken figure script imports, missing environment specifications for non-Python tools, and absence of build orchestration documentation. An experienced developer can navigate these issues, but turnkey reproducibility for arbitrary users is not achieved.

---

## Key Recommendations

### Critical (Address Before Distribution)

1. **Fix Figure Import Paths**: The utils module path mismatch will cause `ModuleNotFoundError` when regenerating figures. Either:
   - Create `manuscript/media/images/utils/` with the required modules, or
   - Update all figure scripts to use the correct path

2. **Add Build Orchestration Script**: Create `build.py` or `Makefile` that:
   ```makefile
   all: figures html pdf

   figures:
       python scripts/generate_all_figures.py

   html:
       cd manuscript/src && quarto render --to html

   pdf:
       cd manuscript/src && quarto render --to pdf
   ```

### High Priority

3. **Centralize Build Documentation**: Add `BUILD.md` or expand README with:
   - Prerequisites (with versions)
   - Installation steps (by OS)
   - Build commands explained
   - Output directory guide
   - Troubleshooting common issues

4. **Add Environment Specification**: Create either:
   - `environment.yml` for conda users
   - `Dockerfile` for containerized builds
   - `devcontainer.json` for VS Code users

### Medium Priority

5. **Align CI Output Paths**: Fix the CI workflow to upload from actual output directories (`_webbook/`, `_book/`) rather than non-existent `_site/`.

6. **Add Build Validation Tests**: Create `tests/test_build.py`:
   ```python
   def test_all_figures_exist():
       expected = ['fig_01_lemniscate_alpha.png', ...]
       for fig in expected:
           assert Path(f'manuscript/media/images/{fig}').exists()
   ```

### Nice to Have

7. **Add pyproject.toml**: Modern Python packaging standard that consolidates:
   - Dependencies
   - Tool configurations (black, ruff, pytest)
   - Project metadata

8. **Pre-commit Hooks**: Add `.pre-commit-config.yaml` to enforce:
   - Code formatting
   - Import sorting
   - File size limits
