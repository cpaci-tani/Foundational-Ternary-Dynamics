# FTD-0670 — Causally isolated action-envelope turning v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only large-buffer execution  
**Parent FTD-0668 JSON:**
`D1EF53978C9B04F9EEC2FF34954D7D04CA9163AAE6FAD6833D7CCF352CEAE0D2`  
**Parent FTD-0668 tick CSV:**
`E34AC8AAE7FC703B037D9F1B730A2A97213419A9A5D01996D5C9716999256FDB`

## 1. Question and prediction

FTD-0668 excludes periodic self-contact through tick 80 on `L=97`, but its
locked recurrence definition depends on an absolute `0.60` amplitude crossing.
The normalized doublet instead has strict local troughs whose late envelope
decreases through tick 72 and then rises. Because that observation was made
after the FTD-0668 verdict, it cannot grade the parent run.

Test the turning pattern at a held-out half amplitude. The prior-favoured
prediction is that both polarity signs show a unique late action-envelope
minimum at tick `71..73`, preceded by three strictly descending local troughs
and followed by two strictly ascending local troughs before tick 80. This is a
prediction of amplitude-stable coupled recurrence, not of an absolute energy
threshold.

## 2. Frozen protocol

- Use the exact FTD-0668 `L=97`, horizon `T=80`, source-radius limit `R_s=8`,
  same-volume unexcited control, recentered FTD-0638 geometry, paired modal
  normalization, selected connected-block common action, and default-off exact
  sparse current path.
- The unexcited, negative-sign, and positive-sign histories may be evaluated
  concurrently as a performance-only schedule. Each path retains its own
  state and solve cache, its within-path arithmetic/order is unchanged, and
  all verdict reductions occur serially after all three steps complete. This
  clause was added after terminating a no-output serial attempt before tick 10;
  no FTD-0670 result artifact existed.
- Use both signs of the FTD-0640 mode-6 momentum kick with maximum constituent
  momentum amplitude `4e-6`, exactly one half of the FTD-0668 amplitude. Do not
  initialize a field difference.
- Require the FTD-0668 JSON/CSV fingerprints, valid normalization/mode basis,
  bitwise-equal initial face/edge fields, sector/fibre preservation, source
  support at most eight, horizon smaller than contact tick 81, common residual
  and complete-energy drift `<=1e-10`, and state-only recovery `<=1e-8`.
- At every tick record the same doublet, field-energy/norm, near fraction,
  radial second moment, dynamic support, source support, energy, and action
  observables as FTD-0668.
- A strict local trough is a tick `t` with
  `R(t)<R(t-1)` and `R(t)<R(t+1)`, where `R` is paired doublet energy divided
  by its own tick-zero value.
- Search only strict troughs with ticks `60..79`. The primary trough `t*` is
  the member with smallest `R`; ties choose the earliest tick.
- For each sign require:
  1. `t*` lies in ticks `71..73`;
  2. the last three strict troughs before `t*` decrease strictly;
  3. the first two strict troughs after `t*` increase strictly;
  4. the second post-trough exceeds `R(t*)` by at least `0.05`;
  5. the tick-80 positive difference-field norm is nonzero, its radius-eight
     near fraction is below `0.40`, and radial second moment exceeds `300`.
- Between signs require primary-trough ticks to differ by at most one,
  primary-trough ratios to differ by at most `1e-4`, and second-post-trough
  recovery increments to differ by at most `1e-4`.

No amplitude, time range, trough count, rise margin, morphology threshold,
normalization, field coefficient, or action parameter may be changed after
viewing the `4e-6` history.

## 3. Locked verdicts

- Any parent-fingerprint, initialization, equality, locality, action, energy,
  sector, inverse, schema, or horizon failure:
  `CAUSALLY_ISOLATED_ENVELOPE_TURNING_EXECUTION_INVALID`.
- Execution passes and every within-sign and between-sign turning/morphology
  gate passes:
  `CAUSALLY_ISOLATED_ENVELOPE_TURNING_CONSTRUCTIVE`.
- Execution passes, both signs have a primary trough in ticks `71..73`, but
  one or more preregistered descent, ascent, recovery, morphology, or polarity
  controls fail:
  `CAUSALLY_ISOLATED_ENVELOPE_TURNING_MIXED`.
- Execution passes and either sign lacks a primary trough in ticks `71..73`:
  `HELD_OUT_ENVELOPE_TURNING_CLOSED_NEGATIVE`.

## 4. Interpretation boundary

A constructive verdict establishes a reproducible, amplitude-stable turning
of the projected internal-action envelope before periodic self-contact while
the difference field remains distributed. It supports a coupled local-core/
field-reservoir recurrence in the selected finite-volume action. It does not
establish an infinite-volume bound state, resonance pole, positive spectral
residue, asymptotic stability, quantum phase, particle, photon, or production
ontology. A mixed or negative verdict remains informative and may not be
repaired by changing the frozen estimator on the same data.
