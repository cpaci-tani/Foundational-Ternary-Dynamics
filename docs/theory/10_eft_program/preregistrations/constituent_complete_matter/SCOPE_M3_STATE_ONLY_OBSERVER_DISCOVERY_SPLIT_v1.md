# FTD-0754 — M3 state-only observer discovery/validation split v1

**Status:** `[SCOPE — DISCOVERY COMPLETED; FTD-0755 CONSUMED
INFRASTRUCTURE-UNRESOLVED; M3 OPEN]`  
**Date:** 2026-07-30  
**Parents:** FTD-0743 state-only predicate contract; FTD-0753 constructive
large causal-horizon witness  
**Scope:** choose one non-circular complete-state classifier, then freeze a
separate held-out perturbation/volume validation; no validation trajectory is
authorized by this document

**Successor status (2026-07-30):** FTD-0755 supplied the separately hash-locked
validation protocol, but all twelve modes stopped before parent initialization.
FTD-0756 localizes the failure to its regional-observer wrapper. This scope
document still authorizes no validation trajectory by itself.

## 1. Purpose

FTD-0753 shows that the selected compact pair can coexist with a persistent
core, stable near field, and detached outward transport before environmental
return. It does not identify which instantaneous field component belongs to
the object or prove that an open family of nearby states shares the history.

FTD-0754 separates those jobs. Existing records are a **discovery corpus** used
to choose the observer and its numerical margins. New trajectories remain
unseen until a later validation protocol freezes every choice.

## 2. Discovery corpus

The observer may inspect only:

1. FTD-0739 `L=145` compact-support formation histories and bound control;
2. FTD-0745 `L=193` environmental continuation;
3. FTD-0732/0734/0735 local perturbation, mixed-corner, and root-regularity
   records at `L=33,65`;
4. FTD-0753 `L=321` face/edge/body causal-horizon records;
5. preregistered negative controls already present in those campaigns.

These data may choose scales and margins, but they may not count as held-out
validation. The archived run-of-record files contain scalar ledgers rather
than complete state checkpoints, so an observer that was not present in the
original executable cannot be evaluated from those files alone. Discovery may
therefore **deterministically replay the exact registered initial conditions
and tick maps of the listed campaigns**, provided the previously registered
scalar rows are reproduced before any new observer quantity is used. Such a
replay is part of the discovery corpus, not a new trajectory and not held-out
validation. New initial conditions, perturbations, volumes, directions,
durations, packet phases, or control families remain prohibited.

## 3. Candidate state-only decomposition

For complete instantaneous state `X=(s,C,F)`, construct:

- `a(X)`: the absolute-polarity-weighted constituent centroid modulo integer
  translations;
- `K(X)`: constituent kinetic energy and pair internal energy from the frozen
  selected action;
- `q(X)`: the ordered polarity multiset and its net sum;
- `F_b(X)`: the unique finite-support minimum-energy face field satisfying the
  current constituent Gauss source on the registered local support, with zero
  support-boundary crossing;
- `F_T(X)=F-F_b(X)`: the exact residual complete field;
- `F_o(X)`: the instantaneous outgoing radial Maxwell characteristic of the
  centered residual-field readout. At every noncentral sample, with radial
  unit vector `n`, residual tangential fields `E_t,B_t`, and the engine's
  equal-unit matched-field normalization,
  `E_o=(E_t-n cross B_t)/2`, `B_o=n cross E_o`;
- `F_bg(X)`: the complementary incoming characteristic plus radial constraint
  field, `E_i=(E_t+n cross B_t)/2`, `B_i=-n cross E_i`, together with
  `n(n dot E)` and `n(n dot B)`. At the central sample the complete residual
  is assigned to background because no state-defined radial direction exists.

The registered readout centers face `E` by averaging the two adjacent normal
faces, centers edge `B` by averaging the four adjacent parallel edges, and
reconstructs integer-time `B` from `(E_n,B_{n-1/2})` by the existing matched
Maxwell half-step. The characteristic decomposition must reconstruct this
centered residual readout exactly and partition its quadratic norm exactly.
It does not claim an exact primitive-cochain decomposition or an ontologically
unique radiation field. All discovery and reserved validation volumes are odd,
avoiding the even-volume Nyquist null mode of the centering map.

Thus the **registered centered readout** satisfies
`F_readout=F_b,readout+F_o+F_bg` exactly. The finite-support minimizer is a
selected bound-field projector, not an ontological theorem. Transverse or far
field is not automatically called radiation: characteristic content plus an
outward shell margin is required. Distance selects only the reporting shell.
An incoming packet, standing wave, or dispersing field must not be absorbed
into `F_o` by a distance-only rule.

If the residual split lacks a unique deterministic projection or fails cubic,
translation, polarity, and exact-reconstruction tests, observer discovery
closes negative before validation. No new trajectory is run to repair it.

## 4. Predicate ingredients

The later frozen predicate `P_theta(X)` must depend only on the complete state
at that tick and contain strict margins for:

1. one connected opposite-polarity constituent component;
2. graph-inside negative pair energy;
3. localized `F_b` energy and a bounded co-motion defect;
4. exact field reconstruction and Gauss compatibility;
5. outward `F_o` shell flux with bounded inward sustaining flux;
6. bounded `F_bg` in the registered no-background sector;
7. separation from empty, free-wave, unbound-pair, incoming-packet, and
   standing-wave controls;
8. covariance under integer translation, the proper cubic group, and polarity
   conjugation.

Route labels, preparation names, tick number, stored history, future evolution,
and periodic-return information are forbidden inputs.

## 5. Validation reservation

After discovery, a distinct FTD-0755 protocol must freeze:

- the complete formulas and implementation hash for `P_theta`;
- every radius, norm, shell, margin, and numerical tolerance;
- a perturbation measure with nonzero registered radius in quotient state
  space;
- all negative controls;
- a causal volume/horizon ladder including at least two volumes and one
  `L=321` confirmation on each cubic ray;
- hostile perturbation arms fixed from the discovery corpus, never selected
  from validation output;
- the root-regularity/Lipschitz argument used to turn strict finite-horizon
  margins into a nonzero open neighborhood;
- a first-failed verdict map and a no-retuning rule.

Validation data remain absent until that document, runner, executable, and
result directory are locked. A finite sample alone earns only sampled
robustness. M3 requires the FTD-0743 level-3 statement: strict margins plus a
regularity bound supporting a nonzero finite-time open neighborhood before
environmental return.

## 6. Consequences

- Discovery success licenses FTD-0755 validation; it proves no family.
- Discovery nonuniqueness closes this state-only separator and forces an
  explicit additional field/connection variable before an M3 claim.
- Validation success would establish only the exact registered finite-time
  M3 level and volume ladder.
- Validation failure closes the classifier/candidate conjunction without
  post-hoc threshold changes.

Autonomous motion, composability, formation, charge, poles, unitarity, and
Lorentz recovery remain downstream.

## 7. Discovery disposition (2026-07-30)

FTD-0754 executed under the separately locked
`PREREG_M3_STATE_ONLY_OBSERVER_DISCOVERY_v1.md`. All 939 prior scalar rows
replayed byte-for-byte, all 24 state-only snapshots passed, and the independent
certificate passed 116/116. The selected centered characteristic separator is
therefore retained for FTD-0755 design. No validation trajectory has run and
no M3 or particle claim advances.
