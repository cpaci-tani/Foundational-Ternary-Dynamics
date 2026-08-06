# PREREG — Causal normalization and mass-role reconciliation

**Prospective claim ID:** FTD-0402 (registry rechecked at lock time; FTD-0401 is the current maximum).  
**Tag:** `[PRE-REGISTRATION — ENGINE CONTRACT TEST]` · LOCK-STD v1 · git tag `preregister-causal-normalization-mass-roles-v1`.  
**Scope:** the current `RenderBridge` raw-lattice implementation, its CPU/GPU backends, diagnostics, and public WASM audit surface. This campaign adds no Framework Commitment, confinement law, mass derivation, or calibration. The clock/bandwidth hypothesis remains `[AXIOM]`.

## 1. Frozen question and coordinate convention

Every stored `Voxel::velocity` component is raw lattice displacement in nodes per tick. Freeze

\[
C=C_{\rm SPEED},\qquad
\beta^2=\frac{|u|^2}{C^2},\qquad
f=1-L^2,\qquad
B=\beta^2+L^2.
\]

The selected transport domain is `B < 1`. On that domain,

\[
\frac{d\tau}{dt}=\sqrt{\max(1-B,0)},\qquad
\gamma_{\rm FTD}=\frac{1}{\sqrt{1-B}},\qquad
u_{\max}=C\sqrt{\max(f,0)}.
\]

`bandwidth_used` is the within-lapse transport fraction `beta^2/f` and reaches one at the selected boundary. The full causal budget `B` is exposed separately. This is an implementation of the existing clock/bandwidth axiom, not a substrate theorem. The historical moving-Schwarzschild `beta < f` identity is retired as a current-engine claim; no Schwarzschild-exact or general-covariance claim is tested here.

Question: can the CPU, GPU, voxel, force, movement, proper-time, Born–Infeld, energy-audit, and public WASM paths be made exactly consistent with this one raw-coordinate contract while retaining deterministic and compatible engine behavior?

## 2. Frozen mass-role contract

The existing numerical calibration `K_B` is separated into explicit roles:

```text
M_INERTIAL      = K_B
E_REST          = M_INERTIAL * C_SPEED^2 = K_B/3
M_GRAVITATIONAL = K_B
M_REST          = M_INERTIAL  // compatibility alias only
```

`M_GRAVITATIONAL = M_INERTIAL` remains an imposed numerical choice. It is not an equivalence-principle derivation or a common stress–energy construction. No production consumer may retain `M_REST`; cluster inertia must consume `M_INERTIAL`, CPU/GPU latency Poisson must consume `M_GRAVITATIONAL`, and the imposed de Broglie frequency must be tied explicitly to `K_B`.

For flat-space particle diagnostics, with `gamma_0=(1-beta^2)^(-1/2)`, freeze

\[
E=\gamma_0E_0,\qquad
\mathbf P=\gamma_0M_{\rm INERTIAL}\mathbf u,\qquad
K=(\gamma_0-1)E_0,
\]

and require the exact identity

\[
E^2=E_0^2+C^2|\mathbf P|^2.
\]

For valid `B < 1`, the selected Born–Infeld pair is

\[
\mathcal L_{\rm BI}=-E_0\sqrt{1-B},\qquad
\mathcal H_{\rm BI}=\frac{E_0f}{\sqrt{1-B}}.
\]

These are implementation definitions under the frozen axiom. They do not derive `K_B`, an electron mass, MeV units, or a confining mass gap.

## 3. Frozen implementation surface

One CUDA-safe causal-kinematics interface must be the source of truth for CPU, GPU, `Voxel`, proper time, Born–Infeld, force integration, and movement.

1. Remove device-side `tau` accumulation and the distinct `C_SPEED*f` cap. Advance `tau` exactly once in the common host post-pass.
2. Accumulate all GPU force contributions before one common momentum integration. Base, color, Yukawa, and exchange additions may not bypass the causal budget.
3. At movement entry, project only an externally injected or directly mutated out-of-budget velocity inside `u_max`; count the projection. Ordinary force evolution must report zero such projections.
4. Extend `EnergyAudit` with `particleRestEnergy`, `particleEnergy`, `particleMomentum`, and `dynamicEnergy`. `particleKE` is the exact normalized kinetic energy. `totalEnergy` is an explicitly incomplete sum of accounted channels until NCEMC supplies interaction energies.
5. Append new fields to the fixed WASM audit view without reordering any existing index. Conservation charts consume `dynamicEnergy`; rest and accounted energy are displayed separately.
6. Production APIs and toggle defaults remain compatible. No confinement energy or field-only gravity source is introduced.

Frozen energy aggregation:

```text
particleRestEnergy = sum(E_REST over manifested particle sites)
particleEnergy     = particleRestEnergy + particleKE
particleMomentum   = vector sum(gamma_0 * M_INERTIAL * u)
dynamicEnergy      = fieldEnergy + waveEnergy + particleKE
totalEnergy        = fieldEnergy + waveEnergy + particleEnergy
```

Strong, weak, gravity, damping, boundary, and other interaction energies remain excluded unless already represented in the named accounted channels. This incompleteness is a required diagnostic label, not an implementation failure for this campaign.

## 4. Exact anchors and correctness gates

All gates precede outcome adjudication.

| Gate | Frozen requirement | Failure |
|---|---|---|
| G1 | preregistration census GREEN at tag cut and FTD-0402 is the next registry ID | lock cannot be cut |
| G2 | exact anchors: `C^2=1/3`; rate squared is `0` at `u=C,L=0`, `3/4` at `u=C/2,L=0`, and `1-L^2` at rest | INVALID |
| G3 | rate/gamma reciprocity, `bandwidth_used=1` at the selected speed boundary, full budget `B`, and `u_max` agree with §1 | INVALID |
| G4 | Born–Infeld/Hamiltonian Legendre consistency, `E_0=M C^2`, and the flat energy–momentum invariant pass within frozen floating-point tolerances plus exact algebraic verifier checks | INVALID |
| G5 | horizons and non-finite inputs do not produce NaN propagation or out-of-budget movement | INVALID |
| G6 | external over-speed state is projected and counted; ordinary base/color/Yukawa/exchange force evolution has zero movement-entry projection events and remains `B<1` | INVALID |
| G7 | CPU/GPU one-tick and sixteen-tick parity covers `tau`, phase, evaporation hazard, causal budget, energy, and momentum | INVALID |
| G8 | relevant voxel, Born–Infeld, Lorentz, Lagrangian, de Broglie, cluster-inertia, energy-ledger, WASM-contract, GPU-parity, and movement tests pass twice deterministically | INVALID |
| G9 | WSL2 build, golden 7/7, GPU parity, web contract tests, exact Python verifier, documentation/link checks, `git diff --check`, census GREEN, and full CTest pass | PARTIAL if the implemented exact anchors and causal enforcement remain valid; otherwise INVALID |
| G10 | no production use of `M_REST`; existing WASM indices retain their meanings and positions; production APIs and default toggles remain compatible | INVALID |

The source and binary SHA256 values, git SHA, platform, effective toggles, and exact command lines are recorded in the result document. Any golden hash delta must be explained field-by-field and match only the frozen causal/mass deltas.

Vacuity controls: the old implementation must fail at least the `u=C,L=0` clock anchor; the external over-speed control must increment the projection diagnostic; disabling the mutation must leave that counter at zero; a deliberately separated CPU/GPU legacy `tau` path would fail the sixteen-tick parity gate. Tests that merely restate one implementation expression without recomputation do not satisfy G2–G7.

## 5. Frozen outcomes and precedence

Apply correctness gates first, then the following mutually exclusive ordered outcomes:

1. **CONSISTENT-RAW:** every gate G2–G10 closes.
2. **PARTIAL:** G2–G8 establish the selected raw causal contract, but G9 leaves mass-role propagation, ledger/UI reconciliation, backend parity, or bounded full-suite verification incomplete. The result must name each incomplete item.
3. **INVALID:** any exact-anchor, causal-enforcement, determinism, compatibility, or non-vacuity gate fails.

Precedence is `INVALID` over `PARTIAL` over `CONSISTENT-RAW`, irrespective of narrative ordering. A backend parity failure is INVALID when it contradicts G7; a bounded environmental inability to execute an otherwise unchanged peripheral test may be PARTIAL only when G2–G8 and the relevant targeted tests pass and the result books the limitation explicitly.

Partition proof: after gate evaluation, a required validity failure selects INVALID. Otherwise, any explicitly permitted incomplete G9 reconciliation item selects PARTIAL. Only an empty incomplete set selects CONSISTENT-RAW. No valid execution can select two outcomes.

## 6. Licensed interpretation

CONSISTENT-RAW licenses only `[THEOREM — current engine implementation conforms to the selected raw-lattice causal and mass-role contract]`. It establishes internal coherence of the present implementation. It does not establish that the clock budget is substrate-derived, that `K_B` is derived, that inertial and gravitational mass have a common native origin, that confinement creates mass, or that any MeV prediction follows.

PARTIAL licenses only the exact subcontracts whose gates close. INVALID licenses no corrected-engine claim.

Under every outcome, FTD-0015, FC-2, FC-W, the clock hypothesis, FTD-0252/0268, FTD-0400, and FTD-0401 retain their existing epistemic status unless an independently authorized result says otherwise. NCEMC and a strong Hamiltonian are deferred. Only CONSISTENT-RAW may close `§12-cnorm` and make FTD-0403/NCEMC the next admissible arc.

## 7. Frozen execution and reconciliation protocol

Prospective instruments:

- `engine/tests/test_causal_normalization.cpp` and the relevant existing engine targets;
- `scripts/proofs/verify_causal_normalization_mass_roles.py`;
- WSL2 canonical build `engine/build_wsl`, CPU reference and WSL2 CUDA backend where available;
- web/WASM contract tests and documentation/link checks only.

No numerical coincidence search, target-value fit, substitution identity, or tag promotion by narrative is permitted. Lock, implementation/tests, result, and reconciliation are separate commits.

The reconciliation commit must update the LEDGER, open-items tracker, engine specification, framework, Lagrangian, checklist, UI, META index, and preregistration manifest. Historical ledger text remains as provenance with explicit supersession notes. On CONSISTENT-RAW, `§12-cnorm` closes and FTD-0403 is reserved only as the next available NCEMC arc, not allocated automatically.

## 8. Execution window and executor

Executor: the current Codex repository session on branch `codex/ftd-0402-causal-normalization`. Window: from tag creation through `2026-07-24T18:43:32Z` (72 hours). A missed execution or unbooked frozen verdict creates preregistration debt and blocks a successor lock.

**LOCKED CONTENT ENDS HERE.** The immutable git commit/tag plus the preregistration SHA256 recorded in the manifest bind this text. Any normative edit requires v2.
