# Project Atlas: Foundational Ternary Dynamics (FTD)

This document serves as a high-level map for AI agents and researchers to navigate the FTD repository. For the complete documentation catalog, see [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md).

## Architecture Overview

FTD is organized into four primary layers:
1.  **Core Engine**: The C++ simulation engine (`engine/`) with Qt6 native GUI.
2.  **Epistemic Layer**: Physical models and axiomatization (`models/`, `docs/theory/`).
3.  **Verification & Execution**: Scripts for running experiments and validating results (`simulations/`, `tests/`, `scripts/`).
4.  **Dissemination**: Documentation, manuscripts, whitepapers, and media assets (`docs/`, `dissemination/`, `media/`).

---

## Directory Index

### Core & Logic
*   **engine/** - C++ simulation engine with Qt6 native GUI.
    *   `include/ftd/` - Core headers (constants, lattice, render_bridge, lagrangian).
    *   `src/` - Core source (lattice, render_bridge, lagrangian).
    *   `qt_gui/` - Qt6 native GUI (9 panels, OpenGL viewport).
    *   `tests/` - 61 CTests (variational proof, forces, SM sectors).

### Domain Models
*   **models/** - Logical and physical interpretations of the ternary dynamics.
    *   `epistemic/` - Axiomatic definitions (Planck scale, constants, master quadratic).

### Execution & Verification
*   **simulations/** - Primary verification suite for SM parameters (masses, mixing, etc.).
*   **scripts/** - Operational scripts for running the environment.
    *   `runners/` - Production experiment orchestration.
    *   `investigation/` - One-off research and exploratory scripts.
    *   `visualization/` - Scripts for generating Manim scenes and interactive dashboards.
*   **tests/** - Unit and integration tests for code stability.

### Documentation
*   **docs/** - All project documentation.
    *   `internal/` - Primary simulation manual ([SPEC_CLAUDE.md](docs/internal/SPEC_CLAUDE.md)), walkthroughs, editorial guidance.
    *   `theory/` - 83 core theory documents organized in 9 categories. See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md).
    *   `reference/` - Canonical reference materials (epistemic labels, symbol glossary, scope/limitations).
    *   `papers/` - Published papers (PDFs) and submissions. `src/` contains LaTeX sources; `src/figures/` contains PNG figures.
    *   `articles/` - Popular writing.

### Dissemination & Media
*   **dissemination/** - Publication-ready outputs.
    *   `manuscript/` - Quarto-based book (82 chapters across 15 books).
    *   `whitepaper/` - Academic whitepaper with LaTeX source.
    *   `notebooks/` - Jupyter notebooks for tutorials.
    *   `interactive/` - Web-based demos and tools.
    *   `visuals/` - Visual galleries for presentations.
*   **media/** - Centralized repository for all non-text assets.
    *   `images/` - PNG/SVG figures.
    *   `videos/` - MP4 renders of simulations.
    *   `animations/` - GIF animations.

### Assessment & Evaluation
*   **evaluation/** - Multi-domain assessment framework (~90 files). See [evaluation/META_INDEX.md](evaluation/META_INDEX.md).
    *   `agent_findings/` - 25 AI domain evaluations.
    *   `expert_reviews/` - 24 expert reviews.
    *   `final_report/` - Certification reports (Grade B, 6.56/10).

### Archive & Historical
*   **archive/** - Deprecated and historical content.
    *   `engine_imgui_gui/` - Former ImGui GUI (replaced by Qt6).
    *   `python_engine/` - Former Python simulation engine (`ternary_matrix/`).
    *   `web_frontend/` - Former Next.js + WebSocket bridge.
    *   `visualizer_frontend/` - Former React Three.js visualizer.
    *   `special_projects/` - Experimental side projects (antigravity, fusion, ancient-history).

---

## Technical Guides for AI Agents
*   **Documentation Map**: See [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) — the definitive navigation guide.
*   **Simulation Manual**: See [docs/internal/SPEC_CLAUDE.md](docs/internal/SPEC_CLAUDE.md) — the primary specification document.
*   **Theory Index**: See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) for theory document navigation.
*   **Verification Protocol**: Run `python simulations/run_all.py` to validate all theoretical claims.
*   **History**: See [CHANGELOG.md](CHANGELOG.md).

---

## Quick Reference

| Task | Command/Location |
|------|------------------|
| Run all verifications | `python simulations/run_all.py` |
| Run unit tests | `python tests/run_all_tests.py` |
| Build whitepaper PDF | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |
| Build manuscript | `cd dissemination/manuscript/src && quarto render` |
| View theory docs | `docs/theory/META_INDEX.md` |
| View all documentation | `META_DOCUMENTATION_MAP.md` |
| View evaluation | `evaluation/META_INDEX.md` |
| View figures | `media/images/` |
| Build C++ engine | `cd engine && cmake -B build && cmake --build build` |

---

*Last updated: February 26, 2026*
*Project version: FTD v5.27-bell*
