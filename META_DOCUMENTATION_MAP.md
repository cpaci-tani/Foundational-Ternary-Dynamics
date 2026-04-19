# FTD Documentation Map

**The Card Catalog** — Find any document in two clicks.

**Last updated:** April 13, 2026
**Framework version:** v5.30
**Engine version:** v2.14

For current documentation drift, cleanup status, and deferred remediation items, see [AUDIT_DOCUMENT_CLEANUP_LEDGER.md](AUDIT_DOCUMENT_CLEANUP_LEDGER.md).

---

## Quick Start

| If you want to... | Start here |
|-------------------|------------|
| Understand FTD from scratch | [docs/SPEC_FTD.md](docs/SPEC_FTD.md) — **the single source of truth** |
| Get contributor onboarding | [META_CONTRIBUTOR_ONBOARDING.md](META_CONTRIBUTOR_ONBOARDING.md) — balanced guide across theory, engine, verification, and critique |
| Audit documentation drift | [AUDIT_DOCUMENT_CLEANUP_LEDGER.md](AUDIT_DOCUMENT_CLEANUP_LEDGER.md) — cleanup ledger, status model, and remediation queue |
| Check project health | [evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md](evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md) — weighted project health scorecard and priorities |
| See the core mathematics | [docs/theory/01_reference/MATH_MASTER_QUADRATIC.md](docs/theory/01_reference/MATH_MASTER_QUADRATIC.md) |
| Assess what's genuinely derived | [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) (~35 genuine) |
| Find an unresolved item to work on | [docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md](docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md) — every `[OPEN]` across code + theory in one ledger |
| Run the verification suite | `python scripts/tests/run_all_tests.py` |
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
1. [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) — Engine architecture and API
2. [META_CONTRIBUTOR_ONBOARDING.md](META_CONTRIBUTOR_ONBOARDING.md) — Public contributor guide
3. `engine/` — C++ simulation engine with Three.js web dashboard
4. `engine/tests/` — Large native test surface; see `engine/SPEC_ENGINE.md` for the current breakdown

### For Experimentalists
1. [SPEC_NOVEL_PREDICTIONS.md](docs/theory/01_reference/SPEC_NOVEL_PREDICTIONS.md) — Falsifiable predictions catalog
2. [REF_EXPERIMENTAL_STATUS.md](docs/reference/REF_EXPERIMENTAL_STATUS.md) — Current testing status
3. `scripts/experiments/` — Bell tests, CERN analysis, physics simulations

### For Skeptics
1. [AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) — Honest accounting (~30 genuine, ~50 parametric)
2. [AUDIT_HIDDEN_SELECTIONS.md](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) — Every implicit assumption made explicit
3. [AUDIT_BELL_ANALYSIS.md](docs/theory/07_assessment/AUDIT_BELL_ANALYSIS.md) — Bell inequality theory and simulation
4. [AUDIT_WEAKNESSES_MASTER.md](evaluation/AUDIT_WEAKNESSES_MASTER.md) — Master weakness compilation

---

## Directory Map

```
ftd/
├── README.md                        # Public overview + epistemic notice
├── CHANGELOG.md                     # Version history
├── CONTRIBUTING.md                  # How to contribute
├── CLAUDE.md                        # AI agent project instructions
├── AUDIT_DOCUMENT_CLEANUP_LEDGER.md # Repo-wide documentation cleanup ledger
├── META_DOCUMENTATION_MAP.md        # THIS FILE — the card catalog
├── META_PROJECT_ATLAS.md            # AI agent navigation guide
│
├── docs/
│   ├── SPEC_FTD.md                  # THE authoritative FTD specification
│   ├── theory/                      # Curated theory catalog plus archive
│   │   ├── META_INDEX.md            # Curated catalog; raw directory counts may be higher during cleanup
│   │   ├── 01_reference/            # Master references and proofs
│   │   ├── 02_foundations/          # Ontological emergence
│   │   ├── 03_derivations/          # Core physics derivations
│   │   ├── 04_coupling/             # Coupling constants
│   │   ├── 05_particles/            # Particle physics
│   │   ├── 06_consciousness/        # Consciousness and measurement
│   │   ├── 07_assessment/           # Epistemic audits
│   │   ├── 08_structural/           # Geometry and information theory
│   │   │   └── DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md  # BCC multiplicative structure: Watson identity + SU(3) (April 2026)
│   │   ├── 09_mathematical/         # Number theory and connections
│   │   └── archive/                 # Superseded/historical documents
│   │
│   ├── reference/                   # Canonical reference materials
│   │   ├── REF_EPISTEMIC_LABELS.md  # Tag definitions ([AXIOM], [THEOREM], etc.)
│   │   ├── REF_SYMBOL_GLOSSARY.md   # All notation and symbols
│   │   ├── REF_SCOPE_LIMITATIONS.md # What FTD does NOT address
│   │   ├── REF_EXPERIMENTAL_STATUS.md # Current testing/validation status
│   │   └── REF_NAMING_CONVENTIONS.md  # File naming standards
│   │
│   ├── papers/                      # Published/submitted papers
│   │   ├── *.pdf                    # Single active PDF shelf for papers and exported figure PDFs
│   │   ├── *.tex, *.md              # Root-level companion sources and paper indexes
│   │   ├── speculative/             # Speculative TeX source tree (active PDFs publish to root)
│   │   ├── src/                     # Main LaTeX source tree + figures (active PDFs publish to root)
│   │   └── archive/                 # Historical versions and unused figures
│   │
│   ├── articles/                    # Popular writing
│   │   └── quantum_isnt_weird.md
│   │
│   └── internal/                    # Working documents & editorial guidance (gitignored)
│       ├── SPEC_CLAUDE.md           # Simulation manual
│       ├── META_WALKTHROUGH.md      # Local-only working walkthrough
│       ├── META_IMPLEMENTATION_PLAN.md
│       ├── META_BULLETPROOFING_STRATEGY.md
│       ├── REF_IMAGE_INVENTORY.md   # Image catalog
│       └── explorations/            # Research scripts & visualizations
│           ├── lemniscate/           # Lemniscate curve explorations
│           ├── consciousness/       # Consciousness/G* explorations
│           ├── mandelbrot/          # Mandelbrot-FTD connections
│           └── number_theory/       # Number theory explorations
│
├── engine/                          # C++ simulation engine (see engine/SPEC_ENGINE.md for current version details)
│   ├── SPEC_ENGINE.md               # Engine reference document
│   ├── include/ftd/                 # 29 headers (ontic.h is the constant chain; scenarios.h for Scale-0 library)
│   ├── src/                         # 14 source files
│   ├── tests/                       # Large CTest surface (CPU, campaign, optional GPU)
│   ├── cuda/                        # GPU acceleration
│   ├── wasm/                        # Emscripten WASM bindings
│   └── web/                         # Three.js browser dashboard
│
├── scripts/                         # Python verification and proof stack
│   ├── constants.py                 # Canonical shared constants
│   ├── verification/                # Formal derivation verification
│   ├── proofs/                      # Formal mathematical proofs with error bounds
│   │   └── proof_modular_hamiltonian.py  # Tomita-Takesaki modular operator on finite FTD lattice (April 2026)
│   ├── experiments/                 # Bell tests, CERN analysis, physics sims
│   ├── exploration/                 # Focused research investigations
│   │   ├── gap_equation_layer_convergence.py  # Sublattice Watson integrals and gap equation convergence (April 2026)
│   │   ├── verify_zero_modes.py               # Zero mode count verification across lattice types (April 2026)
│   │   └── verify_nmeas_18.py                 # Three routes to N_meas = 18 — all negative (April 2026)
│   ├── tests/                       # Python test suites (pytest)
│   │   └── comprehensive/           # 7-tier verification framework
│   ├── visualization/               # Publication figure generation
│   └── runners/                     # Test protocol runners
│
├── evaluation/                      # Multi-domain assessment
│   ├── agent_findings/              # AI domain evaluations
│   ├── expert_reviews/              # Expert reviews + physicist final report
│   ├── findings/                    # Cross-cutting domain findings
│   ├── AUDIT_UNRESOLVED_ISSUES.md   # Post-defense mandatory acknowledgments
│   ├── AUDIT_WEAKNESSES_MASTER.md   # Master weakness compilation (18 agents)
│   └── ISSUE_TRACKER.md             # 116 prioritized issues (12 P0, 35 P1)
│
├── dissemination/                   # Publication pipeline
│   ├── manuscript/                  # Quarto manuscript (original)
│   ├── manuscript_v2/               # Complete rewrite for physicists (April 2026)
│   │   ├── CHECKLIST.md             # Live progress tracker
│   │   ├── src/
│   │   │   ├── _quarto.yml          # 17-part book config
│   │   │   ├── index.qmd            # Introduction
│   │   │   ├── preface.qmd          # Preface
│   │   │   └── chapters/            # 83 .qmd files (26 new + 57 from v1)
│   │   └── media/ -> ../manuscript/media  # Symlink to shared images
│   ├── book/                        # Narrative companion book
│   ├── whitepaper/                  # LaTeX academic paper
│   ├── notebooks/                   # Jupyter pedagogy tutorials
│   ├── interactive/                 # Standalone HTML simulations and explainers
│   └── FTD_Symbol_Cheatsheet.html   # Interactive symbol reference (~130 cards)
│
├── models/                          # Physics derivation package
│   ├── epistemic/                   # Axiomatic definitions
│   └── *.py                         # Core physics modules
│
└── archive/                         # All legacy/superseded content (gitignored)
    ├── ftd_archive/                 # Legacy engines (Python, Qt GUI, ImGui, etc.)
    ├── pre_ftd_root/                # Original root files before restructure
    ├── trd_working_docs/            # Early TRD-era working documents
    ├── legacy_scripts/              # Superseded scripts
    ├── web-app/                     # Superseded web platform
    └── [atoms, fermat_writeup, symmetry-of-zero, verification]
```

---

## Theory Documents At A Glance

Use [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) as the **curated theory catalog**.

Cleanup note:

- The curated index and the raw filesystem are not the same thing right now.
- The live theory tree currently contains more active-category Markdown files than the indexed catalog because some superseded or historical-in-place documents still live outside `archive/`.
- For current cleanup findings and raw-count context, see [AUDIT_DOCUMENT_CLEANUP_LEDGER.md](AUDIT_DOCUMENT_CLEANUP_LEDGER.md).
- For category boundaries and archive rules, see [docs/theory/META_STRUCTURE.md](docs/theory/META_STRUCTURE.md).

The current theory categories are:

1. Master Reference
2. Ontological Foundations
3. Core Physics Derivations
4. Coupling Constants and Precision
5. Particle Physics Applications
6. Consciousness and Measurement
7. Critical Self-Assessment
8. Structural Principles
9. Mathematical Connections

---

## Reference Materials

Located in `docs/reference/`:

| Document | Purpose |
|----------|---------|
| [REF_EPISTEMIC_LABELS.md](docs/reference/REF_EPISTEMIC_LABELS.md) | How to read epistemic tags: [AXIOM], [THEOREM], [CONJECTURE], [IMPOSED], [EMERGENT], [OPEN] |
| [REF_SYMBOL_GLOSSARY.md](docs/reference/REF_SYMBOL_GLOSSARY.md) | All mathematical notation, symbols, and dimensions |
| [REF_SCOPE_LIMITATIONS.md](docs/reference/REF_SCOPE_LIMITATIONS.md) | What FTD does NOT address — required acknowledgments |
| [REF_EXPERIMENTAL_STATUS.md](docs/reference/REF_EXPERIMENTAL_STATUS.md) | Current testing and validation status |
| [REF_NAMING_CONVENTIONS.md](docs/reference/REF_NAMING_CONVENTIONS.md) | File and code naming standards |
| [AUDIT_PROJECT_HEALTH_SCORECARD.md](evaluation/AUDIT_PROJECT_HEALTH_SCORECARD.md) | Current weighted project health scorecard |
| [REF_PROJECT_HEALTH_SCORING.md](evaluation/REF_PROJECT_HEALTH_SCORING.md) | Stable methodology for future health scoring |

---

## Build & Run Commands

| Task | Command |
|------|---------|
| Run all Python tests | `python scripts/tests/run_all_tests.py` |
| Run 7-tier verification | `python scripts/tests/comprehensive/run_ultimate_test.py` |
| Run proof chain | `python scripts/proofs/proof_10_ultimate_chain.py` |
| Build C++ engine | `cmake -S engine -B engine/build && cmake --build engine/build --config Release` |
| Run C++ tests | `cd engine/build && ctest --output-on-failure -C Release` |
| Build WASM | `emcmake cmake -S engine -B engine/build_wasm && emmake cmake --build engine/build_wasm --target ftd_wasm` |
| Launch web dashboard | `python -m http.server 8080 -d engine/web` |
| Build manuscript (HTML) | `cd dissemination/manuscript && quarto render` |
| Build whitepaper | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |

---

## Archive Policy

A document is **archived** when:
- A newer document **supersedes** it
- It uses **outdated naming** (TRD -> FTD)
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
