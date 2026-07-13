# Foundational Ternary Dynamics (FTD) — Project Instructions

**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)
**Authoritative project instructions:** [`CLAUDE.md`](CLAUDE.md) — this file (AGENTS.md) is a sibling reference; if the two disagree, CLAUDE.md wins.

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

## Documentation Cleanup Discipline

> **These rules are mandatory for AI cleanup work.** The goal is persistent consolidation, not one-off tidying that creates future drift.

- **Preserve provenance; move, do not erase.** Superseded, retracted, resolved, and closed-negative theory documents should be archived with `git mv`, not deleted, unless the user explicitly asks for deletion.
- **Keep active directories active.** Documents whose live status is `[CLOSED NEGATIVE]`, `[RETRACTED]`, or `[CLOSED -- RESOLVED]` should live under `docs/theory/archive/` or a local archive such as `docs/theory/10_eft_program/archive/{closed_negative,resolved,retracted}/`.
- **Track cleanup provenance deliberately.** Theory archives used as canonical cleanup provenance must be tracked in git. The top-level `archive/` directory remains ignored. Local cleanup archives such as `docs/theory/10_eft_program/archive/**` are tracked wholesale; the broad `docs/theory/archive/` directory uses explicit `.gitignore` exceptions, so add a matching exception whenever a new canonical top-level archived file is introduced.
- **Update all navigation layers in the same cleanup.** If a file is moved or status-changed, update the relevant index/tracker/spec references in the same commit: `docs/theory/META_INDEX.md`, local sub-indexes such as `docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`, `docs/theory/07_assessment/core_ledgers/LEDGER.md`, `TRACKER_OPEN_ITEMS.md`, and any project-level maps that link to the file.
- **Open trackers must contain open work.** Do not leave closed, retracted, or resolved items counted as `[OPEN]`. Either remove them from `TRACKER_OPEN_ITEMS.md`, move them to a resolved/provenance tracker, or clearly mark them as "not counted as open" until a resolved tracker exists.
- **Do not promote claims during cleanup.** Cleanup may clarify status, archive provenance, and align links; it must not upgrade epistemic tags or introduce new derivations without a separate audit.
- **Verify before committing.** At minimum run `git diff --check` and `rg` for old active paths after any move. Use documentation/link checks only; do not run numerical near-miss or coincidence searches as part of cleanup.
- **Commit cleanup in small coherent batches.** Prefer one commit per cleanup theme (archive tracking, tracker split, index reconciliation, sector consolidation) so future agents can audit the history.

---

## What FTD Is

A discrete computational framework for simulating physical systems from explicit postulates. The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive (+1), or negative (−1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at maximum one lattice unit per discrete time step.

**Two-layer ontology:**
- **Flux field** J ∈ ℝ³ — continuous vector field encoding potential energy density (dispositional)
- **State field** s ∈ {−1, 0, +1} — discrete ternary states representing manifestation (actual)

**Five postulates:** Discrete space (3D cubic lattice), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Key results** (within framework assumptions):
- Fine structure constant α = 1/137.036 from lemniscatic constant G* (1.26 ppm tree-level) — identification x₊ = 1/α is [STRONGLY MOTIVATED CONJECTURE]
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.19% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.75 GeV — **−0.36% vs PDG 2024's 125.20 ± 0.11 GeV, a −4.1σ discrepancy at current precision** (corrected 2026-07-01, FTD-0348: the earlier "0.24% error" reproduced only against the superseded PDG-2020 value 125.10; at PDG-2024 precision the exact relation is experimentally excluded), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb — **[PARAMETRIC]** (imported multi-loop QED with FTD's α inserted; evidence for α's value, not a substrate derivation of g-2)
- Lamb shift: 1055.4 MHz (0.23% from experiment) — **[PARAMETRIC]** (standard QED one-loop with FTD's α inserted)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell correlations: S = 2√2 (Tsirelson) is imported standard QM conditional on the [SELECTION] singlet — "FTD does not violate Bell"; the substrate itself is local/classical, S ≤ 2 natively (corrected 2026-07-01, FTD-0347)
- Full nonlinear Einstein equations via Deser iterative bootstrap — the bootstrap *completes* a posited massless spin-2 field, it does not derive one (FTD-0189; emergent spin-2 mode is [OPEN])
- D = 3 uniquely selected (no longer axiomatic)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py` — an internal-consistency check (insertions reproduce fitted values), not 50 independently-confirmed predictions
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~21 derived/theorem-grade claims (corrected 2026-07-01, FTD-0348 — the μ/τ mass-ratio rows moved to [STRUCTURALLY MOTIVATED PARAMETRIC]), ~131 parametric insertions (FTD values in standard QFT formulas), ~10 imposed/selected, ~50+ external physics adopted. 50 physics tests pass across three tiers. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md).

---

## Project Structure

```
ftd/                                     # Project root
├── docs/
│   ├── SPEC_FTD.md              # THE authoritative FTD specification
│   ├── theory/                   # 586 theory documents across 10 categories + archives
│   │   ├── META_INDEX.md         # Curated catalog
│   │   ├── 01_reference/         # Master references, algebraic spine, construction monograph
│   │   ├── 02_foundations/       # Ontological emergence, lattice physics
│   │   ├── 03_derivations/       # Core physics derivations (QM, EM, gravity, SM sectors)
│   │   ├── 04_coupling/          # Coupling constants and precision
│   │   ├── 05_particles/         # Particle physics applications
│   │   ├── 06_reference_frames_and_measurement/     # Reference frame context and measurement
│   │   ├── 07_assessment/        # Epistemic audits, ledgers, campaigns
│   │   ├── 08_structural/        # Moore theorem, geometry, BCC structure
│   │   ├── 09_mathematical/      # Number theory, CM curves, L-values
│   │   ├── 10_eft_program/       # Native EFT recovery program + pre-registrations
│   │   └── archive/              # 118 archived documents (provenance preserved)
│   ├── reference/                # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY, etc.
│   ├── papers/                   # 66 LaTeX source files + compiled PDFs
│   └── internal/                 # Session summaries, exploration scripts (gitignored)
├── engine/                       # C++ simulation engine (v2.18.0)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 59 headers (ontic.h, voxel.h, lattice.h, etc.)
│   ├── src/                      # 22 source files
│   ├── tests/                    # 298 test source files (211 active CTest targets)
│   ├── cuda/                     # GPU acceleration (WSL2 + RTX 5090)
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, 868 JS files, 5 CSS themes)
├── scripts/                      # 520 Python scripts (138K LOC)
│   ├── constants.py              # Canonical shared constants (single source of truth)
│   ├── verification/             # Formal derivation verification (61 scripts)
│   ├── proofs/                   # Formal mathematical proofs with error bounds (143 scripts)
│   ├── experiments/              # Bell tests, CERN analysis, physics sims (19 scripts)
│   ├── exploration/              # Focused research investigations (195+ scripts)
│   ├── tests/                    # Python test suites — pytest (14 scripts)
│   │   └── comprehensive/        # 7-tier verification framework
│   ├── visualization/            # Publication figure generation (23 scripts)
│   ├── benchmarks/               # Engine vs theory benchmarks (6 scripts)
│   └── runners/                  # Test protocol runners (3 scripts)
├── evaluation/                   # Multi-domain assessment & certification
├── dissemination/                # All publication/outreach content
│   ├── whitepaper/               # LaTeX whitepaper + figures
│   ├── papers/                   # 4 additional compiled LaTeX papers
│   ├── interactive/              # 31 standalone HTML simulations
│   └── notebooks/                # Jupyter pedagogy notebooks
├── models/                       # Physics derivation package (gitignored)
├── archive/                      # Curated historical record (gitignored)
├── META_DOCUMENTATION_MAP.md     # Master catalog / card catalog
└── META_PROJECT_ATLAS.md         # AI agent navigation guide
```

---

## C++ Engine

**Build**: `cmake -S engine -B engine/build && cmake --build engine/build --config Release --parallel 24` (Maximize CPU threads on the AMD 9950X3D)
**Test**: `cd engine/build && ctest -j 24 --output-on-failure -C Release` (Always use parallel execution to avoid sequential runs taking forever)
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

**Tick cycle (10 phases):** phase_read → phase_write → pair_production → gauss_project → latency_solve → phase_forces → phase_movement → boundary → weak/triad → proper_time

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
- **Engine coupling test (of record)**: `engine/tests/test_native_moore_layer_coupling.cpp` + `test_native_moore_temporal_layers.cpp` (native Moore-layer flux status). The legacy `test_intervoxel_coupling.cpp` was DELETED 2026-05-03 (`e8eb8e82`) — its FCC/SC = sin²θ_W/α and BCC/SC = α_s/α targets used demoted [PARAMETRIC] identifications; Moore-shell flux falloff is geometric, not coupling-ratio
- **Complete Chain**: `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`
- **QM as Statistics**: `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md`
- **Lattice Physics Reference**: `docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md`
- **Stellar Lifecycle**: `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md`
- **Master Verification**: `scripts/proofs/proof_master_verification.py` (54/54 checks)

---

## Naming Conventions

- Markdown files: `UPPER_SNAKE_CASE` with semantic prefix
- Prefixes: `SPEC_` (specifications), `DERIV_` (derivations), `FOUND_` (foundations), `AUDIT_` (assessment), `EXPLR_` (exploratory), `REF_` (reference), `ARCH_` (archived), `META_` (meta-documentation)
- Engine: C++17, snake_case functions, CamelCase types

---

## Environment Notes

- Platform: Windows 11. No `rsync` — use `cp -r` for directory copies
- **Hardware Profile / CPU Parallelization**: The host machine is equipped with an ultra-high-end AMD Ryzen 9 9950X3D (16 cores, 32 threads) and an NVIDIA RTX 5090. When running CPU-based CTests or C++ compilations, **always maximize CPU resource load by passing high concurrency flags** (e.g. `ctest -j 24` or `ctest -j 32`, and `cmake --build ... --parallel 24` or `--parallel 32`). Without parallelization, sequential test execution will run pathologically slow and waste massive compute capacity.
- **GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA.** RTX 5090 speedup (~30×) is only available via the WSL2 build at `engine/build_wsl`. Any measurement campaign, sweep, or multi-seed run goes through WSL2.
- Python tests: `scripts/tests/` (pytest). C++ tests: `engine/tests/` (CTest). No overlap between them
- `scripts/constants.py` is the canonical shared constants module imported by 20+ scripts
- Build `.bat` files live in `engine/` — use `vswhere.exe` for portable VS detection
- `dissemination/media/`, `models/`, and `archive/` are gitignored — they exist on disk but not in git
- `docs/internal/` is gitignored — session summaries and explorations are local-only
