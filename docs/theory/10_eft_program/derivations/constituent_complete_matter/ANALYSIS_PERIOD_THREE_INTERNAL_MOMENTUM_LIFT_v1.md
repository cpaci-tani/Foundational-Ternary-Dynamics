# FTD-0715 — Period-three internal-momentum lift v1

**Status:** `[SELECTED KINEMATICS — CONSTRUCTIVE]`  
**Verdict:** `PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE`  
**Production status:** unchanged

## Result

The causal FTD-0713 deformation admits an exact period-three momentum lift
under the unchanged production dispersion. For each of the 16 labeled
constituents the registered trajectory is

\[
u_{a0}=\tfrac13\hat x+\delta_a,
\quad u_{a1}=\tfrac13\hat x-2\delta_a,
\quad u_{a2}=\tfrac13\hat x+\delta_a.
\]

The center advances by exactly `1/3` site on each tick, while the relative
shape follows

\[
0\longrightarrow\delta\longrightarrow-\delta\longrightarrow0.
\]

All 48 segment velocities were lifted to periodic momenta satisfying the exact
discrete-gradient relation. The locked numerical results are

```text
maximum velocity residual        8.8057339198144291e-13
maximum segment speed            0.38848308619501171
C_SPEED                           0.5773502691896258
maximum phase edge deformation   0.059942630577461076
maximum center residual          5.5511151231257827e-17
maximum non-rigid segment offset 0.1110265266349572
maximum work residual            2.7755575615628914e-17
impulse telescope residual       5.7331335576094475e-17
cubic covariance residual        1.6653345369377348e-16
mirror residual                  0
```

The solve required at most five Newton iterations. All 24 proper cubic
rotations and the direction mirror reproduced the transformed momenta.

## Why three is different from two

Near a uniform momentum, the endpoint-sum part of a period-`q`
discrete-gradient velocity map contains the cyclic matrix `I+S_q`. Its
eigenvalues are

\[
1+e^{2\pi i m/q}.
\]

For even `q`, the alternating mode `m=q/2` gives a zero eigenvalue. For odd
`q`, no such mode exists; in particular

\[
\det(I+S_3)=2.
\]

This explains the observed local lifting behavior. Period two collapses the
two requested increments to the same symmetric endpoint pair (FTD-0714),
whereas period three has a locally invertible endpoint-pair map. This does not
exclude nonlinear even-period solutions, but it makes odd internal cycles the
first structurally regular family.

## Recoil burden

The matter momentum is not constant within the cycle. Its total per-tick
impulses are

```text
tick 0  (-0.0793612065545, +0.000819063499, +0.000819063499)
tick 1  approximately zero
tick 2  (+0.0793612065545, -0.000819063499, -0.000819063499)
```

They telescope to zero over the cycle. A complete isolated solution must make
the face/edge field carry the opposite tick-resolved impulse and return with
the translated dressing. This finite `0.07937` recoil is not a defect hidden
by the observer; it is the next quantitative field-action constraint.

## Ontological reading

The result supports, but does not prove, the following candidate:

> A moving matter object is a localized recurrent orbit of constituent
> manifestation, subcell position, momentum, and face/edge dressing. A lattice
> site records one phase of that orbit; it is not the complete object.

On this reading, rest is a fixed localized orbit and translation is a
phase-locked relative orbit. Motion is not obtained by attaching momentum to
a frozen shape. The constituents deform, the field takes recoil, and the
complete pattern returns only after a temporal cycle.

This remains a selected kinematic construction. The action has not generated
the cycle, the co-moving field has not been solved, and stability has not been
tested. No universal three-generation, spin, or particle-family claim follows
from the period.

## Provenance

- protocol: `668C2D55...C2B4F9`
- summary: `210E0D6D...A36A9`
- segments: `BAB1F813...19A87`
- impulses: `1BEFF097...7FC88`
- runner: `BD582D46...C0F95`
- independent proof: `383C4389...212BC`

