# Project Atlas: Foundational Ternary Dynamics (FTD)

This document serves as a high-level map for AI agents and researchers to navigate the FTD repository. For the complete documentation catalog, see [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md).

## Architecture Overview

FTD is organized into four primary layers:
1.  **Core Engine**: The C++ simulation engine (`engine/`) with Three.js web dashboard, CUDA GPU support, and WASM bindings.
2.  **Epistemic Layer**: Physical models and axiomatization (`models/`, `docs/theory/`).
3.  **Verification & Execution**: Scripts for running experiments and validating results (`scripts/`).
4.  **Dissemination**: Documentation, manuscripts, whitepapers, and interactive content (`docs/`, `dissemination/`).

---

## Directory Index

### Core & Logic
*   **engine/** - C++ simulation engine (v2.11) with Three.js web dashboard.
    *   `include/ftd/` - 28 headers (ontic.h is the constant derivation chain).
    *   `src/` - 7 core source files.
    *   `tests/` - 168 test files (119 unit tests + 49 physics campaigns).
    *   `cuda/` - 5 GPU kernels ( speedup on GPU).
    *   `wasm/` - Emscripten WASM bindings.
    *   `web/` - Three.js browser dashboard (28 JS modules, 211 scenarios across 4 scales).

### Domain Models
*   **models/** - Logical and physical interpretations of the ternary dynamics.
    *   `epistemic/` - Axiomatic definitions (Planck scale, constants, master quadratic).
    *   Core modules: `ftd_core.py`, `particle_physics.py`, `cosmology.py`, `mixing_matrices.py`, etc.

### Verification & Execution
*   **scripts/** - All Python scripts (~149 total).
    *   `constants.py` - Canonical shared constants (single source of truth).
    *   `verification/` (40) - Formal derivation verification.
    *   `proofs/` (57) - Mathematical proofs with error bounds.
    *   `experiments/` (17) - Bell tests, CERN analysis, photon simulations.
    *   `exploration/` (9) - Focused research investigations.
    *   `tests/` (11+) - pytest suites + `comprehensive/` 7-tier verification framework.
    *   `visualization/` (11) - Manim scenes and figure generators.
    *   `runners/` (2) - Test protocol orchestration.

### Documentation
*   **docs/** - All project documentation.
    *   `SPEC_FTD.md` - **Single source of truth** for the FTD specification.
    *   `theory/` - 114 active + 67 archived theory documents in 9 categories. See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md).
    *   `reference/` - 5 canonical reference materials (epistemic labels, symbol glossary, scope/limitations, naming conventions).
    *   `papers/` - Published papers organized into main (core), `speculative/`, `src/` (TeX sources), and `archive/`.
    *   `articles/` - Popular writing.
    *   `internal/` - Working documents, editorial guidance, explorations (gitignored).

### Dissemination
*   **dissemination/** - Publication-ready outputs.
    *   `manuscript/` - Quarto-based book (96 .qmd chapters).
    *   `book/` - "The Golden Thread" narrative (53 .qmd files).
    *   `whitepaper/` - Academic whitepaper with LaTeX source.
    *   `notebooks/` (12) - Jupyter pedagogy tutorials.
    *   `interactive/` (6) - Standalone HTML force/photon simulations.

### Assessment & Evaluation
*   **evaluation/** - Multi-domain assessment framework.
    *   `agent_findings/` (6) - AI domain evaluations (math, physics subdomains).
    *   `expert_reviews/` (6) - Expert reviews + physicist final report.
    *   `findings/` (4) - Cross-cutting domain findings.
    *   Root: `ISSUE_TRACKER.md` (116 issues), `AUDIT_WEAKNESSES_MASTER.md`, `AUDIT_UNRESOLVED_ISSUES.md`.

### Archive & Historical
*   **archive/** - Deprecated and historical content (gitignored).
    *   `ftd_archive/` - Legacy engines (Python simulation, Qt GUI, ImGui GUI).
    *   `pre_ftd_root/` - Original root files before March 2026 restructure.
    *   `trd_working_docs/` - Early TRD-era working documents.
    *   `legacy_scripts/` - Superseded Python scripts.
    *   `web-app/` - Superseded web platform.

---

## Technical Guides for AI Agents
*   **Project Instructions**: See [CLAUDE.md](CLAUDE.md) — mandatory rules for AI work on this project.
*   **Documentation Map**: See [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) — the definitive navigation guide.
*   **Theory Index**: See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) for theory document navigation.
*   **Engine Spec**: See [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) for engine architecture.
*   **History**: See [CHANGELOG.md](CHANGELOG.md).

---

## Quick Reference

| Task | Command/Location |
|------|------------------|
| Run all Python tests | `python scripts/tests/run_all_tests.py` |
| Run 7-tier verification | `python scripts/tests/comprehensive/run_ultimate_test.py` |
| Run proof chain | `python scripts/proofs/proof_10_ultimate_chain.py` |
| Build C++ engine | `cmake -S engine -B engine/build && cmake --build engine/build --config Release` |
| Run C++ tests | `cd engine/build && ctest --output-on-failure -C Release` |
| Build whitepaper PDF | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |
| Build manuscript | `cd dissemination/manuscript && quarto render` |
| Launch web dashboard | `python -m http.server 8080 -d engine/web` |
| View theory docs | `docs/theory/META_INDEX.md` |
| View all documentation | `META_DOCUMENTATION_MAP.md` |

---

*Last updated: March 27, 2026*
*Project version: FTD v5.28-consolidated*
*Engine version: v2.11*
