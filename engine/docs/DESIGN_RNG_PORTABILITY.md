# RNG Portability Design — CPUGPU Bit-Exact Stochastic Operations

**Status:** [DESIGN — awaiting implementation choice]
**Date:** 2026-05-05
**Scope:** bug-hunt deferred items BH-F5 (evaporation Boltzmann probability), BH-F8 (spin fallback), BH-F9 (RNG portability).

## Problem statement

The 2026-05-04 bug-hunt audit (commit `f2a721a`) flagged three deferred CPUGPU parity bugs whose root cause is shared:

- **BH-F5** — evaporation Boltzmann probability uses different RNG streams: CPU `voxel_uniform()` (SplitMix64) at `engine/src/render_bridge_phases/phase_write.cpp:53-64`; GPU `curandGenerateUniformDouble` (Philox4_32_10 internally) at `engine/cuda/gpu_engine.cu:256`.
- **BH-F8** — genesis spin fallback (when curl is zero, spin is chosen randomly): CPU at `phase_write.cpp:104-106`; GPU has no equivalent fallback at all (it leaves spin uninitialised on the genesis-with-zero-curl path).
- **BH-F9** — RNG portability writ large: same as F5 but framed as a cross-cutting issue. Any future stochastic kernel will hit the same divergence by default.

All three boil down to: **the same physical operation reads different random streams on CPU vs GPU**. Per-voxel state diverges deterministically from tick 1 onwards in any test that exercises a stochastic toggle. The 2026-05-04 `gpu_parity_complete` 20-domain sweep does pass, because its assertions either disable stochastic toggles or compare ensemble quantities (total manifested count) rather than per-voxel state.

## Two options

The choice is structural — it determines whether "CPUGPU bit-exact at unit mass" extends to stochastic operations or stays a property only of the deterministic core.

### Option A — bit-exact via shared SplitMix64

Drop cuRAND for these operations; implement a device-side `voxel_uniform_d(seed, voxel_idx, tick, salt)` that mirrors the CPU formula. The CPU formula is purely arithmetic and trivially portable to CUDA:

```cuda
__device__ __forceinline__
double voxel_uniform_d(uint64_t seed, int voxel_idx, int tick, uint64_t salt) {
    uint64_t x = seed
        ^ (static_cast<uint64_t>(voxel_idx) * 0x9E3779B97F4A7C15ULL)
        ^ (static_cast<uint64_t>(tick)      * 0xBF58476D1CE4E5B9ULL)
        ^ (salt                              * 0x94D049BB133111EBULL);
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    x =  x ^ (x >> 31);
    return (x >> 11) * (1.0 / 9007199254740992.0);
}
```

Salt enum (`VoxelRng::GenesisManifest = 1`, `GenesisSpin = 2`, `Evaporation = 3`) lives in a shared header consumed by both CPU and GPU. New stochastic kernels add a new salt value; CPU and GPU automatically agree.

**Pros:**
- Per-voxel CPUGPU bit-exact agreement at unit mass for all stochastic operations.
- Eliminates the per-tick `curandGenerateUniformDouble` call + the `bufs_.d_random` buffer (modest perf win — one cudaMalloc-equivalent + one kernel launch saved per tick when langevin is off; offset by `voxel_uniform_d` being inline per thread).
- Removes a hidden dependency on cuRAND's internal version-specific state advancement.
- Aligns with FTD's "deterministic logic-first" project philosophy and CLAUDE.md's epistemic discipline.
- Adds a fourth RNG salt (BH-F8 spin fallback) by adding one enum value, no kernel signature changes.

**Cons:**
- cuRAND Philox4_32_10 is statistically higher quality than SplitMix64 for very long streams (~2¹²⁸ period vs 2⁶⁴). For genesis-event probabilities at L ≤ 384 over 10⁶ ticks, both are fine — neither approaches the exhaustion regime — but if a future workload pushes that, SplitMix64 would need replacement.
- Removes cuRAND from the `langevin_noise` Gaussian-noise generation path too (currently `curandGenerateNormalDouble`); we'd need a Box-Muller transform applied to two SplitMix64 outputs. ~10 extra LOC, no behavioural risk.
- Requires touching every stochastic kernel: `genesis_kernel`, `evaporation_kernel`, the dual-substrate equivalents, and the Langevin OU update.

**Estimated size:** ~150 LOC across kernel files + 1 new shared header (`engine/include/ftd/voxel_rng.h`) + Box-Muller wrapper.

**Risk:** medium. Gaussian noise path replacement needs careful Box-Muller correctness check (can subtly bias if implemented wrong). Mitigated by adding a parity test that compares CPU and GPU first-moment + second-moment of langevin noise over a sample of 10⁵ voxels.

### Option B — accept ensemble equivalence; document divergence

Keep cuRAND Philox on GPU. Document that CPU and GPU stochastic operations agree only in distribution, not per voxel. Tighten parity tests:

- **Per-voxel assertions** in `gpu_parity_complete` and similar tests: replace with **ensemble assertions** — total manifest count over T ticks within ±5%, charge balance within ±3, mean particle lifetime within 10%.
- **Bit-exact gates** like the golden hash: confirm they don't run with stochastic toggles ON. If they do (current state: `genesis = true` is a default but the golden test seed is fixed across CPU/GPU runs), audit and either pin to one backend or relax the gate.

Add a CONVENTION note to `engine/SPEC_ENGINE.md`: "CPU and GPU diverge at per-voxel level for any operation that reads `voxel_uniform()` (CPU) or cuRAND state (GPU). Statistical agreement is asserted at ensemble level. Tests that need bit-exact CPUGPU agreement must disable all stochastic toggles."

**Pros:**
- Minimal code change (~50 LOC of test relaxation + documentation).
- Retains cuRAND's high statistical quality on GPU.
- No risk to the existing golden hash gate (which already runs with genesis on but matches because... wait, this is exactly the divergence). On audit: `render_bridge_golden` currently produces hash `0xcd957b601d47868a` at L=16, 100 ticks. If genesis stochastic operations diverge between CPU and GPU, the hash should fire. **The fact that it doesn't suggests the golden test happens to run the CPU path only.** Verify before declaring this option safe.

**Cons:**
- "Bit-exact CPUGPU" claim becomes scope-restricted to deterministic operations — a documentation-and-marketing-grade weakening.
- Larger error bars on every future stochastic parity test.
- Each new stochastic kernel needs its own ensemble assertion design.
- Does not align with FTD's deterministic-engine philosophy — invites a slow drift toward "GPU is fast but approximate, CPU is canonical".

**Estimated size:** ~50 LOC test edits + ~20 LOC SPEC_ENGINE.md prose.

**Risk:** low (mostly test-relaxation). The single live concern is: does the golden hash gate currently rely on stochastic per-voxel agreement that this option would break? Investigate before committing.

## Recommendation

**Option A (bit-exact via shared SplitMix64).** Three reasons:

1. **Project philosophy.** FTD is sold and tested as a deterministic logic-first physics engine; "CPUGPU bit-exact" is a load-bearing claim in `engine/SPEC_ENGINE.md` and the golden-hash gate. Option B silently scopes that claim down. Option A keeps it.

2. **Future-proofing.** Every stochastic kernel that lands in the future (currently 3, plausibly 5-8 if the FTD-0136 Phase B program lands) hits the same divergence under Option B and needs its own ensemble assertion. Option A makes the bit-exact agreement automatic.

3. **Reasonable cost.** ~150 LOC + 1 header + a Box-Muller wrapper is bounded, and the existing CPU `voxel_uniform()` is already the canonical stream — the GPU just needs to call it. No new physics, no new design discovery, just propagation.

The 2¹²⁸-period concern for cuRAND vs SplitMix64's 2⁶⁴ is real but distant: at L=384 with one stochastic call per voxel per tick, SplitMix64's 2⁶⁴ period is exhausted at ~7×10¹³ ticks ≈ 2 million years of CPU time. Not a concern for any realistic workload.

## Implementation plan if Option A is chosen

Single commit (BH-F5 + BH-F8 + BH-F9 closed together):

1. Create `engine/include/ftd/voxel_rng.h` with the salt enum + `voxel_uniform()` + a Box-Muller `voxel_normal2()` for Langevin (returns two Gaussians per call).
2. Refactor CPU `phase_write.cpp` lines 47-64 to include this header instead of defining its own `VoxelRng` enum + `voxel_uniform()` (zero behaviour change).
3. Add `__device__` overloads in a sibling `engine/cuda/voxel_rng_d.cuh` that mirror the same arithmetic.
4. Update `genesis_kernel`, `evaporation_kernel`, `genesis_dual_kernel` (and dual evap path) to call `voxel_uniform_d()` instead of reading from `bufs_.d_random[idx]`.
5. Update Langevin OU update kernels to call `voxel_normal2_d()` instead of reading from `bufs_.d_langevin_noise`.
6. Drop the cuRAND prefill calls in `gpu_engine.cu:256, 265, 317, 390, 396` and the `bufs_.d_random` / `bufs_.d_langevin_noise` allocations.
7. Add BH-F8: GPU genesis spin fallback in `genesis_kernel` mirroring `phase_write.cpp:104-106`.
8. Add a `gpu_parity_complete` row asserting per-voxel state agreement at L=8 over 50 ticks with `genesis=true` + a fixed seed.

Verify: golden hash bit-exact (it already runs this path — see "verify before declaring B safe" note above; under Option A the hash is unchanged because CPU stream is canonical and GPU now matches); `gpu_parity_complete` adds a new green row; existing `genesis` and `baryogenesis` tests pass; full sweep stays at the post-Tier-A tally.

## Implementation plan if Option B is chosen

Single commit:

1. Audit which existing parity tests assert per-voxel state on stochastic toggles. Likely candidates: `gpu_parity_complete`, `genesis`, `baryogenesis`, `gpu_continuity_ledger` GCL-5/GCL-7/GCL-8.
2. For each, relax the per-voxel assertion to ensemble (total count + charge balance).
3. Add a `[CONVENTION]` block to `engine/SPEC_ENGINE.md` documenting the divergence.
4. Update `LEDGER.md` to mark BH-F5, BH-F8, BH-F9 as `[CLOSED — DOCUMENTED, NOT FIXED]` with cross-reference to the convention note.
5. Verify: golden hash gate must remain green. If it doesn't, Option B is unsafe and Option A is mandatory.

## Open question for the user

Which option? My recommendation is **A** for the philosophy + future-proofing reasons above, but B is defensible if the project decides "CPUGPU bit-exact" is intentionally scope-restricted to deterministic operations.

Either way, this is one commit (or a small commit chain), and the work is bounded. The decision gate is whether you want CPU-equivalence on stochastic kernels as a structural property of the engine, or as an ensemble-level approximation.
