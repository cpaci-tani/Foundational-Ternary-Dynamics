# FTD-0718 — Period-three field-bound common-action selector v1

**Status:** `[CLOSED NEGATIVE — LOCKED ORBIT / STRICTLY CO-MOVING FIELD FAMILY]`  
**Verdict:** `PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT`  
**Production status:** unchanged

## Result

The complete registered divergence-free homogeneous field family cannot supply
the constituent impulses of the locked FTD-0715 orbit.

```text
force equations                         144
real co-moving field basis columns     1094
force-response rank                      35
target impulse norm                0.826135
projected impulse norm             0.321943
maximum remaining vector residual  0.340904
minimum-norm coefficient L2      8.342389e9
maximum correction coefficient   3.727730e8
```

The deficit is not a shortage of field coefficients.  Although 1,094 real
source-free, divergence-free modes satisfy exact three-tick translated return,
their orbit gathers span only 35 independent directions in the 144-dimensional
constituent-impulse space.  The registered target has a large component outside
that image.

The minimum-norm least-squares vector is also physically unusable.  Its norm is
`8.34e9`, so roundoff-level kernel defects are amplified into C++ replay
residuals of order `1e-7`, the field energy becomes enormous, and the force
residual remains `0.351`.  This ill conditioning is secondary: the `0.341`
unreachable force component already closes the candidate.

## What is closed

The following conjunction is closed for the registered 16-constituent orbit:

1. the FTD-0715 positions and endpoint momenta are held fixed;
2. the field is the FTD-0716 particular solution plus a divergence-free
   source-free homogeneous correction;
3. the full field returns as an exact translated copy every three ticks;
4. no explicit binding force acts;
5. the correction is selected by the locked minimum-norm rule.

Adding another hand-selected resonant mode cannot repair this result: every
mode in the allowed family was already included.

## What is not closed

This result does not close composite matter, period-three internal phase, or
the face/edge field ontology.  In particular it does not test:

- a trajectory solved simultaneously with the field rather than prescribed;
- unlabeled constituent permutation after a cycle;
- a metastable dressing with a detached wake or radiative tail instead of
  exact co-moving return;
- causal formation of the matter pattern from field dynamics;
- a binding term derived from an additional part of the same action.

No new primitive is licensed.  The negative result identifies the overconstraint:
the imposed orbit and exact co-moving recurrence were chosen before local
force balance.

## Matter-dynamics consequence

The viable ontology has narrowed from “a rigid object carrying a field” to a
spatiotemporal process.  A candidate matter state must be a self-selected
orbit of the joint matter–field map.  Its internal path cannot be designed for
source compatibility and then dressed afterward.  Motion, binding, current,
and field response must select one another.

The highest-value next existing-variable test is therefore a simultaneous
period-three relative-orbit root with these unknowns:

\[
(x_{a,t},p_{a,t},E_t,B_{t+1/2}),\qquad t=0,1,2,
\]

subject to exact current deposition, field update, discrete-gradient
kinematics, local force balance, translated set return, Gauss, causality, and
one algorithmic minimum-action selector.  Constituent labels must not be made
ontic: the endpoint condition should permit polarity-preserving permutations.

In parallel, the exact-return assumption should be challenged with a finite
energy outgoing-tail condition.  A stable particle would then be the compact
recurrent core; radiation and wake would be physically distinct field content,
not a failed dressing residual.

## Provenance

- protocol: `EAC3AF44...CCED10`
- C++ runner: `AD62583D...C85528`
- selector: `6527742A...83113`
- force seed: `29FC20F7...59516`
- correction: `D15DFBE...30298`
- solve record: `50F65F58...97832`
- replay record: `1108F991...9A368`

