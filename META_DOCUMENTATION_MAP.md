# FTD Documentation Map

**The Card Catalog** — Find any document in two clicks.

**Last updated:** March 27, 2026
**Framework version:** v5.28-consolidated
**Engine version:** v2.11

---

## Quick Start

| If you want to... | Start here |
|-------------------|------------|
| Understand FTD from scratch | [docs/SPEC_FTD.md](docs/SPEC_FTD.md) — **the single source of truth** |
| See the core mathematics | [docs/theory/01_reference/SPEC_THE_MASTER_CUBIC.md](docs/theory/01_reference/SPEC_THE_MASTER_CUBIC.md) |
| Assess what's genuinely derived | [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) (~30 genuine) |
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
2. [docs/internal/META_WALKTHROUGH.md](docs/internal/META_WALKTHROUGH.md) — Navigation guide
3. `engine/` — C++ simulation engine with Three.js web dashboard
4. `engine/tests/` — 168 test files (119 unit + 49 campaign)

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
├── CHANGELOG.md                     # Version history (v5.0 → v5.28)
├── CONTRIBUTING.md                  # How to contribute
├── CLAUDE.md                        # AI agent project instructions
├── META_DOCUMENTATION_MAP.md        # THIS FILE — the card catalog
├── META_PROJECT_ATLAS.md            # AI agent navigation guide
│
├── docs/
│   ├── SPEC_FTD.md                  # THE authoritative FTD specification
│   ├── theory/                      # 114 active + 67 archived theory documents
│   │   ├── META_INDEX.md            # Complete catalog (9 categories)
│   │   ├── 01_reference/ (12)       # Master references and proofs
│   │   ├── 02_foundations/ (18)     # Ontological emergence
│   │   ├── 03_derivations/ (37)     # Core physics derivations
│   │   ├── 04_coupling/ (9)         # Coupling constants
│   │   ├── 05_particles/ (6)        # Particle physics
│   │   ├── 06_consciousness/ (6)    # Consciousness and measurement
│   │   ├── 07_assessment/ (7)       # Epistemic audits
│   │   ├── 08_structural/ (6)       # Geometry and information theory
│   │   ├── 09_mathematical/ (13)    # Number theory and connections
│   │   └── archive/ (67)            # Superseded/historical documents
│   │
│   ├── reference/                   # Canonical reference materials
│   │   ├── REF_EPISTEMIC_LABELS.md  # Tag definitions ([AXIOM], [THEOREM], etc.)
│   │   ├── REF_SYMBOL_GLOSSARY.md   # All notation and symbols
│   │   ├── REF_SCOPE_LIMITATIONS.md # What FTD does NOT address
│   │   ├── REF_EXPERIMENTAL_STATUS.md # Current testing/validation status
│   │   └── REF_NAMING_CONVENTIONS.md  # File naming standards
│   │
│   ├── papers/                      # Published/submitted papers
│   │   ├── *.tex, *.pdf             # Core papers (TeX+PDF pairs or PDF-only)
│   │   ├── speculative/             # Millennium Prize problems, speculative extensions
│   │   ├── src/                     # LaTeX sources + figures for complete papers
│   │   └── archive/                 # Historical versions and unused figures
│   │
│   ├── articles/                    # Popular writing
│   │   └── quantum_isnt_weird.md
│   │
│   └── internal/                    # Working documents & editorial guidance (gitignored)
│       ├── SPEC_CLAUDE.md           # Simulation manual
│       ├── META_WALKTHROUGH.md      # How to read the project
│       ├── META_IMPLEMENTATION_PLAN.md
│       ├── META_BULLETPROOFING_STRATEGY.md
│       ├── REF_IMAGE_INVENTORY.md   # Image catalog
│       └── explorations/            # Research scripts & visualizations
│           ├── lemniscate/           # Lemniscate curve explorations
│           ├── consciousness/       # Consciousness/G* explorations
│           ├── mandelbrot/          # Mandelbrot-FTD connections
│           └── number_theory/       # Number theory explorations
│
├── engine/                          # C++ simulation engine (v2.11)
│   ├── SPEC_ENGINE.md               # Engine reference document
│   ├── include/ftd/                 # 28 headers (ontic.h is the constant chain)
│   ├── src/                         # 7 source files
│   ├── tests/                       # 168 test files (119 unit + 49 campaign)
│   ├── cuda/                        # 5 GPU kernels (RTX 5090, 363x speedup)
│   ├── wasm/                        # Emscripten WASM bindings
│   └── web/                         # Three.js browser dashboard (28 JS modules)
│
├── scripts/                         # Python scripts (~149 total)
│   ├── constants.py                 # Canonical shared constants
│   ├── verification/ (40)           # Formal derivation verification
│   ├── proofs/ (57)                 # Formal mathematical proofs with error bounds
│   ├── experiments/ (17)            # Bell tests, CERN analysis, physics sims
│   ├── exploration/ (9)             # Focused research investigations
│   ├── tests/ (11+)                 # Python test suites (pytest)
│   │   └── comprehensive/           # 7-tier verification framework
│   ├── visualization/ (11)          # Publication figure generation
│   └── runners/ (2)                 # Test protocol runners
│
├── evaluation/                      # Multi-domain assessment
│   ├── agent_findings/ (6)          # AI domain evaluations
│   ├── expert_reviews/ (6)          # Expert reviews + physicist final report
│   ├── findings/ (4)               # Cross-cutting domain findings
│   ├── AUDIT_UNRESOLVED_ISSUES.md   # Post-defense mandatory acknowledgments
│   ├── AUDIT_WEAKNESSES_MASTER.md   # Master weakness compilation (18 agents)
│   └── ISSUE_TRACKER.md             # 116 prioritized issues (12 P0, 35 P1)
│
├── dissemination/                   # Publication pipeline
│   ├── manuscript/                  # Quarto book (96 .qmd chapters)
│   ├── book/                        # "The Golden Thread" narrative (53 .qmd files)
│   ├── whitepaper/                  # LaTeX academic paper
│   ├── notebooks/ (12)              # Jupyter pedagogy tutorials
│   ├── interactive/ (6)             # Standalone HTML force/photon simulations
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

## Theory Documents at a Glance

The 114 core theory documents in `docs/theory/` are organized into 9 categories.
See [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) for the complete catalog with descriptions.

| # | Category | Files | Key Documents |
|---|----------|-------|---------------|
| 1 | Master Reference | 12 | FTD_REFERENCE, MASTER_QUADRATIC (3-layer), MASTER_CUBIC, SIX_ALGORITHMS, LAGRANGIAN, SM_REPLACEMENT, COMPARATIVE_PHYSICS, NOVEL_PREDICTIONS |
| 2 | Ontological Foundations | 18 | THE_FIRST_DISTINCTION, COMPLETE_ALGEBRA_OF_i, SPACETIME_EMERGENCE, ONTOLOGICAL_GENESIS, ONTIC_FOUNDATIONS, EULER_IDENTITY, D3_UNIQUENESS |
| 3 | Core Physics | 37 | BOTTOM_UP_PHYSICS, QM_RESOLVED, RELATIVITY, HIGGS, PATH_INTEGRAL, EINSTEIN, BLACK_HOLES, CONFINEMENT, BELL_COSINE, MOORE_GAUGE_STRUCTURE |
| 4 | Coupling Constants | 9 | LEMNISCATE_WHITEPAPER, ALPHA_PRECISION, LAMBDA_QCD, WATSON_GSTAR_IDENTITY, COSMOLOGICAL_CONSTANT |
| 5 | Particle Physics | 6 | COMPLETE_PARTICLE_PHYSICS, ELECTROWEAK_MASSES, NEUTRINO_MASS_ABSOLUTE, QUARK_MASSES |
| 6 | Consciousness | 6 | EXISTENCE_FILTER, CONSCIOUSNESS_QFT_GR_SYNTHESIS, VON_NEUMANN_CHAIN, COLLAPSE_MECHANISM |
| 7 | Critical Self-Assessment | 7 | EPISTEMIC_AUDIT, BELL_ANALYSIS, HIDDEN_SELECTIONS, CLAIMS_MATRIX, GENUINELY_NEW |
| 8 | Structural Principles | 6 | CUBOCTAHEDRAL_INTEGERS, LOOP_GRID_DUALITY, GOLDEN_RATIO, TRIT_INFORMATION |
| 9 | Mathematical Connections | 13 | NUMBER_THEORY, RIEMANN_ZETA, CAYLEY_DICKSON, MODULAR_QUADRATIC, LFUNCTION_GSTAR, PARTITION_PRIMES |

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
