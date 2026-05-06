# DECISION · Production Gauss Representation for $S_\text{eff}$ Measurements

**Tag:** [DECISION]
**Date:** 2026-05-05
**Status:** [DECISION] for R3 onward — picks **GPU cuFFT (collocated, single-substrate)** as the canonical Gauss representation against which $S_\text{eff}$ is measured. The CPU SOR variant remains the canonical reference for unit-test parity but is not the measurement target. Source-core fork and dual-cell face-flux remain available for sensitivity analysis but are not production.
**Purpose:** Phase R2 of the FTD-EFT roadmap. Closes `STATUS_EFT_CHECKLIST.md` §2 unchecked items "Resolve source-core compromise" and "Select production Gauss representation."

---

## §1 — The four candidates

The engine implements (or has prototype machinery for) four distinct Gauss representations:

| ID | Representation | Where | Residual at end of tick |
|---|---|---|---|
| **A** | Collocated SOR (CPU) | `engine/src/render_bridge.cpp` SOR sweep, `poisson_solvers.cpp:21-84` | ~$10^{-4}$ on the canonical 30-iteration ω=1.75 schedule |
| **B** | Collocated cuFFT (GPU) | `engine/cuda/kernels_poisson.cu`, called from `gpu_engine.cu` | ≤ $10^{-8}$ (spectral, exact up to FP rounding) |
| **C** | Source-core fork (skip particle sites) | `engine/src/poisson_solvers.cpp` source-core branch | not in production; prototype only |
| **D** | Dual-cell face-flux | discussed in `SPEC_FTD_NATIVE_BLOCKING_MAP.md`; prototype in `engine/include/ftd/sublattice.h` | not in production |

The differences:

- **A vs B**: same constraint $\nabla\cdot\mathbf{J} = \rho$, same field basis (collocated). Different numerical solvers. CALLSTACK F6 (RESOLVED) documents both in `engine/SPEC_ENGINE.md` §"FFT Poisson Solver" lines 937-946.
- **A,B vs C**: source-core fork modifies which sites participate in the Gauss enforcement — particle sites are skipped because their flux is locked by manifestation, not by the Gauss source equation. C is conceptually cleaner but was a prototype that didn't graduate.
- **A,B vs D**: dual-cell face-flux uses face-centered field variables instead of collocated. This is the alternative to the field-basis decision in `DECISION_FIELD_BASIS.md`; the two decisions are coupled — see that doc.

---

## §2 — Decision criteria

The R3 deliverable measures the explicit nonlinear blocked $S_\text{eff}[J, s]$ by extending the operator-mixing-matrix campaign to L ∈ {64, 96, 128} with b ∈ {2, 4} blockings. Whichever Gauss representation is canonical for those measurements becomes the **reference** for $S_\text{eff}$.

Three criteria, in priority order:

1. **Numerical accuracy at the constraint level.** A representation whose residual is $10^{-4}$ adds noise to the operator-mixing measurements equal to that residual. A representation at $10^{-8}$ leaves measurement-grade headroom.
2. **Determinism.** The representation must be bit-exact reproducible across runs (for pre-registration purposes).
3. **GPU-native.** Per CLAUDE.md: "GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA. ... Any measurement campaign, sweep, or multi-seed run goes through WSL2." The R3 campaign will run on GPU; the canonical reference should be the path the GPU takes.

These criteria pick **B (collocated cuFFT)** uniquely.

A is a strong second on (2) and (3) but loses on (1). C and D are not in production — they're proto-stage. Adopting them as canonical would require porting + parity testing first.

---

## §3 — The decision [DECISION]

**For R3 onward, the canonical Gauss representation is:**

$$
\boxed{\text{Representation B — Collocated cuFFT, single-substrate, GPU production path}}
$$

Specifically:

- The constraint $\nabla\cdot\mathbf{J} = \rho$ is enforced via spectral inversion of the cubic-lattice Laplacian using cuFFT 3D C2C plans (`gpu_engine.cu:107-112`).
- Residual tolerance: $\max|\nabla\cdot\mathbf{J} - \rho| \leq 10^{-8}$ at end of `gauss_project()`.
- Field basis: collocated $(s, J)$ at lattice vertices (consistent with `DECISION_FIELD_BASIS.md`).
- Per-tick solve in production, no warm-starting hack (cuFFT is non-iterative; SOR's warm-start is not relevant).

**The CPU SOR variant stays as a parity reference.** Its residual floor at $\sim 10^{-4}$ is acceptable for unit tests that don't measure operator-mixing matrices. CALLSTACK F6 documentation in `engine/SPEC_ENGINE.md` 937-946 records the divergence for posterity. The `gpu_parity_complete` regression test compares CPU and GPU at the per-voxel state level, accepting the documented $\sim 10^{-4}$ Gauss residual difference between SOR and cuFFT outputs as fingerprinted noise (not as physics divergence).

**The source-core fork (C) and dual-cell face-flux (D)** are **not adopted**. They remain available as prototype paths if R5 inter-scale work surfaces a measurement that benefits from skipped-particle-site convention or face-centered variables. Adopting either would require a fresh decision document.

---

## §4 — Implications for downstream work

- **R3a (mixing-matrix campaign at L ∈ {64, 96, 128})**: runs on cuFFT. Pre-registered residual tolerance for each measurement: $\max|\nabla\cdot\mathbf{J} - \rho| \leq 10^{-8}$. Failures at this tolerance halt the campaign rather than silently lowering tolerance.
- **R3b (12 dim-6 operator measurements)**: same.
- **R4 (β(g, L) extraction)**: same. Phase-structure-flow analysis treats Gauss-residual noise as a known-bounded contribution to operator drift across L.
- **R5 inter-scale work**: scale 0→1 measurement protocols should declare which Gauss representation they use — by default, the production representation (B). If scale-1→2 or higher needs face-centered convention (D), that's a fresh decision.

---

## §5 — What this decision does NOT cover

- The **value** of $\lambda_G$ in the Lagrangian's Gauss penalty `lagrangian.h:55` (`LAMBDA_G = 100.0` finite, vs spec target $\lambda_G \to \infty$). The decision here is about how the constraint is enforced numerically, not about the analytical action's penalty strength.
- **Source-core handling at the operator level** (i.e. whether per-particle observables are computed at site $\mathbf{v}_p$ vs at neighbouring sites). The production Gauss solver enforces the constraint everywhere; downstream observables can still adopt source-core conventions where appropriate.
- **Boundary handling** under non-periodic conditions. The current production cuFFT assumes periodic boundary; non-periodic R5 work (e.g. open-boundary planetary scenarios) needs a separate decision.

---

## §6 — Refresh policy

If a future engine refactor changes the Gauss enforcement path (e.g. CPU production switches from SOR to FFT, or GPU adopts a multigrid replacement for cuFFT), this decision document needs to be revisited. Until that happens, B is canonical.

The engine respects this decision by default — `gpu_engine.cu` already uses cuFFT in `gpu_gauss_project()` at `:333`. No engine code changes are required by this DECISION.
