# Strict-discrete stack: implementation and audit ledger

Date: 2026-09-05. Status: **ACTIVE; successor candidate, not canonical adoption**.
Owner authorization: implement the accepted seven-phase roadmap with independent
audits and gated migration. Baseline HEAD:
`ece0e64e22a7e9b952bccf22ecc19e46fbee74de` plus the pre-existing working tree.
No unrelated edits are part of this program's release evidence.

## Contracts

One owner advances one complete finite state by charged microscopic ticks.
Observation is passive. Reduced evolution requires a separately accepted
closure/error certificate. Physical interpretations, units and probability
measures are explicit; token count is not physical energy by declaration.

The current Phi-v2 reference law remains unchanged. Its one-tick composed
support fails the requested radius-one requirement. The candidate changes the
physical schedule, declares finite pending state, and requires its own causal
and continuum certificates. Neither the legacy wave speed nor the legacy
error budget is inherited.

Candidate four-tick schedule:

1. Compute admission and saved crossing gates from current records; transfer
   uniquely absorbed tokens from field channels into relation reserves.
2. Apply local pair collisions and gated relation evolution; advance layers.
3. Stream channels by one SC hop using the retained manifestation record.
4. Recompute manifestation from relation incidence and clear pending masks.

The complete state includes all active lattice arrays, finite masks, schedule
phase and the global ordinal microtick. Masks represent control information,
not extra carriers. Snapshot/restore includes every record needed for the next
step. External journals and runtime generations are observational metadata.

## Program gates

| Phase | Required exit | Disposition |
|---|---|---|
| 0: evidence | Exact reference causal witness; law/table identities; continuum scope separated | PASS, reproduced witness and bounded certificate audit |
| 1: causal candidate | Finite validation, complete checkpoints, all-stage accounting, structural dependency argument and independent audit | PASS for declared finite periodic candidate; undefined boundaries remain open |
| 2: observations | Exact spatial/temporal aggregation, phase/channel/port readouts, pending state, closure counterexamples | PASS for exact diagnostic observations; autonomous closure explicitly rejected |
| 3: continuum | Full-ensemble/correlation and leakage control; executable finite-horizon error contract | BLOCKED scientifically; exact obstruction instruments and independent audit complete |
| 4: native | Accepted causal candidate, exact C++/WASM/WSL2-CUDA parity, complete single-owner interface | PASS within tested research backend scope; CPU/CUDA/WASM and final transport independently accepted |
| 5: matter/macroscopic | Persistent carriers, bound composites, atomic/molecular mechanisms, bulk laws and causal gravity | Open physical recovery; no catalog substitutions |
| 6: product | Shared state lineage, strict views, lifecycle/performance gates, certified acceleration only | Local microscopic/block laboratory PASS: 14 browser tests including 20 hardware cases; production migration and reduced acceleration remain open |

An implementation gate passing is not a physical recovery gate passing.
Research failure is recorded with its witness; success criteria are not retuned.
Canonical adoption of a successor and public deployment remain separate acts.

## Foundation wave ownership

| Owner | Exclusive responsibility |
|---|---|
| Causal implementation agent | Staged candidate, validation/checkpoint tests, reference causal witness, candidate specification |
| Observation implementation agent | Resolved histograms, field moments, exact temporal transfers, tests |
| Continuum implementation agent | Exact feasibility, operator/correlation negative controls, scope contracts, audit report/tests |
| Coordinator | Runtime facade, integration tests, program ledger, navigation, evidence integration |

Writers preserve others' work. Shared interfaces are fixed before consumption.
The next review wave rotates responsibilities: no agent approves its own work.
Independent findings return to the implementing owner for repair and re-audit.

## Phase 0 evidence of record

Baseline suite: `python -m pytest scripts/tests/phi_v2_lattice -q` reported
44 passed on 2026-09-05, before this implementation wave.

Frozen reference collision hash:
`D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25`.

Reference causal counterexample: L=7; otherwise blank state; site (3,3,3)
contains channels {0,34}. Adding channel 2 at (4,3,3) changes output bank at
(2,3,3) from {113} to {115} after one reference tick. The source and observed
site are at periodic Chebyshev distance two. This is independent of relation
owner conventions. Mechanism: contested absorption changes local collision
participation, then streaming carries the change one additional hop.

Continuum issues to adjudicate:

* Product-reference marginal Jacobians omit generated correlation state.
* A projected one-step bound does not control unresolved-mode excursions and
  return over multiple steps.
* At half occupation, the probability of exactly two occupied channels among
  192 is `binomial(192,2)/2^192`. Feasible collision and relaxation budgets must
  be established analytically before any hydrodynamic campaign.

## Public research facade

`StrictRuntime` provides `advance(microticks)`, `checkpoint()`,
`restore(checkpoint)`, `observe(resolution, observable)`, `capabilities()` and
`diagnostics()`. It is Python research infrastructure, not a production bridge.
Supported observations are exact counts and resolved channel/relation records.
Unknown semantic/continuum observables fail explicitly.

Each observation carries law/table identity, owner generation, microscopic
tick, phase, block width/support, and symbolic units. Snapshot restoration
increments the external generation even when physical time moves backward.
Batch advancement publishes no partial state if a stage fails. Decimal-string
wire integers preserve precision beyond JavaScript's exact numeric range.
An external synchronous block observation is not an instantaneous physical
measurement by an observer inside the simulated world.

## Remaining roadmap and release policy

The candidate now has a dedicated C++17 kernel, compiled WASM worker adapter,
and four WSL2 CUDA kernels. Tests compare complete state and event equality.
The optional parent build is `FTD_BUILD_STRICT_CANDIDATE=ON`; its default is OFF.
Frozen collision tables retain their hash and encoding. Native compile/test
work uses project concurrency flags; GPU measurements use WSL2.

Continuum development must distinguish microscopic trajectories, full counting
ensembles, marginal/tangent approximations and projected predictions. Missing
correlation, leakage, preparation or sampling terms prevent certification.
The staged candidate's tick calibration is separately determined.

Physical recovery proceeds from persistent transported carriers through bound
composites, atomic/molecular interpretation, bulk constitutive behavior and
causal gravitating macroscopic systems. Each capability remains unavailable
until its evidence passes. Versioned candidate research is authorized; a
candidate reruns all upstream causal/accounting gates after any law change.

Production migration exposes accepted microscopic/block observations first.
Every view refers to the same state lineage. Refinement restores retained state;
it never invents a microscopic history from averages. Exact patches require
causal halos; approximate acceleration requires an accepted error/boundary
contract. Per-interface lifecycle and hardware 60 FPS gates precede activation.

No public deployment is authorized by the instruction to deploy agents. Any
later public release requires existing merge gates and live artifact/hash
verification as well as this program's candidate-specific checks.

## Evidence updates

Final integrated run: **220 passed in 22.99 seconds**, with both
`FTD_STRICT_CUDA_REQUIRED=1` and `FTD_STRICT_WASM_MODULE` set to the newly compiled
module. This includes 35 native parity tests, 38 CUDA tests, and four compiled
WASM tests. No backend test was skipped. These are bounded transition and
rejection tests, not exhaustive enumeration of all complete states.

Independent reviews rotate ownership: the causal writer reviewed observations
and CPU; the continuum/native writer reviewed causal state and CUDA; the
observation/transport writer reviewed continuum and the coordinator's owner
facade. CUDA memcheck and initcheck each reported zero errors for the specified
eight-tick L=5 sample. Browser and final transport review results are recorded
in the [integrated audit](AUDIT_STRICT_STACK_FOUNDATION.md).

Resolved findings:

| Finding | Repair and regression |
|---|---|
| Concurrent Python batches could lose an update | One reentrant owner lock serializes mutation and observations; concurrent advance/restore tests |
| NumPy Boolean dtype admitted noncanonical raw bytes | Require byte values 0/1 before transition/checkpoint; native validation matches |
| Equal generations could refer to different owners | External UUID owner identity accompanies every observation and worker request |
| Duplicate transfer log keys could be overwritten | Validate unique crossing keys, direction and port transformations before aggregation |
| CUDA launch-grid narrowing | Bound required blocks against integer and device limits before narrowing |
| Browser reset/error could retain stale publication | Clear views and owner, disable dependent controls, reject obsolete epoch callbacks |
| Width control could disagree with published blocks | Serialize observation and use the returned width in publication |
| SharedArrayBuffer checkpoint could change before execution | Reject shared buffers at receipt, adapter and compiled binding; 11 protocol tests and four compiled artifact tests independently pass |
| WASM observation provenance omitted required fields | Report actual law/table, tick interval, spatial support, units and status; 32 compiled observation comparisons and browser assertions |

The final browser suite passed **14 tests in 4.9 minutes** against frozen
artifacts. Its 20-case matrix measured 14,420 frame intervals: minimum 60.0018
FPS, worst p95/p99/interval 16.8 ms, no long tasks, no intervals above 33.4 ms,
and no page errors. Each case includes 721 intervals over at least 12 seconds.
The hardware probe identified ANGLE / NVIDIA RTX 5090 / Direct3D11. Canvas2D
rendering and the separate WebGL device probe are explicitly distinguished.
All five browser artifact hashes remained unchanged; all 69 source/artifact
manifest identities were rechecked after measurement. See the
[complete measured report](evidence/strict-stack-hardware-2026-09-05.json).
This passes the local workload gate, not an unbounded 60 FPS guarantee or
qualification of the existing Scale 0–4 production interfaces.

## Next research gates: fixed dependencies

| Gate | Required deliverable before execution/adoption | Current disposition |
|---|---|---|
| C1: full response | Complete ensemble restriction/lift and generated correlation coordinates for this staged law | OPEN; marginal product tangent is insufficient |
| T1: exact prepared transport | Invariant complete-state sector and finite-horizon trajectory/ensemble error bound | PASS, restricted singleton advection; eight anisotropic velocities and held-time error <=11a/12 |
| K1: stationary counting reference | Invariant complete reference family, saved-gate correlations and analytic collision eligibility | PASS, selected homogeneous occupied background; p=1/96 is a separately named feasible event-density proposal; no relaxation or tangent closure |
| C2: projected return | Finite-horizon retained/unresolved transfer estimate, including repeated returns | OPEN; one-step projected error does not imply it |
| C3: executable scale window | Relaxation and wavelength bounds with total preparation, dispersion, projection and sampling error | OPEN; half-occupation collision campaign infeasible under stated reference |
| C4: registered comparison | Freeze law, preparation, observable, norm, orientations, amplitudes, lengths, time interval and rejection thresholds after C1-C3 | NOT ELIGIBLE; no numerical campaign launched |
| M1: persistent carriers | Constituent-resolved lifetimes, transport and interactions over registered perturbations | OPEN; token conservation alone does not pass |
| M2: bound composites | Dynamical binding and response with complete exchange budgets | OPEN; requires M1 |
| M3: atomic/molecular meaning | Mechanism-backed state structure, spectra and bonding | OPEN; requires M2; catalog assignments excluded |
| M4: bulk limit | Constitutive equations, transport and finite-scale errors from accepted microscopic evolution | OPEN; requires C4 and appropriate matter gates |
| M5: planetary meaning | Causal field/matter interaction and independently validated gravitational limit | OPEN; Newtonian reference dynamics do not pass |
| B1: constitutional boundary | State-complete undefined-boundary extension law and causal/conservation validation | OPEN; current runtime explicitly periodic |
| A1: acceleration | Exact retained halo/compression proof, or accepted reduced error and boundary contract | OPEN; current runtimes execute every required transaction |

A failed research gate yields a bounded obstruction or a new versioned law
proposal. It never enables a higher semantic capability. This implementation
delivers the roadmap's permitted early microscopic/block candidate; it does not
declare the entire material-to-planetary recovery program completed.

The owner-directed [recovery wave 1](PROGRAM_STRICT_RECOVERY_WAVE_1.md) now
records the first exact prepared transport continuum comparison, complete
finite counting ensembles, constituent campaigns and scoped material limits.
Its restricted T1 result does not supersede the open general C1-C4 gates.
The K1 reference-family result likewise leaves perturbed response and
multi-step correlation bounds open; it does not retune the half-occupation
certificate or certify an interacting continuum equation.
Its final integrated verification passed **335 tests in 38.68 seconds** with
CUDA required and compiled WASM enabled, including all 220 foundation tests.
See the wave disposition for independently reviewed evidence and open gates.
