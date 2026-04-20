# Foundational Ternary Dynamics (FTD) — Project Instructions

**Version:** 5.31 (post-audit, 2026-04-19)
**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)
**🔑 Start here if resuming:** [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md) — single-doc context recovery from the April 19 audit cycle.

---

## Current epistemic state (2026-04-19 evening)

After a 10-commit audit cycle on April 19, the project is in a
narrowed but defensible state. Do **not** claim results stronger than
what's listed in `docs/WHERE_WE_LEFT_OFF.md` §4 without re-auditing.

**Firm theorems (5):** G* algebraic identity, master quadratic
polynomial + roots, CM curve uniqueness among class-number-1 fields,
Phase G emergent Coulomb = lattice Poisson Green's function at every
finite L, Phase J partition-function ultralocality.

**[STRONGLY MOTIVATED CONJECTURE]:** x+ = 1/α (1.26 ppm); x− = N_c
(0.80%); m_e formula (0.19%); m_p/m_e formula (173 ppm).

**[OPEN] (the real research program):** lattice-to-physical-length
conversion a_phys; first-principles g_c via Mechanism B (lattice-to-
continuum matching).

**Demoted today:** sin²θ_W (3.5%), sin²θ_13 (12.6%), α_s = 7/59, PMNS
angles — all now [PARAMETRIC] or [STRUCTURALLY MOTIVATED PARAMETRIC].

**Foundational commitment:** undefined-boundary lattice ontology (not
completed-infinity ℤ³). See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md`.

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

**Five postulates:** Discrete space (3D cubic lattice, no defined boundary — at every specified position, axis-adjacent sites exist; **not** a completed-infinity ℤ³ totality, per `AUDIT_INFINITY_REFRAME.md`), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Foundational commitment (2026-04-19):** FTD uses **undefined-boundary** lattice ontology, not completed-infinity. Arbitrarily large finite computations are permitted; claims of the form "in the L → ∞ limit" are not well-posed without explicit ε-L restatement. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` for the full triage of which claims survive, which need restatement, and which need re-derivation.

**Key results** (within framework assumptions):
- Fine structure constant: master quadratic `x² − 16G*²x + 16G*³ = 0` has x₊ = 137.036 matching 1/α to **1.26 ppm** at tree level (pure algebra [THEOREM]; physical identification [STRONGLY MOTIVATED CONJECTURE] per `AUDIT_MASTER_QUADRATIC.md`). Same polynomial gives x₋ = 3.024 ≈ N_c = 3 — the dual-prediction property is the strongest structural evidence. The 7-term series matching CODATA to 24 digits is a post-hoc fit [CONJECTURE] beyond experimental precision (CODATA 2022 has ~11 digits), not a "< 0.001 ppt derivation"
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.27% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.8 GeV (0.24% error), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb
- Lamb shift: 1055.4 MHz (0.23% from experiment)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- BCC multiplicative structure: Watson identity W₃ = G*²/(2π) and SU(3) gauge group both arise from the BCC eigenvalue's triple cosine product (docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell violation S = 2√2 [SELECTION] resolved as emergent from QM lattice emergence (Tsirelson's bound; April 2026)
- Full nonlinear Einstein equations via Deser iterative bootstrap
- D = 3 uniquely selected (no longer axiomatic)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py`
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~50 predictions tested (20 structural theorems, 20 G*-derived, 10 novel cube predictions), ~50 parametric insertions (FTD values in standard QFT formulas), ~50+ external physics adopted. Manuscript v2: 83 chapters (26 new + 57 editorial pass). April 11 audit: 267/267 Python tests pass, 54/54 master verification pass, 3 META_INDEX overclaims fixed. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md).

**Engine-theory bridge (April 13, 2026):** 20-benchmark suite connects engine output to theory. Coulomb 1/r^2 converges (B+), hydrogen 1/n^2 < 0.001% (A+), color forces correct (A+), Higgs threshold exact (A+), Bell S=2.000 (A+), Born lattice bias 10x (A-). EFT reconstruction: alpha = G_C^2 (derived, not input). Added Wilson loops (12/17, flux tube detected), gluon dynamics (7/11, linear E(r)), budget equation (0.2% at r=6). LATENCY FIX unlocked GR: time dilation 0.004% match, BH gravitational wells L_peak=0.62. Three theorem papers: continuum limit -> QED, singlet from void event, N_c from topology. WASM rebuilt and deployed. 148/166 CTest passing. Scientific status: C+ -> B+.

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
├── engine/                       # C++ simulation engine (v2.14)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 29 headers (ontic.h, voxel.h, lattice.h, scenarios.h, etc.)
│   ├── src/                      # 14 source files
│   ├── tests/                    # 169+ test files (120 unit + 49 campaign + 4 GPU)
│   ├── cuda/                     # GPU acceleration
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, modular JS)
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
│   ├── manuscript_v2/            # 83-chapter physicist-targeted rewrite
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
- **Parametric insertions catalog** (April 19, 2026): `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` (~162 rows enumerated: ~23 [DERIVED]/[THEOREM], ~129 [PARAMETRIC], ~10 [IMPOSED]/[SELECTION])
- **EFT Recovery Program** (April 19, 2026, COMPLETE Phase 0 → F): `docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md` — pre-registered 7-phase campaign run to completion. **Headline:** α_∞ plateau at ~3.6× α_ref across L ∈ {64, 128, 256, 384} GPU scan (4.05× → 3.61×, three scaling laws agree on α_∞ ∈ [3.35, 3.74] × α_ref). Day-2 interim "1.23×" claim **RETRACTED** (ticks=100 under-equilibrated). Falsifiable FTD deviation from CODATA QED, not agreement. Day-2 shipped: matched-stencil CG Poisson (Ward floor 1% → 1e-8), EWSB sharp first-order transition at amp ∈ (0.6, 0.7), condensate m ≈ 0.18 (flux/charge channels agree 3%), Rutherford α = 0.042 ± 0.005 independent cross-check. WSL2 + CUDA 13 path unblocks RTX 5090 (30× speedup). Pipeline<Backend> architecture with CPU/GPU parity. Paper: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`. Day-2 doc: `docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md`. Plan: `C:\Users\cpaci\.claude\plans\vivid-marinating-pudding.md`
- **Engine callstack audit**: `docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md` (CPU/GPU parity, toggle gaps, 10 findings)
- **Open items tracker**: `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md` (every `[OPEN]` across code + theory, one place)
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
- **BCC Unification** (April 2026): `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`
- **Observer Formalism** (April 2026): `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` (Part II: 3³ lattice grounding)
- **Manuscript v2**: `dissemination/manuscript_v2/CHECKLIST.md` (83 chapters, physicist-targeted)
- **Engine-Theory Bridge** (April 13, 2026): `engine/tests/benchmark_engine_theory.cpp` (20 benchmarks)
- **Emergent Alpha** (April 13, 2026): `engine/tests/benchmark_emergent_alpha.cpp` (6 EFT experiments)
- **Benchmark Harness**: `scripts/benchmarks/benchmark_engine_vs_theory.py` (Python analysis)
- **Convergence Analysis**: `scripts/benchmarks/analyze_convergence.py` (20-benchmark report + plots)
- **Benchmark Results**: `scripts/benchmarks/results/` (reports, plots, CSV)
- **Wilson Loops** (April 13, 2026): `engine/tests/benchmark_wilson_loops.cpp` (12/17 pass, flux tube detected)
- **Gluon Dynamics** (April 13, 2026): `engine/tests/campaign_gluon_dynamics.cpp` (7/11 pass, linear E(r))
- **Einstein Equations** (April 13, 2026): `engine/tests/test_einstein_equations.cpp` (time dilation 0.004% match after latency fix)
- **BH Thermodynamics** (April 13, 2026): `engine/tests/benchmark_black_hole_thermo.cpp` (L_peak 0.62, proper time dilation)
- **Budget Equation** (April 13, 2026): `engine/tests/benchmark_budget_equation.cpp` (x/K+G*/x=1 to 0.2%)
- **Continuum Limit -> QED** (April 13, 2026): `docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (x+ = 1/alpha conditional [THEOREM])
- **Singlet from Void** (April 13, 2026): `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md` (Bell loop closed via 5 lemmas)
- **N_c from Topology** (April 13, 2026): `docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md` (N_c = 3 from 4 independent routes)
- **Web refactor spec** (April 18-19, 2026): `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` (Waves 0-3 split of viewport/wasm-bridge-dag/app_dag + RF-1/3/4/5/6/7/8/10 post-audit cleanup; Ticket 14 + RF-9-full deferred). Final: viewport 5325→3900, wasm-bridge-dag 5736→2132, app_dag 1898→1723 (−5204 LOC, −40% across three files)
- **Whole-project extraction** (April 19, 2026, v2.15.0): 16-agent parallel refactor split every file ≥500 LOC into discrete-responsibility modules. C++: render_bridge.cpp 2139→1097, constructors.cpp 1245→0 (deleted, 5 split files), scenarios.cpp 1241→79, ftd_wasm.cpp 1224→607, cosmic_engine 1193→500, atom_engine 1029→325, main.cpp 938→74, ontic.h 806→45 (+6 theme-headers), ws_server 831→496. JS: mock-scale5 1903→313, consciousness triad −1711, scale controllers −1248, backgrounds 846→178, field-overlays 976→455. Python: 3 common helpers extracted. Total: ~13800 LOC redistributed across ~97 new files, every module nameable in ≤5 words
- **Scenario library (C++)** (April 18, 2026): `engine/include/ftd/scenarios.h` + `engine/src/scenarios/{flux,light,quantum,s0_seed,s0_field}.cpp` (all 83 Scale-0 scenarios ported from JS MockBridge; 84/84 Playwright coverage, 5/5 parity CI guard)
- **Web power-user guide** (April 18-19, 2026): `engine/web/docs/USER_GUIDE.md` (15-section reference for dashboard + console workflows)
- **Scenario parity CI guard** (April 19, 2026): `engine/web/tests/scenario-parity.spec.js` (5 assertions covering JS↔C++ scenario name drift; runs in <1s)
- **Viewport extracted modules** (April 19, 2026): `engine/web/js/viewport/{color-ramps,molecular-renderer,boundary-geometry,topology-sheet-renderer}.js` (own their Three.js concerns; viewport.js keeps thin delegators)

---

## EFT Reconstruction (April 13, 2026)

Alpha is now a DERIVED quantity in the engine:
- `ALPHA_EFT = G_C * G_C` defined in `constants.h` with compile-time `static_assert`
- G_C (wave equation coupling) is the fundamental lattice parameter
- All force computations use `ALPHA_EFT` (= G_C²), not hardcoded `ALPHA`
- New toggle `emergent_forces` computes force from flux gradient without Poisson solver
- 20-benchmark suite validates: Coulomb convergence (B+), hydrogen spectrum (A+), color forces (A+), Higgs threshold (A+), Bell S=2 (A+), Born lattice bias (A-), 139/179 CTest passing

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
