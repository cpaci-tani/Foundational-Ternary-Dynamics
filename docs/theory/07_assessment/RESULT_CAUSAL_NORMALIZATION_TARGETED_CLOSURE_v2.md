# FTD-0403 v2 — Targeted causal-normalization closure result

**Frozen outcome:** **TARGETED-CLOSURE**

**Lock:** `preregister-causal-normalization-targeted-closure-v2` at commit `6ceaa76e1271f6e9e768f4e71d12f9c30a7a59bd`; preregistration SHA256 `efe5533f7276870ad4a276e317d61933a56535371a8aba45417c063372c37b2a`.

## Verdict

The selected raw-lattice causal and mass-role implementation closes over its exact transitive regression surface. FTD-0403 v2 therefore supplies the proportional regression verdict missing from FTD-0402 and closes `§12-cnorm`.

FTD-0402 remains historically `PARTIAL`: its frozen repository-wide G9 was not completed. FTD-0403 v1 remains `INVALID`: it exposed a stale boundary-test premise. Neither prior verdict is rewritten. This v2 result is an independent successor result under a new lock.

## Frozen gates

| Gate | Result | Evidence |
|---|---|---|
| T1 exact contract | PASS | A1–A7 and S1–S9 pass; the legacy raw-`c=1` normalization remains rejected |
| T2 native changed surface | PASS 14/14 | Includes the repaired non-vacuous boundary crossing, causal normalization, proper time, energy, inertia, Born–Infeld, Lorentz, Lagrangian, irreversibility, and movement targets |
| T3 CUDA changed surface | PASS 6/6 | `gpu_parity_complete`, `gpu_evaporation_parity`, `gpu_parity`, `force_diag_parity`, `causal_normalization`, and `gauge_gpu_parity` |
| T4 golden boundary | PASS 7/7 | All registered `golden` targets retain the accepted FTD-0402 hashes |
| T5 WASM/web | PASS | Release WASM target, Node time analysis, one physical-energy browser contract, and both scenario-telemetry contracts |
| T6 repository contracts | PASS | `git diff --check`; added links; compatibility-only `M_REST`; preregistration census GREEN |

No full CTest aggregate, unrelated campaign, or numerical coincidence search was run.

## Instrument repair

The v1 fixture used raw `velocity.x=-1`, which is outside `C_SPEED=1/sqrt(3)`. Movement correctly projected that external mutation before the fixture's expected one-tick crossing. Test commit `4325b36a` replaces it with `velocity.x=-0.5*C_SPEED` plus `remainder.x=-0.75`. The accumulated displacement crosses the boundary while remaining causally admissible. Both exhaustive and reflective arms pass and each records zero projection events. Production engine and UI sources are unchanged from FTD-0402 implementation commit `6526fefa`.

## Licensed consequence

This result licenses only:

> **[THEOREM — current engine implementation conforms to the selected raw-lattice causal and mass-role contract over the frozen changed surface].**

It closes `§12-cnorm` and makes NCEMC admissible as a separately pre-registered successor. It does **not** derive the clock/bandwidth axiom, `K_B`, an electron mass scale, Lorentz covariance, inertial–gravitational equivalence, confinement energy, a strong Hamiltonian, or a common stress–energy source. `M_GRAVITATIONAL=M_INERTIAL` remains an imposed numerical equality.

## Reproduction record

- Git execution SHA: `6ceaa76e1271f6e9e768f4e71d12f9c30a7a59bd`
- Platform: WSL2 Ubuntu 22.04, Linux `6.6.87.2-microsoft-standard-WSL2`, NVIDIA GeForce RTX 5090, driver `610.47`
- Boundary-test source SHA256: `aa3538c47d048e78f8ddcd6ae753a0924a7cc8679cca6a6ccd802f86ae4d0d06`
- Boundary-test binary SHA256: `c2d3387816cbb5b89742ca9a284e4dda8a2a08564ae2ebdf89df8c71ece74c8c`
- Exact verifier SHA256: `3bb62704d5f77c1ea4f1488129e8cc4bef3e10fb93610e2c5b0bfb849ee7f2ad`
- Causal-test source SHA256: `a164eed0406b110cdf62b09985ba64ab6fafd330d82c7e9319809d984d52dbdc`
- WASM SHA256: `0b9c04210a09b92fb638cfae7884280e870941f57bd9d86b9a115abe317ef011`
- Effective execution: CPU-forced only where targets require it; `FTD_FORCE_GPU` unset for the CUDA group; repository/default target toggles otherwise unchanged
- Raw command and count record: `engine/results/causal_normalization_targeted_closure_2026-07-21/v2_verification.txt`
