# FTD-0476 — Dynamical Flux Dressing / Wake / Release Probe v1

**Date locked:** 2026-07-25  
**Status:** `[RUN — INVALID MOVEMENT-EVENT ACCUMULATION; SUPERSEDED BY v2]`  
**Scope:** frozen CPU production tick; observer-only analysis plus declared
source interventions

## 1. Question

Does a manifested ternary polarity generate a field that is honestly describable
as a dynamical near-field dressing (informal shorthand: an *aura*), and what
happens to that field when the polarity moves or is removed?

The word *aura* is not an additional field or engine type.  The tested object is
the existing flux response generated from zero initial field by the production
coupling term

$$
\Delta_t^2J=C_{\rm wave}^2\Delta_{18}J-G_C\nabla_c s.
$$

The campaign must distinguish four claims:

1. **source-built dressing:** a polarity generates a polarity-odd, approximately
   radial flux response from zero initial `J` and `wave_vel`;
2. **attached dressing:** a substantial near-field component follows an actual
   production movement history;
3. **wake:** the moving source leaves a trailing excess outside its near field;
4. **released radiation candidate:** after a declared source-removal
   intervention, source-free field activity moves outward while the exact native
   wave invariant remains conserved.

No result in this campaign establishes electromagnetism, a photon, a gauge
charge, or a physical radiation quantum.

## 2. Frozen engine sector

- volumes: `L in {49,65}`;
- computational flux boundary: periodic;
- initial state: one locked `s=+1` site at the lattice centre, with
  `J=wave_vel=0` everywhere;
- active production terms during build-up: `wave_propagation=true`,
  `coupling=true`;
- `dual_substrate`, both Gauss mechanisms, damping, forces, gravity, movement,
  genesis, evaporation, pair production, weak transmutation, Langevin, alternate
  integrators, and Lorentz prototypes are off;
- build ticks: `0..12`; post-intervention ticks: `13..24`;
- samples: every tick;
- polarity mirror: repeat the stationary build with `s=-1`;
- empty control: identical term profile with `s=0`.

The movement arm starts from the completed `+1` build-up history.  At tick 12,
the source is unlocked, assigned `velocity=(C_SPEED,0,0)`, and movement is
enabled.  All other frozen terms remain unchanged.  The real production
movement phase supplies the subsequent face hops and its existing local flux
carry.  The history journal must record movement events and no reactions.

The release arm starts from the same completed build-up history.  At tick 12,
the source state is set to zero and locked velocity/remainder are cleared.  This
is an explicitly selected source-off intervention, not native evaporation.  It
is admissible only as a response probe.

## 3. Frozen observer

At source position `x_s`, define the nonnegative local wave activity

$$
e_i={1\over2}|W_i|^2-L_{\rm grad}(i),
$$

using the exact 18-point field-gradient convention.  It is a morphology weight,
not a replacement for the exact kick-drift invariant.

Record:

- total activity `E_a=sum_i e_i`;
- exact native tick invariant from `native_energy_contract.h`;
- activity-weighted periodic Euclidean mean radius about the current source;
- near fraction within `R_near=4`;
- sign-corrected radial alignment within `0<r<=6`,
  `A_r=sum sign(s) J_i dot rhat_i / sum |J_i|`;
- source divergence `sign(s) div J(x_s)`;
- for moving histories, activity ahead of `dx>R_near`, behind
  `dx<-R_near`, and transverse to the motion;
- field mirror residual `||J_+ + J_-||_2 / max(||J_+||_2,1e-30)`;
- source support and history-journal event counts.

Observer neutrality is checked by comparing primitive-state/field hashes for
an observed and unobserved copy after 24 ticks.

## 4. Locked classifications

### 4.1 Dynamical source-built dressing

Classify `SOURCE_BUILT_RADIAL_DRESSING` only if, at both volumes:

- the empty control remains below `1e-15` total activity;
- source activity at tick 12 exceeds `1e-8`;
- the polarity mirror residual is at most `1e-12`;
- radial alignment at tick 12 is at least `0.75`;
- sign-corrected source divergence at tick 12 is positive;
- the manifested count and source position remain exact through build-up.

Otherwise classify `NO_QUALIFIED_SOURCE_BUILT_DRESSING`.

### 4.2 Attachment versus wake under native movement

Let `f_near(12)` be the stationary build-up near fraction and let the final
moving source position be measured from the primitive state field.

Classify `ATTACHED_COMPONENT` only if, at both volumes:

- at least four production movement events occur and no reaction event occurs;
- final near activity about the current source is at least `50%` of the tick-12
  near activity;
- final radial alignment about the current source is at least `0.50`;
- observer neutrality is exact.

Classify `TRAILING_WAKE_COMPONENT` only if, at both volumes, final trailing
activity is at least `15%` of total activity and at least twice the leading
activity.  The two classifications may coexist.  If attachment fails while a
wake passes, the field is not licensed as a co-moving dressing for this moving
history.

### 4.3 Released outgoing field

Classify `RELEASED_OUTGOING_FIELD` only if, at both volumes, between ticks 12
and 24 of the source-off arm:

- mean activity radius increases by at least `2.0` lattice units;
- near fraction falls by at least `0.20` absolutely;
- exact source-free tick-invariant drift is at most `1e-10`;
- the manifested count remains zero.

This label means an outgoing source-free field disturbance.  It is not a photon
or quantized radiation claim.

## 5. Scenario admission

Add `s0-seed-dynamical-flux-dressing` only if an automated behavioral test
establishes all of the following from a fresh bridge:

- exactly one locked central `+1` state and exactly zero initial field;
- only the declared wave/coupling sector is active;
- after one tick, the field has exactly the six face-neighbour source support
  implied by `-G_C grad_c s`, with outward sign;
- subsequent evolution remains causal, finite, polarity sourced, and does not
  create or move matter.

The dashboard label and description must say *dynamical flux dressing probe*.
It must not say electron, electromagnetic aura, pilot wave, or established
radiation.  Flux lines remain integral-curve visualization geometry, not literal
substrate strings.

## 6. Failure consequences

- Failure of 4.1 closes the aura/dressing language for this frozen coupling
  sector.
- Passage of 4.1 without 4.2 licenses only a dynamically generated source field,
  not a co-moving object dressing.
- Passage of the wake clause means the moving source leaves detached/trailing
  field activity under the registered history; it does not by itself establish
  energetic backreaction.
- Passage of 4.3 licenses a classical outgoing-field candidate only.  A physical
  radiation claim still requires reciprocal source work, momentum transfer, and
  an operational detector.

## 7. Run-of-record artifacts

Before first execution, record SHA-256 hashes for the campaign source, observer,
C++ scenario source, JS scenario mirror, and this preregistration.  Write
versioned CSV/JSON/TXT results under `engine/results/ftd_0476/`.  No threshold or
estimator may be changed after the first run; any repair requires a v2 document
and preservation of v1.

## 8. Pre-run source lock

SHA-256 values recorded after successful compilation and before the first
behavioral-test or campaign execution:

| artifact | SHA-256 |
|---|---|
| campaign source | `AA873C8B7151C2432607D708344185BE3EED158DB5081EE43FEF6AF3FE0401BA` |
| scenario admission test | `824EDF8A12ADE1D6EFE5DD4A242DEAA80243B561B55D734BE15BC051E4643FC4` |
| read-only observer | `0B6219344470E61EBB9008BBD41D74D56EDF3A70F742296538B01B7B69C3D31E` |
| C++ scenario source | `A5FE3FD6F56E269F831E7C6806E1919D588F83EA52B64EFE0B49BA6BF52FAF6A` |
| JavaScript scenario mirror | `F9FEDF2D89E8027D6879726C9FCEF145B695E4E69B9004FBB769CD2096A266BE` |
| this preregistration, pre-lock-table | `DFE5E49E62424E37969F3A209F9D085A146B832821FA8E34EC1E30ECAE58D0F5` |

## 9. Preserved v1 execution defect

The first execution produced the preserved artifacts
`dynamical_flux_dressing_v1.csv` and `verdict_v1.txt`.  The field morphology,
state histories, exact energies, mirror test, and observer-neutrality test were
valid, but the movement-event count was not.  The history journal is cleared at
the beginning of each production tick, while v1 read it only after all 24 ticks.
It therefore reported zero events despite the source's six recorded face hops.

The v1 movement classification is invalid.  No threshold, estimator, engine
rule, scenario state, or field value is changed in v2.  The sole repair is to
accumulate immutable journal events immediately after each completed tick.
