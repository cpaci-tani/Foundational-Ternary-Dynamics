# FTD-0405 — Native Confinement Energy–Momentum Contract feasibility

**Frozen outcome:** **DOUBLE-OBSTRUCTION**

**Scope/tag:** **[SCOPED NO-GO — current RenderBridge direct pair-force tick and no-new-calibration domain]**

**Lock:** `preregister-native-confinement-energy-momentum-contract-v1` at commit `2d74956a425d2758cd8942b87b79e83d20104c9e`; preregistration SHA256 `86c2418062711d3e9e308533c29ae87530e1f35c71a049ad58bf75f6d1ccb849`.

## Verdict

The present RenderBridge colour force clears two limited gates but fails the two gates needed to identify confinement energy with inertial and gravitational mass.

1. The CPU/GPU interaction is a central radial force, so a conservative pair-potential family exists:

   \[
   U_\kappa(r)=-c_f\int_{r_*}^{r}g(s)\,ds+\kappa.
   \]

   This closes mathematical existence only up to an additive constant. It does not mean the engine currently evaluates a canonical `H_strong`.

2. In the isolated frozen two-body arm, forces and normalized manifested-particle momenta are nonzero individually and cancel exactly. This is a valid two-body momentum result, not the complete field-plus-particle translation current required by NCEMC-3.

3. Exact work exchange fails on the actual tick map. At `r_0=16`, the capped harmonic arm gives `|F|=1/4` and `U=r²/128+kappa`. After one tick, including each voxel's signed sub-lattice `remainder` in effective position,

   ```text
   particle KE       = +0.10586117245549945
   potential change  = -0.18227154956055602
   work residual     = -0.076410377105056576
   ```

   The residual is finite, deterministic, and nonzero. Therefore the current force-kick/movement splitting does not exchange particle kinetic energy exactly with the force-derived potential. The result is not an artifact of treating unmoved lattice indices as fixed positions: the comparator explicitly includes `remainder`.

4. Gravity remains underdetermined even if the integration error were repaired. `U` and `U+kappa` generate identical colour forces and momenta, but shift total energy and any gravitational charge by `kappa` per active pair. Because manifestation and evaporation can change the number of pairs, this is not merely one fixed global offset. No current FTD rule selects `kappa`, and this arc forbids introducing an energy-zero calibration or a link/endpoint localization rule by fiat.

The frozen precedence therefore returns **DOUBLE-OBSTRUCTION**, not FULL-CONTRACT.

## NCEMC disposition

| Requirement | Result |
|---|---|
| NCEMC-1 — one Hamiltonian | **PARTIAL:** radial potential family exists, but no canonical additive zero or installed engine object |
| NCEMC-2 — exact work | **BLOCKED:** actual one-tick work residual is nonzero |
| NCEMC-3 — total momentum | **PARTIAL:** isolated manifested two-body momentum closes; full translation current remains absent |
| NCEMC-4 — same energy sources gravity | **BLOCKED:** absolute energy zero and local stress distribution are not fixed |
| NCEMC-5 — target-blind invariant | **NOT ADMISSIBLE** until 1–4 close |
| NCEMC-6 — calibration boundary | Unchanged; FTD-0096 still forbids a first-principles MeV scale |

ParticleEngine and the Yukawa `strong_force` path were frozen out of scope. ParticleEngine's constant long-range force remains different from RenderBridge's harmonic arm.

## Correctness and verification

- Exact verifier: A1–A7 and S1–S9 pass.
- New C++ instrument: 24/24 checks, twice; observation lines identical.
- Negative control: colour force off gives zero velocity, remainder, and kinetic change.
- Neighboring regressions: `confinement_test`, `asymptotic_freedom`, `force_diag_parity`, `gpu_parity`, and `causal_normalization` pass 5/5 with `FTD_FORCE_GPU` unset.
- No production engine source changed, so the lock did not require goldens.
- No full CTest, WASM/web build, mass campaign, near-miss search, or substitution search ran.

The preregistration used the shorthand `confinement`; the existing registered CTest name is `confinement_test`. The intended existing target was run and passed; no test definition or physics was changed.

## Licensed consequence

This result licenses only:

> **[SCOPED NO-GO]:** the current direct colour pair-force path cannot satisfy NCEMC-1–4 without both an energy-conserving update architecture and additional data fixing the strong-energy zero/localization used by gravity.

It does not prove confinement-generated mass impossible. A local strong-field action could in principle fix energy transport, momentum, stress localization, and a vacuum reference together. Building that architecture would be a new separately locked task and, because the current axioms do not select the additive zero/localization, requires explicit owner authorization rather than narrative promotion.

FTD-0025 remains measured at an inserted/selected coupling, FTD-0096 remains closed theorem-negative for the mass unit, FTD-0250 remains imposed, FTD-0399 remains invalid at G2, and FTD-0400's SPLIT-BOOKKEEPING finding remains in force. No mass, gravity, equivalence-principle, or Standard Model claim is promoted.

## Reproduction record

- Instrument commit: `726c2dafa2f74e727475db2984fba6b62e67aeb7`
- Platform: WSL2 Ubuntu 22.04, Linux `6.6.87.2-microsoft-standard-WSL2`, NVIDIA GeForce RTX 5090, driver `610.47`
- Test source SHA256: `9614087978b3a541f1cc89f3065b372a103986fe0b2548b358c94d95800909d7`
- Test binary SHA256: `bdb49a8865124388e2eb9592803f1c293e203b6c4f1eb71a709e2d75a3115b16`
- Exact verifier SHA256: `41c7961527027420cf2b65c7734ad9f3dd0cf3c1ae0e92d05c06f6f81e649ff0`
- Raw command and observation record: `engine/results/ncemc_feasibility_2026-07-21/verification.txt`
