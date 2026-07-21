# Lattice-c, Proper-Time, and Mass-Role Normalization Audit (FTD-0401)

**Date:** 2026-07-21
**Status:** **[SCOPED NO-GO — current engine normalization map]**
**Verdict:** **UNMAPPED-DUAL-NORMALIZATION**
**Scope:** Static source-contract audit of the current RenderBridge CPU/GPU transport, proper-time, Born–Infeld, energy, inertia, and gravity paths. No physical target, fitted residual, numerical near-miss, or new framework commitment is used.

## 0. Verdict

FTD has a derived native causal speed,

\[
c_{\rm lat}=C_{\rm SPEED}=\frac{1}{\sqrt3}\quad\text{voxels/tick}.
\]

The current engine does not use it consistently. The same stored `Voxel::velocity`, explicitly documented as nodes per tick, enters two incompatible conventions:

1. the CPU/GPU force integrators use the raw-lattice budget `v²/C_SPEED² + L² < 1`;
2. the proper-time, Born–Infeld, and legacy gamma paths use `v² + L²`-style formulas with an explicitly separate `c=1` convention.

No map `beta = |v_raw|/C_SPEED` lies between them. Declaring the conventions “intentionally distinct” does not supply such a map because both consume the same numeric field.

The mass side has the same defect. `M_REST = K_B` is consumed literally as rest energy, inertial mass, and gravitational source while `c_lat²=1/3`. Those roles may be related, but they cannot be the same unconverted number in raw tick coordinates unless `c=1` has first been established by a coordinate rescaling. No such rescaling is implemented.

Therefore the current engine cannot form a dimensionally coherent native energy–momentum pair or claim that its `voxel.tau` clock is covariant with the derived causal cone. This is a no-go for the current normalization map, not for a consistent normalization in principle.

## 1. Frozen outcomes

The source audit distinguishes four mutually exclusive outcomes:

- **CONSISTENT-LATTICE:** raw velocity is used everywhere and every kinematic formula carries `C_SPEED` explicitly.
- **CONSISTENT-RESCALED:** raw transport velocity and dimensionless `beta` are distinct, with an explicit `beta=v_raw/C_SPEED` map at every `c=1` consumer.
- **UNMAPPED-DUAL-NORMALIZATION:** the same raw field enters both conventions without a conversion.
- **INVALID:** the source contracts or exact algebraic anchors are represented incorrectly.

All correctness gates pass. The frozen result is **UNMAPPED-DUAL-NORMALIZATION**.

## 2. Exact normalization lemma

Let `u` be the stored raw lattice velocity and `C=1/√3` the causal speed. The transport integrator correctly constructs

\[
\beta^2=\frac{|u|^2}{C^2}.
\]

The legacy matter clock instead consumes `|u|²` as if it were `beta²`. The two readings agree for arbitrary nonzero `u` only if `C²=1`. But FTD fixes `C²=1/3`, so they do not agree.

At the exact transport causal cap `|u|=C` and zero latency:

\[
\left(\frac{d\tau}{dt}\right)^2_{\rm causal}=1-\frac{|u|^2}{C^2}=0,
\]

whereas the implemented clock gives

\[
\left(\frac{d\tau}{dt}\right)^2_{\rm current}=1-|u|^2
=1-C^2=\frac23.
\]

Thus a raw-lattice particle at the declared light-speed cap retains clock rate `√(2/3)` instead of reaching the causal boundary. This is exact rational algebra, not a tolerance judgment.

## 3. Current source contracts

| Role | Current implementation | Consequence |
|---|---|---|
| Native causal speed | `C_SPEED=C_WAVE=1/√3` in [`gauge_couplings.h`](../../../engine/include/ftd/ontic/gauge_couplings.h) | The raw engine coordinate does not have `c=1`. |
| Stored velocity | `Voxel::velocity`: “nodes per G*-tick” in [`voxel.h`](../../../engine/include/ftd/voxel.h) | It is a raw transport quantity, not already `beta`. |
| CPU/GPU force push | Both use `budget=v²/C_SPEED²+L²` in [`phase_forces.cpp`](../../../engine/src/render_bridge_phases/phase_forces.cpp) and [`kernels_forces.cu`](../../../engine/cuda/kernels_forces.cu) | Transport recognizes the derived causal speed. |
| Matter clock | `proper_time_rate(latency,speed2)` evaluates `f²-speed2` with no `C_SPEED` argument in [`proper_time_rate.h`](../../../engine/include/ftd/proper_time_rate.h) | The raw transport speed is silently treated as a `c=1` beta. |
| Clock caller | CPU passes `v.speed()²` directly in [`transmutation_phases.cpp`](../../../engine/src/transmutation_phases.cpp) | No conversion occurs at the interface. |
| Voxel gamma | `1/√(1-v²)` at zero latency in [`voxel.h`](../../../engine/include/ftd/voxel.h) | It disagrees with the force integrator's `1/√(1-v²/C_SPEED²)`. |
| Born–Infeld core | `-M_REST√(1-v²)` in [`voxel.h`](../../../engine/include/ftd/voxel.h) | Its kinetic expansion uses the legacy `c=1` velocity. |
| Public diagnostics | Sum `|born_infeld_core|` and report its legacy bandwidth in [`diagnostics_compute.cpp`](../../../engine/src/diagnostics_compute.cpp) | The mismatch is load-bearing, not dead code. |
| Kinetic energy | CPU, GPU, ledger, and cluster observables use `½|v|²` without `M_REST` | The reported total is neither the declared massive-particle KE nor a documented per-unit-mass diagnostic. |
| Cluster inertia | Uses `m=N·M_REST` directly in [`phase_forces.cpp`](../../../engine/src/render_bridge_phases/phase_forces.cpp) | Here `M_REST` is inertial mass. |
| Rest term | Born–Infeld gives magnitude `M_REST` at rest | Here the same scalar is rest energy. |
| Gravity | CPU latency source uses `M_REST·|state|` in [`poisson_solvers.cpp`](../../../engine/src/poisson_solvers.cpp) | A third role is fused without the energy conversion. |

## 4. Exact mass–energy fork

In raw lattice coordinates the invariant relation is

\[
E^2=(m c_{\rm lat}^2)^2+c_{\rm lat}^2|\mathbf P|^2.
\]

At rest, `E₀=m c_lat²`. Because `c_lat²=1/3`, the role choice has two honest branches:

- if `M_REST` denotes inertial mass, the rest energy is `E₀=M_REST/3`;
- if `M_REST` denotes rest energy, the inertial mass is `m=3·M_REST`.

Natural units do permit the same number to represent mass and energy, but only after time/velocity/momentum have been rescaled so that the causal speed is one. The current movement coordinate remains voxels per tick with causal speed `1/√3`, so natural-unit equality cannot simply be asserted locally inside the clock and action while the force integrator keeps raw units.

This audit does not choose which role `M_REST` should own. That choice changes production semantics and requires an owner decision.

## 5. Why prior passing tests do not close the gap

`test_de_broglie_redshift.cpp` sets the raw stored velocity to `0.3`, notes that it is below `C_SPEED≈0.577`, and then defines its expected answer as `√(1-v_read²)`. It therefore verifies that the implemented `c=1` clock reproduces itself. The physically cone-matched comparator would use `√(1-(v_read/C_SPEED)²)` at zero latency.

FTD-0252 is not a counterexample. Its independent wave-clock construction explicitly defines `v=v_g/C_WAVE` before comparing with `√(1-v²)`, and explicitly never reads `voxel.tau`. FTD-0252 and its FTD-0268 blind extension therefore retain their scoped measured status. What fails is the later claim that the raw `voxel.tau` implementation inherits that result without the same normalization map.

Consequences:

- FTD-0271's imposed Klein–Gordon clock and its textbook conditional consequences remain.
- Its A5 statement that the engine's raw proper-time rate is already a physically covariant FTD-native input is **withdrawn**. The test is reclassified as implementation self-consistency under the legacy `c=1` rule.
- The two-clock hazard amendment still makes decay consume the same matter-clock function as `tau`; at rest its latency factor is unaffected. It does not validate the moving-clock normalization.
- FTD-0208's clock-hypothesis `[AXIOM]` and FTD-0253/0256's causal-cone boundary remain unchanged.

## 6. GPU side-finding

The audit also finds an exact GPU parity defect when latency is active:

1. `latency_tau_bandwidth_kernel` in [`kernels_poisson.cu`](../../../engine/cuda/kernels_poisson.cu) advances `tau` on device and clamps speed to `C_SPEED·f`;
2. the force kernel uses the different asymptotic limit `C_SPEED·√f`;
3. after the GPU tick, the host state is downloaded and `RenderBridge::tick()` invokes `accumulate_proper_time()` again.

Thus the GPU latency path can advance `tau` twice per public tick and applies a speed cap absent from the CPU post-pass. This side-finding is exact source dataflow. It is not needed for the main dual-normalization proof, but it must be resolved by the same reconciliation.

## 7. Required reconciliation before NCEMC

Before the Native Confinement Energy–Momentum Contract from FTD-0400 can be implemented, the engine needs a **Causal Normalization and Mass-Role gate**:

1. **Coordinate contract:** declare whether each consumer receives raw `u` or dimensionless `beta=u/C_SPEED`; make the conversion explicit.
2. **Single causal budget:** proper time, force integration, movement, and latency must reach the same causal boundary. The current `C√f`, `Cf`, and legacy `c=1` boundaries cannot all stand.
3. **Mass-role contract:** distinguish rest energy, inertial mass, and gravitational source until a common Hamiltonian/stress–energy object proves their relation.
4. **Energy–momentum contract:** replace unit-mass `½v²` totals with one convention consistent with the selected role and `C_SPEED`.
5. **CPU/GPU parity:** advance proper time exactly once and enforce the same speed law on both backends.

No new confinement, mass, equivalence-principle, or moving-clock measurement is admissible before these five gates are fixed. A production patch must be separately authorized because choosing the raw-lattice or rescaled convention changes established engine behavior and golden results.

## 8. Verification

The recomputing verifier [`audit_c_speed_mass_normalization.py`](../../../scripts/proofs/audit_c_speed_mass_normalization.py) checks nineteen exact source contracts and the rational anchors:

```text
RESULT  19/19 source-contract checks passed
ANCHOR  c_lat^2 = 1/3
ANCHOR  legacy matter-clock rate^2 at the transport cap = 2/3 (not 0)
ANCHOR  rest-energy/inertial-mass conversion factor = 1/3 or 3
VERDICT UNMAPPED-DUAL-NORMALIZATION
```

No production engine behavior, framework type, calibration, or value of `C_SPEED` changes. The next free registry id is FTD-0402.

Engine verification used the canonical WSL2 `engine/build_wsl` tree. The five
affected targets (`cluster_inertia`, `de_broglie_redshift`, `lorentz`,
`gamma_ftd_momentum`, and `voxel_properties`) passed twice, and the golden gate
passed 7/7. A repository-wide `ctest -j 24 --output-on-failure` was also
attempted, but no complete aggregate was obtained: the unchanged
`halo_forcedness`, `maxwell`, and `proton_stability` campaign targets each
remained CPU-active past successive 20-, 10-, and 5-minute outer command
bounds. No failure was emitted before those bounds, but these attempts are not
booked as a full-suite pass.
