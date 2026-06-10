# SPEC - FTD-0253 Spacetime-Forcing Demo

**Status:** `[DEMO]` + `[BOUNDARY]`
**Date:** 2026-06-10
**Theory source:** `docs/theory/02_foundations/FOUND_SPACETIME_FORCING_BOUNDARY.md`
**Regression source:** `engine/tests/test_spacetime_forcing_demo.cpp`

## Purpose

This is the user-visible promotion of the FTD-0253 engine demo. It makes one
boundary result runnable from `engine/web`:

- shared locality: the same lattice and same local Laplacian keep both branches
  inside the same causal cone;
- wave-only metric behavior: clock oscillation, reversible energy bookkeeping,
  and ballistic-ruler behavior appear only in the second-order wave branch;
- no promotion: this is not a derivation of the Lorentzian metric from P1-P5.

## User-visible artifacts

| Artifact | Path | What it shows |
|---|---|---|
| Dashboard scenario | `s0-field-spacetime-forcing-boundary` | The live JS/mock wave-side seed: one sub-threshold center pulse with non-wave phases disabled by scenario profile. A C++ mirror branch exists for parity and future WASM rebuilds. |
| Standalone demo | `engine/web/demos/spacetime-forcing-boundary.html` | Side-by-side replay of WAVE vs first-order DIFFUSION using the same scalar z-component initial condition as the C++ test. |

Run locally:

```powershell
python engine/web/serve.py 8080
```

Then open:

- `http://localhost:8080` and choose `Spacetime forcing boundary (FTD-0253)` in Scale 0 field configurations.
- `http://localhost:8080/demos/spacetime-forcing-boundary.html` for the side-by-side counterfactual replay.

## Provenance

The standalone page mirrors the controlled comparison in
`test_spacetime_forcing_demo.cpp`:

- `L = 48`, `dt = 0.2`, `c^2 = 1/3`, `D = c^2 / 4`;
- same 18-point isotropic Moore Laplacian weights (`face = 1/3`, `edge = 1/6`);
- same seeded component: `J_z` only. The vector components decouple for this
  initial condition, so the browser replay uses a scalar z-component grid.

Pinned C++ baseline from the test:

| Check | Result |
|---|---|
| CTest | `spacetime_forcing_demo`, 9/9 PASS |
| Cone front at `t=8` | WAVE = 7.211, DIFFUSION = 7.211 |
| RMS growth, `t=40 -> 160` | WAVE about 4.68, DIFFUSION about 2.46 |
| Clock branch | WAVE oscillates; DIFFUSION decays monotonically |

## Honesty constraints

- The dashboard scenario is an engine scenario, but it is only the WAVE branch.
- The dashboard route is explicitly JS/mock-owned so it remains visible even
  when the checked-in WASM bundle predates the new C++ scenario branch.
- The dashboard route also forces its required display profile after restoring
  user overlay preferences: flux volume on, flux slice on, lower flux threshold,
  larger point scale, and higher opacity. These are visual controls only.
- The DIFFUSION branch is a labelled counterfactual, not a physics phase in the
  FTD engine.
- The artifact demonstrates the FTD-0253 boundary result; it does not derive
  spacetime, gamma, alpha, or a new postulate.
- Do not use this page as a numerical search surface. It is a fixed replay of
  one already-registered regression demo.

## Verification

```powershell
cmake --build engine/build --config Release --parallel 24
Push-Location engine/build
ctest -R spacetime_forcing_demo --output-on-failure -C Release -j 24
Pop-Location
```

For web scenario wiring:

```powershell
Push-Location engine/web/tests
npx playwright test scenario-parity.spec.js --reporter=list
Pop-Location
```
