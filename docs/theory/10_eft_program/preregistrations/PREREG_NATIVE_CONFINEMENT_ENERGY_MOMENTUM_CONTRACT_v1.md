# PREREG — Native Confinement Energy–Momentum Contract feasibility

**Prospective claim ID:** FTD-0405 (registry maximum FTD-0404; preregistration census GREEN before lock).  
**Tag:** `[PRE-REGISTRATION — SCOPED CONTRACT FEASIBILITY / OBSTRUCTION]` · LOCK-STD v1 · git tag `preregister-native-confinement-energy-momentum-contract-v1`.  
**Question:** can the current RenderBridge colour-force path satisfy NCEMC-1–4 without adopting a new force law, energy-zero calibration, framework type, or mass target?

## 1. Frozen scope

This arc audits only the current RenderBridge `color_forces` interaction on CPU and GPU. ParticleEngine is declared out of scope under the option explicitly allowed by NCEMC-1 because its long-range law differs. The Yukawa `strong_force` path, SU(3) gauge-link relaxation, target-blind particlehood, mass spectroscopy, NCEMC-5, and any MeV calibration are also out of scope.

Production force, movement, gravity, state-transition, toggle, constant, and public-API behavior is frozen. Permitted changes are one exact C++ instrument, one recomputing Python verifier, result/provenance records, CMake test registration, and documentation. A later production construction is admissible only under a fresh lock if this feasibility gate does not return an obstruction.

The current pair force is frozen as

\[
\mathbf F_{i\leftarrow j}=-c_f g(r)\,\hat{\mathbf r}_{ij},
\]

where `c_f=+1/2` for equal nonzero colours and `c_f=-1` for different nonzero colours, and

\[
g(r)=\begin{cases}
\alpha_s(r)/r^2,&1\le r<3,\\
\alpha_s(r)/(3r),&3\le r<8,\\
\alpha_s(r)r/64,&r\ge8.
\end{cases}
\]

The corresponding conservative family, if CPU/GPU source parity succeeds, is

\[
U_\kappa(r)=-c_f\int_{r_*}^{r}g(s)\,ds+\kappa.
\]

No value of `r_*` or `kappa` is selected; changing either is the same additive-zero freedom.

## 2. Frozen NCEMC questions

1. **NCEMC-1 / Hamiltonian existence:** confirm that the active CPU/GPU colour force is the radial variation of the displayed pair-potential family. This establishes existence only up to `kappa`; it does not install a canonical engine Hamiltonian.
2. **NCEMC-2 / exact work:** test the actual one-tick map, including the sub-voxel `remainder` as part of effective position, for exact exchange between normalized particle kinetic energy and the force-derived potential.
3. **NCEMC-3 / momentum:** test equal-and-opposite force and complete manifested-particle momentum closure for the isolated two-body colour arm. This is not a proof of a full ordinary-field/strong-field translation current.
4. **NCEMC-4 / gravity:** decide whether the force law and existing axioms fix an absolute strong energy density usable as gravitational charge. `U` and `U+kappa` generate identical dynamics but shift total energy and any gravity source by `kappa` per active pair; with pair number mutable, that shift is not a globally irrelevant constant.

No field-energy localization rule (endpoint split, link/string deposition, or another partition) may be adopted in this arc. Such a choice is additional order-bearing data for the stress distribution even if its far-field monopole is fixed.

## 3. Exact adequacy anchor

Use `L=33`, `dt=1`, CPU-forced reference execution, periodic geometry, and two initially resting `+1` manifested voxels at `(8,16,16)` and `(24,16,16)` with different nonzero colour labels. Enable only `forces`, `movement`, and `color_forces`; disable wave propagation, coupling, damping, genesis, evaporation, Gauss projection, gravity, latency, Coulomb, Lorentz, weak, exchange, pair production, and every external drive.

At separation `r_0=16`, the running coupling is in its frozen non-perturbative cap `alpha_s=1`, so the attractive harmonic arm has

\[
|F|=r_0/64=1/4,\qquad U_\kappa(r)=r^2/128+\kappa.
\]

The instrument must check:

- force diagnostics are exactly equal and opposite with magnitude `1/4`;
- neither voxel changes lattice site on the first tick;
- remainders are equal and opposite and define `r_1=16-2|u_1|`, so the comparator does not hide sub-voxel motion;
- total manifested-particle momentum is zero within the bit/tolerance contract;
- `Delta K + U(r_1)-U(r_0)` is recomputed from the normalized FTD-0402 kinetic energy and is tested against exact zero;
- a duplicate arm is bit-identical;
- a colour-force-off negative control remains at zero velocity, zero remainder, and zero kinetic change.

A pre-lock scratch calculation was used only to confirm that this is a non-vacuous, finite anchor and to select safe coordinates. It is not result evidence. The committed verifier and C++ instrument are the executions of record.

## 4. Correctness gates

- **G1 source parity:** CPU and CUDA implement the same `c_f`, periodic displacement, running coupling, and three force regimes.
- **G2 anchor validity:** the two sites persist, remain distinct and interior, stay in the `r>=8` capped harmonic regime, and experience no causal projection, collision, boundary event, state transition, or unrelated force.
- **G3 effective-position comparator:** potential change uses lattice coordinate plus signed remainder; a lattice-coordinate-only comparison is invalid.
- **G4 momentum non-vacuity:** both forces and both momenta are nonzero individually before cancellation.
- **G5 zero-point theorem:** the verifier symbolically checks `d(U+kappa)/dr=dU/dr` and that total/gravitating energy shifts by `kappa*N_pairs`.
- **G6 epistemic ceiling:** no mass value, CODATA datum, `m_e`, `M_REST`, `alpha^11`, fit residual, numerical near-miss search, new calibration, or framework type may enter the verdict.
- **G7 determinism:** the execution-of-record C++ target passes twice with identical observation lines.

## 5. Frozen outcomes and precedence

After correctness gates, precedence is:

1. **INVALID:** any source-parity, regime, persistence, negative-control, determinism, or comparator gate fails.
2. **NONCONSERVATIVE-FORCE:** the active CPU/GPU force cannot be represented by the frozen radial potential family.
3. **DOUBLE-OBSTRUCTION:** Hamiltonian existence and isolated momentum closure succeed, but the actual tick has nonzero work residual and the additive-zero freedom leaves gravitational charge underdetermined.
4. **PHASE-SPLIT-OBSTRUCTION:** exact work fails while an existing substrate rule fixes the absolute gravitational energy zero.
5. **ZERO-POINT-OBSTRUCTION:** exact work closes while gravitational charge remains additive-zero dependent.
6. **FULL-CONTRACT:** NCEMC-1–4 all close without adding a calibration, localization rule, force law, or framework type.

For an obstruction outcome, the result is scoped to the current direct pair-force ontology and tick splitting. It does not prove that an energy-conserving local strong-field formulation is impossible. It blocks crediting the present colour force with inertial or gravitational mass and keeps NCEMC open pending separately authorized architecture.

## 6. Frozen verification surface

No full CTest, WASM build, web suite, numerical coincidence search, or mass campaign is permitted.

1. Run the new exact Python verifier.
2. Build and run the new `ncemc_feasibility` C++ target twice.
3. Run only `confinement`, `asymptotic_freedom`, `force_diag_parity`, `gpu_parity`, and `causal_normalization` through WSL2 with `FTD_FORCE_GPU` unset.
4. Run `ctest -L golden -j 24 --output-on-failure` only if production engine sources change. Under the frozen diagnostic-only boundary, goldens are not required.
5. Run `git diff --check`, the theory-index link checker, the component/source static verifier, and `tools/preregister_census.py` to GREEN.

## 7. Commits and reconciliation

Commit lock, instrument, result, and reconciliation separately. Record git SHA, source/binary/verifier SHA256, platform, GPU, effective toggles, commands, both observation records, and whether production sources changed. Update LEDGER, tracker, canonical Lagrangian reference, META index, EFT checklist, and preregistration manifest. The next registry ID after booking is FTD-0406.

## 8. Executor and window

Executor: current Codex repository session on `codex/ftd-0405-ncemc`.  
Window: tag creation through `2026-07-24T22:39:29Z` (72 hours).  
Platform: Windows 11 host; WSL2 Ubuntu 22.04 canonical `engine/build_wsl`; RTX 5090 for CUDA parity.
