# GPU Completeness Program — Scale-0 TermToggles

**Status:** in progress (2026-08-19)
**Owner intent:** port every CPU-only Scale-0 term to CUDA, keep the CUDA
path on the same physics as the CPU path, then run Approach 2 (axiom
tests + empirical map at LEDGER tags).
**Not this program:** fitting PDG numbers, combinatorial toggle explosion,
or promoting `[SELECTION]` / `[IMPOSED]` CUDA ports to `[DERIVED]`.
Default-off selected extensions stay golden-neutral.

Related but **separate**: AtomEngine / ParticleEngine GPU work stays in
[`GPU_PORT_PLAN.md`](GPU_PORT_PLAN.md). Do not mix those Scale-1/2 ports
into this Scale-0 term matrix.

---

## What “follows the physics” means here

CPU/GPU **bit-parity** (or a declared numerical contract, as for FFT vs
SOR Poisson) on the same toggle configuration. Not empirical retuning.
Goldens never absorb a port. Tags stay where the LEDGER has them.

---

## Slice 0 — Completeness oracle (do this first)

Machine-checked table: every `TOGGLE_SPECS` row classified by *live*
implementation, not by the `ToggleBackend` mask’s marketing.

| Class | Meaning |
|---|---|
| `NativeCuda` | Device kernel; no host fallback required |
| `GpuOnlyNoCpu` | CUDA kernel; CPU is an advertised no-op |
| `CpuOnly` | Forces CPU backend when enabled |
| `CpuFallbackSync` | CUDA acknowledges the term by syncing to CPU |
| `HostMirrorHybrid` | GPU tick then full AoS mirror |
| `IntentFlag` | No physics branch (none remaining after `confinement` was wired) |
| `ControlOnly` | Validator / diagnostics, not a field term |

**Fail closed:** `ToggleBackend::ANY` on an intent flag, a CPU-only
integrator, or a GPU-only no-op is a lie. The oracle CTest
(`gpu_term_contract`) pins the table; ports update the row in the same
change as the kernel.

Source: `engine/include/ftd/gpu_term_contract.h`
Test: `engine/tests/test_gpu_term_contract.cpp`

Known truths the oracle must record *before* any port:

- `langevin` is already `NativeCuda` (stale “CPU only at runtime” comment).
- `confinement` is now `NativeCuda`: linear `SIGMA_STRING` colour shell, not FTD-0025.
- `strong_force` / `exchange_force` / `cluster_inertia` are `NativeCuda` (CPU + CUDA share the same pairwise helpers / DFS order). `cluster_inertia` accepts any force channel (EM, colour, Yukawa, or exchange), not only `forces`. `knot_tracking` remains `HostMirrorHybrid`.
- `strong_stress_energy` and `matched_gauss_dynamics` are `NativeCuda` (isolated default-off sectors).
- P11 default-order movement is a serial CUDA commit with `moved[]`. `symmetric_movement_order` is native SplitMix64 Fisher-Yates on both backends.
- GPU triad detection runs after movement + weak, matching CPU Rule 7.

---

## Slice 1+ — CUDA ports (one term at a time)

TDD: WSL CPU/GPU parity test first, then kernel, then flip the oracle
row and `ToggleBackend` mask together. `engine\build_native.bat` +
`ctest -L merge_gate -j 32 -C Release` after any tick/toggle change.
Multi-tick GPU runs go through WSL2 `engine/build_wsl`.

| Order | Term | Why this order |
|---|---|---|
| 1 | `verlet_wave_integrator` | **Done 2026-08-19** — native CUDA KDK; CPU/GPU field parity test `gpu_verlet_parity` |
| 2 | `lorentz_period2_floquet` | **Done 2026-08-19** — native period-two kick +3/13, −1/13 from live `d_tick` |
| 3 | `lorentz_bcc_time_floquet` | **Done 2026-08-19** — native BCC-time IR surrogate; same injection point |
| 4 | `symmetric_movement_order` + **P11** | **Done 2026-08-19** — serial CUDA commit already matched default P11; shuffle is native SplitMix64 Fisher-Yates |
| 4b | force stack + higher-order | **Done 2026-08-19** — CPU Yukawa/exchange; native `cluster_inertia`; GPU triad after movement |
| 5 | `strong_stress_energy` | **Done 2026-08-19** — remainder colour, Hamiltonian projection, CIC T00; `gpu_strong_stress_parity` |
| 6 | `matched_gauss_dynamics` | **Done 2026-08-19** — skip legacy writer, device Faraday/Ampere/current; `gpu_matched_gauss_parity` |
| 7 | `knot_tracking` | Observation-only host mirror; device-resident only if interactive GPU requires it |

`confinement` is wired as a linear colour-string kernel (CPU+CUDA), default OFF, [SELECTION] — **not** a port of FTD-0025 Wilson loops.

---

## Slice N — Approach 2 (after the CPU-only matrix is honest)

1. Completeness oracle stays red if any `ANY` mask is a lie.
2. P1–P5 + FC-1 axiom tests on WSL GPU (including P11 after slice 4).
3. Empirical map of *engine-visible* phenomena at LEDGER tags:
   `native match` / `parametric` / `mismatch` / `absent` / `backend-split`.
   Not the 162-row Python catalog, and never a golden retune.

---

## Non-goals

- Combinatorial empirical gate over all toggle pairs.
- Master-quadratic `x₊` in the force path (source-lint forbids it).
- Fake CUDA for intent flags.
- Mixing UI v2 Phase 1 into these commits.
