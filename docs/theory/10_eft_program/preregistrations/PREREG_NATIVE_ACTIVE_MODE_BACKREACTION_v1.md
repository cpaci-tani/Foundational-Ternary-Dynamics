# Pre-registration — Native active-mode backreaction (FTD-0582)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0476, FTD-0574, FTD-0578, FTD-0580, FTD-0581.  
**Production changes permitted:** none. Observer code, source audit, exact proof,
test, theorem, audit, and documentation reconciliation only.

## 1. Question

FTD-0581 proved that stable passive dressing cannot remove the FTD-0580 chord
barrier. The remaining frozen-variable candidate is an active native `(J,W)`
mode carrying at least one barrier of internal excitation and transferring it
to manifested momentum with a recurrent phase.

This campaign asks the prior question that must close before phase locking can
be studied:

```text
Does the frozen production tick contain any native J,W -> velocity/remainder
backreaction path when every selected legacy force branch is disabled?       (1)
```

No common-action branch, force, toggle, source, movement rule, scenario, field
subtraction, counterterm, or hidden state may be added.

## 2. Source-graph theorem target

Audit the production tick and prove the following scoped dataflow statement:

1. `phase_read` and `phase_write` update `flux` and `wave_vel`; the coupling
   source reads manifested `state` and stored `velocity` but neither phase
   writes manifested `velocity` or `remainder`.
2. The ordinary collision-free movement rule advances
   `remainder += velocity*dt` and changes anchor only when a remainder
   component crosses `+/-1`.
3. The only field-dependent write to ordinary manifested velocity in the
   frozen tick occurs inside `phase_forces`, which is guarded by
   `toggles.forces` and consists of selected Poisson/gradient/Lorentz/gravity/
   color mechanisms rather than the FTD-0574--0580 common action.
4. Reaction, collision, boundary, strong-energy, damping, and external-drive
   branches are disabled in the registered arms.

It follows by induction that a collision-free carrier initialized with
`velocity=remainder=0` stays at exactly the same manifested site for every
tick, regardless of the native `(J,W)` history, while `forces=false`.

This is a theorem about the current source graph, not a theorem that no
reciprocal extension can exist.

## 3. Registered dynamic arms

### 3.1 Active native field arms

- `L in {17,33}`;
- polarity `s in {-1,+1}`;
- spatial mode directions `<100>`, `<110>`, `<111>` at `n=1`;
- field phases `0, pi/2, pi, 3pi/2` represented by the registered `(J,W)`
  normal-mode initial data;
- initial native field-energy ratios `E_field/Delta_max in {2,8,32}`, where
  `Delta_max` is the largest locked FTD-0581 barrier;
- 128 ticks per arm;
- periodic field boundary, CPU path, wave propagation, state--flux coupling,
  and ordinary movement ON;
- forces, gravity, Poisson, Lorentz, emergent force, damping, Gauss projection,
  dual substrate, genesis, evaporation, pair production, weak transmutation,
  strong-energy projection, clocks, Langevin, and all drives OFF.

Total: 144 arms and 18,432 field-evolution ticks.

For every arm require:

- initial native tick energy reproduces its registered target within `1e-12`;
- the primitive field hash changes, proving a nontrivial dynamic history;
- the unique manifested state and anchor remain exact;
- `velocity`, `remainder`, and movement-event count remain exactly zero;
- both polarities and cubic-equivalent directions satisfy the same null
  backreaction verdict.

### 3.2 Sensitivity controls

- 12 ballistic arms: both volumes, both polarities, and the three directions,
  initial speed `0.5*C_SPEED`, 24 ticks. Require at least three legitimate
  movement events, unchanged speed in the isolated sector, and no reactions.
- four selected-force controls: both volumes and polarities, asymmetric
  tier-2 field density, `forces=true` and `emergent_forces=true`. Require a
  nonzero velocity response with polarity-mirrored sign. These controls do not
  qualify the selected force as a common-action mechanism.
- six coupling controls: source-present versus source-absent field histories
  with otherwise identical initial data. Require different field hashes, so
  the test cannot pass because coupling or field evolution was accidentally
  disabled.

## 4. Exact proof and source lock

The independent proof must:

- verify the no-write source graph against the hash-locked production files;
- prove the zero-velocity/remainder induction;
- prove the `+/-1` threshold cannot fire from the zero invariant;
- classify every velocity write outside the force phase as transport,
  collision/boundary reset, causal projection, or reaction-adjacent handling,
  all excluded by the registered domain.

All production hashes must remain those frozen by FTD-0581.

## 5. Outcome map

If the source proof and all dynamic/sensitivity gates pass, record

```text
FROZEN_NATIVE_FIELD_IS_ONE_WAY_TO_MATTER_ACTIVE_TRAVERSAL_CLOSED
```

This closes a phase-carrying native `(J,W)` excitation as a reciprocal mover
in the current production ontology because no field-to-momentum channel
exists. It does not close a newly derived common-action extension. Under the
face-flux plan, however, the failure rule applies: no FTD-0481 production
toggle or scenario is licensed for the frozen tick.

## 6. Frozen production provenance

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
phase_forces.cpp                F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322
phase_movement.cpp              6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB
render_bridge.cpp               A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```
