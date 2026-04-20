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
*   **engine/** - C++ simulation engine with Three.js web dashboard. See [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) for current version details.
    *   `include/ftd/` - 28 headers (ontic.h is the constant derivation chain).
    *   `src/` - 7 core source files.
    *   `tests/` - Large native test surface (CPU, campaign, optional GPU).
    *   `cuda/` - GPU kernels and acceleration path.
    *   `wasm/` - Emscripten WASM bindings.
    *   `web/` - Three.js browser dashboard and WASM integration.

### Domain Models
*   **models/** - Logical and physical interpretations of the ternary dynamics.
    *   `epistemic/` - Axiomatic definitions (Planck scale, constants, master quadratic).
    *   Core modules: `ftd_core.py`, `particle_physics.py`, `cosmology.py`, `mixing_matrices.py`, etc.

### Verification & Execution
*   **scripts/** - Python verification, proofs, experiments, and test runners.
    *   `constants.py` - Canonical shared constants (single source of truth).
    *   `verification/` - Formal derivation verification.
    *   `proofs/` - Mathematical proofs with error bounds.
    *   `experiments/` - Bell tests, CERN analysis, photon simulations.
    *   `exploration/` - Focused research investigations.
    *   `tests/` - pytest suites + `comprehensive/` 7-tier verification framework.
    *   `visualization/` - Manim scenes and figure generators.
    *   `runners/` - Test protocol orchestration.

### Documentation
*   **docs/** - All project documentation.
    *   `SPEC_FTD.md` - **Single source of truth** for the FTD specification.
    *   `theory/` - Curated theory catalog plus archive. See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) and [AUDIT_DOCUMENT_CLEANUP_LEDGER.md](AUDIT_DOCUMENT_CLEANUP_LEDGER.md).
    *   `reference/` - 5 canonical reference materials (epistemic labels, symbol glossary, scope/limitations, naming conventions).
    *   `papers/` - Published paper surface: active PDFs at the root, source trees in `speculative/` and `src/`, archive in `archive/`.
    *   `articles/` - Popular writing.
    *   `internal/` - Local working documents and editorial guidance (gitignored, not public-first).

### Dissemination
*   **dissemination/** - Publication-ready outputs.
    *   `manuscript/` - Quarto-based manuscript.
    *   `book/` - Narrative companion book.
    *   `whitepaper/` - Academic whitepaper with LaTeX source.
    *   `notebooks/` - Jupyter pedagogy tutorials.
    *   `interactive/` - Standalone HTML simulations and explainers.

### Assessment & Evaluation
*   **evaluation/** - Multi-domain assessment framework.
    *   `agent_findings/` (6) - AI domain evaluations (math, physics subdomains).
    *   `expert_reviews/` (6) - Expert reviews + physicist final report.
    *   `findings/` (4) - Cross-cutting domain findings.
    *   Root: `ISSUE_TRACKER.md` (116 issues), `AUDIT_WEAKNESSES_MASTER.md`, `AUDIT_UNRESOLVED_ISSUES.md`.

### Archive & Historical
*   **archive/** - Curated historical record (gitignored). Bulk legacy material (TRD-era engines, pre-restructure root, superseded web platform, etc.) deleted 2026-04-19. Archived theory docs live at `docs/theory/archive/`.

---

## Technical Guides for AI Agents
*   **Project Instructions**: See [CLAUDE.md](CLAUDE.md) — mandatory rules for AI work on this project.
*   **Contributor Onboarding**: See [META_CONTRIBUTOR_ONBOARDING.md](META_CONTRIBUTOR_ONBOARDING.md) — balanced public guide across theory, engine, verification, and critique.
*   **Documentation Cleanup Ledger**: See [AUDIT_DOCUMENT_CLEANUP_LEDGER.md](AUDIT_DOCUMENT_CLEANUP_LEDGER.md) — repo-wide drift findings, status model, and remediation queue.
*   **Project Health Scorecard**: See [evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md](evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md) and [evaluation/REF_PROJECT_HEALTH_SCORING.md](evaluation/REF_PROJECT_HEALTH_SCORING.md).
*   **Documentation Map**: See [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) — the definitive navigation guide.
*   **Theory Index**: See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) for theory document navigation.
*   **Theory Structure Guide**: See [docs/theory/META_STRUCTURE.md](docs/theory/META_STRUCTURE.md) for category boundaries, placement rules, and archive guidance.
*   **Engine Spec**: See [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) for engine architecture.
*   **History**: See [CHANGELOG.md](CHANGELOG.md).

---

## Quick Reference

| Task | Command/Location |
|------|------------------|
| Run all Python tests | `python scripts/tests/run_all_tests.py` |
| Run 7-tier verification | `python scripts/tests/comprehensive/run_ultimate_test.py` |
| Run proof chain | `python scripts/proofs/proof_10_ultimate_chain.py` |
| Contributor onboarding | `META_CONTRIBUTOR_ONBOARDING.md` |
| Documentation cleanup ledger | `AUDIT_DOCUMENT_CLEANUP_LEDGER.md` |
| Project health scorecard | `evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md` |
| Build C++ engine | `cmake -S engine -B engine/build && cmake --build engine/build --config Release` |
| Run C++ tests | `cd engine/build && ctest --output-on-failure -C Release` |
| Build whitepaper PDF | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |
| Build manuscript | `cd dissemination/manuscript && quarto render` |
| Launch web dashboard | `python -m http.server 8080 -d engine/web` |
| View theory docs | `docs/theory/META_INDEX.md` |
| View all documentation | `META_DOCUMENTATION_MAP.md` |

---

*Last updated: April 11, 2026*
*Project version: FTD v5.29*
*Engine version: v2.13*
