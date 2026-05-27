# Handoff Report — Codebase Inventory Mapping (M1)

**Working Directory:** `c:\Users\cpaci\Desktop\ftd\.agents\explorer_m1\`  
**Target File:** `c:\Users\cpaci\Desktop\ftd\.agents\explorer_m1\handoff.md`  

---

## 1. Observation

Direct scans and investigations were performed across the `engine/` directory structure. Specifically:
* Confirmed the absolute path and layout of the core simulation components under `engine/` via the high-level description in `engine/SPEC_ENGINE.md`:
  > "The FTD simulation engine is built in C++17 with optional CUDA 13.0 GPU acceleration and Emscripten WebAssembly bindings."
* Inspected structural data structures, including `Voxel` and `ForceDiag` in `engine/include/ftd/voxel.h`:
  ```cpp
  struct Voxel {
      int8_t state = 0;       // -1, 0, +1
      Vec3 flux;              // dispositional field (J)
      Vec3 wave_vel;          // conjugate velocity for leapfrog
      // ...
  };
  ```
* Analyzed the coordinate and neighborhood wrapping stencils in `engine/include/ftd/lattice.h` (lines 56–107):
  * `neighbors_6` (face-sharing)
  * `neighbors_12` (edge-sharing)
  * `neighbors_8_corner` (BCC sub-stencil corner neighbors)
  * `neighbors_26` (full 26-neighbor Moore neighborhood)
* Documented the multi-scale class hierarchy from `engine/include/ftd/scale_engine.h` and continuous scale engines in `particle_engine.h`, `atom_engine.h`, and `cosmic_engine.h`, tracing their coarsening/refinement bridges to `scale.h`.
* Inspected the table-driven term toggle metadata in `engine/include/ftd/term_toggles.h` containing 27 boolean toggles (`wave_propagation` to `strict_validation`).
* Confirmed the existence of decomposed 6-phase tick loop TUs in `engine/src/render_bridge_phases/` (`phase_read.cpp`, `phase_write.cpp`, `phase_forces.cpp`, `phase_movement.cpp`).
* Cataloged standard CTest validation files in `engine/tests/` and confirmed 211 active CMake verification targets.
* Mapped the Three.js modular frontend components under `engine/web/js/` including the app state, WebAssembly/Mock bridge configurations, rendering viewports, and interactive pedagogical libraries.

The detailed inventory report has been written to:
`c:\Users\cpaci\Desktop\ftd\.agents\orchestrator_engine_map\M1_inventory_report.md`

---

## 2. Logic Chain

1. **Premise 1**: A comprehensive inventory must identify the exact roles, component boundaries, structures, and definitions of all subdirectories under `engine/`.
2. **Step 2**: Executed split file searches to successfully catalog all public headers in `include/ftd/` and source implementations in `src/` without context/timeout limitations.
3. **Step 3**: Inspected core headers (`voxel.h`, `lattice.h`, `scale_engine.h`, `scale.h`, `term_toggles.h`) to identify mathematical invariants, neighbor mapping mechanics, and polymorphic multi-scale bindings.
4. **Step 4**: Inspected the decomposed phase translation units (`phase_read.cpp`, etc.) and multi-scale continuous engines (`particle_engine.cpp`, `atom_engine.cpp`, `cosmic_engine.cpp`) to map the discrete-to-continuous simulation boundaries.
5. **Step 5**: Investigated parallel execution (`cuda/`), WASM mapping (`wasm/`), validation suites (`tests/`), and visualization layers (`web/`) to establish a unified boundary map.
6. **Step 6**: Synthesized all findings into a highly granular, structured markdown inventory report (`M1_inventory_report.md`) specifying the exact roles of all files and structures.

---

## 3. Caveats

* **Assumptions**: We assume MSVC/Ninja and NVCC are configured as defined in `CMakeLists.txt` for standard builds, and Emscripten is available for building the WASM dashboard.
* **Limitations**: High-level structural analysis was performed read-only. We did not run or build any C++ code directly.
* **Excluded Areas**: Sparse voxel DAG prototypes (`DagEngine`, `DagLattice`, etc.) were noted as experimental stubs but their numerical mechanics were not evaluated in detail as they are toggle-gated OFF and do not belong to the active physical paths.

---

## 4. Conclusion

The codebase inventory and mapping task (M1) is successfully complete. All files, directories, components, structures, and definitions under `engine/` have been surveyed, analyzed, and structured in a unified, highly granular codebase inventory report at `.agents/orchestrator_engine_map/M1_inventory_report.md`. The report establishes clean component boundaries and outlines exact data flows, providing a self-contained baseline for future implementation or auditing subagents.

---

## 5. Verification Method

To independently verify the mapping and check for regression issues:
1. View the newly created files:
   * `c:\Users\cpaci\Desktop\ftd\.agents\orchestrator_engine_map\M1_inventory_report.md`
   * `c:\Users\cpaci\Desktop\ftd\.agents\explorer_m1\progress.md`
2. Run standard CTest verification tests in the workspace to confirm engine legitimacy:
   ```powershell
   cmake -S engine -B engine/build
   cmake --build engine/build --config Release
   cd engine/build
   ctest --output-on-failure -C Release
   ```
3. Check for the specific presence of `test_render_bridge_golden` in the test suite execution. A successful completion verifies that all refactored phases run with bit-exact compliance.
