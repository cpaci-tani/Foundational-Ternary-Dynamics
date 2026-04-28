# META_PROJECT_ATLAS — FTD Engine Navigation Hub

**Audience: LLM agents and humans needing to find code/docs fast.**
**Update trigger: any directory creation, public-API change, or architectural decision.**
**Last refreshed: 2026-04-27 (Phase 0 of refactor sweep).**

This file is the entry point for navigating the FTD codebase. If you are a
fresh agent looking at this project for the first time, read this file →
read [CLAUDE.md](CLAUDE.md) → read [docs/WHERE_WE_LEFT_OFF.md](docs/WHERE_WE_LEFT_OFF.md) → start work.

---

## §1 — "Where do I look for X?"

| If you want to… | Look here | Entry-point file |
|---|---|---|
| Add a new physics scenario | `engine/web/js/bridge/scenarios/` | `index.js` (prefix dispatcher) |
| Modify scenario toggle defaults | `engine/web/js/config/toggles.js` | `SCALE0_TOGGLES`, `SCALE0_SCENARIO_OVERRIDES` |
| Change a physics constant | `engine/web/js/constants.js` (JS) + `engine/include/ftd/ontic.h` (C++) | layered by category |
| Add a new toggle | `engine/include/ftd/term_toggles.h` (C++) + `engine/web/js/config/toggles.js` (JS) + `engine/wasm/bindings_render_bridge.cpp` (binding) | (Phase 6 will make this 1-place edit) |
| Change MockBridge physics (JS) | `engine/web/js/wasm-bridge-dag.js` `MockBridge` class (lines 62–1568); helpers in `engine/web/js/bridge/mock-*.js` | `_tickFlux`, `_computePairwiseForces` |
| Change WasmBridge bindings | `engine/wasm/ftd_wasm.cpp`, `engine/wasm/bindings_*.cpp` | `EMSCRIPTEN_BINDINGS` block |
| Change C++ engine physics | `engine/src/render_bridge.cpp` phase methods | `phase_read`, `phase_write`, `phase_forces`, `phase_movement` |
| Change CUDA kernels | `engine/cuda/kernels_stencil.cu`, `kernels_forces.cu`, `kernels_poisson.cu` | `phase_*_kernel` |
| Add a new dashboard view (3D rendering) | `engine/web/js/viewport.js` + `engine/web/js/viewport/*.js` | `Viewport` class methods |
| Add a new dashboard panel | `engine/web/js/scales/scale0/ui/` (Scale 0 panels) | `bindings.js`, `controls/`, `overlays/` |
| Change scenario load flow | `engine/web/js/scales/scale0/runtime/scenario-loader.js` | `loadScale0Scenario`, `applyToggleDefaults` |
| Change diagnostics panel rows | `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js` | `sections[]` |
| Add a C++ unit test | `engine/tests/test_*.cpp` + register in `engine/CMakeLists.txt` (`ftd_add_test` macro) | template: `engine/tests/test_audit_regression.cpp` |
| Add a JS Playwright test | `engine/web/tests/*.spec.js` | template: `engine/web/tests/audit-regression.spec.js` |
| Add a Python verification script | `scripts/verification/` | template: `scripts/verification/verify_*.py` |
| Add a formal proof | `scripts/proofs/proof_*.py` | run via `python -m scripts.proofs.<name>` |
| Find a physics derivation | `docs/theory/03_derivations/` (DERIV_*.md) | `docs/theory/META_INDEX.md` catalog |
| Find a foundational document | `docs/theory/02_foundations/` (FOUND_*.md) | same catalog |
| Find an audit / assessment | `docs/theory/07_assessment/` (AUDIT_*.md) | same catalog |
| Find a load-bearing claim | `docs/theory/07_assessment/LEDGER.md` | single source of truth for claim status |
| Find an architectural decision | `docs/adr/` | `docs/adr/INDEX.md` |
| Find an audit ledger | `docs/audits/` | `docs/audits/INDEX.md` |
| Find a contract / interface spec | `CONTRACTS.md` (root) | section per contract |
| Run all tests | `engine/build && ctest` (C++) + `pytest scripts/tests/` (Python) + Playwright (JS) | see `engine/tests/README.md` |
| Build C++ engine | `cmake -S engine -B engine/build && cmake --build engine/build --config Release` | |
| Build WASM | `engine/build_wasm.bat` (Windows) or `emcmake cmake -S engine -B engine/build_wasm` | |
| Run web dashboard | `python engine/web/serve.py 8080` | no-cache dev server |
| Run GPU campaign | WSL2 only: `wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && engine/build_wsl/<test>"` | per CLAUDE.md mandate |
| Find session-resume context | `docs/WHERE_WE_LEFT_OFF.md` | always start here when resuming |
| Find project epistemic state | `CLAUDE.md` §"Current epistemic state" | versioned per session |
| Read engine architecture | `engine/SPEC_ENGINE.md` | living doc |
| Read FTD theory spec | `docs/SPEC_FTD.md` | authoritative |

---

## §2 — Annotated directory tree

Top-level layout. Each directory's `README.md` (where present) is the local entry point.

```
ftd/
├── CLAUDE.md                          # Project instructions for AI agents (versioned)
├── AGENTS.md                          # Sibling synced with CLAUDE.md
├── META_PROJECT_ATLAS.md              # ← you are here
├── META_DOCUMENTATION_MAP.md          # Card catalog with reading paths by audience
├── CONTRACTS.md                       # Formal interface specs (bridge state, capabilities, etc.)
├── MAINTAINABILITY.md                 # Hazards, recipes, tech-debt ledger
├── CHANGELOG.md
├── README.md
├── docs/
│   ├── SPEC_FTD.md                    # Authoritative FTD theory spec
│   ├── WHERE_WE_LEFT_OFF.md           # Session-resume context (read first when resuming)
│   ├── adr/                           # Architecture Decision Records
│   │   ├── INDEX.md                   # ADR catalog
│   │   └── 0001…0009-*.md             # Per-decision records
│   ├── audits/                        # Archived audit ledgers (per sweep)
│   │   ├── INDEX.md
│   │   └── AUDIT_2026-04_pre-refactor.md   # Phase 0 archived from AUDIT_LEDGER.md
│   ├── theory/                        # 115+ theory documents
│   │   ├── META_INDEX.md              # Catalog
│   │   ├── 01_reference/              # SPEC_ALGEBRAIC_SPINE, SPEC_FTD_COMPLETE_CHAIN
│   │   ├── 02_foundations/            # FOUND_*.md
│   │   ├── 03_derivations/            # DERIV_*.md
│   │   ├── 04_coupling/               # alpha, alpha_s, sin²θ_W, ...
│   │   ├── 05_particles/              # masses, mixing, generations
│   │   ├── 06_consciousness/          # Scale 11 / observer formalism
│   │   ├── 07_assessment/             # AUDIT_*.md, LEDGER.md, TRACKER_OPEN_ITEMS.md
│   │   ├── 08_structural/             # geometry, information theory
│   │   ├── 09_mathematical/           # number theory, Chowla-Selberg
│   │   ├── 10_eft_program/            # EFT recovery program
│   │   └── archive/                   # superseded
│   ├── papers/                        # PDFs + TeX sources
│   ├── reference/                     # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY
│   └── internal/                      # Local-only session summaries (gitignored)
├── engine/
│   ├── SPEC_ENGINE.md                 # Engine architecture spec
│   ├── CMakeLists.txt
│   ├── README.md                      # Build / test / run instructions
│   ├── include/ftd/                   # ~30 headers
│   │   ├── ontic/                     # 9-layer derivation chain (README.md inside)
│   │   ├── eft/                       # EFT recovery program headers
│   │   ├── render_bridge.h            # Main API + 4 diagnostic structs (Phase 1 will split)
│   │   ├── particle_engine.h, atom_engine.h, cosmic_engine.h
│   │   ├── term_toggles.h             # 20 runtime toggles + validate (Phase 6 → table-driven)
│   │   ├── constants.h                # Layer-organized canonical constants
│   │   ├── constants_gpu.cuh          # Device-side mirror
│   │   ├── voxel.h, lattice.h, vec3.h # Core types
│   │   ├── test_telemetry.h           # NDJSON test API (Phase 7 → split impl)
│   │   └── ...
│   ├── src/                           # ~40 .cpp files
│   │   ├── render_bridge.cpp          # 6-phase tick (Phase 4 will decompose)
│   │   ├── poisson_solvers.cpp        # R1 — extracted SOR Poisson chain
│   │   ├── transmutation_phases.cpp   # R2 — weak/pair/triad/proper-time
│   │   ├── energy_ledger_compute.cpp  # R3 — conservation bookkeeping
│   │   ├── diagnostics_compute.cpp    # R4 — diagnostics() + energy_audit()
│   │   ├── injection.cpp              # R5 — inject_flux/particle/wavepacket
│   │   ├── particle_engine.cpp, atom_engine.cpp, cosmic_engine.cpp
│   │   ├── scenarios/                 # 89 C++ scenarios (flux, light, quantum, s0_seed, s0_field)
│   │   ├── atom/                      # AE force decomposition
│   │   ├── cosmic/                    # CE phase decomposition
│   │   ├── constructors/              # Shared lattice builders
│   │   └── eft/                       # EFT recovery TUs
│   ├── cuda/                          # ~10 CUDA TUs (README.md inside)
│   │   ├── kernels_stencil.cu         # 14 kernels (Phase 5 will split)
│   │   ├── kernels_forces.cu, kernels_poisson.cu
│   │   ├── cuda_index.cuh             # Shared idx3d / wrap / decode_xyz / periodic_delta
│   │   ├── gpu_engine.cu, gpu_buffers.cu
│   │   └── atom_engine_gpu.cu, particle_engine_gpu.cu
│   ├── wasm/                          # Emscripten bindings
│   │   ├── ftd_wasm.cpp               # Main embind file
│   │   ├── bindings_render_bridge.cpp # RenderBridge methods + toggle map
│   │   ├── bindings_particle.cpp, bindings_atom.cpp
│   │   └── bindings_internal.h
│   ├── tests/                         # 250+ test files (README.md inside)
│   │   ├── test_*.cpp                 # Unit tests (use test_telemetry.h)
│   │   ├── benchmark_*.cpp            # Engine-theory bridge tests
│   │   ├── campaign_*.cpp             # Long-running measurement campaigns
│   │   └── test_audit_regression.cpp  # Regression suite for recent audit fixes
│   └── web/                           # Browser dashboard
│       ├── serve.py                   # No-cache dev server
│       ├── index_dag.html             # Main entry
│       ├── docs/                      # USER_GUIDE.md
│       ├── tests/                     # Playwright .spec.js
│       └── js/                        # ~250+ modules
│           ├── constants.js           # JS canonical constants (mirror of ontic.h)
│           ├── viewport.js            # 3D renderer (Phase 3 will split)
│           ├── wasm-bridge-dag.js     # MockBridge + WasmBridge (Phase 2 will split)
│           ├── app_dag.js             # Main entry / orchestrator
│           ├── config/toggles.js      # SCALE0_TOGGLES + scenario overrides
│           ├── bridge/                # Bridge layer (README.md inside)
│           │   ├── mock-diagnostics.js       # Live-reference factory exemplar
│           │   ├── mock-particle-engine.js, mock-lattice-samplers.js
│           │   ├── mock-atom-engine.js, mock-scale4.js, mock-scale5.js
│           │   ├── boundary.js
│           │   └── scenarios/         # 84 JS scenarios (README.md inside)
│           ├── scales/scale0/         # Scale-0 controller (README.md inside)
│           │   ├── controller.js
│           │   ├── runtime/           # tick, frame-sync, scenario-loader, diagnostics
│           │   ├── ui/                # bindings, controls, overlays, panels
│           │   └── state/             # store
│           ├── scales/scale1/...scale11/
│           ├── viewport/              # Sub-renderers (extracted from viewport.js)
│           │   ├── molecular-renderer.js, boundary-geometry.js
│           │   ├── topology-sheet-renderer.js, color-ramps.js
│           └── ui/                    # Shared UI components
└── scripts/                           # Python tooling (README.md inside)
    ├── constants.py                   # Canonical Python constants (root of derivation chain)
    ├── verification/                  # Formal derivation verification
    ├── proofs/                        # Mathematical proofs with error bounds
    ├── experiments/                   # Bell tests, CERN analysis
    ├── exploration/                   # Research investigations (50-test physics battery)
    ├── tests/                         # pytest suites
    ├── visualization/                 # Publication figure generation
    └── runners/                       # Test protocol runners
```

---

## §3 — Subsystem dependency graph

```mermaid
graph TD
    Theory["docs/theory/<br/>SPEC_FTD.md, LEDGER.md"]
    PyConst["scripts/constants.py<br/>(canonical Python)"]
    Ontic["engine/include/ftd/ontic/<br/>(canonical C++)"]
    JSConst["engine/web/js/constants.js<br/>(canonical JS)"]
    GpuConst["engine/include/ftd/constants_gpu.cuh<br/>(device mirror)"]

    Theory -.cites.-> Ontic
    PyConst -.values.-> Ontic
    Ontic --> JSConst
    Ontic --> GpuConst

    RB["engine/src/render_bridge.cpp<br/>(6-phase C++ tick)"]
    RBPhases["src/render_bridge_phases/<br/>(R1-R5 extracted)"]
    Cuda["engine/cuda/*.cu<br/>(GPU kernels)"]
    Wasm["engine/wasm/ftd_wasm.cpp<br/>(embind)"]

    Ontic --> RB
    GpuConst --> Cuda
    RB --> RBPhases
    RB <-.parity.-> Cuda
    RB --> Wasm

    MockBridge["wasm-bridge-dag.js<br/>MockBridge"]
    WasmBridge["wasm-bridge-dag.js<br/>WasmBridge"]
    BridgeHelpers["bridge/mock-*.js<br/>(live-ref factories)"]
    Capabilities["createScale0/1/2Capabilities<br/>(symmetric surface)"]

    JSConst --> MockBridge
    Wasm --> WasmBridge
    MockBridge --> BridgeHelpers
    MockBridge --> Capabilities
    WasmBridge --> Capabilities

    AppDag["app_dag.js<br/>(main entry)"]
    Scale0["scales/scale0/controller.js"]
    OtherScales["scales/scale1..11"]
    Viewport["viewport.js"]

    AppDag --> Scale0
    AppDag --> OtherScales
    AppDag --> Viewport
    Scale0 --> Capabilities
    OtherScales --> Capabilities

    Scenarios["bridge/scenarios/<br/>(84 JS, prefix-dispatched)"]
    SceneCpp["src/scenarios/<br/>(89 C++, mirrors JS)"]

    MockBridge --> Scenarios
    RB --> SceneCpp
    Scenarios <-.mirror.-> SceneCpp

    Tests["engine/tests + engine/web/tests"]
    AuditLedger["docs/audits/INDEX.md"]
    ADR["docs/adr/INDEX.md"]
    Contracts["CONTRACTS.md"]

    RB -.tested.-> Tests
    MockBridge -.tested.-> Tests
    Tests --> AuditLedger
    AuditLedger -.feeds.-> ADR
    ADR -.codifies.-> Contracts
    Contracts -.governs.-> BridgeHelpers
```

---

## §4 — Per-file header convention

**Every file modified in the refactor sweep must carry this block at top of file:**

```js
/**
 * @file <path/relative/to/repo/root>
 * @purpose <one sentence — what does this file own?>
 * @consumers <key files that import/include this>
 * @contract <CONTRACTS.md#section-name if applicable>
 * @related <paired files: CPU↔GPU, JS↔C++ mirrors>
 * @epistemic [THEOREM]/[CONJECTURE]/[IMPOSED] <if physics-bearing>
 */
```

**STATE CONTRACT block (required for any module touching live-ref state):**

```js
/* ============ STATE CONTRACT ============
 * Reads:    state.<field>      // why it needs this
 * Writes:   state.<field>      // who else reads it
 * Owns:     state.<field>      // sole writer
 * Derives:  state.<field>      // recomputed each call
 * Invariants:
 *   - <invariant>
 * ======================================== */
```

Reference exemplar: [`engine/web/js/bridge/mock-diagnostics.js`](engine/web/js/bridge/mock-diagnostics.js) lines 26–50.

Required for files modified in subsequent phases; soft-warn for legacy files.

---

## §5 — Cross-reference policy

Three rules. Every refactor PR must satisfy them:

1. **Physics-bearing code cites LEDGER.** Any function implementing a load-bearing claim writes `// Implements LEDGER#C-NNN [TAG]`. Validated by grep (every `[THEOREM]` tag in code resolves to a LEDGER row).
2. **JS↔C++ mirrors cross-link reciprocally.** If `wasm-bridge.js` mirrors a C++ class, both files carry `@related` headers pointing to each other. Validated by reciprocal-link CI check.
3. **Public-API consumers cite their contract.** Any file calling `createBridge`, a capability factory, or a STATE CONTRACT module carries a comment naming the relevant `CONTRACTS.md` section.

---

## §6 — AUDIT_LEDGER lifecycle

- **Active sweep**: a single `AUDIT_LEDGER.md` at root for the in-flight refactor.
- **On merge**: rename to `AUDIT_<YYYY-MM>_<slug>.md`, move under `docs/audits/`, append entry to `docs/audits/INDEX.md`.
- **Retention**: indefinite (audits are history with high LLM value).
- **Concurrent sweeps**: place under `docs/audits/active/<slug>/AUDIT.md`.
- **Quarterly roll-up**: INDEX.md gains a "patterns observed" section.

---

## §7 — Refactor companion workflow

Every refactor session produces a triad:

1. **Open**: author `SPEC_REFACTOR_<name>.md` (plan, scope, success criteria) + create `AUDIT_<name>.md` skeleton.
2. **During**: AUDIT live-tracks findings with `[x]/[~]/[d]/[n]` legend.
3. **Close**: SPEC marked `Status: Implemented`; AUDIT moved to `docs/audits/`; META_PROJECT_ATLAS.md updated; emit ADR(s) for any pattern decisions; affected READMEs updated.

PR-merge gate: SPEC.Status = Implemented AND ATLAS diff exists.

---

## §8 — Anti-drift CI checks (target)

- **Pre-commit hook**: file-header `@purpose` change → directory README must be in same diff (warn → block).
- **Per-PR CI**:
  - All `@related` links resolve reciprocally
  - All `LEDGER#C-NNN` cites resolve to existing rows
  - Every directory with >3 source files has README.md
  - SPEC_REFACTOR_*.md `Status` field non-empty
- **Per-PR linter**: new files under `engine/web/js/` or `engine/include/ftd/` lacking the structured header → warn.
- **Quarterly freshness audit**: `git log` last-modified per dir vs README mtime; older READMEs flagged in `docs/audits/freshness-<date>.md`.
- **PR template**: checkbox "ATLAS / READMEs / ADRs updated as needed."

---

## §9 — Deferred / WSL2-only operations

Per CLAUDE.md, GPU campaigns route through WSL2:

```bash
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    engine/build_wsl/<binary> --args"
```

Windows-native CUDA build (`engine/build/`) is acceptable for compile-time
checks and single-tick correctness only. Any measurement campaign, sweep,
or multi-seed run goes through WSL2.

---

## §10 — Quick command reference

| Task | Command |
|------|---------|
| Run all Python tests | `python scripts/tests/run_all_tests.py` |
| Run 7-tier verification | `python scripts/tests/comprehensive/run_ultimate_test.py` |
| Run proof chain | `python scripts/proofs/proof_master_verification.py` |
| Build C++ engine | `cmake -S engine -B engine/build && cmake --build engine/build --config Release` |
| Run C++ tests | `cd engine/build && ctest --output-on-failure -C Release` |
| Run audit regression test | `engine/build/Release/test_audit_regression.exe` |
| Build whitepaper PDF | `cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex` |
| Build manuscript | `cd dissemination/manuscript && quarto render` |
| Launch web dashboard | `python engine/web/serve.py 8080` |
| View theory docs | `docs/theory/META_INDEX.md` |

---

**Cross-references:**
- [CLAUDE.md](CLAUDE.md) — current epistemic state, naming conventions, build/test instructions
- [CONTRACTS.md](CONTRACTS.md) — formal interface specs
- [docs/adr/INDEX.md](docs/adr/INDEX.md) — architectural decisions
- [docs/audits/INDEX.md](docs/audits/INDEX.md) — historical audit ledgers
- [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) — theory document catalog
- [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) — engine architecture
- [docs/SPEC_FTD.md](docs/SPEC_FTD.md) — authoritative FTD theory spec
- [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) — audience-specific reading paths
- [MAINTAINABILITY.md](MAINTAINABILITY.md) — hazards, recipes, tech-debt ledger

---

*Project version: FTD v5.33 | Engine version: v2.15 | Atlas refreshed 2026-04-27*
