# FTD-0620 — Balanced-gait phase-return analysis

**Status:** `[MEASURED — INTERMITTENT MULTIMODE TRANSPORT]` +
`[CLOSED NEGATIVE — LOCKED 512-TICK MATTER-INTERNAL RECURRENCE]` +
`[OPEN — LONGER-TIME / AMPLITUDE-SCALING / EXTENDED-CARRIER PHASE LOCK]`
**Verdict:** `BALANCED_GAIT_PHASE_BEHAVIOR_MIXED`
**Production status:** unchanged

## 1. Result

The FTD-0618 balanced neutral gait is neither a registered recurrent internal
cycle nor a one-time decaying kick in the 512-tick window.

The locked matter-internal observer removes each core's centre position and
centre momentum, then compares all six relative constituent positions and
momenta with their launch values.  A return required both residuals below
`A/20`, where the inherited excitation amplitude is
`A=0.021500475272102783`.

No return event occurs.  The closest post-launch sample is tick 510:

```text
minimum normalized phase distance        5.22037110
registered return threshold              0.05
position return residual                  0.11224046
momentum return residual                  0.02224724
```

Thus the closest sample is still more than 100 times the registered normalized
return threshold.  The two-mode momentum angle winds by about `9.53704` radians
over the run, but that angle does not identify a return of the full internal
matter state.  A single oscillator phase is therefore an insufficient reduced
coordinate for this gait at the locked energy.

## 2. Motion is intermittent, not extinguished

The positive-sign axial increments in the four fixed 128-tick windows are

```text
1.09170740, 0.96410366, 0.21084535, 1.10214374 cells.
```

The negative arm mirrors these values.  The third window misses the locked
`0.5`-cell persistence threshold, while the fourth recovers to `1.10214`
cells.  Total displacement reaches `+/-3.36880015` cells.

This excludes the simple stories “constant-speed translator” and “motion that
merely dies away.”  Final internal-momentum norm is `0.63031418` of launch, far
above the registered 10-percent relaxation threshold, and the final window is
the fastest after the first.  The supported description is intermittent
multimode transport or a period longer than the observed window.

## 3. Exactness and symmetry

All 2,304 forward/reverse common-action transactions pass.  Worst values are

```text
common-action residual          2.00e-13
energy drift                    7.24e-14
state-only recovery             9.83e-10
active-sign mirror residual     1.91e-9
```

The compact geometry and multiplicity-two chart remain admissible.  The mixed
verdict is therefore dynamical rather than a solver failure or sign asymmetry.

## 4. Ontological consequence

The already present constituent phase space contains more dynamical structure
than one centre coordinate plus one gait angle.  No new primitive is forced by
this result: the missing coordinates are visible in the existing relative
positions and momenta.  What fails is the proposed low-dimensional reduction.

The motion currently resembles a nonlinear internal beat: translation is fast,
then weak, then fast again while internal shape and momentum traverse different
parts of phase space.  Calling this a stable particle clock, periodic swimmer,
or inertial pole would overstate the measurement.

The next compact discriminator is an amplitude ladder with fixed modes.  It
must test whether the slow window is a beat node with a phase timescale that
scales regularly, or irregular mode mixing.  In parallel, the higher-value
ontology gate remains the native extended-carrier test: a coherent wider
object could average the compact Peierls force and phase beats, but that must be
constructed rather than assumed.

