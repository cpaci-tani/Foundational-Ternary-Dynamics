# FTD Software Architecture

This document provides a technical overview of the software engineering patterns, execution call stacks, memory layouts, and API integrations within the FTD Simulation Engine. It focuses exclusively on the software architecture ("how it works") outside the scope of theoretical physics models.

## 1. The Core Tick Call Stack

The simulation execution loop is driven primarily within the Scale 0 `RenderBridge` implementation, which serves as the substrate layer for all localized lattice operations. The execution is broken into strict sequential phases, each guarding memory mutations tightly to prevent race conditions during parallel processing.

### Execution Path
When `RenderBridge::tick()` is invoked by a client (either `main.cpp` using the CLI runner or `app.js` using browser interval triggers), the engine proceeds down the following execution stack:

```
RenderBridge::tick()
├─ phase_read()        -> 18-point Moore Laplacian + coupling source. Reads voxel_[i].flux, writes delta_j_[i].
├─ phase_write()       -> Advance (forward-Euler-like) + damping + genesis/evaporation. Writes voxel state.
├─ gauss_project()     -> Gauss constraint normalization via SOR. 18-point stencil, red/black parallelism.
├─ phase_forces()      -> Solves discrete gradients across memory spaces to accumulate acceleration inputs.
├─ phase_movement()    -> Updates particle index grids. Collision handling executed sequentially via guards.
├─ [optional] weak_transmutation / latency_field / proper_time  -> gated by toggles, for GR + weak sector
└─ tick_++ + update_energy_ledger()  -> per-tick conservation drift snapshot (CPU path auto-populates)
```

The advance pair in `phase_write` is mathematically forward Euler, not symplectic leapfrog — `DAMPING = α` masks Euler drift for typical benchmarks. See [`SPEC_ENGINE.md`](SPEC_ENGINE.md) §4 and [`TRACKER_OPEN_ITEMS.md`](../docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) §1.4 for the leapfrog upgrade plan.

### Mutability Guardians
During execution, the `Voxel` object space utilizes intermediate memory buffers (e.g., `wave_vel`) so that a "Read" phase evaluates an entirely isolated snapshot of the last tick, outputting purely to "Next State" accumulators. This double-buffering protects the spatial differentiation kernels from propagating immediate neighboring side-effects within the identical loop pass.

## 2. Class Hierarchy and Scaling Framework

The simulation requires the ability to switch dynamically between localized differential processing (Scale 0) and analytical macro-object tracking (Scale 1, 2, 5). This transition is governed by strict class hierarchies extending `ScaleEngine`.

### Inheritance Tree
*   **ScaleEngine**: The abstract base class. It enforces uniform interfaces across all computational scales, exposing virtual constraints for `tick()`, `dt()`, `set_dt()`, and `base_diagnostics()`.
    *   **RenderBridge**: Inherits `ScaleEngine`. The production Scale-0 engine — flat voxel array, six-phase tick, SOR Gauss solver. All WASM / benchmark / test code exercises this class.
    *   **ParticleEngine**: Inherits `ScaleEngine`. Handles purely decoupled continuous-space tracking. Bypasses the 3D local-lattice structure and executes $O(N \log N)$ Barnes-Hut Octree aggregations for point-objects.
    *   **AtomEngine**: Inherits `ScaleEngine`. Governs dynamic linked-list structures modeling valences and bindings in discrete space without matrix operations.
    *   **CosmicEngine**: Inherits `ScaleEngine`. SPH density mechanics and octrees modeling stellar life cycles.
    *   **DagEngine**: Inherits `ScaleEngine`. **EXPERIMENTAL** sparse-voxel-DAG prototype. `phase_read` + `phase_write` work against the `SparseVoxelDAG`; `gauss_project` / `phase_forces` / `phase_movement` are `[OPEN]` stubs. Do not use for physics — see `include/ftd/dag_engine.h` header banner.

### The Scale Bridge
The structural unification of these layers happens via `scale_bridge.cpp`. The mechanism is not direct state transfer, but conversion procedures utilizing `coarsen()` and `refine()` operations.
1.  **Coarsening**: Aggregates a volume of Scale 0 `Voxel` memory grids, identifying stable centroid bounds and dispatching analytical objects into the `ParticleEngine` stack.
2.  **Refining**: Executes `inject_wavepacket()` utilizing the exact coordinate bounds captured from macroscopic elements to recreate spatial density mapping in the Scale 0 engine arrays.

## 3. Data Structures and Memory Infrastructure

The engine dynamically switches between Array of Structures (AoS) architecture for localized CPU routines and Structure of Arrays (SoA) layouts for parallel hardware acceleration processing.

### CPU Implementation: Array of Structures (AoS)
The primitive memory construct for the Scale 0 engine is the `Voxel` struct (`voxel.h`). The lattice executes tightly packed linear loops iterating neighbor checks (Moore Neighborhood).
By compiling the fields into single struct instances (`state`, `flux`, `velocity`, `spin`), the CPU avoids cache missing on memory lookups since spatial adjacency mapping translates into linear `index + x` lookups within L3 memory lines.

### CUDA Backend Implementation: Structure of Arrays (SoA)
`gpu_buffers.cu` maintains a translated state of the simulation specifically decoupled for native CUDA grids.
Because thousands of GPU threads operate in lock-step (Warp threading), memory coalescing is required for high performance.
1.  The `gpu_buffers.h` unpacks the `Voxel` struct into independent memory arrays (`d_state`, `d_flux_x`, `d_flux_y`).
2.  When a block of threads requires state checks for Phase Read, the hardware executes a single contiguous memory fetch on `d_state`, effectively maxing bandwidth utilization across identical operations. 

## 4. WASM & Web Interop Application Interface

The simulation UI leverages the Emscripten toolchain via `ftd_wasm.cpp` to surface high-performance C++ execution within the browser's JavaScript V8 runtime.

### Exposing Classes (Embind)
Using the `embind` directive, the complete C++ class surfaces (`RenderBridge`, `ParticleEngine`) are wrapped securely. Property getters bypass the serialization layers to provide fast primitive reading (e.g. integer counts or bool toggles).

### Marshalling Data (Zero-Copy Transfer)
For arrays carrying large states of variables resulting from `getParticleData()`, standard serializing methods would throttle FPS heavily. The engine avoids transferring objects and uses typed view bridging:
1.  The `Float32Array` or `Float64Array` typed constructs are generated utilizing the WASM instance Heap offset buffers.
2.  JavaScript can point directly into the simulated C++ heap vectors (`HEAPF32`), observing rendering position inputs efficiently without memory duplicating, enabling continuous 60FPS read loops on heavily populated lattice configurations without activating JavaScript garbage collection routines.
