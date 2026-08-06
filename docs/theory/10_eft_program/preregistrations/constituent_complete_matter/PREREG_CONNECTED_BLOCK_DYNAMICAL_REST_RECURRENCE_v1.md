# FTD-0627 — Connected-block dynamical-rest recurrence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0626 JSON SHA-256
`DEDFF2C31C510A7944CF5FD213E1165172342324B6C38432D599F4F212570308`  
**Scope:** long-horizon classification of the fibre-enabled zero-centre-
momentum background  
**Date:** 2026-07-27

## 1. Question

FTD-0626 finds a centre-rest state whose constituents undergo a small exactly
reversible internal response. This campaign asks:

> Is that response bounded and spectrally organized over 256 ticks, does the
> complete matter-plus-field state recur, or does it develop delayed drift,
> fibre overload, or internal instability?

The campaign does not assume a physical clock, particle, ground state, or
quantum mode. “Dynamical rest” means zero macroscopic centre motion with
bounded internal evolution.

## 2. Frozen state and action

Use the unchanged FTD-0626 exact-half initialization and selected action:

- `L=17`, `w=2`, 16 `+1/-1` constituents, 72 reference-Moore bonds;
- phase exactly `1/2`, zero constituent momentum;
- minimum-energy Gauss field and zero magnetic half-field;
- `kappa=1`, `dt=1`, `C_SPEED=1/sqrt(3)`;
- unchanged production dispersion, quadratic coats, straight face currents,
  matched field update, binding, normalization, tolerances, and 48-iteration
  common-action solve;
- `allow_shared_anchor_chart=true`, maximum multiplicity two.

No damping, launch momentum, reaction, annihilation, graph change, force,
field correction, fitted coefficient, altered tolerance, or new state variable
is admitted.

## 3. Locked arms

Run two arms in parallel:

1. base orientation/phase axis `x`;
2. cyclic orientation/phase axis `y`.

Each arm receives 256 forward transactions followed by 256 state-only inverse
transactions. Total: 1,024 common-action solves. No failed arm may be replaced
or shortened.

## 4. Fixed observables

At every forward endpoint record:

- complete-state distance from tick zero;
- centre displacement and total matter momentum;
- centre-subtracted constituent position/momentum distance from tick zero;
- maximum labelled shape displacement and maximum bond strain;
- anchor multiplicity, shared-pair count, and minimum shared separation;
- kinetic, binding, field, and total energy;
- signed mean bond-strain coordinates
  `Q_r=mean_(e:rest_squared=r)(|X_i-X_j|^2-r)` for `r=1,2,3`;
- polarity-interface coordinate along the body axis;
- all common-action residuals and translation-reaction diagnostics.

The spectral record is fixed before execution. For each mean-subtracted
sequence `Q_1,Q_2,Q_3` and the interface coordinate, compute the unwindowed
256-point discrete Fourier powers at bins `k=1,...,128`. Record the eight
largest bins and

`C_8 = (power in the eight largest bins)/(total nonzero-bin power)`.

Zero-power sequences are recorded as such and excluded from concentration
claims. No frequency range, smoothing, or window may be selected after the
run.

## 5. Exactness and boundedness gates

Every forward and reverse transaction must pass the unchanged common-action
gate at `1e-10`. Across each arm require:

- total-energy drift `<=1e-8`;
- final state-only recovery `<=1e-8`;
- centre displacement and centre momentum `<=1e-8` at every tick;
- labelled shape displacement `<=1e-2`;
- squared-edge strain `<=3e-2`;
- maximum anchor multiplicity two and minimum shared separation `>=0.90` cell;
- exact constituent count, charge order, and graph fingerprint;
- cyclic scalar/vector histories agree under the registered rotation within
  `1e-8`.

These boundedness envelopes are fixed from the FTD-0626 maxima with more than
a factor-seven margin. They are research gates, not experimental bounds.

## 6. Recurrence and spectrum classifiers

For each integer `P` with `16<=P<=128`, call `P` a complete-state period
candidate only if both tick `P` and tick `2P` are within `1e-6` of the initial
complete state. Base and cyclic arms must select the same smallest `P` and
agree after rotation within `1e-8`.

Call the bounded response spectrally concentrated only if every nonzero fixed
sequence has `C_8>=0.90` and the base/cyclic top-eight bin sets agree exactly.
This is a finite-record classifier, not proof of mathematical quasiperiodicity.

## 7. Verdicts

- `CONNECTED_BLOCK_PERIODIC_DYNAMICAL_REST_CONSTRUCTIVE`: every exactness and
  boundedness gate passes and a common complete-state period candidate exists.
- `CONNECTED_BLOCK_BOUNDED_SPECTRAL_DYNAMICAL_REST_CONSTRUCTIVE`: boundedness
  and reversal pass, no complete-state period closes, and the fixed spectra are
  concentrated.
- `CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN`: boundedness and reversal pass,
  but neither recurrence nor spectral-concentration conjunction closes.
- `CONNECTED_BLOCK_DYNAMICAL_REST_CLOSED_NEGATIVE`: the campaign is executable
  but a boundedness, fibre, or inverse gate fails.
- `CONNECTED_BLOCK_DYNAMICAL_REST_EXECUTION_INVALID`: initialization,
  coverage, parent fingerprint, numerical record, or covariance is invalid.

No verdict establishes a physical particle, Compton clock, quantum energy
level, mass, spin, statistics, photon, Lorentz recovery, or production rule.
Only a periodic verdict licenses Floquet analysis; a bounded spectral verdict
licenses longer recurrence and small-amplitude normal-mode probes without the
word Floquet.
