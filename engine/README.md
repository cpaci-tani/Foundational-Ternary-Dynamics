# FTD Simulation Engine

The FTD engine is a multi-scale, logic-first discrete simulation system for Foundational Ternary Dynamics. Its fundamental Scale 0 model is a 3D cubic lattice. The engine provides a hierarchy of integrated macroscopic physics engines (Particles, Atoms, Molecules, Cosmic) derived from this fundamental substrate.

**Current Engine Version:** 2.18.0

Detailed references:
- [SPEC_ENGINE.md](SPEC_ENGINE.md) - complete living engine reference.
- [ARCHITECTURE.md](ARCHITECTURE.md) - software call stacks, memory ownership, loop dynamics, manifestation lifecycle.
- [PHYSICS_STATUS.md](PHYSICS_STATUS.md) - detailed force tables and toggle coverage across all scales.
- [CALLSTACKS.md](CALLSTACKS.md) - feature-by-feature runtime callstacks from entrypoint to phase/kernel.
- [SCENARIO_ARCHITECTURE.md](SCENARIO_ARCHITECTURE.md) - scenario lifecycle, bridge ownership, and cross-scale seed architecture.
- [VISUAL_GUIDE.md](VISUAL_GUIDE.md) - visual learner's guide to how the simulation works and why the discrete perspective matters.
- [docs/ENGINE_CODE_MAP.md](docs/ENGINE_CODE_MAP.md) - file/subsystem navigation map: what code lives where, largest files, split status, doc gaps.
- [docs/ENGINE_FILE_MANIFEST.md](docs/ENGINE_FILE_MANIFEST.md) - per-file catalog (every code file, one-line purpose); machine-readable mirror `ENGINE_FILE_MANIFEST.json`.
- [../docs/SPEC_FTD.md](../docs/SPEC_FTD.md) - project-level theory specification.
- [../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) - claim-status and epistemic accounting.

---

## Quick Start

### Build (Windows native, CPU + CUDA)
MUST use the MSVC 14.44 toolset — VS 18's default (14.51+) crashes CUDA 13.0's
`cudafe++`. The wrapper enters `vcvarsall.bat x64 -vcvars_ver=14.44` and drives
`engine/CMakePresets.json` (Ninja Multi-Config → `engine/build`, CUDA ON by default):
```bash
engine\build_native.bat
```

### Test CPU
```bash
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

### Build CUDA GPU
CUDA is already ON in the canonical `engine/build` tree (see above); the legacy
separate `engine/build_cuda` tree is retired. Measurement campaigns use the
documented WSL2 path (`engine/build_wsl`), not Windows-native CUDA.

### Build WASM Dashboard
```bash
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm
cp engine/build_wasm/wasm/ftd_core.{js,wasm} engine/web/wasm/
```

### Run
```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
python engine/web/serve.py 8080
```
Open `http://localhost:8080` for the dashboard.

---

## Multi-Scale Engine Architecture

All scale engines inherit from the `ScaleEngine` abstract base class, providing a unified `tick()`, `dt()`, diagnostics, and string-based toggle registry interface.

| Scale | Engine | Status | Notes |
|---|---|---|---|
| **0** | `RenderBridge` | Production | Flat 3D voxel lattice, CPU tick ladder, SOR constraint solvers |
| **0 (GPU)** | `GpuEngine` | Production | CUDA drop-in for RenderBridge; Device SoA mirror, lazy host/device synchronization |
| **1** | `ParticleEngine` | Production | Continuous particles, Velocity Verlet, analytical forces (Coulomb, Gravity, Exchange, Strong, Lorentz, etc.) |
| **2** | `AtomEngine` | Production | Composite atoms, ionic/vdW forces, covalent auto-bonding, VSEPR angle strain |
| **3** | `MoleculeEngine`| Production | Natively handled via `AtomEngine`; 25 molecular presets |
| **5** | `CosmicEngine` | Production | N-body + SPH cosmic layer, 9 body types, Barnes-Hut octree gravity |
| **DAG** | `DagEngine` | Experimental | Sparse-lattice prototype; do not use for physics claims |

---

## Scale 0: Lattice Field Theory

The fundamental Scale 0 substrate consists of a finite cubic lattice. Each site has a discrete ternary state and a continuous flux field:

| Layer | Type | Engine fields | Role |
|---|---|---|---|
| State | `{-1, 0, +1}` | `Voxel::state` | Void, negative manifestation, positive manifestation |
| Flux | `R^3` vector field | `Voxel::flux`, `Voxel::wave_vel` | Dispositional field that propagates, couples, and mediates forces |
| Motion | lattice kinematics | `velocity`, `remainder` | Manifested-particle motion through integer cells |
| Labels | discrete metadata | `particle_id`, `pair_id`, `spin`, `color`, `locked` | Identity, correlations, internal labels, bound-state locks |

### The Scale 0 Tick Cycle (Loop Dynamics)

The engine's dynamics are staged so local field updates can run in parallel, while collision mutation remains ordered. `RenderBridge::tick()` orchestrates the CPU ladder:

1. **Field read:** (`phase_read.cpp`) Read voxels, write delta buffers via 18-point Moore Laplacian, state-flux coupling, and curl source.
2. **Field write:** (`phase_write.cpp`) Mutate flux/state in parallel via staggered wave update, genesis, evaporation, and damping.
3. **Constraint:** (`poisson_solvers.cpp`) Enforce Gauss relation by solving Poisson and subtracting `grad(phi)`.
4. **Force:** (`phase_forces.cpp`) Field-mediated force integration (Coulomb, gravity, Lorentz, color/strong).
5. **Movement:** (`phase_movement.cpp`) Sequential integer moves, bounce, annihilation, self-field transfer.
6. **Transmutation:** (`transmutation_phases.cpp`) Toggle-gated mutations like pair production, weak flips, triad locks, proper time.

### Manifestation Lifecycle

Manifestation is the field-to-state transition. In `phase_write()`, a void site can become a manifested ternary state when local field density exceeds `K_GENESIS`:

- **Genesis:** Single-site stochastic manifestation from high flux.
- **Evaporation:** Low local 7-site energy clears unlocked manifested states.
- **Pair production:** High-flux void creates neighboring `-1/+1` pair.
- **Movement:** Velocity and remainder move manifested states across cells.
- **Bounce:** Same-sign collision reverses attempted motion.
- **Annihilation:** Opposite-sign collision clears both states and bursts field energy.
- **Weak transmutation:** Stress-threshold polarity flip when enabled.
- **Triad binding:** Compact same-sign triples become locked when enabled.

---

## CPU/GPU Parity & Memory Ownership

- **CPU (`RenderBridge`)**: Host Array-of-Structures (AoS) is authoritative.
- **CUDA (`GpuEngine`)**: Device Structure-of-Arrays (SoA) is authoritative between syncs.
- **WASM**: Host AoS inside Emscripten heap; JS reads typed views via Embind.

Host mutations are flushed to the GPU before a device tick; host reads download the device state lazily. Both paths preserve the same logical phase order and provide bit-exact parity for stochastic toggles via a shared `SplitMix64` RNG.

---

## Runtime Toggle Surface

The multi-scale engine uses a table-driven `TermToggles` registry (configured via `toggles.json`). Core rules default on; exploratory extensions are toggle-gated.

- **Scale 0 (33 toggles):** Wave propagation, coupling, damping, genesis, Gauss projection, gravity, Lorentz force, emergent forces, latency field, etc.
- **Scale 1 (12 toggles):** Coulomb, gravity, damping, exchange, strong, magnetic dipole, spin-orbit, radiation reaction, relativistic corrections.
- **Scale 2 (12 toggles):** Ionic, vdW, covalent bonds, auto-bonding, angle strain (VSEPR), H-bonds, thermostat.
- **Scale 5 (14 toggles):** Gravity, SPH gas, Hubble expansion, dark energy, star formation, stellar evolution.

---

## Constants And Claim Status

Engine constants come from a mix of framework constants, simulation parameters, and theory-side reference values exported through `ontic.h` and `constants.h`.

- `ALPHA` uses the tree-level master-quadratic root (`X_PLUS_PRECISION = 137.035999177`) by default.
- `N_C = 3` is an explicit framework/topological integer used by color paths.
- `G_N = 0.01` is a lattice-scaled simulation gravity parameter, not the physical gravitational coupling.
- Precision Standard Model computations live in scripts and theory docs, not in the Scale 0 runtime kernels.

For epistemic tags and derivation/parameter distinctions, see the project assessment docs.

---

## Generated Outputs

Build trees, CTest scratch data, Playwright artifacts, and campaign outputs are not part of the source surface. Campaign outputs should land under `engine/results/` from the repository root; avoid running commands from `engine/` with an `engine/results/...` output path, which creates accidental nested paths like `engine/engine/results/...`.

The result tracking policy is documented in [results/README.md](results/README.md). Do not delete tracked campaign provenance during cleanup unless the project owner explicitly approves it.

---

## Known Limits

- The lattice defines a preferred discrete frame; continuum symmetries are approximate and scale-dependent in the engine.
- Gauss projection is non-variational and can move field energy; conservation accounting tracks it separately.
- Macroscopic forces (Gravity, Coulomb) use Barnes-Hut octrees in macro engines to achieve O(N log N) computation scaling.
- The engine is a computational ontology and simulation instrument, not a claim that FTD is experimentally confirmed.

---

## Directory Pointers

```text
engine/
  CMakeLists.txt                Build system and test registration
  include/ftd/                  Public C++ headers (scale_engine.h, ontic.h, etc.)
  src/                          Core C++ implementations (render_bridge.cpp, macro engines)
  src/render_bridge_phases/     Decomposed Scale 0 logic phases
  cuda/                         CUDA backend and SoA kernels
  wasm/                         Emscripten bindings via Embind
  web/                          Browser dashboard (HTML/JS/CSS)
  config/                       Data-driven configurations (toggles.json, scenario manifests)
  tests/                        C++ test suites, benchmarks, and campaigns
```
