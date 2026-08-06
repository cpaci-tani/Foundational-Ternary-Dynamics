# PREREGISTRATION — Causal excitation separation v3

**Identifier:** `FTD-0687`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION AND EXECUTION]`  
**Date locked:** 2026-07-28  
**Production changes:** forbidden

## Reason for the new version

FTD-0685 passed initialization and entered its first forward tick, but emitted
no checkpoint or result file before manual termination after more than twenty
minutes.  Its repeated scalar FTD-0671 masks projected completion at or beyond
the six-hour CTest limit.  No tick, shell, trajectory, or classifier value was
exposed.

V3 replaces six repeated scalar FTD-0671 calls with the FTD-0686 batched
observer only after that observer passes pointwise equivalence against
FTD-0671.  The accepted state, solver, arithmetic order inside each dynamical
path, fields, current, and physical outputs are unchanged.

## Frozen inheritance

Inherit every FTD-0685 choice without change: `L=129`, ticks `0..112`, contact
tick 113, fixed origin `(64,64,64)`, fresh `p_max=1.25e-7`, both signs and
control, six radii, threshold `0.001`, late window `88..112`, target schema,
all exact/polarity gates, and both factorized physical classifiers.

The center preflight remains `norm(center-origin)<=1e-12`.  Initial field
equality remains bitwise.  The batched observer must return one record for each
radius in the original order and each record must use the original `1e-10`
validity threshold.

## Run of record

- planned CTest: `causal_excitation_separation_v3`;
- outputs: `engine/results/ftd_0687/`;
- observer-equivalence, wrapper, embedded runner, and Release executable hashes
  must be locked before invocation;
- independent output certification remains mandatory.
