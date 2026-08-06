# FTD-0664 — Volume-scaled internal-mode transfer v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only finite-volume discriminator  
**Parent:** FTD-0662 constructive transfer and FTD-0663 field-band embedding

## 1. Question

Is the early loss of action from the first internal matter doublet and the
outward growth of its dynamic face/edge field residual already present before
a causal disturbance can wrap around the periodic lattice, or is the FTD-0662
observation primarily a small-box echo?

This protocol does not attempt an infinite-volume pole or lifetime fit. It
tests the logically prior condition: volume-independent **pre-return** transfer.

## 2. Frozen construction

- Use volumes `L={17,25,33}`.
- Recenter the same FTD-0638 orientation-0 constituent geometry in every
  volume, preserve its charges and bond graph, and recompute the minimum-energy
  longitudinal dressing on that volume.
- Use the unchanged FTD-0640 orientation-0 first-doublet vector as one fixed
  localized perturbation. It is a source profile, not claimed to be the exact
  finite-volume eigenvector.
- Prepare the `8e-6` maximum-displacement momentum quadratures `+pi/2` and
  `-pi/2`, plus an unexcited control, in every volume.
- Evolve each excited/control pair with the unchanged selected common action
  for `4L` forward ticks and then invert both trajectories for `4L` ticks.
- No production toggle, coefficient, field rule, force, tolerance, or
  constituent state is changed.

The primary comparison window is ticks `0..16`. Because the selected update is
nearest-step causal and the smallest periodic circumference is 17 sites, a
newly generated disturbance cannot return to its source within that window.
The initial longitudinal dressing is volume dependent and nonlocal; therefore
every observable is formed as an excited-minus-control difference on the same
volume.

## 3. Frozen observers

At every tick:

1. project the excited-minus-control constituent displacement and momentum
   onto the fixed first doublet and record its quadratic energy ratio;
2. redress the instantaneous excited and control geometries separately;
3. subtract both instantaneous dressings from their actual fields, then form
   the excited-minus-control dynamic residual;
4. record its positive face/edge norm, its energy, and its radial second moment
   about the control centre;
5. record common-action residuals, complete-energy drift, sector/fibre state,
   and state-only inverse recovery for both trajectories.

A finite-volume return is recorded only after the doublet ratio has first
fallen below `0.60` and subsequently rises above `0.80` at a tick `>=L`.
Return classification is descriptive: all-volume return times with coefficient
of variation of `t_return/L <=0.25` are `SCALED_RETURN`; no return in any arm is
`NO_RETURN_IN_WINDOW`; every other pattern is `MIXED_RETURN`. It does not alter
the primary verdict.

## 4. Locked gates

Execution requires every forward/reverse root, common-action gate, redressing
observer, sector/fibre check, and recovery `<=1e-10` to pass. Complete-energy
drift and field-decomposition residual must remain `<=1e-10`.

The pre-return transfer verdict is constructive only if, for both excitation
signs:

- the three volume histories agree over ticks `0..16` with RMS residual
  `<=0.05` after normalizing doublet energy and dynamic-field energy by their
  own initial doublet energy;
- the doublet ratio at tick 16 is `<0.95` in every volume;
- the dynamic residual energy is positive at tick 16;
- its radial second moment at tick 16 exceeds its tick-4 value by at least
  `4.0` lattice-site squared.

Verdicts:

- failed execution: `VOLUME_SCALED_INTERNAL_TRANSFER_EXECUTION_INVALID`;
- all primary gates pass:
  `VOLUME_SCALED_PRE_RETURN_TRANSFER_CONSTRUCTIVE`;
- executed but one or more primary gates fail:
  `VOLUME_SCALED_INTERNAL_TRANSFER_MIXED`.

Even a constructive verdict establishes only local, pre-return, reversible
transfer from this prepared internal deformation into a dynamic field
residual. It does not establish asymptotic radiation, exponential decay, a
resonance pole, a photon, a quantum transition, or a particle lifetime.
