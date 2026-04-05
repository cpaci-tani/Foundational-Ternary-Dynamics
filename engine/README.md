# FTD Simulation Engine

A discrete physics engine implementing Foundational Ternary Dynamics: a 3D cubic lattice where each site carries a ternary state {-1, 0, +1} and a continuous flux vector, evolving under six local rules derived from an action principle. All physical constants cascade from two inputs (D=3 and the lemniscate constant) through the ontic derivation chain.

---

## Quick Start

### Build (CPU)

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
```

### Build (CUDA GPU — 363x speedup)

```bash
# Requires CUDA 13.0 + MSVC + Ninja. Set MSVC env vars first.
cmake -S engine -B engine/build_cuda -DFTD_ENABLE_CUDA=ON -G Ninja \
      -DCMAKE_CUDA_FLAGS="--allow-unsupported-compiler"
cmake --build engine/build_cuda --config Release
```

### Build (WASM — browser dashboard)

```bash
# Requires Emscripten SDK
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm
cp engine/build_wasm/wasm/ftd_core.{js,wasm} engine/web/wasm/
```

### Test

```bash
cd engine/build && ctest --output-on-failure -C Release
```

### Run (CLI)

```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
# Scenarios: A (single particle), B (collision pair), D (dense genesis), ...
```

### Run (Web Dashboard)

```bash
python -m http.server 8080 -d engine/web
# Open http://localhost:8080
```

---

## Architecture Overview

### Two-Layer Ontology

Every lattice site carries two coupled layers:

| Layer | Type | Role |
|-------|------|------|
| **State** | Discrete: {-1, 0, +1} | Manifested particles (matter/antimatter) or void |
| **Flux** | Continuous: (Jx, Jy, Jz) | Energy-momentum field, carries forces, mediates interactions |

### Six Rules (nothing else)

1. **Flux wave equation**: dJ/dt = c^2 nabla^2 J (local linear dynamics for a vector field)
2. **State-flux coupling**: source term g_c * grad(s) + g_c * curl(s*v) (from action principle)
3. **Gauss projection**: enforce div(J) = s each tick (charge conservation)
4. **Manifestation/Evaporation**: |J| > K_GENESIS -> manifest; neighborhood energy < threshold -> evaporate
5. **Field-mediated forces**: F = -alpha*s*grad(phi_C) + G_N*grad(rho) + alpha*s*(v x B)
6. **Movement + Collision**: remainder accumulation, speed limit C = 1/sqrt(3), annihilation on contact

### Three Scales

| Scale | Engine | Description |
|-------|--------|-------------|
| 0 | `RenderBridge` | Voxel-level field dynamics on a cubic lattice |
| 1 | `ParticleEngine` | Continuous-position particles with Velocity Verlet integration |
| 2 | `AtomEngine` | Composite atoms with ionic, van der Waals, and covalent bond forces |
| 3 | `AtomEngine` (bonding) | Molecules — pre-bonded multi-atom structures with 1-3 exclusion |

### GPU Acceleration

`GpuEngine` is a drop-in alternative to `RenderBridge`. All field data lives on the GPU; the host transfers only diagnostics. FFT Poisson solver replaces iterative SOR. Benchmarks: 363x speedup at 64^3 lattice size.

---

## The Tick Cycle

```text
tick() {
  1.  phase_read()          Wave equation + state-flux coupling
  2.  phase_write()         Leapfrog integration, damping, genesis/evaporation
  3.  gauss_project()       SOR Poisson solver: enforce div(J) = s
  4.  phase_forces()        F_EM + F_Lorentz + F_grav (field-mediated)
  5.  phase_movement()      Velocity integration, collisions, annihilation
  6.  ++tick_
}
```

Every phase is gated by a runtime toggle (`TermToggles` struct, 20 booleans — 10 core ON, 10 extensions).

---

## Constants from First Principles

All physical constants derive from two inputs via the ontic derivation chain (`ontic.h`, 9 layers):

- **D = 3** (spatial dimensions)
- **varpi** (lemniscate constant from elliptic geometry)

The master quadratic x^2 - 16*G*^2*x + 16*G*^3 = 0 yields:
- x+ = 137.036 (1/alpha, fine structure constant)
- x- = 3.024 (N_c, color charges)

All other constants — coupling strengths, mass scales, mixing angles — cascade deterministically.

---

## Force Pipeline

Forces are **field-mediated** (O(N) per tick), not pairwise:

| Force | Formula | Physics |
|-------|---------|---------|
| **Coulomb** | F = -alpha * s * grad(phi_C) | Poisson solver: nabla^2 phi = -s |
| **Lorentz** | F = alpha * s * (v x B), B = curl(J) | Magnetic deflection of moving charges |
| **Gravity** | F = G_N * grad(rho) | Attraction toward high-density regions (tier-2 stencil) |

---

## Directory Layout

```text
engine/
  CMakeLists.txt              # Build system — all targets and test registration
  SPEC_ENGINE.md              # Comprehensive living reference document
  README.md                   # This file
  include/ftd/
    ontic.h                   # Ontic derivation chain (9 layers), D=3 + varpi -> all constants (858L)
    constants.h               # Re-exports ontic constants, adds engine-specific values (186L)
    voxel.h                   # Vec3, Voxel struct (state, flux, velocity, spin, color) (156L)
    lattice.h                 # Lattice class — 3D cubic grid with periodic boundaries (59L)
    render_bridge.h           # RenderBridge — main engine API, tick(), diagnostics() (196L)
    lagrangian.h              # 4-term Lagrangian + Rayleigh dissipation (137L)
    term_toggles.h            # 11 runtime physics toggles (35L)
    csv_export.h              # Header-only CSV export utility (385L)
    particle_engine.h         # ParticleEngine — Scale 1 continuous-position particles (108L)
    atom_engine.h             # AtomEngine — Scale 2 composite atoms + bonds (215L)
    scale.h                   # OnticEntity + scale bridge declarations (68L)
    gpu_engine.h              # GpuEngine — CUDA GPU drop-in for RenderBridge (101L)
    gpu_buffers.h             # SoA device memory layout (94L)
  src/
    render_bridge.cpp         # Core engine — 6-phase tick cycle (~989L)
    lattice.cpp               # Index/coordinate conversion, wrapping, neighbors (65L)
    lagrangian.cpp            # Lagrangian diagnostics computation (56L)
    main.cpp                  # CLI entry point (scenarios A-K) (937L)
    particle_engine.cpp       # ParticleEngine: Velocity Verlet + analytical forces (234L)
    atom_engine.cpp           # AtomEngine: ionic + vdW + covalent forces (427L)
    scale_bridge.cpp          # Scale 0<->1<->2 coarsen/refine round-trip (202L)
  cuda/
    gpu_buffers.cu            # SoA device allocation, upload, download (318L)
    gpu_engine.cu             # GpuEngine tick loop, host<->device sync (315L)
    kernels_stencil.cu        # GPU phase_read + phase_write kernels (402L)
    kernels_poisson.cu        # FFT Poisson solver (cuFFT spectral) (253L)
    kernels_forces.cu         # GPU forces + movement kernels (339L)
    CMakeLists.txt            # CUDA build rules (35L)
  wasm/
    ftd_wasm.cpp              # Emscripten Embind bindings (774L)
    CMakeLists.txt            # WASM build rules
  web/
    index.html                # Browser dashboard (HTML + CSS + tab system) (1204L)
    js/
      app.js                  # Main controller: WASM loading, frame loop, 3 scale modes (1727L)
      viewport.js             # Three.js 3D: particles, bonds, orbitals, field overlays (795L)
      wasm-bridge.js          # WasmBridge + MockBridge (auto-fallback, identical API) (1389L)
      charts.js               # Ring-buffered time-series charts (202L)
      diagnostics.js          # Live number displays with sparkline mini-charts (179L)
      lagrangian.js           # Stacked area chart (5 terms) + constraint display (226L)
      inspector.js            # Click-to-inspect voxel properties + force decomposition (648L)
      constants.js            # JS mirror of ontic.h derivation chain (130L)
      particle-catalog.js     # Complete SM particle data with FTD mass formulas (613L)
      zoo.js                  # Interactive particle zoo table (123L)
      fields.js               # Force field visualization (heatmap + arrows) (183L)
      elements.js             # Periodic table data (118 elements with CPK colors) (175L)
      atomic-energy.js        # Bethe-Weizsacker nuclear binding energies (179L)
      spectroscopy.js         # Hydrogen energy levels and spectral series (148L)
      cross-sections.js       # Scattering cross-sections from ontic chain (215L)
      decay-rates.js          # Particle lifetimes from Fermi theory + FTD constants (253L)
      ontic-observatory.js    # Ontic incompleteness theorems (324L)
      aggregation-bridge.js   # 4-level aggregation hierarchy + emergence monitoring (388L)
      orbitals.js             # Electron orbital cloud generation + nuclear structure (443L)
      molecules.js            # 25-molecule library for Scale 2 (584L)
    wasm/
      ftd_core.js             # Emscripten JS loader (generated)
      ftd_core.wasm           # WebAssembly binary (generated)
  tests/                      # 175+ test files (unit, campaign, GPU parity)
  thirdparty/glad/            # OpenGL loader (legacy)
```

### Archived Components

```text
archive/engine_v1_phenomenological/   # Original ~1382-line phenomenological engine
archive/qt_gui/                       # Qt6 native GUI (replaced by web dashboard)
```

---

## What Emerges

The following behaviors arise from the six update rules without being explicitly programmed:

| Emergent Feature | How It Arises |
|-----------------|---------------|
| **Bound structures** | Geometry + flux gradients stabilize multi-particle configurations |
| **Wave interference** | Vector addition of flux produces constructive/destructive patterns |
| **U(1) gauge symmetry** | Gauss constraint removes longitudinal modes, leaving 2 transverse polarizations |
| **Maxwell electrodynamics** | Wave equation + Gauss constraint recovers Faraday's law, E perp B perp k |
| **Spin-statistics** | Spin from curl(J), Pauli-like exclusion from exchange |
| **Pair production** | High flux density triggers stochastic genesis of +1/-1 pairs |

These are verified by the test suite (175+ tests, GPU parity 21/21, Five Minds campaigns 15/15).

---

## Verified Results (from engine dynamics)

| Observable | Method | Result |
|-----------|--------|--------|
| **Coulomb force exponent** | Power-law fit at L=48 | -2.34 (converging to -2.0) |
| **Octahedral symmetry** | 6-axis field comparison | max/min = 1.000005 |
| **Bell anti-correlation** | EPR pair flux measurement | E = -1.000000 |
| **CPT invariance** | Charge-swapped scattering | energy difference = 0.000000 |
| **Annihilation** | Opposite-sign collision | charge conserved exactly |
| **Alpha from scattering** | Rutherford deflection at b=4 | alpha = 0.027 (right order) |
| **Energy conservation** | 3-particle system, 500 ticks | charge conserved, energy tracks damping |

## Computed Standard Model (46 observables, zero free parameters)

See `scripts/proofs/proof_complete_sm.py` for the complete computation. Key results:

| Observable | FTD Value | Experiment | Error |
|-----------|-----------|------------|-------|
| 1/alpha (7-term) | 137.035999177 | 137.035999177 | 0.00 ppb |
| a_e (5-loop) | 0.00115965218 | 0.00115965218 | 2.55 ppb |
| m_tau/m_e | 3477 | 3477.48 | 0.014% |
| m_p/m_e | 1836.47 | 1836.15 | 174 ppm |
| Lamb shift | 1055.4 MHz | 1057.8 MHz | 0.23% |
| Proton lifetime | Infinite | > 10^34 yr | [THEOREM] |

---

## Test Suite

107 test files: 67 unit + 40 campaigns + 3 GPU (conditional on `FTD_ENABLE_CUDA`). **10-Phase Proof-Out: 125+ checks, all PASS.**

| Category | Count | What They Verify |
|----------|-------|-----------------|
| Core infrastructure | 3 | Constants, Lorentz factor, lattice wrapping |
| Lagrangian verification | 6 | Energy, Gauss constraint, stress-energy, thermodynamics |
| Ontic physics | 5 | Genesis, gravity, annihilation, wave collapse |
| Wave and field | 8 | Wave speed, interference, gauge, polarization, momentum, magnetic, entanglement |
| Lagrangian forces | 6 | Variational derivation of each force from Euler-Lagrange equations |
| Phase 3-5 | 8 | Poisson Coulomb, energy conservation, free dynamics |
| Phase 6 (aggregates) | 3 | Self-field profile, wavepackets, two-body interactions |
| Phase 7 (multi-scale) | 5 | ParticleEngine, scale bridge, hydrogen, cross-scale, Born ensemble |
| Phase 8 (AtomEngine) | 3 | Atom forces, scale bridge, H₂ molecule |
| FDTD-aligned EM | 4 | E/B fields, Gauss convergence, Lorentz force, selective damping |
| Campaigns (pre-existing) | 15 | Dispersion, gauge, force law, energy, binding, structure |
| 10-Phase Proof-Out | 22 | Statistical convergence, continuum limits, Bell test, mass spectrum, color, weak, gravity, particle zoo, cosmology, novel predictions |
| GPU (CUDA) | 3 | Parity, benchmark, physics campaigns (Coulomb, Gauss, wave, energy, gravity, annihilation) |

```bash
cd engine/build && ctest --output-on-failure -C Release
```

---

## Known Limitations

### Discreteness Artifacts

- **Angular momentum is not conserved.** Integer position jumps at small orbit radii cause perturbations.
- **Kepler orbits do not close.** With alpha = 0.00729, orbital dynamics are discreteness-dominated at lattice scale.
- **Rotation symmetry is broken** at the lattice scale. Isotropy emerges at scales >> lattice spacing.
- **Lorentz invariance is approximate.** The lattice defines a preferred frame.

### What the Engine Does NOT Do

- It does not solve the Standard Model equations numerically
- It does not reproduce quantized energy levels from first principles
- It does not produce smooth continuous-space orbits
- It does not claim to be a confirmed physical theory

The engine is a **computational ontology**: minimal rules that produce physics-like behavior as emergent phenomena.

---

## Further Reading

- [SPEC_ENGINE.md](SPEC_ENGINE.md) — Comprehensive technical reference (tick phases, constants, forces, test catalog, all phase characterizations)
- [CLAUDE.md](../CLAUDE.md) — Framework specification (ontological postulates, interpretive mappings, theoretical foundations)
- [docs/theory/SPEC_FTD_REFERENCE.md](../docs/theory/SPEC_FTD_REFERENCE.md) — Complete theoretical reference
