# FTD-0605 — Full mirrored internal-shape matter core v1

**Status:** `[THEOREM — EXACT RANK-THREE BINDING HESSIAN] + [MEASURED —
LOCAL SHAPE BASIN HAS NO INTERIOR STATIONARY CORE] + [CLOSED NEGATIVE —
REGISTERED COMPACT LOCAL DEFORMATION]`  
**Protocol:**
[`PREREG_FULL_MIRRORED_INTERNAL_SHAPE_CORE_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_FULL_MIRRORED_INTERNAL_SHAPE_CORE_v1.md),
SHA-256 `388926B3947F0C0A378FC3B52BD99E3C94D8F9BBB0A4D325E26CE1252B79C70F`  
**Verdict:** `FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE`

## Exact internal binding theorem

Write the zero-centroid trimer offsets as `r_0(q)`, `r_1(q)`, and
`r_2(q)=-r_0(q)-r_1(q)` for `q in R^6`. With two mirrored trimers, the selected
distance binding is

\[
 V(q)=\frac12\sum_{a<b}
 \left(\lVert r_a(q)-r_b(q)\rVert^2-2\right)^2.
\]

At the registered equilateral reference shape, exact differentiation gives

\[
H_V=
\begin{pmatrix}
20&4&16&4&-4&8\\
4&8&-4&-4&4&-8\\
16&-4&20&8&-8&16\\
4&-4&8&8&4&4\\
-4&4&-8&4&20&-16\\
8&-8&16&4&-16&20
\end{pmatrix}.
\]

It is a sum of three exact outer products and has rank `3`. Its spectrum is

\[
\{0,0,0,24,36-12\sqrt3,36+12\sqrt3\}.
\]

Thus the selected central distance potential supplies three restoring strain
modes and exactly three soft rigid-rotation modes. This is not a numerical
accident and not a property of matter generally; it follows from the chosen
distance-only binding.

## Locked campaign result

All 32 phase arms were attempted from the same undeformed state without warm
starts. Twenty-nine exhaust the locked 900-evaluation budget without meeting
the simplex termination gate. The three returned candidates, at phases
`15/32`, `16/32`, and `19/32`, all reach the registered component boundary
`max|q_i|=0.20` and retain downhill gradients between `5.56e-5` and
`7.06e-5`, far above the `5e-7` stationarity gate.

Their internal distances remain in the narrow interval
`1.4140108..1.4144011`, close to `sqrt(2)`. The motion toward the boundary is
therefore predominantly orientational rather than a collapse or large strain.
The registered local chart does not contain a stationary dressed core.

The accelerated Green kernel is not the cause: its construction residual is
`5.43e-16`, and the three returned candidates agree with independent direct
Gauss solves in energy to `2.22e-17` or better. The common-action residuals of
the three executable boundary arms are below `4.20e-15`, and state-only
inversion closes below `1.78e-15`. Those checks validate the evaluator and
transaction where executed; they do not supply the missing 32-phase static
coverage.

## Correct statement

The registered compact local six-coordinate matter core is closed negative
because it has no interior stable dressed configuration. FTD-0605 does not
show that arbitrary orientation fails: its boundary result instead requires a
new global orientation chart, such as `SO(3)` factored from the three strain
coordinates, with no artificial component boundary. That is a new
preregistered observer, not a widening of this failed protocol.

If global orientation also fails to produce phase-robust stationary matter,
the next live carrier is a spatially extended low-momentum pattern. No
production, particle, electromagnetic, pole, Lorentz, or unitarity claim
follows.

