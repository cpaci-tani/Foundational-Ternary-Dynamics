# FTD-0402 — Causal normalization and mass-role reconciliation result

**Frozen outcome:** **PARTIAL**

**Epistemic scope:** `[MEASURED — selected current-engine implementation contract]`; no framework type, physical mass derivation, covariance theorem, or confinement-energy claim is added.

**Lock:** `preregister-causal-normalization-mass-roles-v1` at commit `41e06051c08af7e0f1a3d531b08077837ea34312`; preregistration SHA256 `1ebbd3359b8766120feb8cd0bbf372786246a2d2adecc1dc9b240b2ffdbc2ee0`.

**Implementation:** `6526fefa637cd0c0d1feb56e421bdc60c19a290e`, executed inside the frozen 72-hour window.

## 1. Verdict

Gates G2–G8 and G10 close. The exact anchors, causal enforcement, mass-role separation, ordinary-force non-projection control, determinism, CPU/GPU parity, WASM append-only contract, relevant browser contract, and production `M_REST` removal all pass.

G9 does not close because the repository-wide CTest aggregation did not finish. The first full command logged three unrelated long targets as passing—`halo_forcedness` in 1446.36 s, `maxwell` in 961.47 s, and `proton_stability` in 198.39 s—before the outer terminal wrapper detached during `light_deflection`. An equivalent remainder run was started and then stopped at owner direction rather than spending further time rerunning unrelated campaigns. There is therefore no full-suite verdict.

The frozen precedence makes the outcome **PARTIAL**, not `CONSISTENT-RAW`: no exact-anchor, causal, determinism, compatibility, or parity gate failed, but the required aggregate G9 evidence is incomplete. `§12-cnorm` remains open under this result and FTD-0403 is not allocated by this document.

## 2. Closed subcontracts

| Gate | Evidence | Status |
|---|---|---|
| G2 | `C²=1/3`; rate² is `0` at `u=C,L=0`, `3/4` at `u=C/2,L=0`, and `1-L²` at rest | PASS |
| G3 | rate/gamma reciprocity; selected speed boundary; `bandwidth_used=1`; full budget `B` | PASS |
| G4 | Born–Infeld Legendre identity; `E_REST=M_INERTIAL*C²`; `E²=E0²+C²|P|²` | PASS |
| G5 | horizon and non-finite controls avoid NaN propagation and repair invalid movement input | PASS |
| G6 | external over-speed mutations are projected and counted; base/color/Yukawa/exchange force evolution remains `B<1` with zero movement-entry projections | PASS |
| G7 | exact CPU/GPU one-tick and sixteen-tick parity for `tau`, phase, evaporation decision/hazard, causal budget, energy, and momentum | PASS |
| G8 | relevant native targets pass twice; golden 7/7 passes twice; relevant WASM/browser contracts pass | PASS |
| G9 | build, verifier, targeted parity, golden, web contract, and `diff --check` pass; full CTest aggregate incomplete | INCOMPLETE |
| G10 | fixed WASM indices 0–18 preserved; new fields appended; no production `M_REST` consumer; defaults/public API compatible | PASS |

The old implementation supplies the required non-vacuity failure: at raw `u=C_SPEED,L=0`, its legacy clock returned rate² `2/3`, whereas the selected causal boundary requires zero. The corrected external-mutation control increments `causal_projection_events`; ordinary force evolution leaves the counter at zero.

## 3. Implemented contract

One CUDA-safe interface now defines raw-lattice kinematics for CPU, GPU, `Voxel`, proper time, Born–Infeld, force integration, and movement:

\[
\beta^2=|u|^2/C_{\rm SPEED}^2,\qquad
B=\beta^2+L^2,\qquad
d\tau/dt=\sqrt{\max(1-B,0)}.
\]

GPU force paths accumulate before one common momentum integration. Device-side `tau` accumulation and the distinct `C_SPEED(1-L²)` clamp are removed; the common host post-pass advances `tau` and phase once. Movement repairs only externally injected or directly mutated out-of-budget velocities and exposes a projection count.

The former fused alias is separated as:

```text
M_INERTIAL      = K_B
E_REST          = M_INERTIAL * C_SPEED^2 = K_B/3
M_GRAVITATIONAL = K_B
M_REST          = M_INERTIAL  // compatibility only
```

`M_GRAVITATIONAL=M_INERTIAL` remains an imposed numerical equality. It is not a common stress–energy construction. The de Broglie frequency remains explicitly tied to imposed `K_B`.

`EnergyAudit` now distinguishes particle rest energy, normalized particle kinetic/total energy, vector momentum, dynamic energy, and accounted total energy. The accounted total remains incomplete because FTD-0402 adds no confinement Hamiltonian or interaction-energy completion.

## 4. Determinism, parity, and compatibility evidence

- `test_causal_normalization`: all exact anchors and controls pass, including the four large-force paths and exact one-/sixteen-tick CPU/GPU comparisons; repeated twice.
- Targeted native group: voxel, Born–Infeld, Lorentz, Lagrangian, gamma momentum, de Broglie, cluster inertia, energy conservation, tick order, and full-state irreversibility pass twice.
- GPU group: evaporation, gauge, force diagnostics, causal normalization, `gpu_parity`, and `gpu_parity_complete` pass twice.
- Golden gate: 7/7 passes twice after field-by-field reconciliation.
- WASM Release build succeeds. The physical-energy browser contract passes twice; the scenario telemetry contract passes 2/2. One intervening browser page-boot timeout passed on immediate unchanged rerun and is not a physics failure.
- Exact Python verifier: A1–A7 and S1–S9 pass.
- `git diff --check`: pass.

The pre-lock/current golden comparison found the following direct and downstream changes:

- direct: particle velocity and movement remainder from normalized `F/M_INERTIAL` momentum integration;
- downstream: ordinary/dual flux, wave, acceleration, and energy-audit values reached from the changed motion;
- semantic: particle kinetic/total energy now use the frozen normalized mass-energy definitions;
- unchanged: ternary state, latency, `tau`, phase, identity labels, strong field, and weak field.

Reconciled hashes are `minimal=450fca908f536e36`, `default=ca1aada0203f0229`, `L9=ac32d7b46e718b38`, `reflective_flux=14eb33a180b3d319`, `dispersal=7d6315d321c7ced7`, `absorbing=b5f1e6b2a1713f35`, `reflective_move=923f310b5af1e2ec`, and `GPU=26eb5cacd8b49734`.

## 5. Reproducibility record

Platform: Windows 11 host; WSL2 Ubuntu 22.04, Linux `6.6.87.2-microsoft-standard-WSL2`; NVIDIA GeForce RTX 5090, driver `610.47`. Reference runs forced CPU where the instrument requires it; CUDA comparison runs used WSL2 with `FTD_FORCE_GPU` unset.

Hashes:

- causal test source: `a164eed0406b110cdf62b09985ba64ab6fafd330d82c7e9319809d984d52dbdc`
- exact verifier: `3bb62704d5f77c1ea4f1488129e8cc4bef3e10fb93610e2c5b0bfb849ee7f2ad`
- causal interface: `705501451985333d64128a0896216a137a2d836673aeb02e9ace6de4f2e53aa2`
- WSL2 `test_causal_normalization` binary: `4ba9b21b32795b403f56ad48e6dd6a6115240b5dd0f1d7a37f9893727648046f`
- WASM `ftd_core.wasm`: `0b9c04210a09b92fb638cfae7884280e870941f57bd9d86b9a115abe317ef011`

Principal commands:

```text
cmake --build engine/build_wsl --target test_causal_normalization ... --parallel 24
ctest -R <targeted-native-regex> -j 8 --timeout 1800 --output-on-failure
ctest -R <targeted-gpu-regex> -j 1 --timeout 1800 --output-on-failure
cmake --build engine/build_wasm --config Release --target ftd_wasm --parallel 24
python scripts/proofs/verify_causal_normalization_mass_roles.py
npx playwright test scale0-conservation-panel.spec.js -g "WASM vacuum diagnostics"
npx playwright test scale0-scenario-telemetry-contract.spec.js
ctest -j 24 --timeout 1800 --output-on-failure  # aggregate interrupted; no full verdict
```

The concise raw verification record is tracked at `engine/results/causal_normalization_2026-07-21/verification.txt`.

## 6. Licensed conclusion

The executed engine conforms to the exact selected raw-lattice causal and mass-role subcontracts covered by G2–G8 and G10. Because G9 is incomplete, this result does not license the full `[THEOREM — current engine implementation conforms ...]` wording reserved for `CONSISTENT-RAW`.

It derives neither `K_B`, an electron mass, MeV units, inertial–gravitational equivalence, physical covariance, confinement-generated mass, nor a common gravity source. FTD-0015, FC-2, FC-W, the clock hypothesis, FTD-0252/0268, FTD-0400, and FTD-0401 retain their prior epistemic status. NCEMC remains deferred.
