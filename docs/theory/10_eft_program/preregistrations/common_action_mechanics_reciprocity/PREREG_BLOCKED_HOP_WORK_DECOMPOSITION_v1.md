# PRE-REGISTRATION — Blocked-hop work decomposition v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0460`  
**Status:** `[INVALID EXECUTION — OBSERVER TIMEOUT; SUPERSEDED BY v2]`  
**Parent:** `FTD-0459`  
**Engine artifact:** `engine/tests/campaign_blocked_hop_work_decomposition.cpp`

**Locked campaign SHA-256:**
`216E799FF8424129D2C1F3D6FAE2767E20CC1F6E0403D466819DE1282C290172`

## 1. Question

Which additive field history causes the FTD-0459 forward hop to fail: the
incoming localized packet, the one-time dressing, the static polarity source,
or the velocity-dependent curl source?

## 2. Frozen protocol

Reproduce the FTD-0459 `L=33`, 48-tick history through the first-event gate,
but do not solve recoil capacity or enumerate neighbours. Evolve five parallel
observer histories with the same reversible kick-drift:

1. the full packet + dressing + moving manifestation;
2. the packet alone, source-free;
3. the dressing alone, source-free;
4. a stationary `s=+1` source from zero field;
5. a moving `s=+1`, `v=(0.15,0,0)` source from zero field.

Define the velocity-curl component as history 5 minus history 4. At every tick
require exact linear reconstruction of full `J/W` from packet + dressing +
moving-source histories. At the same scheduled ticks 6--47, decompose

`W_hop = W_packet + W_dressing + W_static + W_curl`.

For each component, record its RMS work over attempted ticks and the number of
otherwise-invalid full particle updates made kinematically valid by removing
that component alone. Also record the source-free packet+dressing validity
count. No amplitude, offset, phase, speed, support, or tolerance is scanned.

## 3. Frozen gates and classification

- work-superposition residual and full field-component residual `<=1e-12`;
- reverse all five 48-tick histories to their exact initial states with maximum
  `J/W` residual `<=1e-10`;
- all values finite and exactly 42 attempts reproduced;
- a component `X` receives `X_DOMINATES_BLOCKED_HOP_WORK` only if its RMS work
  is at least twice the second-largest component RMS and removing it rescues at
  least one otherwise-invalid particle update;
- otherwise return `NO_SINGLE_COMPONENT_DOMINATES_BLOCKED_HOP_WORK`;
- any gate failure returns `PROTOCOL_INVALID`.

The dominance rule is a diagnostic selection, not a theorem of ontology.

## 4. Interpretation boundary

This audit attributes the scalar work obstruction only. It does not prove that
removing a source is physically licensed, and it does not decompose the
nonlinear constrained recoil minimum. A counterfactual kinematics rescue is a
mechanistic locator, not a repaired matter law.

## 5. Invalid first execution

The locked binary exceeded the 600-second observer timeout without emitting a
complete record. No physical classification is admitted. Inspection located
an `O(L^6)` cache-rebuild path in the observer helper; v2 preserves this file
and every registered estimator while replacing only that implementation.
