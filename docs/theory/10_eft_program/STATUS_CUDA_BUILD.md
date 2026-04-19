# STATUS · CUDA Build (Day-2, updated 2026-04-19)

**Tag:** [SOLVED-VIA-WSL2]
**Purpose:** Record the exact state of the CUDA-acceleration path after
the Day-2 WSL2 migration.

## Outcome — RESOLVED VIA WSL2

The Windows CMake 4 + NVCC 13 escape bug is **sidestepped entirely** by
building from Ubuntu 22.04 inside WSL2. Linux builds clean; the escape
bug was a Windows-specific CMake generator issue.

### WSL2 path (working, adopted)

| Step | Status |
|---|---|
| WSL2 Ubuntu 22.04 install | ✅ `wsl --install -d Ubuntu-22.04 --no-launch` |
| NVIDIA CUDA 13.0 toolkit in Ubuntu | ✅ via `cuda-keyring_1.1-1_all.deb` + apt |
| RTX 5090 GPU passthrough to WSL2 | ✅ nvidia-smi inside Ubuntu shows the card |
| `ftd_cuda` + `ftd_core` build | ✅ ~45s clean (Ninja + GCC 11 + nvcc 13.0) |
| `benchmark_beta_function --quick` runtime on GPU | ✅ 0.54s (was > 60s CPU) |
| Full-precision L=256 β scan runtime | ✅ 4m40s (CPU took > 2h and didn't finish L=128) |
| GPU / CPU parity for α_fit at L=64,128 | ✅ 2% agreement (0.123 vs 0.120, 0.134 vs 0.131) |

### Build script

`engine/wsl/build_cuda_wsl.sh` — ~20-line shell script run from Ubuntu 22.04
that invokes cmake + ninja. Entire build path is:

```bash
wsl -d Ubuntu-22.04 -u root -- bash -c \
  'cd /mnt/c/Users/cpaci/Desktop/ftd && bash engine/wsl/build_cuda_wsl.sh'
```

### Code fixes required on Linux (stricter than MSVC)

Linux GCC-11 + nvcc 13.0 is stricter about standard-library includes than
Windows MSVC. Four surgical additions:

1. `engine/include/ftd/gpu_buffers.h` — added `#include <cstdint>` for `uint8_t`
2. `engine/cuda/kernels_stencil.cu` — added `#include <cstdio>` and `#include <cstdlib>` for `fprintf`/`exit` used in CUDA_CHECK macro
3. `engine/cuda/kernels_forces.cu` — same cstdio/cstdlib additions
4. `engine/src/atom_engine.cpp` — constructor AND destructor both need to be guarded (not just destructor) so CUDA build routes to the definitions in `src/atom/atom_forces.cpp` where GpuBackend is complete

These are all backward-compatible improvements — zero impact on the
Windows CPU build.

### Build script (the actual one)

```bash
#!/usr/bin/env bash
# engine/wsl/build_cuda_wsl.sh
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd
export PATH=/usr/local/cuda-13.0/bin:/usr/bin:/bin
export LD_LIBRARY_PATH=/usr/local/cuda-13.0/lib64
cmake -S engine -B engine/build_wsl -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DFTD_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES="89;120"
cmake --build engine/build_wsl --target ftd_core ftd_cuda -j 4
```

## Performance measurements (first WSL2 run)

| Lattice | CPU (ticks=300) | GPU (ticks=300) | speedup |
|---|---|---|---|
| L=64 β-fit | 60s | ≤ 2s | > 30× |
| L=128 β-fit | ~8 min | ~15s | > 30× |
| L=256 β-fit | > 2h (never completed on Windows) | 4m 40s | > 30× |

### Memory footprint

RTX 5090 with 32 GB VRAM comfortably handles:
- L=256: ~2.8 GB
- L=512: ~22 GB (fits)
- L=1024: ~180 GB (does NOT fit — would need multi-GPU or streaming)

## Important physics finding surfaced by GPU run

The full-precision L=256 GPU measurement (ticks=300) gives
**α_r(r=82, L=256) = 0.0271** (ratio 3.72× α_ref), **not 0.010** as the
earlier fast-big CPU run (ticks=100) suggested. The fast-big ticks=100
was under-equilibrated — the flux field around static charges had not
yet reached its steady-state Coulomb tail amplitude.

The correct 3-point r_max series (all ticks=300):
- L=64, r=20: α_r = 0.030 (4.1× α_ref)
- L=128, r=40: α_r = 0.028 (3.8× α_ref)
- L=256, r=82: α_r = 0.027 (3.7× α_ref)

**These plateau**, they don't converge to α_ref. The 3.7× gap is not a
finite-size effect that shrinks with L — it may be a real offset.

This finding directly affects the Day-2 manuscript claim of
"α_∞ = 1.23× α_ref from 1/L extrapolation." The fast-big CPU measurement
that supported that extrapolation was an artefact of insufficient tick
count. The proper extrapolation — to be done in Phase F of the pipeline
plan — must use the full-precision GPU data at L=256 (and L=512 when
available) and may tell a different story about FTD's continuum limit.

**Action**: the manuscript and DERIV_DAY2_CAMPAIGN.md need correction
in Phase F. Queued as "revisit continuum extrapolation with
full-precision data" ticket.

## Legacy Windows build (deferred, not planned)

The four paths previously enumerated for fixing the Windows build are
all parked:

1. Downgrade CMake to 3.29 — not pursued (WSL2 works)
2. Upgrade to NVCC 13.1+ — wait for NVIDIA
3. Direct-nvcc bypass — not pursued (WSL2 works)
4. WSL2 — **chosen, working**

If we want a Windows-native build for release engineering reasons in the
future, any of (1)/(2)/(3) remains available. For research compute the
WSL2 path is sufficient and adopted.
