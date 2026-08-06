# FTD-0735 — Capture-root regularity and finite-time neighborhood v1

**Status:** `[CONDITIONAL THEOREM + NUMERICAL FACT — FINITE-TIME OPEN
NEIGHBORHOOD]`  
**Verdict:** `CAPTURE_FINITE_TIME_OPEN_NEIGHBORHOOD_NUMERICALLY_SUPPORTED`  
**Production status:** unchanged

## Result

All 18 registered captured histories and all 9,216 forward/reverse implicit
roots pass the locked regularity gates.

```text
histories                                                   18 / 18
forward plus reverse roots                              9216 / 9216
minimum measured singular value                         0.938619877
maximum measured condition number                       1.087957209
maximum h versus h/2 sigma-min difference              6.36490e-8
minimum forward energy margin in units of D             0.01199125
minimum forward graph margin                            0.02505513
maximum state-only inverse recovery                    4.15703e-11
```

The matrix contains each FTD-0734 captured center plus its locked
minimum-energy and distinct minimum-graph corner, for three lattice directions
and both polarity orders.  No new perturbation or selector was introduced.

The final-root observer computes the residual Jacobian at `h=2e-7` and `h/2`,
diagonalizes `J^T J` independently of the nonlinear solver, and never supplies
that matrix back to the solve.  A dedicated regression confirms zero endpoint
difference with the observer disabled.

## The finite-time theorem

On a finite-dimensional admissible state manifold, suppose a finite history is
defined by continuously differentiable implicit residuals

```text
R(x_t,p_{t+1}) = 0
```

whose endpoint Jacobians are nonsingular.  The implicit-function theorem gives
a locally unique continuous step map near each transaction.  The finite
composition through tick `T` is continuous.  If graph membership and negative
energy have strict margins at every stored state, their finite inverse images
have an open intersection containing the initial state.

Thus the registered histories are numerically supported members of open
finite-time capture neighborhoods relative to the fixed-count, fixed-polarity,
Gauss-admissible complete-state manifold.  They are not merely 18 isolated
passing points.

The theorem itself is exact.  Its application here remains conditional on the
finite-difference regularity measurement rather than an interval enclosure of
the exact Jacobian.  The very large separation from singularity and the
two-scale agreement make a branch fold numerically unsupported; they do not
turn floating-point evidence into an analytic determinant proof.

## Matter-dynamics consequence

The selected captured object now has three distinct demonstrated properties:

1. **formation:** an initially unbound pair enters a negative relational
   sector while the matched field receives the balancing energy;
2. **persistence and robustness:** the core remains captured under mixed
   position, three-axis momentum, and dynamic-field perturbations; and
3. **local state determinacy:** the implicit transaction stays uniformly far
   from a root singularity along both forward and state-only reverse histories.

The narrow ontological inference is that the candidate behaves as a locally
well-defined flow on relational matter--field phase space.  Existing
constituent phase space, polarity, derived event current, and face/edge field
state are sufficient for this finite behavior.  No hidden bond bit, stored
trajectory, or new connection variable is priced by the measured sector.

## Strict boundary

This result does not establish:

- an invariant open basin under arbitrarily long evolution;
- a dissipative attractor in the reversible closed system;
- persistence while outgoing field energy escapes an uncontained environment;
- a postulate-native compact interaction;
- generic formation measure, collision closure, or count-changing reactions;
- a particle pole, mass, charge, spin, statistics, or common infrared cone.

The next physical gate is environmental rather than local: keep the core in a
growing causal buffer while separating co-moving near field from outgoing
field transport.  If the core survives before any return signal can reach it,
the flame/wake ontology gains support.  If it dissolves as the field leaves,
the present capture is a finite-volume recurrent complex rather than
environmentally persistent matter.

## Verification anchors

- protocol `C8439AD7…C2A668`;
- runner `5E242010…B6DD48`;
- JSON `2927040F…3D178`;
- CSV `C924AA73…C22E9D`;
- certificate `C94EFC68…8CFE8`, `92236/92236 PASS`.
