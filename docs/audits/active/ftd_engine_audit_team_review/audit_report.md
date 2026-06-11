# FTD Engine Audit Team Review

**Date:** 2026-06-10
**Status:** Completed
**Scope:** Architecture, UI/UX, Performance, Mathematical Physics

## Executive Summary
The FTD Engine reflects an exceptional degree of maturity. The two-layer ontology is strictly respected, the visual snapping is perfectly aligned, and the Standard Model constants are mathematically robust. 

However, the redteam identified **4 high-value performance bottlenecks**, **1 UI accessibility bug**, and **3 failing physics conservation tests** that require immediate attention.

## 1. Architecture & Ontology (Lead Architect)
- **Ontology Preservation:** The engine strictly respects FTD's two-layer ontology. `Voxel` memory footprint extensions for dual-substrate implementations are pragmatically separated.
- **Module Boundaries:** Symmetric JS/C++ bridging correctly isolates WASM and Mock targets. The refactored `render_bridge.cpp` is beautifully decoupled.
- **Telemetry Note:** The C++ compilation (`cpp_build.log`) was completely flawless.

## 2. Performance & Scaling (Performance Optimizer)
- **C++ Poisson Solver Paralyzation & Reductions:** The 18-point SOR Poisson solver (`poisson_solvers.cpp`) currently runs strictly sequentially because standard 2-color red-black ordering fails for 18-point Moore stencils (data races on radius-1 diagonals). **Fix:** Implement an 8-color ($2^3$) scheme to mathematically decouple the points and wrap the sweeps in `#pragma omp parallel for`. Additionally, `phi_sum` reductions in the solver lack `reduction(+:phi_sum)` pragmas, severely bottlenecking threads.
- **C++ Cache Locality Inversion:** The SOR loop incorrectly treats `x` as the fastest-varying inner loop dimension, but the memory layout uses `z` (stride 1). **Fix:** Transpose loop nesting to `x -> y -> z` to achieve linear memory access and drastically cut cache misses.
- **C++ Thread Load Balancing:** The core `phase_forces_main_loop` uses a `static` schedule. Workloads are heavily asymmetric due to early exits in empty space vs. dense colored active particles. **Fix:** Switch OpenMP to `schedule(dynamic, 64)` or `schedule(guided)` to keep all AMD 9950X3D cores saturated.
- **Web UI Garbage Collection Thrashing:** High-frequency WebGL allocations remain in `molecular-renderer.js`. It repeatedly calls `document.createElement('canvas')` and `new THREE.CanvasTexture` without pooling. **Fix:** Implement object pooling for HTMLCanvasElements and textures to eliminate GC stutters.

## 3. UI/UX & Web Accessibility (UX Designer)
- **Lattice Snapping:** Visual snapping of E/B/Poynting vectors is flawless, perfectly respecting the `+ 0.5` center-voxel offset natively.
- **Reduced Motion Bug:** While the CSS correctly defines global accessibility overrides under `body[data-reduced-motion="1"]`, the `app.js` state manager never actually applies this attribute to the body, meaning animations ignore OS-level accessibility preferences.
- **Action Item:** Wire up a `window.matchMedia('(prefers-reduced-motion: reduce)')` listener in `app.js` to automatically broadcast the attribute.

## 4. Mathematics & Physics (Mathematical Physicist)
- **Mathematical Correctness:** The 18-point isotropic Moore Laplacian and bare symplectic leapfrog integrator operate flawlessly within the exact $C_{SPEED} = 1/\sqrt{3}$ von Neumann stability limit. Standard Model calibrations ($\alpha$, $K_B$, $N_c$) are precise.
- **Test Failures (Action Required):**
  - `stress_energy` (Test #148): Failed invariant `|P_x| > |P_y|` (found $P_y = 0.001591 \gg |P_x| = 0.000211$). Expected longitudinal momentum flow is being overshadowed by transverse flow.
  - `wz_mass` (Test #234): Chirality at +1/-1 sites yielded small negative values instead of opposite signs.
  - `cluster_persistence_quiescent` (Test #20): Failed to track clusters of minimum size 4, contradicting the FTD-0110 canonical injection prediction.
- **Timeout Cascading:** The CTest suite is suffering from severe timeouts because of the explicit `TIMEOUT 600` configurations inside CMake. Expanding these explicit limits is necessary for the CI pipeline to stabilize.
