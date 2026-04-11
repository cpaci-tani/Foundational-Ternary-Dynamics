# Foundational Ternary Dynamics (FTD) — Project Instructions

**Version:** 5.29
**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)

---

## Epistemic Discipline

> **These rules are mandatory for all AI work on this project:**
> - **Do NOT** run numerical search scripts looking for near-misses or coincidences
> - **Do NOT** create substitution identities (plugging FTD values into formulas and calling the result a "discovery")
> - **Do NOT** label parametric insertions as "derivations" — if standard physics provides the formula and FTD provides the numbers, that is a **parametric insertion**, not a derivation

### Epistemic Tags

| Tag | Meaning | Reviewer expectation |
|-----|---------|---------------------|
| **[AXIOM]** | Structural postulate (not derivable) | Accept as model definition |
| **[THEOREM]** | Rigorously proven from axioms | Check proof |
| **[SELECTION]** | Argued from consistency, not uniquely proven | Critique argument |
| **[CONJECTURE]** | Proposed interpretation requiring validation | Demand evidence |
| **[IMPOSED]** | Parameter choice or model calibration | Note as input, not output |
| **[EMERGENT]** | Behavior arising from dynamics (not designed in) | Verify in simulation |
| **[OPEN]** | Unresolved question | Research opportunity |

---

## What FTD Is

A discrete computational framework for simulating physical systems from explicit postulates. The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive (+1), or negative (−1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at maximum one lattice unit per discrete time step.

**Two-layer ontology:**
- **Flux field** J ∈ ℝ³ — continuous vector field encoding potential energy density (dispositional)
- **State field** s ∈ {−1, 0, +1} — discrete ternary states representing manifestation (actual)

**Five postulates:** Discrete space (3D cubic lattice), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Key results** (within framework assumptions):
- Fine structure constant α = 1/137.036 derived from lemniscatic constant G* (1.26 ppm tree-level; 9.6 ppb one-loop; < 0.001 ppt with 7-term expansion)
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.27% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.8 GeV (0.24% error), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb
- Lamb shift: 1055.4 MHz (0.23% from experiment)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell violation S = 2√2 from Gauss constraint → 2D transverse flux
- Full nonlinear Einstein equations via Deser iterative bootstrap
- D = 3 uniquely selected (no longer axiomatic)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py`
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~50 predictions tested (20 structural theorems, 20 G*-derived, 10 novel cube predictions), ~50 parametric insertions (FTD values in standard QFT formulas), ~50+ external physics adopted. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md).

---

## Project Structure

```
ftd/                                     # Project root
├── docs/
│   ├── SPEC_FTD.md              # THE authoritative FTD specification (single source of truth)
│   ├── theory/                   # 115 core theory documents (10 categories)
│   │   ├── META_INDEX.md         # Complete catalog
│   │   ├── 01_reference/         # Master references and proofs
│   │   ├── 02_foundations/       # Ontological emergence
│   │   ├── 03_derivations/       # Core physics derivations
│   │   ├── 04_coupling/          # Coupling constants
│   │   ├── 05_particles/         # Particle physics
│   │   ├── 06_consciousness/     # Consciousness and measurement
│   │   ├── 07_assessment/        # Epistemic audits
│   │   ├── 08_structural/        # Geometry and information theory
│   │   ├── 09_mathematical/      # Number theory and connections
│   │   └── archive/              # Superseded/historical documents
│   ├── reference/                # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY, etc.
│   ├── papers/                   # Published/submitted PDFs and TeX sources
│   └── internal/                 # Session summaries, exploration scripts
├── engine/                       # C++ simulation engine (v2.11)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 28 headers (ontic.h, voxel.h, lattice.h, etc.)
│   ├── src/                      # 7 source files
│   ├── tests/                    # 169+ test files (120 unit + 49 campaign + 4 GPU)
│   ├── cuda/                     # GPU acceleration (RTX 5090, 363x speedup)
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, 28 JS modules)
├── scripts/                      # ALL Python scripts (~149 scripts)
│   ├── constants.py              # Canonical shared constants (single source of truth)
│   ├── verification/             # Formal derivation verification (40 scripts)
│   ├── proofs/                   # Formal mathematical proofs with error bounds (57 scripts)
│   ├── experiments/              # Bell tests, CERN analysis, physics sims (17 scripts)
│   ├── exploration/              # Focused research investigations (25+ scripts)
│   ├── tests/                    # Python test suites — pytest (11 scripts)
│   │   └── comprehensive/        # 7-tier verification framework
│   ├── visualization/            # Publication figure generation (11 scripts)
│   └── runners/                  # Test protocol runners (2 scripts)
├── evaluation/                   # Multi-domain assessment & certification
├── dissemination/                # All publication/outreach content
│   ├── manuscript/               # 96-chapter Quarto book (src/ + media/images/)
│   ├── whitepaper/               # LaTeX whitepaper + figures
│   ├── book/                     # "The Golden Thread" narrative (53 .qmd files)
│   ├── notebooks/                # 12 Jupyter pedagogy notebooks
│   └── interactive/              # 8+ standalone HTML simulations (forces, photon, Hamiltonian bridge, ternary cube)
├── models/                       # Physics derivation package (gitignored)
├── archive/                      # All legacy/superseded content (gitignored)
│   ├── ftd_archive/              # Legacy engines (Python, Qt GUI, ImGUI, etc.)
│   ├── pre_ftd_root/             # Original root files before restructure
│   ├── trd_working_docs/         # Early TRD-era working documents
│   └── web-app/                  # Superseded web platform (content, schemas, packages)
├── META_DOCUMENTATION_MAP.md     # Master catalog / card catalog
└── META_PROJECT_ATLAS.md         # AI agent navigation guide
```

---

## C++ Engine

**Build**: `cmake -S engine -B engine/build && cmake --build engine/build --config Release`
**Test**: `cd engine/build && ctest --output-on-failure -C Release`
**WASM**: `emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release && emmake cmake --build engine/build_wasm --target ftd_wasm`
**Web UI**: `python -m http.server 8080 -d engine/web`

### Key Constants (all derived from D=3 + varpi via `ontic.h`)

| Constant | Value | Origin |
|----------|-------|--------|
| G* (lemniscatic) | 2.95868... | Γ(1/4)/Γ(3/4) |
| α (fine structure) | 1/137.036 | Master quadratic x₊ |
| N_c (colors) | 3 | Master quadratic x₋ |
| K_B (manifestation) | 0.511 | m_e = m_P·√(2π)·(16/3)·α¹¹ |
| C_SPEED | 1/√3 | CFL stability on cubic lattice |
| G_N (gravity) | 0.01 | 1/(b₃+N_c)² |

### Engine Philosophy

Logic-first: only 6 rules derived from axioms. All phenomenological features are toggle-gated extensions (default OFF).

**Tick cycle:** phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++

---

## Key Navigation Documents

- **Full FTD spec**: `docs/SPEC_FTD.md`
- **Engine spec**: `engine/SPEC_ENGINE.md`
- **Theory catalog**: `docs/theory/META_INDEX.md`
- **Documentation map**: `META_DOCUMENTATION_MAP.md`
- **Epistemic audit**: `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md`
- **Changelog**: `CHANGELOG.md`
- **Complete SM**: `scripts/proofs/proof_complete_sm.py`
- **Motivic proof**: `scripts/proofs/proof_motivic_master_quadratic.py`
- **Moore Layer Theorem**: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- **Phase lattice**: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`
- **50-test battery**: `scripts/exploration/test_all_physics.py`
- **Loop derivations**: `scripts/exploration/compute_c2.py`, `derive_all_loops.py`, `gauge_loops.py`
- **Arrow paper**: `docs/papers/PAPER_RATIO_AND_THE_ARROW.tex`
- **Engine coupling test**: `engine/tests/test_intervoxel_coupling.cpp`
- **Complete Chain** (April 2026): `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`
- **QM as Statistics** (April 2026): `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md`
- **Lattice Physics Reference** (April 2026): `docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md`
- **Stellar Lifecycle** (April 2026): `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md`
- **Master Verification** (April 2026): `scripts/proofs/proof_master_verification.py` (54/54 checks)

---

## Naming Conventions

- Markdown files: `UPPER_SNAKE_CASE` with semantic prefix
- Prefixes: `SPEC_` (specifications), `DERIV_` (derivations), `FOUND_` (foundations), `AUDIT_` (assessment), `EXPLR_` (exploratory), `REF_` (reference), `ARCH_` (archived), `META_` (meta-documentation)
- Engine: C++17, snake_case functions, CamelCase types

---

## Environment Notes

- Platform: Windows 11. No `rsync` — use `cp -r` for directory copies
- Python tests: `scripts/tests/` (pytest). C++ tests: `engine/tests/` (CTest). No overlap between them
- `scripts/constants.py` is the canonical shared constants module imported by 20+ scripts
- Build `.bat` files live in `engine/` — use `vswhere.exe` for portable VS detection
- `dissemination/media/`, `models/`, and `archive/` are gitignored — they exist on disk but not in git
- `docs/internal/` is gitignored — session summaries and explorations are local-only
