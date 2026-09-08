# One discrete causal evolution, multiple resolutions

Date: 2026-09-04.
Status: **[OWNER-REQUESTED DIRECTION] [IMPLEMENTATION PLAN]**.
Physical recovery beyond the existing selected-law certificates is **[OPEN]**.
This document does not amend the constitution or select a replacement Phi.

**Execution update (2026-09-05):** the owner requested implementation with
independent audits. The [program ledger](PROGRAM_STRICT_DISCRETE_STACK.md)
tracks acceptance. Planning review reproduced a radius-two dependency in the
reference Phi-v2 tick and identified unresolved continuum correlation/leakage
claims. The implementation therefore starts with a separately versioned staged
candidate; the older recovery description below is provenance of the starting
specification, not an independent certification of microscopic convergence.

## Required architecture

The user's requirement is one strict-discrete causal stack from microscopic
records to macroscopic physics, with an explicit connection to continuum
coarse-graining. One state owner advances one declared microscopic law:

`X[n+1] = Phi(X[n])`.

Every resolution is an observation `Y_b[n] = R_b(X[n])`. Changing the displayed
scale changes the observation, not the physical universe or its clock.
Semantic views (particle, atom, molecule, planet) are not automatically cubic
blocks: identifying those persistent structures remains a separate recovery
problem. Block restriction supplies a common mathematical foundation beneath
those views.

The existing dashboard engines are effective references. The present
`RenderBridge` with continuous flux and elliptic solves is not the v3 finite
law. The initial research owner is `scripts/phi_v2_lattice`, implementing the
selected reference law documented in
[Phi R2–R5 v2](../../docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md).
Production migration requires independent parity and complete-tick support
certification; passing the new block tests is not that certification.

## Exact bridge before continuum approximation

For a partition into blocks B, retain exact extensive counts. In particular,
use the integer incidence Q of relation primaries, not just the ternary quotient
`s = bal3(Q)`. The latter discards multiples of three and cannot replace Q in
an integer continuity equation.

Summing the microscopic continuity equation gives

`Q_B[n+1] - Q_B[n] + sum(outward boundary currents during tick n) = 0`.

Every internal edge contributes once with each sign and cancels. This is a
finite combinatorial identity, with no smoothness, probability, units, or
thermodynamic limit assumed. FCC diagonals and periodic seams must use their
actual endpoints. Boundary transfers are graph currents until a continuum
area/metric convention is justified.

Nested restriction must satisfy `R_bc(X) = merge_c(R_b(X))`. Restriction is
lossy, so an inverse is not available in general. Zooming back in uses retained
microscopic state or an exact reconstructible encoding. A sampled refinement
is an explicitly conditional preparation, not recovery of the original state.

An exact autonomous macro law Psi exists on the chosen state family precisely
when the restriction is dynamically closed:

`R_b(X) = R_b(X')  =>  R_b(Phi^m(X)) = R_b(Phi^m(X'))`.

Necessity follows by applying the same Psi to equal inputs. For sufficiency,
define Psi on each image value using any representative; the implication makes
the result independent of representative. This is a general quotient criterion,
not a new physical derivation.

When it fails, retain more phase/boundary/correlation state, keep the exact
microscopic owner, or register an approximate closure with a validity domain.
Finite history need not suffice. A probabilistic closure additionally needs a
declared preparation/ensemble; counting alternatives alone does not select a
physical probability measure.

## What continuum recovery must demonstrate

After exact accounting, define observer densities and currents using explicitly
declared spatial and temporal units, e.g. `rho_B = Q_B / (b*a)^3`. Values may
be rational before a plotting conversion to floating point. This does not add
a primitive continuous field. Token count is not automatically physical energy,
and Q is not automatically electric charge in coulombs.

A smooth interpolation u is an approximation to the block observables over a
specified finite region, preparation family and time horizon. A continuum
candidate must provide all of:

1. Retained slow variables and the preparation/regularity assumptions.
2. Derived transport/balance terms and identified constitutive closure.
3. One scale map for length, time and each physically identified quantity.
4. An error bound or registered convergence criterion for observables,
   boundary transfer and dynamics, uniformly over the stated time interval.
5. Corrections for anisotropy, finite wavelengths and unresolved correlations.
6. The domain in which reduced evolution can replace direct Phi execution.

The useful regime is separation of lengths `a << b*a << macroscopic variation
length`, with a corresponding temporal separation. This is not a claim that
increasing block size alone makes the error vanish. Scaling may be ballistic,
diffusive, or otherwise sector dependent. A limit at fixed coarse step count
does not by itself establish convergence over a fixed physical time whose step
count grows under refinement.

There is existing scoped starting evidence: the selected Phi-v2 specification
records a prepared transverse vacuum sector with speed 1/6, an effective
quadratic action, and a finite-region infrared error contract (§§5–9).
Charged coupling normalization, nonlinear closure, stable extended matter,
common material/light cones and gravity are explicitly open there. Reuse and
audit that result at its stated scope; do not import `RenderBridge`'s
`1/sqrt(3)` as the reference law's wave speed.

Discrete-to-continuum recovery is an established type of construction: the
[Frisch–Hasslacher–Pomeau lattice-gas paper](https://journals.aps.org/prl/abstract/10.1103/PhysRevLett.56.1505)
exhibits deterministic Boolean lattice gases with Navier–Stokes behavior.
It does not establish this result for FTD's law. Reduced dynamics can also
require memory and a specified measure, as in
[Darve, Solomon and Kia's generalized Langevin construction](https://pmc.ncbi.nlm.nih.gov/articles/PMC2708778/).
Neither external result licenses silently adding primitive randomness or
assuming closure for the chosen FTD block variables.

## Causality across resolutions

The invariant is dependency support in underlying space-time, not the number
of times a renderer updates. One macro update spanning m microscopic ticks
may use at most the union of their causal pasts. For aligned cubic blocks of
width b and a radius-one microscopic law, this permits at most
`ceil(m/b)` neighboring block indices in each direction. Such a coarse jump
represents m elapsed microticks, not a faster microscopic influence.

Synchronous block totals are external observations. A physical observer at one
site cannot receive a whole block's instantaneous total without communication
latency. A future live API must distinguish the represented time interval,
the support of the observation, and when its information reaches a receiver.
The present offline readout does not model an in-universe measurement device.

An approximate PDE with instantaneous tails cannot be the exact causal owner.
It may be a valid large-scale approximation within a certified region/error
budget. Elliptic field solves can describe constrained or quasistatic limits;
they cannot silently become instantaneous microscopic signal channels.

## Implementation gates

| Gate | Concrete acceptance | Current status |
|---|---|---|
| A: exact block observation | Integer counts, exact boundary balance, nested restriction, mutation-free observation; nonvacuous SC/FCC/seam witnesses | Implemented and tested on selected finite fixtures |
| B: causal state owner | Complete-record input validation, law/version identity, complete one-tick dependency proof/tests, deterministic replay, boundary and expiry accounting | Open for production migration |
| C: closure characterization | Equal coarse states have equal next coarse states, or explicit counterexamples identify omitted state; causal boundary ports retained | Count-only closure fails; exact witness implemented |
| D: controlled continuum sector | Prepared Phi-v2 transverse sector measured through block fields; finite-time errors and scale consistency independently checked | Existing theory certificate; block-field runtime validation open |
| E: common production runtime | Verified native implementation and worker transport; one global tick owner; every UI view references the same state lineage | Open |
| F: material hierarchy | Persistent carriers, interactions, bound composites and bulk constitutive laws recovered at actual epistemic status | Open; cannot be supplied by particle/element catalog assignment |
| G: macroscopic acceleration | Reduced solver/refinement contracts preserve boundary transfers, error bounds, causal time and provenance; fail closed outside the certified domain | Open |

Gates B/C/D can inform one another, but a failed requirement cannot be hidden
by turning on an independent higher-scale engine. No existing reference engine
is removed until its replacement has the behavior and tests its users need.
The two atom-runtime defects in the
[Scale 0–4 audit](../../docs/audits/AUDIT_2026-09_SCALE0_SCALE4_DISCRETE_CAUSAL_PHYSICS.md)
remain defects in those reference engines; they were not fixed by this work.

## First delivered implementation and next bounded task

[`coarse.py`](../../scripts/phi_v2_lattice/coarse.py) supplies passive exact
integer block counts, nested merging, and boundary current aggregation from
the actual crossing event log. It leaves the microscopic state authoritative.
It does not provide a macro step, a matter classifier, a token-energy current,
or physical units. Relation token counts use declared stored-owner assignment;
the certified block continuity here concerns Q, not local token-energy balance.

[`test_coarse.py`](../../scripts/tests/phi_v2_lattice/test_coarse.py) includes a
decisive closure counterexample: reserve tokens at identical positions with
phase 0 versus phase 1 have identical retained block counts, but produce
different next-block incidence. Densities/counts alone therefore cannot evolve
this selected law exactly. That negative result determines the next design
requirement instead of being averaged away.

Validation: 13 new tests pass; all 44 tests in `scripts/tests/phi_v2_lattice`
pass (2026-09-04). These include live boundary currents and a corrupted-event
negative control. They are finite fixtures, not exhaustive proofs over all
complete microscopic states.

Next bounded task: certify complete-tick dependency support of the selected
lattice implementation, and add phase-resolved channel/port readouts for the
registered transverse sector. Then compare restricted Phi trajectories to
that sector's stated continuum generator over a declared finite-time family.
No near-match search, new physical identification, or modified collision law
is part of this gate.
