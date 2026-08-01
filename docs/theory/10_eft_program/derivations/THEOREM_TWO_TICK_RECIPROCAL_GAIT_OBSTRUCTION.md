# FTD-0714 — Two-tick reciprocal-gait obstruction

**Status:** `[THEOREM — FROZEN CONNECTED COMMON-ACTION KINEMATICS]`  
**Production status:** unchanged  
**Inputs:** FTD-0713 and the selected production-dispersion discrete gradient

## Statement

For the current connected common action, a labeled constituent whose complete
momentum returns after two ticks cannot follow unequal first- and second-tick
displacements. Consequently the nonzero FTD-0713 midpoint gait, although it
cancels the field nullspace kinematically, cannot itself be a two-tick complete
relative orbit of the frozen action.

## Proof

The constituent energy and one-tick discrete-gradient velocity are

\[
H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},
\qquad
V(p,q)=\frac{C_{\rm SPEED}^2(p+q)}{H(p)+H(q)}.
\]

Both numerator and denominator are symmetric under exchange of the endpoint
momenta, so

\[
\boxed{V(p,q)=V(q,p).}
\]

For a two-tick complete relative orbit translated by one site, labeled-state
return requires `p_2=p_0`. Therefore

\[
x_1-x_0=V(p_0,p_1),
\qquad
x_2-x_1=V(p_1,p_2)=V(p_1,p_0)=V(p_0,p_1),
\]

and hence

\[
\boxed{x_1=\frac{x_0+x_2}{2}.}
\]

The FTD-0713 family instead has

\[
x_1-x_0=\tfrac12\hat x+\delta_a,
\qquad
x_2-x_1=\tfrac12\hat x-\delta_a.
\]

Equality of the two increments forces `delta_a=0` for every labeled
constituent. FTD-0713 records

```text
max_a ||delta_a||_infinity = 0.055089412116501112
```

so its gait is excluded as a two-tick action orbit. More quantitatively, the
two requested velocities differ by `2 delta_a`; any single equal velocity
approximating both incurs worst-tick infinity error at least

```text
0.055089412116501112.
```

This is more than eight orders of magnitude above the `1e-9` relative-orbit
gate.

## Boundary

The theorem does not exclude:

1. a period of at least three ticks, for which the endpoint pairs
   `(p_0,p_1)`, `(p_1,p_2)`, and `(p_2,p_0)` need not give equal velocities;
2. a translated orbit that permutes indistinguishable constituent labels;
3. creation/annihilation or a changing constituent graph;
4. an additional internal phase variable;
5. a nonperiodic causal formation history with radiation.

It does exclude repairing the `v=1/2`, two-tick orbit merely by installing the
FTD-0713 midpoint current while leaving the frozen labeled state and
discrete-gradient kinematics unchanged.

## Ontological consequence

Within the current candidate ontology, non-rigid mobile matter requires an
internal temporal cycle, a constituent permutation, or a nonperiodic history.
The first existing-variable candidate is a period-`q>=3` momentum/shape cycle.
This is a deduction about selected dynamics, not a derivation of physical spin,
generation number, or a universal ternary clock.

