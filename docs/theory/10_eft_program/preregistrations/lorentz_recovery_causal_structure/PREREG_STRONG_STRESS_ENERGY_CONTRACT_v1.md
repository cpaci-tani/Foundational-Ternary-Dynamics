# PREREG — Strong stress–energy contract v1

**Prospective claim ID:** FTD-0406 (registry maximum FTD-0405; preregistration census GREEN before lock).  
**Tag:** `[PRE-REGISTRATION — OWNER-AUTHORIZED SELECTED ARCHITECTURE]` · LOCK-STD v1 · git tag `preregister-strong-stress-energy-contract-v1`.  
**Question:** after the explicit owner authorization on 2026-07-21, can the current RenderBridge colour interaction be given one energy-conserving CPU update, a fixed vacuum zero, a local string stress allocation, and a gravity source using the canonical `C_SPEED`, without importing a mass target or relabelling the choices as derivations?

## 1. Frozen epistemic boundary

FTD-0405 returned `DOUBLE-OBSTRUCTION`: the direct colour force admitted a radial potential family but the tick failed exact work, while the additive energy zero and local stress distribution were not selected. The owner has now explicitly authorized the three missing choices:

1. an energy-conserving strong-sector update;
2. a native vacuum-energy-zero convention;
3. a local stress-energy allocation.

These choices are **not** substrate theorems. In this arc they are recorded as `[IMPOSED — owner-authorized numerical architecture]` for the update and `[SELECTION — owner-authorized convention]` for the vacuum reference and localization. No Framework Commitment, Standard Model mass, `m_e`, `M_REST`, `alpha^11`, CODATA value, fit residual, or numerical near-miss search is admissible.

Success can establish only a scoped current-engine contract. It cannot derive a rest-mass scale, MeV conversion, equivalence principle, general covariance, local Yang–Mills field action, or the physical correctness of the selected stress localization. FTD-0096 and the imposed equality `M_GRAVITATIONAL=M_INERTIAL=K_B` remain unchanged.

## 2. Frozen interaction Hamiltonian and vacuum zero

For every unordered pair of manifested voxels carrying nonzero colour, retain the existing CPU colour force

\[
\mathbf F_{i\leftarrow j}=-c_f g(r)\,\hat{\mathbf r}_{ij},
\qquad
c_f=\begin{cases}+1/2,&c_i=c_j,\\-1,&c_i\ne c_j,\end{cases}
\]

with the existing periodic minimum-image distance, `r=max(r,1)`, running `alpha_s_lattice(r)`, and three-regime `g(r)` frozen by FTD-0405. Adopt the pair energy

\[
U_{ij}(r)=-c_f\int_1^r g(s)\,ds.
\]

The selected vacuum convention is:

- an empty set of coloured pairs has `U_strong=0`;
- every pair has `U_ij(1)=0`;
- no target mass or external energy datum fixes this zero.

The numerical primitive must split at `r=3` and `r=8`, use the same `alpha_s_lattice` function as the force, use the exact capped harmonic primitive for intervals wholly in `r>=8`, and use one deterministic fixed quadrature contract below `r=8`. The evaluator must be order-independent and finite for every admissible periodic separation.

The scoped Hamiltonian is

\[
H_{\rm strong}=\sum_i\left(\sqrt{E_0^2+C_{\rm SPEED}^2|\mathbf p_i|^2}-E_0\right)
+\sum_{i<j}U_{ij}(r_{ij}),
\qquad E_0=M_{\rm INERTIAL}C_{\rm SPEED}^2.
\]

Rest energy is displayed separately and cancels from the update constraint.

## 3. Frozen energy-conserving update

The v1 update is a default-off CPU projection layered on the existing force and movement proposal. It is scoped to the isolated, flat, collision-free colour sector: `color_forces`, `forces`, and `movement` are on; damping, genesis/evaporation, pair production, Coulomb/emergent/gravity/Lorentz/Yukawa/exchange forces, latency, weak transmutation, triad binding, and absorbing/reflecting particle boundaries are off. Wave fields may be present only as spectators and are excluded from `H_strong`.

At tick start record coloured particle IDs, total physical momentum, `K_0`, `U_0`, and `H_0=K_0+U_0`. Run the existing colour-force momentum kick and movement proposal. If coloured particle identity/topology changed, do not silently manufacture conservation; record a topology failure.

For unchanged topology, hold the proposed positions fixed, convert each proposed velocity to physical momentum, and preserve total momentum by scaling only deviations from the arithmetic mean momentum:

\[
\mathbf p_i(\lambda)=\bar{\mathbf p}+\lambda(\mathbf p_i^{\rm prop}-\bar{\mathbf p}),
\qquad \sum_i\mathbf p_i(\lambda)=\sum_i\mathbf p_i^{\rm prop}.
\]

Select the unique nonnegative `lambda` satisfying

\[
\sum_i K(\mathbf p_i(\lambda))=H_0-U(\mathbf q_{n+1}).
\]

Use deterministic bracket expansion plus a frozen 96-step bisection. The zero-relative-momentum value is the minimum kinetic energy at fixed total momentum. If the requested kinetic energy is below that minimum, non-finite, or cannot be bracketed, record a projection failure; strict validation must surface the failure rather than accepting a false conservation claim. Convert the projected momenta back with

\[
\mathbf u_i=\frac{C_{\rm SPEED}^2\mathbf p_i}
{\sqrt{E_0^2+C_{\rm SPEED}^2|\mathbf p_i|^2}}.
\]

This projection is an imposed energy–momentum integrator, not a derivation from the five substrate postulates. Collision, annihilation, creation, locked-cluster motion, latency, GPU execution, and mixed-force energy exchange are explicitly outside v1 and remain open even on a successful verdict.

## 4. Frozen local stress allocation and gravity coupling

For each unordered pair, choose the shortest periodic displacement with the same tie convention as the force. Let `n=max(1,ceil(r))`. Sample the segment at the `n` cell midpoints and deposit each sample to the surrounding eight voxels by periodic trilinear cloud-in-cell weights. Normalize each sample's weights before deposition. This freezes a symmetric, translation-covariant string localization whose integrated energy is exactly the pair energy up to floating summation tolerance.

Deposit:

- pair energy `U_ij/n` as local `T00_strong`;
- the central-force Irving–Kirkwood pair stress `-d_a F_b/n` in the six symmetric spatial components.

This localization is `[SELECTION]`, imported as a microscopic stress convention. The engine does not claim that it is uniquely forced or that the resulting object is a continuum GR stress tensor.

When the CPU latency Poisson solver is enabled, the same local strong energy must enter gravitational **mass** density as

\[
\rho_{g,\rm strong}=\frac{T^{00}_{\rm strong}}{C_{\rm SPEED}^2},
\]

not as `T00_strong` with an implicit `c=1`. This is required by the already selected FTD-0402 normalization `E=M C_SPEED^2`. The pre-existing generic `field_energy_gravity` path and the CUDA latency solver are outside this v1 change; their broader reconciliation remains open.

The local strong source is computed before the latency solve from the current configuration and recomputed after the projected movement for diagnostics and the next tick. A static locked-pair arm may test gravity sourcing without invoking the collision-free projection.

## 5. Exact adequacy anchors

Use `L=33`, `dt=1`, CPU-forced execution and the FTD-0405 pair at `(8,16,16)` and `(24,16,16)`, initially at rest with different nonzero colours.

The verifier and native target must recompute:

1. `C_SPEED^2=1/3` and `E0=M_INERTIAL*C_SPEED^2`.
2. No coloured pair gives exactly zero strong energy and density.
3. `U_ij(1)=0` for either colour factor.
4. In the capped harmonic arm, a different-colour pair satisfies `U(16)-U(8)=3/2`; a same-colour pair satisfies `U(16)-U(8)=-3/4`.
5. The force remains `+1/4` and `-1/4` on the frozen pair.
6. With the new contract off, the historical FTD-0405 residual remains nonzero.
7. With the contract on, the same proposed positions are retained, total momentum remains zero, both particles move non-vacuously, and `Delta K+Delta U` is at most `1e-12` in absolute value.
8. The sum of local strong energy density equals `U_strong` within `1e-12`; endpoint exchange and an integer periodic translation preserve the integrated energy and stress.
9. A static locked-pair latency arm has a nonzero strong gravity-source delta; the integrated added gravitational mass equals `U_strong/C_SPEED^2` within `1e-12`.
10. Duplicate runs are bit-identical, and topology/projection failure controls are detected rather than reported as conserved.

At least one three-particle, zero-total-momentum arm must pass the same energy and momentum gates so the result is not only a two-body cancellation artifact.

## 6. Correctness and vacuity gates

- **G1 legacy force:** force diagnostics and the contract-off FTD-0405 witness remain unchanged.
- **G2 one energy:** audit, projection, localization, ledger, and gravity source call the same pair-energy implementation; duplicated formulas are invalid.
- **G3 non-vacuous movement:** the projected anchor retains the proposed positions and has nonzero individual momentum.
- **G4 exact scoped exchange:** `|Delta K+Delta U|<=1e-12` for two- and three-body anchors.
- **G5 momentum:** total physical momentum changes by at most `1e-12` and individual momenta are nonzero.
- **G6 local sum:** local `T00` integrates to the Hamiltonian potential within `1e-12`; no hidden residual reservoir is permitted.
- **G7 derived-c gravity conversion:** strong gravitational mass is `T00/C_SPEED^2`; an implicit `c=1` source is invalid.
- **G8 failure honesty:** topology and infeasible-projection controls increment explicit diagnostics and cannot pass the conservation verdict.
- **G9 default neutrality:** with the new toggle off, existing CPU behavior and all seven golden hashes remain unchanged.
- **G10 determinism:** the new target passes twice with identical observation lines.
- **G11 epistemic ceiling:** the result is a selected implementation contract, never a first-principles mass derivation.

## 7. Frozen outcomes and precedence

After correctness gates:

1. **INVALID:** a legacy-force, source-sharing, default-neutrality, determinism, exact-anchor, or comparator gate fails.
2. **TOPOLOGY-BLOCKED:** the frozen collision-free anchors cannot preserve particle identity or the failure control is not detected.
3. **PROJECTION-BLOCKED:** a valid frozen anchor cannot reach the energy surface while preserving total momentum.
4. **SOURCE-PARTIAL:** energy/momentum projection closes but local integration or the `1/C_SPEED^2` gravity source does not.
5. **CPU-SCOPED-CONTRACT:** every frozen v1 gate closes.

`CPU-SCOPED-CONTRACT` closes NCEMC-1–4 only on the explicitly isolated, flat, collision-free CPU domain. It opens—but does not execute—separate GPU, collision/state-transition, mixed-force, and target-blind NCEMC-5 work. It establishes a chosen route by which strong interaction energy gravitates; it does not establish a derived mass scale or physical equivalence principle.

## 8. Frozen verification surface

No full CTest, mass campaign, numerical coincidence search, WASM build, or browser suite is permitted.

1. Run the new exact Python verifier.
2. Build and run the new `strong_stress_energy_contract` target twice through WSL2 `engine/build_wsl`.
3. Run only: `ncemc_feasibility`, `confinement_test`, `asymptotic_freedom`, `force_diag_parity`, `energy_ledger`, `latency_field`, `causal_normalization`, `toggle_matrix`, and `strict_validation`.
4. Because default-off production sources change, run the seven registered golden tests only.
5. Run `git diff --check`, exact index-link checks, a static source-sharing verifier, and `tools/preregister_census.py` to GREEN.

GPU physics/parity, WASM/web, the full CTest suite, and unrelated proof scripts are not part of this CPU v1 lock.

## 9. Commits and reconciliation

Commit lock, implementation/test, result, and reconciliation separately. Record the lock/result SHA256, git SHA, source/binary/verifier hashes, platform, effective toggles, commands, duplicate observations, projection iterations/status, local-density sum, gravity-source sum, and targeted test results. Update the LEDGER, tracker, engine specification, FTD specification, Lagrangian/framework references, EFT checklist, META index, and preregistration manifest. The next registry ID after booking is FTD-0407.

## 10. Executor and window

Executor: current Codex repository session on `codex/ftd-0406-strong-stress-energy`.  
Window: tag creation through `2026-07-24T22:56:39Z` (72 hours).  
Platform: Windows 11 host; WSL2 Ubuntu 22.04 canonical `engine/build_wsl`; CPU-forced v1 execution on Ryzen 9 9950X3D.
