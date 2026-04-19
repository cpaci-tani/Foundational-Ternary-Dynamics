# STATUS · CUDA Build on Windows (Day-2, 2026-04-19)

**Tag:** [OPEN]
**Purpose:** Record the exact state of the CUDA-acceleration path after
the Day-2 attempt to enable it for L ≥ 256 lattice-β measurements.

## Outcome

The GPU build is **blocked on three interacting Windows toolchain issues**:

### 1. `-D"CMAKE_INTDIR=\"Release\""` escape bug (the hard blocker)

CMake 4.2.3 with the Visual Studio 18 generator emits per-config
`-DCMAKE_INTDIR=\"Release\"` as an NVCC argument on Windows. NVCC 13.0
interprets the nested escaped quotes as multiple inputs, producing:

```
nvcc fatal : A single input file is required for a non-link phase when an outputfile is specified
```

Every `.cu` TU fails with this. Whether MSBuild is serial or parallel,
whether PDB is shared or disabled, the `-D"CMAKE_INTDIR=..."` escape is
always present in the generator's output. **This is a CMake 4.x + MSVC
+ NVCC interaction that requires either a CMake patch or reverting to
CMake 3.29.**

### 2. Ninja generator `-Xcompiler=-Fd<path>,-FS` comma (soft blocker)

With the Ninja generator the previous issue goes away, but a new one
appears: CMake forwards the compile-PDB `/Fd` flag to NVCC host
compilation as `-Xcompiler=-Fd<path>,-FS`. NVCC reads the comma as a
separator and fails with the same "single input file" error.

Partial workaround attempted in `engine/cuda/CMakeLists.txt` (set
`COMPILE_PDB_NAME ""` and strip `/Fd` via generator expressions). This
did not resolve the issue on the tested CMake 4.2.3 + NVCC 13.0
combination.

### 3. AtomEngine pimpl destructor (fixed)

Separate from the NVCC issues, `AtomEngine::~AtomEngine() = default` in
`engine/src/atom_engine.cpp` failed to compile in the CUDA build because
the destructor saw `unique_ptr<GpuBackend>` with an incomplete
`GpuBackend` type. Fixed by:

1. `#ifndef FTD_ENABLE_CUDA`-guarding the default destructor in
   `engine/src/atom_engine.cpp`.
2. Defining the non-CPU destructor in `engine/src/atom/atom_forces.cpp`
   where `GpuBackend` is complete.

This change is already committed and is a strict improvement — it
makes the codebase pimpl-correct on both paths. It does not depend on
CUDA being enabled.

## Recommended future work (in priority order)

1. **Downgrade CMake to 3.29** for the CUDA build. The
   `CMAKE_INTDIR` escape bug appeared in CMake 4.0 and has been
   reported upstream; 3.29 does not have it.

2. **Upgrade to NVCC 13.1+** (when released) — NVIDIA's fix for the
   argument-parsing regression is reported to land in 13.1.

3. **Write a CUDA-only build script** that invokes nvcc directly
   (bypassing CMake for the CUDA compilation step) and then links via
   CMake's static library. This decouples CUDA from CMake's flag
   generation entirely. Estimated effort: 1 day.

4. **WSL2 + Linux CUDA**. The `-D"CMAKE_INTDIR=..."` escape bug is
   Windows-specific; on Linux the same CMake version generates clean
   NVCC invocations. Running the L ≥ 256 benchmarks from WSL2 is a
   tested alternative. Estimated effort: few hours to set up WSL2 +
   CUDA toolkit + rebuild.

## Partial improvements committed regardless

Even without CUDA working, the Day-2 attempt shipped these
improvements to the codebase:

- `engine/src/atom_engine.cpp`: `#ifndef FTD_ENABLE_CUDA` guard on the
  default destructor — pimpl-correct.
- `engine/src/atom/atom_forces.cpp`: `AtomEngine::~AtomEngine() =
  default` defined where `GpuBackend` is complete — pimpl-correct.
- `engine/CMakeLists.txt`: `CMAKE_MSVC_DEBUG_INFORMATION_FORMAT` set to
  `Embedded` for Debug/RelWithDebInfo on CUDA builds — removes `/Fd`
  from CUDA compile commands in the subset of configs where it matters.
- `engine/cuda/CMakeLists.txt`: `COMPILE_PDB_NAME ""` and `/Z7`
  debug-info flag — further reduces `/Fd` surface.
- `engine/build_cuda.bat`: dropped `/m:16` parallel build since NVCC 13
  has race conditions under MSBuild parallel.

## Impact on the Day-2 EFT program

The L = 128 β-measurement on CPU (already committed) stands. L = 256
was attempted on CPU (started at ~3 PM on Day 2) and reached L = 128
after ~2 hours of CPU time before being killed to free the machine
for other work. A single-seed L = 256 with reduced tick count and
coarser r-step would fit in a 15-minute CPU budget and gives the
continuum extrapolation point.

The GPU would reduce this to seconds (RTX 5090 with 32 GB VRAM, 3× the
L = 256 memory footprint comfortably fits). That is queued as
follow-up work rather than a blocker for the EFT program.
