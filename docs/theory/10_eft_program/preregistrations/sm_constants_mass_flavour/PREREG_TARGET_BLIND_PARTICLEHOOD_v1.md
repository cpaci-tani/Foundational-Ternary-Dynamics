# PREREG — Target-blind particlehood before mass

**Prospective claim ID:** FTD-0399 (registry rechecked at lock time; FTD-0398 is the current maximum).  
**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] · LOCK-STD v1 · git tag `preregister-target-blind-particlehood-v1`.  
**Question:** do different production histories converge to one target-blind localized species, before any mass observable is attempted?

## 1. Exclusions and frozen protocols

The campaign reuses the exact A/C/E radial birth histories from the seed-diversity campaign. It introduces no particle label or target, `m_e`, `M_REST`, `alpha^11`, de Broglie clock, force law, imported mass diagnostic, dispersion relation, or calibration. It reads only state already produced by the current engine.

For each `L in {33,65}` and each seed, run two protocols from initialization:

- **dissipative:** `wave_propagation`, `coupling`, `gauss_projection`, `genesis`, `damping`, and `selective_damping` ON;
- **undamped:** the identical protocol except `damping` and `selective_damping` are OFF.

Every other Boolean toggle is OFF, including forces, movement, gravity, dual substrate, stochastic terms, clocks, and imported effective sectors. Non-Boolean settings remain engine defaults. Execution is CPU-forced and `FTD_FORCE_GPU` must be unset.

Each history is aligned at its first manifestation tick, defined as `t_post=0`. Record through `t_post=12` at `L=33` and through `t_post=24` at `L=65`. The causal-window gate is geometric: at every record, the distance from the fixed radius-4 observation cube to the nearest lattice face must be at least `t_post`, the engine's maximum one-site-per-tick propagation allowance. Matched `L=33/65` local profiles provide the finite-size boundary-dependence measurement.

## 2. Frozen local state and observables

The local window is the fixed Chebyshev-radius-4 cube centered on the first manifested site. In lexicographic `(dx,dy,dz)` order, its raw vector `Y` contains, at every one of 729 sites:

`(Jx,Jy,Jz,wave_vel_x,wave_vel_y,wave_vel_z,state,color,spin,flavor)`.

It deliberately excludes `particle_id`, pair identity, mass constants, phase/clock data, forces, and every imported mass proxy.

At each tick record:

- the number of 26-connected manifested clusters;
- `N`, the total number of manifested sites;
- charge `sum state` and the arithmetic centroid of manifested sites;
- whether all manifested sites lie inside the radius-4 window (`localized`) and `N<=729` (the flooding control);
- local field energy `E=0.5*sum_window(|J|^2+|wave_vel|^2)`;
- all three pairwise raw distances
  `d_ab=||Y_a-Y_b||_2/max(||Y_a||_2,||Y_b||_2,1e-15)`;
- all three normalized shape distances
  `s_ab=||Y_a/||Y_a||_2-Y_b/||Y_b||_2||_2`, using `1e-15` norm floors;
- population energy coefficient of variation across A/C/E;
- matched-time cross-size raw profile distance for each seed.

The required summary CSV schema is exactly:

`protocol,L,seed,t_post,N,charge,centroid,local_energy,raw_distance,shape_distance,energy_cv,cross_L_distance`

For an individual seed row, `raw_distance` and `shape_distance` are the maxima over the two pairs containing that seed. Therefore all three seed maxima are `<=1%` iff every pairwise distance is `<=1%`. `centroid` is encoded as `x;y;z`. Cross-size distance is `nan` only for the unmatched `L=65` ticks 13..24.

To make the summary independently recomputable, the campaign also writes a frozen detail CSV containing all 166,212 local-site records with schema:

`protocol,L,seed,t_post,dx,dy,dz,Jx,Jy,Jz,Vx,Vy,Vz,state,color,spin,flavor,cluster_count,N,charge,centroid_x,centroid_y,centroid_z,local_energy,localized,boundary_clear`.

## 3. Frozen instruments and execution

- Campaign: `engine/tests/campaign_target_blind_particlehood.cpp`, SHA256 `14833be2d81d31b682af73b51618126fa9a6c1991b2d965d0538a907c367b501`.
- Recomputing verifier: `scripts/proofs/verify_target_blind_particlehood.py`, SHA256 `a35a7c1b2a4b50818678cc59fcf4343d6d818bde61171813fd5bfa370d1e75d0`.
- Canonical build: WSL2 Ubuntu-22.04 `engine/build_wsl`, target `campaign_target_blind_particlehood`, `-j 32`.
- Canonical run: `unset FTD_FORCE_GPU && ./engine/build_wsl/campaign_target_blind_particlehood details.csv > summary.csv 2> run.stderr`.

Every individual history is executed twice inside the instrument and compared bit-for-bit over every recorded global field and raw local profile. The complete campaign is then executed twice externally; both summary, detail, and stderr files must be byte-identical. The independent verifier reconstructs local energy, connected clusters when local, all distances, CVs, summary rows, and the verdict from the raw detail file.

## 4. Correctness and vacuity gates

Correctness gates have absolute precedence; a failure gives **INVALID**.

| Gate | Frozen requirement |
|---|---|
| G1 | census GREEN; FTD-0399 next at tag cut |
| G2 | all 12 protocol/size/seed arms first manifest, reproducing freeze tick 2 |
| G3 | every internal duplicate is bit-identical; both complete external executions are byte-identical |
| G4 | active backend is CPU after `force_cpu`; `FTD_FORCE_GPU` unset; effective toggles exactly match §1 |
| G5 | the radius-4 observation cube stays inside the geometric one-site-per-tick causal boundary window; every grid/schema/finiteness/detail-recomputation check passes |
| G6 | at freeze, for each protocol and size, at least one A/C/E pair has raw or normalized-shape distance `>1%` |

G6 is the non-vacuity control: the comparator must reject the initially different histories. A zero-returning comparator, omitted labels, missing detail rows, or a norm-floor artifact cannot establish invariance. The duplicate control ensures convergence is dynamical rather than run-to-run noise.

Persistence/localization is a physical outcome gate, not a correctness repair: at every recorded tick every history must have exactly one 26-connected cluster, `N>=1`, all manifested sites inside the radius-4 window, and `N<=729`. Failure gives NO-STABLE-EXCITATION, not INVALID.

## 5. Frozen outcomes and precedence

After correctness gates, evaluate in this order:

1. **NO-STABLE-EXCITATION:** any history at any recorded tick fails persistence, single-cluster, localization, or flooding control.
2. **SPECIES-INVARIANT:** all histories remain one localized excitation, and at every final matched tick `t_post=9..12`, for both sizes and both protocols, every pairwise raw distance, normalized shape distance, energy CV, and per-seed cross-size distance is `<=1%`.
3. **DISSIPATIVE-ATTRACTOR:** all histories remain one localized excitation and the same final-four criteria pass for the dissipative protocol at both sizes, but fail for the undamped protocol.
4. **HISTORY-FAMILY:** all histories remain one localized excitation but neither row 2 nor row 3 holds; at least one final-four criterion retains `>1%` history/size dependence.
5. **INVALID:** any correctness gate fails; evaluated before rows 1–4 despite its display position.

The precedence is mutually exclusive. NO-STABLE is evaluated before convergence. Given stability, the two protocol pass bits have four values: `(1,1)` gives SPECIES; `(1,0)` gives DISSIPATIVE; `(0,0)` or `(0,1)` gives HISTORY-FAMILY. Exact `<=` owns the 1% boundary; normative criteria outrank prose.

## 6. Licensed interpretation

Only SPECIES-INVARIANT establishes native particlehood on this target-blind protocol. It does **not** establish mass; it opens a separately priced and separately locked obligation to derive a native energy–momentum pair before any dispersion relation.

DISSIPATIVE-ATTRACTOR shows thermostat-dependent convergence, not substrate-native species identity. HISTORY-FAMILY shows persistent excitations remain production-history families. NO-STABLE-EXCITATION shows the current dynamics does not maintain one localized excitation. These outcomes stop first-principles mass generation on the current engine. INVALID licenses no physical conclusion and cannot be repaired by interpreting partial data.

No outcome changes FTD-0015, FTD-0084, FTD-0096, FC-2, FC-W, MC-T4.3, any calibration, or any framework type. Any observation outside this scope becomes a new open item, not a narrative generalization.

## 7. Execution window and executor

Executor: the current Codex repository session on branch `codex/invariant-quotient-roadmap-2026-07-20`. Execution window: from creation of tag `preregister-target-blind-particlehood-v1` through exactly 72 hours after that tag's creation instant.

**LOCKED CONTENT ENDS HERE.** Normative changes require v2 before any execution.
