# FTD-0428 — Integrated Matched Maxwell/Gauss Branch

> **FTD-0429 native successor (2026-07-23):** independently of this selected
> branch, the unchanged production wave/coupling sector has now been shown to
> generate a finite infrared polarity susceptibility `(div J)_k/s_k -> 3G_C`
> with both Gauss mechanisms off. That licenses restricted coarse-scale native
> charge, but not the microscopic `U(1)`, reaction, photon, force, or common-
> cone claims excluded below.

**Status:** [THEOREM — SELECTED FINITE-LATTICE COMPLEX/MINIMUM] +
[MEASURED — INTEGRATED ENGINE COMPATIBILITY] + [SELECTED ENGINE EXTENSION]
**Verdict:** `A_SELECTED_INTEGRATED_PROJECTION_FREE_MAXWELL`
**Date:** 2026-07-23
**Pre-registration:**
[`PREREG_MATCHED_MAXWELL_INTEGRATION_v1.md`](../10_eft_program/preregistrations/PREREG_MATCHED_MAXWELL_INTEGRATION_v1.md)

## 1. Result

The FTD-0427 oriented-face mechanism now runs inside the production
`RenderBridge` tick behind the default-off, CPU-scoped
`matched_gauss_dynamics` toggle. It has three coupled pieces:

\[
(D D^T)\phi=\rho,\qquad E=D^T\phi
\]

for one-time minimum-energy initialization, followed by

\[
B^{n+1/2}=B^{n-1/2}-c\,\Delta t\,C^T E^n,
\]

\[
E^{n+1}=E^n+c\,\Delta t\,C B^{n+1/2}-K^{n+1/2}.
\]

The source current `K` is extracted from actual production movement. The
centered face field is mirrored into `Voxel::flux` after every selected tick.
No per-tick Poisson solve or Gauss projection occurs.

All preregistered gates passed under MSVC and WSL2 GCC at `L=32,64`. This is
an integrated selected mechanism, not native electromagnetic emergence.
FTD-0421 still closes additive charge conservation for the complete frozen
reaction set in the tested feature basis.

## 2. Finite-lattice statements proved by construction

On the periodic matched complex, the backward face divergence `D`, edge curl
`C`, and implemented transpose `C^T` obey

\[
D C=0,
\qquad
\langle E,C B\rangle=\langle C^T E,B\rangle.
\]

For neutral `rho`, conjugate gradients solves the positive semidefinite scalar
problem on the zero-mean subspace. `E=D^T phi` is orthogonal to every
divergence-free addition and therefore minimizes `1/2 ||E||^2` subject to
`D E=rho`. The exact unit test checks the transpose identity, Gauss relation,
longitudinal condition, and lower energy relative to the deterministic path
string.

For source-free staggered evolution, the implemented map preserves

\[
H_h={1\over2}\left(\|E\|^2+\|B\|^2
-c\Delta t\langle B,C^T E\rangle\right)
\]

to floating-point accumulation error. This is the discrete modified energy of
the selected update, not a derivation of the physical electromagnetic
Hamiltonian.

## 3. Production integration contract

The table-registered toggle defaults false and is excluded from bulk profiles.
When enabled, validation requires periodic, single-substrate, conservative
movement isolation. Legacy wave/coupling, damping, manifestation, reactions,
forces, Gauss projectors, alternate integrators, boundaries, and gauge-link
relaxations are rejected. A GPU-backed bridge copies its state to the host and
explicitly falls back to CPU. Unlike ordinary advisory toggle validation, an
invalid FTD-0428 combination throws even when `strict_validation=false`; a
process-level `FTD_FORCE_GPU` override that prevents CPU fallback also rejects
initialization/execution. The branch rejects `dt!=1`, preserving the stability
and modified-energy map actually frozen by the campaign.

The branch fails closed unless explicitly initialized. Each tick snapshots
ternary state immediately before movement, routes the before/after difference
through the existing finite-volume history extractor, rejects reaction-bearing
histories, advances the face/edge fields, and mirrors centered `E` into
`Voxel::flux`. This preserves the legacy default tick because the branch is
dead when its toggle is false.

## 4. Campaign and observed bounds

Four run-of-record datasets contain 11 summary rows each: four static surface
radii, six polarity/direction movement arms, and one transverse-wave arm.

| quantity | worst observed | frozen gate |
|---|---:|---:|
| CG iterations | `211` | `<=12L` (`768` at `L=64`) |
| CG residual | `9.15e-13` | `<=1e-10` |
| initialized `max|C^T E|` | `4.17e-17` | `<=1e-10` |
| all-arm `max|D E-rho|` | `9.15e-13` | `<=1e-9` |
| surface/telescope error | `3.86e-11` | `<=1e-9` |
| minimum/string energy ratio | `0.00538`–`0.0111` | strictly `<1` |
| static relative modified-energy drift | `4.44e-16` | `<=1e-9` |
| wave relative modified-energy drift | `2.51e-13` | `<=1e-8` |
| voxel/face mirror residual | `0` | `<=1e-9` |
| movement face-current count per arm | `7` | `>=5` |
| reaction L1 | `0` | exactly zero |
| final stationary current | `0` | exactly zero |

The compact transverse disturbance grows from Chebyshev support radius `1` to
`13` by tick 12. At tick 32 it reaches radius `16` on `L=32` and `32` on
`L=64`, without ever appearing beyond `initial_radius+tick`. The latter values
touch the periodic half-box and must not be read as infinite-volume data.

MSVC/GCC scalar differences are at most `5.81e-15` for `L=32` and
`2.71e-14` for `L=64`, below the frozen `1e-9` compiler gate. The source lock
passes 22/22 and the result verifier passes 90/90.

## 5. What this resolves

FTD-0427's two immediate defects are resolved at selected-mechanism scope:

1. the flux string is replaced by a minimum-energy periodic Gauss dressing;
2. the transverse challenge is replaced by a coupled Faraday/Ampere-like
   staggered evolution with a conserved discrete modified energy.

The selected state is no longer a disconnected observer sidecar: it owns the
field update in an isolated live tick and exports its centered electric field
through the engine's normal voxel surface.

## 6. What remains open

1. No force consumes the selected field. A `1/r^2` static surface flux follows
   from Gauss plus minimum energy, but a Coulomb force law and its normalization
   are untested.
2. No stable manifested matter pole, response residue, or common matter/light
   cone has been measured in this branch.
3. Reactions are deliberately rejected. The complete production event set has
   no nontrivial additive charge in the frozen FTD-0421 feature basis.
4. The one-time global solve is selected initial data preparation. The theory
   has not shown a local dynamical process that creates this dressing.
5. The classical transverse field has not produced gauge redundancy, Ward
   identities, quantization, a photon state, or radiative protection.
6. Positivity of the modified quadratic form at the saturated CFL boundary
   and long-time behavior beyond 32 ticks need a separate spectral gate.
7. The branch is CPU-only and is not the shipping default physics profile.

## 7. Next licensed gate

The next native-first step is an on-shell response campaign within this
selected branch: measure the transverse dispersion and residue, couple a
read-only probe to a stable manifested structure, and determine whether both
objects possess identifiable low-width poles with a common continuum
intercept. No force feedback or reaction repair is licensed by this result.

## 8. Artifacts

- operator/state: `engine/include/ftd/eft/matched_gauss_transport.h`
- implementation: `engine/src/eft/matched_gauss_transport.cpp`
- production toggle/API: `engine/include/ftd/term_toggles.h`,
  `engine/include/ftd/render_bridge.h`, `engine/src/render_bridge.cpp`
- exact integration test: `engine/tests/test_matched_maxwell_integration.cpp`
- campaign: `engine/tests/campaign_matched_maxwell_integration.cpp`
- source lock: `scripts/proofs/matched_maxwell_integration_lock.json`
- verifiers: `scripts/proofs/proof_matched_maxwell_integration_{lock,results}.py`
- run records: `engine/results/ftd_0428/`
