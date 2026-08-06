# FTD-0643 — Analytic-center collective boost ladder v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0642
`CONNECTED_BLOCK_COUPLED_TRANSVERSE_WEAK_HYBRID_CONSTRUCTIVE`, result SHA-256
`E4DCBC8F3BC0A8AE30986581C7B518F08155C28C5412697DEB01B6BECC782930`  
**Scope:** finite collective transport, depinning, internal deformation, and
dressing retention of the exact analytic-center composite  
**Date:** 2026-07-27

## 1. Question

Does the FTD-0638 exact-center object translate as one coherent dressed
pattern when every constituent receives the same finite momentum, or does the
lattice convert collective launch energy into pinning, internal deformation,
coat loss, or direction-dependent failure?

This is a finite classical boost campaign. It is not a dispersion fit, a
zero-threshold proof, a relativistic boost, or a particle-pole campaign.

## 2. Frozen state and dynamics

Use the orientation-zero `L=17` FTD-0638 center, the FTD-0640 analytic
48-coordinate basis, and the unchanged common-action matter/field solver with
the qualified exact-residual cache. Set the same initial momentum on all 16
constituents. Retain the minimum-energy longitudinal dressing and zero initial
magnetic field. Run 16 forward ticks and 16 state-only reverse ticks.

No force branch, damping, reaction, collision, graph change, external packet,
neutralizer, force amplification, post-hoc recoil, or production toggle is
admitted.

## 3. Locked arms

For each normalized canonical direction

- `<100>=(1,0,0)`;
- `<110>=(1,1,0)/sqrt(2)`;
- `<111>=(1,1,1)/sqrt(3)`;

run positive uniform constituent momenta

`p={0.001875,0.00375,0.0075,0.015,0.03,0.06,0.12}`.

Also run:

- one zero-momentum rest control;
- negative mirrors at `p=0.03` and `p=0.12` for all three families;
- `p=0.12` cyclic copies `<010>`, `<001>`, `<011>`, and `<101>`.

This fixes 29 arms. No amplitude or arm may be added after execution.

## 4. Locked observables

At each forward tick record center displacement, mean velocity, total matter
momentum, site hops, energy sectors, common-action residual, chart/fibre state,
center-subtracted shape RMS, maximum bond strain, and all 48 FTD-0640 modal
projections.

Define the free reference displacement

`D_free=16*|v_flat(p)|`

from the unchanged production dispersion and mobility

`mu=D_parallel/D_free`.

Define soft fraction from the analytic matter basis,

`F_soft=sum_{a=0}^5 Q_a^2 / sum_{a=0}^{47} Q_a^2`,

using the trajectory-integrated powers. This is a coordinate-subspace
diagnostic; it is not a quantum occupation.

At each tick recompute the instantaneous minimum-Gauss electric dressing for
the current constituent positions without altering the evolved state. Define

`R_dress=||E_dynamic-E_minGauss(X)||/||E_minGauss(X)||`.

This measures longitudinal dressing fidelity. It does not classify transverse
radiation by itself.

## 5. Exactness and coherence gates

Every arm must initialize, finish, and invert; preserve the registered graph
and remain in valid chart/spline branches; have zero collision ambiguity, fibre multiplicity `<=8`,
and same-anchor separation `>=0.9`; and satisfy:

- common-action residual `<=1e-10`;
- total-energy drift `<=1e-10`;
- inverse recovery `<=1e-9`;
- maximum center-subtracted shape RMS `<=0.05` cell;
- maximum squared-edge strain `<=0.05`.

The rest arm must have center displacement and mean speed `<=1e-10` and zero
site hops.

Every canonical `p=0.12` arm must additionally have:

- projected center displacement `>=0.75` cell;
- mobility `>=0.75`;
- transverse displacement `<=0.10` cell;
- at least 16 legitimate constituent site hops;
- positive final velocity projection;
- integrated `F_soft>=0.95`;
- maximum `R_dress<=0.50`.

Within each canonical positive ladder, projected displacement must be
nondecreasing to `1e-6` and never have the wrong sign beyond `1e-10`.

## 6. Mirror and cubic gates

For each registered positive/negative pair, compare tickwise center
displacement after reversing the sign, shape RMS, field energy, soft fraction,
and dressing residual. Their maximum absolute residual must be `<=1e-7`.

The high-amplitude cyclic copies must agree with their canonical arm after the
corresponding cyclic coordinate map in center, momentum, shape, energy,
soft fraction, dressing residual, hop count, and recovery to `<=1e-7`.

## 7. Locked classification

For a positive ladder arm call the response:

- `mobile` if `mu>=0.5` with positive displacement;
- `pinned` if `|mu|<=0.1`;
- `transition` otherwise.

Record for each direction the last pinned momentum and first mobile momentum.

- `ANALYTIC_CENTER_COHERENT_FINITE_DEPINNING_CONSTRUCTIVE`: all exact,
  coherence, rest, high-boost, monotonic, mirror, and cubic gates pass, and
  every canonical family contains at least one pinned and one mobile arm.
- `ANALYTIC_CENTER_COHERENT_NO_THRESHOLD_AT_LADDER_RESOLUTION`: those gates
  pass and every nonzero canonical arm is mobile.
- `ANALYTIC_CENTER_COHERENT_MIXED_ONSET`: exact/coherence/rest/high-boost,
  mirror, and cubic gates pass but onset is neither of the two cases above.
- `ANALYTIC_CENTER_DIRECTIONAL_TRANSPORT_CLOSED`: exact/coherence/rest pass but
  any high-boost, monotonic, mirror, or cubic gate fails.
- `ANALYTIC_CENTER_BOOST_EXECUTION_INVALID`: initialization, solver, sector,
  energy, coherence, inversion, coverage, or output provenance fails.

No verdict establishes a vanishing continuum threshold, inertial mass,
relativistic dispersion, co-moving radiation-free solution, physical charge,
particle pole, common cone, Lorentz recovery, or production ontology.

## 8. Artifacts

Produce one focused CTest, arm/tick CSV and JSON summary, independent
certificate, analysis/audit, and synchronized canonical records. Production
remains unchanged.
