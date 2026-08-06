# FTD-0616 — Internal-walker direction and persistence

**Status:** `[MEASURED — CONTINUOUS CURVED INTERNAL TRANSPORT]` +
`[MEASURED — SIGN-PARITY SPLIT]` +
`[CLOSED NEGATIVE — STRAIGHT SIGN-CONTROLLED PERSISTENCE]` +
`[OPEN — GAIT BALANCE / PHASE RECURRENCE / ISOLATED MOMENTUM]`
**Verdict:** `INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED`
**Production status:** unchanged

## 1. Result

All twelve locked arms complete 512 forward ticks and 512 state-only inverse
ticks.  The compact core remains intact through 53 or 54 constituent-anchor
changes.  Common-action residuals stay below `2.00e-13`, energy drift below
`4.04e-14`, and state recovery below `1.47e-9`.  Whole-history proper-cubic
covariance closes at `9.62e-12`.

The walker does not stop.  Every fixed 128-tick window translates at least
`0.9161202263` cell, and each zero-rotation arm reaches
`3.31109525` cells after 512 ticks.  It nevertheless fails the registered
straight-persistence gate because successive window directions reach cosine
`0.36047462`, below `0.95`.  The locked verdict's “transient or uncontrolled”
alternative is therefore realized by **uncontrolled curved transport**, not
by disappearance of motion.

## 2. Exact empirical parity structure

For zero rotation, write the two sign trajectories as

```text
d_even = (d_+ + d_-)/2,
d_odd  = (d_+ - d_-)/2.
```

At tick 512 the measured vectors are, to the shown precision,

```text
mode 0: d_even = (1.072976479, -1.245021641, 0)
        d_odd  = (0,           0,           2.874368510)

mode 1: d_even = (1.245021643, -1.072976479, 0)
        d_odd  = (0,           0,           2.874368509)
```

The maximum sign-even axial residue is below `1.3e-9`; maximum sign-odd
in-plane leakage is below `7.3e-9`.  The internal tangent histories satisfy
`q_+(t)=-q_-(t)` and `p_+(t)=-p_-(t)` within `1e-8`, while modes 0 and 1 have
the same `(q,p)` history to that tolerance.  Cyclically rotating the complete
state rotates the complete trajectory.

Thus internal sign controls a genuine axial component, but it does not control
the whole displacement.  A body/substrate-dependent sign-even component is
superposed on it.  The sign-pair cosine is only `-0.5072`, not antipodal,
despite their equal displacement magnitudes.

## 3. Ontological consequence

The current selected compact object behaves more like a gait-bearing lattice
excitation than a point particle.  Its internal rotational phase couples to a
directed component of transport, while the oriented composite and discrete
substrate produce a second, phase-even drift.  Translation therefore depends
on at least:

1. centre position and momentum;
2. internal orientation/phase;
3. body orientation relative to the lattice;
4. the matched field state.

All four are already present in the selected constituent-complete state.  No
new primitive is forced by this result.  A reduced centre-only law would lose
the measured parity split and curvature.

## 4. Boundary of the claim

The observed motion is not yet an inertial free particle.  It is not straight,
its phase-return map has not been registered, and the sign-even drift has not
been cancelled or derived from a symmetry-reduced response law.  The uniform
neutralizer remains external and the maximum field-plus-matter
pseudomomentum defect reaches `9.13e-4`.  “Self-propulsion,” conserved total
momentum, gapless mobility, and microscopic electromagnetism remain
unlicensed.

## 5. Next derivation

The next step is a response-tensor campaign over preregistered linear
combinations of the two degenerate rotational tangents and their discrete
body mirrors.  Its purpose is not to search for a favorable direction.  It
must determine the symmetry-allowed sign-even and sign-odd transport tensors,
then test a symmetry-balanced combination predicted in advance to cancel the
even drift while retaining or cancelling the odd component.  Only after that
map is known should a closed dynamical neutral partner replace the uniform
neutralizer.

