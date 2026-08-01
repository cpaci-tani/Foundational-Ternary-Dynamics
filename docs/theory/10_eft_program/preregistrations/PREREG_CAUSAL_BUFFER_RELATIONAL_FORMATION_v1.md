# FTD-0736 — Causal-buffer relational-formation discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-29  
**Parents:** FTD-0730, FTD-0731, and FTD-0735  
**Scope:** observer-only selected dynamics; no production rule, field equation,
binding law, coefficient, state type, scenario, or physical target changes

## 1. Question

FTD-0731 observed entry, exit, and later re-entry of an initially unbound
opposite-polarity pair on periodic quotients `L=33,65`. FTD-0735 established
regular finite-time neighborhoods around already captured states. Neither
result excludes the possibility that the formation sequence is sustained by
field disturbance returning through periodic identification.

Repeat the formation history on `L=129` while measuring the exact spatial
support of every deposited face-current segment. The finite matched curl and
adjoint-curl stencils enlarge disturbance support by at most one lattice site
per tick. If all current sources remain within periodic Chebyshev radius
`R_s=8` of the fixed matter center, no disturbance emitted by one source copy
can reach the core of another periodic copy before

```text
T_contact = L - 2 R_s = 129 - 16 = 113.
```

The locked observation horizon is `T=112`. Re-entry and continued negative-
energy graph membership before tick 113 therefore test local relational
formation without a returning emitted disturbance. This does not remove the
global periodic static dress present at tick zero; comparison with the frozen
`L=33,65` transition sequence tests sensitivity to that static environment.

## 2. Frozen dynamics

- `L=129`, `dt=1/4`, selected `DerivedCompactPair` binding law;
- well depth `D=0.01`, cutoff squared `3/2`;
- matched face-electric and edge-magnetic fields;
- measured face-flux normalization already used by FTD-0722--0735;
- sparse exact current, local residual evaluation, solve tolerance `2e-14`,
  gate tolerance `1e-10`, and at most 384 nonlinear iterations;
- unbound initial separation `1.30`, opposing momenta `p=0.0120`;
- bound control separation `1.00`, opposing momenta `p=0.0150`;
- no legacy force, damping, reaction, collision, boundary absorption, field
  rescaling, source retuning, or post-hoc correction.

The initial field is the unchanged minimum-energy longitudinal redress on the
same quotient. It is not truncated or reinterpreted as a causal field.

## 3. Locked matrix

Run five complete histories:

1. `plus_minus`, face ray `<001>`, unbound;
2. `plus_minus`, edge ray `<01-1>`, unbound;
3. `plus_minus`, body ray `<111>`, unbound;
4. `minus_plus`, body ray `<111>`, unbound polarity control;
5. `plus_minus`, face ray `<001>`, initially bound control.

The opposite-polarity body arm is the held-out conjugation check. Exact charge
conjugation for the other rays is already covered by FTD-0722--0735 and is not
re-sampled here.

Run 112 forward steps and 112 state-only inverse steps for every history.
Persist every forward scalar state and every forward/reverse action residual.

## 4. Causal-support and algebra gates

For every sparse face-current entry, assign the conservative source radius

```text
r_s = 1 + max(periodic_abs(x-c_x),
              periodic_abs(y-c_y),
              periodic_abs(z-c_z)).
```

The added unit covers the staggered face offset. Require the maximum over all
constituents and ticks to be at most 8. Require `T < L-2R_s` from the measured
maximum as well as from the locked cap.

Every forward and reverse step must satisfy:

- valid implicit root and common-action gate;
- maximum registered action residual `<=1e-10`;
- matter recoil defect `<=1e-9`;
- pair-plus-field energy defect `<=1e-8` over the complete history;
- causal-speed excess zero within `1e-12`;
- final state-only inverse recovery `<=1e-8`.

## 5. Formation and receiver gates

For each unbound history require:

- positive initial pair internal energy and initial graph exclusion;
- exactly three graph transitions through tick 112;
- transition ticks within two ticks of the frozen FTD-0731 sequence:
  `7,26,63` for `<001>`, `7,26,79` for `<01-1>`, and `7,26,96`
  for `<111>`;
- graph membership and pair internal energy `<-1e-6` at every stored state
  from the third transition through tick 112;
- positive field-energy gain greater than `1e-6`;
- at one of ticks `48,96,112`, an instantaneous-static-dress residual with
  norm `>1e-8`, magnetic energy `>1e-10`, and doubled median radius at least 5.

The bound control must remain graph-inside with pair internal energy
`<-1e-6` for every stored state and have no graph transition.

The two body-diagonal polarity histories must have identical transition ticks
and maximum scalar-history difference `<=1e-9` for separation, pair energy,
and total field energy.

## 6. Locked verdicts

- Any initialization, matrix, causal-support, root, action, energy, recoil,
  speed, inverse, or morphology-observer failure:
  `CAUSAL_BUFFER_RELATIONAL_FORMATION_EXECUTION_INVALID`.
- Algebra and support pass but the bound control releases:
  `CAUSAL_BUFFER_BOUND_CONTROL_UNSTABLE`.
- The body polarity histories disagree:
  `CAUSAL_BUFFER_FORMATION_POLARITY_SENSITIVE`.
- Any unbound ray lacks the registered third transition before contact:
  `NO_PRECONTACT_RELATIONAL_FORMATION_ALL_RAYS`.
- Re-entry occurs but any core releases again or becomes nonnegative before
  tick 112:
  `PRECONTACT_REENTRY_WITHOUT_PERSISTENT_CORE`.
- Every core gate passes but the registered dynamic-field receiver threshold
  does not:
  `PRECONTACT_CORE_WITHOUT_QUALIFIED_FIELD_RECEIVER`.
- Every gate passes:
  `CAUSAL_BUFFER_RELATIONAL_FORMATION_CONSTRUCTIVE`.

## 7. Interpretation boundary

A constructive result establishes that the selected constituent/current/
face-edge dynamics can form and temporarily maintain the registered
relational core before emitted disturbance can return through periodic
identification. It falsifies periodic-return support as the cause of the first
registered re-entry.

It does not establish an invariant or asymptotic basin, attraction, an
uncontained/open-support solution, a production-law particle, physical mass,
charge, spin, statistics, quantum behavior, or indefinite persistence. The
tick-zero static dress remains quotient-defined. A constructive result opens
a later outgoing-tail or domain-of-dependence construction; a negative result
does not by itself force a new primitive.
