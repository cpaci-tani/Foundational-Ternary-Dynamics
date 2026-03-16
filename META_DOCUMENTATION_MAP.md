# FTD Documentation Map

**The Card Catalog** — Find any document in two clicks.

**Last updated:** March 15, 2026
**Framework version:** v5.28-consolidated

---

## Quick Start

| If you want to... | Start here |
|-------------------|------------|
| Understand FTD from scratch | [docs/SPEC_FTD.md](docs/SPEC_FTD.md) — **the single source of truth** |
| See the core mathematics | [docs/theory/01_reference/SPEC_THE_MASTER_CUBIC.md](docs/theory/01_reference/SPEC_THE_MASTER_CUBIC.md) |
| Assess what's genuinely derived | [docs/theory/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) (~30 genuine) |
| Run the verification suite | `python simulations/run_all.py` |
| Read the manuscript (book) | `cd dissemination/manuscript && quarto render` |

---

## Reading Paths

### For Physicists
1. [SPEC_FTD_REFERENCE.md](docs/theory/01_reference/SPEC_FTD_REFERENCE.md) — Technical reference
2. [DERIV_ALPHA_PRECISION_FORMULA.md](docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md) — The 1.26 ppm result
3. [AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) — What's proven vs parametric
4. [AUDIT_HIDDEN_SELECTIONS.md](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) — Explicit selection principles

### For Philosophers
1. [FOUND_THE_FIRST_DISTINCTION.md](docs/theory/02_foundations/FOUND_THE_FIRST_DISTINCTION.md) — From void to existence
2. [FOUND_ONTOLOGICAL_GENESIS.md](docs/theory/02_foundations/FOUND_ONTOLOGICAL_GENESIS.md) — 13-level emergence hierarchy
3. [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](docs/theory/06_consciousness/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) — Consciousness from QFT/GR bridge
4. [FOUND_THE_EXISTENCE_FILTER.md](docs/theory/06_consciousness/FOUND_THE_EXISTENCE_FILTER.md) — E(x) = Re(x) and the Born rule

### For Programmers
1. [docs/internal/SPEC_CLAUDE.md](docs/internal/SPEC_CLAUDE.md) — Architecture and update cycle
2. [docs/internal/META_WALKTHROUGH.md](docs/internal/META_WALKTHROUGH.md) — Navigation guide
3. `engine/` — C++ simulation engine with web UI
4. `engine/tests/` — 155 CTests (variational proof, forces, SM sectors)

### For Experimentalists
1. [ARCH_EMPIRICAL_CERN_CAVITATION.md](docs/theory/archive/ARCH_EMPIRICAL_CERN_CAVITATION.md) — CMS Open Data test of topological cavitation
2. [SPEC_NOVEL_PREDICTIONS.md](docs/theory/01_reference/SPEC_NOVEL_PREDICTIONS.md) — Falsifiable predictions catalog
3. [REF_EXPERIMENTAL_STATUS.md](docs/reference/REF_EXPERIMENTAL_STATUS.md) — Current testing status
4. `simulations/ftd_cern_*.py` — Analysis scripts (Docker + XRootD)

### For Skeptics
1. [AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) — Honest accounting (~30 genuine, ~50 parametric)
2. [AUDIT_HIDDEN_SELECTIONS.md](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) — Every implicit assumption made explicit
3. [AUDIT_BELL_ANALYSIS.md](docs/theory/07_assessment/AUDIT_BELL_ANALYSIS.md) — Bell inequality theory and simulation: why S ≤ 2 is expected, not a failure
4. [evaluation/final_report/CERTIFICATION_REPORT_v1.0.md](evaluation/final_report/CERTIFICATION_REPORT_v1.0.md) — External assessment (Grade B)

---

## Directory Map

```
Foundational-Ternary-Dynamics/
│
├── README.md                    # Public overview + epistemic notice
├── CHANGELOG.md                 # Version history (v5.0 → v5.27-neutrino)
├── CONTRIBUTING.md              # How to contribute
├── META_DOCUMENTATION_MAP.md         # THIS FILE — the card catalog
├── META_PROJECT_ATLAS.md             # AI agent navigation guide
│
├── docs/
│   ├── internal/                # Working documents & editorial guidance
│   │   ├── SPEC_CLAUDE.md       # PRIMARY: Simulation manual (v5.27-bell)
│   │   ├── META_WALKTHROUGH.md  # How to read the project
│   │   ├── META_IMPLEMENTATION_PLAN.md
│   │   ├── META_BULLETPROOFING_STRATEGY.md
│   │   ├── META_GEMINI_CONTEXT_TRANSFER.md
│   │   ├── REF_IMAGE_INVENTORY.md  # 456 images catalog
│   │   ├── REF_PUBLICATION_EDITOR_INSTRUCTIONS.md
│   │   ├── session_summaries/   # Session logs (ARCH_ prefixed)
│   │   └── explorations/        # Research scripts & visualizations
│   │       ├── lemniscate/      # Lemniscate curve explorations (15 files)
│   │       ├── consciousness/   # Consciousness/G* explorations (8 files)
│   │       ├── mandelbrot/      # Mandelbrot-FTD connections (7 files)
│   │       └── number_theory/   # Number theory explorations (7 files)
│   │
│   ├── theory/                  # 85 CORE THEORY DOCUMENTS
│   │   ├── META_INDEX.md         # Complete catalog (9 categories)
│   │   ├── [84 .md files]       # See docs/theory/META_INDEX.md for full listing
│   │   └── archive/             # 48 superseded/historical documents
│   │
│   ├── reference/               # Canonical reference materials
│   │   ├── REF_EPISTEMIC_LABELS.md  # Tag definitions ([AXIOM], [THEOREM], etc.)
│   │   ├── REF_SYMBOL_GLOSSARY.md   # All notation and symbols
│   │   ├── REF_SCOPE_LIMITATIONS.md # What FTD does NOT address
│   │   └── REF_EXPERIMENTAL_STATUS.md # Current testing/validation status
│   │
│   ├── papers/                  # Published/submitted papers (SPEC_, DERIV_, FOUND_, ARCH_ prefixed)
│   │   ├── SPEC_MASTER_QUADRATIC_PAPER.pdf
│   │   ├── DERIV_SELF_ORGANIZED_CRITICALITY.pdf
│   │   ├── DERIV_CASIMIR_RATCHET.pdf
│   │   ├── DERIV_SONOLUMINESCENCE.pdf
│   │   ├── DERIV_GEOMETRIC_BIOPHYSICS.pdf
│   │   ├── DERIV_GRAND_UNIFIED_MASS.pdf
│   │   ├── DERIV_SOFTPLUS_RELU_DUALITY.pdf
│   │   ├── FOUND_ONTIC_CONSTANT_CHAIN.pdf
│   │   ├── ARCH_*.pdf           # 3 archived papers
│   │   ├── SPEC_FTD_FINE_STRUCTURE_CONSTANT*.docx
│   │   └── src/                 # LaTeX sources + figures
│   │       ├── DERIV_*.tex      # 5 derivation paper sources
│   │       ├── FOUND_*.tex      # 1 foundations paper source
│   │       └── figures/         # 11 PNG figures for papers
│   │
│   └── articles/                # Popular writing
│       └── quantum_isnt_weird.md
│
├── evaluation/                  # Multi-domain assessment (~90 files)
│   ├── META_INDEX.md            # Assessment catalog
│   ├── agent_findings/          # 25 AI domain evaluations
│   ├── expert_reviews/          # 24 expert reviews
│   ├── findings/                # 18 detailed domain findings
│   ├── defenses/                # 8 defense documents (AUDIT_ prefixed)
│   ├── synthesis/               # 4 consolidated analyses (AUDIT_/REF_ prefixed)
│   ├── final_report/            # 9 certification + regrading files
│   ├── certification/           # 5 certification reports (AUDIT_/REF_ prefixed)
│   ├── TIER*.md                 # 8 verification tier reports
│   └── archive/                 # 7 superseded intermediate work (ARCH_ prefixed)
│
├── dissemination/               # Publication pipeline
│   ├── manuscript/              # Quarto book (82 chapters, 15 books)
│   │   └── src/                 # .qmd source files
│   ├── whitepaper/              # LaTeX academic paper
│   ├── notebooks/               # 14 Jupyter tutorials (00-13 numbered)
│   ├── interactive/             # Web demos
│   ├── visuals/                 # Publication graphics
│   ├── META_KEYNOTE_PRESENTATION.md  # 45-minute slide deck
│   └── REF_PRESENTER_QUICK_REFERENCE.md
│
├── engine/                      # C++ simulation engine (primary)
│   ├── CMakeLists.txt           # Build system (62 test targets + Qt6 GUI)
│   ├── include/ftd/             # Headers (constants, lattice, render_bridge, lagrangian)
│   ├── src/                     # Core source (lattice, render_bridge, lagrangian)
│   ├── qt_gui/                  # Qt6 native GUI (9 panels, OpenGL viewport)
│   ├── thirdparty/glad/         # OpenGL loader (shared dependency)
│   └── tests/                   # 62 CTests (variational proof, forces, SM sectors)
│
├── simulations/                 # Mathematical verification + empirical analysis
│   ├── run_all.py               # Master test runner
│   ├── constants.py             # Framework integers
│   ├── verify_*.py              # ~55 verification scripts
│   ├── ftd_cern_*.py            # 6 CERN Open Data analysis scripts
│   └── ANALYSIS_CERN_CAVITATION_SUMMARY.md  # CMS MET cavitation results
│
├── tests/                       # Integration test suite
│   └── test_*.py                # 7 test categories
│
├── scripts/                     # Operational scripts
│   ├── investigation/           # Research scripts
│   ├── verification/            # Parameter validation
│   ├── visualization/           # Manim scenes + figure generators (gen_*.py)
│   └── runners/                 # Orchestration
│
├── archive/                     # Archived legacy components
│   ├── engine_imgui_gui/        # Former ImGui GUI (replaced by Qt6)
│   ├── web_frontend/            # Former Next.js + WebSocket bridge
│   ├── visualizer_frontend/     # Former React Three.js visualizer
│   └── python_engine/           # Former Python simulation (ternary_matrix/)
│
├── media/                       # Non-text assets
│   └── images/                  # PNG/SVG figures
│       ├── theory/              # Theory document images (moved from docs/theory/)
│       └── evaluation/          # Evaluation images
│
├── models/                      # Epistemic models
│   └── epistemic/               # Axiomatic definitions
│
└── archive/                     # Historical special projects
    └── special_projects/
        ├── ancient-history/     # Sacred geometry, consciousness
        ├── antigravity/         # Engineering explorations
        └── ftd-fusion/          # Fusion/binding energy
```

---

## Theory Documents at a Glance

The 84 core theory documents in `docs/theory/` are organized into 10 categories.
See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) for the complete catalog with descriptions.

| # | Category | Files | Key Documents |
|---|----------|-------|---------------|
| 1 | Master Reference | 13 | FTD_REFERENCE, FTD_FORMAL, MASTER_QUADRATIC, COMPLETE_PROOF, MASTER_CUBIC, SIX_ALGORITHMS, LAGRANGIAN, SM_REPLACEMENT, COMPARATIVE_PHYSICS |
| 2 | Ontological Foundations | 8 | THE_FIRST_DISTINCTION, COMPLETE_ALGEBRA_OF_i, SPACETIME_EMERGENCE, ONTOLOGICAL_GENESIS, ONTIC_FOUNDATIONS, EULER_IDENTITY, FOURCIER_ONTIC_TOOL |
| 3 | Core Physics | 23 | BOTTOM_UP_PHYSICS, QM_RESOLVED, RELATIVITY, STATE_FLUX_COUPLING, BLACK_HOLE_PHYSICS, EINSTEIN_FIELD_EQUATIONS, FERMI_COUPLING, LATTICE_SCHWARZSCHILD |
| 4 | Coupling Constants | 8 | LEMNISCATE_WHITEPAPER, ALPHA_PRECISION, LAMBDA_QCD, VACUUM_ENERGY, COSMOLOGICAL_CONSTANT, PLANCK_MASS_LAMBDA_QCD, TWO_LOOP_ALPHA |
| 5 | Particle Physics | 5 | COMPLETE_PARTICLE_PHYSICS, PHYSICS_REFERENCE, OCTONIONIC, ELECTROWEAK_MASSES, **NEUTRINO_MASS_ABSOLUTE** |
| 6 | Consciousness | 4 | CONSCIOUSNESS_MATHEMATICS, SLOOP, AGENT_MEANING (incl. VON_NEUMANN), EXISTENCE_FILTER |
| 7 | Critical Self-Assessment | 8 | EPISTEMIC_AUDIT, BELL_ANALYSIS, HIDDEN_SELECTIONS, CLAIMS_MATRIX, PANEL_RESPONSE |
| 8 | Structural Principles | 8 | CUBOCTAHEDRAL, CUBOCTAHEDRAL_INTEGERS, LOOP_GRID_DUALITY, GOLDEN_RATIO, TRIT_INFORMATION, VARIATIONAL_PROOF |
| 9 | Mathematical Connections | 8 | NUMBER_THEORY, RIEMANN_ZETA_CONNECTION, FEIGENBAUM, CURVE_FAMILY, CAYLEY_DICKSON_FOURCIER, FRACTAL_DEPTH_AND_MASS, **MODULAR_QUADRATIC** |
| 10 | Empirical Validation | 1 | **EMPIRICAL_CERN_CAVITATION** — CMS Open Data topological cavitation test (65.6σ excess, β=0.12≠0.5) |

---

## Reference Materials

Located in `docs/reference/`:

| Document | Purpose |
|----------|---------|
| [REF_EPISTEMIC_LABELS.md](docs/reference/REF_EPISTEMIC_LABELS.md) | How to read epistemic tags: [AXIOM], [THEOREM], [CONJECTURE], [IMPOSED], [EMERGENT], [OPEN] |
| [REF_SYMBOL_GLOSSARY.md](docs/reference/REF_SYMBOL_GLOSSARY.md) | All mathematical notation, symbols, and dimensions |
| [REF_SCOPE_LIMITATIONS.md](docs/reference/REF_SCOPE_LIMITATIONS.md) | What FTD does NOT address — required acknowledgments |
| [REF_EXPERIMENTAL_STATUS.md](docs/reference/REF_EXPERIMENTAL_STATUS.md) | Current testing and validation status |

---

## Build & Run Commands

| Task | Command |
|------|---------|
| Run all verifications | `python simulations/run_all.py` |
| Run unit tests | `python tests/run_all_tests.py` |
| Build manuscript (HTML) | `cd dissemination/manuscript/src && quarto render --profile html` |
| Build manuscript (PDF) | `cd dissemination/manuscript/src && quarto render --profile pdf` |
| Build whitepaper | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |
| Start visualizer | Replaced by Qt6 GUI in `engine/qt_gui/` |

---

## Archive Policy

A document is **archived** when:
- A newer document **supersedes** it
- It uses **outdated naming** (TRD → FTD)
- It is a **session log** older than 30 days
- It contains **speculative content** the project has moved beyond

A document is **deleted** only when:
- It is zero-byte or empty
- It is a true exact duplicate
- It is a build artifact

Everything else is preserved in archive/ as institutional memory.

---

*This map is the definitive navigation guide to the FTD project.*
*For theory documents specifically, see [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md).*
*For evaluation details, see [evaluation/META_INDEX.md](evaluation/META_INDEX.md).*
